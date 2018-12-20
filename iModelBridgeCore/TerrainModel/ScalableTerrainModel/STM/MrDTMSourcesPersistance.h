/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMSourcesPersistance.h $
|    $RCSfile: MrDTMSourcesPersistance.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/18 15:50:49 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>

// External forward declarations
namespace IDTMFile { class SourcesDir; }


BEGIN_BENTLEY_MRDTM_NAMESPACE


// Internal forward declarations
struct IDTMSourceCollection;
struct DocumentEnv;

bool                            SaveSources                        (const IDTMSourceCollection&         sources,
                                                                    IDTMFile::SourcesDir&               sourcesDir,
                                                                    const DocumentEnv&                  sourceEnv);



bool                            LoadSources                        (IDTMSourceCollection&               sources,
                                                                    const IDTMFile::SourcesDir&         sourcesDir,
                                                                    const DocumentEnv&                  sourceEnv);

END_BENTLEY_MRDTM_NAMESPACE