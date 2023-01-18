/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeDirectoryIterator.h>
#include "ECPresentationUtils.h"
#include "DgnDbECInstanceChangeEventSource.h"
#include "ECPresentationSerializer.h"
#include "UpdateRecordsHandler.h"
#include "UiStateProvider.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::CreateResultFromException(folly::exception_wrapper const& e)
    {
    if (e.is_compatible_with<CancellationException>())
        return ECPresentationResult(ECPresentationStatus::Canceled, "");

    if (e.is_compatible_with<InvalidArgumentException>())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8String(e.what().c_str()));

    return ECPresentationResult(ECPresentationStatus::Error, Utf8String(e.what().c_str()));
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationStaticSetupHelper
{
private:
    IModelJsECPresentationSerializer* m_serializer;
public:
    IModelJsECPresentationStaticSetupHelper()
        : m_serializer(new IModelJsECPresentationSerializer())
        {
        ECPresentationManager::SetSerializer(m_serializer);
        }
    ~IModelJsECPresentationStaticSetupHelper()
        {
        ECPresentationManager::SetSerializer(nullptr);
        }
    IModelJsECPresentationSerializer& GetSerializer() {return *m_serializer;}
    };
static IModelJsECPresentationStaticSetupHelper s_staticSetup;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TValue>
struct ParseResult
{
private:
    std::shared_ptr<TValue> m_value;
    Utf8String m_error;
private:
    ParseResult() {}
public:
    static ParseResult<TValue> CreateSuccess(TValue value)
        {
        ParseResult result;
        result.m_value = std::make_unique<TValue>(std::forward<TValue>(value));
        return result;
        }
    static ParseResult<TValue> CreateError(Utf8String error)
        {
        ParseResult result;
        result.m_error = error;
        return result;
        }
    bool HasError() const {return !m_error.empty();}
    bool IsSuccess() const {return !HasError();}
    Utf8StringCR GetError() const {return m_error;}
    TValue& GetValue() {return *m_value;}
    TValue const& GetValue() const {return *m_value;}
};
template<typename TValue> static ParseResult<TValue> CreateParseResult(TValue value) {return ParseResult<TValue>::CreateSuccess(std::forward<TValue>(value));}
template<typename TValue> static ParseResult<TValue> CreateParseError(Utf8String error) {return ParseResult<TValue>::CreateError(error);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ParseInt(int& value, Napi::Value const& jsValue)
    {
    if (jsValue.IsNumber())
        {
        value = jsValue.ToNumber().Int32Value();
        return true;
        }
    if (jsValue.IsString())
        {
        value = atoi(jsValue.ToString().Utf8Value().c_str());
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2022
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyDiskCacheParams(ECPresentationManager::Params::CachingParams& cachingParams, Napi::Object const& diskCacheConfig)
    {
    if (diskCacheConfig.Has("directory") && diskCacheConfig.Get("directory").IsString())
        cachingParams.SetCacheDirectoryPath(BeFileName(diskCacheConfig.Get("directory").ToString().Utf8Value()));

    if (diskCacheConfig.Has("memoryCacheSize") && diskCacheConfig.Get("memoryCacheSize").IsNumber())
        cachingParams.SetDiskCacheMemoryCacheSize((uint64_t)diskCacheConfig.Get("memoryCacheSize").ToNumber().Int64Value());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPresentationManager::Params::CachingParams CreateCachingParams(Napi::Object const& cacheConfig)
    {
    ECPresentationManager::Params::CachingParams cachingParams;
    if (!cacheConfig.Has("mode"))
        return cachingParams;

    Utf8String cacheMode = cacheConfig.Get("mode").ToString().Utf8Value();
    if (cacheMode.Equals("memory"))
        {
        cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Memory);
        }
    else if (cacheMode.Equals("disk"))
        {
        cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Disk);
        ApplyDiskCacheParams(cachingParams, cacheConfig);
        }
    else if (cacheMode.Equals("hybrid"))
        {
        cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Hybrid);
        if (cacheConfig.Has("disk") && cacheConfig.Get("disk").IsObject())
            ApplyDiskCacheParams(cachingParams, cacheConfig.Get("disk").As<Napi::Object>());
        }

    return cachingParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<int, unsigned> CreateTaskAllocationSlotsMap(Napi::Object const& jsMap)
    {
    bmap<int, unsigned> slots;
    Napi::Array jsMemberNames = jsMap.GetPropertyNames();
    for (uint32_t i = 0; i < jsMemberNames.Length(); ++i)
        {
        Napi::Value const& jsMemberName = jsMemberNames[i];
        Napi::Value const& jsMemberValue = jsMap.Get(jsMemberName);
        int priority, slotsCount;
        if (ParseInt(priority, jsMemberName) && ParseInt(slotsCount, jsMemberValue))
            slots.Insert(priority, slotsCount);
        }
    if (slots.empty())
        slots.Insert(INT_MAX, 1);
    return slots;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<IConnectionManager> CreateConfiguredConnectionManager(Napi::Object const& props)
    {
    ConnectionManager::Props connectionManagerProps;

    if (props.Get("useMmap").IsBoolean() && true == props.Get("useMmap").As<Napi::Boolean>())
        connectionManagerProps.SetMmapFileSize(nullptr);
    else if (props.Get("useMmap").IsNumber())
        connectionManagerProps.SetMmapFileSize(props.Get("useMmap").As<Napi::Number>().Int64Value());

    if (props.Has("workerConnectionCacheSize") && props.Get("workerConnectionCacheSize").IsNumber())
        connectionManagerProps.SetMemoryCacheSize((uint64_t)props.Get("workerConnectionCacheSize").ToNumber().Int64Value());

    return std::make_unique<ConnectionManager>(connectionManagerProps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<ECPresentation::UnitSystem> ParseUnitSystemFromString(Utf8CP str)
    {
    if (0 == strcmp("metric", str))
        return CreateParseResult(ECPresentation::UnitSystem::Metric);

    if (0 == strcmp("british-imperial", str))
        return CreateParseResult(ECPresentation::UnitSystem::BritishImperial);

    if (0 == strcmp("us-customary", str))
        return CreateParseResult(ECPresentation::UnitSystem::UsCustomary);

    if (0 == strcmp("us-survey", str))
        return CreateParseResult(ECPresentation::UnitSystem::UsSurvey);

    return CreateParseError<ECPresentation::UnitSystem>(Utf8PrintfString("Unrecognized unit system value '%s'. "
        "Expected one of: 'metric', 'british-imperial', 'us-customary', 'us-survey'.", str));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<ECPresentation::UnitSystem> ParseUnitSystemFromJson(RapidJsonValueCR json)
    {
    if (!json.IsString())
        return CreateParseError<ECPresentation::UnitSystem>("Expected unit system to be a string");
    return ParseUnitSystemFromString(json.GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaCP GetUnitsSchema()
    {
	static ECSchemaReadContextPtr s_context = ECSchemaReadContext::CreateContext();
    SchemaKey key("Units", 1, 0, 0);
    static ECSchemaPtr s_unitsSchema = s_context->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
	return s_unitsSchema.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::map<std::pair<Utf8String, ECPresentation::UnitSystem>, std::shared_ptr<Formatting::Format>> CreateDefaultFormatsMap(Napi::Object const& jsMap)
    {
    std::map<std::pair<Utf8String, ECPresentation::UnitSystem>, std::shared_ptr<Formatting::Format>> defaultFormats;
    ECSchemaCP unitsSchema = GetUnitsSchema();
    if (unitsSchema == nullptr)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to find Units schema");
    Napi::Array jsMemberNames = jsMap.GetPropertyNames();
    for (uint32_t i = 0; i < jsMemberNames.Length(); ++i)
        {
        Napi::Value const& jsMemberName = jsMemberNames[i];
        Napi::Object const& jsMemberValue = jsMap.Get(jsMemberName).ToObject();
        if (jsMemberValue.Has("serializedFormat") && jsMemberValue.Has("unitSystems"))
            {
            Utf8String phenomenon = jsMemberName.ToString().Utf8Value();
            auto format = std::make_shared<Formatting::Format>();
            Utf8String serializedFormatJson = jsMemberValue.Get("serializedFormat").ToString().Utf8Value();
            if (!Formatting::Format::FromJson(*format, serializedFormatJson.c_str(), &unitsSchema->GetUnitsContext()))
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to read Format from JSON: '%s'", serializedFormatJson.c_str()));
            Napi::Array unitSystems = jsMemberValue.Get("unitSystems").As<Napi::Array>();
            for (uint32_t arrIndex = 0; arrIndex < unitSystems.Length(); ++arrIndex)
                {
                Napi::Value arrValue = unitSystems[arrIndex];
                if (arrValue.IsString())
                    {
                    auto unitSystem = ParseUnitSystemFromString(arrValue.ToString().Utf8Value().c_str());
                    if (unitSystem.IsSuccess())
                        defaultFormats.insert(std::make_pair(std::make_pair(phenomenon.ToUpper(), unitSystem.GetValue()), format));
                    }
                }
            }
        }
    return defaultFormats;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECPresentationResult::GetSerializedSuccessResponse() const
    {
    static Utf8String const s_empty;
    if (IsError())
        return s_empty;

    if (m_serializedSuccessResponse.empty())
        {
        if (m_isJsonCppResponse)
            {
            m_serializedSuccessResponse = m_jsoncppSuccessResponse.ToString();
            }
        else
            {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            m_successResponse.Accept(writer);
            m_serializedSuccessResponse = buffer.GetString();
            }
        }
    return m_serializedSuccessResponse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECPresentationUtils::GetLoggerNamespace() {return "ECPresentation.Node";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NativeLogging::CategoryLogger ECPresentationUtils::GetLogger() {return NativeLogging::CategoryLogger(GetLoggerNamespace());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationManager* ECPresentationUtils::CreatePresentationManager(Dgn::PlatformLib::Host::IKnownLocationsAdmin& locations, IJsonLocalState& localState,
    std::shared_ptr<IUpdateRecordsHandler> updateRecordsHandler, std::shared_ptr<IUiStateProvider> uiStateProvider, Napi::Object const& props)
    {
    BeFileName assetsDir = locations.GetDgnPlatformAssetsDirectory();
    BeFileName tempDir = locations.GetLocalTempDirectoryBaseName();
    tempDir.AppendToPath(L"ecpresentation");
    if (!tempDir.DoesPathExist())
        BeFileName::CreateNewDirectory(tempDir.c_str());

    Utf8String id = props.Get("id").As<Napi::String>().Utf8Value();
    if (!id.empty())
        {
        tempDir.AppendToPath(WString(id.c_str(), true).c_str());
        if (!tempDir.DoesPathExist())
            BeFileName::CreateNewDirectory(tempDir.c_str());
        }
    ECPresentationManager::Paths pathParams(assetsDir, tempDir);

    ECPresentationManager::Params::ContentCachingParams contentCacheParams;
    if (props.Get("contentCacheSize").IsNumber())
        contentCacheParams.SetPrivateCacheSize((size_t)props.Get("contentCacheSize").As<Napi::Number>().Int64Value());

    ECPresentationManager::Params::MultiThreadingParams threadingParams(CreateTaskAllocationSlotsMap(props.Get("taskAllocationsMap").As<Napi::Object>()));

    ECPresentationManager::Params params(pathParams);
    params.SetConnections(CreateConfiguredConnectionManager(props));
    params.SetContentCachingParams(contentCacheParams);
    params.SetMultiThreadingParams(threadingParams);
    params.SetECPropertyFormatter(new DefaultPropertyFormatter(CreateDefaultFormatsMap(props.Get("defaultFormats").As<Napi::Object>())));
    params.SetCachingParams(CreateCachingParams(props.Get("cacheConfig").As<Napi::Object>()));
    params.SetLocalState(&localState);
    if (props.Get("isChangeTrackingEnabled").As<Napi::Boolean>())
        {
        params.SetECInstanceChangeEventSources({std::make_shared<DgnDbECInstanceChangeEventSource>()});
        params.SetUpdateRecordsHandlers({updateRecordsHandler});
        params.SetUiStateProvider(uiStateProvider);
        }
    return new ECPresentationManager(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetupRulesetDirectories(ECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str()));
    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetupSupplementalRulesetDirectories(ECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str())));
    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    return ECPresentationResult(std::move(json), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::AddRuleset(SimpleRuleSetLocater& locater, Utf8StringCR rulesetJsonString)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonString(rulesetJsonString);
    if (ruleset.IsNull())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Failed to create rule set from serialized JSON");
    locater.AddRuleSet(*ruleset);
    rapidjson::Document result;
    result.SetString(ruleset->GetHash().c_str(), result.GetAllocator());
    return ECPresentationResult(std::move(result), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::RemoveRuleset(SimpleRuleSetLocater& locater, Utf8StringCR rulesetId, Utf8StringCR hash)
    {
    bvector<PresentationRuleSetPtr> rulesets = locater.LocateRuleSets(rulesetId.c_str());
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        {
        if (ruleset->GetHash().Equals(hash))
            {
            locater.RemoveRuleSet(rulesetId);
            return ECPresentationResult(rapidjson::Value(true), true);
            }
        }
    return ECPresentationResult(rapidjson::Value(false), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::ClearRulesets(SimpleRuleSetLocater& locater)
    {
    locater.Clear();
    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), true);
    }

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
static BeJsConst ToBeJsConst(rapidjson::Value const& value)
    {
    // `BeJsConst` for `rapidjson::Value const&` requires an allocator, although it's completely
    // read-only and doesn't use it. The only reason it requires an allocator is that it uses
    // a read-write `BeJsValue` for all the operations.
    static rapidjson::MemoryPoolAllocator<> s_staticRapidJsonAllocator(8);
    return BeJsConst(value, s_staticRapidJsonAllocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NativeLogging::SEVERITY ParseSeverityFromJson(RapidJsonValueCR json)
    {
    if (json.IsBool())
        return json.GetBool() ? NativeLogging::LOG_INFO : NativeLogging::LOG_ERROR;

    if (json.IsString())
        {
        if (0 == strcmp("error", json.GetString()))
            return NativeLogging::LOG_ERROR;
        if (0 == strcmp("warning", json.GetString()))
            return NativeLogging::LOG_WARNING;
        if (0 == strcmp("info", json.GetString()))
            return NativeLogging::LOG_INFO;
        if (0 == strcmp("debug", json.GetString()))
            return NativeLogging::LOG_TRACE;
        if (0 == strcmp("trace", json.GetString()))
            return NativeLogging::LOG_TRACE;
        }

    return NativeLogging::LOG_ERROR;
    }

#define PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf_MinimumDuration  "minimumDuration"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Nullable<uint64_t> ParseDurationFromJson(RapidJsonValueCR json)
    {
    if (json.IsBool() && json.GetBool())
        return (uint64_t)0;
    if (json.IsObject() && json.HasMember(PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf_MinimumDuration) && json[PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf_MinimumDuration].IsNumber())
        return json[PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf_MinimumDuration].GetUint64();
    return nullptr;
    }

#define PRESENTATION_JSON_ATTRIBUTE_Diagnostics         "diagnostics"
#define PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf    "perf"
#define PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Dev     "dev"
#define PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Editor  "editor"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentation::Diagnostics::Options ECPresentationUtils::CreateDiagnosticsOptions(RapidJsonValueCR json)
    {
    ECPresentation::Diagnostics::Options options;
    if (json.IsObject() && json.HasMember(PRESENTATION_JSON_ATTRIBUTE_Diagnostics) && json[PRESENTATION_JSON_ATTRIBUTE_Diagnostics].IsObject())
        {
        RapidJsonValueCR diagnosticsJson = json[PRESENTATION_JSON_ATTRIBUTE_Diagnostics];
        if (diagnosticsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf))
            options.SetMinimumDuration(ParseDurationFromJson(diagnosticsJson[PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Perf]));
        if (diagnosticsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Dev))
            options.SetDevSeverity(ParseSeverityFromJson(diagnosticsJson[PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Dev]));
        if (diagnosticsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Editor))
            options.SetEditorSeverity(ParseSeverityFromJson(diagnosticsJson[PRESENTATION_JSON_ATTRIBUTE_Diagnostics_Editor]));
        }
    return options;
    }

#define PRESENTATION_JSON_ATTRIBUTE_Paging          "paging"
#define PRESENTATION_JSON_ATTRIBUTE_Paging_Start    "start"
#define PRESENTATION_JSON_ATTRIBUTE_Paging_Size     "size"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<PageOptions> ParsePageOptionsFromJson(RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_Paging))
        return CreateParseResult(PageOptions());

    if (!json[PRESENTATION_JSON_ATTRIBUTE_Paging].IsObject())
        return CreateParseError<PageOptions>("Expected `" PRESENTATION_JSON_ATTRIBUTE_Paging "` to be an object");

    PageOptions pageOptions;
    RapidJsonValueCR pagingParams = json[PRESENTATION_JSON_ATTRIBUTE_Paging];
    if (pagingParams.HasMember(PRESENTATION_JSON_ATTRIBUTE_Paging_Start))
        {
        if (!pagingParams[PRESENTATION_JSON_ATTRIBUTE_Paging_Start].IsInt())
            return CreateParseError<PageOptions>("Expected `" PRESENTATION_JSON_ATTRIBUTE_Paging "." PRESENTATION_JSON_ATTRIBUTE_Paging_Start "` to be an integer");
        pageOptions.SetPageStart((size_t)pagingParams[PRESENTATION_JSON_ATTRIBUTE_Paging_Start].GetUint64());
        }
    if (pagingParams.HasMember(PRESENTATION_JSON_ATTRIBUTE_Paging_Size))
        {
        if (!pagingParams[PRESENTATION_JSON_ATTRIBUTE_Paging_Size].IsInt())
            return CreateParseError<PageOptions>("Expected `" PRESENTATION_JSON_ATTRIBUTE_Paging "." PRESENTATION_JSON_ATTRIBUTE_Paging_Size "` to be an integer");
        pageOptions.SetPageSize((size_t)pagingParams[PRESENTATION_JSON_ATTRIBUTE_Paging_Size].GetUint64());
        }
    return CreateParseResult(pageOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::GetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType)
    {
    rapidjson::Document response;
    IUserSettings& settings = manager.GetUserSettings().GetSettings(rulesetId);

    if (variableType.Equals("bool"))
        response.SetBool(settings.GetSettingBoolValue(variableId.c_str()));
    else if (variableType.Equals("string"))
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
    return ECPresentationResult(std::move(response), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType, BeJsConst value)
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
        for (BeJsConst::ArrayIndex i = 0; i < value.size(); i++)
            values.push_back(BeInt64Id::FromString(value[i].asCString()).GetValue());
        settings.SetSettingIntValues(variableId.c_str(), values);
        }
    else if (variableType.Equals("int"))
        settings.SetSettingIntValue(variableId.c_str(), value.asInt64());
    else if (variableType.Equals("int[]"))
        {
        bvector<int64_t> values;
        for (BeJsConst::ArrayIndex i = 0; i < value.size(); i++)
            values.push_back(value[i].asInt64());
        settings.SetSettingIntValues(variableId.c_str(), values);
        }
    else
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "type");
    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::UnsetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId)
    {
    IUserSettings& settings = manager.GetUserSettings().GetSettings(rulesetId);
    settings.UnsetValue(variableId.c_str());
    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), false);
    }

#define PRESENTATION_JSON_ATTRIBUTE_BaseRequestParams_UnitSystem    "unitSystem"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<BaseRequestParams> ParseBaseRequestParamsFromJson(RapidJsonValueCR json)
    {
    BaseRequestParams params;
    if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_BaseRequestParams_UnitSystem))
        {
        auto unitSystem = ParseUnitSystemFromJson(json[PRESENTATION_JSON_ATTRIBUTE_BaseRequestParams_UnitSystem]);
        if (unitSystem.HasError())
            return CreateParseError<BaseRequestParams>(unitSystem.GetError());
        params.SetUnitSystem(unitSystem.GetValue());
        }
    return CreateParseResult(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<Utf8String> ParseRulesetIdFromJson(RapidJsonValueCR json, Utf8CP attributeName)
    {
    if (!json.HasMember(attributeName) || !json[attributeName].IsString())
        return CreateParseError<Utf8String>(Utf8PrintfString("Expected `%s` to be a string", attributeName));

    return CreateParseResult(Utf8String(json[attributeName].GetString()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<RulesetVariables> ParseRulesetVariablesFromJson(RapidJsonValueCR json, Utf8CP attributeName)
    {
    if (!json.HasMember(attributeName))
        return CreateParseResult(RulesetVariables());

    if (!json[attributeName].IsArray())
        return CreateParseError<RulesetVariables>(Utf8PrintfString("Expected `%s` to be an array", attributeName));

    return CreateParseResult(IModelJsECPresentationSerializer::GetRulesetVariablesFromJson(ToBeJsConst(json[attributeName])));
    }

#define PRESENTATION_JSON_ATTRIBUTE_RulesetParams_RulesetId         "rulesetId"
#define PRESENTATION_JSON_ATTRIBUTE_RulesetParams_RulesetVariables  "rulesetVariables"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<RequestWithRulesetParams> ParseRulesetParamsFromJson(RapidJsonValueCR json)
    {
    auto baseParams = ParseBaseRequestParamsFromJson(json);
    if (baseParams.HasError())
        return CreateParseError<RequestWithRulesetParams>(baseParams.GetError());

    auto rulesetId = ParseRulesetIdFromJson(json, PRESENTATION_JSON_ATTRIBUTE_RulesetParams_RulesetId);
    if (rulesetId.HasError())
        return CreateParseError<RequestWithRulesetParams>(rulesetId.GetError());

    auto rulesetVariables = ParseRulesetVariablesFromJson(json, PRESENTATION_JSON_ATTRIBUTE_RulesetParams_RulesetVariables);
    if (rulesetVariables.HasError())
        return CreateParseError<RequestWithRulesetParams>(rulesetVariables.GetError());

    return CreateParseResult(RequestWithRulesetParams(baseParams.GetValue(), rulesetId.GetValue(), rulesetVariables.GetValue()));
    }

#define PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_ClassName    "className"
#define PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_Id           "id"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<ECClassInstanceKey> ParseECInstanceKeyFromJson(IConnectionCR connection, RapidJsonValueCR json, Utf8CP attributeName)
    {
    if (!json.IsObject())
        return CreateParseError<ECClassInstanceKey>(Utf8PrintfString("Expected `%s` to be an object", attributeName));

    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_ClassName) || !json[PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_ClassName].IsString())
        return CreateParseError<ECClassInstanceKey>(Utf8PrintfString("Expected `%s." PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_ClassName "` to be a string", attributeName));

    ECClassCP ecClass = IModelJsECPresentationSerializer::GetClassFromFullName(connection, ToBeJsConst(json[PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_ClassName]));
    if (!ecClass)
        return CreateParseError<ECClassInstanceKey>(Utf8PrintfString("`%s." PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_ClassName "` specifies an invalid class", attributeName));

    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_Id) || !json[PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_Id].IsString())
        return CreateParseError<ECClassInstanceKey>(Utf8PrintfString("Expected `%s." PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_Id "` to be a string", attributeName));

    BeInt64Id id;
    if (SUCCESS != BeInt64Id::FromString(id, json[PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_Id].GetString()))
        return CreateParseError<ECClassInstanceKey>(Utf8PrintfString("Expected `%s." PRESENTATION_JSON_ATTRIBUTE_ECInstanceKey_Id "` to specify a valid ID", attributeName));

    return CreateParseResult(ECClassInstanceKey(*ecClass, ECInstanceId(id)));
    }

#define PRESENTATION_JSON_ATTRIBUTE_AsyncParams_Priority "priority"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBaseParams>
static WithAsyncTaskParams<TBaseParams> CreateAsyncParams(TBaseParams baseParams, ECDbCR db)
    {
    return WithAsyncTaskParams<TBaseParams>::Create(db, baseParams);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBaseParams>
static WithAsyncTaskParams<TBaseParams> CreateAsyncParams(TBaseParams baseParams, ECDbCR db, RapidJsonValueCR json)
    {
    auto params = CreateAsyncParams(baseParams, db);
    if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_AsyncParams_Priority) && json[PRESENTATION_JSON_ATTRIBUTE_AsyncParams_Priority].IsInt())
        params.SetRequestPriority(json[PRESENTATION_JSON_ATTRIBUTE_AsyncParams_Priority].GetInt());
    else
        params.SetRequestPriority(DEFAULT_REQUEST_PRIORITY);
    return params;
    }

#define PRESENTATION_JSON_ATTRIBUTE_HierarchyParams_InstanceFilter "instanceFilter"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<std::unique_ptr<InstanceFilterDefinition>> ParseHierarchyLevelInstanceFilterFromJson(IConnectionCR connection, RapidJsonValueCR json)
    {
    if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyParams_InstanceFilter))
        return CreateParseResult<std::unique_ptr<InstanceFilterDefinition>>(ECPresentationManager::GetSerializer().GetInstanceFilterFromJson(connection, ToBeJsConst(json[PRESENTATION_JSON_ATTRIBUTE_HierarchyParams_InstanceFilter])));
    return CreateParseResult<std::unique_ptr<InstanceFilterDefinition>>(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetRootNodesCount(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto instanceFilterParam = ParseHierarchyLevelInstanceFilterFromJson(*connection, paramsJson);
    if (instanceFilterParam.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, instanceFilterParam.GetError());

    auto params = HierarchyRequestParams(rulesetParams.GetValue());
    params.SetInstanceFilter(std::move(instanceFilterParam.GetValue()));

    return manager.GetNodesCount(CreateAsyncParams(params, db, paramsJson)).then([](NodesCountResponse response)
        {
        return ECPresentationResult(rapidjson::Value((int64_t)*response), true);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetRootNodes(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto instanceFilterParam = ParseHierarchyLevelInstanceFilterFromJson(*connection, paramsJson);
    if (instanceFilterParam.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, instanceFilterParam.GetError());

    auto pageParams = ParsePageOptionsFromJson(paramsJson);
    if (pageParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, pageParams.GetError());

    auto params = HierarchyRequestParams(rulesetParams.GetValue());
    params.SetInstanceFilter(std::move(instanceFilterParam.GetValue()));

    return manager.GetNodes(ECPresentation::MakePaged(CreateAsyncParams(params, db, paramsJson), pageParams.GetValue()))
        .then([](NodesResponse response)
            {
            rapidjson::Document json;
            json.SetArray();
            for (NavNodeCPtr const& node : *response)
                PUSH_JSON_IF_VALID(json, json.GetAllocator(), node);
            return ECPresentationResult(std::move(json), true);
        });
    }

#define PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey "nodeKey"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<NavNodeKeyCPtr> ParseParentNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json, bool isRequired)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey))
        {
        if (isRequired)
            return CreateParseError<NavNodeKeyCPtr>("Missing required `" PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey "` attribute");
        return CreateParseResult<NavNodeKeyCPtr>(nullptr);
        }

    if (!json[PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey].IsObject())
        return CreateParseError<NavNodeKeyCPtr>("Expected `" PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey "` to be an object");

    NavNodeKeyCPtr key = NavNodeKey::FromJson(connection, ToBeJsConst(json[PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey]));
    if (key.IsNull())
        return CreateParseError<NavNodeKeyCPtr>("`" PRESENTATION_JSON_ATTRIBUTE_GetNode_NodeKey "` contains invalid value");

    return CreateParseResult(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetChildrenCount(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto parentKeyParams = ParseParentNodeKeyFromJson(*connection, paramsJson, true);
    if (parentKeyParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, parentKeyParams.GetError());

    auto instanceFilterParam = ParseHierarchyLevelInstanceFilterFromJson(*connection, paramsJson);
    if (instanceFilterParam.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, instanceFilterParam.GetError());

    auto params = HierarchyRequestParams(rulesetParams.GetValue(), parentKeyParams.GetValue().get());
    params.SetInstanceFilter(std::move(instanceFilterParam.GetValue()));

    return manager.GetNodesCount(CreateAsyncParams(params, connection->GetECDb(), paramsJson))
        .then([](NodesCountResponse nodesCountResponse)
            {
            return ECPresentationResult(rapidjson::Value((int64_t)*nodesCountResponse), true);
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetChildren(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto pageParams = ParsePageOptionsFromJson(paramsJson);
    if (pageParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, pageParams.GetError());

    auto parentKeyParams = ParseParentNodeKeyFromJson(*connection, paramsJson, true);
    if (parentKeyParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, parentKeyParams.GetError());

    auto instanceFilterParam = ParseHierarchyLevelInstanceFilterFromJson(*connection, paramsJson);
    if (instanceFilterParam.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, instanceFilterParam.GetError());

    auto params = HierarchyRequestParams(rulesetParams.GetValue(), parentKeyParams.GetValue().get());
    params.SetInstanceFilter(std::move(instanceFilterParam.GetValue()));

    return manager.GetNodes(ECPresentation::MakePaged(CreateAsyncParams(params, db, paramsJson), pageParams.GetValue()))
        .then([](NodesResponse nodesResponse)
            {
            rapidjson::Document json;
            json.SetArray();
            for (NavNodeCPtr const& node : *nodesResponse)
                PUSH_JSON_IF_VALID(json, json.GetAllocator(), node);
            return ECPresentationResult(std::move(json), true);
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetHierarchyLevelDescriptor(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto parentKeyParams = ParseParentNodeKeyFromJson(*connection, paramsJson, false);
    if (parentKeyParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, parentKeyParams.GetError());

    auto params = HierarchyLevelDescriptorRequestParams(rulesetParams.GetValue(), parentKeyParams.GetValue().get());
    return manager.GetNodesDescriptor(CreateAsyncParams(params, db, paramsJson))
        .then([formatter = &manager.GetFormatter()](NodesDescriptorResponse descriptorResponse)
            {
            auto const& descriptor = *descriptorResponse;
            if (descriptor.IsNull())
                return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), true);

            ECPresentationSerializerContext serializerCtx(descriptor->GetUnitSystem(), formatter);
            return ECPresentationResult(descriptor->AsJson(serializerCtx), true);
            });
    }

#define PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_MarkedIndex   "markedIndex"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<Nullable<size_t>> ParseMarkedIndexFromJson(RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_MarkedIndex))
        return CreateParseResult<Nullable<size_t>>(nullptr);

    if (!json[PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_MarkedIndex].IsInt())
        return CreateParseError<Nullable<size_t>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_MarkedIndex "` to be an integer");

    return CreateParseResult<Nullable<size_t>>((size_t)json[PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_MarkedIndex].GetInt());
    }

#define PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths  "paths"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<bvector<bvector<ECInstanceKey>>> ParseECInstanceKeyPathsFromJson(IConnectionCR connection, RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths) || !json[PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths].IsArray())
        return CreateParseError<bvector<bvector<ECInstanceKey>>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths "` to be an array");

    bvector<bvector<ECInstanceKey>> keyPaths;
    RapidJsonValueCR keyPathsJson = json[PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths];
    for (rapidjson::SizeType i = 0; i < keyPathsJson.Size(); ++i)
        {
        RapidJsonValueCR keysJson = keyPathsJson[i];
        if (!keysJson.IsArray())
            return CreateParseError<bvector<bvector<ECInstanceKey>>>(Utf8PrintfString("Expected `" PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths "[%" PRIu64 "]` to be an array", (uint64_t)i));

        keyPaths.push_back(bvector<ECInstanceKey>());
        for (rapidjson::SizeType j = 0; j < keysJson.Size(); ++j)
            {
            auto key = ParseECInstanceKeyFromJson(connection, keysJson[j], Utf8PrintfString(PRESENTATION_JSON_ATTRIBUTE_GetNodesPaths_KeyPaths "[%" PRIu64 "][%" PRIu64 "]", (uint64_t)i, (uint64_t)j).c_str());
            if (key.HasError())
                return CreateParseError<bvector<bvector<ECInstanceKey>>>(key.GetError());

            keyPaths.back().push_back(ECInstanceKey(key.GetValue().GetClass()->GetId(), key.GetValue().GetId()));
            }
        }
    return CreateParseResult(keyPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetNodesPaths(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto markedIndex = ParseMarkedIndexFromJson(paramsJson);
    if (markedIndex.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, markedIndex.GetError());

    auto keyPaths = ParseECInstanceKeyPathsFromJson(*connection, paramsJson);
    if (keyPaths.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, keyPaths.GetError());

    NodePathsFromInstanceKeyPathsRequestParams params(rulesetParams.GetValue(), keyPaths.GetValue(), markedIndex.GetValue());
    return manager.GetNodePaths(CreateAsyncParams(params, db, paramsJson))
        .then([](NodePathsResponse response)
            {
            auto const& paths = *response;
            rapidjson::Document json;
            json.SetArray();
            for (size_t i = 0; i < paths.size(); i++)
                json.PushBack(paths[i].AsJson(&json.GetAllocator()), json.GetAllocator());
            return ECPresentationResult(std::move(json), true);
            });
    }

#define PRESENTATION_JSON_ATTRIBUTE_GetFilteredNodesPaths_FilterText "filterText"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<Utf8String> ParseFilterTextFromJson(RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_GetFilteredNodesPaths_FilterText) || !json[PRESENTATION_JSON_ATTRIBUTE_GetFilteredNodesPaths_FilterText].IsString())
        return CreateParseError<Utf8String>("Expected `" PRESENTATION_JSON_ATTRIBUTE_GetFilteredNodesPaths_FilterText "` to be an string");

    return CreateParseResult(Utf8String(json[PRESENTATION_JSON_ATTRIBUTE_GetFilteredNodesPaths_FilterText].GetString()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetFilteredNodesPaths(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto filterText = ParseFilterTextFromJson(paramsJson);
    if (filterText.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, filterText.GetError());

    NodePathsFromFilterTextRequestParams params(rulesetParams.GetValue(), filterText.GetValue());
    return manager.GetNodePaths(CreateAsyncParams(params, db, paramsJson))
        .then([](NodePathsResponse response)
            {
            auto const& paths = *response;
            rapidjson::Document json;
            json.SetArray();
            for (size_t i = 0; i < paths.size(); i++)
                json.PushBack(paths[i].AsJson(&json.GetAllocator()), json.GetAllocator());
            return ECPresentationResult(std::move(json), true);
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<IContentFieldMatcher> CreatePropertyFieldMatcher(RapidJsonValueCR classNameJson, RapidJsonValueCR propertyNameJson, RelatedClassPathCR pathFromSelectToPropertyClass, ECDbCR db)
    {
    Utf8CP className = classNameJson.IsString() ? classNameJson.GetString() : "";
    Utf8CP propertyName = propertyNameJson.IsString() ? propertyNameJson.GetString() : "";
    if (*className && *propertyName)
        {
        ECClassCP propertyClass = IModelJsECPresentationSerializer::GetClassFromFullName(db, ToBeJsConst(rapidjson::Value(rapidjson::StringRef(className))));
        ECPropertyCP prop = propertyClass ? propertyClass->GetPropertyP(propertyName) : nullptr;
        if (prop)
            return std::make_unique<PropertiesContentFieldMatcher>(*prop, pathFromSelectToPropertyClass);
        }
    return nullptr;
    }

#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Type                           "type"
#define PRESENTATION_JSON_ATTRIBUTE_VALUE_FieldMatcherType_Name                 "name"
#define PRESENTATION_JSON_ATTRIBUTE_VALUE_FieldMatcherType_Properties           "properties"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_FieldName                      "fieldName"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties                     "properties"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties_Class               "class"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties_Name                "name"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PathFromSelectToPropertyClass  "pathFromSelectToPropertyClass"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PropertyClass                  "propertyClass"
#define PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PropertyName                   "propertyName"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<std::unique_ptr<IContentFieldMatcher>> CreateFieldMatchersFromJson(RapidJsonValueCR json, ECDbCR db)
    {
    bvector<std::unique_ptr<IContentFieldMatcher>> matchers;
    Utf8CP type = (json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Type].IsString()) ? json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Type].GetString() : "";
    if (0 == strcmp(PRESENTATION_JSON_ATTRIBUTE_VALUE_FieldMatcherType_Name, type))
        {
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_FieldName) && json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_FieldName].IsString())
            matchers.push_back(std::make_unique<NamedContentFieldMatcher>(json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_FieldName].GetString()));
        }
    else if (0 == strcmp(PRESENTATION_JSON_ATTRIBUTE_VALUE_FieldMatcherType_Properties, type))
        {
        RelatedClassPath pathFromSelectToPropertyClass;
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PathFromSelectToPropertyClass))
            pathFromSelectToPropertyClass = IModelJsECPresentationSerializer::GetRelatedClassPathFromJson(db, ToBeJsConst(json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PathFromSelectToPropertyClass]));

        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties) && json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties].IsArray())
            {
            for (rapidjson::SizeType i = 0; i < json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties].Size(); ++i)
                {
                RapidJsonValueCR propertyDefJson = json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties][i];
                if (!propertyDefJson.IsObject() || !propertyDefJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties_Class) || !propertyDefJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties_Name))
                    continue;

                auto matcher = CreatePropertyFieldMatcher(propertyDefJson[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties_Class], propertyDefJson[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_Properties_Name], pathFromSelectToPropertyClass, db);
                if (matcher)
                    matchers.push_back(std::move(matcher));
                }
            }
        else if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PropertyClass) && json.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PropertyName))
            {
            auto matcher = CreatePropertyFieldMatcher(json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PropertyClass], json[PRESENTATION_JSON_ATTRIBUTE_FieldMatcher_PropertyName], pathFromSelectToPropertyClass, db);
            if (matcher)
                matchers.push_back(std::move(matcher));
            }
        }
    return matchers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<IContentFieldMatcher> CreateFieldMatcherFromJson(RapidJsonValueCR json, ECDbCR db)
    {
    auto matchers = CreateFieldMatchersFromJson(json, db);
    if (matchers.empty())
        return nullptr;
    if (matchers.size() == 1)
        return std::move(matchers.front());
    return std::make_unique<CombinedContentFieldMatcher>(std::move(matchers));
    }

