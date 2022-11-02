/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnChangeSummary.h>
#include    <DgnPlatform/LocksManager.h>

#ifdef USE_PRES_RULES
#include    <ECPresentation/Content.h>
USING_NAMESPACE_BENTLEY_ECPRESENTATION
#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnChangeSummary::FindChangedRelationshipEndIds(ECInstanceIdSet& endInstanceIds, Utf8CP relationshipSchemaName, Utf8CP relationshipClassName, ECRelationshipEnd relationshipEnd)
    {
    ECN::ECClassId relationshipClassId = m_dgndb.Schemas().GetClassId(relationshipSchemaName, relationshipClassName);
    BeAssert(relationshipClassId.IsValid());

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
                UNUSED_VARIABLE(stepStatus);
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
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnChangeSummary::FindUpdatedInstanceIds(ECInstanceIdSet& updatedInstanceIds, Utf8CP schemaName, Utf8CP className)
    {
    ECN::ECClassId classId = m_dgndb.Schemas().GetClassId(schemaName, className);
    BeAssert(classId.IsValid());

    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    QueryByClass(changes, classId, true, ChangeSummary::QueryDbOpcode::Update);

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        ChangeSummary::Instance instance = iter->second;
        updatedInstanceIds.insert(instance.GetInstanceId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnChangeSummary::GetChangedElements(DgnElementIdSet& elementIds, ChangeSummary::QueryDbOpcode queryOpcode)
    {
    ECClassId elClassId = m_dgndb.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);

    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    QueryByClass(changes, elClassId, true, queryOpcode);

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        DgnElementId elementId(iter->first.GetValueUnchecked());
        elementIds.insert(elementId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DgnChangeSummary::ParseClassFullName(Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName)
    {
    bvector<Utf8String> classParts;
    BeStringUtilities::Split(classFullName, ".:", nullptr, classParts);

    if (classParts.size() != 2)
        return ERROR;

    ECN::ECClassCP ecClass = m_dgndb.Schemas().GetClass(classParts[0].c_str(), classParts[1].c_str());
    if (ecClass == nullptr)
        return ERROR;

    schemaName = ecClass->GetSchema().GetName();
    className = ecClass->GetName();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnChangeSummary::GetElementsWithGeometryUpdates(DgnElementIdSet& elementIds)
    {
#ifdef NEEDSWORK_ELEMENT_GEOM_DOES_NOT_EXIST
    // Element -> ElementOwnsGeom -> ElementGeom
    BentleyStatus status = GetElementsWithAspectUpdates(elementIds, "dgn.Element", "dgn.ElementOwnsGeom", "dgn.ElementGeom");
    BeAssert(status == SUCCESS);

    // ElementGeomUsesParts
    ECInstanceIdSet changedGeomUsesPartEnds;
    FindChangedRelationshipEndIds(changedGeomUsesPartEnds, "dgn", "ElementGeomUsesParts", ECRelationshipEnd_Source);
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM dgn.Element el JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom WHERE InVirtualSet(?, elg.ECInstanceId)";
    FindRelatedInstanceIds(elementIds, ecsql, changedGeomUsesPartEnds);

    // GeometryPart
    ECInstanceIdSet updatedGeometryParts;
    FindUpdatedInstanceIds(updatedGeometryParts, "dgn", "GeometryPart");
    ecsql = "SELECT el.ECInstanceId FROM dgn.Element el JOIN dgn.ElementGeom USING dgn.ElementOwnsGeom  JOIN dgn.GeometryPart gp USING dgn.ElementGeomUsesParts WHERE InVirtualSet(?, gp.ECInstanceId)";
    FindRelatedInstanceIds(elementIds, ecsql, updatedGeometryParts);
#else
    elementIds.clear();
#endif
    }

//=======================================================================================
// Used to accumulate changes between versions
// @bsistruct
//=======================================================================================
struct SummaryElementInfo
    {
    BentleyApi::BeSQLite::DbOpcode   m_opcode;
    BentleyApi::ECN::ECClassId       m_ecclassId;

    SummaryElementInfo(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId) : m_opcode(opcode), m_ecclassId(classId) { }
    SummaryElementInfo() { }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod
     +---------------+---------------+---------------+---------------+---------------+------*/
    void AccumulateChange(SummaryElementInfo info)
        {
        // The change should be happening to the same ECClass, as it is supposed to be the same instance
        BeAssert(m_ecclassId == info.m_ecclassId);

        // 1. Insert then modified, still an insertion, so don't do anything
        // 2. Two updates, keep it as update
        if ((m_opcode == DbOpcode::Insert && info.m_opcode == DbOpcode::Update) ||
                (m_opcode == DbOpcode::Update && info.m_opcode == DbOpcode::Update))
            return;

        // 3. Element was inserted then deleted, invalidate this change
        if (m_opcode == DbOpcode::Insert && info.m_opcode == DbOpcode::Delete)
            {
            Invalidate();
            return;
            }

        // 4. Update and then deleted, so we accumulate as a deletion
        if (m_opcode == DbOpcode::Update && info.m_opcode == DbOpcode::Delete)
            {
            m_opcode = DbOpcode::Delete;
            return;
            }

        // Should never happen, the combination of opcodes is invalid
        // e.g. Deleted then Inserted, Deleted then Deleted, Deleted then Modified, etc.
        BeAssert(false && "Invalid combination of opcodes. Cannot accumulate.");
        }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod
     +---------------+---------------+---------------+---------------+---------------+------*/
    bool IsValid()
        {
        return (m_opcode != (DbOpcode)0 && m_ecclassId.Validate());
        }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod
     +---------------+---------------+---------------+---------------+---------------+------*/
    void Invalidate()
        {
        m_opcode = (DbOpcode)0;
        m_ecclassId.Invalidate();
        }
    }; // SummaryElementInfo

//=======================================================================================
// @bsistruct
//=======================================================================================
struct CompareChangeSet : BentleyApi::BeSQLite::ChangeSet
{
    /*---------------------------------------------------------------------------------**//**
     * @bsimethod
     +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyApi::BeSQLite::ChangeSet::ConflictResolution _OnConflict(BentleyApi::BeSQLite::ChangeSet::ConflictCause cause, BentleyApi::BeSQLite::Changes::Change iter) override
        {
        Utf8CP tableName = nullptr;
        int nCols, indirect;
        DbOpcode opcode;
        DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(result == BE_SQLITE_OK);
        UNUSED_VARIABLE(result);

        if (cause == ChangeSet::ConflictCause::NotFound && opcode == DbOpcode::Delete) // a delete that is already gone.
            return ChangeSet::ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.

        return ChangeSet::ConflictResolution::Replace;
        }
}; // CompareChangeSet

typedef bmap<DgnElementId, SummaryElementInfo> ChangedElementsMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt GetAppliableChangesets(DgnDbR currentDb, bvector<DgnRevisionPtr> const& changesets, bvector<bvector<DgnRevisionPtr>>& appliableChangesets)
    {
    bvector<DgnRevisionPtr> currentAppliableChangesets;

    // Get a collection of changesets that can be put together into a change summary
    for (DgnRevisionPtr changeset : changesets)
        {
        if (changeset->ContainsSchemaChanges(currentDb))
            {
            // Set the current appliable changesets we have
            appliableChangesets.push_back(currentAppliableChangesets);
            // Add the schema changeset as a separate change summary
            bvector<DgnRevisionPtr> schemaChangeOnly;
            schemaChangeOnly.push_back(changeset);
            appliableChangesets.push_back(schemaChangeOnly);
            // Clear to keep going with a new vector for follow up changesets
            currentAppliableChangesets.clear();
            continue;
            }

        currentAppliableChangesets.push_back(changeset);
        }

    // Push the last list
    if (!currentAppliableChangesets.empty())
        appliableChangesets.push_back(currentAppliableChangesets);

    return appliableChangesets.empty() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void   ProcessChangedElements(ChangedElementsMap& changedElements, DgnElementIdSet const& elementIds, DbOpcode opcode, DgnDbR currentDb, DgnDbPtr targetDb)
    {
    for (DgnElementId id : elementIds)
        {
        DgnElementCPtr element = currentDb.Elements().GetElement(id);
        if (!element.IsValid())
            element = targetDb->Elements().GetElement(id);
        if (!element.IsValid())
            continue;

        SummaryElementInfo info(opcode, element->GetElementClass()->GetId());
        if (changedElements.find(id) != changedElements.end())
            changedElements[id].AccumulateChange(info);
        else
            changedElements[id] = info;

        // If the changed element got invalidated during accumulation of changes (e.g. inserted then deleted)
        // get rid of the changed element completely
        if (!changedElements[id].IsValid())
            changedElements.erase(id);
        }
    }

#ifdef USE_PRES_RULES
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr   VersionCompareChangeSummary::GetContentDescriptorForElements(DgnDbR db)
    {
    static ContentDescriptorCPtr s_descriptorCache = nullptr;
    if (s_descriptorCache.IsValid())
        return s_descriptorCache;

    // Obtain the content descriptor for element and their derived classes
    // this should only be called once per comparison and it should be cached
    // we use this to obtain the related property paths of elements later on
    bvector<ECClassCP> ecclasses;

    // Make sure this is the right element class
    ECClassCP elementClass = db.Schemas().GetClass("BisCore", "Element");

    BeAssert(nullptr != elementClass);
    if (nullptr == elementClass)
        return nullptr;

    AddClassAndDerivedClasses(ecclasses, db, elementClass);

    // TODO: Set the presentation rules based on application ??
    SelectionInfo selectionInfo(ecclasses);
    ECPresentationManager::ContentOptions options("Items");

    // TFS#749084: Temporary and terrible band-aid fix for descriptor getting too many classes until we get Grigas to look at performance issues
    int threshold = 120;
    if (ecclasses.size() > threshold)
        ecclasses.resize(threshold);

    s_descriptorCache = ECPresentationManager::GetManager().GetContentDescriptor(db, ContentDisplayType::PropertyPane, selectionInfo, options.GetJson());
    BeAssert(descriptor.IsValid());

    return s_descriptorCache;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   ProcessChangeSummary(ChangedElementsMap& changedElements, DgnChangeSummary* changeSummary, DgnDbR currentDb, DgnDbPtr targetDb)
    {
    DgnElementIdSet insertedElemIds, updatedElemIds, deletedElemIds;
    // Get instances of elements that changed
    changeSummary->GetChangedElements(insertedElemIds, ChangeSummary::QueryDbOpcode::Insert);
    changeSummary->GetChangedElements(updatedElemIds, ChangeSummary::QueryDbOpcode::Update);
    changeSummary->GetChangedElements(deletedElemIds, ChangeSummary::QueryDbOpcode::Delete);

#ifdef USE_PRES_RULES
    // TODO: Use newer Db
    ContentDescriptorCPtr descriptorCache = GetContentDescriptorForElements(currentDb);
    if (!descriptorCache.IsValid())
        return ERROR;

    // Get all elements that changed based on instances that are related through what's presented to the user
    for (SelectClassInfo const& selectInfo : descriptorCache->GetSelectClasses())
        {
        bvector<RelatedClassPath> allPaths = selectInfo.GetRelatedPropertyPaths();
        for (RelatedClassPath path : allPaths)
            {
            for (RelatedClass relatedClass : path)
                {
                changeSummary->GetElementsWithAspectUpdates(updatedElemIds, relatedClass.GetSourceClass()->GetFullName(), relatedClass.GetRelationship()->GetFullName(), relatedClass.GetTargetClass()->GetFullName());
                }
            }
        }

    // Filter out modified elements that are contained in both inserted or deleted element lists, as those take precedence
    // e.g. If an element got inserted, and an aspect of that element also got inserted, we mark the insertion of an aspect
    // as an update, but in reality the element got inserted. To handle this, just give more important to insertion/deletions
    // than modifies where we find duplicates
    bvector<DgnElementId> filterList;
    for (DgnElementId const& elementId : updatedElemIds)
        {
        if (insertedElemIds.find(elementId) != insertedElemIds.end())
            filterList.push_back(elementId);
        if (deletedElemIds.find(elementId) != deletedElemIds.end())
            filterList.push_back(elementId);
        }

    for (DgnElementId const& elementId : filterList)
        updatedElemIds.erase(elementId);
#endif

    // Process the changed elements and accumulate them
    ProcessChangedElements(changedElements, insertedElemIds, DbOpcode::Insert, currentDb, targetDb);
    ProcessChangedElements(changedElements, updatedElemIds, DbOpcode::Update, currentDb, targetDb);
    ProcessChangedElements(changedElements, deletedElemIds, DbOpcode::Delete, currentDb, targetDb);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    DgnChangeSummary::GetChangedElements(DgnDbR currentDb, DgnDbPtr targetDb, bvector<DgnRevisionPtr> const & changesets, bvector<DgnElementId>& elementIds, bvector<ECN::ECClassId>& ecclassIds, bvector<BeSQLite::DbOpcode>& opcodes)
    {
    if (changesets.empty())
        return ERROR;

    bvector<bvector<DgnRevisionPtr>> appliableChangesets;
    if (SUCCESS != GetAppliableChangesets(currentDb, changesets, appliableChangesets))
        return ERROR;

    if (appliableChangesets.empty())
        return ERROR;

    // Map used to accumulate the changed elements across summaries
    ChangedElementsMap changedElements;

    // Construct change summaries
    for (bvector<DgnRevisionPtr>& changesetList : appliableChangesets)
        {
        ChangeGroup group;
        // TODO: Get newer Db
        DgnChangeSummary changeSummary(currentDb);
        // Merge changes into a change group
        for (auto changeset : changesetList)
            {
            BeFileNameCR changesetChangesFile = changeset->GetRevisionChangesFile();
            RevisionChangesFileReader changeStream(changesetChangesFile, currentDb);
            changeStream.AddToChangeGroup(group);
            }

        // Put together the changeset
        CompareChangeSet set;
        set.FromChangeGroup(group);
        changeSummary.FromChangeSet(set);
        // Process the change summary to get all changed elements
        ProcessChangeSummary(changedElements, &changeSummary, currentDb, targetDb);
        }

    if (changedElements.empty())
        return ERROR;

    // Put the elements in the output vectors
    for (bmap<DgnElementId,SummaryElementInfo>::iterator it = changedElements.begin(); it != changedElements.end(); ++it)
        {
        elementIds.push_back(it->first);
        ecclassIds.push_back(it->second.m_ecclassId);
        opcodes.push_back(it->second.m_opcode);
        }

    return elementIds.empty() ? ERROR : SUCCESS;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
