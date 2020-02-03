/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include "StdAfx.h"  // Always include first

#include <TerrainModel/ElementHandler/IRasterTextureSourceManager.h>

/*----------------------------------------------------------------------------+
|IRasterTextureSourceManager Method Definition Section - Begin
+----------------------------------------------------------------------------*/
IRasterTextureSourcePtr IRasterTextureSourceManager::GetSource(StringList& sourceFileNames)
    {
    return _GetSource(sourceFileNames);
    }
    
IRasterTextureSourcePtr IRasterTextureSourceManager::GetSource(StringList& sourceFileNames,
                                                               WCharCP     cacheFileName)
    {
    return _GetSource(sourceFileNames, cacheFileName);
    }
    
IRasterTextureSourcePtr IRasterTextureSourceManager::GetSource(SourceID sourceID)
    {
    return _GetSource(sourceID);
    }
    
int IRasterTextureSourceManager::CreatePSSMosaicSource(IRasterTextureSourcePtr& rasterTextureSourcePtr, 
                                                       WCharCP                  pssFileName,
                                                       bool                     isOnDemand)
    {
    return _CreatePSSMosaicSource(rasterTextureSourcePtr, pssFileName, isOnDemand);
    }
/*----------------------------------------------------------------------------+
|IRasterTextureSourceManager Method Definition Section - End
+----------------------------------------------------------------------------*/