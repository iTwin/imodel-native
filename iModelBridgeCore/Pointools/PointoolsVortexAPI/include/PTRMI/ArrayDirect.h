
#pragma once

#include <PTRMI/Array.h>

#define ARRAYDIRECT_T	template<typename T>

namespace PTRMI
{

	template<typename T = unsigned char> class ArrayDirect : public Array<T>
	{
	protected:

		typedef		Array<T>			Super;

	public:
					ArrayDirect			(void) : Super() {}

                    ArrayDirect(typename Super::Size initSize, T *initArray) : Super(initSize, initArray) {}

		void		readPartial			(DataBuffer &readBuffer, DataBuffer &writeBuffer);

	};


	ARRAYDIRECT_T
	void ArrayDirect<T>::readPartial(DataBuffer &readBuffer, DataBuffer &writeBuffer)
	{
		Super::readPartial(readBuffer);
															// Write size ready for the return
        Super::writePartial(writeBuffer);
															// Allocate memory directly from write buffer
        Super::setArray(reinterpret_cast<T *>(writeBuffer.allocate(Super::getSizeBytes())));
	}



} // End PTRMI namespace