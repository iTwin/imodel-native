/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/LinearReferencingDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

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
}; // LinearReferencingDomain

END_BENTLEY_LINEARREFERENCING_NAMESPACE
