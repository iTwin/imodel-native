/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! @bsiclass                                   Grigas.Petraitis                05/2018
//=======================================================================================
enum class ECPresentationStatus
    {
    Success = 0,
    Error = 0x10000,                        /** Base error */
    InvalidArgument = Error + 1,            /** Argument is invalid */
    };

//=======================================================================================
//! @bsiclass                                   Grigas.Petraitis                05/2018
//=======================================================================================
struct ECPresentationResult
{
private:
    ECPresentationStatus m_status;
    rapidjson::Document m_successResponse;
    Json::Value m_jsoncppSuccessResponse;
    Utf8String m_errorMessage;
    bool m_isJsonCppResponse;

public:
    //! Create a success result with response
    ECPresentationResult(rapidjson::Document&& successResponse) : m_status(ECPresentationStatus::Success), m_successResponse(std::move(successResponse)), m_isJsonCppResponse(false) {}
    //! Create a success result with response
    ECPresentationResult(rapidjson::Value&& successResponse) : m_status(ECPresentationStatus::Success), m_isJsonCppResponse(false) {successResponse.Swap(m_successResponse);}
    //! Create a success result with response
    ECPresentationResult(Json::Value&& successResponse) : m_status(ECPresentationStatus::Success), m_jsoncppSuccessResponse(std::move(successResponse)), m_isJsonCppResponse(true) {}
    //! Create a succes result with no response
    ECPresentationResult(): m_status(ECPresentationStatus::Success), m_isJsonCppResponse(false) {m_successResponse.SetNull();}

    //! Create an error result
    ECPresentationResult(ECPresentationStatus errorCode, Utf8String message) : m_status(errorCode), m_errorMessage(message) {}

    bool IsError() const {return ECPresentationStatus::Success != m_status;}
    bool IsJsonCppResponse() const {return m_isJsonCppResponse;}
    ECPresentationStatus GetStatus() const {return m_status;}
    Utf8StringCR GetErrorMessage() const {return m_errorMessage;}
    rapidjson::Document const& GetSuccessResponse() const {return m_successResponse;}
    JsonValueCR GetJsonCppSuccessResponse() const {return m_jsoncppSuccessResponse;}
};

//=======================================================================================
//! @bsiclass                                   Grigas.Petraitis                12/2017
//=======================================================================================
struct ECPresentationUtils
{
    static RulesDrivenECPresentationManager* CreatePresentationManager(Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin&, IJsonLocalState&, 
        Utf8StringCR, bvector<Utf8String> const&, bmap<int, unsigned>);

    static ECPresentationResult SetupRulesetDirectories(RulesDrivenECPresentationManager&, bvector<Utf8String> const&);
    static ECPresentationResult SetupSupplementalRulesetDirectories(RulesDrivenECPresentationManager&, bvector<Utf8String> const&);
    
    static ECPresentationResult GetRulesets(SimpleRuleSetLocater&, Utf8StringCR ruleSetId);
    static ECPresentationResult AddRuleset(SimpleRuleSetLocater&, Utf8StringCR rulesetJsonString);
    static ECPresentationResult RemoveRuleset(SimpleRuleSetLocater&, Utf8StringCR ruleSetId, Utf8StringCR hash);
    static ECPresentationResult ClearRulesets(SimpleRuleSetLocater&);

    static folly::Future<ECPresentationResult> GetRootNodesCount(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetRootNodes(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetChildrenCount(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetChildren(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetNodesPaths(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetFilteredNodesPaths(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> LoadHierarchy(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);

    static folly::Future<ECPresentationResult> GetContentDescriptor(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetContent(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetContentSetSize(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetDistinctValues(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);
    static folly::Future<ECPresentationResult> GetDisplayLabel(RulesDrivenECPresentationManager&, ECDbR, JsonValueCR params, PresentationRequestContextCR);

    static ECPresentationResult SetRulesetVariableValue(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType, JsonValueCR value);
    static ECPresentationResult GetRulesetVariableValue(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType);

};

END_BENTLEY_ECPRESENTATION_NAMESPACE
