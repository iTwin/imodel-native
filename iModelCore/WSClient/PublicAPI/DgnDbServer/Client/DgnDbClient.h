/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbClient.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <WebServices/Client/WSClient.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbClient> DgnDbClientPtr;
struct DgnDbClient
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_host;
    WebServices::Credentials m_credentials;
    WebServices::ClientInfoPtr m_clientInfo;

    DgnDbRepositoryConnectionPtr ConnectToRepository(RepositoryInfoCR repository, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
    DgnDbClient(Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
//__PUBLISH_SECTION_START__
public:
    static DgnDbClientPtr Create(Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbServerResult<bvector<RepositoryInfo>>> GetRepositories(WebServices::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbServerResult<RepositoryInfo>> CreateNewRepository(Utf8StringCR repositoryId, Utf8StringCR description, Utf8StringCR localPath, WebServices::HttpRequest::ProgressCallbackCR callback = nullptr, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbServerResult<DgnDbBriefcasePtr>> OpenBriefcase(Dgn::DgnDbPtr db, bool doSync = true, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbServerResult<FileInfo>> AquireBriefcase(RepositoryInfoCR repository, FileInfoCR file, bool doSync = true, WebServices::HttpRequest::ProgressCallbackCR callback = nullptr, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
