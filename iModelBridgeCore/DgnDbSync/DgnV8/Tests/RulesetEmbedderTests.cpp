/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/RulesetEmbedderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    BentleyApi::Utf8CP RuleSetJsonString = R"({
        "id" : "default",
        "rules" : [
        {
        "ruleType": "RootNodes",
            "specifications" : [
            {
            "specType": "CustomNode",
                "type" : "root",
                "label" : "root"
            }
            ]
        }
        ]
    })";

    void SetUp();
    void TearDown();
    };

void RulesetEmbedderTests::SetUp()
    {
    T_Super::SetUp();
    LineUpFiles(L"Design3dSelfReference.ibim", L"Test3d.dgn", false);

    m_db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECPresentation::IECPresentationManager::SetLocalizationProvider(new BentleyApi::ECPresentation::SQLangLocalizationProvider());
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(m_db->IsDbOpen());
    }

void RulesetEmbedderTests::TearDown()
    {
    m_db->AbandonChanges();
    m_db->CloseDb();
    T_Super::TearDown();
    }

BentleyApi::ECPresentation::RulesDrivenECPresentationManager::Paths GetPaths(BentleyApi::BeTest::Host& host)
    {
    BentleyApi::BeFileName assetsDirectory, temporaryDirectory;
    host.GetDgnPlatformAssetsDirectory(assetsDirectory);
    host.GetTempDir(temporaryDirectory);
    return BentleyApi::ECPresentation::RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory);
    }

TEST_F(RulesetEmbedderTests, RulesetIsEmbeddedAndLocated)
    {
    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString);
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

TEST_F(RulesetEmbedderTests, RulesetIsEmbeddedAndLocatedFromPresentationManager)
    {
    RulesetEmbedder embedder = RulesetEmbedder(*m_db);

    BentleyApi::ECPresentation::ConnectionManager connections;
    BentleyApi::ECPresentation::RulesDrivenECPresentationManager presentationManager(connections, GetPaths(BentleyApi::BeTest::GetHost()));
    BentleyApi::ECPresentation::IConnectionPtr connection = presentationManager.Connections().CreateConnection(*m_db);
    BentleyApi::bvector<BentleyApi::ECPresentation::PresentationRuleSetPtr> rulesets = presentationManager.GetLocaters().LocateRuleSets(*connection, nullptr);
    int countBefore = rulesets.size();

    BentleyApi::ECPresentation::PresentationRuleSetPtr ruleset = BentleyApi::ECPresentation::PresentationRuleSet::ReadFromJsonString(RuleSetJsonString);
    ASSERT_TRUE(ruleset.IsValid());

    DgnElementId rulesetId = embedder.InsertRuleset(*ruleset);
    ASSERT_TRUE(rulesetId.IsValid());

    rulesets = presentationManager.GetLocaters().LocateRuleSets(*connection, nullptr);
    ASSERT_TRUE(1 == rulesets.size() - countBefore);
    }