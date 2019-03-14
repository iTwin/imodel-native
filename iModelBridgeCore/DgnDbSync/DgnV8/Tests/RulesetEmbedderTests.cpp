/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/RulesetEmbedderTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include "..\RulesetEmbedder.h"
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <Bentley/BeTest.h>

struct RulesetEmbedderTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);

    DgnDbPtr m_db;
    BentleyApi::ECPresentation::ConnectionManager m_connections;
    BentleyApi::ECPresentation::RulesDrivenECPresentationManager* m_presentationManager;
    BentleyApi::Utf8CP RuleSetJsonString1 = R"(
        {
            "id" : "default",
            "rules" : 
            [
                {
                    "ruleType": "RootNodes",
                    "specifications" : 
                    [{
                        "specType": "CustomNode",
                            "type" : "root",
                            "label" : "root"
                    }]
                }
            ]
        }
    )";

    BentleyApi::Utf8CP RuleSetJsonString2 = R"(
        {
            "id" : "default",
            "rules" : 
            [
                {
                    "ruleType": "RootNodes",
                    "specifications" : 
                    [{
                        "specType": "CustomNode",
                            "type" : "root",
                            "label" : "root"
                    }]
                }, 
                {
                    "ruleType": "ChildNodes",
                    "specifications": 
                    [{
                        "specType": "AllInstanceNodes",
                        "groupByClass": true,
                        "groupByLabel": true
                    }]
                }
            ]
        }
    )";

    void SetUp();
    void TearDown();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::ECPresentation::RulesDrivenECPresentationManager::Paths GetPaths(BentleyApi::BeTest::Host& host)
    {
    BentleyApi::BeFileName assetsDirectory, temporaryDirectory;
    host.GetDgnPlatformAssetsDirectory(assetsDirectory);
    host.GetTempDir(temporaryDirectory);
    return BentleyApi::ECPresentation::RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetEmbedderTests::SetUp()
    {
    T_Super::SetUp();
    LineUpFiles(L"Design3dSelfReference.bim", L"Test3d.dgn", false);

    m_db = OpenExistingDgnDb(m_dgnDbFileName);
    m_presentationManager = new BentleyApi::ECPresentation::RulesDrivenECPresentationManager(m_connections, GetPaths(BentleyApi::BeTest::GetHost()));
    m_presentationManager->SetLocalizationProvider(new BentleyApi::ECPresentation::SQLangLocalizationProvider());
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(m_db->IsDbOpen());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetEmbedderTests::TearDown()
    {
    FREE_AND_CLEAR(m_presentationManager);
    m_db->AbandonChanges();
    m_db->CloseDb();
    T_Super::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetEmbedderTests, RulesetIsEmbeddedAndLocated)
    {
    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString1);
    ASSERT_TRUE(ruleset.IsValid());

    RulesetEmbedder embedder = RulesetEmbedder(*m_db);
    DgnElementId rulesetId = embedder.InsertRuleset(*ruleset);
    ASSERT_TRUE(rulesetId.IsValid());

    BentleyApi::ECPresentation::ConnectionManager manager;
    BentleyApi::ECPresentation::IConnectionPtr connection = manager.CreateConnection(*m_db);
    BentleyApi::RefCountedPtr<BentleyApi::ECPresentation::EmbeddedRuleSetLocater> locater = BentleyApi::ECPresentation::EmbeddedRuleSetLocater::Create(*connection);
    BentleyApi::bvector<BentleyApi::ECPresentation::PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(1 == rulesets.size());
    ASSERT_TRUE(ruleset->GetRuleSetId() == rulesets[0]->GetRuleSetId());
    ASSERT_TRUE(ruleset->WriteToJsonValue() == rulesets[0]->WriteToJsonValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetEmbedderTests, RulesetIsEmbeddedAndLocatedFromPresentationManager)
    {
    RulesetEmbedder embedder = RulesetEmbedder(*m_db);
    
    BentleyApi::ECPresentation::IConnectionPtr connection = m_presentationManager->Connections().CreateConnection(*m_db);
    BentleyApi::bvector<BentleyApi::ECPresentation::PresentationRuleSetPtr> rulesets = m_presentationManager->GetLocaters().LocateRuleSets(*connection, nullptr);
    int countBefore = rulesets.size();

    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString1);
    ASSERT_TRUE(ruleset.IsValid());

    DgnElementId rulesetId = embedder.InsertRuleset(*ruleset);
    ASSERT_TRUE(rulesetId.IsValid());

    rulesets = m_presentationManager->GetLocaters().LocateRuleSets(*connection, nullptr);
    ASSERT_TRUE(1 == rulesets.size() - countBefore);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetEmbedderTests, SameRulesetIsNotEmbeddedTwice)
    {
    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString1);
    ASSERT_TRUE(ruleset.IsValid());

    RulesetEmbedder embedder = RulesetEmbedder(*m_db);

    DgnElementId rulesetId1 = embedder.InsertRuleset(*ruleset);
    ASSERT_TRUE(rulesetId1.IsValid());

    DgnElementId rulesetId2 = embedder.InsertRuleset(*ruleset);
    ASSERT_EQ(rulesetId1, rulesetId2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetEmbedderTests, DuplicateRulesetIsSkipped)
    {
    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString1);
    ASSERT_TRUE(ruleset.IsValid());

    RulesetEmbedder embedder = RulesetEmbedder(*m_db);

    DgnElementId rulesetId1 = embedder.InsertRuleset(*ruleset);
    ASSERT_TRUE(rulesetId1.IsValid());

    BentleyApi::ECPresentation::PresentationRuleSetPtr changedRuleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString2);
    ASSERT_TRUE(0 == std::strcmp(ruleset->GetRuleSetId().c_str(), changedRuleset->GetRuleSetId().c_str()));
    ASSERT_FALSE(ruleset->WriteToJsonValue() == changedRuleset->WriteToJsonValue());

    DgnElementId rulesetId2 = embedder.InsertRuleset(*changedRuleset, RulesetEmbedder::DuplicateHandlingStrategy::SKIP);
    ASSERT_EQ(rulesetId1, rulesetId2);
    
    BentleyApi::ECPresentation::ConnectionManager manager;
    BentleyApi::ECPresentation::IConnectionPtr connection = manager.CreateConnection(*m_db);
    BentleyApi::RefCountedPtr<BentleyApi::ECPresentation::EmbeddedRuleSetLocater> locater = BentleyApi::ECPresentation::EmbeddedRuleSetLocater::Create(*connection);
    BentleyApi::bvector<BentleyApi::ECPresentation::PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(1 == rulesets.size());
    
    BentleyApi::ECPresentation::PresentationRuleSetPtr locatedRuleset = rulesets[0];
    ASSERT_TRUE(0 == std::strcmp(ruleset->GetRuleSetId().c_str(), locatedRuleset->GetRuleSetId().c_str()));
    ASSERT_TRUE(ruleset->WriteToJsonValue() == locatedRuleset->WriteToJsonValue());
    ASSERT_FALSE(changedRuleset->WriteToJsonValue() == locatedRuleset->WriteToJsonValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetEmbedderTests, DuplicateRulesetIsReplaced)
    {
    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString1);
    ASSERT_TRUE(ruleset.IsValid());

    RulesetEmbedder embedder = RulesetEmbedder(*m_db);

    DgnElementId rulesetId1 = embedder.InsertRuleset(*ruleset);
    ASSERT_TRUE(rulesetId1.IsValid());

    BentleyApi::ECPresentation::PresentationRuleSetPtr changedRuleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString2);
    ASSERT_TRUE(0 == std::strcmp(ruleset->GetRuleSetId().c_str(), changedRuleset->GetRuleSetId().c_str()));
    ASSERT_FALSE(ruleset->WriteToJsonValue() == changedRuleset->WriteToJsonValue());

    DgnElementId rulesetId2 = embedder.InsertRuleset(*changedRuleset, RulesetEmbedder::DuplicateHandlingStrategy::REPLACE);
    ASSERT_EQ(rulesetId1, rulesetId2);

    BentleyApi::ECPresentation::ConnectionManager manager;
    BentleyApi::ECPresentation::IConnectionPtr connection = manager.CreateConnection(*m_db);
    BentleyApi::RefCountedPtr<BentleyApi::ECPresentation::EmbeddedRuleSetLocater> locater = BentleyApi::ECPresentation::EmbeddedRuleSetLocater::Create(*connection);
    BentleyApi::bvector<BentleyApi::ECPresentation::PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(1 == rulesets.size());

    BentleyApi::ECPresentation::PresentationRuleSetPtr locatedRuleset = rulesets[0];
    ASSERT_TRUE(0 == std::strcmp(changedRuleset->GetRuleSetId().c_str(), locatedRuleset->GetRuleSetId().c_str()));
    ASSERT_TRUE(changedRuleset->WriteToJsonValue() == locatedRuleset->WriteToJsonValue());
    ASSERT_FALSE(ruleset->WriteToJsonValue() == locatedRuleset->WriteToJsonValue());
    }