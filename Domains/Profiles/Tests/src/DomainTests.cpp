/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/DomainTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesTestCase.h"
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct DomainTestCase : ProfilesTestCase
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, EnsureDomainsAreRegistered)
    {
    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (ProfilesDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_TRUE (BentleyStatus::SUCCESS == registrationStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bssimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, EnsureProfilesDomainIsPresentInBim)
    {
    DgnDomainCP profilesDomain = GetDb().Domains().FindDomain (ProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE (NULL != profilesDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, ValidateSchema)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext (true, true);
    context->AddSchemaLocater (GetDb().GetSchemaLocater());

    ECN::SchemaKey refKey (PRF_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema (refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE (refSchema.IsValid());

    ASSERT_TRUE (refSchema->Validate());
    }

TEST_F(DomainTestCase, IshapeGraphics)
    {
    PhysicalElementPtr el = GenericPhysicalObject::Create(GetPhysicalModel(), GetCategoryId());
    el->SetUserLabel("Petras");

    //test geometry
    Placement3d placement;
    placement.GetOriginR() = DPoint3d::From(10.0, 10.0, -50.0);
    el->SetPlacement(placement);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*el->ToGeometrySourceP());

    GeometrySourceP geomElem = el->ToGeometrySourceP();
    geomElem->SetCategoryId(el->GetCategoryId());

    builder->Append(el->GetCategoryId(), GeometryBuilder::CoordSystem::World);

    Dgn::Render::GeometryParams params;

    DgnCategoryId c = el->GetCategoryId();

    params.SetCategoryId(el->GetCategoryId());
    params.SetFillDisplay(Render::FillDisplay::Always);
    params.SetLineColor(ColorDef::Red());
    params.SetFillColor(ColorDef::Green());
    params.SetWeight(1);
    builder->Append(params, GeometryBuilder::CoordSystem::World);

    IGeometryPtr blockGeom = ProfilesGeomApi::CreateIShape(30, 50, 10, 10);

    builder->Append(*blockGeom, GeometryBuilder::CoordSystem::World);
    builder->Finish(*el->ToGeometrySourceP());

    DgnDbStatus status;
    el->Insert(&status);
    ASSERT_TRUE(status == DgnDbStatus::Success);


    //create a view, it is neccessary if you like to see geoemetry with Gist
    DefinitionModelR dictionary = GetDb().GetDictionaryModel();
    CategorySelectorPtr categorySelector = new CategorySelector(dictionary, "Default");

    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "Default");
    modelSelector->AddModel(GetPhysicalModel().GetModelId());

    DisplayStyle3dPtr displayStyle = new DisplayStyle3d(dictionary, "Default");

    displayStyle->SetBackgroundColor(ColorDef::DarkYellow());
    displayStyle->SetSkyBoxEnabled(false);
    displayStyle->SetGroundPlaneEnabled(false);

    Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SolidFill);
    viewFlags.SetShowTransparency(true);
    viewFlags.ShowTransparency();

    displayStyle->SetViewFlags(viewFlags);

    //create view 
    OrthographicViewDefinition view(dictionary, "Structure View", *categorySelector, *displayStyle, *modelSelector);
    view.SetStandardViewRotation(StandardView::Iso); // Default to a rotated view
    view.LookAtVolume(GetDb().GeoLocation().GetProjectExtents());
    view.Insert();
    DgnViewId viewId = view.GetViewId();
    GetDb().SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    }
