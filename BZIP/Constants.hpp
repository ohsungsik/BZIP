#pragma once

/*!
 *  \file Constants.hpp
 *  \brief 상수 목록
 *
 *  \author sso
 *  \date   2023/11/26
 *  \copyright Copyright 2023 sso
 */

namespace BZIP::Constants
{
	static constexpr unsigned int MIN_BLOCK_SIZE = 1u;
	static constexpr unsigned int MAX_BLOCK_SIZE = 9u;
	static constexpr unsigned long long BLOCK_MAGIC = 0x314159265359;
	static constexpr unsigned long long STREAM_MAGIC = 0x177245385090;

	struct Huffman
	{
		static constexpr unsigned int MIN_GROUP_COUNT = 2u;
		static constexpr unsigned int MAX_GROUP_COUNT = 6u;

		static constexpr unsigned int MAX_SELECTOR_SIZE = 2u + 900000u / 50u;

		static constexpr unsigned int MAX_TABLE_COUNT = 6u;
		static constexpr unsigned int MAX_CODE_LENGTH = 23u;
		static constexpr unsigned int MAX_SYMBOL_COUNT = 258u;

		static constexpr unsigned int SYMBOL_RUNA = 0u;
		static constexpr unsigned int SYMBOL_RUNB = 1u;
	};

	struct Return
	{
		static constexpr bool INVALID_FILE = false;
		static constexpr bool VALID_FILE = true;

		static constexpr int INVALID_CRC = -2;

		static constexpr int END_OF_BLOCK = -3;
	};
}

