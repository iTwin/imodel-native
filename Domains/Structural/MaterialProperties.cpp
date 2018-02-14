/*--------------------------------------------------------------------------------------+
|
|     $Source: MaterialProperties.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\MaterialProperties.h"

#ifdef _EXCLUDED_FROM_EAP_BUILD_

USING_NAMESPACE_BENTLEY_STRUCTURAL

HANDLER_DEFINE_MEMBERS(MaterialPropertiesHandler);

MaterialPropertiesPtr MaterialProperties::Create()
{
    return new MaterialProperties();
}

MaterialPropertiesPtr MaterialProperties::Create(double elasticModulus, double poissonsRatio, double tensileStrength, double thermalExpansionCoefficient)
{
    return new MaterialProperties(elasticModulus, poissonsRatio, tensileStrength, thermalExpansionCoefficient);
}

MaterialPropertiesPtr MaterialProperties::Clone() const
{
    return new MaterialProperties(*this);
}

MaterialProperties::MaterialProperties(MaterialPropertiesCR rhs)
{
    *this = rhs;
}

MaterialProperties& MaterialProperties::operator= (MaterialPropertiesCR rhs)
{
    m_elasticModulus = rhs.m_elasticModulus;
    m_poissonsRatio = rhs.m_poissonsRatio;
    m_tensileStrength = rhs.m_tensileStrength;
    m_thermalExpansionCoefficient = rhs.m_thermalExpansionCoefficient;
    return *this;
}
bool MaterialProperties::IsValid() const { return true; }
bool MaterialProperties::IsEqual(MaterialPropertiesCR rhs) const
{
    if (m_elasticModulus == rhs.m_elasticModulus && m_poissonsRatio == rhs.m_poissonsRatio && m_tensileStrength == rhs.m_tensileStrength && m_thermalExpansionCoefficient == rhs.m_thermalExpansionCoefficient)
        return true;
    return false;
}
double MaterialProperties::GetElasticModulus() const { return m_elasticModulus; }
double MaterialProperties::GetPoissonsRatio() const { return m_poissonsRatio; }
double MaterialProperties::GetTensileStrength() const { return m_tensileStrength; }
double MaterialProperties::GetThermalExpansionCoefficient() const { return m_thermalExpansionCoefficient; }
void   MaterialProperties::SetElasticModulus(double val) { m_elasticModulus = val; }
void   MaterialProperties::SetPoissonsRatio(double val) { m_poissonsRatio = val; }
void   MaterialProperties::SetTensileStrength(double val) { m_tensileStrength = val; }
void   MaterialProperties::SetThermalExpansionCoefficient(double val) { m_thermalExpansionCoefficient = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus MaterialProperties::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus MaterialProperties::_UpdateProperties(Dgn::DgnElementCR el)
    {
    /*
    Utf8PrintfString ecsql("UPDATE %s.%s SET K1 = ?, K2 = ?, K3 = ? WHERE ECInstanceId = ?;", _GetECSchemaName(), _GetECClassName());

    DCachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(ecsql);

    double elasticModulus = GetElasticModulus();
    stmt->BindDouble(1, elasticModulus);

    double poissonsRatio = GetPoissonsRatio();
    stmt->BindDouble(2, poissonsRatio);

    double tensileStrength = GetTensileStrength();
    stmt->BindDouble(3, tensileStrength);

    double thermalExpansionCoefficient = GetThermalExpansionCoefficient();
    stmt->BindDouble(3, thermalExpansionCoefficient);

    stmt->BindId(4, GetAspectInstanceId(el));

    if (BE_SQLITE_DONE != stmt->Step())
    return Dgn::DgnDbStatus::BadElement;
    */
    return Dgn::DgnDbStatus::Success;
    }


Dgn::DgnDbStatus MaterialProperties::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<Dgn::DgnElement::Aspect> MaterialPropertiesHandler::_CreateInstance()
{
    return MaterialProperties::Create();
}


// Dgn::DgnDbStatus StructuralElement::_OnDelete() const 
//     {
//     return Dgn::DgnDbStatus::Success;
//     }
#endif