/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpRequest.h>
#include <iModelDmsSupport/CommonDefinition.h>

#define REPOSITORYTYPE "PROJECTSHARE"

BEGIN_BENTLEY_DGN_NAMESPACE

struct PWShareDmsSupport
    {
    private:
        Utf8String m_projectId;
        Utf8String m_folderId;
        Utf8String m_fileId;
        bool _ParseInputURL(Utf8String url);

    public:
        bool _InitializeSession(Utf8String fileUrl);
        bool _UnInitializeSession();
        bmap<WString, WString> _GetDownloadURLs(Utf8String token);
    };

END_BENTLEY_DGN_NAMESPACE