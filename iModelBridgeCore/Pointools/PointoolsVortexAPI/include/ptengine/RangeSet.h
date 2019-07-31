#pragma once

#include <set>

namespace pointsengine
{


template<typename T, typename ItemSet> class Range
{

public:

	enum SearchMode
	{
		SearchModeNULL,

		SearchModeNormal,
		SearchModeEquals,
		SearchModeAdjacentMin,
		SearchModeAdjacentMax,

		SearchModeCount
	};

	typedef Range<T, ItemSet>	This;

protected:

	T			start;
	T			end;

	ItemSet		itemSet;

	SearchMode	searchMode;

public:

	void		setStart		(T initStart);
	T			getStart		(void) const;

	void		setEnd			(T initEnd);
	T			getEnd			(void) const;

	void		set				(T initStart, T initEnd);

	void		setSearchMode	(SearchMode mode);
	SearchMode	getSearchMode	(void) const;

	bool		isInRange		(T value) const;
	bool		isAdjacentMin	(T value) const;
	bool		isAdjacentMax	(T value) const;

	bool		isInRange		(const This &range) const;
	bool		isAdjacentMin	(const This &range) const;
	bool		isAdjacentMax	(const This &range) const;
	bool		isAdjacent		(const This &range) const;

	bool		operator <		(const This &range) const;
	bool		operator >		(const This &range) const;

	void		merge			(Range &range);


};


template<typename T, typename ItemSet> class RangeSet
{
protected:

	typedef Range<T, ItemSet>		Range;
	typedef std::set<Range>			Set;

public:

	typedef typename Set::iterator	RangeIterator;


protected:

	Set					rangeSet;

public:

	Range			*	getMinRange		(Range &range);
	Range			*	getMaxRange		(Range &range);

	bool				insert			(Range &range);
	bool				remove			(Range &range);

	RangeIterator		find			(Range &range);
};




template<typename T, typename ItemSet>
void pointsengine::Range<T, ItemSet>::merge(Range &range)
{
	setStart(std::min(getStart(), range.getStart()));

	setEnd(std::max(getEnd(), range.getEnd()));
}

template<typename T, typename ItemSet>
inline void Range<T, ItemSet>::setSearchMode(SearchMode mode)
{
	searchMode = mode;
}


template<typename T, typename ItemSet>
inline typename Range<T, ItemSet>::SearchMode Range<T, ItemSet>::getSearchMode(void) const
{
	return searchMode;
}



template<typename T, typename ItemSet>
void pointsengine::Range<T, ItemSet>::set(T initStart, T initEnd)
{
	setStart(initStart);
	setEnd(initEnd);
}

template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::operator<(const This &range) const
{
															// Enforce commutative false when requested adjacency is true
	switch(range.getSearchMode())
	{

	case SearchModeNULL:
															// Range is not the search key and thus has no mode.
															// so reverse the logical proposition to make the key the given parameter
		return range > (*this);

	case SearchModeAdjacentMin:

		if(isAdjacentMin(range))
			return false;

		break;

	case SearchModeAdjacentMax:

		if(isAdjacentMax(range))
			return false;

		break;

	case SearchModeEquals:
															// Sort based on start position. If equal, sort based on end position
		if(getStart() == range.getStart())
		{
			return getEnd() < range.getEnd();
		}

		return getStart() < range.getStart();

	case SearchModeNormal:
		break;
	}

	return getEnd() < range.getStart();
}

template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::operator>(const This &range) const
{
															// Enforce commutative false when requested adjacency is true
	switch(range.getSearchMode())
	{
	case SearchModeAdjacentMin:

		if(isAdjacentMin(range))
			return false;

		break;

	case SearchModeAdjacentMax:

		if(isAdjacentMax(range))
			return false;

		break;

	case SearchModeNULL:

		return range < (*this);

	case SearchModeEquals:

		if(getStart() == range.getStart())
		{
			return getEnd() > range.getEnd();
		}

		return getStart() > range.getStart();

	case SearchModeNormal:
		break;
	}

	return getStart() > range.getEnd();
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isAdjacent(const This &range) const
{
	return isAdjacentMin(range) || isAdjacentMax(range);
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isAdjacentMin(const This &range) const
{
	return isAdjacentMin(range.getEnd());
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isAdjacentMax(const This &range) const
{
	return isAdjacentMax(range.getStart());
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isAdjacentMin(T value) const
{
	return (value == getStart() - 1);
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isAdjacentMax(T value) const
{
	return (value == getEnd() + 1);
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isInRange(const This &range) const
{
	return isInRange(range.getStart()) || isInRange(range.getEnd());
}


template<typename T, typename ItemSet>
inline bool Range<T, ItemSet>::isInRange(T value) const
{
	return getStart() <= value && value <= getEnd();
}



template<typename T, typename ItemSet>
inline void Range<T, ItemSet>::setStart(T initStart)
{
	start = initStart;
}


template<typename T, typename ItemSet>
inline T Range<T, ItemSet>::getStart(void) const
{
	return start;
}


template<typename T, typename ItemSet>
inline void Range<T, ItemSet>::setEnd(T initEnd)
{
	end = initEnd;
}


template<typename T, typename ItemSet>
inline T Range<T, ItemSet>::getEnd(void) const
{
	return end;
}





template<typename T, typename ItemSet>
bool pointsengine::RangeSet<T, ItemSet>::insert(Range &range)
{
	Range	*rangeMin;
	Range	*rangeMax;
															// Get adjacent range on left if it exists
	rangeMin = getMinRange(range);
															// Get adjacent range on right if it exists
	rangeMax = getMaxRange(range);

	if(rangeMin)
	{
															// Merge left pair if adjacent
		rangeMin->merge(range);

		if(rangeMax)
		{
															// Merge all three ranges if adjacent
			rangeMin->merge(*rangeMax);
			remove(*rangeMax);
		}

		return true;
	}
	else
	{
		if(rangeMax)
		{
															// Merge right pair if adjacent
			rangeMax->merge(range);
		}
		else
		{
															// If range is isolated, just insert
			range.setSearchMode(Range::SearchModeNormal);
			std::pair<Set::iterator, bool> r;
			r = rangeSet.insert(range);
			r.first->setSearchMode(Range::SearchModeNULL);
		}

		return true;
	}
															// Error
	return false;
}


template<typename T, typename ItemSet>
typename RangeSet<T, ItemSet>::RangeIterator RangeSet<T, ItemSet>::find(Range &range)
{
	range.setSearchMode(Range::SearchModeEquals);

	return rangeSet.find(range);
}


template<typename T, typename ItemSet>
bool pointsengine::RangeSet<T, ItemSet>::remove(Range &range)
{
	RangeIterator it;

	if((it = find(range)) != rangeSet.end())
	{
		rangeSet.erase(it);
		return true;
	}

	return false;
}


template<typename T, typename ItemSet>
typename RangeSet<T, ItemSet>::Range *RangeSet<T, ItemSet>::getMinRange(Range &range)
{
	range.setSearchMode(Range::SearchModeAdjacentMax);

	Set::iterator it = rangeSet.find(range);

	if(it != rangeSet.end())
	{
		return &(*it);
	}

	return NULL;
}


template<typename T, typename ItemSet>
typename RangeSet<T, ItemSet>::Range *RangeSet<T, ItemSet>::getMaxRange(Range &range)
{
	range.setSearchMode(Range::SearchModeAdjacentMin);

	Set::iterator it = rangeSet.find(range);

	if(it != rangeSet.end())
	{
		return &(*it);
	}

	return NULL;
}


} // End pointsengine namespace

