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

struct EXPORT_VTABLE_ATTRIBUTE CurvedProfiledExtrusion : Form
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_CurvedProfiledExtrusion, Form)

        friend struct  CurvedProfiledExtrusionHandler;
    protected:
        CurvedProfiledExtrusion();
    public:
        FORMS_DOMAIN_EXPORT static CurvedProfiledExtrusionPtr Create() { return new CurvedProfiledExtrusion(); };
        FORMS_DOMAIN_EXPORT static CurvedProfiledExtrusionCP GetAspect(Dgn::DgnElementCR);
        FORMS_DOMAIN_EXPORT static CurvedProfiledExtrusionP GetAspectP(Dgn::DgnElementR);
        FORMS_DOMAIN_EXPORT static ECN::ECClassCP GetECClass(Dgn::DgnDbR);
    private:
        BE_PROP_NAME(Curve)
    private:
        IGeometryPtr m_Curve;
        StartProfileId m_startProfileId;
        EndProfileId m_endProfileId;
    public:
        FORMS_DOMAIN_EXPORT void SetCurve(IGeometryPtr curve);
        const IGeometryPtr GetCurve() const { return m_Curve; };
        StartProfileId GetStartProfileId() const { return m_startProfileId; }
        void SetStartProfileId(StartProfileId profileId) { m_startProfileId = profileId; }
        EndProfileId GetEndProfileId() const { return m_endProfileId; }
        void SetEndProfileId(EndProfileId profileId) { m_endProfileId = profileId; }
        FORMS_DOMAIN_EXPORT void SetStartProfile(Dgn::DgnElementCR profile);
        FORMS_DOMAIN_EXPORT void SetEndProfile(Dgn::DgnElementCR profile);
    protected:
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
        Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
        Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE CurvedProfiledExtrusionHandler : FormHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(FORMS_CLASS_CurvedProfiledExtrusion, CurvedProfiledExtrusionHandler, FormHandler, FORMS_DOMAIN_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new CurvedProfiledExtrusion(); };
    };


END_BENTLEY_FORMS_NAMESPACE
