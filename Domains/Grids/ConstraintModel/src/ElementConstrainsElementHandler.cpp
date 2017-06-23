/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/src/ElementConstrainsElementHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../PublicApi/ElementConstrainsElementHandler.h"
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

HANDLER_DEFINE_MEMBERS(ElementConstrainsElementHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementConstrainsElementHandler::_OnRootChanged
(
Dgn::DgnDbR db,
BeSQLite::EC::ECInstanceId relationshipId,
Dgn::DgnElementId source,
Dgn::DgnElementId target
)
    {
    if (false)
        db.Txns ().ReportError (*new TxnManager::ValidationError (TxnManager::ValidationError::Severity::Fatal, "ABC failed"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementConstrainsElementHandler::_ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData)
    {
    if (false)
        db.Txns ().ReportError (*new Dgn::TxnManager::ValidationError (Dgn::TxnManager::ValidationError::Severity::Warning, "ABC failed"));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey ElementConstrainsElementHandler::Insert(Dgn::DgnDbR db, Dgn::DgnElementId root, Dgn::DgnElementId dependent, int geomid1, int geomid2)
    {
    BeSQLite::EC::ECInstanceKey relKey;
    auto relClass = (ECN::ECRelationshipClassCP)(db.Schemas().GetClass(GetECClass(db).GetId()));
    auto relInst = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass)->CreateRelationshipInstance();

    relInst->SetValue("geomIdSource", ECN::ECValue(geomid1));
    relInst->SetValue("geomIdTarget", ECN::ECValue(geomid2));

    auto result = db.InsertLinkTableRelationship (relKey, *relClass, BeSQLite::EC::ECInstanceId(root), BeSQLite::EC::ECInstanceId(dependent), relInst.get());
    BeAssert(result == BeSQLite::DbResult::BE_SQLITE_OK);

    return relKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ElementConstrainsElementHandler::IsConstrained 
(
Dgn::DgnDbR db,
Dgn::DgnElementId elementId
)
    {
    Utf8String ecsql ("SELECT SourceECInstanceId FROM ");
    ecsql.append (GetECClass (db).GetECSqlName ()).append (" WHERE TargetECInstanceId=?");

    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (db, ecsql.c_str ());
    statement.BindId (1, elementId);

    if (BeSQLite::DbResult::BE_SQLITE_ROW != statement.Step ())
        return false;

    return true;
    }
