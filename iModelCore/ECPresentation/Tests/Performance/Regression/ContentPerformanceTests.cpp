/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Content.h>
#include "../PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* Contains content-specific tests that may be run regularly to detect performance regressions
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentPerformanceTests : RulesEngineSingleProjectTests
    {
    void SetUp() override
        {
        RulesEngineSingleProjectTests::SetUp();

        BeFileName assetsDirectory;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
        BeFileName supplementalRulesetsDirectory(assetsDirectory);
        supplementalRulesetsDirectory.AppendToPath(L"SupplementalRulesets");
        m_manager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalRulesetsDirectory.GetNameUtf8().c_str())));
        }

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
            </PresentationRuleSet>
            )ruleset");
        return ruleset;
        }

    void GetContent(SelectionInfo const*, KeySetCR inputKeys, Utf8CP type, int expectedContentSize, int flags, Utf8CP passName);
    void GetContentForAllGeometricElements(Utf8CP type, int expectedContentSize, int flags);
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentPerformanceTests::GetContent(SelectionInfo const* selection, KeySetCR inputKeys, Utf8CP type, int expectedContentSize, int flags, Utf8CP passName)
    {
    // start the timer
    Utf8PrintfString timerName("%s: %s pass", BeTest::GetNameOfCurrentTest(), passName);
    Timer _timer(timerName.c_str());

    // get the descriptor
    auto descriptorParams = AsyncContentDescriptorRequestParams::Create(m_project, GetRulesetId(), RulesetVariables(), type, 0, inputKeys);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());
    if (descriptor->GetContentFlags() != (flags | descriptor->GetContentFlags()))
        {
        ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
        modifiedDescriptor->SetContentFlags(flags | descriptor->GetContentFlags());
        descriptor = modifiedDescriptor;
        }

    // get the content
    auto contentParams = AsyncContentRequestParams::Create(m_project, *descriptor);
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(contentParams));
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(expectedContentSize, content->GetContentSet().GetSize());
    for (ContentSetItemCPtr record : content->GetContentSet())
        EXPECT_TRUE(record.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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

    GetContent(selection.get(), *KeySet::Create(keys), type, expectedContentSize, flags, "First");
    GetContent(selection.get(), *KeySet::Create(keys), type, expectedContentSize, flags, "Second");
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @betest
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
    ContentDescriptorResponse descriptorResponse = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_project, "Items", RulesetVariables(), ContentDisplayType::PropertyPane, 0, *input)).get();
    EXPECT_TRUE(descriptorResponse.GetResult().IsValid());
    _timer1.Finish();

    /*
    WIP: need a way to clear content cache

    // force clear content cache so descriptors aren't cached
    m_manager->NotifyCategoriesChanged();

    Utf8PrintfString timerName2("%s: Second pass", BeTest::GetNameOfCurrentTest());
    Timer _timer2(timerName2.c_str());
    descriptorResponse = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_project, "Items", RulesetVariables(), ContentDisplayType::PropertyPane, 0, *input)).get();
    EXPECT_TRUE(descriptorResponse.GetResult().IsValid());
    */
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on Version Compare use case where the library requests content
* classes for all derived classes of BisCore:Element to find property relationship
* paths.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetContentClassesForBisElements)
    {
    ECClassCP elementClass = m_project.Schemas().GetClass("BisCore", "Element");

    Utf8PrintfString timerName1("%s: First pass", BeTest::GetNameOfCurrentTest());
    Timer _timer1(timerName1.c_str());
    ContentClassesResponse classesResponse = m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(m_project, "Items", RulesetVariables(), ContentDisplayType::PropertyPane, 0, bvector<ECClassCP>{ elementClass })).get();
    EXPECT_TRUE(!classesResponse.GetResult().empty());
    _timer1.Finish();

    Utf8PrintfString timerName2("%s: Second pass", BeTest::GetNameOfCurrentTest());
    Timer _timer2(timerName2.c_str());
    classesResponse = m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(m_project, "Items", RulesetVariables(), ContentDisplayType::PropertyPane, 0, bvector<ECClassCP>{ elementClass })).get();
    EXPECT_TRUE(!classesResponse.GetResult().empty());
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on a use case where application has a list of instance keys and wants
* to get their display labels.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetDisplayLabels)
    {
    GetContentForAllGeometricElements(ContentDisplayType::List, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetPropertyPaneContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetPropertyPaneContentWithLabelsForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1, (int)ContentFlags::ShowLabels);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for grid view
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetGridContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Grid, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for dgn view
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetGraphicsContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Graphics, 7414);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FunctionalRelatedContentPerformanceTests : ContentPerformanceTests
    {
    BeFileName _SupplyProjectPath() const override
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(L"Datasets");
        path.AppendToPath(L"Functional-ea6313fa-8322-47f6-b0e4-7edf717394d7.bim");
        return path;
        }
    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonString(R"json({
            "id": "PlantSightPhysicalProperties",
            "rules": [{
                "ruleType": "Content",
                "condition": "SelectedNode.IsOfClass(\"FunctionalElement\", \"Functional\")",
                "specifications": [{
                    "specType": "ContentRelatedInstances",
                    "relationships": {
                        "schemaName": "Functional",
                        "classNames": ["PhysicalElementFulfillsFunction"]
                    },
                    "requiredDirection": "Backward",
                    "relatedClasses": {
                        "schemaName": "ProcessPhysical",
                        "classNames": ["PLANT_BASE_OBJECT"]
                    },
                    "arePolymorphic": true
                }]
            }]
        })json");
        return ruleset;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FunctionalRelatedContentPerformanceTests, GetPropertyGridContent)
    {
    ECClassCP functionalElementClass = m_project.Schemas().GetClass("ProcessFunctional", "TOWER");
    ASSERT_NE(nullptr, functionalElementClass);
    ECClassInstanceKey inputKey(functionalElementClass, (ECInstanceId)ECInstanceId::FromString("0x30000000400"));
    GetContent(nullptr, *KeySet::Create({inputKey}), ContentDisplayType::PropertyPane, 1, 0, "First");
    GetContent(nullptr, *KeySet::Create({inputKey}), ContentDisplayType::PropertyPane, 1, 0, "Second");
    }

