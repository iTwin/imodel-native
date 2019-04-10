/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/BePointCloudCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace BePointCloud {
#define END_BENTLEY_BEPOINTCLOUD_NAMESPACE      } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_BEPOINTCLOUD    using namespace BentleyApi::BePointCloud;

#define BEPOINTCLOUD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_BEPOINTCLOUD_NAMESPACE

