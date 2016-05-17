
#include <iostream>
#include <pt/scenegraph.h>
#include <ptengine/pointsSelect.h>
#include <ptengine/pointsscene.h>

#include <ptcloud2/pointcloud.h>
#include <boost/thread.hpp>

#include <ptcmdppe/cmdstate.h>

#include <pt/trace.h>

#define _VERBOSE	1

#undef PTTRACE
#undef PTTRACEOUT

#define PTTRACE pt::Trace<pt::TraceOut<pt::CoutTrace> > _trace_obj
#define PTTRACEOUT pt::TraceOut<pt::CoutTrace>()

using namespace pointsengine;
using namespace pcloud;
namespace 
{
	bool g_run = true;
	bool g_active = true;
	bool g_pause = false;
	bool g_working = false;
	bool g_clear = false;
	bool g_used = false;

	SelectionFilter *g_firstFilter=0;
	ptapp::CmdStateIcon *g_stateicon=0;

	struct Pause
	{
		Pause()
		{
			pauseState = g_pause;
			if (!g_pause)
			{
				g_pause = true;
				while (g_working) BeThreadUtilities::BeSleep(5);
			}
		}
		~Pause()
		{
			g_pause = pauseState;
		}
		bool pauseState;
	};
};
#define PAUSE_POINTS_SELECT Pause __pause__;

#define WAS_PAUSED __pause__.pauseState

//--------------------------------------------------------------------
// PointsFilter
//--------------------------------------------------------------------
PointsSelect::PointsSelect() 
{ 
	_first = 0;
	_mode = Points_Select;
}
PointsSelect::~PointsSelect()
{
	g_pause = true;
	g_active = false;
}
//--------------------------------------------------------------------
// addFilterRequest
//--------------------------------------------------------------------
void PointsSelect::addSelectionFilter(SelectionFilter *sf)
{
	if (!g_firstFilter)
	{
		g_firstFilter = sf;
	}
	else g_firstFilter->addFilter(sf);
}
//
//
//
void PointsSelect::clearSelectionFilters()
{
	/* clear selection */ 
	g_clear = true;
}
//--------------------------------------------------------------------
// initialize
//--------------------------------------------------------------------
bool PointsSelect::initialize()
{
	SelectionFilterThread sft;
	boost::thread t(sft);

	return true;
}

void PointsSelect::pause()
{
	PTTRACE("PointsFilter: pause");
	if (g_pause) return;

	g_pause = true;
	while (g_working) BeThreadUtilities::BeSleep(15);
}
void PointsSelect::unpause()
{
	PTTRACE("PointsFilter: unpause");
	g_pause = false;
}
bool PointsSelect::paused() const
{
	return (g_pause && !g_working);
}	
//--------------------------------------------------------------------
// Filter thread
//--------------------------------------------------------------------
// Functor
//--------------------------------------------------------------------
PointsSelect::SelectionFilterThread::SelectionFilterThread()
{

}
PointsSelect::SelectionFilterThread::~SelectionFilterThread()
{
	g_working = false;
	if (g_stateicon) g_stateicon->state(2);
}
void PointsSelect::SelectionFilterThread::operator ()()
{
	PTTRACE("PointsFilter::FilterThread");

	while (g_run)
	{
		g_working = false;

		if (g_active && !g_pause)
		{
			processSelectionFilters();
		}
		else g_working = false;

        BeThreadUtilities::BeSleep(g_used ? 50 : 500);
	}
}	
struct HidePoints
{
	HidePoints(pcloud::Voxel *v) : _vox(v) {}

	void point(float *pnt, unsigned int index)
	{
		short s;
		_vox->channel(PCloud_Filter)->getval(s, index);			
		if (s == POINT_SEL_CODE)
		{
			s = POINT_HIDE_CODE;
			_vox->channel(PCloud_Filter)->set(index, &s);
		}
	};
	void point(double *pnt, unsigned int index)
	{
		short s;
		_vox->channel(PCloud_Filter)->getval(s, index);			
		if (s == POINT_SEL_CODE)
		{
			s = POINT_HIDE_CODE;
			_vox->channel(PCloud_Filter)->set(index, &s);	
		}
	};
	pcloud::Voxel *_vox;
};
struct ShowPoints
{
	ShowPoints(pcloud::Voxel *v, bool sel) : select(sel), _vox(v) {};

