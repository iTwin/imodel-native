/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeDirectoryIterator.h>
#include "ECPresentationUtils.h"
#include "DgnDbECInstanceChangeEventSource.h"
#include "ECPresentationSerializer.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2018
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationLocalizationProvider : ILocalizationProvider
{
private:
    bvector<BeFileName> m_localeDirectories;
    mutable bmap<Utf8String, bmap<Utf8String, rapidjson::Document*>> m_cache;
private:
    static bvector<Utf8String> GetLocaleVariants(Utf8StringCR activeLocale)
        {
        bvector<Utf8String> variants;
        variants.push_back(activeLocale);
        Utf8String cultureNeutral;
        if (Utf8String::npos != activeLocale.GetNextToken(cultureNeutral, "-", 0) && cultureNeutral.length() != activeLocale.length())
            variants.push_back(cultureNeutral);
        if (!activeLocale.EqualsI("en") && !cultureNeutral.EqualsI("en"))
            variants.push_back("en");
        return variants;
        }
    bvector<BeFileName> GetLocalizationDirectoryPaths(Utf8StringCR locale) const
        {
        bvector<BeFileName> result;
        bvector<Utf8String> localeVariants = GetLocaleVariants(locale);
        for (Utf8StringCR locale : localeVariants)
            {
            for (BeFileName dir : m_localeDirectories)
                {
                BeFileName localeDir(dir);
                localeDir.AppendUtf8(locale.c_str());
                if (localeDir.DoesPathExist())
                    result.push_back(localeDir);
                }
            }
        return result;
        }
    bvector<BeFileName> GetLocalizationFilePaths(Utf8StringCR locale, Utf8StringCR ns) const
        {
        BeFileName nsFileName(ns);
        nsFileName.AppendExtension(L"json");
        bvector<BeFileName> result;
        bvector<BeFileName> dirs = GetLocalizationDirectoryPaths(locale);
        for (BeFileNameCR dir : dirs)
            {
            bvector<BeFileName> matches;
            BeDirectoryIterator::WalkDirsAndMatch(matches, dir, nsFileName.c_str(), true);
            result.insert(result.end(), matches.begin(), matches.end());
            }
        return result;
        }
    static rapidjson::Document ReadLocalizationFile(BeFileNameCR path)
        {
        rapidjson::Document json;
        BeFile file;
        if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Read))
            {
            BeAssert(false);
            return json;
            }
        bvector<Byte> data;
        if (BeFileStatus::Success != file.ReadEntireFile(data))
            {
            BeAssert(false);
            return json;
            }
        data.push_back(0);
        json.Parse((Utf8CP)&*data.begin());
        return json;
        }
    static void MergeLocalizationData(RapidJsonValueR target, rapidjson::Document::AllocatorType& targetAllocator, RapidJsonValueCR source)
        {
        if (source.IsObject())
            {
            BeAssert(target.IsNull() || target.IsObject());
            if (target.IsNull())
                target.SetObject();
            for (auto sourceIter = source.MemberBegin(); sourceIter != source.MemberEnd(); ++sourceIter)
                {
                Utf8CP key = sourceIter->name.GetString();
                if (!target.HasMember(key))
                    target.AddMember(rapidjson::Value(key, targetAllocator), rapidjson::Value(), targetAllocator);
                MergeLocalizationData(target[key], targetAllocator, sourceIter->value);
                }
            }
        else if (source.IsString())
            {
            BeAssert(target.IsNull() || target.IsString());
            // note: do not overwrite if the string is already set - we parse
            // more specific locales first and finish with less specific ones
            if (target.IsNull())
                target.SetString(source.GetString(), targetAllocator);
            }
        else
            {
            BeAssert(false);
            }
        }
    rapidjson::Document* CreateNamespace(Utf8StringCR locale, Utf8StringCR ns) const
        {
        if (m_localeDirectories.empty() || locale.empty())
            return nullptr;

        rapidjson::Document* json = new rapidjson::Document();
        json->SetObject();
        bvector<BeFileName> filePaths = GetLocalizationFilePaths(locale, ns);
        for (BeFileNameCR filePath : filePaths)
            MergeLocalizationData(*json, json->GetAllocator(), ReadLocalizationFile(filePath));
        return json;
        }
    void ClearCache()
        {
        for (auto localeEntry : m_cache)
            {
            for (auto namespaceEntry : localeEntry.second)
                DELETE_AND_CLEAR(namespaceEntry.second);
            }
        m_cache.clear();
        }
