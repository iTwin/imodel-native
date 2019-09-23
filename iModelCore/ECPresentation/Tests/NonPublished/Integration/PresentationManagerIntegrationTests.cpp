/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PresentationManagerIntegrationTests.h"
#include "../RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

ECDbTestProject* PresentationManagerIntegrationTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<Utf8String, Utf8String>& GetRegisteredSchemaXmls()
    {
    static bmap<Utf8String, Utf8String> s_registeredSchemaXmls;
    return s_registeredSchemaXmls;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("PresentationManagerIntegrationTests", "RulesEngineTest.01.00.ecschema.xml");

    bvector<ECSchemaPtr> schemas;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(s_project->GetECDb().GetSchemaLocater());
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

    if (!schemas.empty())
        {
        bvector<ECSchemaCP> importSchemas;
        importSchemas.resize(schemas.size());
        std::transform(schemas.begin(), schemas.end(), importSchemas.begin(), [](ECSchemaPtr const& schema) { return schema.get(); });

        ASSERT_TRUE(SUCCESS == s_project->GetECDb().Schemas().ImportSchemas(importSchemas));
        s_project->GetECDb().SaveChanges();
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::RegisterSchemaXml(Utf8String name, Utf8String schemaXml) {GetRegisteredSchemaXmls()[name] = schemaXml;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::_ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params)
    {
    RulesDrivenECPresentationManager::Params::CachingParams cachingParams;
    cachingParams.SetDisableDiskCache(true);
    params.SetCachingParams(cachingParams);
    params.SetConnections(m_connections);
    params.SetLocalizationProvider(&m_localizationProvider);
    params.SetLocalState(&m_localState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::SetUp()
    {
    ECPresentationTest::SetUp();
    Localization::Init();

    m_connections = new TestConnectionManager();

    RulesDrivenECPresentationManager::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    _ConfigureManagerParams(params);

    m_manager = new RulesDrivenECPresentationManager(params);
    m_manager->GetConnections().CreateConnection(s_project->GetECDb());

    m_locater = DelayLoadingRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    m_locater = nullptr;
    DELETE_AND_CLEAR(m_manager);
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::InitTestL10N()
    {
    BeFileName sqlangFile;
    BeTest::GetHost().GetDocumentsRoot(sqlangFile);
    sqlangFile.AppendToPath(L"ECPresentationTestData");
    sqlangFile.AppendToPath(L"RulesEngineLocalizedStrings.sqlang.db3");
    BeSQLite::L10N::Shutdown();
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::ShutDownTestL10N()
    {
    BeSQLite::L10N::Shutdown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP PresentationManagerIntegrationTests::GetSchema()
    {
    return s_project->GetECDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP PresentationManagerIntegrationTests::GetClass(Utf8CP schemaName, Utf8CP className)
    {
    return s_project->GetECDb().Schemas().GetClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP PresentationManagerIntegrationTests::GetClass(Utf8CP name)
    {
    return GetClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP PresentationManagerIntegrationTests::GetRelationshipClass(Utf8CP schemaName, Utf8CP className)
    {
    ECClassCP ecClass = GetClass(schemaName, className);
    return (nullptr == ecClass) ? nullptr : ecClass->GetRelationshipClassCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP PresentationManagerIntegrationTests::GetRelationshipClass(Utf8CP name)
    {
    return GetRelationshipClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationManagerIntegrationTests::GetClassNamesList(bvector<ECClassCP> const& classes)
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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationManagerIntegrationTests::GetDisplayLabel(IECInstanceCR instance)
    {
    Utf8String label;
    if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
        return label;

    return instance.GetClass().GetDisplayLabel();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetDefaultDisplayLabel(Utf8StringCR className, ECInstanceId id)
    {
    Utf8String label = className;
    label.append("-");
    label.append(CommonTools::ToBase36String(CommonTools::GetBriefcaseId(id)));
    label.append("-");
    label.append(CommonTools::ToBase36String(CommonTools::GetLocalId(id)));
    return label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationManagerIntegrationTests::GetDefaultDisplayLabel(IECInstanceCR instance)
    {
    ECInstanceId id;
    ECInstanceId::FromString(id, instance.GetInstanceId().c_str());
    return ::GetDefaultDisplayLabel(instance.GetClass().GetDisplayLabel(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::SetUpDefaultLabelRule(PresentationRuleSetR rules)
    {
    rules.AddPresentationRule(*new LabelOverride("", -9999, "ThisNode.ClassName & \"-\" & ThisNode.BriefcaseId & \"-\" & ThisNode.LocalId", ""));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerIntegrationTests, InitializesUserSettings)
    {
    ASSERT_TRUE(m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").empty());

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("MyRulesetId", 1, 0, false, "", "", "", false);
    UserSettingsGroupP settingsGroup = new UserSettingsGroup("Label");
    settingsGroup->AddSettingsItem(*new UserSettingsItem("TestSetting", "Label", "StringValue", "DefaultValue"));
    ruleset->AddPresentationRule(*settingsGroup);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*locater);
    locater->AddRuleSet(*ruleset);

    ASSERT_STREQ("DefaultValue", m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").c_str());
    }
