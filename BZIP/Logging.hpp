#pragma once
/*!
 *  \file Logging.hpp
 *  \brief 로깅 클래스
 *
 *  \author sso
 *  \date   2023/11/23
 *  \copyright Copyright 2023 sso
 */


#define WIN32_LEAN_AND_MEAN

#include <string>
#include <Windows.h>

namespace BZIP
{
	class Logging final
	{
	private:
		enum Color : WORD
		{
			Gray = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
			White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
			Yellow = FOREGROUND_RED | FOREGROUND_GREEN,
			Red = FOREGROUND_RED | FOREGROUND_INTENSITY
		};

	public:
		static bool Initialize();
		[[nodiscard]] static Logging& GetInstance();

		Logging& operator=(const Logging& rhs) = delete;

		void Debug(const std::wstring& log) const;
		void Info(const std::wstring& log) const;
		void Warning(const std::wstring& log) const;
		void Error(const std::wstring& log) const;

	private:
		explicit Logging();
		~Logging();

		static void Close();

		static Logging* mLogging;
		static HANDLE mOutputHandle;

		void Write(const Color& color, const std::wstring& log) const;
	};
}


#define LOG_DEBUG(message) Logging::GetInstance().Debug(L##message)
#define LOG_INFO(message) Logging::GetInstance().Info(L##message)
#define LOG_WARNING(message) Logging::GetInstance().Warning(L##message)
#define LOG_ERROR(message) Logging::GetInstance().Error(L##message)
