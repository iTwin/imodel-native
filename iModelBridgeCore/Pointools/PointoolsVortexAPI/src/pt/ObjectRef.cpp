#include "PointoolsVortexAPIInternal.h"
#include <pt/ObjectRef.h>
#include <pt/sceneclassmanager.h>

using namespace pt;

//-----------------------------------------------------------------------------
PersistentObjectRef::PersistentObjectRef()
{
	m_dataType[0]=0;
	m_key=0;
	memset(m_index, 0, sizeof(m_index));
}
//-----------------------------------------------------------------------------
PersistentObjectRef::PersistentObjectRef( const PersistentObjectRef &ref )
{
	m_dataType[0]=0;
	*this = ref;	
}
//-----------------------------------------------------------------------------
PersistentObjectRef::PersistentObjectRef( const pt::Object3D *obj )
{
	m_dataType[0]=0;
	m_key=0;
	memset(m_index, 0, sizeof(m_index));
	if (obj) createReference(obj);
}
//-----------------------------------------------------------------------------
PersistentObjectRef::PersistentObjectRef( pt::Object3D *obj )	// required for scripting
{
	m_dataType[0]=0;
	m_key=0;
	memset(m_index, 0, sizeof(m_index));
	if (obj) createReference(obj);
}
//-----------------------------------------------------------------------------
bool PersistentObjectRef::operator == ( const PersistentObjectRef &ref ) const
{
	bool same = false;
	
	if (m_guid.isValid()	
		&& ((ref.m_guid == m_guid) || (ref.m_file == m_file)))
			same = true;

	if (!m_guid.isValid() && (ref.m_file == m_file))
		same = true;

	if (same)
	{
		return ref.resolveReference() == resolveReference() ? true : false;
	}
	return false;
}
//-----------------------------------------------------------------------------
PersistentObjectRef			&PersistentObjectRef::operator = ( const PersistentObjectRef &ref )
{
	m_guid = ref.m_guid;
	m_file = ref.m_file;
	m_identifier = ref.m_identifier;
	m_key = ref.m_key;
	wcscpy_s(m_dataType,48, ref.m_dataType);

	for (int i=0; i<8; i++)
		m_index[i] = ref.m_index[i];

	return *this;
}
//-----------------------------------------------------------------------------
int PersistentObjectRef::findIndexInParent( const pt::Object3D *obj ) const
{
	if (!obj) return -1;
	if (obj->parent() && 
		(
		strncmp( obj->parent()->objectClass(), "Group3D", 7)==0
		|| strncmp( obj->parent()->objectClass(), "Scene3D", 7)==0))
	{
		const Group3D *group = static_cast<const Group3D*>(obj->parent());
		for (int i=0;i<group->numObjects();i++)
		{
			if (group->object(i)==obj) return i;
		}
	}
	return -1;
}
//-----------------------------------------------------------------------------	
bool PersistentObjectRef::isEmpty() const
{
	return (m_guid.isValid() || 
		m_identifier.length() || 
		!m_file.isEmpty() || 
		m_key) 
		? false : true;
}
//-----------------------------------------------------------------------------	
bool PersistentObjectRef::isObjectAvailable() const
{
	return resolveReference(false) ? true : false;
}
//-----------------------------------------------------------------------------	
PersistentObjectRef::PersistentObjectRef( const Guid &objGuid )
{
	m_guid = objGuid;
	m_dataType[0] = 0; //ie unknown
}
//-----------------------------------------------------------------------------
pt::Object3D	*PersistentObjectRef::resolveReference(bool loadIfNeeded) const
{
	SceneClassManager *manager = 0;
	Object3D *obj=0;

	if ( m_dataType[0] )
	{
		// get the scene class manager that handles this type
		manager = SceneClassManager::manager( m_dataType );

		//try guid first
		if (m_guid && manager)
		{
			obj = manager->findObjectByGuid( m_guid );
			if (obj) return obj;
		}
	}
	else
	{
		if (m_guid)
		{
			int numManagers = SceneClassManager::numClassManagers();

			SceneClassManager **managers = new SceneClassManager*[numManagers];
			SceneClassManager::getClassManagers( managers );

			for (int i=0; i<numManagers; i++)
			{
				obj = managers[i]->findObjectByGuid( m_guid );
				if (obj) break;
			}
			delete [] managers;
			
			if (obj)
			{
				// store data type name to make future resolution easier
				wcsncpy( const_cast<PersistentObjectRef*>(this)->m_dataType, 
					obj->typeDescriptor(), sizeof( m_dataType )/sizeof(wchar_t));
				return obj;
			}
		}
	}
	if (!manager) return 0;	// catastrophic failure	

	// then filename
	Scene3D *scene = manager->findSceneByFilepath( m_file );

	if (scene) obj = findObjectInSceneByIndex( scene );
	if (!obj) obj = manager->findObjectByKey( m_key );

	return obj;
	
	// then identifier ?? - risky, might give false positive
}
//-----------------------------------------------------------------------------
bool PersistentObjectRef::createReference( const pt::Object3D *obj )
{
	m_identifier = obj->identifier();
	wcsncpy( m_dataType, obj->typeDescriptor(), sizeof( m_dataType )/sizeof(wchar_t));

	m_guid = obj->objectGuid();	// preferred way, but it might be null
	
	m_key = obj->key();	// session key - needed for instance handling

	const pt::Object3D *sceneObj=obj;
	
	m_index[0]=m_index[1]=m_index[2]=m_index[3]=-1;

	while ( sceneObj && strncmp(sceneObj->objectClass(), "Scene3D", 7) != 0)
	{
		sceneObj = sceneObj->parent();
	}
	if (sceneObj)
	{
		const pt::Scene3D *scene = static_cast<const pt::Scene3D *>(sceneObj);
		m_file = scene->filepath();

		int depth = 0;
		// find index of this object in the scene if it is a sub object
		int index = findIndexInParent(obj);
		m_index[depth] = (short)index;

		while (index != -1 && depth < 4)
		{
			if (!obj) break;
			obj = obj->parent();
			m_index[++depth] = (short)findIndexInParent(obj);
		}
		return true;
	}
	else return false; // should never happen
}
//-----------------------------------------------------------------------------
Object3D *PersistentObjectRef::findObjectInSceneByIndex( pt::Scene3D *scene ) const
{
	int depth = 0;

	// work through depth
	int index = m_index[depth];
	Object3D *obj=scene;

	while (index >0 && depth < 4)
	{
		Group3D *group=0;
		if (obj->isGroup()) 
		{
			group = static_cast<Group3D*>(obj);
			if (index >= group->numObjects()) return 0;	// bad index
			obj = group->object(index);
		}
		index = m_index[++depth];
	}
	return obj;
}
