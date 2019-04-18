/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\SteelMaterialProperties.h"

#ifdef _EXCLUDED_FROM_EAP_BUILD_

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(SteelMaterialPropertiesHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SteelMaterialPropertiesPtr SteelMaterialProperties::Create() 
    { return new SteelMaterialProperties(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SteelMaterialPropertiesPtr SteelMaterialProperties::Create(double k1, double k2, double k3) 
    { return new SteelMaterialProperties(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SteelMaterialPropertiesPtr SteelMaterialProperties::Clone() const
    { return new SteelMaterialProperties(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SteelMaterialProperties::SteelMaterialProperties(SteelMaterialPropertiesCR rhs) 
    { *this = rhs; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SteelMaterialProperties& SteelMaterialProperties::operator= (SteelMaterialPropertiesCR rhs)
    {
    /*
    m_k1 = rhs.m_k1;
    m_k2 = rhs.m_k2;
    m_k3 = rhs.m_k3;
        */
    return *this;
    }
bool SteelMaterialProperties::IsValid() const { return true; }
bool SteelMaterialProperties::IsEqual(SteelMaterialPropertiesCR rhs) const
    {
    /* NEEDSWORK - system nothandled exception here
    if (m_k1 == rhs.m_k1 && m_k2 == rhs.m_k2 && m_k3 == rhs.m_k3)
        return true;
    */
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<Dgn::DgnElement::Aspect> SteelMaterialPropertiesHandler::_CreateInstance() 
    { return SteelMaterialProperties::Create(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SteelMaterialProperties::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus SteelMaterialProperties::_UpdateProperties(Dgn::DgnElementCR el)
    {
    /*
    Utf8PrintfString ecsql("UPDATE %s.%s SET K1 = ?, K2 = ?, K3 = ? WHERE ECInstanceId = ?;", _GetECSchemaName(), _GetECClassName());

    DCachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(ecsql);

    double k1 = GetK1();
    stmt->BindDouble(1, k1);

    double k2 = GetK2();
    stmt->BindDouble(2, k2);

    double k3 = GetK3();
    stmt->BindDouble(3, k3);

    stmt->BindId(4, GetAspectInstanceId(el));

    if (BE_SQLITE_DONE != stmt->Step())
        return Dgn::DgnDbStatus::BadElement;
    */
    return Dgn::DgnDbStatus::Success;
    }

Dgn::DgnDbStatus SteelMaterialProperties::_LoadProperties(Dgn::DgnElementCR el)
{
    return Dgn::DgnDbStatus::WriteError;
}


END_BENTLEY_STRUCTURAL_NAMESPACE

#endif
