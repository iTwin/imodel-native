/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/IScalableMeshSourceImporter.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh/Import/DataSQLite.h>


// External forward declarations
namespace ISMStore { class SourcesDir; }

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


// Internal forward declarations
struct IDTMSourceCollection;
struct DocumentEnv;

BENTLEY_SM_EXPORT bool                            SaveSources                        (const IDTMSourceCollection&         sources,
                                                                    SourcesDataSQLite&                  sourcesData,
                                                                    const DocumentEnv&                  sourceEnv);

BENTLEY_SM_EXPORT bool                            LoadSources                        (IDTMSourceCollection&               sources,
                                                                    SourcesDataSQLite&                  sourcesData,
                                                                    const DocumentEnv&                  sourceEnv);


BENTLEY_SM_EXPORT bool                            SaveSources                        (const IDTMSourceCollection&         sources,
                                                                    IScalableMeshSourceImporterStoragePtr&     sourceImporterStoragePtr,
                                                                    const DocumentEnv&                  sourceEnv);

BENTLEY_SM_EXPORT bool                            LoadSources                        (IDTMSourceCollection&               sources,
                                                                    IScalableMeshSourceImporterStoragePtr&     sourceImporterStoragePtr,
                                                                    const DocumentEnv&                  sourceEnv);

END_BENTLEY_SCALABLEMESH_NAMESPACE
