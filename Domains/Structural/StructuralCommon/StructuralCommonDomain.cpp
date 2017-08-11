/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralCommon/StructuralCommonDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain/StructuralCommon/StructuralCommonDomain.h>


BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(StructuralCommonDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
StructuralCommonDomain::StructuralCommonDomain() : DgnDomain(BENTLEY_STRUCTURAL_COMMON_SCHEMA_NAME, "Bentley Structural Common Domain", 1)
    {
    //RegisterHandler(StructuralPhysicalModelHandler::GetHandler());
    //RegisterHandler(StructuralTypeDefinitionModelHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralCommonDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralCommonDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::CodeSpecId  StructuralCommonDomain::QueryStructuralCommonCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_STRUCTURAL_COMMON_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCode StructuralCommonDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_STRUCTURAL_COMMON_AUTHORITY, value);
    }

END_BENTLEY_STRUCTURAL_NAMESPACE
