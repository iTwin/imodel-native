/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/GridElementsAPI.h"

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

HANDLER_DEFINE_MEMBERS(GridCurveBundleCreatesGridCurveHandler)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridCurveBundleCreatesGridCurveHandler::_OnRootChanged
(
    Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge
)
    {

    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridCurveBundleCreatesGridCurveHandler::_OnDeletedDependency
(
    Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge
)
    {

    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
BeSQLite::EC::ECInstanceKey GridCurveBundleCreatesGridCurveHandler::Insert
(
    Dgn::DgnDbR db,
    GridCurveBundleCR source,
    GridCurveCR target
)
    {
    return RelationshipUtils::InsertRelationship(db, GetECClass(db), source.GetElementId(), target.GetElementId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipClassCR GridCurveBundleCreatesGridCurveHandler::GetECClass
(
    Dgn::DgnDbCR db
)
    {
    return static_cast<ECN::ECRelationshipClassCR>(*db.Schemas().GetClass(GRIDS_SCHEMA_NAME, GRIDS_REL_GridCurveBundleCreatesGridCurve));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
GridCurveBundleCPtr GridCurveBundleCreatesGridCurveHandler::GetBundle
(
    GridCurveCR target
)
    {
    Dgn::DgnDbR db = target.GetDgnDb();
    ElementIdIterator sourceIter = RelationshipUtils::MakeSourceIterator(db, GetECClass(db), target.GetElementId());
    bvector<Dgn::DgnElementId> ids = sourceIter.BuildIdList<Dgn::DgnElementId>();
    if (ids.empty())
        return nullptr;

    BeAssert(1 == ids.size());
    return GridCurveBundle::Get(db, ids.front());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
GridCurvePtr GridCurveBundleCreatesGridCurveHandler::GetGridCurveForEdit
(
    GridCurveBundleCR source
)
    {
    GridCurveCPtr curve = GetGridCurve(source);
    if (curve.IsNull())
        return nullptr;

    return curve->MakeCopy<GridCurve>();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
GridCurveCPtr GridCurveBundleCreatesGridCurveHandler::GetGridCurve
(
    GridCurveBundleCR source
)
    {
    Dgn::DgnDbR db = source.GetDgnDb();
    ElementIdIterator targetIter = RelationshipUtils::MakeTargetIterator(db, GetECClass(db), source.GetElementId());
    bvector<Dgn::DgnElementId> ids = targetIter.BuildIdList<Dgn::DgnElementId>();
    if (ids.empty())
        return nullptr;

    BeAssert(1 == ids.size());
    return GridCurve::Get(db, ids.front());
    }