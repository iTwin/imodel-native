#include <iostream>
#include <ptengine/selectionFilter.h>
#include <ptgl/glviewstore.h>

#include <pt/boundingbox.h>
#include <pt/project.h>

#include <ptengine/queryscene.h>

using namespace pointsengine;
using namespace pt;
using namespace pcloud;

short filtered_in = POINT_SEL_CODE;

SelectionFilter::~SelectionFilter()
{}
//
// Rectangle point filter
//
static vector3d s_ptd, s_spt;
static vector3 s_sptf;
static Voxel* s_vox;
static vector2i s_spti;
static short s_mode;
static short s_val;

mmatrix4d _mid = mmatrix4d::identity();
pointsengine::PointVisibleCondition _clipcond(_mid);

template<class Tester> struct TestProjectedPoint
{
	static void testpoint(const ptgl::Viewstore &vstore, double *pnt, unsigned int index)
	{
		vstore.project3v(pnt, s_spt);
		if (s_spt.z >= 0 && s_spt.z <=1)
		{
			s_spti.set((int)s_spt.x, (int)s_spt.y);
				
			if (Tester::testpoint(s_spti))
			{
				s_vox->channel(PCloud_Filter)->getval(s_val, index);
				if (s_val < POINT_HIDE_CODE)
					s_vox->channel(PCloud_Filter)->set(index, &s_mode);
			}
		}
	}
	static void testpoint(const ptgl::Viewstore &vstore, float *pnt, unsigned int index)
	{
		vstore.project3v(pnt, s_sptf);
		if (s_sptf.z >= 0 && s_sptf.z <=1)
		{
			s_spti.set((int)s_sptf.x, (int)s_sptf.y);
				
			if (Tester::testpoint(s_spti))
			{
				s_vox->channel(PCloud_Filter)->getval(s_val, index);
				if (s_val < POINT_HIDE_CODE)
					s_vox->channel(PCloud_Filter)->set(index, &s_mode);
			}
		}
	}
};
template<class Tester> struct TestClippedProjectedPoint
{
	static void testpoint(const ptgl::Viewstore &vstore, double *pnt, unsigned int index)
	{
		vstore.project3v(pnt, s_spt);
		if (s_spt.z >= 0 && s_spt.z <=1)
		{
			s_spti.set((int)s_spt.x, (int)s_spt.y);
			
			if (Tester::testpoint(s_spti) && _clipcond(vector3d(pnt)))
			{				
				s_vox->channel(PCloud_Filter)->getval(s_val, index);
				if (s_val < POINT_HIDE_CODE)
					s_vox->channel(PCloud_Filter)->set(index, &s_mode);
			}
		}
	}
	static void testpoint(const ptgl::Viewstore &vstore, float *pnt, unsigned int index)
	{
		vstore.project3v(pnt, s_sptf);
		if (s_sptf.z >= 0 && s_sptf.z <=1)
		{
			s_spti.set((int)s_sptf.x, (int)s_sptf.y);
				
			if (Tester::testpoint(s_spti) && _clipcond(vector3(pnt)))
			{
				s_vox->channel(PCloud_Filter)->getval(s_val, index);
				if (s_val < POINT_HIDE_CODE)
					s_vox->channel(PCloud_Filter)->set(index, &s_mode);
			}
		}
	}
};
// Point tests
static Rect<int> s_rect;
struct RectTest { inline static bool testpoint(vector2i pnt) { return s_rect.inBounds(pnt); } };

struct Fence<int> s_fence;
struct FenceTest { inline static bool testpoint(vector2i pnt) { return s_fence.pntInFence(pnt); } };
//
template < class TestPoint >
struct FilterPoint
{
	FilterPoint(ptgl::Viewstore vs, int from=0) : _vstore(vs), _from(from), _count(0) {}
	
