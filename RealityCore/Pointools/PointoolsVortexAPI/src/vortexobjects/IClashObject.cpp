/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"
// IClashObject.cpp

#include <Vortex/IClashObject.h>
#include <ptclash/clashObjectManager.h>


using namespace vortex;


//---------------------------------------------------------------------------
// Public NVI
//---------------------------------------------------------------------------
PTres IClashObject::getClashTree(IClashTree*& clashTreeRet)
{
	return _getClashTree(clashTreeRet);
}

PTres IClashObject::generateClashTree(IClashObjectCallback* callback)
{
	return _generateClashTree(callback);
}

PTbool IClashObject::clashTreeFileExists()
{
	return _clashTreeFileExists();
}

const PTstr IClashObject::getClashTreeFilename()
{
	return _getClashTreeFilename();
}
