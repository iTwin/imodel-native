// IClashObjectManager.h
#ifndef ICLASHOBJECTMANAGER_3415BD2B_2F44_4f96_9D28_E5D91C59EFBA_H
#define ICLASHOBJECTMANAGER_3415BD2B_2F44_4f96_9D28_E5D91C59EFBA_H


#include "VortexObjects_import.h"


namespace vortex
{	
	//------------------------------------------------------------------------
	// IClashObjectManager
	//------------------------------------------------------------------------
	class PTVOBJECT_API IClashObjectManager
	{
	public:		
		//--------------------------------------------------------------------
		// Given a valid vortex scene handle, create and return a new clash 
		// object for this scene. The new object is reference counted and
		// should not be deleted, call releaseClashObject() when no longer
		// required.
		// @param sceneHandle		a valid vortex scene handle		
		// @param clashObjectRet	a pointer to the newly created clash 
		//							object is returned here
		// @return					PTV_SUCCESS
		//							PTV_INVALID_HANDLE: sceneHandle is invalid
		//							PTV_UNKNOWN_ERROR: an unknown error occurred						
		//--------------------------------------------------------------------		
		static PTres createClashObjectFromScene(PThandle sceneHandle, IClashObject*& clashObjectRet);

		//--------------------------------------------------------------------
		// Given a valid vortex scene handle, create and return a new clash 
		// object for this scene. The new object is reference counted and
		// should not be deleted, call releaseClashObject() when no longer
		// required.		
		// @param objectHandle	a valid vortex object handle
		// @param clashObjectRet	a pointer to the newly created clash 
		//							object is returned here
		// @return					PTV_SUCCESS
		//							PTV_INVALID_HANDLE: sceneHandle is invalid
		//							PTV_UNKNOWN_ERROR: an unknown error occurred
		//--------------------------------------------------------------------		
		static PTres createClashObjectFromCloud(PThandle cloudHandle, IClashObject*& clashObjectRet);

		//--------------------------------------------------------------------
		// Release a clash object that was created using 
		// createClashObjectFromScene() or createClashObjectFromCloud(). The
		// clash object will be deleting when all references to it are 
		// released.
		// @param					a pointer to a valid clash object
		//--------------------------------------------------------------------
		static PTres releaseClashObject(IClashObject* clashObject);

		//--------------------------------------------------------------------
		// Set a location where all clash tree data should be stored/loaded
		// from.
		// @param dir	The location in which to save new clash data trees
		//				to file, and the location to search for existing
		//				clash data tree files.
		// @return		PTV_SUCCESS, if the directory location is set correctly,
		//				PTV_INVALID_VALUE_FOR_PARAMETER if filepath has a length
		//				of less than 2 characters
		//--------------------------------------------------------------------
		static PTres setClashTreeFolder(const PTstr filepath);

	};


}

#endif // ICLASHOBJECTMANAGER_3415BD2B_2F44_4f96_9D28_E5D91C59EFBA_H