	void point(double *pnt, unsigned int index)
	{
		if (_count > _from)
			TestPoint::testpoint(_vstore, pnt, index);
		_count++;
	}
	void point(float *pnt, unsigned int index)
	{
		if (_count > _from)
			TestPoint::testpoint(_vstore, pnt, index);
		_count++;
	}
	ptgl::Viewstore _vstore;
	int			_count;
	int			_from;
};
//
// Fence Selection Filter
//
void FenceSelectionFilter::processVoxel(pcloud::Voxel *v, bool propagate)
{
	int vp = voxelProgress(v);
	if (vp != POINT_FILTER_WHOLE_VOXEL && vp != POINT_FILTER_DONE)
	{
		pt::BoundingBox bb(v->extents());
		vector3 basepoint(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2));

		/* need to project voxel and check corners for selection */ 
		bb.translateBy(-basepoint);
		vector3 pt;
		
		bool do_clip = _useClipMat;

		/* check for clipped out */ 
		if (_useClipMat)
		{
			_clipcond.B = _clipmat;
			int incount = 0;

			for (int i=0; i<8; i++)
			{
				bb.getExtrema(i, pt);
				if (_clipcond(pt)) incount++;
			}
			if (incount == 8) do_clip = false;
			if (!incount)
			{
				setVoxelProgress(v, POINT_FILTER_DONE);		
				return;
			}
		}
		vector3d ptd, spt;
		vector3i spti;

		int in = 0;
		Recti vrect;

		for (int i=0; i<8; i++)
		{
			bb.getExtrema(i, pt);
			ptd.set(pt.x, pt.y, pt.z);
			_vstore.project3v(ptd, spt);
			
			spti.set((int)spt.x, (int)spt.y, 0);

			vrect.expand(spti);
		}
		switch (_fence.intersectsRect(vrect))
		{
		case Fence<int>::Intersects:
			if (vp == POINT_FILTER_INVALID || vp < v->lodPointCount())
			{
				/* we have to do a point by point check */ 
				v->flag(pcloud::PartSelected, true);
				v->buildEditChannel();
				
				s_mode = _mode;
				s_fence = _fence;
				s_vox = v;

				if (do_clip)
				{
					FilterPoint< TestClippedProjectedPoint<FenceTest> > r(_vstore, vp);
					v->iterateTransformedPoints(r, pt::ProjectSpace, false);
				}
				else
				{
					FilterPoint< TestProjectedPoint<FenceTest> > r(_vstore, vp);
					v->iterateTransformedPoints(r, pt::ProjectSpace, false);
				}
				setVoxelProgress(v, v->lodPointCount());
			}
			break;
		case Fence<int>::Contains:		
			setVoxelProgress(v, POINT_FILTER_WHOLE_VOXEL);
			v->flag(pcloud::WholeSelected, true);
			break;
		case Fence<int>::None:
			setVoxelProgress(v, POINT_FILTER_DONE);
			break;
		}
	}
	if (_next && propagate) _next->processVoxel(v, true);
}
bool SelectionFilter::needsLock(pcloud::Voxel *v, bool propagate)
{
	int vp = voxelProgress(v);
	if (vp != POINT_FILTER_WHOLE_VOXEL && vp != POINT_FILTER_DONE)
	{
		return true;
	}
	if (propagate && _next) return _next->needsLock(v, propagate);

	return false;
}

//
// Rectangle Selecttion FIlter
//
void RectSelectionFilter::processVoxel(pcloud::Voxel*v, bool propagate)
{
	int vp = voxelProgress(v);
	if (vp != POINT_FILTER_WHOLE_VOXEL && vp != POINT_FILTER_DONE)
	{
		pt::BoundingBox bb(v->extents());
		vector3 basepoint(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2));

		/* need to project voxel and check corners for selection */ 
		bb.translateBy(-basepoint);

		vector3 pt;
		bool do_clip = _useClipMat;

		/* check for clipped out */ 
		if (_useClipMat)
		{
			_clipcond.B = _clipmat;
			int incount = 0;

			for (int i=0; i<8; i++)
			{
				bb.getExtrema(i, pt);
				if (_clipcond(pt)) incount++;
			}
			if (incount == 8) do_clip = false;
			if (!incount)
			{
				setVoxelProgress(v, POINT_FILTER_DONE);		
				return;
			}
		}
		vector3d ptd, spt;
		vector3i spti;

		int in = 0;
		Recti vrect;

		for (int i=0; i<8; i++)
		{
			bb.getExtrema(i, pt);
			ptd.set(pt.x, pt.y, pt.z);
			_vstore.project3v(ptd, spt);
			
			spti.set((int)spt.x, (int)spt.y, 0);

			vrect.expand(spti);
		}
		/* check for full inclusion */ 
		if (_rect.contains(&vrect)) 
		{
			setVoxelProgress(v, POINT_FILTER_WHOLE_VOXEL);
			v->flag(pcloud::PartSelected, true);
		}
		/* complete exclusion */ 
		else if (vrect.contains(&_rect) || _rect.intersects(&vrect))
		{
			if (vp == POINT_FILTER_INVALID || vp < v->lodPointCount())
			{
				/* we have to do a point by point check */ 
				v->flag(pcloud::PartSelected, true);
				v->buildEditChannel();

				s_mode = _mode;
				s_rect = _rect;
				s_vox = v;

				if (do_clip)
				{
					FilterPoint< TestClippedProjectedPoint<RectTest> > r(_vstore, vp);
					v->iterateTransformedPoints(r, pt::ProjectSpace, false);
				}
				else
				{
					FilterPoint< TestProjectedPoint<RectTest> > r(_vstore, vp);
					v->iterateTransformedPoints(r, pt::ProjectSpace, false);
				}
				setVoxelProgress(v, v->lodPointCount());
			}
		}
		else
		{
			setVoxelProgress(v, POINT_FILTER_DONE);
		}
	}
	if (_next && propagate) _next->processVoxel(v, true);
}