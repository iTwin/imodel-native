/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/ArchitecturalPhysicalDefinitions.h $
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

#ifdef __ARCHITECTURAL_PHYSICAL_BUILD__
    #define ARCHITECTURAL_PHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
    #define ARCHITECTURAL_PHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::ARCHITECTURAL_PHYSICAL %ARCHITECTURAL_PHYSICAL data types */
#define BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace ArchitecturalPhysical {
#define END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ARCHITECTURAL_PHYSICAL        using namespace BentleyApi::ArchitecturalPhysical;

#define ARCHITECTURAL_PHYSICAL_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

#define ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE 

#define PRIMARY_BASELINE_LABEL "__Primary"

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME                        "ArchitectrualPhysical"
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/ArchitectrualPhysical.01.00.00.ecschema.xml"
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA(className)                  BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_CODE                        BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME 
#define BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY                          BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME

//#define ARCHITECTURAL_PHYSICAL_SCHEMA(name)                               BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME "." name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
/*#define BP_CLASS_Plan                                       "Plan"
#define BP_CLASS_PlanningElement                            "PlanningElement"
#define BP_CLASS_WorkBreakdown                              "WorkBreakdown"
#define BP_CLASS_Activity                                   "Activity"
#define BP_CLASS_Baseline                                   "Baseline"
#define BP_CLASS_TimeSpan                                   "TimeSpan"
#define BP_CLASS_CameraAnimation                            "CameraAnimation"
#define BP_CLASS_CameraKeyFrame                             "CameraKeyFrame"
#define BP_CLASS_PlanningPartition                          "PlanningPartition"
#define BP_CLASS_PlanningModel                              "PlanningModel"
#define BP_CLASS_ElementAppearanceProfile                   "ElementAppearanceProfile"
*/
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
// Authority names
//-----------------------------------------------------------------------------------------
//#define BP_AUTHORITY_Planning                               "Planning"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
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

//END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE
