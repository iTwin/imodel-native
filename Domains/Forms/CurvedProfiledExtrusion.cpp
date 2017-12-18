#include "FormsDomain\CurvedProfiledExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE
HANDLER_DEFINE_MEMBERS(CurvedProfiledExtrusionHandler)

Dgn::DgnDbStatus CurvedProfiledExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus CurvedProfiledExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }
END_BENTLEY_FORMS_NAMESPACE