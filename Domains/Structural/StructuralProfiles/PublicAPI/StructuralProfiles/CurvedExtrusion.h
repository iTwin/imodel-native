#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralProfilesDefinitions.h"
#include <DgnPlatform/DgnCoreAPI.h>
#include "ProfiledExtrusion.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE CurvedExtrusion : ProfiledExtrusion
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_COMMON_SCHEMA_NAME, STRUCTURAL_COMMON_CLASS_CurvedExtrusion, ProfiledExtrusion)

        friend struct  CurvedExtrusionHandler;
    protected:
        CurvedExtrusion() {};
    public:
        STRUCTURAL_DOMAIN_EXPORT static CurvedExtrusionPtr Create() { return new CurvedExtrusion(); };
    protected:
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
        Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
        Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE CurvedExtrusionHandler : ProfiledExtrusionHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_COMMON_CLASS_CurvedExtrusion, CurvedExtrusionHandler, ProfiledExtrusionHandler, STRUCTURAL_DOMAIN_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new CurvedExtrusion(); };
    };


END_BENTLEY_STRUCTURAL_NAMESPACE
