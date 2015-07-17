/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/TMHandlersResources.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

enum TerrainModelMessageLists
    {
    TerrainModelMessageList_Default                   = 1,
    };


enum TerrainModelHandlersMessages
    {
    MSG_TERRAINMODEL_NotAllowed                   = 100,
    MSG_TERRAINMODEL_ApplicationRequired,
    MSG_TERRAINMODEL_ScalableTerrainModel,
    MSG_TERRAINMODEL_InvalidVersion,
    MSG_TERRAINMODEL_Elevation,
    MSG_TERRAINMODEL_Slope,
    MSG_TERRAINMODEL_Aspect,
    MSG_TERRAINMODEL_FromParent,
    MSG_TERRAINMODEL_FromBoundary,
    MSG_TERRAINMODEL_None,
    MSG_TERRAINMODEL_Sliver,
    MSG_TERRAINMODEL_MaxEdgeLength,
    // STM
    MSG_TERRAINMODEL_STMNotAllowedForOperation    = 200,
    MSG_TERRAINMODEL_STMFileNotExist,               
    MSG_TERRAINMODEL_CannotOpenSTM,    
    MSG_TERRAINMODEL_CannotOpenSTMFile,
    MSG_TERRAINMODEL_UnsupportedFileVersion,
    };
