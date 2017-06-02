/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterials/StructuralMaterialsDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralMaterialsSchemaInternal.h"

BEGIN_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

DOMAIN_DEFINE_MEMBERS(StructuralMaterialsDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralMaterialsDomain::StructuralMaterialsDomain() : DgnDomain(BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME, "Bentley Structural Materials Domain", 1)
    {
    //RegisterHandler(ArchitecturalBaseElementHandler::GetHandler());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralMaterialsDomain::InsertDomainCodeSpecs(DgnDbR db)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, BENTLEY_STRUCTURAL_MATERIALS_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        codeSpec->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Bentley.Systems
+---------------+---------------+---------------+---------------+---------------+------*/
void StructuralMaterialsDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    InsertDomainCodeSpecs( dgndb );
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/16
//---------------------------------------------------------------------------------------
void StructuralMaterialsDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    }



END_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

