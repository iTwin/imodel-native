/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/DimensionsAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionsAspect::DimensionsAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionsAspect::DimensionsAspect
(
double length,
double width,
double height
) : m_length(length), m_width(width), m_height(height)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus DimensionsAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Length ", "
            QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Width ", "
            QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Height
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_DimensionsAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_length = select->GetValueDouble(0);
    m_width = select->GetValueDouble(1);
    m_height = select->GetValueDouble(2);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus DimensionsAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_DimensionsAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Length "=?, "
            QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Width "=?, "
            QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Height "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindDouble(1, m_length);
    update->BindDouble(2, m_width);
    update->BindDouble(3, m_height);
    update->BindId(4, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus DimensionsAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Length))
        {
        value.SetDouble(m_length);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Width))
        {
        value.SetDouble(m_width);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Height))
        {
        value.SetDouble(m_height);
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus DimensionsAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Length))
        {
        m_length = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }
    
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Width))
        {
        m_width = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Height))
        {
        m_height = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionsAspectPtr DimensionsAspect::Create
(
double length,
double width,
double height
)
    {
    return new DimensionsAspect(length, width, height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId DimensionsAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_DimensionsAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP DimensionsAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionsAspectCP DimensionsAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<DimensionsAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionsAspectP DimensionsAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<DimensionsAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double DimensionsAspect::GetLength() const
    { 
    return m_length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionsAspect::SetLength(double newLength)
    {
    m_length = newLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double DimensionsAspect::GetWidth() const
    { 
    return m_width;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionsAspect::SetWidth(double newWidth)
    {
    m_width = newWidth;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double DimensionsAspect::GetHeight() const
    { 
    return m_height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionsAspect::SetHeight(double newHeight)
    {
    m_height = newHeight;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
