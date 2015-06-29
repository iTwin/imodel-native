/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudSchemaCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace PointCloudSchema {
#define END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA       using namespace BentleyApi::PointCloudSchema;

#define POINTCLOUDSCHEMA_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
