#pragma once

#include <ptds/DataSource.h>


namespace ptds
{


class DataSourceRead
{
public:

	typedef void						*	ClientID;
	typedef unsigned int					ItemSize;

	typedef DataSource::DataPointer			DataPointer;
	typedef DataSource::DataSize			DataSize;
	typedef DataSource::Data				Data;

protected:

	ClientID				clientID;

	ItemSize				itemSize;

	DataPointer				position;
	DataSize				size;

	Data				*	buffer;

public:

							DataSourceRead		(ClientID initClientID, ItemSize initItemSize, DataPointer initPosition, DataSize initSize, Data *initBuffer);

	bool					setClientID			(ClientID id);
	ClientID				getClientID			(void) const;

	bool					setItemSize			(ItemSize initItemSize);
	ItemSize				getItemSize			(void) const;

	bool					setPosition			(DataPointer initPosition);
	DataPointer				getPosition			(void) const;

	DataPointer				getEndPosition		(void);

	bool					setSize				(DataSize initSize);
	DataSize				getSize				(void) const;

	bool					setBuffer			(Data *initBuffer);
	Data				*	getBuffer			(void);

	DataSize				read				(DataSource &dataSource, Data *buffer);

};




} // End ptds namespace