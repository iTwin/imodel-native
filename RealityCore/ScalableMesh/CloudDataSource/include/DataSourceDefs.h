/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#if _WIN32
#ifdef __CLOUD_DATA_SOURCE_BUILD__ 
    #define CLOUD_EXPORT __declspec(dllexport)
#else
    #define CLOUD_EXPORT __declspec(dllimport)
#endif
#else
    #define CLOUD_EXPORT
#endif
