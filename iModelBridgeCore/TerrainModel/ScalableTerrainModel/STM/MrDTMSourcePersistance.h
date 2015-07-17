/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMSourcePersistance.h $
|    $RCSfile: MrDTMSourcePersistance.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/18 15:50:46 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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