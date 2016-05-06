
#pragma once

#include <PTRMI/Name.h>
#include <PTRMI/Status.h>
#include <PTDS/FilePath.h>

namespace ptds
{
	class DataSource;
}

namespace PTRMI
{

	class Pipe;

	const unsigned int		DATA_BUFFER_MAX_STRING_LENGTH	= 2048;
	const unsigned long		DataPtrNULL						= 0xFFFFFFFF;

	class DataBuffer
	{

	public:

		enum Mode
		{
			Mode_Internal,
			Mode_External
		};

	public:

		typedef unsigned long				DataPtr;
		typedef DataPtr						DataSize;
		typedef unsigned char				Data;

	protected:

		Mode								mode;

		DataPtr								readPtr;
		DataPtr								writePtr;

		DataPtr								messageStartPtr;
		DataPtr								messageStatusPtr;
		DataPtr								messageSizeTotalPtr;
		DataPtr								messageParameterPtr;

		Data							*	internalBuffer;
		DataSize							internalBufferSize;

		Data							*	externalBuffer;
		DataSize							externalBufferSize;

		Data							*	buffer;

		Pipe							*	pipe;

	protected:

		void								setBuffer				(Data *initBuffer);

		DataSize							getRequiredReceiveMin	(DataSize minBytesRequired);

		void								translatePtrBack		(DataPtr &ptr, DataPtr minusDelta);

		void								clearDataPtrs			(void);

	public:

											DataBuffer				(void);
											DataBuffer				(DataSize internalSize);
											DataBuffer				(Data *initExternalBuffer, DataSize initExternalSize);
										   ~DataBuffer				(void);

		void								clear					(void);
		void								clearExternalBuffer		(void);

		void								setMode					(Mode initMode);
		Mode								getMode					(void);

		Status								copyExternalBufferState	(DataBuffer &source);

		void								setExternalBuffer		(Data *initBuffer, DataSize initSize);
		void								setExternalBuffer		(Data *initBuffer);
		Data							*	getExternalBuffer		(void);
		void								setExternalBufferSize	(DataSize initSize);
		DataSize							getExternalBufferSize	(void);

		void								setInternalBuffer		(Data *initBuffer);
		Data							*	getInternalBuffer		(void);
		void								setInternalBufferSize	(DataSize numBytes);
		DataSize							getInternalBufferSize	(void);


		DataSize							readFromBuffer			(void *dest, DataSize numBytes);
		DataSize							writeToBuffer			(const Data *source, DataSize numBytes);

		template<typename T>	DataSize	readFromBuffer			(T &value);
		template<typename T>	DataSize	readFromBuffer			(const T &value);
		template<typename T>	DataSize	writeToBuffer			(const T &value);

		Status								readFileToBuffer		(const wchar_t *filepath, bool limitFileSize = false);
		Status								readFileToBuffer		(const wchar_t *filepath, ptds::DataSource *dataSource, bool limitFileSize);
		Status								writeFileFromBuffer		(const wchar_t *filepath);
		Status								writeFileFromBuffer		(const wchar_t *filepath, ptds::DataSource *dataSource);
		Status								WriteToBufferFromBuffer	(DataBuffer &source, DataSize numBytes);

		DataBuffer::Data				*	allocate				(DataSize numBytes, bool advanceWritePtr = true);
		template<typename T>	Data	*	allocate				(void);
		DataBuffer::Data				*	allocateTo				(DataPtr position);

		Status								advanceReadPtr			(DataSize numBytes);
		Status								advanceWritePtr			(DataSize numBytes);

		void								reset					(void);
		void								resetRead				(void);
		void								resetWrite				(void);
		void								resetMessageStatusPtr	(void);
		void								resetMessageParameterPtr(void);
		Status								setReadPtr				(DataPtr position);
		DataPtr								getReadPtr				(void);
		Data *								getReadPtrAddress		(void);
		Status								setWritePtr				(DataPtr position);
		DataPtr								getWritePtr				(void);
		Data *								getWritePtrAddress		(void);

		Data *								getPtrAddress			(DataPtr position);

		Status								setMessageStartPtr		(DataPtr startPtr);
		DataPtr							&	getMessageStartPtr		(void);

		Status								setMessageStatusPtr		(DataPtr initStatusPtr);
		DataPtr							&	getMessageStatusPtr		(void);

		Status								setMessageSizeTotalPtr	(DataPtr initMessageSizeTotalPtr);
		DataBuffer::DataPtr				&	getMessageSizeTotalPtr	(void);

		Status								setMessageParameterPtr	(DataPtr initParameterStart);
		DataPtr							&	getMessageParameterPtr	(void);

		Status								translatePtrsBack		(DataPtr minusDelta);


		Status								setReadParameters		(void);
		Status								setWriteParameters		(void);

		Data							*	getBuffer				(void);

		DataSize							getBufferSize			(void);
		DataSize							getDataSize				(void);

		bool								isEmpty					(void);
		DataSize							getBufferSizeRemaining	(void);

