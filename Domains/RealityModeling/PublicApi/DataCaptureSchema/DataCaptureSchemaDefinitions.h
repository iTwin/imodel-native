/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>

#include <DgnPlatform/DgnGeoCoord.h>
#include <Geom/GeomApi.h>

#ifdef __DATACAPTURE_BUILD__
#define DATACAPTURE_EXPORT EXPORT_ATTRIBUTE
#else
#define DATACAPTURE_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_DATACAPTURE    "DataCapture"
#if defined (ANDROID)
#include <android/log.h>
#define DGNCLIENTFX_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#else
#include <Logging/bentleylogging.h>
#define DATACAPTURE_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_DATACAPTURE))
#define DATACAPTURE_LOGD(...)           DATACAPTURE_LOG.debugv (__VA_ARGS__);
#define DATACAPTURE_LOGI(...)           DATACAPTURE_LOG.infov (__VA_ARGS__);
#define DATACAPTURE_LOGW(...)           DATACAPTURE_LOG.warningv (__VA_ARGS__);
#define DATACAPTURE_LOGE(...)           DATACAPTURE_LOG.errorv (__VA_ARGS__);
#define DATACAPTURE_ASSERT_LOGD(...)    BeAssert(!__VA_ARGS__); DATACAPTURE_LOGD(__VA_ARGS__);
#define DATACAPTURE_ASSERT_LOGI(...)    BeAssert(!__VA_ARGS__); DATACAPTURE_LOGI(__VA_ARGS__);
#define DATACAPTURE_ASSERT_LOGW(...)    BeAssert(!__VA_ARGS__); DATACAPTURE_LOGW(__VA_ARGS__);   
#define DATACAPTURE_ASSERT_LOGE(...)    BeAssert(!__VA_ARGS__); DATACAPTURE_LOGE(__VA_ARGS__);   
#endif

/** @namespace BentleyApi::DataCapture %DataCapture data types */
#define BEGIN_BENTLEY_DATACAPTURE_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace DataCapture {
#define END_BENTLEY_DATACAPTURE_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DATACAPTURE        using namespace BENTLEY_NAMESPACE_NAME::DataCapture;

// create the Bentley.DataCapture namespace
BEGIN_BENTLEY_DATACAPTURE_NAMESPACE
END_BENTLEY_DATACAPTURE_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BDCP_SCHEMA_NAME                             "DataCapture"
#define BDCP_SCHEMA_FILE                             L"DataCapture.01.00.00.ecschema.xml"
#define BDCP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BDCP_SCHEMA(name)                            BDCP_SCHEMA_NAME "." name
#define BDCP_SCHEMA_CODE(name)                       BDCP_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Elements
#define BDCP_CLASS_CameraDeviceModel                             "CameraDeviceModel"
#define BDCP_CLASS_CameraDevice                                  "CameraDevice"
#define BDCP_CLASS_Shot                                          "Shot"
#define BDCP_CLASS_Pose                                          "Pose"                                    
#define BDCP_CLASS_RadialDistortion                              "RadialDistortion"
#define BDCP_CLASS_TangentialDistortion                          "TangentialDistortion"
#define BDCP_CLASS_GimbalAngleRange                              "GimbalAngleRange"                                    
#define BDCP_CLASS_Gimbal                                        "Gimbal"                                    
#define BDCP_CLASS_Drone                                         "Drone"                                    

// Relationships
#define BDCP_REL_CameraDeviceIsDefinedByCameraDeviceModel            "CameraDeviceIsDefinedByCameraDeviceModel"
#define BDCP_REL_ShotIsTakenByCameraDevice                           "ShotIsTakenByCameraDevice"
#define BDCP_REL_ShotIsTakenAtPose                                   "ShotIsTakenAtPose"
#define BDCP_REL_GimbalHasGimbalAngleRanges                          "GimbalHasGimbalAngleRanges"
#define BDCP_REL_GimbalHasCameras                                    "GimbalHasCameras"
#define BDCP_REL_DroneHasGimbal                                      "DroneHasGimbal"

//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BDCP_CATEGORY_AcquisitionDevice                                    "AcquisitionDevice"
#define BDCP_CATEGORY_Shot                                                 "Shot"

//-----------------------------------------------------------------------------------------
// Authority names
//-----------------------------------------------------------------------------------------
#define BDCP_AUTHORITY_DataCapture                                   "DataCapture"

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_DATACAPTURE_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BDCP_SCHEMA_NAME, BDCP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BDCP_SCHEMA_NAME, BDCP_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(__name__) \
    DATACAPTURE_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    DATACAPTURE_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); } \
    DATACAPTURE_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); } \
    DATACAPTURE_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }   


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the DataCapture namespace
//-----------------------------------------------------------------------------------------
#define DATACAPTURE_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_DATACAPTURE_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_DATACAPTURE_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define DATACAPTURE_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_DATACAPTURE_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_DATACAPTURE_NAMESPACE 


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the DataCapture namespace
//-----------------------------------------------------------------------------------------
DATACAPTURE_TYPEDEFS(CameraDevice)
DATACAPTURE_TYPEDEFS(CameraDeviceModel)
DATACAPTURE_TYPEDEFS(RadialDistortion)
DATACAPTURE_TYPEDEFS(TangentialDistortion)
DATACAPTURE_TYPEDEFS(Shot)
DATACAPTURE_TYPEDEFS(Pose)
DATACAPTURE_TYPEDEFS(GimbalAngleRange)
DATACAPTURE_TYPEDEFS(Gimbal)
DATACAPTURE_TYPEDEFS(Drone)

DATACAPTURE_REFCOUNTED_PTR(RadialDistortion)
DATACAPTURE_REFCOUNTED_PTR(TangentialDistortion)
DATACAPTURE_REFCOUNTED_PTR(CameraDeviceModel)
DATACAPTURE_REFCOUNTED_PTR(CameraDevice)
DATACAPTURE_REFCOUNTED_PTR(Shot)
DATACAPTURE_REFCOUNTED_PTR(Pose)
DATACAPTURE_REFCOUNTED_PTR(GimbalAngleRange)
DATACAPTURE_REFCOUNTED_PTR(Gimbal)
DATACAPTURE_REFCOUNTED_PTR(Drone)

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

BEBRIEFCASEBASED_ID_SUBCLASS(CameraDeviceModelElementId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(CameraDeviceElementId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(ShotElementId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(PoseElementId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(GimbalAngleRangeElementId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(GimbalElementId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(DroneElementId, Dgn::DgnElementId)

/**
@addtogroup DataCaptureGroup DataCapture
*/


END_BENTLEY_DATACAPTURE_NAMESPACE
