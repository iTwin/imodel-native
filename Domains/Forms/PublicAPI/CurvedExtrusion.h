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

struct EXPORT_VTABLE_ATTRIBUTE CurvedExtrusion : Form
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_CurvedExtrusion, Form)

        friend struct  CurvedExtrusionHandler;
    protected:
        CurvedExtrusion();
    public:
        FORMS_DOMAIN_EXPORT static CurvedExtrusionPtr Create() { return new CurvedExtrusion(); };
        FORMS_DOMAIN_EXPORT static CurvedExtrusionCP GetAspect(Dgn::DgnElementCR);
        FORMS_DOMAIN_EXPORT static CurvedExtrusionP GetAspectP(Dgn::DgnElementR);
        FORMS_DOMAIN_EXPORT static ECN::ECClassCP GetECClass(Dgn::DgnDbR);

    private:
        BE_PROP_NAME(StartShape)
        BE_PROP_NAME(EndShape)
        BE_PROP_NAME(Curve)

        IGeometryPtr m_startShape;
        IGeometryPtr m_endShape;
        IGeometryPtr m_Curve;

    public:
        FORMS_DOMAIN_EXPORT void SetStartShape(IGeometryPtr shape);
        FORMS_DOMAIN_EXPORT void SetEndShape(IGeometryPtr shape);
        FORMS_DOMAIN_EXPORT void SetCurve(IGeometryPtr curve);

        const IGeometryPtr GetStartShape() const { return m_startShape; };
        const IGeometryPtr GetEndShape() const { return m_endShape; };
        const IGeometryPtr GetCurve() const { return m_Curve; };

    protected:
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
        EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
        Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
        Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE CurvedExtrusionHandler : FormHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(FORMS_CLASS_CurvedExtrusion, CurvedExtrusionHandler, FormHandler, FORMS_DOMAIN_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new CurvedExtrusion(); };
    };

END_BENTLEY_FORMS_NAMESPACE
