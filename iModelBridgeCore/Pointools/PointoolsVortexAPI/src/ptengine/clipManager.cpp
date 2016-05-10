//----------------------------------------------------------------------------
//
// clipManager.cpp
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include "PointoolsVortexAPIInternal.h"
#include <ptengine/clipPlane.h>
#include <ptengine/clipManager.h>
#include <ptapi/PointoolsVortexAPI.h>
#include <ptedit/isolationFilter.h>
#include <assert.h>
#include <fastdelegate/fastdelegate.h>

namespace pointsengine
{

// for writing edit stack state
const char* CLIPMANAGER = "clipmanager";
const char* CLIP_STYLE = "clipstyle";
const char* NUM_CLIP_PLANES = "numclipplanes";
const char* CLIP_PLANE_ID = "id";
const char* CLIP_PLANE_ENABLED = "enabled";

ptedit::IsolationFilterNone s_filterNone;
ptedit::IsolationFilterClip s_filterClip;

ClipManager* ClipManager::_instance = NULL;

ClipManager& ClipManager::instance()
{
	if (_instance == NULL)
		_instance = new ClipManager();

	return *_instance;
}

ClipManager::ClipManager() :
m_enabled(false),
m_clipStyle(PT_CLIP_OUTSIDE)
{
#ifdef _DEBUG
	assert(_instance == NULL);
#endif // _DEBUG

	// we only support 6 clipping planes at the moment, create them and add to the clip list
	for (int i = 0; i < 6; i++)
	{
		ClipPlane* plane = new ClipPlane();
		m_clipPlanes.push_back(plane);
	}

	m_currentIsolationFilter.intersect = fastdelegate::MakeDelegate(&s_filterNone, &ptedit::IsolationFilterNone::intersect);
	m_currentIsolationFilter.inside = fastdelegate::MakeDelegate(&s_filterNone, &ptedit::IsolationFilterNone::inside);

	_instance = this;
}

ClipManager::~ClipManager()
{
	_instance = NULL;

	ClipPlanes::iterator it;
	for (it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
	{
		if (ClipObject* obj = (*it))		
			delete obj;		
	}
	m_clipPlanes.clear();
}

void ClipManager::enableClipping()
{	
	m_enabled = true;

	// set the current isolation filter that will be used to clip editing operations
	m_currentIsolationFilter.intersect = fastdelegate::MakeDelegate(&s_filterClip, &ptedit::IsolationFilterClip::intersect);
	m_currentIsolationFilter.inside = fastdelegate::MakeDelegate(&s_filterClip, &ptedit::IsolationFilterClip::inside);

	ptEnable(PT_CLIPPING);
}

void ClipManager::disableClipping()
{
	m_enabled = false;

	// switch off the isolation filter
	m_currentIsolationFilter.intersect = fastdelegate::MakeDelegate(&s_filterNone, &ptedit::IsolationFilterNone::intersect);
	m_currentIsolationFilter.inside = fastdelegate::MakeDelegate(&s_filterNone, &ptedit::IsolationFilterNone::inside);

	ptDisable(PT_CLIPPING);
}

PTres ClipManager::setClipStyle( PTuint style )
{
	// only allow PT_CLIP_OUTSIDE (the default) and PT_CLIP_INSIDE
	if (style == PT_CLIP_OUTSIDE || style == PT_CLIP_INSIDE)
	{
		m_clipStyle = style;
		return PTV_SUCCESS;
	}

	return PTV_INVALID_VALUE_FOR_PARAMETER;
}

uint ClipManager::getNumClippingPlanes()
{
	// if other types of clip objects that are not clip planes are added in the future this will need updating
	return static_cast<uint>(m_clipPlanes.size());
}

uint ClipManager::getNumEnabledClippingPlanes()
{
	uint numEnabled = 0;

	for (ClipPlanes::iterator it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
	{		
		if (ClipPlane* obj = (*it))
		{
			if (obj->enabled())
				numEnabled++;
		}
	}

	return numEnabled;
}

bool ClipManager::isClippingPlaneEnabled( PTuint id )
{		
	if (ClipPlane* plane = getPlane(id))		
	{
		return plane->enabled();		
	}
		
	return false;
}

PTres ClipManager::enableClippingPlane( PTuint id )
{
	if (ClipPlane* plane = getPlane(id))		
	{
		plane->enable(true);
		return PTV_SUCCESS;
	}

	return PTV_INVALID_VALUE_FOR_PARAMETER;
}

PTres ClipManager::disableClippingPlane( PTuint id )
{
	if (ClipPlane* plane = getPlane(id))		
	{
		plane->enable(false);
		return PTV_SUCCESS;
	}

	return PTV_INVALID_VALUE_FOR_PARAMETER;
}

PTres ClipManager::setClippingPlaneParameters( PTuint id, PTdouble a, PTdouble b, PTdouble c, PTdouble d )
{
	if (ClipPlane* obj = getPlane(id))		
	{
		obj->setParameters(a, b, c, d);
		return PTV_SUCCESS;
	}

	return PTV_INVALID_VALUE_FOR_PARAMETER;
}

PTres ClipManager::getClippingPlaneParameters( PTuint id, PTdouble& a, PTdouble& b, PTdouble& c, PTdouble& d )
{
	if (ClipPlane* obj = getPlane(id))		
	{
		obj->getParameters(a, b, c, d);
		return PTV_SUCCESS;
	}

	return PTV_INVALID_VALUE_FOR_PARAMETER;
}

ClipPlane* ClipManager::getPlane( PTuint id )
{		
	for (ClipPlanes::iterator it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
	{		
		if (ClipPlane* obj = (*it))
		{	
			if (obj->id() == id)
				return obj;
		}
	}
	return NULL;
}

ClipResult ClipManager::clipNode( pcloud::PointCloud& cloud, pcloud::Node* n )
{
	int numInside = 0;
	int numOutside = 0;
//	int numInterects = 0;
	int numActivePlanes = 0;

	if (!n) 
		return OUTSIDE;

	// Test all the currently enabled clip objects against the extents of the passed node
	ClipPlanes::iterator it;
	for (it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
	{
		if (ClipPlane* plane = (*it))
		{
			if (plane->enabled())
			{
				numActivePlanes++;

				ClipResult res = plane->clipNode(cloud, n);
				if (res == INSIDE)
					numInside++;
				else if (res == OUTSIDE)
					numOutside++;
			//	else
			//		numInterects++;
			}
		}
	}
	
	// no active clip planes so inside
	if (numActivePlanes == 0)
		return INSIDE; 

	PTuint clipStyle = ClipManager::instance().getClipStyle();

	// if the node has been marked as totally outside any of the clip planes then it will be wholly clipped
	if (numOutside)
		return (clipStyle == PT_CLIP_OUTSIDE) ? OUTSIDE : INSIDE; 
	
	// fully inside all the clip planes
	if (numActivePlanes == numInside)
		return (clipStyle == PT_CLIP_OUTSIDE) ? INSIDE : OUTSIDE; 

	return INTERSECTS;
}

/** Test the passed box against the current clip objects in ProjectSpace 
 */
ptedit::SelectionResult ClipManager::intersect(const pt::BoundingBoxD &box)
{
	int numInside = 0;
	int numOutside = 0;
//	int numInterects = 0;
	int numActivePlanes = 0;

	// Test all the currently enabled clip objects against the extents of the passed node
	ClipPlanes::iterator it;
	for (it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
	{
		if (ClipPlane* plane = (*it))
		{
			if (plane->enabled())
			{
				numActivePlanes++;

				ClipResult res = plane->clipBox(box);
				if (res == INSIDE)
					numInside++;
				else if (res == OUTSIDE)
					numOutside++;
			//	else
			//		numInterects++;
			}
		}
	}

	// no active clip planes so inside
	if (numActivePlanes == 0)
		return ptedit::FullyInside; 

	PTuint clipStyle = ClipManager::instance().getClipStyle();

	// if the node has been marked as totally outside any of the clip planes then it will be wholly clipped
	if (numOutside)
		return (clipStyle == PT_CLIP_OUTSIDE) ? ptedit::FullyOutside : ptedit::FullyInside; 

	// fully inside all the clip planes
	if (numActivePlanes == numInside)
		return (clipStyle == PT_CLIP_OUTSIDE) ? ptedit::FullyInside : ptedit::FullyOutside; 

	return ptedit::PartiallyInside;
}

/** Multithreaded check for a point being inside all the clip objects
 */
bool ClipManager::inside(int thread, const pt::vector3d &pnt)
{
	int numInside[EDT_MAX_THREADS];	
	int numOutside[EDT_MAX_THREADS];
	int numActivePlanes[EDT_MAX_THREADS];

	numInside[thread] = 0;
	numOutside[thread] = 0;
	numActivePlanes[thread] = 0;

	// Test all the currently enabled clip objects against the extents of the passed node
	ClipPlanes::iterator it[EDT_MAX_THREADS];
	for (it[thread] = m_clipPlanes.begin(); it[thread] != m_clipPlanes.end(); it[thread]++)
	{
		if (*it[thread])
		{
			if ((*it[thread])->enabled())
			{
				numActivePlanes[thread]++;
				
				if ((*it[thread])->inside(thread, pnt))
					numInside[thread]++;
				else
					numOutside[thread]++;			
			}
		}
	}

	// no active clip planes so inside
	if (numActivePlanes[thread] == 0)
		return true; //ptedit::FullyInside; 

	PTuint clipStyle[EDT_MAX_THREADS];
	clipStyle[thread] = ClipManager::instance().getClipStyle();

	// fully inside all the clip planes
	if (numActivePlanes[thread] == numInside[thread])
		return ((clipStyle[thread] == PT_CLIP_OUTSIDE) ? ptedit::FullyInside : ptedit::FullyOutside) != 0; 

	// if the node has been marked as totally outside any of the clip planes then it will be wholly clipped	
	return ((clipStyle[thread] == PT_CLIP_OUTSIDE) ? ptedit::FullyOutside : ptedit::FullyInside) != 0; 
}

/** Read the current clip manager state from the edit stack
 */
bool ClipManager::readState(const pt::datatree::Branch* b) 
{	
	if (pt::datatree::Branch* clipManagerBranch = b->getBranch(CLIPMANAGER))
	{				
		PTuint clipStyle;      
		if (clipManagerBranch->getNode(CLIP_STYLE, clipStyle))		
			setClipStyle(clipStyle);
		
		PTuint numPlanes;
		if (clipManagerBranch->getNode(NUM_CLIP_PLANES, numPlanes))
		{
			for (uint i = 0; i < numPlanes; i++)
			{
				if (i < getNumClippingPlanes())
				{					
					char name[32] = {0};
					sprintf(name, "clipplane%d", i);
					if (pt::datatree::Branch* planeBranch = clipManagerBranch->getBranch(name))
					{
						int id;
						if (planeBranch->getNode(CLIP_PLANE_ID, id))
						{
							PTdouble a, b, c, d;
							if (planeBranch->getNode("a", a)
								&& planeBranch->getNode("b", b)
								&& planeBranch->getNode("c", c)
								&& planeBranch->getNode("d", d))
							{
								setClippingPlaneParameters(id, a, b, c, d);
							}

							bool enabled;
							if (planeBranch->getNode(CLIP_PLANE_ENABLED, enabled))
							{
								if (enabled)
									enableClippingPlane(id);
								else
									disableClippingPlane(id);
							}
						}												
					}
				}
			}				
		}

		enableClipping();
	}
	else
	{
		disableClipping();
	}
	
	return true; 
}

/** Write the current clip manager state to the edit stack
 */
bool ClipManager::writeState( pt::datatree::Branch* b)  
{ 	
	if (m_enabled)
	{
		if (pt::datatree::Branch* clipManagerBranch = b->addBranch(CLIPMANAGER))
		{
			clipManagerBranch->addNode(CLIP_STYLE, m_clipStyle);
			clipManagerBranch->addNode(NUM_CLIP_PLANES, (PTuint)m_clipPlanes.size());
			ClipPlanes::iterator it;
			int id = 0;
			for (it = m_clipPlanes.begin(); it != m_clipPlanes.end(); it++)
			{				
				char name[32] = {0};
				sprintf(name, "clipplane%d", id);
				if (pt::datatree::Branch* planeBranch = clipManagerBranch->addBranch(name))
				{
					PTdouble a, b, c, d;
					getClippingPlaneParameters(id, a, b, c, d);
					
					planeBranch->addNode(CLIP_PLANE_ID, id);
					planeBranch->addNode(CLIP_PLANE_ENABLED, isClippingPlaneEnabled(id));
					planeBranch->addNode("a", a);
					planeBranch->addNode("b", b);
					planeBranch->addNode("c", c);
					planeBranch->addNode("d", d);
				}
				id++;
			}
		}
	}

	return true;
}

}