#define PRESENTATION_JSON_ATTRIBUTE_Classes "classes"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentSources(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    if (!paramsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_Classes) || !paramsJson[PRESENTATION_JSON_ATTRIBUTE_Classes].IsArray())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, PRESENTATION_JSON_ATTRIBUTE_Classes);

    bvector<ECClassCP> requestedClasses;
    RapidJsonValueCR requestedClassesJson = paramsJson[PRESENTATION_JSON_ATTRIBUTE_Classes];
    for (rapidjson::SizeType i = 0; i < requestedClassesJson.Size(); ++i)
        {
        ECClassCP ecClass = IModelJsECPresentationSerializer::GetClassFromFullName(*connection, ToBeJsConst(requestedClassesJson[i]));
        if (ecClass == nullptr)
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString(PRESENTATION_JSON_ATTRIBUTE_Classes "[%" PRIu64 "].className", (uint64_t)i));

        requestedClasses.push_back(ecClass);
        }

    ContentClassesRequestParams params(ContentMetadataRequestParams(rulesetParams.GetValue(), "", 0), requestedClasses);
    return manager.GetContentClasses(CreateAsyncParams(params, db, paramsJson))
        .then([](ContentClassesResponse response)
            {
            CompressedClassSerializer ecClassSerializer;
            ECPresentationSerializerContext serializerCtx;
            serializerCtx.SetClassSerializer(&ecClassSerializer);

            rapidjson::Document json;
            json.SetObject();

            rapidjson::Value selectClassesJson;
            selectClassesJson.SetArray();
            for (auto const& classInfo : *response)
                selectClassesJson.PushBack(ECPresentationManager::GetSerializer().AsJson(serializerCtx, classInfo, &json.GetAllocator()), json.GetAllocator());
            json.AddMember("sources", selectClassesJson, json.GetAllocator());
            json.AddMember("classesMap", ecClassSerializer.CreateAccumulatedClassesMap(&json.GetAllocator()), json.GetAllocator());

            return ECPresentationResult(std::move(json), true);
            });
    }

