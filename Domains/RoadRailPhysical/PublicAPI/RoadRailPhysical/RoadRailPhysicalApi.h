#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>

#ifdef __ROADRAILPHYSICAL_BUILD__
#define ROADRAILPHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
#define ROADRAILPHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ROADRAILPHYSICAL    "RoadRailPhysical"
#if defined (ANDROID)
#include <android/log.h>
#define DGNCLIENTFX_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define ROADRAILPHYSICAL_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_ROADRAILPHYSICAL))
#define ROADRAILPHYSICAL_LOGD(...)           ROADRAILPHYSICAL_LOG.debugv (__VA_ARGS__);
#define ROADRAILPHYSICAL_LOGI(...)           ROADRAILPHYSICAL_LOG.infov (__VA_ARGS__);
#define ROADRAILPHYSICAL_LOGW(...)           ROADRAILPHYSICAL_LOG.warningv (__VA_ARGS__);
#define ROADRAILPHYSICAL_LOGE(...)           ROADRAILPHYSICAL_LOG.errorv (__VA_ARGS__);
#define ROADRAILPHYSICAL_ASSERT_LOGD(...)    BeAssert(!__VA_ARGS__); ROADRAILPHYSICAL_LOGD(__VA_ARGS__);
#define ROADRAILPHYSICAL_ASSERT_LOGI(...)    BeAssert(!__VA_ARGS__); ROADRAILPHYSICAL_LOGI(__VA_ARGS__);
#define ROADRAILPHYSICAL_ASSERT_LOGW(...)    BeAssert(!__VA_ARGS__); ROADRAILPHYSICAL_LOGW(__VA_ARGS__);   
#define ROADRAILPHYSICAL_ASSERT_LOGE(...)    BeAssert(!__VA_ARGS__); ROADRAILPHYSICAL_LOGE(__VA_ARGS__);   
#endif

/** @namespace BentleyApi::RoadRailPhysical %RoadRailPhysical data types */
#define BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace RoadRailPhysical {
#define END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADRAILPHYSICAL        using namespace BENTLEY_NAMESPACE_NAME::RoadRailPhysical;

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BRRP_SCHEMA_NAME                             "RoadRailPhysical"
#define BRRP_SCHEMA_FILE                             L"RoadRailPhysical.ecschema.xml"
#define BRRP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRP_SCHEMA_PATH                             BRRP_SCHEMA_LOCATION BRRP_SCHEMA_FILE
#define BRRP_SCHEMA(name)                            BRRP_SCHEMA_NAME "." name
#define BRRP_SCHEMA_CODE(name)                       BRRP_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Models
#define BRRP_CLASS_TypicalSectionModel                              "TypicalSectionModel"
#define BRRP_CLASS_TypicalSectionPortionBreakDownModel              "TypicalSectionPortionBreakDownModel"
#define BRRP_CLASS_EndConditionDefinitionModel                      "EndConditionDefinitionModel"
#define BRRP_CLASS_RoadClassDefinitionModel                         "RoadClassDefinitionModel"
#define BRRP_CLASS_RoadClassDefinitionTableModel                    "RoadClassDefinitionTableModel"
#define BRRP_CLASS_RoadDesignSpeedDefinitionModel                   "RoadDesignSpeedDefinitionModel"
#define BRRP_CLASS_RoadDesignSpeedDefinitionTableModel              "RoadDesignSpeedDefinitionTableModel"
#define BRRP_CLASS_RoadRailCategoryModel                            "RoadRailCategoryModel"
#define BRRP_CLASS_RoadwayStandardsModel                            "RoadwayStandardsModel"
#define BRRP_CLASS_TravelwayDefinitionModel                         "TravelwayDefinitionModel"