		bool								isValid					(void);
		bool								isValidPtr				(DataPtr position);
		bool								isValidReadPtr			(DataPtr ptr);
		bool								isValidWritePtr			(DataPtr ptr);

		void								setPipe					(Pipe *initPipe);
		Pipe							*	getPipe					(void);

		Status								manageBuffer			(DataSize extraBytes);

		Status								createInternalBuffer	(DataSize size);
		Status								normalizeInternalBuffer	(void);
		Status								resizeInternalBuffer	(DataSize newSize, bool resizeMinimum = false);
		Status								resizeInternalBufferBy	(DataSize numBytes);
		Status								resizeInternalBufferMin	(DataSize numBytes);
		void								deleteInternalBuffer	(void);

		DataBuffer &						operator >>				(unsigned char &value);
		DataBuffer &						operator >>				(char &value);	
		DataBuffer &						operator >>				(unsigned short &value);
		DataBuffer &						operator >>				(short &value);	
		DataBuffer &						operator >>				(unsigned long &value);
		DataBuffer &						operator >>				(long &value);
		DataBuffer &						operator >>				(unsigned int &value);
		DataBuffer &						operator >>				(int &value);
		DataBuffer &						operator >>				(uint64_t &value);
		DataBuffer &						operator >>				(int64_t &value);
		DataBuffer &						operator >>				(wchar_t &value);
		DataBuffer &						operator >>				(bool &value);				
		DataBuffer &						operator >>				(float &value);				
		DataBuffer &						operator >>				(double &value);

		DataBuffer &						operator <<				(unsigned char value);
		DataBuffer &						operator <<				(char value);	
		DataBuffer &						operator <<				(unsigned short value);
		DataBuffer &						operator <<				(short value);	
		DataBuffer &						operator <<				(unsigned long value);
		DataBuffer &						operator <<				(long value);
		DataBuffer &						operator <<				(unsigned int value);
		DataBuffer &						operator <<				(int value);
		DataBuffer &						operator <<				(uint64_t value);
		DataBuffer &						operator <<				(int64_t value);
		DataBuffer &						operator <<				(wchar_t value);
		DataBuffer &						operator <<				(bool value);				
		DataBuffer &						operator <<				(float value);				
		DataBuffer &						operator <<				(double value);	

		template<typename T> DataBuffer &	operator <<				(const T &value);
		template<typename T> DataBuffer &	operator >>				(T &value);

		template<typename T> DataBuffer &	operator >>				(const T &value);

		DataBuffer &						operator <<				(const std::wstring &str);
		DataBuffer &						operator >>				(std::wstring &str);

		static Status						testSimple				(void);

		template<typename T> Status			setValue				(DataPtr dest, T value)
		{
															// If given value fits in the current buffer space
			if(isValidPtr(dest) && isValidPtr(dest + sizeof(T) - 1))
			{
															// Copy value to buffer
				*reinterpret_cast<T *>(getPtrAddress(dest)) = value;

				return Status();
			}

			return Status(Status::Status_Error_Data_Buffer_Write_Pointer_Invalid);
		}
	};

	inline void DataBuffer::setBuffer(Data *initBuffer)
	{
		buffer = initBuffer;
	}

	inline DataBuffer::Data * DataBuffer::getBuffer(void)
	{
		return buffer;
	}

	template<typename T>
	inline DataBuffer::DataSize DataBuffer::readFromBuffer(T &value)
	{
		return readFromBuffer(reinterpret_cast<void *>(&value), sizeof(T));
	}

	template<typename T>
	inline DataBuffer::DataSize DataBuffer::readFromBuffer(const T &value)
	{
															// Still need to read const values from buffers,
		return readFromBuffer(reinterpret_cast<void *>(const_cast<T *>(&value)), sizeof(T));
	}


	template<typename T>
	inline DataBuffer::DataSize DataBuffer::writeToBuffer(const T &value)
	{
		return writeToBuffer(reinterpret_cast<const Data *>(&value), sizeof(T));
	}

	template<typename T>
	inline DataBuffer::Data *DataBuffer::allocate(void)
	{
															// Allocate size of given data type
		return allocate(sizeof(T));
	}

	inline DataBuffer::Data *DataBuffer::allocateTo(DataPtr position)
	{
		if(position < getWritePtr())
		{
			return NULL;
		}
															// Allocate 
		return allocate(position - getWritePtr());
	}

	inline Status DataBuffer::setReadPtr(DataPtr position)
	{

		if(isValidReadPtr(position))
		{
			readPtr = position;
			return Status();
		}

		return Status(Status::Status_Error_Data_Buffer_Read_Pointer_Invalid);		
	}

	inline DataBuffer::DataPtr DataBuffer::getReadPtr(void)
	{
		return readPtr;
	}

	inline Status DataBuffer::setWritePtr(DataPtr position)
	{
		if(isValidWritePtr(position))
		{
			writePtr = position;
			return Status();
		}

		return Status(Status::Status_Error_Data_Buffer_Write_Pointer_Invalid);		
	}

