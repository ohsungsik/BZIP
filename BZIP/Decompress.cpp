#include "Decompress.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>

#include "Constants.hpp"
#include "Huffman.hpp"
#include "Logging.hpp"
#include "MoveToFront.hpp"

namespace BZIP
{
	Decompress::Decompress(BZipFile& file) :
		mFile(file),
		mStreamHeader({}),
		mStreamBlock({}),
		mStreamFooter({})
	{

	}

	Decompress::~Decompress() = default;

	bool Decompress::Open()
	{
		LOG_INFO("스트림 헤더 파싱\n");

		mStreamHeader.mFileFlag = mFile.GetBits(2 * CHAR_BIT);
		const BYTE lowerFileFlag = mStreamHeader.mFileFlag & 255;
		const BYTE highFileFlag = (mStreamHeader.mFileFlag & (255 << CHAR_BIT)) >> CHAR_BIT;
		if (highFileFlag != 'B' || lowerFileFlag != 'Z')
			return Constants::Return::INVALID_FILE;

		mStreamHeader.mVersion = mFile.GetBits(1 * CHAR_BIT);
		if (mStreamHeader.mVersion != 'h')
		{
			LOG_WARNING("BZip1 파일은 지원되지 않습니다");
			return Constants::Return::INVALID_FILE;
		}


		mStreamHeader.mLevel = mFile.GetBits(1 * CHAR_BIT) - '0';
		if (mStreamHeader.mLevel < Constants::MIN_BLOCK_SIZE || mStreamHeader.mLevel > Constants::MAX_BLOCK_SIZE)
			return Constants::Return::INVALID_FILE;

		LOG_INFO("스트림 헤더 파싱 성공\n");
		return Constants::Return::VALID_FILE;
	}

	int Decompress::Read(std::vector<BYTE>& buffer)
	{
		assert(buffer.empty() == false);

		INT readCount = -1;

		if (mStreamBlock.mInitialized == true)
		{
			readCount = ReadBlock(buffer);
		}

		if (readCount < 0)
		{
			if (readCount == Constants::Return::END_OF_BLOCK)
			{
				const UINT blockCRC = mStreamBlock.mCRC.GetCRC();
				if (blockCRC != mStreamBlock.mBlockHeader.mBlockCRC)
				{
					LOG_WARNING("잘못된 블록 CRC\n");
					return Constants::Return::INVALID_CRC;
				}

				mStreamFooter.mCurrentStreamCRC = (mStreamFooter.mCurrentStreamCRC << 1 | mStreamFooter.mCurrentStreamCRC >> 31) ^ blockCRC;
			}

			mStreamBlock = {};
			LOG_INFO("다음 블록 파싱 시작\n");
			if (InitializeBlock())
			{
				readCount = ReadBlock(buffer);
			}
			else
			{
				if (mStreamFooter.mEOS == true)
				{
					if (mStreamFooter.mCurrentStreamCRC != mStreamFooter.mStreamCRC)
					{
						LOG_WARNING("잘못된 스트림 CRC\n");
						return Constants::Return::INVALID_CRC;
					}
				}
			}
		}

		return readCount;
	}

	bool Decompress::InitializeBlock()
	{
		bool result = ReadBlockHeader();
		if (result == Constants::Return::INVALID_FILE)
			return Constants::Return::INVALID_FILE;

		UINT endOfBlockSymbol = 0;
		std::vector<std::vector<BYTE>> trees;
		std::vector<BYTE> selectors;
		result = ReadHuffmanTrees(endOfBlockSymbol, trees, selectors);
		if (result == Constants::Return::INVALID_FILE)
			return Constants::Return::INVALID_FILE;

		Huffman huffman(mFile, endOfBlockSymbol + 1, trees, selectors);
		result = DecodeHuffmanTransform(endOfBlockSymbol, huffman);
		if (result == Constants::Return::INVALID_FILE)
			return Constants::Return::INVALID_FILE;

		result = DecodeBWT();
		if (result == Constants::Return::INVALID_FILE)
			return Constants::Return::INVALID_FILE;

		return Constants::Return::VALID_FILE;
	}

