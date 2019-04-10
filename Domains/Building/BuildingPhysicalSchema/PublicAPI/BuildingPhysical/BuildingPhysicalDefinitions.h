/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingPhysicalSchema/PublicAPI/BuildingPhysical/BuildingPhysicalDefinitions.h $
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


#ifdef __BUILDING_DOMAIN_BUILD__
#define BUILDING_PHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDING_PHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::BuildingPhysical %BUILDING_PHYSICAL data types */
#define BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace BuildingPhysical {
#define END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_BUILDING_PHYSICAL        using namespace BentleyApi::BuildingPhysical;

#define BUILDING_PHYSICAL_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE

#define BUILDING_PHYSICAL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE 


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_BUILDING_PHYSICAL_SCHEMA_NAME                        "BuildingPhysical"
#define BENTLEY_BUILDING_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/BuildingPhysical.ecschema.xml"
#define BENTLEY_BUILDING_PHYSICAL_SCHEMA(className)                  BENTLEY_BUILDING_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_BUILDING_PHYSICAL_SCHEMA_CODE                        BENTLEY_BUILDING_PHYSICAL_SCHEMA_NAME 
#define BENTLEY_BUILDING_PHYSICAL_AUTHORITY                          BENTLEY_BUILDING_PHYSICAL_SCHEMA_NAME



//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BP_CLASS_BuildingTypeDefinitionModel                                 "BuildingTypeDefinitionModel"
#define BP_CLASS_BuildingPhysicalModel                                       "BuildingPhysicalModel"

//-----------------------------------------------------------------------------------------
// ECRelationshipClass names (combine with PLANNING_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
/*
#define BP_REL_WorkBreakdownOwnsWorkBreakdowns              "WorkBreakdownOwnsWorkBreakdowns"
#define BP_REL_WorkBreakdownOwnsActivities                  "WorkBreakdownOwnsActivities"
#define BP_REL_ActivityAffectsElements                      "ActivityAffectsElements"
#define BP_REL_ActivityHasConstraint                        "ActivityHasConstraint"
#define BP_REL_PlanOwnsBaselines                            "PlanOwnsBaselines"

#define BP_REL_CameraAnimationOwnsKeyFrames                 "CameraAnimationOwnsKeyFrames"
*/
//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
//#define BP_CATEGORY_Planning                                "Planning"

//-----------------------------------------------------------------------------------------                           
//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
BUILDING_PHYSICAL_TYPEDEFS(BuildingPhysicalModel)
BUILDING_PHYSICAL_REFCOUNTED_TYPEDEFS(BuildingPhysicalModel)
BUILDING_PHYSICAL_TYPEDEFS(BuildingTypeDefinitionModel)
BUILDING_PHYSICAL_REFCOUNTED_TYPEDEFS(BuildingTypeDefinitionModel)

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
BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE

END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE


