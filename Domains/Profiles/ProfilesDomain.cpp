#include "PublicAPI\ProfilesDomain\ProfilesDomain.h"

#include "PublicAPI\ProfilesDomain\Profile.h"
#include "PublicAPI\ProfilesDomain\ConstantProfile.h"
#include "PublicAPI\ProfilesDomain\BuiltUpProfile.h"
#include "PublicAPI\ProfilesDomain\BuiltUpProfileComponent.h"
#include "PublicAPI\ProfilesDomain\ParametricProfile.h"
#include "PublicAPI\ProfilesDomain\PublishedProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

DOMAIN_DEFINE_MEMBERS(ProfilesDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ProfilesDomain::ProfilesDomain() : DgnDomain(BENTLEY_PROFILES_SCHEMA_NAME, "Bentley Profiles Domain", 1)
    {
    RegisterHandler(ProfilesModelHandler::GetHandler());
    RegisterHandler(ProfileHandler::GetHandler());
    RegisterHandler(ConstantProfileHandler::GetHandler());
    RegisterHandler(BuiltUpProfileComponentHandler::GetHandler());
    RegisterHandler(BuiltUpProfileHandler::GetHandler());
    RegisterHandler(ParametricProfileHandler::GetHandler());
    RegisterHandler(PublishedProfileHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ProfilesDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
 
    InsertDomainCodeSpecs(dgndb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ProfilesDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::CodeSpecId  ProfilesDomain::QueryStructuralCommonCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_PROFILES_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    
    return codeSpecId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ProfilesDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_PROFILES_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());

    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCode ProfilesDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_PROFILES_AUTHORITY, value);
    }

END_BENTLEY_PROFILES_NAMESPACE
