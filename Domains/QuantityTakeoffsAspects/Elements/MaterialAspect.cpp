/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/MaterialAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialAspect::MaterialAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialAspect::MaterialAspect
(
Utf8StringCR material,
double materialDensity,
double weight
) : m_material(material), m_materialDensity(materialDensity), m_weight(weight)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus MaterialAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    BeSQLite::EC::CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT "
            QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Material ", "
            QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_MaterialDensity ", "
            QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Weight
        " FROM " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_MaterialAspect)
        " WHERE Element.Id=?");

    if (!select.IsValid())
        return Dgn::DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if (BeSQLite::BE_SQLITE_ROW != select->Step())
        return Dgn::DgnDbStatus::ReadError;

    m_material = select->GetValueText(0);
    m_materialDensity = select->GetValueDouble(1);
    m_weight = select->GetValueDouble(2);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus MaterialAspect::_UpdateProperties
(
Dgn::DgnElementCR el,
BeSQLite::EC::ECCrudWriteToken const* writeToken
)
    {
    BeSQLite::EC::CachedECSqlStatementPtr update = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " QUANTITYTAKEOFFSASPECTS_SCHEMA(QUANTITYTAKEOFFSASPECTS_CLASS_MaterialAspect)
        " SET "
            QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Material "=?, "
            QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_MaterialDensity "=?, "
            QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Weight "=?"
        " WHERE Element.Id=?", writeToken);

    if (!update.IsValid())
        return Dgn::DgnDbStatus::WriteError;

    update->BindText(1, m_material.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    update->BindDouble(2, m_materialDensity);
    update->BindDouble(3, m_weight);
    update->BindId(4, el.GetElementId());

    if (BeSQLite::BE_SQLITE_DONE != update->Step())
        return Dgn::DgnDbStatus::WriteError;

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus MaterialAspect::_GetPropertyValue
(
ECN::ECValueR value,
Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Material))
        {
        value.SetUtf8CP(m_material.c_str());
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_MaterialDensity))
        {
        value.SetDouble(m_materialDensity);
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Weight))
        {
        value.SetDouble(m_weight);
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus MaterialAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value,
Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Material))
        {
        m_material = value.GetUtf8CP();
        return Dgn::DgnDbStatus::Success;
        }
    
    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_MaterialDensity))
        {
        m_materialDensity = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Weight))
        {
        m_weight = value.GetDouble();
        return Dgn::DgnDbStatus::Success;
        }

    return Dgn::DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialAspectPtr MaterialAspect::Create
(
Utf8StringCR material,
double materialDensity,
double weight
)
    {
    return new MaterialAspect(material, materialDensity, weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId MaterialAspect::QueryECClassId(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClassId(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_MaterialAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP MaterialAspect::QueryECClass(Dgn::DgnDbR db)
    {
    return db.Schemas().GetClass(QueryECClassId(db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialAspectCP MaterialAspect::GetCP(Dgn::DgnElementCR el)
    {
    return UniqueAspect::Get<MaterialAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialAspectP MaterialAspect::GetP(Dgn::DgnElementR el)
    {
    return UniqueAspect::GetP<MaterialAspect>(el, *QueryECClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP MaterialAspect::GetMaterial() const
    { 
    return &m_material;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialAspect::SetMaterial(Utf8StringCR newMaterial)
    {
    m_material = newMaterial;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double MaterialAspect::GetMaterialDensity() const
    { 
    return m_materialDensity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialAspect::SetMaterialDensity(double newMaterialDensity)
    {
    m_materialDensity = newMaterialDensity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double MaterialAspect::GetWeight() const
    { 
    return m_weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialAspect::SetWeight(double newWeight)
    {
    m_weight = newWeight;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
