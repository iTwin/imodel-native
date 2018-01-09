/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridDrivesGridSurfaceHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include <BuildingShared/BuildingSharedApi.h>
#include <Grids/gridsApi.h>

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
    //if elements are not valid or not in the db, then can't do anything about it.
    if (!thisGrid.GetElementId ().IsValid () || !surface.GetElementId ().IsValid ())
        return BeSQLite::EC::ECInstanceKey ();

    BeSQLite::EC::ECInstanceKey relKey;
    auto relClass = (ECN::ECRelationshipClassCP)(db.Schemas().GetClass(GetECClass(db).GetId()));
    auto relInst = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass)->CreateRelationshipInstance();

    BeSQLite::DbResult result = db.InsertLinkTableRelationship(relKey, *relClass, BeSQLite::EC::ECInstanceId(thisGrid.GetElementId()), BeSQLite::EC::ECInstanceId(surface.GetElementId()), relInst.get());
    BeAssert(result == BeSQLite::DbResult::BE_SQLITE_OK);

    return relKey;
    }
