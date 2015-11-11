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

REALITYPLATFORM_TYPEDEFS(RealityData)
REALITYPLATFORM_REF_COUNTED_PTR(RealityData)

REALITYPLATFORM_TYPEDEFS(RasterData)
REALITYPLATFORM_REF_COUNTED_PTR(RasterData)

REALITYPLATFORM_TYPEDEFS(PointCloudData)
REALITYPLATFORM_REF_COUNTED_PTR(PointCloudData)

REALITYPLATFORM_TYPEDEFS(WmsData)
REALITYPLATFORM_REF_COUNTED_PTR(WmsData)

REALITYPLATFORM_TYPEDEFS(WmsMapInfo)
REALITYPLATFORM_REF_COUNTED_PTR(WmsMapInfo)

REALITYPLATFORM_TYPEDEFS(UsgsSource)
REALITYPLATFORM_REF_COUNTED_PTR(UsgsSource)

REALITYPLATFORM_TYPEDEFS(RealityDataDownload)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataDownload)

REALITYPLATFORM_TYPEDEFS(SpatioTemporalDataset)
REALITYPLATFORM_REF_COUNTED_PTR(SpatioTemporalDataset)

REALITYPLATFORM_TYPEDEFS(SpatioTemporalData)
REALITYPLATFORM_REF_COUNTED_PTR(SpatioTemporalData)
