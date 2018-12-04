/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/ChangeSetCacheManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

DEFINE_POINTER_SUFFIX_TYPEDEFS(ChangeSetCacheManager);
DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelConnection);
typedef std::function<void(EventPtr)> EventCallback;
typedef std::shared_ptr<EventCallback> EventCallbackPtr;

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             01/2017
//=======================================================================================
struct ChangeSetCacheManager
{
private:
    friend struct iModelConnection;
    iModelConnectionP        m_connection;
    mutable int              m_preDownloadCacheSize; //10 MB
    mutable EventCallbackPtr m_preDownloadCallback;

    ChangeSetCacheManager() {};
    ChangeSetCacheManager(iModelConnectionP connection)
        : m_connection(connection), m_preDownloadCacheSize(10 * 1024 * 1024){}

    //! Gets path for pre-downloaded changeSet
    BeFileName static BuildChangeSetPredownloadPathname(Utf8String changeSetId);

    //! Pre-downloads single changeSet by changeSetId
    StatusTaskPtr PredownloadChangeSet(iModelConnectionCP imodelConnectionP, Utf8StringCR changeSetId,
                                       Http::Request::ProgressCallbackCR callback = nullptr, 
                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    bvector<BeFileName> GetOrderedCacheFiles(BeFileName directoryName) const;
    uint64_t GetCacheSize(BeFileName directoryName) const;
    void CheckCacheSize(BeFileName changeSetFileName) const;

    bool TryGetChangeSetFile(BeFileName changeSetFileName, Utf8String changeSetId) const;

public:
    //! Enables change set background download.
    //! @return Asynchronous task that returns if background download enable succeeded.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr EnableBackgroundDownload() const;

    //! Disables change set background download.
    //! @return Asynchronous task that returns if background download disable succeeded.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DisableBackgroundDownload() const;

    //! Gets maximum allowed background download cache size.
    int GetMaxCacheSize() const {return m_preDownloadCacheSize;}

    //! Sets maximum allowed background cache size in bytes.
    //! @param[in] cacheSize
    void SetMaxCacheSize(int cacheSize) const {if (cacheSize > 0) m_preDownloadCacheSize = cacheSize;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
