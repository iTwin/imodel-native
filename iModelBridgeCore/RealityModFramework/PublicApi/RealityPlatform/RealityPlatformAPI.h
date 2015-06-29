/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityPlatformAPI.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BentleyApi/BentleyApi.h>
#include <Bentley/RefCounted.h>

#if defined (__REALITYPLATFORM_BUILD__)
#   define REALITYDATAPLATFORM_EXPORT EXPORT_ATTRIBUTE
#else
#   define REALITYDATAPLATFORM_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE         BEGIN_BENTLEY_API_NAMESPACE namespace RealityPlatform {
#define END_BENTLEY_REALITYPLATFORM_NAMESPACE           }}
#define USING_BENTLEY_NAMESPACE_REALITYPLATFORM         using namespace BentleyApi::RealityPlatform;


#define REALITYPLATFORM_TYPEDEFS(t) \
    BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE struct t; END_BENTLEY_REALITYPLATFORM_NAMESPACE \
    ADD_BENTLEY_NAMESPACE_TYPEDEFS (RealityPlatform,t);

#define REALITYPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE struct _sname_; END_BENTLEY_REALITYPLATFORM_NAMESPACE \
    BEGIN_BENTLEY_API_NAMESPACE typedef RefCountedPtr<RealityPlatform::_sname_> _sname_##Ptr; END_BENTLEY_API_NAMESPACE


REALITYPLATFORM_TYPEDEFS(RealityDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataHandler)

REALITYPLATFORM_TYPEDEFS(RasterDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(RasterDataHandler)

REALITYPLATFORM_TYPEDEFS(PointCloudDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(PointCloudDataHandler)

REALITYPLATFORM_TYPEDEFS(WMSDataHandler)
REALITYPLATFORM_REF_COUNTED_PTR(WMSDataHandler)

REALITYPLATFORM_TYPEDEFS(WmsSource)
REALITYPLATFORM_REF_COUNTED_PTR(WmsSource)

REALITYPLATFORM_TYPEDEFS(MapInfo)
REALITYPLATFORM_REF_COUNTED_PTR(MapInfo)