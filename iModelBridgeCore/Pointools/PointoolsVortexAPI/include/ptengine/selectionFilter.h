#ifndef POINTOOLS_CLOUD_SELECTION_H
#define POINTOOLS_CLOUD_SELECTION_H

#include <ptengine/ptengine_api.h>
#include <ptcloud2/voxel.h>
#include <pt/rect.h>
#include <pt/fence.h>
#include <ptgl/glviewstore.h>

#define POINT_FILTER_WHOLE_VOXEL	-1
#define POINT_FILTER_DONE			-3
#define POINT_FILTER_INVALID		-2

#define POINT_SHOW_CODE -32767
#define POINT_SEL_CODE	1

#ifndef POINT_HIDE_CODE
#define POINT_HIDE_CODE 32000
#define POINT_HIDE_ONLY_CODE 32600
#endif


namespace pointsengine
{
enum SelectionFilterMode
{
	Points_Hide = POINT_HIDE_CODE, 
	Points_Show = POINT_SHOW_CODE,
	Points_Select = POINT_SEL_CODE
};	
class PTENGINE_API SelectionFilter
{
public:
	enum Operand { Select_ADD=1, Select_REM=2, Select_TOGGLE=3 };

	SelectionFilter(Operand o=Select_ADD) 
	{ 
		_operand = o; _prev = 0; _next=0; _mode = (short)Points_Select; 
		_useClipMat = false;
	}
	virtual ~SelectionFilter();

	Operand operand() const { return _operand; }

	virtual void processVoxel(pcloud::Voxel *v, bool propagate)=0;
	void useClipMatrix(const mmatrix4d &clipmat) { _useClipMat = true; _clipmat = clipmat; }

#ifndef __INTEL_COMPILER
	SelectionFilter *next() { return _next; }
	SelectionFilter *prev() { return _prev; }
	SelectionFilter *first() { if (_prev) return _prev->first(); else return this; }
	SelectionFilter *last() { if (_next) return _next->last(); else return this; }
	bool needsLock(pcloud::Voxel *v, bool propagate);
	void refilter() { _voxelProgress.clear(); }
	void setSelectionFilterMode(SelectionFilterMode m) { _mode = (short)m; }
#endif

protected:	
	friend class SelectionFilter;
	friend class PointsSelect;

	void addFilter(SelectionFilter *next) { _next = next; _next->_prev = this; }

	int voxelProgress(pcloud::Voxel *v)
	{
		std::map<pcloud::Voxel*, int>::iterator it = _voxelProgress.find(v);
		return it == _voxelProgress.end() ? POINT_FILTER_INVALID : it->second;
	}
	void setVoxelProgress(pcloud::Voxel *v, int p)
	{
		std::map<pcloud::Voxel*, int>::iterator it = _voxelProgress.find(v);
		if (it == _voxelProgress.end())
			_voxelProgress.insert(std::pair<pcloud::Voxel*, int>(v, p));
		else
			it->second = p;
	}
	int selectTo(pcloud::Voxel * v)
	{
		if (_prev)
		{
			if (_prev->selectTo(v) == POINT_FILTER_WHOLE_VOXEL
				&& _prev->operand() == _operand)
			{
				return POINT_FILTER_WHOLE_VOXEL;		
			}
		}
		return voxelProgress(v);
	}
	/* double linked list */ 
#ifdef __INTEL_COMPILER
	SelectionFilter *next() { return _next; }
	SelectionFilter *prev() { return _prev; }
	SelectionFilter *first() { if (_prev) return _prev->first(); else return this; }
	SelectionFilter *last() { if (_next) return _next->last(); else return this; }
	bool needsLock(pcloud::Voxel *v, bool propagate);
	void refilter() { _voxelProgress.clear(); }
	void setSelectionFilterMode(SelectionFilterMode m) { _mode = (short)m; }
#endif
	Operand _operand;
	short	 _mode;
	SelectionFilter *_prev;
	SelectionFilter *_next;
	std::map<pcloud::Voxel*, int> _voxelProgress;
	mmatrix4d	_clipmat;
	bool	_useClipMat;
};
//
class PTENGINE_API RectSelectionFilter : public SelectionFilter
{
public:
	RectSelectionFilter(pt::Rect<int> rect) : _rect(rect) {}

	void captureView()	{ _vstore.store(); }
	void processVoxel(pcloud::Voxel *v, bool propagate);

	ptgl::Viewstore _vstore;
	pt::Rect<int> _rect;
};
//
class PTENGINE_API FenceSelectionFilter : public SelectionFilter
{
public:	
	FenceSelectionFilter(pt::Fence<int> fence) : _fence(fence) {}

	void captureView()	{ _vstore.store(); }
	void processVoxel(pcloud::Voxel *v, bool propagate);

	ptgl::Viewstore _vstore;
	pt::Fence<int> _fence;
};
}

#endif