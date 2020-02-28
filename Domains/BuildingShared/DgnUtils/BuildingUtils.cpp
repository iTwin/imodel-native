/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "PublicApi/BuildingDgnUtilsApi.h"
#include <DgnPlatform/LineStyle.h>
#include <RoadRailPhysical\RoadRailPhysicalApi.h>
#include <RoadRailPhysical\RoadRailCategory.h>
#include <BuildingShared/Units/UnitConverter.h>

//TODO: these should NOT be here

#define ROADWAY_CENTERLINE_STYLE_NAME "RoadwayCenterline1"
#define BUILDING_SPACEPLANNING_CLASS_Building                                 "Building"
#define BUILDING_SPACEPLANNING_CLASS_Site                                     "Site"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_Story                        "Story"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_SharedStory                  "SharedStory"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_SpatialZone                  "SpatialZone"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_Space                        "Space"
#define BUILDING_SPACEPLANNING_SUBCATEGORY_CODE_SpatialElementLabels      "Labels"
#define SPATIALCOMPOSITION_CATEGORY_CODE_CompositeElement                 "CompositeElement"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_ConflictVolume               "ConflictVolume"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_PlanarShape                  "PlanarShape"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_Site                         "Site"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_BuildableVolume              "BuildableVolume"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_Building                     "Building"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_BoundedPlane                 "BoundedPlane"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_GraphicalDimension           "GraphicalDimension"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_UserDefinedParkingIsland     "UserDefinedParkingIsland"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_UserDefinedMarkup            "UserDefinedMarkup"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingArea                  "ParkingArea"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingLot                   "ParkingLot"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingIsland                "ParkingIsland"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_SpaceDivider                 "SpaceDivider"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_OpeningLocation              "OpeningLocation"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingSpace                 "ParkingSpace"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPath                   "EgressPath"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPathSection            "EgressPathSection"
#define BUILDING_SPACEPLANNING_SUBCATEGORY_CODE_MainEgressPath            "Main"
#define BUILDING_SPACEPLANNING_SUBCATEGORY_CODE_AlternativeEgressPath     "Alternative"
#define DEFAULT_BUILDING_CATEGORY_SELECTOR_NAME "DefaultBuildingCategories"
#define FLOOR_VIEW_CATEGORY_SELECTOR_NAME "FloorViewCategories"
#define SITE_VIEW_CATEGORY_SELECTOR_NAME "SiteViewCategories"
#define FLOOR_VIEW_3D_DISPLAY_STYLE_NAME "FloorView3dDisplayStyle"
#define PLANAR_VIEW_MODEL_SELECTOR_NAME "PlanarViewModelSelector"
#define BUILDING_VIEW_CATEGORY_SELECTOR_NAME "BuildingViewCategories"
#define EGRESS_VIEW_CATEGORY_SELECTOR_NAME "EgressPathCategories"
#define GRIDS_CATEGORY_CODE_GridSurface                         "GridSurface"
#define GRIDS_CATEGORY_CODE_GridCurve                           "GridCurve"

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BUILDING_SHARED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool BuildingUtils::TryExtractIndexFromName
(
Utf8CP name,
int& indexOut
)
    {
    Utf8String tmpStr (name);
    // unused - bool addDash = !tmpStr.empty ();
    // unused - int index = 0;
    size_t lastDash = tmpStr.find_last_of ('-');
    if (lastDash != Utf8String::npos)
        {
        if (Utf8String::Sscanf_safe(&tmpStr[lastDash], "-%d", &indexOut) == 1)
            {
            return true;
            }
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                03/2017
//---------------------------------------------------------------------------------------
int BuildingUtils::ExtractNameIndexAndTemplateString
(
Utf8String& tmpStr
)
    {
    bool addDash = !tmpStr.empty();
    int index = 0;
    size_t lastDash = tmpStr.find_last_of('-');
    if (lastDash != Utf8String::npos)
        {
        if (Utf8String::Sscanf_safe(&tmpStr[lastDash], "-%d", &index) == 1)
            {
            addDash = false;
            tmpStr.resize(lastDash + 1);
            }
        else
            {
            tmpStr.append("-");
            index = 0;
            }
        }
    else
        {
        size_t lastNonDigitIndex = tmpStr.find_last_not_of("0123456789");
        if (lastNonDigitIndex != Utf8String::npos && tmpStr.length() > (lastNonDigitIndex + 1))
            {
            if (Utf8String::Sscanf_safe(&tmpStr[lastNonDigitIndex + 1], "%d", &index) == 1)
                {
                tmpStr.resize(lastNonDigitIndex + 1);
                }
            }
        }

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr             BuildingUtils::GetECInstance
(
Dgn::DgnDbR db,
BeSQLite::EC::ECInstanceId instanceId,
Utf8CP ECSqlName
)
    {
    Utf8String ecsql ("SELECT * FROM ");
    ecsql.append (ECSqlName).append (" WHERE ECInstanceId=?");

    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (db, ecsql.c_str ());
    statement.BindId (1, instanceId);

    if (BeSQLite::DbResult::BE_SQLITE_ROW != statement.Step () && BeSQLite::DbResult::BE_SQLITE_DONE != statement.Step ())
        return nullptr;

    BeSQLite::EC::ECInstanceECSqlSelectAdapter sourceReader (statement);
    return sourceReader.GetInstance ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
CategorySelectorPtr GetCategorySelector (Utf8CP name, DgnDbR db)
    {
    DgnElementId cid = db.Elements ().QueryElementIdByCode (CategorySelector::CreateCode (db.GetDictionaryModel (), name));
    return db.Elements ().GetForEdit<CategorySelector> (cid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::CreateDefaultCategorySelector (DgnDbR db)
    {
    // A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    // We have to give the selector a unique name of its own.
    DgnCategoryIdSet categories;
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Site));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_BuildableVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), SPATIALCOMPOSITION_CATEGORY_CODE_CompositeElement));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_ConflictVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Story));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Space));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Building));
    //the 2 below would crash the bridge
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultDrawingCategoryId(db));
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultSpatialCategoryId(db));

    CategorySelector catSel (db.GetDictionaryModel (), DEFAULT_BUILDING_CATEGORY_SELECTOR_NAME);
    catSel.SetCategories (categories);

    auto insertedElement = db.Elements ().Insert (catSel);
    if (insertedElement.IsValid ())
        {
        return db.Elements ().GetForEdit<CategorySelector> (insertedElement->GetElementId ());
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                05/2017
//---------------------------------------------------------------------------------------
void addParkingCategories (DgnCategoryIdSet& categories, DgnDbR db)
    {
    categories.insert (SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingArea));
    categories.insert (SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingLot));
    categories.insert (SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingIsland));
    categories.insert (SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), BUILDING_SPACEPLANNING_CATEGORY_CODE_SpaceDivider));
    categories.insert (SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), BUILDING_SPACEPLANNING_CATEGORY_CODE_ParkingSpace));
    //the 2 below would crash the bridge
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultDrawingCategoryId(db));
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultSpatialCategoryId(db));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                05/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::CreateSiteViewCategorySelector (DgnDbR db)
    {
    DgnCategoryIdSet categories;
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Site));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_BuildableVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Building));
    //the 2 below would crash the bridge
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultDrawingCategoryId(db));
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultSpatialCategoryId(db));
    addParkingCategories (categories, db);
    categories.insert (RoadRailPhysical::RoadRailCategory::GetRoadway(db));

    CategorySelector catSel (db.GetDictionaryModel (), SITE_VIEW_CATEGORY_SELECTOR_NAME);
    catSel.SetCategories (categories);

    auto insertedElement = db.Elements ().Insert (catSel);
    if (insertedElement.IsValid ())
        {
        return db.Elements ().GetForEdit<CategorySelector> (insertedElement->GetElementId ());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas                06/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr BuildingUtils::CreateEgressPathViewCategorySelector(DgnDbR db)
    {
    DgnCategoryIdSet categories;
    categories.insert(SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Space));
    categories.insert(SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_OpeningLocation));
    categories.insert(SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPath));
    categories.insert(SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Story));
    categories.insert(SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPathSection));
    //the 2 below would crash the bridge
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultDrawingCategoryId(db));
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultSpatialCategoryId(db));

    CategorySelector catSel(db.GetDictionaryModel(), EGRESS_VIEW_CATEGORY_SELECTOR_NAME);
    catSel.SetCategories(categories);

    auto insertedElement = db.Elements().Insert(catSel);
    if (insertedElement.IsValid())
        {
        return db.Elements().GetForEdit<CategorySelector>(insertedElement->GetElementId());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas                06/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr BuildingUtils::GetEgressPathViewCategorySelector(Dgn::DgnDbR db)
    {
    CategorySelectorPtr catSel = GetCategorySelector(EGRESS_VIEW_CATEGORY_SELECTOR_NAME, db);
    if (!catSel.IsValid())
        catSel = CreateEgressPathViewCategorySelector(db);

    return catSel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nerijus.Jakeliunas                05/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::GetSiteViewCategorySelector (DgnDbR db)
    {
    CategorySelectorPtr catSel = GetCategorySelector (SITE_VIEW_CATEGORY_SELECTOR_NAME, db);
    if (!catSel.IsValid ())
        {
        catSel = CreateSiteViewCategorySelector (db);
        }

    return catSel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                05/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::CreateBuildingViewCategorySelector (DgnDbR db)
    {
    DgnCategoryIdSet categories;
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Story));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Building));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_GridSurface));
    //the 2 below would crash the bridge
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultDrawingCategoryId(db));
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultSpatialCategoryId(db));

    CategorySelector catSel (db.GetDictionaryModel (), BUILDING_VIEW_CATEGORY_SELECTOR_NAME);
    catSel.SetCategories (categories);

    auto insertedElement = db.Elements ().Insert (catSel);
    if (insertedElement.IsValid ())
        {
        return db.Elements ().GetForEdit<CategorySelector> (insertedElement->GetElementId ());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nerijus.Jakeliunas                09/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::GetBuildingCategorySelector (DgnDbR db)
    {
    CategorySelectorPtr catSel = GetCategorySelector (BUILDING_VIEW_CATEGORY_SELECTOR_NAME, db);
    if (!catSel.IsValid ())
        {
        catSel = CreateBuildingViewCategorySelector (db);
        }

    return catSel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::GetDefaultCategorySelector (DgnDbR db)
    {
    CategorySelectorPtr catSel = GetCategorySelector (DEFAULT_BUILDING_CATEGORY_SELECTOR_NAME, db);
    if (!catSel.IsValid ())
        {
        catSel = CreateDefaultCategorySelector (db);
        }

    return catSel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::CreateAndInsertFloorViewCategorySelector (DgnDbR db)
    {
    // A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    // We have to give the selector a unique name of its own.
    DgnCategoryIdSet categories;
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_ConflictVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Story));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Space));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_OpeningLocation));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPath));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPathSection));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_GridCurve));
    //the 2 below would crash the bridge
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultDrawingCategoryId(db));
    //categories.insert(DgnClientFx::DgnClientApp::App().GetDefaultSpatialCategoryId(db));

    CategorySelector catSel (db.GetDictionaryModel (), FLOOR_VIEW_CATEGORY_SELECTOR_NAME);
    catSel.SetCategories (categories);
    auto insertedElement = db.Elements ().Insert (catSel);
    if (insertedElement.IsValid ())
        {
        return db.Elements ().GetForEdit<CategorySelector> (insertedElement->GetElementId ());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
CategorySelectorPtr  BuildingUtils::GetFloorViewCategorySelector (DgnDbR db)
    {
    return GetCategorySelector (FLOOR_VIEW_CATEGORY_SELECTOR_NAME, db);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::CategorySelectorPtr BuildingUtils::GetOrCreateAndInsertFloorViewCategorySelector
(
    Dgn::DgnDbR db
)
    {
    CategorySelectorPtr catSel = GetFloorViewCategorySelector(db);
    if (!catSel.IsValid())
        {
        catSel = CreateAndInsertFloorViewCategorySelector(db);
        }

    return catSel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr  BuildingUtils::CreateFloorView3dDisplayStyle (DgnDbR db)
    {
    // DisplayStyle is a definition element that is normally shared by many ViewDefinitions.
    // We have to give the style a unique name of its own.
    DisplayStyle3d dstyle (db.GetDictionaryModel (), FLOOR_VIEW_3D_DISPLAY_STYLE_NAME);
    dstyle.SetBackgroundColor (ColorDef::White ());
    Render::ViewFlags viewFlags = dstyle.GetViewFlags ();
    viewFlags.SetRenderMode (Render::RenderMode::Wireframe);
    viewFlags.SetShowGrid (true);
    viewFlags.SetShowVisibleEdges (true);
    dstyle.SetViewFlags (viewFlags);

    dstyle.SetSkyBoxEnabled (false);
    dstyle.SetGroundPlaneEnabled (false);
    SetDisplayStyleOverrides(dstyle);
    auto insertedElement = db.Elements ().Insert (dstyle);
    if (insertedElement.IsValid ())
        {
        return db.Elements ().GetForEdit<DisplayStyle3d> (insertedElement->GetElementId ());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nerijus.Jakeliunas                09/2016
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr  BuildingUtils::GetFloorView3dDisplayStyle (DgnDbR db)
    {
    DgnElementId cid = db.Elements ().QueryElementIdByCode (DisplayStyle::CreateCode (db.GetDictionaryModel (), FLOOR_VIEW_3D_DISPLAY_STYLE_NAME));
    DisplayStyle3dPtr dstyle = db.Elements ().GetForEdit<DisplayStyle3d> (cid);
    if (!dstyle.IsValid ())
        {
        dstyle = CreateFloorView3dDisplayStyle (db);
        }

    return dstyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  04/2018
//---------------------------------------------------------------------------------------
void                BuildingUtils::SetDisplayStyleOverrides
(
Dgn::DisplayStyle3dR displaystyle
)
    {
    Dgn::DgnDbR db = displaystyle.GetDgnDb();
    Dgn::DgnSubCategory::Override genericGraphics3dOverride;
    genericGraphics3dOverride.SetColor(Dgn::ColorDef::White());
    Dgn::DgnSubCategory::Override genericGraphics2dOverride;
    genericGraphics2dOverride.SetColor(Dgn::ColorDef::Black());

    DefinitionModelR dictionary = db.GetDictionaryModel();
    DgnCode spatialCategoryCode = Dgn::SpatialCategory::CreateCode(dictionary, "Default3D");
    DgnCategoryId spatialCategoryId = DgnCategory::QueryCategoryId(db, spatialCategoryCode);
    DgnCode drawingCategoryCode = SpatialCategory::CreateCode(dictionary, "Default");
    DgnCategoryId drawingCategoryId = DgnCategory::QueryCategoryId(db, drawingCategoryCode);

    displaystyle.OverrideSubCategory(DgnCategory::GetDefaultSubCategoryId(drawingCategoryId), genericGraphics2dOverride);
    displaystyle.OverrideSubCategory(DgnCategory::GetDefaultSubCategoryId(spatialCategoryId), genericGraphics3dOverride);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                05/2017
//--------------+---------------+---------------+---------------+---------------+--------
DgnStyleId BuildingUtils::GetCenterLineStyleId (DgnDbR dgnDb)
    {
    LineStyleElementCPtr centerLineStyle = LineStyleElement::Get (dgnDb, ROADWAY_CENTERLINE_STYLE_NAME);
    if (centerLineStyle.IsValid ())
        {
        return DgnStyleId (centerLineStyle->GetElementId ().GetValue ());
        }

    LsComponentId componentId (LsComponentType::Internal, 2);
    DgnStyleId centerLineStyleId;
    dgnDb.LineStyles ().Insert (centerLineStyleId, ROADWAY_CENTERLINE_STYLE_NAME, componentId, 0, 1.0);

    return centerLineStyleId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                05/2017
//--------------+---------------+---------------+---------------+---------------+--------
CurveVectorPtr BuildingUtils::CreateRoadwayCurveVector (const bvector<DPoint3d>& points, double width)
    {
    CurveVectorPtr centerLine = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Open);

    CurveOffsetOptions offsetOptionsRight (width / 2);
    CurveVectorPtr roadVectorRight = centerLine->CloneOffsetCurvesXY (offsetOptionsRight);

    CurveOffsetOptions offsetOptionsLeft (-width / 2);
    CurveVectorPtr roadVectorLeft = centerLine->CloneOffsetCurvesXY (offsetOptionsLeft)->CloneReversed ();

    bvector<DPoint3d> roadVectorRightPoints = GeometryUtils::ExtractSingleCurvePoints (*roadVectorRight);
    bvector<DPoint3d> roadVectorLeftPoints = GeometryUtils::ExtractSingleCurvePoints (*roadVectorLeft);

    bvector<DPoint3d> roadVectorStartPoints = {
        roadVectorRightPoints.front (),
        roadVectorLeftPoints.back ()
        };
    ICurvePrimitivePtr roadVectorStartPrimitive = ICurvePrimitive::CreateLineString (roadVectorStartPoints);

    bvector<DPoint3d> roadVectorEndPoints = {
        roadVectorRightPoints.back (),
        roadVectorLeftPoints.front ()
        };
    ICurvePrimitivePtr roadVectorEndPrimitive = ICurvePrimitive::CreateLineString (roadVectorEndPoints);

    CurveVectorPtr roadVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    roadVector->AddPrimitives (*roadVectorRight);
    roadVector->Add (roadVectorEndPrimitive);
    roadVector->AddPrimitives (*roadVectorLeft);
    roadVector->Add (roadVectorStartPrimitive);

    return roadVector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
void BuildingUtils::AppendRoadwayGeometry (Dgn::DgnElementPtr roadway, CurveVectorPtr horizAlignment, double width)
    {
    bvector<DPoint3d> points = GeometryUtils::ExtractSingleCurvePoints (*horizAlignment);
    CurveVectorPtr roadVector = CreateRoadwayCurveVector (points, width);

    GeometrySourceP roadwayGeomElem = roadway->ToGeometrySourceP ();
    GeometryBuilderPtr roadwayGeomBuilder = GeometryBuilder::Create (*roadwayGeomElem);
    roadwayGeomBuilder->Append (*roadVector);
    Render::GeometryParams geomParams = roadwayGeomBuilder->GetGeometryParams ();
    // unused - Render::LineStyleInfoCP lineStyleCP = geomParams.GetLineStyle ();
    Render::LineStyleInfoPtr lineStyle = Render::LineStyleInfo::Create (GetCenterLineStyleId (roadway->GetDgnDb ()), nullptr);
    geomParams.SetLineStyle (lineStyle.get ());
    geomParams.SetWeight (1);
    ColorDef color (0xfe, 0xfe, 0xfe);
    geomParams.SetLineColor (color);
    roadwayGeomBuilder->Append (geomParams);
    roadwayGeomBuilder->Append (*horizAlignment);
    roadwayGeomBuilder->Finish (*roadwayGeomElem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus BuildingUtils::InsertElement(DgnElementPtr element)
    {
    if (!element.IsValid())
        return BentleyStatus::ERROR;

    DgnDbStatus status;
    element->Insert(&status);
    if (DgnDbStatus::Success != status)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
bool            BuildingUtils::CheckIfModelHasElements(Dgn::DgnModelP model)
    {
    ElementIterator itor = model->MakeIterator();

    return !itor.BuildIdList<DgnElementId>().empty();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
int BuildingUtils::ParseStringForInt(Utf8CP string)
    {
    char* converted;
    int result = strtol(string, &converted, 0);
    if ('\0' == *converted && converted != string)
        return result;
    else
        return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
double BuildingUtils::ParseStringForDouble(Utf8CP string)
    {
    char* converted;
    double result = strtod(string, &converted);
    if ('\0' == *converted && converted != string)
        return result;
    else
        return 0;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnElementId BuildingElementsUtils::GetElementIdByParentElementAuthorityAndName(DgnDbR db, Utf8CP authorityName, DgnElementId parentId, Utf8CP elementName)
    {
    DgnCode code(db.CodeSpecs().QueryCodeSpecId(authorityName), parentId, elementName);
    return db.Elements().QueryElementIdByCode(code);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator BuildingElementsUtils::MakeIterator
(
    Dgn::DgnElementCR element,
    ECN::ECClassId classId
)
    {
    DgnModelPtr subModel = element.GetSubModel();
    if (subModel.IsNull())
        return ElementIterator();

    ElementIterator elementIterator = subModel->MakeIterator("ECClassId=?");
    elementIterator.GetStatement()->BindId(2, classId);

    return elementIterator;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Wouter.Rombouts                 08/2016
//---------------------------------------------------------------------------------------
void BuildingElement_notifyFail(Utf8CP pOperation, Dgn::DgnElement& elm, Dgn::DgnDbStatus* stat)
    {
#ifdef NO_NOTIFICATION_MANAGER_IMODEL02
    if (stat)
        {
        if (*stat == Dgn::DgnDbStatus::LockNotHeld)
            {
            auto eid = elm.GetElementId();
            int64_t eidVal = 0;
            if (eid.IsValid())
                {
                eidVal = eid.GetValue();
                }
            Utf8String notify = Utf8PrintfString("Error> Operation %s Failed on Element:%I64u, (Label:\"%s\"), due to LockNotHeld!", pOperation, eidVal, elm.GetUserLabel());
            Dgn::NotifyMessageDetails nmd(Dgn::OutputMessagePriority::Error, notify.c_str());
            Dgn::NotificationManager::OutputMessage(nmd);
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonas.Valiunas   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DisplayLockFailedMessage
(
Dgn::DgnElementCR el,
BeSQLite::DbOpcode op,
Dgn::IBriefcaseManager::Response* pResponse
)
    {
#ifdef NO_NOTIFICATION_MANAGER_IMODEL02
    Dgn::RepositoryStatus status = pResponse ? pResponse->Result () : Dgn::RepositoryStatus::Success;
    Utf8String statusString;
    Utf8String additionalInfo = "";

    switch (status)
        {
    case RepositoryStatus::ServerUnavailable:
        statusString = "ServerUnavailable";
        break;
    case RepositoryStatus::LockAlreadyHeld:
        statusString = "LockAlreadyHeld";
        //extract who's using the element
        {
        Dgn::DgnLockInfoSet const& lockStates = pResponse->LockStates ();
        if (lockStates.size () > 0)
            {
            Dgn::DgnLockInfo lockInfo = *lockStates.begin ();
            Dgn::DgnLockOwnershipCR lockOwner = lockInfo.GetOwnership ();

            lockOwner.GetLockLevel ();
            }
        else
            additionalInfo += " Owner is not defined";

        break;
        }

        break;
    case RepositoryStatus::SyncError:
        statusString = "SyncError";
        break;
    case RepositoryStatus::InvalidResponse:
        statusString = "InvalidResponse";
        break;
    case RepositoryStatus::PendingTransactions:
        statusString = "PendingTransactions";
        break;
    case RepositoryStatus::LockUsed:
        statusString = "LockUsed";
        break;
    case RepositoryStatus::CannotCreateRevision:
        statusString = "CannotCreateRevision";
        break;
    case RepositoryStatus::InvalidRequest:
        statusString = "InvalidRequest";
        break;
    case RepositoryStatus::RevisionRequired:
        statusString = "RevisionRequired";
        break;
    case RepositoryStatus::CodeUnavailable:
        statusString = "CodeUnavailable";
        break;
    case RepositoryStatus::CodeNotReserved:
        statusString = "CodeNotReserved";
        break;
    case RepositoryStatus::CodeUsed:
        statusString = "CodeUsed";
        break;
    case RepositoryStatus::LockNotHeld:
        statusString = "LockNotHeld";
        break;
    case RepositoryStatus::RepositoryIsLocked:
        statusString = "RepositoryIsLocked";
        break;
        }

    Utf8String notify = Utf8PrintfString ("Error> acquire lock Failed on Element:%I64u, (Label:\"%s\"), due to \"%s\"", el.GetElementId ().GetValue(), el.GetUserLabel (), statusString.c_str()).append(additionalInfo);
    Dgn::NotifyMessageDetails nmd (Dgn::OutputMessagePriority::Error, notify.c_str ());
    Dgn::NotificationManager::OutputMessage (nmd);
#endif
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DefinitionPartitionCPtr BuildingUtils::CreateDefinitionPartition
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    DgnCode partitionCode = GetDefinitionPartitionCode (db, partitionName);
    DefinitionPartitionPtr partitionPtr = DefinitionPartition::Create (*db.Elements ().GetRootSubject (), partitionName);
    partitionPtr->Insert ();
    return partitionPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DefinitionPartitionCPtr BuildingUtils::GetDefinitionPartition
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    DgnCode partitionCode = GetDefinitionPartitionCode (db, partitionName);
    DgnElementId partitionId = db.Elements ().QueryElementIdByCode (partitionCode);
    return db.Elements ().Get<DefinitionPartition> (partitionId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DgnCode BuildingUtils::GetDefinitionPartitionCode
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    return DefinitionPartition::CreateCode (*db.Elements ().GetRootSubject (), partitionName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DefinitionPartitionCPtr BuildingUtils::GetOrCreateDefinitionPartition
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    DefinitionPartitionCPtr partition = GetDefinitionPartition (db, partitionName);
    if (partition.IsValid ())
        return partition;

    partition = CreateDefinitionPartition (db, partitionName);
    return partition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DefinitionModelPtr BuildingUtils::CreateDefinitionModel
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    DefinitionPartitionCPtr partition = GetOrCreateDefinitionPartition (db, partitionName);
    DefinitionModelPtr definitionModel = DefinitionModel::Create (*partition);
    Dgn::IBriefcaseManager::Request definitionReq;
    db.BriefcaseManager ().PrepareForModelInsert (definitionReq, *definitionModel, Dgn::IBriefcaseManager::PrepareAction::Acquire);
    definitionModel->Insert ();
    return definitionModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DefinitionModelCPtr BuildingUtils::GetDefinitionModel
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    DgnModelId modelId = db.Models ().QuerySubModelId (GetDefinitionPartitionCode (db, partitionName));
    return db.Models ().Get<DefinitionModel> (modelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2017
//--------------+---------------+---------------+---------------+---------------+--------
Dgn::DefinitionModelCPtr BuildingUtils::GetOrCreateDefinitionModel
(
    Dgn::DgnDbR db,
    Utf8CP partitionName
)
    {
    DefinitionModelCPtr model = GetDefinitionModel (db, partitionName);
    if (model.IsValid ())
        return model;

    model = CreateDefinitionModel (db, partitionName);
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                03/2017
//---------------------------------------------------------------------------------------
Dgn::DgnCode            BuildingUtils::GetUniqueElementCode
(
    Dgn::DgnDbR db,
    CodeSpecId codeSpecId,
    Utf8String nameTemplate,
    Dgn::DgnElementId modeledElementId,
    int startIndex
)
    {
    Utf8String uniqueName;

    int index = startIndex;
    uniqueName.Sprintf(nameTemplate.c_str(), index++);

    DgnCode code = DgnCode(codeSpecId, modeledElementId, uniqueName);

    while (db.Elements().QueryElementIdByCode(code).IsValid())
        {
        uniqueName.Sprintf(nameTemplate.c_str(), index++);
        code = DgnCode(codeSpecId, modeledElementId, uniqueName);
        }
    return code;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis           03/2017
//---------------------------------------------------------------------------------------
Dgn::DgnCode            BuildingUtils::GetNamedElementCode
(
    Dgn::DgnDbR db,
    CodeSpecId codeSpecId,
    Utf8String name,
    Dgn::DgnElementId modeledElementId,
    int startIndex
)
    {
    DgnCode code = DgnCode(codeSpecId, modeledElementId, name);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int index = startIndex;
    Utf8String modifiedName = GetTemplateFromName(name, index);

    return GetUniqueElementCode(db, codeSpecId, modifiedName, modeledElementId, index);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis           03/2017
//---------------------------------------------------------------------------------------
Utf8String BuildingUtils::GetTemplateFromName
(
    Utf8String name,
    int &index
)
    {
    Utf8String suffix;
    auto pos = name.find_last_of('-');
    bool isNumber = false;
    if (pos != std::string::npos && pos < (name.length() - 1))
        {
        suffix = name.substr(++pos);
        isNumber = true;
        for (char& c : suffix)
            {
            if (!isdigit(c))
                isNumber = false;
            }
        }
    Utf8String modifiedName = name + "-%d";
    if (isNumber)
        {
        index = std::stoi(suffix.c_str(), nullptr, 10);
        modifiedName = modifiedName.substr(0, pos) + "%d";
        }
    return modifiedName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis           06/2017
//---------------------------------------------------------------------------------------
Dgn::DgnModelCR BuildingUtils::GetGroupInformationModel(Dgn::DgnDbR db)
    {
    return *db.Models().GetModel(db.Models().QuerySubModelId(Dgn::DefinitionPartition::CreateCode(*db.Elements().GetRootSubject(), BUILDING_InformationPartition)));
    }

Dgn::DgnElementId ElementIdIteratorEntry::GetElementId() const { return m_statement->GetValueId<Dgn::DgnElementId>(0); }

END_BUILDING_SHARED_NAMESPACE

