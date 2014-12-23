/*----------------------------------------------------------*/ 
/* PointsFilter.h											*/ 
/* Points Filtering Thread Interface file					*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_POINTCLOUD_FILTER
#define POINTOOLS_POINTCLOUD_FILTER 1

#include <ptengine/ptengine_api.h>

#include <ptcloud2/datachannel.h>
#include <pt/typedefs.h>
#include <list>
#include <vector>

namespace pointsengine
{
/*-----------------------------------------------------------------*/ 
/*		Points Filter											   */ 
/*-----------------------------------------------------------------*/ 
class PTENGINE_API PointsFilteringState
{
public:
	PointsFilteringState() : 
	  _filterPagingByDisplay(false), _clipboxFilter(false),
		  _selectionFilter(false), _displayChannelFilter(0) {}
	
	bool filterPagingByDisplay() const	{ return _filterPagingByDisplay; }
	bool clipboxFilter() const			{ return _clipboxFilter; }
	bool selectionFilter() const		{ return _selectionFilter; }
	uint displayChannelFilter() const	{ return _displayChannelFilter; }
	
	void filterPagingByDisplay(const bool &filter)		{ _filterPagingByDisplay = filter; };
	void clipboxFilter(const bool &filter)				{ _clipboxFilter = filter; }
	void selectionFilter(const bool &filter)			{ _selectionFilter = filter; }

	void addDisplayChannelFilter(pcloud::Channel c)	{ _displayChannelFilter |= 1 << (c-1);	};
	void remDisplayChannelFilter(pcloud::Channel c)	{ _displayChannelFilter &= ~(1 << (c-1)); };
	void remDisplayChannelFilters()					{ _displayChannelFilter = 0; }

private:
	bool	_filterPagingByDisplay;
	bool	_clipboxFilter;
	bool	_selectionFilter;
	uint	_displayChannelFilter;
};
}
#endif