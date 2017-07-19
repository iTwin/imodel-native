/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysicalSchema/StructuralPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainInternal.h"


BEGIN_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(StructuralPhysicalDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
StructuralPhysicalDomain::StructuralPhysicalDomain() : DgnDomain(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, "Bentley Structural Physical Domain", 1)
    {
    RegisterHandler(StructuralPhysicalModelHandler::GetHandler());
    RegisterHandler(StructuralTypeDefinitionModelHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralPhysicalDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralPhysicalDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::CodeSpecId  StructuralPhysicalDomain::QueryStructuralPhysicalCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCode StructuralPhysicalDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, value);
    }


END_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE
