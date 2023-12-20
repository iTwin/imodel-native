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
    ResultSetTooLarge = Error + 2,          /** Requested result set is too large */
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECPresentationResult
{
private:
    ECPresentationStatus m_status;
    std::unique_ptr<rapidjson::Document::AllocatorType> m_successResponseAllocator;
    std::unique_ptr<BeJsDocument> m_successResponse;
    Utf8String m_errorMessage;
    std::unique_ptr<rapidjson::Document> m_diagnostics;
    mutable Utf8String m_serializedSuccessResponse;

private:
    void SerializeSuccessResponse() const;

public:
    //! Don't allow copying
    ECPresentationResult(ECPresentationResult const& other) = delete;
    ECPresentationResult& operator=(ECPresentationResult const& other) = delete;

    //! Create a success result from BeJsDocument
    ECPresentationResult(BeJsDocument&& successResponse, std::unique_ptr<rapidjson::Document> diagnostics = nullptr)
        : m_status(ECPresentationStatus::Success), m_successResponse(std::make_unique<BeJsDocument>(std::move(successResponse))), m_diagnostics(std::move(diagnostics))
        {
        SerializeSuccessResponse();
        }
    //! Create a success result from rapidjson document that uses a custom allocator
    ECPresentationResult(std::unique_ptr<rapidjson::Document::AllocatorType> successResponseAllocator, rapidjson::Document&& successResponse, std::unique_ptr<rapidjson::Document> diagnostics = nullptr)
        : m_status(ECPresentationStatus::Success), m_successResponseAllocator(std::move(successResponseAllocator)), m_successResponse(std::make_unique<BeJsDocument>(std::move(successResponse))), m_diagnostics(std::move(diagnostics))
        {
        SerializeSuccessResponse();
        }
    //! Create a success result from rapidjson document that holds its own allocator
    ECPresentationResult(rapidjson::Document&& successResponse, std::unique_ptr<rapidjson::Document> diagnostics = nullptr)
        : ECPresentationResult(nullptr, std::move(successResponse), std::move(diagnostics))
        {}
    //! Create a success result with primitive value response (no allocator needed to create it)
    ECPresentationResult(rapidjson::Value const& successResponse, std::unique_ptr<rapidjson::Document> diagnostics = nullptr)
        : m_status(ECPresentationStatus::Success), m_diagnostics(std::move(diagnostics))
        {
        m_successResponseAllocator = std::make_unique<rapidjson::Document::AllocatorType>(8u);
        m_successResponse = std::make_unique<BeJsDocument>(m_successResponseAllocator.get());
        m_successResponse->From(BeJsConst(successResponse, *m_successResponseAllocator));
        }
    //! Create an error result
    ECPresentationResult(ECPresentationStatus errorCode, Utf8String message, std::unique_ptr<rapidjson::Document> diagnostics = nullptr)
        : m_status(errorCode), m_errorMessage(message), m_diagnostics(std::move(diagnostics))
        {}
    //! Move constructor
    ECPresentationResult(ECPresentationResult&& other)
        : m_status(other.m_status), m_errorMessage(std::move(other.m_errorMessage)), m_serializedSuccessResponse(std::move(other.m_serializedSuccessResponse)),
        m_successResponse(std::move(other.m_successResponse)), m_successResponseAllocator(std::move(other.m_successResponseAllocator))
        {
        m_diagnostics = std::move(other.m_diagnostics);
        }
    //! Move assignment
    ECPresentationResult& operator=(ECPresentationResult&& other)
        {
        m_status = other.m_status;
        m_errorMessage.swap(other.m_errorMessage);
        m_diagnostics = std::move(other.m_diagnostics);
        m_successResponse = std::move(other.m_successResponse);
        m_successResponseAllocator = std::move(other.m_successResponseAllocator);
        m_serializedSuccessResponse.swap(other.m_serializedSuccessResponse);
        return *this;
        }
    void SetDiagnostics(std::unique_ptr<rapidjson::Document> diagnostics) {m_diagnostics = std::move(diagnostics);}
    bool IsError() const {return ECPresentationStatus::Success != m_status;}
    ECPresentationStatus GetStatus() const {return m_status;}
    Utf8StringCR GetErrorMessage() const {return m_errorMessage;}
    rapidjson::Document const* GetDiagnostics() const {return m_diagnostics.get();}
    BeJsDocument const& GetSuccessResponse() const {return *m_successResponse;}
    Utf8StringCR GetSerializedSuccessResponse() const {SerializeSuccessResponse(); return m_serializedSuccessResponse;}
    Utf8StringR GetSerializedSuccessResponse() {SerializeSuccessResponse(); return m_serializedSuccessResponse;}
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECPresentationUtils
{
    static Utf8CP GetLoggerNamespace();
    static NativeLogging::CategoryLogger GetLogger();

    static ECPresentation::Diagnostics::Options CreateDiagnosticsOptions(RapidJsonValueCR);
    static ECPresentationResult CreateResultFromException(folly::exception_wrapper const&, std::unique_ptr<rapidjson::Document> diagnostics = nullptr);

    static ECPresentationManager* CreatePresentationManager(Dgn::PlatformLib::Host::IKnownLocationsAdmin&, IJsonLocalState&,
        std::shared_ptr<IUpdateRecordsHandler>, BeJsConst props);

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
    static folly::Future<ECPresentationResult> GetContentSet(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetContentSetSize(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetPagedDistinctValues(ECPresentationManager&, ECDbR, RapidJsonValueCR params);
    static folly::Future<ECPresentationResult> GetDisplayLabel(ECPresentationManager&, ECDbR, RapidJsonValueCR params);

    static ECPresentationResult SetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType, BeJsConst value);
    static ECPresentationResult UnsetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId);
    static ECPresentationResult GetRulesetVariableValue(ECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
