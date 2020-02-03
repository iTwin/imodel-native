/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableTerrainModel/Import/ImportSequence.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

struct BinaryOStream;
struct BinaryIStream;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportSequenceSerializer
    {
    static const UInt           FORMAT_VERSION;

    bool                        Serialize                              (const Import::ImportSequence&   sequence,
                                                                        BinaryOStream&                  stream) const;

    bool                        Deserialize                            (BinaryIStream&                  stream,
                                                                        Import::ImportSequence&         sequence,
                                                                        UInt                            formatVersion) const;
    };

END_BENTLEY_MRDTM_NAMESPACE