/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <dgnPlatform/DgnModel.h>
#include <dgnPlatform/ViewController.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/Render.h>
#include "DgnPlatform/DgnDomain.h"
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/RangeIndex.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>
#include <Grids/Elements/GridElementsAPI.h>

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

HANDLER_DEFINE_MEMBERS(GridDrivesGridSurfaceHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void GridDrivesGridSurfaceHandler::_OnRootChanged
(
Dgn::DgnDbR db,
BeSQLite::EC::ECInstanceId relationshipId,
Dgn::DgnElementId source,
Dgn::DgnElementId target
)
    {
    GridSurfacePtr thisSurface = db.Elements ().GetForEdit<GridSurface> (target);

    if (thisSurface.IsNull())
        db.Txns().ReportError(*new TxnManager::ValidationError(TxnManager::ValidationError::Severity::Fatal, "target surface is NULL"));
    thisSurface->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void GridDrivesGridSurfaceHandler::_ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData)
    {

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey GridDrivesGridSurfaceHandler::Insert
(
Dgn::DgnDbR     db,
GridCR        thisGrid,
GridSurfaceCR surface
)
    {
    return RelationshipUtils::InsertRelationship(db, GetECClass(db), thisGrid.GetElementId(), surface.GetElementId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipClassCR GridDrivesGridSurfaceHandler::GetECClass
(
    Dgn::DgnDbR db
) 
    { 
    return static_cast<ECN::ECRelationshipClassCR>(*db.Schemas().GetClass(GRIDS_SCHEMA_NAME, GRIDS_REL_GridDrivesGridSurface)); 
    }
