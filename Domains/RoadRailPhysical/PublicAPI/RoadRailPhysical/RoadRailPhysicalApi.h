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
#include <Costing/CostingApi.h>
#include <BridgePhysical/BridgePhysicalApi.h>

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
#define BRRP_SCHEMA_FILE                             L"RoadRailPhysical.01.00.00.ecschema.xml"
#define BRRP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRP_SCHEMA(name)                            BRRP_SCHEMA_NAME "." name
#define BRRP_SCHEMA_CODE(name)                       BRRP_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Models
#define BRRP_CLASS_CrossSectionBreakDownModel                       "CrossSectionBreakDownModel"
#define BRRP_CLASS_CrossSectionDefinitionModel                      "CrossSectionDefinitionModel"
#define BRRP_CLASS_RoadClassDefinitionModel                         "RoadClassDefinitionModel"
#define BRRP_CLASS_RoadClassDefinitionTableModel                    "RoadClassDefinitionTableModel"
#define BRRP_CLASS_RoadDesignSpeedDefinitionModel                   "RoadDesignSpeedDefinitionModel"
#define BRRP_CLASS_RoadDesignSpeedDefinitionTableModel              "RoadDesignSpeedDefinitionTableModel"
#define BRRP_CLASS_RoadwayStandardsModel                            "RoadwayStandardsModel"


// Elements
#define BRRP_CLASS_CrossSectionElement                              "CrossSectionElement"
#define BRRP_CLASS_IntersectionElement                              "IntersectionElement"
#define BRRP_CLASS_ElevatedRoadIntersection                         "ElevatedRoadIntersection"
#define BRRP_CLASS_ElevatedRoadIntersectionSegment                  "ElevatedRoadIntersectionSegment"
#define BRRP_CLASS_ElevatedRoadSegment                              "ElevatedRoadSegment"
#define BRRP_CLASS_IntersectionSegment                              "IntersectionSegment"
#define BRRP_CLASS_IntersectionSegmentElement                       "IntersectionSegmentElement"
#define BRRP_CLASS_RailRange                                        "RailRange"
#define BRRP_CLASS_RegularSegmentElement                            "RegularSegmentElement"
#define BRRP_CLASS_RoadCrossSection                                 "RoadCrossSection"
#define BRRP_CLASS_RoadClass                                        "RoadClass"
#define BRRP_CLASS_RoadClassDefinition                              "RoadClassDefinition"
#define BRRP_CLASS_RoadClassDefinitionTable                         "RoadClassDefinitionTable"
#define BRRP_CLASS_RoadClassStandards                               "RoadClassStandards"
#define BRRP_CLASS_RoadDesignSpeed                                  "RoadDesignSpeed"
#define BRRP_CLASS_RoadDesignSpeedDefinition                        "RoadDesignSpeedDefinition"
#define BRRP_CLASS_RoadDesignSpeedDefinitionTable                   "RoadDesignSpeedDefinitionTable"
#define BRRP_CLASS_RoadDesignSpeedStandards                         "RoadDesignSpeedStandards"
#define BRRP_CLASS_RoadIntersection                                 "RoadIntersection"
#define BRRP_CLASS_RoadIntersectionSegment                          "RoadIntersectionSegment"
#define BRRP_CLASS_RoadRange                                        "RoadRange"
#define BRRP_CLASS_RoadSegment                                      "RoadSegment"
#define BRRP_CLASS_RoadTransitionSegment                            "RoadTransitionSegment"
#define BRRP_CLASS_SegmentElement                                   "SegmentElement"
#define BRRP_CLASS_SegmentRangeElement                              "SegmentRangeElement"
#define BRRP_CLASS_TransitionSegmentElement                         "TransitionSegmentElement"


// Aspects
#define BRRP_CLASS_StatusAspect                                     "StatusAspect"


// Relationships
#define BRRP_REL_SegmentRangeOwnsSegments                           "SegmentRangeOwnsSegments"
#define BRRP_REL_SegmentRangeRefersToAlignment                      "SegmentRangeRefersToAlignment"
#define BRRP_REL_RoadClassRefersToDefinition                        "RoadClassRefersToDefinition"
#define BRRP_REL_RoadDesignSpeedRefersToDefinition                  "RoadDesignSpeedRefersToDefinition"
#define BRRP_REL_RoadSegmentRefersToCrossSection                    "RoadSegmentRefersToCrossSection"
#define BRRP_REL_RoadRangeHasDesignSpeeds                           "RoadRangeHasDesignSpeeds"
#define BRRP_REL_RoadRangeHasRoadClasses                            "RoadRangeHasRoadClasses"
#define BRRP_REL_SegmentRangeAssemblesSegments                      "SegmentRangeAssemblesSegments"


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
#define BRRP_CODESPEC_RoadCrossSection                             "RoadCrossSection"
#define BRRP_CODESPEC_RoadClassDefinition                          "RoadClassDefinition"
#define BRRP_CODESPEC_RoadClassDefinitionTable                     "RoadClassDefinitionTable"
#define BRRP_CODESPEC_RoadDesignSpeedDefinition                    "RoadDesignSpeedDefinition"
#define BRRP_CODESPEC_RoadDesignSpeedDefinitionTable               "RoadDesignSpeedDefinitionTable"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetECClass(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); }


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
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(CrossSectionBreakDownModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(CrossSectionDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(CrossSectionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(IntersectionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(IntersectionSegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RegularSegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(SegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(SegmentRangeElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(StatusAspect)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TransitionSegmentElement)

// Road-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(ElevatedRoadIntersection)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(ElevatedRoadIntersectionSegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(ElevatedRoadSegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadCrossSection)
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
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadIntersection)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadIntersectionSegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RailRange)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClass)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeed)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRange)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadSegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadTransitionSegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadwayStandardsModel)


//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"
#include "RoadRailPhysicalDomain.h"
#include "ElementAspects.h"
#include "LinearReferencing.h"
#include "RoadDesignSpeed.h"
#include "RoadClass.h"
#include "SegmentRange.h"
#include "CrossSection.h"
#include "Segment.h"
#include "RoadSegment.h"