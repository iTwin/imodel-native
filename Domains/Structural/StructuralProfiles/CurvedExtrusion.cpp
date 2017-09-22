#include <StructuralDomain/StructuralProfiles/CurvedExtrusion.h>

USING_NAMESPACE_BENTLEY_STRUCTURAL

HANDLER_DEFINE_MEMBERS(CurvedExtrusionHandler)

Dgn::DgnDbStatus CurvedExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus CurvedExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }