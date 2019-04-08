#include "FormsDomain\FormsDomain.h"
#include "PublicAPI\Form.h"
#include "PublicAPI\StraightExtrusion.h"
#include "PublicAPI\CurvedProfiledExtrusion.h"
#include "PublicAPI\StraightProfiledExtrusion.h"
#include "PublicAPI\CurvedExtrusion.h"


BEGIN_BENTLEY_FORMS_NAMESPACE

DOMAIN_DEFINE_MEMBERS(FormsDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
FormsDomain::FormsDomain() : DgnDomain(BENTLEY_FORMS_SCHEMA_NAME, "Bentley Forms Domain", 1)
    {
    RegisterHandler(FormHandler::GetHandler());
    RegisterHandler(StraightExtrusionHandler::GetHandler());
    RegisterHandler(CurvedExtrusionHandler::GetHandler());
    RegisterHandler(StraightProfiledExtrusionHandler::GetHandler());
    RegisterHandler(CurvedProfiledExtrusionHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void FormsDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void FormsDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::CodeSpecId  FormsDomain::QueryStructuralCommonCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_FORMS_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    
    return codeSpecId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCode FormsDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_FORMS_AUTHORITY, value);
    }

END_BENTLEY_FORMS_NAMESPACE
