/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ExpectedQueriesTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ExpectedQueries.h"
#include "TestHelpers.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomFunctions.h"
#include "../../../Source/RulesDriven/RulesEngine/ECSchemaHelper.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryBuilder.h"
#include <Logging/bentleylogging.h>

ExpectedQueries* ExpectedQueries::s_instance = nullptr;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpectedQueries& ExpectedQueries::GetInstance(BeTest::Host& host)
    {
    if (nullptr == s_instance)
        s_instance = new ExpectedQueries(host);
    return *s_instance;
    }
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
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationId(spec.GetId());
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
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& ExpectedQueries::AddField(ContentDescriptorR descriptor, ECClassCR primaryClass, ContentDescriptor::Property prop)
    {
    return RulesEngineTestHelpers::AddField(descriptor, primaryClass, prop, m_categorySupplier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::PrepareSchemaContext()
    {
    m_project.Create("ExpectedQueries", "RulesEngineTest.01.00.ecschema.xml");

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

    bvector<ECSchemaCP> importSchemas;
    importSchemas.resize(schemas.size());
    std::transform(schemas.begin(), schemas.end(), importSchemas.begin(), [](ECSchemaPtr const& schema){return schema.get();});

    ASSERT_TRUE(SUCCESS == m_project.GetECDb().Schemas().ImportSchemas(importSchemas));
    m_project.GetECDb().SaveChanges();

    m_schemaHelper = new ECSchemaHelper(m_project.GetECDbCR(), m_relatedPathsCache, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectedQueries::RegisterExpectedQueries()
    {
    static Utf8PrintfString ecInstanceNodesQuerySortedDisplayLabel("%s(%s)", FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName);
    static Utf8PrintfString ecClassGroupingNodesQuerySortedDisplayLabel("%s(%s)", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);
    static Utf8PrintfString baseClassGroupingNodesQuerySortedDisplayLabel("%s(%s)", FUNCTION_NAME_GetSortingValue, BaseECClassGroupingNodesQueryContract::DisplayLabelFieldName);
    static Utf8PrintfString labelGroupingQuerySortedDisplayLabel("%s(%s)", FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName);

    static Utf8PrintfString ecClassPriority("%s(%s)", FUNCTION_NAME_GetECClassPriority, ECClassGroupingNodesQueryContract::ECClassIdFieldName);
    static Utf8PrintfString ecClassGroupingNodesOrderByClause("%s DESC, %s", ecClassPriority.c_str(), ecClassGroupingNodesQuerySortedDisplayLabel.c_str());

    static Utf8PrintfString baseClassPriority("%s(%s)", FUNCTION_NAME_GetECClassPriority, BaseECClassGroupingNodesQueryContract::BaseClassIdFieldName);
    static Utf8PrintfString baseClassGroupingNodesOrderByClause("%s DESC, %s", baseClassPriority.c_str(), baseClassGroupingNodesQuerySortedDisplayLabel.c_str());

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
    ECEntityClassR ret_ClassA2Base = *GetECClassP("RulesEngineTest", "ClassA2Base")->GetEntityClassP();
    ECEntityClassR ret_BaseOfBAndC = *GetECClassP("RulesEngineTest", "BaseOfBAndC")->GetEntityClassP();
    ECEntityClassR ret_ClassB = *GetECClassP("RulesEngineTest", "ClassB")->GetEntityClassP();
    ECEntityClassR ret_ClassB2 = *GetECClassP("RulesEngineTest", "ClassB2")->GetEntityClassP();
    ECEntityClassR ret_ClassC = *GetECClassP("RulesEngineTest", "ClassC")->GetEntityClassP();
    ECRelationshipClassR ret_ClassAHasBAndC = *GetECClassP("RulesEngineTest", "ClassAHasBAndC")->GetRelationshipClassP();
    ECEntityClassR ret_ClassD = *GetECClassP("RulesEngineTest", "ClassD")->GetEntityClassP();
    ECEntityClassR ret_ClassE = *GetECClassP("RulesEngineTest", "ClassE")->GetEntityClassP();
    ECRelationshipClassR ret_ClassDHasClassE = *GetECClassP("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassP();
    ECRelationshipClassR ret_ClassDReferencesClassE = *GetECClassP("RulesEngineTest", "ClassDReferencesClassE")->GetRelationshipClassP();
    ECEntityClassR ret_ClassF = *GetECClassP("RulesEngineTest", "ClassF")->GetEntityClassP();
    ECEntityClassR ret_ClassG = *GetECClassP("RulesEngineTest", "ClassG")->GetEntityClassP();
    ECEntityClassR ret_ClassI = *GetECClassP("RulesEngineTest", "ClassI")->GetEntityClassP();
    ECEntityClassR ret_ClassJ = *GetECClassP("RulesEngineTest", "ClassJ")->GetEntityClassP();
    ECEntityClassR ret_ClassK = *GetECClassP("RulesEngineTest", "ClassK")->GetEntityClassP();
    ECEntityClassR ret_ClassL = *GetECClassP("RulesEngineTest", "ClassL")->GetEntityClassP();
    ECEntityClassR ret_ClassM = *GetECClassP("RulesEngineTest", "ClassM")->GetEntityClassP();
    ECRelationshipClassR ret_ClassJHasClassK = *GetECClassP("RulesEngineTest", "ClassJHasClassK")->GetRelationshipClassP();
    ECRelationshipClassR ret_ClassJHasClassL = *GetECClassP("RulesEngineTest", "ClassJHasClassL")->GetRelationshipClassP();
    ECRelationshipClassR ret_ClassLHasClassM = *GetECClassP("RulesEngineTest", "ClassLHasClassM")->GetRelationshipClassP();
    ECEntityClassR ret_ClassN = *GetECClassP("RulesEngineTest", "ClassN")->GetEntityClassP();
    ECEntityClassR ret_ClassO = *GetECClassP("RulesEngineTest", "ClassO")->GetEntityClassP();
    ECEntityClassR ret_ClassP = *GetECClassP("RulesEngineTest", "ClassP")->GetEntityClassP();
    ECRelationshipClassR ret_ClassNGroupsClassN = *GetECClassP("RulesEngineTest", "ClassNGroupsClassN")->GetRelationshipClassP();
    ECEntityClassR ret_ClassQ = *GetECClassP("RulesEngineTest", "ClassQ")->GetEntityClassP();
    ECEntityClassR ret_ClassR = *GetECClassP("RulesEngineTest", "ClassR")->GetEntityClassP();
    ECEntityClassR ret_ClassS = *GetECClassP("RulesEngineTest", "ClassS")->GetEntityClassP();
    ECEntityClassR ret_ClassT = *GetECClassP("RulesEngineTest", "ClassT")->GetEntityClassP();
    ECEntityClassR ret_ClassU = *GetECClassP("RulesEngineTest", "ClassU")->GetEntityClassP();

    ECEntityClassR ret_Widget = *GetECClassP("RulesEngineTest", "Widget")->GetEntityClassP();
    ECEntityClassR ret_Gadget = *GetECClassP("RulesEngineTest", "Gadget")->GetEntityClassP();
    ECEntityClassR ret_Sprocket = *GetECClassP("RulesEngineTest", "Sprocket")->GetEntityClassP();
    ECRelationshipClassR ret_WidgetHasGadget = *GetECClassP("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassP();
    ECRelationshipClassR ret_WidgetHasGadgets = *GetECClassP("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassP();
    ECRelationshipClassR ret_WidgetsHaveGadgets = *GetECClassP("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassP();
    ECRelationshipClassR ret_WidgetsHaveGadgets2 = *GetECClassP("RulesEngineTest", "WidgetsHaveGadgets2")->GetRelationshipClassP();
    ECRelationshipClassR ret_GadgetHasSprockets = *GetECClassP("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassP();

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
        classes[&b4_ClassA] = true;
        classes[&ret_Widget] = true;
        classes[&ret_Gadget] = true;
        classes[&ret_Sprocket] = true;
        classes[&ret_ClassA] = true;
        classes[&ret_ClassA2Base] = true;
        classes[&ret_ClassB2] = true;
        classes[&ret_BaseOfBAndC] = true;
        classes[&ret_ClassD] = true;
        classes[&ret_ClassE] = true;
        classes[&ret_ClassI] = true;
        classes[&ret_ClassJ] = true;
        classes[&ret_ClassK] = true;
        classes[&ret_ClassN] = true;
        classes[&ret_ClassO] = true;
        classes[&ret_ClassP] = true;
        classes[&ret_ClassQ] = true;
        classes[&ret_ClassR] = true;
        classes[&ret_ClassS] = true;
        classes[&ret_ClassT] = true;
        classes[&ret_ClassU] = true;
        
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
            query.Where("InVirtualSet(?, [this].[ECInstanceId]) AND "
                "GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[DisplayLabel], '[]') = ?",
                {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("MyLabel"))});
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
        nested->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[DisplayLabel], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});

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
        
    // AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
        
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this");
        nestedQuery1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, true, "this");
        nestedQuery2->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Gadget, true, "this");
        nestedQuery3->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Gadget, true, "this");
        nestedQuery4->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
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
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
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
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery5->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5));
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadget.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetsHaveGadgets2.GetId());
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_NoGrouping_BothDirections", *expected);
        }

    // RelatedInstanceNodes_SkipOneRelatedLevel_WidgetToSprocket
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
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*contract, "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery5->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectAll();
        nested->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5));
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
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByClass", *expected);
        }

    // AllRelatedInstanceNodes_GroupByClass_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
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
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget"), false);
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
        nestedQuery1->From(ret_ClassB, true, "this").Join(RelatedClass(ret_ClassB, ret_ClassA, ret_ClassAHasBAndC, false, "related", "rel_RET_ClassAHasBAndC"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassC), "this");
        nestedQuery2->From(ret_ClassC, true, "this").Join(RelatedClass(ret_ClassC, ret_ClassA, ret_ClassAHasBAndC, false, "related", "rel_RET_ClassAHasBAndC"), false);
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
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget), "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery5 = ComplexNavigationQuery::Create();
        nestedQuery5->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery5->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery5->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*UnionNavigationQuery::Create(*nestedQuery1, *nestedQuery2), *nestedQuery3), *nestedQuery4), *nestedQuery5));
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
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByLabel", *expected);
        }

    // AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);

        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("MyLabel"))});

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery", *expected);
        }

    // AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery
        {
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(&ret_Widget);
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
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
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery1->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[MyID], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery2->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[MyID], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});
        
        ComplexNavigationQueryPtr nestedQuery3 = ComplexNavigationQuery::Create();
        nestedQuery3->SelectContract(*contract, "this");
        nestedQuery3->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0"), false);
        nestedQuery3->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery3->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[MyID], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});
        
        ComplexNavigationQueryPtr nestedQuery4 = ComplexNavigationQuery::Create();
        nestedQuery4->SelectContract(*contract, "this");
        nestedQuery4->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0"), false);
        nestedQuery4->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery4->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[MyID], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});

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
        nestedQuery->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget"), false);
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
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget"), false);
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
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget"), false);
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
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("MyLabel"))});

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
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});

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
        nestedQuery1->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_Sprocket), "this");
        nestedQuery2->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0"), false);
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
        nestedQuery->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
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
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Gadget.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_OnlyIncluded", *expected);
        }

    // RelatedInstanceNodes_RelatedClassesOverridesSupportedSchemas
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Sprocket);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Sprocket, true, "this").Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0"), false);
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
        nestedQuery->From(ret_Widget, true, "this").Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0"), false);
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
    
        ComplexNavigationQueryPtr includeQuery = ComplexNavigationQuery::Create();
        includeQuery->SelectContract(*contract, "this");
        includeQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0"), false);
        includeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr orderedInclude = ComplexNavigationQuery::Create();
        orderedInclude->SelectAll();
        orderedInclude->From(*includeQuery);
        orderedInclude->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
    
        ComplexNavigationQueryPtr excludeQuery = ComplexNavigationQuery::Create();
        excludeQuery->SelectContract(*contract, "this");
        excludeQuery->From(ret_Gadget, false, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_1", false), false);
        excludeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ExceptNavigationQueryPtr expected = ExceptNavigationQuery::Create(ComplexNavigationQuery::Create()->SelectAll().From(*orderedInclude), *excludeQuery);
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_AllDerivedClasses", *expected);
        }

    // RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*contract, "this");
        nestedQuery1->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*contract, "this");
        nestedQuery2->From(ret_Gadget, false, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_1", false), false);
        nestedQuery2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*ExceptNavigationQuery::Create(*nestedQuery1, *nestedQuery2));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy(ecClassGroupingNodesOrderByClause.c_str());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded", *expected);        
        }

    // RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_ClassE);
    
        ComplexNavigationQueryPtr includeQuery = ComplexNavigationQuery::Create();
        includeQuery->SelectContract(*contract, "this");
        includeQuery->From(ret_ClassE, true, "this").Join(RelatedClass(ret_ClassE, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0"), false);
        includeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});

        ComplexNavigationQueryPtr orderedInclude = ComplexNavigationQuery::Create();
        orderedInclude->SelectAll();
        orderedInclude->From(*includeQuery);
        orderedInclude->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
    
        ComplexNavigationQueryPtr excludeQuery1 = ComplexNavigationQuery::Create();
        excludeQuery1->SelectContract(*contract, "this");
        excludeQuery1->From(ret_ClassF, true, "this").Join(RelatedClass(ret_ClassF, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_4"), false);
        excludeQuery1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr excludeQuery2 = ComplexNavigationQuery::Create();
        excludeQuery2->SelectContract(*contract, "this");
        excludeQuery2->From(ret_ClassG, false, "this").Join(RelatedClass(ret_ClassG, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_5", false), false);
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
        includeQuery->From(ret_ClassE, false, "this").Join(RelatedClass(ret_ClassE, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0"), false);
        includeQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        includeQuery->OrderBy("[this].[IntProperty]");

        ComplexNavigationQueryPtr wrappedInclude = ComplexNavigationQuery::Create();
        wrappedInclude->SelectAll();
        wrappedInclude->From(*includeQuery);

        ComplexNavigationQueryPtr unorderedInclude = ComplexNavigationQuery::Create();
        unorderedInclude->SelectContract(*contract, "this");
        unorderedInclude->From(ret_ClassG, true, "this").Join(RelatedClass(ret_ClassG, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_0"), false);
        unorderedInclude->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
    
        UnionNavigationQueryPtr includes = UnionNavigationQuery::Create(*wrappedInclude, *unorderedInclude);

        ComplexNavigationQueryPtr excludeQuery = ComplexNavigationQuery::Create();
        excludeQuery->SelectContract(*contract, "this");
        excludeQuery->From(ret_ClassF, true, "this").Join(RelatedClass(ret_ClassF, ret_ClassD, ret_ClassDHasClassE, false, "related", "rel_RET_ClassDHasClassE_4"), false);
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
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("[this].[Description] = '2'", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter", *expected);
        }

    // RelatedInstanceNodes_InstanceFilter_LikeOperator
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this").Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("[this].[Description] LIKE 'Test'", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter_LikeOperator", *expected);
        }

    // RelatedInstanceNodes_InstanceFilter_IsOfClassFunction
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget);
    
        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(ret_Gadget, true, "this");
        nestedQuery->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0"), false);
        nestedQuery->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        nestedQuery->Where("IsOfClass([this].[ECClassId], 'ClassName', 'SchemaName')", BoundQueryValuesList(), true);
    
        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetParentECClassId(ret_Sprocket.GetId());
        expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        expected->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_GadgetHasSprockets.GetId());

        RegisterQuery("RelatedInstanceNodes_InstanceFilter_IsOfClassFunction", *expected);
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
        nestedQuery1->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[DisplayLabel], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("MyLabel"))});

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this");
        nestedQuery2->From(b1_Class1B, false, "this");
        nestedQuery2->Where("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[DisplayLabel], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("MyLabel"))});

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
        nestedQuery->Where("InVirtualSet(?, [this].[ECInstanceId]) AND "
            "GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], [this].[DisplayLabel], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});

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
        nestedQuery1->Where("[this].[DisplayLabel] = 2", BoundQueryValuesList());

        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*ECInstanceNodesQueryContract::Create(&b1_Class1B), "this");
        nestedQuery2->From(b1_Class1B, false, "this");
        nestedQuery2->Where("[this].[DisplayLabel] = 2", BoundQueryValuesList());

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
        nestedQuery->Where("[this].[Name] LIKE 'Test'", BoundQueryValuesList());

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
        nestedQuery->Where("[this].[DisplayLabel] = [parent].[Name]", BoundQueryValuesList(), true);

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
        nestedQuery->Where("[this].[DisplayLabel] = [parent_parent].[Name]", BoundQueryValuesList(), true);

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
        nestedQuery->Where("[this].[DisplayLabel] = [parent_parent].[Name] OR [this].[DisplayLabel] = [parent].[Name]", BoundQueryValuesList(), true);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*nestedQuery);
        expected->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());

        RegisterQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance", *expected);
        }

    // SearchResultInstanceNodes_NoGrouping
        {
        StringNavigationQueryPtr searchQuery = StringNavigationQuery::Create(SEARCH_NODE_QUERY_PROCESSED);

        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(&ret_Widget);
        contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
        contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);

        ComplexNavigationQueryPtr instancesQuery = ComplexNavigationQuery::Create();
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");

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
        query->SelectContract(*contract, "searchQuery");
        query->From(*searchQuery, "searchQuery");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr expected = ComplexNavigationQuery::Create();
        expected->SelectAll();
        expected->From(*grouped);
        expected->OrderBy("GetECClassPriority(" SEARCH_QUERY_FIELD_ECClassId ") DESC, GetSortingValue(DisplayLabel)");
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
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");
        instancesQuery->Where("[searchQuery].[" SEARCH_QUERY_FIELD_ECClassId "] = ?", {new BoundQueryId(ret_Widget.GetId())});

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
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");

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
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");
        instancesQuery->Where("InVirtualSet(?, [searchQuery].[" SEARCH_QUERY_FIELD_ECInstanceId "]) "
            "AND GetECInstanceDisplayLabel([searchQuery].[" SEARCH_QUERY_FIELD_ECClassId "], [searchQuery].[" SEARCH_QUERY_FIELD_ECInstanceId "], [searchQuery].[MyID], '[]') = ?",
            {new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("MyLabel"))});

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
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");
        instancesQuery->Where("[searchQuery].[" SEARCH_QUERY_FIELD_ECClassId "] = ?", {new BoundQueryId(ret_Widget.GetId())});

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
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");
        instancesQuery->Where("[searchQuery].[" SEARCH_QUERY_FIELD_ECClassId "] = ?"
            " AND InVirtualSet(?, [searchQuery].[" SEARCH_QUERY_FIELD_ECInstanceId "])"
            " AND GetECInstanceDisplayLabel([searchQuery].[" SEARCH_QUERY_FIELD_ECClassId "], [searchQuery].[" SEARCH_QUERY_FIELD_ECInstanceId "], [searchQuery].[MyID], '[]') = ?",
            {new BoundQueryId(ret_Widget.GetId()), new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), new BoundQueryECValue(ECValue("Label Grouping Node"))});

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
        instancesQuery->SelectContract(*contract, "searchQuery");
        instancesQuery->From(*searchQuery, "searchQuery");

        ComplexNavigationQueryPtr ordered = ComplexNavigationQuery::Create();
        ordered->SelectAll();
        ordered->From(*instancesQuery);
        ordered->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        
        RegisterQuery("SearchResultInstanceNodes_UsesParentPropertyValueQuery", *ordered);
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
        expected->Join(relatedInstanceClass, true);
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
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_GroupsByProperty", *grouped);
        }

    // Grouping_PropertyGroup_GroupsMultipleClassesByProperty
        {
        PropertyGroupP spec = new PropertyGroup();
        RegisterForDelete(*spec);

        NavigationQueryContractPtr contract1 = ECPropertyGroupingNodesQueryContract::Create(b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*contract1, "this");
        nested1->From(b1_Class1A, true, "this");
        ComplexNavigationQueryPtr grouped1 = ComplexNavigationQuery::Create();
        grouped1->SelectAll().From(*nested1).GroupByContract(*contract1);
        
        NavigationQueryContractPtr contract2 = ECPropertyGroupingNodesQueryContract::Create(b1_Class1B, *b1_Class1B.GetPropertyP("DisplayLabel"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*contract2, "this");
        nested2->From(b1_Class1B, true, "this");
        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectAll().From(*nested2).GroupByContract(*contract2);
                
        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*grouped2, *grouped1);
        unionQuery->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        unionQuery->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

        RegisterQuery("Grouping_PropertyGroup_GroupsMultipleClassesByProperty", *unionQuery);
        }

    // Grouping_PropertyGroup_GroupsByRange
        {
        PropertyGroupP spec = new PropertyGroup("", "", false, "DisplayLabel", "");
        spec->GetRangesR().push_back(new PropertyRangeGroupSpecification("", "", "0", "5"));
        spec->GetRangesR().push_back(new PropertyRangeGroupSpecification("", "", "6", "10"));
        spec->GetRangesR().push_back(new PropertyRangeGroupSpecification("", "", "11", "20"));
        RegisterForDelete(*spec);

        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel"), nullptr, *spec, nullptr);
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll().From(*nested).GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        
        RegisterQuery("Grouping_PropertyGroup_OverridesImageId", *grouped);
        }

    // Grouping_PropertyGroup_ValueFiltering
        {
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&b1_Class1A);
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("[this].[DisplayLabel] = ?", {new BoundQueryECValue(ECValue(9))});

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
        nested->Where("[this].[DisplayLabel] BETWEEN ? AND ?", {new BoundQueryECValue(ECValue(1)), new BoundQueryECValue(ECValue(5))});

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
        nested->Where("[this].[DisplayLabel] NOT BETWEEN ? AND ?", {new BoundQueryECValue(ECValue(1)), new BoundQueryECValue(ECValue(5))});
        nested->Where("[this].[DisplayLabel] NOT BETWEEN ? AND ?", {new BoundQueryECValue(ECValue(7)), new BoundQueryECValue(ECValue(9))});
        nested->Where("[this].[DisplayLabel] NOT BETWEEN ? AND ?", {new BoundQueryECValue(ECValue(10)), new BoundQueryECValue(ECValue(15))});

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
        nested1->Join(RelatedClass(sc3_ChildClassWithNavigationProperty, sc3_GroupingClass, sc3_NavigationGrouping, false, "parentInstance", "rel_sc3_NavigationGrouping"), true);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll().From(*nested1).GroupByContract(*contract1);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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

        RelatedClass relatedInstanceClass(ret_ClassF, ret_ClassD, ret_ClassDHasClassE, false, "d", "rel_RET_ClassDHasClassE_0", true);
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create(ret_ClassF, *ret_ClassE.GetPropertyP("IntProperty"), nullptr, *spec, nullptr);
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectContract(*propertyGroupingContract, "this");
        grouped->From(ret_ClassF, true, "this");
        grouped->Join(relatedInstanceClass, false);
        
        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*grouped);
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouping->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouping->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());
        RegisterQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances_1", *grouping);

        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassE, {relatedInstanceClass}), "this");
        nested1->From(ret_ClassE, false, "this");
        nested1->Join(relatedInstanceClass, false);
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassG, {relatedInstanceClass}), "this");
        nested2->From(ret_ClassG, true, "this");
        nested2->Join(relatedInstanceClass, false);
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
        grouped->Join(RelatedClass(ret_ClassD, ret_ClassF, ret_ClassDHasClassE, true, "e", "rel_RET_ClassDHasClassE_0"), false);

        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*grouped);
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
        grouping->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);
        grouping->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_ClassDHasClassE.GetId());
        RegisterQuery("Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes_1", *grouping);

        RelatedClass relatedInstanceClass1(ret_ClassD, ret_ClassE, ret_ClassDHasClassE, true, "e", "rel_RET_ClassDHasClassE_0", false);
        ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
        nested1->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassD, {relatedInstanceClass1}), "this");
        nested1->From(ret_ClassD, true, "this");
        nested1->Join(relatedInstanceClass1, false);

        RelatedClass relatedInstanceClass2(ret_ClassD, ret_ClassG, ret_ClassDHasClassE, true, "e", "rel_RET_ClassDHasClassE_0");
        ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
        nested2->SelectContract(*ECInstanceNodesQueryContract::Create(&ret_ClassD, {relatedInstanceClass2}), "this");
        nested2->From(ret_ClassD, true, "this");
        nested2->Join(relatedInstanceClass2, false);

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
        grouping->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0"), false);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0"), false);
        query->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        query1->Join(RelatedClass(ret_ClassF, ret_ClassD, ret_ClassDReferencesClassE, false, "related", "rel_RET_ClassDReferencesClassE_0"), false);
        query1->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        NavigationQueryContractPtr contract2 = ECPropertyGroupingNodesQueryContract::Create(ret_ClassDReferencesClassE, *ret_ClassDReferencesClassE.GetPropertyP("Priority"), "rel_RET_ClassDReferencesClassE_1", *spec, nullptr);
        ComplexNavigationQueryPtr query2 = ComplexNavigationQuery::Create();
        query2->SelectContract(*contract2, "this");
        query2->From(ret_ClassG, true, "this");
        query2->Join(RelatedClass(ret_ClassG, ret_ClassD, ret_ClassDReferencesClassE, false, "related", "rel_RET_ClassDReferencesClassE_1"), false);
        query2->Where("[related].[ECInstanceId] = ?", {new BoundQueryId(ECInstanceId((uint64_t)123))});
        
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*UnionNavigationQuery::Create(*query1, *query2));
        grouped->GroupByContract(*contract1);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        sorted->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        sorted->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        sorted->OrderBy(ECPropertyGroupingNodesQueryContract::GroupingValueFieldName);
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
        nested->From(b4_ClassC, true, "this").Join(RelatedClass(b4_ClassC, b4_ClassB, b4_ClassBHasClassC, false, "related", "rel_b4_ClassBHasClassC"), false);
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
        nested->Join(RelatedClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0"), false);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        nested->Join(RelatedClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0"), false);
        nested->Where("[this].[SomeProperty] = ?", {new BoundQueryECValue(ECValue(9))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        nested->Join(RelatedClass(b4_ClassB, b4_ClassC, b4_ClassBHasClassC, true, "c", "rel_b4_ClassBHasClassC_0"), false);
        nested->Where("[this].[Description] = ? AND [this].[SomeProperty] = ?", 
            {new BoundQueryECValue(ECValue("TestGroupingDescription")), new BoundQueryECValue(ECValue(9))});

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).c_str());
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
        nested->Join(relatedInstanceClass, false);
        nested->Where("[c].[SomeProperty] = ? AND [this].[Description] = ? AND [this].[SomeProperty] = ?",
            {new BoundQueryECValue(ECValue(99)), new BoundQueryECValue(ECValue("TestGroupingDescription")), new BoundQueryECValue(ECValue(9))});

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
    
        Utf8PrintfString whereClause("InVirtualSet(?, [this].[ECInstanceId]) "
            "AND GetECInstanceDisplayLabel([this].[ECClassId], [this].[ECInstanceId], '', '[{\"Alias\":\"c\",\"ECClassId\":' || CAST(IFNULL([c].[ECClassId], %" PRIu64 ") AS TEXT) || ',\"ECInstanceId\":' || CAST(IFNULL([c].[ECInstanceId], 0) AS TEXT) || '}]') = ? "
            "AND [c].[SomeProperty] = ? "
            "AND [this].[Description] = ? "
            "AND [this].[SomeProperty] = ?", b4_ClassC.GetId().GetValue());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(b4_ClassB, false, "this");
        nested->Join(relatedInstanceClass, true);
        nested->Where(whereClause.c_str(),
            {
            new BoundQueryIdSet({ECInstanceId((uint64_t)1)}), 
            new BoundQueryECValue(ECValue("test")),
            new BoundQueryECValue(ECValue(99)),
            new BoundQueryECValue(ECValue("TestGroupingDescription")),
            new BoundQueryECValue(ECValue(9))
            });

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
        nested->Join(relatedInstanceClass, false);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->OrderBy(ecInstanceNodesQuerySortedDisplayLabel.c_str());
        sorted->GetResultParametersR().GetMatchingRelationshipIds().insert(ret_WidgetHasGadgets.GetId());
        
        RegisterQuery("JoinsWithAdditionalRelatedInstances", *sorted);
        }

    // FiltersByRelatedInstanceProperties
        {
        RelatedClass relatedInstanceClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget", "rel_RET_WidgetHasGadgets_0");
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(&ret_Gadget, {relatedInstanceClass});
    
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(ret_Gadget, false, "this");
        nested->Join(relatedInstanceClass, false);
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
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        
        RegisterQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet(bvector<ECInstanceId>{ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        
        RegisterQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass", *query);
        }
        
    // SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        field = &AddField(*descriptor, b2_Class2, ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query1), "this");
        query1->From(b1_Class1A, false, "this");
        query1->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &b2_Class2, *query2), "this");
        query2->From(b2_Class2, false, "this");
        query2->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses", *query);
        }

    // SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b4_ClassB, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b4_ClassC, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(b4_ClassC, b4_ClassB, b4_ClassBHasClassC, false, "nav_b4_ClassB_0", "")}
            });

        field = &AddField(*descriptor, b4_ClassB, ContentDescriptor::Property("this", b4_ClassB, *b4_ClassB.GetPropertyP("SomeProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", b4_ClassC, *b4_ClassC.GetPropertyP("SomeProperty")));
        descriptor->GetAllFields().back()->SetName("ClassB_ClassC_SomeProperty");

        field = &AddField(*descriptor, b4_ClassB, ContentDescriptor::Property("this", b4_ClassB, *b4_ClassB.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", b4_ClassC, *b4_ClassC.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->SetName("ClassB_ClassC_Description");

        field = &AddField(*descriptor, b4_ClassB, ContentDescriptor::Property("nav_b4_ClassB_0", b4_ClassC, *b4_ClassC.GetPropertyP("B")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &b4_ClassB, *query1), "this");
        query1->From(b4_ClassB, false, "this");
        query1->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &b4_ClassC, *query2), "this");
        query2->From(b4_ClassC, false, "this");
        query2->Join(RelatedClass(b4_ClassC, b4_ClassB, b4_ClassBHasClassC, false, "nav_b4_ClassB_0", ""), true);
        query2->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically", *query);
        }

    // SelectedNodeInstances_RemovesHiddenProperty
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class3, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "")}
            });

        field = &AddField(*descriptor, sc_Class3, ContentDescriptor::Property("this", sc_Class3, *sc_Class3.GetPropertyP("PropertyD")));
        field = &AddField(*descriptor, sc_Class3, ContentDescriptor::Property("nav_sc_Class1_0", sc_Class3, *sc_Class3.GetPropertyP("C1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &sc_Class3, *query), "this");
        query->From(sc_Class3, false, "this");
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_RemovesHiddenProperty", *query);
        }

    // SelectedNodeInstances_RemovesMultipleHiddenProperties
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class3, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "")}
            });

        field = &AddField(*descriptor, sc_Class3, ContentDescriptor::Property("nav_sc_Class1_0", sc_Class3, *sc_Class3.GetPropertyP("C1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &sc_Class3, *query), "this");
        query->From(sc_Class3, false, "this");
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        RegisterQuery("SelectedNodeInstances_RemovesMultipleHiddenProperties", *query);
        }

    // SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class2, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class3, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_1", "")}
            });

        descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), "Class2_Class3_C1", "C1"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("nav_sc_Class1_0", sc_Class2, *sc_Class2.GetPropertyP("C1")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("nav_sc_Class1_1", sc_Class3, *sc_Class3.GetPropertyP("C1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(descriptor->GetAllFields().back()->AsPropertiesField()));
        field = &AddField(*descriptor, sc_Class3, ContentDescriptor::Property("this", sc_Class3, *sc_Class3.GetPropertyP("PropertyD")));
                
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &sc_Class2, *query1), "this");
        query1->From(sc_Class2, false, "this");
        query1->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", ""), true);
        query1->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &sc_Class3, *query2), "this");
        query2->From(sc_Class3, false, "this");
        query2->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_1", ""), true);
        query2->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses", *query);
        }

    // SelectedNodeInstances_AddsSingleRelatedProperty
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0")},
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "")}
            });

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0"), true);
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsSingleRelatedProperty", *query);
        }

    // SelectedNodeInstances_AddsMultipleRelatedProperties
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0")},
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "")}
            });

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_LongProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget LongProperty");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0"), true);
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsMultipleRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsAllRelatedProperties
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"),RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(gadgetKeyField);

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_MyID");
        descriptor->GetAllFields().back()->SetLabel("Gadget MyID");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget");
        descriptor->GetAllFields().back()->SetLabel("Gadget Widget");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"), true);
        query->Join({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0")
            }, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsAllRelatedProperties", *query);
        }


    // SelectedNodeInstances_AddsBackwardRelatedProperties
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(sc_Class2, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "rel_sc_Class1_1", "rel_sc_Class1HasClass2And3_0")},
            {RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", "")}
            });

        field = &AddField(*descriptor, sc_Class2, ContentDescriptor::Property("this", sc_Class2, *sc_Class2.GetPropertyP("PropertyB")));
        field = &AddField(*descriptor, sc_Class2, ContentDescriptor::Property("nav_sc_Class1_0", sc_Class2, *sc_Class2.GetPropertyP("C1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto class1KeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(class1KeyField);

        field = &AddField(*descriptor, sc_Class2, ContentDescriptor::Property("rel_sc_Class1_1", sc_Class1, *sc_Class1.GetPropertyP("PropertyA")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(sc_Class1, sc_Class2, sc_Class1HasClass2And3, true, "rel_sc_Class2_0", "rel_sc_Class1HasClass2And3_0"));
        descriptor->GetAllFields().back()->SetName("rel_Class2_Class1_PropertyA");
        descriptor->GetAllFields().back()->SetLabel("Class1 PropertyA");
        class1KeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &sc_Class2, *query), "this");
        query->From(sc_Class2, false, "this");
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "rel_sc_Class1_1", "rel_sc_Class1HasClass2And3_0"), true);
        query->Join(RelatedClass(sc_Class2, sc_Class1, sc_Class1HasClass2And3, false, "nav_sc_Class1_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsBackwardRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsBothDirectionsRelatedProperties
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassL, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "rel_RET_ClassJ_2", "rel_RET_ClassJHasClassL_0")},
            {RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0")},
            {RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_1", "rel_RET_ClassLHasClassM_1")},
            {
            RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_1", "rel_RET_ClassLHasClassM_1"),
            RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_3"),
            },
            {
            RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_1", "rel_RET_ClassLHasClassM_1"),
            RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_3")
            },
            {RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_0")},
            {RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "nav_RET_ClassJ_1")}
            });
        
        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("this", ret_ClassL, *ret_ClassL.GetPropertyP("PropertyK")));

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassJ_0", ret_ClassL, *ret_ClassL.GetPropertyP("J1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("this", ret_ClassL, *ret_ClassL.GetPropertyP("PropertyL")));
        
        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassJ_1", ret_ClassL, *ret_ClassL.GetPropertyP("J2")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto classJKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(classJKeyField);

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("rel_RET_ClassJ_2", ret_ClassJ, *ret_ClassJ.GetPropertyP("PropertyJ")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassJ, ret_ClassL, ret_ClassJHasClassL, true, "rel_RET_ClassL_0", "rel_RET_ClassJHasClassL_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassJ_PropertyJ");
        descriptor->GetAllFields().back()->SetLabel("ClassJ PropertyJ");
        classJKeyField->AddKeyField(*field->AsPropertiesField());
        
        auto classM0KeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(classM0KeyField);

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("rel_RET_ClassM_0", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyM")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_1", "rel_RET_ClassLHasClassM_0"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_ClassM_1", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyM")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_2", "rel_RET_ClassLHasClassM_1"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_PropertyM");
        descriptor->GetAllFields().back()->SetLabel("ClassM PropertyM");
        classM0KeyField->AddKeyField(*field->AsPropertiesField());

        // note: this property comes from RelatedItemsDisplaySpecifications custom attribute
        auto classM1KeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(classM1KeyField);

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("rel_RET_ClassM_1", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyK")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_2", "rel_RET_ClassLHasClassM_1"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_PropertyK");
        descriptor->GetAllFields().back()->SetLabel("ClassM PropertyK");
        classM1KeyField->AddKeyField(*field->AsPropertiesField());
        
        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassJ_3", ret_ClassM, *ret_ClassM.GetPropertyP("J1")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_2", "rel_RET_ClassLHasClassM_1"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_J1");
        descriptor->GetAllFields().back()->SetLabel("ClassM J1");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassL_3", ret_ClassM, *ret_ClassM.GetPropertyP("J3")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_2", "rel_RET_ClassLHasClassM_1"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_J3");
        descriptor->GetAllFields().back()->SetLabel("ClassM J3");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_ClassL, *query), "this");
        query->From(ret_ClassL, false, "this");
        query->Join(RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "rel_RET_ClassJ_2", "rel_RET_ClassJHasClassL_0"), true);
        query->Join(RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"), true);
        query->Join(RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_1", "rel_RET_ClassLHasClassM_1"), true);
        query->Join({
            RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_1", "rel_RET_ClassLHasClassM_1"),
            RelatedClass(ret_ClassM, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_3"),
            }, true);
        query->Join({
            RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_1", "rel_RET_ClassLHasClassM_1"),
            RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_3")
            }, true);
        query->Join(RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_0"), true);
        query->Join(RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "nav_RET_ClassJ_1"), true);

        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsBothDirectionsRelatedProperties", *query);
        }

    // SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0")},
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_2", "rel_RET_WidgetHasGadget_0")},
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "")}
            });

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), "Widget_Description", "Description"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_Widget_2", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_Description");
        descriptor->GetAllFields().back()->SetLabel("Widget Description");
        widgetKeyField->AddKeyField(*descriptor->GetAllFields().back()->AsPropertiesField());

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_1", "rel_RET_WidgetHasGadgets_0"), true);
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_2", "rel_RET_WidgetHasGadget_0"), true);
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

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
        
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            path1, 
            path2,
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), "Widget_IntProperty", "IntProperty"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(nestedRelatedPropertiesPath1);
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_Widget_1", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(nestedRelatedPropertiesPath2);
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget_IntProperty");
        descriptor->GetAllFields().back()->SetLabel("Widget IntProperty");
        widgetKeyField->AddKeyField(*descriptor->GetAllFields().back()->AsPropertiesField());

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(path1, true);
        query->Join(path2, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);

        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
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

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            relationshipPath,
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(nestedRelatedPropertiesPath);
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget_Description");
        descriptor->GetAllFields().back()->SetLabel("Widget Description");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(relationshipPath, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);

        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
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

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            widgetRelationshipPath, 
            gadgetRelationshipPath,
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(gadgetKeyField);

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(gadgetRelatedPropertiesPath);
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(widgetRelatedPropertiesPath);
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Widget_Description");
        descriptor->GetAllFields().back()->SetLabel("Widget Description");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(gadgetRelationshipPath, true);
        query->Join(widgetRelationshipPath, true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);

        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_AddsNestedRelatedProperties2", *query);
        }

    // SelectedNodeInstances_SetsShowImagesFlag
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        descriptor->SetContentFlags((int)ContentFlags::ShowImages);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("SelectedNodeInstances_SetsShowImagesFlag", *query);
        }

    // SelectedNodeInstances_SetsShowLabelsFlagForGridContentType
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create(ContentDisplayType::Grid);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        descriptor->SetContentFlags((int)ContentFlags::ShowLabels);
        
        ComplexContentQueryPtr nested = ComplexContentQuery::Create();
        nested->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *nested), "this");
        nested->From(b1_Class1A, false, "this");
        nested->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

