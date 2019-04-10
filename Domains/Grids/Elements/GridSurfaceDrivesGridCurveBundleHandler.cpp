/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridSurfaceDrivesGridCurveBundleHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GridElementsAPI.h"

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

HANDLER_DEFINE_MEMBERS(GridSurfaceDrivesGridCurveBundleHandler)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridSurfaceDrivesGridCurveBundleHandler::_OnRootChanged
(
    Dgn::DgnDbR db, 
    BeSQLite::EC::ECInstanceId relationshipId, 
    Dgn::DgnElementId source, 
    Dgn::DgnElementId target
)
    {

    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void GridSurfaceDrivesGridCurveBundleHandler::_ProcessDeletedDependency
(
    Dgn::DgnDbR db, 
    Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData
)
    {

    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
BeSQLite::EC::ECInstanceKey GridSurfaceDrivesGridCurveBundleHandler::Insert
(
    Dgn::DgnDbR db, 
    GridSurfaceCR source, 
    GridCurveBundleCR target
)
    {
    return RelationshipUtils::InsertRelationship(db, GetECClass(db), source.GetElementId(), target.GetElementId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipClassCR GridSurfaceDrivesGridCurveBundleHandler::GetECClass
(
    Dgn::DgnDbCR db
)
    {
    return static_cast<ECN::ECRelationshipClassCR>(*db.Schemas().GetClass(GRIDS_SCHEMA_NAME, GRIDS_REL_GridSurfaceDrivesGridCurveBundle));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator GridSurfaceDrivesGridCurveBundleHandler::MakeGridSurfaceIterator
(
    GridCurveBundleCR target
)
    {
    Dgn::DgnDbR db = target.GetDgnDb();
    return RelationshipUtils::MakeSourceIterator(db, GetECClass(db), target.GetElementId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator GridSurfaceDrivesGridCurveBundleHandler::MakeGridCurveBundleIterator
(
    GridSurfaceCR source
)
    {
    Dgn::DgnDbR db = source.GetDgnDb();
    return RelationshipUtils::MakeTargetIterator(db, GetECClass(db), source.GetElementId());
    }