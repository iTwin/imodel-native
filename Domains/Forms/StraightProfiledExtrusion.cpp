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

END_BENTLEY_FORMS_NAMESPACE