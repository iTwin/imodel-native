#include <ptds/DataSourceRead.h>

namespace ptds
{

DataSourceRead::DataSourceRead(ClientID initClientID, ItemSize initItemSize, DataPointer initPosition, DataSize initSize, Data *initBuffer)
{
	setClientID(initClientID);

	setItemSize(initItemSize);

	setPosition(initPosition);

	setSize(initSize);

	setBuffer(initBuffer);
}



bool DataSourceRead::setClientID(ClientID id)
{
	clientID = id;

	return true;
}


DataSourceRead::ClientID DataSourceRead::getClientID(void) const
{
	return clientID;
}


bool DataSourceRead::setItemSize(ItemSize initItemSize)
{
	itemSize = initItemSize;

	return true;
}


DataSourceRead::ItemSize DataSourceRead::getItemSize(void) const
{
	return itemSize;
}


bool DataSourceRead::setPosition(DataPointer initPosition)
{
	position = initPosition;

	return true;
}


DataSourceRead::DataPointer DataSourceRead::getPosition(void) const
{
	return position;
}


bool DataSourceRead::setSize(DataSize initSize)
{
	size = initSize;

	return true;
}


DataSourceRead::DataSize DataSourceRead::getSize(void) const
{
	return size;
}


bool DataSourceRead::setBuffer(Data *initBuffer)
{
	buffer = initBuffer;

	return true;
}


DataSource::Data *DataSourceRead::getBuffer(void)
{
	return buffer;
}


ptds::DataPointer DataSourceRead::getEndPosition(void)
{
	return getPosition() + getSize() - 1;
}


ptds::DataSize DataSourceRead::read(DataSource &dataSource, Data *buffer)
{
	dataSource.movePointerTo(getPosition());

	return dataSource.readBytes(buffer, getSize());
}


} // End ptds namespace