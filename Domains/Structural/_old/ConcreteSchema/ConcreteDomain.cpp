/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/ConcreteSchema/ConcreteDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainInternal.h"


BEGIN_BENTLEY_CONCRETE_NAMESPACE

DOMAIN_DEFINE_MEMBERS(ConcreteDomain)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteDomain::ConcreteDomain() : Dgn::DgnDomain(BENTLEY_CONCRETE_SCHEMA_NAME, "Bentley Concrete Domain", 1)
    {
    //RegisterHandler(ConcreteElementHandler::GetHandler());

    //RegisterHandler(FrameElementHandler::GetHandler());
    //RegisterHandler(BeamHandler::GetHandler());
    //RegisterHandler(ColumnHandler::GetHandler());

    //RegisterHandler(SurfaceElementHandler::GetHandler());
    //RegisterHandler(SlabHandler::GetHandler());
    //RegisterHandler(WallHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_CONCRETE_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

    InsertDomainCodeSpecs(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ConcreteCategory::InsertDomainCategories(Dgn::DgnDbR db)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId ConcreteCategory::InsertCategory(Dgn::DgnDbR db, Utf8CP codeValue, Dgn::ColorDef const& color)
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
Dgn::DgnSubCategoryId ConcreteCategory::InsertSubCategory(Dgn::DgnDbR db, Dgn::DgnCategoryId categoryId, Utf8CP name, Dgn::ColorDef const& color)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    Dgn::DgnSubCategoryPtr newSubCategory = new Dgn::DgnSubCategory(Dgn::DgnSubCategory::CreateParams(db, categoryId, name, appearance));
    if (!newSubCategory.IsValid())
        return Dgn::DgnSubCategoryId();

    Dgn::DgnSubCategoryCPtr insertedSubCategory = newSubCategory->Insert();
    if (!insertedSubCategory.IsValid())
        return Dgn::DgnSubCategoryId();

    return insertedSubCategory->GetSubCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId ConcreteCategory::QueryStructuralPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
    {
    Dgn::DgnCategoryId id = Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), categoryName));

    // Create it if is does not exist.

    if (!id.IsValid())
        {
        id = InsertCategory(db, categoryName, Dgn::ColorDef::White());
        }
    return id;
    }


END_BENTLEY_CONCRETE_NAMESPACE
