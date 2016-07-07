#include "PointoolsVortexAPIInternal.h"
#include <pt/OBBox.h>
#ifdef HAVE_WILDMAGIC
#include <wildmagic/math/Wm5ApprGaussPointsFit3.h>
#endif

namespace pt
{
/*
OBBoxf	createFittingOBBf( const vector3 *pts, int numPoints )
{
	Wm5::Box3f wbox= Wm5::GaussPointsFit3<float>(numPoints, (Wm5::Vector3f*)pts);
	vector3 axis[] = { &wbox.Axis[0].X(), &wbox.Axis[1].X(), &wbox.Axis[2].X() };

	OBBoxf box( &wbox.Center.X(), axis, wbox.Extent );

	// Let C be the box center and let U0, U1, and U2 be the box axes.  Each
	// input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.  The
	// following code computes min(y0), max(y0), min(y1), max(y1), min(y2),
	// and max(y2).  The box center is then adjusted to be
	//   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1 +
	//        0.5*(min(y2)+max(y2))*U2

	vector3 diff = pts[0] - box.center();


	vector3 pmin(diff.dot(box.axis(0)), diff.dot(box.axis(1)),
		diff.dot(box.axis(2)));

	vector3 pmax = pmin;

	for (int i = 1; i < numPoints; ++i)
	{
		diff = pts[i] - box.center();
		for (int j = 0; j < 3; ++j)
		{
			float dot = diff.dot(box.axis(j));
			if (dot < pmin[j])
			{
				pmin[j] = dot;
			}
			else if (dot > pmax[j])
			{
				pmax[j] = dot;
			}
		}
	}

	vector3 cen(box.center());
	cen +=
		box.axis(0) * (((float)0.5)*(pmin[0] + pmax[0])) +
		box.axis(1) * (((float)0.5)*(pmin[1] + pmax[1])) +
		box.axis(2) * (((float)0.5)*(pmin[2] + pmax[2]));

	box.center(cen);

	box.extent(0,((float)0.5)*(pmax[0] - pmin[0]));
	box.extent(1,((float)0.5)*(pmax[1] - pmin[1]));
	box.extent(2,((float)0.5)*(pmax[2] - pmin[2]));

	return box;
}
*/
/*
OBBoxf	createFittingOBBf( const std::vector< vector3 > &pts )
{
	return createFittingOBBf( &pts[0], pts.size() );
}
*/


OBBoxd createFittingOBBd( const std::vector<vector3d> &pts )
{
	return createFittingOBBd(&pts[0], static_cast<int>(pts.size()));
}


OBBoxd	createFittingOBBd(const vector3d *pts, int numPoints)
{
#ifdef HAVE_WILDMAGIC
	Wm5::Box3d wbox= Wm5::GaussPointsFit3<double>(numPoints, (Wm5::Vector3d*) pts);
	vector3d axis[] = { &wbox.Axis[0].X(), &wbox.Axis[1].X(), &wbox.Axis[2].X() };
	OBBoxd box( &wbox.Center.X(), axis, wbox.Extent );
	// Let C be the box center and let U0, U1, and U2 be the box axes.  Each
	// input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.  The
	// following code computes min(y0), max(y0), min(y1), max(y1), min(y2),
	// and max(y2).  The box center is then adjusted to be
	//   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1 +
	//        0.5*(min(y2)+max(y2))*U2

	vector3d diff = pts[0] - box.center();


	vector3d pmin(diff.dot(box.axis(0)), diff.dot(box.axis(1)),
		diff.dot(box.axis(2)));

	vector3d pmax = pmin;

	for (int i = 1; i < numPoints; ++i)
	{
		diff = pts[i] - box.center();
		for (int j = 0; j < 3; ++j)
		{
			double dot = diff.dot(box.axis(j));
			if (dot < pmin[j])
			{
				pmin[j] = dot;
			}
			else if (dot > pmax[j])
			{
				pmax[j] = dot;
			}
		}
	}

	vector3d cen(box.center());
	cen +=
		box.axis(0) * (((double)0.5)*(pmin[0] + pmax[0])) +
		box.axis(1) * (((double)0.5)*(pmin[1] + pmax[1])) +
		box.axis(2) * (((double)0.5)*(pmin[2] + pmax[2]));

	box.center(cen);

	box.extent(0,((double)0.5)*(pmax[0] - pmin[0]));
	box.extent(1,((double)0.5)*(pmax[1] - pmin[1]));
	box.extent(2,((double)0.5)*(pmax[2] - pmin[2]));

#else
    // &&RB TODO: the following geomlibs function call must be tested in this context
    bvector<DPoint3d> points(numPoints);
    memcpy(points.data(), pts, numPoints);

    BENTLEY_NAMESPACE_NAME::Transform transform;
    DPoint3dOps::PrincipalExtents(points, transform);

    // Set box object members to proper values from geomlibs transform structure:
    // PrincipalExtents transform with axes in the direction of principal axes.
    //         origin at lower left of a tight (principal axes) bounding box.
    //         x,y,z column vectors are full extent along the bounding box.
    //         z column is the smallest column.
    //         z column has positive z if possible.
    //         x column has positive x if possible.
    DVec3d origin, px, py, pz;
    transform.GetOriginAndVectors(origin, px, py, pz);

    DVec3d mx(origin + 0.5*px), my(origin + 0.5*py), mz(origin + 0.5*pz);
    OBBoxd box;
    DVec3d cen(mx + my + mz);
    box.center(vector3d(cen.x, cen.y, cen.z));
    //vector3d axis[] = { &px.x, &py.x, &pz.x };

    box.extent(0, 0.5*bsiDVec3d_magnitude(&px));
    box.extent(1, 0.5*bsiDVec3d_magnitude(&py));
    box.extent(2, 0.5*bsiDVec3d_magnitude(&pz));

#endif

	return box;
}

/*
OBBoxf	createFittingAABBf( const vector3 *pts, int numPoints )
{
	// simple AABB fit
	BoundingBox bb;
	
	for (int i=0; i<numPoints; i++)
	{
		const vector3 &pnt = pts[i];
		bb.expand(pnt);
	}

	OBBoxf box;
	box.axis(0,vector3(1.0f,0,0));
	box.axis(1,vector3(0,1.0f,0));
	box.axis(2,vector3(0,0,1.0f));

	box.extents( vector3(bb.dx()*0.5, bb.dy()*0.5, bb.dz()*0.5) );
	box.center( bb.center() );

	return box;
}
*/
/*
OBBoxf	createFittingAABBf( const std::vector< vector3 > &pts )
{
	return createFittingAABBf( &pts[0], pts.size() );
}
*/
OBBoxd	createFittingAABBd(const vector3d *pts, int numPoints)
{
	// simple AABB fit
	BoundingBoxD bb;

	for (int i=0; i<numPoints; i++)
	{
		const vector3d &pnt = pts[i];
		bb.expand(pnt);
	}

	OBBoxd box;
	box.axis(0, vector3d(1.0f,0,0));
	box.axis(1, vector3d(0,1.0f,0));
	box.axis(2, vector3d(0,0,1.0f));

	box.extents(vector3d(bb.dx()*0.5, bb.dy()*0.5, bb.dz()*0.5) );
	box.center(bb.center());

	return box;
}
/*
bool createFittingABBf( OBBoxf &box, const vector3 *pts, int numPoints )
{
	throw "createFittingABBf not implemented";
	return false;
}
*/
/*
bool createFittingABBd( OBBoxf &box, const vector3d *pts, int numPoints )
{
	throw "createFittingABBd not implemented";
	return false;
}
*/
}
