// clashObjectManager.h
#ifndef CLASH_OBJECT_MANAGER_28620271_3E49_4900_997E_1EAA68926362_H
#define CLASH_OBJECT_MANAGER_28620271_3E49_4900_997E_1EAA68926362_H

#include <Vortex/VortexErrors.h>

#include <ptclash/clashSet.h>

#include <Vortex/IClashObject.h>

namespace vortex
{	

class ClashObjectManager
{
public:
	ClashObjectManager();
	~ClashObjectManager();

	static ClashObjectManager& theClashObjectManager();

	PTres clashObjectFromSceneOrCloudHandle(PThandle handle, IClashObject*& clashObjectRet);	
	PTres releaseClashObject(IClashObject* clashObject);

	PTres getClashTreeFolder(pt::String& dir);
	PTres setClashTreeFolder(const PTstr dir);	

	const pt::Object3D* resolveReference(const pt::PersistentObjectRef& ref) const;

private:
	void clear();	


	// ClashSet is used to manage current ClashObjects
	pt::ClashSet				m_clashSet;

	pt::String					m_clashTreeFolder;

	// only allow one ClashObjectManager
	static ClashObjectManager*	_this;
};

}

#endif // CLASH_OBJECT_MANAGER_28620271_3E49_4900_997E_1EAA68926362_H
