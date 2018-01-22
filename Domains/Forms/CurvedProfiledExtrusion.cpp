#include "FormsDomain\CurvedProfiledExtrusion.h"
#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_FORMS_NAMESPACE
HANDLER_DEFINE_MEMBERS(CurvedProfiledExtrusionHandler)

CurvedProfiledExtrusion::CurvedProfiledExtrusion() : m_Curve(nullptr)
    {
    }

CurvedProfiledExtrusionCP CurvedProfiledExtrusion::GetAspect(Dgn::DgnElementCR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::Get<CurvedProfiledExtrusion>(element, *aspectClass) : nullptr;
    }

CurvedProfiledExtrusionP CurvedProfiledExtrusion::GetAspectP(Dgn::DgnElementR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::GetP<CurvedProfiledExtrusion>(element, *aspectClass) : nullptr;
    }

ECN::ECClassCP CurvedProfiledExtrusion::GetECClass(Dgn::DgnDbR db)
    {
    ECN::ECClassCP aspectClass = db.Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_CurvedProfiledExtrusion);
    BeAssert(nullptr != aspectClass);
    return aspectClass;
    }

void CurvedProfiledExtrusion::SetStartProfile(Dgn::DgnElementCR profile)
    {
    StartProfileId profileId(profile.GetElementId().GetValueUnchecked());
    m_startProfileId = profileId;
    }

void CurvedProfiledExtrusion::SetEndProfile(Dgn::DgnElementCR profile)
    {
    EndProfileId profileId(profile.GetElementId().GetValueUnchecked());
    m_endProfileId = profileId;
    }

Dgn::DgnDbStatus CurvedProfiledExtrusion::_LoadProperties(Dgn::DgnElementCR element)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("select StartProfile, EndProfile, Curve from " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_CurvedProfiledExtrusion) " where Element.Id=?");

    statement->BindId(1, element.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != statement->Step())
        {
        return Dgn::DgnDbStatus::ReadError;
        }

    ECN::ECClassId relStartPofileId;
    ECN::ECClassId relEndPofileId;

    SetStartProfileId(statement->GetValueNavigation<StartProfileId>(0, &relStartPofileId));
    SetEndProfileId(statement->GetValueNavigation<EndProfileId>(1, &relEndPofileId));
    SetCurve(statement->GetValueGeometry(2));

    return Dgn::DgnDbStatus::Success;
    }

Dgn::DgnDbStatus CurvedProfiledExtrusion::_UpdateProperties(Dgn::DgnElementCR element, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement("update " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_CurvedProfiledExtrusion) " set StartProfile=?, EndProfile=?, Curve=? where Element.Id=?", writeToken);

    ECN::ECRelationshipClassCP relClassStartProfile = element.GetDgnDb().Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_CurvedProfiledExtrusionHasStartProfile)->GetRelationshipClassCP();
    ECN::ECRelationshipClassCP relClassEndProfile = element.GetDgnDb().Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_CurvedProfiledExtrusionHasEndProfile)->GetRelationshipClassCP();

    statement->BindNavigationValue(1, GetStartProfileId(), relClassStartProfile->GetId());
    statement->BindNavigationValue(2, GetEndProfileId(), relClassEndProfile->GetId());
    (m_Curve.IsNull()) ? statement->BindNull(3) : statement->BindGeometry(3, *GetCurve());

    statement->BindId(4, element.GetElementId());


    return (BeSQLite::BE_SQLITE_DONE != statement->Step()) ? Dgn::DgnDbStatus::WriteError : Dgn::DgnDbStatus::Success;
    }

void CurvedProfiledExtrusion::SetCurve(IGeometryPtr curve)
    {
    m_Curve = curve;
    }

END_BENTLEY_FORMS_NAMESPACE