#ifdef WIP_SORTING_GRID_CONTENT
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s), %s", ContentQueryContract::DisplayLabelFieldName, ContentQueryContract::ECInstanceIdFieldName).c_str());
        RegisterQuery("SelectedNodeInstances_SetsShowLabelsFlagForGridContentType", *query);
#else
        RegisterQuery("SelectedNodeInstances_SetsShowLabelsFlagForGridContentType", *nested);
#endif
        }

    // SelectedNodeInstances_SetsKeysOnlyFlagForGraphicsContentType
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create(ContentDisplayType::Graphics);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        descriptor->SetContentFlags((int)ContentFlags::KeysOnly);
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        RegisterQuery("SelectedNodeInstances_SetsKeysOnlyFlagForGraphicsContentType", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, true));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperty
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, true));

        field = &AddField(*descriptor, b2_Class2, ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b2_Class2, *query), "this");
        query->From(b2_Class2, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        
        RegisterQuery("ContentInstacesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperty", *query);
        }

    //  ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, true));

        field = &AddField(*descriptor, b2_Class2, ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::CalculatedPropertyField("LabelTest_1", "CalculatedProperty_0", "\"Value\" & 1", nullptr, 1200));
        descriptor->GetAllFields().push_back(new ContentDescriptor::CalculatedPropertyField("LabelTest_2", "CalculatedProperty_1", "this.Name & \"Test\"", nullptr, 1500));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b2_Class2, *query), "this");
        query->From(b2_Class2, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1B, false));
        
        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        field = &AddField(*descriptor, b1_Class1B, ContentDescriptor::Property("this", b1_Class1B, *b1_Class1B.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query1), "this");
        query1->From(b1_Class1A, false, "this");
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1B, *query2), "this");
        query2->From(b1_Class1B, false, "this");

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses", *query);
        }

    // ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b2_Class2, false));
        
        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        field = &AddField(*descriptor, b2_Class2, ContentDescriptor::Property("this", b2_Class2, *b2_Class2.GetPropertyP("Name")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query1), "this");
        query1->From(b1_Class1A, false, "this");
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &b2_Class2, *query2), "this");
        query2->From(b2_Class2, false, "this");

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses", *query);
        }

    // ContentInstancesOfSpecificClasses_AppliesInstanceFilter
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));

        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");
        query->Where("[this].[DisplayLabel] = 10", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentInstancesOfSpecificClasses_AppliesInstanceFilter", *query);
        }

    // ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType1
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create(ContentDisplayType::PropertyPane);
        descriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        
        descriptor->AddContentFlag(ContentFlags::MergeResults);
        field = &AddField(*descriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &b1_Class1A, *query), "this");
        query->From(b1_Class1A, false, "this");

        RegisterQuery("ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType1", *query);
        }

    // ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType2
        {
        ContentDescriptorPtr innerDescriptor = ContentDescriptor::Create(ContentDisplayType::PropertyPane);
        field = &AddField(*innerDescriptor, b1_Class1A, ContentDescriptor::Property("this", b1_Class1A, *b1_Class1A.GetPropertyP("DisplayLabel")));
        field = &AddField(*innerDescriptor, b3_Class3, ContentDescriptor::Property("this", b3_Class3, *b3_Class3.GetPropertyP("SomeProperty")));
    
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(*innerDescriptor, &b1_Class1A, *q1), "this");
        q1->From(b1_Class1A, false, "this");
    
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(*innerDescriptor, &b3_Class3, *q2), "this");
        q2->From(b3_Class3, false, "this");
    
        ContentDescriptorPtr outerDescriptor = ContentDescriptor::Create(*innerDescriptor);
        outerDescriptor->GetSelectClasses().push_back(SelectClassInfo(b1_Class1A, false));
        outerDescriptor->GetSelectClasses().push_back(SelectClassInfo(b3_Class3, false));
        for (ContentDescriptor::Field* field : outerDescriptor->GetAllFields())
            {
            for (ContentDescriptor::Property& fieldProperty : field->AsPropertiesField()->GetProperties())
                fieldProperty.SetPrefix("");
            }
        outerDescriptor->AddContentFlag(ContentFlags::MergeResults); // note: inner descriptor doesn't have this flag!

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*outerDescriptor, nullptr, *query)); // note: null for the class argument
        query->From(*UnionContentQuery::Create(*q1, *q2));

        RegisterQuery("ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType2", *query);
        }

    // ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, true, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false), false);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
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
        
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query1), "this");
        query1->From(ret_Widget, true, "this");
        query1->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", false), false);
        query1->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query2), "this");
        query2->From(ret_Widget, true, "this");
        query2->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", false), false);
        query2->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query3 = ComplexContentQuery::Create();
        query3->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query3), "this");
        query3->From(ret_Widget, true, "this");
        query3->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", false), false);
        query3->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query4 = ComplexContentQuery::Create();
        query4->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query4), "this");
        query4->From(ret_Widget, true, "this");
        query4->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", false), false);
        query4->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*query1, *query2), *query3), *query4);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
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
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), "Widget_Sprocket_Description", "Description"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), "Widget_Sprocket_MyID", "MyID"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query1), "this");
        query1->From(ret_Widget, true, "this");
        query1->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "related", "rel_RET_WidgetHasGadgets_0", false), false);
        query1->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query2), "this");
        query2->From(ret_Widget, true, "this");
        query2->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "related", "rel_RET_WidgetHasGadget_0", false), false);
        query2->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query3 = ComplexContentQuery::Create();
        query3->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query3), "this");
        query3->From(ret_Widget, true, "this");
        query3->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets, true, "related", "rel_RET_WidgetsHaveGadgets_0", false), false);
        query3->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query4 = ComplexContentQuery::Create();
        query4->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query4), "this");
        query4->From(ret_Widget, true, "this");
        query4->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetsHaveGadgets2, true, "related", "rel_RET_WidgetsHaveGadgets2_0", false), false);
        query4->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        ComplexContentQueryPtr query5 = ComplexContentQuery::Create();
        query5->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query5), "this");
        query5->From(ret_Sprocket, true, "this");
        query5->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false), false);
        query5->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query5->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*query1, *query2), *query3), *query4), *query5);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode", *query);
        }

    // ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, true, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false), false);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet(bvector<ECInstanceId>{ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass", *query);
        }

    // ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true , "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "")}
            });

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
                        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query1), "this");
        query1->From(ret_Gadget, true, "this");
        query1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false), false);
        query1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", ""), true);
        query1->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query2), "this");
        query2->From(ret_Gadget, true, "this");
        query2->Join(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true , "related", "rel_RET_GadgetHasSprockets_0", false), false);
        query2->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", ""), true);
        query2->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)125)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses", *query);
        }

    // ContentRelatedInstances_AppliesInstanceFilter
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, true, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "related", "rel_RET_GadgetHasSprockets_0", false), false);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), false);
        query->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        query->Where("[this].[MyID] = 'Sprocket MyID'", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_AppliesInstanceFilter", *query);
        }

    // ContentRelatedInstances_SkipsRelatedLevel
        {
        RelatedClassPath relationshipPath1 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "related", "rel_RET_WidgetHasGadgets_0", false)
            };
        RelatedClassPath relationshipPath2 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "related", "rel_RET_WidgetHasGadget_0", false)
            };
        RelatedClassPath relationshipPath3 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets, false, "related", "rel_RET_WidgetsHaveGadgets_0", false)
            };
        RelatedClassPath relationshipPath4 = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget", "rel_RET_GadgetHasSprockets_0"),
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetsHaveGadgets2, false, "related", "rel_RET_WidgetsHaveGadgets2_0", false)
            };

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath1);
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath2);
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath3);
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath4);
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query1), "this");
        query1->From(ret_Sprocket, true, "this");
        query1->Join(relationshipPath1, false);
        query1->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query1->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query2), "this");
        query2->From(ret_Sprocket, true, "this");
        query2->Join(relationshipPath2, false);
        query2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query2->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query3 = ComplexContentQuery::Create();
        query3->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query3), "this");
        query3->From(ret_Sprocket, true, "this");
        query3->Join(relationshipPath3, false);
        query3->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query3->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        ComplexContentQueryPtr query4 = ComplexContentQuery::Create();
        query4->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query4), "this");
        query4->From(ret_Sprocket, true, "this");
        query4->Join(relationshipPath4, false);
        query4->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        query4->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
        
        UnionContentQueryPtr query = UnionContentQuery::Create(*UnionContentQuery::Create(*UnionContentQuery::Create(*query1, *query2), *query3), *query4);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("ContentRelatedInstances_SkipsRelatedLevel", *query);
        }

    // ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship
        {
        RelatedClassPath relationshipPath;
        relationshipPath.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "gadget", "rel_RET_WidgetHasGadgets_0"));
        relationshipPath.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "related", "rel_RET_GadgetHasSprockets_0", false));

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass(relationshipPath);
        
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, true, "this");
        query->Join(relationshipPath, false);
        query->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)123)})});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship", *query);
        }

    // ContentRelatedInstances_CreatesRecursiveQuery
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassN, true));
        descriptor->GetSelectClasses().back().SetPathToPrimaryClass({
            RelatedClass(ret_ClassN, ret_ClassN, ret_ClassNGroupsClassN, false, "related", "rel_RET_ClassNGroupsClassN_0", false)
            });
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_ClassN, ret_ClassN, ret_ClassNGroupsClassN, false, "nav_RET_ClassN_0", "")}
            });

        field = &AddField(*descriptor, ret_ClassN, ContentDescriptor::Property("this", ret_ClassN, *ret_ClassN.GetPropertyP("IntProperty")));
        field = &AddField(*descriptor, ret_ClassN, ContentDescriptor::Property("nav_RET_ClassN_0", ret_ClassN, *ret_ClassN.GetPropertyP("N")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        bvector<ECInstanceId> selectedIds;
        selectedIds.push_back(ECInstanceId((uint64_t)123));
        selectedIds.push_back(ECInstanceId((uint64_t)456));

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(&ret_ClassNGroupsClassN);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_ClassN, *query), "this");
        query->From(ret_ClassN, true, "this");
        query->Join(RelatedClass(ret_ClassN, ret_ClassN, ret_ClassNGroupsClassN, false, "nav_RET_ClassN_0", ""), true);
        query->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryRecursiveChildrenIdSet(relationships, true, selectedIds)});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("ContentRelatedInstances_CreatesRecursiveQuery", *query);
        }

    // NestedContentField_WithSingleStepRelationshipPath
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, true, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "primary_instance", "rel"), false);

        RegisterQuery("NestedContentField_WithSingleStepRelationshipPath", *query);
        }

    // NestedContentField_WithMultiStepRelationshipPath
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, true, "this");

        RelatedClassPath path = {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "intermediate", "rel_gs"), 
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "primary_instance", "rel_wg")
            };
        query->Join(path, false);

        RegisterQuery("NestedContentField_WithMultiStepRelationshipPath", *query);
        }

    // NestedContentField_WithNestedContentFields
        {
        ContentDescriptor::Category category("name", "label", 1, true);

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::NestedContentField(category, "sprocket_field_name", "sprocket_field_label", ret_Sprocket, 
            {
            RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "gadget_instance", "rel_gs")
            }, 
            {
            new ContentDescriptor::ECPropertiesField(ret_Sprocket, ContentDescriptor::Property("sprocket_instance", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")))
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, true, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "widget_instance", "rel_wg"), false);

        RegisterQuery("NestedContentField_WithNestedContentFields", *query);
        }

    // RelatedItemsDisplaySpecification_IncludesPropertiesOfRelatedClass
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassL, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0")},
            {RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"),RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_2", "")},
            {RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"),RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_1", "")},
            {RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_0", "")},
            {RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "nav_RET_ClassJ_1", "")}
            });

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("this", ret_ClassL, *ret_ClassL.GetPropertyP("PropertyK")));
        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassJ_0", ret_ClassL, *ret_ClassL.GetPropertyP("J1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("this", ret_ClassL, *ret_ClassL.GetPropertyP("PropertyL")));
        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassJ_1", ret_ClassL, *ret_ClassL.GetPropertyP("J2")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(descriptor->GetAllFields().back()->AsPropertiesField()));

        auto classMKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(classMKeyField);

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("rel_RET_ClassM_0", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyK")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_0", "rel_RET_ClassLHasClassM_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_PropertyK");
        descriptor->GetAllFields().back()->SetLabel("ClassM PropertyK");
        classMKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassJ_2", ret_ClassM, *ret_ClassM.GetPropertyP("J1")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_0", "rel_RET_ClassLHasClassM_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_J1");
        descriptor->GetAllFields().back()->SetLabel("ClassM J1");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        classMKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("rel_RET_ClassM_0", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyM")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_0", "rel_RET_ClassLHasClassM_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_PropertyM");
        descriptor->GetAllFields().back()->SetLabel("ClassM PropertyM");
        classMKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_ClassL, ContentDescriptor::Property("nav_RET_ClassL_1", ret_ClassM, *ret_ClassM.GetPropertyP("J3")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_0", "rel_RET_ClassLHasClassM_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassL_ClassM_J3");
        descriptor->GetAllFields().back()->SetLabel("ClassM J3");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        classMKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_ClassL, *query), "this");
        query->From(ret_ClassL, false, "this");

        query->Join(RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"), true);

        RelatedClassPath path;
        path.push_back(RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"));
        path.push_back(RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_2", ""));
        query->Join(path, true);

        path.clear();
        path.push_back(RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"));
        path.push_back(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_1", ""));
        query->Join(path, true);

        query->Join(RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_0", ""), true);

        query->Join(RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "nav_RET_ClassJ_1", ""), true);

#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("RelatedItemsDisplaySpecification_IncludesPropertiesOfRelatedClass", *query);
        }

    // RelatedItemsDisplaySpecification_IncludesPropertiesOfRelatedClassPolymorphically
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassJ, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_ClassJ, ret_ClassM, ret_ClassJHasClassK, true, "rel_RET_ClassM_0", "rel_RET_ClassJHasClassK_0")},
            {RelatedClass(ret_ClassJ, ret_ClassM, ret_ClassJHasClassK, true, "rel_RET_ClassM_0", "rel_RET_ClassJHasClassK_0"),
            RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_1", "")},
            {RelatedClass(ret_ClassJ, ret_ClassM, ret_ClassJHasClassK, true, "rel_RET_ClassM_0", "rel_RET_ClassJHasClassK_0"),
            RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_0", "")}
            });

        field = &AddField(*descriptor, ret_ClassJ, ContentDescriptor::Property("this", ret_ClassJ, *ret_ClassJ.GetPropertyP("PropertyJ")));
        
        auto classMKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(classMKeyField);

        field = &AddField(*descriptor, ret_ClassJ, ContentDescriptor::Property("rel_RET_ClassM_0", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyK")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassJ, ret_ClassJHasClassK, false, "rel_RET_ClassJ_0", "rel_RET_ClassJHasClassK_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassJ_ClassM_PropertyK");
        descriptor->GetAllFields().back()->SetLabel("ClassM PropertyK");
        classMKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_ClassJ, ContentDescriptor::Property("nav_RET_ClassJ_1", ret_ClassM, *ret_ClassM.GetPropertyP("J1")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassJ, ret_ClassJHasClassK, false, "rel_RET_ClassJ_0", "rel_RET_ClassJHasClassK_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassJ_ClassM_J1");
        descriptor->GetAllFields().back()->SetLabel("ClassM J1");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        classMKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_ClassJ, ContentDescriptor::Property("rel_RET_ClassM_0", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyM")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassJ, ret_ClassJHasClassK, false, "rel_RET_ClassJ_0", "rel_RET_ClassJHasClassK_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassJ_ClassM_PropertyM");
        descriptor->GetAllFields().back()->SetLabel("ClassM PropertyM");
        classMKeyField->AddKeyField(*field->AsPropertiesField());

        field = &AddField(*descriptor, ret_ClassJ, ContentDescriptor::Property("nav_RET_ClassL_0", ret_ClassM, *ret_ClassM.GetPropertyP("J3")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_ClassM, ret_ClassJ, ret_ClassJHasClassK, false, "rel_RET_ClassJ_0", "rel_RET_ClassJHasClassK_0"));
        descriptor->GetAllFields().back()->SetName("rel_ClassJ_ClassM_J3");
        descriptor->GetAllFields().back()->SetLabel("ClassM J3");
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        classMKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_ClassJ, *query), "this");
        query->From(ret_ClassJ, false, "this");
        query->Join(RelatedClass(ret_ClassJ, ret_ClassM, ret_ClassJHasClassK, true, "rel_RET_ClassM_0", "rel_RET_ClassJHasClassK_0"), true);
        RelatedClassPath path;
        path.push_back(RelatedClass(ret_ClassJ, ret_ClassM, ret_ClassJHasClassK, true, "rel_RET_ClassM_0", "rel_RET_ClassJHasClassK_0"));
        path.push_back(RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_1", ""));
        query->Join(path, true);
        path.clear();
        path.push_back(RelatedClass(ret_ClassJ, ret_ClassM, ret_ClassJHasClassK, true, "rel_RET_ClassM_0", "rel_RET_ClassJHasClassK_0"));
        path.push_back(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_0", ""));
        query->Join(path, true);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("RelatedItemsDisplaySpecification_IncludesPropertiesOfRelatedClassPolymorphically", *query);
        }

    // RelatedItemsDisplaySpecification_IncludesPropertiesOfRelatedClassByFollowingMultipleRelationships
        {
        RelatedClassPath relationshipPath;
        relationshipPath.push_back(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "rel_RET_ClassL_1", "rel_RET_ClassLHasClassM_0"));
        relationshipPath.push_back(RelatedClass(ret_ClassL, ret_ClassJ, ret_ClassJHasClassL, false, "rel_RET_ClassJ_1", "rel_RET_ClassJHasClassL_0"));

        RelatedClassPath relatedPropertiesPath;
        relatedPropertiesPath.push_back(RelatedClass(ret_ClassL, ret_ClassM, ret_ClassLHasClassM, true, "rel_RET_ClassM_0", "rel_RET_ClassLHasClassM_0"));
        relatedPropertiesPath.push_back(RelatedClass(ret_ClassJ, ret_ClassL, ret_ClassJHasClassL, true, "rel_RET_ClassL_1", "rel_RET_ClassJHasClassL_0"));

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_ClassM, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {relationshipPath},
            {RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_0", "")},
            {RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_0", "")}
            });

        field = &AddField(*descriptor, ret_ClassM, ContentDescriptor::Property("this", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyK")));
        field = &AddField(*descriptor, ret_ClassM, ContentDescriptor::Property("nav_RET_ClassJ_0", ret_ClassM, *ret_ClassM.GetPropertyP("J1")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        field = &AddField(*descriptor, ret_ClassM, ContentDescriptor::Property("this", ret_ClassM, *ret_ClassM.GetPropertyP("PropertyM")));
        field = &AddField(*descriptor, ret_ClassM, ContentDescriptor::Property("nav_RET_ClassL_0", ret_ClassM, *ret_ClassM.GetPropertyP("J3")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto classJKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(classJKeyField);

        field = &AddField(*descriptor, ret_ClassM, ContentDescriptor::Property("rel_RET_ClassJ_1", ret_ClassJ, *ret_ClassJ.GetPropertyP("PropertyJ")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(relatedPropertiesPath);
        descriptor->GetAllFields().back()->SetName("rel_ClassM_ClassL_ClassJ_PropertyJ");
        descriptor->GetAllFields().back()->SetLabel("ClassJ PropertyJ");
        classJKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_ClassM, *query), "this");
        query->From(ret_ClassM, false, "this");
        query->Join(relationshipPath, true);
        query->Join(RelatedClass(ret_ClassK, ret_ClassJ, ret_ClassJHasClassK, false, "nav_RET_ClassJ_0", ""), true);
        query->Join(RelatedClass(ret_ClassM, ret_ClassL, ret_ClassLHasClassM, false, "nav_RET_ClassL_0", ""), true);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif

        RegisterQuery("RelatedItemsDisplaySpecification_IncludesPropertiesOfRelatedClassByFollowingMultipleRelationships", *query);
        }

    // FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Gadget_Widget_MyID");
        
        auto widgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(widgetKeyField);

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("rel_RET_Widget_0", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadgets, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadgets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Gadget_Widget_MyID");
        descriptor->GetAllFields().back()->SetLabel("Widget MyID");
        widgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *q1), "this");
        q1->From(ret_Gadget, false, "this");
        q1->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0"), true);
        q1->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)1)})});
        
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *q2), "this");
        q2->From(ret_Widget, false, "this");
        q2->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet({ECInstanceId((uint64_t)2)})});

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass", *query);
        }

    // FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadget_0")}
            });
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_2", "rel_RET_GadgetHasSprockets_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_1", "")}
            });
        
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->SetName("Widget_Sprocket_Description");

        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->SetName("Widget_Sprocket_MyID");

        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(gadgetKeyField);

        descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), "", ""));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_Gadget_0", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadget, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadget_0"));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().push_back(ContentDescriptor::Property("rel_RET_Gadget_2", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Widget_Sprocket_Gadget_MyID");
        descriptor->GetAllFields().back()->SetLabel("Gadget MyID");
        gadgetKeyField->AddKeyField(*descriptor->GetAllFields().back()->AsPropertiesField());
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_1", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));
        
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *q1), "this");
        q1->From(ret_Widget, false, "this");
        q1->Join(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "rel_RET_Gadget_0", "rel_RET_WidgetHasGadget_0"), true);
        
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *q2), "this");
        q2->From(ret_Sprocket, false, "this");
        q2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_2", "rel_RET_GadgetHasSprockets_0"), true);
        q2->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_1", ""), true);

        UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty", *query);
        }

    // AppliesRelatedPropertiesSpecificationFromContentModifier
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(gadgetKeyField);

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"), true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("AppliesRelatedPropertiesSpecificationFromContentModifier", *query);
        }

    // DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(gadgetKeyField);

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"), true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses", *query);
        }

    // RelatedPropertiesAreAppendedCorrectlyWhenUsingCustomDescriptor
        {
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Sprocket, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0")},
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", "")}
            });

        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("this", ret_Sprocket, *ret_Sprocket.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("nav_RET_Gadget_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Gadget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        auto gadgetKeyField = new ContentDescriptor::ECInstanceKeyField();
        descriptor->GetAllFields().push_back(gadgetKeyField);
        
        field = &AddField(*descriptor, ret_Sprocket, ContentDescriptor::Property("rel_RET_Gadget_1", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));
        descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back().SetIsRelated(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "rel_RET_Sprocket_0", "rel_RET_GadgetHasSprockets_0"));
        descriptor->GetAllFields().back()->SetName("rel_Sprocket_Gadget_Description");
        descriptor->GetAllFields().back()->SetLabel("Gadget Description");
        gadgetKeyField->AddKeyField(*field->AsPropertiesField());
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Sprocket, *query), "this");
        query->From(ret_Sprocket, false, "this");
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_1", "rel_RET_GadgetHasSprockets_0"), true);
        query->Join(RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "nav_RET_Gadget_0", ""), true);
        
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("RelatedPropertiesAreAppendedCorrectlyWhenUsingCustomDescriptor", *query);
        }

    // CreatesContentFieldsForXToManyRelatedInstanceProperties
        {
        ContentDescriptor::Category sprocketCategory(ret_Sprocket.GetName(), ret_Sprocket.GetDisplayLabel(), DefaultCategorySupplier::NESTED_CONTENT_CATEGORY_PRIORITY, false);
        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Gadget, false));
        descriptor->GetSelectClasses().back().SetRelatedPropertyPaths({
            {RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", "")}
            });

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("this", ret_Gadget, *ret_Gadget.GetPropertyP("Description")));

        field = &AddField(*descriptor, ret_Gadget, ContentDescriptor::Property("nav_RET_Widget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Widget")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field->AsPropertiesField()));

        descriptor->GetAllFields().push_back(new ContentDescriptor::NestedContentField(sprocketCategory, "Gadget_Sprocket", "Sprocket", ret_Sprocket, 
            {RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_0", "rel_RET_GadgetHasSprockets_0")}, 
            {
            new ContentDescriptor::ECPropertiesField(ret_Gadget, ContentDescriptor::Property("rel_RET_Sprocket_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")))
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Gadget, *query), "this");
        query->From(ret_Gadget, false, "this");
        query->Join(RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "nav_RET_Widget_0", ""), true);

