#pragma once
/*!
 *  \file Huffman.hpp
 *  \brief Huffman 역변환 클래스
 *
 *  \author sso
 *  \date   2023/11/25
 *  \copyright Copyright 2023 sso
 */

#define WIN32_LEAN_AND_MEAN

#include <vector>
#include <Windows.h>

namespace BZIP
{
	class BZipFile;
	class Huffman final
	{
	public:
		explicit Huffman(BZipFile& file, const UINT& symbolCount, const std::vector<std::vector<BYTE>>& table, const std::vector<BYTE>& selectors);

		Huffman& operator=(const Huffman& rhs) = delete;

		~Huffman();

		bool GetNextSymbol(UINT& ret);

	private:
		BZipFile& mFile;

		std::vector<BYTE> mSelectors;
		BYTE mCurrentTable;

		std::vector<BYTE> mMinimumLengths;
		std::vector<std::vector<UINT>> mCodeBases;
		std::vector<std::vector<UINT>> mCodeLimits;
		std::vector<std::vector<UINT>> mCodeSymbols;

		INT mGroupIndex;
		INT mGroupPosition;
	};
}
