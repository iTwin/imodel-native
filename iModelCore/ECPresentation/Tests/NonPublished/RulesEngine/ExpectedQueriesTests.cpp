/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ExpectedQueries.h"
#include "TestHelpers.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomFunctions.h"
#include "../../../Source/RulesDriven/RulesEngine/ECSchemaHelper.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryBuilder.h"
#include <Logging/bentleylogging.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpectedQueries& ExpectedQueries::GetInstance(BeTest::Host& host)
    {
    static ExpectedQueries* s_instance = nullptr;
    if (nullptr == s_instance)
        s_instance = new ExpectedQueries(host);
    return *s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<Utf8String, Utf8String>& GetRegisteredSchemaXmls()
    {
    static bmap<Utf8String, Utf8String> s_registeredSchemaXmls;
    return s_registeredSchemaXmls;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::RegisterSchemaXml(Utf8String name, Utf8String schemaXml) {GetRegisteredSchemaXmls()[name] = schemaXml;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::RegisterQuery(Utf8CP name, NavigationQuery const& query)
    {
    BeAssert(m_navigationQueries.end() == m_navigationQueries.find(name));
    m_navigationQueries[name] = &query;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::RegisterQuery(Utf8CP name, ContentQuery const& query)
    {
    BeAssert(m_contentQueries.end() == m_contentQueries.find(name));
    m_contentQueries[name] = &query;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryCPtr ExpectedQueries::GetNavigationQuery(Utf8CP name, ChildNodeSpecificationCR spec) const
    {
    auto iter = m_navigationQueries.find(name);
    if (m_navigationQueries.end() != iter)
        {
        NavigationQueryPtr query = iter->second->Clone();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRulesetId("NavigationQueryBuilderTests");
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash(spec.GetHash());
        query->GetResultParametersR().SetSpecification(&spec);
        return query;
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryCPtr ExpectedQueries::GetContentQuery(Utf8CP name) const
    {
    auto iter = m_contentQueries.find(name);
    if (m_contentQueries.end() != iter)
        {
        ContentQueryPtr query = iter->second->Clone();
        return query;
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String, NavigationQueryCPtr> const& ExpectedQueries::GetNavigationQueries() const {return m_navigationQueries;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String, ContentQueryCPtr> const& ExpectedQueries::GetContentQueries() const {return m_contentQueries;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpectedQueries::~ExpectedQueries()
    {
    delete m_schemaHelper;
    for (PropertyGroupCP spec : m_propertyGroupsToDelete)
        delete spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ExpectedQueries::GetECClassP(Utf8CP schemaName, Utf8CP className)
    {
    return const_cast<ECClassP>(m_schemaHelper->GetECClass(schemaName, className));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ExpectedQueries::GetECClass(Utf8CP schemaName, Utf8CP className) {return GetECClassP(schemaName, className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> ExpectedQueries::GetECClasses(Utf8CP schemaName)
    {
    bvector<ECClassCP> classes;
    bset<ECClassCP> set;
    ECSchemaCP schema = GetDb().Schemas().GetSchema(schemaName);
    if (nullptr != schema)
        {
        ECClassContainerCR classContainer = schema->GetClasses();
        for (ECClassCP ecClass : classContainer)
            {
            if (set.end() != set.find(ecClass))
                continue;

            classes.push_back(ecClass);
            set.insert(ecClass);
            }
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ExpectedQueries::GetEmptyContentDescriptor(Utf8CP displayType = ContentDisplayType::Undefined) const
    {
    RulesDrivenECPresentationManager::ContentOptions options("", "test locale");
    return ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create(), displayType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& ExpectedQueries::AddField(ContentDescriptorR descriptor, ContentDescriptor::Category category, ContentDescriptor::Property prop)
    {
    descriptor.AddField(new ContentDescriptor::ECPropertiesField(category, prop));
    return *descriptor.GetAllFields().back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Property CreateProperty(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty)
    {
    return ContentDescriptor::Property(prefix, propertyClass, ecProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Property CreateProperty(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty, RelatedClassPath relatedClassPath, RelationshipMeaning relationshipMeaning)
    {
    ContentDescriptor::Property p = CreateProperty(prefix, propertyClass, ecProperty);
    p.SetIsRelated(relatedClassPath, relationshipMeaning);
    return p;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Property CreateProperty(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty, RelatedClass relatedClass, RelationshipMeaning relationshipMeaning)
    {
    RelatedClassPath path;
    path.push_back(relatedClass);
    return CreateProperty(prefix, propertyClass, ecProperty, path, relationshipMeaning);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Category CreateCategory(ECClassCR ecClass)
    {
    return ContentDescriptor::Category(ecClass.GetName(), ecClass.GetDisplayLabel(), ecClass.GetDescription(), 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::PrepareSchemaContext()
    {
    m_project.Create("ExpectedQueries", "RulesEngineTest.01.00.ecschema.xml");
    m_connection = m_connections.NotifyConnectionOpened(m_project.GetECDb());

    bvector<ECSchemaPtr> schemas;
    schemas.resize(6);
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_project.GetECDb().GetSchemaLocater());
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemas[0], SCHEMA_BASIC_1, *schemaReadContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemas[1], SCHEMA_BASIC_2, *schemaReadContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemas[2], SCHEMA_BASIC_3, *schemaReadContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemas[3], SCHEMA_BASIC_4, *schemaReadContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemas[4], SCHEMA_COMPLEX_1, *schemaReadContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemas[5], SCHEMA_COMPLEX_3, *schemaReadContext));

    for (auto pair : GetRegisteredSchemaXmls())
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

    bvector<ECSchemaCP> importSchemas;
    importSchemas.resize(schemas.size());
    std::transform(schemas.begin(), schemas.end(), importSchemas.begin(), [](ECSchemaPtr const& schema){return schema.get();});

    ASSERT_TRUE(SUCCESS == m_project.GetECDb().Schemas().ImportSchemas(importSchemas));
    m_project.GetECDb().SaveChanges();

    m_connection = m_connections.NotifyConnectionOpened(m_project.GetECDb());
    m_schemaHelper = new ECSchemaHelper(*m_connection, nullptr, nullptr, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::RegisterExpectedQueries()
    {
    Localization::Init();
    static Utf8PrintfString ecInstanceNodesQuerySortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName);
    static Utf8PrintfString ecClassGroupingNodesQuerySortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);
    static Utf8PrintfString baseClassGroupingNodesQuerySortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, BaseECClassGroupingNodesQueryContract::DisplayLabelFieldName);
    static Utf8PrintfString labelGroupingQuerySortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName);

    static Utf8PrintfString ecClassGroupingNodesOrderByClause("%s", ecClassGroupingNodesQuerySortedDisplayLabel.c_str());

    static Utf8PrintfString baseClassGroupingNodesOrderByClause("%s", baseClassGroupingNodesQuerySortedDisplayLabel.c_str());

    ECEntityClassR b1_Class1A = *GetECClassP("Basic1", "Class1A")->GetEntityClassP();
    ECEntityClassR b1_Class1B = *GetECClassP("Basic1", "Class1B")->GetEntityClassP();
    ECEntityClassR b1_Class2 = *GetECClassP("Basic1", "Class2")->GetEntityClassP();
    
    ECEntityClassR b2_Class2 = *GetECClassP("Basic2", "Class2")->GetEntityClassP();

    ECEntityClassR b3_Class3 = *GetECClassP("Basic3", "Class3")->GetEntityClassP();

    ECEntityClassR b4_ClassA = *GetECClassP("Basic4", "ClassA")->GetEntityClassP();
    ECEntityClassR b4_ClassB = *GetECClassP("Basic4", "ClassB")->GetEntityClassP();
    ECEntityClassR b4_ClassC = *GetECClassP("Basic4", "ClassC")->GetEntityClassP();
    ECRelationshipClassR b4_ClassBHasClassC = *GetECClassP("Basic4", "ClassBHasClassC")->GetRelationshipClassP();

    ECEntityClassR sc_Class1 = *GetECClassP("SchemaComplex", "Class1")->GetEntityClassP();
    //ECEntityClassR sc_BaseOf2and3 = *GetECClassP("SchemaComplex", "BaseOf2And3")->GetEntityClassP();
    ECEntityClassR sc_Class2 = *GetECClassP("SchemaComplex", "Class2")->GetEntityClassP();
    ECEntityClassR sc_Class3 = *GetECClassP("SchemaComplex", "Class3")->GetEntityClassP();
    ECRelationshipClassR sc_Class1HasClass2And3 = *GetECClassP("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassP();
    ECEntityClassR sc3_GroupingClass = *GetECClassP("SchemaComplex3", "GroupingClass")->GetEntityClassP();
    ECEntityClassR sc3_ChildClassWithNavigationProperty = *GetECClassP("SchemaComplex3", "ChildClassWithNavigationProperty")->GetEntityClassP();
    ECRelationshipClassR sc3_NavigationGrouping = *GetECClassP("SchemaComplex3", "NavigationGrouping")->GetRelationshipClassP();

    ECEntityClassR ret_ClassA = *GetECClassP("RulesEngineTest", "ClassA")->GetEntityClassP();
    //ECEntityClassR ret_ClassA2Base = *GetECClassP("RulesEngineTest", "ClassA2Base")->GetEntityClassP();
    //ECEntityClassR ret_BaseOfBAndC = *GetECClassP("RulesEngineTest", "BaseOfBAndC")->GetEntityClassP();
    ECEntityClassR ret_ClassB = *GetECClassP("RulesEngineTest", "ClassB")->GetEntityClassP();
    //ECEntityClassR ret_ClassB2 = *GetECClassP("RulesEngineTest", "ClassB2")->GetEntityClassP();
    ECEntityClassR ret_ClassC = *GetECClassP("RulesEngineTest", "ClassC")->GetEntityClassP();
    ECRelationshipClassR ret_ClassAHasBAndC = *GetECClassP("RulesEngineTest", "ClassAHasBAndC")->GetRelationshipClassP();
    ECEntityClassR ret_ClassD = *GetECClassP("RulesEngineTest", "ClassD")->GetEntityClassP();
    ECEntityClassR ret_ClassE = *GetECClassP("RulesEngineTest", "ClassE")->GetEntityClassP();
    ECRelationshipClassR ret_ClassDHasClassE = *GetECClassP("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassP();
    ECRelationshipClassR ret_ClassDReferencesClassE = *GetECClassP("RulesEngineTest", "ClassDReferencesClassE")->GetRelationshipClassP();
    ECEntityClassR ret_ClassF = *GetECClassP("RulesEngineTest", "ClassF")->GetEntityClassP();
    ECEntityClassR ret_ClassG = *GetECClassP("RulesEngineTest", "ClassG")->GetEntityClassP();
    ECEntityClassR ret_ClassH = *GetECClassP("RulesEngineTest", "ClassH")->GetEntityClassP();
    //ECEntityClassR ret_ClassI = *GetECClassP("RulesEngineTest", "ClassI")->GetEntityClassP();
    //ECEntityClassR ret_ClassJ = *GetECClassP("RulesEngineTest", "ClassJ")->GetEntityClassP();
    //ECEntityClassR ret_ClassQ = *GetECClassP("RulesEngineTest", "ClassQ")->GetEntityClassP();
    //ECEntityClassR ret_ClassR = *GetECClassP("RulesEngineTest", "ClassR")->GetEntityClassP();
    //ECEntityClassR ret_ClassS = *GetECClassP("RulesEngineTest", "ClassS")->GetEntityClassP();
    //ECEntityClassR ret_ClassT = *GetECClassP("RulesEngineTest", "ClassT")->GetEntityClassP();
    //ECEntityClassR ret_ClassU = *GetECClassP("RulesEngineTest", "ClassU")->GetEntityClassP();

    ECEntityClassR ret_Widget = *GetECClassP("RulesEngineTest", "Widget")->GetEntityClassP();
    ECEntityClassR ret_Gadget = *GetECClassP("RulesEngineTest", "Gadget")->GetEntityClassP();
    ECEntityClassR ret_Sprocket = *GetECClassP("RulesEngineTest", "Sprocket")->GetEntityClassP();
    ECRelationshipClassR ret_WidgetHasGadget = *GetECClassP("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassP();
    ECRelationshipClassR ret_WidgetHasGadgets = *GetECClassP("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassP();
    ECRelationshipClassR ret_WidgetsHaveGadgets = *GetECClassP("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassP();
    ECRelationshipClassR ret_WidgetsHaveGadgets2 = *GetECClassP("RulesEngineTest", "WidgetsHaveGadgets2")->GetRelationshipClassP();
    ECRelationshipClassR ret_GadgetHasSprockets = *GetECClassP("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassP();
    ECRelationshipClassR ret_GadgetHasSprocket = *GetECClassP("RulesEngineTest", "GadgetHasSprocket")->GetRelationshipClassP();

    ContentDescriptor::Field* field = nullptr;

    // AllInstanceNodes_SupportedSchemas_NoSpaces
        {
        ECClassSet expectedClasses;
        expectedClasses[&b1_Class1A] = true;
        expectedClasses[&b1_Class1B] = true;
        expectedClasses[&b1_Class2] = true;
        expectedClasses[&b3_Class3] = true;
        
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectAll();
        query->From(*RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(expectedClasses, "this"));
        query->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_SupportedSchemas_NoSpaces", *query);
        }

    // AllInstanceNodes_SupportedSchemas_WithSpaces
        {
        ECClassSet expectedClasses;
        expectedClasses[&b1_Class1A] = true;
        expectedClasses[&b1_Class1B] = true;
        expectedClasses[&b1_Class2] = true;
        expectedClasses[&b3_Class3] = true;
    
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectAll();
        query->From(*RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(expectedClasses, "this"));
        query->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_SupportedSchemas_WithSpaces", *query);
        }

    // AllInstanceNodes_SupportedSchemas_Excluded
        {
        ECClassSet classes;
        classes[&b2_Class2] = true;
        
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(classes, "this"));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_SupportedSchemas_Excluded", *expected);
        }

    // AllInstanceNodes_SupportedSchemas_UsedFromRulesetIfNotSpecified
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[&b1_Class1A] = true;
        expectedQueryClasses[&b1_Class1B] = true;
        expectedQueryClasses[&b1_Class2] = true;

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(expectedQueryClasses, "this"));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_SupportedSchemas_UsedFromRulesetIfNotSpecified", *expected);
        }

    // AllInstanceNodes_CheckNotPolymorphic
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[&b4_ClassA] = true;

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(expectedQueryClasses, "this"));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("AllInstanceNodes_CheckNotPolymorphic", *expected);
        }

    // AllInstanceNodes_GroupByClass
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectAll();
        nested->From(*RulesEngineTestHelpers::CreateQuery(*contract, GetECClasses("Basic1"), true, "this"));
        nested->GroupByContract(*contract);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nested);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);
        
        RegisterQuery("AllInstanceNodes_GroupByClass", *expected);
        }

    // AllInstanceNodes_GroupByClass_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b1_Class1A, false, "this"));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_GroupByClass_ChildrenQuery", *expected);
        }

    // AllInstanceNodes_GroupByLabel
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[&b1_Class1A] = true;
        expectedQueryClasses[&b1_Class1B] = true;
        expectedQueryClasses[&b1_Class2] = true;

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*RulesEngineTestHelpers::CreateLabelGroupingNodesQueryForClasses(expectedQueryClasses, "this"));
        expected->GroupByContract(*DisplayLabelGroupingNodesQueryContract::Create(nullptr));
        expected->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        RegisterQuery("AllInstanceNodes_GroupByLabel", *expected);
        }

    // AllInstanceNodes_GroupByLabel_ChildrenQuery
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[&b1_Class1A] = true;
        expectedQueryClasses[&b1_Class1B] = true;
        expectedQueryClasses[&b1_Class2] = true;

        NavigationQueryPtr nestedQueries = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(expectedQueryClasses, "this", [](ComplexNavigationQuery& query)
            {
            query.Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)1))});
            query.Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[Name], '[]') = ?", {new BoundQueryECValue(ECValue("MyLabel"))});
            });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQueries);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_GroupByLabel_ChildrenQuery", *expected);
        }

    // AllInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&b1_Class1A);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b1_Class1A, false, "this"));
        expected->GroupByContract(*contract);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        RegisterQuery("AllInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery", *expected);
        }

    // AllInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)1))});
        nested->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[Name], '[]') = ?", {new BoundQueryECValue(ECValue("Label Grouping Node"))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nested);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery", *expected);
        }

    // AllInstanceNodes_RecursiveNodeRelationships
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b3_Class3, true, "this"));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("AllInstanceNodes_RecursiveNodeRelationships", *expected);
        }

    // AllInstanceNodes_InstanceLabelOverride_AppliedByPriority
        {
        Utf8CP queryName = "AllInstanceNodes_InstanceLabelOverride_AppliedByPriority";
        ECClassCR class1 = *GetECClass(queryName, "Class1");
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&class1, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("Code")});   
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(class1, true, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*query);
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery(queryName, *sorted);
        }

    // AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels
        {
        Utf8CP queryName = "AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels";
        ECClassCR class1 = *GetECClass(queryName, "Class1");
        NavigationQueryContractPtr class1Contract = ECInstanceNodesQueryContract::Create(&class1, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("Code")});
        ComplexNavigationQueryPtr class1Query = ComplexNavigationQuery::Create();
        class1Query->SelectContract(*class1Contract, "this");
        class1Query->From(class1, true, "this");

        ECClassCR class2 = *GetECClass(queryName, "Class2");
        NavigationQueryContractPtr class2Contract = ECInstanceNodesQueryContract::Create(&class2);   
        ComplexNavigationQueryPtr class2Query = ComplexNavigationQuery::Create();
        class2Query->SelectContract(*class2Contract, "this");
        class2Query->From(class2, true, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*UnionNavigationQuery::Create(*class1Query, *class2Query));
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery(queryName, *sorted);
        }

    // AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
        
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this");
        nestedQuery1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this");
        nestedQuery2->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Gadget, true, "this");
        nestedQuery3->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Gadget, true, "this");
        nestedQuery4->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());

        RegisterQuery("AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection", *expected);
        }

    // AllRelatedInstanceNodes_NoGrouping_BackwardRelationDirection
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());

        RegisterQuery("AllRelatedInstanceNodes_NoGrouping_BackwardRelationDirection", *expected);
        }

    // AllRelatedInstanceNodes_NoGrouping_BothDirections
        {
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery5->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
    
        ComplexNavigationQueryPtr nestedQuery6 = ComplexNavigationQuery::Create();
        nestedQuery6->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery6->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery6->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5), *nestedQuery6));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_NoGrouping_BothDirections", *expected);
        }

    // AllRelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID")});
        
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this");
        nestedQuery1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this");
        nestedQuery2->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Gadget, true, "this");
        nestedQuery3->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Gadget, true, "this");
        nestedQuery4->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());

        RegisterQuery("AllRelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority", *expected);
        }

    // AllRelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels
        {
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")}), "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0",  true, false));
        nestedQuery5->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
    
        ComplexNavigationQueryPtr nestedQuery6 = ComplexNavigationQuery::Create();
        nestedQuery6->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")}), "this");
        nestedQuery6->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery6->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5), *nestedQuery6));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", *expected);
        }

    // RelatedInstanceNodes_SkipOneRelatedLevel_WidgetToSprocket
        {
        NavigationQueryContractPtr contract11 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery11 = ComplexNavigationQuery::Create();
        nestedQuery11->SelectContract(*contract11, "this");
        nestedQuery11->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath11;
        relationshipPath11.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"));
        relationshipPath11.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0"));
        nestedQuery11->Join(relationshipPath11, false);
        nestedQuery11->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr groupedQuery11 = ComplexNavigationQuery::Create();
        groupedQuery11->SelectAll();
        groupedQuery11->From(*nestedQuery11);
        groupedQuery11->GroupByContract(*contract11);
        contract11->SetRelationshipPath(relationshipPath11);

        NavigationQueryContractPtr contract12 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery12 = ComplexNavigationQuery::Create();
        nestedQuery12->SelectContract(*contract12, "this");
        nestedQuery12->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath12;
        relationshipPath12.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"));
        relationshipPath12.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0"));
        nestedQuery12->Join(relationshipPath12, false);
        nestedQuery12->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr groupedQuery12 = ComplexNavigationQuery::Create();
        groupedQuery12->SelectAll();
        groupedQuery12->From(*nestedQuery12);
        groupedQuery12->GroupByContract(*contract12);
        contract12->SetRelationshipPath(relationshipPath12);

        NavigationQueryContractPtr contract13 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery13 = ComplexNavigationQuery::Create();
        nestedQuery13->SelectContract(*contract13, "this");
        nestedQuery13->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath13;
        relationshipPath13.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"));
        relationshipPath13.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0"));
        nestedQuery13->Join(relationshipPath13, false);
        nestedQuery13->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr groupedQuery13 = ComplexNavigationQuery::Create();
        groupedQuery13->SelectAll();
        groupedQuery13->From(*nestedQuery13);
        groupedQuery13->GroupByContract(*contract13);
        contract13->SetRelationshipPath(relationshipPath13);

        NavigationQueryContractPtr contract14 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery14 = ComplexNavigationQuery::Create();
        nestedQuery14->SelectContract(*contract14, "this");
        nestedQuery14->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath14;
        relationshipPath14.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"));
        relationshipPath14.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0"));
        nestedQuery14->Join(relationshipPath14, false);
        nestedQuery14->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr groupedQuery14 = ComplexNavigationQuery::Create();
        groupedQuery14->SelectAll();
        groupedQuery14->From(*nestedQuery14);
        groupedQuery14->GroupByContract(*contract14);
        contract14->SetRelationshipPath(relationshipPath14);
        
        NavigationQueryContractPtr contract21 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery21 = ComplexNavigationQuery::Create();
        nestedQuery21->SelectContract(*contract21, "this");
        nestedQuery21->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath21;
        relationshipPath21.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath21.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0"));
        nestedQuery21->Join(relationshipPath21, false);
        nestedQuery21->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery21 = ComplexNavigationQuery::Create();
        groupedQuery21->SelectAll();
        groupedQuery21->From(*nestedQuery21);
        groupedQuery21->GroupByContract(*contract21);
        contract21->SetRelationshipPath(relationshipPath21);

        NavigationQueryContractPtr contract22 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery22 = ComplexNavigationQuery::Create();
        nestedQuery22->SelectContract(*contract22, "this");
        nestedQuery22->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath22;
        relationshipPath22.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath22.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0"));
        nestedQuery22->Join(relationshipPath22, false);
        nestedQuery22->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery22 = ComplexNavigationQuery::Create();
        groupedQuery22->SelectAll();
        groupedQuery22->From(*nestedQuery22);
        groupedQuery22->GroupByContract(*contract22);
        contract22->SetRelationshipPath(relationshipPath22);

        NavigationQueryContractPtr contract23 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery23 = ComplexNavigationQuery::Create();
        nestedQuery23->SelectContract(*contract23, "this");
        nestedQuery23->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath23;
        relationshipPath23.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath23.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0"));
        nestedQuery23->Join(relationshipPath23, false);        
        nestedQuery23->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery23 = ComplexNavigationQuery::Create();
        groupedQuery23->SelectAll();
        groupedQuery23->From(*nestedQuery23);
        groupedQuery23->GroupByContract(*contract23);
        contract23->SetRelationshipPath(relationshipPath23);

        NavigationQueryContractPtr contract24 = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        ComplexNavigationQueryPtr nestedQuery24 = ComplexNavigationQuery::Create();
        nestedQuery24->SelectContract(*contract24, "this");
        nestedQuery24->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath24;
        relationshipPath24.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath24.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0"));
        nestedQuery24->Join(relationshipPath24, false);        
        nestedQuery24->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery24 = ComplexNavigationQuery::Create();
        groupedQuery24->SelectAll();
        groupedQuery24->From(*nestedQuery24);
        groupedQuery24->GroupByContract(*contract24);
        contract24->SetRelationshipPath(relationshipPath24);

        UnionNavigationQueryPtr expected = UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*groupedQuery11, *groupedQuery12), *groupedQuery13), *groupedQuery14), *groupedQuery21), *groupedQuery22), *groupedQuery23), *groupedQuery24);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_SkipOneRelatedLevel_WidgetToSprocket", *expected);
        }

    // RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
        
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath1;
        relationshipPath1.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath1.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0"));
        nestedQuery1->Join(relationshipPath1, false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr groupedQuery1 = ComplexNavigationQuery::Create();
        groupedQuery1->SelectAll();
        groupedQuery1->From(*nestedQuery1);
        groupedQuery1->GroupByContract(*contract);

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath2;
        relationshipPath2.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath2.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0"));
        nestedQuery2->Join(relationshipPath2, false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery2 = ComplexNavigationQuery::Create();
        groupedQuery2->SelectAll();
        groupedQuery2->From(*nestedQuery2);
        groupedQuery2->GroupByContract(*contract);

        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath3;
        relationshipPath3.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath3.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0"));
        nestedQuery3->Join(relationshipPath3, false);        
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery3 = ComplexNavigationQuery::Create();
        groupedQuery3->SelectAll();
        groupedQuery3->From(*nestedQuery3);
        groupedQuery3->GroupByContract(*contract);

        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Sprocket, true, "this");
        RelatedClassPath relationshipPath4;
        relationshipPath4.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath4.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0"));
        nestedQuery4->Join(relationshipPath4, false);        
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr groupedQuery4 = ComplexNavigationQuery::Create();
        groupedQuery4->SelectAll();
        groupedQuery4->From(*nestedQuery4);
        groupedQuery4->GroupByContract(*contract);

        // note: this is not really correct - each query should use a separate contract with individual relationship paths;
        // it does work because contract doesn't care about the last step in the provided path (which in this case differs between queries)
        contract->SetRelationshipPath(relationshipPath4);

        UnionNavigationQueryPtr expected = UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*groupedQuery1, *groupedQuery2), *groupedQuery3), *groupedQuery4);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey", *expected);
        }

    // AllRelatedInstanceNodes_GroupByClass
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*contract, "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery5->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        
        ComplexNavigationQueryPtr nestedQuery6 = ComplexNavigationQuery::Create();
        nestedQuery6->SelectContract(*contract, "this");
        nestedQuery6->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery6->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectAll();
        nested->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5), *nestedQuery6));
        nested->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nested);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByClass", *expected);
        }

    // AllRelatedInstanceNodes_GroupByClass_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByClass_ChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationship_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationship_ChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationship_MultipleClassesInRelationship
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassB), "this");
        nestedQuery1->From(ret_ClassB, true, "this").Join(RelatedClass(ret_ClassB, ret_ClassA, ret_ClassAHasBAndC, false, "related", "rel_RET_ClassAHasBAndC", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassC), "this");
        nestedQuery2->From(ret_ClassC, true, "this").Join(RelatedClass(ret_ClassC, ret_ClassA, ret_ClassAHasBAndC, false, "related", "rel_RET_ClassAHasBAndC", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_ClassA.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassAHasBAndC.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassAHasBAndC.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationship_MultipleClassesInRelationship", *expected);
        }

    // AllRelatedInstanceNodes_GroupByLabel
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery5->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr nestedQuery6 = ComplexNavigationQuery::Create();
        nestedQuery6->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery6->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery6->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5), *nestedQuery6));
        expected->GroupByContract(*DisplayLabelGroupingNodesQueryContract::Create(nullptr));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByLabel", *expected);
        }

    // AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);

        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        nestedQuery1->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery1->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery2->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery2->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4));
        expected->GroupByContract(*contract);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)1))});
        nestedQuery1->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", {new BoundQueryECValue(ECValue("Label Grouping Node"))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery2->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery2->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("Label Grouping Node")) });
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", true, false));
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery3->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery3->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("Label Grouping Node")) });
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", true, false));
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery4->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery4->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("Label Grouping Node")) });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationshipAndClass_RelationshipNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this");
        nestedQuery->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nestedQuery);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationshipAndClass_RelationshipNodeChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationshipAndClass_ClassNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationshipAndClass_ClassNodeChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationshipAndLabel_RelationshipNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->GroupByContract(*contract);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationshipAndLabel_RelationshipNodeChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationshipAndLabel_LabelNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationshipAndLabel_LabelNodeChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByRelationshipAndClassAndLabel_LabelNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?", { new BoundQueryECValue(ECValue("Label Grouping Node")) });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByRelationshipAndClassAndLabel_LabelNodeChildrenQuery", *expected);
        }

    // RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery2->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded", *expected);
        }

    // RelatedInstanceNodes_RelationshipClassNamesOverridesSupportedSchemas
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("RelatedInstanceNodes_RelationshipClassNamesOverridesSupportedSchemas", *expected);
        }

    // RelatedInstanceNodes_RelatedClasses_OnlyIncluded
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Sprocket);

        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_OnlyIncluded", *expected);
        }

    // RelatedInstanceNodes_RelatedClassesOverridesSupportedSchemas
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClassesOverridesSupportedSchemas", *expected);
        }

    // RelatedInstanceNodes_RelatedClassesAndRelationships_OnlyIncluded
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClassesAndRelationships_OnlyIncluded", *expected);
        }

    // RelatedInstanceNodes_RelatedClasses_AllDerivedClasses
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);

        ComplexNavigationQueryPtr includeQuery1 = ComplexNavigationQuery::Create();
        includeQuery1->SelectContract(*contract, "this");
        includeQuery1->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        includeQuery1->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr excludeQuery1 = ComplexNavigationQuery::Create();
        excludeQuery1->SelectContract(*contract, "this");
        excludeQuery1->From(ret_Gadget, false, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_1", false, false));
        excludeQuery1->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
    
        ComplexNavigationQueryPtr includeQuery2 = ComplexNavigationQuery::Create();
        includeQuery2->SelectContract(*contract, "this");
        includeQuery2->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        includeQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr excludeQuery2 = ComplexNavigationQuery::Create();
        excludeQuery2->SelectContract(*contract, "this");
        excludeQuery2->From(ret_Gadget, false, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_1", false, false));
        excludeQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr orderedInclude = ComplexNavigationQuery::Create();
        orderedInclude->SelectAll();
        orderedInclude->From(*UnionNavigationQuery::Create(*includeQuery1, *includeQuery2));
        orderedInclude->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        ExceptNavigationQueryPtr exceptQuery = ExceptNavigationQuery::Create(*excludeQuery1, *excludeQuery2);

        ExceptNavigationQueryPtr expected = ExceptNavigationQuery::Create(ComplexNavigationQuery::Create()->SelectAll().From(*orderedInclude), *exceptQuery);
        
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_AllDerivedClasses", *expected);
        }

    // RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();

        ComplexNavigationQueryPtr nestedQuery11 = ComplexNavigationQuery::Create();
        nestedQuery11->SelectContract(*contract, "this");
        nestedQuery11->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery11->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr nestedQuery12 = ComplexNavigationQuery::Create();
        nestedQuery12->SelectContract(*contract, "this");
        nestedQuery12->From(ret_Gadget, false, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_1", false, false));
        nestedQuery12->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
    
        ComplexNavigationQueryPtr nestedQuery21 = ComplexNavigationQuery::Create();
        nestedQuery21->SelectContract(*contract, "this");
        nestedQuery21->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery21->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery22 = ComplexNavigationQuery::Create();
        nestedQuery22->SelectContract(*contract, "this");
        nestedQuery22->From(ret_Gadget, false, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_1", false, false));
        nestedQuery22->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*nestedQuery11, *nestedQuery21);
        ExceptNavigationQueryPtr exceptQuery = ExceptNavigationQuery::Create(*ExceptNavigationQuery::Create(*unionQuery, *nestedQuery12), *nestedQuery22);
        
        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectAll();
        grouped2->From(*exceptQuery);
        grouped2->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped2);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded", *expected);        
        }

    // RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_ClassE);
    
        ComplexNavigationQueryPtr includeQuery = ComplexNavigationQuery::Create();
        includeQuery->SelectContract(*contract, "this");
        includeQuery->From(ret_ClassE, true, "this").Join(RelatedClass(ret_ClassE, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0", true, false));
        includeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr orderedInclude = ComplexNavigationQuery::Create();
        orderedInclude->SelectAll();
        orderedInclude->From(*includeQuery);
        orderedInclude->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
    
        ComplexNavigationQueryPtr excludeQuery1 = ComplexNavigationQuery::Create();
        excludeQuery1->SelectContract(*contract, "this");
        excludeQuery1->From(ret_ClassF, true, "this").Join(RelatedClass(ret_ClassF, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_4", true, false));
        excludeQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr excludeQuery2 = ComplexNavigationQuery::Create();
        excludeQuery2->SelectContract(*contract, "this");
        excludeQuery2->From(ret_ClassG, false, "this").Join(RelatedClass(ret_ClassG, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_5", false, false));
        excludeQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ExceptNavigationQueryPtr excludes = ExceptNavigationQuery::Create(*excludeQuery1, *excludeQuery2);
        ExceptNavigationQueryPtr expected = ExceptNavigationQuery::Create(ComplexNavigationQuery::Create()->SelectAll().From(*orderedInclude), *excludes);

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_ClassD.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses", *expected);
        }

    // RelatedInstanceNodes_ExcludedWithDifferentSorting
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_ClassE);
    
        ComplexNavigationQueryPtr includeQuery = ComplexNavigationQuery::Create();
        includeQuery->SelectContract(*contract, "this");
        includeQuery->From(ret_ClassE, false, "this").Join(RelatedClass(ret_ClassE, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0", true, false));
        includeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        includeQuery->OrderBy("[this].[IntProperty]");

        ComplexNavigationQueryPtr wrappedInclude = ComplexNavigationQuery::Create();
        wrappedInclude->SelectAll();
        wrappedInclude->From(*includeQuery);

        ComplexNavigationQueryPtr unorderedInclude = ComplexNavigationQuery::Create();
        unorderedInclude->SelectContract(*contract, "this");
        unorderedInclude->From(ret_ClassG, true, "this").Join(RelatedClass(ret_ClassG, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0", true, false));
        unorderedInclude->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        UnionNavigationQueryPtr includes = UnionNavigationQuery::Create(*wrappedInclude, *unorderedInclude);

        ComplexNavigationQueryPtr excludeQuery = ComplexNavigationQuery::Create();
        excludeQuery->SelectContract(*contract, "this");
        excludeQuery->From(ret_ClassF, true, "this").Join(RelatedClass(ret_ClassF, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_4", true, false));
        excludeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ExceptNavigationQueryPtr expected = ExceptNavigationQuery::Create(*includes, *excludeQuery);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_ClassD.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());

        RegisterQuery("RelatedInstanceNodes_ExcludedWithDifferentSorting", *expected);
        }

    // RelatedInstanceNodes_InstanceFilter
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery1->Where("[this].[Description] = '2'", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        nestedQuery2->Where("[this].[Description] = '2'", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter", *expected);
        }

    // RelatedInstanceNodes_InstanceFilter_LikeOperator
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery1->Where("CAST([this].[Description] AS TEXT) LIKE 'Test' ESCAPE \'\\\'", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        nestedQuery2->Where("CAST([this].[Description] AS TEXT) LIKE 'Test' ESCAPE \'\\\'", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter_LikeOperator", *expected);
        }

    // RelatedInstanceNodes_InstanceFilter_IsOfClassFunction
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);

        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this");
        nestedQuery1->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        nestedQuery1->Where("IsOfClass([this].[ECClassId], 'ClassName', 'SchemaName')", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this");
        nestedQuery2->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery2->Where("IsOfClass([this].[ECClassId], 'ClassName', 'SchemaName')", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprocket.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter_IsOfClassFunction", *expected);
        }

    // RelatedInstanceNodes_InstanceFilter_AllowsAccessingParentNode
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Gadget, true, "this");
        nested->From(ret_Sprocket, false, "parent", true);
        nested->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nested->Where("[related].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        nested->Where("[parent].[ECInstanceId] = ?", { new BoundQueryId(ECInstanceId((uint64_t)123)) });
        nested->Where("[this].[Description] = [parent].[Description]", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nested);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter_AllowsAccessingParentNode", *expected);
        }

    // RelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority
        {    
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID")}), "this");
        query->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        query->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*query);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority", *expected);
        }

    // RelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")}), "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", true, false));
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery2->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", true, false));
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", *expected);
        }

    // InstancesOfSpecificClasses_ClassNames_NotPolymorphic
        {
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1A), "this");
        nestedQuery1->From(b1_Class1A, false, "this");

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this");
        nestedQuery2->From(b1_Class1B, false, "this");

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_ClassNames_NotPolymorphic", *expected);
        }

    // InstancesOfSpecificClasses_ClassNames_Polymorphic
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b1_Class1A, true, "this");
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_ClassNames_Polymorphic", *expected);
        }

    // InstancesOfSpecificClasses_GroupByClass
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b1_Class1A, false, "this"));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

        RegisterQuery("InstancesOfSpecificClasses_GroupByClass", *expected);
        }

    // InstancesOfSpecificClasses_GroupByClass_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b1_Class1A, false, "this"));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_GroupByClass_ChildrenQuery", *expected);
        }

    // InstancesOfSpecificClasses_GroupByLabel
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&b1_Class1A), "this");
        nestedQuery1->From(b1_Class1A, false, "this");

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&b1_Class1B), "this");
        nestedQuery2->From(b1_Class1B, false, "this");

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->GroupByContract(*DisplayLabelGroupingNodesQueryContract::Create(nullptr));
        expected->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        RegisterQuery("InstancesOfSpecificClasses_GroupByLabel", *expected);
        }

    // InstancesOfSpecificClasses_GroupByLabel_ChildrenQuery
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1A), "this");
        nestedQuery1->From(b1_Class1A, false, "this");
        nestedQuery1->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery1->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[Name], '[]') = ?", { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this");
        nestedQuery2->From(b1_Class1B, false, "this");
        nestedQuery2->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery2->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[Name], '[]') = ?", { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_GroupByLabel_ChildrenQuery", *expected);
        }

    // InstancesOfSpecificClasses_GroupByClassAndLabel_ClassNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&b1_Class1A);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b1_Class1A, false, "this"));
        expected->GroupByContract(*contract);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        RegisterQuery("InstancesOfSpecificClasses_GroupByClassAndLabel_ClassNodeChildrenQuery", *expected);
        }

    // InstancesOfSpecificClasses_GroupByClassAndLabel_LabelNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);

        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b1_Class1A, false, "this");
        nestedQuery->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nestedQuery->Where("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[Name], '[]') = ?", { new BoundQueryECValue(ECValue("Label Grouping Node")) });

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_GroupByClassAndLabel_LabelNodeChildrenQuery", *expected);
        }

    // InstancesOfSpecificClasses_InstanceFilter
        {    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1A), "this");
        nestedQuery1->From(b1_Class1A, false, "this");
        nestedQuery1->Where("[this].[Name] = 2", BoundQueryValuesList());

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this");
        nestedQuery2->From(b1_Class1B, false, "this");
        nestedQuery2->Where("[this].[Name] = 2", BoundQueryValuesList());

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter", *expected);
        }

    // InstancesOfSpecificClasses_InstanceFilter_LikeOperator
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b2_Class2);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b2_Class2, false, "this");
        nestedQuery->Where("CAST([this].[Name] AS TEXT) LIKE 'Test' ESCAPE \'\\\'", BoundQueryValuesList());

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter_LikeOperator", *expected);
        }

    // InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstanceWithoutParentNodeDoesntApplyTheFilter
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b1_Class1A, false, "this");

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstanceWithoutParentNodeDoesntApplyTheFilter", *expected);
        }

    // InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstance_WithParentInstanceNode
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b1_Class1A, false, "this");
        nestedQuery->From(b2_Class2, false, "parent", true);
        nestedQuery->Where("[parent].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))}, true);
        nestedQuery->Where("[this].[Name] = [parent].[Name]", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstance_WithParentInstanceNode", *expected);
        }

    // InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentInstance
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b1_Class1A, false, "this");
        nestedQuery->From(b2_Class2, false, "parent_parent", true);
        nestedQuery->Where("[parent_parent].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))}, true);
        nestedQuery->Where("[this].[Name] = [parent_parent].[Name]", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentInstance", *expected);
        }

    // InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(b1_Class1A, false, "this");
        nestedQuery->From(b2_Class2, false, "parent", true);
        nestedQuery->From(b2_Class2, false, "parent_parent", true);
        nestedQuery->Where("[parent].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)456))}, true);
        nestedQuery->Where("[parent_parent].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))}, true);
        nestedQuery->Where("[this].[Name] = [parent_parent].[Name] OR [this].[Name] = [parent].[Name]", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance", *expected);
        }

    // InstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels
        {
        NavigationQueryContractPtr gadgetContract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
        ComplexNavigationQueryPtr gadgetQuery = ComplexNavigationQuery::Create();
        gadgetQuery->SelectContract(*gadgetContract, "this");
        gadgetQuery->From(ret_Gadget, false, "this");

        NavigationQueryContractPtr widgetContract = ECInstanceNodesQueryContract::Create(&ret_Widget, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});   
        ComplexNavigationQueryPtr widgetQuery = ComplexNavigationQuery::Create();
        widgetQuery->SelectContract(*widgetContract, "this");
        widgetQuery->From(ret_Widget, false, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*UnionNavigationQuery::Create(*gadgetQuery, *widgetQuery));
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("InstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", *sorted);
        }

    // InstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority
        {
        NavigationQueryContractPtr widgetContract = ECInstanceNodesQueryContract::Create(&ret_Widget, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID")});   
        ComplexNavigationQueryPtr widgetQuery = ComplexNavigationQuery::Create();
        widgetQuery->SelectContract(*widgetContract, "this");
        widgetQuery->From(ret_Widget, false, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*widgetQuery);
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("InstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority", *sorted);
        }

    // SearchResultInstanceNodes_NoGrouping
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("SearchResultInstanceNodes_NoGrouping", *ordered);
        }
        
    // SearchResultInstanceNodes_GroupByClass
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECClassGroupingNodesQueryContract> contract = ECClassGroupingNodesQueryContract::Create();
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, SEARCH_QUERY_Alias);
        query->From(*searchQuery, SEARCH_QUERY_Alias);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);
        
        RegisterQuery("SearchResultInstanceNodes_GroupByClass", *expected);
        }

    // SearchResultInstanceNodes_GroupByClass_ChildrenQuery
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);
        instancesQuery->Where("[" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECClassId "] = ?", {new BoundQueryId(ret_Widget.GetId())});

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("SearchResultInstanceNodes_GroupByClass_ChildrenQuery", *ordered);
        }
        
    // SearchResultInstanceNodes_GroupByLabel
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<DisplayLabelGroupingNodesQueryContract> contract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->GroupByContract(*contract);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        
        RegisterQuery("SearchResultInstanceNodes_GroupByLabel", *ordered);
        }

    // SearchResultInstanceNodes_GroupByLabel_ChildrenQuery
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);
        instancesQuery->Where("[" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECInstanceId "] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        instancesQuery->Where("GetECInstanceDisplayLabel([" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECClassId "], [" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECInstanceId "], '', '[]') = ?", { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("SearchResultInstanceNodes_GroupByLabel_ChildrenQuery", *ordered);
        }

    // SearchResultInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<DisplayLabelGroupingNodesQueryContract> contract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);
        instancesQuery->Where("[" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECClassId "] = ?", {new BoundQueryId(ret_Widget.GetId())});

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->GroupByContract(*contract);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        RegisterQuery("SearchResultInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery", *ordered);
        }

    // SearchResultInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);
        instancesQuery->Where("[" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECClassId "] = ?", { new BoundQueryId(ret_Widget.GetId()) });
        instancesQuery->Where("[" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECInstanceId "] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        instancesQuery->Where("GetECInstanceDisplayLabel([" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECClassId "], [" SEARCH_QUERY_Alias "].[" SEARCH_QUERY_FIELD_ECInstanceId "], '', '[]') = ?",
            { new BoundQueryECValue(ECValue("MyLabel")) });

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("SearchResultInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery", *ordered);
        }

    // SearchResultInstanceNodes_UsesParentPropertyValueQuery
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("SearchResultInstanceNodes_UsesParentPropertyValueQuery", *ordered);
        }

    // SearchResultInstanceNodes_InstanceLabelOverride_AppliedByPriority
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<DisplayLabelGroupingNodesQueryContract> contract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget, true, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("IntProperty"), new InstanceLabelOverridePropertyValueSpecification("MyID")});
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, SEARCH_QUERY_Alias);
        instancesQuery->From(*searchQuery, SEARCH_QUERY_Alias);

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->GroupByContract(*contract);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        
        RegisterQuery("SearchResultInstanceNodes_InstanceLabelOverride_AppliedByPriority", *ordered);
        }

    // SearchResultInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels
        {
        StringNavigationQueryPtr widgetSearchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<DisplayLabelGroupingNodesQueryContract> widgetContract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget, true, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
        widgetContract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        widgetContract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);
        ComplexNavigationQueryPtr widgetInstancesQuery = ComplexNavigationQuery::Create();
        widgetInstancesQuery->SelectContract(*widgetContract, SEARCH_QUERY_Alias);
        widgetInstancesQuery->From(*widgetSearchQuery, SEARCH_QUERY_Alias);

        StringNavigationQueryPtr gadgetSearchQuery = StringNavigationQuery::Create("SELECT MyID, ECInstanceId AS [" SEARCH_QUERY_FIELD_ECInstanceId "], ECClassId AS [" SEARCH_QUERY_FIELD_ECClassId "] FROM [RulesEngineTest].[Gadget] WHERE [Gadget].[ECInstanceId] > 0");
        RefCountedPtr<DisplayLabelGroupingNodesQueryContract> gadgetContract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Gadget);
        gadgetContract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        gadgetContract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);
        ComplexNavigationQueryPtr gadgetInstancesQuery = ComplexNavigationQuery::Create();
        gadgetInstancesQuery->SelectContract(*gadgetContract, SEARCH_QUERY_Alias);
        gadgetInstancesQuery->From(*gadgetSearchQuery, SEARCH_QUERY_Alias);

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*UnionNavigationQuery::Create(*widgetInstancesQuery, *gadgetInstancesQuery));
        ordered->GroupByContract(*gadgetContract);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        ordered->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        
        RegisterQuery("SearchResultInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", *ordered);
        }

    // SortingRule_SortByRulesAndLabelAndNotSorted
        {        
        ComplexNavigationQueryPtr rulesSortedQuery = ComplexNavigationQuery::Create();
        rulesSortedQuery->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassA), "this");
        rulesSortedQuery->From(b4_ClassA, false, "this");
        rulesSortedQuery->OrderBy("[this].[SomeProperty]");

        ComplexNavigationQueryPtr rulesSortedQueryWrapper = ComplexNavigationQuery::Create();
        rulesSortedQueryWrapper->SelectAll().From(*rulesSortedQuery);

        ComplexNavigationQueryPtr nestedLabelSortedQuery = ComplexNavigationQuery::Create();
        nestedLabelSortedQuery->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassC), "this");
        nestedLabelSortedQuery->From(b4_ClassC, true, "this");

        ComplexNavigationQueryPtr labelSortedQuery = ComplexNavigationQuery::Create();
        labelSortedQuery->SelectAll().From(*nestedLabelSortedQuery).OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        ComplexNavigationQueryPtr labelSortedQueryWrapper = ComplexNavigationQuery::Create();
        labelSortedQueryWrapper->SelectAll().From(*labelSortedQuery);

        ComplexNavigationQueryPtr notSortedQuery = ComplexNavigationQuery::Create();
        notSortedQuery->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassB), "this");
        notSortedQuery->From(b4_ClassB, false, "this");

        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*rulesSortedQueryWrapper, *labelSortedQueryWrapper), *notSortedQuery);
        RegisterQuery("SortingRule_SortByRulesAndLabelAndNotSorted", *unionQuery);
        }

    // SortingRule_SortSingleECClassByRule
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);
        
        ComplexNavigationQueryPtr rulesSortedQuery = ComplexNavigationQuery::Create();
        rulesSortedQuery->SelectContract(*contract, "this");
        rulesSortedQuery->From(b3_Class3, true, "this");
        rulesSortedQuery->OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "SomeProperty").c_str());

        RegisterQuery("SortingRule_SortSingleECClassByRule", *rulesSortedQuery);
        }

    // SortingRule_SortSingleECClassByDoNotSortRule
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);
        
        ComplexNavigationQueryPtr notSortedQuery = ComplexNavigationQuery::Create();
        notSortedQuery->SelectContract(*contract, "this");
        notSortedQuery->From(b3_Class3, true, "this");

        RegisterQuery("SortingRule_SortSingleECClassByDoNotSortRule", *notSortedQuery);
        }

    // SortingRule_SortDescending
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);
        
        ComplexNavigationQueryPtr rulesSortedQuery = ComplexNavigationQuery::Create();
        rulesSortedQuery->SelectContract(*contract, "this");
        rulesSortedQuery->From(b3_Class3, true, "this");
        rulesSortedQuery->OrderBy(Utf8PrintfString("%s([this].[%s]) DESC", FUNCTION_NAME_GetSortingValue, "SomeProperty").c_str());

        RegisterQuery("SortingRule_SortDescending", *rulesSortedQuery);
        }

    // SortingRule_AppliedToAllSchemaClasses
        {        
        ComplexNavigationQueryPtr rulesSortedQuery = ComplexNavigationQuery::Create();
        rulesSortedQuery->SelectContract(*ECInstanceNodesQueryContract::Create(&b3_Class3), "this");
        rulesSortedQuery->From(b3_Class3, true, "this");
        rulesSortedQuery->OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "SomeProperty").c_str());

        ComplexNavigationQueryPtr rulesSortedQueryWrapper = ComplexNavigationQuery::Create();
        rulesSortedQueryWrapper->SelectAll().From(*rulesSortedQuery);

        ComplexNavigationQueryPtr nestedLabelSortedQuery = ComplexNavigationQuery::Create();
        nestedLabelSortedQuery->SelectContract(*ECInstanceNodesQueryContract::Create(&b2_Class2), "this");
        nestedLabelSortedQuery->From(b2_Class2, true, "this");

        ComplexNavigationQueryPtr labelSortedQuery = ComplexNavigationQuery::Create();
        labelSortedQuery->SelectAll().From(*nestedLabelSortedQuery).OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        ComplexNavigationQueryPtr labelSortedQueryWrapper = ComplexNavigationQuery::Create();
        labelSortedQueryWrapper->SelectAll().From(*labelSortedQuery);

        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*rulesSortedQueryWrapper, *labelSortedQueryWrapper);

        RegisterQuery("SortingRule_AppliedToAllSchemaClasses", *unionQuery);
        }

    // SortingRule_AppliedToClassesOfAllSchemas
        {
        ComplexNavigationQueryPtr rulesSortedQuery1 = ComplexNavigationQuery::Create();
        rulesSortedQuery1->SelectContract(*ECInstanceNodesQueryContract::Create(&b3_Class3), "this");
        rulesSortedQuery1->From(b3_Class3, true, "this");
        rulesSortedQuery1->OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "SomeProperty").c_str());

        ComplexNavigationQueryPtr rulesSortedQueryWrapper1 = ComplexNavigationQuery::Create();
        rulesSortedQueryWrapper1->SelectAll().From(*rulesSortedQuery1);

        ComplexNavigationQueryPtr rulesSortedQuery2 = ComplexNavigationQuery::Create();
        rulesSortedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassA), "this");
        rulesSortedQuery2->From(b4_ClassA, true, "this");
        rulesSortedQuery2->OrderBy("[this].[SomeProperty]");

        ComplexNavigationQueryPtr rulesSortedQueryWrapper2 = ComplexNavigationQuery::Create();
        rulesSortedQueryWrapper2->SelectAll().From(*rulesSortedQuery2);

        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*rulesSortedQueryWrapper1, *rulesSortedQueryWrapper2);

        RegisterQuery("SortingRule_AppliedToClassesOfAllSchemas", *unionQuery);
        }
        
    // SortingRule_OverridenBySpecificationsDoNotSortFlag
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);
        
        ComplexNavigationQueryPtr notSortedQuery = ComplexNavigationQuery::Create();
        notSortedQuery->SelectContract(*contract, "this");
        notSortedQuery->From(b3_Class3, true, "this");

        RegisterQuery("SortingRule_OverridenBySpecificationsDoNotSortFlag", *notSortedQuery);
        }

    // SortingRule_SortByInvalidProperty
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);
        
        ComplexNavigationQueryPtr nestedLabelSortedQuery = ComplexNavigationQuery::Create();
        nestedLabelSortedQuery->SelectContract(*contract, "this");
        nestedLabelSortedQuery->From(b3_Class3, true, "this");

        ComplexNavigationQueryPtr labelSortedQuery = ComplexNavigationQuery::Create();
        labelSortedQuery->SelectAll().From(*nestedLabelSortedQuery).OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("SortingRule_SortByInvalidProperty", *labelSortedQuery);
        }

    // SortingRule_DoesntSortWhenGroupingByClass
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b3_Class3, true, "this"));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

        RegisterQuery("SortingRule_DoesntSortWhenGroupingByClass", *expected);
        }

    // SortingRule_DoesntSortWhenGroupingByLabel
        {
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&b3_Class3);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b3_Class3, true, "this"));
        expected->GroupByContract(*contract);
        expected->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        RegisterQuery("SortingRule_DoesntSortWhenGroupingByLabel", *expected);
        }

    // SortingRule_SortsByProperty_WhenUsingParentInstanceOfTheSameClass
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b3_Class3);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectContract(*contract, "this");
        expected->From(b3_Class3, false, "this");
        expected->From(b3_Class3, false, "parent");
        expected->Where("[parent].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        expected->Where("[this].[SomeProperty] = [parent].[SomeProperty]", BoundQueryValuesList());
        expected->OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "SomeProperty").c_str());

        RegisterQuery("SortingRule_SortsByProperty_WhenUsingParentInstanceOfTheSameClass", *expected);
        }

    // SortingRule_SortsUsingRelatedInstanceProperty
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0");
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget, {relatedInstanceClass});
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectContract(*contract, "this");
        expected->From(ret_Gadget, false, "this");
        expected->Join(relatedInstanceClass);
        expected->OrderBy("[widget].[IntProperty]");
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        RegisterQuery("SortingRule_SortsUsingRelatedInstanceProperty", *expected);
        }

    // Grouping_ClassFilterIsPolymorphic
        {
        NavigationQueryContractPtr groupingContract = DisplayLabelGroupingNodesQueryContract::Create(nullptr, false);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassB), "this").From(b4_ClassB, false, "this"));
        grouped->GroupByContract(*groupingContract);
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::SameLabelInstance);
        grouped->GetResultParametersR().SetHasInstanceGroups(true);

        RegisterQuery("Grouping_ClassFilterIsPolymorphic", *grouped);
        }

    // Grouping_IgnoresGroupingRulesWithInvalidSchemas
        {
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassA), "this").From(b4_ClassA, false, "this"));
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        RegisterQuery("Grouping_IgnoresGroupingRulesWithInvalidSchemas", *grouped);
        }

    // Grouping_IgnoresGroupingRulesWithInvalidClasses
        {
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b4_ClassA), "this").From(b4_ClassA, false, "this"));
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        RegisterQuery("Grouping_IgnoresGroupingRulesWithInvalidClasses", *grouped);
        }

    // Grouping_SameLabelInstanceGroup_OnlyGroupedInstanceNodes
        {
        NavigationQueryContractPtr groupingContract = DisplayLabelGroupingNodesQueryContract::Create(nullptr, false);

        ComplexNavigationQueryPtr grouped1 = ComplexNavigationQuery::Create();
        grouped1->SelectAll();
        grouped1->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1A), "this").From(b1_Class1A, true, "this"));
        grouped1->GroupByContract(*groupingContract);

        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectAll();
        grouped2->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this").From(b1_Class1B, true, "this"));
        grouped2->GroupByContract(*groupingContract);

        UnionNavigationQueryPtr sorted = UnionNavigationQuery::Create(*grouped1, *grouped2);
        sorted->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::SameLabelInstance);
        sorted->GetResultParametersR().SetHasInstanceGroups(true);

        RegisterQuery("Grouping_SameLabelInstanceGroup_OnlyGroupedInstanceNodes", *sorted);
        }

    // Grouping_SameLabelInstanceGroup_MultipleGroupedAndUngroupedInstanceNodes
        {
        NavigationQueryContractPtr groupingContract = DisplayLabelGroupingNodesQueryContract::Create(nullptr, false);

        ComplexNavigationQueryPtr grouped1 = ComplexNavigationQuery::Create();
        grouped1->SelectAll();
        grouped1->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1A), "this").From(b1_Class1A, true, "this"));
        grouped1->GroupByContract(*groupingContract);

        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectAll();
        grouped2->From(ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this").From(b1_Class1B, true, "this"));
        grouped2->GroupByContract(*groupingContract);

        ComplexNavigationQueryPtr notGrouped = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(b4_ClassA, true, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*grouped1, *grouped2), *notGrouped));
        sorted->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::SameLabelInstance);
        sorted->GetResultParametersR().SetHasInstanceGroups(true);

        RegisterQuery("Grouping_SameLabelInstanceGroup_MultipleGroupedAndUngroupedInstanceNodes", *sorted);
        }

    // Grouping_ClassGroup_GroupsByClass
        {
        NavigationQueryContractPtr contract = BaseECClassGroupingNodesQueryContract::Create(b2_Class2.GetId());

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(b2_Class2, true, "this"));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(baseClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::BaseClass);
        
        RegisterQuery("Grouping_ClassGroup_GroupsByClass", *expected);
        }
        
    // Grouping_ClassGroup_GroupsByBaseClass
        {
        NavigationQueryContractPtr contract = BaseECClassGroupingNodesQueryContract::Create(b4_ClassA.GetId());

        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*contract, "this");
        nested1->From(b4_ClassB, false, "this");
        
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*contract, "this");
        nested2->From(b4_ClassC, false, "this");
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*UnionNavigationQuery::Create(*nested1, *nested2));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(baseClassGroupingNodesOrderByClause.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::BaseClass);
        
        RegisterQuery("Grouping_ClassGroup_GroupsByBaseClass", *sorted);
        }
        
    // Grouping_ClassGroup_BaseClassGroupingAndGroupByClass
        {
        NavigationQueryContractPtr baseClassGroupingContract = BaseECClassGroupingNodesQueryContract::Create(b4_ClassA.GetId());
        NavigationQueryContractPtr classGroupingContract = ECClassGroupingNodesQueryContract::Create();
        
        ComplexNavigationQueryPtr classBQuery = ComplexNavigationQuery::Create();
        classBQuery->SelectContract(*baseClassGroupingContract, "this");
        classBQuery->From(b4_ClassB, true, "this");

        ComplexNavigationQueryPtr baseClassGroupingQueryGrouped = ComplexNavigationQuery::Create();
        baseClassGroupingQueryGrouped->SelectAll();
        baseClassGroupingQueryGrouped->From(*classBQuery);
        baseClassGroupingQueryGrouped->GroupByContract(*baseClassGroupingContract);

        ComplexNavigationQueryPtr baseClassGroupingQuery = ComplexNavigationQuery::Create();
        baseClassGroupingQuery->SelectAll();
        baseClassGroupingQuery->From(*baseClassGroupingQueryGrouped);
        baseClassGroupingQuery->OrderBy(baseClassGroupingNodesOrderByClause.c_str());
        baseClassGroupingQuery->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::BaseClass);

        ComplexNavigationQueryPtr classGroupingQueryGrouped = ComplexNavigationQuery::Create();
        classGroupingQueryGrouped->SelectAll();
        classGroupingQueryGrouped->From(ComplexNavigationQuery::Create()->SelectContract(*classGroupingContract, "this").From(b4_ClassA, false, "this"));
        classGroupingQueryGrouped->GroupByContract(*classGroupingContract);

        ComplexNavigationQueryPtr classGroupingQuery = ComplexNavigationQuery::Create();
        classGroupingQuery->SelectAll();
        classGroupingQuery->From(*classGroupingQueryGrouped);
        classGroupingQuery->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        classGroupingQuery->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

        RegisterQuery("Grouping_ClassGroup_BaseClassGroupingAndGroupByClass_BaseClass", *baseClassGroupingQuery);
        RegisterQuery("Grouping_ClassGroup_BaseClassGroupingAndGroupByClass_Class", *classGroupingQuery);
        }

    // Grouping_ClassGroup_ECInstanceNodesChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b2_Class2);
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectContract(*contract, "this");
        expected->From(b2_Class2, true, "this");        
        RegisterQuery("Grouping_ClassGroup_ECInstanceNodesChildrenQuery", *expected);
        }

    // Grouping_PropertyGroup_GroupsByProperty
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b2_Class2, *b2_Class2.GetPropertyP("Name"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b2_Class2, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_GroupsByProperty", *grouped);
        }

    // Grouping_PropertyGroup_GroupsMultipleClassesByProperty
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);

        NavigationQueryContractPtr contract1 = ECPropertyGroupingNodesQueryContract::Create(b1_Class1A, *b1_Class1A.GetPropertyP("Name"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*contract1, "this");
        nested1->From(b1_Class1A, true, "this");
        ComplexNavigationQueryPtr grouped1 = ComplexNavigationQuery::Create();
        grouped1->SelectAll().From(*nested1).GroupByContract(*contract1);
        
        NavigationQueryContractPtr contract2 = ECPropertyGroupingNodesQueryContract::Create(b1_Class1B, *b1_Class1B.GetPropertyP("Name"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*contract2, "this");
        nested2->From(b1_Class1B, true, "this");
        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectAll().From(*nested2).GroupByContract(*contract2);
                
        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*grouped2, *grouped1);
        unionQuery->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        unionQuery->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

        RegisterQuery("Grouping_PropertyGroup_GroupsMultipleClassesByProperty", *unionQuery);
        }

    // Grouping_PropertyGroup_GroupsByRange
        {
        PropertyGroupP spec = new PropertyGroup("", "", false, "Name", "");
        spec->AddRange(*new PropertyRangeGroupSpecification("", "", "0", "5"));
        spec->AddRange(*new PropertyRangeGroupSpecification("", "", "6", "10"));
        spec->AddRange(*new PropertyRangeGroupSpecification("", "", "11", "20"));
        RegisterForDelete(*spec);

        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b1_Class1A, *b1_Class1A.GetPropertyP("Name"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll().From(*nested).GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_GroupsByRange", *grouped);
        }

    // Grouping_PropertyGroup_OverridesImageId
        {
        PropertyGroupP spec = new PropertyGroup("", "TestImageId", true, "", "");
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b2_Class2, *b2_Class2.GetPropertyP("Name"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b2_Class2, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_OverridesImageId", *grouped);
        }

    // Grouping_PropertyGroup_ValueFiltering
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[Name] = ?", {new BoundQueryECValue(ECValue(9))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("Grouping_PropertyGroup_ValueFiltering", *grouped);
        }

    // Grouping_PropertyGroup_RangeFiltering
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[Name] BETWEEN ? AND ?", {new BoundQueryECValue(ECValue(1)), new BoundQueryECValue(ECValue(5))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("Grouping_PropertyGroup_RangeFiltering", *grouped);
        }

    // Grouping_PropertyGroup_OtherRangeFiltering
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[Name] NOT BETWEEN ? AND ? AND [this].[Name] NOT BETWEEN ? AND ? AND [this].[Name] NOT BETWEEN ? AND ?", 
            {new BoundQueryECValue(ECValue(1)), new BoundQueryECValue(ECValue(5)), 
            new BoundQueryECValue(ECValue(7)), new BoundQueryECValue(ECValue(9)),
            new BoundQueryECValue(ECValue(10)), new BoundQueryECValue(ECValue(15))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("Grouping_PropertyGroup_OtherRangeFiltering", *grouped);
        }

    // Grouping_PropertyGroup_CreatesGroupingQueryWhenRelationshipWithNavigationPropertyIsUsed
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);

        NavigationQueryContractPtr contract1 = ECPropertyGroupingNodesQueryContract::Create(sc3_ChildClassWithNavigationProperty, *sc3_ChildClassWithNavigationProperty.GetPropertyP("Group"), nullptr, *spec, sc3_GroupingClass.GetEntityClassCP());
        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*contract1, "this");
        nested1->From(sc3_ChildClassWithNavigationProperty, false, "this");
        nested1->Join(RelatedClass(sc3_ChildClassWithNavigationProperty, sc3_GroupingClass, sc3_NavigationGrouping, false, "parentInstance", "rel_sc3_NavigationGrouping"));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll().From(*nested1).GroupByContract(*contract1);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
                
        RegisterQuery("Grouping_PropertyGroup_CreatesGroupingQueryWhenRelationshipWithNavigationPropertyIsUsed", *grouped);
        }

    // Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodes
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "IntProperty");
        RegisterForDelete(*spec);
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_ClassF, *ret_ClassE.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*propertyGroupingContract, "this").From(ret_ClassF, true, "this"));
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        RegisterQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodes_1", *grouped);

        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassE), "this");
        nested1->From(ret_ClassE, false, "this");
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassG), "this");
        nested2->From(ret_ClassG, true, "this");
        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*UnionNavigationQuery::Create(*nested1, *nested2));
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodes_2", *sorted);
        }

    // Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "IntProperty");
        RegisterForDelete(*spec);

        RelatedClass relatedInstanceClass(ret_ClassF, ret_ClassD, ret_ClassDHasClassE, false, "d", "rel_RET_ClassDHasClassE_0");
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_ClassF, *ret_ClassE.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectContract(*propertyGroupingContract, "this");
        grouped->From(ret_ClassF, true, "this");
        grouped->Join(relatedInstanceClass);
        
        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*grouped);
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouping->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouping->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());
        RegisterQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances_1", *grouping);

        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassE, {relatedInstanceClass}), "this");
        nested1->From(ret_ClassE, false, "this");
        nested1->Join(relatedInstanceClass);
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassG, {relatedInstanceClass}), "this");
        nested2->From(ret_ClassG, true, "this");
        nested2->Join(relatedInstanceClass);
        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*UnionNavigationQuery::Create(*nested1, *nested2));
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());

        RegisterQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances_2", *sorted);
        }

    // Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "IntProperty");
        RegisterForDelete(*spec);

        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_ClassF, *ret_ClassF.GetPropertyP("IntProperty"), "e", *spec, nullptr);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectContract(*propertyGroupingContract, "this");
        grouped->From(ret_ClassD, true, "this");
        grouped->Join(RelatedClass(ret_ClassD, ret_ClassF, ret_ClassDHasClassE, true, "e", "rel_RET_ClassDHasClassE_0"));

        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*grouped);
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouping->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouping->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());
        RegisterQuery("Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes_1", *grouping);

        RelatedClass relatedInstanceClass1(ret_ClassD, ret_ClassE, ret_ClassDHasClassE, true, "e", "rel_RET_ClassDHasClassE_0", false, true);
        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassD, {relatedInstanceClass1}), "this");
        nested1->From(ret_ClassD, true, "this");
        nested1->Join(relatedInstanceClass1);

        RelatedClass relatedInstanceClass2(ret_ClassD, ret_ClassG, ret_ClassDHasClassE, true, "e", "rel_RET_ClassDHasClassE_0");
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassD, {relatedInstanceClass2}), "this");
        nested2->From(ret_ClassD, true, "this");
        nested2->Join(relatedInstanceClass2);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*UnionNavigationQuery::Create(*nested1, *nested2));
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());

        RegisterQuery("Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes_2", *sorted);
        }

    // Grouping_PropertyGroup_GroupsByBaseClassPropertyWhenReturningDerivedClassInstances
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "IntProperty");
        RegisterForDelete(*spec);

        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_ClassE, *ret_ClassE.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);

        ComplexNavigationQueryPtr grouped1 = ComplexNavigationQuery::Create();
        grouped1->SelectContract(*propertyGroupingContract, "this");
        grouped1->From(ret_ClassF, false, "this");
        
        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectContract(*propertyGroupingContract, "this");
        grouped2->From(ret_ClassG, false, "this");

        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*UnionNavigationQuery::Create(*grouped1, *grouped2));
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouping->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        RegisterQuery("Grouping_PropertyGroup_GroupsByBaseClassPropertyWhenReturningDerivedClassInstances", *grouping);
        }

    // Grouping_GroupsByRelatedInstanceProperty
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "MyID");
        RegisterForDelete(*spec);

        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_Widget, *ret_Widget.GetPropertyP("IntProperty"), "widget", *spec, nullptr);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*propertyGroupingContract, "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0"));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());

        RegisterQuery("Grouping_GroupsByRelatedInstanceProperty", *grouped);
        }

    // Grouping_GroupsByRelationshipPropertyWhenUsedWithRelatedInstancesSpecification
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "Priority");
        RegisterForDelete(*spec);

        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_WidgetHasGadget, *ret_WidgetHasGadget.GetPropertyP("Priority"), "rel_RET_WidgetHasGadget_0", *spec, nullptr);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*propertyGroupingContract, "this");
        query->From(ret_Gadget, true, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0", true, false));
        query->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Widget.GetId());
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());

        RegisterQuery("Grouping_GroupsByRelationshipPropertyWhenUsedWithRelatedInstancesSpecification", *grouped);
        }

    // Grouping_GroupsMultipleClassesByTheSameRelationshipProperty
        {
        PropertyGroupP spec = new PropertyGroup("", "", true, "Priority");
        RegisterForDelete(*spec);

        NavigationQueryContractPtr contract1 = ECPropertyGroupingNodesQueryContract::Create(ret_ClassDReferencesClassE, *ret_ClassDReferencesClassE.GetPropertyP("Priority"), "rel_RET_ClassDReferencesClassE_0", *spec, nullptr);
        ComplexNavigationQueryPtr query1 = ComplexNavigationQuery::Create();
        query1->SelectContract(*contract1, "this");
        query1->From(ret_ClassF, true, "this");
        query1->Join(RelatedClass(ret_ClassF, ret_ClassD, ret_ClassDReferencesClassE, false, "related", "rel_RET_ClassDReferencesClassE_0", true, false));
        query1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        NavigationQueryContractPtr contract2 = ECPropertyGroupingNodesQueryContract::Create(ret_ClassDReferencesClassE, *ret_ClassDReferencesClassE.GetPropertyP("Priority"), "rel_RET_ClassDReferencesClassE_1", *spec, nullptr);
        ComplexNavigationQueryPtr query2 = ComplexNavigationQuery::Create();
        query2->SelectContract(*contract2, "this");
        query2->From(ret_ClassG, true, "this");
        query2->Join(RelatedClass(ret_ClassG, ret_ClassD, ret_ClassDReferencesClassE, false, "related", "rel_RET_ClassDReferencesClassE_1", true, false));
        query2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*UnionNavigationQuery::Create(*query1, *query2));
        grouped->GroupByContract(*contract1);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_ClassD.GetId());
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDReferencesClassE.GetId());

        RegisterQuery("Grouping_GroupsMultipleClassesByTheSameRelationshipProperty", *grouped);
        }

    // Grouping_PropertyGroup_GroupsByPropertyValue
        {
        PropertyGroupP spec = new PropertyGroup();
        spec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(ret_Widget, *ret_Widget.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Widget, false, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_GroupsByPropertyValue", *sorted);
        }

    // Grouping_PropertyGroup_SortsByDisplayLabelWhenTryingToSortByPropertyValueAndGroupByLabel
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(ret_Widget, *ret_Widget.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Widget, false, "this");
                
        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->GroupByContract(*contract);
        sorted->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_SortsByDisplayLabelWhenTryingToSortByPropertyValueAndGroupByLabel", *sorted);
        }

    // Grouping_PropertyGroup_GroupsAndSortsByPropertyValue
        {
        PropertyGroupP spec = new PropertyGroup();
        spec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(ret_Widget, *ret_Widget.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Widget, false, "this");
        
        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->GroupByContract(*contract);
        sorted->OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::GroupingValueFieldName).append("]").c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_GroupsAndSortsByPropertyValue", *sorted);
        }

    // RootNodesQueryReturnsRelationshipGroupingNodes
        {
        }

    // RelationshipGroupingNodeChildrenQueryReturnsBaseClassGroupingNodes
        {
        NavigationQueryContractPtr contract = BaseECClassGroupingNodesQueryContract::Create(b4_ClassA.GetId());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassC, true, "this").Join(RelatedClass(b4_ClassC, b4_ClassB, b4_ClassBHasClassC, false, "related", "rel_b4_ClassBHasClassC", true, false));
        nested->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(baseClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::BaseClass);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(b4_ClassB.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(b4_ClassBHasClassC.GetId());
        
        RegisterQuery("RelationshipGroupingNodeChildrenQueryReturnsBaseClassGroupingNodes", *expected);
        }

    // RootNodesQueryReturnsBaseClassGroupingNodes_1
        {
        NavigationQueryContractPtr contract = BaseECClassGroupingNodesQueryContract::Create(b4_ClassA.GetId());
        
        ComplexNavigationQueryPtr nestedA = ComplexNavigationQuery::Create();
        nestedA->SelectContract(*contract, "this");
        nestedA->From(b4_ClassA, true, "this");
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nestedA);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(baseClassGroupingNodesOrderByClause.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::BaseClass);
        
        RegisterQuery("RootNodesQueryReturnsBaseClassGroupingNodes_1", *sorted);
        }

    // RootNodesQueryReturnsBaseClassGroupingNodes_2
        {
        NavigationQueryContractPtr contract = BaseECClassGroupingNodesQueryContract::Create(b4_ClassA.GetId());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassA, true, "this");
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(baseClassGroupingNodesOrderByClause.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::BaseClass);
        
        RegisterQuery("RootNodesQueryReturnsBaseClassGroupingNodes_2", *sorted);
        }

    // BaseClassNodeChildrenQueryReturnsClassGroupingNodes_1
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
        
        ComplexNavigationQueryPtr nestedA = ComplexNavigationQuery::Create();
        nestedA->SelectContract(*contract, "this");
        nestedA->From(b4_ClassA, true, "this");
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nestedA);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);
        
        RegisterQuery("BaseClassNodeChildrenQueryReturnsClassGroupingNodes_1", *sorted);
        }

    // BaseClassNodeChildrenQueryReturnsClassGroupingNodes_2
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
        
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassA, true, "this");
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);
        
        RegisterQuery("BaseClassNodeChildrenQueryReturnsClassGroupingNodes_2", *sorted);
        }

    // ClassNodeChildrenQueryReturnsFirstPropertyGroupingNodes
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b4_ClassA, *b4_ClassB.GetPropertyP("SomeProperty"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassB, false, "this");
        nested->Join(RelatedClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0"));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(b4_ClassBHasClassC.GetId());
        
        RegisterQuery("ClassNodeChildrenQueryReturnsFirstPropertyGroupingNodes", *grouped);
        }

    // FirstPropertyNodeChildrenQueryReturnsSecondPropertyGroupingNodes
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b4_ClassA, *b4_ClassB.GetPropertyP("Description"), nullptr, *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassB, false, "this");
        nested->Join(RelatedClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0"));
        nested->Where("[this].[SomeProperty] = ?", {new BoundQueryECValue(ECValue(9))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(b4_ClassBHasClassC.GetId());
        
        RegisterQuery("FirstPropertyNodeChildrenQueryReturnsSecondPropertyGroupingNodes", *grouped);
        }

    // SecondPropertyGroupingNodeChildrenQueryReturnsThirdPropertyGroupingNodes
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b4_ClassC, *b4_ClassC.GetPropertyP("SomeProperty"), "c", *spec, nullptr);
                
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassB, false, "this");
        nested->Join(RelatedClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0"));
        nested->Where("[this].[Description] = ?", {new BoundQueryECValue(ECValue("TestGroupingDescription"))});
        nested->Where("[this].[SomeProperty] = ?", {new BoundQueryECValue(ECValue(9))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(b4_ClassBHasClassC.GetId());
        
        RegisterQuery("SecondPropertyGroupingNodeChildrenQueryReturnsThirdPropertyGroupingNodes", *grouped);
        }

    // ThirdPropertyGroupingNodeChildrenQueryReturnsLabelGroupingNodes
        {
        RelatedClass relatedInstanceClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0");
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&b4_ClassB, true, {relatedInstanceClass});
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassB, false, "this");
        nested->Join(relatedInstanceClass);
        nested->Where("[c].[SomeProperty] = ?", { new BoundQueryECValue(ECValue(99)) });
        nested->Where("[this].[Description] = ?", { new BoundQueryECValue(ECValue("TestGroupingDescription")) });
        nested->Where("[this].[SomeProperty] = ?", { new BoundQueryECValue(ECValue(9)) });

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(labelGroupingQuerySortedDisplayLabel.c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        grouped->GetResultParametersR().GetMatchingRelationshipIds().insert(b4_ClassBHasClassC.GetId());
        
        RegisterQuery("ThirdPropertyGroupingNodeChildrenQueryReturnsLabelGroupingNodes", *grouped);
        }

    // LabelGroupingNodeChildrenQueryReturnsInstanceNodes
        {
        RelatedClass relatedInstanceClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0");
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b4_ClassB, {relatedInstanceClass});
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassB, false, "this");
        nested->Join(relatedInstanceClass);
        nested->Where("[this].[ECInstanceId] IN (?)", { new BoundQueryId(ECInstanceId((uint64_t)1)) });
        nested->Where(Utf8PrintfString("GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[{\"Alias\":\"c\",\"ECClassId\":' || CAST(IFNULL([c].[ECClassId], %" PRIu64 ") AS TEXT) || ',\"ECInstanceId\":' || CAST(IFNULL([c].[ECInstanceId], 0) AS TEXT) || '}]') = ?", b4_ClassC.GetId().GetValue()).c_str(), { new BoundQueryECValue(ECValue("test")) });
        nested->Where("[c].[SomeProperty] = ?", { new BoundQueryECValue(ECValue(99)) });
        nested->Where("[this].[Description] = ?", { new BoundQueryECValue(ECValue("TestGroupingDescription")) });
        nested->Where("[this].[SomeProperty] = ?", { new BoundQueryECValue(ECValue(9)) });

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->GroupByContract(*DisplayLabelGroupingNodesQueryContract::Create(nullptr, false, {relatedInstanceClass})); // grouping because of "same label" grouping
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::SameLabelInstance);
        sorted->GetResultParametersR().SetHasInstanceGroups(true);
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(b4_ClassBHasClassC.GetId());
        
        RegisterQuery("LabelGroupingNodeChildrenQueryReturnsInstanceNodes", *sorted);
        }

    // JoinsWithAdditionalRelatedInstances
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0");
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget, {relatedInstanceClass});
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Gadget, false, "this");
        nested->Join(relatedInstanceClass);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        
        RegisterQuery("JoinsWithAdditionalRelatedInstances", *sorted);
        }

    // InnerJoinsWithAdditionalRelatedInstances
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0", true, false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget, {relatedInstanceClass});
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Gadget, false, "this");
        nested->Join(relatedInstanceClass);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        
        RegisterQuery("InnerJoinsWithAdditionalRelatedInstances", *sorted);
        }

    // FiltersByRelatedInstanceProperties
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0");
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget, {relatedInstanceClass});
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Gadget, false, "this");
        nested->Join(relatedInstanceClass);
        nested->Where("[widget].[IntProperty] > 5 AND [widget].[MyID] <> [this].[MyID]", BoundQueryValuesList());

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        
        RegisterQuery("FiltersByRelatedInstanceProperties", *sorted);
        }

    // SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        
        RegisterQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("[this].[ECInstanceId] IN (?,?)", {new BoundQueryId(ECInstanceId((uint64_t)123)), new BoundQueryId(ECInstanceId((uint64_t)125))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        
        RegisterQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass", *query);
        }
        
    // SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, false));
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query1), "this");
        query1->From(b1_Class1A, false, "this");
        query1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &b2_Class2, *query2), "this");
        query2->From(b2_Class2, false, "this");
        query2->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses", *query);
        }

    // SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b4_ClassB, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b4_ClassC, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(b4_ClassC, b4_ClassB, b4_ClassBHasClassC, false, "nav_b4_ClassB_0", "nav_b4_ClassBHasClassC_0")
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b4_ClassB, *b4_ClassB.GetPropertyP("SomeProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", b4_ClassC, *b4_ClassC.GetPropertyP("SomeProperty")));
        descriptor->GetAllFields().back()->SetName("ClassB_ClassC_SomeProperty");

        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b4_ClassB, *b4_ClassB.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", b4_ClassC, *b4_ClassC.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->SetName("ClassB_ClassC_Description");

        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_b4_ClassB_0", b4_ClassC, *b4_ClassC.GetPropertyP("B")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b4_ClassB, *query1), "this");
        query1->From(b4_ClassB, false, "this");
        query1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &b4_ClassC, *query2), "this");
        query2->From(b4_ClassC, false, "this");
        query2->Join(RelatedClass(b4_ClassC, b4_ClassB, b4_ClassBHasClassC, false, "nav_b4_ClassB_0", "nav_b4_ClassBHasClassC_0"));
        query2->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically", *query);
        }

    // SelectedNodeInstances_RemovesHiddenProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class3, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0")
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", sc_Class3, *sc_Class3.GetPropertyP("PropertyD")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_sc_Class1_0", sc_Class3, *sc_Class3.GetPropertyP("C1")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &sc_Class3, *query), "this");
        query->From(sc_Class3, false, "this");
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_RemovesHiddenProperty", *query);
        }

    // SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel
        {
        Utf8CP queryName = "SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel";
        ECClassCP selectClass = GetECClass(queryName, "PhysicalElement");

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(*selectClass, false));
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, selectClass, *query), "this");
        query->From(*selectClass, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        RegisterQuery("SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel", *query);
        }

    // SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel
        {
        Utf8CP queryName = "SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel";
        ECClassCP selectClass = GetECClass(queryName, "PhysicalElement");

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(*selectClass, false));
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", *selectClass , *selectClass->GetPropertyP("PhysicalProperty")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, selectClass, *query), "this");
        query->From(*selectClass, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        RegisterQuery("SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel", *query);
        }

    // SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel
        {
        Utf8CP queryName = "SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel";
        ECClassCP selectClass = GetECClass(queryName, "PhysicalElement");

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(*selectClass, false));
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, selectClass, *query), "this");
        query->From(*selectClass, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        RegisterQuery("SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel", *query);
        }

    // SelectedNodeInstances_RemovesMultipleHiddenProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class3, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0")
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_sc_Class1_0", sc_Class3, *sc_Class3.GetPropertyP("C1")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &sc_Class3, *query), "this");
        query->From(sc_Class3, false, "this");
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        RegisterQuery("SelectedNodeInstances_RemovesMultipleHiddenProperties", *query);
        }

    // SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class2, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class3, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_1", "nav_sc_Class1HasClass2And3_1")
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        field = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), "Class2_Class3_C1", "C1");
        field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("nav_sc_Class1_0", sc_Class2, *sc_Class2.GetPropertyP("C1")));
        field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("nav_sc_Class1_1", sc_Class3, *sc_Class3.GetPropertyP("C1")));
        descriptor->AddField(field);
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*descriptor->GetAllFields().back()->AsPropertiesField()));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", sc_Class3, *sc_Class3.GetPropertyP("PropertyD")));
                
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &sc_Class2, *query1), "this");
        query1->From(sc_Class2, false, "this");
        query1->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0"));
        query1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &sc_Class3, *query2), "this");
        query2->From(sc_Class3, false, "this");
        query2->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_1", "nav_sc_Class1HasClass2And3_1"));
        query2->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses", *query);
        }

    // SelectedNodeInstances_AddsSingleRelatedProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0"));
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsSingleRelatedProperty", *query);
        }

    // SelectedNodeInstances_AddsMultipleRelatedProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("LongProperty"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_LongProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget LongProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0"));
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsMultipleRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsAllRelatedProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"),RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")},
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(gadgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("MyID"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_MyID");
        descriptor->GetAllFields().back()->SetLabel("Gadget MyID");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget");
        descriptor->GetAllFields().back()->SetLabel("Gadget Widget");
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        query->Join({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            }, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsAllRelatedProperties", *query);
        }


    // SelectedNodeInstances_AddsBackwardRelatedProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class2, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "rel_sc_Class1_1", "rel_sc_Class1HasClass2And3_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", sc_Class2, *sc_Class2.GetPropertyP("PropertyB")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_sc_Class1_0", sc_Class2, *sc_Class2.GetPropertyP("C1")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto class1KeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(class1KeyField);

        field = &AddField(*descriptor, CreateCategory(sc_Class1), CreateProperty("rel_sc_Class1_1", sc_Class1, *sc_Class1.GetPropertyP("PropertyA"),
            RelatedClass(sc_Class1, sc_Class2, sc_Class1HasClass2And3, true, "rel_sc_Class2_0", "rel_sc_Class1HasClass2And3_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Class2_Class1_PropertyA");
        descriptor->GetAllFields().back()->SetLabel("Class1 PropertyA");
        class1KeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &sc_Class2, *query), "this");
        query->From(sc_Class2, false, "this");
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "rel_sc_Class1_1", "rel_sc_Class1HasClass2And3_0"));
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "nav_sc_Class1HasClass2And3_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsBackwardRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsBothDirectionsRelatedProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadget_0")},
            {RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprocket_0")},
            {
                RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprocket_0"),
                RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_2", "nav_RET_GadgetHasSprockets_0")
            }
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));

        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadget_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        auto sprocketKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(sprocketKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Sprocket), CreateProperty("nav_RET_Gadget_2", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget"),
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprocket_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Sprocket_Gadget");
        descriptor->GetAllFields().back()->SetLabel("Sprocket Gadget");
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        sprocketKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadget_0"));
        query->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprocket_0"));
        query->Join({
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprocket_0"),
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_2", "nav_RET_GadgetHasSprockets_0"),
            }, true);
        query->Join({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });


        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsBothDirectionsRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0")},
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_2", "rel_RET_WidgetHasGadget_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = new ContentDescriptor::ECPropertiesField(CreateCategory(ret_Widget), "Widget_Description", "Description");
        field->AsPropertiesField()->AddProperty(CreateProperty("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("Description"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"), RelationshipMeaning::RelatedInstance));
        field->AsPropertiesField()->AddProperty(CreateProperty("rel_RET_Widget_2", ret_Widget, *ret_Widget.GetPropertyP("Description"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->AddField(field);
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_Description");
        descriptor->GetAllFields().back()->SetLabel("Widget Description");
        widgetKeyField->AddKeyField(*descriptor->GetAllFields().back()->AsPropertiesField());

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0"));
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_2", "rel_RET_WidgetHasGadget_0"));
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships", *query);
        }

    // SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships2
        {
        RelatedClassPath nestedRelatedPropertiesPath1;
        nestedRelatedPropertiesPath1.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        nestedRelatedPropertiesPath1.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_1", "rel_RET_WidgetHasGadgets_0"));
        RelatedClassPath path1;
        path1.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        path1.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0"));
        
        RelatedClassPath nestedRelatedPropertiesPath2;
        nestedRelatedPropertiesPath2.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        nestedRelatedPropertiesPath2.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_1", "rel_RET_WidgetHasGadgets_0"));
        RelatedClassPath path2;
        path2.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        path2.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadget_0"));
        
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({ path1, path2 });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = new ContentDescriptor::ECPropertiesField(CreateCategory(ret_Widget), "Widget_IntProperty", "IntProperty");
        field->AsPropertiesField()->AddProperty(CreateProperty("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("IntProperty"),
            nestedRelatedPropertiesPath1, RelationshipMeaning::RelatedInstance));
        field->AsPropertiesField()->AddProperty(CreateProperty("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty"),
            nestedRelatedPropertiesPath2, RelationshipMeaning::RelatedInstance));
        descriptor->AddField(field);
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*descriptor->GetAllFields().back()->AsPropertiesField());

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(path1, true);
        query->Join(path2, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));

        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsNestedPropertiesOfTheSameClassFoundByFollowingDifferentRelationships", *query);
        }

    // SelectedNodeInstances_AddsNestedRelatedProperties
        {
        RelatedClassPath nestedRelatedPropertiesPath;
        nestedRelatedPropertiesPath.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        nestedRelatedPropertiesPath.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_1", "rel_RET_WidgetHasGadget_0"));
        RelatedClassPath relationshipPath;
        relationshipPath.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        relationshipPath.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadget_0"));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({ relationshipPath });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("Description"),
            nestedRelatedPropertiesPath, RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget_Description");
        descriptor->GetAllFields().back()->SetLabel("Widget Description");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(relationshipPath, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));

        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsNestedRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsNestedRelatedProperties2
        {
        RelatedClassPath gadgetRelatedPropertiesPath;
        gadgetRelatedPropertiesPath.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        RelatedClassPath gadgetRelationshipPath;
        gadgetRelationshipPath.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));

        RelatedClassPath widgetRelatedPropertiesPath;
        widgetRelatedPropertiesPath.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        widgetRelatedPropertiesPath.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_1", "rel_RET_WidgetHasGadget_0"));
        RelatedClassPath widgetRelationshipPath;
        widgetRelationshipPath.push_back(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        widgetRelationshipPath.push_back(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadget_0"));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({ widgetRelationshipPath, gadgetRelationshipPath });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(gadgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description"),
            gadgetRelatedPropertiesPath, RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("Description"),
            widgetRelatedPropertiesPath, RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget_Description");
        descriptor->GetAllFields().back()->SetLabel("Widget Description");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(gadgetRelationshipPath, true);
        query->Join(widgetRelationshipPath, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));

        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsNestedRelatedProperties2", *query);
        }

    // SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue   
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddContentFlag(ContentFlags::DistinctValues);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassH, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_ClassH, *ret_ClassH.GetPropertyP("PointProperty")));

        ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(1, *descriptor, &ret_ClassH, *nestedQuery);
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_ClassH, false, "this");
        nestedQuery->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr groupedQuery = ComplexContentQuery::Create();
        groupedQuery->SelectAll();
        groupedQuery->From(*nestedQuery);
        groupedQuery->GroupByContract(*contract);

        RegisterQuery("SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue", *groupedQuery);
        }

    //SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, { new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID") });
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));

        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        RegisterQuery("SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority", *query);
        }

    //SelectedNodeInstances_InstanceLabelOverride_OverrideSpecifiedClassInstancesLabelsWhenMultipleClassesSelected
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, {new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);        

        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Gadget_Widget_MyID");

        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);
        
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *q1), "this");
        q1->From(ret_Gadget, false, "this");
        q1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)1))});
        
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *q2), "this");
        q2->From(ret_Widget, false, "this");
        q2->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)2))});

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
        RegisterQuery("SelectedNodeInstances_InstanceLabelOverride_OverrideSpecifiedClassInstancesLabelsWhenMultipleClassesSelected", *query);
        }

    //SelectedNodeInstances_InstanceLabelOverride_OverrideNavigationProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, {new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);

        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Gadget_Widget_MyID");

        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);

        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *q1), "this");
        q1->From(ret_Gadget, false, "this");
        q1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        q1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)1))});

        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *q2), "this");
        q2->From(ret_Widget, false, "this");
        q2->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)2))});

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
        RegisterQuery("SelectedNodeInstances_InstanceLabelOverride_OverrideNavigationProperty", *query);
        }

    //SelectedNodeInstances_JoinsRelatedInstanceWithInnerJoin
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "sprocketAlias", "rel_RET_GadgetHasSprockets_0", true, false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedInstanceClasses({relatedInstanceClass});

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query, {relatedInstanceClass}), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Join(relatedInstanceClass);
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)1))});

        RegisterQuery("SelectedNodeInstances_JoinsRelatedInstanceWithInnerJoin", *query);
        }

    // SetsShowImagesFlag
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        descriptor->SetContentFlags((int)ContentFlags::ShowImages);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SetsShowImagesFlag", *query);
        }

    // SetsShowLabelsFlagForGridContentType
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);
        
        ComplexContentQueryPtr nested = ComplexContentQuery::Create();
        nested->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *nested), "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

