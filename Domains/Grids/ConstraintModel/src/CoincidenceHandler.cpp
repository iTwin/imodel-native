/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/src/CoincidenceHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../PublicApi/CoincidenceHandler.h"
#include "../PublicApi/IConstrainable.h"
#include <dgnPlatform/DgnModel.h>
#include <dgnPlatform/ViewController.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/Render.h>
#include "DgnPlatform/DgnDomain.h"
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/RangeIndex.h>


USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BENTLEY_DGN

HANDLER_DEFINE_MEMBERS(CoincidenceHandler)

//NEEDS WORK: currently ignores GEOM tags, only transforms placement

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CoincidenceHandler::_OnRootChanged
(
    Dgn::DgnDbR db,
    BeSQLite::EC::ECInstanceId relationshipId,
    Dgn::DgnElementId source,
    Dgn::DgnElementId target
)
    {

    if (db.Txns ().HasFatalErrors ())
        return;

    Utf8String ecsql ("SELECT geomIdSource, geomIdTarget FROM ");
    ecsql.append (GetECClass (db).GetECSqlName ());
    ecsql.append (" WHERE ECInstanceId = ? ");
    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (db, ecsql.c_str ());
    statement.BindId (1, relationshipId);
    statement.Step ();
    int geomId1 = statement.GetValueInt (0);
    int geomId2 = statement.GetValueInt (1);

    //source is supposed to be boundedPlane and target a spatialStory
    DgnElementCPtr sourceElem = db.Elements ().GetElement (source);// nullptr;// db.Elements ().Get<BoundedPlane> (source);
    DgnElementPtr targetElem = db.Elements ().GetForEdit<DgnElement> (target); //db.Elements ().GetForEdit<SpatialStory> (target);


    IConstrainable const* sourceConstrainable = dynamic_cast<IConstrainable const*>(sourceElem.get ());
    IConstrainable* targetConstrainable = dynamic_cast<IConstrainable*>(targetElem.get ());

    if (sourceConstrainable == NULL || targetConstrainable == NULL)
        {
        BeAssert (!"constrained elements are not IConstrainables");
        return;
        }

    DPlane3d sourcePlane, targetPlane;
    sourceConstrainable->GetGeomIdPlane (geomId1, sourcePlane);
    targetConstrainable->GetGeomIdPlane (geomId2, targetPlane);

    double distance = bsiDPlane3d_evaluate (&targetPlane, &sourcePlane.origin);
    if (fabs (distance) > 1.0E-3)
        {
        if (geomId2 == 0)
            {

            DPoint3d projectedPt;
            DVec3d dirVec;
            bsiDPlane3d_projectPoint (&targetPlane, &projectedPt, &sourcePlane.origin);
            bsiDVec3d_subtractDPoint3dDPoint3d (&dirVec, &sourcePlane.origin, &projectedPt);

            Transform trans;
            bsiTransform_initIdentity (&trans);
            bsiTransform_setTranslation (&trans, &dirVec);

            Placement3d placement = ((Dgn::DgnElement*)(targetElem.get ()))->ToGeometrySource3d ()->GetPlacement ();

            trans.Multiply (placement.GetOriginR ());
            ((Dgn::DgnElement*)(targetElem.get ()))->ToGeometrySource3dP ()->SetPlacement (placement);

            db.Elements ().Update (*targetElem);
            }
        else if (geomId2 == 1)
            {
            targetConstrainable->StretchGeomIdToPlane (geomId2, sourcePlane);
            db.Elements ().Update (*targetElem);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CoincidenceHandler::_ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData)
    {
    if (false)
        db.Txns ().ReportError (*new Dgn::TxnManager::ValidationError (Dgn::TxnManager::ValidationError::Severity::Warning, "ABC failed"));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey CoincidenceHandler::Insert(Dgn::DgnDbR db, Dgn::DgnElementId root, Dgn::DgnElementId dependent, int geomid1, int geomid2)
    {

    BeSQLite::EC::ECInstanceKey relKey;
    ECN::ECRelationshipClassCP relClass = (ECN::ECRelationshipClassCP)(&GetECClass(db));
    auto relInst = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass)->CreateRelationshipInstance ();
    relInst->SetValue ("geomIdSource", ECN::ECValue (geomid1));
    relInst->SetValue ("geomIdTarget", ECN::ECValue (geomid2));
    BeSQLite::EC::ECInstanceKey rkey;
    db.InsertLinkTableRelationship (rkey, (ECN::ECRelationshipClassCR)(GetECClass (db)), BeSQLite::EC::ECInstanceId (root), BeSQLite::EC::ECInstanceId (dependent), relInst.get());

    return rkey;
    }