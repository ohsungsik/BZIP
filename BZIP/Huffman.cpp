#include "Huffman.hpp"

#include "BZipFile.hpp"
#include "Constants.hpp"

namespace BZIP
{
	Huffman::Huffman(BZipFile& file, const UINT& symbolCount, const std::vector<std::vector<BYTE>>& table,
		const std::vector<BYTE>& selectors) :
		mFile(file),
		mSelectors(selectors),
		mCurrentTable(mSelectors[0]),
		mMinimumLengths(Constants::Huffman::MAX_TABLE_COUNT),
		mCodeBases(std::vector(Constants::Huffman::MAX_TABLE_COUNT, std::vector<UINT>(Constants::Huffman::MAX_CODE_LENGTH + 2))),
		mCodeLimits(std::vector(Constants::Huffman::MAX_TABLE_COUNT, std::vector<UINT>(Constants::Huffman::MAX_CODE_LENGTH + 1))),
		mCodeSymbols(std::vector(Constants::Huffman::MAX_TABLE_COUNT, std::vector<UINT>(Constants::Huffman::MAX_SYMBOL_COUNT))),
		mGroupIndex(-1),
		mGroupPosition(-1)
	{
		for (size_t tableIndex = 0; tableIndex < table.size(); tableIndex++)
		{
			BYTE minLength = Constants::Huffman::MAX_CODE_LENGTH;
			BYTE maxLength = 0;

			for (UINT i = 0; i < symbolCount; i++)
			{
				maxLength = max(table[tableIndex][i], maxLength);
				minLength = min(table[tableIndex][i], minLength);
			}
			mMinimumLengths[tableIndex] = minLength;

			for (UINT i = 0; i < symbolCount; i++)
			{
				mCodeBases[tableIndex][table[tableIndex][i] + 1]++;
			}
			for (UINT i = 1; i < mCodeBases[tableIndex].size(); i++)
			{
				mCodeBases[tableIndex][i] += mCodeBases[tableIndex][i - 1];
			}

			UINT code = 0;
			for (UINT i = minLength; i <= maxLength; i++)
			{
				const UINT base = code;
				code += mCodeBases[tableIndex][i + 1] - mCodeBases[tableIndex][i];
				mCodeBases[tableIndex][i] = base - mCodeBases[tableIndex][i];
				mCodeLimits[tableIndex][i] = code - 1;
				code = code << 1;
			}

			UINT codeIndex = 0;
			for (UINT i = minLength; i <= maxLength; i++)
			{
				for (UINT j = 0; j < symbolCount; j++)
				{
					if (table[tableIndex][j] != i)
						continue;

					mCodeSymbols[tableIndex][codeIndex] = j;
					codeIndex += 1;
				}
			}
		}
	}

	Huffman::~Huffman() = default;

	bool Huffman::GetNextSymbol(UINT& ret)
	{
		if (++mGroupPosition % 50 == 0) {
			mGroupIndex++;
			if (mGroupIndex == mSelectors.size()) {
				return false;
			}
			mCurrentTable = mSelectors[mGroupIndex];
		}

		const BYTE currentTable = mCurrentTable;
		const std::vector<UINT>& limitsTable = mCodeLimits[currentTable];
		BYTE codeLength = mMinimumLengths[currentTable];

		UINT codeBits = mFile.GetBits(codeLength);
		while (codeLength <= Constants::Huffman::MAX_CODE_LENGTH)
		{
			if (codeBits <= limitsTable[codeLength]) {

				ret = mCodeSymbols[currentTable][codeBits - mCodeBases[currentTable][codeLength]];
				return true;
			}

			codeBits = (codeBits << 1) | mFile.GetBits(1);

			codeLength += 1;
		}

		return false;
	}
}
