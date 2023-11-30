#include "MoveToFront.hpp"

#include <numeric>

namespace BZIP
{
	MoveToFront::MoveToFront() :
		mMTFDictionary(256)
	{
		std::iota(mMTFDictionary.begin(), mMTFDictionary.end(), static_cast<BYTE>(0));
	}

	MoveToFront::~MoveToFront() = default;

	BYTE MoveToFront::IndexOfFront(const INT& index)
	{
		const BYTE word = mMTFDictionary[index];
		mMTFDictionary.erase(mMTFDictionary.begin() + index);
		mMTFDictionary.insert(mMTFDictionary.begin(), word);

		return word;
	}
}
