/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Domain/GridsDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GridsDomain.h"
#include <DrivingSurfaceHandler.h>
#include <GridSurfaceHandler.h>
#include <GridPlaneSurfaceHandler.h>
#include <PortionHandlers.h>
#include <IntersectionCurveHandlers.h>
#include <GridArcSurfaceHandler.h>
#include <GridSurfaceCreatesGridCurveHandler.h>
#include <DgnPlatform/DgnCategory.h>

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
    RegisterHandler (DrivingSurfaceHandler::GetHandler ());
    RegisterHandler (GridSurfaceHandler::GetHandler ());
    RegisterHandler (GridArcSurfaceHandler::GetHandler ());
    RegisterHandler (GridPlaneSurfaceHandler::GetHandler ());

    RegisterHandler (SurfaceSetHandler::GetHandler ());
    RegisterHandler (GridPortionHandler::GetHandler ());
    RegisterHandler (OrthogonalGridPortionHandler::GetHandler ());
    RegisterHandler (RadialGridPortionHandler::GetHandler ());
    RegisterHandler (SketchGridPortionHandler::GetHandler ());

    RegisterHandler (IntersectionCurveHandler::GetHandler ());
    RegisterHandler (GridCurveHandler::GetHandler ());
    RegisterHandler (GridLineHandler::GetHandler ());
    RegisterHandler (GridArcHandler::GetHandler ());
    RegisterHandler (GridSplineHandler::GetHandler ());


    RegisterHandler (GridSurfaceCreatesGridCurveHandler::GetHandler ());
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
    InsertCodeSpec (db, GRIDS_AUTHORITY_SurfaceSet);
    InsertCodeSpec (db, GRIDS_AUTHORITY_GridPortion);
    InsertCodeSpec (db, GRIDS_AUTHORITY_OrthogonalGridPortion);
    InsertCodeSpec (db, GRIDS_AUTHORITY_RadialGridPortion);
    InsertCodeSpec (db, GRIDS_AUTHORITY_SketchGridPortion);
    InsertCodeSpec (db, GRIDS_AUTHORITY_GridCurve);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas               04/2017
//---------------------------------------------------------------------------------------
void GridsDomain::EnsureDomainAuthoritiesExist (Dgn::DgnDbR db)
    {
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_SurfaceSet).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_SurfaceSet);
        }
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_GridPortion).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_GridPortion);
        }
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_OrthogonalGridPortion).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_OrthogonalGridPortion);
        }
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_RadialGridPortion).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_RadialGridPortion);
        }
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_SketchGridPortion).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_SketchGridPortion);
        }
    if (!db.CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_GridCurve).IsValid ())
        {
        InsertCodeSpec (db, GRIDS_AUTHORITY_GridCurve);
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
    InsertCategory (db, GRIDS_CATEGORY_CODE_DrivingSurface, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    tmpColorDef = Dgn::ColorDef::DarkBlue ();
    tmpColorDef.SetAlpha (0xaa);
    InsertCategory (db, GRIDS_CATEGORY_CODE_GridSurface, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    tmpColorDef = Dgn::ColorDef::DarkGrey ();
    InsertCategory (db, GRIDS_CATEGORY_CODE_IntersectionCurve, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    tmpColorDef = Dgn::ColorDef::Green ();
    DgnStyleId gridlineStyleId = getGridlineStyleId (db);
    InsertCategory (db, GRIDS_CATEGORY_CODE_GridCurve, &tmpColorDef, NULL, NULL, &bTRUE, &bTRUE, NULL, &gridlineStyleId, NULL, NULL, NULL);
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

    if (!(queryCategoryId (db, GRIDS_CATEGORY_CODE_DrivingSurface).IsValid ()))
        {
        tmpColorDef = Dgn::ColorDef::DarkGrey ();
        tmpColorDef.SetAlpha (0xaa);
        if (BSISUCCESS == InsertCategory (db, GRIDS_CATEGORY_CODE_DrivingSurface, &tmpColorDef, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
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