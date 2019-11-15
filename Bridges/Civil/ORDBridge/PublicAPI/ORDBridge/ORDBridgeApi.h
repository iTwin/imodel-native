/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <iModelBridge/iModelBridge.h>

#ifdef __ORDBRIDGE_BUILD__
#define ORDBRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
#define ORDBRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

// For some reason, the lib file won't be created if no C++ class is exported
struct DummyClass
    {
    ORDBRIDGE_EXPORT static bool DummyMethod();
    };

extern "C"
    {
    ORDBRIDGE_EXPORT BentleyApi::Dgn::iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeName);
    ORDBRIDGE_EXPORT BentleyStatus iModelBridge_releaseInstance(BentleyApi::Dgn::iModelBridge* bridge);
    ORDBRIDGE_EXPORT void iModelBridge_getAffinity(WCharP buffer, const size_t bufferSize, BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
        WCharCP affinityLibraryPath, WCharCP sourceFileName);
    ORDBRIDGE_EXPORT wchar_t const* iModelBridge_getRegistrySubKey();
    }