/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI\StructuralPhysicalDefinitions.h"
#include "PublicAPI\ConcreteMaterialProperties.h"
#ifdef _EXCLUDED_FROM_EAP_BUILD_
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(ConcreteMaterialPropertiesHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteMaterialPropertiesPtr ConcreteMaterialProperties::Create() 
    { return new ConcreteMaterialProperties(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteMaterialPropertiesPtr ConcreteMaterialProperties::Create(double k1, double k2, double k3) 
    { return new ConcreteMaterialProperties(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteMaterialPropertiesPtr ConcreteMaterialProperties::Clone() const
    { return new ConcreteMaterialProperties(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteMaterialProperties::ConcreteMaterialProperties(ConcreteMaterialPropertiesCR rhs) 
    { *this = rhs; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteMaterialProperties& ConcreteMaterialProperties::operator= (ConcreteMaterialPropertiesCR rhs)
    {
    /*
    m_k1 = rhs.m_k1;
    m_k2 = rhs.m_k2;
    m_k3 = rhs.m_k3;
        */
    return *this;
    }
bool ConcreteMaterialProperties::IsValid() const { return true; }

bool ConcreteMaterialProperties::IsEqual(ConcreteMaterialPropertiesCR rhs) const
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
RefCountedPtr<Dgn::DgnElement::Aspect> ConcreteMaterialPropertiesHandler::_CreateInstance() 
    { return ConcreteMaterialProperties::Create(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jason.Chickneas                     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus ConcreteMaterialProperties::_UpdateProperties(Dgn::DgnElementCR el)
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

Dgn::DgnDbStatus ConcreteMaterialProperties::_LoadProperties(Dgn::DgnElementCR el)
{
    return Dgn::DgnDbStatus::WriteError;
}


Dgn::DgnDbStatus ConcreteMaterialProperties::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
{
    return Dgn::DgnDbStatus::WriteError;
}

END_BENTLEY_STRUCTURAL_NAMESPACE

#endif