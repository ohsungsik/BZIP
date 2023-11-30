#include "BZipFile.hpp"

#include <cassert>

#include "Logging.hpp"

namespace BZIP
{
	BZipFile::BZipFile(std::wstring path) :
		mPath(std::move(path)),
		mFileHandle(INVALID_HANDLE_VALUE),
		mBuffer(0),
		mStoredBitsCount(0)
	{
	}

	BZipFile::~BZipFile() = default;

	bool BZipFile::Open()
	{
		mFileHandle = CreateFile(mPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, NULL, nullptr);
		if (mFileHandle == INVALID_HANDLE_VALUE)
		{
			LOG_WARNING("파일을 열 수 없습니다.\n");
			return false;
		}

		return true;
	}

	UINT BZipFile::GetBits(const BYTE& count)
	{
		assert(count <= CHAR_BIT * 4);

		UINT ret;
		if (mStoredBitsCount >= count)
		{
			const BYTE shift = mStoredBitsCount - count;
			ret = mBuffer >> shift;

			mStoredBitsCount -= count;

			const UINT mask = (1 << shift) - 1;
			mBuffer = mBuffer & mask;
		}
		else
		{
			BYTE requiredCount = count - mStoredBitsCount;

			ret = mBuffer;

			mBuffer = 0;
			mStoredBitsCount = 0;

			std::vector<BYTE> buffer(static_cast<size_t>(ceil(static_cast<double>(requiredCount) / static_cast<double>(CHAR_BIT))));
			const DWORD readCount = Read(buffer);

			assert(readCount == buffer.size());

			for (const BYTE& byte : buffer)
			{
				if (requiredCount >= CHAR_BIT)
				{
					ret = (ret << CHAR_BIT) | byte;
					requiredCount -= CHAR_BIT;
				}
				else if (requiredCount <= 0)
				{
					mBuffer = mBuffer << CHAR_BIT | byte;
					mStoredBitsCount += CHAR_BIT;
				}
				else
				{
					const BYTE shift = CHAR_BIT - requiredCount;
					BYTE temp = byte >> shift;
					ret = ret << requiredCount | temp;

					temp = (byte << requiredCount & 0xff) >> requiredCount;

					mBuffer = (mBuffer << shift) | temp;
					mStoredBitsCount += shift;

					requiredCount = requiredCount - shift;
				}
			}
		}
		return ret;
	}

	DWORD BZipFile::Read(std::vector<BYTE>& buffer) const
	{
		DWORD readCount = 0;
		const BOOL ret = ReadFile(mFileHandle, buffer.data(), static_cast<DWORD>(buffer.size()), &readCount, nullptr);
		if (ret == 0)
			return 0;

		return readCount;
	}
}
