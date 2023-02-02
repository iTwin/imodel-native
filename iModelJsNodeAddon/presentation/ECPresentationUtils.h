/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include <DgnPlatform/PlatformLib.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
//! @bsiclass
//=======================================================================================
enum class ECPresentationStatus
    {
    Success = 0,
    Canceled = 1,                           /** Request was canceled */
    Pending = 2,                            /** Request is still being processed */
    Error = 0x10000,                        /** Base error */
    InvalidArgument = Error + 1,            /** Argument is invalid */
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECPresentationResult
{
private:
    ECPresentationStatus m_status;
    rapidjson::Document m_successResponse;
    Json::Value m_jsoncppSuccessResponse;
    Utf8String m_errorMessage;
    bool m_isJsonCppResponse;
    rapidjson::Document m_diagnostics;
    mutable Utf8String m_serializedSuccessResponse;

public:
    //! Don't allow copying
    ECPresentationResult(ECPresentationResult const& other) = delete;
    ECPresentationResult& operator=(ECPresentationResult const& other) = delete;

    //! Create a success result with response
    ECPresentationResult(rapidjson::Document&& successResponse, bool serializeResponse, rapidjson::Document&& diagnostics = rapidjson::Document())
        : m_status(ECPresentationStatus::Success), m_successResponse(std::move(successResponse)), m_isJsonCppResponse(false), m_diagnostics(std::move(diagnostics))
        {
        if (serializeResponse)
            GetSerializedSuccessResponse();
        }
    //! Create a success result with response
    ECPresentationResult(rapidjson::Value&& successResponse, bool serializeResponse, rapidjson::Document&& diagnostics = rapidjson::Document())
        : m_status(ECPresentationStatus::Success), m_isJsonCppResponse(false), m_diagnostics(std::move(diagnostics))
        {
        successResponse.Swap(m_successResponse);
        if (serializeResponse)
            GetSerializedSuccessResponse();
        }
    //! Create a success result with response
    ECPresentationResult(Json::Value successResponse, bool serializeResponse, rapidjson::Document&& diagnostics = rapidjson::Document())
        : m_status(ECPresentationStatus::Success), m_jsoncppSuccessResponse(successResponse), m_isJsonCppResponse(true), m_diagnostics(std::move(diagnostics))
        {
        if (serializeResponse)
            GetSerializedSuccessResponse();
        }
    //! Create an error result
    ECPresentationResult(ECPresentationStatus errorCode, Utf8String message, rapidjson::Document&& diagnostics = rapidjson::Document())
        : m_status(errorCode), m_errorMessage(message), m_diagnostics(std::move(diagnostics))
        {}
    //! Move constructor
    ECPresentationResult(ECPresentationResult&& other)
        : m_status(other.m_status), m_isJsonCppResponse(other.m_isJsonCppResponse), m_errorMessage(std::move(other.m_errorMessage)), m_serializedSuccessResponse(std::move(other.m_serializedSuccessResponse))
        {
        m_diagnostics.Swap(other.m_diagnostics);
        m_successResponse.Swap(other.m_successResponse);
        m_jsoncppSuccessResponse.swap(other.m_jsoncppSuccessResponse);
        }
    //! Move assignment
    ECPresentationResult& operator=(ECPresentationResult&& other)
        {
        m_status = other.m_status;
        m_isJsonCppResponse = other.m_isJsonCppResponse;
        m_errorMessage.swap(other.m_errorMessage);
        m_diagnostics.Swap(other.m_diagnostics);
        m_successResponse.Swap(other.m_successResponse);
        m_jsoncppSuccessResponse.swap(other.m_jsoncppSuccessResponse);
        m_serializedSuccessResponse.swap(other.m_serializedSuccessResponse);
        return *this;
        }
    void SetDiagnostics(rapidjson::Document&& diagnostics) {m_diagnostics.Swap(diagnostics);}
    bool IsError() const {return ECPresentationStatus::Success != m_status;}
    bool IsJsonCppResponse() const {return m_isJsonCppResponse;}
    ECPresentationStatus GetStatus() const {return m_status;}
    Utf8StringCR GetErrorMessage() const {return m_errorMessage;}
    rapidjson::Document const& GetDiagnostics() const {return m_diagnostics;}
    rapidjson::Document const& GetSuccessResponse() const {return m_successResponse;}
    JsonValueCR GetJsonCppSuccessResponse() const {return m_jsoncppSuccessResponse;}
    Utf8StringCR GetSerializedSuccessResponse() const;
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECPresentationUtils
{
    static Utf8CP GetLoggerNamespace();
    static NativeLogging::CategoryLogger GetLogger();

    static ECPresentation::Diagnostics::Options CreateDiagnosticsOptions(RapidJsonValueCR);
    static ECPresentationResult CreateResultFromException(folly::exception_wrapper const&);

    static ECPresentationManager* CreatePresentationManager(Dgn::PlatformLib::Host::IKnownLocationsAdmin&, IJsonLocalState&,
        std::shared_ptr<IUpdateRecordsHandler>, std::shared_ptr<IUiStateProvider>, Napi::Object const& props);

    static ECPresentationResult SetupRulesetDirectories(ECPresentationManager&, bvector<Utf8String> const&);
    static ECPresentationResult SetupSupplementalRulesetDirectories(ECPresentationManager&, bvector<Utf8String> const&);

    static ECPresentationResult GetRulesets(SimpleRuleSetLocater&, Utf8StringCR ruleSetId);
    static ECPresentationResult AddRuleset(SimpleRuleSetLocater&, Utf8StringCR rulesetJsonString);
    static ECPresentationResult RemoveRuleset(SimpleRuleSetLocater&, Utf8StringCR ruleSetId, Utf8StringCR hash);
    static ECPresentationResult ClearRulesets(SimpleRuleSetLocater&);

    static folly::Future<ECPresentationResult> GetRootNodesCount(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetRootNodes(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetChildrenCount(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetChildren(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetHierarchyLevelDescriptor(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetNodesPaths(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetFilteredNodesPaths(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> CompareHierarchies(ECPresentationManager&, ECDbR, RapidJsonValueCR params);

    static folly::Future<ECPresentationResult> GetContentSources(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetContentDescriptor(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetContent(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetContentSetSize(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetPagedDistinctValues(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetDisplayLabel(ECPresentationManager&, ECDbR, RapidJsonValueCR params);

    static ECPresentationResult SetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType, BeJsConst value);
    static ECPresentationResult UnsetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId);
    static ECPresentationResult GetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
