/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityPlatformAPI.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Geom/GeomApi.h>

#if defined (__REALITYPLATFORM_BUILD__)
#   define REALITYDATAPLATFORM_EXPORT EXPORT_ATTRIBUTE
#else
#   define REALITYDATAPLATFORM_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE         BEGIN_BENTLEY_NAMESPACE namespace RealityPlatform {
#define END_BENTLEY_REALITYPLATFORM_NAMESPACE           } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_REALITYPLATFORM         using namespace BentleyApi::RealityPlatform;


#define REALITYPLATFORM_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_REALITYPLATFORM_NAMESPACE

#define REALITYPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_REALITYPLATFORM_NAMESPACE

REALITYPLATFORM_TYPEDEFS(RealityDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataHandler)

REALITYPLATFORM_TYPEDEFS(RasterDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(RasterDataHandler)

REALITYPLATFORM_TYPEDEFS(PointCloudDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(PointCloudDataHandler)

REALITYPLATFORM_TYPEDEFS(WMSDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(WMSDataHandler)

REALITYPLATFORM_TYPEDEFS(WmsMapInfo)
REALITYPLATFORM_REF_COUNTED_PTR(WmsMapInfo)

REALITYPLATFORM_TYPEDEFS(UsgsSource)
REALITYPLATFORM_REF_COUNTED_PTR(UsgsSource)