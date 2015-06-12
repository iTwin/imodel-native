/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//=======================================================================================
//! The DgnDomain for the raster schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct RasterDomain : DgnPlatform::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(RasterDomain, RASTERSCHEMA_EXPORT)

protected:
    virtual void _OnSchemaImported(DgnDbR) const override;

public:
    RasterDomain();
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
