#include "PublicAPI\FormsDomain\CurvedExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE
HANDLER_DEFINE_MEMBERS(CurvedExtrusionHandler)

Dgn::DgnDbStatus CurvedExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus CurvedExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }
END_BENTLEY_FORMS_NAMESPACE