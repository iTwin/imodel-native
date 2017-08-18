#include <StructuralDomain/StructuralProfiles/VaryingProfileZone.h>

HANDLER_DEFINE_MEMBERS(VaryingProfileZoneHandler)

Dgn::DgnDbStatus VaryingProfileZone::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }


Dgn::DgnDbStatus VaryingProfileZone::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }
