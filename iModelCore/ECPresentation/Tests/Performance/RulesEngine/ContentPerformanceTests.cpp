/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RulesEnginePerformanceTests.h"
#include <ECPresentation/Content.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentPerformanceTests : RulesEnginePerformanceTests
    {
    BeFileName _SupplyProjectPath() const override
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(L"Performance");
        path.AppendToPath(L"Oakland.ibim");
        return path;
        }

    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        // taken from Gist
        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromXmlString(R"ruleset(
            <PresentationRuleSet RuleSetId="Items" VersionMajor="1" VersionMinor="3" SupportedSchemas="Generic,BisCore"
                                 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

                <!-- Content rules -->
                <!-- Grid / DGN view -->
                <ContentRule Condition='(ContentDisplayType="Grid" OR ContentDisplayType="Graphics") ANDALSO SelectedNode.ECInstance.IsOfClass("Model", "BisCore")' OnlyIfNotHandled='true'>
                    <ContentRelatedInstances RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward' />
                </ContentRule>
                <ContentRule Condition='(ContentDisplayType="Grid" OR ContentDisplayType="Graphics") ANDALSO SelectedNode.ECInstance.IsOfClass("Category", "BisCore")' OnlyIfNotHandled='true'>
                    <ContentRelatedInstances RelationshipClassNames='BisCore:GeometricElement2dIsInCategory,GeometricElement3dIsInCategory' RequiredDirection='Backward' />
                </ContentRule>
                <ContentRule Condition='(ContentDisplayType="Grid" OR ContentDisplayType="Graphics") ANDALSO SelectedNode.ECInstance.IsOfClass("Element", "BisCore")' OnlyIfNotHandled='true'>
                    <ContentRelatedInstances RelationshipClassNames='BisCore:ElementOwnsChildElements' RelatedClassNames='BisCore:Element' RequiredDirection='Forward' IsRecursive='true' />
                    <SelectedNodeInstances />
                </ContentRule>
                <!-- Any other (property pane, list, other) -->
                <ContentRule OnlyIfNotHandled='true'>
                    <SelectedNodeInstances />
                </ContentRule>

                <!-- Content modifiers that apply to any content rule -->
                <ContentModifier ClassName="Element" SchemaName="BisCore">
                    <RelatedProperties RelationshipClassNames='BisCore:ElementOwnsUniqueAspect' RelatedClassNames='BisCore:ElementUniqueAspect'
                                       RequiredDirection='Forward' IsPolymorphic='True' />
                    <RelatedProperties RelationshipClassNames='BisCore:ElementOwnsMultiAspects' RelatedClassNames='BisCore:ElementMultiAspect'
                                       RequiredDirection='Forward' IsPolymorphic='True' />
                </ContentModifier>
                <ContentModifier ClassName="PhysicalElement" SchemaName="BisCore">
                    <RelatedProperties RelationshipClassNames='BisCore:PhysicalElementIsOfType' RelatedClassNames='BisCore:PhysicalType' 
                                       RequiredDirection='Forward' IsPolymorphic='True' />
                </ContentModifier>
                <ContentModifier ClassName="SpatialLocationElement" SchemaName="BisCore">
                    <RelatedProperties RelationshipClassNames='BisCore:SpatialLocationIsOfType' RelatedClassNames='BisCore:SpatialLocationType'
                                       RequiredDirection='Forward' IsPolymorphic='True' />
                </ContentModifier>

            </PresentationRuleSet>
            )ruleset");
        return ruleset;
        }

    void GetContent(SelectionInfo const&, KeySetCR inputKeys, Utf8CP type, int expectedContentSize, int flags, Utf8CP passName);
    void GetContentForAllGeometricElements(Utf8CP type, int expectedContentSize, int flags);
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentPerformanceTests::GetContent(SelectionInfo const& selection, KeySetCR inputKeys, Utf8CP type, int expectedContentSize, int flags, Utf8CP passName)
    {
    // start the timer
    Utf8PrintfString timerName("%s: %s pass", BeTest::GetNameOfCurrentTest(), passName);
    Timer _timer(timerName.c_str());

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options = CreateContentOptions();
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_project, type, inputKeys, &selection, options.GetJson()).get();

    if (descriptor->GetContentFlags() != (flags | descriptor->GetContentFlags()))
        {
        ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
        modifiedDescriptor->SetContentFlags(flags | descriptor->GetContentFlags());
        descriptor = modifiedDescriptor;
        }
        
    // get the content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(expectedContentSize, content->GetContentSet().GetSize());
    for (ContentSetItemCPtr record : content->GetContentSet())
        EXPECT_TRUE(record.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentPerformanceTests::GetContentForAllGeometricElements(Utf8CP type, int expectedContentSize, int flags = 0)
    {
    // getting content for all geometric elements in the dataset
    bvector<ECClassInstanceKey> keys;
    ECSqlStatement stmt;
    stmt.Prepare(m_project, "SELECT ECClassId, ECInstanceId FROM [BisCore].[GeometricElement]");
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        keys.push_back(ECClassInstanceKey(m_project.Schemas().GetClass(stmt.GetValueId<ECClassId>(0)), stmt.GetValueId<ECInstanceId>(1)));
    SelectionInfoCPtr selection = SelectionInfo::Create("", false);

    GetContent(*selection, *KeySet::Create(keys), type, expectedContentSize, flags, "First");
    GetContent(*selection, *KeySet::Create(keys), type, expectedContentSize, flags, "Second");
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> GetDerivedClasses(ECDbCR db, ECClassCR base)
    {
    bset<ECClassCP> allDerivedClasses;
    for (ECClassCP derivedClass : db.Schemas().GetDerivedClasses(base))
        {
        bset<ECClassCP> const& derivedClasses = GetDerivedClasses(db, *derivedClass);
        allDerivedClasses.insert(derivedClasses.begin(), derivedClasses.end());
        allDerivedClasses.insert(derivedClass);
        }
    return allDerivedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on products' property selector use case where a content descriptor
* is requested for all element subclasses classes.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetDescriptorForAllElementSubclasses)
    {    
    // set up selection
    ECClassCP elementClass = m_project.Schemas().GetClass("BisCore", "Element");
    bset<ECClassCP> allElementClassesSet = GetDerivedClasses(m_project, *elementClass);
    bvector<ECClassCP> allElementClasses(allElementClassesSet.begin(), allElementClassesSet.end());
    bvector<ECClassInstanceKey> keys;
    for (ECClassCP ecClass : allElementClasses)
        keys.push_back(ECClassInstanceKey(m_project.Schemas().GetClass(ecClass->GetId()), ECInstanceId()));

    KeySetCPtr input = KeySet::Create(keys);
    
    // get the descriptor
    Utf8PrintfString timerName1("%s: First pass", BeTest::GetNameOfCurrentTest());
    Timer _timer1(timerName1.c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_project, ContentDisplayType::PropertyPane, *input, nullptr, CreateContentOptions().GetJson()).get();
    EXPECT_TRUE(descriptor.IsValid());
    _timer1.Finish();

    // force clear content cache so descriptors aren't cached
    m_manager->NotifyCategoriesChanged();

    Utf8PrintfString timerName2("%s: Second pass", BeTest::GetNameOfCurrentTest());
    Timer _timer2(timerName2.c_str());
    descriptor = m_manager->GetContentDescriptor(m_project, ContentDisplayType::PropertyPane, *input, nullptr, CreateContentOptions().GetJson()).get();
    EXPECT_TRUE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on Version Compare use case where the library requests content 
* classes for all derived classes of BisCore:Element to find property relationship
* paths.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetContentClassesForBisElements)
    {    
    ECClassCP elementClass = m_project.Schemas().GetClass("BisCore", "Element");
    
    Utf8PrintfString timerName1("%s: First pass", BeTest::GetNameOfCurrentTest());
    Timer _timer1(timerName1.c_str());
    bvector<SelectClassInfo> classes = m_manager->GetContentClasses(m_project, ContentDisplayType::PropertyPane, {elementClass}, CreateContentOptions().GetJson()).get();
    EXPECT_TRUE(!classes.empty());
    _timer1.Finish();
    
    Utf8PrintfString timerName2("%s: Second pass", BeTest::GetNameOfCurrentTest());
    Timer _timer2(timerName2.c_str());
    classes = m_manager->GetContentClasses(m_project, ContentDisplayType::PropertyPane, {elementClass}, CreateContentOptions().GetJson()).get();
    EXPECT_TRUE(!classes.empty());
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on a use case where application has a list of instance keys and wants
* to get their display labels.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetDisplayLabels)
    {
    GetContentForAllGeometricElements(ContentDisplayType::List, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetPropertyPaneContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetPropertyPaneContentWithLabelsForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1, (int)ContentFlags::ShowLabels);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for grid view
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetGridContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Grid, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for dgn view
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetGraphicsContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Graphics, 7414);
    }

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                01/2018
+===============+===============+===============+===============+===============+======*/
struct LabelOverrideContentPerformanceTests : ContentPerformanceTests
    {
    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        PresentationRuleSetPtr ruleset = ContentPerformanceTests::_SupplyRuleset();
        ruleset->AddPresentationRule(*new LabelOverride(R"(ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Element", "BisCore"))", 100, R"(IIF(IsNull(this.UserLabel) OR this.UserLabel="", this.CodeValue, this.UserLabel))", ""));
        return ruleset;
        }
    };

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for dgn view
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideContentPerformanceTests, GetGridContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Grid, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideContentPerformanceTests, GetPropertyPaneContentWithLabelsForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1, (int)ContentFlags::ShowLabels);
    }

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                01/2018
+===============+===============+===============+===============+===============+======*/
struct InstanceLabelOverrideContentPerformanceTests : ContentPerformanceTests
    {
    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        PresentationRuleSetPtr ruleset = ContentPerformanceTests::_SupplyRuleset();
        ruleset->AddPresentationRule(*new InstanceLabelOverride(100, true, "BisCore:Element", "CodeValue,UserLabel"));
        return ruleset;
        }
    };

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for dgn view
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideContentPerformanceTests, GetGridContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Grid, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideContentPerformanceTests, GetPropertyPaneContentWithLabelsForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1, (int)ContentFlags::ShowLabels);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2018
+===============+===============+===============+===============+===============+======*/
struct TilesPublisherPerformanceTests : ContentPerformanceTests
    {
    BeFileName _SupplyProjectPath() const override
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(L"Performance");
        path.AppendToPath(L"TilesPublisherSample.bim");
        return path;
        }
    
    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        // taken from https://bentleycs.visualstudio.com/beconnect/_git/TilesPublisherService?path=%2Fsrc%2FInterop%2FAssets%2FPresentationRules%2FBisCore.PresentationRuleSet.xml&version=GB2018_07_25_04_Platform_update
        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromXmlString(R"ruleset(
            <PresentationRuleSet RuleSetId="Items" VersionMinor="0" VersionMajor="1">
              <InstanceLabelOverride ClassName='BisCore:Element' OnlyIfNotHandled='true' Priority='100' PropertyNames='UserLabel,CodeValue' />
              <LabelOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Model", "BisCore")'
                Label='this.GetRelatedValue("BisCore:ModelModelsElement", "Forward", "BisCore:Element", "UserLabel")' OnlyIfNotHandled='true' Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Category", "BisCore")' ImageId='"ECLiteralImage://CATEGORY"'
                Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("GeometricModel2d", "BisCore")' ImageId='"ECLiteralImage://MODEL_2D"'
                Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("GeometricModel3d", "BisCore")' ImageId='"ECLiteralImage://MODEL_3D"'
                Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Model", "BisCore")' ImageId='"ECLiteralImage://MODEL' Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("ViewDefinition2d", "BisCore")' ImageId='"ECLiteralImage://VIEWDEF_2D"'
                Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("ViewDefinition3d", "BisCore")' ImageId='"ECLiteralImage://VIEWDEF_3D"'
                Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("ViewDefinition", "BisCore")' ImageId='"ECLiteralImage://VIEWDEF"'
                Priority='100' />
              <ImageIdOverride Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Element", "BisCore")' ImageId='"ECLiteralImage://ELEMENT"'
                Priority='100' />
              <ContentModifier ClassName="Element" SchemaName="BisCore">
                <RelatedProperties IsPolymorphic='True' RelatedClassNames='BisCore:ElementUniqueAspect' RelationshipClassNames='BisCore:ElementOwnsUniqueAspect'
                  RequiredDirection='Forward' />
                <RelatedProperties IsPolymorphic='True' RelatedClassNames='BisCore:ElementMultiAspect' RelationshipClassNames='BisCore:ElementOwnsMultiAspects'
                  RequiredDirection='Forward' />
              </ContentModifier>
              <ContentModifier ClassName="PhysicalElement" SchemaName="BisCore">
                <RelatedProperties IsPolymorphic='True' RelatedClassNames='BisCore:PhysicalType' RelationshipClassNames='BisCore:PhysicalElementIsOfType'
                  RelationshipMeaning='RelatedInstance' RequiredDirection='Forward' />
              </ContentModifier>
              <ContentModifier ClassName="SpatialLocationElement" SchemaName="BisCore">
                <RelatedProperties IsPolymorphic='True' RelatedClassNames='BisCore:SpatialLocationType' RelationshipClassNames='BisCore:SpatialLocationIsOfType'
                  RequiredDirection='Forward' />
              </ContentModifier>
              <ContentModifier ClassName="PhysicalType" SchemaName="BisCore">
                <HiddenProperties ClassName="BisCore:PhysicalType" PropertyNames="*" />
              </ContentModifier>
              <ContentModifier ClassName="SpatialLocationType" SchemaName="BisCore">
                <HiddenProperties ClassName="BisCore:SpatialLocationType" PropertyNames="*" />
              </ContentModifier>
              <ContentRule>
                <SelectedNodeInstances />
              </ContentRule>
            </PresentationRuleSet>
            )ruleset");
        return ruleset;
        }

    void ExtractElementsPaged(ContentDescriptorCR descriptor, size_t pageSize);
    };
