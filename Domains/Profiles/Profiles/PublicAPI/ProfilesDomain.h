/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ProfilesDomain.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Profiles schema.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ProfilesDomain : Dgn::DgnDomain
    {
DOMAIN_DECLARE_MEMBERS (ProfilesDomain, PROFILES_EXPORT)

public:
    //! @private
    ProfilesDomain();

private:
    WCharCP _GetSchemaRelativePath() const override { return PRF_SCHEMA_PATH; }
    }; // ProfilesDomain

END_BENTLEY_PROFILES_NAMESPACE
