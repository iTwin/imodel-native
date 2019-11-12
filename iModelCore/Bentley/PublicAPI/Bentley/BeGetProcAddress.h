/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
// __PUBLISH_SECTION_START__
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Static methods for loading a shared library and function dynamically by name
// @bsiclass                                                    Sam.Wilson      09/2017
//=======================================================================================
struct BeGetProcAddress
    {
    BENTLEYDLL_EXPORT static void  SetLibrarySearchPath(BeFileNameCR pathname);
    BENTLEYDLL_EXPORT static void* LoadLibrary(BeFileNameCR szMyLib);
    BENTLEYDLL_EXPORT static void  UnloadLibrary(void*);
    BENTLEYDLL_EXPORT static void* GetProcAddress(void*, Utf8CP procName);
    };

END_BENTLEY_NAMESPACE
