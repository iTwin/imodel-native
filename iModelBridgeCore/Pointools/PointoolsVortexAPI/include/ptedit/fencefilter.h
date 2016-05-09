#pragma once
#include <ptedit/editSelect.h>

#include <ptgl/glviewstore.h>
#include <pt/fence.h>
#include <pt/plane.h>
#include <pt/datatree.h>

#include <pt/ptstring.h>
#include <vector>
namespace ptedit
{
	struct FenceSelect: public EditNodeDef
	{		
		FenceSelect() : EditNodeDef("Fence"), maxDepth(0), isValid(false), isPerspective(true), isConvex(false), units(1.0)
		{};
		
		SELECTION_FILTER_DETAIL

			void buildFromScreenFence(const pt::Fence<int> &f, const pt::ViewParams &vparams, double units);
		void draw();
	
		bool apply() 
		{ 
			SelectionFilter<FenceSelect> f(*this);
			return f.processFilter(); 
		}

		const pt::String &name() const { static pt::String n("Fence"); return n; }
		const pt::String &desc() const 
		{ 
			static pt::String s; 
			s.format("Fence : %dpnts", fence.numPoints()); 
			return s; 
		}
		int icon() const { return 1; }
		uint flags() { return EditNodeMultithread | EditNodePostConsolidateSel; } 

		void clear()
		{
			fence.clear();
		}
		bool writeState(pt::datatree::Branch *b) const
		{
			b->addNode("numFencePoints", (int)fence.numPoints()); 
			b->addNode("unitsPerMeter", units); 
			b->addNode("isPerspective", isPerspective);
			b->addBlob("fencePoints", sizeof(pt::Fenced::PointType)*fence.numPoints(), (void*)&fence[0], true);
			b->addBlob("modelMatrix", sizeof(mmatrix4d), (void*)modelm.data(), true);
			return true;
		}
		bool readState(const pt::datatree::Branch *b)
		{
			int numPoints = 0;
			pt::Fenced::PointType points[255];

			if (!b->getNode("numFencePoints", numPoints)) return false;
			const pt::datatree::Blob* bl = b->getBlob("fencePoints");
			memcpy(points, bl->_data, bl->_size);

			fence.clear();
			for (int i=0; i<numPoints; i++) fence.addPoint(points[i]);

			memcpy(&modelm, b->getBlob("modelMatrix")->_data, sizeof(mmatrix4d));
			b->getNode("isPerspective", isPerspective);
			b->getNode("unitsPerMeter", units); 
			isValid = generateHullPlanes();
			return true;
		}
		bool generateHullPlanes();

	// the filtering bit
	static void minMaxBoxFromPlane(const pt::BoundingBoxD &box, 
			const pt::Planed *plane, pt::vector3d &p, pt::vector3d &n)
	{
		if (plane->normal().x >= 0)	{	p.x = box.upper(0); n.x = box.lower(0);	}
		else	{	p.x = box.lower(0); n.x = box.upper(0); }
		if (plane->normal().y >= 0)	{	p.y = box.upper(1);	n.y = box.lower(1);	}
		else	{	p.y = box.lower(1);	n.y = box.upper(1);	}
		if (plane->normal().z >= 0)	{	p.z = box.upper(2);	n.z = box.lower(2);	}
		else	{	p.z = box.lower(2);	n.z = box.upper(2);	}
	}
	static SelectionResult intersect(const FenceSelect &f, const pt::BoundingBoxD &box)
	{
		if (!f.isValid) return FullyOutside;
		int numPlanes = static_cast<int>(f.hullPlanes.size());

		int x; pt::vector3d p,n;

		for (x = 0; x < numPlanes; x++)
		{
			minMaxBoxFromPlane(box, &f.hullPlanes[x], p, n);
			if (f.hullPlanes[x].whichSide(p) <= 0) break;
		}
		int y;
		for (y = 0; y < numPlanes; y++)
		{
			minMaxBoxFromPlane(box, &f.hullPlanes[y], p, n);
			if (f.hullPlanes[y].whichSide(n) <= 0) break;
		}
		return (x==numPlanes && y==numPlanes) ? (f.isConvex ? FullyInside  : PartiallyInside) : ((x==numPlanes || y==numPlanes) ? PartiallyInside : FullyOutside);
	}
	static void eyeSpaceBoundingBox(const FenceSelect &fence, const pt::BoundingBox &bb, pt::Rect<double> &r)
	{
		pt::vector3 v;
		pt::vector3d vp;
		r.makeEmpty();

		for (int i=0; i<8; i++)
		{
			bb.getExtrema(i, v);
			fence.modelm.vec3_multiply_mat4(pt::vector3d(v), vp);
			if (fence.isPerspective)
			{
				vp.x /= vp.z;
				vp.y /= vp.z;
			}
			r.expand(vp);
		}
	}
	inline static bool insideFence(int th, double x, double y, const pt::Fenced &f)
	{
		int size[EDT_MAX_THREADS];
		size[th] = f.numPoints();

		if (size[th] > 2)
		{
			bool oddNodes[] ={ false, false, false, false }; 

			float x_i[EDT_MAX_THREADS];
			float x_j[EDT_MAX_THREADS];
			float y_i[EDT_MAX_THREADS];
			float y_j[EDT_MAX_THREADS];

			int j[] = { 0, 0 };
			int i[EDT_MAX_THREADS];

			for (i[th]=0; i[th]<size[th]; i[th]++)
			{
				++j[th];
				
				x_i[th] = static_cast<float>(f.point(i[th]).x);
				y_i[th] = static_cast<float>(f.point(i[th]).y);
				x_j[th] = static_cast<float>(f.point(j[th]%size[th]).x);
				y_j[th] = static_cast<float>(f.point(j[th]%size[th]).y);

				if (y_i[th] < y && y_j[th] >= y || y_j[th] < y && y_i[th] >= y)
					if (x_i[th] + (y - y_i[th]) /(y_j[th] - y_i[th]) * (x_j[th] - x_i[th]) < x)
						oddNodes[th] = !oddNodes[th];
			}
			return oddNodes[th];
		}
		else false;
	}
	inline static bool inside(int th, const FenceSelect &f, const pt::vector3d &pnt)
	{
		pt::vector3d epnt[EDT_MAX_THREADS];
		pt::vec2<double> epnt2[EDT_MAX_THREADS];

		f.modelm.vec3_multiply_mat4(pnt, epnt[th]);

		if (f.isPerspective)
		{
			if (epnt[th].z > 0) return false;

			epnt2[th].x = epnt[th].x / epnt[th].z;
			epnt2[th].y = epnt[th].y / epnt[th].z;

			return f.fence.pntInFence(epnt2[th].x, epnt2[th].y);	
		}
		else
			return f.fence.pntInFence(epnt[th].x, epnt[th].y);	

	}
	/* data */ 
	pt::Fenced 	fence;
	mmatrix4d 	modelm;
	std::vector <pt::Planed> hullPlanes;

	float 	maxDepth;
	bool	isPerspective;
	bool 	isConvex;
	bool 	isValid;
	double 	units;
};
} // end namespace ptedit