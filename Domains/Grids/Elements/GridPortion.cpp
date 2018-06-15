/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridPortion.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/BuildingSharedApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(Grid)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(ElevationGrid)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(PlanGrid)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Grid::Grid
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {
    if (!m_categoryId.IsValid () && m_classId.IsValid()) //really odd tests in platform.. attempts to create elements with 0 class id and 0 categoryId. NEEDS WORK: PLATFORM
        {
        Dgn::DgnCategoryId catId = SpatialCategory::QueryCategoryId (params.m_dgndb.GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);
        if (catId.IsValid ())
            DoSetCategoryId (catId);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Grid::CreateParams           Grid::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnElement::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId);

    return CreateParams(createParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     12/17
//---------------------------------------------------------------------------------------
PlanGrid::PlanGrid
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid()) // elements created via handler have no classid.
        {
        SetDefaultStartElevation(params.m_defaultStartElevation);
        SetDefaultEndElevation(params.m_defaultEndElevation);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     09/17
//---------------------------------------------------------------------------------------
DPlane3d PlanGrid::GetPlane 
(
) const
    {
    DPlane3d plane;
    plane.InitFromOriginAndNormal(0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    YawPitchRollAngles angles = GetPlacement().GetAngles ();
    RotMatrix rotMatrix = angles.ToRotMatrix ();
    rotMatrix.Multiply (plane.normal);
    return plane;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/17
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator Grid::MakeIterator () const
    {
    Dgn::ElementIterator iterator;
    if (GetSubModelId ().IsValid ())
        {
        iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridSurface), "WHERE Model.Id=?", "ORDER BY ECInstanceId ASC");
        iterator.GetStatement ()->BindId (1, GetSubModelId ());
        }
    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SpatialLocationModelPtr    Grid::GetSurfacesModel
(
) const
    {
    Dgn::DgnModelPtr subModel = GetSubModel ();

    if (subModel.IsValid ())
        {
        Dgn::SpatialLocationModelPtr surfacesModel = dynamic_cast<Dgn::SpatialLocationModel*>(subModel.get ());
        BeAssert (surfacesModel.IsValid () && "Grid submodel is not spatialLocationModel!");
        return surfacesModel;
        }
    return CreateSubModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SpatialLocationModelPtr    Grid::CreateSubModel
(
) const
    {
    Dgn::SpatialLocationModelPtr model = SpatialLocationModel::Create (*this);
    if (!model.IsValid ())
        return nullptr;

    Dgn::IBriefcaseManager::Request req;
    GetDgnDb().BriefcaseManager ().PrepareForModelInsert (req, *model, Dgn::IBriefcaseManager::PrepareAction::Acquire);

    if (Dgn::DgnDbStatus::Success != model->Insert ())
        return nullptr;

    return model;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator Grid::MakeAxesIterator () const
    {
    Dgn::ElementIterator iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridAxis), "WHERE Grid=?", "ORDER BY ECInstanceId ASC");
    ECN::ECClassId relClassId = GetDgnDb ().Schemas ().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridHasAxes);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement ())
        {
        pStmnt->BindNavigationValue (1, GetElementId (), relClassId);
        }
    return iterator;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
GridPtr Grid::TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName)
    {
    return db.Elements().GetForEdit<Grids::Grid>(BuildingElementsUtils::GetElementIdByParentElementAuthorityAndName(db,
                                                                                                                    GRIDS_AUTHORITY_Grid,
                                                                                                                    parentId,
                                                                                                                    gridName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
Utf8CP  Grid::GetName() const
    {
    return GetCode().GetValueUtf8CP();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void Grid::SetName
(
    Utf8String newName
)
    {
    Dgn::DgnCode currentCode = GetCode();
    Dgn::DgnCode newCode(currentCode.GetCodeSpecId(), currentCode.GetScopeElementId(GetDgnDb()), newName);
    SetCode(newCode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus Grid::_OnDelete() const
    {
    for (DgnElementId axisId : MakeAxesIterator().BuildIdList<DgnElementId>())
        GetDgnDb().Elements().Delete(axisId);

    GetSurfacesModel()->Delete();

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Grid::IntersectGridSurface 
(
GridSurfaceCPtr surface,
GridCurvesPortionCR targetPortion
) const
    {
    Dgn::DgnModelPtr model = GetSubModel ();
    if (!model.IsValid ())
        {
        return ERROR;
        }

    Dgn::DgnDbR db = model->GetDgnDb ();

    for (Dgn::ElementIteratorEntry elementEntry : model->MakeIterator ())
        {
        GridSurfaceCPtr innerSurface = db.Elements ().Get<GridSurface> (elementEntry.GetElementId ());
        if (innerSurface.IsValid())
            {
            GridCurveBundle::CreateAndInsert(db, targetPortion, *innerSurface, *surface);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   ElevationGrid::ValidateSurfaces
(
bvector<CurveVectorPtr> const& surfaces
)
    {
    DPlane3d zPlane;
    zPlane.origin = DPoint3d::FromZero ();
    zPlane.normal = DVec3d::From (0.0, 0.0, 1.0);
    for (CurveVectorPtr gridPlaneGeom : surfaces)
        {
        DPlane3d planeThis;
        Transform localToWorld, worldToLocal;
        DRange3d range;

        if (!gridPlaneGeom->IsPlanar (localToWorld, worldToLocal, range))
            return ERROR;

        bsiTransform_getOriginAndVectors (&localToWorld, &planeThis.origin, NULL, NULL, &planeThis.normal);

        //if planes are not parallel to Z, then fail
        if (!bsiDVec3d_areParallelTolerance (&planeThis.normal, &zPlane.normal, BUILDING_TOLERANCE))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   ElevationGrid::CreateElevationGridPlanes
(
bvector<CurveVectorPtr> const& surfaces,
GridAxisCR gridAxis
)
    {
    BentleyStatus status = BentleyStatus::ERROR;
    Dgn::SpatialLocationModelPtr subModel = GetSurfacesModel ();

    if (subModel.IsValid ())
        {
        status = BentleyStatus::SUCCESS;

        //not putting into same method yet.. will do as GridAxis element is introduced.
        GridPlanarSurfacePtr lastGridPlane = nullptr;
        DPlane3d planeLast;
        for (CurveVectorPtr gridPlaneGeom : surfaces)
            {
            Transform localToWorld, worldToLocal;
            DRange3d range;
            gridPlaneGeom->IsPlanar (localToWorld, worldToLocal, range);
            DPlane3d geomPlane;
            bsiTransform_getOriginAndVectors (&localToWorld, &geomPlane.origin, NULL, NULL, &geomPlane.normal);

            if (!DoubleOps::AlmostEqualFraction (abs (geomPlane.normal.z), 1.0))
                return BentleyStatus::ERROR;   //if the Z direction is not 1, fail

            double elevation = geomPlane.origin.z;

            Transform negatedElev = Transform::From(0.0, 0.0, -elevation);

            CurveVectorPtr surface = gridPlaneGeom->Clone (negatedElev);

            ElevationGridSurface::CreateParams params (*subModel, gridAxis, surface.get(), elevation);
            ElevationGridSurfacePtr gridPlane = ElevationGridSurface::Create (params);
            BuildingLocks_LockElementForOperation (*gridPlane, BeSQLite::DbOpcode::Insert, "Inserting ElevationSurface");
            gridPlane->Insert ();
            DPlane3d planeThis;
            planeThis = gridPlane->GetPlane ();
            planeLast = planeThis;
            lastGridPlane = gridPlane;
            }
        }
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridPtr        ElevationGrid::CreateAndInsertWithSurfaces
(
CreateParams const& params,
bvector<CurveVectorPtr> const& surfaces
)
    {
    if (BentleyStatus::SUCCESS != ValidateSurfaces (surfaces))
        return nullptr;

    ElevationGridPtr thisGrid = ElevationGrid::CreateAndInsert (params);
    
    if (BentleyStatus::SUCCESS != thisGrid->CreateElevationGridPlanes (surfaces, *thisGrid->GetAxis()))
        BeAssert (!"error inserting gridSurfaces into elevation grid..");
    return thisGrid;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridPtr        ElevationGrid::CreateAndInsert
(
CreateParams const& params
)
    {
    ElevationGridPtr thisGrid = new ElevationGrid (params);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting elevation grid");
    if (!thisGrid->Insert ().IsValid ())
        return nullptr;

    Dgn::DgnModelCR defModel = BuildingUtils::GetGroupInformationModel(thisGrid->GetDgnDb());

    GeneralGridAxisPtr horizontalAxis = GeneralGridAxis::CreateAndInsert(defModel, *thisGrid);

    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            ElevationGrid::SetDefaultSurface2d
(
CurveVectorPtr surface
)
    {
    if (!surface.IsValid())
        return;
    Transform localToWorld, worldToLocal;
    DRange3d range;
    surface->IsPlanar (localToWorld, worldToLocal, range);
    DPlane3d surfacePlane;
    bsiTransform_getOriginAndVectors (&localToWorld, &surfacePlane.origin, NULL, NULL, &surfacePlane.normal);

    if (!DoubleOps::AlmostEqualFraction (surfacePlane.origin.z, 0.0) ||
        !DoubleOps::AlmostEqualFraction (abs (surfacePlane.normal.z), 1.0)) //must be a zero Z plane
        return;

    IGeometryPtr geometryPtr = IGeometry::Create (surface);

    ECN::ECValue surface2dValue;
    surface2dValue.SetIGeometry (*geometryPtr);

    SetPropertyValue (prop_DefaultSurface2d(), surface2dValue);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr                   ElevationGrid::GetDefaultSurface2d
(
) const
    {
    ECN::ECValue surface2dValue;

    GetPropertyValue (surface2dValue, prop_DefaultSurface2d ());

    IGeometryPtr geometryPtr = surface2dValue.GetIGeometry ();
    if (!geometryPtr.IsValid ())
        return nullptr;

    CurveVectorPtr surface = geometryPtr->GetAsCurveVector ();

    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxisCPtr                    ElevationGrid::GetAxis
(
) const
    {
    return GetDgnDb().Elements().Get<GridAxis>((*MakeAxesIterator().begin()).GetElementId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
ElevationGridSurfaceCPtr ElevationGrid::GetSurface
(
    double elevation
) const
    {
    for (Dgn::ElementIteratorEntry surfaceEntry : MakeIterator())
        {
        ElevationGridSurfaceCPtr surface = GetDgnDb().Elements().Get<ElevationGridSurface>(surfaceEntry.GetElementId());
        if (surface.IsNull())
            continue;

        if (DoubleOps::AlmostEqual(surface->GetElevation(), elevation))
            return surface;
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
ElevationGridSurfaceCPtr ElevationGrid::GetBottomSurface() const
    {
    ElevationGridSurfaceCPtr bottomSurface = nullptr;

    for (Dgn::ElementIteratorEntry surfaceEntry : MakeIterator())
        {
        ElevationGridSurfaceCPtr surface = GetDgnDb().Elements().Get<ElevationGridSurface>(surfaceEntry.GetElementId());
        if (surface.IsNull())
            continue;

        if (bottomSurface.IsNull() || bottomSurface->GetElevation() > surface->GetElevation())
            bottomSurface = surface;
        }

    return bottomSurface;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
ElevationGridSurfaceCPtr ElevationGrid::GetTopSurface() const
    {
    ElevationGridSurfaceCPtr topMostSurface = nullptr;

    for (Dgn::ElementIteratorEntry surfaceEntry : MakeIterator())
        {
        ElevationGridSurfaceCPtr surface = GetDgnDb().Elements().Get<ElevationGridSurface>(surfaceEntry.GetElementId());
        if (surface.IsNull())
            continue;

        if (topMostSurface.IsNull() || topMostSurface->GetElevation() < surface->GetElevation())
            topMostSurface = surface;
        }

    return topMostSurface;
    }

END_GRIDS_NAMESPACE

