/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/BuildingUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/utilsApi.h"

//#include "PublicApi/BuildingUtils.h"
#include <DgnView/DgnTool.h>
#include <dgnPlatform/ViewController.h>
#include <DgnView/AccuSnap.h>
//#include <ConstraintSystem/Domain/ConstraintModelMacros.h>
//#include "PublicApi\GeometryUtils.h"
#include <DgnPlatform/LineStyle.h>
#include <RoadRailPhysical\RoadRailPhysicalApi.h>
#include <RoadRailPhysical\RoadRailCategory.h>
#include <BuildingShared/Units/UnitConverter.h>

//TODO: these should NOT be here

#define ROADWAY_CENTERLINE_STYLE_NAME "RoadwayCenterline1"
#define BUILDING_SPACEPLANNING_CLASS_Building                                 "Building"
#define BUILDING_SPACEPLANNING_CLASS_Site                                     "Site"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_AllocatedVolume              "AllocatedVolume"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_Story                        "Story"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_SharedStory                  "SharedStory"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_SpatialZone                  "SpatialZone"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_Space                        "Space"
#define BUILDING_SPACEPLANNING_SUBCATEGORY_CODE_SpatialElementLabels      "Labels"
#define BUILDING_SPACEPLANNING_CATEGORY_CODE_ProjectDefinition            "ProjectDefinition"
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
    bool addDash = !tmpStr.empty ();
    int index = 0;
    size_t lastDash = tmpStr.find_last_of ('-');
    if (lastDash != Utf8String::npos)
        {
        if (BE_STRING_UTILITIES_UTF8_SSCANF (&tmpStr[lastDash], "-%d", &indexOut) == 1)
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
        if (BE_STRING_UTILITIES_UTF8_SSCANF(&tmpStr[lastDash], "-%d", &index) == 1)
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
            if (BE_STRING_UTILITIES_UTF8_SSCANF(&tmpStr[lastNonDigitIndex + 1], "%d", &index) == 1)
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
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_AllocatedVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_ConflictVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Story));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_SpatialZone));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Space));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Building));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_BoundedPlane));

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
    addParkingCategories (categories, db);
    categories.insert (RoadRailPhysical::RoadRailCategory::GetRoad (db));

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
CategorySelectorPtr  BuildingUtils::CreateFloorViewCategorySelector (DgnDbR db)
    {
    // A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    // We have to give the selector a unique name of its own.
    DgnCategoryIdSet categories;
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_ConflictVolume));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Story));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_SpatialZone));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_Space));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_OpeningLocation));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPath));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), BUILDING_SPACEPLANNING_CATEGORY_CODE_EgressPathSection));
    categories.insert (SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), GRIDS_CATEGORY_CODE_GridCurve));

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
    CategorySelectorPtr catSel = GetCategorySelector (FLOOR_VIEW_CATEGORY_SELECTOR_NAME, db);
    if (!catSel.IsValid ())
        {
        catSel = CreateFloorViewCategorySelector (db);
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
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
BentleyStatus    BuildingUtils::ExtractHitListFromPoint(HitListP& hitList, DgnViewportP vp, DPoint3d point)
    {
    ElementPicker&  picker = ElementLocateManager::GetManager().GetElementPicker();
    AccuSnap& snap = AccuSnap::GetInstance();

    LocateOptions   options = ElementLocateManager::GetManager().GetLocateOptions();
    options.SetHitSource(HitSource::DataPoint);

    bool wasAborted;
    double locateFactor = snap.GetSettings().searchDistance;
    double aperture = (locateFactor * vp->PixelsFromInches(ElementLocateManager::GetManager().GetApertureInches()) / 2.0) + 1.5;

    int hitCount = picker.DoPick(&wasAborted, *vp, point, aperture, nullptr, options);

    if (0 == hitCount)
        return BentleyStatus::ERROR;

    hitList = picker.GetHitList(true);
    return BentleyStatus::SUCCESS;
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

    bvector<DPoint3d> roadVectorRightPoints = GeometryUtils::ExtractSingleCurvePoints (roadVectorRight);
    bvector<DPoint3d> roadVectorLeftPoints = GeometryUtils::ExtractSingleCurvePoints (roadVectorLeft);

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
    bvector<DPoint3d> points = GeometryUtils::ExtractSingleCurvePoints (horizAlignment);
    CurveVectorPtr roadVector = CreateRoadwayCurveVector (points, width);

    GeometrySourceP roadwayGeomElem = roadway->ToGeometrySourceP ();
    GeometryBuilderPtr roadwayGeomBuilder = GeometryBuilder::Create (*roadwayGeomElem);
    roadwayGeomBuilder->Append (*roadVector);
    Render::GeometryParams geomParams = roadwayGeomBuilder->GetGeometryParams ();
    Render::LineStyleInfoCP lineStyleCP = geomParams.GetLineStyle ();
    Render::LineStyleInfoPtr lineStyle = Render::LineStyleInfo::Create (GetCenterLineStyleId (roadway->GetDgnDb ()), nullptr);
    geomParams.SetLineStyle (lineStyle.get ());
    geomParams.SetWeight (1);
    ColorDef color (0xfe, 0xfe, 0xfe);
    geomParams.SetLineColor (color);
    roadwayGeomBuilder->Append (geomParams);
    roadwayGeomBuilder->Append (*horizAlignment);
    roadwayGeomBuilder->Finish (*roadwayGeomElem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mykolas.Simutis                 09/2016
//--------------------------------------------------------------------------------------
void BuildingElementsUtils::AppendTableCell (JsonValueR jsonArr, Json::Value &&key, Json::Value &&value)
    {
    Json::Value cell;
    cell["key"] = key;
    cell["value"] = value;
    jsonArr.append (cell);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus BuildingUtils::InsertElement(DgnElementPtr element)
    {
    if (!element.IsValid() ||
        Dgn::RepositoryStatus::Success != BuildingLocks_LockElementForOperation(*element, BeSQLite::DbOpcode::Insert, "Insert element"))
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

END_BUILDING_SHARED_NAMESPACE

