//----------------------------------------------------------------------------
//
// clipManager.h
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#pragma once


#include <pt/typedefs.h>
#include <Vortex/VortexAPI.h>
#include <ptengine/clipPlane.h>
#include <ptcloud2/node.h>
#include <ptedit/isolationFilter.h>
#include <pt/datatree.h>


namespace pointsengine
{

class ClipManager
{
public:	
	static ClipManager& instance();

	void	enableClipping();
	void	disableClipping();
	bool	clippingEnabled() { return m_enabled; }
	bool	clippingEnabledAndClippingPlanesEnabled() { return clippingEnabled() && getNumEnabledClippingPlanes(); }
		
	// current clip styles are PT_CLIP_OUSTIDE (the default) and PT_CLIP_INSIDE
	PTres	setClipStyle( PTuint style );
	PTuint	getClipStyle() { return m_clipStyle; }
	uint	getNumClippingPlanes();	
	uint	getNumEnabledClippingPlanes();
	bool	isClippingPlaneEnabled( PTuint plane );
	PTres	enableClippingPlane( PTuint plane );
	PTres	disableClippingPlane( PTuint plane );
	PTres	setClippingPlaneParameters( PTuint plane, PTdouble a, PTdouble b, PTdouble c, PTdouble d );
	PTres	getClippingPlaneParameters( PTuint plane, PTdouble& a, PTdouble& b, PTdouble& c, PTdouble& d );
	
	ClipResult	clipNode( pcloud::PointCloud& cloud, pcloud::Node* n );	

	// Used by IsolationFilterClip to clip editing operations
	ptedit::IsolationFilter::IntersectCallback getIntersectFunction() { return m_currentIsolationFilter.intersect; }
	ptedit::IsolationFilter::InsideCallback getInsideFunction() { return m_currentIsolationFilter.inside; }
	ptedit::SelectionResult intersect( const pt::BoundingBoxD &box );
	bool inside( int thread, const pt::vector3d &pnt );

	// Used by queries to clip points	
	inline bool inside( const pt::vector3d &pnt )
	{
		if (!m_enabled) 
			return true;

		int numInside = 0;	
		int numOutside = 0;
		int numActivePlanes = 0;	

		// Test all the currently enabled clip objects against the extents of the passed node
		ClipPlanes::iterator it;
		for (it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
		{
			if (*it)
			{
				if ((*it)->enabled())
				{
					numActivePlanes++;

					if ((*it)->inside(pnt))
						numInside++;
					else
						numOutside++;			
				}
			}
		}

		// no active clip planes so inside
		if (numActivePlanes == 0)
			return true; // inside

		PTuint clipStyle;
		clipStyle = ClipManager::instance().getClipStyle();

		// fully inside all the clip planes
		if (numActivePlanes == numInside)
			return (clipStyle == PT_CLIP_OUTSIDE) ? true : false; 

		// if the node has been marked as totally outside any of the clip planes then it will be wholly clipped	
		return (clipStyle == PT_CLIP_OUTSIDE) ? false : true; 
	} 

	// edit stack persistence
	bool readState( const pt::datatree::Branch* b ); 
	bool writeState( pt::datatree::Branch* b );  

private:
	ClipManager();
	~ClipManager();

	ClipPlane* getPlane( PThandle handle );

	bool m_enabled;

	PTuint m_clipStyle;

	typedef std::list<ClipPlane*> ClipPlanes;	
	ClipPlanes m_clipPlanes;

	ptedit::IsolationFilter m_currentIsolationFilter;

	static ClipManager* _instance;	
};

}
