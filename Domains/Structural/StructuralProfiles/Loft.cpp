#include <StructuralDomain/StructuralProfiles/Loft.h>

USING_NAMESPACE_BENTLEY_STRUCTURAL

HANDLER_DEFINE_MEMBERS(LoftHandler)

Dgn::DgnDbStatus Loft::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus Loft::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }