/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridCurveBundle.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GridElementsAPI.h"
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(GridCurveBundle)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
GridCurveBundlePtr GridCurveBundle::CreateAndInsert
(
    Dgn::DgnDbR db,
    GridCurvesSetCR portion,
    GridSurfaceCR surface1,
    GridSurfaceCR surface2
)
    {
    CreateParams params = CreateParams(db, db.GetRepositoryModel()->GetModelId(), QueryClassId(db));
    GridCurveBundlePtr bundle = new GridCurveBundle(params);
    bundle->SetCurvesSet(portion);

    Dgn::DgnDbStatus status;
    bundle->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        BeAssert(Dgn::DgnDbStatus::Success == status);
        return nullptr;
        }

    GridSurfaceDrivesGridCurveBundleHandler::Insert(db, surface1, *bundle);
    GridSurfaceDrivesGridCurveBundleHandler::Insert(db, surface2, *bundle);

    return bundle;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ECN::ECClassId GridCurveBundle::GetCurvesSetRelClassId
(
    Dgn::DgnDbR db
)
    {
    return db.Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_REL_GridCurveBundleRefersToGridCurvesSet);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridCurveBundle::SetCurvesSet
(
    GridCurvesSetCR portion
)
    {
    ECN::ECClassId relClassId = GetCurvesSetRelClassId(GetDgnDb());
    SetPropertyValue(prop_CurvesSet(), portion.GetElementId(), relClassId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementId GridCurveBundle::GetCurvesSetId() const
    {
    return GetPropertyValueId<Dgn::DgnElementId>(prop_CurvesSet());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator GridCurveBundle::MakeGridCurveBundleIterator
(
    GridCurvesSetCR portion
)
    {
    Dgn::DgnDbR db = portion.GetDgnDb();

    Dgn::ElementIterator bundleIterator = db.Elements().MakeIterator(GRIDS_SCHEMA(GRIDS_CLASS_GridCurveBundle), "WHERE CurvesSet=?");
    ECN::ECClassId relClassId = GetCurvesSetRelClassId(db);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = bundleIterator.GetStatement())
        {
        pStmnt->BindNavigationValue(1, portion.GetElementId(), relClassId);
        }

    return bundleIterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
GridCurveCPtr GridCurveBundle::GetGridCurve() const
    {
    return GridCurveBundleCreatesGridCurveHandler::GetGridCurve(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               07/2019
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator GridCurveBundle::MakeDrivingSurfaceIterator() const
    {
    return GridSurfaceDrivesGridCurveBundleHandler::MakeGridSurfaceIterator(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator GridCurveBundle::MakeDrivingSurfaceIterator
(
    GridCurveCR curve
)
    {
    GridCurveBundleCPtr bundle = GridCurveBundleCreatesGridCurveHandler::GetBundle(curve);
    if (bundle.IsNull())
        return ElementIdIterator();
        
    return bundle->MakeDrivingSurfaceIterator();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus GridCurveBundle::_OnDelete() const
    {
    GridCurveCPtr gridCurve = GetGridCurve();
    if (gridCurve.IsValid())
        {
        Dgn::DgnDbStatus gridCurveDeleteStatus = gridCurve->Delete();
        if (Dgn::DgnDbStatus::Success != gridCurveDeleteStatus)
            return gridCurveDeleteStatus;
        }

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               07/19
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::IBRepEntityPtr getBRepFromElevationSurface(ElevationGridSurfaceCR surface, DRange3dCR range)
    {
    constexpr double tolerance = 1.0f;

    GridCPtr surfaceGrid = Grid::Get(surface.GetDgnDb(), surface.GetGridId());;

    if (surfaceGrid.IsNull())
        return nullptr;

    CurveVectorPtr curve = surface.GetSurface2d();
    if (curve.IsNull())
        {
        // Limited by other surface range, because psolid only works in small box.
        // Added 1 for when rounded curves get split into two at the plane edges
        curve = CurveVector::CreateRectangle(
            range.low.x - tolerance,
            range.low.y - tolerance,
            range.high.x + tolerance,
            range.high.y + tolerance,
            surface.GetElevation() + surfaceGrid->GetPlacementTransform().Translation().z);
        }

    Dgn::IBRepEntityPtr bRep;
    BRepUtil::Create::BodyFromCurveVector(bRep, *curve);

    return bRep;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               07/19
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::IBRepEntityPtr getBRepFromSurface(GridSurfaceCR surface, DRange3dCR otherSurfaceRange)
    {
    if (!surface.HasGeometry())
        {
        // Elevation surface might have no curve specified, but still have an elevation,
        // so we try to create an infinite plane if that's the case
        ElevationGridSurfaceCP elevationSurface = dynamic_cast<ElevationGridSurfaceCP>(&surface);
        if(!elevationSurface)
            return nullptr;

        return getBRepFromElevationSurface(*elevationSurface, otherSurfaceRange);
        }

    bvector<Dgn::IBRepEntityPtr> bReps;
    if (SUCCESS != DgnGeometryUtils::GetIBRepEntitiesFromGeometricElement(bReps, &surface))
        return nullptr;

    if (bReps.size() == 0)
        return nullptr;

    return *bReps.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               07/19
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr GridCurveBundle::ComputeIntersection
(
GridSurfaceCR firstSurface,
GridSurfaceCR secondSurface
)
    {
    Dgn::IBRepEntityPtr firstSurfaceBRep = getBRepFromSurface(firstSurface, secondSurface.CalculateRange3d());
    if(firstSurfaceBRep.IsNull())
        return nullptr;

    Dgn::IBRepEntityPtr secondSurfaceBRep = getBRepFromSurface(secondSurface, firstSurface.CalculateRange3d());
    if (secondSurfaceBRep.IsNull())
        return nullptr;

    CurveVectorPtr intersectionCurve;
    BRepUtil::Modify::IntersectSheetFaces(intersectionCurve, *firstSurfaceBRep, *secondSurfaceBRep);

    if (intersectionCurve.IsNull() || intersectionCurve->size() == 0)
        return nullptr;

    return *intersectionCurve->begin();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               07/2019
//---------------+---------------+---------------+---------------+---------------+------
Utf8String createCurveLabel(GridSurfaceCPtr surface1, GridSurfaceCPtr surface2)
    {
    auto elevationSurfaceClassId = surface2->GetDgnDb().GetClassLocater().LocateClassId(GRIDS_SCHEMA_NAME, GRIDS_CLASS_ElevationGridSurface);

    if(surface2->GetElementClassId() == elevationSurfaceClassId && surface1->GetElementClassId() != elevationSurfaceClassId)
        return Utf8String(surface2->GetUserLabel()) + "-" + Utf8String(surface1->GetUserLabel());

    return Utf8String(surface1->GetUserLabel()) + "-" + Utf8String(surface2->GetUserLabel());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridCurveBundle::UpdateGridCurve()
    {
    ElementIdIterator idIter = GridSurfaceDrivesGridCurveBundleHandler::MakeGridSurfaceIterator(*this);
    bvector<Dgn::DgnElementId> ids = idIter.BuildIdList<Dgn::DgnElementId>();
    if (ids.size() != 2)
        return;

    GridSurfaceCPtr surface1 = GridSurface::Get(GetDgnDb(), ids[0]);
    GridSurfaceCPtr surface2 = GridSurface::Get(GetDgnDb(), ids[1]);

    BeAssert(surface1.IsValid());
    BeAssert(surface2.IsValid());

    GridCurvePtr gridCurve = GridCurveBundleCreatesGridCurveHandler::GetGridCurveForEdit(*this);
    ICurvePrimitivePtr intersection = ComputeIntersection(*surface1, *surface2);

    if (gridCurve.IsValid())
        {
        if (intersection.IsValid())
            {
            gridCurve->SetCurve(intersection);
            gridCurve->SetUserLabel(createCurveLabel(surface1, surface2).c_str());
            gridCurve->Update();
            }
        else
            {
            gridCurve->Delete();
            }
        }
    else
        {
        if (intersection.IsNull())
            return;

        Dgn::DgnElementId portionId = GetPropertyValueId<Dgn::DgnElementId>(prop_CurvesSet());
        GridCurvesSetCPtr portion = GridCurvesSet::Get(GetDgnDb(), portionId);
        BeAssert(portion.IsValid());

        if (intersection->GetLineCP())
            gridCurve = GridLine::Create(*portion, intersection);
        else if (intersection->GetArcCP())
            gridCurve = GridArc::Create(*portion, intersection);
        else if (intersection->GetBsplineCurveCP() || intersection->GetInterpolationCurveCP())
            gridCurve = GridSpline::Create(*portion, intersection);

        gridCurve->SetUserLabel(createCurveLabel(surface1, surface2).c_str());

        Dgn::DgnDbStatus status;
        gridCurve->Insert(&status);
        BeAssert(Dgn::DgnDbStatus::Success == status);

        GridCurveBundleCreatesGridCurveHandler::Insert(GetDgnDb(), *this, *gridCurve);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridCurveBundle::_OnAllInputsHandled()
    {
    UpdateGridCurve();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridCurveBundle::_OnBeforeOutputsHandled()
    {
    UpdateGridCurve();
    }