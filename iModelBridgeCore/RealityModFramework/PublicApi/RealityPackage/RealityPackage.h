/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityPackage.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BentleyApi/BentleyApi.h>
#include <Bentley/RefCounted.h>

#if defined (__REALITYPACKAGE_BUILD__)
#   define REALITYPACKAGE_EXPORT EXPORT_ATTRIBUTE
#else
#   define REALITYPACKAGE_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE              BEGIN_BENTLEY_API_NAMESPACE namespace RealityPackage {
#define END_BENTLEY_REALITYPACKAGE_NAMESPACE                }}
#define USING_BENTLEY_NAMESPACE_REALITYPACKAGE              using namespace BentleyApi::RealityPackage;


#define REALITYPACKAGE_TYPEDEFS(t) \
    BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE struct t; END_BENTLEY_REALITYPACKAGE_NAMESPACE \
    ADD_BENTLEY_NAMESPACE_TYPEDEFS (RealityPackage,t);

#define REALITYPACKAGE_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE struct _sname_; END_BENTLEY_REALITYPACKAGE_NAMESPACE \
    BEGIN_BENTLEY_API_NAMESPACE typedef RefCountedPtr<RealityPackage::_sname_> _sname_##Ptr; END_BENTLEY_API_NAMESPACE


REALITYPACKAGE_TYPEDEFS(RealityDataPackage)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataPackage)

REALITYPACKAGE_TYPEDEFS(RealityDataSource)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataSource)
REALITYPACKAGE_TYPEDEFS(PinnedSource)
REALITYPACKAGE_REF_COUNTED_PTR(PinnedSource)

REALITYPACKAGE_TYPEDEFS(BoundingPolygon)
REALITYPACKAGE_REF_COUNTED_PTR(BoundingPolygon)

