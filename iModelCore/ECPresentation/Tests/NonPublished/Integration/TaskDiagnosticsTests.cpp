/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationManagerIntegrationTests.h"
#include <regex>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDiagnosticTests : PresentationManagerIntegrationTests
    {
    virtual void _ConfigureManagerParams(ECPresentationManager::Params& p) override
        {
        p.SetMultiThreadingParams(ECPresentationManager::Params::MultiThreadingParams(make_bpair(INT_MAX, 4)));
        }
    virtual std::unique_ptr<IConnectionManager> _CreateConnectionManager() override
        {
        // return no manager to use default
        return nullptr;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ExpectedScopesDef : bpair<Utf8CP, Nullable<bvector<ExpectedScopesDef>>>
    {
    ExpectedScopesDef(Utf8CP name)
        : bpair<Utf8CP, Nullable<bvector<ExpectedScopesDef>>>(name, nullptr)
        {}
    ExpectedScopesDef(Utf8CP name, bvector<ExpectedScopesDef> childScopeDefs)
        : bpair<Utf8CP, Nullable<bvector<ExpectedScopesDef>>>(name, childScopeDefs)
        {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String EscapeForRegularExpression(Utf8CP in)
    {
    static Utf8CP metacharacters = R"(\.^$-+()[]{}|)";
    Utf8String out;
    for (size_t i = 0; i < strlen(in); ++i) 
        {
        if (std::strchr(metacharacters, in[i]))
            out.push_back('\\');
        out.push_back(in[i]);
        }
    return out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssertPartialScopesMatch(RapidJsonValueCR scopeJson, ExpectedScopesDef const& expectedScopes)
    {
    Utf8String regexPattern = EscapeForRegularExpression(expectedScopes.first);
    regexPattern.ReplaceAll("*", ".*");
    regexPattern.ReplaceAll("?", ".");

    ASSERT_TRUE(std::regex_match(scopeJson["scope"].GetString(), std::regex(regexPattern)))
        << "Actual scope `" << scopeJson["scope"].GetString() << "` doesn't match expected scope pattern `" << expectedScopes.first << "`";

    if (expectedScopes.second.IsNull())
        return;

    auto const& scopeLogs = scopeJson["logs"];
    size_t scopeLogIndex = 0;
    for (size_t i = 0; i < expectedScopes.second.Value().size(); ++i)
        {
        while (scopeLogIndex < scopeLogs.Size() && (!scopeLogs[scopeLogIndex].IsObject() || !scopeLogs[scopeLogIndex].HasMember("scope")))
            ++scopeLogIndex;
        if (scopeLogIndex >= scopeLogs.Size())
            FAIL() << "Expected more scopes than there actually are";

        AssertPartialScopesMatch(scopeLogs[scopeLogIndex], expectedScopes.second.Value().at(i));
        ++scopeLogIndex;
        }

    for (; scopeLogIndex < scopeLogs.Size(); ++scopeLogIndex)
        {
        ASSERT_TRUE(!scopeLogs[scopeLogIndex].IsObject() || !scopeLogs[scopeLogIndex].HasMember("scope"))
            << "Found more scopes than expected";
        }
    }

/*---------------------------------------------------------------------------------**//**
* Simulate a case where content set size and content are requested immediately one
* after other.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(BuildsCorrectTaskDiagnosticsWithMultipleChainedTasksExecutedInParallel, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(TaskDiagnosticTests, BuildsCorrectTaskDiagnosticsWithMultipleChainedTasksExecutedInParallel)
    {
    auto classA = GetClass("A");
    auto instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    auto ruleset = PresentationRuleSet::CreateInstance("test");
    auto rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    Diagnostics::Options diagnosticsOptions;
    diagnosticsOptions.SetMinimumDuration((uint64_t)0);

    std::vector<std::shared_ptr<Diagnostics::Scope>> diagnosticScopes;
    std::vector<folly::Future<rapidjson::Document>> futures;
    {
    auto sizeScope = Diagnostics::Scope::ResetAndCreate("size", diagnosticsOptions);
    diagnosticScopes.push_back(*sizeScope);
    futures.push_back(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()))
        .then([this, sizeScope = *sizeScope](ContentDescriptorResponse r)
            {
            auto diagnostics = sizeScope->Hold();
            return m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), **r));
            })
        .then([sizeScope = *sizeScope](ContentSetSizeResponse) -> rapidjson::Document
            {
            return sizeScope->BuildJson();
            }));
    }
    {
    auto contentScope = Diagnostics::Scope::ResetAndCreate("content", diagnosticsOptions);
    diagnosticScopes.push_back(*contentScope);
    futures.push_back(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()))
        .then([this, contentScope = *contentScope](ContentDescriptorResponse r)
            {
            auto diagnostics = contentScope->Hold();
            return m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), **r));
            })
        .then([contentScope = *contentScope](ContentResponse) -> rapidjson::Document
            {
            return contentScope->BuildJson();
            }));
    }

    auto requestDiagnostics = folly::collectAll(futures).get();
    ASSERT_EQ(diagnosticScopes.size(), requestDiagnostics.size());

    AssertPartialScopesMatch(diagnosticScopes[0]->BuildJson(), 
        ExpectedScopesDef("size", 
            {
            ExpectedScopesDef("Get content descriptor", 
                {
                ExpectedScopesDef("Tasks scheduler: schedule task *"),
                ExpectedScopesDef("Task * executing"),
                ExpectedScopesDef("Task * completing"),
                }),
            ExpectedScopesDef("Get content set size", 
                {
                ExpectedScopesDef("Tasks scheduler: schedule task *"),
                ExpectedScopesDef("Task * executing"),
                ExpectedScopesDef("Task * completing"),
                }),
            }));

    AssertPartialScopesMatch(diagnosticScopes[1]->BuildJson(),
        ExpectedScopesDef("content", 
            {
            ExpectedScopesDef("Get content descriptor", 
                {
                ExpectedScopesDef("Tasks scheduler: schedule task *"),
                ExpectedScopesDef("Task * executing"),
                ExpectedScopesDef("Task * completing"),
                }),
            ExpectedScopesDef("Get content", 
                {
                ExpectedScopesDef("Tasks scheduler: schedule task *"),
                ExpectedScopesDef("Task * executing"),
                ExpectedScopesDef("Task * completing"),
                }),
        }));

    // confirm diagnostic scope content after task completion matches what we had in `then` callback
    for (size_t i = 0; i < diagnosticScopes.size(); ++i)
        {
        EXPECT_EQ(diagnosticScopes[i]->BuildJson(), requestDiagnostics[i].value())
            << "Diagnostic scope content after task completion doesn't match what we had in `then` callback \r\n"
            << "  After task completion: \r\n" << BeRapidJsonUtilities::ToString(diagnosticScopes[i]->BuildJson()).c_str() << "\r\n"
            << "  In `then` callback: \r\n" << BeRapidJsonUtilities::ToString(requestDiagnostics[i].value()).c_str() << "\r\n";
        }
    }