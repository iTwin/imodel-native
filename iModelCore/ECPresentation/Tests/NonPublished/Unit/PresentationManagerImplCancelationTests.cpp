/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../Helpers/TestHelpers.h"
#include "../Helpers/TestRulesDrivenECPresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(RulesDrivenECPresentationManagerImplCancelationTests, name, schema_xml)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CountCancelationToken : ICancelationToken
{
private:
    mutable int m_currCount;
    int m_targetCount;
protected:
    bool _IsCanceled() const override 
        {
        m_currCount++;
        if (m_currCount >= m_targetCount)
            return true;
        return false;
        }
public:
    CountCancelationToken(int index) 
        : m_currCount(0), m_targetCount(index)
    {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplCancelationTests : ECPresentationTest
    {
    DECLARE_SCHEMA_REGISTRY(RulesDrivenECPresentationManagerImplCancelationTests);
    static ECDbTestProject* s_project;

    std::shared_ptr<TestConnectionManager> m_connections;
    IConnectionPtr m_connection;
    RulesDrivenECPresentationManagerImpl* m_impl;
    TestCategorySupplier m_categorySupplier;
    TestRuleSetLocaterPtr m_locater;

    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    void Reset();
    void Clear();

    template <typename TReturn>
    TReturn GetCancelationCount(std::function<TReturn(ICancelationTokenCR)> function, int& count);
    template <typename TReturn>
    void RunAllPossibleCancelations(int cancelationCheckCount, std::function<TReturn(ICancelationTokenCR)> function, TReturn const& expectedReturn, std::function<void(TReturn const&, TReturn const&)> validationFunction);
    template <typename TReturn>
    void TestCancelations(std::function<TReturn(ICancelationTokenCR)> function, std::function<void(TReturn const&, TReturn const&)> validationFunction);

    ECClassCP GetClass(Utf8CP name);
    ECClassCP GetClass(Utf8CP schemaName, Utf8CP className);
    static void ValidateContentSets(Content const& expectedResult, Content const& actualResult);


    RulesDrivenECPresentationManagerImplCancelationTests()
        : m_categorySupplier(ContentDescriptor::Category("cat", "cat", "descr", 1)), m_impl(nullptr)
        {}
    };
ECDbTestProject* RulesDrivenECPresentationManagerImplCancelationTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(RulesDrivenECPresentationManagerImplCancelationTests);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerImplCancelationTests");
    INIT_SCHEMA_REGISTRY(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_locater = TestRuleSetLocater::Create();
    m_connections = std::make_shared<TestConnectionManager>();
    Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::TearDown()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::Clear()
    {
    m_connections->CloseConnections();
    m_connection = nullptr;
    DELETE_AND_CLEAR(m_impl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::Reset()
    {
    Clear();
    RulesDrivenECPresentationManagerImpl::Params::CachingParams cachingParams;
#ifdef USE_HYBRID_CACHE
    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    BeFileName::EmptyDirectory(temporaryDirectory);
    cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Hybrid);
    cachingParams.SetCacheDirectoryPath(temporaryDirectory);
#else
    cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Memory);
#endif
    RulesDrivenECPresentationManagerImpl::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    params.SetConnections(m_connections);
    params.SetCachingParams(cachingParams);
    params.SetCategorySupplier(&m_categorySupplier);

    m_impl = new RulesDrivenECPresentationManagerImpl(params);
    m_impl->GetLocaters().RegisterLocater(*m_locater);
    m_impl->GetConnections().CreateConnection(s_project->GetECDb());
    m_connection = m_impl->GetConnections().GetConnection(s_project->GetECDb());
    m_impl->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP RulesDrivenECPresentationManagerImplCancelationTests::GetClass(Utf8CP schemaName, Utf8CP className)
    {
    return s_project->GetECDb().Schemas().GetClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP RulesDrivenECPresentationManagerImplCancelationTests::GetClass(Utf8CP name)
    {
    return GetClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename TReturn>
TReturn RulesDrivenECPresentationManagerImplCancelationTests::GetCancelationCount(std::function<TReturn(ICancelationTokenCR)> function, int& count)
    {
    RefCountedPtr<TestCancelationToken> cancelationToken = new TestCancelationToken([&]() mutable -> bool
        {
        count++;
        return false;
        });
    return function(*cancelationToken);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename TReturn>
void RulesDrivenECPresentationManagerImplCancelationTests::RunAllPossibleCancelations(int cancelationCheckCount, std::function<TReturn(ICancelationTokenCR)> function, TReturn const& expectedReturn, std::function<void(TReturn const&, TReturn const&)> validationFunction)
    {
    for (int i = 1; i <= cancelationCheckCount; ++i)
        {
        Reset();
        RefCountedPtr<CountCancelationToken> cancelationToken = new CountCancelationToken(i);
        bool wasCancelled = false;
        try
            {
            function(*cancelationToken);
            }
        catch (CancellationException const&)
            {
            wasCancelled = true;
            }
        EXPECT_TRUE(wasCancelled);
        validationFunction(expectedReturn, function(*NeverCanceledToken::Create()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename TReturn>
void RulesDrivenECPresentationManagerImplCancelationTests::TestCancelations(std::function<TReturn(ICancelationTokenCR)> function, std::function<void(TReturn const&, TReturn const&)> validationFunction)
    {
    int cancelationCheckCount = 0;
    auto expectedReturn = GetCancelationCount<TReturn>(function, cancelationCheckCount);
    RunAllPossibleCancelations<TReturn>(cancelationCheckCount, function, expectedReturn, validationFunction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplCancelationTests::ValidateContentSets(Content const& expectedResult, Content const& actualResult)
    {
    ASSERT_EQ(expectedResult.GetContentSet().GetSize(), actualResult.GetContentSet().GetSize());
    DataContainer<ContentSetItemCPtr> expectedContentSet = expectedResult.GetContentSet();
    DataContainer<ContentSetItemCPtr> actualContentSet = actualResult.GetContentSet();
    for (int i = 0; i < expectedContentSet.GetSize(); ++i)
        {
        EXPECT_EQ(actualContentSet.Get(i)->GetClass(), expectedContentSet.Get(i)->GetClass());
        EXPECT_EQ(actualContentSet.Get(i)->GetDisplayLabelDefinition(), expectedContentSet.Get(i)->GetDisplayLabelDefinition());
        EXPECT_EQ(actualContentSet.Get(i)->GetDisplayValues(), expectedContentSet.Get(i)->GetDisplayValues());
        EXPECT_EQ(actualContentSet.Get(i)->GetImageId(), expectedContentSet.Get(i)->GetImageId());
        EXPECT_EQ(actualContentSet.Get(i)->GetInputKeys(), expectedContentSet.Get(i)->GetInputKeys());
        EXPECT_EQ(actualContentSet.Get(i)->GetKeys(), expectedContentSet.Get(i)->GetKeys());
        EXPECT_EQ(actualContentSet.Get(i)->GetMergedFieldNames(), expectedContentSet.Get(i)->GetMergedFieldNames());
        EXPECT_EQ(actualContentSet.Get(i)->GetNestedContent(), expectedContentSet.Get(i)->GetNestedContent());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetNodes, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetNodes)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ruleset->AddPresentationRule(*new RootNodeRule("", 1, false));
    ruleset->GetRootNodesRules().back()->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    m_locater->AddRuleSet(*ruleset);

    TestCancelations<NavNodesContainer>(
        [&](ICancelationTokenCR token) -> NavNodesContainer 
            {
            INavNodesDataSourcePtr source = m_impl->GetNodes(HierarchyRequestImplParams::Create(*m_connection, &token, ruleset->GetRuleSetId(), RulesetVariables()));
            if (source.IsNull())
                return NavNodesContainer();
            return NavNodesContainer(*ConstNodesDataSource::Create(*source));
            }, 
        [](NavNodesContainer const& expectedResult, NavNodesContainer const& actualResult)-> void
            {
            EXPECT_EQ(expectedResult.GetSize(), actualResult.GetSize());
            }
        );
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetNodesCount, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetNodesCount)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ruleset->AddPresentationRule(*new RootNodeRule("", 1, false));
    ruleset->GetRootNodesRules().back()->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    m_locater->AddRuleSet(*ruleset);

    TestCancelations<size_t>(
        [&](ICancelationTokenCR token) -> size_t
            {
            return m_impl->GetNodesCount(HierarchyRequestImplParams::Create(*m_connection, &token, ruleset->GetRuleSetId(), RulesetVariables()));
            }, 
        [](size_t const& expectedResult, size_t const& actualResult)-> void
            {
            EXPECT_EQ(expectedResult, actualResult);
            }
        );
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetFilteredNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetFilteredNodes)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("a")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("b")); });

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ruleset->AddPresentationRule(*new RootNodeRule("", 1, false));
    ruleset->GetRootNodesRules().back()->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "MyProperty"));
    m_locater->AddRuleSet(*ruleset);

    TestCancelations<bvector<NavNodeCPtr>>(
        [&](ICancelationTokenCR token) -> bvector<NavNodeCPtr>
            {
            return  m_impl->GetFilteredNodes(NodePathsFromFilterTextRequestImplParams::Create(*m_connection, &token, ruleset->GetRuleSetId(), RulesetVariables(), "a"));
            }, 
        [](bvector<NavNodeCPtr> const& expectedResult, bvector<NavNodeCPtr> const& actualResult)-> void
            {
            ASSERT_EQ(expectedResult.size(), actualResult.size());
            for (int i = 0; i < expectedResult.size(); i++)
                EXPECT_TRUE(expectedResult.at(i)->Equals(*actualResult.at(i)));
            }
        ); 
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetContentDescriptor, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetContentDescriptor)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("a")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("b")); });

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    TestCancelations<ContentDescriptorCPtr>(
        [&](ICancelationTokenCR token) -> ContentDescriptorCPtr 
            {
            return m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*m_connection, &token,
            ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));
            }, 
        [](ContentDescriptorCPtr const& expectedResult, ContentDescriptorCPtr const& actualResult)-> void
            {
            ASSERT_TRUE(expectedResult.IsValid());
            ASSERT_TRUE(actualResult.IsValid());
            EXPECT_TRUE(expectedResult->Equals(*actualResult));
            }
        );
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetContent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetContent)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("a")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("b")); });

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);
    auto descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*m_connection, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    TestCancelations<ContentCPtr>(
        [&](ICancelationTokenCR token) -> ContentCPtr 
            {
            ContentCPtr content = m_impl->GetContent(ContentRequestImplParams::Create(*m_connection, &token, *descriptor));
            if (content.IsNull())
                return nullptr;
            return ContentCPtr(Content::Create(content->GetDescriptor(), *PreloadedDataSource<ContentSetItemCPtr>::Create(content->GetContentSet().GetDataSource())));
            }, 
        [](ContentCPtr const& expectedResult, ContentCPtr const& actualResult)-> void
            {
            ValidateContentSets(*expectedResult, *actualResult);
            EXPECT_TRUE(expectedResult->GetDescriptor().Equals(actualResult->GetDescriptor()));
            }
        );
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetContentSetSize, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetContentSetSize)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("a")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("b")); });

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);
    auto descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*m_connection, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    TestCancelations<size_t>(
        [&](ICancelationTokenCR token) -> size_t 
            {
            return m_impl->GetContentSetSize(ContentRequestImplParams::Create(*m_connection, &token, *descriptor));
            }, 
        [](size_t const& expectedResult, size_t const& actualResult)-> void
            {
            EXPECT_EQ(expectedResult, actualResult);
            }
        );
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDisplayLabel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerImplCancelationTests, GetDisplayLabel) 
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("MyProperty", ECValue("a"));});

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());
    ruleset->AddPresentationRule(*rule);
    ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "MyProperty"));
    m_locater->AddRuleSet(*ruleset);
    auto keys = KeySet::Create({ ECClassInstanceKey(classA, ECInstanceId(BeInt64Id::FromString(instanceA->GetInstanceId().c_str()))) }); 
   
    TestCancelations<LabelDefinitionCPtr>(
        [&](ICancelationTokenCR token) -> LabelDefinitionCPtr 
            {   
            return m_impl->GetDisplayLabel(KeySetDisplayLabelRequestImplParams::Create(*m_connection, &token, ruleset->GetRuleSetId(), RulesetVariables(), *keys));
            }, 
        [](LabelDefinitionCPtr const& expectedResult, LabelDefinitionCPtr const& actualResult)-> void
            {
            EXPECT_EQ(expectedResult->GetDisplayValue(), actualResult->GetDisplayValue());
            }
        );
    }