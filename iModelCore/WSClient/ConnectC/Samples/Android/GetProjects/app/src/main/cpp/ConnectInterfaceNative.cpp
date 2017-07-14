//
// Created by Robert.Priest on 2/16/2017.
//

#include <android/log.h>
#include "ConnectInterfaceNative.h"

#define LOG_TAG "ConnectInterface"

#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_WARN(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOG_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace std;
USING_NAMESPACE_BENTLEY

#define LOGAPPNAME "ConnectInterface"

static std::wstring s_appDir;
static std::wstring s_tempDir;
static std::wstring s_localStateDir;
static std::wstring s_deviceId;
static std::wstring s_assetsDir;
static std::wstring s_externalStorageDir;
static void* s_securityStoreValue;

class ConnectInitializationInfo
{

public:
    static const std::wstring GetUserName() {return L"cwsccDEV_pmadm1@mailinator.com";}
    static const std::wstring GetPassword() {return L"cwsccpmadm1";}
    static const std::wstring GetApplicationProductId() {return L"2223";}  // Using Navigator Mobile - need to get our own id.
    static const std::wstring GetApplicationName() {return L"Bentley-Android-Test";}
    static const std::wstring GetApplicationVersion() {return L"1.0.0.0";}
    static const std::wstring GetApplicationGuid() {return L"TestAppGUID";}
    static const void         SetDeviceId(std::wstring deviceId) {s_deviceId = deviceId;}
    static const std::wstring GetDeviceId() {return s_deviceId;}
    static const void         SetExternalStorageDir(std::wstring externalStorageDir) {s_externalStorageDir = externalStorageDir;}
    static const std::wstring GetExternalStorageDir() {return s_externalStorageDir;}
    static const void         SetTempDir(std::wstring tempDir) {s_tempDir = tempDir;}
    static const std::wstring GetTempDir() {return s_tempDir;} //{return L"/data/data/com.bentley.loadprojects/cache";} //just going to hardcode for now.
    static const void         SetAssetsDir(std::wstring assetsDir) {s_assetsDir = assetsDir;}
    static const std::wstring GetAssetsDir() {return s_assetsDir;} //{return L"/data/data/com.bentley.loadprojects/Assets";} //just going to hardcode for now.
    static const std::wstring GetProxyUrl() {return L"http://10.224.17.72:8888";} //just going to hardcode for now.
    static const std::wstring GetProxyUsername() {return L"1";} //just going to hardcode for now.
    static const std::wstring GetProxyPassword() {return L"1";} //just going to hardcode for now.
    static const void SetSecurityStoreValue(void* securityStoreValue) {s_securityStoreValue = securityStoreValue;}
    static void*  GetSecurityStoreValue() {return s_securityStoreValue;}
};

void ConnectInterfaceNative::GetAPIHandle() {
    if (m_apiHandle != nullptr)
        return;

    //initialize the api handle
    m_apiHandle = ConnectWebServicesClientC_InitializeApiWithCredentials (
            ConnectInitializationInfo::GetUserName().c_str(),
            ConnectInitializationInfo::GetPassword().c_str(),
            ConnectInitializationInfo::GetTempDir().c_str(),
            ConnectInitializationInfo::GetAssetsDir().c_str(),
            ConnectInitializationInfo::GetApplicationName().c_str(),
            ConnectInitializationInfo::GetApplicationVersion().c_str(),
            ConnectInitializationInfo::GetApplicationGuid().c_str(),
            ConnectInitializationInfo::GetApplicationProductId().c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            ConnectInitializationInfo::GetSecurityStoreValue()
    );

    if (m_apiHandle == nullptr){
        LOG_ERROR("Could not initialize api with creds. API is null");
        return;
    }
}

ConnectInterfaceNative::ConnectInterfaceNative() {
    GetAPIHandle();
}

ConnectInterfaceNative::~ConnectInterfaceNative() {
    if (m_apiHandle)
        ConnectWebServicesClientC_FreeApi(m_apiHandle);
}

std::string ConnectInterfaceNative::GetConnectUserName() {
    if (m_apiHandle == nullptr)
        return "";

    CWSCCDATABUFHANDLE userInfo;
    CallStatus  status = ConnectWebServicesClientC_GetIMSUserInfo(m_apiHandle, &userInfo);
    if (status != SUCCESS){
        LOG_ERROR("Could not Get User Data from ConnectWebServicesClientC_GetIMSUserInfo api.");
    }

    wchar_t stringBuf[4096];
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_apiHandle, userInfo, 1, 0, 4096, stringBuf);
    Utf8String utf8String;
    BeStringUtilities::WCharToUtf8(utf8String, stringBuf);
    return utf8String.c_str();
}

int ConnectInterfaceNative::GetProjects(std::map<wstring, wstring>* idToNameMap) {
    CWSCCDATABUFHANDLE projects;
    CallStatus  status = ConnectWebServicesClientC_ReadProject_V4List(m_apiHandle, &projects);
    if (status != SUCCESS)
        return 0;

    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(projects);

    for (int i=0; i<bufferCount; i++) {
        //TODO: It would be ideal if we could get the InstanceId here, since that is what is used everywhere to retrieve objects.
        //TODO: However, the generated code does not include a way to access that, nor does it store it in the buffer anywhere.
        //TODO: So we will have to address that later. For now I will put the "Name" where I expect the InstanceId to be. But that is just for a stub
        wchar_t id[4096];
        status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_apiHandle, projects, PROJECT_V4_BUFF_OBJECTID, i, 4096, id);

        wchar_t name[4096];
        status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_apiHandle, projects, PROJECT_V4_BUFF_NAME, i, 4096, name);
        if (status == SUCCESS) {
            idToNameMap->insert(std::make_pair (id,name));
        }
    }

    status = ConnectWebServicesClientC_DataBufferFree(m_apiHandle, projects);
    return bufferCount;
}