#ifdef WIP_SORTING_GRID_CONTENT
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s), %s", ContentQueryContract::DisplayLabelFieldName, ContentQueryContract::ECInstanceIdFieldName).c_str());
        RegisterQuery("SetsShowLabelsFlagForGridContentType", *query);
#else
        RegisterQuery("SetsShowLabelsFlagForGridContentType", *nested);
#endif
        }

    // SetsNoFieldsAndKeysOnlyFlagForGraphicsContentType
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Graphics);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->SetContentFlags((int)ContentFlags::KeysOnly | (int)ContentFlags::NoFields);
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        RegisterQuery("SetsNoFieldsAndKeysOnlyFlagForGraphicsContentType", *query);
        }

    // SetsNoFieldsAndShowLabelsFlagsForListContentType
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::List);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels | (int)ContentFlags::NoFields);
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        
        ComplexContentQueryPtr nested = ComplexContentQuery::Create();
        nested->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *nested), "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

#ifdef WIP_SORTING_GRID_CONTENT
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s), %s", ContentQueryContract::DisplayLabelFieldName, ContentQueryContract::ECInstanceIdFieldName).c_str());
        RegisterQuery("SetsNoFieldsAndShowLabelsFlagsForListContentType", *query);
#else
        RegisterQuery("SetsNoFieldsAndShowLabelsFlagsForListContentType", *nested);
#endif
        }

    // AppliesRelatedInstanceSpecificationForTheSameXToManyRelationshipAndClassTwoTimes
        {
        RelatedClass relatedInstanceClass1(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "widgetAlias", "rel_RET_WidgetsHaveGadgets_0");
        RelatedClass relatedInstanceClass2(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "widgetAlias2", "rel_RET_WidgetsHaveGadgets_1");

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedInstanceClasses({relatedInstanceClass1, relatedInstanceClass2});

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query, {relatedInstanceClass1, relatedInstanceClass2}), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Join(relatedInstanceClass1);
        query->Join(relatedInstanceClass2);
        query->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t) 123))});

        RegisterQuery("AppliesRelatedInstanceSpecificationForTheSameXToManyRelationshipAndClassTwoTimes", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, true));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, true));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b2_Class2, *query), "this");
        query->From(b2_Class2, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        
        RegisterQuery("ContentInstacesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperty", *query);
        }

    //  ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, true));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));
        descriptor->AddField(new ContentDescriptor::CalculatedPropertyField("LabelTest_1", "CalculatedProperty_0", "\"Value\" & 1", nullptr, 1200));
        descriptor->AddField(new ContentDescriptor::CalculatedPropertyField("LabelTest_2", "CalculatedProperty_1", "this.Name & \"Test\"", nullptr, 1500));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b2_Class2, *query), "this");
        query->From(b2_Class2, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1B, false));
 
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));       
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1B, *b1_Class1B.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query1), "this");
        query1->From(b1_Class1A, false, "this");
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &b1_Class1B, *query2), "this");
        query2->From(b1_Class1B, false, "this");

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, false));
  
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));      
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query1), "this");
        query1->From(b1_Class1A, false, "this");
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &b2_Class2, *query2), "this");
        query2->From(b2_Class2, false, "this");

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses", *query);
        }

    // ContentInstancesOfSpecificClasses_AppliesInstanceFilter
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("[this].[Name] = 10", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_AppliesInstanceFilter", *query);
        }

    // ContentInstancesOfSpecificClasses_AppliesInstanceFilterUsingRelatedInstanceSpecification
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "sprocketAlias", "rel_RET_GadgetHasSprockets_0");

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedInstanceClasses({relatedInstanceClass});

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query, {relatedInstanceClass}), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Join(relatedInstanceClass);
        query->Where("[sprocketAlias].[MyID] = 'Sprocket MyID'", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_AppliesInstanceFilterUsingRelatedInstanceSpecification", *query);
        }

    // ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType1
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::PropertyPane);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        
        descriptor->AddContentFlag(ContentFlags::MergeResults);

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");

        RegisterQuery("ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType1", *query);
        }

    // ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType2
        {
        ContentDescriptorPtr innerDescriptor = GetEmptyContentDescriptor(ContentDisplayType::PropertyPane);
        
        innerDescriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*innerDescriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("Name")));
        AddField(*innerDescriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", b3_Class3, *b3_Class3.GetPropertyP("SomeProperty")));
    
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(1, *innerDescriptor, &b1_Class1A, *q1), "this");
        q1->From(b1_Class1A, false, "this");
    
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(2, *innerDescriptor, &b3_Class3, *q2), "this");
        q2->From(b3_Class3, false, "this");
    
        ContentDescriptorPtr outerDescriptor = ContentDescriptor::Create(*innerDescriptor);
        outerDescriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        outerDescriptor->GetSelectClasses().push_back(SelectClassInfo(b3_Class3, false));
        for (ContentDescriptor::Field* field : outerDescriptor->GetVisibleFields())
            {
            for (ContentDescriptor::Property const& fieldProperty : field->AsPropertiesField()->GetProperties())
                const_cast<ContentDescriptor::Property&>(fieldProperty).SetPrefix("");
            }
        outerDescriptor->AddContentFlag(ContentFlags::MergeResults); // note: inner descriptor doesn't have this flag!

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(0, *outerDescriptor, nullptr, *query)); // note: null for the class argument
        query->From(*UnionContentQuery::Create(*q1, *q2));

        RegisterQuery("ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType2", *query);
        }

     // ContentInstancesOfSpecificClasses_SelectPointPropertyRawDataGroupedByDisplayValue
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddContentFlag(ContentFlags::DistinctValues);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassH, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_ClassH, *ret_ClassH.GetPropertyP("PointProperty")));

        ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(1, *descriptor, &ret_ClassH, *nestedQuery);
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_ClassH, false, "this");

        ComplexContentQueryPtr groupedQuery = ComplexContentQuery::Create();
        groupedQuery->SelectAll();
        groupedQuery->From(*nestedQuery);
        groupedQuery->GroupByContract(*contract);

        RegisterQuery("ContentInstancesOfSpecificClasses_SelectPointPropertyRawDataGroupedByDisplayValue", *groupedQuery);
        }

     // ContentInstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, false, "this");

        RegisterQuery("ContentInstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority", *query);
        }

    //ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, {new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Gadget_Widget_MyID");
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);
        
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *q1), "this");
        q1->From(ret_Gadget, false, "this");
        
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *q2), "this");
        q2->From(ret_Widget, false, "this");

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
        RegisterQuery("ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", *query);
        }

    //ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideNavigationProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, {new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Gadget_MyID");
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        RegisterQuery("ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideNavigationProperty", *query);
        }

    // ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query1), "this");
        query1->From(ret_Sprocket, true, "this");
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false, false));
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *query2), "this");
        query2->From(ret_Sprocket, true, "this");
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", false)
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));        
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query1), "this");
        query1->From(ret_Widget, true, "this");
        query1->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", false, false));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *query2), "this");
        query2->From(ret_Widget, true, "this");
        query2->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", false, false));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query3 = ComplexContentQuery::Create();
        query3->SelectContract(*ContentQueryContract::Create(3, *descriptor, &ret_Widget, *query3), "this");
        query3->From(ret_Widget, true, "this");
        query3->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", false, false));
        query3->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query4 = ComplexContentQuery::Create();
        query4->SelectContract(*ContentQueryContract::Create(4, *descriptor, &ret_Widget, *query4), "this");
        query4->From(ret_Widget, true, "this");
        query4->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", false, false));
        query4->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*query1, *query2), *query3), *query4);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        field = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), "Widget_Sprocket_Description", "Description");
        field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        descriptor->AddField(field);
        field = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), "Widget_Sprocket_MyID", "MyID");
        field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        descriptor->AddField(field);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query1), "this");
        query1->From(ret_Widget, true, "this");
        query1->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", false, false));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *query2), "this");
        query2->From(ret_Widget, true, "this");
        query2->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", false, false));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query3 = ComplexContentQuery::Create();
        query3->SelectContract(*ContentQueryContract::Create(3, *descriptor, &ret_Widget, *query3), "this");
        query3->From(ret_Widget, true, "this");
        query3->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", false, false));
        query3->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query4 = ComplexContentQuery::Create();
        query4->SelectContract(*ContentQueryContract::Create(4, *descriptor, &ret_Widget, *query4), "this");
        query4->From(ret_Widget, true, "this");
        query4->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", false, false));
        query4->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query5 = ComplexContentQuery::Create();
        query5->SelectContract(*ContentQueryContract::Create(5, *descriptor, &ret_Sprocket, *query5), "this");
        query5->From(ret_Sprocket, true, "this");
        query5->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false, false));
        query5->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query5->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query6 = ComplexContentQuery::Create();
        query6->SelectContract(*ContentQueryContract::Create(6, *descriptor, &ret_Sprocket, *query6), "this");
        query6->From(ret_Sprocket, true, "this");
        query6->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query6->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query6->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*query1, *query2), *query3), *query4), *query5), *query6);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query1), "this");
        query1->From(ret_Sprocket, true, "this");
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false, false));
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query1->Where("[related].[ECInstanceId] IN (?,?)", {new BoundQueryId(ECInstanceId((uint64_t)123)), new BoundQueryId(ECInstanceId((uint64_t)125))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *query2), "this");
        query2->From(ret_Sprocket, true, "this");
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query2->Where("[related].[ECInstanceId] IN (?,?)", {new BoundQueryId(ECInstanceId((uint64_t)123)), new BoundQueryId(ECInstanceId((uint64_t)125))});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass", *query);
        }

    // ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true , "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
                        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query1), "this");
        query1->From(ret_Gadget, true, "this");
        query1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false, false));
        query1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Gadget, *query2), "this");
        query2->From(ret_Gadget, true, "this");
        query2->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true , "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query2->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)125)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses", *query);
        }

    // ContentRelatedInstances_AppliesInstanceFilter
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query1), "this");
        query1->From(ret_Sprocket, true, "this");
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false, false));
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        query1->Where("[this].[MyID] = 'Sprocket MyID'", BoundQueryValuesList());

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *query2), "this");
        query2->From(ret_Sprocket, true, "this");
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        query2->Where("[this].[MyID] = 'Sprocket MyID'", BoundQueryValuesList());

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_AppliesInstanceFilter", *query);
        }

        // ContentRelatedInstances_AppliesInstanceFilterUsingRelatedInstanceSpecification
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "sprocket", "rel_RET_GadgetHasSprockets_0");

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedInstanceClasses({relatedInstanceClass});

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query, {relatedInstanceClass}), "this");
        query->From(ret_Gadget, true, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));
        query->Join(relatedInstanceClass);
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", false, false));
        query->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        query->Where("[sprocket].[MyID] = 'Sprocket MyID'", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_AppliesInstanceFilterUsingRelatedInstanceSpecification", *query);
        }

    // ContentRelatedInstances_SkipsRelatedLevel
        {
        RelatedClassPath relationshipPath11 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false)
            };
        RelatedClassPath relationshipPath12 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0", false)
            };
        RelatedClassPath relationshipPath13 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", false)
            };
        RelatedClassPath relationshipPath14 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "gadget", "rel_RET_GadgetHasSprocket_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0", false)
            };
        RelatedClassPath relationshipPath21 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false)
            };
        RelatedClassPath relationshipPath22 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0", false)
            };
        RelatedClassPath relationshipPath23 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", false)
            };
        RelatedClassPath relationshipPath24 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0", false)
            };

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath11);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath12);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath13);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath14);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath21);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath22);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath23);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath24);
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr query11 = ComplexContentQuery::Create();
        query11->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query11), "this");
        query11->From(ret_Sprocket, true, "this");
        query11->Join(relationshipPath11, false);
        query11->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query11->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query12 = ComplexContentQuery::Create();
        query12->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *query12), "this");
        query12->From(ret_Sprocket, true, "this");
        query12->Join(relationshipPath12, false);
        query12->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query12->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query13 = ComplexContentQuery::Create();
        query13->SelectContract(*ContentQueryContract::Create(3, *descriptor, &ret_Sprocket, *query13), "this");
        query13->From(ret_Sprocket, true, "this");
        query13->Join(relationshipPath13, false);
        query13->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query13->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query14 = ComplexContentQuery::Create();
        query14->SelectContract(*ContentQueryContract::Create(4, *descriptor, &ret_Sprocket, *query14), "this");
        query14->From(ret_Sprocket, true, "this");
        query14->Join(relationshipPath14, false);
        query14->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query14->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query21 = ComplexContentQuery::Create();
        query21->SelectContract(*ContentQueryContract::Create(5, *descriptor, &ret_Sprocket, *query21), "this");
        query21->From(ret_Sprocket, true, "this");
        query21->Join(relationshipPath21, false);
        query21->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query21->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query22 = ComplexContentQuery::Create();
        query22->SelectContract(*ContentQueryContract::Create(6, *descriptor, &ret_Sprocket, *query22), "this");
        query22->From(ret_Sprocket, true, "this");
        query22->Join(relationshipPath22, false);
        query22->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query22->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query23 = ComplexContentQuery::Create();
        query23->SelectContract(*ContentQueryContract::Create(7, *descriptor, &ret_Sprocket, *query23), "this");
        query23->From(ret_Sprocket, true, "this");
        query23->Join(relationshipPath23, false);
        query23->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query23->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query24 = ComplexContentQuery::Create();
        query24->SelectContract(*ContentQueryContract::Create(8, *descriptor, &ret_Sprocket, *query24), "this");
        query24->From(ret_Sprocket, true, "this");
        query24->Join(relationshipPath24, false);
        query24->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query24->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});
        
        UnionContentQueryPtr query = UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(
            *UnionContentQuery::Create(*UnionContentQuery::Create(*query11, *query12), *query13), *query14), *query21), *query22), *query23), *query24);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_SkipsRelatedLevel", *query);
        }

    // ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship
        {
        RelatedClassPath relationshipPath1;
        relationshipPath1.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "gadget", "rel_RET_WidgetHasGadgets_0"));
        relationshipPath1.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprocket, true, "related", "rel_RET_GadgetHasSprocket_0", false));
        RelatedClassPath relationshipPath2;
        relationshipPath2.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "gadget", "rel_RET_WidgetHasGadgets_0"));
        relationshipPath2.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", false));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath1);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath2);
     
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));   
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query1), "this");
        query1->From(ret_Widget, true, "this");
        query1->Join(relationshipPath1, false);
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *query2), "this");
        query2->From(ret_Widget, true, "this");
        query2->Join(relationshipPath2, false);
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship", *query);
        }

    // ContentRelatedInstances_CreatesRecursiveQuery
        {
        ECClassCR class_Element = *GetECClass("ContentRelatedInstances_CreatesRecursiveQuery", "Element");
        ECRelationshipClassCR class_ElementOwnsChildElements = *GetECClass("ContentRelatedInstances_CreatesRecursiveQuery", "ElementOwnsChildElements")->GetRelationshipClassCP();

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, "related", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 0), false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0))
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", class_Element, *class_Element.GetPropertyP("ElementProperty")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property(TABLE_ALIAS("nav", class_Element, 0), class_Element, *class_Element.GetPropertyP("Parent")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        bvector<ECInstanceId> selectedIds;
        selectedIds.push_back(ECInstanceId((uint64_t)123));
        selectedIds.push_back(ECInstanceId((uint64_t)456));

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(&class_ElementOwnsChildElements);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &class_Element, *query), "this");
        query->From(class_Element, true, "this");
        query->Join(RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0)));
        query->Where("FALSE", BoundQueryValuesList()); // note: filtering by recursive children ids (there're no children in the dataset, so the result is just "FALSE"
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_CreatesRecursiveQuery", *query);
        }

    // ContentRelatedInstances_CreatesRecursiveQueryWhenRelationshipIsOnBaseClass
        {
        Utf8CP queryName = "ContentRelatedInstances_CreatesRecursiveQueryWhenRelationshipIsOnBaseClass";

        ECClassCR class_Element = *GetECClass(queryName, "Element");
        ECClassCR class_Sheet = *GetECClass(queryName, "Sheet");
        ECRelationshipClassCR class_ElementOwnsChildElements = *GetECClass(queryName, "ElementOwnsChildElements")->GetRelationshipClassCP();

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Sheet, class_ElementOwnsChildElements, false, "related", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 0), false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0))
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, "element", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 2), true),
            RelatedClass(class_Element, class_Sheet, class_ElementOwnsChildElements, false, "related", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 0), false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0))
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", class_Element, *class_Element.GetPropertyP("ElementProperty")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property(TABLE_ALIAS("nav", class_Element, 0), class_Element, *class_Element.GetPropertyP("Parent")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        bvector<ECInstanceId> selectedIds = {ECInstanceId((uint64_t)123)};

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(&class_ElementOwnsChildElements);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &class_Element, *query), "this");
        query->From(class_Element, true, "this");
        query->Join(RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0)));
        query->Where("FALSE", BoundQueryValuesList()); // note: filtering by recursive children ids (there're no children in the dataset, so the result is just "FALSE"
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery(queryName, *query);
        }

    // ContentRelatedInstances_DoesntSplitRecursiveQueryClassesIntoDerivedClasses
        {
        Utf8CP queryName = "ContentRelatedInstances_DoesntSplitRecursiveQueryClassesIntoDerivedClasses";

        ECClassCR class_Element = *GetECClass(queryName, "Element");
        ECClassCR class_Sheet = *GetECClass(queryName, "Sheet");
        ECRelationshipClassCR class_ElementOwnsChildElements = *GetECClass(queryName, "ElementOwnsChildElements")->GetRelationshipClassCP();

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Sheet, class_ElementOwnsChildElements, false, "related", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 0), false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0))
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, "element", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 2), true),
            RelatedClass(class_Element, class_Sheet, class_ElementOwnsChildElements, false, "related", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 0), false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0))
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", class_Element, *class_Element.GetPropertyP("ElementProperty")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property(TABLE_ALIAS("nav", class_Element, 0), class_Element, *class_Element.GetPropertyP("Parent")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        bvector<ECInstanceId> selectedIds = {ECInstanceId((uint64_t)123)};

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(&class_ElementOwnsChildElements);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &class_Element, *query), "this");
        query->From(class_Element, true, "this");
        query->Join(RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, TABLE_ALIAS("nav", class_Element, 0), TABLE_ALIAS("nav", class_ElementOwnsChildElements, 0)));
        query->Where("FALSE", BoundQueryValuesList()); // note: filtering by recursive children ids (there're no children in the dataset, so the result is just "FALSE"
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery(queryName, *query);
        }

    // ContentRelatedInstances_DoesntDuplicateRecursiveQueryClasses
        {
        Utf8CP queryName = "ContentRelatedInstances_DoesntDuplicateRecursiveQueryClasses";

        ECClassCR class_Element = *GetECClass(queryName, "Element");
        ECRelationshipClassCR class_ElementOwnsChildElements = *GetECClass(queryName, "ElementOwnsChildElements")->GetRelationshipClassCP();
        ECRelationshipClassCR class_ElementRefersToElements = *GetECClass(queryName, "ElementRefersToElements")->GetRelationshipClassCP();

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Element, class_ElementOwnsChildElements, false, "related", TABLE_ALIAS("rel", class_ElementOwnsChildElements, 0), false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(class_Element, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(class_Element, class_Element, class_ElementRefersToElements, false, "related", TABLE_ALIAS("rel", class_ElementRefersToElements, 0), false)
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        
        bvector<ECInstanceId> selectedIds = {ECInstanceId((uint64_t)123)};
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &class_Element, *query), "this");
        query->From(class_Element, true, "this");
        query->Where("FALSE", BoundQueryValuesList()); // note: filtering by recursive children ids (there're no children in the dataset, so the result is just "FALSE"
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery(queryName, *query);
        }

    // ContentRelatedInstances_SelectPointPropertyRawDataGroupedByDisplayValue
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddContentFlag(ContentFlags::DistinctValues);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassH, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_ClassH, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0", false)
            });
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_ClassH, *ret_ClassH.GetPropertyP("PointProperty")));

        ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(1, *descriptor, &ret_ClassH, *nestedQuery);
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_ClassH, true, "this");
        nestedQuery->Join(RelatedClass(ret_ClassH, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0", false, false));
        nestedQuery->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr groupedQuery = ComplexContentQuery::Create();
        groupedQuery->SelectAll();
        groupedQuery->From(*nestedQuery);
        groupedQuery->GroupByContract(*contract);

        RegisterQuery("ContentRelatedInstances_SelectPointPropertyRawDataGroupedByDisplayValue", *groupedQuery);
        }

    //ContentRelatedInstances_InstanceLabelOverride_AppliedByPriorityForSpecifiedClass
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false)
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        
        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Sprocket, {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query1), "this");
        query1->From(ret_Sprocket, false, "this");
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false, false));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *query2), "this");
        query2->From(ret_Sprocket, false, "this");
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
        RegisterQuery("ContentRelatedInstances_InstanceLabelOverride_AppliedByPriorityForSpecifiedClass", *query);
        }

    //ContentRelatedInstances_InstanceLabelOverride_OverrideNavigationProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Gadget, {new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query1), "this");
        query1->From(ret_Sprocket, true, "this");
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprocket, false, "related", "rel_RET_GadgetHasSprocket_0", false, false));
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query1->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *query2), "this");
        query2->From(ret_Sprocket, true, "this");
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false, false));
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        query2->Where("[related].[ECInstanceId] IN (?)", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
        RegisterQuery("ContentRelatedInstances_InstanceLabelOverride_OverrideNavigationProperty", *query);
        }

    // NestedContentField_WithSingleStepRelationshipPath
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("sprocket", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("sprocket", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "sprocket");
        query->From(ret_Sprocket, true, "sprocket");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "primary_instance", "rel", true, false));

        RegisterQuery("NestedContentField_WithSingleStepRelationshipPath", *query);
        }

    // NestedContentField_WithMultiStepRelationshipPath
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("sprocket", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("sprocket", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "sprocket");
        query->From(ret_Sprocket, true, "sprocket");

        RelatedClassPath path = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "intermediate", "rel_gs"), 
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "primary_instance", "rel_wg")
            };
        query->Join(path, false);

        RegisterQuery("NestedContentField_WithMultiStepRelationshipPath", *query);
        }

    // NestedContentField_WithNestedContentFields
        {
        ContentDescriptor::Category category("name", "label", "", 1);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, true));
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("gadget", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("gadget", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->AddField(new ContentDescriptor::NestedContentField(category, "sprocket_field_name", "sprocket_field_label", ret_Sprocket, "sprocket",
            {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget_instance", "rel_gs")
            }, 
            {
            new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(),
                ContentDescriptor::Property("sprocket_instance", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")))
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "gadget");
        query->From(ret_Gadget, true, "gadget");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget_instance", "rel_wg", true, false));

        RegisterQuery("NestedContentField_WithNestedContentFields", *query);
        }

    // FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Gadget_Widget_MyID");
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(widgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Widget), CreateProperty("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("MyID"),
            RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_MyID");
        descriptor->GetAllFields().back()->SetLabel("Widget MyID");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *q1), "this");
        q1->From(ret_Gadget, false, "this");
        q1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0"));
        q1->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)1)})});
        
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Widget, *q2), "this");
        q2->From(ret_Widget, false, "this");
        q2->Where("[this].[ECInstanceId] IN (?)", {new BoundQueryId({ECInstanceId((uint64_t)2)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass", *query);
        }

    // FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadget_0")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_1", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_2", "rel_RET_GadgetHasSprockets_0")}
            });

        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0));        
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->SetName("Widget_Sprocket_Description");

        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Widget_Sprocket_MyID");

        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(gadgetKeyField);

        field = new ContentDescriptor::ECPropertiesField(CreateCategory(ret_Gadget), "", "");
        field->AsPropertiesField()->AddProperty(CreateProperty("rel_RET_Gadget_0", ret_Gadget, *ret_Gadget.GetPropertyP("MyID"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadget_0"), RelationshipMeaning::RelatedInstance));
        field->AsPropertiesField()->AddProperty(CreateProperty("rel_RET_Gadget_2", ret_Gadget, *ret_Gadget.GetPropertyP("MyID"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->AddField(field);
        descriptor->GetAllFields().back()->SetName("rel_Widget_Sprocket_Gadget_MyID");
        descriptor->GetAllFields().back()->SetLabel("Gadget MyID");
        gadgetKeyField->AddKeyField(*descriptor->GetAllFields().back()->AsPropertiesField());
        
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_1", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));
        
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *q1), "this");
        q1->From(ret_Widget, false, "this");
        q1->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadget_0"));
        
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &ret_Sprocket, *q2), "this");
        q2->From(ret_Sprocket, false, "this");
        q2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_2", "rel_RET_GadgetHasSprockets_0"));
        q2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_1", "nav_RET_GadgetHasSprockets_0"));

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty", *query);
        }

    // AppliesRelatedPropertiesSpecificationFromContentModifier
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(gadgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("AppliesRelatedPropertiesSpecificationFromContentModifier", *query);
        }

    // DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(gadgetKeyField);

        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses", *query);
        }

    // RelatedPropertiesAreAppendedCorrectlyWhenUsingCustomDescriptor
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0")
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")}
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->AddField(gadgetKeyField);
        
        field = &AddField(*descriptor, CreateCategory(ret_Gadget), CreateProperty("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description"),
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"), RelationshipMeaning::RelatedInstance));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"));
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "nav_RET_GadgetHasSprockets_0"));
        
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("RelatedPropertiesAreAppendedCorrectlyWhenUsingCustomDescriptor", *query);
        }

    // CreatesContentFieldsForXToManyRelatedInstanceProperties
        {
        ContentDescriptor::Category sprocketCategory(ret_Sprocket.GetName(), ret_Sprocket.GetDisplayLabel(), ret_Sprocket.GetDescription(), 1000);
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetNavigationPropertyClasses({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0")
            });
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->AddField(new ContentDescriptor::ECNavigationInstanceIdField(*field->AsPropertiesField()));

        descriptor->AddField(new ContentDescriptor::NestedContentField(sprocketCategory, "Gadget_Sprocket", "Sprocket", ret_Sprocket, "rel_RET_Sprocket_0",
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_0", "rel_RET_GadgetHasSprockets_0")}, 
            {
            new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(),
                ContentDescriptor::Property("rel_RET_Sprocket_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")))
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "nav_RET_WidgetHasGadgets_0"));

#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("CreatesContentFieldsForXToManyRelatedInstanceProperties", *query);
        }

    // CreatesNestedContentFieldsForXToManyRelatedInstanceProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        descriptor->AddField(new ContentDescriptor::NestedContentField(CreateCategory(ret_Gadget), "Widget_Gadget", "Gadget", ret_Gadget, "rel_RET_Gadget_0", 
            {
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0")
            }, 
            {
            new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(),
                ContentDescriptor::Property("rel_RET_Gadget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Description"))),
            new ContentDescriptor::NestedContentField(CreateCategory(ret_Sprocket), "Gadget_Sprocket", "Sprocket", ret_Sprocket, "rel_RET_Sprocket_0",
                {
                RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_0", "rel_RET_GadgetHasSprockets_0")
                }, 
                {
                new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(),
                    ContentDescriptor::Property("rel_RET_Sprocket_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")))
                })
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("CreatesNestedContentFieldsForXToManyRelatedInstanceProperties", *query);
        }

    // CreatesNestedContentFieldsForXToManySameInstanceProperties
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        descriptor->AddField(new ContentDescriptor::NestedContentField(CreateCategory(ret_Gadget), "Widget_Gadget", "Gadget", ret_Gadget, "rel_RET_Gadget_0", 
            {
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0")
            }, 
            {
            new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(),
                ContentDescriptor::Property("rel_RET_Gadget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Description"))),
            new ContentDescriptor::NestedContentField(ContentDescriptor::Category(), "Gadget_Sprocket", "Sprocket", ret_Sprocket, "rel_RET_Sprocket_0",
                {
                RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_0", "rel_RET_GadgetHasSprockets_0")
                }, 
                {
                new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(),
                    ContentDescriptor::Property("rel_RET_Sprocket_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")))
                })
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("CreatesNestedContentFieldsForXToManySameInstanceProperties", *query);
        }

    // FilterExpressionQueryTest
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->SetFilterExpression("Widget_MyID = \"WidgetId\"");
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));
        
        descriptor->AddField(new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()), 0)); 
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));

        ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
        nestedQuery->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *nestedQuery), "this");
        nestedQuery->From(ret_Widget, false, "this");

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectAll();
        query->From(*nestedQuery, "");
        query->Where("([Widget_MyID] = 'WidgetId')", BoundQueryValuesList());
        RegisterQuery("FilterExpressionQueryTest", *query);
        }

    //InstanceLabelOverride_AppliedByPriority
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField("", 1000);
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverrideMap;
        labelOverrideMap.Insert(&ret_Widget, {new InstanceLabelOverridePropertyValueSpecification("Description"), new InstanceLabelOverridePropertyValueSpecification("MyID")});
        displayLabelField->SetOverrideValueSpecs(labelOverrideMap);
        descriptor->AddField(displayLabelField);
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        AddField(*descriptor, ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(1, *descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, false, "this");
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        RegisterQuery("InstanceLabelOverride_AppliedByPriority", *query);
        }

        Localization::Terminate();
    }

