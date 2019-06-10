/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/SlabAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabAspect::SlabAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabAspect::SlabAspect
(
SlabDirectionType slabDirection,
SlabType type
) : m_slabDirection(slabDirection), m_type(type)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SlabAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_SLABASPECT_SlabDirection ", "
            QUANTITYTAKEOFFSASPECTS_SLABASPECT_Type
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_SlabAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_slabDirection = static_cast<SlabDirectionType>(select->GetValueInt(0));
    m_type = static_cast<SlabType>(select->GetValueInt(1));

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SlabAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_SlabAspect)
        " SET " 
            QUANTITYTAKEOFFSASPECTS_SLABASPECT_SlabDirection "=?, "
            QUANTITYTAKEOFFSASPECTS_SLABASPECT_Type "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindInt(1, static_cast<uint32_t>(m_slabDirection));
    update->BindInt(2, static_cast<uint32_t>(m_type));
    update->BindId(3, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SlabAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SLABASPECT_SlabDirection))
        {
        value.SetInteger(static_cast<uint32_t>(m_slabDirection));
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SLABASPECT_Type))
        {
        value.SetInteger(static_cast<uint32_t>(m_type));
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SlabAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SLABASPECT_SlabDirection))
        {
        m_slabDirection = static_cast<SlabDirectionType>(value.GetInteger());
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_SLABASPECT_Type))
        {
        m_type = static_cast<SlabType>(value.GetInteger());
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabAspectPtr SlabAspect::Create(SlabDirectionType slabDirection, SlabType type)
    {
    return new SlabAspect(slabDirection, type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId SlabAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SlabAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP SlabAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabAspectCP SlabAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<SlabAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabAspectP SlabAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<SlabAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabDirectionType SlabAspect::GetSlabDirection() const
    { 
    return m_slabDirection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SlabAspect::SetSlabDirection(SlabDirectionType newSlabDirection)
    {
    m_slabDirection = newSlabDirection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SlabType SlabAspect::GetType() const
    { 
    return m_type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SlabAspect::SetType(SlabType newType)
    {
    m_type = newType;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
