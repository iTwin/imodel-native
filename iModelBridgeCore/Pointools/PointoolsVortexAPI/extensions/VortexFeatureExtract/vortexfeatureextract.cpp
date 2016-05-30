// VortexFeatureFit.cpp : Defines the entry point for the DLL application.
//

#include "Includes.h"
#include "QueryBuffer.h"
#include "ClusterAnalyser.h"

#include "Plane.h"

#ifdef HAVE_WILDMAGIC
#include <wildmagic/math/Wm5ApprCylinderFit3.h>
#include <wildmagic/math/Wm5ApprPlaneFit3.h>
#endif

//#define ENABLE_CLUSTER_ANALYSIS	1

namespace vortex
{

struct Col3ub
{ 
	Col3ub(PTubyte _r, PTubyte _g, PTubyte _b )
		: r(_r), g(_g), b(_b) {}
	
	Col3ub(){};

	void random()
	{
		r = 64 + rand() * 190.0f / RAND_MAX;
		g = 64 + rand() * 190.0f / RAND_MAX;
		b = 64 + rand() * 190.0f / RAND_MAX;
	}
	PTubyte r;
	PTubyte g;
	PTubyte b;
};

template <typename T>
T	FitCylinderToPoints( PThandle query, Cylinder<T> &res_cylinder, 
						 bool constrainToAxis, bool constrainToRadius )
{
	Cylinder<T>	cylinder(res_cylinder);

	// run query
	QueryBuffer<T> buffer((int)5e5);
	buffer.initialize(false, false);
	buffer.setQuery( query );

	T		min_error = 1e6;
	int		min_error_cluster = -1;

	int		num_pnts=0;
	int		i=0;
	T		error0 = 0, error1 = 0;
	T		radius = cylinder.radius;

	std::vector< Cylinder<T> > cylinders;

#ifdef ENABLE_CLUSTER_ANALYSIS

	// put results in the diagnostics channel if there is one
	PThandle channel = ptGetChannelByName( L"diagnostic" );

	ClusterAnalyser clusters( &buffer, 0.025f );

	if (clusters.extractFastClusters( true, false ))
	{
		// test clusters for best fit
		for ( i=0;i<clusters.numClusters();i++)
		{
			const Wm5::Vector3<T> *candidate_pnts = reinterpret_cast<const Wm5::Vector3<T> *>
				(clusters.getPointsCluster(i, num_pnts));

			if (num_pnts<5) continue;	// ignore small numbers of points

#else
			buffer.executeQuery();
#ifdef HAVE_WILDMAGIC
			const Wm5::Vector3<T> *candidate_pnts = reinterpret_cast<const Wm5::Vector3<T> *>(buffer.getPointsBuffer());
#else
			const Vector3<T> *candidate_pnts = reinterpret_cast<const Vector3<T> *>(buffer.getPointsBuffer());
#endif
			num_pnts = buffer.numPntsInQueryIteration();

			if (num_pnts<5) return 0;
#endif	

#ifdef HAVE_WILDMAGIC
			Wm5::Vector3<T> cen, cen2;
			Wm5::Vector3<T> axis;
#else
			Vector3<T> cen, cen2;
			Vector3<T> axis;
#endif
			T height = 0;

			Cylinder<T> candidate_cylinders[10];
			T errors[10];
			for (int j=0;j<10;j++) errors[j] =1e6;

#ifdef HAVE_WILDMAGIC
			min_error = error0 = Wm5::CylinderFit3<T> (num_pnts, 
				candidate_pnts, cen, axis, radius, height, 
				(radius > 0 && !cylinder.axis.isZero()) || constrainToAxis);
#else
            // &&RB TODO: replace wilmagic function with geomlibs function
            // &&RB TODO: the following geomlibs function call must be tested
#endif
		
			cen2 = cen;
			cen2 -= axis * height * 0.5f; 
#ifdef HAVE_WILDMAGIC
			cylinder.base.set(cen2.X(),cen2.Y(),cen2.Z());
			cylinder.axis.set(axis.X(),axis.Y(),axis.Z());
#else
			cylinder.base.set(cen2.x,cen2.y,cen2.z);
			cylinder.axis.set(axis.x,axis.y,axis.z);
#endif
			cylinder.height = height;
			cylinder.radius = radius;

			// multiple
			for (int j = 1; j <= 10; j++)
			{
#ifdef HAVE_WILDMAGIC
				error1 = Wm5::CylinderFit3<T> (num_pnts, 
					candidate_pnts, cen, axis, radius, height, true);
#else
            // &&RB TODO: replace wilmagic function with geomlibs function
            // &&RB TODO: the following geomlibs function call must be tested
#endif

				if (error1 < min_error)
				{
					cen2 = cen;
					cen2 -= axis * height * 0.5f; 
#ifdef HAVE_WILDMAGIC
        			cylinder.base.set(cen2.X(),cen2.Y(),cen2.Z());
        			cylinder.axis.set(axis.X(),axis.Y(),axis.Z());
#else
        			cylinder.base.set(cen2.x,cen2.y,cen2.z);
        			cylinder.axis.set(axis.x,axis.y,axis.z);
#endif
					cylinder.height = height;
					cylinder.radius = radius;

					min_error = error1;
				}
				if ( fabs(error1-error0) < 0.001)
				{
					break;
				}
				error0=error1;
			}
			cylinders.push_back(cylinder);

			// compute RMS
			float total_sq_error=0;
			for (int j=0; j<num_pnts; j++)
			{
				const Vector3<T> &p( *reinterpret_cast<const Vector3<T>*>(&candidate_pnts[j]) );
				T d = cylinder.distanceToPnt( p );
				total_sq_error += d * d;
			}
			total_sq_error /= num_pnts;

			if (min_error > total_sq_error)
			{
				min_error = total_sq_error;
				min_error_cluster = i;
			}
#ifdef ENABLE_CLUSTER_ANALYSIS
		}
#endif
		// return cylinder with least error
		if (cylinders.size() && min_error_cluster>=0)
		{
			cylinder = cylinders[min_error_cluster];
			
			if (constrainToAxis)
			{
				cylinder.axis = res_cylinder.axis;
			}
			if (constrainToRadius)
			{
				cylinder.radius = res_cylinder.radius;
			}
			res_cylinder = cylinder;

			return sqrt(min_error);
		}

#ifdef ENABLE_CLUSTER_ANALYSIS
	}
#endif
	return -1;
}
//-----------------------------------------------------------------------------
float	PTVFIT_API FitCylinderToPointsf( PThandle query, Cylinderf &res_cylinder, 
						bool constrainToAxis, bool constrainToRadius )
//-----------------------------------------------------------------------------
{
	return FitCylinderToPoints( query, res_cylinder, constrainToAxis, constrainToRadius );
}
//-----------------------------------------------------------------------------
double	PTVFIT_API FitCylinderToPointsd( PThandle query, Cylinderd &res_cylinder, 
										bool constrainToAxis, bool constrainToRadius )
//-----------------------------------------------------------------------------
{
	return FitCylinderToPoints( query, res_cylinder, constrainToAxis, constrainToRadius );
}
//-----------------------------------------------------------------------------
template <typename T>
#ifdef HAVE_WILDMAGIC
static T computeRMSToPlane( const Wm5::Plane3<T> &plane, int num_points, const Wm5::Vector3<T> *points )
#else
static T computeRMSToPlane( const vortex::Plane<T> &plane, int num_points, const Vector3<T> *points )
#endif
//-----------------------------------------------------------------------------
{
	T rms=0;
	for (int i=0; i<num_points; i++)
	{
#ifdef HAVE_WILDMAGIC
		T d = plane.DistanceTo( points[i] );
#else
		T d = plane.distToPlane( points[i] );
#endif
        
		rms += d * d;
	}
	return sqrt(rms/num_points);
}
//-----------------------------------------------------------------------------
template <typename T>
T	FitPlaneToPoints( QueryBuffer<T> &pointsBuffer, Vector3<T> &planeNormal, 
					 Point3<T> &planeOrigin, bool constrainToNormal )
//-----------------------------------------------------------------------------
{
	T	min_error = 1e6;
	int		min_error_cluster = -1;

	int		i=0;
	T	error0, error1;

	int		num_pnts=pointsBuffer.numPntsInQueryIteration();

#ifdef HAVE_WILDMAGIC
	const Wm5::Vector3<T> *candidate_pnts = 
		reinterpret_cast<const Wm5::Vector3<T> *>(pointsBuffer.getPointsBuffer());

    Wm5::Plane3<T> plane =  Wm5::OrthogonalPlaneFit3( 
		pointsBuffer.numPntsInQueryIteration(), candidate_pnts );
#else
    const Vector3<T> *candidate_pnts = reinterpret_cast<const Vector3<T> *>(pointsBuffer.getPointsBuffer());

    // &&RB TODO: replace wilmagic function with geomlibs function
    // &&RB TODO: the following geomlibs function call must be tested
    vortex::Plane<T> plane;
#endif

	if (constrainToNormal)
	{
#ifdef HAVE_WILDMAGIC
		memcpy(&plane.Normal, &planeNormal, sizeof(Vector3<T>));
#else
        plane.normal(planeNormal);
#endif
	}
	T rms = computeRMSToPlane( plane, num_pnts, candidate_pnts );

#ifdef HAVE_WILDMAGIC
	Wm5::Vector3<T> pOrigin = plane.Normal * plane.Constant;

    memcpy(&planeNormal, &plane.Normal, sizeof(Vector3<T>));
	memcpy(&planeOrigin, &pOrigin, sizeof(Vector3<T>));
#else
    Vector3<T> pOrigin = plane.normal() * plane.constant();

        plane.normal(planeNormal);
    	memcpy(&planeOrigin, &pOrigin, sizeof(Vector3<T>));
#endif

	return rms;
}
//-----------------------------------------------------------------------------
float	PTVFIT_API FitPlaneToPointsf( PThandle query, Vector3f &planeNormal, Point3f &planeOrigin, bool constrainToNormal )
//-----------------------------------------------------------------------------
{
	// run query
	QueryBufferf buffer((int)5e5);
	buffer.initialize(false, false);
	buffer.setQuery( query );
	buffer.executeQuery();

	if (!buffer.numPntsInQueryIteration()) 
		return -1;

	return FitPlaneToPoints( buffer, planeNormal, planeOrigin, constrainToNormal );
}
//-----------------------------------------------------------------------------
double	PTVFIT_API FitPlaneToPointsd( PThandle query, Vector3d &planeNormal, Point3d &planeOrigin, bool constrainToNormal )
//-----------------------------------------------------------------------------
{
	// run query
	QueryBufferd buffer((int)5e5);
	buffer.initialize(false, false);
	buffer.setQuery( query );
	buffer.executeQuery();

	if (!buffer.numPntsInQueryIteration()) 
		return -1;

	return FitPlaneToPoints( buffer, planeNormal, planeOrigin, constrainToNormal );
}
//-----------------------------------------------------------------------------
template <typename T>
T	FitPlanarRectangleToPoints( PThandle query, Point3<T> corners[4], bool constrainToNormal )
//-----------------------------------------------------------------------------
{
	Vector3<T> planeNormal;
	Point3<T> planeOrigin;

	// run query
	QueryBuffer<T> buffer((int)5e5);
	buffer.initialize(false, false);
	buffer.setQuery( query );
	buffer.executeQuery();

	int num_points = buffer.numPntsInQueryIteration();
	if (!num_points) 
		return -1;

	T rms = FitPlaneToPoints( buffer, planeNormal, planeOrigin, constrainToNormal );

	// generate corners
	Plane<T> plane( planeOrigin, planeNormal );

	/*bounds of data */ 
	double min_u = 1e9;
	double max_u = -1e9;
	double min_v = 1e9;
	double max_v = -1e9;

	Point3<T> pnt;

	for (int i=0;i<num_points;i++)
	{
		plane.to2D( &buffer.getPointsBuffer()[i*3], pnt );

		//projectToPlane(&pnts[i*3], pnt);
		if (min_u > pnt.x) min_u = pnt.x;
		if (max_u < pnt.x) max_u = pnt.x;

		if (min_v > pnt.y) min_v = pnt.y;
		if (max_v < pnt.y) max_v = pnt.y;
	}

	plane.to3D(Point3<T>(min_u, min_v, 0), corners[0]);
	plane.to3D(Point3<T>(min_u, max_v, 0), corners[1]);
	plane.to3D(Point3<T>(max_u, max_v, 0), corners[2]);
	plane.to3D(Point3<T>(max_u, min_v, 0), corners[3]);

	return rms;
}
//-----------------------------------------------------------------------------
float	PTVFIT_API FitPlanarRectangleToPointsf( PThandle query, Point3f corners[4], bool constrainToNormal )
//-----------------------------------------------------------------------------
{
	return FitPlanarRectangleToPoints( query, corners, constrainToNormal );
}
//-----------------------------------------------------------------------------
double	PTVFIT_API FitPlanarRectangleToPointsd( PThandle query, Point3d corners[4], bool constrainToNormal )
//-----------------------------------------------------------------------------
{
	return FitPlanarRectangleToPoints( query, corners, constrainToNormal );
}
}