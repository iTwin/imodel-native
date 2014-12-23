#ifndef POINTOOLS_VORTEX_FEATURE_FIT_API_H
#define POINTOOLS_VORTEX_FEATURE_FIT_API_H

#include "VortexFeatureExtractAPI.h"
#include "GeomTypes.h"

namespace vortex
{
	//! GetFeatureExtractAPIVersion
	//! Returns the libraries version number
	//\major Major version integer
	//\minor Minor version integer
	void	PTVFIT_API GetFeatureExtractAPIVersion( int &major, int &minor );

	//! Initialize the API
	//! To use the Vortex Feature Fit API the main Vortex API must already be loaded and initialized by the calling process
	//! Will return false if the Vortex version is not supported
	//!\licenseCode the license code for the feature fitting API provided by Pointools
	//!\vortexDll the system handle to the Vortex API dll loaded by your process
	bool	PTVFIT_API InitializeFeatureExtractAPI( const char *licenseCode );

	//! Fit a cylinder to points 
	//!\query The handle to the Vortex query to extract the points. Note that the queries density settings will be used to extract the points
	//!\cylinder The cylinder to receive the results. None zero values will be treated as hints
	//!\constrainToAxis Set to true if the cylinder.axis is a constraint
	//!\constrainToRadius Set to true if the cylinder.radius is a constraint
	float	PTVFIT_API FitCylinderToPointsf( PThandle query, Cylinderf &cylinder, bool constrainToAxis, bool constrainToRadius );
	double	PTVFIT_API FitCylinderToPointsd( PThandle query, Cylinderd &cylinder, bool constrainToAxis, bool constrainToRadius );

	//! Fit a planar rectangle to points 
	//!\query The handle to the Vortex query to extract the points. Note that the queries density settings will be used to extract the points
	//!\corners The resulting 4 corners of the planar rectangle. Note that these can be used to form 2 vectors to calculate the plane normal as the cross product
	float	PTVFIT_API FitPlanarRectangleToPointsf( PThandle query, Point3f corners[4], bool constrainToNormal );
	double	PTVFIT_API FitPlanarRectangleToPointsd( PThandle query, Point3d corners[4], bool constrainToNormal );

	//! Fit a plane to points 
	//!\query The handle to the Vortex query to extract the points. Note that the queries density settings will be used to extract the points
	//!\planeNormal The normal of the resulting plane
	//!\planeOrigin The origin of the plane as point
	//!\constrainToNormal The normal of the plane will be constrained to the planeNormal
	float	PTVFIT_API FitPlaneToPointsf( PThandle query, Vector3f &planeNormal, Point3f &planeOrigin, bool constrainToNormal );
	double	PTVFIT_API FitPlaneToPointsd( PThandle query, Vector3d &planeNormal, Point3d &planeOrigin, bool constrainToNormal );
}

#endif