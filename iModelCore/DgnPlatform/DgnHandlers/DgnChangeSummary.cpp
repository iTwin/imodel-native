/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnChangeSummary.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnChangeSummary.h>

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
            if (relInstance.ContainsValue(endInstanceIdAccessStr))
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

                DbResult stepStatus = stmt->Step();
                BeAssert(stepStatus == BE_SQLITE_ROW);

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

    DbResult stepStatus;
    while ((stepStatus = stmt->Step()) == BE_SQLITE_ROW)
        {
        relatedElements.insert(stmt->GetValueId<DgnElementId>(0));
        }
    BeAssert(stepStatus == BE_SQLITE_DONE);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnChangeSummary::ModelIterator::ModelIterator(DgnChangeSummary const& summary, DgnChangeSummary::QueryDbOpcode opcodes)
    : Iterator(summary, summary.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Model), opcodes)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnChangeSummary::ElementIterator::ElementIterator(DgnChangeSummary const& summary, DgnChangeSummary::QueryDbOpcode opcodes)
    : Iterator(summary, summary.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element), opcodes)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnChangeSummary::ElementEntry::GetModelId(bool old) const
    {
    auto instance = GetImpl().GetInstance();
    DgnModelId modelId;
    if (instance.ContainsValue("ModelId"))
        {
        DbDupValue value = (old ? instance.GetOldValue("ModelId") : instance.GetNewValue("ModelId"));
        if (value.IsValid())
            modelId = value.GetValueId<DgnModelId>();
        }
    else if (DbOpcode::Delete != GetDbOpcode())
        {
        static const Utf8CP s_selectModelId { "SELECT ModelId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?" };
        DgnDbR dgndb = static_cast<DgnChangeSummary const&>(GetImpl().GetChangeSummary()).GetDgnDb();
        CachedStatementPtr stmt = dgndb.Elements().GetStatement(s_selectModelId);
        stmt->BindId(1, instance.GetInstanceId());
        if (BE_SQLITE_ROW == stmt->Step())
            modelId = stmt->GetValueId<DgnModelId>(0);
        }

    return modelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static AuthorityIssuedCode getOriginalCode(T const& entry, DbOpcode op)
    {
    AuthorityIssuedCode code, currentCode;
    switch (op)
        {
        case DbOpcode::Insert:
            return code; // no original code...
        case DbOpcode::Update:
            currentCode = entry.GetCurrentCode();
            break;
        }

    auto instance = entry.GetImpl().GetInstance();
    DbDupValue oldAuthId = instance.GetOldValue("Code.AuthorityId"),
               oldNamespace = instance.GetOldValue("Code.NameSpace"),
               oldValue = instance.GetOldValue("Code.Value");

    if (DbOpcode::Delete == op)
        {
        if (oldAuthId.IsValid() && oldNamespace.IsValid() && oldValue.IsValid())
            code.From(oldAuthId.GetValueId<DgnAuthorityId>(), oldValue.GetValueText(), oldNamespace.GetValueText());
        }
    else
        {
        if (oldAuthId.IsValid() || oldNamespace.IsValid() || oldValue.IsValid())
            {
            DgnAuthorityId authId = oldAuthId.IsValid() ? oldAuthId.GetValueId<DgnAuthorityId>() : currentCode.GetAuthority();
            Utf8String nameSpace = oldNamespace.IsValid() ? oldNamespace.GetValueText() : currentCode.GetNamespace();
            Utf8String value = oldValue.IsValid() ? oldValue.GetValueText() : currentCode.GetValue();
            code.From(authId, value, nameSpace);
            }
        else
            {
            code = currentCode;
            }
        }

    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
AuthorityIssuedCode DgnChangeSummary::ElementEntry::GetCode(bool old) const
    {
    auto op = GetDbOpcode();
    if (old)
        return getOriginalCode(*this, op);

    if (DbOpcode::Delete == op)
        return AuthorityIssuedCode(); // no new code...

    DgnDbR db = static_cast<DgnChangeSummary const&>(GetImpl().GetChangeSummary()).GetDgnDb();
    auto elem = db.Elements().GetElement(GetElementId());
    BeAssert(elem.IsValid());
    return elem.IsValid() ? elem->GetCode() : AuthorityIssuedCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
AuthorityIssuedCode DgnChangeSummary::ModelEntry::GetCode(bool old) const
    {
    auto op = GetDbOpcode();
    if (old)
        return getOriginalCode(*this, op);

    if (DbOpcode::Delete == op)
        return AuthorityIssuedCode();

    DgnDbR db = static_cast<DgnChangeSummary const&>(GetImpl().GetChangeSummary()).GetDgnDb();
    auto model = db.Models().GetModel(GetModelId());
    BeAssert(model.IsValid());
    return model.IsValid() ? model->GetCode() : AuthorityIssuedCode();
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
    // Element -> ElementOwnsGeom -> ElementGeom
    BentleyStatus status = GetElementsWithAspectUpdates(elementIds, "dgn.Element", "dgn.ElementOwnsGeom", "dgn.ElementGeom");
    BeAssert(status == SUCCESS);

    // ElementGeomUsesParts
    ECInstanceIdSet changedGeomUsesPartEnds;
    FindChangedRelationshipEndIds(changedGeomUsesPartEnds, "dgn", "ElementGeomUsesParts", ECRelationshipEnd_Source);
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM dgn.Element el JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom WHERE InVirtualSet(?, elg.ECInstanceId)";
    FindRelatedInstanceIds(elementIds, ecsql, changedGeomUsesPartEnds);

    // GeomPart
    ECInstanceIdSet updatedGeomParts;
    FindUpdatedInstanceIds(updatedGeomParts, "dgn", "GeomPart");
    ecsql = "SELECT el.ECInstanceId FROM dgn.Element el JOIN dgn.ElementGeom USING dgn.ElementOwnsGeom  JOIN dgn.GeomPart gp USING dgn.ElementGeomUsesParts WHERE InVirtualSet(?, gp.ECInstanceId)";
    FindRelatedInstanceIds(elementIds, ecsql, updatedGeomParts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void insertCode(AuthorityIssuedCodeSet& into, AuthorityIssuedCode const& code, AuthorityIssuedCodeSet& ifNotIn)
    {
    if (!code.IsValid())
        return;

    // At most, we can expect one discard and one assign per unique code.
    BeAssert(into.end() == into.find(code));

    auto existing = ifNotIn.find(code);
    if (ifNotIn.end() != existing)
        {
        // Code was discarded by one and assigned to another within the same changeset...so no net change
        ifNotIn.erase(existing);
        return;
        }

    into.insert(code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void collectCodes(AuthorityIssuedCodeSet& assigned, AuthorityIssuedCodeSet& discarded, T& collection)
    {
    for (auto const& entry : collection)
        {
        if (entry.IsIndirectChange())
            continue;

        auto oldCode = entry.GetOriginalCode(),
             newCode = entry.GetCurrentCode();

        if (oldCode == newCode)
            continue;

        insertCode(discarded, oldCode, assigned);
        insertCode(assigned, newCode, discarded);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnChangeSummary::GetCodes(AuthorityIssuedCodeSet& assigned, AuthorityIssuedCodeSet& discarded) const
    {
    assigned.clear();
    discarded.clear();

    auto elems = MakeElementIterator();
    collectCodes(assigned, discarded, elems);
    auto models = MakeModelIterator();
    collectCodes(assigned, discarded, models);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
