//----------------------------------------------------------------------------
//
// clipPlane.h
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#pragma once

#include <pt/os.h>
#include <pt/typedefs.h>
#include <ptcloud2/node.h>
#include <ptcloud2/voxel.h>
#include <ptapi/PointoolsVortexAPI.h>
#include <ptengine/clipObject.h>
#include <ptengine/clipDefines.h>
#include <pt/plane.h>
#include <ptedit/edit.h>
#include <list>


namespace pointsengine
{

class ClipPlane : public ClipObject
{
public:
	ClipPlane() { ; }
	virtual ~ClipPlane() { ; }

	// set the parameters for this plane according to the plane equation Ax + By + Cz = D
	void setParameters( double a, double b, double c, double d );
	void getParameters( double& a, double& b, double& c, double& d );

	ClipResult clipNode( pcloud::PointCloud& cloud, pcloud::Node* n );	
	ClipResult clipBox( const pt::BoundingBoxD &box );

	inline bool inside(int thread, const pt::vector3d &pnt)
	{
		int side[EDT_MAX_THREADS];
		side[thread] = m_plane.whichSide(pnt);
		if (side[thread] <= 0) // treat points actually on the plane as inside
			return true;

		return false;
	}

	inline bool inside(const pt::vector3d &pnt)
	{
		int side = m_plane.whichSide(pnt);
		if (side <= 0) // treat points actually on the plane as inside
			return true;

		return false;
	}

private:

	// plane equation parameters for equation Ax + By + Cz = D, where m_normal is (A,B,C) and m_const is D
	pt::Planed m_plane;

};

}
