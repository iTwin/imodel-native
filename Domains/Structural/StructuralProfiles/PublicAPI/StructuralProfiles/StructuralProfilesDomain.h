/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralProfiles/PublicAPI/StructuralProfiles/StructuralProfilesDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralProfilesApi.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Structural Profiles schema.
//! @ingroup GROUP_StructuralProfiles
//=======================================================================================
struct StructuralProfilesDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(StructuralProfilesDomain, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STRUCTURAL_PROFILES_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
        static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

    public:
        StructuralProfilesDomain();
        STRUCTURAL_DOMAIN_EXPORT static Dgn::CodeSpecId QueryStructuralProfilesCodeSpecId(Dgn::DgnDbCR dgndb);
        STRUCTURAL_DOMAIN_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

END_BENTLEY_STRUCTURAL_NAMESPACE