	void point(float *pnt, unsigned int index)
	{
		short cs;
		static short s = POINT_SHOW_CODE;

		_vox->channel(PCloud_Filter)->getval(cs, index);
		
		if (cs >= POINT_HIDE_CODE)
			_vox->channel(PCloud_Filter)->set(index, &s);
	};
	void point(double *pnt, unsigned int index)
	{
		short cs;
		static short s = POINT_SHOW_CODE;

		_vox->channel(PCloud_Filter)->getval(cs, index);
		
		if (cs >= POINT_HIDE_CODE)
			_vox->channel(PCloud_Filter)->set(index, &s);
	};
	bool select;
	pcloud::Voxel *_vox;
};
//
//
//
void PointsSelect::completeVoxelFiltering(pcloud::Voxel *vox)
{
	PAUSE_POINTS_SELECT
	if (!WAS_PAUSED) 
		if (g_firstFilter) g_firstFilter->processVoxel(vox, true);
}
//--------------------------------------------------------------------
// process Requests
//--------------------------------------------------------------------
void PointsSelect::hideSelectedPoints()
{
	PAUSE_POINTS_SELECT

	/*add all voxels to queue that have data (ie lod>0)*/ 		
	PointsScene::VOXELSLIST vlist;
	PointsScene::UseSceneVoxels voxels(vlist);

	PointsScene::VOXELSLIST::iterator i = vlist.begin();
	PointsScene::VOXELSLIST::iterator e = vlist.end();

	while (i!=e)
	{
		HidePoints hide(*i);

		if ((*i)->channel(PCloud_Filter))
		{
			boost::try_mutex::scoped_try_lock vlock((*i)->mutex());
			if (vlock.locked())
				(*i)->iterateTransformedPoints(hide, pt::ProjectSpace, false);
		}
		if ((*i)->flag(pcloud::WholeSelected))//0
			(*i)->flag(pcloud::WholeHidden, true);//2

		if ((*i)->flag(pcloud::PartSelected)) //1
			(*i)->flag(pcloud::PartHidden, true);//3
		
		++i;
	};
	SelectionFilter *f = g_firstFilter;
	while (f)
	{
		f->setSelectionFilterMode(Points_Hide);
		f = f->next();
	};
}
//--------------------------------------------------------------------
// process Requests
//--------------------------------------------------------------------
void PointsSelect::showAllPoints(bool sel)
{
	PAUSE_POINTS_SELECT

	if (!sel) clearSelectionFilters();

	/*add all voxels to queue that have data (ie lod>0)*/ 		
	PointsScene::VOXELSLIST vlist;
	PointsScene::UseSceneVoxels voxels(vlist);

	PointsScene::VOXELSLIST::iterator i = vlist.begin();
	PointsScene::VOXELSLIST::iterator e = vlist.end();

	while (i!=e)
	{
		ShowPoints show(*i, sel);
		if ((*i)->channel(PCloud_Filter))
		{
			if ((*i)->flag(pcloud::PartHidden))
			{
				boost::try_mutex::scoped_try_lock vlock((*i)->mutex());
				if (vlock.locked())
					(*i)->iterateTransformedPoints(show, pt::ProjectSpace, false);
			}
		}
		(*i)->flag(pcloud::WholeHidden, false);
		(*i)->flag(pcloud::PartHidden, false);
		++i;
	};
	SelectionFilter *f = g_firstFilter;
	while (f)
	{
		f->setSelectionFilterMode(Points_Select);
		f = f->next();
	};
}
//--------------------------------------------------------------------
// process Requests
//--------------------------------------------------------------------
void PointsSelect::refilter()
{
	if (!g_firstFilter) return;

	PAUSE_POINTS_SELECT
	
	SelectionFilter *f = g_firstFilter;
	
	PointsScene::VOXELSLIST vlist;
	PointsScene::UseSceneVoxels voxels(vlist);

	PointsScene::VOXELSLIST::iterator i = vlist.begin();
	PointsScene::VOXELSLIST::iterator e = vlist.end();

	while (i!=e)
	{
		ShowPoints show(*i, false);
		if ((*i)->channel(PCloud_Filter))
		{
			boost::try_mutex::scoped_try_lock vlock((*i)->mutex());
			if (vlock.locked())
				(*i)->iterateTransformedPoints(show, pt::ProjectSpace, false);
		}

		++i;
	};
	f = g_firstFilter;

	while (f)
	{
		f->refilter();
		f = f->next();
	};

}
//--------------------------------------------------------------------
// process Requests
//--------------------------------------------------------------------
void PointsSelect::SelectionFilterThread::processSelectionFilters()
{
	if (!g_firstFilter || g_pause) return;
	g_working = true;

	/*add all voxels to queue that have data (ie lod>0)*/ 		
	PointsScene::VOXELSLIST vlist;
	PointsScene::UseSceneVoxels voxels(vlist);

	PointsScene::VOXELSLIST::iterator i = vlist.begin();
	PointsScene::VOXELSLIST::iterator e = vlist.end();

	pcloud::Voxel *vox = 0;

	if (g_clear)
	{
		g_clear = false;
		SelectionFilter *sel = g_firstFilter->last();
		g_firstFilter = 0;
		
		while (sel)
		{
			SelectionFilter *n = sel->prev();
			delete sel;
			sel = n;
		};
		while (i != e) 
		{ 
			Voxel *v = (*i);

			boost::try_mutex::scoped_try_lock vlock(v->mutex());
			
			if (!v->flag(pcloud::PartHidden) && vlock.locked())
				v->destroyEditChannel();
			
			v->flag(pcloud::WholeSelected, false); 
			v->flag(pcloud::PartSelected, false); 

			++i; 
		}
	}
	else
	{
		while (i != e)
		{
			g_used = true;

			if (g_clear || g_pause) break;
			if (g_firstFilter->needsLock(*i, true))
			{
				boost::try_mutex::scoped_try_lock vlock((*i)->mutex());
				if (vlock.locked())
					g_firstFilter->processVoxel(*i, true);
			}
			++i;
            BeThreadUtilities::BeSleep(2);
		};
	}
	g_working = false;
}