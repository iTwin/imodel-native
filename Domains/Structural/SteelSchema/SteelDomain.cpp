/*--------------------------------------------------------------------------------------+
|
|     $Source: SteelSchema/SteelDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainInternal.h"


BEGIN_BENTLEY_STEEL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(SteelDomain)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SteelDomain::SteelDomain() : Dgn::DgnDomain(BENTLEY_STEEL_SCHEMA_NAME, "Bentley Steel Domain", 1)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SteelDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_STEEL_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SteelDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

    InsertDomainCodeSpecs(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SteelDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void SteelCategory::InsertDomainCategories(Dgn::DgnDbR db)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId SteelCategory::InsertCategory(Dgn::DgnDbR db, Utf8CP codeValue, Dgn::ColorDef const& color)
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
Dgn::DgnSubCategoryId SteelCategory::InsertSubCategory(Dgn::DgnDbR db, Dgn::DgnCategoryId categoryId, Utf8CP name, Dgn::ColorDef const& color)
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
Dgn::DgnCategoryId SteelCategory::QueryStructuralPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
    {
    Dgn::DgnCategoryId id = Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), categoryName));

    // Create it if is does not exist.

    if (!id.IsValid())
        {
        id = InsertCategory(db, categoryName, Dgn::ColorDef::White());
        }
    return id;
    }


END_BENTLEY_STEEL_NAMESPACE
