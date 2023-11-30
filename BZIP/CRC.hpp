#pragma once
/*!
 *  \file CRC.hpp
 *  \brief CRC 검증 클래스
 *
 *  \author sso
 *  \date   2023/11/30
 *  \copyright Copyright 2023 sso
 */

#include <wtypes.h>

namespace BZIP
{
	class CRC final
	{
	public:
		CRC();

		CRC& operator=(const CRC& rhs);

		~CRC();
 
		UINT GetCRC() const;
		void UpdateCRC(const INT& byte);
		void UpdateCRC(const INT& byte, const UINT& count);

	private:
		UINT mCRC;
	};
}
