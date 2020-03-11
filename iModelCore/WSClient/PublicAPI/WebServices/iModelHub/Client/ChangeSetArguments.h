/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/BridgeProperties.h>
#include <WebServices/iModelHub/Client/ChangeSetInfo.h>
#include <WebServices/iModelHub/Client/ConflictsInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef RefCountedPtr<struct PushChangeSetArguments> PushChangeSetArgumentsPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(PushChangeSetArguments);
typedef RefCountedPtr<struct PullChangeSetsArguments> PullChangeSetsArgumentsPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(PullChangeSetsArguments);
typedef std::function<Tasks::AsyncTaskPtr<void>(Dgn::DgnCodeSet const&, Dgn::DgnCodeSet const&)> CodeCallbackFunction;

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2019
//=======================================================================================
struct PushChangeSetArguments : RefCountedBase
{
private:
    Utf8CP m_description;
    ChangeSetKind m_containingChanges;
    BridgePropertiesPtr m_bridgeProperties;
    bool m_relinquishCodesLocks;
    Http::Request::ProgressCallbackCR m_callback;
    Dgn::IBriefcaseManager::ResponseOptions m_options;
    ICancellationTokenPtr m_cancellationToken;
    ConflictsInfoPtr m_conflictsInfo;
    CodeCallbackFunction* m_codesCallback;

    PushChangeSetArguments(Utf8CP description, ChangeSetKind containingChanges,
        BridgePropertiesPtr bridgeProperties, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR callback,
        Dgn::IBriefcaseManager::ResponseOptions options, ICancellationTokenPtr cancellationToken,
        ConflictsInfoPtr conflictsInfo, CodeCallbackFunction* codesCallback) :
        m_description(description), m_containingChanges(containingChanges), m_bridgeProperties(bridgeProperties),
        m_relinquishCodesLocks(relinquishCodesLocks), m_callback(callback), m_options(options), 
        m_cancellationToken(cancellationToken), m_conflictsInfo(conflictsInfo), m_codesCallback(codesCallback)
        {}
public:
    //! Create an instance of ChangeSet push arguments.
    //! @param[in] description ChangeSet description.
    //! @param[in] containingChanges Kind of changes are contained in this ChangeSet.
    //! @param[in] bridgeProperties Additional bridge properties attached to the ChangeSet.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] callback Upload progress callback.
    //! @param[in] options
    //! @param[in] cancellationToken
    //! @param[in] conflictsInfo Retruns codes/locks conflicts that occured during push.
    //! @param[in] codesCallback Callback for external codes. First argument is assigned codes, second is discarded codes.
    //! @return Returns a shared pointer to the created instance.
    static PushChangeSetArgumentsPtr Create
        (
        Utf8CP description = nullptr, 
        ChangeSetKind containingChanges = ChangeSetKind::NotSpecified,
        BridgePropertiesPtr bridgeProperties = nullptr,
        bool relinquishCodesLocks = false,
        Http::Request::ProgressCallbackCR callback = nullptr,
        Dgn::IBriefcaseManager::ResponseOptions options = Dgn::IBriefcaseManager::ResponseOptions::None,
        ICancellationTokenPtr cancellationToken = nullptr,
        ConflictsInfoPtr conflictsInfo = nullptr,
        CodeCallbackFunction* codesCallback = nullptr
        )
        { return PushChangeSetArgumentsPtr(new PushChangeSetArguments(description, containingChanges, bridgeProperties,
            relinquishCodesLocks, callback, options, cancellationToken, conflictsInfo, codesCallback)); }

    Utf8CP GetDescription() const { return m_description; }
    ChangeSetKind GetContainingChanges() const { return m_containingChanges; }
    const BridgePropertiesPtr GetBridgeProperties() const { return m_bridgeProperties; }
    bool GetRelinquishCodesLocks() const { return m_relinquishCodesLocks; }
    Http::Request::ProgressCallbackCR GetProgressCallback() const { return m_callback; }
    Dgn::IBriefcaseManager::ResponseOptions GetResponseOptions() const { return m_options; }
    ICancellationTokenPtr GetCancelationToken() const { return m_cancellationToken; }
    ConflictsInfoPtr GetConflictsInfo() const { return m_conflictsInfo; }
    CodeCallbackFunction* GetCodesCallback() const { return m_codesCallback; }

    //! Validates push ChangeSet arguments.
    //! @param[in] changeSet
    //! @param[in] dgndb
    IMODELHUBCLIENT_EXPORT StatusResult Validate(Dgn::DgnRevisionPtr changeSet, Dgn::DgnDbR dgndb);
};


//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2019
//=======================================================================================
struct PullChangeSetsArguments : RefCountedBase
{
private:
    Http::Request::ProgressCallbackCR m_callback;
    ICancellationTokenPtr m_cancellationToken;

    PullChangeSetsArguments(Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) :
        m_callback(callback), m_cancellationToken(cancellationToken)
        {}
public:
    //! Create an instance of ChangeSets pull arguments.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Returns a shared pointer to the created instance.
    static PullChangeSetsArgumentsPtr Create(
        Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr)
        { return PullChangeSetsArgumentsPtr(new PullChangeSetsArguments(callback, cancellationToken)); }

    Http::Request::ProgressCallbackCR GetProgressCallback() const { return m_callback; }
    ICancellationTokenPtr GetCancelationToken() const { return m_cancellationToken; }
};

END_BENTLEY_IMODELHUB_NAMESPACE
