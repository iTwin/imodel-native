/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/LinearReferencingDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "LinearReferencing.h"
#include "ILinearElement.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! The DgnDomain for the LinearReferencing schema.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearReferencingDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(LinearReferencingDomain, LINEARREFERENCING_EXPORT)

public:
    LinearReferencingDomain();

private:
    WCharCP _GetSchemaRelativePath() const override { return BLR_SCHEMA_PATH; }
}; // LinearReferencingDomain

END_BENTLEY_LINEARREFERENCING_NAMESPACE
