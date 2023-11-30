#include <iostream>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <fstream>
#include <format>

#include "BZipFile.hpp"
#include "Constants.hpp"
#include "Decompress.hpp"
#include "Logging.hpp"

using namespace BZIP;

int wmain(const int argc, const wchar_t* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (Logging::Initialize() == false)
		return -1;

	if (argc != 2)
	{
		LOG_WARNING("Command line arguments가 잘못되었습니다.\n");
		LOG_WARNING("		e.g) BZIP.exe <bzip2 file path>\n");
		return -1;
	}

	const std::wstring inputFileName(argv[1], argv[1] + wcslen(argv[1]));
	const std::wstring outputFileName = inputFileName.substr(0, inputFileName.find_last_of(L'.'));

	const HANDLE outputFile = CreateFile(outputFileName.c_str(), GENERIC_WRITE, NULL, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (outputFile == INVALID_HANDLE_VALUE)
	{
		LOG_ERROR("결과 파일을 생성할 수 없습니다.\n");
		if (GetFileAttributes(outputFileName.c_str()) != INVALID_FILE_ATTRIBUTES)
			LOG_ERROR("결과 파일이 이미 존재합니다.\n");
		return -1;
	}

	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	LARGE_INTEGER startingTime;
	QueryPerformanceCounter(&startingTime);

	BZipFile file(inputFileName);
	bool result = file.Open();
	if (result == false)
	{
		return -1;
	}

	Decompress decompressor(file);
	result = decompressor.Open();

	if (result == false)
	{
		LOG_WARNING("BZip2 파일이 아닙니다.\n");
		return -1;
	}

	std::vector<BYTE> buffer(900000u);

	INT readCount = 0;
	while (true)
	{
		readCount = decompressor.Read(buffer);
		if (readCount < 0)
			break;

		WriteFile(outputFile, buffer.data(), readCount, nullptr, nullptr);
	}

	CloseHandle(outputFile);

	if (readCount == Constants::Return::INVALID_CRC)
	{
		DeleteFile(outputFileName.c_str());
	}

	LOG_INFO("압축해제를 완료하였습니다.\n");

	LARGE_INTEGER endingTime;
	QueryPerformanceCounter(&endingTime);

	LARGE_INTEGER elapsedMicroseconds;
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;

	elapsedMicroseconds.QuadPart *= 1000000;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;

	const std::wstring elapsedLogBuffer = L"소요 시간: " + std::format(L"{:.3f}sec", elapsedMicroseconds.QuadPart * 0.000001) + L"\n";
	Logging::GetInstance().Info(elapsedLogBuffer);

	return 0;
}