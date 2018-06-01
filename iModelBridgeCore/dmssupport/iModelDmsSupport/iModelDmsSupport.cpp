/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/iModelDmsSupport.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelDmsSupport/DmsSession.h>
#include "PWWorkspaceHelper.h"

USING_NAMESPACE_BENTLEY_DGN
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
    {

    IMODEL_DMSSUPPORT_EXPORT IDmsSupport*    iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password)
        {
        return iModelDmsSupport::GetInstance((iModelDmsSupport::SessionType)sessionType, userName, password);
        }
    }

 BentleyApi::Dgn::IDmsSupport*   iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IDmsSupport*    iModelDmsSupport::GetInstance(iModelDmsSupport::SessionType sessionType, Utf8StringCR userName, Utf8StringCR password)
    {
    if (sessionType != SessionType::PWDI)
        return NULL;
    
    DmsSession session(userName, password, sessionType);
    
    return new PWWorkspaceHelper(session);
    }