/*
static Utf8String SerializeJson(rapidjson::Document const& json)
    {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
    }*/
/*
static void WriteToFile(BeFileNameCR directory, Utf8StringCR fileName, Utf8StringCR value)
    {
    auto path = directory;
    BeFile file;
    file.Create(path.AppendToPath(BeFileName(fileName)).c_str());
    file.Write(nullptr, value.data(), static_cast<uint32_t>(value.size()));
    }*/

#define KEEP_UNUSED(var) do { (void)var; } while(false);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesPublisherPerformanceTests::ExtractElementsPaged(ContentDescriptorCR descriptor, size_t pageSize)
    {
    PageOptions pageOptions;
    auto i = 0ull;
    pageOptions.SetPageSize(pageSize);
    ContentCPtr content;
    do
        {
        pageOptions.SetPageStart(i);
        content = m_manager->GetContent(descriptor, pageOptions).get();
        for (const auto contentItem : content->GetContentSet())
            {
            const auto instanceId = contentItem->GetKeys()[0].GetId();
            auto json = contentItem->AsJson();
            // auto serialized = SerializeJson(json);
            // WriteToFile(directory, fileName, serialized);
            KEEP_UNUSED(json);
            }
        i += pageSize;
        } while (content->GetContentSet().GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TilesPublisherPerformanceTests, GetPropertiesOfAllElementsInImodel)
    {
    /*
    - stepping through all elements just to get their classes?
    - stepping through all elements **again** and then filtering by class???
    - writing to file
    */

    Timer _timer(BeTest::GetNameOfCurrentTest());

    bset<Utf8String> classes;
    const auto options = RulesDrivenECPresentationManager::ContentOptions("Items").GetJson();

    ECSqlStatement elementsStmt1;
    elementsStmt1.Prepare(m_project, "SELECT ECClassId, ECInstanceId FROM BisCore.Element");

    while (BE_SQLITE_ROW == elementsStmt1.Step())
    //for (const auto classEntry : file->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Element)))
        {
        const auto ecClass = m_project.Schemas().GetClass(elementsStmt1.GetValueId<ECClassId>(0));
        const Utf8String className = ecClass->GetFullName();
        if (classes.find(className) != classes.end())
            continue;

        classes.insert(className);

        bvector<ECClassInstanceKey> keys;
        ECSqlStatement elementsStmt2;
        elementsStmt2.Prepare(m_project, "SELECT ECClassId, ECInstanceId FROM BisCore.Element");
        while (BE_SQLITE_ROW == elementsStmt2.Step())
        //for (auto instanceEntry : file->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Element)))
            {
            const auto instanceClass = m_project.Schemas().GetClass(elementsStmt2.GetValueId<ECClassId>(0));
            if (!className.CompareTo(instanceClass->GetFullName()))
                keys.push_back(ECClassInstanceKey(ecClass, elementsStmt2.GetValueId<ECInstanceId>(1)));
            }

        const auto set = KeySet::Create(keys);

        Utf8PrintfString _timerClassMsg("Getting content for '%s' with %" PRIu64 " elements", className.c_str(), (uint64_t)keys.size());
        Timer _timerClass(_timerClassMsg.c_str());

        ContentDescriptorCPtr descriptorPtr = m_manager->GetContentDescriptor(m_project, ContentDisplayType::PropertyPane, *set, nullptr, options).get();
        const auto descriptor = ContentDescriptor::Create(*descriptorPtr);
        descriptor->RemoveContentFlag(ContentFlags::MergeResults);
        ExtractElementsPaged(*descriptor, 50000);
        }
    }