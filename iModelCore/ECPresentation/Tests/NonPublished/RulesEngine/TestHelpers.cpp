/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::InitSchemaRegistry(ECDbR ecdb, bmap<Utf8String, Utf8String> const& schemaXmls)
    {
    bvector<ECSchemaPtr> schemas;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    for (auto pair : schemaXmls)
        {
        ECSchemaPtr schema;
        ECSchema::ReadFromXmlString(schema, pair.second.c_str(), *schemaReadContext);
        if (!schema.IsValid())
            {
            BeAssert(false);
            continue;
            }
        schemas.push_back(schema);
        }

    if (!schemas.empty())
        {
        bvector<ECSchemaCP> importSchemas;
        importSchemas.resize(schemas.size());
        std::transform(schemas.begin(), schemas.end(), importSchemas.begin(), [](ECSchemaPtr const& schema) { return schema.get(); });

        ASSERT_TRUE(SUCCESS == ecdb.Schemas().ImportSchemas(importSchemas));
        ecdb.SaveChanges();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::Paths RulesEngineTestHelpers::GetPaths(BeTest::Host& host)
    {
    BeFileName assetsDirectory, temporaryDirectory;
    host.GetDgnPlatformAssetsDirectory(assetsDirectory);
    host.GetTempDir(temporaryDirectory);
    return RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RulesEngineTestHelpers::GetDisplayLabel(IECInstanceCR instance)
    {
    Utf8String label;
    if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
        return Utf8String(label.c_str());

    return Utf8String(instance.GetClass().GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr RulesEngineTestHelpers::InsertInstance(ECDbR db, ECInstanceInserter& inserter, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer, bool commit)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance();
    if (nullptr != instancePreparer)
        instancePreparer(*instance);
    inserter.Insert(*instance);
    
    if (commit)
        db.SaveChanges();

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RulesEngineTestHelpers::InsertRelationship(ECDbR db, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer, bool commit)
    {
    ECInstanceId sourceId;
    ECInstanceId::FromString(sourceId, source.GetInstanceId().c_str());
    ECInstanceKey sourceKey(source.GetClass().GetId(), sourceId);

    ECInstanceId targetId;
    ECInstanceId::FromString(targetId, target.GetInstanceId().c_str());
    ECInstanceKey targetKey(target.GetClass().GetId(), targetId);

    ECSqlStatement stmt;
    if (!stmt.Prepare(db, "SELECT Name FROM meta.ECPropertyDef WHERE NavigationRelationshipClass.Id=?").IsSuccess())
        {
        BeAssert(false);
        return ECInstanceKey();
        }
    stmt.BindId(1, relationship.GetId());
    if (BE_SQLITE_ROW == stmt.Step())
        {
        // Navigation property-backed relationship handling
        Utf8CP navigationPropertyName = stmt.GetValueText(0);
        ECClassCP targetClass = db.Schemas().GetClass(targetKey.GetClassId());
        BeAssert(nullptr == instancePreparer && "Navigation property relationships aren't backed by an instance");
    
        ECSqlStatement updateStmt;
        Utf8String updateQuery = Utf8String("UPDATE ").append(targetClass->GetECSqlName())
            .append(" SET ").append(navigationPropertyName).append(" = ?")
            .append(" WHERE ECInstanceId = ?");
        if (!updateStmt.Prepare(db, updateQuery.c_str()).IsSuccess())
            {
            BeAssert(false);
            return ECInstanceKey();
            }
        updateStmt.BindNavigationValue(1, sourceKey.GetInstanceId(), relationship.GetId());
        updateStmt.BindId(2, targetKey.GetInstanceId());
        updateStmt.Step();
        target.SetValue(navigationPropertyName, ECValue(sourceKey.GetInstanceId()));

        if (commit)
            db.SaveChanges();

        return ECInstanceKey(relationship.GetId(), targetKey.GetInstanceId());
        }
        
    // regular relationship
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(relationship);
    StandaloneECRelationshipInstancePtr instance = relationshipEnabler->CreateRelationshipInstance();
    if (nullptr != instancePreparer)
        instancePreparer(*instance);
    
    ECInstanceKey key;
    ECInstanceInserter inserter(db, relationship, nullptr);
    DbResult result = inserter.InsertRelationship(key, sourceKey.GetInstanceId(), targetKey.GetInstanceId(), instance.get());
    BeAssert(DbResult::BE_SQLITE_OK == result);
    
    if (commit)
        db.SaveChanges();

    return key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RulesEngineTestHelpers::InsertRelationship(ECDbTestProject& project, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer, bool commit)
    {
    return InsertRelationship(project.GetECDb(), relationship, source, target, instancePreparer, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr RulesEngineTestHelpers::InsertInstance(ECDbR db, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer, bool commit)
    {
    ECInstanceInserter inserter(db, ecClass, nullptr);
    return InsertInstance(db, inserter, ecClass, instancePreparer, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstances(ECDbR db, ECClassCR ecClass, bool polymorphic, bool commit)
    {
    Utf8String ecsql = Utf8String("DELETE FROM ").append(ecClass.GetECSqlName());
    ECSqlStatement stmt;
    stmt.Prepare(db, ecsql.c_str());
    stmt.Step();

    if (commit)
        db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbR db, ECInstanceKeyCR key, bool commit)
    {
    ECClassCP ecClass = db.Schemas().GetClass(key.GetClassId());
    if (nullptr == ecClass)
        {
        BeAssert(false);
        return;
        }

    if (ecClass->IsRelationshipClass())
        {
        ECSqlStatement stmt;
        stmt.Prepare(db, "SELECT Class.Id, Name FROM meta.ECPropertyDef WHERE NavigationRelationshipClass.Id=?");
        stmt.BindId(1, key.GetClassId());
        if (BE_SQLITE_ROW == stmt.Step())
            {
            // Navigation property-backed relationship handling
            ECClassId targetClassId = stmt.GetValueId<ECClassId>(0);
            Utf8CP navigationPropertyName = stmt.GetValueText(1);
            ECClassCP targetClass = db.Schemas().GetClass(targetClassId);

            ECSqlStatement updateStmt;
            Utf8String updateQuery = Utf8String("UPDATE ").append(targetClass->GetECSqlName())
                .append(" SET ").append(navigationPropertyName).append(" = NULL")
                .append(" WHERE ECInstanceId = ?");
            if (!updateStmt.Prepare(db, updateQuery.c_str()).IsSuccess())
                {
                BeAssert(false);
                return;
                }
            updateStmt.BindId(1, key.GetInstanceId());
            updateStmt.Step();

            if (commit)
                db.SaveChanges();

            return;
            }
        }
    
    ECSqlStatement statement;
    Utf8PrintfString sql("DELETE FROM %s WHERE ECInstanceId = ?", ecClass->GetECSqlName().c_str());
    ECSqlStatus status = statement.Prepare(db, sql.c_str());
    if (!status.IsSuccess())
        {
        BeAssert(false);
        return;
        }
    statement.BindId(1, key.GetInstanceId());
    BeSQLite::DbResult result = statement.Step();
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_DONE);
    
    if (commit)
        db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbR db, IECInstanceCR instance, bool commit)
    {
    ECInstanceId id;
    ECInstanceId::FromString(id, instance.GetInstanceId().c_str());
    ECInstanceKey key(instance.GetClass().GetId(), id);
    DeleteInstance(db, key, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbTestProject& project, IECInstanceCR instance, bool commit)
    {
    DeleteInstance(project.GetECDb(), instance, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbTestProject& project, ECInstanceKeyCR key, bool commit)
    {
    DeleteInstance(project.GetECDb(), key, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr RulesEngineTestHelpers::GetInstance(ECDbR db, ECClassCR ecClass, ECInstanceId id)
    {
    SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s] WHERE ECInstanceId = ?", ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare(db, ecSql.GetUtf8CP());
    EXPECT_EQ(ECSqlStatus::Success, status) << "RulesEngineTestHelpers::GetInstance> Preparing ECSQL '" << ecSql.GetUtf8CP () << "' failed.";
    if (status != ECSqlStatus::Success)
        return nullptr;

    status = ecStatement.BindId(1, id);
    EXPECT_EQ(ECSqlStatus::Success, status) << "RulesEngineTestHelpers::GetInstance> Failed to bind ECInstanceId";

    DbResult result = ecStatement.Step();
    EXPECT_EQ(BE_SQLITE_ROW, result) << "RulesEngineTestHelpers::GetInstance> Instance not found.";
    if (result != BE_SQLITE_ROW)
        return nullptr;        

    ECInstanceECSqlSelectAdapter adapter(ecStatement);
    IECInstancePtr instance = adapter.GetInstance();
    BeAssert (instance.IsValid());
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassInstanceKey RulesEngineTestHelpers::GetInstanceKey(IECInstanceCR instance)
    {
    ECInstanceId instanceId;
    ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());
    return ECClassInstanceKey(instance.GetClass(), instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexNavigationQueryPtr RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(ECClassCR ecClass, NavigationQueryR instanceNodesQuery)
    {
    instanceNodesQuery.GetResultParametersR().SetResultType(NavigationQueryResultType::Invalid);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*MultiECInstanceNodesQueryContract::Create(&ecClass, false));
    query->From(instanceNodesQuery);
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexNavigationQueryPtr RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(ECEntityClassCR ecClass, bool polymorphic, Utf8CP alias, bvector<RelatedClass> const& relatedClasses)
    {
    RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ecClass, relatedClasses);
    return &ComplexNavigationQuery::Create()->SelectContract(*contract, alias).From(ecClass, polymorphic, alias);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryPtr RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler)
    {
    NavigationQueryPtr q;
    for (auto pair: classes)
        {
        ComplexNavigationQueryPtr query = CreateECInstanceNodesQueryForClass(*pair.first, pair.second, alias);
        if (handler)
            handler(*query);

        query = CreateMultiECInstanceNodesQuery(*pair.first, *query);

        if (q.IsNull())
            q = query;
        else
            q = UnionNavigationQuery::Create(*q, *query);
        }
    return q;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryPtr RulesEngineTestHelpers::CreateLabelGroupingNodesQueryForClasses(ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler)
    {
    NavigationQueryPtr q;
    for (auto pair : classes)
        {
        RefCountedPtr<DisplayLabelGroupingNodesQueryContract> contract = DisplayLabelGroupingNodesQueryContract::Create(pair.first);
        ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract, alias).From(*pair.first, pair.second, alias);
        if (handler)
            handler(*query);

        if (q.IsNull())
            q = query;
        else
            q = UnionNavigationQuery::Create(*q, *query);
        }
    return q;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryPtr RulesEngineTestHelpers::CreateQuery(NavigationQueryContract const& contract, bset<ECN::ECClassCP> classes, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler)
    {
    if (classes.empty())
        return nullptr;

    NavigationQueryPtr query;
    for (ECClassCP from : classes)
        {
        ComplexNavigationQueryPtr thisQuery = &ComplexNavigationQuery::Create()->SelectContract(contract, alias).From(*from, polymorphic, alias);
        if (handler)
            handler(*thisQuery);

        if (query.IsNull())
            query = thisQuery;
        else
            query = UnionNavigationQuery::Create(*query, *thisQuery);
        }
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryPtr RulesEngineTestHelpers::CreateQuery(NavigationQueryContract const& contract, bvector<ECN::ECClassCP> classes, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler)
    {
    if (classes.empty())
        return nullptr;

    NavigationQueryPtr query;
    for (ECClassCP from : classes)
        {
        ComplexNavigationQueryPtr thisQuery = &ComplexNavigationQuery::Create()->SelectContract(contract, alias).From(*from, polymorphic, alias);
        if (handler)
            handler(*thisQuery);

        if (query.IsNull())
            query = thisQuery;
        else
            query = UnionNavigationQuery::Create(*query, *thisQuery);
        }
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ECValue GetECValueFromJson(RapidJsonValueCR json, ECPropertyCR ecProperty)
    {
    ECValue value;
    if (json.IsNull())
        value.SetIsNull(true);
    else if (ecProperty.GetIsNavigation())
        value.SetNavigationInfo(BeInt64Id(json.GetInt64()));
    else
        {
        switch (ecProperty.GetAsPrimitiveProperty()->GetType())
            {
            case PRIMITIVETYPE_Boolean:
                value.SetBoolean(json.GetBool());
                break;
            case PRIMITIVETYPE_Integer:
                value.SetInteger(json.GetInt());
                break;
            case PRIMITIVETYPE_Long:
                value.SetLong(json.GetInt64());
                break;
            case PRIMITIVETYPE_Double:
                value.SetDouble(json.GetDouble());
                break;
            case PRIMITIVETYPE_String:
                value.SetUtf8CP(json.GetString());
                break;
            default:
                BeAssert(false);
            }        
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssertInstanceValueValid(IECInstanceCR instance, RapidJsonValueCR value, Utf8CP propertyName)
    {
    ECClassCR instanceClass = instance.GetClass();
    ECPropertyCP property = instanceClass.GetPropertyP(propertyName);
    ASSERT_TRUE(nullptr != property);

    ASSERT_TRUE(property->GetIsNavigation() || property->GetIsPrimitive());
    
    ECValue instanceValue;
    ECObjectsStatus status = instance.GetValue(instanceValue, propertyName);
    EXPECT_EQ(ECObjectsStatus::Success, status);
    EXPECT_EQ(instanceValue, GetECValueFromJson(value, *property));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateContentSetItem(IECInstanceCR instance, ContentSetItemCR item, ContentDescriptorCR descriptor, Utf8CP expectedLabel, Utf8CP expectedImageId)
    {
    rapidjson::Document json = item.AsJson();
    EXPECT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("PrimaryKeys"));
    RapidJsonValueCR keys = json["PrimaryKeys"];
    ASSERT_EQ(1, keys.Size());
    EXPECT_STREQ(instance.GetClass().GetId().ToString().c_str(), keys[0]["ECClassId"].GetString());
    EXPECT_STREQ(instance.GetInstanceId().c_str(), keys[0]["ECInstanceId"].GetString());
    
    ASSERT_TRUE(json.HasMember("DisplayLabel"));
    if (nullptr != expectedLabel)
        EXPECT_STREQ(expectedLabel, json["DisplayLabel"].GetString());
    else
        EXPECT_STREQ("", json["DisplayLabel"].GetString());

    ASSERT_TRUE(json.HasMember("ImageId"));
    if (nullptr != expectedImageId)
        EXPECT_STREQ(expectedImageId, json["ImageId"].GetString());
    else
        EXPECT_STREQ("", json["ImageId"].GetString());

    ASSERT_TRUE(json.HasMember("Values"));
    RapidJsonValueCR values = json["Values"];

    bvector<ContentDescriptor::Field*> fields = descriptor.GetVisibleFields();
    for (ContentDescriptor::Field const* field : fields)
        {
        Utf8CP fieldName = field->GetName().c_str();
        ASSERT_TRUE(values.HasMember(fieldName));
        if (field->IsDisplayLabelField())
            EXPECT_STREQ(expectedLabel, values[fieldName].GetString());
        else
            {
            for (ContentDescriptor::Property const& prop : field->AsPropertiesField()->GetProperties())
                {
                if (&prop.GetProperty().GetClass() == &instance.GetClass())
                    AssertInstanceValueValid(instance, values[fieldName], prop.GetProperty().GetName().c_str());
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP> instances, Content const& content, bool validateOrder)
    {
    DataContainer<ContentSetItemCPtr> contentSet = content.GetContentSet();
    ASSERT_EQ(instances.size(), contentSet.GetSize());
    size_t index = 0;
    for (ContentSetItemCPtr const& item : contentSet)
        {
        IECInstanceCP instance = nullptr;
        if (validateOrder)
            {
            instance = instances[0];
            instances.erase(instances.begin());
            }
        else
            {
            bvector<ECClassInstanceKey> const& itemKeys = item->GetKeys();
            bset<Utf8String> itemInstanceIds;
            std::transform(itemKeys.begin(), itemKeys.end(), std::inserter(itemInstanceIds, itemInstanceIds.end()), [](ECClassInstanceKeyCR key){return key.GetId().ToString();});
            auto iter = std::find_if(instances.begin(), instances.end(),
                [&itemInstanceIds](IECInstanceCP instance){return itemInstanceIds.end() != itemInstanceIds.find(instance->GetInstanceId());});
            ASSERT_TRUE(instances.end() != iter);
            instance = *iter;
            instances.erase(iter);
            }
        ValidateContentSetItem(*instance, *item, content.GetDescriptor());
        index++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* The first time we request nodes, we get them straight from the creator data providers.
* The second time - we get them from cache providers. To make sure both types of providers
* return the same result, nodes should be requested through this helper function.
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DataContainer<NavNodeCPtr> RulesEngineTestHelpers::GetValidatedNodes(std::function<DataContainer<NavNodeCPtr>()> getter)
    {
    // allow the test to verify the nodes
    DataContainer<NavNodeCPtr> initialNodes = getter();

    // additionally, make sure another get, which returns nodes from cache,
    // returns the same result
    DataContainer<NavNodeCPtr> cachedNodes = getter();

    EXPECT_EQ(initialNodes.GetSize(), cachedNodes.GetSize())
        << "Results of initial request (generator) should match results of second request (cached)";
    if (initialNodes.GetSize() != cachedNodes.GetSize())
        return initialNodes;

    for (size_t i = 0; i < initialNodes.GetSize(); ++i)
        {
        EXPECT_TRUE(cachedNodes[i]->Equals(*initialNodes[i]))
            << "Nodes from initial request should match nodes from cached request";
        }

    return initialNodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& RulesEngineTestHelpers::AddField(ContentDescriptorR descriptor, ECClassCR primaryClass, 
    ContentDescriptor::Property prop, IPropertyCategorySupplierR categorySupplier)
    {
    ContentDescriptor::Field* field = new ContentDescriptor::ECPropertiesField(categorySupplier.CreateCategory(primaryClass, prop.GetProperty(), RelationshipMeaning::SameInstance), prop);
    descriptor.AddField(field);
    return *descriptor.GetAllFields().back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::CacheNode(IHierarchyCacheR cache, JsonNavNodeR node)
    {
    NavNodeExtendedData extendedData(node);
    uint64_t virtualParentId = extendedData.HasVirtualParentId() ? extendedData.GetVirtualParentId() : 0;
    HierarchyLevelInfo hlInfo = cache.FindHierarchyLevel(extendedData.GetConnectionId(),
        extendedData.GetRulesetId(), extendedData.GetLocale(), extendedData.HasVirtualParentId() ? &virtualParentId : nullptr);
    if (!hlInfo.IsValid())
        {
        hlInfo = HierarchyLevelInfo(extendedData.GetConnectionId(), extendedData.GetRulesetId(),
            extendedData.GetLocale(), node.GetParentNodeId(), virtualParentId);
        cache.Cache(hlInfo);
        }
    DataSourceInfo dsInfo = cache.FindDataSource(hlInfo.GetId(), { 0 });
    if (!dsInfo.IsValid())
        {
        dsInfo = DataSourceInfo(hlInfo.GetId(), { 0 });
        cache.Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
        }
    cache.Cache(node, dsInfo, 0, NodeVisibility::Visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TestPropertyFormatter::_GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
    {
    if (!ecValue.IsPrimitive())
        {
        EXPECT_TRUE(false);
        return ERROR;
        }
    Utf8String value = ecValue.ToString();
    if (value.empty())
        formattedValue = value;
    else
        formattedValue = Utf8String().append("_").append(value).append("_");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksonras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TestPropertyFormatter::_GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR, RelatedClassPath const&, RelationshipMeaning) const
    {   
    formattedLabel = Utf8String().append("_").append(ecProperty.GetDisplayLabel()).append("_");
    return SUCCESS;
    }