/*=================================================================================**//**
* @bsiclass
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideContentPerformanceTests, GetGridContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Grid, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideContentPerformanceTests, GetPropertyPaneContentWithLabelsForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1, (int)ContentFlags::ShowLabels);
    }

/*=================================================================================**//**
* @bsiclass
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideContentPerformanceTests, GetGridContentForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::Grid, 7414);
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideContentPerformanceTests, GetPropertyPaneContentWithLabelsForAllGeometricElements)
    {
    GetContentForAllGeometricElements(ContentDisplayType::PropertyPane, 1, (int)ContentFlags::ShowLabels);
    }

struct ContentFilteringPerformanceTests : ContentPerformanceTests
    {
    BeFileName _SupplyProjectPath() const override
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(L"Datasets");
        path.AppendToPath(L"M - Bentley - DRWR04-IFC.bim");
        // path.AppendToPath(L"L - SRD Hatch Internar.bim");
        return path;
        }

    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("FilterContentRules");
        ContentRule* rule = new ContentRule();
        rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(100, false, "", { new MultiSchemaClass("BisCore", true, {"GeometricElement3d"}) }, {}, true));
        ruleset->AddPresentationRule(*rule);
        return ruleset;
        }
    };

/*---------------------------------------------------------------------------------**//**
* The test is based on DGN view selection use case where the user uses fence selection to
* select a bunch of elements and the rules engine has to get content for property pane
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFilteringPerformanceTests, GetAllContent)
    {
    // get the descriptor
    auto descriptorParams = AsyncContentDescriptorRequestParams::Create(m_project, GetRulesetId(), RulesetVariables(), nullptr, 0, *KeySet::Create());
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());

    std::shared_ptr<Timer> _timer = std::make_shared<Timer>("Getting content size");
    // get the content
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_project, *descriptor)));
    EXPECT_EQ(14553, content->GetContentSet().GetSize());
    _timer = nullptr;

    // IFCDynamic.Footprint
    ECClassCP filterClass = m_project.Schemas().GetClass("IFCDynamic", "Footprint");
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(InstanceFilterDefinition("this.IFCClassName=\"IfcPipeSegment\"", filterClass, {}));

    // get filtered content
    _timer = std::make_shared<Timer>("Getting filterred content size");
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_project, *ovr)));
    EXPECT_EQ(775, content->GetContentSet().GetSize());
    _timer = nullptr;
    }