/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VersionCompareChangeSummary.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnElement.h>
#include <Bentley/ByteStream.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/ECPresentationManager.h>
#include <Bentley/BeConsole.h>
#include <DgnPlatform/DgnDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY

#define VCLOG (NativeLogging::CategoryLogger("DgnCore.VersionCompare"))

// #define PROFILE_VC_PROCESSING
#ifdef PROFILE_VC_PROCESSING

Utf8String prof_operation;
bmap<Utf8String, clock_t> prof_starts;
bmap<Utf8String, clock_t> prof_ends;

void Profile_StartClock(Utf8String operation)
    {
    prof_operation = operation;
    prof_starts[operation] = clock();
    }

void Profile_EndClock(Utf8String operation)
    {
    prof_ends[operation] = clock();
    }

void Profile_EndAndReport(Utf8String operation)
    {
    Profile_EndClock(operation);
    if (prof_starts.find(operation) == prof_starts.end() || prof_ends.find(operation) == prof_ends.end())
        {
        printf("Profile: Invalid operation for profiling");
        return;
        }

    clock_t t0 = prof_starts[operation];
    clock_t t1 = prof_ends[operation];
    double elapsed = (t1-t0)/(double)CLOCKS_PER_SEC;
    printf("Profile - %s: %lf seconds\n", operation.c_str(), elapsed);
    }

#endif

