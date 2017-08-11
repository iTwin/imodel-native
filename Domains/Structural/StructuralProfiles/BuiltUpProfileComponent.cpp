#include <StructuralDomain/StructuralProfiles/BuiltUpProfileComponent.h>

HANDLER_DEFINE_MEMBERS(BuiltUpProfileComponentHandler)

Dgn::DgnDbStatus BuiltUpProfileComponent::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }


Dgn::DgnDbStatus BuiltUpProfileComponent::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

/*BuiltUpProfileComponentPtr BuiltUpProfileComponent::GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id)
{
    //BeAssert(nullptr != db.Domains().FindDomain(SCHEMA_NAME)); // Domain must be registered
    return db.Elements().GetForEdit<BuiltUpProfileComponent>(id);
}*/
