#include "FormsDomain\StraightProfiledExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

HANDLER_DEFINE_MEMBERS(StraightProfiledExtrusionHandler)

Dgn::DgnDbStatus StraightProfiledExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus StraightProfiledExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

double StraightProfiledExtrusion::GetLength()
{
    ECN::ECValue value;
    Dgn::DgnDbStatus status = GetPropertyValue(value, prop_Length(), Dgn::PropertyArrayIndex());

    BeAssert(Dgn::DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);

    return value.IsNull() ? 0.0 : value.GetDouble();
};


END_BENTLEY_FORMS_NAMESPACE