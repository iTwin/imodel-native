/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "FormsDomainDefinitions.h"
#include <DgnPlatform/DgnCoreAPI.h>
#include "Form.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE StraightProfiledExtrusion : Form
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_StraightProfiledExtrusion, Form)

    friend struct StraightProfiledExtrusionHandler;
protected:
    StraightProfiledExtrusion();
public:
    FORMS_DOMAIN_EXPORT static StraightProfiledExtrusionPtr Create() { return new StraightProfiledExtrusion(); };
    FORMS_DOMAIN_EXPORT static StraightProfiledExtrusionCP GetAspect(Dgn::DgnElementCR);
    FORMS_DOMAIN_EXPORT static StraightProfiledExtrusionP GetAspectP(Dgn::DgnElementR);
    FORMS_DOMAIN_EXPORT static ECN::ECClassCP GetECClass(Dgn::DgnDbR);

private:
    BE_PROP_NAME(Length)
private:
    double m_Length;
    ProfileId m_profileId;
public:
    FORMS_DOMAIN_EXPORT void SetLength(double length);
    double GetLength() const { return m_Length; };
    ProfileId GetProfileId() const { return m_profileId; }
    void SetProfileId(ProfileId profileId) { m_profileId = profileId; }
    FORMS_DOMAIN_EXPORT void SetProfile(Dgn::DgnElementCR profile);
protected:
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE StraightProfiledExtrusionHandler : FormHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(FORMS_CLASS_StraightProfiledExtrusion, StraightProfiledExtrusionHandler, FormHandler, FORMS_DOMAIN_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new StraightProfiledExtrusion(); };
    };

END_BENTLEY_FORMS_NAMESPACE
