#include "PointoolsVortexAPIInternal.h"
#ifdef HAVE_WILDMAGIC
#include <wildmagic/math/Wm5Plane3.h>
#include <wildmagic/math/Wm5ApprPlanefit3.h>
#endif

#include <ptedit/planefilter.h>

using namespace ptedit;
using namespace pt;

void  PlaneSelect::computePlane()
{
	/* least squares fit a plane*/ 
	if (points.size() < 3) return;
	if (points.size() == 3)
	{
		plane.from3points(points[0], points[1], points[2]);
	}
	else
	{
#ifdef HAVE_WILDMAGIC
		Wm5::Vector3d mgcPoints[500];
		Wm5::Vector3d planeOrigin, planeNormal;
#else
        pt::vector3d mgcPoints[500];
        pt::vector3d planeOrigin, planeNormal;
#endif
		
		int size = static_cast<int>(points.size());
		if (size > 500) size = 500;

		for (int i=0; i<size; i++)
		{
#ifdef HAVE_WILDMAGIC
			mgcPoints[i].X() = points[i].x;
			mgcPoints[i].Y() = points[i].y;
			mgcPoints[i].Z() = points[i].z;
#else
            mgcPoints[i] = points[i];
#endif
		}
		
#ifdef HAVE_WILDMAGIC
		Wm5::Plane3d wplane = Wm5::OrthogonalPlaneFit3( size, mgcPoints );
		plane.normal( (const double*)wplane.Normal );
		plane.constant( wplane.Constant );
#else
        // &&RB TODO: the following geomlibs function call must be tested in this context
        bvector<DPoint3d> geomlibs_points(points.size());
        memcpy(geomlibs_points.data(), points.data(), points.size());
        DVec3d centroid, moments;
        RotMatrix axes;
        DPoint3dOps::PrincipalAxes(geomlibs_points, centroid, axes, moments);
        // &&RB TODO: set plane object members to proper values from geomlibs centroid, axes and moment structures:
        //! @param [out] axes principal axes.
        //! @param [out] moments second moments wrt x,y,z -- sums of (yy+zz,xx+zz,xx+yy)
        //! @remarks axes are orderd so x is largest moment, y next, z smallest.
#endif
		plane.base();
	}
	/* build the fence */ 
	fence.clear();
	if (points.size() >= 3)
	{
		for (size_t i=0; i<points.size(); i++)
		{
			vec2<double> p2;
			plane.to2D(points[i], p2);
			fence.addPoint(p2);
		}
	}
}
void  PlaneSelect::draw() {}
bool PlaneSelect::readState(const pt::datatree::Branch *b)
{
	pt::Planed::VectorType n, base;

	if (!b->getNode("planeNormal", n)) return false;
	b->getNode("planeBase", base);

	plane.normal(n);
	plane.base(base);

	b->getNode("thickness", thickness);
	b->getNode("unbounded", unbounded);

	pt::Fenced::PointType fpnts[255];
	int numPoints;

	b->getNode("numFencePoints", numPoints);
	const pt::datatree::Blob* bl = b->getBlob("fencePoints");
	memcpy(fpnts, bl->_data, bl->_size);

	fence.clear();
	for (int i=0; i<numPoints; i++) fence.addPoint(fpnts[i]);

	return true;
}
bool  PlaneSelect::writeState(pt::datatree ::Branch *b) const
{
	if (fence.numPoints() < 3) return false;
	pt::Planed::VectorType n, base;

	b->addNode("planeNormal", plane.normal());
	b->addNode("planeBase", plane.base());
	b->addNode("thickness", thickness);
	b->addNode("unbounded", unbounded);
	
	b->addNode("numFencePoints", (int)fence.numPoints()); 
	b->addBlob("fencePoints", sizeof(pt::Fenced::PointType)*fence.numPoints(), (void*)&fence[0], true);
	return true;
}
void PlaneSelect::addPoint(const pt::vector3d &pnt)
{
	if (!points.size() || pnt != points.back())
	{
		points.push_back(pnt);
		computePlane();
	}
}
void PlaneSelect::clear()
{
	points.clear();
	fence.clear();
}

/* return corners of the box that are the min dist and max distance from the plane */ 
void PlaneSelect::minMaxBoxFromPlane(const pt::BoundingBoxD &box, 
	const pt::Planed *pl, pt::vector3d &p, pt::vector3d &n)
{
	if (pl->normal().x >= 0)	{	p.x = box.upper(0); n.x = box.lower(0);	}
	else	{	p.x = box.lower(0); n.x = box.upper(0); }
	if (pl->normal().y >= 0)	{	p.y = box.upper(1);	n.y = box.lower(1);	}
	else	{	p.y = box.lower(1);	n.y = box.upper(1);	}
	if (pl->normal().z >= 0)	{	p.z = box.upper(2);	n.z = box.lower(2);	}
	else	{	p.z = box.lower(2);	n.z = box.upper(2);	}
}
/* check plane box intersection */ 
SelectionResult PlaneSelect::intersect(const PlaneSelect &f, const pt::BoundingBoxD &box)
{
	pt::vector3d n, p;
	minMaxBoxFromPlane(box, &f.plane, p, n);
	int sidep = f.plane.whichSide(p);
	int siden = f.plane.whichSide(n);
	double dp = f.plane.distToPlane( p );
	double dn = f.plane.distToPlane( n );
	SelectionResult res = FullyOutside;

	/* check plane */ 
	if (sidep == siden)
	{
		if (fabs(dp) < f.thickness)
		{
			if (fabs(dn) < f.thickness) res = FullyInside;
			else res = PartiallyInside;
		}
		else
		{
			if (fabs(dn) > f.thickness) return FullyOutside;
			else res = PartiallyInside;
		}
	}
	else res = PartiallyInside;

	if (f.unbounded) return res;

	/* check for overlap */ 
	pt::Rectd boxr;
	boxr.makeEmpty();
	pt::vector3d v;
	pt::vec2<double> v2;
	for (int i=0; i<8; i++)
	{
		box.getExtrema(i, v);
		f.plane.to2D(v, v2);
		boxr.expand(v2);
	}
	if (boxr.contains(&f.fence.bounds())) return PartiallyInside;

	pt::Fenced::FenceIntersectResult fres = f.fence.intersectsRect(boxr);

	if (fres == pt::Fenced::Contains)
		return res == PartiallyInside ? PartiallyInside : FullyInside;

	if (fres == pt::Fenced::Intersects)
		return PartiallyInside;

	return FullyOutside;
}
