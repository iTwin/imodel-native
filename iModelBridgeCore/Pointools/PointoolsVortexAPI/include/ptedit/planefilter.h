#pragma once
#include <ptedit/edit.h>
#include <ptedit/editSelect.h>
#include <ptedit/editNodeDef.h>
#include <pt/fence.h>
#include <pt/plane.h>

#include <vector>
namespace ptedit
{
	struct PlaneSelect : public EditNodeDef
	{
		PlaneSelect() : EditNodeDef("PlaneSelect"), thickness(0.25), unbounded(true) {};
		
		SELECTION_FILTER_DETAIL

		const pt::String &name() const { static pt::String p("PlaneSelect"); return p; }
		const pt::String &desc() const 
		{ 
			static pt::String s;
			s.format("Planar: %0.2fm %s", thickness, unbounded ? "UB" : " ");
			return s; 
		}
		void setPlane( pt::vector3d origin, pt::vector3d normal )
		{
			plane.base(origin);
			plane.normal(normal);
		}

		bool apply() 
		{
			SelectionFilter<PlaneSelect> p(*this);
			p.processFilter();

			return p.didSelect() ? true : false;
		}
		int icon() const { return 5; }
		uint flags() const { return EditNodeMultithread | EditNodePostConsolidateSel; }

		void addPoint(const pt::vector3d &pnt);
		void computePlane();
		void draw();

		void clear();
		bool readState(const pt::datatree::Branch *b);
		bool writeState(pt::datatree ::Branch *b) const;

		static void minMaxBoxFromPlane(const pt::BoundingBoxD &box, 
			const pt::Planed *plane, pt::vector3d &p, pt::vector3d &n);

		static SelectionResult intersect(const PlaneSelect &f, const pt::BoundingBoxD &box);

		inline static bool inside(int th, const PlaneSelect &f, const pt::vector3d &pnt)
		{
			pt::vec2<double> pnt2;
			f.plane.to2D(pnt, pnt2);
			return (fabs(f.plane.distToPlane(pnt)) < f.thickness && 
				(f.unbounded || f.fence.pntInFence(pnt2)));
		}
		double thickness;
		bool unbounded;

	private:
		pt::Planed plane;
		std::vector<pt::vector3d> points;
		pt::Fenced fence;
	};
} // end namespace ptedit