#include "ProfilesDomain\BuiltUpProfileComponent.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE
HANDLER_DEFINE_MEMBERS(BuiltUpProfileComponentHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus BuiltUpProfileComponent::_LoadProperties(Dgn::DgnElementCR element)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement(
        "select Index, PlacementPoint, MirrorProfileAboutYAxis, OffsetPoint, Rotation, Profile from " 
        BENTLEY_PROFILES_SCHEMA(PROFILES_CLASS_BuiltUpProfileComponent) " where Element.Id=?");
    statement->BindId(1, element.GetElementId());

   if (BeSQLite::BE_SQLITE_ROW != statement->Step())
        return Dgn::DgnDbStatus::ReadError;

    ECN::ECClassId relProfileId;
    SetIndex(statement->GetValueInt(0));
    SetPlacementPointId(statement->GetValueInt(1));
    SetMirrorProfileAboutYAxis(statement->GetValueBoolean(2));
    SetOffsetPoint(statement->GetValuePoint2d(3));
    SetRotation(statement->GetValueDouble(4));
    SetProfileId(statement->GetValueNavigation<Dgn::DgnElementId>(5, &relProfileId));
    return Dgn::DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus BuiltUpProfileComponent::_UpdateProperties(Dgn::DgnElementCR element, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "update " BENTLEY_PROFILES_SCHEMA(PROFILES_CLASS_BuiltUpProfileComponent) 
        " set Index=?, PlacementPoint=?, MirrorProfileAboutYAxis=?, OffsetPoint=?, Rotation=?, Profile=? where Element.Id=?",
         writeToken);
    ECN::ECRelationshipClassCP relClassProfile = 
        element.GetDgnDb().Schemas().GetClass(BENTLEY_PROFILES_SCHEMA_NAME,
        PROFILES_CLASS_BuiltUpProfileComponentUsesConstantProfile)->GetRelationshipClassCP();
   
    statement->BindInt(1, _index);
    statement->BindInt(2, _placementPointId);
    statement->BindBoolean(3, _mirrorProfileAboutYAxis);
    statement->BindPoint2d(4, _offsetPoint);
    statement->BindDouble(5, _rotation);
    statement->BindNavigationValue(6, _profileId, relClassProfile->GetId());

    statement->BindId(7, element.GetElementId());
    
    return (BeSQLite::BE_SQLITE_DONE != statement->Step()) ? Dgn::DgnDbStatus::WriteError : Dgn::DgnDbStatus::Success;
    }

END_BENTLEY_PROFILES_NAMESPACE
