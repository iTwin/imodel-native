/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ScalableMeshSchemaCommon.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace ScalableMeshSchema {
#define END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA       using namespace BentleyApi::ScalableMeshSchema;

#define SCALABLEMESH_SCHEMA_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

#define SCALABLEMESH_SCHEMA_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE