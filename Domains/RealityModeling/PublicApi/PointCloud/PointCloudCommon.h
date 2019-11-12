/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define BEGIN_BENTLEY_POINTCLOUD_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace PointCloud {
#define END_BENTLEY_POINTCLOUD_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_POINTCLOUD       using namespace BentleyApi::PointCloud;

#define POINTCLOUD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_POINTCLOUD_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_POINTCLOUD_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECClass name
//-----------------------------------------------------------------------------------------
#define POINTCLOUD_CLASSNAME_PointCloudModel        "PointCloudModel"