protected:
    bool _GetString(Utf8StringCR locale, Utf8StringCR key, Utf8StringR localizedValue) const override
        {
        Utf8String ns;
        size_t pos;
        if (Utf8String::npos == (pos = key.GetNextToken(ns, ":", 0)))
            return false;

        auto localeIter = m_cache.find(locale);
        if (m_cache.end() == localeIter)
            localeIter = m_cache.Insert(locale, bmap<Utf8String, rapidjson::Document*>()).first;

        auto namespaceIter = localeIter->second.find(ns);
        if (localeIter->second.end() == namespaceIter)
            namespaceIter = localeIter->second.Insert(ns, CreateNamespace(locale, ns)).first;

        rapidjson::Value const* curr = namespaceIter->second;
        if (nullptr == curr)
            return false;

        Utf8String id(key.begin() + pos, key.end());
        bvector<Utf8String> idPath;
        BeStringUtilities::Split(id.c_str(), ".", idPath);
        for (Utf8StringCR idPart : idPath)
            {
            auto iter = curr->FindMember(idPart.c_str());
            if (curr->MemberEnd() == iter)
                return false;
            curr = &iter->value;
            }
        if (!curr || !curr->IsString())
            return false;
        localizedValue.AssignOrClear(curr->GetString());
        return true;
        }
