/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterSchemaCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE       BEGIN_BENTLEY_API_NAMESPACE namespace RasterSchema {
#define END_BENTLEY_RASTERSCHEMA_NAMESPACE         } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_RASTERSCHEMA       using namespace BentleyApi::RasterSchema;

#define RASTERSCHEMA_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE struct _name_; END_BENTLEY_RASTERSCHEMA_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS(RasterSchema,_name_)

#define RASTERSCHEMA_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_RASTERSCHEMA_NAMESPACE

//&&MM add missing 'DgnPlatform::' in MODELHANDLER_DECLARE_MEMBERS(DgnModel.h)? 
#define RASTERMODELHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
        private: virtual DgnPlatform::DgnModel* _CreateInstance(DgnPlatform::DgnModel::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)


//-----------------------------------------------------------------------------------------
// ECClass name
//-----------------------------------------------------------------------------------------
#define RASTER_CLASSNAME_RasterModel        "RasterModel"
#define RASTER_CLASSNAME_WmsModel           "WmsModel"

