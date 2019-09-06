/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "FormsDomain\StraightProfiledExtrusion.h"
#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_FORMS_NAMESPACE

HANDLER_DEFINE_MEMBERS(StraightProfiledExtrusionHandler)

StraightProfiledExtrusion::StraightProfiledExtrusion() : m_Length(0.0)
    {
    }

void StraightProfiledExtrusion::SetProfile(Dgn::DgnElementCR profile) 
    { 
    ProfileId profileId(profile.GetElementId().GetValueUnchecked()); 
    m_profileId = profileId; 
    }

StraightProfiledExtrusionCP StraightProfiledExtrusion::GetAspect(Dgn::DgnElementCR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::Get<StraightProfiledExtrusion>(element, *aspectClass) : nullptr;
    }

StraightProfiledExtrusionP StraightProfiledExtrusion::GetAspectP(Dgn::DgnElementR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::GetP<StraightProfiledExtrusion>(element, *aspectClass) : nullptr;
    }

ECN::ECClassCP StraightProfiledExtrusion::GetECClass(Dgn::DgnDbR db)
    {
    ECN::ECClassCP aspectClass = db.Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_StraightProfiledExtrusion);
    BeAssert(nullptr != aspectClass);
    return aspectClass;
    }

Dgn::DgnDbStatus StraightProfiledExtrusion::_LoadProperties(Dgn::DgnElementCR element)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("select Length, Profile from " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_StraightProfiledExtrusion) " where Element.Id=?");

    statement->BindId(1, element.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != statement->Step())
        {
        return Dgn::DgnDbStatus::ReadError;
        }

    SetLength(statement->GetValueDouble(0));
    // unused - ECN::ECRelationshipClassCP relClass = element.GetDgnDb().Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_StraightProfiledExtrusionHasProfile)->GetRelationshipClassCP();

    ECN::ECClassId relationshipId;
    SetProfileId(statement->GetValueNavigation<ProfileId>(1, &relationshipId));

    return Dgn::DgnDbStatus::Success;
    }

Dgn::DgnDbStatus StraightProfiledExtrusion::_UpdateProperties(Dgn::DgnElementCR element, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement("update " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_StraightProfiledExtrusion) " set Length=?, Profile=? where Element.Id=?", writeToken);

    statement->BindDouble(1, m_Length);

    ECN::ECRelationshipClassCP relClass = element.GetDgnDb().Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, "StraightProfiledExtrusionHasProfile")->GetRelationshipClassCP();

    statement->BindNavigationValue(2, GetProfileId(), relClass->GetId());
    statement->BindId(3, element.GetElementId());

    return (BeSQLite::BE_SQLITE_DONE != statement->Step()) ? Dgn::DgnDbStatus::WriteError : Dgn::DgnDbStatus::Success;
    }

void StraightProfiledExtrusion::SetLength(double length)
    {
    m_Length = length;
    }

END_BENTLEY_FORMS_NAMESPACE