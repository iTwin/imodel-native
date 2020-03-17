/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpRequest.h>
#include <iModelDmsSupport/CommonDefinition.h>

#define PSREPOSITORYTYPE "PROJECTSHARE"
#define PWREPOSITORYTYPE "PWDI"
#define APPLICATIONTYPE "MICROSTATION"

BEGIN_BENTLEY_DGN_NAMESPACE
struct DmsResponseData
    {
    WString fileName;
    WString downloadURL;
    WString parentFolderId;
    WString fileId;
    };
struct DmsClient
    {
    private:
        Utf8String m_projectId;
        Utf8String m_folderId;
        Utf8String m_fileId;
        Utf8String m_repositoryType;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _ParseInputUrlForPS(Utf8String url);
        IMODEL_DMSSUPPORT_EXPORT virtual bool _ParseInputUrlForPW(Utf8String url);
        IMODEL_DMSSUPPORT_EXPORT virtual Utf8PrintfString _CreateQuery(Utf8String datasource = Utf8String(), bool isWSQuery = false);

    public:
        IMODEL_DMSSUPPORT_EXPORT DmsClient() {}
        IMODEL_DMSSUPPORT_EXPORT virtual ~DmsClient() {}
        IMODEL_DMSSUPPORT_EXPORT virtual bool _InitializeSession(Utf8String fileUrl, Utf8String repositoryType);
        IMODEL_DMSSUPPORT_EXPORT virtual bool _UnInitializeSession();
        IMODEL_DMSSUPPORT_EXPORT virtual bvector<DmsResponseData> _GetDownloadURLs(Utf8String token, Utf8String datasource = Utf8String());
        IMODEL_DMSSUPPORT_EXPORT virtual bvector<DmsResponseData> _GetWorkspaceFiles(Utf8String token, Utf8String datasource, DmsResponseData& cfgData);
    };

END_BENTLEY_DGN_NAMESPACE