#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("CreatesContentFieldsForXToManyRelatedInstanceProperties", *query);
        }

    // CreatesNestedContentFieldsForXToManyRelatedInstanceProperties
        {
        ContentDescriptor::Category gadgetCategory(ret_Gadget.GetName(), ret_Gadget.GetDisplayLabel(), DefaultCategorySupplier::NESTED_CONTENT_CATEGORY_PRIORITY, false);
        ContentDescriptor::Category sprocketCategory(ret_Sprocket.GetName(), ret_Sprocket.GetDisplayLabel(), DefaultCategorySupplier::NESTED_CONTENT_CATEGORY_PRIORITY, false);

        ContentDescriptorPtr descriptor = ContentDescriptor::Create();
        descriptor->GetSelectClasses().push_back(SelectClassInfo(ret_Widget, false));

        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("Description")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("MyID")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("IntProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("BoolProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DoubleProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("LongProperty")));
        field = &AddField(*descriptor, ret_Widget, ContentDescriptor::Property("this", ret_Widget, *ret_Widget.GetPropertyP("DateProperty")));
        descriptor->GetAllFields().push_back(new ContentDescriptor::NestedContentField(gadgetCategory, "Widget_Gadget", "Gadget", ret_Gadget, 
            {
            RelatedClass(ret_Gadget, ret_Widget, ret_WidgetHasGadgets, false, "rel_RET_Widget_0", "rel_RET_WidgetHasGadgets_0")
            }, 
            {
            new ContentDescriptor::ECPropertiesField(ret_Widget, ContentDescriptor::Property("rel_RET_Gadget_0", ret_Gadget, *ret_Gadget.GetPropertyP("Description"))),
            new ContentDescriptor::NestedContentField(sprocketCategory, "Gadget_Sprocket", "Sprocket", ret_Sprocket, 
                {
                RelatedClass(ret_Sprocket, ret_Gadget, ret_GadgetHasSprockets, false, "rel_RET_Gadget_0", "rel_RET_GadgetHasSprockets_0")
                }, 
                {
                new ContentDescriptor::ECPropertiesField(ret_Widget, ContentDescriptor::Property("rel_RET_Sprocket_0", ret_Sprocket, *ret_Sprocket.GetPropertyP("Description")))
                })
            }));
        
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*ContentQueryContract::Create(*descriptor, &ret_Widget, *query), "this");
        query->From(ret_Widget, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        RegisterQuery("CreatesNestedContentFieldsForXToManyRelatedInstanceProperties", *query);
        }
    }

