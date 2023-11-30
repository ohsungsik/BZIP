#pragma once
/*!
 *  \file MoveToFront.hpp
 *  \brief Move-To-Front 역변환 클래스
 *
 *  \author sso
 *  \date   2023/11/26
 *  \copyright Copyright 2023 sso
 */

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <vector>

namespace BZIP
{
	class MoveToFront final
	{
	public:
		explicit MoveToFront();

		MoveToFront& operator=(const MoveToFront& rhs) = delete;

		~MoveToFront();

		BYTE IndexOfFront(const INT& index);

	private:
		std::vector<BYTE> mMTFDictionary;
	};
}
