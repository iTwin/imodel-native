/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VersionCompareChangeSummary.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnElement.h>
#include <Bentley/ByteStream.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <Bentley/BeConsole.h>
#include <DgnPlatform/DgnDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY

#define STATEMENT_CACHE_SIZE 20

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
VersionCompareChangeSummary::~VersionCompareChangeSummary()
    {
    CleanUp();

    // Clean up target db temporary file
    if (m_targetDb.IsValid())
        {
        BeFileName tempFileName = m_targetDb->GetFileName();
        m_targetDb->CloseDb();
        m_targetDb = NULL;
        if (BeFileNameStatus::Success != tempFileName.BeDeleteFile())
            {
            BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareChangeSummary::CleanUp()
    {
    // Clear statement cache
    if (nullptr != m_statementCache)
        {
        delete m_statementCache;
        m_statementCache = nullptr;
        }

    m_changedInstances.clear();
    m_changedElements.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
void    VersionCompareChangeSummary::FindChangedRelationshipEndIds (DgnChangeSummary* changeSummary, ECInstanceIdSet& endInstanceIds, Utf8CP relationshipSchemaName, Utf8CP relationshipClassName, ECRelationshipEnd relationshipEnd, ChangeSummary::QueryDbOpcode opcode)
    {
    ECN::ECClassId relationshipClassId = m_targetDb->Schemas().GetClassId(relationshipSchemaName, relationshipClassName);
    BeAssert(relationshipClassId.IsValid());

    Utf8CP endInstanceIdAccessStr = (relationshipEnd == ECRelationshipEnd_Source) ? "SourceECInstanceId" : "TargetECInstanceId";

    bmap<ECInstanceId, ChangeSummary::Instance> relationshipChanges;
    changeSummary->QueryByClass(relationshipChanges, relationshipClassId, true, opcode);

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

                if (nullptr == m_statementCache)
                    m_statementCache = new ECSqlStatementCache(STATEMENT_CACHE_SIZE);

                CachedECSqlStatementPtr stmt = m_statementCache->GetPreparedStatement(*m_targetDb, sql.c_str());
                BeAssert(stmt.IsValid());
                stmt->BindId(1, relInstance.GetInstanceId());

                DbResult stepStatus = stmt->Step();
                BeAssert(stepStatus == BeSQLite::BE_SQLITE_ROW);

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
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
void    VersionCompareChangeSummary::FindRelatedInstanceIds(ECInstanceKeySet& relatedInstances, Utf8CP ecsql, ECInstanceIdSet const& inInstances, ChangeSummary::QueryDbOpcode opcode)
    {
    if (nullptr == m_statementCache)
        m_statementCache = new ECSqlStatementCache(STATEMENT_CACHE_SIZE);

    CachedECSqlStatementPtr stmt = m_statementCache->GetPreparedStatement(*m_targetDb, ecsql);

    if (stmt.IsValid())
        {
        BeAssert(stmt.IsValid());

        stmt->BindInt64(1, (int64_t) &inInstances);

        DbResult stepStatus;
        while ((stepStatus = stmt->Step()) == BeSQLite::BE_SQLITE_ROW)
            {
            ECInstanceKey key (stmt->GetValueId<ECClassId>(1), stmt->GetValueId<ECInstanceId>(0));
            relatedInstances.insert(key);
            }

        BeAssert(stepStatus == BeSQLite::BE_SQLITE_DONE);
        }
    else
        {
        // If we couldn't cache the statement or something failed, try with a normal statement
        ECSqlStatement stmt2;
        ECSqlStatus status = stmt2.Prepare(*m_targetDb, ecsql);
        if (status == ECSqlStatus::InvalidECSql)
            {
            // BeAssert(false && "Invalid ECSql");
            return;
            }

        if (status == ECSqlStatus::Error)
            {
            BeAssert(false && "Failed to prepare statement");
            return;
            }

        stmt2.BindInt64(1, (int64_t) &inInstances);
        DbResult stepStatus;
        while ((stepStatus = stmt2.Step()) == BeSQLite::BE_SQLITE_ROW)
            {
            ECInstanceKey key (stmt->GetValueId<ECClassId>(1), stmt->GetValueId<ECInstanceId>(0));
            relatedInstances.insert(key);
            }

        BeAssert(stepStatus == BeSQLite::BE_SQLITE_DONE);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
void    VersionCompareChangeSummary::FindUpdatedInstanceIds(DgnChangeSummary* changeSummary, ECInstanceIdSet& updatedInstanceIds, Utf8CP schemaName, Utf8CP className, ChangeSummary::QueryDbOpcode opcode)
    {
    ECN::ECClassId classId = m_targetDb->Schemas().GetClassId(schemaName, className);
    BeAssert(classId.IsValid());

    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    changeSummary->QueryByClass(changes, classId, true, opcode);

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        ChangeSummary::Instance instance = iter->second;
        updatedInstanceIds.insert(instance.GetInstanceId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::ParseClassFullName(Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName)
    {
    bvector<Utf8String> classParts;
    BeStringUtilities::Split(classFullName, ".:", nullptr, classParts);

    if (classParts.size() != 2)
        return ERROR;

    ECN::ECClassCP ecClass = m_targetDb->Schemas().GetClass(classParts[0].c_str(), classParts[1].c_str());
    if (ecClass == nullptr)
        return ERROR;

    schemaName = ecClass->GetSchema().GetName();
    className = ecClass->GetName();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::GetInstancesWithAspectUpdates(DgnChangeSummary* changeSummary, ECInstanceKeySet& instances, Utf8CP elementClassFullName, Utf8CP aspectRelationshipClassFullName, Utf8CP aspectClassFullName)
    {
    StatusInt status;

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
    // Find the source end (i.e., Element end) of all the changed relationship instances of type ElementOwnsGeom.
    ECInstanceIdSet changedAspectRelEnds;
    // TODO: Change ECRelationshipEnd_Source/Target based on the direction of the relationship!
    FindChangedRelationshipEndIds(changeSummary, changedAspectRelEnds, aspectRelationshipSchemaName.c_str(), aspectRelationshipClassName.c_str(), ECRelationshipEnd_Source, ChangeSummary::QueryDbOpcode::Update);
    // Narrow the above list of Elements to the specified class
    Utf8PrintfString ecSql("SELECT el.ECInstanceId, el.ECClassId FROM %s.%s el WHERE InVirtualSet(?, el.ECInstanceId)", elementSchemaName.c_str(), elementClassName.c_str());
    FindRelatedInstanceIds(instances, ecSql.c_str(), changedAspectRelEnds, ChangeSummary::QueryDbOpcode::Update);

    // Aspect (e.g., ElementGeom)
    ECInstanceIdSet changedAspects;
    FindUpdatedInstanceIds(changeSummary, changedAspects, aspectSchemaName.c_str(), aspectClassName.c_str(), ChangeSummary::QueryDbOpcode::Update);
    Utf8PrintfString ecSql2("SELECT el.ECInstanceId, el.ECClassId FROM %s.%s el JOIN %s.%s elg USING %s.%s WHERE InVirtualSet(?, elg.ECInstanceId)",
                            elementSchemaName.c_str(), elementClassName.c_str(), aspectSchemaName.c_str(), aspectClassName.c_str(), aspectRelationshipSchemaName.c_str(), aspectRelationshipClassName.c_str());
    FindRelatedInstanceIds(instances, ecSql2.c_str(), changedAspects, ChangeSummary::QueryDbOpcode::Update);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetAppliableChangesets(bvector<bvector<DgnRevisionPtr>>& appliableChangesets)
    {
    // TODO: Restructure code: no need for putting together changesets, it is less performant
    for (DgnRevisionPtr changeset : m_changesets)
        {
        bvector<DgnRevisionPtr> current;
        current.push_back(changeset);
        appliableChangesets.push_back(current);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* This method will create a list of change summaries that are separated by schema changes
* For example, let's say we have the following changesets:
* C1, C2, C3(SC), C4, C5, C6(SC), C7(SC)
* Consider that (SC) means the changeset contains schema changes
* The change summary API does not let us merge changesets that contain those schema changes
* so we must separate them into different change summaries that contain a progression of
* changesets without schema changes
* This method would generate 5 change-summaries based on the 7 provided changesets, each
* containing the following collections of changesets:
* [C1, C2], [C3], [C4, C5], [C6], [C7]
*
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    VersionCompareChangeSummary::ProcessChangesets()
    {
    if (m_changesets.empty())
        return ERROR;

    CleanUp();

    // Clone the m_db
    m_targetDb = CloneDb(m_db);

    if (!m_targetDb.IsValid())
        {
        BeAssert(false && "Problem cloning Db");
        return ERROR;
        }

    // Presentation rules need to know about the db
    m_presentationManager.GetConnections().CreateConnection(*m_targetDb);

    // Construct change summaries
    for (DgnRevisionPtr changeset : m_changesets)
        {
        // When going forwards, we need to apply the changeset before we process it
        if (!m_backwardsComparison)
            {
            bvector<DgnRevisionPtr> changesets;
            changesets.push_back(changeset);
            if (SUCCESS != RollTargetDb(changesets, m_backwardsComparison))
                return ERROR;
            }

        // Create a summary with the current target db
        DgnChangeSummary* changeSummary = new DgnChangeSummary(*m_targetDb);
        // Put together the changeset
        RevisionChangesFileReader fr (changeset->GetRevisionChangesFile(), *m_targetDb);
        changeSummary->FromChangeSet(fr);
#ifdef DUMP_CHANGE_SUMMARIES
        changeSummary->Dump();
#endif
        ECInstanceKeySet inserted, updated, deleted;
        // Process change summary and get the currently affected elements
        ProcessChangeSummary(changeSummary, inserted, updated, deleted);
        // De-allocate the change summary to clear up any queries
        // Required to close the db
        delete changeSummary;

        // Process changed instances and accumulate the changes
        ProcessChangedElements(inserted, DbOpcode::Insert);
        ProcessChangedElements(updated, DbOpcode::Update);
        ProcessChangedElements(deleted, DbOpcode::Delete);

        // When going backwards, we need to reverse the changeset after we process it
        if (m_backwardsComparison)
            {
            bvector<DgnRevisionPtr> changesets;
            changesets.push_back(changeset);
            if (SUCCESS != RollTargetDb(changesets, m_backwardsComparison))
                return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr    VersionCompareChangeSummary::GetTargetDb()
    {
    return m_targetDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr    VersionCompareChangeSummary::CloneDb(DgnDbR db)
    {
    BeSQLite::DbResult result;
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck));

    BeFileName tempFilename;
    T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFilename, L"VersionCompareTemp");

    WString name = WString(L"Temp_") + db.GetFileName().GetFileNameWithoutExtension();
    tempFilename.AppendToPath(name.c_str());
    tempFilename.AppendExtension(L"bim");

    // Try to create temporary file by copying base bim file
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(db.GetFileName(), tempFilename);

    // If we get any error besides already exists, return error, something went wrong
    if (BeFileNameStatus::Success != fileStatus && BeFileNameStatus::AlreadyExists != fileStatus)
        return nullptr;

    // If the file was copied before, act on it instead of re-copying each time we compare revisions
    // Open the target db using the temporary filename
    DgnDbPtr targetDb = DgnDb::OpenDgnDb(&result, tempFilename, params);
    if (!targetDb.IsValid())
        return nullptr;

    return targetDb;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
bvector<SelectClassInfo> VersionCompareChangeSummary::GetContentClasses(Utf8String schemaName, Utf8String className)
    {
    DgnDbR db = *m_targetDb;

    // Make sure this is the right element class
    ECClassCP cls = db.Schemas().GetClass(schemaName, className);
    BeAssert(nullptr != cls);
    if (nullptr == cls)
        return bvector<SelectClassInfo>();

    // TODO: Pass presentation rules as part of Generate call
    RulesDrivenECPresentationManager::ContentOptions options(Utf8String::IsNullOrEmpty(m_rulesetId.c_str()) ? "Items" : m_rulesetId);
    return m_presentationManager.GetContentClasses(db, ContentDisplayType::PropertyPane, 0, {cls}, options.GetJson()).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr  VersionCompareChangeSummary::GetElement(DgnElementId elementId, DbOpcode opcode)
    {
    if (m_backwardsComparison)
        {
        if (opcode == DbOpcode::Delete)
            return m_targetDb->Elements().GetElement(elementId);

        return m_db.Elements().GetElement(elementId);
        }

    if (opcode == DbOpcode::Insert)
        return m_targetDb->Elements().GetElement(elementId);

    return m_db.Elements().GetElement(elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void   VersionCompareChangeSummary::ProcessChangedElements(ECInstanceKeySet const& changedInstancesKeys, DbOpcode opcode)
    {
    for (ECInstanceKey id : changedInstancesKeys)
        {
        DgnElementCPtr element = GetElement(DgnElementId(id.GetInstanceId().GetValue()), opcode);
        GeometrySource3dCP source = element.IsValid() ? element->ToGeometrySource3d() : nullptr;
        SummaryElementInfo info(opcode, id.GetClassId(), element.IsValid() ? element->GetModelId() : DgnModelId(), nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d());
        if (m_changedElements.find(id) != m_changedElements.end())
            m_changedElements[id].AccumulateChange(info, m_backwardsComparison);
        else
            m_changedElements[id] = info;

        // If the changed element got invalidated during accumulation of changes (e.g. inserted then deleted)
        // get rid of the changed element completely
        if (!m_changedElements[id].IsValid())
            m_changedElements.erase(id);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::RollTargetDb(bvector<DgnRevisionPtr> const& changesets, bool backwardsComparison)
    {
    // Clear statement cache to be able to close db
    if (nullptr != m_statementCache)
        {
        delete m_statementCache;
        m_statementCache = nullptr;
        }

    bvector<DgnRevisionCP> changesetsCP;
    for (DgnRevisionPtr changeset : changesets)
        changesetsCP.push_back(changeset.get());

    // Close db
    BeFileName filename = m_targetDb->GetFileName();
    m_targetDb->CloseDb();
    // Re-open to apply changesets that contain schema changes
    BeSQLite::DbResult result;
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite);
    params.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(changesetsCP, backwardsComparison ? RevisionProcessOption::Reverse : RevisionProcessOption::Merge);
    params.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
    m_targetDb = DgnDb::OpenDgnDb(&result, filename, params);
    BeAssert(result == BeSQLite::BE_SQLITE_OK && m_targetDb.IsValid());

    // Presentation rules need to know about the reopened Db
    if (m_targetDb.IsValid())
        m_presentationManager.GetConnections().CreateConnection(*m_targetDb);

    BeAssert(m_targetDb.IsValid() && !m_targetDb->IsReadonly());

    return (result == BeSQLite::BE_SQLITE_OK && m_targetDb.IsValid()) ? SUCCESS : ERROR;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void    VersionCompareChangeSummary::CacheRelatedPaths(Utf8String schemaName, Utf8String className)
    {
    bvector<SelectClassInfo> contentClasses = GetContentClasses(schemaName, className);

    Utf8String accessor = schemaName + Utf8String(":") + className;
    m_relatedClassCache[accessor] = RelatedPathCache();
    // Cache the related paths class names
    for (SelectClassInfo const& selectInfo : contentClasses)
        {
        bvector<RelatedClassPath> allPaths = selectInfo.GetRelatedPropertyPaths();
        for (RelatedClassPath path : allPaths)
            {
            for (RelatedClass relatedClass : path)
                {
                m_relatedClassCache[accessor].Add(relatedClass.GetSourceClass()->GetFullName(), relatedClass.GetRelationship()->GetFullName(), relatedClass.GetTargetClass().GetClass().GetFullName());
                }
            }
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void    VersionCompareChangeSummary::AddInstancesWithPresentationRulesUpdates(ECInstanceKeySet& updatedInstances, Utf8String schemaName, Utf8String className, DgnChangeSummary* changeSummary, ECInstanceKeySet const& inserted, ECInstanceKeySet const& deleted)
    {
    Utf8String fullName = schemaName + Utf8String(":") + className;
    if (m_relatedClassCache.find(fullName) == m_relatedClassCache.end())
        CacheRelatedPaths(schemaName, className);

    for (RelatedPathCache::PathNames const& path : m_relatedClassCache[fullName].Paths())
        GetInstancesWithAspectUpdates(changeSummary, updatedInstances, path.m_sourceClassName.c_str(), path.m_relationshipClassName.c_str(), path.m_targetClassName.c_str());

    // Filter out modified elements that are contained in both inserted or deleted element lists, as those take precedence
    // e.g. If an element got inserted, and an aspect of that element also got inserted, we mark the insertion of an aspect
    // as an update, but in reality the element got inserted. To handle this, just give more important to insertion/deletions
    // than modifies where we find duplicates
    bvector<ECInstanceKey> filterList;
    for (ECInstanceKey const& elementId : updatedInstances)
        {
        if (inserted.find(elementId) != inserted.end())
            filterList.push_back(elementId);
        if (deleted.find(elementId) != deleted.end())
            filterList.push_back(elementId);
        }

    for (ECInstanceKey const& elementId : filterList)
        updatedInstances.erase(elementId);

    // TODO: Feature gate this functionality
    // Get rid of instances that are not spatial elements
    if (m_filterSpatial)
        {
        ECN::ECClassCP filterClass = m_backwardsComparison ? m_db.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialElement) :
            m_targetDb->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialElement);
        if (filterClass != nullptr)
            {
            for (ECInstanceKey const& key : updatedInstances)
                {
                ECClassId classId = key.GetClassId();
                ECClassCP cls = m_backwardsComparison ? m_db.Schemas().GetClass(classId) : m_targetDb->Schemas().GetClass(classId);
                if (cls != nullptr && !cls->Is(filterClass))
                    updatedInstances.erase(key);
                }
            }
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void    VersionCompareChangeSummary::GetChangedInstances(ECInstanceKeySet& instanceIds, DgnChangeSummary* changeSummary, ECClassId ecclassId, ChangeSummary::QueryDbOpcode opcode)
    {
    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    changeSummary->QueryByClass(changes, ecclassId, true, opcode);

    // Filter out updates of elements that only have last modified data changes and no other property change
    if (m_filterLastMod && opcode == ChangeSummary::QueryDbOpcode::Update)
        {
        Utf8String lastMod = "LastMod";
        bvector<ECInstanceId> toErase;
        for (auto entry : changes)
            {
            ChangeSummary::ValueIterator it = entry.second.MakeValueIterator();
            // If there's only change to LastModified date, then we don't care about this change
            if (it.QueryCount() == 1)
                {
                Utf8String accessString = it.begin().GetAccessString();
                if (lastMod.Equals(accessString))
                    toErase.push_back(entry.first);
                }
            }

        for (auto id : toErase)
            changes.erase(id);
        }

    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        instanceIds.insert(ECInstanceKey(iter->second.GetClassId(), iter->second.GetInstanceId()));
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void InstanceIDsToElementIDs(DgnElementIdSet& elementIds, ECInstanceKeySet const& instanceIds)
    {
    for (ECInstanceKey const& key : instanceIds)
        elementIds.insert(DgnElementId(key.GetInstanceId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    VersionCompareChangeSummary::ProcessChangeSummary(DgnChangeSummary* changeSummary, ECInstanceKeySet& inserted, ECInstanceKeySet& updated, ECInstanceKeySet& deleted)
    {
    ECClassId elClassId = m_targetDb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, m_filterSpatial ? BIS_CLASS_SpatialElement : BIS_CLASS_Element);
    GetChangedInstances(inserted, changeSummary, elClassId, ChangeSummary::QueryDbOpcode::Insert);
    GetChangedInstances(updated, changeSummary, elClassId, ChangeSummary::QueryDbOpcode::Update);
    GetChangedInstances(deleted, changeSummary, elClassId, ChangeSummary::QueryDbOpcode::Delete);

    AddInstancesWithPresentationRulesUpdates(updated, BIS_ECSCHEMA_NAME, BIS_CLASS_Element, changeSummary, inserted, deleted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    VersionCompareChangeSummary::SummaryElementInfo::Invalidate()
    {
    m_opcode = (DbOpcode)0;
    m_ecclassId.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    VersionCompareChangeSummary::SummaryElementInfo::IsValid()
    {
    return (m_opcode != (DbOpcode)0 && m_ecclassId.Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    VersionCompareChangeSummary::SummaryElementInfo::AccumulateChange(VersionCompareChangeSummary::SummaryElementInfo info, bool backwards)
    {
    // The change should be happening to the same ECClass, as it is supposed to be the same instance
    BeAssert(m_ecclassId == info.m_ecclassId);

    if (!backwards)
        {
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
        }
    else
        {
        // 1. Modified then found inserted in an older version, so we have to keep it as insert
        if (info.m_opcode == DbOpcode::Insert && m_opcode == DbOpcode::Update)
            {
            m_opcode = DbOpcode::Insert;
            return;
            }

        // 2. Two updates, keep it as update
        if (info.m_opcode == DbOpcode::Update && m_opcode == DbOpcode::Update)
            return;

        // 3. Element was inserted then deleted, invalidate this change
        if (info.m_opcode == DbOpcode::Insert && m_opcode == DbOpcode::Delete)
            {
            Invalidate();
            return;
            }

        // 4. Update and then deleted, so we accumulate as a deletion
        if (info.m_opcode == DbOpcode::Update && m_opcode == DbOpcode::Delete)
            {
            m_opcode = DbOpcode::Delete;
            return;
            }
        }

    // Should never happen, the combination of opcodes is invalid
    // e.g. Deleted then Inserted, Deleted then Deleted, Deleted then Modified, etc.
    BeAssert(false && "Invalid combination of opcodes. Cannot accumulate.");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetChangedElements(bvector<DgnElementId>& elementIds, bvector<ECClassId>& ecclassIds, bvector<DbOpcode>& opcodes, bvector<DgnModelId>& modelIds, bvector<AxisAlignedBox3d>& bboxes)
    {
    for (bmap<ECInstanceKey,SummaryElementInfo>::iterator it = m_changedElements.begin(); it != m_changedElements.end(); ++it)
        {
        elementIds.push_back(DgnElementId(it->first.GetInstanceId().GetValue()));
        ecclassIds.push_back(it->first.GetClassId());
        opcodes.push_back(it->second.m_opcode);
        modelIds.push_back(it->second.m_modelId);
        bboxes.push_back(it->second.m_bbox);
        }

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/18
//-------------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::GetChangedElementsOfClass(bvector<DgnElementId>& elementIds, bvector<DbOpcode>& opcodes, bvector<DgnModelId>& modelIds, bvector<AxisAlignedBox3d>& bboxes, ECClassCP classp)
    {
    for (bmap<ECInstanceKey,SummaryElementInfo>::iterator it = m_changedElements.begin(); it != m_changedElements.end(); ++it)
        {
        ECClassId classId = it->first.GetClassId();
        ECClassCP cls = m_backwardsComparison ? m_db.Schemas().GetClass(classId) : m_targetDb->Schemas().GetClass(classId);
        BeAssert(cls != nullptr);
        if (cls != nullptr && cls->Is(classp))
            {
            elementIds.push_back(DgnElementId(it->first.GetInstanceId().GetValue()));
            opcodes.push_back(it->second.m_opcode);
            modelIds.push_back(it->second.m_modelId);
            bboxes.push_back(it->second.m_bbox);
            }
        }

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/18
//-------------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::GetChangedModels(bset<DgnModelId>& modelIds, bvector<DbOpcode>& opcodes)
    {
    ECClassCP modelClass = m_db.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Model);
    bvector<DgnElementId> elementIds;
    bvector<DgnModelId> modelIdArray;
    bvector<AxisAlignedBox3d> bboxes;
    if (SUCCESS != GetChangedElementsOfClass(elementIds, opcodes, modelIdArray, bboxes, modelClass))
        return ERROR;

    for (int index = 0; index < modelIdArray.size(); ++index)
        modelIds.insert(modelIdArray.at(index));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetElement(DgnElementCPtr& element, DgnElementId elementId, ECClassId ecclassId, bool targetState)
    {
    if (m_changedElements.empty() || m_changedElements.find(ECInstanceKey(ecclassId, ECInstanceId(elementId.GetValue()))) == m_changedElements.end())
        return ERROR;

    if (targetState)
        {
        element = m_targetDb->Elements().GetElement(elementId);
        return SUCCESS;
        }

    element = m_db.Elements().GetElement(elementId);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetElement(DgnElementCPtr& element, DgnElementId elementId, bool targetState)
    {
    for (bmap<ECInstanceKey,SummaryElementInfo>::iterator iter = m_changedElements.begin(); iter != m_changedElements.end(); ++iter)
        {
        if ((iter->first.GetInstanceId().GetValue() == elementId.GetValue()) && (SUCCESS == GetElement(element, elementId, iter->first.GetClassId(), targetState)))
            return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::SetChangesets(bvector<DgnRevisionPtr>& changesets)
    {
    m_changesets = changesets;

    if (SUCCESS != ProcessChangesets())
        return ERROR;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
CachedECSqlStatementPtr     VersionCompareChangeSummary::GetCachedStatement(DgnDbR db, Utf8String sql)
    {
    if (nullptr == m_statementCache)
        m_statementCache = new ECSqlStatementCache(STATEMENT_CACHE_SIZE);

    return m_statementCache->GetPreparedStatement(db, sql.c_str());
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
StatusInt    VersionCompareChangeSummary::GetTopAssembly(ECInstanceId& topInstanceId, ECClassId& classId, DgnDbPtr db, DgnElementId elementId)
    {
    DgnElementCPtr el = db->Elements().GetElement(elementId);
    if (!el.IsValid())
        return ERROR;

    // Get top assembly
    DgnElementId topElementId = ElementAssemblyUtil::GetAssemblyParentId(*el);
    if (!topElementId.IsValid())
        return ERROR;

    // Get ECClassId of the top assembly
    CachedECSqlStatementPtr stmt = GetCachedStatement(*db, "SELECT el.ECClassId FROM Bis.Element el WHERE el.ECInstanceId=?");
    stmt->BindId(1, topElementId);
    if (stmt->Step() != BeSQLite::BE_SQLITE_ROW)
        return ERROR;

    topInstanceId = ECInstanceId(topElementId.GetValue());
    classId = stmt->GetValueId<ECClassId>(0);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetElementContent(DgnDbPtr db, DgnElementId elementId, ECClassId ecclassId, JsonValueR contentJson)
    {
    // TFS#767958: Provide presentation rules with the top assembly ECInstance ID instead of the element ID to show all desired properties
    ECClassId topClassId = ecclassId;
    ECInstanceId topElementId = ECInstanceId(elementId.GetValue());
    GetTopAssembly(topElementId, topClassId, db, elementId);

    KeySetPtr inputKeys = KeySet::Create({ECClassInstanceKey(db->Schemas().GetClass(topClassId), topElementId)});
    RulesDrivenECPresentationManager::ContentOptions options (Utf8String::IsNullOrEmpty(m_rulesetId.c_str()) ? "Items" : m_rulesetId);
    ContentDescriptorCPtr descriptor = m_presentationManager.GetContentDescriptor(*db, ContentDisplayType::PropertyPane, 0, *inputKeys, nullptr, options.GetJson()).get();
    if (descriptor.IsNull())
        return ERROR;

    PageOptions pageOptions;
    pageOptions.SetPageStart(0);
    pageOptions.SetPageSize(0);

    ContentCPtr content = m_presentationManager.GetContent(*descriptor, pageOptions).get();
    if (content.IsNull())
        {
        BeAssert(false);
        return ERROR;
        }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    content->AsJson().Accept(writer);
    Utf8String exportedJson (buffer.GetString());

    Json::Value parsedValue;
    Json::Reader reader;
    if (!reader.parse(exportedJson.c_str(), parsedValue))
        {
        BeAssert(false); // Shouldn't happen ever
        return ERROR;
        }

    contentJson = parsedValue;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbOpcodeToString(DbOpcode opcode)
    {
    switch (opcode)
        {
        case DbOpcode::Update:
            return "Update";
        case DbOpcode::Insert:
            return "Insert";
        case DbOpcode::Delete:
            return "Delete";
        }

    return "Unknown";
    }

//-------------------------------------------------------------------------------------------
// Flattens out the properties into a JSON object containing all properties
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void    FlattenProperties (JsonValueR output, bool changedOnly, JsonValueCR descriptorField, JsonValueCR currentValues, JsonValueCR targetValues, JsonValueCR currentDisplays, JsonValueCR targetDisplays, Utf8String path = "")
    {
    if (!descriptorField["NestedFields"].isNull())
        {
        Json::Value nestedFields = descriptorField["NestedFields"];
        Utf8String accessor = descriptorField["Name"].asString();
        Utf8String newPath = path + descriptorField["DisplayLabel"].asString() + Utf8String(" -> ");
        for (size_t index = 0; index < nestedFields.size(); ++index)
            {
            FlattenProperties(output, changedOnly, nestedFields[(int)index],
                                      currentValues[accessor][0]["Values"],
                                      targetValues[accessor][0]["Values"],
                                      currentDisplays[accessor][0]["DisplayValues"],
                                      targetDisplays[accessor][0]["DisplayValues"], newPath);
            }

        return;
        }

    Utf8String accessor         = descriptorField["Name"].asString();
    Utf8String displayLabel     = descriptorField["DisplayLabel"].asString();
    Json::Value currentValue    = currentValues[accessor];
    Json::Value currentDisplay  = currentDisplays[accessor];
    Json::Value targetValue     = targetValues[accessor];
    Json::Value targetDisplay   = targetDisplays[accessor];

    Json::Value property = Json::Value(Json::objectValue);
    property["path"] = path;
    property["name"] = accessor;
    property["displayLabel"] = displayLabel;
    property["currentValue"] = currentValue;
    property["currentDisplayValue"] = currentDisplay;
    property["targetValue"] = targetValue;
    property["targetDisplayValue"] = targetDisplay;

    // Do not add property if we are asking for changed only
    if (changedOnly && targetValue == currentValue)
        return;

    Utf8String outputAccessor = path + displayLabel;
    output[outputAccessor] = property;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::GetPropertyComparison(DgnElementId elementId, ECClassId ecclassId, bool changedOnly, JsonValueR content)
    {
    ECInstanceKey key(ecclassId, ECInstanceId(elementId.GetValue()));
    if (m_changedElements.find(key) == m_changedElements.end())
        return ERROR;

    SummaryElementInfo info = m_changedElements[key];

    Json::Value currentContent, targetContent;
    if (SUCCESS != GetElementContent(&m_db, elementId, ecclassId, currentContent) ||
        SUCCESS != GetElementContent(m_targetDb, elementId, ecclassId, targetContent))
        return ERROR;

    // Merge data together, use any of the contents to create the "template"
    Json::Value currentValues = currentContent["ContentSet"][0]["Values"];
    Json::Value targetValues = targetContent["ContentSet"][0]["Values"];
    Json::Value currentDisplays = currentContent["ContentSet"][0]["DisplayValues"];
    Json::Value targetDisplays = targetContent["ContentSet"][0]["DisplayValues"];

    // Use newer in case of schema changes
    Json::Value descriptorFields = m_backwardsComparison ? currentContent["Descriptor"]["Fields"] : targetContent["Descriptor"]["Fields"];

    for (size_t index = 0; index < descriptorFields.size(); ++index)
        FlattenProperties(content, changedOnly, descriptorFields[(int)index], currentValues, targetValues, currentDisplays, targetDisplays);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void populatePropertyNode(JsonValueR node, JsonValueR currentValue, JsonValueR targetValue)
    {
    // Must be data only at this point
    Json::Value mergedValue (Json::objectValue);
    mergedValue["Current"] = currentValue;
    mergedValue["Target"] = targetValue;
    node = mergedValue;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void populateProperties(JsonValueR root, JsonValueR currentValues, JsonValueR targetValues, bool changedOnly, bool backwardsComparison)
    {
    if (!root.isObject())
        {
        if (changedOnly && currentValues == targetValues)
            return;

        populatePropertyNode(root, currentValues, targetValues);
        return;
        }

    bvector<Utf8String> const& members = backwardsComparison ? currentValues.getMemberNames() : targetValues.getMemberNames();
    for (Utf8String const& propertyName : members)
        populateProperties(root[propertyName], currentValues[propertyName], targetValues[propertyName], changedOnly, backwardsComparison);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/18
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetPropertyContentComparisonNested(DgnElementId elementId, ECClassId ecclassId, bool changedOnly, JsonValueR content)
    {
    ECInstanceKey key (ecclassId, ECInstanceId(elementId.GetValue()));
    if (m_changedElements.find(key) == m_changedElements.end())
        return ERROR;

    SummaryElementInfo info = m_changedElements[key];

    Json::Value currentContent, targetContent;
    if (SUCCESS != GetElementContent(&m_db, elementId, ecclassId, currentContent) ||
        SUCCESS != GetElementContent(m_targetDb, elementId, ecclassId, targetContent))
        return ERROR;

    // Create the response using one of the contents as template
    content = currentContent.isNull() ? targetContent : currentContent;
    if (content.isNull())
        return ERROR;

    // Merge data together, use any of the contents to create the "template"
    Json::Value currentValuesRoot = currentContent["ContentSet"][0]["Values"];
    Json::Value targetValuesRoot = targetContent["ContentSet"][0]["Values"];
    Json::Value currentDisplayRoot = currentContent["ContentSet"][0]["DisplayValues"];
    Json::Value targetDisplayRoot = targetContent["ContentSet"][0]["DisplayValues"];

    if (changedOnly)
        {
        // Clear all content
        Json::Value empty(Json::objectValue);
        content["ContentSet"][0]["Values"] = empty;
        content["ContentSet"][0]["DisplayValues"] = empty;
        }

    populateProperties(content["ContentSet"][0]["Values"], currentValuesRoot, targetValuesRoot, changedOnly, m_backwardsComparison);
    populateProperties(content["ContentSet"][0]["DisplayValues"], currentDisplayRoot, targetDisplayRoot, changedOnly, m_backwardsComparison);

    content["ContentSet"][0]["Values"]["__ver_compare___Opcode"] = DbOpcodeToString(info.m_opcode);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetPropertyContentComparison(DgnElementId elementId, ECClassId ecclassId, bool changedOnly, JsonValueR content, bool nestedContent)
    {
    // Handle nested content
    if (nestedContent)
        return GetPropertyContentComparisonNested(elementId, ecclassId, changedOnly, content);

    ECInstanceKey key (ecclassId, ECInstanceId(elementId.GetValue()));
    if (m_changedElements.find(key) == m_changedElements.end())
        return ERROR;

    SummaryElementInfo info = m_changedElements[key];

    Json::Value currentContent, targetContent;
    if (SUCCESS != GetElementContent(&m_db, elementId, ecclassId, currentContent) ||
        SUCCESS != GetElementContent(m_targetDb, elementId, ecclassId, targetContent))
        return ERROR;

    // Create the response using one of the contents as template
    content = currentContent.isNull() ? targetContent : currentContent;
    if (content.isNull())
        return ERROR;

    // Merge data together, use any of the contents to create the "template"
    Json::Value currentValues = currentContent["ContentSet"][0]["Values"];
    Json::Value targetValues = targetContent["ContentSet"][0]["Values"];
    Json::Value currentDisplay = currentContent["ContentSet"][0]["DisplayValues"];
    Json::Value targetDisplay = targetContent["ContentSet"][0]["DisplayValues"];
    Json::Value mergedValues (Json::arrayValue);

    if (changedOnly)
        {
        // Clear all content
        Json::Value empty(Json::objectValue);
        content["ContentSet"][0]["Values"] = empty;
        content["ContentSet"][0]["DisplayValues"] = empty;
        }

    bvector<Utf8String> const& members = m_backwardsComparison ? currentValues.getMemberNames() : targetValues.getMemberNames();
    for (Utf8String const& propertyName : members)
        {
        Json::Value mergedValue (Json::objectValue);
        Json::Value mergedDisplay (Json::objectValue);
        mergedValue["Current"] = currentValues[propertyName];
        mergedValue["Target"] = targetValues[propertyName];
        mergedDisplay["Current"] = currentDisplay[propertyName];
        mergedDisplay["Target"] = targetDisplay[propertyName];

        // If we are asking only for changed properties, only add them to the content if it has changed
        if (changedOnly && (mergedValue["Current"] == mergedValue["Target"] || mergedDisplay["Current"] == mergedDisplay["Target"]))
            continue;

        content["ContentSet"][0]["Values"][propertyName] = mergedValue;
        content["ContentSet"][0]["DisplayValues"][propertyName] = mergedDisplay;
        }

    content["ContentSet"][0]["Values"]["__ver_compare___Opcode"] = DbOpcodeToString(info.m_opcode);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
VersionCompareChangeSummaryPtr  VersionCompareChangeSummary::Generate(DgnDbR db, bvector<DgnRevisionPtr>& changesets, IECPresentationManagerR presentationManager, Utf8String rulesetId, bool backwardsComparison)
    {
    // Create the change summary
    VersionCompareChangeSummaryPtr changeSummary = new VersionCompareChangeSummary(db, presentationManager, backwardsComparison);
    changeSummary->m_rulesetId = rulesetId;
    // Set changesets and store all changed elements
    if (SUCCESS != changeSummary->SetChangesets(changesets))
        return nullptr;

    return changeSummary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
VersionCompareChangeSummaryPtr  VersionCompareChangeSummary::Generate(DgnDbR db, bvector<DgnRevisionPtr>& changesets, IECPresentationManagerR presentationManager, Utf8String rulesetId, bool backwardsComparison, bool filterSpatial, bool filterLastMod)
    {
    // Create the change summary
    VersionCompareChangeSummaryPtr changeSummary = new VersionCompareChangeSummary(db, presentationManager, backwardsComparison);
    changeSummary->m_filterSpatial = filterSpatial;
    changeSummary->m_filterLastMod = filterLastMod;
    changeSummary->m_rulesetId = rulesetId;

    // Set changesets and store all changed elements
    if (SUCCESS != changeSummary->SetChangesets(changesets))
        return nullptr;

    return changeSummary;
    }