#define CATEGORY_AS "Category.Id"
#define INSPATIALINDEX_AS "InSpatialIndex"
#define ORIGIN_X_AS "Origin.X"
#define ORIGIN_Y_AS "Origin.Y"
#define ORIGIN_Z_AS "Origin.Z"
#define YAW_AS "Yaw"
#define PITCH_AS "Pitch"
#define ROLL_AS "Roll"
#define BBOXLOW_X_AS "BBoxLow.X"
#define BBOXLOW_Y_AS "BBoxLow.Y"
#define BBOXLOW_Z_AS "BBoxLow.Z"
#define BBOXHIGH_X_AS "BBoxHigh.X"
#define BBOXHIGH_Y_AS "BBoxHigh.Y"
#define BBOXHIGH_Z_AS "BBoxHigh.Z"
#define GEOMETRYSTREAM_AS "GeometryStream"
#define LASTMOD_AS "LastMod"
#define ASPECT_ELEMID "Element.Id"
#define ASPECT_RELCLASSID "Element.RelECClassId"
#define MODEL_ID "Model.Id"
#define PARENT_ID "Parent.Id"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HiddenPropertyCache::Clear()
    {
    m_cache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HiddenPropertyCache::CleanUpStatement()
    {
    m_hiddenPropertiesStmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool HiddenPropertyCache::Has(BentleyApi::ECN::ECClassId classId)
    {
    return m_cache.find(classId) != m_cache.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool HiddenPropertyCache::IsHiddenProperty(DgnDbR db, ECClassId classId, Utf8CP property)
    {
    if (!Has(classId))
        QueryHiddenProperties(db, classId);

    return m_cache[classId].find(property) != m_cache[classId].end();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void HiddenPropertyCache::QueryHiddenProperties(DgnDbR db, ECClassId classId)
    {
    if (!m_hiddenPropertiesStmt.IsPrepared())
        {
        // Prepare the statement
        Utf8CP query = "SELECT DISTINCT ec_Property.[Name] FROM ec_Property "
           "INNER JOIN ec_CustomAttribute ON ec_CustomAttribute.[ContainerId] = ec_Property.[Id] "
           "INNER JOIN ec_Class ON ec_Class.[Name] = \"HiddenProperty\" AND ec_Class.[Id] = ec_CustomAttribute.[ClassId] "
           "WHERE ec_Property.[ClassId] IN "
           "( "
           "SELECT ec_cache_ClassHierarchy.[BaseClassId] "
                  "FROM ec_cache_ClassHierarchy "
                  "WHERE ec_cache_ClassHierarchy.[ClassId] = ? "
           ")";
        m_hiddenPropertiesStmt.Prepare(db, query);
        }

    // Check if results are in the cache already
    if (m_cache.find(classId) == m_cache.end())
        {
        // Bind class id
        m_hiddenPropertiesStmt.BindId(1, classId);
        bset<Utf8String> results;
        while (m_hiddenPropertiesStmt.Step() == BE_SQLITE_ROW)
            {
            // Add results to set
            results.insert(Utf8String(m_hiddenPropertiesStmt.GetValueText(0)));
            // Add also "Prop.Id" for cases like CodeSpec, CodeScope, etc
            results.insert(Utf8PrintfString("%s.Id", m_hiddenPropertiesStmt.GetValueText(0)));
            }
        // Cache results
        m_cache.Insert(classId, results);
        // Reset query
        m_hiddenPropertiesStmt.Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    ChangedElementRecord::Invalidate()
    {
    m_opcode = (DbOpcode)0;
    m_ecclassId.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ChangedElementRecord::IsValid()
    {
    return (m_opcode != (DbOpcode)0 && m_ecclassId.Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    ChangedElementRecord::AccumulateChange(ChangedElementRecord info)
    {
    // The change should be happening to the same ECClass, as it is supposed to be the same instance
    BeAssert(m_ecclassId == info.m_ecclassId);

    // Merge changes type
    m_changes.Merge(info.m_changes);

    if (m_parentKey.GetInstanceId() != info.m_parentKey.GetInstanceId())
        {
        m_parentKey = info.m_parentKey;
        m_changes.AddType(ElementChangesType::Type::Mask_Parent);
        }

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

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
Utf8String RelatedPropertyPathCache::RelatedPaths::MakeKey()
    {
    Utf8String key;
    for (PathNames const& path : m_data)
        key += path.MakeKey();
    return key;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
bvector<SelectClassInfo> RelatedPropertyPathCache::GetContentClasses(DgnDbR db, Utf8StringCR schemaName, Utf8StringCR className)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("RelatedPropertyPathCache::GetContentClasses");
#endif
    // Make sure this is the right element class
    ECClassCP cls = db.Schemas().GetClass(schemaName, className);
    BeAssert(nullptr != cls);
    if (nullptr == cls)
        return bvector<SelectClassInfo>();

    try
        {
        auto params = AsyncContentClassesRequestParams::Create(db, m_rulesetId.empty() ? "Items" : m_rulesetId, RulesetVariables(),
            ContentDisplayType::PropertyPane, 0, bvector<ECClassCP>{ cls });
        auto response = m_presentationManager.GetContentClasses(params).get();
#ifdef PROFILE_VC_PROCESSING
        Profile_EndAndReport("RelatedPropertyPathCache::GetContentClasses");
#endif
        return *response;
        }
    catch (...)
        {
        return bvector<SelectClassInfo>();
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
bvector<RelatedPropertyPathCache::RelatedPaths> const& RelatedPropertyPathCache::Get() const
    {
    return m_cache;
    }
//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
bool RelatedPropertyPathCache::RelatedPaths::IsPropertyProvider(BentleyApi::ECN::ECClassId const &classId) const
    {
    // Property paths from GetContentClasses will now be the generic BIS paths
    // So we must check if any of those paths match any of the target derived classes
    return m_targetClasses.find(classId) != m_targetClasses.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ForEachDerivedClass(SchemaManagerCR schemas, ECClassCR base, std::function<void(ECClassCR)> const& derivedClassCallback)
    {
    for (ECClassCP derived : schemas.GetDerivedClasses(base))
        {
        derivedClassCallback(*derived);
        ForEachDerivedClass(schemas, *derived, derivedClassCallback);
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelatedPropertyPathCache::RelatedPaths::SetTargetClass(DgnDbR db, BentleyApi::ECN::ECClassCP targetClassP)
    {
    if (targetClassP == nullptr)
        return;

    // Set target class Id
    m_targetClassId = targetClassP->GetId();
    // Property paths are now based on the generic BIS schema classes
    // Find all derived classes that elements could be for this property path to be relevant
    m_targetClasses.clear();
    m_targetClasses.insert(m_targetClassId);
    ForEachDerivedClass(db.Schemas(), *targetClassP, [this](ECClassCR derivedClass)
        {
            this->m_targetClasses.insert(derivedClass.GetId());
        }
    );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertyPathExplorer::RelatedPropertyPathExplorer(DgnDbR db, DgnChangeSummary& changeSummary) : m_helper(db, changeSummary), m_chunkHelper(db, changeSummary)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertyPathExplorer::FindCacheableClasses(DgnDbR db, DgnChangeSummary& changeSummary, bmap<Utf8String, RelatedPropertyPathCache::RelatedPaths> const& paths)
    {
    m_helper.FindCacheableClasses(db, changeSummary, paths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipQueryHelper::RelationshipQueryHelper(DgnDbR db, DgnChangeSummary& changeSummary): m_statementCache(STATEMENT_CACHE_SIZE)
    {
    // Query for finding the changed relationships targets and sources from the temp tables created by the change summary
    Utf8PrintfString sql("SELECT it.InstanceId, vt.AccessString, vt.OldValue "
        "FROM ec_cache_ClassHierarchy ch "
        "JOIN %s it ON ch.ClassId = it.ClassId "
        "JOIN %s vt ON vt.ClassId = it.ClassId AND vt.InstanceId = it.InstanceId "
        "WHERE ch.[BaseClassId] = ? AND it.DbOpcode = %d AND vt.AccessString in (\"TargetECInstanceId\", \"TargetECClassId\", \"SourceECInstanceId\", \"SourceECClassId\")",
        changeSummary.GetInstancesTableName().c_str(),
        changeSummary.GetValuesTableName().c_str(),
        DbOpcode::Delete);

    DbResult result = m_deletedRelationshipStmt.Prepare(db, sql.c_str());
    if (result != DbResult::BE_SQLITE_OK)
        {
        BeAssert(result == DbResult::BE_SQLITE_OK);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipQueryHelper::~RelationshipQueryHelper()
    {
    m_deletedRelationshipStmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr RelationshipQueryHelper::GetCachedStatement(DgnDbR db, Utf8StringCR ecsql)
    {
    return m_statementCache.GetPreparedStatement(db, ecsql.c_str());
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelationshipQueryHelper::FindSourcesOfPathInDb(bset<ECInstanceKey>& sources, DgnDbR db, PathNames const& path, ECInstanceKey const& instanceKey)
    {
    if (m_cacheableClassIds.find(path.m_relationshipClassId) == m_cacheableClassIds.end())
        {
        // Query Db directly
        GetCurrentRelatedInstances(sources, db, path, instanceKey);
        return;
        }

    // Query all relationships of the class that exist in the db to be able to find the related instances
    RelationshipCacheEntry& entry = GetCurrentRelationships(db, path);
    if (path.m_forward)
        {
        // Map lookups are faster than iterating over the db every time for each potential path node for each element
        auto srcs = entry.m_targetToSourcesMap.find(instanceKey);
        if (srcs == entry.m_targetToSourcesMap.end())
            return;

        for (auto const& src : srcs->second)
            sources.insert(src);
        }
    else
        {
        // Map lookups are faster than iterating over the db every time for each potential path node for each element
        auto srcs = entry.m_sourceToTargetsMap.find(instanceKey);
        if (srcs == entry.m_sourceToTargetsMap.end())
            return;

        for (auto const& src : srcs->second)
            sources.insert(src);
        }
    }

struct RelationshipQueryResult
    {
    ECInstanceId sourceId;
    ECClassId sourceClassId;
    ECInstanceId targetId;
    ECClassId targetClassId;
    };

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
RelationshipCacheEntry& RelationshipQueryHelper::GetDeletedRelationshipChanges(DgnDbR db, DgnChangeSummary& changeSummary, ECClassId const& relClassId)
    {
    if (m_deletedRelationshipInstancesCache.find(relClassId) == m_deletedRelationshipInstancesCache.end())
        {
        VCLOG.infov("RelatedPropertyPathExplorer: caching deleted relationship changes for class %s", relClassId.ToHexStr().c_str());
        m_deletedRelationshipStmt.Reset();
        m_deletedRelationshipStmt.BindId(1, relClassId);

        bmap<ECInstanceId, RelationshipQueryResult> relationshipToValues;
        while (m_deletedRelationshipStmt.Step() != DbResult::BE_SQLITE_DONE)
            {
            ECInstanceId relationshipId = m_deletedRelationshipStmt.GetValueId<ECInstanceId>(0);
            Utf8CP accessStr = m_deletedRelationshipStmt.GetValueText(1);
            RelationshipQueryResult& result = relationshipToValues[relationshipId];
            if (strcmp(accessStr, "TargetECInstanceId") == 0)
                result.targetId = m_deletedRelationshipStmt.GetValueId<ECInstanceId>(2);
            else if (strcmp(accessStr, "TargetECClassId") == 0)
                result.targetClassId = m_deletedRelationshipStmt.GetValueId<ECClassId>(2);
            else if (strcmp(accessStr, "SourceECInstanceId") == 0)
                result.sourceId = m_deletedRelationshipStmt.GetValueId<ECInstanceId>(2);
            else if (strcmp(accessStr, "SourceECClassId") == 0)
                result.sourceClassId = m_deletedRelationshipStmt.GetValueId<ECClassId>(2);
            }

        // Create entry and populate it with change summary values before the relationship got deleted
        RelationshipCacheEntry& entry = m_deletedRelationshipInstancesCache[relClassId];
        int count = 0;
        for (auto const& iter : relationshipToValues)
            {
            ECInstanceKey sourceKey(iter.second.sourceClassId, iter.second.sourceId);
            ECInstanceKey targetKey(iter.second.targetClassId, iter.second.targetId);
            entry.m_sourceToTargetsMap[sourceKey].insert(targetKey);
            entry.m_targetToSourcesMap[targetKey].insert(sourceKey);
            count++;
            }
        VCLOG.infov("RelatedPropertyPathExplorer: found %d deleted changed relationships for class: %s", count, relClassId.ToHexStr().c_str());
        }

    return m_deletedRelationshipInstancesCache[relClassId];
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
int RelationshipQueryHelper::GetCountOfCurrentRelationships(DgnDbR db, Utf8String const& relationshipClassName)
    {
    Utf8PrintfString ecsql("SELECT COUNT(*) FROM %s", relationshipClassName.c_str());
    CachedECSqlStatementPtr stmt = GetCachedStatement(db, ecsql);
    if (DbResult::BE_SQLITE_ROW != stmt->Step())
        return 0;

    return stmt->GetValueInt(0);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelationshipQueryHelper::FindCacheableClasses(DgnDbR db, DgnChangeSummary& changeSummary, bmap<Utf8String, RelatedPropertyPathCache::RelatedPaths> const& paths)
    {
    // If we don't want caching, return without adding any cacheable classes to our set
    if (!m_options.m_wantRelationshipCaching)
        return;

    // Only allow caching of relationships that don't count more than the maximum set
    for (auto const& path : paths)
        {
        for (auto const& innerPath : path.second.Paths())
            {
            // Get count of relationships existing in the Db of the given edge of a property path
            int relCount = GetCountOfCurrentRelationships(db, innerPath.m_relationshipClassName);
            // We want to cache a class for faster processing if and only if
            // the number of existing relationships of this edge of a property path
            // is not more than the cache size in the options passed to the processing function
            // This is done to avoid customizing memory usage vs performance in cases where
            // the changeset contains pure additions of millions of elements and aspects.
            // When not caching, we will just query the database as needed for each of those
            // elements, which may not be as fast, but will save us memory and avoid agents
            // to run out of memory in their pod
            if (relCount < m_options.m_cacheSize)
                {
                m_cacheableClassIds.insert(innerPath.m_relationshipClassId);
                }
            }
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
RelationshipCacheEntry& RelationshipQueryHelper::GetCurrentRelationships(DgnDbR db, PathNames const& path)
    {
    ECClassId relClassId = path.m_relationshipClassId;

    if (m_currentRelationshipInstancesCache.find(relClassId) == m_currentRelationshipInstancesCache.end())
        {
        VCLOG.infov("RelatedPropertyPathExplorer: caching existing relationships for class %s", relClassId.ToHexStr().c_str());
        // Query all sources and targets for the related property paths and cache so that we only query a single relationship once
        Utf8PrintfString ecsql("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM %s", path.m_relationshipClassName.c_str());
        CachedECSqlStatementPtr stmt = GetCachedStatement(db, ecsql);

        RelationshipCacheEntry& entry = m_currentRelationshipInstancesCache[relClassId];
        int count = 0;
        while(stmt->Step() != DbResult::BE_SQLITE_DONE)
            {
            ECInstanceId sourceId = stmt->GetValueId<ECInstanceId>(0);
            ECClassId sourceClassId = stmt->GetValueId<ECClassId>(1);
            ECInstanceId targetId = stmt->GetValueId<ECInstanceId>(2);
            ECClassId targetClassId = stmt->GetValueId<ECClassId>(3);
            ECInstanceKey sourceKey(sourceClassId, sourceId);
            ECInstanceKey targetKey(targetClassId, targetId);
            entry.m_sourceToTargetsMap[sourceKey].insert(targetKey);
            entry.m_targetToSourcesMap[targetKey].insert(sourceKey);
            count++;
            }
        VCLOG.infov("RelatedPropertyPathExplorer: found %d existing relationships for class %s", count, relClassId.ToHexStr().c_str());
        }

    return m_currentRelationshipInstancesCache[relClassId];
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelationshipQueryHelper::GetCurrentRelatedInstances(bset<ECInstanceKey>& outInstances, DgnDbR db, PathNames const& path, ECInstanceKey const& instanceKey)
    {
    // Get related instances without caching
    Utf8String whereSourceId = "TargetECInstanceId";
    Utf8String whereSourceClass = "TargetECClassId";
    if (!path.m_forward)
        {
        whereSourceId = "SourceECInstanceId";
        whereSourceClass = "SourceECClassId";
        }

    Utf8PrintfString ecsql("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM %s WHERE %s = ? AND %s = ?", path.m_relationshipClassName.c_str(), whereSourceId.c_str(), whereSourceClass.c_str());
    CachedECSqlStatementPtr stmt = GetCachedStatement(db, ecsql);
    stmt->BindId(1, instanceKey.GetInstanceId());
    stmt->BindId(2, instanceKey.GetClassId());
    while (DbResult::BE_SQLITE_ROW == stmt->Step())
        {
        if (path.m_forward)
            {
            ECInstanceId id = stmt->GetValueId<ECInstanceId>(0);
            ECClassId classId = stmt->GetValueId<ECClassId>(1);
            outInstances.insert(ECInstanceKey(classId, id));
            }
        else
            {
            ECInstanceId id = stmt->GetValueId<ECInstanceId>(2);
            ECClassId classId = stmt->GetValueId<ECClassId>(3);
            outInstances.insert(ECInstanceKey(classId, id));
            }
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelationshipQueryHelper::FindSourcesOfPathInSummary(bset<ECInstanceKey>& sources, DgnDbR db, DgnChangeSummary& changeSummary, PathNames const& path, ECInstanceKey const& instanceKey)
    {
    // Query all relationships of the class that got deleted to be able to find the related instances
    RelationshipCacheEntry& entry = GetDeletedRelationshipChanges(db, changeSummary, path.m_relationshipClassId);
    if (path.m_forward)
        {
        // Map lookups are faster than iterating over the change summary temp tables every time for each potential path node
        auto srcs = entry.m_targetToSourcesMap.find(instanceKey);
        if (srcs == entry.m_targetToSourcesMap.end())
            return;

        for (auto const& src : srcs->second)
            sources.insert(src);
        }
    else
        {
        // Map lookups are faster than iterating over the change summary temp tables every time for each potential path node
        auto srcs = entry.m_sourceToTargetsMap.find(instanceKey);
        if (srcs == entry.m_sourceToTargetsMap.end())
            return;

        for (auto const& src : srcs->second)
            sources.insert(src);
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelatedPropertyPathExplorer::Traverse(bset<ECInstanceKey>& output, DgnDbR db, DgnChangeSummary& changeSummary, bvector<PathNames> const& paths, ECInstanceKey const& instance)
    {
    size_t size = paths.size();
    if (size == 0)
        return;

    // Process the remaining related property path
    bvector<PathNames> newPaths;
    for (int i = 0; i < size - 1; ++i)
        newPaths.push_back(paths[i]);

    bset<ECInstanceKey> sources;
    m_helper.FindSourcesOfPathInDb(sources, db, paths[size - 1], instance);
    m_helper.FindSourcesOfPathInSummary(sources, db, changeSummary, paths[size - 1], instance);

    if (size == 1)
        {
        // Reached top of property path, return the related instance keys
        for (auto const& key : sources)
            output.insert(key);
        }
    else
        {
        // Intermediary node in property path, keep traversing
        for (auto const& key : sources)
            Traverse(output, db, changeSummary, newPaths, key);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChunkRelationshipQueryHelper::ChunkRelationshipQueryHelper(DgnDbR db, DgnChangeSummary& changeSummary) : m_statementCache(STATEMENT_CACHE_SIZE)
    {
    // Query for finding the changed relationships targets and sources from the temp tables created by the change summary
    Utf8PrintfString sql("SELECT it.InstanceId, vt.AccessString, vt.OldValue "
        "FROM ec_cache_ClassHierarchy ch "
        "JOIN %s it ON ch.ClassId = it.ClassId "
        "JOIN %s vt ON vt.ClassId = it.ClassId AND vt.InstanceId = it.InstanceId "
        "WHERE ch.[BaseClassId] = ? AND it.DbOpcode = %d AND vt.AccessString in (\"TargetECInstanceId\", \"TargetECClassId\", \"SourceECInstanceId\", \"SourceECClassId\")",
        changeSummary.GetInstancesTableName().c_str(),
        changeSummary.GetValuesTableName().c_str(),
        DbOpcode::Delete);

    DbResult result = m_deletedRelationshipStmt.Prepare(db, sql.c_str());
    if (result != DbResult::BE_SQLITE_OK)
        {
        BeAssert(result == DbResult::BE_SQLITE_OK);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChunkRelationshipQueryHelper::~ChunkRelationshipQueryHelper()
    {
    m_deletedRelationshipStmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ChunkRelationshipQueryHelper::GetCachedStatement(DgnDbR db, Utf8StringCR ecsql)
    {
    return m_statementCache.GetPreparedStatement(db, ecsql.c_str());
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
RelationshipCacheEntry ChunkRelationshipQueryHelper::GetDeletedRelationshipChanges(DgnDbR db, DgnChangeSummary& changeSummary, ECClassId const& relClassId, bset<ECInstanceKey> const& instanceKeys)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("RelationshipQueryHelper:GetDeletedRelationshipChanges");
#endif
        VCLOG.infov("RelatedPropertyPathExplorer: caching deleted relationship changes for class %s", relClassId.ToHexStr().c_str());
        m_deletedRelationshipStmt.Reset();
        m_deletedRelationshipStmt.BindId(1, relClassId);

        bmap<ECInstanceId, RelationshipQueryResult> relationshipToValues;
        while (m_deletedRelationshipStmt.Step() == DbResult::BE_SQLITE_ROW)
            {
            ECInstanceId relationshipId = m_deletedRelationshipStmt.GetValueId<ECInstanceId>(0);
            Utf8CP accessStr = m_deletedRelationshipStmt.GetValueText(1);
            RelationshipQueryResult& result = relationshipToValues[relationshipId];
            if (strcmp(accessStr, "TargetECInstanceId") == 0)
                result.targetId = m_deletedRelationshipStmt.GetValueId<ECInstanceId>(2);
            else if (strcmp(accessStr, "TargetECClassId") == 0)
                result.targetClassId = m_deletedRelationshipStmt.GetValueId<ECClassId>(2);
            else if (strcmp(accessStr, "SourceECInstanceId") == 0)
                result.sourceId = m_deletedRelationshipStmt.GetValueId<ECInstanceId>(2);
            else if (strcmp(accessStr, "SourceECClassId") == 0)
                result.sourceClassId = m_deletedRelationshipStmt.GetValueId<ECClassId>(2);
            }

        // Create entry and populate it with change summary values before the relationship got deleted
        RelationshipCacheEntry entry;
        int count = 0;
        for (auto const& iter : relationshipToValues)
            {
            ECInstanceKey sourceKey(iter.second.sourceClassId, iter.second.sourceId);
            ECInstanceKey targetKey(iter.second.targetClassId, iter.second.targetId);
            if (instanceKeys.find(targetKey) == instanceKeys.end() && instanceKeys.find(sourceKey) == instanceKeys.end())
                continue;

            entry.m_sourceToTargetsMap[sourceKey].insert(targetKey);
            entry.m_targetToSourcesMap[targetKey].insert(sourceKey);
            count++;
            }
        VCLOG.infov("RelatedPropertyPathExplorer: found %d deleted changed relationships for class: %s", count, relClassId.ToHexStr().c_str());
#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("RelationshipQueryHelper:GetDeletedRelationshipChanges");
#endif

    return entry;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
RelationshipCacheEntry ChunkRelationshipQueryHelper::GetCurrentRelatedInstances(DgnDbR db, PathNames const& path, bset<ECInstanceKey> const& instanceKeys)
    {
    Utf8String whereSourceId = "TargetECInstanceId";
    if (!path.m_forward)
        whereSourceId = "SourceECInstanceId";

    VCLOG.infov("ChunkRelationshipQueryHelper: Querying for relationship class: %s", path.m_relationshipClassId.ToHexStr().c_str());

    // Query all sources and targets for the related property paths that relate to the instance id
    IdSet<BeInt64Id> ids;
    for (auto const& instanceKey : instanceKeys)
        ids.insert(instanceKey.GetInstanceId());

    Utf8PrintfString ecsql("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM %s WHERE InVirtualSet(?, %s)", path.m_relationshipClassName.c_str(), whereSourceId.c_str());
    CachedECSqlStatementPtr stmt = GetCachedStatement(db, ecsql);
    stmt->BindIdSet(1, ids);

    RelationshipCacheEntry entry;
    while (stmt->Step() == DbResult::BE_SQLITE_ROW)
    {
        ECInstanceId sourceId = stmt->GetValueId<ECInstanceId>(0);
        ECClassId sourceClassId = stmt->GetValueId<ECClassId>(1);
        ECInstanceId targetId = stmt->GetValueId<ECInstanceId>(2);
        ECClassId targetClassId = stmt->GetValueId<ECClassId>(3);
        ECInstanceKey sourceKey(sourceClassId, sourceId);
        ECInstanceKey targetKey(targetClassId, targetId);
        entry.m_sourceToTargetsMap[sourceKey].insert(targetKey);
        entry.m_targetToSourcesMap[targetKey].insert(sourceKey);
    }

    // VCLOG.infov("ChunkRelationshipQueryHelper: found %d existing relationships", count);

    return entry;
    }


//-------------------------------------------------------------------------------------------
// Consolidate the related keysets of an input keyset and put all of the related keys
// in the output set
// @bsimethod
//-------------------------------------------------------------------------------------------
void ConsolidateRelatedKeySets(ECInstanceKeySet& output, ECInstanceKeySet const& relatedInputKeys, bmap<ECInstanceKey,ECInstanceKeySet> const& relatedKeySets)
    {
    for (auto const& key : relatedInputKeys)
        {
        auto relatedSet = relatedKeySets.find(key);
        if (relatedSet != relatedKeySets.end())
            {
            for (auto const& relatedKey : relatedSet->second)
                output.insert(relatedKey);
            }
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void RelatedPropertyPathExplorer::TraverseInChunks(bmap<ECInstanceKey,ECInstanceKeySet>& output, DgnDbR db, DgnChangeSummary& changeSummary, bvector<PathNames> const& paths, bset<ECInstanceKey> const& instanceKeys)
    {
    // Vector of each path's results (Instance Key -> Related Instance Keys via Relationship)
    bvector<bmap<ECInstanceKey,ECInstanceKeySet>> instanceResults;
    // Temporary set of keys to use for each path traversal
    bset<ECInstanceKey> nextKeys = instanceKeys;
    // Iterate the paths from the ending to the beginning and find related instances on each path backwards towards the element
    for (int i = paths.size() - 1; i >= 0; --i)
        {
        auto const& path = paths[i];

        // Find current existing path instances in Db
        RelationshipCacheEntry currentPathInstances = m_chunkHelper.GetCurrentRelatedInstances(db, path, nextKeys);
        // Find deleted path instances based on change summary
        RelationshipCacheEntry deletedPathInstances = m_chunkHelper.GetDeletedRelationshipChanges(db, changeSummary, path.m_relationshipClassId, nextKeys);

        // Create the intermediary map of key to related instance of the current path
        bmap<ECInstanceKey, ECInstanceKeySet> currentInstanceResults;
        // Take into account direction of relationship
        if (!path.m_forward)
            {
            for (auto const& pair : currentPathInstances.m_sourceToTargetsMap)
                currentInstanceResults.insert(pair);
            for (auto const& pair : deletedPathInstances.m_sourceToTargetsMap)
                currentInstanceResults.insert(pair);
            }
        else
            {
            for (auto const& pair : currentPathInstances.m_targetToSourcesMap)
                currentInstanceResults.insert(pair);
            for (auto const& pair : deletedPathInstances.m_targetToSourcesMap)
                currentInstanceResults.insert(pair);
            }
        instanceResults.push_back(currentInstanceResults);

        // Only merge the next output keys to process until the last iteration
        if (i > 0)
            {
            // Get the next keys to process
            nextKeys.clear();
            for (auto const& pair : currentInstanceResults)
                for (auto const& key : pair.second)
                    nextKeys.insert(key);
            }
        }

    // No need to keep this around
    nextKeys.clear();

    // Put together results of the initial instance keys to the set of keys they are related to at the end of the path
    for (auto const& originKey : instanceKeys)
        {
        ECInstanceKeySet currentIntermediaryKeys;
        currentIntermediaryKeys.insert(originKey);

        // Go through paths and find all the leaf keys
        for (auto const& relatedInstanceKeys : instanceResults)
            {
            ECInstanceKeySet nextIntermediaryKeys;
            ConsolidateRelatedKeySets(nextIntermediaryKeys, currentIntermediaryKeys, relatedInstanceKeys);
            currentIntermediaryKeys = nextIntermediaryKeys;
            }

        // Merge the found related keys to this instance
		ECInstanceKeySet& keySet = output[originKey];
        for (auto const& key : currentIntermediaryKeys)
            keySet.insert(key);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr RelatedPropertyPathCache::GetCachedStatement(DgnDbR db, Utf8StringCR ecsql)
    {
    return m_statementCache.GetPreparedStatement(db, ecsql.c_str());
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void    RelatedPropertyPathCache::CachePaths(DgnDbR db, Utf8StringCR schemaName, Utf8StringCR className)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("RelatedPropertyPathCache::CachePaths");
#endif
    // Get content classes from presentation rules
    bvector<SelectClassInfo> contentClasses = GetContentClasses(db, schemaName, className);

    // Cache the related paths class names
    for (SelectClassInfo const& selectInfo : contentClasses)
        {
        for (RelatedClassPathCR path : selectInfo.GetRelatedPropertyPaths())
            {
            RelatedPaths currentPaths;
            for (RelatedClassCR relatedClass : path)
                {
                currentPaths.Add(
                    relatedClass.GetSourceClass()->GetFullName(),
                    relatedClass.GetRelationship().GetClass().GetFullName(),
                    relatedClass.GetTargetClass().GetClass().GetFullName(),
                    relatedClass.GetRelationship().GetClass().GetId(),
                    relatedClass.IsForwardRelationship());
                }

            // Set the target ECClassId of the path for faster check of related instances later on
            RelatedClassCR finalPath = path[path.size() - 1];
            if (finalPath.IsForwardRelationship())
                currentPaths.SetTargetClass(db, &finalPath.GetTargetClass().GetClass());
            else
                currentPaths.SetTargetClass(db, finalPath.GetSourceClass());

            m_cache.push_back(currentPaths);
            }
        }
#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("RelatedPropertyPathCache::CachePaths");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceFinder::RelatedInstanceFinder(DgnDbR db, DgnChangeSummary& changeSummary, ECPresentationManagerR manager, Utf8StringCR rulesetId, RelationshipCachingOptions const& options)
    : m_db(db), m_changeSummary(changeSummary), m_relatedPropertyExplorer(db, changeSummary)
    {
    SetCachingOptions(options);
    // Find related property paths of the database at the current state
    RelatedPropertyPathCache afterStatePaths(manager, rulesetId);
    afterStatePaths.CachePaths(db, BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    VCLOG.infov("RelatedInstanceFinder: cached %d after state property paths", afterStatePaths.Get().size());
    // Add related property paths to cache map to be traversed later on
    AddRelatedPropertyPaths(afterStatePaths);
    // Find cacheable classes for performance if desired
    m_relatedPropertyExplorer.FindCacheableClasses(db, changeSummary, m_relatedPropertyPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceFinder::AddRelatedPropertyPaths(RelatedPropertyPathCache const& cache)
    {
    // Add to map so that clones are not traversed multiple times
    for (auto path : cache.Get())
        m_relatedPropertyPaths[path.MakeKey()] = path;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt parseClassFullName(DgnDbR db, Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName)
    {
    bvector<Utf8String> classParts;
    BeStringUtilities::Split(classFullName, ".:", nullptr, classParts);

    if (classParts.size() != 2)
        return ERROR;

    ECN::ECClassCP ecClass = db.Schemas().GetClass(classParts[0].c_str(), classParts[1].c_str());
    if (ecClass == nullptr)
        return ERROR;

    schemaName = ecClass->GetSchema().GetName();
    className = ecClass->GetName();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceFinder::GetAllRelatedInstances(bset<ECInstanceKey>& output, ECInstanceKey const& instance)
    {
    for (auto& pair : m_relatedPropertyPaths)
        {
        if (pair.second.IsPropertyProvider(instance.GetClassId()))
            m_relatedPropertyExplorer.Traverse(output, m_db, m_changeSummary, pair.second.Paths(), instance);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceFinder::GetAllRelatedInstancesForClassIds(bmap<ECInstanceKey, ECInstanceKeySet>& output, bmap<ECClassId,bset<ECInstanceKey>>& classToInstanceKeys)
    {
    VCLOG.infov("Getting all related instances for relevant class Ids");
    for (auto& pair : m_relatedPropertyPaths)
        {
        ECClassId targetClassId = pair.second.GetTargetClassId();
        if (!pair.second.IsPropertyProvider(targetClassId))
            continue;

        // Get all target keys that are relevant for property path
        bset<ECInstanceKey> allKeys;
        for (ECClassId const& classId : pair.second.GetTargetClassIds()) {
            auto currentKeys = classToInstanceKeys.find(classId);
            if (currentKeys == classToInstanceKeys.end())
                continue;
            for (auto key : currentKeys->second)
                allKeys.insert(key);
        }
        // Traverse property path if needed
        if (allKeys.size() > 0)
            m_relatedPropertyExplorer.TraverseInChunks(output, m_db, m_changeSummary, pair.second.Paths(), allKeys);
        }
    VCLOG.infov("Finished getting all related instances for relevant class Ids");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ParentFinder::CacheParents(DgnDbR db, DgnChangeSummary& changeSummary)
    {
    CacheDeletedChildToParentMap(db, changeSummary);
    CacheCurrentChildToParentMap(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ParentFinder::CacheDeletedChildToParentMap(DgnDbR db, DgnChangeSummary& changeSummary)
    {
    VCLOG.infov("CacheDeletedChildToParentMap: caching deleted ElementOwnsChildElements");
    ECClassCP cls = db.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
    if (cls == nullptr)
        {
        VCLOG.errorv("CacheDeletedChildToParentMap: Could not find ElementOwnsChildElements class!");
        return;
        }

    ECClassId relClassId = cls->GetId();
    // Query for finding the changed relationships targets and sources from the temp tables created by the change summary
    Utf8PrintfString sql("SELECT it.InstanceId, vt.AccessString, vt.OldValue "
        "FROM ec_cache_ClassHierarchy ch "
        "JOIN %s it ON ch.ClassId = it.ClassId "
        "JOIN %s vt ON vt.ClassId = it.ClassId AND vt.InstanceId = it.InstanceId "
        "WHERE ch.[BaseClassId] = ? AND it.DbOpcode = %d AND vt.AccessString in (\"TargetECInstanceId\", \"TargetECClassId\", \"SourceECInstanceId\", \"SourceECClassId\")",
        changeSummary.GetInstancesTableName().c_str(),
        changeSummary.GetValuesTableName().c_str(),
        DbOpcode::Delete);

    Statement stmt;
    DbResult stmtresult = stmt.Prepare(db, sql.c_str());
    if (stmtresult != DbResult::BE_SQLITE_OK)
        {
        VCLOG.errorv("CacheDeletedChildToParentMap: preparing statement for caching deleted ElementOwnsChildElements failed");
        return;
        }

    stmt.BindId(1, relClassId);

    bmap<ECInstanceId, RelationshipQueryResult> relationshipToValues;
    while (stmt.Step() != DbResult::BE_SQLITE_DONE)
        {
        ECInstanceId relationshipId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP accessStr = stmt.GetValueText(1);
        RelationshipQueryResult& qresult = relationshipToValues[relationshipId];
        if (strcmp(accessStr, "TargetECInstanceId") == 0)
            qresult.targetId = stmt.GetValueId<ECInstanceId>(2);
        else if (strcmp(accessStr, "TargetECClassId") == 0)
            qresult.targetClassId = stmt.GetValueId<ECClassId>(2);
        else if (strcmp(accessStr, "SourceECInstanceId") == 0)
            qresult.sourceId = stmt.GetValueId<ECInstanceId>(2);
        else if (strcmp(accessStr, "SourceECClassId") == 0)
            qresult.sourceClassId = stmt.GetValueId<ECClassId>(2);
        }

    // Create entry and populate it with change summary values before the relationship got deleted
    for (auto const& iter : relationshipToValues)
        {
        ECInstanceKey parentKey(iter.second.sourceClassId, iter.second.sourceId);
        ECInstanceKey childKey(iter.second.targetClassId, iter.second.targetId);
        m_deletedChildToParentMap[childKey] = parentKey;
        }
    VCLOG.infov("CacheDeletedChildToParentMap: found %d deleted ElementOwnsChildElements relationships", m_deletedChildToParentMap.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ParentFinder::CacheCurrentChildToParentMap(DgnDbR db)
    {
    VCLOG.infov("CacheCurrentChildToParentMap: caching ElementOwnsChildElements existing in db");
    Utf8PrintfString ecsql("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM %s.%s", BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(db, ecsql.c_str());
    if (status != ECSqlStatus::Success)
        {
        VCLOG.errorv("CacheCurrentChildToParentMap: could not prepare query for caching ElementOwnsChildElements");
        return;
        }

    while (stmt.Step() != BeSQLite::BE_SQLITE_DONE)
        {
        ECInstanceId parentId = stmt.GetValueId<ECInstanceId>(0);
        ECClassId parentClassId = stmt.GetValueId<ECClassId>(1);
        ECInstanceId childId = stmt.GetValueId<ECInstanceId>(2);
        ECClassId childClassId = stmt.GetValueId<ECClassId>(3);
        ECInstanceKey parentKey(parentClassId, parentId);
        ECInstanceKey childKey(childClassId, childId);
        m_currentChildToParentMap[childKey] = parentKey;
        }
    VCLOG.infov("CacheCurrentChildToParentMap: cached %d ElementOwnsChildElements relationships from db", m_currentChildToParentMap.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ParentFinder::FindDeletedParentKey(ECInstanceKey const& childKey)
    {
    auto parentKey = m_deletedChildToParentMap.find(childKey);
    if (parentKey == m_deletedChildToParentMap.end())
        return ECInstanceKey();

    return parentKey->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ParentFinder::FindCurrentParentKey(ECInstanceKey const& childKey)
    {
    auto parentKey = m_currentChildToParentMap.find(childKey);
    if (parentKey == m_currentChildToParentMap.end())
        return ECInstanceKey();

    return parentKey->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey MakeElementKey(DgnElementCPtr element)
    {
    if (!element.IsValid())
        return ECInstanceKey();

    return ECInstanceKey(
        ECClassId(element->GetElementClassId().GetValue()),
        ECInstanceId(element->GetElementId().GetValue())
        );
    }

/*---------------------------------------------------------------------------------**//**
* Find the parent key of the SummaryElementInfo by searching the change summary or Db
* depending on the change that has happened to the element (or parent)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ParentFinder::FindParentKey(DgnDbR db, DgnChangeSummary& changeSummary, ECInstanceKey const& key)
    {
    ChangeSummary::Instance instance = changeSummary.GetInstance(key.GetClassId(), key.GetInstanceId());
    if (!instance.IsValid() || instance.GetDbOpcode() != DbOpcode::Delete)
        return FindCurrentParentKey(key);

    // Use change summary to determine parent
    return FindDeletedParentKey(key);
    }

/*---------------------------------------------------------------------------------**//**
* Set the parent key of the SummaryElementInfo by searching the change summary or Db
* to find the top parent depending on the change that has happened to the element (or parent)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ParentFinder::FindTopParentKey(DgnDbR db, DgnChangeSummary& changeSummary, ECInstanceKey const& key)
    {
    ECInstanceKey parentKey;
    ECInstanceKey currentKey = key;
    while(currentKey.IsValid())
        {
        parentKey = currentKey;
        currentKey = FindParentKey(db, changeSummary, currentKey);
        }

    // Do not return the same key as the parent, just return an invalid key
    if (parentKey.GetInstanceId().GetValue() == key.GetInstanceId().GetValue())
        return ECInstanceKey();

    return parentKey;
    }

bool IsPlacementProperty(Utf8CP str)
    {
    return std::strcmp(str, ORIGIN_X_AS) == 0
        || std::strcmp(str, ORIGIN_Y_AS) == 0
        || std::strcmp(str, ORIGIN_Z_AS) == 0
        || std::strcmp(str, YAW_AS) == 0
        || std::strcmp(str, PITCH_AS) == 0
        || std::strcmp(str, ROLL_AS) == 0
        || std::strcmp(str, BBOXLOW_X_AS) == 0
        || std::strcmp(str, BBOXLOW_Y_AS) == 0
        || std::strcmp(str, BBOXLOW_Z_AS) == 0
        || std::strcmp(str, BBOXHIGH_X_AS) == 0
        || std::strcmp(str, BBOXHIGH_Y_AS) == 0
        || std::strcmp(str, BBOXHIGH_Z_AS) == 0;
    }

bool IsGeometricProperty(Utf8CP str)
    {
    return std::strcmp(str, CATEGORY_AS) == 0
        || std::strcmp(str, INSPATIALINDEX_AS) == 0
        || std::strcmp(str, GEOMETRYSTREAM_AS) == 0;
    }

bool IsLastMod(Utf8CP str)
    {
    return std::strcmp(str, LASTMOD_AS) == 0;
    }

bool IsModelId(Utf8CP str)
    {
    return std::strcmp(str, MODEL_ID) == 0;
    }

bool IsAspectReservedProperty(Utf8CP str)
    {
    return std::strcmp(str, ASPECT_ELEMID) == 0
        || std::strcmp(str, ASPECT_RELCLASSID) == 0;
    }

bool IsParentId(Utf8CP str)
    {
    return std::strcmp(str, PARENT_ID) == 0;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
template<typename T> static uint32_t computeFnv1Hash(T const& input)
    {
    auto cur = reinterpret_cast<unsigned char const*>(&input);
    auto end = cur + sizeof(input);

    uint32_t hash = 0x811c9dc5;
    while (cur < end)
        {
        hash *= 0x01000193;
        hash ^= static_cast<uint32_t>(*cur);
        ++cur;
        }

    return hash;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
static uint32_t computeFnv1HashUtf8CP(Utf8CP input)
    {
    uint32_t hash = 0x811c9dc5;
    for (size_t i = 0; i < strlen(input); ++i)
        {
        hash *= 0x01000193;
        hash ^= static_cast<uint32_t>(*(input + i));
        }

    return hash;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
uint32_t computeHash(DbDupValue const& value)
    {
    switch (value.GetValueType())
        {
        case DbValueType::IntegerVal:
            return computeFnv1Hash(value.GetValueInt64());
        case DbValueType::FloatVal:
            return computeFnv1Hash(value.GetValueDouble());
        case DbValueType::TextVal:
            return computeFnv1HashUtf8CP(value.GetValueText());
        case DbValueType::BlobVal:
            return computeFnv1Hash(value.GetValueBlob());
        case DbValueType::NullVal:
            return 0;
        }
    return 0;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
bool valueIsEqual(DbDupValue const& oldValue, DbDupValue const& newValue)
    {
    switch (oldValue.GetValueType())
        {
        case DbValueType::IntegerVal:
            return oldValue.GetValueInt64() == newValue.GetValueInt64();
        case DbValueType::FloatVal:
            return oldValue.GetValueDouble() == newValue.GetValueDouble();
        case DbValueType::TextVal:
            {
            Utf8CP oldVal = oldValue.GetValueText();
            Utf8CP newVal = newValue.GetValueText();
            if (newVal == nullptr)
                newVal = "";
            if (oldVal == nullptr)
                oldVal = "";
            return std::strcmp(oldVal, newVal) == 0;
            }
        case DbValueType::BlobVal:
            {
            uint32_t oldhash = computeFnv1Hash(oldValue.GetValueBlob());
            uint32_t newhash = computeFnv1Hash(newValue.GetValueBlob());
            return oldhash == newhash;
            }
        case DbValueType::NullVal:
            return newValue.GetValueType() == DbValueType::NullVal;
        }
    return true;
    }


//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void    ChangedElementFinder::GetChangesType(ElementChangesType& changes, DgnModelId& instanceModelId, DgnDbR db, ChangeSummary::Instance const& instance)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("VersionCompareChangeSummary::GetChangesType");
#endif
    ChangeSummary::ValueIterator changesIt = instance.MakeValueIterator();
    for (auto iter = changesIt.begin(); iter != changesIt.end(); ++iter)
        {
        auto str = iter.GetAccessString();
        auto oldVal = iter.GetOldValue();
        auto newVal = iter.GetNewValue();
        if (valueIsEqual(oldVal, newVal))
            {
            continue;
            }

        if (IsModelId(str))
            {
            DgnModelId newId = newVal.GetValueId<DgnModelId>();
            DgnModelId oldId = oldVal.GetValueId<DgnModelId>();
            instanceModelId = newId.IsValid() ? newId : oldId;
            }
        else if (IsGeometricProperty(str))
            {
            changes.AddType(ElementChangesType::Type::Mask_Geometry);
            }
        else if (IsPlacementProperty(str))
            {
            changes.AddType(ElementChangesType::Type::Mask_Placement);
            }
        else if (IsParentId(str))
            {
            changes.AddType(ElementChangesType::Type::Mask_Parent);
            }
        else if (!IsLastMod(str) && !IsAspectReservedProperty(str))
            {
            // Check for hidden properties
            if (!m_hiddenPropertyCache.IsHiddenProperty(db, instance.GetClassId(), str))
                {
                // Insert the changed property
                // Compute checksums based on the bytes of the old and new values
                uint32_t oldValueChecksum = m_wantChecksums ? computeHash(iter.GetOldValue()) : 0;
                uint32_t newValueChecksum = m_wantChecksums ? computeHash(iter.GetNewValue()) : 0;
                // Add property with checksum
                changes.AddProperty(str, oldValueChecksum, newValueChecksum);
                changes.AddType(ElementChangesType::Type::Mask_Property);
                }
            else
                changes.AddType(ElementChangesType::Type::Mask_Hidden);
            }
        }
#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("VersionCompareChangeSummary::GetChangesType");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangedElementFinder::QueryRelatedInstanceModelIds(DgnDbR db)
    {
    Utf8PrintfString ecsql("SELECT ECInstanceId, ECClassId, Model.Id FROM %s.%s WHERE InVirtualSet(?, ECInstanceId)", BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    ECSqlStatement stmt;
    ECSqlStatus stmtresult = stmt.Prepare(db, ecsql.c_str());
    if (!stmtresult.IsSuccess())
        {
        VCLOG.errorv("QueryRelatedInstanceModelIds: preparing statement for querying related instance model ids failed.");
        return;
        }

    BeSQLite::IdSet<ECInstanceId> ids;
    for (auto const& related : m_relatedInstanceChanges)
        ids.insert(related.first.GetInstanceId());

    stmt.BindVirtualSet(1, ids);

    bmap<ECInstanceId, RelationshipQueryResult> relationshipToValues;
    while (stmt.Step() != DbResult::BE_SQLITE_DONE)
        {
        ECInstanceId instanceId = stmt.GetValueId<ECInstanceId>(0);
        ECClassId classId = stmt.GetValueId<ECClassId>(1);
        DgnModelId modelId = stmt.GetValueId<DgnModelId>(2);

        ECInstanceKey key(classId, instanceId);
        if (m_relatedInstanceChanges.find(key) != m_relatedInstanceChanges.end())
            m_relatedInstanceChanges[key].m_modelId = modelId;
        else
            VCLOG.warningv("Related instance key does not match changes map");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangedElementFinder::ProcessInstance(DgnDbR db, DgnChangeSummary& changeSummary, ChangeSummary::Instance const& instance, RelatedInstanceFinder& finder)
    {
    DbOpcode opcode = instance.GetDbOpcode();
    DgnElementCPtr element = db.Elements().GetElement(DgnElementId(instance.GetInstanceId().GetValue()));
    ECInstanceKey key(instance.GetClassId(), instance.GetInstanceId());
    GeometrySource3dCP source = element.IsValid() ? element->ToGeometrySource3d() : nullptr;
    ChangedElementRecord info(opcode, instance.GetClassId(), element.IsValid() ? element->GetModelId() : DgnModelId(), nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d());

    // Get type of change
    ElementChangesType changes;
    DgnModelId modelId;
    GetChangesType(changes, modelId, db, instance);
    info.m_changes = changes;

    // Add parent key to the info object if it exists
    if (m_wantParentKeys)
        info.m_parentKey = m_parentFinder.FindTopParentKey(db, changeSummary, key);

    // Set model Id found in change summary if we don't have a valid one by now
    if (!info.m_modelId.IsValid())
        info.m_modelId = modelId;

    // Add to map
    m_changedInstances[key] = info;

    // Find related instances via property paths if we don't want to use chunk traversal
    if (!m_wantChunkTraversal)
        ProcessRelatedInstances(db, changeSummary, key, info, finder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangedElementFinder::ProcessRelatedInstances(DgnDbR db, DgnChangeSummary& changeSummary, ECInstanceKey const& instance, ChangedElementRecord const& record, RelatedInstanceFinder& finder)
    {
    ECInstanceKeySet propertyTargets;
    finder.GetAllRelatedInstances(propertyTargets, instance);
    for (auto const& key : propertyTargets)
        {
        // Create a record from accessor for the target element that displays a property from the instance
        ChangedElementRecord& targetRecord = m_relatedInstanceChanges[key];
        // Make the record an 'update'
        targetRecord.m_opcode = DbOpcode::Update;
        targetRecord.m_ecclassId = key.GetClassId();
        // Merge the property data into the related instance
        targetRecord.m_changes.MergeProperties(record.m_changes);
        // Mark that it has an indirect change
        targetRecord.m_changes.m_flags |= ElementChangesType::Mask_Indirect;
        // Find parent keys of the related instances
        if (m_wantParentKeys)
            targetRecord.m_parentKey = m_parentFinder.FindTopParentKey(db, changeSummary, key);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangedElementFinder::FindElementClassIds(bset<ECClassId>& classIds, DgnDbR db)
    {
    bvector<Utf8String> classParts;
    BeStringUtilities::Split(m_elementClassFullName.c_str(), ".:", nullptr, classParts);
    if (classParts.size() != 2)
        return;

    ECN::ECClassCP ecClass = db.Schemas().GetClass(classParts[0].c_str(), classParts[1].c_str());
    if (ecClass == nullptr)
        return;

    Utf8String ecsql("SELECT SourceECInstanceId FROM meta.ClassHasAllBaseClasses WHERE TargetECInstanceId=?");
    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(db, ecsql.c_str());
    if (status == ECSqlStatus::InvalidECSql)
        {
        BeAssert(false && "Invalid ECSql");
        return;
        }

    if (status == ECSqlStatus::Error)
        {
        BeAssert(false && "Failed to prepare statement");
        return;
        }

    stmt.BindId(1, ecClass->GetId());

    DbResult stepStatus;
    classIds.insert(ecClass->GetId());
    while ((stepStatus = stmt.Step()) == BeSQLite::BE_SQLITE_ROW)
        classIds.insert(stmt.GetValueId<ECClassId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceFinder ChangedElementFinder::CreateRelatedInstanceFinder(DgnDbR db, DgnChangeSummary& changeSummary, RelatedPropertyPathCache& beforeStateCache)
    {
    // Related instance finder based on property paths and ECPresentation
    RelationshipCachingOptions cachingOpts(!m_wantChunkTraversal && m_wantRelationshipCaching, m_relationshipCacheSize);
    RelatedInstanceFinder relatedInstanceFinder(db, changeSummary, m_presentationManager, m_rulesetId, cachingOpts);
    relatedInstanceFinder.AddRelatedPropertyPaths(beforeStateCache);
    return relatedInstanceFinder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangedElementFinder::FindRelatedInstances(RelatedInstanceFinder& finder, DgnDbR db, DgnChangeSummary& changeSummary)
    {
    // Chunk up instances by class id
    bmap<ECClassId, ECInstanceKeySet> classToInstance;
    for (auto const& pair : m_changedInstances)
        {
        // Create entry and get set
        ECInstanceKeySet& set = classToInstance[pair.first.GetClassId()];
        set.insert(pair.first);
        }

    // Target refers to the Aspect, type, etc that has an actual change in it,
    // going back to the source element that will show some of its properties
    bmap<ECInstanceKey, ECInstanceKeySet> targetToSource;
    // Process each class id accordingly and find all source elements from the target changes
    finder.GetAllRelatedInstancesForClassIds(targetToSource, classToInstance);

    // Clear up some memory
    classToInstance.clear();

    // Go through all the found indirectly affected elements and store them in results
    for (auto const &pair : targetToSource)
        {
        // Get the changed instances from the database that affected the element
        ChangedElementRecord& leafRecordWithChanges = m_changedInstances[pair.first];
        for (auto const& relatedKey : pair.second)
            {
            // Create a record from accessor for the target element that displays a property from the instance
            ChangedElementRecord &changeRecord = m_relatedInstanceChanges[relatedKey];
            // Make the record an 'update'
            changeRecord.m_opcode = DbOpcode::Update;
            changeRecord.m_ecclassId = relatedKey.GetClassId();
            // Merge the property data into the related instance
            changeRecord.m_changes.MergeProperties(leafRecordWithChanges.m_changes);
            // Mark that it has an indirect change
            changeRecord.m_changes.m_flags |= ElementChangesType::Mask_Indirect;
            // Find parent keys of the related instances
            if (m_wantParentKeys)
                changeRecord.m_parentKey = m_parentFinder.FindTopParentKey(db, changeSummary, relatedKey);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ChangedElementFinder::GetChangedElementsFromSummary(
    bvector<ChangedElementInfo>& elements,
    DgnDbR db,
    DgnChangeSummary& changeSummary,
    RelatedPropertyPathCache& beforeStateCache)
    {
    // Prepare statements for parent finder
    VCLOG.infov("GetChangedElementsFromSummary: caching parents");
    m_parentFinder.CacheParents(db, changeSummary);

    // 1. Find changed instances and their related instances in change summary
    RelatedInstanceFinder relatedInstanceFinder = CreateRelatedInstanceFinder(db, changeSummary, beforeStateCache);

    VCLOG.infov("GetChangedElementsFromSummary: start processing all instances");
    for (auto const& entry : changeSummary.MakeInstanceIterator())
        ProcessInstance(db, changeSummary, entry.GetInstance(), relatedInstanceFinder);

    // Find related instances using chunk traversal functionality
    if (m_wantChunkTraversal)
        FindRelatedInstances(relatedInstanceFinder, db, changeSummary);

    // Query related instances' model ids
    QueryRelatedInstanceModelIds(db);

    VCLOG.infov("GetChangedElementsFromSummary: finished processing all instances");
    // 2. Merge the found related instance property changes into the main changed instances map
    for (auto const& related : m_relatedInstanceChanges)
        {
        if (m_changedInstances.find(related.first) == m_changedInstances.end())
            // If not found, add the related record as the change to the element
            m_changedInstances[related.first] = related.second;
        else
            // If found, merge the changes and properties
            m_changedInstances[related.first].m_changes.Merge(related.second.m_changes);
        }
    // 3. Get all derived classes of element
    VCLOG.infov("GetChangedElementsFromSummary: finding all element class Ids");
    bset<ECClassId> elementClassIds;
    FindElementClassIds(elementClassIds, db);

    // 4. Add all found elements to the output vector
    VCLOG.infov("GetChangedElementsFromSummary: adding changed instances to results");
    for (auto& pair : m_changedInstances)
        {
        if (elementClassIds.find(pair.first.GetClassId()) != elementClassIds.end())
            {
            // Clean properties off of non-update elements after we have found related instances
            if (pair.second.m_opcode != DbOpcode::Update)
                pair.second.m_changes.ClearProperties();

            // Add element to results
            elements.push_back(ChangedElementInfo(pair.first, pair.second));
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
VersionCompareChangeSummary::~VersionCompareChangeSummary()
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("~VersionCompareChangeSummary");
#endif
    m_changedElements.clear();

    // Clean up target db temporary file if it was cloned to begin with
    bool cloneDb = WantTargetState() && !m_wantBriefcaseRoll;
    BeFileName tempFileName = m_targetDb->GetFileName();
    // Close Db
    m_targetDb->CloseDb();
    if (cloneDb)
        {
        if (BeFileNameStatus::Success != tempFileName.BeDeleteFile())
            {
            BeAssert(false);
            }
        }
#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("~VersionCompareChangeSummary");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    VersionCompareChangeSummary::ProcessChangesets()
    {
    VCLOG.infov("ProcessChangesets: Started processing changesets");

    if (m_changesets.empty())
        {
        VCLOG.errorv("ProcessChangesets: No changesets passed to be processed");
        return ERROR;
        }

    if (!m_dbFilename.DoesPathExist())
        {
        VCLOG.errorv("ProcessChangesets: Db filename does not exist on disk");
        return ERROR;
        }

    // Open Db file
    DbResult openStatus;
	DgnDb::OpenParams params(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes);

    // Clone the db if necessary
    bool cloneDb = WantTargetState() && !m_wantBriefcaseRoll;
    m_targetDb = cloneDb
        ? CloneDb(m_dbFilename, m_tempLocation)
        : DgnDb::OpenDgnDb(&openStatus, m_dbFilename, params);

    if (!m_targetDb.IsValid())
        {
        VCLOG.errorv("ProcessChangesets: Problem cloning Db");
        return ERROR;
        }

    // Presentation rules need to know about the db
    if (m_presentationManager.GetConnections().GetConnection(*m_targetDb) == nullptr)
        m_presentationManager.GetConnections().CreateConnection(*m_targetDb);

    // Element class to use as base class when finding changed elements
    Utf8PrintfString elementClassFullName("%s.%s", BIS_ECSCHEMA_NAME, m_filterSpatial ? BIS_CLASS_SpatialElement : BIS_CLASS_Element);

    // Construct change summaries
    for (DgnRevisionPtr changeset : m_changesets)
        {
#ifdef PROFILE_VC_PROCESSING
    Utf8PrintfString operation("ProcessChangeset");
    Profile_StartClock(operation);
#endif
        VCLOG.infov("ProcessChangesets: Processing changeset %s", changeset->GetChangesetId().c_str());
        VCLOG.infov("ProcessChangesets: Caching deleted property paths");
        // Cache for property paths of the before-state to find relevant paths for deleted elements and relationships
        RelatedPropertyPathCache beforeStateCache(m_presentationManager, m_rulesetId);
        beforeStateCache.CachePaths(*m_targetDb, BIS_ECSCHEMA_NAME, BIS_CLASS_Element);

        VCLOG.infov("ProcessChangesets: Cached %d deleted property paths", beforeStateCache.Get().size());

        // When going forwards, we may need to apply the changeset before we process it
        if ((WantTargetState() || m_wantBriefcaseRoll) && m_targetDb->Revisions().GetParentRevisionId().Equals(changeset->GetParentId()))
            {
            VCLOG.infov("ProcessChangesets: Applying changeset");
            bvector<DgnRevisionPtr> changesets;
            changesets.push_back(changeset);
            if (SUCCESS != RollTargetDb(changesets))
                {
                VCLOG.errorv("ProcessChangesets: Failed to apply changesets");
                return ERROR;
                }
            }

        VCLOG.infov("ProcessChangesets: Extracting change summary for changed elements processing");

        // Create a summary with the current target db
        DgnChangeSummary changeSummary(*m_targetDb);
        // Put together the changeset
        RevisionChangesFileReader fr (changeset->GetRevisionChangesFile(), *m_targetDb);
        changeSummary.FromChangeSet(fr);

// #define DUMP_CHANGE_SUMMARIES
#ifdef DUMP_CHANGE_SUMMARIES
        changeSummary.Dump();
#endif

        VCLOG.infov("ProcessChangesets: Finding changed elements");

        ChangedElementFinder finder(m_presentationManager, m_rulesetId, elementClassFullName, m_wantParentKeys, m_wantPropertyChecksums, m_wantRelationshipCaching, m_relationshipCacheSize, m_wantChunkTraversal);
        bvector<ChangedElementInfo> changedElements;
        // Get changed elements for this changeset
        finder.GetChangedElementsFromSummary(changedElements, *m_targetDb, changeSummary, beforeStateCache);

        VCLOG.infov("ProcessChangesets: Found %d changed elements", changedElements.size());

        // Accumulate the changed elements for the current changeset
        for (auto const& element : changedElements)
            {
            if (m_changedElements.find(element.m_instanceKey) == m_changedElements.end())
                // Add record
                m_changedElements[element.m_instanceKey] = element.m_record;
            else
                // Accumulate change of opcodes and merge properties
                m_changedElements[element.m_instanceKey].AccumulateChange(element.m_record);

            // Remove elements that got invalidated (e.g. Insert + Delete)
            if (!m_changedElements[element.m_instanceKey].IsValid())
                m_changedElements.erase(element.m_instanceKey);
            }

        // Clear temporary element cache
        m_elementCache.clear();
#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport(operation);
#endif
        }

    VCLOG.infov("ProcessChangesets: Finished processing changesets");

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr    VersionCompareChangeSummary::GetTargetDb()
    {
    return m_targetDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr    VersionCompareChangeSummary::CloneDb(BeFileNameCR dbFilename, BeFileNameCR location)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("VersionCompareChangeSummary::CloneDb");
#endif
    BeSQLite::DbResult result;
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck));

    BeFileName tempFilename;
    if (location.IsEmpty())
        T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFilename, L"VersionCompareTemp");
    else
        tempFilename = location;

    WString name = WString(L"Temp_") + dbFilename.GetFileNameWithoutExtension();
    tempFilename.AppendToPath(name.c_str());
    tempFilename.AppendExtension(L"bim");

    // Try to create temporary file by copying base bim file
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(dbFilename, tempFilename);

    // If we get any error besides already exists, return error, something went wrong
    if (BeFileNameStatus::Success != fileStatus && BeFileNameStatus::AlreadyExists != fileStatus)
        return nullptr;

    // If the file was copied before, act on it instead of re-copying each time we compare revisions
    // Open the target db using the temporary filename
    DgnDbPtr targetDb = DgnDb::OpenDgnDb(&result, tempFilename, params);
    if (!targetDb.IsValid())
        return nullptr;

#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("VersionCompareChangeSummary::CloneDb");
#endif
    return targetDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::RollTargetDb(bvector<DgnRevisionPtr> const& changesets)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("VersionCompareChangeSummary::RollTargetDb");
#endif
    bvector<DgnRevisionCP> changesetsCP;
    for (DgnRevisionPtr changeset : changesets)
        changesetsCP.push_back(changeset.get());

    // Close db
    BeFileName filename = m_targetDb->GetFileName();
    m_targetDb->CloseDb();
    // Re-open to apply changesets that contain schema changes
    BeSQLite::DbResult result;
	DgnDb::OpenParams params(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes);
    params.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(changesetsCP, RevisionProcessOption::Merge);
    m_targetDb = DgnDb::OpenDgnDb(&result, filename, params);
    BeAssert(result == BeSQLite::BE_SQLITE_OK && m_targetDb.IsValid());

    // Presentation rules need to know about the reopened Db
    if (m_targetDb.IsValid())
        {
        // Create connections
        m_presentationManager.GetConnections().CreateConnection(*m_targetDb);
        }

    BeAssert(m_targetDb.IsValid() && !m_targetDb->IsReadonly());

#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("VersionCompareChangeSummary::RollTargetDb");
#endif
    return (result == BeSQLite::BE_SQLITE_OK && m_targetDb.IsValid()) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::GetChangedElements(bvector<ChangedElement>& elements)
    {
    for (bmap<ECInstanceKey,ChangedElementRecord>::iterator it = m_changedElements.begin(); it != m_changedElements.end(); ++it)
        {
        elements.push_back(ChangedElement((DgnElementId(it->first.GetInstanceId().GetValue())),
                            it->first.GetClassId(),
                            it->second.m_modelId,
                            it->second.m_bbox,
                            it->second.m_opcode,
                            it->second.m_changes,
                            it->second.m_parentKey
                            ));
        }

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::GetChangedElementsOfClass(bvector<ChangedElement>& elements, ECClassCP classp)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("VersionCompareChangeSummary::GetChangedElementsOfClass");
#endif
    for (bmap<ECInstanceKey,ChangedElementRecord>::iterator it = m_changedElements.begin(); it != m_changedElements.end(); ++it)
        {
        ECClassId classId = it->first.GetClassId();
        ECClassCP cls = m_targetDb->Schemas().GetClass(classId);
        BeAssert(cls != nullptr);
        if (cls != nullptr && cls->Is(classp))
            {
            elements.push_back(ChangedElement((DgnElementId(it->first.GetInstanceId().GetValue())),
                            it->first.GetClassId(),
                            it->second.m_modelId,
                            it->second.m_bbox,
                            it->second.m_opcode,
                            it->second.m_changes,
                            it->second.m_parentKey
                            ));
            }
        }

#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("VersionCompareChangeSummary::GetChangedElementsOfClass");
#endif
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
StatusInt   VersionCompareChangeSummary::GetChangedModels(bset<DgnModelId>& modelIds, bvector<DbOpcode>& opcodes)
    {
#ifdef PROFILE_VC_PROCESSING
    Profile_StartClock("VersionCompareChangeSummary::GetChangedModels");
#endif
    ECClassCP modelClass = m_targetDb->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Model);
    bvector<ChangedElement> elements;
    if (SUCCESS != GetChangedElementsOfClass(elements, modelClass))
        return ERROR;

    for (ChangedElement const& element: elements)
        modelIds.insert(element.m_modelId);

#ifdef PROFILE_VC_PROCESSING
    Profile_EndAndReport("VersionCompareChangeSummary::GetChangedModels");
#endif
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   VersionCompareChangeSummary::SetChangesets(bvector<DgnRevisionPtr>& changesets)
    {
    m_changesets = changesets;

    if (SUCCESS != ProcessChangesets())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
VersionCompareChangeSummaryPtr  VersionCompareChangeSummary::Generate(BeFileName dbFilename, bvector<DgnRevisionPtr>& changesets, ECPresentationManagerR presentationManager, Utf8StringCR rulesetId)
    {
    // Create the change summary
    VersionCompareChangeSummaryPtr changeSummary = new VersionCompareChangeSummary(dbFilename, presentationManager);
    changeSummary->m_rulesetId = rulesetId;
    // Set changesets and store all changed elements
    if (SUCCESS != changeSummary->SetChangesets(changesets))
        return nullptr;

    return changeSummary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
VersionCompareChangeSummaryPtr  VersionCompareChangeSummary::Generate(BeFileName dbFilename, bvector<DgnRevisionPtr>& changesets, SummaryOptions const& options)
    {
    if (options.presentationManager == nullptr)
        {
        BeAssert(false && "SummaryOptions need to provide a presentation manager to generate change summary");
        return nullptr;
        }

    // Create the change summary
    VersionCompareChangeSummaryPtr changeSummary = new VersionCompareChangeSummary(dbFilename, *options.presentationManager);
    changeSummary->m_filterSpatial = options.filterSpatial;
    changeSummary->m_filterLastMod = options.filterLastMod;
    changeSummary->m_rulesetId = options.rulesetId;
    changeSummary->m_tempLocation = options.tempLocation;
    changeSummary->m_wantTargetState = options.wantTargetState;
    changeSummary->m_wantParentKeys = options.wantParents;
    changeSummary->m_wantPropertyChecksums = options.wantPropertyChecksums;
    changeSummary->m_wantBriefcaseRoll = options.wantBriefcaseRoll;
    changeSummary->m_wantRelationshipCaching = options.wantRelationshipCaching;
    changeSummary->m_relationshipCacheSize = options.relationshipCacheSize;
    changeSummary->m_wantChunkTraversal = options.wantChunkTraversal;

    // Set changesets and store all changed elements
    if (SUCCESS != changeSummary->SetChangesets(changesets))
        return nullptr;

    return changeSummary;
    }
