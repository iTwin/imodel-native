/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnDomain.h>
#include "QuantityTakeoffsAspectsMacros.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Quantity Takeoffs Aspects schema.
//! @private
//=======================================================================================
struct QuantityTakeoffsAspectsDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(QuantityTakeoffsAspectsDomain, EXPORT_ATTRIBUTE)

private:
    WCharCP _GetSchemaRelativePath () const override { return L"ECSchemas/Domain/QuantityTakeoffsAspects.ecschema.xml"; }

public:
    QuantityTakeoffsAspectsDomain();
    ~QuantityTakeoffsAspectsDomain() {};
};

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
