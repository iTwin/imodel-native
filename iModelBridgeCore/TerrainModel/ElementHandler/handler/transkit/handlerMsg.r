/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/transkit/handlerMsg.r $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RmgrTools\Tools\rtypes.r.h>
#include <TerrainModel/ElementHandler/TMHandlersResources.r.h>

/*----------------------------------------------------------------------+
|                                                                       |
|   Messages                                                            |
|                                                                       |
+----------------------------------------------------------------------*/
MessageList TerrainModelMessageList_Default =
{
        {
        { MSG_TERRAINMODEL_NotAllowed, "TerrainModelNotAllowed" },
        { MSG_TERRAINMODEL_ApplicationRequired, "An additional application is required to display full resolution of STM elements and draped rasters" },
        { MSG_TERRAINMODEL_ScalableTerrainModel, "Scalable Terrain Model" },
        { MSG_TERRAINMODEL_InvalidVersion, "Terrain invalid version" },
        { MSG_TERRAINMODEL_Elevation, "Elevation {1}" },
        { MSG_TERRAINMODEL_Slope, "Slope {1}" },
        { MSG_TERRAINMODEL_Aspect, "Aspect {1}" },
        { MSG_TERRAINMODEL_FromParent, "From Parent" },
        { MSG_TERRAINMODEL_FromBoundary, "From Boundary" },
        { MSG_TERRAINMODEL_None, "None" },
        { MSG_TERRAINMODEL_Sliver, "Sliver" },
        { MSG_TERRAINMODEL_MaxEdgeLength, "Max Edge Length" },

        /// STM
        { MSG_TERRAINMODEL_STMNotAllowedForOperation, "STM are not allowed for this operation."},
        { MSG_TERRAINMODEL_STMFileNotExist, "The STM file {1} doesn't exist."},
        { MSG_TERRAINMODEL_CannotOpenSTM, "Cannot open STM file."},
        { MSG_TERRAINMODEL_CannotOpenSTMFile, "The STM file {1} cannot be open."},
        { MSG_TERRAINMODEL_UnsupportedFileVersion, "Unsupported file version."},   
    }
};
