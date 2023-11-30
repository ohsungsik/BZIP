#pragma once
/*!
 *  \file BZipFile.hpp
 *  \brief 파일 스트림 클래스
 *
 *  \author sso
 *  \date   2023/11/24
 *  \copyright Copyright 2023 sso
 */

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <Windows.h>

namespace BZIP
{
	class BZipFile final
	{
	public:
		explicit BZipFile(std::wstring path);

		BZipFile& operator=(const BZipFile& rhs) = delete;

		~BZipFile();

		bool Open();
		UINT GetBits(const BYTE& count);

	private:
		DWORD Read(std::vector<BYTE>& buffer) const;
		
	private:
		std::wstring mPath;
		HANDLE mFileHandle;

		UINT mBuffer;
		BYTE mStoredBitsCount;
	};
}
