/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/LinearReferencingApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/Nullable.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

#ifdef __LINEARREFERENCING_BUILD__
#define LINEARREFERENCING_EXPORT EXPORT_ATTRIBUTE
#else
#define LINEARREFERENCING_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_LINEARREFERENCING    "LinearReferencing"
#if defined (ANDROID)
#include <android/log.h>
#define DGNCLIENTFX_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define LINEARREFERENCING_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_LINEARREFERENCING))
#define LINEARREFERENCING_LOGD(...)           LINEARREFERENCING_LOG.debugv (__VA_ARGS__);
#define LINEARREFERENCING_LOGI(...)           LINEARREFERENCING_LOG.infov (__VA_ARGS__);
#define LINEARREFERENCING_LOGW(...)           LINEARREFERENCING_LOG.warningv (__VA_ARGS__);
#define LINEARREFERENCING_LOGE(...)           LINEARREFERENCING_LOG.errorv (__VA_ARGS__);
#define LINEARREFERENCING_ASSERT_LOGD(...)    BeAssert(!__VA_ARGS__); LINEARREFERENCING_LOGD(__VA_ARGS__);
#define LINEARREFERENCING_ASSERT_LOGI(...)    BeAssert(!__VA_ARGS__); LINEARREFERENCING_LOGI(__VA_ARGS__);
#define LINEARREFERENCING_ASSERT_LOGW(...)    BeAssert(!__VA_ARGS__); LINEARREFERENCING_LOGW(__VA_ARGS__);   
#define LINEARREFERENCING_ASSERT_LOGE(...)    BeAssert(!__VA_ARGS__); LINEARREFERENCING_LOGE(__VA_ARGS__);   
#endif

/** @namespace BentleyApi::LinearReferencing %LinearReferencing data types */
#define BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace LinearReferencing {
#define END_BENTLEY_LINEARREFERENCING_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_LINEARREFERENCING        using namespace BENTLEY_NAMESPACE_NAME::LinearReferencing;

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE
END_BENTLEY_LINEARREFERENCING_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BLR_SCHEMA_NAME                             "LinearReferencing"
#define BLR_SCHEMA_FILE                             L"LinearReferencing.01.00.00.ecschema.xml"
#define BLR_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BLR_SCHEMA(name)                            BLR_SCHEMA_NAME "." name
#define BLR_SCHEMA_CODE(name)                       BLR_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define BLR_CLASS_ILinearElement                                    "ILinearElement"
#define BLR_CLASS_ILinearlyLocated                                  "ILinearlyLocated"
#define BLR_CLASS_LinearlyReferencedAtLocation                      "LinearlyReferencedAtLocation"
#define BLR_CLASS_LinearlyReferencedFromToLocation                  "LinearlyReferencedFromToLocation"
#define BLR_CLASS_LinearlyReferencedLocation                        "LinearlyReferencedLocation"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(BLR_SCHEMA_NAME, BLR_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetECClass(BLR_SCHEMA_NAME, BLR_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the Conceptual namespace
//-----------------------------------------------------------------------------------------
#define LINEARREFERENCING_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_LINEARREFERENCING_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define LINEARREFERENCING_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_LINEARREFERENCING_NAMESPACE 


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the LinearReferencing namespace
//-----------------------------------------------------------------------------------------
LINEARREFERENCING_TYPEDEFS(DistanceExpression)
LINEARREFERENCING_TYPEDEFS(ILinearElement)
LINEARREFERENCING_TYPEDEFS(ILinearlyLocated)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedLocation)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedAtLocation)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedFromToLocation)

LINEARREFERENCING_REFCOUNTED_PTR(LinearlyReferencedAtLocation)
LINEARREFERENCING_REFCOUNTED_PTR(LinearlyReferencedFromToLocation)


//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "DistanceExpression.h"
#include "LinearlyReferencedLocation.h"
#include "ILinearElement.h"
#include "LinearReferencingDomain.h"