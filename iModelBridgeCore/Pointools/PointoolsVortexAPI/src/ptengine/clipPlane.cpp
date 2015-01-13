//----------------------------------------------------------------------------
//
// clipPlane.cpp
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include <ptengine/clipPlane.h>
#include <ptengine/clipManager.h>
#include <ptcloud2/pointcloud.h>
#include <pt/project.h>


//#define POINTS_ON_PLANE_ARE_INSIDE


namespace pointsengine
{

void ClipPlane::setParameters(double a, double b, double c, double d)
{
	m_plane.normal(pt::vector3d(a,b,c));
	m_plane.constant(d);
}

void ClipPlane::getParameters( double& a, double& b, double& c, double& d )
{
	a = m_plane.normal().x;
	b = m_plane.normal().y;
	c = m_plane.normal().z;
	d = m_plane.constant();
}

ClipResult ClipPlane::clipNode( pcloud::PointCloud& cloud, pcloud::Node* n )
{
	mmatrix4d mp;
	cloud.registration().compileMatrix(mp, pt::ProjectSpace);

	int numInside = 0;

	if (n)
	{
		pt::vector3d v;	
		pt::BoundingBoxD bb = n->extents();		

		// convert to ProjectSpace
		pt::vector3d basePoint = pt::vector3d(pt::Project3D::project().registration().matrix()(3,0), 
			pt::Project3D::project().registration().matrix()(3,1), 
			pt::Project3D::project().registration().matrix()(3,2));
		bb.translateBy(-basePoint);

		for (int i = 0; i < 8; i++)
		{		
			
			bb.getExtrema(i, v);				
			int side = m_plane.whichSide(v);	
#ifdef POINTS_ON_PLANE_ARE_INSIDE
			if (side <= 0) // treat points actually on the plane as inside
#else
			if (side < 0)
#endif
				numInside++;
		}
	}	

	if (numInside == 0)
		return OUTSIDE;
	else if (numInside == 8)
		return INSIDE;
	else
		return INTERSECTS;
}

ClipResult ClipPlane::clipBox( const pt::BoundingBoxD &box )
{
	int numInside = 0;
	pt::vector3d v, vt;

	for (int i = 0; i < 8; i++)
	{		
		// N.B. assumes box is in ProjectSpace
		box.getExtrema(i, v);	
		int side = m_plane.whichSide(v);
#ifdef POINTS_ON_PLANE_ARE_INSIDE
		if (side <= 0) // treat points actually on the plane as inside
#else
		if (side < 0)
#endif
			numInside++;	
	}	

	if (numInside == 0)
		return OUTSIDE;
	else if (numInside == 8)
		return INSIDE;
	else
		return INTERSECTS;
}

}