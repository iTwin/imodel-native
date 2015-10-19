/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryConnection.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/RevisionInfo.h>
#include <DgnDbServer/Client/FileInfo.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbRepositoryConnection> DgnDbRepositoryConnectionPtr;
struct DgnDbRepositoryConnection
{
//__PUBLISH_SECTION_END__
private:
    RepositoryInfo m_repositoryInfo;
    WebServices::IWSRepositoryClientPtr m_wsRepositoryClient;

    DgnDbRepositoryConnection(RepositoryInfoCR repository, Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
//__PUBLISH_SECTION_START__
public:
    static DgnDbRepositoryConnectionPtr Create(RepositoryInfoCR repository, Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
//__PUBLISH_SECTION_END__
    MobileDgn::Utils::AsyncTaskPtr<DgnDbServerResult<RevisionInfo>> Push(FileInfo revision, WebServices::HttpRequest::ProgressCallbackCR callback = nullptr, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
    MobileDgn::Utils::AsyncTaskPtr<DgnDbServerResult<bvector<RevisionInfo>>> Pull(WebServices::HttpRequest::ProgressCallbackCR callback = nullptr, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
//__PUBLISH_SECTION_START__
    DgnDbServerTaskPtr VerifyConnection(WebServices::ICancellationTokenPtr cancellationToken = nullptr);

    RepositoryInfoCR GetRepositoryInfo();
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
