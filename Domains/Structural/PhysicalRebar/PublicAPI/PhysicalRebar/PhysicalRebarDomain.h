/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! The DgnDomain for the PhysicalRebar schema.
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct PhysicalRebarDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(PhysicalRebarDomain, PHYSICALREBAR_EXPORT)

public:
    //! @private
    PhysicalRebarDomain();

private:
    WCharCP _GetSchemaRelativePath() const override { return SPR_SCHEMA_PATH; }
}; // PhysicalRebarDomain

END_BENTLEY_PHYSICALREBAR_NAMESPACE
