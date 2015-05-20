/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/BePointCloudCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE    BEGIN_BENTLEY_API_NAMESPACE namespace BePointCloud {
#define END_BENTLEY_BEPOINTCLOUD_NAMESPACE      } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_BEPOINTCLOUD    using namespace BentleyApi::BePointCloud;

#define BEPOINTCLOUD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE struct _name_; END_BENTLEY_BEPOINTCLOUD_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS(BePointCloud,_name_)

