/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include "../../NonPublished/RulesEngine/ECDbTestProject.h"
#include "../../NonPublished/RulesEngine/TestHelpers.h"

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct RuleSetEmbedderTests : ECPresentationTest
{
    static ECDbTestProject* s_project;
    RuleSetEmbedder* m_embedder;

    void SetUp()
        {
        ECPresentationTest::SetUp();
        m_embedder = new RuleSetEmbedder(s_project->GetECDb());
        }

    void TearDown()
        {
        s_project->GetECDb().AbandonChanges();
        DELETE_AND_CLEAR(m_embedder);
        }

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("RuleSetEmbedderTests");
        }
    
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
};
ECDbTestProject* RuleSetEmbedderTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetEmbedderTests, EmbedSingleRuleset)
    {
    // create ruleset and embed into ECDb
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset));

    // validate there is embedded ruleset
    DbEmbeddedFileTable& files = s_project->GetECDb().EmbeddedFiles();

    DbEmbeddedFileTable::Iterator filesIter = files.MakeIterator();
    ASSERT_EQ(filesIter.QueryCount(), 1);

    DbEmbeddedFileTable::Iterator::Entry const& file = filesIter.begin();
    EXPECT_STREQ(RuleSetEmbedder::FILE_TYPE_PresentationRuleSet, file.GetTypeUtf8());
    EXPECT_STREQ(ruleset->GetFullRuleSetId().c_str(), file.GetNameUtf8());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetEmbedderTests, EmbedMultipleRulesetsWithDifferentNames)
    {
    // create multiple rulesets and embed into ECDb
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleset1", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("TestRuleset2", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("TestRuleset3", 1, 0, false, "", "", "", false);
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset1));
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset2));
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset3));

    // validate there are embedded rulesets
    DbEmbeddedFileTable& files = s_project->GetECDb().EmbeddedFiles();

    DbEmbeddedFileTable::Iterator filesIter = files.MakeIterator();
    ASSERT_EQ(filesIter.QueryCount(), 3);

    for (DbEmbeddedFileTable::Iterator::Entry const& file : filesIter)
        {
        EXPECT_STREQ(RuleSetEmbedder::FILE_TYPE_PresentationRuleSet, file.GetTypeUtf8());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetEmbedderTests, EmbedMultipleRulesetsWithDifferentVersions)
    {
    // create multiple rulesets and embed into ECDb
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("TestRuleset", 2, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("TestRuleset", 1, 1, false, "", "", "", false);
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset1));
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset2));
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset3));

    // validate there are embedded rulesets
    DbEmbeddedFileTable& files = s_project->GetECDb().EmbeddedFiles();

    DbEmbeddedFileTable::Iterator filesIter = files.MakeIterator();
    ASSERT_EQ(filesIter.QueryCount(), 3);

    for (DbEmbeddedFileTable::Iterator::Entry const& file : filesIter)
        {
        EXPECT_STREQ(RuleSetEmbedder::FILE_TYPE_PresentationRuleSet, file.GetTypeUtf8());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetEmbedderTests, DontEmbedRuleSetsWithSameIds)
    {
    // create rulesets
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);

    // try embed them
    ASSERT_EQ(BE_SQLITE_OK, m_embedder->Embed(*ruleset1));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, m_embedder->Embed(*ruleset2));
    }