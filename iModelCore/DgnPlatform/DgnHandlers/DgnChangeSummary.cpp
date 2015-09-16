/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnChangeSummary.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnHandlers/DgnChangeSummary.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE_EC 
USING_NAMESPACE_EC

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummary::FindChangedRelationshipEndIds(ECInstanceIdSet& endInstanceIds, Utf8CP relationshipSchemaName, Utf8CP relationshipClassName, ECRelationshipEnd relationshipEnd)
    {
    ECN::ECClassId relationshipClassId = m_dgndb.Schemas().GetECClassId(relationshipSchemaName, relationshipClassName);
    BeAssert(relationshipClassId);

    Utf8CP endInstanceIdAccessStr = (relationshipEnd == ECRelationshipEnd_Source) ? "SourceECInstanceId" : "TargetECInstanceId";

    bmap<ECInstanceId, ChangeSummary::Instance> relationshipChanges;
    QueryByClass(relationshipChanges, relationshipClassId, true, ChangeSummary::QueryDbOpcode::All);

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = relationshipChanges.begin(); iter != relationshipChanges.end(); iter++)
        {
        ChangeSummary::Instance relInstance = iter->second;

        ECInstanceId oldEndInstanceId, newEndInstanceId, currentEndInstanceId;
        DbOpcode dbOpcode = relInstance.GetDbOpcode();
        if (dbOpcode == DbOpcode::Insert)
            {
            newEndInstanceId = relInstance.GetNewValue(endInstanceIdAccessStr).GetValueId<ECInstanceId>();
            BeAssert(newEndInstanceId.IsValid());
            }
        else if (dbOpcode == DbOpcode::Delete)
            {
            oldEndInstanceId = relInstance.GetOldValue(endInstanceIdAccessStr).GetValueId<ECInstanceId>();
            BeAssert(oldEndInstanceId.IsValid());
            }
        else /* if (dbOpcode == DbOpcode::Update) */
            {
            // The end instance id may not be part of the update record - look in the current database if it's not present. 
            if (relInstance.HasValue(endInstanceIdAccessStr))
                {
                newEndInstanceId = relInstance.GetNewValue(endInstanceIdAccessStr).GetValueId<ECInstanceId>();
                oldEndInstanceId = relInstance.GetOldValue(endInstanceIdAccessStr).GetValueId<ECInstanceId>();
                }
            else
                {
                Utf8PrintfString sql("SELECT %s FROM %s.%s WHERE ECInstanceId=?", endInstanceIdAccessStr, relationshipSchemaName, relationshipClassName);

                CachedECSqlStatementPtr stmt = m_statementCache.GetPreparedStatement(m_dgndb, sql.c_str());
                BeAssert(stmt.IsValid());
                stmt->BindId(1, relInstance.GetInstanceId());

                ECSqlStepStatus stepStatus = stmt->Step();
                BeAssert(stepStatus == ECSqlStepStatus::HasRow);

                currentEndInstanceId = stmt->GetValueId<ECInstanceId>(0);
                }
            }

        if (oldEndInstanceId.IsValid())
            endInstanceIds.insert(oldEndInstanceId);
        if (newEndInstanceId.IsValid())
            endInstanceIds.insert(newEndInstanceId);
        if (currentEndInstanceId.IsValid())
            endInstanceIds.insert(currentEndInstanceId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummary::FindUpdatedInstanceIds(ECInstanceIdSet& updatedInstanceIds, Utf8CP schemaName, Utf8CP className)
    {
    ECN::ECClassId classId = m_dgndb.Schemas().GetECClassId(schemaName, className);
    BeAssert(classId);

    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    QueryByClass(changes, classId, true, ChangeSummary::QueryDbOpcode::Update);

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        ChangeSummary::Instance instance = iter->second;
        updatedInstanceIds.insert(instance.GetInstanceId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummary::FindRelatedInstanceIds(DgnElementIdSet& relatedElements, Utf8CP ecsql, ECInstanceIdSet const& inInstances)
    {
    CachedECSqlStatementPtr stmt = m_statementCache.GetPreparedStatement(m_dgndb, ecsql);
    BeAssert(stmt.IsValid());

    stmt->BindInt64(1, (int64_t) &inInstances);

    ECSqlStepStatus stepStatus;
    while ((stepStatus = stmt->Step()) == ECSqlStepStatus::HasRow)
        {
        relatedElements.insert(stmt->GetValueId<DgnElementId>(0));
        }
    BeAssert(stepStatus == ECSqlStepStatus::Done);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummary::GetChangedElements(DgnElementIdSet& elementIds, ChangeSummary::QueryDbOpcode queryOpcode)
    {
    ECClassId elClassId = m_dgndb.Schemas().GetECClassId("dgn", "Element");

    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    QueryByClass(changes, elClassId, true, queryOpcode);

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        DgnElementId elementId(iter->first.GetValueUnchecked());
        elementIds.insert(elementId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummary::GetElementsWithItemUpdates(DgnElementIdSet& elementIds)
    {
    // Element -> ElementOwnsItem -> ElementItem
    BentleyStatus status = GetElementsWithAspectUpdates(elementIds, "dgn.Element", "dgn.ElementOwnsItem", "dgn.ElementItem");
    BeAssert(status == SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnChangeSummary::ParseClassFullName(Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName)
    {
    bvector<Utf8String> classParts;
    BeStringUtilities::Split(classFullName, ".:", nullptr, classParts);

    if (classParts.size() != 2)
        return ERROR;

    ECN::ECClassCP ecClass = m_dgndb.Schemas().GetECClass(classParts[0].c_str(), classParts[1].c_str());
    if (ecClass == nullptr)
        return ERROR;

    schemaName = ecClass->GetSchema().GetName();
    className = ecClass->GetName();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnChangeSummary::GetElementsWithAspectUpdates(DgnElementIdSet& elementIds, Utf8CP elementClassFullName, Utf8CP aspectRelationshipClassFullName, Utf8CP aspectClassFullName)
    {
    BentleyStatus status;

    Utf8String elementSchemaName, elementClassName;
    status = ParseClassFullName(elementSchemaName, elementClassName, elementClassFullName);
    if (SUCCESS != status)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String aspectSchemaName, aspectClassName;
    status = ParseClassFullName(aspectSchemaName, aspectClassName, aspectClassFullName);
    if (SUCCESS != status)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String aspectRelationshipSchemaName, aspectRelationshipClassName;
    status = ParseClassFullName(aspectRelationshipSchemaName, aspectRelationshipClassName, aspectRelationshipClassFullName);
    if (SUCCESS != status)
        {
        BeAssert(false);
        return ERROR;
        }

    // AspectRelationship (e.g., ElementOwnsGeom)
    ECInstanceIdSet changedAspectRelEnds;
    FindChangedRelationshipEndIds(changedAspectRelEnds, aspectRelationshipSchemaName.c_str(), aspectRelationshipClassName.c_str(), ECRelationshipEnd_Source);
    Utf8PrintfString ecSql("SELECT el.ECInstanceId FROM %s.%s el WHERE InVirtualSet(?, el.ECInstanceId)", elementSchemaName.c_str(), elementClassName.c_str());
    FindRelatedInstanceIds(elementIds, ecSql.c_str(), changedAspectRelEnds);

    // Aspect (e.g., ElementGeom)
    ECInstanceIdSet changedAspects;
    FindUpdatedInstanceIds(changedAspects, aspectSchemaName.c_str(), aspectClassName.c_str());
    Utf8PrintfString ecSql2("SELECT el.ECInstanceId FROM %s.%s el JOIN %s.%s elg USING %s.%s WHERE InVirtualSet(?, elg.ECInstanceId)",
                            elementSchemaName.c_str(), elementClassName.c_str(), aspectSchemaName.c_str(), aspectClassName.c_str(), aspectRelationshipSchemaName.c_str(), aspectRelationshipClassName.c_str());
    FindRelatedInstanceIds(elementIds, ecSql2.c_str(), changedAspects);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummary::GetElementsWithGeometryUpdates(DgnElementIdSet& elementIds)
    {
    // GeometricElement -> ElementOwnsGeom -> ElementGeom
    BentleyStatus status = GetElementsWithAspectUpdates(elementIds, "dgn.GeometricElement", "dgn.ElementOwnsGeom", "dgn.ElementGeom");
    BeAssert(status == SUCCESS);

    // ElementGeomUsesParts
    ECInstanceIdSet changedGeomUsesPartEnds;
    FindChangedRelationshipEndIds(changedGeomUsesPartEnds, "dgn", "ElementGeomUsesParts", ECRelationshipEnd_Source);
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM dgn.GeometricElement el JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom WHERE InVirtualSet(?, elg.ECInstanceId)";
    FindRelatedInstanceIds(elementIds, ecsql, changedGeomUsesPartEnds);

    // GeomPart
    ECInstanceIdSet updatedGeomParts;
    FindUpdatedInstanceIds(updatedGeomParts, "dgn", "GeomPart");
    ecsql = "SELECT el.ECInstanceId FROM dgn.GeometricElement el JOIN dgn.ElementGeom USING dgn.ElementOwnsGeom  JOIN dgn.GeomPart gp USING dgn.ElementGeomUsesParts WHERE InVirtualSet(?, gp.ECInstanceId)";
    FindRelatedInstanceIds(elementIds, ecsql, updatedGeomParts);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