	bool Decompress::ReadBlockHeader()
	{
		LOG_INFO("블록 헤더 파싱\n");

		for (int i = 0; i < 2; i++)
		{
			const UINT blockMagicSub = mFile.GetBits(3 * CHAR_BIT);
			mStreamBlock.mBlockHeader.mBlockMagic = (mStreamBlock.mBlockHeader.mBlockMagic << (3 * CHAR_BIT)) | blockMagicSub;
		}

		if (mStreamBlock.mBlockHeader.mBlockMagic == Constants::STREAM_MAGIC)
		{
			mStreamFooter.mFooterMagic = mStreamBlock.mBlockHeader.mBlockMagic;
			mStreamFooter.mEOS = true;
			mStreamFooter.mStreamCRC = mFile.GetBits(4 * CHAR_BIT);
			return Constants::Return::INVALID_FILE;
		}

		if (mStreamBlock.mBlockHeader.mBlockMagic != Constants::BLOCK_MAGIC)
			return Constants::Return::INVALID_FILE;

		mStreamBlock.mBlockHeader.mBlockCRC = mFile.GetBits(4 * CHAR_BIT);

		mStreamBlock.mBlockHeader.mRandomized = static_cast<BYTE>(mFile.GetBits(1));
		if (mStreamBlock.mBlockHeader.mRandomized != 0)
		{
			LOG_WARNING("Randomized는 지원되지 않습니다\n");
			return Constants::Return::INVALID_FILE;
		}

		mStreamBlock.mBlockHeader.mOriginPointer = mFile.GetBits(3 * CHAR_BIT);
		if (mStreamBlock.mBlockHeader.mOriginPointer < 0 || mStreamBlock.mBlockHeader.mOriginPointer > 10 + 100000 * mStreamHeader.mLevel)
			return Constants::Return::INVALID_FILE;

		LOG_INFO("블록 헤더 파싱 성공\n");
		return Constants::Return::VALID_FILE;
	}

	bool Decompress::ReadHuffmanTrees(UINT& endOfBlockSymbol, std::vector<std::vector<BYTE>>& trees, std::vector<BYTE>& selectors)
	{
		mStreamBlock.mBWTBlock = std::vector<BYTE>(mStreamHeader.mLevel * 100000);

		const UINT huffmanUsedRanges = mFile.GetBits(2 * CHAR_BIT);
		UINT huffmanSymbolCount = 0;
		for (UINT i = 0; i < 16; i++)
		{
			if ((huffmanUsedRanges & ((1 << 15) >> i)) == 0)
				continue;

			for (UINT j = 0, k = i << 4; j < 16; j++, k++)
			{
				if (mFile.GetBits(1) != 0)
				{
					mStreamBlock.mSymMap[huffmanSymbolCount] = static_cast<BYTE>(k);
					huffmanSymbolCount++;
				}
			}
		}

		if (huffmanSymbolCount == 0)
			return Constants::Return::INVALID_FILE;

		endOfBlockSymbol = huffmanSymbolCount + 1;

		const UINT numTrees = mFile.GetBits(3);
		if (numTrees < Constants::Huffman::MIN_GROUP_COUNT || numTrees > Constants::Huffman::MAX_GROUP_COUNT)
			return Constants::Return::INVALID_FILE;

		UINT numSels = mFile.GetBits(15);
		if (numSels < 1)
			return Constants::Return::INVALID_FILE;

		if (numSels > Constants::Huffman::MAX_SELECTOR_SIZE)
			numSels = Constants::Huffman::MAX_SELECTOR_SIZE;

		MoveToFront moveToFront;
		selectors = std::vector<BYTE>(numSels);
		for (UINT i = 0; i < numSels; i++)
		{
			int index = 0;
			while (true)
			{
				const UINT bitList = mFile.GetBits(1);
				if (bitList == 0)
					break;

				index++;

			}

			selectors[i] = moveToFront.IndexOfFront(index);
		}

		trees = std::vector(6, std::vector<BYTE>(258));
		for (UINT i = 0; i < numTrees; i++)
		{
			UINT currentLength = mFile.GetBits(5);
			for (UINT j = 0; j <= endOfBlockSymbol; j++)
			{
				do
				{
					const UINT lengthList = mFile.GetBits(1);
					if (lengthList == 0)
						break;

					currentLength += mFile.GetBits(1) != 0 ? -1 : 1;
				} while (true);

				trees[i][j] = static_cast<BYTE>(currentLength);
			}
		}

		return Constants::Return::VALID_FILE;
	}

