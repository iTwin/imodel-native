#include "PublicAPI\FormsDomain\StraightExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

HANDLER_DEFINE_MEMBERS(StraightExtrusionHandler)

Dgn::DgnDbStatus StraightExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus StraightExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }
END_BENTLEY_FORMS_NAMESPACE