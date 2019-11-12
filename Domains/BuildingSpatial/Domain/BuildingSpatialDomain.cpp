/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BuildingSpatial/Domain/BuildingSpatialDomain.h>
#include <BuildingSpatial/Handlers/SpaceHandler.h>
#include <BuildingSpatial/Handlers/BuildingHandler.h>
#include <BuildingSpatial/Handlers/RegularStoryHandler.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BUILDINGSPATIAL_NAMESPACE

//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(BuildingSpatialDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                 02/2019
//---------------------------------------------------------------------------------------
BuildingSpatialDomain::BuildingSpatialDomain () : DgnDomain(BUILDINGSPATIAL_SCHEMA_NAME, "BuildingBuildingSpatial Domain", 1)
    {
    RegisterHandler(SpaceHandler::GetHandler());
    RegisterHandler(BuildingHandler::GetHandler());
    RegisterHandler(RegularStoryHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                 02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialDomain::InsertDomainAuthorities (DgnDbR db) const
    {
    InsertCodeSpec (db, BUILDINGSPATIAL_AUTHORITY_Building);
    InsertCodeSpec (db, BUILDINGSPATIAL_AUTHORITY_Space);
    InsertCodeSpec (db, BUILDINGSPATIAL_AUTHORITY_RegularStory);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                 02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialDomain::InsertCodeSpec (DgnDbR db, Utf8CP name)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create (db, name);
    BeAssert (codeSpec.IsValid ());
    if (codeSpec.IsValid ())
        {
        codeSpec->Insert ();
        BeAssert (codeSpec->GetCodeSpecId ().IsValid ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas                 10/2016
//---------------------------------------------------------------------------------------
Dgn::DgnSubCategoryId insertSubCategory(Dgn::DgnDbR db, Dgn::DgnCategoryId categoryId, Dgn::ColorDefR color, Utf8CP name, bool isSnappable = false)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    appearance.SetInvisible(false);
    appearance.SetDontSnap(!isSnappable);


    Dgn::DgnSubCategoryPtr newSubCategory = new Dgn::DgnSubCategory(Dgn::DgnSubCategory::CreateParams(db, categoryId, name, appearance));
    if (!newSubCategory.IsValid())
        {
        return Dgn::DgnSubCategoryId();
        }

    Dgn::DgnSubCategoryCPtr insertedSubCategory = newSubCategory->Insert();
    if (!insertedSubCategory.IsValid())
        {
        return Dgn::DgnSubCategoryId();
        }

    return insertedSubCategory->GetSubCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                   02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialDomain::_OnSchemaImported(DgnDbR db) const
    {
    InsertDomainAuthorities (db);
    Dgn::ColorDef tmpColorDef = Dgn::ColorDef::White ();

    Dgn::DgnSubCategory::Appearance buildingLevelAppearance;
    buildingLevelAppearance.SetColor(tmpColorDef);
    Dgn::SpatialCategory dgnBuildingCat(db.GetDictionaryModel (), BUILDINGSPATIAL_CATEGORY_CODE_Building, Dgn::DgnCategory::Rank::Domain);
    dgnBuildingCat.Insert(buildingLevelAppearance);

    Dgn::DgnSubCategory::Appearance spaceLevelAppearance;
    spaceLevelAppearance.SetColor(tmpColorDef);
    spaceLevelAppearance.SetWeight(1);
    Dgn::SpatialCategory dgnSpaceCat(db.GetDictionaryModel (), BUILDINGSPATIAL_CATEGORY_CODE_Space, Dgn::DgnCategory::Rank::Domain);
    dgnSpaceCat.Insert(spaceLevelAppearance);
    tmpColorDef = Dgn::ColorDef::Black();
    insertSubCategory(db, Dgn::SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_Space), tmpColorDef, BUILDINGSPATIAL_SUBCATEGORY_CODE_SpatialElementLabels);

    tmpColorDef = Dgn::ColorDef::White();
    Dgn::DgnSubCategory::Appearance regularStoryAppearance;
    regularStoryAppearance.SetColor(tmpColorDef);
    regularStoryAppearance.SetWeight(1);
    Dgn::SpatialCategory regularStoryCategory(db.GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_RegularStory, Dgn::DgnCategory::Rank::Domain);
    regularStoryCategory.Insert(regularStoryAppearance);
    
    insertSubCategory(db, Dgn::SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_RegularStory), tmpColorDef, BUILDINGSPATIAL_SUBCATEGORY_CODE_SpatialElementLabels);
    }

END_BUILDINGSPATIAL_NAMESPACE