	bool Decompress::DecodeHuffmanTransform(const UINT& endOfBlockSymbol, Huffman& huffman)
	{
		MoveToFront symbolMoveToFront;
		mStreamBlock.mBWTBlockLength = 0;
		int repeatCount = 0;
		int repeatIncrement = 1;
		int mtfValue = 0;

		while (true)
		{
			UINT nextSymbol = 0;
			bool result = huffman.GetNextSymbol(nextSymbol);

			if (result == false)
				return Constants::Return::INVALID_FILE;

			if (nextSymbol == Constants::Huffman::SYMBOL_RUNA)
			{
				repeatCount += repeatIncrement;
				repeatIncrement = repeatIncrement << 1;
			}
			else if (nextSymbol == Constants::Huffman::SYMBOL_RUNB)
			{
				repeatCount += repeatIncrement << 1;
				repeatIncrement = repeatIncrement << 1;
			}
			else
			{
				if (repeatCount > 0)
				{
					if (mStreamBlock.mBWTBlockLength + repeatCount > mStreamHeader.mLevel * 100000)
						return Constants::Return::INVALID_FILE;

					BYTE nextByte = mStreamBlock.mSymMap[mtfValue];
					mStreamBlock.mBWTByteCount[nextByte] += repeatCount;

					while (--repeatCount >= 0)
					{
						mStreamBlock.mBWTBlock[mStreamBlock.mBWTBlockLength++] = nextByte;
					}

					repeatCount = 0;
					repeatIncrement = 1;
				}

				if (nextSymbol == endOfBlockSymbol)
					break;

				if (mStreamBlock.mBWTBlockLength >= mStreamHeader.mLevel * 100000)
					return Constants::Return::INVALID_FILE;

				mtfValue = symbolMoveToFront.IndexOfFront(nextSymbol - 1);

				BYTE nextByte = mStreamBlock.mSymMap[mtfValue];
				mStreamBlock.mBWTByteCount[nextByte] += 1;
				mStreamBlock.mBWTBlock[mStreamBlock.mBWTBlockLength] = nextByte;
				mStreamBlock.mBWTBlockLength += 1;
			}
		}

		return Constants::Return::VALID_FILE;
	}

	bool Decompress::DecodeBWT()
	{
		mStreamBlock.mBWTMergedPointers = std::vector<UINT>(mStreamBlock.mBWTBlockLength);
		std::vector<UINT> characterBase(256);

		if (mStreamBlock.mBWTBlockLength <= mStreamBlock.mBlockHeader.mOriginPointer)
			return Constants::Return::INVALID_FILE;

		std::copy_n(mStreamBlock.mBWTByteCount.begin(), 255, characterBase.begin() + 1);
		for (int i = 2; i <= 255; i++)
		{
			characterBase[i] += characterBase[i - 1];
		}

		for (UINT i = 0; i < mStreamBlock.mBWTBlockLength; i++)
		{
			BYTE value = mStreamBlock.mBWTBlock[i];
			mStreamBlock.mBWTMergedPointers[characterBase[value]++] = (i << 8) + value;
		}

		mStreamBlock.mBWTCurrentMergedPointer = mStreamBlock.mBWTMergedPointers[mStreamBlock.mBlockHeader.mOriginPointer];

		mStreamBlock.mInitialized = true;

		return Constants::Return::VALID_FILE;
	}

	UINT Decompress::DecodeNextBWTByte()
	{
		const UINT mergedPointer = mStreamBlock.mBWTCurrentMergedPointer;
		const UINT nextByte = mergedPointer & 0xff;
		mStreamBlock.mBWTCurrentMergedPointer = mStreamBlock.mBWTMergedPointers[mergedPointer >> 8];
		mStreamBlock.mBWTBlockReadCount += 1;

		return nextByte;
	}

	INT Decompress::ReadBlock(std::vector<BYTE>& buffer)
	{
		for (INT i = 0, offset = 0; i < buffer.size(); i++, offset++) {
			const INT decoded = ReadNextByte();
			if (decoded < 0) {
				return (i == 0) ? decoded : i;
			}
			buffer[offset] = static_cast<BYTE>(decoded);
		}
		return static_cast<INT>(buffer.size() - 1);
	}

	INT Decompress::ReadNextByte()
	{
		while (mStreamBlock.mRLELength < 1) {

			if (mStreamBlock.mBWTBlockReadCount >= mStreamBlock.mBWTBlockLength) {
				return Constants::Return::END_OF_BLOCK;
			}

			const INT nextByte = static_cast<INT>(DecodeNextBWTByte());

			if (nextByte != mStreamBlock.mRLELastDecodedByte) {
				mStreamBlock.mRLELastDecodedByte = nextByte;
				mStreamBlock.mRLELength = 1;
				mStreamBlock.mRLECount = 1;
				mStreamBlock.mCRC.UpdateCRC(nextByte);
			}
			else {
				mStreamBlock.mRLECount += 1;
				if (mStreamBlock.mRLECount == 4) {
					mStreamBlock.mRLELength = DecodeNextBWTByte() + 1;
					mStreamBlock.mRLECount = 0;
					mStreamBlock.mCRC.UpdateCRC(nextByte, mStreamBlock.mRLELength);
				}
				else {
					mStreamBlock.mRLELength = 1;
					mStreamBlock.mCRC.UpdateCRC(nextByte);
				}
			}
		}

		mStreamBlock.mRLELength--;
		return mStreamBlock.mRLELastDecodedByte;
	}
}
