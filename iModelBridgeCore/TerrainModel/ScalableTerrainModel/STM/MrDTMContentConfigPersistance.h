/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMContentConfigPersistance.h $
|    $RCSfile: MrDTMContentConfigPersistance.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/11/18 15:50:30 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableTerrainModel/Import/ContentConfig.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

struct BinaryOStream;
struct BinaryIStream;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigSerializer
    {
    static const UInt           FORMAT_VERSION;

    bool                        Serialize                              (const Import::ContentConfig&    config,
                                                                        BinaryOStream&                  stream) const;

    bool                        Deserialize                            (BinaryIStream&                  stream,
                                                                        Import::ContentConfig&          config,
                                                                        UInt                            formatVersion) const;
    };

END_BENTLEY_MRDTM_NAMESPACE