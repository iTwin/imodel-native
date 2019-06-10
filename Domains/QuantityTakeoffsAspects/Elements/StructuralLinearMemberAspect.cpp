/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/StructuralLinearMemberAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralLinearMemberAspect::StructuralLinearMemberAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralLinearMemberAspect::StructuralLinearMemberAspect
(
double crossSectionalArea,
Utf8StringCR sectionName,
StructuralFramingType type
) : m_crossSectionalArea(crossSectionalArea), m_sectionName(sectionName), m_type(type)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StructuralLinearMemberAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_CrossSectionalArea ", "
            QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_SectionName ", "
            QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_Type
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_StructuralLinearMemberAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_crossSectionalArea = select->GetValueDouble(0);
    m_sectionName = select->GetValueText(1);
    m_type = static_cast<StructuralFramingType>(select->GetValueInt(2));

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StructuralLinearMemberAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_StructuralLinearMemberAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_CrossSectionalArea "=?, "
            QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_SectionName "=?, "
            QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_Type "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindDouble(1, m_crossSectionalArea);
    update->BindText(2, m_sectionName.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    update->BindInt(3, static_cast<uint32_t>(m_type));
    update->BindId(4, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StructuralLinearMemberAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_CrossSectionalArea))
        {
        value.SetDouble(m_crossSectionalArea);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_SectionName))
        {
        value.SetUtf8CP(m_sectionName.c_str());
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_Type))
        {
        value.SetInteger(static_cast<uint32_t>(m_type));
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus StructuralLinearMemberAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_CrossSectionalArea))
        {
        m_crossSectionalArea = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }
    
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_SectionName))
        {
        m_sectionName = value.GetUtf8CP();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_Type))
        {
        m_type = static_cast<StructuralFramingType>(value.GetInteger());
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralLinearMemberAspectPtr StructuralLinearMemberAspect::Create
(
double crossSectionalArea,
Utf8StringCR sectionName,
StructuralFramingType type
)
    {
    return new StructuralLinearMemberAspect(crossSectionalArea, sectionName, type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId StructuralLinearMemberAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_StructuralLinearMemberAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP StructuralLinearMemberAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralLinearMemberAspectCP StructuralLinearMemberAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<StructuralLinearMemberAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralLinearMemberAspectP StructuralLinearMemberAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<StructuralLinearMemberAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double StructuralLinearMemberAspect::GetCrossSectionalArea() const
    { 
    return m_crossSectionalArea;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StructuralLinearMemberAspect::SetCrossSectionalArea(double newCrossSectionalArea)
    {
    m_crossSectionalArea = newCrossSectionalArea;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP StructuralLinearMemberAspect::GetSectionName() const
    { 
    return &m_sectionName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StructuralLinearMemberAspect::SetSectionName(Utf8StringCR newSectionName)
    {
    m_sectionName = newSectionName;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralFramingType StructuralLinearMemberAspect::GetType() const
    { 
    return m_type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StructuralLinearMemberAspect::SetType(StructuralFramingType newType)
    {
    m_type = newType;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
