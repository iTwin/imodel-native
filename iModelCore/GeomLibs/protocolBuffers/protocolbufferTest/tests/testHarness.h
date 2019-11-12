/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#define NO_RSC_MGR_API  // discourage use of rsc mgr
#define T_LevelIdToDefinitionMapIterator_DEFINED    // Define real T_LevelIdToDefinitionMapIterator

#include <stdlib.h>
#include "../../src/checkers.h"
#if defined (_WIN32)
#include "../../src/CGWriter.h"
#endif
#include <Geom/GeomApi.h>
#include <Mtg/MtgApi.h>
#include <Geom/XYZRangeTree.h>
#include <ctime>
