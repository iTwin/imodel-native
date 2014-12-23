#pragma once

#include <pt/plane.h>
#include <pt/rect.h>

#include <ptedit/editSelect.h>

namespace ptedit
{
	class FrustumSelect : public EditNodeDef
	{
	public: 
		FrustumSelect() : EditNodeDef("Frustum") {}
	
		DECLARE_EDIT_NODE( "Frustum", "Rect Sel", 0, EditNodeMultithread | EditNodePostConsolidateVis );

		SELECTION_FILTER_DETAIL

		bool apply() 
		{ 
			SelectionFilter<FrustumSelect> f(*this);
			return f.processFilter(); 
		}
		void draw();

		bool writeState( pt::datatree::Branch *b) const;
		bool readState(const pt::datatree::Branch *);

		/* functions used in selection template */ 
		static void minMaxBoxFromPlane(const pt::BoundingBoxD &box, 
			const pt::Planed &plane, pt::vector3d &p, pt::vector3d &n)
		{
			if (plane.normal().x >= 0)	{	p.x = box.upper(0); n.x = box.lower(0);	}
			else	{	p.x = box.lower(0); n.x = box.upper(0); }
			if (plane.normal().y >= 0)	{	p.y = box.upper(1);	n.y = box.lower(1);	}
			else	{	p.y = box.lower(1);	n.y = box.upper(1);	}
			if (plane.normal().z >= 0)	{	p.z = box.upper(2);	n.z = box.lower(2);	}
			else	{	p.z = box.lower(2);	n.z = box.upper(2);	}
		}

		static SelectionResult intersect(const FrustumSelect &frustum, const pt::BoundingBoxD &box)
		{
			int x; pt::vector3d p,n;

			for (x = 0; x < 4; x++)
			{
				minMaxBoxFromPlane(box, frustum.planes[x], p, n);
				if (frustum.planes[x].whichSide(p) <= 0) break;
			}
			int y;
			for (y = 0; y < 4; y++)
			{
				minMaxBoxFromPlane(box, frustum.planes[y], p, n);
				if (frustum.planes[y].whichSide(n) <= 0) break;
			}
			return (x==4 && y==4) ? FullyInside : ((x==4 || y==4) ? PartiallyInside : FullyOutside);
		}

		inline static bool inside(int thread, const FrustumSelect &frustum, const pt::vector3d &pnt)
		{
			int x[EDT_MAX_THREADS];
			for (x[thread] = 0; x[thread] < 4; x[thread]++)
			{
				if (frustum.planes[x[thread]].whichSide(pnt) <= 0) break;
			}
			return (x[thread]==4) ? true : false;
		}
		inline static bool inside(int th, const FrustumSelect &frustum, const pt::vector3 &pnt)
		{
			return inside(th, frustum, pt::vector3d(pnt));
		}
		void buildFromScreenRect(const pt::Recti &rect, const pt::ViewParams &vparams, double units);

	private:

		pt::vector3d corners[8];
		pt::Planed planes[6];
		bool valid;
	};
}