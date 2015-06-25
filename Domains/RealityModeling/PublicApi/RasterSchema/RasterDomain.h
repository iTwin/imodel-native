/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnCore/DgnDomain.h>
#include <RasterSchema/RasterSchemaCommon.h>

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//=======================================================================================
//! The DgnDomain for the raster schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct RasterDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(RasterDomain, RASTERSCHEMA_EXPORT)

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

public:
    RasterDomain();
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
