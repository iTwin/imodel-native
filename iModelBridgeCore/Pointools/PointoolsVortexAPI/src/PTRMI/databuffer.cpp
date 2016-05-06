#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/DataBuffer.h>
#include <PTRMI/Pipe.h>
#include <ptds/DataSource.h>
#include <ptds/FilePath.h>
#include <PTRMI/Array.h>

namespace PTRMI
{

DataBuffer::DataBuffer(void)
{
	clear();
}

DataBuffer::DataBuffer(DataSize initInternalSize)
{
	clear();

	setInternalBufferSize(initInternalSize);

	setMode(Mode_Internal);
}

DataBuffer::DataBuffer(Data *initExternalBuffer, DataSize initExternalSize)
{
	clear();

	if(initExternalBuffer != NULL && initExternalSize > 0)
	{
		setExternalBuffer(initExternalBuffer);
		setExternalBufferSize(initExternalSize);

		setMode(Mode_External);
	}
}


void DataBuffer::clear(void)
{
	readPtr					= 0;
	writePtr				= 0;
	messageParameterPtr		= 0;

	setMode(Mode_External);

	setInternalBuffer(NULL);
	setInternalBufferSize(0);

	setExternalBuffer(NULL);
	setExternalBufferSize(0);

	setBuffer(NULL);

	clearDataPtrs();

	setPipe(NULL);
}


void DataBuffer::clearDataPtrs(void)
{
	setMessageStartPtr(DataPtrNULL);
	setMessageStatusPtr(DataPtrNULL);
	setMessageSizeTotalPtr(DataPtrNULL);
	setMessageParameterPtr(DataPtrNULL);
}


void DataBuffer::clearExternalBuffer(void)
{
	readPtr			= 0;
	writePtr		= 0;
	messageParameterPtr	= 0;
	
	setExternalBuffer(NULL);
	setExternalBufferSize(0);

	setBuffer(NULL);

	clearDataPtrs();

	if(getMode() == Mode_External)
	{
		setBuffer(NULL);
	}
}


DataBuffer::~DataBuffer(void)
{
															// Delete any internally allocated buffer
	deleteInternalBuffer();
}


void DataBuffer::setExternalBuffer(Data *initBuffer, DataSize initSize)
{
	setExternalBuffer(initBuffer);
	setExternalBufferSize(initSize);
}

void DataBuffer::setExternalBuffer(Data *initBuffer)
{
	externalBuffer = initBuffer;
}


DataBuffer::Data *DataBuffer::getExternalBuffer(void)
{
	return externalBuffer;
}

void DataBuffer::setExternalBufferSize(DataSize initSize)
{
	externalBufferSize = initSize;
															// Read pointer set to the start
	setReadPtr(0);
															// Write pointer set to the end, so we know we have some data
	setWritePtr(initSize);
}

DataBuffer::DataSize DataBuffer::getExternalBufferSize(void)
{
	return externalBufferSize;
}


void DataBuffer::setMode(Mode initMode)
{
	mode = initMode;
															// If mode is internal, set start to start of vector
	if(initMode == Mode_Internal)
	{
		setBuffer(getInternalBuffer());
	}
	else
	if(initMode == Mode_External)
	{
															// If mode is external, set to external pointer
		setBuffer(getExternalBuffer());
	}
}

DataBuffer::Mode DataBuffer::getMode(void)
{
	return mode;
}


DataBuffer::DataSize DataBuffer::getRequiredReceiveMin(DataSize minBytesRequired)
{
															// If bytes required is less or equal to that available, no more required
	if(minBytesRequired <= getDataSize())
	{
		return 0;
	}
															// Required minimum read is required num bytes minus the number already available
	return minBytesRequired - getDataSize();
}


Status DataBuffer::manageBuffer(DataSize extraBytes)
{
	float	upsizeCoef			= 1.3;
	float	fullRatioUpSize		= 0.7;
	float	fullRatioReset		= 0.3;

															// Only manage internal buffer
	if(getMode() == Mode_Internal)
	{
															// If the buffer is empty
		if(isEmpty())
		{													// Reset buffer pointers to start
			reset();
		}
															// Get amount of physical space remaining at the end of the buffer
		DataSize remainingSize = getInternalBufferSize() - getWritePtr();
															// If there's enough space left at the end
		if(remainingSize >= extraBytes)
		{
															// Use available space, so return OK
			return Status();
		}

															// Get amount of physical free space
        float totalEmptySize = static_cast<float>(getInternalBufferSize() - getDataSize());
															// Get ratio of how full the buffer is
		float fullRatio = (static_cast<float>(getDataSize()) / static_cast<float>(getBufferSize()));
															// If fuller than threshold, do a resize
		if(extraBytes > totalEmptySize || fullRatio > fullRatioUpSize)
		{
			DataSize upSize = static_cast<DataSize>(static_cast<float>(getInternalBufferSize()) * upsizeCoef);

			DataSize newSize = std::max(getInternalBufferSize() + extraBytes, upSize);

			return resizeInternalBuffer(newSize);
		}
															// There is space if the buffer is re-arranged

															// Normalize the buffer (copy remaining data back to start)
		return normalizeInternalBuffer();
	}
															// Return OK for external buffers
	return Status();
}

DataBuffer::DataSize DataBuffer::readFromBuffer(void *dest, DataSize numBytes)
{
	assert(isValid());
															// NOTE: This method is branch frequency optimized, that's why there's replication

															// If destination specified and data exists
	if(dest && numBytes > 0)
	{
															// Calculate pointer after read operation
		DataPtr	finalReadPtr = getReadPtr() + numBytes;
															// If there's enough data present
		if(finalReadPtr < getWritePtr())
		{
															// Copy from buffer to given destination
			memcpy(dest, getReadPtrAddress(), static_cast<size_t>(numBytes));
															// Advance data pointer
			setReadPtr(finalReadPtr);
															// Return number of bytes read
			return numBytes;
		}
															// If all data will be read
		if(finalReadPtr == getWritePtr())
		{
															// Copy from buffer to given destination
			memcpy(dest, getReadPtrAddress(), static_cast<size_t>(numBytes));
															// Advance data pointer
			setReadPtr(finalReadPtr);
															// Reset the buffer
			reset();
															// Return number of bytes read
			return numBytes;
		}

															// Final ReadPtr must past WritePtr, i.e. there's not enough data
		Pipe	*	pipe;
		Status		status;
		Data	*	allocation;
		DataSize	totalReceived;
															// Get Pipe
		if((pipe = getPipe()) == NULL)
		{
															// There's no source of more data, so return nothing correctly read
			return 0;
		}
															// Calculate how much extra is needed (minimum)		
		DataSize extraDataSize = finalReadPtr - getWritePtr();

															// Allocate data without moving write pointer
		if((allocation = allocate(extraDataSize, false)) == NULL)
		{
			Status status(Status::Status_Error_Data_Buffer_Read_Allocate_Failed);
			return 0;
		}
															// Receive minimum amount of data
		if((totalReceived = pipe->receiveData(*this, extraDataSize)) == 0)
		{
			Status status(Status::Status_Warning_Data_Buffer_Read_No_Pipe_For_Data);
			return 0;
		}
															// Copy from buffer to given destination
		memcpy(dest, getReadPtrAddress(), static_cast<size_t>(numBytes));
															// Set the final read pointer
		setReadPtr(getReadPtr() + numBytes);

assert(getReadPtr() <= getWritePtr());
															// Return number of bytes read
		return numBytes;
	}

															// Log a warning
	Status status(Status::Status_Warning_Data_Buffer_Read_Null_Parameter);
															// Error, so return zero bytes read
	return 0;
}

DataBuffer::DataSize DataBuffer::writeToBuffer(const Data *source, DataSize numBytes)
{
	assert(isValid());
															// If source is specified and data exists
	if(source && numBytes > 0)
	{
															// Try to allocate existing or new space
		Data *base = allocate(numBytes);
															// If allocation succeeded
		if(base)
		{
															// Copy from given source to buffer
			memcpy(base, source, numBytes);

			return numBytes;
		}
	}
															// Error, so return zero bytes written
	return 0;
}


Status DataBuffer::readFileToBuffer(const wchar_t *filepath, bool limitFileSize)
{
	std::ifstream	fileIn;
	Status			status;

															// Open binary file for read
	fileIn.open(filepath, std::ios::in | std::ios::binary);
															// If not open, return error
	if(fileIn.is_open() == false)
	{
		Status::log(L"DataBuffer::readFileToBuffer error opening ", filepath);
		return Status(Status::Status_Error_File_Open_For_Read);
	}

															// Get file size
	fileIn.seekg(0, std::ios::end);
	unsigned long fileSize = static_cast<unsigned long>(fileIn.tellg());
	fileIn.seekg(0);
															// Check we have enough buffer allocated
	if(fileSize > getInternalBufferSize())
	{
		if(limitFileSize == false)
		{
			if((status = createInternalBuffer(fileSize)).isFailed())
				return status;
		}
		else
		{
			fileSize = getBufferSize();
		}
	}
															// Read data block	
	fileIn.read(reinterpret_cast<char *>(getBuffer()), fileSize);
															// Check for error
	if(!fileIn)
	{
		Status::log(L"DataBuffer::readFileToBuffer error reading ", filepath);
		Status::log(L"DataBuffer::readFileToBuffer fileSize ", fileSize);
		return Status(Status::Status_Error_File_Read);
	}
															// Close file
	fileIn.close();
															// Move write pointer
	setWritePtr(fileSize);
															// Return OK
	return Status();
}


Status DataBuffer::readFileToBuffer(const wchar_t *filePath, ptds::DataSource *dataSource, bool limitFileSize)
{
	if(filePath == NULL || dataSource == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	DataSize	fileSize;
	Status		status;
															// Open file for reading from data source
	if(dataSource->openForRead(&ptds::FilePath(filePath)) == false)
	{
		Status::log(L"DataBuffer::readFileToBuffer opening ", filePath);
		return Status(Status::Status_Error_File_Open_For_Read);
	}
															// Get size of file
    if ((fileSize = static_cast<PTRMI::DataBuffer::DataSize>(dataSource->getFileSize())) == 0)
	{
		Status::log(L"DataBuffer::readFileToBuffer zero file size", filePath);
		return Status(Status::Status_Error_File_Size_Zero);
	}
															// Check we have enough buffer allocated
	if(fileSize > getInternalBufferSize())
	{
		if(limitFileSize == false)
		{
															// Set up buffer
			if((status = createInternalBuffer(fileSize)).isFailed())
				return status;
		}
		else
		{
			fileSize = getBufferSize();
		}
	}
															// Read file
	dataSource->readBytes(getBuffer(), fileSize);
															// Close file
	dataSource->close();

															// Move write pointer
	setWritePtr(fileSize);
															// Return OK
	return Status();
}


Status DataBuffer::writeFileFromBuffer(const wchar_t *filepath)
{
	std::ofstream fileOut;


	fileOut.open(filepath, std::ios::binary | std::ios::out);

	if(fileOut.is_open() == false)
	{
		Status::log(L"DataBuffer::writeFileFromBuffer error opening ", filepath);
		return Status(Status::Status_Error_File_Open_For_Write);
	}

	fileOut.write(reinterpret_cast<const char *>(getReadPtrAddress()), getDataSize());

	if(!fileOut)
	{
		Status::log(L"DataBuffer::writeFileFromBuffer error writing ", filepath);
		return Status(Status::Status_Error_File_Write);
	}

	fileOut.close();

	return Status();
}

Status DataBuffer::writeFileFromBuffer(const wchar_t *filepath, ptds::DataSource *dataSource)
{
	if(filepath == NULL || dataSource == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	ptds::FilePath path(filepath);

	if(dataSource->openForWrite(&path, true) == false)
	{
		Status::log(L"DataBuffer::writeFileFromBuffer error opening ", filepath);
		return Status(Status::Status_Error_File_Open_For_Write);
	}

	if(dataSource->writeBytes(getReadPtrAddress(), getDataSize()) != getDataSize())
	{
		Status::log(L"DataBuffer::writeFileFromBuffer error writing ", filepath);
		return Status(Status::Status_Error_File_Write);
	}

	dataSource->close();
															// Return OK
	return Status();
}



DataBuffer::Data * DataBuffer::allocate(DataSize numBytes, bool advanceWritePtr)
{
	Status	status;
															// Manage buffer position, size etc.
	if((status = manageBuffer(numBytes)).isFailed())
		return NULL;
															// Get current data pointer memory address
	Data *base = getWritePtrAddress();
															// If write continues after allocation
	if(advanceWritePtr)
	{
															// Establish final size requirement
		DataPtr finalWritePtr = getWritePtr() + numBytes;
															// Advance write ptr past allocation
		setWritePtr(finalWritePtr);
	}
															// Space is available, return the base address
	return base;
}


Status DataBuffer::advanceReadPtr(DataSize numBytes)
{
															// Get final read position
	DataPtr finalReadPtr = getReadPtr() + numBytes;
															// If valid, set read position
	if(isValidReadPtr(finalReadPtr))
	{
		setReadPtr(finalReadPtr);
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Advance_Buffer_Read_Ptr);
}


PTRMI::Status DataBuffer::advanceWritePtr(DataSize numBytes)
{
	DataPtr finalWritePtr = getWritePtr() + numBytes;

	if(isValidWritePtr(finalWritePtr))
	{
		setWritePtr(finalWritePtr);
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Advance_Buffer_Write_Ptr);
}


Status DataBuffer::WriteToBufferFromBuffer(DataBuffer &source, DataSize numBytes)
{
	Status	status;
															// Get source data address
	Data *sourceData = source.getReadPtrAddress();
															// Make sure source has enough data
	if((status = source.advanceReadPtr(numBytes)).isFailed())
		return Status(Status::Status_Error_Failed_To_Write_To_Buffer_From_Buffer);
															// Write data from source buffer to this buffer from source buffer's current pointer position
	if(writeToBuffer(sourceData, numBytes) != numBytes)
		return Status(Status::Status_Error_Failed_To_Write_To_Buffer_From_Buffer);
															// Return OK
	return Status();
}


DataBuffer::DataSize DataBuffer::getDataSize(void)
{
	return getWritePtr() - getReadPtr();
}


bool DataBuffer::isValidPtr(DataPtr position)
{
															// Valid if in or one byte over buffer
	return (position <= getBufferSize() && position != DataPtrNULL);
}


bool DataBuffer::isValidReadPtr(DataPtr ptr)
{
															// Valid if in or one byte over buffer
	return isValidPtr(ptr) && (ptr <= getWritePtr());
}


bool DataBuffer::isValidWritePtr(DataPtr ptr)
{
															// Valid if in or one byte over buffer
	return isValidPtr(ptr) && (ptr >= getReadPtr());
}


PTRMI::Status DataBuffer::resizeInternalBuffer(DataSize newSize, bool resizeMinimum)
{
	Data *newBuffer;
															// Make sure size is specified
	if(newSize == 0)
	{
		return Status(Status::Status_Error_Failed_To_Resize_Internal_Buffer);
	}
															// If size is same, don't resize
	if(newSize == getInternalBufferSize())
	{
		return Status();
	}
															// If size must be at least that specified, exit if it already is
	if(resizeMinimum && getInternalBufferSize() > newSize)
	{
		return Status();
	}
															// Allocate new buffer
	if((newBuffer = new Data[newSize]) == NULL)
	{
		return Status(Status::Status_Error_Memory_Allocation);
	}

															// Translate pointers backwards by the data shift
	translatePtrsBack(getReadPtr());

															// If internal buffer exists already
	if(getInternalBuffer())
	{
															// Copy existing data
		memcpy(newBuffer, getInternalBuffer(), std::min(newSize, getInternalBufferSize()));
															// Delete old buffer
		deleteInternalBuffer();
	}
															// Set new buffer
	setInternalBuffer(newBuffer);
															// Set new buffer size
	setInternalBufferSize(newSize);

															// If we're running internal mode, set the buffer now
	if(getMode() == Mode_Internal)
	{
		setBuffer(getInternalBuffer());
	}

	return Status();
}


void DataBuffer::translatePtrBack(DataPtr &ptr, DataPtr minusDelta)
{
															// Translate message status ptr if defined and valid
	if(ptr >= minusDelta)
	{
		if(isValidPtr(ptr))
		{
			ptr -= minusDelta;
		}
	}
	else
	{
		ptr = DataPtrNULL;
	}
}


PTRMI::Status DataBuffer::translatePtrsBack(DataPtr minusDelta)
{
															// Translate write pointer back
	setWritePtr(getWritePtr() - minusDelta);
															// Translate read pointer back (Usually this will be zero after)
	setReadPtr(getReadPtr() - minusDelta);
															// Translate tracked client pointers back (if used for messages)
	translatePtrBack(getMessageStartPtr(), minusDelta);
	translatePtrBack(getMessageStatusPtr(), minusDelta);
	translatePtrBack(getMessageSizeTotalPtr(), minusDelta);
	translatePtrBack(getMessageParameterPtr(), minusDelta);
															// Return OK
	return Status();
}


PTRMI::Status DataBuffer::resizeInternalBufferBy(DataSize numBytes)
{
	return resizeInternalBuffer(getInternalBufferSize() + numBytes);
}


void DataBuffer::deleteInternalBuffer(void)
{
	if(getInternalBuffer())
	{
		delete []getInternalBuffer();
		setInternalBuffer(NULL);
	}

	setInternalBufferSize(0);
}


DataBuffer::DataSize DataBuffer::getBufferSizeRemaining(void)
{
															// Return how much buffer space is still free
	return (getBufferSize() - getWritePtr());
}


void DataBuffer::setPipe(Pipe *initPipe)
{
	pipe = initPipe;
}


Pipe * DataBuffer::getPipe(void)
{
	return pipe;
}


PTRMI::Status DataBuffer::normalizeInternalBuffer(void)
{
	if(getDataSize() > 0)
	{
															// Move data back to the start of the buffer
		memmove_s(getInternalBuffer(), getBufferSize(), getReadPtrAddress(), getDataSize());
	}
															// Translate defined pointers back
	translatePtrsBack(getReadPtr());
															// Return OK
	return Status();
}


PTRMI::Status DataBuffer::setMessageStatusPtr(DataPtr initStatusPtr)
{
	messageStatusPtr = initStatusPtr;

	return Status();
}


DataBuffer::DataPtr &DataBuffer::getMessageStatusPtr(void)
{
	return messageStatusPtr;
}


PTRMI::Status DataBuffer::setMessageSizeTotalPtr(DataPtr initMessageSizeTotalPtr)
{
	messageSizeTotalPtr = initMessageSizeTotalPtr;

	return Status();
}


DataBuffer::DataPtr &DataBuffer::getMessageSizeTotalPtr(void)
{
	return messageSizeTotalPtr;
}


PTRMI::Status DataBuffer::setMessageParameterPtr(DataPtr initParameterStart)
{
															// Param Ptr may be somewhere in the future, so just set it
	messageParameterPtr = initParameterStart;

	return Status();
}


DataBuffer::DataPtr &DataBuffer::getMessageParameterPtr(void)
{
	return messageParameterPtr;
}



PTRMI::Status DataBuffer::setMessageStartPtr(DataBuffer::DataPtr startPtr)
{
	messageStartPtr = startPtr;

	return Status();
}

DataBuffer::DataPtr &DataBuffer::getMessageStartPtr(void)
{
	return messageStartPtr;
}


PTRMI::Status DataBuffer::setReadParameters(void)
{
															// Skip header padding to start of parameter block
	if(setReadPtr(getMessageParameterPtr()).isOK())
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Set_Read_Parameters);
}


PTRMI::Status DataBuffer::setWriteParameters(void)
{
															// Pad to end of header block to start of parameter block
	if(allocateTo(getMessageParameterPtr()))
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Set_Write_Parameters);
}


PTRMI::Status DataBuffer::copyExternalBufferState(DataBuffer &source)
{
															// Make sure mode is external
	if(getMode() != Mode_External)
	{
		return Status(Status::Status_Error_Copy_Ext_Bufffer_State_Invalid_Mode);
	}
															// Copy all states
	*this = source;
															// Return OK
	return Status();
}


DataBuffer::Data *DataBuffer::getPtrAddress(DataPtr position)
{
	if(isValidPtr(position))
	{
		return &(buffer[position]);
	}

	return NULL;
}



PTRMI::Status DataBuffer::testSimple(void)
{
	unsigned int	numInsertions		= 10;
	unsigned int	maxInsertions		= 1024 * 1024 * 10;
	unsigned int	numIterations		= 100;
	unsigned int	readAllIterations	= 4;
	unsigned int	readAllCounter		= readAllIterations;
	unsigned int	numItems			= 0;
	unsigned int	numInBuffer			= 0;
	unsigned int	numItemsHighest		= 0;
	unsigned int	writeNum, readNum;

	typedef			unsigned long Value;

	unsigned int	i, w, r;
	Value			valueCounterRead	= 0;
	Value			valueCounterWrite	= 0;
	Value			v;

	DataBuffer		testBuffer;
	
	testBuffer.createInternalBuffer(1024);

	for(i = 0; i < numIterations; i++)
	{
        writeNum = static_cast<uint>(static_cast<double>(maxInsertions) * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)));

