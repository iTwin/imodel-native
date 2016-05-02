/*----------------------------------------------------------*/ 
/* PointsFilter.h											*/ 
/* Points Filtering Thread Interface file					*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_POINTCLOUD_SELECT_FILTER
#define POINTOOLS_POINTCLOUD_SELECT_FILTER 1


#include <pt\scenegraph.h>

#include <ptcloud2\voxel.h>
#include <ptcloud2\bitvector.h>

#include <ptengine\selectionFilter.h>
#include <ptengine\ptengine_api.h>

#include <list>
#include <vector>

namespace pointsengine
{
/*-----------------------------------------------------------------*/ 
/*		Points Selection Module									   */ 
/*-----------------------------------------------------------------*/ 
class PTENGINE_API PointsSelect
{
public:
	PointsSelect();
	~PointsSelect();

	bool initialize();

	/*Activation*/ 
	void activateSelectionFiltering()	{ _active = true; }
	void deactivateSelectionFiltering()	{ _active = false; }
	bool isActive() const				{ return _active; }

	bool isEnabled() const	{ return _enabled; }
	void enable()			{ _enabled = true; }
	void disable()			{ _enabled = false; }

	bool paused() const;
	void pause();
	void unpause();

	void addSelectionFilter(SelectionFilter *sf);
	void selectVoxel(pcloud::Voxel *vox);

	void clearSelectionFilters();
	void hideSelectedPoints();
	void showAllPoints(bool sel);

	void refilter();
	void completeVoxelFiltering(pcloud::Voxel *vox);

protected:
	SelectionFilter *_first;
	SelectionFilterMode _mode;
	bool _active;
	bool _enabled;

	class SelectionFilterThread
	{
	public:
		SelectionFilterThread();
		~SelectionFilterThread();
		
		SelectionFilterThread &operator = (const SelectionFilterThread &t) { return *this; }
		
		void operator ()();
		void processSelectionFilters();
	
	};
};
}
#endif