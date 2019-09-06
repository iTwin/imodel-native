/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "FormsDomain\CurvedExtrusion.h"

BEGIN_BENTLEY_FORMS_NAMESPACE
HANDLER_DEFINE_MEMBERS(CurvedExtrusionHandler)

CurvedExtrusion::CurvedExtrusion() : m_startShape(nullptr), m_endShape(nullptr), m_Curve(nullptr)
    {
    }

CurvedExtrusionCP CurvedExtrusion::GetAspect(Dgn::DgnElementCR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::Get<CurvedExtrusion>(element, *aspectClass) : nullptr;
    }

CurvedExtrusionP CurvedExtrusion::GetAspectP(Dgn::DgnElementR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::GetP<CurvedExtrusion>(element, *aspectClass) : nullptr;
    }

ECN::ECClassCP CurvedExtrusion::GetECClass(Dgn::DgnDbR db)
    {
    ECN::ECClassCP aspectClass = db.Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_CurvedExtrusion);
    BeAssert(nullptr != aspectClass);
    return aspectClass;
    }

void CurvedExtrusion::SetStartShape(IGeometryPtr shape)
    {
    m_startShape = shape;
    }

void CurvedExtrusion::SetEndShape(IGeometryPtr shape)
    {
    m_endShape = shape;
    }

void CurvedExtrusion::SetCurve(IGeometryPtr curve)
    {
    m_Curve = curve;
    }

Dgn::DgnDbStatus CurvedExtrusion::_LoadProperties(Dgn::DgnElementCR element)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("select StartShape, EndShape, Curve from " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_CurvedExtrusion) " where Element.Id=?");

    statement->BindId(1, element.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != statement->Step())
        {
        return Dgn::DgnDbStatus::ReadError;
        }

    SetStartShape(statement->GetValueGeometry(0));
    SetEndShape(statement->GetValueGeometry(1));
    SetCurve(statement->GetValueGeometry(2));

    return Dgn::DgnDbStatus::Success;
    }

Dgn::DgnDbStatus CurvedExtrusion::_UpdateProperties(Dgn::DgnElementCR element, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement("update " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_CurvedExtrusion) " set StartShape=?, EndShape=?, Curve=? where Element.Id=?", writeToken);

    (m_startShape.IsNull()) ? statement->BindNull(1) : statement->BindGeometry(1, *m_startShape);
    (m_endShape.IsNull()) ? statement->BindNull(2) : statement->BindGeometry(2, *m_endShape);
    (m_Curve.IsNull()) ? statement->BindNull(3) : statement->BindGeometry(3, *m_Curve);

    statement->BindId(4, element.GetElementId());

    return (BeSQLite::BE_SQLITE_DONE != statement->Step()) ? Dgn::DgnDbStatus::WriteError : Dgn::DgnDbStatus::Success;
    }

END_BENTLEY_FORMS_NAMESPACE