int ConnectInterfaceNative::InsertPropertyIntoMap(CWSCCDATABUFHANDLE project, int propertyId, std::map<int, wstring> *map) {
    return InsertPropertyIntoMap(project, propertyId, 0, map);
}

int ConnectInterfaceNative::InsertPropertyIntoMap(CWSCCDATABUFHANDLE project, int propertyId, int mapIndex, std::map<int, wstring> *map) {
    switch( propertyId)
        {
            case PROJECT_V4_BUFF_STATUS:
            case PROJECT_V4_BUFF_INDUSTRY:
            case PROJECT_V4_BUFF_ASSETTYPE:
                {
                int val = 0;
                int status = ConnectWebServicesClientC_DataBufferGetIntProperty(m_apiHandle, project, propertyId, mapIndex, &val);
                wchar_t stringBuf[4096];
                memset(stringBuf, 0, 4096);
                BeStringUtilities::Snwprintf(stringBuf, 4096, L"%d", val);
                map->insert(std::make_pair ((int)propertyId, stringBuf));
                break;
                }
            case PROJECT_V4_BUFF_LATITUDE:
            case PROJECT_V4_BUFF_LONGITUDE:
                {
                double val;
                int status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(m_apiHandle, project, propertyId, mapIndex, &val);
                wchar_t stringBuf[4096];
                memset(stringBuf, 0, 4096);
                BeStringUtilities::Snwprintf(stringBuf, 4096, L"%0.7G", val);
                map->insert(std::make_pair ((int)propertyId, stringBuf));
                break;
                }
            case PROJECT_V4_BUFF_LOCATIONISUSINGLATLONG:
            case PROJECT_V4_BUFF_ISRBACENABLED:
                {
                bool val = false;
                int status = ConnectWebServicesClientC_DataBufferGetBooleanProperty(m_apiHandle, project, propertyId, mapIndex, &val);
                wchar_t stringBuf[4096];
                memset(stringBuf, 0, 4096);
                if (val)
                    BeStringUtilities::Snwprintf(stringBuf, 4096, L"%ls", L"true");
                else
                    BeStringUtilities::Snwprintf(stringBuf, 4096, L"%ls", L"false");
                map->insert(std::make_pair ((int)propertyId, stringBuf));
                break;
                }
            case PROJECT_V4_BUFF_REGISTEREDDATE:
            case PROJECT_V4_BUFF_LASTMODIFIED:
                {
                wchar_t stringBuf[4096];
                memset(stringBuf, 0, 4096);
                int status =  ConnectWebServicesClientC_DataBufferGetDatetimeProperty(m_apiHandle, project, propertyId, mapIndex, 4096, stringBuf);
                map->insert(std::make_pair ((int)propertyId, stringBuf));
                break;
                }
            default:
                {
                wchar_t stringBuf[4096];
                memset(stringBuf, 0, 4096);
                int status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_apiHandle, project, propertyId, mapIndex, 4096, stringBuf);
                map->insert(std::make_pair ((int)propertyId, stringBuf));
                break;
                }
        }
    wchar_t stringBuf[4096];
    memset(stringBuf, 0, 4096);
    int status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_apiHandle, project, propertyId, mapIndex, 4096, stringBuf);
    map->insert(std::make_pair ((int)propertyId, stringBuf));
    return status;
}

int ConnectInterfaceNative::GetProjectPropertyMapV4(wstring projectId, std::map<int, wstring>* map) {
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(m_apiHandle, projectId.c_str(), &project);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);

    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_NUMBER, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_NAME, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_REGISTEREDDATE, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_LASTMODIFIED, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_TIMEZONELOCATION, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_STATUS, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_ULTIMATEREFID, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_ISRBACENABLED, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_INDUSTRY, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_ASSETTYPE, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_LOCATION, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_LATITUDE, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_LONGITUDE, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_LOCATIONISUSINGLATLONG, map);
    InsertPropertyIntoMap(project, PROJECT_V4_BUFF_DATA_LOCATION_GUID, map);

    status = ConnectWebServicesClientC_DataBufferFree(m_apiHandle, project);
    return bufferCount;
}

void ConnectInterfaceNative::Initialize(
        std::wstring appDir,
        std::wstring tempDir,
        std::wstring externalStorageDir,
        std::wstring deviceId,
        void* secStore) {

    //Init logging
    NativeLogging::LoggingConfig::SetMaxMessageSize(10000);
    NativeLogging::LoggingConfig::SetOption (CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_TRACE);
    NativeLogging::LoggingConfig::ActivateProvider  (NativeLogging::CONSOLE_LOGGING_PROVIDER );
    NativeLogging::LoggingConfig::SetSeverity( L"", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("Bentley.Http", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("Bentley.Tasks", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("WSCache", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("WSClient", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("ConnectC", BentleyApi::NativeLogging::LOG_TRACE);
    
    NativeLogging::LoggingManager::GetLogger("ConnectC")->messagev(BentleyApi::NativeLogging::LOG_DEBUG, "Initializing ConnectInterface...");

    std::wstring assetsDir = L"/sdcard/Android/data/com.bentley.loadprojects/Assets";
    ConnectInitializationInfo::SetAssetsDir(assetsDir);

    ConnectInitializationInfo::SetTempDir(tempDir);
    ConnectInitializationInfo::SetExternalStorageDir(externalStorageDir);
    ConnectInitializationInfo::SetDeviceId(deviceId);
    ConnectInitializationInfo::SetSecurityStoreValue(secStore);
}

