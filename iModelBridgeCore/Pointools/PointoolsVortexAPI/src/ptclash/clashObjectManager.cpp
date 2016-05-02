#include "PointoolsVortexAPIInternal.h"
// clashObjectManager.cpp

#include <ptapi/PointoolsVortexAPI.h>
#include <ptclash/clashObjectManager.h>
#include <ptengine/engine.h>



//-----------------------------------------------------------------------------
// EXTERNS
//-----------------------------------------------------------------------------
extern pcloud::Scene*			sceneFromHandle(PThandle handle);
extern pcloud::PointCloud*		cloudFromHandle(PThandle cloud);



using namespace vortex;

// only allow one ClashObjectManager that is accessible through theClashObjectManager()
ClashObjectManager* ClashObjectManager::_this = NULL;
ClashObjectManager s_clashObjectManager;


ClashObjectManager& ClashObjectManager::theClashObjectManager()
{
	return s_clashObjectManager;
}

ClashObjectManager::ClashObjectManager()
{
	assert(_this == NULL);
	_this = this;
}

ClashObjectManager::~ClashObjectManager()
{
	clear();

	assert(_this);
	_this = NULL;
}

void ClashObjectManager::clear()
{
	m_clashSet.clear();
}

PTres ClashObjectManager::clashObjectFromSceneOrCloudHandle(PThandle handle, IClashObject*& objectRet)
{
	// convert the scene/cloud handle to an Object3D
	pt::Object3D* obj = reinterpret_cast<pt::Object3D*>(cloudFromHandle(handle));
	if (!obj)
		obj = reinterpret_cast<pt::Object3D*>(sceneFromHandle(handle));
	
	// no scene or cloud found with this handle
	if (!obj)
		return PTV_INVALID_HANDLE;
	
	pt::PersistentObjectRef objRef(obj);
	pt::ClashObject* currObj = const_cast<pt::ClashObject*>(m_clashSet.hasObject(objRef));
	if (!currObj)
	{
		// N.B. addObject creates a new ClashObject so don't create it here as well
		m_clashSet.addObject(objRef);
		currObj = m_clashSet.getObject(objRef);
		if (!currObj)
			return PTV_UNKNOWN_ERROR;
	}

	currObj->incRefCount();
	objectRet = currObj;

	return PTV_SUCCESS;	
}

PTres ClashObjectManager::releaseClashObject(IClashObject* clashObject)
{
	if (!clashObject)
		return PTV_INVALID_PARAMETER;

	pt::ClashObject* currObj = reinterpret_cast<pt::ClashObject*>(clashObject);
	if (!currObj)
		return PTV_INVALID_PARAMETER;

	if (!m_clashSet.hasObject(currObj))
		return PTV_INVALID_PARAMETER;

	currObj->decRefCount();
	if (currObj->refCount() == 0)
	{
		// remObject deletes the passed object
		m_clashSet.remObject(currObj);		
	}

	return PTV_SUCCESS;
}

PTres ClashObjectManager::getClashTreeFolder(pt::String& filepath)
{
	if (m_clashTreeFolder.length())
	{
		filepath = m_clashTreeFolder;
		return PTV_SUCCESS;
	}
	
	return PTV_NOT_INITIALIZED;
}

PTres ClashObjectManager::setClashTreeFolder(const PTstr filepath)
{
	if (wcslen(filepath) < 2)
		return PTV_INVALID_VALUE_FOR_PARAMETER;

	m_clashTreeFolder = filepath;
	return PTV_SUCCESS;
}

const pt::Object3D* ClashObjectManager::resolveReference(const pt::PersistentObjectRef& ref) const
{
	pt::Object3D* obj = reinterpret_cast<pt::Object3D*>(cloudFromHandle(ref.key()));
	if (!obj)
		obj = reinterpret_cast<pt::Object3D*>(sceneFromHandle(ref.key()));

	return obj;
}
