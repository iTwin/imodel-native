/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaManager::SchemaManager(ECDbCR ecdb, BeMutex& mutex) : m_dispatcher(new Dispatcher(ecdb, mutex)) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaManager::~SchemaManager()
    {
    if (m_dispatcher != nullptr)
        {
        delete m_dispatcher;
        m_dispatcher = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::ImportSchemas(bvector<ECSchemaCP> const& schemas, SchemaImportOptions options, SchemaImportToken const* token) const
    {
    return Main().ImportSchemas(schemas, options, token);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DropSchemaResult SchemaManager::DropSchema(Utf8StringCR name, SchemaImportToken const* token, bool logIssue) const {
    return Main().DropSchema(name, token, logIssue);
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECSchemaCP> SchemaManager::GetSchemas(bool loadSchemaEntities, Utf8CP tableSpace) const { return m_dispatcher->GetSchemas(loadSchemaEntities, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaManager::ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode, Utf8CP tableSpace)  const { return m_dispatcher->ContainsSchema(schemaNameOrAlias, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaManager::GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetSchema(schemaNameOrAlias, loadSchemaEntities, mode, tableSpace); }


/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetClass(schemaNameOrAlias, className, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMapStrategy SchemaManager::GetClassMapStrategy(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetClassMapStrategy(schemaNameOrAlias, className, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::GetClass(ECClassId ecClassId, Utf8CP tableSpace) const { return m_dispatcher->GetClass(ecClassId, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP SchemaManager::FindClass(Utf8StringCR className, Utf8CP tableSpace) const { return m_dispatcher->FindClass(className, tableSpace); }
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId SchemaManager::GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetClassId(schemaNameOrAlias, className, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaManager::OwnsSchema(ECN::ECSchemaCR schema) const { return m_dispatcher->OwnsSchema(schema); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDerivedClassesList const& SchemaManager::GetDerivedClasses(ECClassCR ecClass, Utf8CP tableSpace) const 
    {
    if (GetDerivedClassesInternal(ecClass, tableSpace) == nullptr)
        {
        BeAssert(false && "SchemaManager::GetDerivedClasses failed");
        }

    return ecClass.GetDerivedClasses();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDerivedClassesList const* SchemaManager::GetDerivedClassesInternal(ECClassCR ecClass, Utf8CP tableSpace) const { return m_dispatcher->GetDerivedClasses(ecClass, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP SchemaManager::GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetEnumeration(schemaNameOrAlias, enumName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP SchemaManager::GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetKindOfQuantity(schemaNameOrAlias, koqName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECUnitCP SchemaManager::GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetUnit(schemaNameOrAlias, unitName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECFormatCP SchemaManager::GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetFormat(schemaNameOrAlias, formatName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
UnitSystemCP SchemaManager::GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitSystemName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetUnitSystem(schemaNameOrAlias, unitSystemName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PhenomenonCP SchemaManager::GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetPhenomenon(schemaNameOrAlias, phenName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyCategoryCP SchemaManager::GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR catName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetPropertyCategory(schemaNameOrAlias, catName, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaManager::ClearCache() const { return m_dispatcher->ClearCache(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSchemaPtr SchemaManager::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) { return m_dispatcher->LocateSchema(key, matchType, schemaContext, nullptr); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::CreateClassViewsInDb() const { return Main().CreateClassViews(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::CreateClassViewsInDb(bvector<ECN::ECClassId> const& ecclassids) const { return Main().CreateClassViews(ecclassids); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaChangeEvent& SchemaManager::OnBeforeSchemaChanges() const { return Main().OnBeforeSchemaChanges();}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaChangeEvent& SchemaManager::OnAfterSchemaChanges() const { return Main().OnAfterSchemaChanges();};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::RepopulateCacheTables() const { return Main().RepopulateCacheTables(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::UpgradeECInstances() const { return Main().UpgradeECInstances() == BE_SQLITE_OK ? SUCCESS : ERROR; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaManager::Dispatcher const& SchemaManager::GetDispatcher() const { BeAssert(m_dispatcher != nullptr); return *m_dispatcher; }

MainSchemaManager const& SchemaManager::Main() const { return GetDispatcher().Main(); }

//! SchemaManager::SchemaManagerUnitsContext

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
template<
    typename LookupFunc,
    typename LookupResult = decltype(std::declval<LookupFunc>()(std::declval<ECSchemaCP>(), "", true)),
    typename NonConstResult = std::remove_const_t<std::remove_pointer_t<LookupResult>>*
>
static auto SchemaManagerUnitsContextGenericLookup(
    SchemaManager const& manager,
    Utf8CP name,
    bool useFullName,
    LookupFunc lookupFunc // e.g.: ECN::ECUnitCP(ECSchemaCP, Utf8CP, bool)
) -> NonConstResult
{
    bvector<Utf8String> nameParts;
    BeStringUtilities::Split(name, ":", nameParts);
    if (nameParts.size() == 2)
        {
        auto const& schemaRef = nameParts[0];
        auto const& unqualifiedName = nameParts[1];
        auto const& schemaObject = manager.GetSchema(schemaRef, true, SchemaLookupMode::AutoDetect);
        if (schemaObject == nullptr) return nullptr;
        //! const_cast is safe because we know SchemaUnitsContext is over the non-const returning _Lookup
        return const_cast<NonConstResult>(lookupFunc(schemaObject, unqualifiedName.c_str(), useFullName));
        }
    else
        LOG.errorv("bad or unqualified unit reference syntax, '%s'", name);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::ECUnitP SchemaManager::SchemaManagerUnitsContext::_LookupUnitP(Utf8CP name, bool useFullName) const
    {
    return SchemaManagerUnitsContextGenericLookup(m_ref, name, useFullName,
        [&](ECSchemaCP schemaObject, Utf8CP unqualifiedName, bool useFullName) {
            return schemaObject->GetUnitsContext().LookupUnit(unqualifiedName, useFullName);
        }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::PhenomenonP SchemaManager::SchemaManagerUnitsContext::_LookupPhenomenonP(Utf8CP name, bool useFullName) const
    {
    return SchemaManagerUnitsContextGenericLookup(m_ref, name, useFullName,
        [&](ECSchemaCP schemaObject, Utf8CP unqualifiedName, bool useFullName) {
            return schemaObject->GetUnitsContext().LookupPhenomenon(unqualifiedName, useFullName);
        }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::UnitSystemP SchemaManager::SchemaManagerUnitsContext::_LookupUnitSystemP(Utf8CP name, bool useFullName) const
    {
    return SchemaManagerUnitsContextGenericLookup(m_ref, name, useFullName,
        [&](ECSchemaCP schemaObject, Utf8CP unqualifiedName, bool useFullName) {
            return schemaObject->GetUnitsContext().LookupUnitSystem(unqualifiedName, useFullName);
        }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaManager::SchemaManagerUnitsContext::_AllPhenomena(bvector<Units::PhenomenonCP>& allPhenomena) const
    {
    for (auto const& schema : m_ref.GetSchemas())
        schema->GetUnitsContext().AllPhenomena(allPhenomena);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaManager::SchemaManagerUnitsContext::_AllUnits(bvector<Units::UnitCP>& allUnits) const
    {
    for (auto const& schema : m_ref.GetSchemas())
        schema->GetUnitsContext().AllUnits(allUnits);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaManager::SchemaManagerUnitsContext::_AllSystems(bvector<Units::UnitSystemCP>& allUnitSystems) const
    {
    for (auto const& schema : m_ref.GetSchemas())
        schema->GetUnitsContext().AllSystems(allUnitSystems);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceFinder::SearchResults::ToJson(BeJsValue v, ECInstanceKey const& key, JsonFormatOptions const* options) {
    v.toObject();
    if (options && options->GetUseClassNameForInstanceKey()) {
        auto cls = options->GetECDb().Schemas().GetClass(key.GetClassId());
        if (cls != nullptr)
            v["class"] = cls->GetFullName();
    } else
        v["class"] = key.GetClassId().ToString(BeInt64Id::UseHex::Yes);
    v["id"] = key.GetInstanceId().ToString(BeInt64Id::UseHex::Yes);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceFinder::SearchResults::ToJson(BeJsValue v, ForignKeyRelation const& key, JsonFormatOptions const* options) {
    v.toObject();
    ToJson(v["thisEnd"], key.GetThisEndKey(), options);
    ToJson(v["otherEnd"], key.GetOtherEndKey(), options);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceFinder::SearchResults::ToJson(BeJsValue v, LinkTableRelation const& key, JsonFormatOptions const* options) {
    v.toObject();
    ToJson(v["relation"], key.GetRelationKey(), options);
    ToJson(v["source"], key.GetSourceKey(), options);
    ToJson(v["target"], key.GetTargetKey(), options);
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceFinder::SearchResults::ToJson(BeJsValue& v, JsonFormatOptions const* options) const {
        if (IsEmpty()) {
            v.SetEmptyObject();
            return;
        }
        v.toObject();
        if (!m_entityKeyMap.empty()) {
            auto jCol = v["entities"];
            for (auto& kp: m_entityKeyMap) {
                const auto classId = kp.first;
                const auto& instances = kp.second;
                auto jObj = jCol.appendValue();
                if (options && options->GetUseClassNameForBaseClass()) {
                    auto cls = options->GetECDb().Schemas().GetClass(classId);
                    if (cls != nullptr)
                        jObj["baseClass"] = cls->GetFullName();
                } else
                    jObj["baseClass"] = classId.ToString(BeInt64Id::UseHex::Yes);

                auto jInstances = jObj["instances"];
                for (auto& inst: instances) {
                    SearchResults::ToJson(jInstances.appendValue(), inst, options);
                }
            }
        }
        if (!m_linktableRelMap.empty()) {
            auto jCol = v["linkTableRelations"];
            for (auto& kp: m_linktableRelMap) {
                const auto classId = kp.first;
                const auto& instances = kp.second;
                auto jObj = jCol.appendValue();
                if (options && options->GetUseClassNameForBaseClass()) {
                    auto cls = options->GetECDb().Schemas().GetClass(classId);
                    if (cls != nullptr)
                        jObj["baseClass"] = cls->GetFullName();
                } else
                    jObj["baseClass"] = classId.ToString(BeInt64Id::UseHex::Yes);

                auto jInstances = jObj["instances"];
                for (auto& inst: instances) {
                    SearchResults::ToJson(jInstances.appendValue(), inst, options);
                }
            }
        }
        if (!m_foreignKeyMap.empty()) {
            auto jCol = v["foreignKeyRelations"];
            for (auto& kp: m_foreignKeyMap) {
                const auto classId = kp.first;
                const auto& instances = kp.second;
                auto jObj = jCol.appendValue();
                if (options && options->GetUseClassNameForBaseClass()) {
                    auto cls = options->GetECDb().Schemas().GetClass(classId);
                    if (cls != nullptr)
                        jObj["baseClass"] = cls->GetFullName();
                } else
                    jObj["baseClass"] = classId.ToString(BeInt64Id::UseHex::Yes);

                auto jInstances = jObj["instances"];
                for (auto& inst: instances) {
                    SearchResults::ToJson(jInstances.appendValue(), inst, options);
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceFinder::SearchResults::Debug(ECDbCR ecdb) {
    auto getName = [&](ECClassId baseClassId) {
        return ecdb.Schemas().GetClass(baseClassId)->GetFullName();
    };
    printf("[entity instances]\n");
    for(auto kp : m_entityKeyMap) {
        auto baseClassId = kp.first;
        auto& instanceKeys = kp.second;
        printf("\t[baseClass: %s]\n", getName(baseClassId));
        for (auto& inst: instanceKeys) {
            printf("\t\t[class: %s, id: %s]\n", getName(inst.GetClassId()), inst.GetInstanceId().ToString(BeInt64Id::UseHex::Yes).c_str());
        }
    }
    printf("[link table relationships]\n");
    for(auto kp : m_linktableRelMap) {
        auto baseClassId = kp.first;
        auto& instanceKeys = kp.second;
        printf("\t[baseClass: %s]\n", getName(baseClassId));
        for (auto& inst: instanceKeys) {
            printf(
                "\t\t[relation: %s, id: %s] [source: %s, id: %s] [target: %s, id: %s]\n",
                getName(inst.GetRelationKey().GetClassId()),
                inst.GetRelationKey().GetInstanceId().ToString(BeInt64Id::UseHex::Yes).c_str(),
                getName(inst.GetSourceKey().GetClassId()),
                inst.GetSourceKey().GetInstanceId().ToString(BeInt64Id::UseHex::Yes).c_str(),
                getName(inst.GetTargetKey().GetClassId()),
                inst.GetTargetKey().GetInstanceId().ToString(BeInt64Id::UseHex::Yes).c_str());
        }
    }
    printf("[foreign key relationships]\n");
    for(auto kp : m_foreignKeyMap) {
        auto baseClassId = kp.first;
        auto& instanceKeys = kp.second;
        printf("\t[baseClass: %s]\n", getName(baseClassId));
        for (auto& inst: instanceKeys) {
            printf(
                "\t\t[thisEnd: %s, id: %s] [otherEnd: %s, id: %s]\n",
                getName(inst.GetThisEndKey().GetClassId()),
                inst.GetThisEndKey().GetInstanceId().ToString(BeInt64Id::UseHex::Yes).c_str(),
                getName(inst.GetOtherEndKey().GetClassId()),
                inst.GetOtherEndKey().GetInstanceId().ToString(BeInt64Id::UseHex::Yes).c_str());
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<ECClassCP> InstanceFinder::GetRootEntityAndRelationshipClasses(ECDbCR ecdb) {
    const auto  sql = R"(
        SELECT
                [cc].[Id]
        FROM   [ec_Class] [cc]
                JOIN [ec_Schema] [ss] ON [ss].[Id] = [cc].[SchemaId]
                LEFT JOIN [ec_ClassHasBaseClasses] [bb] ON [bb].[ClassId] = [cc].[Id]
                JOIN [ec_ClassMap] [mm] ON [mm].[ClassId] = [cc].[Id]
        WHERE  NOT EXISTS (SELECT [c0].[Id]
                FROM   [ec_class] [c0]
                        JOIN [ec_CustomAttribute] [ca] ON [ca].[ClassId] = [c0].[Id]
                WHERE  [c0].[Name] = 'IsMixIn' AND [ca].[ContainerId] = [cc].[Id])
                    AND [mm].[MapStrategy] NOT IN (10, 11, 3)
                    AND [bb].[BaseClassId] IS NULL
                    AND [cc].[Type] IN (0, 1)
                    AND [ss].[Name] NOT IN ('ECDbMeta', 'ECDbSystem', 'ECDbFileInfo');)";

    std::vector<ECClassCP> classes;
    auto stmt = ecdb.GetCachedStatement(sql);
    BeAssert(stmt != nullptr);
    while (stmt->Step() == BE_SQLITE_ROW) {
        const auto classId = stmt->GetValueId<ECClassId>(0);
        const auto classCP = ecdb.Schemas().GetClass(classId);
        BeAssert(classCP != nullptr);
        classes.push_back(classCP);
    }
    return classes;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<NavigationECPropertyCP> InstanceFinder::GetNavigationProps(ECDbCR ecdb) {
    const auto  sql = R"(
        SELECT
                [pp].[ClassId],
                [pp].[Name]
        FROM   [ec_Property] [pp]
                JOIN [ec_Class] [cc] ON [cc].[Id] = [pp].[ClassId]
                JOIN [ec_Schema] [ss] ON [ss].[Id] = [cc].[schemaId]
        WHERE  [ss].[Name] NOT IN ('ECDbMeta', 'ECDbSystem', 'ECDbFileInfo')
                    AND [pp].[NavigationRelationshipClassId] IS NOT NULL
        ORDER  BY
                    [pp].[ClassId],
                    [pp].[Id];)";
    auto stmt = ecdb.GetCachedStatement(sql);
    BeAssert(stmt != nullptr);
    std::vector<NavigationECPropertyCP> props;
    while (stmt->Step() == BE_SQLITE_ROW) {
        const auto classId = stmt->GetValueId<ECClassId>(0);
        const auto propName = stmt->GetValueText(1);
        const auto classCP = ecdb.Schemas().GetClass(classId);
        BeAssert(classCP != nullptr);
        const auto navProp = classCP->GetPropertyP(propName)->GetAsNavigationProperty();
        props.push_back(navProp);
    }
    return props;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
InstanceFinder::SearchResults InstanceFinder::FindInstances(ECDbCR ecdb, ECSchemaId schemaId, bool polymorphic) {
    const auto sql = polymorphic? R"(
        SELECT [cc].[ClassId]
        FROM   [ec_cache_ClassHierarchy] [cc]
                JOIN [ec_ClassMap] [mm] ON [mm].[ClassId] = [cc].[ClassId]
                JOIN [ec_Class] [ss] ON [ss].[Id] = [cc].[BaseClassId]
        WHERE  [ss].[SchemaId] = ? AND [mm].[MapStrategy] NOT IN (10, 11, 3) 
        GROUP BY [cc].[ClassId]; )" : R"(
        SELECT [cc].[Id]
        FROM   [ec_class] [cc]
                JOIN [ec_ClassMap] [mm] ON [mm].[ClassId] = [cc].[Id]
        WHERE  [cc].[SchemaId] = ? AND [mm].[MapStrategy] NOT IN (10, 11, 3);)";
    auto stmt = ecdb.GetCachedStatement(sql);
    stmt->BindId(1, schemaId);
    BeIdSet classIds;
    while (stmt->Step() == BE_SQLITE_ROW) {
        classIds.insert(stmt->GetValueId<ECClassId>(0));
    }
    return InstanceFinder::FindInstances(ecdb, std::move(classIds));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
InstanceFinder::SearchResults InstanceFinder::FindInstances(ECDbCR ecdb, ECClassId classId, bool polymorphic) {
    BeIdSet classIds;
    if (polymorphic) {
        const auto sql = R"(
                SELECT [cc].[Id]
                FROM   [ec_Class] [cc]
                        JOIN [ec_schema] [ss] ON [ss].[Id] = [cc].[SchemaId]
                        JOIN [ec_cache_ClassHierarchy] [ch] ON [ch].[ClassId] = [cc].[Id]
                        JOIN [ec_ClassMap] [mm] ON [mm].[ClassId] = [cc].[Id]
                WHERE  NOT EXISTS (SELECT [c0].[Id]
                        FROM   [ec_class] [c0]
                                JOIN [ec_CustomAttribute] [ca] ON [ca].[ClassId] = [c0].[Id]
                        WHERE  [c0].[Name] = 'IsMixIn' AND [ca].[ContainerId] = [cc].[Id])
                            AND [mm].[MapStrategy] NOT IN (10, 11, 3)
                            AND [cc].[Type] IN (0, 1)
                            AND [ch].[BaseClassId]=?;)";
        auto stmt = ecdb.GetCachedStatement(sql);
        stmt->BindId(1, classId);

        while (stmt->Step() == BE_SQLITE_ROW) {
            classIds.insert(stmt->GetValueId<ECClassId>(0));
        }
    }
    else {
        classIds.insert(classId);
    }
    return InstanceFinder::FindInstances(ecdb, std::move(classIds));
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
InstanceFinder::SearchResults InstanceFinder::FindInstances(ECDbCR ecdb, BeIdSet&& classIds) {
    ECSqlStatementCache stmtCache(20);
    const auto rootClasses = InstanceFinder::GetRootEntityAndRelationshipClasses(ecdb);
    std::shared_ptr<IdSet<BeInt64Id>> classIdSet = std::make_shared<IdSet<BeInt64Id>>(std::move(classIds));
    std::map<ECClassId, std::vector<ECInstanceKey>> entityKeyMap;
    std::map<ECClassId, std::vector<LinkTableRelation>> linkTableKeyMap;
    std::map<ECClassId, std::vector<ForignKeyRelation>> foreignKeyMap;
    bset<ECInstanceKey> recordedEntities;
    //1. record entity first
    //2. link table relationship with classes in source or target
    //3. fk with nav property classid in listed classes
    for (auto entityClass : rootClasses) {
        const std::string entitySql = "SELECT ECClassId, ECInstanceId FROM " + std::string(entityClass->GetFullName()) + " WHERE InVirtualSet(?, ECClassId)";
        auto entityStmt = stmtCache.GetPreparedStatement(ecdb, entitySql.c_str());
        auto& entityKeys = entityKeyMap[entityClass->GetId()];
        entityStmt->BindIdSet(1, classIdSet);
        while (entityStmt->Step() == BE_SQLITE_ROW) {
            entityKeys.emplace_back(ECInstanceKey(entityStmt->GetValueId<ECClassId>(0), entityStmt->GetValueId<ECInstanceId>(1)));
            recordedEntities.insert(entityKeys.back());
        }
        if (entityKeys.empty()) {
            entityKeyMap.erase(entityClass->GetId());
        }

        if (entityClass->IsRelationshipClass()) {
            auto& relationKeys = linkTableKeyMap[entityClass->GetId()];
            const std::string linkTableSql = "SELECT ECClassId, ECInstanceId, SourceECClassId, SourceECInstanceId, TargetECClassId, TargetECInstanceId FROM " + std::string(entityClass->GetFullName()) + " WHERE InVirtualSet(:idset, SourceECClassId) OR InVirtualSet(:idset, TargetECClassId)";
            auto linkTableStmt = stmtCache.GetPreparedStatement(ecdb, linkTableSql.c_str());
            linkTableStmt->BindIdSet(1, classIdSet);
            while (linkTableStmt->Step() == BE_SQLITE_ROW) {
                auto relationKey = ECInstanceKey(linkTableStmt->GetValueId<ECClassId>(0), linkTableStmt->GetValueId<ECInstanceId>(1));
                if (recordedEntities.find(relationKey) != recordedEntities.end())
                    continue;

                recordedEntities.insert(relationKey);
                relationKeys.emplace_back(
                    LinkTableRelation(
                        std::move(relationKey),
                        ECInstanceKey(linkTableStmt->GetValueId<ECClassId>(2), linkTableStmt->GetValueId<ECInstanceId>(3)),
                        ECInstanceKey(linkTableStmt->GetValueId<ECClassId>(4), linkTableStmt->GetValueId<ECInstanceId>(5))));
            }
            if (relationKeys.empty()) {
                linkTableKeyMap.erase(entityClass->GetId());
            }
        }
    }
    for (auto navProp : InstanceFinder::GetNavigationProps(ecdb)) {
        auto rel = navProp->GetRelationshipClass();
        auto targetClassCP = rel->GetTarget().GetAbstractConstraint() ? rel->GetTarget().GetAbstractConstraint() : rel->GetTarget().GetConstraintClasses().front();
        auto sourceClassCP = rel->GetSource().GetAbstractConstraint() ? rel->GetSource().GetAbstractConstraint() : rel->GetSource().GetConstraintClasses().front();
        auto otherClass = navProp->GetDirection() == ECRelatedInstanceDirection::Forward ? targetClassCP: sourceClassCP;
        std::string navPropSql = SqlPrintfString("SELECT aa.ECClassId, aa.ECInstanceId, cc.ECClassId, aa.%s.Id FROM %s aa JOIN %s cc ON cc.ECInstanceId=aa.%s.Id WHERE InVirtualSet(?, cc.ECClassId)",
            navProp->GetName().c_str(), navProp->GetClass().GetFullName(), otherClass->GetFullName(), navProp->GetName().c_str()).GetUtf8CP();
        auto navPropStmt = stmtCache.GetPreparedStatement(ecdb, navPropSql.c_str());
        navPropStmt->BindIdSet(1, classIdSet);
        auto& relationKeys = foreignKeyMap[rel->GetId()];
        while (navPropStmt->Step() == BE_SQLITE_ROW) {
            auto thisEnd = ECInstanceKey(navPropStmt->GetValueId<ECClassId>(0), navPropStmt->GetValueId<ECInstanceId>(1));
            if (recordedEntities.find(thisEnd) != recordedEntities.end())
                continue;

            recordedEntities.insert(thisEnd);
            auto otherEnd = ECInstanceKey(navPropStmt->GetValueId<ECClassId>(2), navPropStmt->GetValueId<ECInstanceId>(3));
            relationKeys.emplace_back(ForignKeyRelation(std::move(thisEnd), std::move(otherEnd)));
        }
        if (relationKeys.empty()) {
            foreignKeyMap.erase(rel->GetId());
        }
    }
    return SearchResults(std::move(entityKeyMap), std::move(linkTableKeyMap), std::move(foreignKeyMap));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP DropSchemaResult::GetStatusAsString() const {
    switch(m_status) {
        case Status::Success:
            return "schema drop successfully";
        case Status::ErrorSchemaNotFound:
            return "failed to drop schema, schema not found";
        case Status::ErrorDeletedSchemaIsReferencedByAnotherSchema:
            return "failed to drop schema, schema to be dropped is referenced by another schema";
        case Status::ErrorDeleteSchemaHasClassesWithInstances:
            return "failed to drop schema, schema to be dropped has one or more instances";
        case Status::ErrorDbIsReadonly:
            return "failed to drop schema, db is readonly";
        case Status::ErrorDbHasLocalChanges:
            return "failed to drop schema, there is local changes";
        case Status::ErrorSchemaLockFailed:
            return "failed to drop schema, unable to acquire schema lock";
        case Status::ErrorCouldNotAcquireLocksOrCodes:
            return "failed to drop schema, unable to acquire lock or codes";
    };
    return "fail to drop schema";
}

END_BENTLEY_SQLITE_EC_NAMESPACE


