/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ScalableMeshSchemaCommon.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace ScalableMeshSchema {
#define END_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_SCALABLEMESHSCHEMA       using namespace BentleyApi::ScalableMeshSchema;

#define SCALABLEMESHSCHEMA_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE

#define SCALABLEMESHSCHEMA_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE