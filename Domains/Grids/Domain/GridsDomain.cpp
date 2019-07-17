/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Domain/GridsDomain.h>
#include <Grids/Elements/GridElementsAPI.h>
#include <Grids/Handlers/GridHandlersAPI.h>
#include <DgnPlatform/DgnCategory.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

#define GRIDLINE_STYLE_NAME "GridlineStyle1"

USING_NAMESPACE_BENTLEY_DGN

USING_NAMESPACE_GRIDS



//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(GridsDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
GridsDomain::GridsDomain () : DgnDomain(GRIDS_SCHEMA_NAME, "Grids Domain", 1)
    {
    RegisterHandler(GridCurvesSetHandler::GetHandler());

    RegisterHandler(PlanCircumferentialGridSurfaceHandler::GetHandler());
    RegisterHandler(ElevationGridSurfaceHandler::GetHandler ());
    RegisterHandler(SketchLineGridSurfaceHandler::GetHandler ());
    RegisterHandler(SketchArcGridSurfaceHandler::GetHandler());
    RegisterHandler(PlanCartesianGridSurfaceHandler::GetHandler ());
    RegisterHandler(PlanRadialGridSurfaceHandler::GetHandler());
    RegisterHandler(SketchSplineGridSurfaceHandler::GetHandler());

    RegisterHandler(OrthogonalGridHandler::GetHandler ());
    RegisterHandler(RadialGridHandler::GetHandler ());
    RegisterHandler(SketchGridHandler::GetHandler ());
    RegisterHandler(ElevationGridHandler::GetHandler ());

    RegisterHandler(OrthogonalAxisXHandler::GetHandler ());
    RegisterHandler(OrthogonalAxisYHandler::GetHandler());
    RegisterHandler(CircularAxisHandler::GetHandler());
    RegisterHandler(RadialAxisHandler::GetHandler());
    RegisterHandler(GeneralGridAxisHandler::GetHandler());

    RegisterHandler(GeneralGridCurveHandler::GetHandler());
    RegisterHandler(GridLineHandler::GetHandler ());
    RegisterHandler(GridArcHandler::GetHandler ());
    RegisterHandler(GridSplineHandler::GetHandler ());

    RegisterHandler(GridCurveBundleHandler::GetHandler());
    RegisterHandler(GridSurfaceDrivesGridCurveBundleHandler::GetHandler());
    RegisterHandler(GridCurveBundleCreatesGridCurveHandler::GetHandler());

    RegisterHandler(GridDrivesGridSurfaceHandler::GetHandler());
    RegisterHandler(GridLabelHandler::GetHandler ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
GridsDomain::~GridsDomain ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    NerijusJakeliunas               05/2017
//---------------------------------------------------------------------------------------
DgnStyleId getGridlineStyleId (DgnDbR dgnDb)
    {
    LineStyleElementCPtr gridlineStyle = LineStyleElement::Get (dgnDb, GRIDLINE_STYLE_NAME);
    if (gridlineStyle.IsValid ())
        {
        return DgnStyleId (gridlineStyle->GetElementId ().GetValue ());
        }

    LsComponentId componentId (LsComponentType::Internal, 7);
    DgnStyleId gridLineStyleId;
    dgnDb.LineStyles ().Insert (gridLineStyleId, GRIDLINE_STYLE_NAME, componentId, LSATTR_UNITDEV | LSATTR_NOSNAP, 1.0);

    return gridLineStyleId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 08/2016
//---------------------------------------------------------------------------------------
StatusInt           GridsDomain::InsertCategory
(
Dgn::DgnDbR dgnDb,
Utf8CP name,
Dgn::ColorDef* pColor,
bool* pIsVisible,
bool* pIsPlotted,
bool* pIsSnappable,
bool* pIsLocatabled,
uint32_t* pWeight,
Dgn::DgnStyleId* pStyleId,
int32_t* pDisplayPriority,
Dgn::RenderMaterialId* pMaterialId,
double* pTransparency
)
    {
    //Dgn::DgnCategory dgnCat(Dgn::DgnCategory::CreateParams(dgnDb, name, Dgn::DgnCategory::Scope::Physical, Dgn::DgnCategory::Rank::Domain));
    Dgn::SpatialCategory dgnCat (dgnDb.GetDictionaryModel (), name, Dgn::DgnCategory::Rank::Domain);

    Dgn::DgnSubCategory::Appearance levelAppearance;
    if (pColor)
        levelAppearance.SetColor (*pColor);
    if (pIsVisible)
        levelAppearance.SetInvisible (!(*pIsVisible));
    if (pIsPlotted)
        levelAppearance.SetDontPlot (!(*pIsPlotted));
    if (pIsSnappable)
        levelAppearance.SetDontSnap (!(*pIsSnappable));

    if (pIsLocatabled)
        levelAppearance.SetDontLocate (!(*pIsLocatabled));
    if (pWeight)
        levelAppearance.SetWeight (*pWeight);
    if (pStyleId)
        levelAppearance.SetStyle (*pStyleId);
    if (pDisplayPriority)
        levelAppearance.SetDisplayPriority (*pDisplayPriority);
    if (pMaterialId)
        levelAppearance.SetRenderMaterial (*pMaterialId);
    if (pTransparency)
        levelAppearance.SetTransparency (*pTransparency);

    if (!dgnCat.Insert (levelAppearance).IsValid ())
        return BSIERROR;

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 08/2016
//---------------------------------------------------------------------------------------
void GridsDomain::InsertDomainAuthorities (DgnDbR db) const
    {
    InsertCodeSpec(db, GRIDS_AUTHORITY_Grid);
    InsertCodeSpec(db, GRIDS_AUTHORITY_GridCurve);
    InsertCodeSpec(db, GRIDS_AUTHORITY_GridCurvesSet);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas               04/2017
//---------------------------------------------------------------------------------------
void GridsDomain::EnsureDomainAuthoritiesExist (Dgn::DgnDbR db)
    {
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_Grid).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_Grid);
        }
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_GridCurve).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_GridCurve);
        }
    if (!db.CodeSpecs().QueryCodeSpecId(GRIDS_AUTHORITY_GridCurvesSet).IsValid())
        {
        InsertCodeSpec(db, GRIDS_AUTHORITY_GridCurvesSet);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
void GridsDomain::InsertCodeSpec (DgnDbR db, Utf8CP name)
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
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
void GridsDomain::_OnSchemaImported(DgnDbR db) const
    {
    Dgn::ColorDef tmpColorDef;
    double tmpTransparency;
    bool bTRUE = true;
    bool bFALSE = false;

    //method        |   db  |               name                        |      color       |    isVisible   |   isPlotted   |   isSnappable |   isLocatable |   weight  |   styleId |   displaypriority |      materialId      |    transparency    |
    tmpColorDef = Dgn::ColorDef::DarkGrey ();
    tmpColorDef.SetAlpha (0xaa);

    InsertCategory (db, GRIDS_CATEGORY_CODE_Uncategorized, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    tmpColorDef = Dgn::ColorDef::DarkBlue ();
    tmpColorDef.SetAlpha (0xaa);
    InsertCategory (db, GRIDS_CATEGORY_CODE_GridSurface, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    tmpColorDef = Dgn::ColorDef::Green ();
    DgnStyleId gridlineStyleId = getGridlineStyleId (db);
    InsertCategory (db, GRIDS_CATEGORY_CODE_GridCurve, &tmpColorDef, NULL, NULL, &bTRUE, &bTRUE, NULL, &gridlineStyleId, NULL, NULL, NULL);

    // Insert GroupInformationModel
    SubjectCPtr rootSubject = db.Elements().GetRootSubject(); // TODO: May want to change this
    DgnCode code = DefinitionPartition::CreateCode(*rootSubject, BUILDING_InformationPartition);
    DgnModelId definitionModelId = db.Models().QuerySubModelId(code);
    if (!definitionModelId.IsValid())
        {
        DefinitionPartitionPtr partition = DefinitionPartition::Create(*rootSubject, BUILDING_InformationPartition);
        BeAssert(partition.IsValid());
        DgnElementCPtr iPartition = partition->Insert();
        BeAssert(iPartition.IsValid());
        DefinitionModelPtr model = DefinitionModel::Create(*partition);

        IBriefcaseManager::Request request;
        auto stat = model->PopulateRequest(request, BeSQLite::DbOpcode::Insert);
        model->GetDgnDb().BriefcaseManager().Acquire(request);

        DgnDbStatus status = model->Insert();
        BeAssert(status == DgnDbStatus::Success);
        }

    InsertDomainAuthorities (db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas               04/2017
//---------------------------------------------------------------------------------------
static DgnCategoryId queryCategoryId (DgnDbR db, Utf8CP categoryName)
    {
    return SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), categoryName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas               04/2017
//---------------------------------------------------------------------------------------
bool GridsDomain::EnsureDomainCategoriesExist (Dgn::DgnDbR db)
    {
    bool itemInserted = false;
    Dgn::ColorDef tmpColorDef;
    double tmpTransparency;
    bool bTRUE = true;
    bool bFALSE = false;

    if (!(queryCategoryId (db, GRIDS_CATEGORY_CODE_GridCurve).IsValid ()))
        {
        tmpColorDef = Dgn::ColorDef::Green();
        tmpColorDef.SetAlpha (0xaa);
        if (BSISUCCESS == InsertCategory (db, GRIDS_CATEGORY_CODE_GridCurve, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
            {
            itemInserted = true;
            }
        }

    if (!(queryCategoryId (db, GRIDS_CATEGORY_CODE_GridSurface).IsValid ()))
        {
        tmpColorDef = Dgn::ColorDef::DarkBlue ();
        tmpColorDef.SetAlpha (0xaa);
        if (BSISUCCESS == InsertCategory (db, GRIDS_CATEGORY_CODE_GridSurface, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
            {
            itemInserted = true;
            }
        }

    return itemInserted;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  03/2017
//---------------------------------------------------------------------------------------
bool GridsDomain::EnsureECSchemaIsLoaded (Dgn::DgnDbR db)
    {
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
void GridsDomain::_OnDgnDbOpened(DgnDbR db) const
    {

    }