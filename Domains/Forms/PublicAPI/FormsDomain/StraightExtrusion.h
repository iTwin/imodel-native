#pragma once

//__PUBLISH_SECTION_START__
#include "FormsDomainDefinitions.h"
#include <DgnPlatform/DgnCoreAPI.h>
#include "ProfiledExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE StraightExtrusion : ProfiledExtrusion
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_StraightExtrusion, ProfiledExtrusion)

    friend struct  StraightExtrusionHandler;
protected:
    StraightExtrusion() {};
public:
    FORMS_DOMAIN_EXPORT static StraightExtrusionPtr Create() { return new StraightExtrusion(); };
protected:
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE StraightExtrusionHandler : ProfiledExtrusionHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(FORMS_CLASS_StraightExtrusion, StraightExtrusionHandler, ProfiledExtrusionHandler, FORMS_DOMAIN_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new StraightExtrusion(); };
    };

END_BENTLEY_FORMS_NAMESPACE
