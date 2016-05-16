/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcePersistance.h $
|    $RCSfile: ScalableMeshSourcePersistance.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/18 15:50:46 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh/Import/DataSQLite.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct BinaryOStream;
struct BinaryIStream;
struct DocumentEnv;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceSerializer
    {
    static const uint32_t           FORMAT_VERSION;

    bool                        Serialize(const IDTMSource&               source,
        const DocumentEnv&              env,
        Import::SourceDataSQLite&                  sourceData) const;

    IDTMSourcePtr               Deserialize(Import::SourceDataSQLite&                  sourceData,
        const DocumentEnv&              env,
        uint32_t                            formatVersion) const;

    };

enum DTMSourceId
    {
    DTM_SOURCE_ID_BASE_V0,
    DTM_SOURCE_ID_LOCAL_FILE_V0,
    DTM_SOURCE_ID_IN_MEMORY_BASE_V0,
    DTM_SOURCE_ID_DGN_V0,
    DTM_SOURCE_ID_DGN_LEVEL_V0,
    DTM_SOURCE_ID_IN_MEMORY_DGN_LEVEL_V0,
    DTM_SOURCE_ID_BC_OBJ_V0,
    DTM_SOURCE_ID_GROUP_V0,
    DTM_SOURCE_ID_DGN_V1,
    DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V0,
    DTM_SOURCE_ID_DGN_LEVEL_V1,
    DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V1,

    DTM_SOURCE_ID_QTY
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
