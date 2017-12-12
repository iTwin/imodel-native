#include "PublicAPI\FormsDomain\ProfiledExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

HANDLER_DEFINE_MEMBERS(ProfiledExtrusionHandler)

Dgn::DgnDbStatus ProfiledExtrusion::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

Dgn::DgnDbStatus ProfiledExtrusion::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

ProfiledExtrusionPtr ProfiledExtrusion::Create() 
    { 
    return new ProfiledExtrusion(); 
    };

END_BENTLEY_FORMS_NAMESPACE