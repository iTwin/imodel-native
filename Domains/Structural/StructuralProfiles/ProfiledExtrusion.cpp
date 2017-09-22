#include <StructuralDomain/StructuralProfiles/ProfiledExtrusion.h>

USING_NAMESPACE_BENTLEY_STRUCTURAL

HANDLER_DEFINE_MEMBERS(ProfiledExtrusionHandler)

Dgn::DgnDbStatus ProfiledExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus ProfiledExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }