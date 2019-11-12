/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/SurfaceAreaAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceAreaAspect::SurfaceAreaAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceAreaAspect::SurfaceAreaAspect
(
double grossSurfaceArea,
double netSurfaceArea
) : m_grossSurfaceArea(grossSurfaceArea), m_netSurfaceArea(netSurfaceArea)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SurfaceAreaAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_GrossSurfaceArea ", "
            QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_NetSurfaceArea
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_SurfaceAreaAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_grossSurfaceArea = select->GetValueDouble(0);
    m_netSurfaceArea = select->GetValueDouble(1);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SurfaceAreaAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_SurfaceAreaAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_GrossSurfaceArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_NetSurfaceArea "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindDouble(1, m_grossSurfaceArea);
    update->BindDouble(2, m_netSurfaceArea);
    update->BindId(3, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SurfaceAreaAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_GrossSurfaceArea))
        {
        value.SetDouble(m_grossSurfaceArea);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_NetSurfaceArea))
        {
        value.SetDouble(m_netSurfaceArea);
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SurfaceAreaAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_GrossSurfaceArea))
        {
        m_grossSurfaceArea = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }
    
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_NetSurfaceArea))
        {
        m_netSurfaceArea = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceAreaAspectPtr SurfaceAreaAspect::Create
(
double grossSurfaceArea,
double netSurfaceArea
)
    {
    return new SurfaceAreaAspect(grossSurfaceArea, netSurfaceArea);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId SurfaceAreaAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SurfaceAreaAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP SurfaceAreaAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceAreaAspectCP SurfaceAreaAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<SurfaceAreaAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceAreaAspectP SurfaceAreaAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<SurfaceAreaAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double SurfaceAreaAspect::GetGrossSurfaceArea() const
    { 
    return m_grossSurfaceArea;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SurfaceAreaAspect::SetGrossSurfaceArea(double newGrossSurfaceArea)
    {
    m_grossSurfaceArea = newGrossSurfaceArea;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double SurfaceAreaAspect::GetNetSurfaceArea() const
    { 
    return m_netSurfaceArea;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SurfaceAreaAspect::SetNetSurfaceArea(double newNetSurfaceArea)
    {
    m_netSurfaceArea = newNetSurfaceArea;
    }


END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
