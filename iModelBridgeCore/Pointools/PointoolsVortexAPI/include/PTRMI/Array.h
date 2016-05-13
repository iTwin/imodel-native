
#pragma once

#include <PTRMI/PTRMI.h>
#include <PTRMI/DataBuffer.h>

#define ARRAY_T		template<typename T>


namespace PTRMI
{
	class DataBuffer;

	template<typename T = unsigned char> class Array
	{
	public:

		typedef unsigned long	Size;

		typedef T				Data;

	protected:

		Size		size;

		T		*	array;

	public:

					Array			(void);
					Array			(Size initSize, T *initArray);

		void		clear			(void);

		bool		isValid			(void);

		void		setSize			(Size initSize);
		Size		getSize			(void) const;

		Size		getSizeBytes	(void) const;

		void		setArray		(T *initArray);
		T		*	getArray		(void) const;

		void		read			(DataBuffer &buffer);
		void		write			(DataBuffer &buffer) const;

static	DataSize	getMaxWriteSize	(DataSize arraySize);

		void		readPartial		(DataBuffer &buffer);
		void		writePartial	(DataBuffer &buffer) const;

		void		readSize		(DataBuffer &buffer);
		void		writeSize		(DataBuffer &buffer);

		Size		allocate		(DataBuffer &stream);
	};


	ARRAY_T
	Array<T>::Array(void)
	{
		clear();
	}

	ARRAY_T
	Array<T>::Array(Size initSize, T *initArray)
	{
															// Clear
		clear();
															// If array specified
		if(initSize > 0 && initArray != NULL)
		{
															// Set size and array
			setSize(initSize);
			setArray(initArray);
		}
	}

	ARRAY_T
	bool PTRMI::Array<T>::isValid(void)
	{
		return (getSize() > 0 && getArray() != NULL);
	}


	ARRAY_T
	void Array<T>::clear(void)
	{
															// Clear back to empty array
		setSize(0);
		setArray(NULL);
	}

	ARRAY_T
	void Array<T>::setSize(Size initSize)
	{
		size = initSize;
	}

	ARRAY_T
	typename PTRMI::Array<T>::Size Array<T>::getSize(void) const
	{
		return size;
	}


	ARRAY_T
	typename PTRMI::Array<T>::Size PTRMI::Array<T>::getSizeBytes(void) const
	{
		return getSize() * sizeof(T);
	}

	ARRAY_T
	void Array<T>::setArray(T *initArray)
	{
		array = initArray;
	}

	ARRAY_T
	T * Array<T>::getArray(void) const
	{
		return array;
	}
	
	ARRAY_T
	void Array<T>::read(DataBuffer &buffer)
	{
		readPartial(buffer);
															// Read buffer data to this array
		if(buffer.readFromBuffer(getArray(), getSizeBytes()) != getSizeBytes())
		{
			setSize(0);
			Status status(Status::Status_Error_Array_Read);
		}
	}

	ARRAY_T
	void Array<T>::write(DataBuffer &buffer) const
	{
		writePartial(buffer);
															// Write this array to buffer
		if(buffer.writeToBuffer(reinterpret_cast<const DataBuffer::Data *>(getArray()), getSizeBytes()) != getSizeBytes())
		{
			Status status(Status::Status_Error_Array_Write);
		}
	}

	ARRAY_T
	PTRMI::DataSize Array<T>::getMaxWriteSize(DataSize maxArrayItems)
	{
															// Max size is partial write size plus array size
		return sizeof(Size) + (maxArrayItems * sizeof(T));
	}

	ARRAY_T
	void Array<T>::readPartial(DataBuffer &buffer)
	{
        // Get size of array to be read in bytes
		Size initSize;
		buffer.readFromBuffer(initSize);
                															// Set size
		setSize(initSize);
	}

	ARRAY_T
	void Array<T>::writePartial(DataBuffer &buffer) const
	{
															// Write array length in bytes
		buffer << (Size) getSize();
	}


	ARRAY_T
	typename PTRMI::Array<T>::Size Array<T>::allocate(DataBuffer &stream)
	{

	}


} // End PTRMI namespace