/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcesPersistance.h $
|    $RCSfile: ScalableMeshSourcesPersistance.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/18 15:50:49 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh\IScalableMeshSourceImporter.h>


// External forward declarations
namespace IDTMFile { class SourcesDir; }


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


// Internal forward declarations
struct IDTMSourceCollection;
struct DocumentEnv;

bool                            SaveSources                        (const IDTMSourceCollection&         sources,
                                                                    IDTMFile::SourcesDir&               sourcesDir,
                                                                    const DocumentEnv&                  sourceEnv);

bool                            SaveSources                        (const IDTMSourceCollection&         sources,
                                                                    IScalableMeshSourceImporterStoragePtr&     sourceImporterStoragePtr,
                                                                    const DocumentEnv&                  sourceEnv);

bool                            LoadSources                        (IDTMSourceCollection&               sources,
                                                                    const IDTMFile::SourcesDir&         sourcesDir,
                                                                    const DocumentEnv&                  sourceEnv);

bool                            LoadSources                        (IDTMSourceCollection&               sources,
                                                                    IScalableMeshSourceImporterStoragePtr&     sourceImporterStoragePtr,
                                                                    const DocumentEnv&                  sourceEnv);

END_BENTLEY_SCALABLEMESH_NAMESPACE