#define PRESENTATION_JSON_ATTRIBUTE_DisplayType "displayType"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<Nullable<Utf8String>> ParseDisplayTypeFromJson(RapidJsonValueCR paramsJson)
    {
    if (!paramsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_DisplayType))
        return CreateParseResult<Nullable<Utf8String>>(nullptr);

    if (!paramsJson[PRESENTATION_JSON_ATTRIBUTE_DisplayType].IsString())
        return CreateParseError<Nullable<Utf8String>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DisplayType "` to be a string");

    return CreateParseResult<Nullable<Utf8String>>(Utf8String(paramsJson[PRESENTATION_JSON_ATTRIBUTE_DisplayType].GetString()));
    }

#define PRESENTATION_JSON_ATTRIBUTE_ContentFlags "contentFlags"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<Nullable<int>> ParseContentFlagsFromJson(RapidJsonValueCR paramsJson)
    {
    if (!paramsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_ContentFlags))
        return CreateParseResult<Nullable<int>>(nullptr);

    if (!paramsJson[PRESENTATION_JSON_ATTRIBUTE_ContentFlags].IsInt())
        return CreateParseError<Nullable<int>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_ContentFlags "` to be an integer");

    return CreateParseResult<Nullable<int>>(paramsJson[PRESENTATION_JSON_ATTRIBUTE_ContentFlags].GetInt());
    }

