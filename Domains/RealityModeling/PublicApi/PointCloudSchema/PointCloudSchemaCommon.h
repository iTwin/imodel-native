/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudSchemaCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE       BEGIN_BENTLEY_API_NAMESPACE namespace PointCloudSchema {
#define END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE         } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA       using namespace BentleyApi::PointCloudSchema;

#define POINTCLOUDSCHEMA_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE struct _name_; END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS(PointCloudSchema,_name_)
