/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/PipeAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PipeAspect::PipeAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PipeAspect::PipeAspect
(
Utf8StringCR schedule,
double thickness,
double length,
double diameter
) : m_schedule(schedule), m_thickness(thickness), m_length(length), m_diameter(diameter)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PipeAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Schedule ", "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Thickness ", "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Length ", "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Diameter
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_PipeAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_schedule = select->GetValueText(0);
    m_thickness = select->GetValueDouble(1);
    m_length = select->GetValueDouble(2);
    m_diameter = select->GetValueDouble(3);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PipeAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_PipeAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Schedule "=?, "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Thickness "=?, "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Length"=?, "
            QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Diameter "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindText(1, m_schedule.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    update->BindDouble(2, m_thickness);
    update->BindDouble(3, m_length);
    update->BindDouble(4, m_diameter);
    update->BindId(5, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PipeAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Schedule))
        {
        value.SetUtf8CP(m_schedule.c_str());
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Thickness))
        {
        value.SetDouble(m_thickness);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Length))
        {
        value.SetDouble(m_length);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Diameter))
        {
        value.SetDouble(m_diameter);
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PipeAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Schedule))
        {
        m_schedule = value.GetUtf8CP();
        return Dgn::DgnDbStatus::Success;
        }
    
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Thickness))
        {
        m_thickness = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Height))
        {
        m_length = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Diameter))
        {
        m_diameter = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PipeAspectPtr PipeAspect::Create
(
Utf8StringCR schedule,
double thickness,
double length,
double diameter
)
    {
    return new PipeAspect(schedule, thickness, length, diameter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId PipeAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_PipeAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP PipeAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PipeAspectCP PipeAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<PipeAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PipeAspectP PipeAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<PipeAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP PipeAspect::GetSchedule() const
    { 
    return &m_schedule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void PipeAspect::SetSchedule(Utf8StringCR newSchedule)
    {
    m_schedule = newSchedule;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double PipeAspect::GetThickness() const
    { 
    return m_thickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void PipeAspect::SetThickness(double newThickness)
    {
    m_thickness = newThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double PipeAspect::GetLength() const
    { 
    return m_length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void PipeAspect::SetLength(double newLength)
    {
    m_length = newLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double PipeAspect::GetDiameter() const
    { 
    return m_diameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void PipeAspect::SetDiameter(double newDiameter)
    {
    m_diameter = newDiameter;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
