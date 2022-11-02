/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>

#if defined(__GEOMLIBS_SERIALIZATION_BUILD__)
    #define GEOMLIBS_SERIALIZATION_EXPORT EXPORT_ATTRIBUTE
#else
    #define GEOMLIBS_SERIALIZATION_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_GEOMETRY_NAMESPACE BEGIN_BENTLEY_NAMESPACE
#define END_BENTLEY_GEOMETRY_NAMESPACE   END_BENTLEY_NAMESPACE

struct BeExtendedDataEntry
    {
    Utf8String Key;
    Utf8String Type;
    Utf8String Value;
    };

typedef bvector<BeExtendedDataEntry> BeExtendedData;
typedef bmap<OrderedIGeometryPtr, BeExtendedData> BeExtendedDataGeometryMap;
