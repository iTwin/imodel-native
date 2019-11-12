/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_RASTER_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace Raster {
#define END_BENTLEY_RASTER_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_RASTER       using namespace BentleyApi::Raster;

#define RASTER_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_RASTER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_RASTER_NAMESPACE

#define RASTER_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_RASTER_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_RASTER_NAMESPACE

#define RASTERMODELHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
        private: virtual Dgn::DgnModel* _CreateInstance(Dgn::DgnModel::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

//-----------------------------------------------------------------------------------------
// ECClass name
//-----------------------------------------------------------------------------------------
#define RASTER_CLASSNAME_RasterModel        "RasterModel"
#define RASTER_CLASSNAME_WmsModel           "WmsModel"
#define RASTER_CLASSNAME_RasterFileModel    "RasterFileModel"

