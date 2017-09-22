#include <StructuralDomain/StructuralProfiles/StraightExtrusion.h>

USING_NAMESPACE_BENTLEY_STRUCTURAL

HANDLER_DEFINE_MEMBERS(StraightExtrusionHandler)

Dgn::DgnDbStatus StraightExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus StraightExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }