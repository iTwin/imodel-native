/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::InitSchemaRegistry(ECDbR ecdb, bvector<bpair<Utf8String, Utf8String>> const& schemaXmls)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationManager::Paths RulesEngineTestHelpers::GetPaths(BeTest::Host& host)
    {
    BeFileName assetsDirectory, temporaryDirectory;
    host.GetDgnPlatformAssetsDirectory(assetsDirectory);
    host.GetTempDir(temporaryDirectory);
    return ECPresentationManager::Paths(assetsDirectory, temporaryDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RulesEngineTestHelpers::GetDisplayLabel(IECInstanceCR instance)
    {
    Utf8String label;
    if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
        return Utf8String(label.c_str());

    return Utf8String(instance.GetClass().GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
    UNUSED_VARIABLE(result);
    BeAssert(DbResult::BE_SQLITE_OK == result);

    if (commit)
        db.SaveChanges();

    return key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RulesEngineTestHelpers::InsertRelationship(ECDbTestProject& project, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer, bool commit)
    {
    return InsertRelationship(project.GetECDb(), relationship, source, target, instancePreparer, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr RulesEngineTestHelpers::InsertInstance(ECDbR db, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer, bool commit)
    {
    ECInstanceInserter inserter(db, ecClass, nullptr);
    return InsertInstance(db, inserter, ecClass, instancePreparer, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbR db, IECInstanceCR instance, bool commit)
    {
    ECInstanceId id;
    ECInstanceId::FromString(id, instance.GetInstanceId().c_str());
    ECInstanceKey key(instance.GetClass().GetId(), id);
    DeleteInstance(db, key, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbTestProject& project, IECInstanceCR instance, bool commit)
    {
    DeleteInstance(project.GetECDb(), instance, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::DeleteInstance(ECDbTestProject& project, ECInstanceKeyCR key, bool commit)
    {
    DeleteInstance(project.GetECDb(), key, commit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassInstanceKey RulesEngineTestHelpers::GetInstanceKey(IECInstanceCR instance)
    {
    ECInstanceId instanceId;
    ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());
    return ECClassInstanceKey(instance.GetClass(), instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr RulesEngineTestHelpers::CreateDisplayLabelField(ECSchemaHelper const& schemaHelper, SelectClass<ECClass> const& selectClass,
    bvector<RelatedClassPath> const& relatedInstancePaths, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrides)
    {
    return QueryBuilderHelpers::CreateDisplayLabelField(ECInstanceNodesQueryContract::DisplayLabelFieldName, schemaHelper,
        selectClass, nullptr, nullptr, relatedInstancePaths, labelOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr RulesEngineTestHelpers::CreateNullDisplayLabelField()
    {
    return PresentationQueryContractSimpleField::Create(ECInstanceNodesQueryContract::DisplayLabelFieldName, "NULL", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilderPtr RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(ECClassCR ecClass, PresentationQueryBuilderR instanceNodesQuery)
    {
    auto displayLabelField = instanceNodesQuery.GetContract()->GetField(ECInstanceNodesQueryContract::DisplayLabelFieldName);
    instanceNodesQuery.GetNavigationResultParameters().SetResultType(NavigationQueryResultType::Invalid);
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*MultiECInstanceNodesQueryContract::Create("", &ecClass, const_cast<PresentationQueryContractFieldP>(displayLabelField.get()), false));
    query->From(instanceNodesQuery);
    query->GetNavigationResultParameters().GetSelectInstanceClasses().insert(&ecClass);
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilderPtr RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(ECSchemaHelper const& schemaHelper, SelectClass<ECClass> const& selectClass, bvector<RelatedClassPath> const& relatedInstancePaths)
    {
    RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create("", &selectClass.GetClass(), CreateDisplayLabelField(schemaHelper, selectClass), relatedInstancePaths);
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract, selectClass.GetAlias().c_str()).From(selectClass);
    query->GetNavigationResultParameters().GetSelectInstanceClasses().insert(&selectClass.GetClass());
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBuilderPtr RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(ECSchemaHelper const& schemaHelper, ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler)
    {
    auto q = UnionQueryBuilder::Create(bvector<PresentationQueryBuilderPtr>());
    for (auto pair: classes)
        {
        ComplexQueryBuilderPtr query = CreateECInstanceNodesQueryForClass(schemaHelper, SelectClass<ECClass>(*pair.first, alias, pair.second));
        if (handler)
            handler(*query);

        query = CreateMultiECInstanceNodesQuery(*pair.first, *query);
        if (1 == classes.size())
            return query;

        q->AddQuery(*query);
        }
    return q;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBuilderPtr RulesEngineTestHelpers::CreateQuery(NavigationQueryContract const& contract, bset<ECN::ECClassCP> classes, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler)
    {
    if (classes.empty())
        return nullptr;

    auto query = UnionQueryBuilder::Create(bvector<PresentationQueryBuilderPtr>());
    for (ECClassCP from : classes)
        {
        ComplexQueryBuilderPtr thisQuery = &ComplexQueryBuilder::Create()->SelectContract(contract, alias).From(*from, polymorphic, alias);
        thisQuery->GetNavigationResultParameters().GetSelectInstanceClasses().insert(from);

        if (handler)
            handler(*thisQuery);

        if (1 == classes.size())
            return thisQuery;

        query->AddQuery(*thisQuery);
        }
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBuilderPtr RulesEngineTestHelpers::CreateQuery(NavigationQueryContract const& contract, bvector<ECN::ECClassCP> classes, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler)
    {
    if (classes.empty())
        return nullptr;

    auto query = UnionQueryBuilder::Create(bvector<PresentationQueryBuilderPtr>());
    for (ECClassCP from : classes)
        {
        ComplexQueryBuilderPtr thisQuery = &ComplexQueryBuilder::Create()->SelectContract(contract, alias).From(*from, polymorphic, alias);
        thisQuery->GetNavigationResultParameters().GetSelectInstanceClasses().insert(from);

        if (handler)
            handler(*thisQuery);

        if (1 == classes.size())
            return thisQuery;

        query->AddQuery(*thisQuery);
        }
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
static ECValue GetECValueFromJson(RapidJsonValueCR json, ECPropertyCR ecProperty)
    {
    ECValue value;
    if (json.IsNull())
        {
        value.SetIsNull(true);
        return value;
        }

    if (ecProperty.GetIsNavigation())
        {
        value.SetNavigationInfo(BeInt64Id::FromString(json["ECInstanceId"].GetString()));
        return value;
        }

    if (!ecProperty.GetIsPrimitive())
        {
        BeAssert(false);
        return value;
        }

    return ValueHelpers::GetECValueFromJson(ecProperty.GetAsPrimitiveProperty()->GetType(), json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateContentSetItemPropertyFields(IECInstanceCR instance, ContentSetItemCR item, bvector<ContentDescriptor::Field*> const& fields)
    {
    rapidjson::Document json = item.AsJson();
    ASSERT_TRUE(json.HasMember("Values"));
    RapidJsonValueCR values = json["Values"];
    for (ContentDescriptor::Field const* field : fields)
        {
        Utf8CP fieldName = field->GetUniqueName().c_str();
        ASSERT_TRUE(values.HasMember(fieldName));
        if (field->IsPropertiesField())
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
* @bsitest
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
        {
        ASSERT_TRUE(json["DisplayLabel"].HasMember("DisplayValue"));
        EXPECT_STREQ(expectedLabel, json["DisplayLabel"]["DisplayValue"].GetString());
        }
    else
        {
        EXPECT_TRUE(!json["DisplayLabel"].IsObject() || json["DisplayLabel"].ObjectEmpty());
        }

    ASSERT_TRUE(json.HasMember("ImageId"));
    if (nullptr != expectedImageId)
        EXPECT_STREQ(expectedImageId, json["ImageId"].GetString());
    else
        EXPECT_STREQ("", json["ImageId"].GetString());

    ValidateContentSetItemPropertyFields(instance, item, descriptor.GetVisibleFields());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP> instances, Content const& content, bool validateOrder)
    {
    ValidateContentSet(ContainerHelpers::TransformContainer<bvector<InstanceInputAndResult>>(instances, [](auto const& resultInstance)
        {
        return InstanceInputAndResult(nullptr, resultInstance);
        }), content, validateOrder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateContentSet(bvector<InstanceInputAndResult> instances, Content const& content, bool validateOrder)
    {
    DataContainer<ContentSetItemCPtr> contentSet = content.GetContentSet();
    ASSERT_EQ(instances.size(), contentSet.GetSize());
    for (ContentSetItemCPtr const& item : contentSet)
        {
        InstanceInputAndResult instance;
        if (validateOrder)
            {
            instance = instances[0];
            instances.erase(instances.begin());
            }
        else
            {
            bvector<ECClassInstanceKey> const& itemKeys = item->GetKeys();
            auto iter = std::find_if(instances.begin(), instances.end(), [&itemKeys](auto const& instance)
                {
                return ContainerHelpers::Contains(itemKeys, [&instance](auto const& key)
                    {
                    return key.GetClass()->GetId() == instance.m_result->GetClass().GetId()
                        && key.GetId().ToString() == instance.m_result->GetInstanceId();
                    });
                });
            ASSERT_TRUE(instances.end() != iter);
            instance = *iter;
            instances.erase(iter);
            }

        EXPECT_EQ(instance.m_inputs.size(), item->GetInputKeys().size());
        for (auto const& expectedInputInstance : instance.m_inputs)
            {
            EXPECT_TRUE(ContainerHelpers::Contains(item->GetInputKeys(), [&expectedInputInstance](auto const& key)
                {
                return key.GetClass()->GetId() == expectedInputInstance->GetClass().GetId()
                    && key.GetId().ToString() == expectedInputInstance->GetInstanceId();
                }));
            }

        if (instance.m_result)
            ValidateContentSetItem(*instance.m_result, *item, content.GetDescriptor());
        }
    }

/*---------------------------------------------------------------------------------**//**
* The function requests nodes in pages and confirms the page is correct. The pages are
* requested in two ways:
* - { start: i, size: 0 } means we're getting nodes from `i` to the end.
* - { start: 1, size: 1 } means we're getting one node at position `i`.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateNodesPagination(std::function<NodesResponse(PageOptionsCR)> getter, bvector<NavNodeCPtr> const& expectedNodes)
    {
    for (size_t i = 1; i < expectedNodes.size(); ++i)
        {
        auto responseNoSize = getter(PageOptions(i, 0));
        auto const& resultNoSize = *responseNoSize;
        EXPECT_EQ(expectedNodes.size() - i, resultNoSize.GetSize());
        for (size_t j = i; j < expectedNodes.size(); ++j)
            EXPECT_EQ(*expectedNodes[j]->GetKey(), *resultNoSize.Get(j - i)->GetKey());

        auto responseOneSize = getter(PageOptions(i, 1));
        auto const& resultOneSize = *responseOneSize;
        EXPECT_EQ(1, resultOneSize.GetSize());
        EXPECT_EQ(*expectedNodes[i]->GetKey(), *resultOneSize.Get(0)->GetKey());
        }
    }

/*---------------------------------------------------------------------------------**//**
* The first time we request nodes, we get them straight from the creator data providers.
* The second time - we get them from cache providers. To make sure both types of providers
* return the same result, nodes should be requested through this helper function.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesContainer RulesEngineTestHelpers::GetValidatedNodes(std::function<NodesResponse(PageOptionsCR)> nodesGetter, std::function<NodesCountResponse()> countGetter)
    {
    // get the count
    Nullable<size_t> count;
    if (countGetter)
        {
        NodesCountResponse nodesCountResponse = countGetter();
        count = *nodesCountResponse;
        }

    // allow the test to verify the nodes
    NodesResponse initialNodesResponse = nodesGetter(PageOptions());
    NavNodesContainer const& initialNodes = *initialNodesResponse;

    if (count.IsValid())
        {
        // ensure nodes count matches the count we got initially
        EXPECT_EQ(count.Value(), initialNodes.GetSize())
            << "Results of initial request (generator) should match result of count request";
        }

    // additionally, make sure another get, which returns nodes from cache,
    // returns the same result
    NodesResponse cachedNodesResponse = nodesGetter(PageOptions());
    NavNodesContainer const& cachedNodes = *cachedNodesResponse;
    EXPECT_EQ(initialNodes.GetSize(), cachedNodes.GetSize())
        << "Results of initial request (generator) should match results of second request (cached)";
    if (initialNodes.GetSize() != cachedNodes.GetSize())
        return initialNodes;
    for (size_t i = 0; i < initialNodes.GetSize(); ++i)
        {
        EXPECT_TRUE(cachedNodes[i]->Equals(*initialNodes[i]))
            << "Nodes from initial request should match nodes from cached request";
        }

    // ensure that requesting nodes in pages also return the same result
    bvector<NavNodeCPtr> nodes;
    for (auto node : cachedNodes)
        nodes.push_back(node);
    ValidateNodesPagination(nodesGetter, nodes);

    // finally, ensure the count hasn't changed
    if (countGetter)
        {
        NodesCountResponse countAfterResponse = countGetter();
        size_t countAfter = *countAfterResponse;
        EXPECT_EQ(count, countAfter) << "Nodes count after getting nodes should match nodes count before the request";
        }

    // return nodes for further validation in the test itself
    return cachedNodes;
    }

/*---------------------------------------------------------------------------------**//**
* The first time we request nodes, we get them straight from the creator data providers.
* The second time - we get them from cache providers. To make sure both types of providers
* return the same result, nodes should be requested through this helper function.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesContainer RulesEngineTestHelpers::GetValidatedNodes(std::function<NodesResponse()> getter)
    {
    // allow the test to verify the nodes
    NodesResponse initialNodesResponse = getter();
    NavNodesContainer const& initialNodes = *initialNodesResponse;

    // additionally, make sure another get, which returns nodes from cache,
    // returns the same result
    NodesResponse cachedNodesResponse = getter();
    NavNodesContainer cachedNodes = *cachedNodesResponse;

    EXPECT_EQ(initialNodes.GetSize(), cachedNodes.GetSize())
        << "Results of initial request (generator) should match results of second request (cached)";
    if (initialNodes.GetSize() != cachedNodes.GetSize())
        return initialNodes;

    for (size_t i = 0; i < initialNodes.GetSize(); ++i)
        {
        EXPECT_TRUE(cachedNodes[i]->Equals(*initialNodes[i]))
            << "Nodes from initial request should match nodes from cached request";
        }

    return cachedNodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECInstanceKey> ReadNodeInstanceKeys(ECDbCR connection, NavNodeCR node)
    {
    auto query = node.GetKey()->GetInstanceKeysSelectQuery();
    ECSqlStatement stmt;
    if (!stmt.Prepare(connection, query->GetQueryString().c_str()).IsSuccess())
        {
        BeAssert(false);
        return bset<ECInstanceKey>();
        }

    if (SUCCESS != query->BindValues(stmt))
        {
        BeAssert(false);
        return bset<ECInstanceKey>();
        }

    bset<ECInstanceKey> keys;
    while (BE_SQLITE_ROW == stmt.Step())
        keys.insert(ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)));
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECInstanceKey> GetECInstanceNodeKeys(NavNodeCR node)
    {
    return ContainerHelpers::TransformContainer<bset<ECInstanceKey>>(node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys(), [](auto const& k)
        {
        return ECInstanceKey(k.GetClass()->GetId(), k.GetId());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyInstanceKeysMatch(bvector<RefCountedPtr<IECInstance const>> const& expected, bset<ECInstanceKey> const& actual)
    {
    ASSERT_EQ(expected.size(), actual.size());
    for (auto const& i : expected)
        {
        ECInstanceId instanceId;
        ECInstanceId::FromString(instanceId, i->GetInstanceId().c_str());
        auto iter = actual.find(ECInstanceKey(i->GetClass().GetId(), instanceId));
        if (actual.end() == iter)
            {
            ADD_FAILURE() << "ECInstanceKey of expected ECInstance was not found in actual keys list";
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateNodeInstances(ECDbCR db, NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& instances)
    {
    auto nodeInstanceKeys = ReadNodeInstanceKeys(db, static_cast<NavNodeCR>(node));
    VerifyInstanceKeysMatch(instances, nodeInstanceKeys);

    if (node.GetKey()->AsECInstanceNodeKey())
        VerifyInstanceKeysMatch(instances, GetECInstanceNodeKeys(node));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateNodeInstances(INodeInstanceKeysProvider const& instanceKeysProvider, NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& instances)
    {
    bset<ECInstanceKey> nodeInstanceKeys;
    instanceKeysProvider.IterateInstanceKeys(node, [&nodeInstanceKeys](ECInstanceKeyCR k)
        {
        nodeInstanceKeys.insert(k);
        return true;
        });
    VerifyInstanceKeysMatch(instances, nodeInstanceKeys);

    if (node.GetKey()->AsECInstanceNodeKey())
        VerifyInstanceKeysMatch(instances, GetECInstanceNodeKeys(node));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ValidateNodeGroupedValues(NavNodeCR node, bvector<ECValue> const& groupedValues)
    {
    auto key = node.GetKey()->AsECPropertyGroupingNodeKey();
    RapidJsonValueCR groupingValuesArray = key->GetGroupingValuesArray();
    ASSERT_TRUE(groupingValuesArray.IsArray());
    ASSERT_EQ(groupingValuesArray.Size(), groupedValues.size());

    bvector<rapidjson::Document> expectedGroupedValues = ContainerHelpers::TransformContainer<bvector<rapidjson::Document>>(groupedValues, [](ECValueCR value)
        {
        if (value.IsNavigation())
            {
            rapidjson::Document json;
            json.SetUint(value.GetNavigationInfo().GetId<BeInt64Id>().GetValue());
            return json;
            }
        if (value.IsPrimitive() && value.IsDateTime())
            {
            rapidjson::Document json;
            double julianDays;
            if (SUCCESS == value.GetDateTime().ToJulianDay(julianDays))
                json.SetDouble(julianDays);
            return json;
            }
        return ValueHelpers::GetJsonFromECValue(value, nullptr);
        });

    for (rapidjson::SizeType i = 0; i < groupingValuesArray.Size(); ++i)
        {
        RapidJsonValueCR groupedValue = groupingValuesArray[i];
        EXPECT_TRUE(ContainerHelpers::Contains(expectedGroupedValues, [&groupedValue](auto const& expected)
            {
            return groupedValue == expected;
            }));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& RulesEngineTestHelpers::AddField(ContentDescriptorR descriptor, ContentDescriptor::Field& field)
    {
    field.SetUniqueName(field.CreateName());
    descriptor.AddRootField(field);
    return *descriptor.GetAllFields().back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& RulesEngineTestHelpers::AddField(ContentDescriptorR descriptor, ECClassCR primaryClass,
    ContentDescriptor::Property prop, IPropertyCategorySupplierR categorySupplier)
    {
    return AddField(descriptor, *new ContentDescriptor::ECPropertiesField(categorySupplier.CreatePropertyCategory(prop.GetProperty()), prop));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::CacheNode(IHierarchyCacheR cache, NavNodeR node, BeGuidCR parentNodeId)
    {
    NavNodeExtendedData extendedData(node);
    extendedData.AddVirtualParentId(parentNodeId);
    auto virtualParentIds = extendedData.GetVirtualParentIds();
    Utf8String rulesetId = extendedData.HasRulesetId() ? extendedData.GetRulesetId() : "";
    HierarchyLevelIdentifier hlInfo(extendedData.GetConnectionId(), rulesetId.c_str(),
        parentNodeId, !virtualParentIds.empty() ? virtualParentIds.front() : BeGuid());
    hlInfo.SetId(cache.FindHierarchyLevelId(extendedData.GetConnectionId(), rulesetId.c_str(), !virtualParentIds.empty() ? virtualParentIds.front() : BeGuid(), BeGuid()));
    if (!hlInfo.IsValid())
        cache.Cache(hlInfo);
    DataSourceIdentifier identifier(hlInfo.GetId(), {0}, "");
    DataSourceInfo dsInfo = cache.FindDataSource(identifier, RulesetVariables());
    if (!dsInfo.GetIdentifier().IsValid())
        {
        dsInfo = DataSourceInfo(identifier, RulesetVariables(), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
        cache.Cache(dsInfo);
        }
    cache.Cache(node, dsInfo.GetIdentifier(), 0, NodeVisibility::Visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECEntityClassP> RulesEngineTestHelpers::CreateNDerivedClasses(ECSchemaR schema, ECEntityClassCR baseClass, int numberOfChildClasses)
    {
    bvector<ECEntityClassP> classes;
    for (int i = 0; i < numberOfChildClasses; ++i)
        {
        ECEntityClassP ecClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(ecClass, Utf8PrintfString("Class%d", i + 1)));
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->AddBaseClass(baseClass));
        classes.push_back(ecClass);
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTestHelpers::ImportSchema(ECDbR ecdb, std::function<void(ECSchemaR)> const& schemaBuilder)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, BeTest::GetNameOfCurrentTest(), Utf8PrintfString("alias_%s", BeTest::GetNameOfCurrentTest()), 0, 0, 0));
    schema->AddReferencedSchema(*const_cast<ECSchemaP>(ecdb.Schemas().GetSchema("ECDbMap")));
    schemaBuilder(*schema);
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({schema.get()}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RulesEngineTestHelpers::CreateClassNamesList(bvector<ECClassCP> const& classes)
    {
    Utf8String list;
    bset<ECSchemaCP> usedSchemas;
    bool firstClass = true;
    for (ECClassCP ecClass : classes)
        {
        if (usedSchemas.end() == usedSchemas.find(&ecClass->GetSchema()))
            {
            if (!usedSchemas.empty())
                list.append(";");
            list.append(ecClass->GetSchema().GetName()).append(":");
            usedSchemas.insert(&ecClass->GetSchema());
            firstClass = true;
            }
        if (!firstClass)
            list.append(",");
        list.append(ecClass->GetName());
        firstClass = false;
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StubPropertyFormatter::_GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue, ECPresentation::UnitSystem unitSystem) const
    {
    formattedValue.clear();
    if (!ecValue.IsPrimitive())
        {
        EXPECT_TRUE(false);
        return ERROR;
        }
    Utf8String value = ecValue.ToString();
    if (!value.empty())
        {
        formattedValue = Utf8String().append("_").append(value).append("_");
        if (m_addUnitSystemSuffix)
            {
            formattedValue.append("[");
            switch (unitSystem)
                {
                case ECPresentation::UnitSystem::BritishImperial: formattedValue.append("British Imperial"); break;
                case ECPresentation::UnitSystem::Metric: formattedValue.append("Metric"); break;
                case ECPresentation::UnitSystem::UsCustomary: formattedValue.append("US Customary"); break;
                case ECPresentation::UnitSystem::UsSurvey: formattedValue.append("US Survey"); break;
                default: formattedValue.append("Default"); break;
                }
            formattedValue.append("]");
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StubPropertyFormatter::_GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR, RelatedClassPathCR, RelationshipMeaning) const
    {
    formattedLabel = Utf8String().append("_").append(ecProperty.GetDisplayLabel()).append("_");
    return SUCCESS;
    }
