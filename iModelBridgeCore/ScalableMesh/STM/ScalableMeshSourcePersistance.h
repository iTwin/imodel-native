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
    static const UInt           FORMAT_VERSION;
#ifdef SCALABLE_MESH_DGN
    bool                        Serialize(const IDTMSource&               source,
        const DocumentEnv&              env,
        Import::SourceDataSQLite&                  sourceData) const;

    IDTMSourcePtr               Deserialize(Import::SourceDataSQLite&                  sourceData,
        const DocumentEnv&              env,
        UInt                            formatVersion) const;
#else
    bool                        Serialize                              (const IDTMSource&               source,
                                                                        const DocumentEnv&              env,
                                                                        BinaryOStream&                  stream) const;

    IDTMSourcePtr               Deserialize                            (BinaryIStream&                  stream,
                                                                        const DocumentEnv&              env,
                                                                        UInt                            formatVersion) const;
#endif
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
