/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelDmsSupport/DmsSession.h>
#include "PWWorkspaceHelper.h"
#include "AzureBlobStorageHelper.h"
#include "DmsHelper.h"

USING_NAMESPACE_BENTLEY_DGN
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
    {

    IMODEL_DMSSUPPORT_EXPORT IDmsSupport*    iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR callBackurl, Utf8StringCR accessToken, Utf8StringCR datasource, int maxReties, unsigned long productId)
        {
        return iModelDmsSupport::GetInstance((iModelDmsSupport::SessionType)sessionType, userName, password, callBackurl, accessToken, datasource, maxReties, productId);
        }
    }

 BentleyApi::Dgn::IDmsSupport*   iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR callBackurl, Utf8StringCR accessToken, Utf8StringCR datasource, int maxReties, unsigned long productId);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
 IDmsSupport*    iModelDmsSupport::GetInstance(iModelDmsSupport::SessionType sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR callBackurl, Utf8StringCR accessToken, Utf8StringCR datasource, int maxReties, unsigned long productId)
    {
    if (sessionType == SessionType::AzureBlobStorage)
        return new AzureBlobStorageHelper();

    if (sessionType == SessionType::PWShare)
         return new DmsHelper(callBackurl, accessToken, maxReties);

    if (sessionType == SessionType::PWDIDMS)
         return new DmsHelper(callBackurl, accessToken, maxReties, PWREPOSITORYTYPE, datasource);

    // default - PWDI session
    DmsSession* session = nullptr;
    if (!accessToken.empty())
        session = new SamlTokenSession(accessToken, productId, sessionType, datasource);
    else
        session = new UserCredentialsSession(userName, password, sessionType, datasource);

    return new PWWorkspaceHelper(*session);
    };