		for(w = 0; w < writeNum; w++)
		{
			testBuffer << valueCounterWrite;
			++valueCounterWrite;
			++numInBuffer;
		}

		numItems = testBuffer.getDataSize() / sizeof(Value);
		if(numItems != numInBuffer)
		{
			return Status(Status::Status_Error_Failed);
		}

		numItemsHighest = std::max(numItemsHighest, numItems);


		if(readAllCounter == 0)
		{
			readNum			= numInBuffer;
			readAllCounter	= readAllIterations;
		}
		else
		{
			readNum			= static_cast<double>(numItems) * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
			readNum			= std::max(readNum, numItems);
			--readAllCounter;
		}		

		for(r = 0; r < readNum; r++)
		{
			testBuffer >> v;

			if(v != valueCounterRead)
			{
				return Status(Status::Status_Error_Failed);
			}

			--numInBuffer;
			++valueCounterRead;
		}
	}

	numItems = testBuffer.getDataSize() / sizeof(Value);
	if(numItems != numInBuffer)
	{
		return Status(Status::Status_Error_Failed);
	}


	return Status();

}

DataBuffer & DataBuffer::operator<<(const std::wstring &str)
    {
    // Wrap string as an array for writing
    PTRMI::Array<const wchar_t>	arrayString(PTRMI::Array<const wchar_t>::Size(str.length()) + 1, str.c_str());
    // Write array
    (*this) << arrayString;

    return (*this);
    }

DataBuffer & DataBuffer::operator>>(std::wstring &str)
    {
    wchar_t					buffer[DATA_BUFFER_MAX_STRING_LENGTH];
    PTRMI::Array<wchar_t>	arrayString(DATA_BUFFER_MAX_STRING_LENGTH, buffer);

    (*this) >> arrayString;

    if (arrayString.isValid())
        {
        str = arrayString.getArray();
        }

    return (*this);
    }

} // End PTRMI namespace
