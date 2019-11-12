/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace ThreeMxSchema {
#define END_BENTLEY_THREEMX_SCHEMA_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA       using namespace BentleyApi::ThreeMxSchema;

#define THREEMXSCHEMA_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_THREEMX_SCHEMA_NAMESPACE

#if defined (__THREEMXREADER_BUILD__)
#   define THREEMX_READER_EXPORT      EXPORT_ATTRIBUTE
#else
#   define THREEMX_READER_EXPORT      IMPORT_ATTRIBUTE
#endif

