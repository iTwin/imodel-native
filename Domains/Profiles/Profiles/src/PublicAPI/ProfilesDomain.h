/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! @defgroup GROUP_Profiles Base Profile classes
//! Base classes of Profiles Domain.
//!
//! @defgroup GROUP_CompositeProfiles Composite Profiles
//! Profiles that are comprised of multiple SinglePerimeterProfile's.
//!
//! @defgroup GROUP_SinglePerimeterProfiles SinglePerimeter Profiles
//! Profiles that are defined by a single outer perimiter. See @ref GROUP_ParametricProfiles "Parametric Profiles" subgroup.
//!
//! @defgroup GROUP_ParametricProfiles Parametric Profiles
//! Profiles that derive from SinglePerimeterProfile and are constructed from defined parameter sets.
//! Thsee profiles guaranteed to have the center of the bounding box at (0, 0).
//!
//! @defgroup GROUP_CenterLineProfiles CenterLine Profiles
//! Profiles that are defined by a "center line".
//!
//! @defgroup GROUP_MaterialProfiles MaterialProfile Classes
//! Classes defining a Material and Profile pair.
//=======================================================================================

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
    virtual WCharCP _GetSchemaRelativePath() const override { return PRF_SCHEMA_PATH; }

    virtual void _OnSchemaImported (Dgn::DgnDb& db) const override;

    }; // ProfilesDomain

END_BENTLEY_PROFILES_NAMESPACE
