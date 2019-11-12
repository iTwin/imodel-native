/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "FormsDomain\StraightExtrusion.h"
#include <DgnPlatform/DgnPlatformErrors.h>
#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_FORMS_NAMESPACE

HANDLER_DEFINE_MEMBERS(StraightExtrusionHandler)

StraightExtrusion::StraightExtrusion() : m_Length(0.0), m_Shape(nullptr)
    {
    }

StraightExtrusionCP StraightExtrusion::GetAspect(Dgn::DgnElementCR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());
    return aspectClass ? T_Super::Get<StraightExtrusion>(element, *aspectClass) : nullptr;
    }

StraightExtrusionP StraightExtrusion::GetAspectP(Dgn::DgnElementR element)
    {
    ECN::ECClassCP aspectClass = GetECClass(element.GetDgnDb());

    return aspectClass ? T_Super::GetP<StraightExtrusion>(element, *aspectClass) : nullptr;
    }

ECN::ECClassCP StraightExtrusion::GetECClass(Dgn::DgnDbR db)
    {
    ECN::ECClassCP aspectClass = db.Schemas().GetClass(BENTLEY_FORMS_SCHEMA_NAME, FORMS_CLASS_StraightExtrusion);
    BeAssert(nullptr != aspectClass);
    return aspectClass;
    }

Dgn::DgnDbStatus StraightExtrusion::_LoadProperties(Dgn::DgnElementCR element)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("select Length, Shape from " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_StraightExtrusion) " where Element.Id=?");

    statement->BindId(1, element.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != statement->Step())
        {
        return Dgn::DgnDbStatus::ReadError;
        }
    
    SetLength(statement->GetValueDouble(0));
    SetShape(statement->GetValueGeometry(1));
    
    return Dgn::DgnDbStatus::Success;
    }

Dgn::DgnDbStatus StraightExtrusion::_UpdateProperties(Dgn::DgnElementCR element, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement("UPDATE " BENTLEY_FORMS_SCHEMA(FORMS_CLASS_StraightExtrusion) " set Length=?, Shape=? where Element.Id=?", writeToken);

    statement->BindDouble(1, m_Length);
    (m_Shape.IsNull()) ? statement->BindNull(2) : statement->BindGeometry(2, *m_Shape);
    statement->BindId(3, element.GetElementId());

    return (BeSQLite::BE_SQLITE_DONE != statement->Step()) ? Dgn::DgnDbStatus::WriteError : Dgn::DgnDbStatus::Success;
    }

void StraightExtrusion::SetLength(double length)
    {
    m_Length = length;
    }

void StraightExtrusion::SetShape(IGeometryPtr shape)
    {
    m_Shape = shape;
    }

END_BENTLEY_FORMS_NAMESPACE