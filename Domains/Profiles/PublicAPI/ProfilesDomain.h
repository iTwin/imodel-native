#pragma once

//__PUBLISH_SECTION_START__
#include "ProfilesDomainDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct ProfilesDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(ProfilesDomain, PROFILES_DOMAIN_EXPORT)

protected:
    WCharCP _GetSchemaRelativePath() const override { return BENTLEY_PROFILES_SCHEMA_PATH; };
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
    virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
    static void InsertDomainCodeSpecs(Dgn::DgnDbR db);
public:
    ProfilesDomain();
    PROFILES_DOMAIN_EXPORT static Dgn::CodeSpecId QueryStructuralCommonCodeSpecId(Dgn::DgnDbCR dgndb);
    PROFILES_DOMAIN_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

END_BENTLEY_PROFILES_NAMESPACE