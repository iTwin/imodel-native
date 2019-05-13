/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*
#pragma managed(push, off)
#define NOMINMAX
#define UNICODE

#include <windows.h>
#undef DialogBox
#undef ERROR
*/

#define NO_RSC_MGR_API  // discourage use of rsc mgr
#define T_LevelIdToDefinitionMapIterator_DEFINED    // Define real T_LevelIdToDefinitionMapIterator

#include <stdlib.h>
/*
#include <StackExaminer.h>
#include <StackExaminerGtestHelper.h>
*/
#include "GeomTestFixture.h"
#include "checkers.h"
#include <Geom/GeomApi.h>
#include <Geom/XYZRangeTree.h>
#include "SampleGeometryCreator.h"
#include "SampleGeometry.h"
#include <ctime>
#include <Vu/VuApi.h>
#include <Mtg/MtgApi.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeNumerical.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"GeomLibs"))

void WriteAsXML (wchar_t const *title, CurveVectorPtr curves, bool indented = false);
IPolyfaceConstructionPtr CreateBuilder (bool normals, bool params);

void PrintPolyface (PolyfaceHeader& mesh, char const* title, FILE *file, size_t maxPrintSize, bool suppressSecondaryData = true);
void PrintPolyfaceSummary (PolyfaceHeader& mesh, char const* title, FILE *file);

void PrintUsageSums
    (
    UsageSums &data,
    const char *title,
    bool fullPrecision = true
    );

// not widely needed, but how to make all the objects in the world depend on it in make files?
#include "FlightPlanner.h"
#include <Mtg/MTGShortestPaths.h>

#define CompileFromGeomLibsGTest
#include "VuSpringModel.h"