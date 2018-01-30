/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridSurfaceCreatesGridCurveHandler.cpp $
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

HANDLER_DEFINE_MEMBERS(GridSurfaceCreatesGridCurveHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GridSurfaceCreatesGridCurveHandler::_OnRootChanged
(
Dgn::DgnDbR db,
BeSQLite::EC::ECInstanceId relationshipId,
Dgn::DgnElementId source,
Dgn::DgnElementId target
)
    {
    if (false)
        db.Txns ().ReportError (*new TxnManager::ValidationError (TxnManager::ValidationError::Severity::Fatal, "ABC failed"));

    Utf8String ecsqlOtherSurface ("SELECT SourceECInstanceId FROM ");
    ecsqlOtherSurface.append (GetECClass (db).GetECSqlName ());
    ecsqlOtherSurface.append (" WHERE TargetECInstanceId = ? AND SourceECInstanceId != ?");
    BeSQLite::EC::ECSqlStatement statementOtherSurface;
    statementOtherSurface.Prepare (db, ecsqlOtherSurface.c_str ());
    statementOtherSurface.BindId (1, target);
    statementOtherSurface.BindId (2, source);
    statementOtherSurface.Step ();

    Dgn::DgnElementId otherSurfaceId = statementOtherSurface.GetValueId<Dgn::DgnElementId> (0);

    GridSurfaceCPtr thisSurface = db.Elements ().Get<GridSurface> (source);
    GridSurfaceCPtr otherSurface = db.Elements ().Get<GridSurface> (otherSurfaceId);
    GridCurvePtr thisCurve = db.Elements ().GetForEdit<GridCurve> (target);

    ICurvePrimitivePtr newCurve;
    
    newCurve = ComputeIntersection (otherSurface, thisSurface);

    if (newCurve.IsValid ())
        {
        thisCurve->SetCurve (newCurve);
        thisCurve->Update ();
        }
    /*else
        db.Txns ().ReportError (*new TxnManager::ValidationError (TxnManager::ValidationError::Severity::Fatal, "failed to create gridCurve"));
*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GridSurfaceCreatesGridCurveHandler::_ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData)
    {
    if (false)
        db.Txns ().ReportError (*new Dgn::TxnManager::ValidationError (Dgn::TxnManager::ValidationError::Severity::Warning, "ABC failed"));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr              GridSurfaceCreatesGridCurveHandler::ComputeIntersection
(
GridSurfaceCPtr thisSurface,
GridSurfaceCPtr otherSurface
)
    {
    bvector<Dgn::IBRepEntityPtr> brepsThis, brepsThat;
    if (SUCCESS == GeometryUtils::GetIBRepEntitiesFromGeometricElement (brepsThis, thisSurface) &&
        SUCCESS == GeometryUtils::GetIBRepEntitiesFromGeometricElement (brepsThat, otherSurface))
        {
        Dgn::IBRepEntity::EntityType type1 = brepsThis[0]->GetEntityType();
        Dgn::IBRepEntity::EntityType type2 = brepsThat[0]->GetEntityType();
        CurveVectorPtr result;
        BRepUtil::Modify::IntersectSheetFaces(result, *brepsThis[0], *brepsThat[0]);
        if (result->size() > 0)
            return *result->begin();
        return nullptr; // TODO get the curvePrimitive
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey GridSurfaceCreatesGridCurveHandler::Insert
(
Dgn::DgnDbR db,
GridSurfaceCPtr thisSurface,
GridSurfaceCPtr otherSurface,
Dgn::DgnModelCR targetModel
)
    {
    //if elements are not valid or not in the db, then can't do anything about it.
    if (thisSurface.IsNull () || otherSurface.IsNull () ||
        !thisSurface->GetElementId ().IsValid () || !otherSurface->GetElementId ().IsValid ())
        return BeSQLite::EC::ECInstanceKey ();

    BeSQLite::EC::ECInstanceKey relKey;
    auto relClass = (ECN::ECRelationshipClassCP)(db.Schemas().GetClass(GetECClass(db).GetId()));
    auto relInstBase = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass)->CreateRelationshipInstance();
    auto relInstIntersecting = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass)->CreateRelationshipInstance ();

    //create and compute the gridcurve
    ICurvePrimitivePtr bodyThis = ComputeIntersection (thisSurface, otherSurface);

    if (bodyThis.IsValid ())
        {
        GridCurvePtr curve;
        if (bodyThis->GetLineCP())
            curve = GridLine::Create(targetModel, bodyThis);
        else if (bodyThis->GetArcCP())
            curve = GridArc::Create(targetModel, bodyThis);
        else if (bodyThis->GetBsplineCurveCP() || bodyThis->GetInterpolationCurveCP())
            curve = GridSpline::Create(targetModel, bodyThis);
        if (curve.IsNull())
            return BeSQLite::EC::ECInstanceKey();

        curve->Insert ();
        BeSQLite::DbResult result = db.InsertLinkTableRelationship (relKey, *relClass, BeSQLite::EC::ECInstanceId (thisSurface->GetElementId ()), BeSQLite::EC::ECInstanceId (curve->GetElementId ()), relInstIntersecting.get ());
        result = db.InsertLinkTableRelationship (relKey, *relClass, BeSQLite::EC::ECInstanceId (otherSurface->GetElementId ()), BeSQLite::EC::ECInstanceId (curve->GetElementId ()), relInstBase.get ());
        BeAssert (result == BeSQLite::DbResult::BE_SQLITE_OK);
        }

    return relKey;
    }
