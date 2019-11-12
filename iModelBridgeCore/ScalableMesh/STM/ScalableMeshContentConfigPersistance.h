/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/Import/ContentConfig.h>
#include "ScalableMeshSourcesPersistance.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//struct BinaryOStream;
//struct BinaryIStream;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigSerializer
    {
    static const uint32_t           FORMAT_VERSION;
    bool                        Serialize(const Import::ContentConfig&    config,
        SourceDataSQLite&                  sourceData) const;

    bool                        Deserialize(SourceDataSQLite&                  sourceData,
        Import::ContentConfig&          config,
        uint32_t                            formatVersion) const;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
