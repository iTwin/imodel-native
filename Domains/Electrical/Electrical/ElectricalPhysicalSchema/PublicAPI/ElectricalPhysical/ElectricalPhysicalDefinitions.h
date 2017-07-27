/*--------------------------------------------------------------------------------------+
|
|     $Source: Electrical/ElectricalPhysicalSchema/PublicAPI/ElectricalPhysical/ElectricalPhysicalDefinitions.h $
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
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define EP_CLASS_Device                                                        "Device"
#define EP_CLASS_DeviceType                                                    "DeviceType"
#define EP_CLASS_ElectricalTypeDefinitionModel                                 "ElectricalTypeDefinitionModel"
#define EP_CLASS_ElectricalPhysicalModel                                       "ElectricalPhysicalModel"
#define EP_CLASS_ElectricalBaseElement                                         "ElectricalBaseElement"

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
ELECTRICAL_PHYSICAL_TYPEDEFS(ElectricalBaseElement)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(ElectricalBaseElement)
ELECTRICAL_PHYSICAL_TYPEDEFS(ElectricalPhysicalModel)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(ElectricalPhysicalModel)
ELECTRICAL_PHYSICAL_TYPEDEFS(ElectricalTypeDefinitionModel)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(ElectricalTypeDefinitionModel)
ELECTRICAL_PHYSICAL_TYPEDEFS(Device)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(Device)
ELECTRICAL_PHYSICAL_TYPEDEFS(DeviceType)
ELECTRICAL_PHYSICAL_REFCOUNTED_TYPEDEFS(DeviceType)

/*PLANNING_TYPEDEFS(Calendar)
PLANNING_TYPEDEFS(Activity)
PLANNING_REFCOUNTED_TYPEDEFS(Activity)
PLANNING_TYPEDEFS(Duration)
PLANNING_TYPEDEFS(Plan)
PLANNING_REFCOUNTED_TYPEDEFS(Plan)
PLANNING_TYPEDEFS(Baseline)
PLANNING_REFCOUNTED_TYPEDEFS(Baseline)
PLANNING_TYPEDEFS(TimeSpan)
PLANNING_REFCOUNTED_TYPEDEFS(TimeSpan)
PLANNING_TYPEDEFS(WorkBreakdown)
PLANNING_REFCOUNTED_TYPEDEFS(WorkBreakdown)
PLANNING_TYPEDEFS(CameraAnimation)
PLANNING_REFCOUNTED_TYPEDEFS(CameraAnimation)
PLANNING_TYPEDEFS(CameraInfo)
PLANNING_TYPEDEFS(CameraKeyFrame)
PLANNING_REFCOUNTED_TYPEDEFS(CameraKeyFrame)
PLANNING_TYPEDEFS(PlanningElement)
PLANNING_REFCOUNTED_TYPEDEFS(PlanningElement)
PLANNING_TYPEDEFS(PlanningPartition)
PLANNING_REFCOUNTED_TYPEDEFS(PlanningPartition)
PLANNING_TYPEDEFS(PlanningModel)
PLANNING_REFCOUNTED_TYPEDEFS(PlanningModel)
PLANNING_TYPEDEFS(ElementAppearanceProfile)
PLANNING_REFCOUNTED_TYPEDEFS(ElementAppearanceProfile)

BEGIN_BENTLEY_PLANNING_NAMESPACE

BEBRIEFCASEBASED_ID_SUBCLASS(WorkBreakdownId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(PlanId, WorkBreakdownId)
BEBRIEFCASEBASED_ID_SUBCLASS(ActivityId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(PlanningModelId, Dgn::DgnModelId)
BEBRIEFCASEBASED_ID_SUBCLASS(CameraAnimationId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(ElementAppearanceProfileId, Dgn::DgnElementId)

typedef BeSQLite::EC::ECInstanceId CameraKeyFrameId;
typedef BeSQLite::EC::ECInstanceId BaselineId;
typedef BeSQLite::EC::ECInstanceId TimeSpanId;
typedef BeSQLite::IdSet<ActivityId> ActivityIdSet;
*/
/**
* @addtogroup GROUP_Planning Planning Module
* Types related to the Planning domain
* @ref PAGE_Planning
*/
BEGIN_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE

END_BENTLEY_ELECTRICAL_PHYSICAL_NAMESPACE


