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
#define BRRP_SCHEMA_FILE                             L"RoadRailPhysical.01.00.00.ecschema.xml"
#define BRRP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRP_SCHEMA(name)                            BRRP_SCHEMA_NAME "." name
#define BRRP_SCHEMA_CODE(name)                       BRRP_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Elements
#define BRRP_CLASS_SegmentRangeElement                              "SegmentRangeElement"


// Relationships
#define BRRP_REL_SegmentRangeOwnsSegments                           "SegmentRangeOwnsSegments"


// Properties
#define BRRP_PROP_Alignment_MainVerticalAlignment                   "MainVerticalAlignment"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BRRP_CATEGORY_Alignment                                     "Alignment"


//-----------------------------------------------------------------------------------------
// Authority names
//-----------------------------------------------------------------------------------------
#define BRRP_AUTHORITY_Alignment                                    "Alignment"

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetECClass(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    ROADRAILPHYSICAL_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); } \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); } \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }   


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
// Define typedefs and Ptrs in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------
ROADRAILPHYSICAL_TYPEDEFS(SegmentRange)


//ROADRAILPHYSICAL_REFCOUNTED_PTR(Alignment)



//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "RoadRailPhysicalDomain.h"
#include "SegmentRange.h"