#define LOGD(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->debugv(__VA_ARGS__)
#define LOGI(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->infov(__VA_ARGS__)
#define LOGW(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->warningv(__VA_ARGS__)
#define LOGE(...) NativeLogging::LoggingManager::GetLogger("ExpectedQueriesTest")->errorv(__VA_ARGS__)

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExecuteQueries(bmap<Utf8String, NavigationQueryCPtr> queries, ECDbR db, PresentationRuleSetCR ruleset, 
    IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache)
    {
    for (auto pair : queries)
        {
        Utf8String const& name = pair.first;
        NavigationQueryCPtr query = pair.second;
        LOGI("---");
        LOGI("Query: '%s'", name.c_str());

        JsonNavNodesFactory nodesFactory;
        CustomFunctionsContext functionsContext(db, ruleset, userSettings, nullptr, ecexpressionsCache, nodesFactory, nullptr, nullptr, &query->GetExtendedData());
        
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
static void ExecuteQueries(bmap<Utf8String, ContentQueryCPtr> queries, ECDbR db, PresentationRuleSetCR ruleset, 
    IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache)
    {
    for (auto pair : queries)
        {
        Utf8String const& name = pair.first;

        ContentQueryCPtr query = pair.second;
        LOGI("---");
        LOGI("Query: '%s'", name.c_str());
        
        JsonNavNodesFactory nodesFactory;
        CustomFunctionsContext functionsContext(db, ruleset, userSettings, nullptr, ecexpressionsCache, nodesFactory, nullptr, nullptr, nullptr);
        
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

    ECDbR db = ExpectedQueries::GetInstance(BeTest::GetHost()).GetDb();
    TestUserSettings userSettings;
    ECExpressionsCache ecexpressionsCache;
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    CustomFunctionsInjector customFunctions(db);

    ExecuteQueries(ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQueries(), db, *ruleset, userSettings, ecexpressionsCache);
    ExecuteQueries(ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQueries(), db, *ruleset, userSettings, ecexpressionsCache);
    }
