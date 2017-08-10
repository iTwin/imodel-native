/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralProfiles/StructuralProfilesDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain\StructuralProfiles\StructuralProfilesDomain.h>

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(StructuralProfilesDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
StructuralProfilesDomain::StructuralProfilesDomain() : DgnDomain(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, "Bentley Structural Profiles Domain", 1)
    {
    // TODO: register handlers once they are created
    // RegisterHandler(StructuralPhysicalModelHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralProfilesDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralProfilesDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::CodeSpecId  StructuralProfilesDomain::QueryStructuralPhysicalCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCode StructuralProfilesDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, value);
    }

END_BENTLEY_STRUCTURAL_NAMESPACE
