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

#ifdef __ROADRAILALIGNMENT_BUILD__
#define ROADRAILALIGNMENT_EXPORT EXPORT_ATTRIBUTE
#else
#define ROADRAILALIGNMENT_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ROADRAILALIGNMENT    "RoadRailAlignment"
#if defined (ANDROID)
#include <android/log.h>
#define DGNCLIENTFX_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define ROADRAILALIGNMENT_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_ROADRAILALIGNMENT))
#define ROADRAILALIGNMENT_LOGD(...)           ROADRAILALIGNMENT_LOG.debugv (__VA_ARGS__);
#define ROADRAILALIGNMENT_LOGI(...)           ROADRAILALIGNMENT_LOG.infov (__VA_ARGS__);
#define ROADRAILALIGNMENT_LOGW(...)           ROADRAILALIGNMENT_LOG.warningv (__VA_ARGS__);
#define ROADRAILALIGNMENT_LOGE(...)           ROADRAILALIGNMENT_LOG.errorv (__VA_ARGS__);
#define ROADRAILALIGNMENT_ASSERT_LOGD(...)    BeAssert(!__VA_ARGS__); ROADRAILALIGNMENT_LOGD(__VA_ARGS__);
#define ROADRAILALIGNMENT_ASSERT_LOGI(...)    BeAssert(!__VA_ARGS__); ROADRAILALIGNMENT_LOGI(__VA_ARGS__);
#define ROADRAILALIGNMENT_ASSERT_LOGW(...)    BeAssert(!__VA_ARGS__); ROADRAILALIGNMENT_LOGW(__VA_ARGS__);   
#define ROADRAILALIGNMENT_ASSERT_LOGE(...)    BeAssert(!__VA_ARGS__); ROADRAILALIGNMENT_LOGE(__VA_ARGS__);   
#endif

/** @namespace BentleyApi::RoadRailAlignment %RoadRailAlignment data types */
#define BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace RoadRailAlignment {
#define END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT        using namespace BENTLEY_NAMESPACE_NAME::RoadRailAlignment;

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BRRA_SCHEMA_NAME                             "RoadRailAlignment"
#define BRRA_SCHEMA_FILE                             L"RoadRailAlignment.01.00.00.ecschema.xml"
#define BRRA_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRA_SCHEMA(name)                            BRRA_SCHEMA_NAME "." name
#define BRRA_SCHEMA_CODE(name)                       BRRA_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Elements
#define BRRA_CLASS_Alignment                                         "Alignment"
#define BRRA_CLASS_AlignmentHorizontal                               "AlignmentHorizontal"
#define BRRA_CLASS_AlignmentModel                                    "AlignmentModel"
#define BRRA_CLASS_AlignmentVertical                                 "AlignmentVertical"


// Relationships
#define BRRA_REL_AlignmentOwnsVerticals                              "AlignmentOwnsVerticals"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(BRRA_SCHEMA_NAME, BRRA_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetECClass(BRRA_SCHEMA_NAME, BRRA_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(__name__) \
    ROADRAILALIGNMENT_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    ROADRAILALIGNMENT_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); } \
    ROADRAILALIGNMENT_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); } \
    ROADRAILALIGNMENT_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }   


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the RoadRailAlignment namespace
//-----------------------------------------------------------------------------------------
#define ROADRAILALIGNMENT_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define ROADRAILALIGNMENT_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE 


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the RoadRailAlignment namespace
//-----------------------------------------------------------------------------------------
ROADRAILALIGNMENT_TYPEDEFS(Alignment)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentHorizontal)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentModel)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentVertical)

ROADRAILALIGNMENT_REFCOUNTED_PTR(Alignment)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentHorizontal)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentModel)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentVertical)


//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "AlignmentModel.h"
#include "Alignment.h"
#include "RoadRailAlignmentDomain.h"