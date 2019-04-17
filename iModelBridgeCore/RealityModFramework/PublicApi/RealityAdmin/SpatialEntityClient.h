/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include "ISpatialEntityTraversalObserver.h"
#include "SpatialEntityHandler.h"

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>



BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct SpatialEntityServer;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityClient : public RefCountedBase
{
public:
    //! Mapping of the remote and local link to a file.
    typedef bvector<bpair<Utf8String, Utf8String>> RepositoryMapping;

    //! Download all files from root and saved them in the specified directory. Default is temp directory.
    REALITYDATAPLATFORM_EXPORT SpatialEntityHandlerStatus DownloadContent(Utf8CP outputDirPath = NULL) const;

    //! Get a list of all files from all directories starting at root.
    REALITYDATAPLATFORM_EXPORT SpatialEntityHandlerStatus GetFileList(bvector<Utf8String>& fileList) const;

    //! Get complete data from root. This includes base, source, thumbnail, metadata and server details.
    //! An observer must be set to catch the data after each extraction.
    REALITYDATAPLATFORM_EXPORT SpatialEntityHandlerStatus GetData() const;

    //! Get server url.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerUrl() const;

    //! Get server name.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const;

    //! Get provider name.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProviderName() const;

    //! Get file pattern.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataset() const;

    //! Get file pattern.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFilePattern() const;

    //! Get classification.
    REALITYDATAPLATFORM_EXPORT SpatialEntity::Classification GetClassification() const;

    //! Get classification.
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataCR GetMetadataSeed() const;

    //! Get repository mapping (remote and local repository).
    REALITYDATAPLATFORM_EXPORT const RepositoryMapping& GetRepositoryMapping() const;

    //! Set the ISpatialEntityTraversalObserver to be called after each extraction. Only one observer
    //! can be set. Typically, the user of the handler would implement the ISpatialEntityTraversalObserver
    //! interface and send "this" as the argument to this method.
    REALITYDATAPLATFORM_EXPORT void SetObserver(ISpatialEntityTraversalObserver* pObserver);

protected:
    REALITYDATAPLATFORM_EXPORT SpatialEntityClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classsification, SpatialEntityMetadataCR metadataSeed);
    REALITYDATAPLATFORM_EXPORT ~SpatialEntityClient();

    //! Recurse into sub directories and create a list of all files.
    virtual SpatialEntityHandlerStatus _GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const = 0;

    virtual SpatialEntityPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const = 0;

    //! Check if content is a directory or not.
    REALITYDATAPLATFORM_EXPORT bool IsDirectory(Utf8CP content) const;

    //! Callback when a file is downloaded to construct the data repository mapping.
    static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);

    REALITYDATAPLATFORM_EXPORT static ISpatialEntityTraversalObserver* GetObserver();

    BeFileName m_certificatePath;
    SpatialEntityServerPtr m_pServer;
    Utf8String m_datasetName;
    Utf8String m_providerName;
    static ISpatialEntityTraversalObserver* m_pObserver;
    static RepositoryMapping m_dataRepositories;
    static int m_retryCount;
    Utf8String m_filePattern;
    bool m_extractThumbnails;
    SpatialEntity::Classification m_classification;
    SpatialEntityMetadataCR m_metadataSeed;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityRequest : public RefCountedBase
{
public:
    //! Get response from request.
    virtual SpatialEntityResponsePtr Perform();

    //! Get url.
    Utf8StringCR GetUrl() const { return m_url; }

    //! Get/Set method option.
    Utf8StringCR GetMethod() const { return m_method; }
    void SetMethod(Utf8CP method) { m_method = method; }

    //! Get/Set dirList option.
    bool IsDirListOnly() const { return m_dirListOnly; }
    void SetDirListOnly(bool isDirListOnly) { m_dirListOnly = isDirListOnly; }

    //! Get/Set verbose option.
    bool IsVerbose() const { return m_verbose; }
    void SetVerbose(bool isVerbose) { m_verbose = isVerbose; }

protected:
    REALITYDATAPLATFORM_EXPORT SpatialEntityRequest(Utf8CP url);

    Utf8String m_url;
    Utf8String m_method;
    bool m_dirListOnly;
    bool m_verbose;
};


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityResponse : public RefCountedBase
{
public:
    //! Create invalid response with SpatialEntityHandlerStatus::UnknownError.
    static SpatialEntityResponsePtr Create();

    //! Create response.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityResponsePtr Create(Utf8CP effectiveUrl, Utf8CP m_content, SpatialEntityHandlerStatus traversalStatus);

    //! Get url.
    Utf8StringCR GetUrl() const;

    //! Get content.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContent() const;

    //! Get status.
    REALITYDATAPLATFORM_EXPORT SpatialEntityHandlerStatus GetStatus() const;

    //! Return if request was a success or not.
    REALITYDATAPLATFORM_EXPORT bool IsSuccess() const;

protected:
    SpatialEntityResponse(Utf8CP effectiveUrl, Utf8CP content, SpatialEntityHandlerStatus status);

    Utf8String m_effectiveUrl;
    Utf8String m_content;
    SpatialEntityHandlerStatus m_status;
};

END_BENTLEY_REALITYPLATFORM_NAMESPACE