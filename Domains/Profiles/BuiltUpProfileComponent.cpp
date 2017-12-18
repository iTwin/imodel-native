#include "ProfilesDomain\BuiltUpProfileComponent.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE
HANDLER_DEFINE_MEMBERS(BuiltUpProfileComponentHandler)

Dgn::DgnDbStatus BuiltUpProfileComponent::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }


Dgn::DgnDbStatus BuiltUpProfileComponent::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

END_BENTLEY_PROFILES_NAMESPACE
