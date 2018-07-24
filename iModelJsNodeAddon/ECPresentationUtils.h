/*--------------------------------------------------------------------------------------+
|
|     $Source: ECPresentationUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    Utf8String m_errorMessage;

public:
    //! Create a success result with response
    ECPresentationResult(rapidjson::Document&& successResponse) : m_status(ECPresentationStatus::Success), m_successResponse(std::move(successResponse)) {}
    //! Create a success result with response
    ECPresentationResult(rapidjson::Value&& successResponse) : m_status(ECPresentationStatus::Success) {successResponse.Swap(m_successResponse);}
    //! Create a succes result with no response
    ECPresentationResult(): m_status(ECPresentationStatus::Success) {m_successResponse.SetNull();}

    //! Create an error result
    ECPresentationResult(ECPresentationStatus errorCode, Utf8String message) : m_status(errorCode), m_errorMessage(message) {}

    bool IsError() const {return ECPresentationStatus::Success != m_status;}
    ECPresentationStatus GetStatus() const {return m_status;}
    Utf8StringCR GetErrorMessage() const {return m_errorMessage;}
    rapidjson::Document const& GetSuccessResponse() const {return m_successResponse;}
};

//=======================================================================================
//! @bsiclass                                   Grigas.Petraitis                12/2017
//=======================================================================================
struct ECPresentationUtils
{
    static RulesDrivenECPresentationManager* CreatePresentationManager(IConnectionManagerR, Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin&);

    static ECPresentationResult SetupRulesetDirectories(RulesDrivenECPresentationManager&, bvector<Utf8String> const&);
    static ECPresentationResult SetupLocaleDirectories(bvector<Utf8String> const&);

    static ECPresentationResult AddRuleSet(SimpleRuleSetLocater&, Utf8StringCR rulesetJsonString);
    static ECPresentationResult RemoveRuleSet(SimpleRuleSetLocater&, Utf8StringCR ruleSetId);
    static ECPresentationResult ClearRuleSets(SimpleRuleSetLocater&);

    static folly::Future<ECPresentationResult> GetRootNodesCount(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetRootNodes(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetChildrenCount(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetChildren(IECPresentationManagerR, ECDbR, JsonValueCR params);

    static folly::Future<ECPresentationResult> GetContentDescriptor(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetContent(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetContentSetSize(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetDistinctValues(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetDisplayLabel(IECPresentationManagerR, ECDbR, JsonValueCR params);

    static folly::Future<ECPresentationResult> GetNodesPaths(IECPresentationManagerR, ECDbR, JsonValueCR params);
    static folly::Future<ECPresentationResult> GetFilteredNodesPaths(IECPresentationManagerR, ECDbR, JsonValueCR params);

    static ECPresentationResult SetUserSetting(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR settingId, Utf8StringCR value);
    static ECPresentationResult GetUserSetting(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR settingId, Utf8StringCR settingType);

};

END_BENTLEY_ECPRESENTATION_NAMESPACE
