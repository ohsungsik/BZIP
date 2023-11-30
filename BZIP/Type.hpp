#pragma once
/*!
 *  \file Type.hpp
 *  \brief BZip2 파일 압축 해제에 필요한 변수들
 *
 *  \author sso
 *  \date   2023/11/28
 *  \copyright Copyright 2023 sso
 */

#include <array>
#include <vector>
#include <wtypes.h>

#include "CRC.hpp"

namespace BZIP
{
	struct StreamHeader
	{
		UINT mFileFlag;
		UINT mVersion;
		UINT mLevel;
	};

	struct BlockHeader
	{
		ULONG64 mBlockMagic;
		UINT mBlockCRC;
		BYTE mRandomized;
		UINT mOriginPointer;
	};

	struct StreamBlock
	{
		bool mInitialized = false;

		BlockHeader mBlockHeader;

		std::array<BYTE, 256> mSymMap = { 0, };
		std::array<UINT, 256> mBWTByteCount = { 0, };

		std::vector<BYTE> mBWTBlock;
		std::vector<UINT> mBWTMergedPointers;
		UINT mBWTCurrentMergedPointer;
		UINT mBWTBlockLength;
		UINT mBWTBlockReadCount;

		UINT mRLELength = 0;
		INT mRLECount = 1;
		INT mRLELastDecodedByte = -1;		

		CRC mCRC;
	};

	struct StreamFooter
	{
		ULONG64 mFooterMagic;
		UINT mStreamCRC;
		UINT mPadding;

		bool mEOS = false;
		UINT mCurrentStreamCRC;
	};
}
