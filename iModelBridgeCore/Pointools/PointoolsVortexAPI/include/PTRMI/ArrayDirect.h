
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

					ArrayDirect			(void) : Array() {}

					ArrayDirect			(Size initSize, T *initArray) : Array(initSize, initArray) {}

		void		readPartial			(DataBuffer &readBuffer, DataBuffer &writeBuffer);

		void		advance				(DataBuffer &buffer);
	};

	template<typename T>
	void PTRMI::ArrayDirect<T>::advance(DataBuffer &buffer)
	{
		buffer.advance(getSizeBytes());
	}

	ARRAYDIRECT_T
	void ArrayDirect<T>::readPartial(DataBuffer &readBuffer, DataBuffer &writeBuffer)
	{
		Super::readPartial(readBuffer);
															// Write size ready for the return
		writePartial(writeBuffer);
															// Allocate memory directly from write buffer
		setArray(reinterpret_cast<T *>(writeBuffer.allocate(getSizeBytes())));
	}



} // End PTRMI namespace