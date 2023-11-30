#include "Logging.hpp"

#include <cstdlib>

namespace BZIP
{
	Logging* Logging::mLogging = nullptr;
	HANDLE Logging::mOutputHandle = nullptr;

	bool Logging::Initialize()
	{
		if (mLogging != nullptr)
			return true;

		mLogging = new Logging();

		if (atexit(Close)!= 0)
			return false;

		mOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (mOutputHandle == INVALID_HANDLE_VALUE)
			return false;

		return true;
	}

	Logging& Logging::GetInstance()
	{
		return *mLogging;
	}

	void Logging::Debug(const std::wstring& log) const
	{
		Write(Color::Gray, log);
	}

	void Logging::Info(const std::wstring& log) const
	{
		Write(Color::White, log);
	}

	void Logging::Warning(const std::wstring& log) const
	{
		Write(Color::Yellow, log);
	}

	void Logging::Error(const std::wstring& log) const
	{
		Write(Color::Red, log);
	}

	Logging::Logging() = default;
	Logging::~Logging() = default;

	void Logging::Close()
	{
		delete mLogging;
	}

	void Logging::Write(const Color& color, const std::wstring& log) const
	{
		CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
		GetConsoleScreenBufferInfo(mOutputHandle, &consoleScreenBufferInfo);

		SetConsoleTextAttribute(mOutputHandle, color);
		WriteConsole(mOutputHandle, log.c_str(), static_cast<DWORD>(log.length()), nullptr, nullptr);

		SetConsoleTextAttribute(mOutputHandle, consoleScreenBufferInfo.wAttributes);
	}
}
