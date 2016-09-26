/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloud/PointCloudDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>
#include <PointCloud/PointCloudCommon.h>

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

//=======================================================================================
//! The DgnDomain for the point cloud schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct PointCloudDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(PointCloudDomain, POINTCLOUD_EXPORT)

    static  void        InitializeApi();

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

public:
    PointCloudDomain();
};

END_BENTLEY_POINTCLOUD_NAMESPACE