public:
    IModelJsECPresentationLocalizationProvider(bvector<BeFileName> directories = bvector<BeFileName>()): m_localeDirectories(directories) {}
    ~IModelJsECPresentationLocalizationProvider() {ClearCache();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationStaticSetupHelper
{
private:
    IModelJsECPresentationSerializer* m_serializer;
public:
    IModelJsECPresentationStaticSetupHelper()
        : m_serializer(new IModelJsECPresentationSerializer())
        {
        IECPresentationManager::SetSerializer(m_serializer);
        }
    ~IModelJsECPresentationStaticSetupHelper()
        {
        IECPresentationManager::SetSerializer(nullptr);
        }
    IModelJsECPresentationSerializer& GetSerializer() {return *m_serializer;}
    };
static IModelJsECPresentationStaticSetupHelper s_staticSetup;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager* ECPresentationUtils::CreatePresentationManager(Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin& locations,
    IJsonLocalState& localState, Utf8StringCR id, bvector<Utf8String> const& localeDirectories, bmap<int, unsigned> taskAllocationSlots, Utf8StringCR mode,
    bool isChangeTrackingEnabled, std::shared_ptr<IUpdateRecordsHandler> updateRecordsHandler, Utf8StringCR cacheDirectory)
    {
    BeFileName assetsDir = locations.GetDgnPlatformAssetsDirectory();
    BeFileName tempDir = locations.GetLocalTempDirectoryBaseName();
    tempDir.AppendToPath(L"ecpresentation");
    if (!tempDir.DoesPathExist())
        BeFileName::CreateNewDirectory(tempDir.c_str());
    if (!id.empty())
        {
        tempDir.AppendToPath(WString(id.c_str(), true).c_str());
        if (!tempDir.DoesPathExist())
            BeFileName::CreateNewDirectory(tempDir.c_str());
        }
    RulesDrivenECPresentationManager::Paths pathParams(assetsDir, tempDir);

    RulesDrivenECPresentationManager::Params::MultiThreadingParams threadingParams(taskAllocationSlots);

    RulesDrivenECPresentationManager::Params::CachingParams cachingParams(cacheDirectory);

    bvector<BeFileName> localeDirectoryPaths;
    for (Utf8StringCR dir : localeDirectories)
        localeDirectoryPaths.push_back(BeFileName(dir).AppendSeparator());

    RulesDrivenECPresentationManager::Params params(pathParams);
    params.SetMultiThreadingParams(threadingParams);
    params.SetCachingParams(cachingParams);
    params.SetLocalizationProvider(new IModelJsECPresentationLocalizationProvider(localeDirectoryPaths));
    params.SetLocalState(&localState);
    params.SetMode(mode.Equals("ro") ? RulesDrivenECPresentationManager::Mode::ReadOnly : RulesDrivenECPresentationManager::Mode::ReadWrite);
    if (isChangeTrackingEnabled)
        {
        params.SetECInstanceChangeEventSources({std::make_shared<DgnDbECInstanceChangeEventSource>()});
        params.SetUpdateRecordsHandlers({updateRecordsHandler});
        }
    return new RulesDrivenECPresentationManager(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetupRulesetDirectories(RulesDrivenECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str()));
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetupSupplementalRulesetDirectories(RulesDrivenECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str())));
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::GetRulesets(SimpleRuleSetLocater& locater, Utf8StringCR rulesetId)
    {
    bvector<PresentationRuleSetPtr> rulesets = locater.LocateRuleSets(rulesetId.c_str());
    Json::Value json(Json::arrayValue);
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        {
        Json::Value hashedRulesetJson;
        hashedRulesetJson["ruleset"] = ruleset->WriteToJsonValue();
        hashedRulesetJson["hash"] = ruleset->GetHash();
        json.append(hashedRulesetJson);
        }
    return ECPresentationResult(std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::AddRuleset(SimpleRuleSetLocater& locater, Utf8StringCR rulesetJsonString)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonString(rulesetJsonString);
    if (ruleset.IsNull())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Failed to create rule set from serialized JSON");
    locater.AddRuleSet(*ruleset);
    rapidjson::Document result;
    result.SetString(ruleset->GetHash().c_str(), result.GetAllocator());
    return ECPresentationResult(std::move(result));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::RemoveRuleset(SimpleRuleSetLocater& locater, Utf8StringCR rulesetId, Utf8StringCR hash)
    {
    bvector<PresentationRuleSetPtr> rulesets = locater.LocateRuleSets(rulesetId.c_str());
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        {
        if (ruleset->GetHash().Equals(hash))
            {
            locater.RemoveRuleSet(rulesetId);
            return ECPresentationResult(rapidjson::Value(true));
            }
        }
    return ECPresentationResult(rapidjson::Value(false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::ClearRulesets(SimpleRuleSetLocater& locater)
    {
    locater.Clear();
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value GetCommonOptions(JsonValueCR params)
    {
    if (!params.isMember("rulesetId") || !params["rulesetId"].isString())
        {
        static const Json::Value s_empty;
        return s_empty;
        }
    RulesDrivenECPresentationManager::CommonOptions options(params["rulesetId"].asCString());
    if (params.isMember("locale") && params["locale"].isString())
        options.SetLocale(params["locale"].asCString());
    if (params.isMember("unitSystem") && params["unitSystem"].isString())
        {
        Utf8CP unitSystem = params["unitSystem"].asCString();
        if (0 == strcmp("metric", unitSystem))
            options.SetUnitSystem(ECPresentation::UnitSystem::Metric);
        else if (0 == strcmp("british-imperial", unitSystem))
            options.SetUnitSystem(ECPresentation::UnitSystem::BritishImperial);
        else if (0 == strcmp("us-customary", unitSystem))
            options.SetUnitSystem(ECPresentation::UnitSystem::UsCustomary);
        else if (0 == strcmp("us-survey", unitSystem))
            options.SetUnitSystem(ECPresentation::UnitSystem::UsSurvey);
        }
    options.SetPriority((params.isMember("priority") && params["priority"].isInt()) ? params["priority"].asInt() : DEFAULT_REQUEST_PRIORITY);
    if (params.isMember("rulesetVariables") && params["rulesetVariables"].isArray())
        options.SetRulesetVariables(params["rulesetVariables"]);
    return options.GetJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static RulesDrivenECPresentationManager::NavigationOptions GetNavigationOptions(JsonValueCR params)
    {
    RulesDrivenECPresentationManager::NavigationOptions options(GetCommonOptions(params));
    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static RulesDrivenECPresentationManager::ContentOptions GetContentOptions(JsonValueCR params)
    {
    RulesDrivenECPresentationManager::ContentOptions options(GetCommonOptions(params));
    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PageOptions GetPageOptions(JsonValueCR params)
    {
    PageOptions pageOptions;
    if (params.isMember("paging"))
        {
        if (params["paging"].isMember("start"))
            pageOptions.SetPageStart((size_t)params["paging"]["start"].asUInt64());
        if (params["paging"].isMember("size"))
            pageOptions.SetPageSize((size_t)params["paging"]["size"].asUInt64());
        }
    return pageOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::GetRulesetVariableValue(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType)
    {
    rapidjson::Document response;
    IUserSettings& settings = manager.GetUserSettings().GetSettings(rulesetId);

    if (variableType.Equals("bool"))
        response.SetBool(settings.GetSettingBoolValue(variableId.c_str()));
    else if(variableType.Equals("string"))
        response.SetString(settings.GetSettingValue(variableId.c_str()).c_str(), response.GetAllocator());
    else if (variableType.Equals("int"))
        response.SetInt64(settings.GetSettingIntValue(variableId.c_str()));
    else if (variableType.Equals("int[]"))
        {
        response.SetArray();
        bvector<int64_t> intValues = settings.GetSettingIntValues(variableId.c_str());
        for (int64_t value : intValues)
            response.PushBack(value, response.GetAllocator());
        }
    else if (variableType.Equals("id64"))
        response.SetString(BeInt64Id(settings.GetSettingIntValue(variableId.c_str())).ToHexStr().c_str(), response.GetAllocator());
    else if (variableType.Equals("id64[]"))
        {
        response.SetArray();
        bvector<int64_t> intValues = settings.GetSettingIntValues(variableId.c_str());
        for (int64_t value : intValues)
            response.PushBack(rapidjson::Value(BeInt64Id(value).ToHexStr().c_str(), response.GetAllocator()), response.GetAllocator());
        }
    else
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "type");
    return ECPresentationResult(std::move(response));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetRulesetVariableValue(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType, JsonValueCR value)
    {
    IUserSettings& settings = manager.GetUserSettings().GetSettings(rulesetId);
    if (variableType.Equals("bool"))
        settings.SetSettingBoolValue(variableId.c_str(), value.asBool());
    else if (variableType.Equals("string"))
        settings.SetSettingValue(variableId.c_str(), value.asCString());
    else if (variableType.Equals("id64"))
        settings.SetSettingIntValue(variableId.c_str(), BeInt64Id::FromString(value.asCString()).GetValue());
    else if (variableType.Equals("id64[]"))
        {
        bvector<int64_t> values;
        for (Json::ArrayIndex i = 0; i < value.size(); i++)
            values.push_back(BeInt64Id::FromString(value[i].asCString()).GetValue());
        settings.SetSettingIntValues(variableId.c_str(), values);
        }
    else if(variableType.Equals("int"))
        settings.SetSettingIntValue(variableId.c_str(), value.asInt64());
    else if (variableType.Equals("int[]"))
        {
        bvector<int64_t> values;
        for (Json::ArrayIndex i = 0; i < value.size(); i++)
            values.push_back(value[i].asInt64());
        settings.SetSettingIntValues(variableId.c_str(), values);
        }
    else
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "type");
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NavNodeCPtr> GetNode(IECPresentationManager& mgr, IConnectionCR connection, JsonValueCR params, PresentationRequestContextCR context)
    {
    NavNodeKeyCPtr key = NavNodeKey::FromJson(connection, params["nodeKey"]);
    if (key.IsNull())
        return NavNodeCPtr();
    return mgr.GetNode(connection.GetECDb(), *key, GetNavigationOptions(params).GetJson(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetRootNodesCount(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    return manager.GetRootNodesCount(db, GetNavigationOptions(params).GetJson(), context)
        .then([](size_t count)
            {
            return ECPresentationResult(rapidjson::Value((int64_t) count));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetRootNodes(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    return manager.GetRootNodes(db, GetPageOptions(params), GetNavigationOptions(params).GetJson(), context)
        .then([context](DataContainer<NavNodeCPtr> nodes)
            {
            context.OnTaskStart();
            rapidjson::Document json;
            json.SetArray();
            for (NavNodeCPtr const& node : nodes)
                json.PushBack(node->AsJson(&json.GetAllocator()), json.GetAllocator());
            return ECPresentationResult(std::move(json));
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetChildrenCount(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetNavigationOptions(params).GetJson();
    return GetNode(manager, *connection, params, context)
        .then([&manager, &db, options, context](NavNodeCPtr parentNode)
            {
            context.OnTaskStart();

            if (parentNode.IsNull())
                {
                BeAssert(false);
                return folly::makeFutureWith([]() {return ECPresentationResult(ECPresentationStatus::InvalidArgument, "parent node");});
                }

            return manager.GetChildrenCount(db, *parentNode, options, context)
                .then([](size_t count)
                    {
                    return ECPresentationResult(rapidjson::Value((int64_t) count));
                    });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetChildren(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetNavigationOptions(params).GetJson();
    PageOptions pageOptions = GetPageOptions(params);
    return GetNode(manager, *connection, params, context)
        .then([&manager, &db, options, pageOptions, context](NavNodeCPtr parentNode)
            {
            context.OnTaskStart();

            if (parentNode.IsNull())
                {
                BeAssert(false);
                return folly::makeFutureWith([]() {return ECPresentationResult(ECPresentationStatus::InvalidArgument, "parent node");});
                }

            return manager.GetChildren(db, *parentNode, pageOptions, options, context)
                .then([context](DataContainer<NavNodeCPtr> nodes)
                    {
                    context.OnTaskStart();
                    rapidjson::Document json;
                    json.SetArray();
                    for (NavNodeCPtr const& node : nodes)
                        json.PushBack(node->AsJson(&json.GetAllocator()), json.GetAllocator());
                    return ECPresentationResult(std::move(json));
                    });
            });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 06/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult>  ECPresentationUtils::GetNodesPaths(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    if (!params.isMember("markedIndex"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "markedIndex");

    bvector<bvector<ECInstanceKey>> keys;
    JsonValueCR keyArraysJson = params["paths"];
    if (!keyArraysJson.isArray())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "paths");

    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetNavigationOptions(params).GetJson();
    for (Json::ArrayIndex x = 0; x < keyArraysJson.size(); x++)
        {
        JsonValueCR keysJson = keyArraysJson[x];

        if (!keysJson.isArray())
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("paths[%" PRIu64 "]", (uint64_t)x));

        keys.push_back(bvector<ECInstanceKey>());
        for (Json::ArrayIndex i = 0; i < keysJson.size(); i++)
            {
            ECInstanceId id;
            ECInstanceId::FromString(id, keysJson[i]["id"].asCString());
            if (!id.IsValid())
                return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("paths[%" PRIu64 "][%" PRIu64 "].id", (uint64_t)x, (uint64_t)i));

            ECClassCP ecClass = IModelJsECPresentationSerializer::GetClassFromFullName(*connection, keysJson[i]["className"].asCString());
            if (ecClass == nullptr)
                return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("paths[%" PRIu64 "][%" PRIu64 "].className", (uint64_t)x, (uint64_t)i));

            keys[x].push_back(ECInstanceKey(ecClass->GetId(), id));
            }
        }

    return manager.GetNodePaths(db, keys, params["markedIndex"].asInt64(), options, context)
        .then([context](bvector<NodesPathElement> result)
            {
            context.OnTaskStart();
            rapidjson::Document response;
            response.SetArray();
            for (size_t i = 0; i < result.size(); i++)
                response.PushBack(result[i].AsJson(&response.GetAllocator()), response.GetAllocator());
            return ECPresentationResult(std::move(response));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                06/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult>  ECPresentationUtils::GetFilteredNodesPaths(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    if (!params.isMember("filterText"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "filterText");

    Json::Value options = GetNavigationOptions(params).GetJson();
    return manager.GetFilteredNodePaths(db, params["filterText"].asCString(), options, context)
        .then([context](bvector<NodesPathElement> result)
            {
            context.OnTaskStart();
            rapidjson::Document response;
            response.SetArray();
            for (size_t i = 0; i < result.size(); i++)
                response.PushBack(result[i].AsJson(&response.GetAllocator()), response.GetAllocator());
            return ECPresentationResult(std::move(response));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NavNodesContainer> GetNodes(RulesDrivenECPresentationManager& manager, ECDbCR db, NavNodeCP parentNode,
    JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    if (parentNode)
        return manager.GetChildren(db, *parentNode, PageOptions(), jsonOptions, context);
    return manager.GetRootNodes(db, PageOptions(), jsonOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<folly::Unit> LoadHierarchyIncrementally(RulesDrivenECPresentationManager& manager, ECDbCR db, NavNodeCP parentNode,
    JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    return GetNodes(manager, db, parentNode, jsonOptions, context).then([&, jsonOptions, context](NavNodesContainer container)
        {
        std::vector<folly::Future<folly::Unit>> childrenFutures;
        for (NavNodeCPtr node : container)
            {
            if (node->HasChildren())
                childrenFutures.push_back(LoadHierarchyIncrementally(manager, db, node.get(), jsonOptions, context));
            }
        return folly::collect(childrenFutures).then();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::LoadHierarchy(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    Json::Value options = GetCommonOptions(params);
    return LoadHierarchyIncrementally(manager, db, nullptr, options, context).then([]()
        {
        return ECPresentationResult();
        });
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct DescriptorOverrideHelper
{
private:
    JsonValueCR m_overridesJson;
private:
    Utf8CP GetSortingFieldName() const { return m_overridesJson["sortingFieldName"].asCString(); }
    SortDirection GetSortDirection() const { return (SortDirection)m_overridesJson["sortDirection"].asInt(); }
    int GetContentFlags() const { return m_overridesJson["contentFlags"].asInt(); }
    Utf8CP GetFilterExpression() const { return m_overridesJson["filterExpression"].isNull() ? "" : m_overridesJson["filterExpression"].asCString(); }
    bvector<Utf8String> GetHiddenFieldNames() const
        {
        bvector<Utf8String> names;
        JsonValueCR arr = m_overridesJson["hiddenFieldNames"];
        for (Json::ArrayIndex i = 0; i < arr.size(); ++i)
            names.push_back(arr[i].asCString());
        return names;
        }
public:
    DescriptorOverrideHelper(JsonValueCR json) : m_overridesJson(json) {}
    Utf8CP GetDisplayType() const { return m_overridesJson["displayType"].asCString(); }
    ContentDescriptorPtr GetOverridenDescriptor(ContentDescriptorCR defaultDescriptor) const
        {
        if (!defaultDescriptor.GetPreferredDisplayType().Equals(GetDisplayType()))
            {
            BeAssert(false);
            return ContentDescriptor::Create(defaultDescriptor);
            }
        ContentDescriptorPtr descriptorCopy = ContentDescriptor::Create(defaultDescriptor);
        descriptorCopy->SetSortingField(GetSortingFieldName());
        descriptorCopy->SetSortDirection(GetSortDirection());
        descriptorCopy->SetContentFlags(GetContentFlags());
        descriptorCopy->SetFilterExpression(GetFilterExpression());
        bvector<Utf8String> hiddenFieldNames = GetHiddenFieldNames();
        for (Utf8StringCR name : hiddenFieldNames)
            descriptorCopy->RemoveField(name.c_str());
        return descriptorCopy;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetDisplayType(JsonValueCR params)
    {
    return params.isMember("displayType") ? params["displayType"].asCString() : ContentDisplayType::Undefined;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetContentFlags(JsonValueCR params) { return params.isMember("contentFlags") ? params["contentFlags"].asInt() : 0; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static KeySetPtr GetKeys(IConnectionCR connection, JsonValueCR params)
    {
    if (!params.isMember("keys"))
        return KeySet::Create();
    return KeySet::FromJson(connection, params["keys"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentDescriptor(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetContentOptions(params).GetJson();
    return manager.GetContentDescriptor(db, GetDisplayType(params), GetContentFlags(params), *GetKeys(*connection, params), nullptr, options, context)
        .then([context](ContentDescriptorCPtr descriptor)
            {
            context.OnTaskStart();
            if (descriptor.IsValid())
                return ECPresentationResult(descriptor->AsJson());
            return ECPresentationResult();
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContent(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    JsonValueCR descriptorOverridesJson = params["descriptorOverrides"];
    PageOptions pageOptions = GetPageOptions(params);
    Json::Value options = GetContentOptions(params).GetJson();
    return manager.GetContentDescriptor(db, GetDisplayType(descriptorOverridesJson), GetContentFlags(descriptorOverridesJson), *GetKeys(*connection, params), nullptr, options, context)
        .then([&manager, descriptorOverridesJson, pageOptions, context](ContentDescriptorCPtr descriptor)
            {
            context.OnTaskStart();
            if (descriptor.IsNull())
                return folly::makeFutureWith([]() { return ECPresentationResult(); });
            ContentDescriptorCPtr overridenDescriptor = DescriptorOverrideHelper(descriptorOverridesJson).GetOverridenDescriptor(*descriptor);
            return manager.GetContent(*overridenDescriptor, pageOptions, context).then([context](ContentCPtr content)
                {
                context.OnTaskStart();
                return ECPresentationResult(content->AsJson());
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentSetSize(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    JsonValueCR descriptorOverridesJson = params["descriptorOverrides"];
    Json::Value options = GetContentOptions(params).GetJson();
    return manager.GetContentDescriptor(db, GetDisplayType(descriptorOverridesJson), GetContentFlags(descriptorOverridesJson), *GetKeys(*connection, params), nullptr, options, context)
        .then([&manager, descriptorOverridesJson, context](ContentDescriptorCPtr descriptor) {
            context.OnTaskStart();
            if (descriptor.IsNull())
                return folly::makeFutureWith([]() { return ECPresentationResult(rapidjson::Value(0)); });
            ContentDescriptorCPtr overridenDescriptor = DescriptorOverrideHelper(descriptorOverridesJson).GetOverridenDescriptor(*descriptor);
            return manager.GetContentSetSize(*overridenDescriptor, context).then([context](size_t size)
                {
                context.OnTaskStart();
                return ECPresentationResult(rapidjson::Value((int64_t)size));
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 06/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetDistinctValues(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    if (!params.isMember("fieldName"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "fieldName");
    if (!params.isMember("maximumValueCount"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "maximumValueCount");

    IConnectionCP connection = manager.GetConnections().GetConnection(db);
    JsonValueCR descriptorOverridesJson = params["descriptorOverrides"];
    Json::Value options = GetContentOptions(params).GetJson();
    Utf8String fieldName = params["fieldName"].asString();
    int64_t maximumValueCount = params["maximumValueCount"].asInt64();
    return manager.GetContentDescriptor(db, GetDisplayType(descriptorOverridesJson), GetContentFlags(descriptorOverridesJson), *GetKeys(*connection, params), nullptr, options, context)
        .then([&manager, descriptorOverridesJson, fieldName, maximumValueCount, context] (ContentDescriptorCPtr descriptor)
            {
            if (descriptor.IsNull())
                return folly::makeFutureWith([] () { return ECPresentationResult(ECPresentationStatus::InvalidArgument, "descriptor"); });

            ContentDescriptorPtr overridenDescriptor = DescriptorOverrideHelper(descriptorOverridesJson).GetOverridenDescriptor(*descriptor);
            bvector<ContentDescriptor::Field*> fieldsCopy = descriptor->GetAllFields();
            for (ContentDescriptor::Field const* field : fieldsCopy)
                {
                if (!field->GetUniqueName().Equals(fieldName))
                    overridenDescriptor->RemoveField(field->GetUniqueName().c_str());
                }
            overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

            return manager.GetContent(*overridenDescriptor, PageOptions(), context)
                .then([fieldName, maximumValueCount, context](ContentCPtr content)
                {
                context.OnTaskStart();
                DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();

                rapidjson::Document response;
                response.SetArray();
                for (size_t i = 0; i < contentSet.GetSize(); i++)
                    {
                    if (maximumValueCount != 0 && response.Size() >= maximumValueCount)
                        break;

                    RapidJsonValueCR value = contentSet.Get(i)->GetDisplayValues()[fieldName.c_str()];
                    if (value.IsNull() || (value.IsString() && 0 == value.GetStringLength()))
                        continue;

                    response.PushBack(rapidjson::Value(value.GetString(), response.GetAllocator()), response.GetAllocator());
                    }
                return ECPresentationResult(std::move(response));
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetDisplayLabel(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (!params.isMember("key"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "key");

    ECClassCP ecClass = IModelJsECPresentationSerializer::GetClassFromFullName(*connection, params["key"]["className"].asCString());
    if (ecClass == nullptr)
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "key.className");

    ECInstanceId id;
    ECInstanceId::FromString(id, params["key"]["id"].asCString());
    if (!id.IsValid())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "key.id");

    return manager.GetDisplayLabel(db, ECInstanceKey(ecClass->GetId(), id), Json::Value(), context)
        .then([context](LabelDefinitionCPtr labelDefinition)
        {
        context.OnTaskStart();
        return ECPresentationResult(labelDefinition->AsJson());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::CompareHierarchies(RulesDrivenECPresentationManager& manager, std::shared_ptr<IUpdateRecordsHandler> updateRecordsHandler, ECDbR db,
    Utf8StringCR prevRulesetId, Utf8StringCR currRulesetId, Utf8StringCR locale, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    RulesDrivenECPresentationManager::CommonOptions options;
    options.SetLocale(locale.c_str());
    return manager.CompareHierarchies(updateRecordsHandler, db, prevRulesetId, currRulesetId, options.GetJson(), context)
        .then([context]()
        {
        context.OnTaskStart();
        return ECPresentationResult();
        });
    }
