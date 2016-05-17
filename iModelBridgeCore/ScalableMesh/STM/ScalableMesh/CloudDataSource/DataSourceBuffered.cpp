#include "stdafx.h"
#include "DataSourceBuffered.h"
#include "DataSourceAccount.h"


DataSourceBuffered::DataSourceBuffered(DataSourceAccount *sourceAccount) : Super(sourceAccount)
{

}

void DataSourceBuffered::setBuffer(DataSourceBuffer * newBuffer)
{
	buffer = newBuffer;
}

DataSourceBuffer *DataSourceBuffered::getBuffer(void)
{
	return buffer;
}

DataSourceBuffer * DataSourceBuffered::transferBuffer(void)
{
	DataSourceBuffer *	sourceBuffer;
															// Get the buffer
	sourceBuffer = getBuffer();
															// Set internal pointer to buffer to NULL because it will be transferred to the caller
	setBuffer(nullptr);
															// Copy data source information
	sourceBuffer->setLocator(*this);
															// Return buffer to the caller
	return sourceBuffer;
}

DataSourceStatus DataSourceBuffered::setSegmentSize(DataSource::DataSize size)
{
	segmentSize = size;

	return DataSourceStatus();
}

DataSource::DataSize DataSourceBuffered::getSegmentSize(void)
{
	return segmentSize;
}

DataSourceBuffered::TimeoutStatus DataSourceBuffered::waitForSegments(DataSourceBuffer::Timeout timeoutMilliseconds)
{
	if (getBuffer())
	{
		return getBuffer()->waitForSegments(timeoutMilliseconds);
	}

	return TimeoutStatus::Status_Error;
}


bool DataSourceBuffered::isValid(void)
{
	return (getBuffer() != nullptr);
}


DataSourceStatus DataSourceBuffered::read(Buffer * dest, DataSize size)
{
	DataSourceStatus		status;
	DataSourceAccount *		account;
																// Size the buffer ready to read data into it
	if ((status = initializeBuffer(size, dest)).isFailed())
		return status;
																// Get the associated account
	if ((account = getAccount()) == nullptr)
		return DataSourceStatus(DataSourceStatus::Status_Error);
																// Download segments to the buffer
	status = account->downloadSegments(*this, dest, size);

	return status;
}

DataSourceStatus DataSourceBuffered::write(Buffer * source, DataSize size)
{
	DataSourceStatus	status;
																// If buffer is not defined, initialize one
	if (getBuffer() == nullptr)
	{
		if ((status = initializeBuffer()).isFailed())
			return status;
	}
																// Append give data to internal buffer
	status = getBuffer()->append(source, size);

	return status;
}



DataSourceStatus DataSourceBuffered::initializeBuffer(DataSourceBuffer::BufferSize size, DataSource::Buffer *existingBuffer)
{
	if (getBuffer())
	{
		delete getBuffer();
	}

	setBuffer(new DataSourceBuffer(size, existingBuffer));

	if (getBuffer())
	{
		getBuffer()->initializeSegments(getSegmentSize());

		return DataSourceStatus();
	}

	return DataSourceStatus(DataSourceStatus::Status_Error);
}


DataSourceStatus DataSourceBuffered::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
	(void) sourceURL;
	(void) sourceMode;

	return DataSourceStatus();
}

DataSourceStatus DataSourceBuffered::flush(void)
{
	DataSourceStatus		status;
	DataSourceAccount	*	account;

	if ((account = getAccount()) == nullptr)
		return DataSourceStatus(DataSourceStatus::Status_Error);

	if (getMode() == DataSourceMode_Write)
	{
		account->uploadSegments(*this);
	}


	return status;
}

DataSourceStatus DataSourceBuffered::close(void)
{
	return flush();
}

