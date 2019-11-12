/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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