// Elements
#define BRRP_CLASS_AlignmentIntersectionElement                     "AlignmentIntersectionElement"
#define BRRP_CLASS_EndConditionDefinition                           "EndConditionDefinition"
#define BRRP_CLASS_LinearlyLocatedStatus                            "LinearlyLocatedStatus"
#define BRRP_CLASS_PathwayElement                                   "PathwayElement"
#define BRRP_CLASS_Railway                                          "Railway"
#define BRRP_CLASS_RegularTravelwaySegment                          "RegularTravelwaySegment"
#define BRRP_CLASS_RoadTypicalSection                               "RoadTypicalSection"
#define BRRP_CLASS_RoadClass                                        "RoadClass"
#define BRRP_CLASS_RoadClassDefinition                              "RoadClassDefinition"
#define BRRP_CLASS_RoadClassDefinitionTable                         "RoadClassDefinitionTable"
#define BRRP_CLASS_RoadClassStandards                               "RoadClassStandards"
#define BRRP_CLASS_RoadDesignSpeed                                  "RoadDesignSpeed"
#define BRRP_CLASS_RoadDesignSpeedDefinition                        "RoadDesignSpeedDefinition"
#define BRRP_CLASS_RoadDesignSpeedDefinitionTable                   "RoadDesignSpeedDefinitionTable"
#define BRRP_CLASS_RoadDesignSpeedStandards                         "RoadDesignSpeedStandards"
#define BRRP_CLASS_RoadIntersectionElement                          "RoadIntersectionElement"
#define BRRP_CLASS_RoadIntersectionLegElement                       "RoadIntersectionLegElement"
#define BRRP_CLASS_Roadway                                          "Roadway"
#define BRRP_CLASS_RoadTravelwayDefinition                          "RoadTravelwayDefinition"
#define BRRP_CLASS_TravelwayDefinitionElement                       "TravelwayDefinitionElement"
#define BRRP_CLASS_TravelwayIntersectionSegmentElement              "TravelwayIntersectionSegmentElement"
#define BRRP_CLASS_TravelwaySegmentElement                          "TravelwaySegmentElement"
#define BRRP_CLASS_TravelwayTransition                              "TravelwayTransition"
#define BRRP_CLASS_TypicalSectionElement                            "TypicalSectionElement"
#define BRRP_CLASS_TypicalSectionPortion                            "TypicalSectionPortion"
#define BRRP_CLASS_TypicalSectionPortionElement                     "TypicalSectionPortionElement"


// Aspects
#define BRRP_CLASS_StatusAspect                                     "StatusAspect"


// Relationships
#define BRRP_REL_PathwayAssemblesElements                           "PathwayAssemblesElements"
#define BRRP_REL_PathwayRefersToMainAlignment                       "PathwayRefersToMainAlignment"
#define BRRP_REL_PhysicalElementOwnsLinearlyLocatedStatus           "PhysicalElementOwnsLinearlyLocatedStatus"
#define BRRP_REL_RegularSegmentRefersToTravelwayDefinition          "RegularSegmentRefersToTravelwayDefinition"
#define BRRP_REL_RoadClassRefersToDefinition                        "RoadClassRefersToDefinition"
#define BRRP_REL_RoadDesignSpeedRefersToDefinition                  "RoadDesignSpeedRefersToDefinition"
#define BRRP_REL_RoadwayOwnsDesignSpeeds                            "RoadwayOwnsDesignSpeeds"
#define BRRP_REL_RoadwayOwnsRoadClasses                             "RoadwayOwnsRoadClasses"


// Properties
#define BRRP_PROP_StatusAspect_Status                               "Status"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BRRP_CATEGORY_Track                                         "Track"
#define BRRP_CATEGORY_Road                                          "Road"


//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------
#define BRRP_CODESPEC_RoadClassDefinition                          "RoadClassDefinition"
#define BRRP_CODESPEC_RoadClassDefinitionTable                     "RoadClassDefinitionTable"
#define BRRP_CODESPEC_RoadDesignSpeedDefinition                    "RoadDesignSpeedDefinition"
#define BRRP_CODESPEC_RoadDesignSpeedDefinitionTable               "RoadDesignSpeedDefinitionTable"
#define BRRP_CODESPEC_RoadTravelway                                "RoadTravelway"

#define BRRP_CODESPEC_RoadTypicalSection                           "RoadTypicalSection"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    ROADRAILPHYSICAL_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }   

#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(__name__) \
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); }
    

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE 

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------

// Road & Rail shared
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(AlignmentIntersectionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(EndConditionDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(EndConditionDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(LinearlyLocatedStatus)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(PathwayElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RegularTravelwaySegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRailCategoryModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayDefinitionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayIntersectionSegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwaySegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayTransition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPortionBreakDownModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPortion)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPortionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(StatusAspect)

// Road-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadTypicalSection)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinitionTable)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinitionTableModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassStandards)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeedDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeedDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeedDefinitionTable)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeedDefinitionTableModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeedStandards)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadIntersectionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadIntersectionLegElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClass)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeed)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRailCategoryModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadTravelwayDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Roadway)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadwayStandardsModel)

// Rail-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Railway)


//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"
#include "RoadRailPhysicalDomain.h"
#include "ElementAspects.h"
#include "RoadDesignSpeed.h"
#include "RoadClass.h"
#include "Pathway.h"
#include "TypicalSection.h"
#include "TravelwaySegment.h"
#include "RoadSegment.h"