#define PRESENTATION_JSON_ATTRIBUTE_Keys "keys"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<KeySetPtr> ParseKeysFromJson(IConnectionCR connection, RapidJsonValueCR paramsJson)
    {
    auto keys = paramsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_Keys)
        ? IModelJsECPresentationSerializer::GetKeySetFromJson(connection, ToBeJsConst(paramsJson[PRESENTATION_JSON_ATTRIBUTE_Keys]))
        : KeySet::Create();
    return CreateParseResult(keys);
    }

#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverrides                                 "descriptorOverrides"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFilterExpression                 "filterExpression"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsFilterExpression           "fieldsFilterExpression"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting                          "sorting"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingField                     "field"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingDirection                 "direction"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector                   "fieldsSelector"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorType               "type"
#define PRESENTATION_JSON_ATTRIBUTE_VALUE_DescriptorOverridesFieldsSelectorTypeInclude  "include"
#define PRESENTATION_JSON_ATTRIBUTE_VALUE_DescriptorOverridesFieldsSelectorTypeExclude  "exclude"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorFields             "fields"
#define PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesInstanceFilter                   "instanceFilter"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DescriptorOverrides
{
private:
    enum class FieldSelectorType
        {
        None,
        Include,
        Exclude,
        };

    typedef bpair<std::shared_ptr<IContentFieldMatcher>, SortDirection> TSortingParams;
    typedef bpair<FieldSelectorType, bvector<std::shared_ptr<IContentFieldMatcher>>> TFieldsSelector;

private:
    Nullable<int> m_contentFlags;
    Nullable<Utf8String> m_displayType;
    Nullable<Utf8String> m_fieldsFilterExpression;
    Nullable<TSortingParams> m_sortParams;
    std::shared_ptr<InstanceFilterDefinition> m_instanceFilter;
    TFieldsSelector m_fieldsSelector;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DescriptorOverrides(Nullable<Utf8String> displayType, Nullable<int> contentFlags, Nullable<Utf8String> fieldsDilterExpression, Nullable<TSortingParams> sortParams,
        TFieldsSelector fieldsSelector, std::shared_ptr<InstanceFilterDefinition> instanceFilter)
        : m_displayType(displayType), m_contentFlags(contentFlags), m_fieldsFilterExpression(fieldsDilterExpression), m_sortParams(sortParams),
        m_fieldsSelector(fieldsSelector), m_instanceFilter(instanceFilter)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ParseResult<Nullable<Utf8String>> ParseFieldsFilterExpression(RapidJsonValueCR json)
        {
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsFilterExpression))
            {
            if (!json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsFilterExpression].IsString())
                return CreateParseError<Nullable<Utf8String>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsFilterExpression "` to be a string");

            return CreateParseResult<Nullable<Utf8String>>(Utf8String(json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsFilterExpression].GetString()));
            }

        // deprecated property
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFilterExpression))
            {
            if (!json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFilterExpression].IsString())
                return CreateParseError<Nullable<Utf8String>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFilterExpression "` to be a string");

            return CreateParseResult<Nullable<Utf8String>>(Utf8String(json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFilterExpression].GetString()));
            }

        return CreateParseResult<Nullable<Utf8String>>(nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ParseResult<std::unique_ptr<InstanceFilterDefinition>> ParseInstanceFilter(IConnectionCR connection, RapidJsonValueCR json)
        {
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesInstanceFilter))
            return CreateParseResult(ECPresentationManager::GetSerializer().GetInstanceFilterFromJson(connection, ToBeJsConst(json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesInstanceFilter])));
        return CreateParseResult<std::unique_ptr<InstanceFilterDefinition>>(nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ParseResult<Nullable<TSortingParams>> ParseSortingParams(ECDbCR db, RapidJsonValueCR json)
        {
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting))
            {
            if (!json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting].IsObject())
                return CreateParseError<Nullable<TSortingParams>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting "` to be an object");

            if (!json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting].HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingField))
                return CreateParseError<Nullable<TSortingParams>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting "` to have member `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingField "`");

            std::shared_ptr<IContentFieldMatcher> fieldMatcher = CreateFieldMatcherFromJson(json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting][PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingField], db);
            if (!fieldMatcher)
                return CreateParseError<Nullable<TSortingParams>>("`" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting "." PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingField "` contains invalid field descriptor");

            SortDirection direction = SortDirection::Ascending;
            if (json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting].HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingDirection))
                {
                if (!json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting][PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingDirection].IsInt())
                    return CreateParseError<Nullable<TSortingParams>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting "." PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingDirection "` to be an integer");

                auto value = json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSorting][PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesSortingDirection].GetInt();
                direction = value == 0 ? SortDirection::Ascending : SortDirection::Descending;
                }

            return CreateParseResult<Nullable<TSortingParams>>(make_bpair(fieldMatcher, direction));
            }

        // support deprecated case:
        if (json.HasMember("sortingFieldName") && json["sortingFieldName"].IsString())
            {
            return CreateParseResult<Nullable<TSortingParams>>(TSortingParams(make_bpair(
                std::make_shared<NamedContentFieldMatcher>(json["sortingFieldName"].GetString()),
                (!json.HasMember("sortDirection") || !json["sortDirection"].IsInt() || json["sortDirection"].GetInt() == 0) ? SortDirection::Ascending : SortDirection::Descending)));
            }

        return CreateParseResult<Nullable<TSortingParams>>(nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ParseResult<TFieldsSelector> ParseFieldsSelector(ECDbCR db, RapidJsonValueCR json)
        {
        if (json.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector))
            {
            RapidJsonValueCR selectorJson = json[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector];
            if (!selectorJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorType) || !selectorJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorType].IsString())
                return CreateParseError<TFieldsSelector>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector "." PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorType "` to be a string");

            Utf8CP typeStr = selectorJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorType].GetString();
            bool isInclude = (0 == strcmp(PRESENTATION_JSON_ATTRIBUTE_VALUE_DescriptorOverridesFieldsSelectorTypeInclude, typeStr));
            bool isExclude = (0 == strcmp(PRESENTATION_JSON_ATTRIBUTE_VALUE_DescriptorOverridesFieldsSelectorTypeExclude, typeStr));
            if (!isInclude && !isExclude)
                {
                return CreateParseError<TFieldsSelector>(Utf8PrintfString("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector "." PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorType "` to be one of: "
                    "'" PRESENTATION_JSON_ATTRIBUTE_VALUE_DescriptorOverridesFieldsSelectorTypeInclude "', '" PRESENTATION_JSON_ATTRIBUTE_VALUE_DescriptorOverridesFieldsSelectorTypeExclude "'. Got: '%s'", typeStr));
                }
            FieldSelectorType type = isInclude ? FieldSelectorType::Include : FieldSelectorType::Exclude;

            if (!selectorJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorFields) || !selectorJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorFields].IsArray())
                return CreateParseError<TFieldsSelector>("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector "." PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorFields "` to be an array");

            bvector<std::shared_ptr<IContentFieldMatcher>> fieldMatchers;
            RapidJsonValueCR fieldsJson = selectorJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorFields];
            for (rapidjson::SizeType i = 0; i < fieldsJson.Size(); ++i)
                {
                auto fieldMatcher = CreateFieldMatcherFromJson(fieldsJson[i], db);
                if (!fieldMatcher)
                    {
                    return CreateParseError<TFieldsSelector>(Utf8PrintfString("Expected `" PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelector "." PRESENTATION_JSON_ATTRIBUTE_DescriptorOverridesFieldsSelectorFields "[" PRIu64 "]` "
                        "contains invalid field descriptor", (uint64_t)i));
                    }
                fieldMatchers.push_back(std::move(fieldMatcher));
                }
            return CreateParseResult(make_bpair(type, std::move(fieldMatchers)));
            }

        if (json.HasMember("hiddenFieldNames") && json["hiddenFieldNames"].IsArray())
            {
            bvector<std::shared_ptr<IContentFieldMatcher>> fieldMatchers;
            RapidJsonValueCR namesJson = json["hiddenFieldNames"];
            for (rapidjson::SizeType i = 0; i < namesJson.Size(); ++i)
                {
                Utf8CP fieldName = namesJson[i].GetString();
                if (fieldName && *fieldName)
                    fieldMatchers.push_back(std::make_unique<NamedContentFieldMatcher>(fieldName));
                }
            return CreateParseResult(make_bpair(FieldSelectorType::Exclude, std::move(fieldMatchers)));
            }

        return CreateParseResult(make_bpair(FieldSelectorType::None, bvector<std::shared_ptr<IContentFieldMatcher>>()));
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ParseResult<DescriptorOverrides> ParseFromJson(IConnectionCR connection, RapidJsonValueCR json)
        {
        auto displayType = ParseDisplayTypeFromJson(json);
        if (displayType.HasError())
            return CreateParseError<DescriptorOverrides>(displayType.GetError());

        auto contentFlags = ParseContentFlagsFromJson(json);
        if (contentFlags.HasError())
            return CreateParseError<DescriptorOverrides>(contentFlags.GetError());

        auto fieldsFilterExpression = ParseFieldsFilterExpression(json);
        if (fieldsFilterExpression.HasError())
            return CreateParseError<DescriptorOverrides>(fieldsFilterExpression.GetError());

        auto sortingParams = ParseSortingParams(connection.GetECDb(), json);
        if (sortingParams.HasError())
            return CreateParseError<DescriptorOverrides>(sortingParams.GetError());

        auto fieldsSelector = ParseFieldsSelector(connection.GetECDb(), json);
        if (fieldsSelector.HasError())
            return CreateParseError<DescriptorOverrides>(fieldsSelector.GetError());

        auto instanceFilter = ParseInstanceFilter(connection, json);
        if (instanceFilter.HasError())
            return CreateParseError<DescriptorOverrides>(instanceFilter.GetError());

        return CreateParseResult(DescriptorOverrides(displayType.GetValue(), contentFlags.GetValue(), fieldsFilterExpression.GetValue(),
            sortingParams.GetValue(), fieldsSelector.GetValue(), std::move(instanceFilter.GetValue())));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int GetContentFlags(int defaultValue = 0) const {return m_contentFlags.IsValid() ? m_contentFlags.Value() : defaultValue;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP GetDisplayType(Utf8CP defaultValue = ContentDisplayType::Undefined) const { return m_displayType.IsValid() ? m_displayType.Value().c_str() : defaultValue; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Apply(ContentDescriptorCPtr& descriptor) const
        {
        bool didModifyDescriptor = false;
        ContentDescriptorPtr descriptorCopy = ContentDescriptor::Create(*descriptor);
        if (!m_contentFlags.IsNull())
            {
            descriptorCopy->SetContentFlags(m_contentFlags.Value());
            didModifyDescriptor = true;
            }
        if (!m_displayType.IsNull())
            {
            descriptorCopy->SetPreferredDisplayType(m_displayType.Value());
            didModifyDescriptor = true;
            }
        if (!m_fieldsFilterExpression.IsNull())
            {
            descriptorCopy->SetFieldsFilterExpression(m_fieldsFilterExpression.Value());
            didModifyDescriptor = true;
            }
        if (!m_sortParams.IsNull())
            {
            auto sortingField = descriptor->FindField(*m_sortParams.Value().first);
            if (sortingField)
                descriptorCopy->SetSortingField(sortingField->GetUniqueName().c_str());
            descriptorCopy->SetSortDirection(m_sortParams.Value().second);
            didModifyDescriptor = true;
            }
        if (m_fieldsSelector.first != FieldSelectorType::None)
            {
            bvector<ContentDescriptor::Field const*> fields;
            for (auto const& matcher : m_fieldsSelector.second)
                {
                auto field = descriptorCopy->FindField(*matcher);
                if (field)
                    fields.push_back(field);
                }
            if (m_fieldsSelector.first == FieldSelectorType::Exclude)
                didModifyDescriptor |= descriptorCopy->ExcludeFields(fields);
            else if (m_fieldsSelector.first == FieldSelectorType::Include)
                didModifyDescriptor |= descriptorCopy->ExclusivelyIncludeFields(fields);
            }
        if (m_instanceFilter)
            {
            descriptorCopy->SetInstanceFilter(m_instanceFilter);
            didModifyDescriptor = true;
            }
        if (didModifyDescriptor)
            descriptor = descriptorCopy;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentDescriptor(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto keys = ParseKeysFromJson(*connection, paramsJson);
    if (keys.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, keys.GetError());

    auto displayType = ParseDisplayTypeFromJson(paramsJson);
    if (displayType.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, displayType.GetError());
    Utf8CP displayTypeValue = displayType.GetValue().IsValid() ? displayType.GetValue().Value().c_str() : "";

    auto contentFlags = ParseContentFlagsFromJson(paramsJson);
    if (contentFlags.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, contentFlags.GetError());
    int contentFlagsValue = contentFlags.GetValue().IsValid() ? contentFlags.GetValue().Value() : 0;

    ContentDescriptorRequestParams descriptorParams(
        ContentMetadataRequestParams(rulesetParams.GetValue(), displayTypeValue, contentFlagsValue),
        *keys.GetValue());
    return manager.GetContentDescriptor(CreateAsyncParams(descriptorParams, db, paramsJson))
        .then([formatter = &manager.GetFormatter()](ContentDescriptorResponse response)
            {
            auto const& descriptor = *response;
            if (descriptor.IsNull())
                return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), true);

            ECPresentationSerializerContext serializerCtx(descriptor->GetUnitSystem(), formatter);
            return ECPresentationResult(descriptor->AsJson(serializerCtx), true);
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContent(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto keys = ParseKeysFromJson(*connection, paramsJson);
    if (keys.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, keys.GetError());

    auto descriptorOverrides = DescriptorOverrides::ParseFromJson(*connection, paramsJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverrides]);
    if (descriptorOverrides.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, descriptorOverrides.GetError());

    auto pageOptions = ParsePageOptionsFromJson(paramsJson);
    if (pageOptions.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, pageOptions.GetError());

    auto diagnostics = ECPresentation::Diagnostics::GetCurrentScope().lock();

    ContentDescriptorRequestParams descriptorParams(
        ContentMetadataRequestParams(rulesetParams.GetValue(), descriptorOverrides.GetValue().GetDisplayType(), descriptorOverrides.GetValue().GetContentFlags()),
        *keys.GetValue());
    return manager.GetContentDescriptor(CreateAsyncParams(descriptorParams, db, paramsJson))
        .then([&manager, &db, descriptorOverrides = descriptorOverrides.GetValue(), pageOptions = pageOptions.GetValue(), formatter = &manager.GetFormatter(), diagnostics](ContentDescriptorResponse descriptorResponse) -> folly::Future<ECPresentationResult>
            {
            auto scope = diagnostics->Hold();

            ContentDescriptorCPtr descriptor = *descriptorResponse;
            if (descriptor.IsNull())
                return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), true);

            descriptorOverrides.Apply(descriptor);
            return manager.GetContent(ECPresentation::MakePaged(AsyncContentRequestParams::Create(db, *descriptor), pageOptions))
                .then([formatter](ContentResponse contentResponse)
                    {
                    auto const& content = *contentResponse;
                    if (content.IsNull())
                        return ECPresentationResult(ECPresentationStatus::Error, "Error creating content");

                    ECPresentationSerializerContext serializerCtx(content->GetDescriptor().GetUnitSystem(), formatter);
                    return ECPresentationResult(content->AsJson(serializerCtx), true);
                    });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentSetSize(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto keys = ParseKeysFromJson(*connection, paramsJson);
    if (keys.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, keys.GetError());

    auto descriptorOverrides = DescriptorOverrides::ParseFromJson(*connection, paramsJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverrides]);
    if (descriptorOverrides.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, descriptorOverrides.GetError());

    auto diagnostics = ECPresentation::Diagnostics::GetCurrentScope().lock();

    ContentDescriptorRequestParams descriptorParams(
        ContentMetadataRequestParams(rulesetParams.GetValue(), descriptorOverrides.GetValue().GetDisplayType(), descriptorOverrides.GetValue().GetContentFlags()),
        *keys.GetValue());
    return manager.GetContentDescriptor(CreateAsyncParams(descriptorParams, db, paramsJson))
        .then([&manager, &db, descriptorOverrides = descriptorOverrides.GetValue(), diagnostics](ContentDescriptorResponse descriptorResponse) -> folly::Future<ECPresentationResult>
        {
        auto scope = diagnostics->Hold();

        ContentDescriptorCPtr descriptor = *descriptorResponse;
        if (descriptor.IsNull())
            return ECPresentationResult(rapidjson::Value(0), true);

        descriptorOverrides.Apply(descriptor);
        return manager.GetContentSetSize(AsyncContentRequestParams::Create(db, *descriptor)).then([](ContentSetSizeResponse contentSetSizeResponse)
            {
            return ECPresentationResult(rapidjson::Value((int64_t)*contentSetSizeResponse), true);
            });
        });
    }

#define PRESENTATION_JSON_ATTRIBUTE_FieldDescriptor     "fieldDescriptor"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetPagedDistinctValues(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    if (!paramsJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_FieldDescriptor))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, PRESENTATION_JSON_ATTRIBUTE_FieldDescriptor);
    std::shared_ptr<IContentFieldMatcher> fieldMatcher = CreateFieldMatcherFromJson(paramsJson[PRESENTATION_JSON_ATTRIBUTE_FieldDescriptor], db);
    if (!fieldMatcher)
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("`" PRESENTATION_JSON_ATTRIBUTE_FieldDescriptor "` contains invalid field descriptor"));

    auto rulesetParams = ParseRulesetParamsFromJson(paramsJson);
    if (rulesetParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, rulesetParams.GetError());

    auto keys = ParseKeysFromJson(*connection, paramsJson);
    if (keys.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, keys.GetError());

    auto descriptorOverrides = DescriptorOverrides::ParseFromJson(*connection, paramsJson[PRESENTATION_JSON_ATTRIBUTE_DescriptorOverrides]);
    if (descriptorOverrides.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, descriptorOverrides.GetError());

    auto pageOptions = ParsePageOptionsFromJson(paramsJson);
    if (pageOptions.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, pageOptions.GetError());

    auto diagnostics = ECPresentation::Diagnostics::GetCurrentScope().lock();

    ContentDescriptorRequestParams descriptorParams(
        ContentMetadataRequestParams(rulesetParams.GetValue(), descriptorOverrides.GetValue().GetDisplayType(), descriptorOverrides.GetValue().GetContentFlags()),
        *keys.GetValue());
    return manager.GetContentDescriptor(CreateAsyncParams(descriptorParams, db, paramsJson))
        .then([&manager, &db, descriptorOverrides = descriptorOverrides.GetValue(), fieldMatcher, pageOptions = pageOptions.GetValue(), diagnostics](ContentDescriptorResponse descriptorResponse) -> folly::Future<ECPresentationResult>
            {
            auto scope = diagnostics->Hold();

            ContentDescriptorCPtr descriptor = *descriptorResponse;
            if (descriptor.IsNull())
                return ECPresentationResult(ECPresentationStatus::InvalidArgument, "descriptor");

            descriptorOverrides.Apply(descriptor);
            return manager.GetDistinctValues(ECPresentation::MakePaged(CreateAsyncParams(DistinctValuesRequestParams(*descriptor, fieldMatcher), db), pageOptions))
                .then([](DistinctValuesResponse distinctValuesResponse)
                    {
                    rapidjson::Document response(rapidjson::kObjectType);
                    rapidjson::Value valuesJson(rapidjson::kArrayType);
                    for (DisplayValueGroupCPtr value : *distinctValuesResponse)
                        PUSH_JSON_IF_VALID(valuesJson, response.GetAllocator(), value);
                    response.AddMember("items", valuesJson, response.GetAllocator());
                    response.AddMember("total", (uint64_t)(*distinctValuesResponse).GetTotalSize(), response.GetAllocator());
                    return ECPresentationResult(std::move(response), true);
                    });
            });
    }

#define PRESENTATION_JSON_ATTRIBUTE_GetDisplayLabel_Key "key"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetDisplayLabel(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto key = ParseECInstanceKeyFromJson(*connection, paramsJson[PRESENTATION_JSON_ATTRIBUTE_GetDisplayLabel_Key], PRESENTATION_JSON_ATTRIBUTE_GetDisplayLabel_Key);
    if (key.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, key.GetError());

    ECInstanceDisplayLabelRequestParams params(ECInstanceKey(key.GetValue().GetClass()->GetId(), key.GetValue().GetId()));
    return manager.GetDisplayLabel(CreateAsyncParams(params, db, paramsJson))
        .then([](DisplayLabelResponse labelResponse)
            {
            auto const& labelDefinition = *labelResponse;
            if (labelDefinition.IsNull())
                return ECPresentationResult(ECPresentationStatus::Error, "Failed to get requested label");

            return ECPresentationResult(labelDefinition->AsJson(), true);
            });
    }

#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken                      "continuationToken"
#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_PrevHierarchyNode    "prevHierarchyNode"
#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_CurrHierarchyNode    "currHierarchyNode"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<HierarchyComparePositionPtr> ParseHierarchyCompareContinuationTokenFromJson(RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken))
        return CreateParseResult<HierarchyComparePositionPtr>(nullptr);

    RapidJsonValueCR tokenJson = json[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken];
    if (!tokenJson.IsObject())
        return CreateParseError<HierarchyComparePositionPtr>("Expected `" PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken "` to be an object");

    if (!tokenJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_PrevHierarchyNode) || !tokenJson[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_PrevHierarchyNode].IsString())
        return CreateParseError<HierarchyComparePositionPtr>("Expected `" PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken "." PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_PrevHierarchyNode "` to be a string");

    if (!tokenJson.HasMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_CurrHierarchyNode) || !tokenJson[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_CurrHierarchyNode].IsString())
        return CreateParseError<HierarchyComparePositionPtr>("Expected `" PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken "." PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_CurrHierarchyNode "` to be a string");

    return CreateParseResult(std::make_shared<HierarchyComparePosition>(
        tokenJson[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_PrevHierarchyNode].GetString(),
        tokenJson[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_CurrHierarchyNode].GetString()));
    }

