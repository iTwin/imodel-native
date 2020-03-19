/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelDmsSupport/DmsSession.h>
#include <ProjectWise_InternalSDK/Include/aadmsapi.h>
#include <ProjectWise_InternalSDK/Include/aaodsapi.h>
#include <Bentley/Desktop/FileSystem.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DmsSession::InitPwLibraries(BeFileNameCR pwBinaryPath)
    {
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
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DmsSession::Initialize()
    {
    if (NULL != m_activeDataSource)
        return true;

    LOG.tracev("PW Initialized successfully.");

    AddServer();
    bool loggedIn = Login();
    if (!loggedIn)
        {
        LOG.error("Unable to login to projectwise data source");
        return false;
        }

    //aaOApi_Initialize();
    aaOApi_InitializeSession();
//    aaOApi_LoadAllClasses(NULL);

    LOG.tracev("Logged into data source successfully.");

    HDSOURCE* dataSources = NULL;
    ULONG dataSourceCount = 0;
    WString dataSource(m_dataSource.c_str(), true);
    if (aaApi_GetDatasourceHandlesByName(dataSource.c_str(), &dataSources, &dataSourceCount))
        {
        LOG.trace("aaApi_GetDatasourceHandlesByName SUCCEEDED");
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
    DeleteServer();
    m_serverUrl.clear();
    m_serverName.clear();
    aaApi_Uninitialize();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName      DmsSession::GetDefaultWorkspacePath(bool isv8i)
    {
    BeFileName applicationResourcePath = Desktop::FileSystem::GetExecutableDir();
    if (isv8i)
        applicationResourcePath.AppendToPath(L"Dgnv8\\v8iConfig");
    else
        applicationResourcePath.AppendToPath(L"Dgnv8\\");
    return applicationResourcePath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName      DmsSession::GetDefaultConfigPath(bool isv8i) const
    {
    BeFileName applicationResourcePath = Desktop::FileSystem::GetExecutableDir();
    if (isv8i)
        applicationResourcePath.AppendToPath(L"Dgnv8\\v8iConfig\\Config\\mslocal.cfg");
    else
        applicationResourcePath.AppendToPath(L"Dgnv8\\Config\\mslocal.cfg");
    return applicationResourcePath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DmsSession::DmsSession(iModelDmsSupport::SessionType sessionType, Utf8StringCR serverUrl)
    :m_activeDataSource(NULL), m_sessionType(sessionType)
    {
    m_serverUrl = serverUrl;
    m_serverName = Utf8String();
    dnsServerAdded = false;
    dsListingServerAdded = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
DmsSession::~DmsSession() 
    {
    };

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
BeFileName    DmsSession::GetApplicationResourcePath(bool isv8i) const
    {
    if (!m_applicationResourcePath.empty())
        return m_applicationResourcePath;

    return GetDefaultWorkspacePath(isv8i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelDmsSupport::SessionType     DmsSession::GetSessionType() const
    {
    return m_sessionType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DmsSession::SetDataSource(Utf8StringCR dataSource)
    {
    if (NULL != m_activeDataSource)
        {
        LOG.error("Setting up  a data source without cleaning up the previous session");
        return false;
        }

    m_dataSource = dataSource;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenSession::SamlTokenSession(Utf8String accessToken, unsigned long productId, iModelDmsSupport::SessionType sessionType, Utf8StringCR serverUrl)
    :DmsSession(sessionType, serverUrl), m_accessToken( accessToken), m_productId(productId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenSession::~SamlTokenSession() 
    {
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SamlTokenSession::Login()
    {
    LOG.tracev("Logging into data source %s with token %s", m_dataSource.c_str(), m_accessToken.c_str());
    WString ds(m_dataSource.c_str(), true);
    WString token(m_accessToken.c_str(), true);

    ULONG productIDs[] = { m_productId, 0 };

    bool loggedIn = aaApi_LoginWithSecurityToken(ds.c_str(), token.c_str(), false, NULL, productIDs);
    if (!loggedIn)
        {
        int code = aaApi_GetLastErrorId();
        auto msg = aaApi_GetLastErrorMessage();
        auto dtl = aaApi_GetLastErrorDetail();
        LOG.errorv("Unable to login to PW with token. %d, %ls, %ls", code, msg, dtl);
        }
    return loggedIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserCredentialsSession::UserCredentialsSession(Utf8String userName, Utf8String password, iModelDmsSupport::SessionType sessionType, Utf8StringCR serverUrl)
    :DmsSession(sessionType, serverUrl), m_userName(userName), m_password(password)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserCredentialsSession::~UserCredentialsSession()
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserCredentialsSession::Login()
    {
    LOG.tracev("Logging into data source. %s with user %s", m_dataSource.c_str(), m_userName.c_str());
    WString userName(m_userName.c_str(), true);
    WString password(m_password.c_str(), true);
    WString ds(m_dataSource.c_str(), true);
    return aaApi_Login(NULL, ds.c_str(), userName.c_str(), password.c_str(), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DmsSession::AddServer()
    {
    //Get Server URL
    bvector<Utf8String> urlParts;
    BeStringUtilities::Split(m_serverUrl.c_str(), "/", urlParts);
    if (urlParts.size() < 2)
        {
        m_serverUrl = Utf8String();
        LOG.error("Unable to get DNS server URL");
        return;
        }
    m_serverUrl = Utf8String(urlParts[1].c_str());
    m_serverName = BeSQLite::BeGuid(true).ToString();

    //Add DNS Server
    if (!IsServerPresentInRegistry(true))
        dnsServerAdded = RunCommandForServer(true, true);

    //Add DS Listing Server
    if (!IsServerPresentInRegistry(false))
        dsListingServerAdded = RunCommandForServer(false, true);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DmsSession::DeleteServer()
    {
    //Delete DNS And DS Lisiting Server
    if (dnsServerAdded)
        RunCommandForServer(true, false);
    if (dsListingServerAdded)
        RunCommandForServer(false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool DmsSession::RunCommandForServer(bool dnsServer, bool addServer)
    {
    BeFileName pwnetworkconfigcmdPath(L"C:\\Program Files\\Bentley\\ProjectWise\\bin\\pwnetworkconfigcmd.exe");
    if (pwnetworkconfigcmdPath.DoesPathExist())
        {
        Utf8String cmd, server;
        if (dnsServer)
            server = "dns";
        else
            server = "dslisting";

        //Create Command
        if (addServer)
            {
            cmd = Utf8PrintfString("start %s --add-%s-server %s %s", Utf8String(pwnetworkconfigcmdPath.GetName()).c_str(), server.c_str(), m_serverName.c_str(), m_serverUrl.c_str());
            LOG.tracev("Add %s server command: %s", server.c_str(), cmd.c_str());
            }
        else
            {
            cmd = Utf8PrintfString("start %s --delete-%s-server %s", Utf8String(pwnetworkconfigcmdPath.GetName()).c_str(), server.c_str(), m_serverName.c_str());
            LOG.tracev("Delete %s server command: %s", server.c_str(), cmd.c_str());
            }

        cmd.ReplaceAll("Program Files", "\"Program Files\"");
        if (system(cmd.c_str()) == 0)
            {
            LOG.tracev("Command run successfully");
            return true;
            }
        }
    else
        {
        LOG.error("PW Network Config exe not found");
        }
    LOG.error("Command run failed");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool DmsSession::IsServerPresentInRegistry(bool dnsServer)
    {
    bmap<HKEY, WString> regKeysForPWServer;
    Utf8String server;
    if (dnsServer)
        {
        server = "dns";
        regKeysForPWServer.Insert(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Bentley\\ProjectWise\\10.00\\NetworkConfig\\Dns\\Servers");
        regKeysForPWServer.Insert(HKEY_CURRENT_USER, L"SOFTWARE\\Bentley\\ProjectWise\\10.00\\NetworkConfig\\Dns\\Servers");
        }
    else
        {
        server = "dslisting";
        regKeysForPWServer.Insert(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Bentley\\ProjectWise\\10.00\\NetworkConfig\\DsListing\\Servers");
        regKeysForPWServer.Insert(HKEY_CURRENT_USER, L"SOFTWARE\\Bentley\\ProjectWise\\10.00\\NetworkConfig\\DsListing\\Servers");
        }

    for (bmap<HKEY, WString>::iterator itor = regKeysForPWServer.begin(); itor != regKeysForPWServer.end(); ++itor)
        {
        HKEY dnsServersKey;
        long result = RegOpenKeyExW(itor->first, itor->second.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &dnsServersKey);
        if (result != ERROR_SUCCESS)
            {
            LOG.errorv("Unable to open reg key: %s & error code: %ld", Utf8String(itor->second).c_str(), result);
            continue;
            }

        WCHAR serverName[257];
        DWORD serverNameLen = _countof(serverName);
        DWORD index = 0;
        while (RegEnumKeyExW(dnsServersKey, index, serverName, &serverNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
            HKEY subKey;
            if (RegOpenKeyExW(dnsServersKey, serverName, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &subKey) != ERROR_SUCCESS)
                {
                LOG.errorv("Unable to open reg key: %s under %s error code: %ld", Utf8String(serverName).c_str(), Utf8String(itor->second).c_str(), result);
                continue;
                }

            //Get the server url
            WStringCR valueName = L"Server";
            WCHAR szBuffer[2049];
            DWORD dwBufferSize = sizeof(szBuffer);
            result = RegQueryValueExW(subKey, valueName.c_str(), NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize);
            if (result == ERROR_SUCCESS)
                {
                if (m_serverUrl.EqualsI(Utf8String(szBuffer)))
                    {
                    RegCloseKey(subKey);
                    LOG.tracev("%s server: %s is already present in the registry", server.c_str(), m_serverUrl.c_str());
                    return true;
                    }
                }
            else
                {
                LOG.errorv("Unable to read server value for key: %s under %s error code: %ld", Utf8String(serverName).c_str(), Utf8String(itor->second).c_str(), result);
                }

            RegCloseKey(subKey);
            serverNameLen = _countof(serverName);
            ++index;
            }

        RegCloseKey(dnsServersKey);
        }
    LOG.tracev("%s server: %s is not present in the registry", server.c_str(), m_serverUrl.c_str());
    return false;
    }