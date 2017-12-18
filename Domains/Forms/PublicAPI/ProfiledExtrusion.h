#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnCoreAPI.h>
#include "FormsDomainDefinitions.h"
#include "Form.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ProfiledExtrusion : Form
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_ProfiledExtrusion, Form)

        friend struct  ProfiledExtrusionHandler;
    protected:
        ProfiledExtrusion() {};
    public:
        FORMS_DOMAIN_EXPORT static ProfiledExtrusionPtr Create();
    protected:
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
        Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
        Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE ProfiledExtrusionHandler : FormHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(FORMS_CLASS_ProfiledExtrusion, ProfiledExtrusionHandler, FormHandler, FORMS_DOMAIN_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new ProfiledExtrusion(); };
    };


END_BENTLEY_FORMS_NAMESPACE
