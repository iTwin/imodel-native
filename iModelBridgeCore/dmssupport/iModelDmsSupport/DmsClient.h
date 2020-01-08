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
        bool _ParseInputUrlForPS(Utf8String url);
        bool _ParseInputUrlForPW(Utf8String url);
        Utf8PrintfString _CreateQuery(Utf8String datasource = Utf8String());

    public:
        bool _InitializeSession(Utf8String fileUrl, Utf8String repositoryType);
        bool _UnInitializeSession();
        bvector<DmsResponseData> _GetDownloadURLs(Utf8String token, Utf8String datasource = Utf8String());
    };

END_BENTLEY_DGN_NAMESPACE