#define LOGD(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->debugv(__VA_ARGS__)
#define LOGI(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->infov(__VA_ARGS__)
#define LOGW(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->warningv(__VA_ARGS__)
#define LOGE(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->errorv(__VA_ARGS__)

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExecuteQueries(bmap<Utf8String, NavigationQueryCPtr> queries, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, 
    IUserSettings const& userSettings)
    {
    ECDbR db = connection.GetECDb();
    for (auto pair : queries)
        {
        Utf8String const& name = pair.first;
        NavigationQueryCPtr query = pair.second;
        LOGI("---");
        LOGI("Query: '%s'", name.c_str());

        JsonNavNodesFactory nodesFactory;
        ECSchemaHelper schemaHelper(connection, nullptr, nullptr, nullptr);
        CustomFunctionsContext functionsContext(schemaHelper, connections, connection, ruleset, "locale", userSettings,
            nullptr, schemaHelper.GetECExpressionsCache(), nodesFactory, nullptr, nullptr, &query->GetExtendedData());
        
        Utf8String queryStr = query->ToString();
        ECSqlStatement statement;
        ECSqlStatus status = statement.Prepare(db, queryStr.c_str());
        EXPECT_EQ(ECSqlStatus::Success, status)
            << "Failure: " << db.GetLastError() << "\r\n"
            << "Failed query: " << name.c_str() << "\r\n" << queryStr.c_str();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExecuteQueries(bmap<Utf8String, ContentQueryCPtr> queries, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, 
    IUserSettings const& userSettings)
    {
    ECDbR db = connection.GetECDb();
    for (auto pair : queries)
        {
        Utf8String const& name = pair.first;

        ContentQueryCPtr query = pair.second;
        LOGI("---");
        LOGI("Query: '%s'", name.c_str());
        
        JsonNavNodesFactory nodesFactory;
        ECSchemaHelper schemaHelper(connection, nullptr, nullptr, nullptr);
        CustomFunctionsContext functionsContext(schemaHelper, connections, connection, ruleset, "locale", userSettings,
            nullptr, schemaHelper.GetECExpressionsCache(), nodesFactory, nullptr, nullptr, nullptr);
        
        Utf8String queryStr = query->ToString();
        ECSqlStatement statement;
        ECSqlStatus status = statement.Prepare(db, queryStr.c_str());
        EXPECT_EQ(ECSqlStatus::Success, status)
            << "Failure: " << db.GetLastError() << "\r\n"
            << "Failed query: " << name.c_str() << "\r\n" << queryStr.c_str();
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ExpectedQueriesTest, RunAllExpectedQueries)
    {
    if (!NativeLogging::LoggingConfig::IsProviderActive())
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity("ExpectedQueriesTest", NativeLogging::LOG_DEBUG);

    Localization::Init();

    IConnectionManagerCR connections = ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnections();
    IConnectionCR connection = ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnection();
    // unused - ECDbR db = connection.GetECDb();
    TestUserSettings userSettings;
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    CustomFunctionsInjector customFunctions(connections, connection);

    ExecuteQueries(ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQueries(), connections, connection, *ruleset, userSettings);
    ExecuteQueries(ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQueries(), connections, connection, *ruleset, userSettings);
    }
