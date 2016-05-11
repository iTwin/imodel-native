#pragma once

#include <ptds/DataSourceRead.h>

#include <PTRMI/DataBuffer.h>

#include <vector>


namespace ptds
{

class DataSourceReadSet
{

public:

	typedef DataSourceRead::ItemSize		ItemSize;
	typedef unsigned int					ReadIndex;

protected:

	typedef std::vector<DataSourceRead>		ReadSet;

protected:

		ReadSet				readSet;

		ItemSize			totalItemSize;
		DataSize			totalReadSize;

protected:

		void				setTotalItemSize		(ItemSize size);
		void				setTotalReadSize		(DataSize size);

public:

							DataSourceReadSet		(void);

		bool				addRead					(DataSourceRead const &read);
		unsigned int		getNumReads				(void) const;
 const	DataSourceRead	*	getRead					(ReadIndex index) const;
		DataSourceRead	*	getRead					(ReadIndex index);
		void				clear					(void);
		ItemSize			getTotalItemSize		(void);
		DataSize			getTotalReadSize		(void) const;

		bool				isDefined				(void);

		void				read					(PTRMI::DataBuffer &buffer);
		void				write					(PTRMI::DataBuffer &buffer) const;

		DataSource::Data *	executeReadSet			(DataSource &dataSource, DataSource::Data *buffer = NULL);

		DataPointer			getFinalDataPointer		(void);

		ptds::DataSize		transferVoxelData(DataSource::Data *source);
};

}