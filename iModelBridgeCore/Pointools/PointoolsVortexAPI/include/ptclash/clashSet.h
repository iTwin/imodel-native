#pragma once
#include <pt/scenegraph.h>
#include <pt/datatree.h>
#include <ptclash/clashObject.h>
#include <vector>
#include <set>

namespace pt
{
class ClashSet
{
public:
	ClashSet();
	virtual ~ClashSet();

	void addObject( Object3D *obj );
	void addObject( const PersistentObjectRef &obj );

	void remObject( const PersistentObjectRef &obj );
	void remObject( ClashObject *obj );

	const ClashObject* hasObject( const pt::PersistentObjectRef &obj ) const;
	bool hasObject( const ClashObject* obj ) const;
	ClashObject* getObject( const pt::PersistentObjectRef &obj ) const;	

	void clear();

	unsigned int size() const	
	{ 
		return m_objects.size(); 
	}
	
	ClashObject *operator [] (unsigned int i)
	{
		return i<m_objects.size() ? m_objects[i] : 0;
	}

	const ClashObject *operator [] (unsigned int i) const
	{
		return i<m_objects.size() ? m_objects[i] : 0;
	}

	int  getObjects( std::vector<PersistentObjectRef> &obj ) const;

#ifdef HAVE_OPENGL
    // diagnostics
	int  drawTrees(int depth);
#endif

	void getDiagnosticString( pt::String &str ) const;	

private:
	
	std::vector<ClashObject*>		m_objects;
};
};