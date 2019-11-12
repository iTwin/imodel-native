/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableTerrainModel/IMrDTMSources.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE


struct BinaryOStream;
struct BinaryIStream;
struct DocumentEnv;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceSerializer
    {
    static const UInt           FORMAT_VERSION;

    bool                        Serialize                              (const IDTMSource&               source,
                                                                        const DocumentEnv&              env,
                                                                        BinaryOStream&                  stream) const;

    IDTMSourcePtr               Deserialize                            (BinaryIStream&                  stream,
                                                                        const DocumentEnv&              env,
                                                                        UInt                            formatVersion) const;
    };

END_BENTLEY_MRDTM_NAMESPACE