/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/ConcreteDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConcreteSchemaInternal.h"

BEGIN_BENTLEY_CONCRETE_NAMESPACE

DOMAIN_DEFINE_MEMBERS(ConcreteDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteDomain::ConcreteDomain() : DgnDomain(BENTLEY_CONCRETE_SCHEMA_NAME, "Bentley Concrete Domain", 1)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::InsertDomainCodeSpecs(DgnDbR db)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, BENTLEY_CONCRETE_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

    InsertDomainCodeSpecs(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    }

END_BENTLEY_CONCRETE_NAMESPACE
