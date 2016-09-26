/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Raster/RasterDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>
#include <Raster/RasterCommon.h>

BEGIN_BENTLEY_RASTER_NAMESPACE

//=======================================================================================
//! The DgnDomain for the raster schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct RasterDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(RasterDomain, RASTER_EXPORT)

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

public:
    RasterDomain();
};

END_BENTLEY_RASTER_NAMESPACE
