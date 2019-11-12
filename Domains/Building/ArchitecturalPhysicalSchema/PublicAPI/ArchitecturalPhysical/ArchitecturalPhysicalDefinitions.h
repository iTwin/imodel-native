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

//namespace ARCHPHY = BentleyApi::ArchitecturalPhysical;
//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME                        "ArchitecturalPhysical"
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/ArchitecturalPhysical.ecschema.xml"
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA(className)                  BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_CODE                        BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME 
#define BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY                          BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME

//#define ARCHITECTURAL_PHYSICAL_SCHEMA(name)                               BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME "." name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define AP_CLASS_Casework                                   "Casework"
#define AP_CLASS_CurtianWall                                "CurtianWall"
#define AP_CLASS_Door                                       "Door"
#define AP_CLASS_DoorType                                   "DoorType"
#define AP_CLASS_Floor                                      "Floor"
#define AP_CLASS_Furniture                                  "Furniture"
#define AP_CLASS_Railing                                    "Railing"
#define AP_CLASS_Ramp                                       "Ramp"
#define AP_CLASS_Stair                                      "Stair"
#define AP_Class_TransportationMechanism                    "TransportationMechanism"
#define AP_CLASS_Wall                                       "Wall"
#define AP_CLASS_WallLeaf                                   "WallLeaf"
#define AP_CLASS_WallType                                   "WallType"
#define AP_CLASS_Window                                     "Window"
#define AP_CLASS_WindowType                                 "WindowType"
#define AP_Class_WallOwnsWallLeafs                          "WallOwnsWallLeafs"

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
#define ARCHITECTURAL_PHYSICAL_CATEGORY_Doors        BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME " Doors"
#define ARCHITECTURAL_PHYSICAL_CATEGORY_Walls        BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME " Walls"
#define ARCHITECTURAL_PHYSICAL_CATEGORY_Windows      BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME " Windows"
#define ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame     "Frame"
#define ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel     "Panel"

//-----------------------------------------------------------------------------------------
// Authority names
//-----------------------------------------------------------------------------------------
//#define BP_AUTHORITY_Planning                               "Planning"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
ARCHITECTURAL_PHYSICAL_TYPEDEFS(ArchitecturalBaseElement)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(ArchitecturalBaseElement)
ARCHITECTURAL_PHYSICAL_TYPEDEFS(Door)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(Door)
ARCHITECTURAL_PHYSICAL_TYPEDEFS(DoorType)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(DoorType)
ARCHITECTURAL_PHYSICAL_TYPEDEFS(Window)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(Window)
ARCHITECTURAL_PHYSICAL_TYPEDEFS(WindowType)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(WindowType)
ARCHITECTURAL_PHYSICAL_TYPEDEFS(WallType)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(WallType)
ARCHITECTURAL_PHYSICAL_TYPEDEFS(Wall)
ARCHITECTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(Wall)

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



