/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>

#define BEGIN_BENTLEY_IMODELJS_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace iModelJs {
#define END_BENTLEY_IMODELJS_NAMESPACE    } END_BENTLEY_NAMESPACE
#define USING_IMODELJS_NAMESPACE          using namespace BentleyApi::iModelJs;

#ifdef __IMODELJS_BUILD__
    #define IMODELJS_EXPORT EXPORT_ATTRIBUTE
#else
    #define IMODELJS_EXPORT IMPORT_ATTRIBUTE
#endif

#define IMODELJS_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_NAMESPACE

#include <Napi/napi.h>

#include <iModelJs/iModelJsUtilities.h>

//__PUBLISH_SECTION_END__
