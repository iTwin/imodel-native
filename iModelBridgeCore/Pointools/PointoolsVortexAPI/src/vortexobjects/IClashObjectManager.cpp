#include "PointoolsVortexAPIInternal.h"
// IClashObjectManager.cpp

#include <Vortex/IClashObjectManager.h>
#include <ptclash/clashObjectManager.h>


using namespace vortex;


//---------------------------------------------------------------------------
// Public NVI
//---------------------------------------------------------------------------
PTres IClashObjectManager::createClashObjectFromScene(PThandle sceneHandle, IClashObject*& clashObjectRet)
{
	return ClashObjectManager::theClashObjectManager().clashObjectFromSceneOrCloudHandle(sceneHandle, clashObjectRet);
}

PTres IClashObjectManager::createClashObjectFromCloud(PThandle cloudHandle, IClashObject*& clashObjectRet)
{
	return ClashObjectManager::theClashObjectManager().clashObjectFromSceneOrCloudHandle(cloudHandle, clashObjectRet);
}

PTres IClashObjectManager::releaseClashObject(IClashObject* clashObject)
{
	return ClashObjectManager::theClashObjectManager().releaseClashObject(clashObject);
}

PTres IClashObjectManager::setClashTreeFolder(const PTstr filepath)
{
	return ClashObjectManager::theClashObjectManager().setClashTreeFolder(filepath);
}
