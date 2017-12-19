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
        CurvedExtrusion() {};
    public:
        FORMS_DOMAIN_EXPORT static CurvedExtrusionPtr Create() { return new CurvedExtrusion(); };
   
    private:
        BE_PROP_NAME(StartShape)
        BE_PROP_NAME(EndShape)
        BE_PROP_NAME(Curve)

    public:
        void SetStartShape(IGeometryPtr shape) { ECN::ECValue shapeGeometry; shapeGeometry.SetIGeometry(*shape); SetPropertyValue(prop_StartShape(), shapeGeometry); };
        void SetEndShape(IGeometryPtr shape) { ECN::ECValue shapeGeometry; shapeGeometry.SetIGeometry(*shape); SetPropertyValue(prop_EndShape(), shapeGeometry); };
        void SetCurve(IGeometryPtr shape) { ECN::ECValue curveGeometry; curveGeometry.SetIGeometry(*shape); SetPropertyValue(prop_Curve(), curveGeometry); };

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
