/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/WString.h>
#include <Geom/GeomApi.h>

#ifdef __BENTLEYSMATPUTIL_BUILD__ 
    #define BENTLEY_SM_ATPUTIL_EXPORT __declspec(dllexport)
#else
    #define BENTLEY_SM_ATPUTIL_EXPORT __declspec(dllimport)
#endif

USING_NAMESPACE_BENTLEY

/*
* This singleton class provides generic key/value storage for intermediate results to be used in ATPs.
*/
struct IScalableMeshAtpUtilities
    {
    private:

        IScalableMeshAtpUtilities() {};
        IScalableMeshAtpUtilities(const IScalableMeshAtpUtilities&) {};
        IScalableMeshAtpUtilities&  operator=   (const IScalableMeshAtpUtilities&) {};
       
    public:
       
        BENTLEY_SM_ATPUTIL_EXPORT static void StoreVolumeTestCase(const Utf8String& volumeTestCaseFile, uint64_t elementId, bvector<PolyfaceHeaderPtr> const& candidateMeshes, double expectedCutTotal, double expectedFillTotal);
    };
