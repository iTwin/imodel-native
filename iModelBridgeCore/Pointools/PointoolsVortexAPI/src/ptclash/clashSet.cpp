#include "PointoolsVortexAPIInternal.h"
// clashSet.cpp

#include <ptapi/PointoolsVortexAPI.h>
#include <ptclash/clashSet.h>

using namespace pt;

ClashSet::ClashSet()
{
}
ClashSet::~ClashSet()
{
}
//-----------------------------------------------------------------------------
void ClashSet::addObject( pt::Object3D *obj )
{
	pt::PersistentObjectRef ref(obj);
	
	if (!hasObject(ref))
	{
		ClashObject *co = new ClashObject( ref );

		m_objects.push_back( co );
	}
}

//-----------------------------------------------------------------------------
const ClashObject* ClashSet::hasObject( const pt::PersistentObjectRef &obj ) const
{
	for (int i=0; i<m_objects.size(); i++)
	{
		const ClashObject *co = m_objects[i];
		if (co->objRef() == obj) return co;
	}
	return false;
}
//-----------------------------------------------------------------------------
ClashObject* ClashSet::getObject( const pt::PersistentObjectRef &obj ) const
{
	for (int i=0; i<m_objects.size(); i++)
	{
		ClashObject *co = m_objects[i];
		if (co->objRef() == obj) return co;
	}
	return NULL;
}
//-----------------------------------------------------------------------------
bool ClashSet::hasObject( const ClashObject* obj ) const
{
	for (int i=0; i<m_objects.size(); i++)
	{
		if (obj == m_objects[i]) return true;		
	}
	return false;
}
//-----------------------------------------------------------------------------
void ClashSet::addObject( const pt::PersistentObjectRef &obj )
{
	if (!hasObject(obj))
	{
		ClashObject *co = new ClashObject( obj );
		m_objects.push_back( co );
	}
}
//-----------------------------------------------------------------------------
int  ClashSet::getObjects( std::vector<pt::PersistentObjectRef> &objs ) const
{
	for (unsigned int i=0; i<m_objects.size(); i++)
	{
		objs.push_back( m_objects[i]->objRef() );
	}
	return m_objects.size();
}
//-----------------------------------------------------------------------------
void ClashSet::clear()
{
	for (unsigned int i=0; i<m_objects.size(); i++)
	{
		delete m_objects[i];
	}
	m_objects.clear();
}
//-----------------------------------------------------------------------------
void ClashSet::remObject( const pt::PersistentObjectRef &obj )
{
	std::vector<ClashObject*> objs;

	for (unsigned int i=0; i<m_objects.size(); i++)
	{
		if (m_objects[i]->objRef() == obj)
		{
			delete m_objects[i];
		}
		else
		{
			objs.push_back(m_objects[i]);
		}
	}
	m_objects = objs;
}
//-----------------------------------------------------------------------------
void ClashSet::remObject( ClashObject *obj )
{
	std::vector<ClashObject*> objs;

	for (unsigned int i=0; i<m_objects.size(); i++)
	{
		if (m_objects[i] == obj)
		{
			delete m_objects[i];
		}
		else
		{
			objs.push_back(m_objects[i]);
		}
	}
	m_objects = objs;
}
//-----------------------------------------------------------------------------
int ClashSet::drawTrees(int depth)
{
	int count = 0;

	for (unsigned int i=0; i<m_objects.size(); i++)
	{
		if (m_objects[i])
			count += m_objects[i]->drawObjBoundsTree(depth);
	}
	return count;
}
//-----------------------------------------------------------------------------
void ClashSet::getDiagnosticString( pt::String &str ) const
{
	int max_depth = 0;
	int min_depth = 200;

	for (unsigned int i=0; i<m_objects.size(); i++)
	{
		if (m_objects[i])
		{
			ClashTree *tree = m_objects[i]->objTree();
			if (tree)
			{
				if (max_depth < tree->maxLeafDepth())
					max_depth = tree->maxLeafDepth();
			
				if (min_depth > tree->minLeafDepth())
					min_depth = tree->minLeafDepth();
			}
		}
	}	
	if (max_depth)
		str.format("DP:%i to %i", min_depth, max_depth);
}

