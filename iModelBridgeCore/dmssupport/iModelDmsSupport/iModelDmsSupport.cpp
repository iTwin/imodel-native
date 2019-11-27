/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelDmsSupport/DmsSession.h>
#include "PWWorkspaceHelper.h"
#include "AzureBlobStorageHelper.h"
#include "PWShareHelper.h"

USING_NAMESPACE_BENTLEY_DGN
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
    {

    IMODEL_DMSSUPPORT_EXPORT IDmsSupport*    iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR callBackurl, Utf8StringCR accessToken)
        {
        return iModelDmsSupport::GetInstance((iModelDmsSupport::SessionType)sessionType, userName, password, callBackurl, accessToken);
        }
    }

 BentleyApi::Dgn::IDmsSupport*   iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR callBackurl, Utf8StringCR accessToken);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
 IDmsSupport*    iModelDmsSupport::GetInstance(iModelDmsSupport::SessionType sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR callBackurl, Utf8StringCR accessToken)
     {
     DmsSession session(userName, password, sessionType);

     if (sessionType == SessionType::AzureBlobStorage)
         return new AzureBlobStorageHelper();

     if (sessionType == SessionType::PWShare)
         return new PWShareHelper(callBackurl, accessToken);

     return new PWWorkspaceHelper(session);
     };
