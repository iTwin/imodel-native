/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "LinearReferencing.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! The DgnDomain for the LinearReferencing schema.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearReferencingDomain
{
private:
    LinearReferencingDomain() {}

public:
    static WCharCP GetSchemaRelativePath() { return BLR_SCHEMA_PATH; }
}; // LinearReferencingDomain

END_BENTLEY_LINEARREFERENCING_NAMESPACE