#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ExpandedNodeKeys                       "expandedNodeKeys"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<bvector<NavNodeKeyCPtr>> ParseNavNodeKeysFromJson(IConnectionCR connection, RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ExpandedNodeKeys) || !json[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ExpandedNodeKeys].IsString())
        return CreateParseError<bvector<NavNodeKeyCPtr>>("Expected `" PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ExpandedNodeKeys "` to be a string");

    Utf8CP serializedExpandedNodeKeys = json[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ExpandedNodeKeys].GetString();
    return CreateParseResult(IModelJsECPresentationSerializer::GetNavNodeKeysFromSerializedJson(connection, serializedExpandedNodeKeys));
    }

#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ResultSetSize                    "resultSetSize"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ParseResult<int> ParseResultSetSizeFromJson(RapidJsonValueCR json)
    {
    if (!json.HasMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ResultSetSize))
        return CreateParseResult(-1);

    if (!json[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ResultSetSize].IsInt())
        return CreateParseError<int>("Expected `" PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ResultSetSize "` to be an integer");

    return CreateParseResult(json[PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ResultSetSize].GetInt());
    }

#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_PrevRulesetId                    "prevRulesetId"
#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_PrevRulesetVariables             "prevRulesetVariables"
#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_CurrRulesetId                    "currRulesetId"
#define PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_CurrRulesetVariables             "currRulesetVariables"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::CompareHierarchies(ECPresentationManager& manager, ECDbR db, RapidJsonValueCR paramsJson)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return folly::makeFutureWith([](){return ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open");});

    auto prevRulesetId = ParseRulesetIdFromJson(paramsJson, PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_PrevRulesetId);
    if (prevRulesetId.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, prevRulesetId.GetError());

    auto prevRulesetVariables = ParseRulesetVariablesFromJson(paramsJson, PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_PrevRulesetVariables);
    if (prevRulesetVariables.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, prevRulesetVariables.GetError());

    auto currRulesetId = ParseRulesetIdFromJson(paramsJson, PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_CurrRulesetId);
    if (currRulesetId.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, currRulesetId.GetError());

    auto currRulesetVariables = ParseRulesetVariablesFromJson(paramsJson, PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_CurrRulesetVariables);
    if (currRulesetVariables.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, currRulesetVariables.GetError());

    auto expandedNodeKeys = ParseNavNodeKeysFromJson(*connection, paramsJson);
    if (expandedNodeKeys.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, expandedNodeKeys.GetError());

    auto continuationToken = ParseHierarchyCompareContinuationTokenFromJson(paramsJson);
    if (continuationToken.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, continuationToken.GetError());

    auto baseParams = ParseBaseRequestParamsFromJson(paramsJson);
    if (baseParams.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, baseParams.GetError());

    auto resultSetSize = ParseResultSetSizeFromJson(paramsJson);
    if (resultSetSize.HasError())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, resultSetSize.GetError());

    auto updateRecordsHandler = std::make_shared<IModelJsECPresentationHierarchiesCompareRecordsHandler>();
    HierarchyCompareRequestParams params(baseParams.GetValue(), updateRecordsHandler,
        prevRulesetId.GetValue(), prevRulesetVariables.GetValue(),
        currRulesetId.GetValue(), currRulesetVariables.GetValue(),
        expandedNodeKeys.GetValue(), continuationToken.GetValue(), resultSetSize.GetValue());
    return manager.CompareHierarchies(CreateAsyncParams(params, db, paramsJson))
        .then([updateRecordsHandler](HierarchiesCompareResponse response)
            {
            HierarchyComparePositionPtr continuationToken = *response;

            rapidjson::Document json(rapidjson::kObjectType);
            json.AddMember("changes", updateRecordsHandler->GetReport(&json.GetAllocator()), json.GetAllocator());
            if (nullptr != continuationToken)
                {
                rapidjson::Value continuationJson(rapidjson::kObjectType);
                continuationJson.AddMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_PrevHierarchyNode,
                    rapidjson::Value(continuationToken->first.c_str(), json.GetAllocator()), json.GetAllocator());
                continuationJson.AddMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken_CurrHierarchyNode,
                    rapidjson::Value(continuationToken->second.c_str(), json.GetAllocator()), json.GetAllocator());
                json.AddMember(PRESENTATION_JSON_ATTRIBUTE_HierarchyCompare_ContinuationToken, continuationJson.Move(), json.GetAllocator());
                }
            return ECPresentationResult(std::move(json), true);
            });
    }
