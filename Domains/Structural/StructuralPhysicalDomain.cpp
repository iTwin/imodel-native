/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\StructuralPhysicalApi.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(StructuralPhysicalDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
StructuralPhysicalDomain::StructuralPhysicalDomain() : DgnDomain(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, "Bentley Structural Physical Domain", 1)
    {
    RegisterHandler(StructuralPhysicalModelHandler::GetHandler());
    RegisterHandler(StructuralElementHandler::GetHandler());
    RegisterHandler(StructuralMemberHandler::GetHandler());
    RegisterHandler(SlabHandler::GetHandler());
    RegisterHandler(BeamHandler::GetHandler());
    RegisterHandler(ColumnHandler::GetHandler());
    RegisterHandler(BraceHandler::GetHandler());
    RegisterHandler(FoundationMemberHandler::GetHandler());
    RegisterHandler(StripFootingHandler::GetHandler());
    RegisterHandler(SpreadFootingHandler::GetHandler());
    RegisterHandler(WallHandler::GetHandler());
#ifdef _EXCLUDED_FROM_EAP_BUILD_
    RegisterHandler(StructuralSubtractionHandler::GetHandler());
    RegisterHandler(StructuralAdditionHandler::GetHandler());
#endif
    RegisterHandler(PileCapHandler::GetHandler());
    RegisterHandler(PileHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralPhysicalDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

    StructuralPhysicalCategory::InsertDomainCategories(dgndb);
    StructuralPhysicalDomain::InsertDomainCodeSpecs(dgndb); //???
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralPhysicalDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
 
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }
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


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void StructuralPhysicalCategory::InsertDomainCategories(Dgn::DgnDbR db)
    {
    Dgn::DgnCategoryId    doorCategoryId = InsertCategory(db, STRUCTURAL_PHYSICAL_CATEGORY_StructuralCategory, Dgn::ColorDef::Red());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId StructuralPhysicalCategory::InsertCategory(Dgn::DgnDbR db, Utf8CP codeValue, Dgn::ColorDef const& color, Dgn::DgnCategory::Rank rank /*= Dgn::DgnCategory::Rank::Domain*/)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    Dgn::SpatialCategory category(db.GetDictionaryModel(), codeValue, Dgn::DgnCategory::Rank::Domain);
    category.Insert(appearance);
    return category.GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
    {
    Dgn::DgnCategoryId id = Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), categoryName));

    // Create it if is does not exist.

    if (!id.IsValid())
        {
        id = InsertCategory(db, categoryName, Dgn::ColorDef::White());
        }

    return id;
    }

END_BENTLEY_STRUCTURAL_NAMESPACE
