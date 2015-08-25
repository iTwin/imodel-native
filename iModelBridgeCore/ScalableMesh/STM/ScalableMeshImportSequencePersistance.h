/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshImportSequencePersistance.h $
|    $RCSfile: ScalableMeshImportSequencePersistance.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/11/18 15:50:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/Import/ImportSequence.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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

END_BENTLEY_SCALABLEMESH_NAMESPACE
