/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DesignModelingUnits/DesignModelingUnitsDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>


#ifdef __DESIGN_MODELING_UNITS_DOMAIN_BUILD__
#define DESIGN_MODELING_UNITS_EXPORT EXPORT_ATTRIBUTE
#else
#define DESIGN_MODELING_UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::DesignModelingUnits %DesignModelingUnits data types */
#define BEGIN_BENTLEY_DESIGN_MODELING_UNITS_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace DesignModelingUnits {
#define END_BENTLEY_DESIGN_MODELING_UNITS_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DESIGN_MODELING_UNITS        using namespace BentleyApi::DesignModelingUnits;

#define DESIGN_MODELING_UNITS_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_DESIGN_MODELING_UNITS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_DESIGN_MODELING_UNITS_NAMESPACE

#define DESIGN_MODELING_UNITS_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_DESIGN_MODELING_UNITS_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_DESIGN_MODELING_UNITS_NAMESPACE 


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_NAME                        "DesignModelingUnits"
#define BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_PATH                        L"ECSchemas/Domain/DesignModelingUnits.ecschema.xml"
#define BENTLEY_DESIGN_MODELING_UNITS_SCHEMA(className)                  BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_NAME "." className
#define BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_CODE                        BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_NAME 
#define BENTLEY_DESIGN_MODELING_UNITS_AUTHORITY                          BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_NAME



