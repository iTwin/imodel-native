#pragma once

#include <ptds/DataCache.h>

#include <queue>


namespace ptds
{

class DataCacheParallelRead;


class DataCacheParallelRead
{

public:

	typedef DataCache::CachePageIndex	CachePageIndex;

	typedef unsigned int				ItemSize;

protected:

	DataSize			readSize;

	DataPointer			currentReadPosition;
	DataSize			currentReadSize;
	unsigned int		currentNumPointsRead;

	ItemSize			itemSize;

public:


						DataCacheParallelRead			(void);

	void				clear							(void);

	void				setReadSize						(DataSize size);
	DataSize			getReadSize						(void) const;

	void				setCurrentReadSize				(DataSize size);
	DataSize			getCurrentReadSize				(void) const;

	void				setCurrentReadPosition			(DataSize size);
	DataSize			getCurrentReadPosition			(void) const;

	void				setCurrentNumPointsRead			(unsigned int numPoints);
	unsigned int		getCurrentNumPointsRead			(void) const;

	void				setItemSize						(ItemSize itemSize);
	ItemSize			getItemSize						(void);

	bool				isReadComplete					(void) const;

	bool				operator <						(const DataCacheParallelRead &other) const;

};

} // End ptds namespace