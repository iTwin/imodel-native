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


#ifdef __BUILDING_DOMAIN_BUILD__
#define MECHANICAL_FUNCTIONAL_EXPORT EXPORT_ATTRIBUTE
#else
#define MECHANICAL_FUNCTIONAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::MechanicalFunctional %Mechanical_Functional data types */
#define BEGIN_BENTLEY_MECHANICAL_FUNCTIONAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace MechanicalFunctional {
#define END_BENTLEY_MECHANICAL_FUNCTIONAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_MECHANICAL_FUNCTIONAL_COMMON        using namespace BentleyApi::MechanicalFunctional;

#define MECHANICAL_FUNCTIONAL_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_MECHANICAL_FUNCTIONAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_MECHANICAL_FUNCTIONAL_NAMESPACE

#define MECHANICAL_FUNCTIONAL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_MECHANICAL_FUNCTIONAL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_MECHANICAL_FUNCTIONAL_NAMESPACE 


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_NAME                        "MechanicalFunctional"
#define BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_PATH                        L"ECSchemas/Domain/MechanicalFunctional.ecschema.xml"
#define BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA(className)                  BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_NAME "." className
#define BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_CODE                        BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_NAME 
#define BENTLEY_MECHANICAL_FUNCTIONAL_AUTHORITY                          BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define MF_CLASS_Pipeline                             "Pipeline"
#define MF_CLASS_PipeRun                              "PipeRun"
#define MF_CLASS_Valve                                "Valve"
#define MF_CLASS_Reducer                              "Reducer"
#define MF_CLASS_RoundTank                            "RoundTank"
#define MF_CLASS_Tank                                 "Tank"
#define MF_CLASS_3WayValve                            "ThreeWayValve"
#define MF_CLASS_Nozzle                               "Nozzle"
#define MF_CLASS_Pump                                 "Pump"
#define MF_CLASS_Vessel                               "Vessel"
#define MF_CLASS_Unit                                 "Unit"
#define MF_CLASS_SubUnit                              "SubUnit"

#define MF_REL_PipeRunConnectsToPipingComponent             "PipeRunConnectsToPipingComponent"
#define MF_REL_EquipmentOwnsNozzle                          "EquipmentOwnsNozzle"
#define MF_REL_PipeRunOwnsPipingComponents                  "PipeRunOwnsPipingComponents"
#define MF_REL_PipelineOwnsPipeRuns                         "PipelineOwnsPipeRuns"
#define MF_REL_UnitContainsSubUnit                          "UnitContainsSubUnit"
#define MF_REL_SubUnitContainsFunctionalBreakdownElements   "SubUnitContainsFunctionalBreakdownElements"
#define MF_REL_SubUnitContainsFunctionalComponentElements   "SubUnitContainsFunctionalComponentElements"



