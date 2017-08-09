/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/StructuralMaterials/StructuralMaterialsDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainInternal.h"


BEGIN_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

DOMAIN_DEFINE_MEMBERS(StructuralMaterialsDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralMaterialsDomain::StructuralMaterialsDomain() : Dgn::DgnDomain(BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME, "Bentley Structural Materials Domain", 1)
    {
    //RegisterHandler(ArchitecturalBaseElementHandler::GetHandler());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralMaterialsDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_STRUCTURAL_MATERIALS_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        codeSpec->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Bentley.Systems
+---------------+---------------+---------------+---------------+---------------+------*/
void StructuralMaterialsDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    InsertDomainCodeSpecs( dgndb );
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/16
//---------------------------------------------------------------------------------------
void StructuralMaterialsDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }



END_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

