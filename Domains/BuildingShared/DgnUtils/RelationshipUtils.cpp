/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/BuildingDgnUtilsApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator RelationshipUtils::MakeTargetIterator
(
    Dgn::DgnDbR db,
    ECN::ECRelationshipClassCR relationshipClass,
    Dgn::DgnElementId sourceId
)
    {
    Utf8String ecsql("SELECT TargetECInstanceId FROM ");
    ecsql.append(relationshipClass.GetECSqlName()).append(" WHERE SourceECInstanceId=?");
    
    ElementIdIterator iterator;
    iterator.Prepare(db, ecsql.c_str(), 0);
    iterator.GetStatement()->BindId(1, sourceId);

    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator RelationshipUtils::MakeSourceIterator
(
    Dgn::DgnDbR db,
    ECN::ECRelationshipClassCR relationshipClass,
    Dgn::DgnElementId targetId
)
    {
    Utf8String ecsql("SELECT SourceECInstanceId FROM ");
    ecsql.append(relationshipClass.GetECSqlName()).append(" WHERE TargetECInstanceId=?");

    ElementIdIterator iterator;
    iterator.Prepare(db, ecsql.c_str(), 0);
    iterator.GetStatement()->BindId(1, targetId);

    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
BeSQLite::EC::ECInstanceKey RelationshipUtils::InsertRelationship
(
    Dgn::DgnDbR db, 
    ECN::ECRelationshipClassCR relationshipClass, 
    Dgn::DgnElementId sourceId, 
    Dgn::DgnElementId targetId
)
    {
    BeSQLite::EC::ECInstanceKey relKey;

    if (!sourceId.IsValid())
        {
        BeAssert(sourceId.IsValid());
        return relKey;
        }
    if (!targetId.IsValid())
        {
        BeAssert(targetId.IsValid());
        return relKey;
        }

    BeSQLite::DbResult result = db.InsertLinkTableRelationship(
        relKey,
        relationshipClass,
        BeSQLite::EC::ECInstanceId(sourceId),
        BeSQLite::EC::ECInstanceId(targetId));
    BeAssert(result == BeSQLite::DbResult::BE_SQLITE_OK);

    return relKey;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus RelationshipUtils::DeleteRelationships
(
    Dgn::DgnDbR db,
    ECN::ECRelationshipClassCR relationshipClass,
    Dgn::DgnElementId sourceId,
    Dgn::DgnElementId targetId
)
    {
    if (!sourceId.IsValid())
        {
        BeAssert(sourceId.IsValid());
        return Dgn::DgnDbStatus::BadElement;
        }
    if (!targetId.IsValid())
        {
        BeAssert(targetId.IsValid());
        return Dgn::DgnDbStatus::BadElement;
        }

    BeSQLite::DbResult result = db.DeleteLinkTableRelationships(relationshipClass.GetECSqlName().c_str(), sourceId, targetId);
    return BeSQLite::DbResult::BE_SQLITE_OK == result ? Dgn::DgnDbStatus::Success : Dgn::DgnDbStatus::SQLiteError;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool RelationshipUtils::RelationshipExists
(
    Dgn::DgnDbR db,
    ECN::ECRelationshipClassCR relationshipClass,
    Dgn::DgnElementId sourceId,
    Dgn::DgnElementId targetId
)
    {
    Utf8String ecsql("SELECT 1 FROM ");
    ecsql.append(relationshipClass.GetECSqlName()).append(" WHERE TargetECInstanceId=? AND SourceECInstanceId=?");

    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare(db, ecsql.c_str());
    statement.BindId(1, targetId);
    statement.BindId(2, sourceId);

    BeSQLite::DbResult result = statement.Step();
    return BeSQLite::DbResult::BE_SQLITE_ROW == result;
    }