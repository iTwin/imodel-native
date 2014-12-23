#pragma once
#include <pt/sceneGraph.h>
#include <pt/Guid.h>

namespace pt
{
	namespace datatree	{	class Branch;	}

// Object Reference
// DOES NOT HANDLE INSTANCES OF SAME OBJECT WITH SAME TRANSFORM
class PersistentObjectRef
{
public:
	PersistentObjectRef(void);								// null reference
	PersistentObjectRef( const PersistentObjectRef &ref );	// copy constructor
	PersistentObjectRef( const pt::Object3D *obj );
	PersistentObjectRef( pt::Object3D *obj );	
	PersistentObjectRef( const Guid &objGuid );				// may be incomplete reference

	bool			isEmpty() const;
	bool			isObjectAvailable() const;	
	pt::Object3D	*resolveReference(bool loadIfNeeded=false) const;
	
	const String	&identifier()	const	{ return m_identifier; }
	const Guid		&guid() const			{ return m_guid; }
	const ObjectKey &key() const			{ return m_key; }

	bool			operator == ( const PersistentObjectRef &ref ) const;
	PersistentObjectRef		&operator = ( const PersistentObjectRef &ref );

	// Note that resolveReference fails in Vortex as there is no SceneClassManager
	// currently that manages objects with class name "Point Cloud"
	template <class ObjectType>
	bool resolveReference( ObjectType *&objPtr, bool loadIfNeeded=false ) const
	{
		Object3D *obj = resolveReference(loadIfNeeded);
		if (!obj) return 0;

		// type muat have s_className static method
		if (ObjectType::s_className() == obj->className()
			&& wcsncmp(obj->typeDescriptor(), m_dataType, sizeof( m_dataType ) / sizeof(wchar_t))==0)
		{
			objPtr = static_cast<ObjectType*>(obj);
			return true;
		}
		return false;
	}

	const ptds::FilePath &filePath() const { return m_file; }

private:
	bool			createReference( const pt::Object3D *obj );
	int				findIndexInParent( const pt::Object3D *obj ) const;
	Object3D		*findObjectInSceneByIndex( pt::Scene3D *scene ) const;

	Guid			m_guid;

	wchar_t			m_dataType[48];
	String			m_identifier;
	short			m_index[8];
	ptds::FilePath	m_file;
	ObjectKey		m_key;
};
};