	inline DataBuffer::DataPtr DataBuffer::getWritePtr(void)
	{
		return writePtr;
	}


	inline Status DataBuffer::createInternalBuffer(DataSize size)
	{
		Status	status;

		if(getInternalBuffer())
		{
			deleteInternalBuffer();
		}

		clear();

		setMode(Mode_Internal);

		return resizeInternalBuffer(size);
	}


	inline DataBuffer::DataSize DataBuffer::getBufferSize(void)
	{
		if(getMode() == Mode_Internal)
		{
			return getInternalBufferSize();
		}
		else
		{
			return getExternalBufferSize();
		}
	}

	inline void DataBuffer::setInternalBuffer(Data *initBuffer)
	{
		internalBuffer = initBuffer;
	}


	inline DataBuffer::Data *DataBuffer::getInternalBuffer(void)
	{
		return internalBuffer;
	}

	inline void DataBuffer::setInternalBufferSize(DataSize size)
	{
		internalBufferSize = size;
	}

	inline Status DataBuffer::resizeInternalBufferMin(DataSize size)
	{
		if(getInternalBufferSize() < size)
		{
			return resizeInternalBuffer(size);
		}

		return Status();
	}


	inline DataBuffer::DataSize DataBuffer::getInternalBufferSize(void)
	{
															// Return the current capacity
		return internalBufferSize;
	}

	inline bool DataBuffer::isValid(void)
	{
		return isValidReadPtr(getReadPtr()) && isValidWritePtr(getWritePtr());
	}


	inline DataBuffer::Data * DataBuffer::getReadPtrAddress(void)
	{
															// If data pointer is valid
		if(isValidReadPtr(getReadPtr()))
		{
															// Return the memory address of the data pointer
			return &(buffer[getReadPtr()]);
		}
															// Not valid, so return NULL
		return NULL;
	}

	inline DataBuffer::Data * DataBuffer::getWritePtrAddress(void)
	{
															// If write pointer is valud
		if(isValidWritePtr(getWritePtr()))
		{
															// Reurn the memory address of the data pointer
			return &(buffer[getWritePtr()]);
		}
															// Not valid, so return NULL
		return NULL;
	}

															// Handle standard atomic type reads
															// These are written directly without a read() method

	inline DataBuffer &DataBuffer::operator>>(unsigned char &value)		{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(char &value)				{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(unsigned short &value)	{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(short &value)				{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(unsigned long &value)		{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(long &value)				{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(unsigned int &value)		{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(int &value)				{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(uint64_t &value)	{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(int64_t &value)			{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(wchar_t &value)			{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(bool &value)				{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(float &value)				{readFromBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator>>(double &value)			{readFromBuffer(value); return (*this);}

															// Handle standard atomic type writes
															// These are written directly without a write() method
	inline DataBuffer &DataBuffer::operator<<(unsigned char value)		{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(char value)				{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(unsigned short value)		{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(short value)				{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(unsigned long value)		{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(long value)				{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(unsigned int value)		{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(int value)				{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(uint64_t value)	{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(int64_t value)			{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(wchar_t value)			{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(bool value)				{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(float value)				{writeToBuffer(value); return (*this);}
	inline DataBuffer &DataBuffer::operator<<(double value)				{writeToBuffer(value); return (*this);}


	template<typename T>
	inline DataBuffer & DataBuffer::operator>>(T &value)
	{

#ifndef NO_DATA_SOURCE_SERVER
		value.read(*this);
#endif

		return *this;
	}	

	template<typename T>
	inline DataBuffer & DataBuffer::operator<<(const T &value)
	{

#ifndef NO_DATA_SOURCE_SERVER
		value.write(*this);
#endif
		return *this;
	}

	template<typename T>
	inline DataBuffer &DataBuffer::operator>>(const T &value)
	{
															// Cast read value to non const (necessary !)
		(const_cast<T *>(&value))->read(*this);

		return *this;
	}

	inline void DataBuffer::reset(void)
	{
		resetRead();
		resetWrite();
	}

	inline void DataBuffer::resetRead(void)
	{
															// Translate parameter pointer
		if(getMessageParameterPtr() != DataPtrNULL)
		{
			setMessageParameterPtr(getMessageParameterPtr() - getReadPtr());
		}
															// Reset read pointer to zero
		setReadPtr(0);
	}

	inline void DataBuffer::resetWrite(void)
	{
															// Reset write pointer to zero
		setWritePtr(0);
	}

	inline void DataBuffer::resetMessageStatusPtr(void)
	{
		setMessageStatusPtr(DataPtrNULL);
	}

	inline void DataBuffer::resetMessageParameterPtr(void)
	{
		setMessageParameterPtr(DataPtrNULL);
	}

	inline bool DataBuffer::isEmpty(void)
	{
		return getReadPtr() == getWritePtr();
	}

}; // End PTRMI namespace

