/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/StairsAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StairsAspect::StairsAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StairsAspect::StairsAspect
(
uint32_t numberOfRisers,
double riserHeight
) : m_numberOfRisers(numberOfRisers), m_riserHeight(riserHeight)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StairsAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_NumberOfRisers ", "
            QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_RiserHeight
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_StairsAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_numberOfRisers = select->GetValueInt(0);
    m_riserHeight = select->GetValueDouble(1);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StairsAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_StairsAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_NumberOfRisers "=?, "
            QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_RiserHeight "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindInt(1, m_numberOfRisers);
    update->BindDouble(2, m_riserHeight);
    update->BindId(3, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StairsAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_NumberOfRisers))
        {
        value.SetInteger(m_numberOfRisers);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_RiserHeight))
        {
        value.SetDouble(m_riserHeight);
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StairsAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_NumberOfRisers))
        {
        m_numberOfRisers = value.GetInteger();
        return Dgn::DgnDbStatus::Success;
        }
    
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_RiserHeight))
        {
        m_riserHeight = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StairsAspectPtr StairsAspect::Create
(
uint32_t numberOfRisers,
double riserHeight
)
    {
    return new StairsAspect(numberOfRisers, riserHeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId StairsAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_StairsAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP StairsAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StairsAspectCP StairsAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<StairsAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StairsAspectP StairsAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<StairsAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t StairsAspect::GetNumberOfRisers() const
    { 
    return m_numberOfRisers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StairsAspect::SetNumberOfRisers(uint32_t newNumberOfRisers)
    {
    m_numberOfRisers = newNumberOfRisers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double StairsAspect::GetRiserHeight() const
    { 
    return m_riserHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StairsAspect::SetRiserHeight(double newRiserHeight)
    {
    m_riserHeight = newRiserHeight;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
