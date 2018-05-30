/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/DmsSession.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelDmsSupport/DmsSession.h>
#include <ProjectWise_InternalSDK/Include/aadmsapi.h>
#include <ProjectWise_InternalSDK/Include/aaodsapi.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DmsSession::Initialize(BeFileNameCR pwBinaryPath)
    {
    if (NULL != m_activeDataSource)
        return true;

    SetPWBinaryPath(pwBinaryPath);

    if (!aaApi_Initialize(0))
        {
        LOG.fatalv("Unable to initialize projectwise api");
        return false;
        }

    if (!aaOApi_Initialize())
        {
        LOG.fatalv("Unable to initialize projectwise oapi");
        return false;
        }

    LOG.tracev("PW Initialized successfully.");
    LOG.tracev("Logging into data source. %s with user %s", m_dataSource.c_str(), m_userName.c_str());
    WString userName(m_userName.c_str(), true);
    WString password(m_password.c_str(), true);
    WString dataSource(m_dataSource.c_str(), true);
    if (!aaApi_Login(NULL, dataSource.c_str(), userName.c_str(), password.c_str(), NULL))
        {
        LOG.error("Unable to login to projectwise data source");
        return false;
        }

    aaOApi_Initialize();
    aaOApi_InitializeSession();
//    aaOApi_LoadAllClasses(NULL);

    LOG.tracev("Logged into data source successfully.");

    HDSOURCE* dataSources = NULL;
    ULONG dataSourceCount = 0;
    if (aaApi_GetDatasourceHandlesByName(dataSource.c_str(), &dataSources, &dataSourceCount))
        {
        m_activeDataSource = dataSources[0];
        aaApi_Free(dataSources);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DmsSession::UnInitialize()
    {
    aaOApi_UninitializeSession();
    bool status = aaApi_LogoutByHandle(m_activeDataSource) ? true : false;
    m_activeDataSource = NULL;
    aaApi_Uninitialize();
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DmsSession::DmsSession(Utf8StringCR userName, Utf8StringCR password, Utf8StringCR dataSource, iModelDmsSupport::SessionType sessionType)
    :m_userName(userName), m_activeDataSource(NULL), m_password(password), m_dataSource(dataSource),m_sessionType(sessionType)
    {

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            DmsSession::SetApplicationResourcePath(BeFileNameCR applicationResourcePath)
    {
    m_applicationResourcePath = applicationResourcePath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            DmsSession::SetPWBinaryPath(BeFileNameCR pwBinaryPath)
    {
    m_pwBinaryPath = pwBinaryPath;
    LOG.tracev("Setting up  projectwise dll search path to %S", pwBinaryPath.c_str());
    SetDllDirectoryW(m_pwBinaryPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR    DmsSession::GetApplicationResourcePath() const
    {
    return m_applicationResourcePath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelDmsSupport::SessionType     DmsSession::GetSessionType() const
    {
    return m_sessionType;
    }
