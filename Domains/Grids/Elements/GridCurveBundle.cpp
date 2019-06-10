/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

    return GridSurfaceDrivesGridCurveBundleHandler::MakeGridSurfaceIterator(*bundle);
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
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr              GridCurveBundle::ComputeIntersection
(
GridSurfaceCR thisSurface,
GridSurfaceCR otherSurface
)
    {
    bvector<Dgn::IBRepEntityPtr> brepsThis, brepsThat;
    if (SUCCESS == DgnGeometryUtils::GetIBRepEntitiesFromGeometricElement (brepsThis, &thisSurface) &&
        SUCCESS == DgnGeometryUtils::GetIBRepEntitiesFromGeometricElement (brepsThat, &otherSurface))
        {
        // unused - Dgn::IBRepEntity::EntityType type1 = brepsThis[0]->GetEntityType();
        // unused - Dgn::IBRepEntity::EntityType type2 = brepsThat[0]->GetEntityType();
        CurveVectorPtr result;
        BRepUtil::Modify::IntersectSheetFaces(result, *brepsThis[0], *brepsThat[0]);
        if (result->size() > 0)
            return *result->begin();
        return nullptr; // TODO get the curvePrimitive
        }
    return nullptr;
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