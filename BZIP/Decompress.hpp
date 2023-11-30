#pragma once
/*!
 *  \file Decompress.hpp
 *  \brief BZip2 파일 압축 해제 클래스
 *
 *  \author sso
 *  \date   2023/11/24
 *  \copyright Copyright 2023 sso
 */

#include "BZipFile.hpp"
#include "Type.hpp"

namespace BZIP
{
	class Huffman;
	class Decompress final
	{
	public:
		explicit Decompress(BZipFile& file);

		Decompress& operator=(const Decompress& rhs) = delete;

		~Decompress();

		bool Open();
		INT Read(std::vector<BYTE>& buffer);

	private:
		bool InitializeBlock();	
		bool ReadBlockHeader();
		bool ReadHuffmanTrees(UINT& endOfBlockSymbol, std::vector<std::vector<BYTE>>& trees, std::vector<BYTE>& selectors);

		bool DecodeHuffmanTransform(const UINT& endOfBlockSymbol, Huffman& huffman);
		bool DecodeBWT();
		UINT DecodeNextBWTByte();
		INT ReadBlock(std::vector<BYTE>& buffer);
		INT ReadNextByte();

	private:
		BZipFile& mFile;

		StreamHeader mStreamHeader;
		StreamBlock mStreamBlock;
		StreamFooter mStreamFooter;
	};
}
