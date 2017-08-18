#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralProfilesDefinitions.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileZone : Dgn::DgnElement::MultiAspect
{
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_VaryingProfile, Dgn::DgnElement::MultiAspect);

    friend struct  VaryingProfileZoneHandler;

private:
    VaryingProfileZone() {};

protected:
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
};

struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileZoneHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_BuiltUpProfileComponent, VaryingProfileZoneHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new VaryingProfileZone(); }
};

END_BENTLEY_STRUCTURAL_NAMESPACE