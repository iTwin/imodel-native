// IClashObject.h
#ifndef ICLASHOBJECT_D812C84B_F4FD_4b3b_B2AC_F22966CBE251_H
#define ICLASHOBJECT_D812C84B_F4FD_4b3b_B2AC_F22966CBE251_H

#include "VortexObjects\VortexObjects_import.h"
#include <Vortex/VortexErrors.h>
namespace vortex
{	
	class IClashTree;

	//------------------------------------------------------------------------
	// IClashObjectCallback
	// Used to return information about clash tree generation. Clients
	// should create derive an object from this class in order to receive
	// tree generation feedback.
	//------------------------------------------------------------------------
	struct PTVOBJECT_API IClashObjectCallback
	{	
		//------------------------------------------------------------------------
		// Returns the progress so far in the clash tree generation
		// @param pcentComplete	The percentage of the tree that has been generated
		//						so far is returned here (0% - 100%)
		//------------------------------------------------------------------------
		virtual PTvoid clashTreeGenerationProgress(PTint pcentComplete) { ; }

	};

	//------------------------------------------------------------------------
	// IClashObject
	//------------------------------------------------------------------------
	class PTVOBJECT_API IClashObject
	{
	public:
		typedef PTvoid (__stdcall *generateClashTreeCB)(PTint pcentComplete);			

	protected:				
		virtual PTres			_getClashTree(IClashTree*& clashTreeRet) = 0;		
		virtual PTres			_generateClashTree(IClashObjectCallback* callback) = 0;		
		virtual PTbool			_clashTreeFileExists(PTvoid) = 0;
		virtual const PTstr 	_getClashTreeFilename(PTvoid) = 0;			

	public:		
		//--------------------------------------------------------------------		
		// Get the clash tree for this clash object if one exists. 
		// @param clashTree	a clash tree object for this clash object is 
		//					returned here
		// @return			PTV_SUCCESS
		//					PTV_FILE_NOT_ACCESSIBLE: cannot find the local
		//					clash tree, this should be generated using 
		//					generateClashTree()
		//--------------------------------------------------------------------		
		PTres getClashTree(IClashTree*& clashTreeRet);

		//--------------------------------------------------------------------		
		// Generate a clash tree for a clash object. The clash tree consists
		// of a tree of tightly fitted bounding boxes that represent the
		// object. Note clash tree generation can take times in the order
		// of minutes.
		// @param callback	A callback function that returns information on
		//					clash tree generation progress
		// @return			PTV_SUCCESS: tree generation is started
		//					PTV_FILE_FAILED_TO_CREATE: tree generation is not 
		//					started
		//--------------------------------------------------------------------		
		PTres generateClashTree(IClashObjectCallback* callback);

		//--------------------------------------------------------------------		
		// Check if the clash tree has already been generated for this clash
		// object.
		// @return			true if the clash tree has already been generated
		//					for this clash object, false otherwise
		//--------------------------------------------------------------------		
		PTbool clashTreeFileExists(PTvoid);
				
		//--------------------------------------------------------------------		
		// Get the filename where the clash tree data is cached locally. Note
		// that the pointer to the filename will only be valid for the lifetime
		// of this IClashObject and should not be deleted by the caller. The 
		// filename is returned regardless of whether the tree has been 
		// @param filename	The local filename is returned here
		// @return			const pointer to the local filename.
		//					or NULL if an error occurs;
		//--------------------------------------------------------------------		
		const PTstr getClashTreeFilename(PTvoid);
		
	};

	
}

#endif // ICLASHOBJECT_D812C84B_F4FD_4b3b_B2AC_F22966CBE251_H
