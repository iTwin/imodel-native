/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/SideAreasAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspect::SideAreasAspect() : m_netAreas(), m_grossAreas()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspect::SideAreasAspect(SideAreasCR netAreas, SideAreasCR grossAreas) : m_netAreas(netAreas), m_grossAreas(grossAreas)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SideAreasAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideNetArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideNetArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopNetArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomNetArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideGrossArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideGrossArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopGrossArea ", "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomGrossArea
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_SideAreasAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_netAreas.rightSide = select->GetValueDouble(0);
    m_netAreas.leftSide = select->GetValueDouble(1);
    m_netAreas.top = select->GetValueDouble(2);
    m_netAreas.bottom = select->GetValueDouble(3);

    m_grossAreas.rightSide = select->GetValueDouble(4);
    m_grossAreas.leftSide = select->GetValueDouble(5);
    m_grossAreas.top = select->GetValueDouble(6);
    m_grossAreas.bottom = select->GetValueDouble(7);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SideAreasAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_SideAreasAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideNetArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideNetArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopNetArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomNetArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideGrossArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideGrossArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopGrossArea "=?, "
            QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomGrossArea "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindDouble(1, m_netAreas.rightSide);
    update->BindDouble(2, m_netAreas.leftSide);
    update->BindDouble(3, m_netAreas.top);
    update->BindDouble(4, m_netAreas.bottom);
    update->BindDouble(5, m_grossAreas.rightSide);
    update->BindDouble(6, m_grossAreas.leftSide);
    update->BindDouble(7, m_grossAreas.top);
    update->BindDouble(8, m_grossAreas.bottom);
    update->BindId(9, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SideAreasAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideNetArea))
        {
        value.SetDouble(m_netAreas.rightSide);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideNetArea))
        {
        value.SetDouble(m_netAreas.leftSide);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopNetArea))
        {
        value.SetDouble(m_netAreas.top);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomNetArea))
        {
        value.SetDouble(m_netAreas.bottom);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideGrossArea))
        {
        value.SetDouble(m_grossAreas.rightSide);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideGrossArea))
        {
        value.SetDouble(m_grossAreas.leftSide);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopGrossArea))
        {
        value.SetDouble(m_grossAreas.top);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomGrossArea))
        {
        value.SetDouble(m_grossAreas.bottom);
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SideAreasAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideNetArea))
        {
        m_netAreas.rightSide = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideNetArea))
        {
        m_netAreas.leftSide = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopNetArea))
        {
        m_netAreas.top = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomNetArea))
        {
        m_netAreas.bottom = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideGrossArea))
        {
        m_grossAreas.rightSide = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideGrossArea))
        {
        m_grossAreas.leftSide = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopGrossArea))
        {
        m_grossAreas.top = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomGrossArea))
        {
        m_grossAreas.bottom = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspectPtr SideAreasAspect::Create(SideAreasCR netAreas, SideAreasCR grossAreas)
    {
    return new SideAreasAspect(netAreas, grossAreas);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId SideAreasAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SideAreasAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP SideAreasAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspectCP SideAreasAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<SideAreasAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspectP SideAreasAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<SideAreasAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspect::SideAreasCP SideAreasAspect::GetNetAreas() const
    { 
    return &m_netAreas;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SideAreasAspect::SideAreasCP SideAreasAspect::GetGrossAreas() const
    { 
    return &m_grossAreas;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetNetAreas(SideAreasCR newAreas)
    {
    m_netAreas = newAreas;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetRightSideNetArea(double area)
    {
    m_netAreas.rightSide = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetLeftSideNetArea(double area)
    {
    m_netAreas.leftSide = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetTopNetArea(double area)
    {
    m_netAreas.top = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetBottomNetArea(double area)
    {
    m_netAreas.bottom = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetGrossAreas(SideAreasCR newAreas)
    {
    m_grossAreas = newAreas;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetRightSideGrossArea(double area)
    {
    m_grossAreas.rightSide = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetLeftSideGrossArea(double area)
    {
    m_grossAreas.leftSide = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetTopGrossArea(double area)
    {
    m_grossAreas.top = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SideAreasAspect::SetBottomGrossArea(double area)
    {
    m_grossAreas.bottom = area;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
