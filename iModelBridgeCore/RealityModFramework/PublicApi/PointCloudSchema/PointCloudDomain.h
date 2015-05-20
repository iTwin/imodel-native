/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

//=======================================================================================
//! The DgnDomain for the point cloud schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct PointCloudDomain : DgnPlatform::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(PointCloudDomain, POINTCLOUDSCHEMA_EXPORT)

protected:
    virtual void _OnSchemaImported(DgnDbR) const override;

public:
    PointCloudDomain();
};

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
