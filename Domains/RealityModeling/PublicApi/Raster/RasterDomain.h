/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

private:
    WCharCP _GetSchemaRelativePath() const override { return L"ECSchemas/Domain/" RASTER_SCHEMA_FILE; }

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

public:
    RasterDomain();
};

END_BENTLEY_RASTER_NAMESPACE
