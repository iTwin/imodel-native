
#include <ptds/DataCacheParallelRead.h>

namespace ptds
{

DataCacheParallelRead::DataCacheParallelRead(void)
{
	clear();
}


void DataCacheParallelRead::clear(void)
{
	setCurrentReadPosition(0);
	setReadSize(0);

	setCurrentReadSize(0);

	setCurrentNumPointsRead(0);

	setItemSize(0);
}


void DataCacheParallelRead::setCurrentReadPosition(DataPointer position)
{
	currentReadPosition = position;
}


ptds::DataPointer DataCacheParallelRead::getCurrentReadPosition(void) const
{
	return currentReadPosition;
}


void DataCacheParallelRead::setCurrentNumPointsRead(unsigned int numPoints)
{
	currentNumPointsRead = numPoints;
}


unsigned int DataCacheParallelRead::getCurrentNumPointsRead(void) const
{
	return currentNumPointsRead;
}


bool DataCacheParallelRead::operator<(const DataCacheParallelRead &other) const
{
															// Note: Used for priority queue sorting.
															// This is greater than to implement a Min priority queue
	return getCurrentNumPointsRead() > other.getCurrentNumPointsRead();
}


void DataCacheParallelRead::setReadSize(DataSize size)
{
	readSize = size;
}


ptds::DataSize DataCacheParallelRead::getReadSize(void) const
{
	return readSize;
}

bool DataCacheParallelRead::isReadComplete(void) const
{
															// Read is complete when enough data has been read
	return getCurrentReadSize() >= getReadSize();
}


void DataCacheParallelRead::setCurrentReadSize(DataSize size)
{
	currentReadSize = size;
}


ptds::DataSize DataCacheParallelRead::getCurrentReadSize(void) const
{
	return currentReadSize;
}


void DataCacheParallelRead::setItemSize(ItemSize initItemSize)
{
	itemSize = initItemSize;
}


DataCacheParallelRead::ItemSize DataCacheParallelRead::getItemSize(void)
{
	return itemSize;
}


 


} // End ptds namespace

