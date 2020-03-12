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

    AddDNServer();
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
    DeleteDNServer();
    m_dnsServerUrl.clear();
    m_dnsServerName.clear();
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
DmsSession::DmsSession(iModelDmsSupport::SessionType sessionType, Utf8StringCR dnsServerUrl)
    :m_activeDataSource(NULL), m_sessionType(sessionType)
    {
    m_dnsServerUrl = dnsServerUrl;
    m_dnsServerName = Utf8String();
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
SamlTokenSession::SamlTokenSession(Utf8String accessToken, unsigned long productId, iModelDmsSupport::SessionType sessionType, Utf8StringCR dnsServerUrl)
    :DmsSession(sessionType, dnsServerUrl), m_accessToken( accessToken), m_productId(productId)
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
UserCredentialsSession::UserCredentialsSession(Utf8String userName, Utf8String password, iModelDmsSupport::SessionType sessionType, Utf8StringCR dnsServerUrl)
    :DmsSession(sessionType, dnsServerUrl), m_userName(userName), m_password(password)
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
void DmsSession::AddDNServer()
    {
    //Get DNS server URL
    bvector<Utf8String> urlParts;
    BeStringUtilities::Split(m_dnsServerUrl.c_str(), "/", urlParts);
    if (urlParts.size() < 2)
        {
        m_dnsServerUrl = Utf8String();
        LOG.error("Unable to get DNS server URL");
        return;
        }
    m_dnsServerUrl = Utf8String(urlParts[1].c_str());

    if (IsDnsServerPresentInRegistry())
        {
        LOG.tracev("DNS server: %s is already present in the registry", m_dnsServerUrl.c_str());
        m_dnsServerUrl = Utf8String();
        return;
        }

    LOG.tracev("DNS server: %s is not present in the registry", m_dnsServerUrl.c_str());
    m_dnsServerName = BeSQLite::BeGuid(true).ToString();
    //Add DNS Server
    if (RunCommandForDNSServer(true))
        LOG.tracev("Successfully added DNS server");
    else
        LOG.error("Unable to run add DNS server command");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DmsSession::DeleteDNServer()
    {
    if (!Utf8String::IsNullOrEmpty(m_dnsServerName.c_str()))
        {
        //Delete DNS Server
        if (RunCommandForDNSServer(false))
            LOG.tracev("Successfully deleted DNS server");
        else
            LOG.error("Unable to run delete DNS server command");
        }
    else
        {
        LOG.error("Unable to delete DNS server as server url is empty");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool DmsSession::RunCommandForDNSServer(bool addDNSServer)
    {
    BeFileName pwnetworkconfigcmdPath(L"C:\\Program Files\\Bentley\\ProjectWise\\bin\\pwnetworkconfigcmd.exe");
    if (pwnetworkconfigcmdPath.DoesPathExist())
        {
        Utf8String cmd;
        if (addDNSServer)
            {
            cmd = Utf8PrintfString("start %s --add-dns-server %s %s", Utf8String(pwnetworkconfigcmdPath.GetName()).c_str(), m_dnsServerName.c_str(), m_dnsServerUrl.c_str());
            LOG.tracev("Add DNS server command: %s", cmd.c_str());
            }
        else
            {
            cmd = Utf8PrintfString("start %s --delete-dns-server %s", Utf8String(pwnetworkconfigcmdPath.GetName()).c_str(), m_dnsServerName.c_str());
            LOG.tracev("Delete DNS server command: %s", cmd.c_str());
            }

        cmd.ReplaceAll("Program Files", "\"Program Files\"");
        if (system(cmd.c_str()) == 0)
            return true;
        }
    else
        {
        LOG.error("PW Network Config exe not found");
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                  03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool DmsSession::IsDnsServerPresentInRegistry()
    {
    bmap<HKEY, WString> regKeysForPWDnsServer;
    regKeysForPWDnsServer.Insert(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Bentley\\ProjectWise\\10.00\\NetworkConfig\\Dns\\Servers");
    regKeysForPWDnsServer.Insert(HKEY_CURRENT_USER, L"SOFTWARE\\Bentley\\ProjectWise\\10.00\\NetworkConfig\\Dns\\Servers");

    for (bmap<HKEY, WString>::iterator itor = regKeysForPWDnsServer.begin(); itor != regKeysForPWDnsServer.end(); ++itor)
        {
        HKEY dnsServersKey;
        long result = RegOpenKeyExW(itor->first, itor->second.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &dnsServersKey);
        if (result != ERROR_SUCCESS)
            {
            LOG.errorv("Unable to open reg key: %s & error code: %ld", Utf8String(itor->second).c_str(), result);
            continue;
            }

        WCHAR dnsServerName[257];
        DWORD dnsServerNameLen = _countof(dnsServerName);
        DWORD index = 0;
        while (RegEnumKeyExW(dnsServersKey, index, dnsServerName, &dnsServerNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
            HKEY subKey;
            if (RegOpenKeyExW(dnsServersKey, dnsServerName, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &subKey) != ERROR_SUCCESS)
                {
                LOG.errorv("Unable to open reg key: %s under %s error code: %ld", Utf8String(dnsServerName).c_str(), Utf8String(itor->second).c_str(), result);
                continue;
                }

            //Get the server url
            WStringCR valueName = L"Server";
            WCHAR szBuffer[2049];
            DWORD dwBufferSize = sizeof(szBuffer);
            result = RegQueryValueExW(subKey, valueName.c_str(), NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize);
            if (result == ERROR_SUCCESS)
                {
                if (m_dnsServerUrl.EqualsI(Utf8String(szBuffer)))
                    {
                    RegCloseKey(subKey);
                    return true;
                    }
                }
            else
                {
                LOG.errorv("Unable to read server value for key: %s under %s error code: %ld", Utf8String(dnsServerName).c_str(), Utf8String(itor->second).c_str(), result);
                }

            RegCloseKey(subKey);
            dnsServerNameLen = _countof(dnsServerName);
            ++index;
            }

        RegCloseKey(dnsServersKey);
        }
    return false;
    }