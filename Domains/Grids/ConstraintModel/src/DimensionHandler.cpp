/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/src/DimensionHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../PublicApi/DimensionHandler.h"
#include <dgnPlatform/DgnModel.h>
#include <dgnPlatform/ViewController.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/Render.h>
#include "DgnPlatform/DgnDomain.h"
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/RangeIndex.h>
#include <BuildingUtils.h>

USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BENTLEY_DGN

HANDLER_DEFINE_MEMBERS(DimensionHandler)

//NEEDS WORK: currently ignores GEOM tags, only transforms placement

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionHandler::_OnRootChanged
(
Dgn::DgnDbR db,
BeSQLite::EC::ECInstanceId relationshipId,
Dgn::DgnElementId source,
Dgn::DgnElementId target
)
    {

    if (db.Txns ().HasFatalErrors ())
        return;

    if (false)
        db.Txns ().ReportError (*new TxnManager::ValidationError (TxnManager::ValidationError::Severity::Fatal, "ABC failed"));

    Utf8String ecsql ("SELECT geomIdSource, geomIdTarget, offsetvalue, direction FROM ");
    ecsql.append (GetECClass (db).GetECSqlName ());
    ecsql.append (" WHERE SourceECInstanceId = ? AND TargetECInstanceId = ?");
    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (db, ecsql.c_str ());
    statement.BindId (1, source);
    statement.BindId (2, target);
    statement.Step ();
    //int geomId1 = statement.GetValueInt (0);
    //int geomId2 = statement.GetValueInt (1);
    DPoint3d dir = statement.GetValuePoint3d (3);
    double offset = statement.GetValueDouble (2);

    Dgn::DgnElementCPtr sourceElem = db.Elements ().Get<Dgn::DgnElement> (source);
    Dgn::DgnElementPtr targetElem = db.Elements ().GetForEdit<Dgn::DgnElement> (target);

    Transform trans;
    bsiTransform_initIdentity (&trans);
    bsiDPoint3d_scaleInPlace (&dir, offset);
    bsiTransform_setTranslation (&trans, &dir);

    Placement3d placement = sourceElem->ToGeometrySource3d ()->GetPlacement ();

    trans.Multiply (placement.GetOriginR ());
    targetElem->ToGeometrySource3dP ()->SetPlacement (placement);
    db.Elements ().Update (*targetElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionHandler::_ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData)
    {
    if (false)
        db.Txns ().ReportError (*new Dgn::TxnManager::ValidationError (Dgn::TxnManager::ValidationError::Severity::Warning, "ABC failed"));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey DimensionHandler::Insert(Dgn::DgnDbR db, Dgn::DgnElementId root, Dgn::DgnElementId dependent, int geomid1, int geomid2, DVec3d direction, double offset)
    {
    BeSQLite::EC::ECInstanceKey relKey;
    ECN::ECRelationshipClassCP relClass = (ECN::ECRelationshipClassCP)(&GetECClass (db));
    auto relInst = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass)->CreateRelationshipInstance ();
    relInst->SetValue ("geomIdSource", ECN::ECValue (geomid1));
    relInst->SetValue ("geomIdTarget", ECN::ECValue (geomid2));
    relInst->SetValue ("offsetvalue", ECN::ECValue (offset));
    relInst->SetValue ("direction", ECN::ECValue ((DPoint3d)direction));
    BeSQLite::EC::ECInstanceKey rkey;
    db.InsertLinkTableRelationship (rkey, (ECN::ECRelationshipClassCR)(GetECClass (db)), BeSQLite::EC::ECInstanceId (root), BeSQLite::EC::ECInstanceId (dependent), relInst.get ());
    return rkey;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<BeSQLite::EC::ECInstanceId> DimensionHandler::GetDimensioningRelationshipInstances
(
Dgn::DgnDbR db,
Dgn::DgnElementId elementId,
bool elementIsSource/* = false*/
)
    {
    bvector<BeSQLite::EC::ECInstanceId> returnVec;
    Utf8String ecsql ("SELECT ECInstanceId FROM ");
    ecsql.append (GetECClass (db).GetECSqlName ()).append(" WHERE ").append (elementIsSource ? "SourceECInstanceId=?" : "TargetECInstanceId=?");

    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (db, ecsql.c_str ());
    statement.BindId (1, elementId);

    BeSQLite::DbResult result = statement.Step ();
    if (BeSQLite::DbResult::BE_SQLITE_ROW != result)
        {
        return returnVec;
        }

    returnVec.push_back (statement.GetValueId<BeSQLite::EC::ECInstanceId> (0));
    return returnVec;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeSQLite::EC::ECInstanceId>            DimensionHandler::IsDimensioned
(
Dgn::DgnDbR db,
Dgn::DgnElementId elementId
)
    {
    return GetDimensioningRelationshipInstances (db, elementId);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::AdjustOffsetByVector
(
Dgn::DgnDbR db,
BeSQLite::EC::ECInstanceId relationshipId,
DVec3d delta,
bool adjustSingle/*=false*/
)
    {
    Utf8String ecsql ("SELECT direction, offsetvalue, TargetECInstanceId  FROM ");
    ecsql.append (GetECClass (db).GetECSqlName ()).append (" WHERE ECInstanceId=?");

    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (db, ecsql.c_str ());
    statement.BindId (1, relationshipId);

    if (BeSQLite::DbResult::BE_SQLITE_ROW != statement.Step () && BeSQLite::DbResult::BE_SQLITE_DONE != statement.Step ())
        return false;
    
    DPoint3d direction = statement.GetValuePoint3d (0);
    double offset = statement.GetValueDouble (1);

    DVec3d dirVec;
    bsiDVec3d_scaleToLength (&dirVec, (DVec3dCP)&direction, offset);

    DVec3d projectedVec;
    bsiGeom_projectVectorToVector (&projectedVec, NULL, NULL, &delta, &dirVec);


    double subResult = offset - bsiDVec3d_magnitude (&projectedVec);

    bsiDVec3d_addInPlace (&projectedVec, &dirVec);
    double newOffset = bsiDVec3d_magnitude (&projectedVec);


    //can't change direction 
    if (fabs(newOffset + subResult) < 1.0E-8)
        {
        newOffset = 0.01;
        }

    auto relInst = Building::BuildingUtils::GetECInstance (db, relationshipId, GetECClass (db).GetECSqlName ().c_str());
    relInst->SetValue ("offsetvalue", ECN::ECValue (newOffset));
    db.UpdateLinkTableRelationshipProperties (BeSQLite::EC::ECInstanceKey (GetECClass (db).GetId (), relationshipId), *relInst);

    if (adjustSingle)
        {
        DgnElementId elementId = statement.GetValueId<DgnElementId> (2);
        bvector<BeSQLite::EC::ECInstanceId> relationshipInstances = GetDimensioningRelationshipInstances (db, elementId, true);
        if (relationshipInstances.size () < 1)
            {
            return false;
            }

        DVec3d delta = DVec3d::From (direction);
        delta.ScaleToLength (offset - newOffset);

        DimensionHandler::AdjustOffsetByVector (db, relationshipInstances[0], delta, false);
        }

    return true;
    }
