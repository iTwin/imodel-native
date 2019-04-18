/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>

#define BENTLEY_BUILDING_NAMESPACE_NAME BENTLEY_NAMESPACE_NAME::Building
#define BEGIN_BUILDING_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Building {
#define END_BUILDING_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BUILDING using namespace BENTLEY_BUILDING_NAMESPACE_NAME;

#define BENTLEY_BUILDING_SHARED_NAMESPACE_NAME BENTLEY_BUILDING_NAMESPACE_NAME::Shared
#define BEGIN_BUILDING_SHARED_NAMESPACE BEGIN_BUILDING_NAMESPACE namespace Shared {
#define END_BUILDING_SHARED_NAMESPACE } END_BUILDING_NAMESPACE
#define USING_NAMESPACE_BUILDING_SHARED using namespace BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

#define BUILDING_InformationPartition                             "BuildingConceptStationGroup_InformationPartition"

#if defined (__BUILDINGSHAREDUNITS_BUILD__)
#define BUILDINGSHAREDUNITS_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSHAREDUNITS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__BUILDINGSHAREDUTILS_BUILD__)
#define BUILDINGSHAREDUTILS_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSHAREDUTILS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__BUILDINGSHAREDDGNUTILS_BUILD__)
#define BUILDINGSHAREDDGNUTILS_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSHAREDDGNUTILS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__BUILDINGSHAREDDGNVIEWUTILS_BUILD__)
#define BUILDINGSHAREDDGNVIEWUTILS_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSHAREDDGNVIEWUTILS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__GEOMETRYMANIPULATIONSTRATEGIES_BUILD__)
#define GEOMETRYMANIPULATIONSTRATEGIES_EXPORT EXPORT_ATTRIBUTE
#else
#define GEOMETRYMANIPULATIONSTRATEGIES_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__DGNELEMENTMANIPULATIONSTRATEGIES_BUILD__)
#define DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT EXPORT_ATTRIBUTE
#else
#define DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__STRATEGYPLATFORM_BUILD__)
#define STRATEGYPLATFORM_EXPORT EXPORT_ATTRIBUTE
#else
#define STRATEGYPLATFORM_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define BUILDING_SHARED_TYPEDEFS(_name_) \
    BEGIN_BUILDING_SHARED_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
    END_BUILDING_SHARED_NAMESPACE

#ifdef DGNV8_BUILD
#define BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BUILDING_SHARED_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_POINTER(_name_) \
        typedef RefCountedPtr<_name_> _name_##CPtr; \
    END_BUILDING_SHARED_NAMESPACE
#else
#define BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BUILDING_SHARED_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_BUILDING_SHARED_NAMESPACE
#endif
