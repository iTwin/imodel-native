/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>

#ifdef __ELECTRICAL_DOMAIN_BUILD__
#define ELECTRICAL_PHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
#define ELECTRICAL_PHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::ElectricalPhysical %ELECTRICAL_PHYSICAL data types */
#define BEGIN_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace ElectricalPhysical {
#define END_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ELECTRICAL_PHYSICAL        using namespace BentleyApi::ElectricalPhysical;

#define ELECTRICAL_PHYSICAL_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE

#define ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE 

#define PRIMARY_BASELINE_LABEL "__Primary"

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME                        "ElectricalPhysical"
#define BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/ElectricalPhysical.ecschema.xml"
#define BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA(className)                  BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_CODE                        BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME 
#define BENTLEY_ELECTRICAL_PHYSICAL_AUTHORITY                          BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME



//-----------------------------------------------------------------------------------------
// ECClass names (combine with ELECTRICAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define EP_CLASS_Device                                                        "Device"
#define EP_CLASS_ElectricalTypeDefinitionModel                                 "ElectricalTypeDefinitionModel"
#define EP_CLASS_ElectricalPhysicalModel                                       "ElectricalPhysicalModel"
#define EC_CLASS_Classification                                                "Classification"
#define EC_CLASS_Manufacturer                                                  "Manufacturer"

//-----------------------------------------------------------------------------------------
// ECRelationshipClass names (combine with PLANNING_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
//#define EP_CATEGORY_Electrical                                "Electrical"
#define ELECTRICAL_PHYSICAL_CATEGORY_Devices        BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME " Devices"

//-----------------------------------------------------------------------------------------                           
//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
ELECTRICAL_PHYSICAL_TYPEDEFS(ElectricalPhysicalModel)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(ElectricalPhysicalModel)
ELECTRICAL_PHYSICAL_TYPEDEFS(ElectricalTypeDefinitionModel)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(ElectricalTypeDefinitionModel)
ELECTRICAL_PHYSICAL_TYPEDEFS(Device)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(Device)

BEGIN_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE

END_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE


