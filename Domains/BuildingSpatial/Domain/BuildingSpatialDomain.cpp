/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/BuildingSpatialDomain.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingSpatial/Domain/BuildingSpatialDomain.h>
#include <BuildingSpatial/Handlers/SpaceHandler.h>
#include <BuildingSpatial/Handlers/BuildingHandler.h>
#include <BuildingSpatial/Handlers/ElevationStoryHandler.h>

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
    RegisterHandler(ElevationStoryHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                 02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialDomain::InsertDomainAuthorities (DgnDbR db) const
    {
    InsertCodeSpec (db, BUILDINGSPATIAL_AUTHORITY_Building);
    InsertCodeSpec (db, BUILDINGSPATIAL_AUTHORITY_Space);
    InsertCodeSpec (db, BUILDINGSPATIAL_AUTHORITY_ElevationStory);
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

    Dgn::DgnSubCategory::Appearance elevationStoryAppearance;
    elevationStoryAppearance.SetColor(tmpColorDef);
    elevationStoryAppearance.SetWeight(1);
    Dgn::SpatialCategory elevationStoryCategory(db.GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_ElevationStory, Dgn::DgnCategory::Rank::Domain);
    elevationStoryCategory.Insert(elevationStoryAppearance);
    }

END_BUILDINGSPATIAL_NAMESPACE
