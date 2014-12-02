#define UNICODE
#include <Windows.h>
#include <objbase.h>
#include "..\DgnCore\DgnPlatformInternal.h"
#include <DgnPlatform\DgnCore\IntegrationManager.h>

#include <windows.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 1638
typedef DgnDocumentManager*                 (__cdecl *InitializeDocumentManagerFP)             (StatusInt & initStatus);
typedef IntegratedRepositoryManager*        (__cdecl *InitializeRepositoryManagerFP)           (StatusInt & initStatus);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

OpenRepositoryStatus IntegratedRepositoryManager::_OpenSession 
    (
    WCharP repositoryName, 
    long respositoryNameLength, 
    WCharCP userName, 
    WCharCP password, 
    bool useSingleSignOn, 
    bool allowDialogPrompt, 
    unsigned long parentWindow
    )
    {
    return OPENREPOSITORY_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

void IntegratedRepositoryManager::_CloseSession (WCharCP repositoryName)
    {
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    06/11
+---------------+---------------+---------------+---------------+---------------+------*/
OpenRepositoryStatus IntegratedRepositoryManager::_ConnectToExistingSession () 
    {
    return OPENREPOSITORY_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

LoadIntegrationStatus IntegratedRepositoryManager::_Initialize (IntegratedApplicationType const appType)
    {
    return LOADINTEGRATION_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

void IntegratedRepositoryManager::_Uninitialize ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

OpenRepositoryStatus IntegratedRepositoryManager::OpenSession
    (
    WCharP repositoryName, 
    long respositoryNameLength, 
    WCharCP userName, 
    WCharCP password, 
    bool useSingleSignOn, 
    bool allowDialogPrompt, 
    unsigned long parentWindow
    )
    {
    return _OpenSession (repositoryName, respositoryNameLength, userName, password, useSingleSignOn, allowDialogPrompt, parentWindow);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    06/11
+---------------+---------------+---------------+---------------+---------------+------*/
OpenRepositoryStatus IntegratedRepositoryManager::ConnectToExistingSession () 
    {
    return _ConnectToExistingSession();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

void IntegratedRepositoryManager::CloseSession (WCharCP repositoryName)
    {
    _CloseSession (repositoryName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

LoadIntegrationStatus IntegratedRepositoryManager::Initialize (IntegratedApplicationType const appType)
    {
    return _Initialize (appType);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

void IntegratedRepositoryManager::Uninitialize ()
    {
    _Uninitialize ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

IntegratedRepositoryManager::IntegratedRepositoryManager ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

IntegratedRepositoryManager::~IntegratedRepositoryManager ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnPlatformIntegration::SetString (StringId id, WCharCP newStr, bool storeEmptyString) const
    {
    WCharP& str = m_strings[id];

    if (str == newStr)
        return;

    if (str != NULL)
        {
        memutil_free (str);
        str = NULL;
        }

    if (newStr != NULL && (storeEmptyString || *newStr != '\0'))
        str = memutil_wstrdup (newStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP       DgnPlatformIntegration::GetString (StringId id) const
    {
    wchar_t const* p = m_strings[id];
    return p? p: L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    04/11
+---------------+---------------+---------------+---------------+---------------+------*/
LoadIntegrationStatus DgnPlatformIntegration::Initialize(IntegratedApplicationType const appType)
    {
    if (NULL != m_repositoryManager)
        return LOADINTEGRATION_STATUS_Success;

    WString path = GetPath();
    if (0 == *(path.c_str()))
        return LOADINTEGRATION_STATUS_MalformedRegistryKey;

    HINSTANCE integrationLib = LoadLibraryEx (path.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (0 == integrationLib)
        return LOADINTEGRATION_STATUS_LibraryNotFound;

    InitializeRepositoryManagerFP InitRepositoryFunc = (InitializeRepositoryManagerFP)GetProcAddress (integrationLib, "InitializeRepositoryManager");
    int initStatus= ERROR;
    if (NULL != InitRepositoryFunc)
        m_repositoryManager = (*InitRepositoryFunc) (initStatus);
    if (NULL == m_repositoryManager)
        return LOADINTEGRATION_STATUS_RequestedClassNotFound;

    initStatus = m_repositoryManager->Initialize(appType);
    if (LOADINTEGRATION_STATUS_Success != initStatus)
        return LOADINTEGRATION_STATUS_RequestedClassNotFound;
    
    InitializeDocumentManagerFP InitDocManFunc = (InitializeDocumentManagerFP)GetProcAddress (integrationLib, "InitializeDocumentManager");
    initStatus= ERROR;
    
    if (NULL != InitDocManFunc)
        m_integratedDocumentManager = (*InitDocManFunc) (initStatus);
    if (NULL == m_integratedDocumentManager)
        {
        m_repositoryManager->Uninitialize();
        return LOADINTEGRATION_STATUS_RequestedClassNotFound;
        }
    return LOADINTEGRATION_STATUS_Success;
    }                                    

WString DgnPlatformIntegration::GetIntegrationKey() const {return GetString (STRINGID_IntegrationKey);}
WString DgnPlatformIntegration::GetProductName() const {return GetString (STRINGID_ProductName);}
WString DgnPlatformIntegration::GetVersionString() const {return GetString(STRINGID_Version);}
WString DgnPlatformIntegration::GetPath() const {return GetString(STRINGID_Path);}
DgnPlatform::DgnDocumentManager* DgnPlatformIntegration::GetDocumentManager() const {return m_integratedDocumentManager;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformIntegration::CloseSession (WCharCP repositoryName)
    {
    if (NULL != m_repositoryManager)
        m_repositoryManager->CloseSession (repositoryName);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    04/11
+---------------+---------------+---------------+---------------+---------------+------*/
OpenRepositoryStatus   DgnPlatformIntegration::OpenSession 
(
WCharP repositoryName,
long repositoryNameLength,
WCharCP userName,
WCharCP password,
bool useSingleSignOn,
bool allowDialogPrompt,
unsigned long parentWindow)
    {
    if (NULL == m_repositoryManager)
        return OPENREPOSITORY_STATUS_AccessDenied;
    
    return m_repositoryManager->OpenSession (repositoryName, repositoryNameLength, userName, password, useSingleSignOn, allowDialogPrompt, parentWindow);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    06/11
+---------------+---------------+---------------+---------------+---------------+------*/
OpenRepositoryStatus    DgnPlatformIntegration::ConnectToExistingSession ()
    {
    if (NULL == m_repositoryManager)
        return OPENREPOSITORY_STATUS_AccessDenied;
    
    return m_repositoryManager->ConnectToExistingSession ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformIntegration::DgnPlatformIntegration (WCharCP integrationKey, WCharCP productName, WCharCP versionString, WCharCP path):
    m_integratedDocumentManager (NULL), m_repositoryManager(NULL) 
    {
    memset (m_strings, 0, sizeof m_strings);
    SetString (STRINGID_IntegrationKey, integrationKey);
    SetString (STRINGID_ProductName, productName);
    SetString (STRINGID_Version, versionString);
    SetString (STRINGID_Path, path);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformIntegration::~DgnPlatformIntegration()
    {
    if (NULL != m_repositoryManager)
        m_repositoryManager->Uninitialize();
    }

// IntegrationManager implementation

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformIntegrationList::Add (DgnPlatformIntegrationPtr integration)
    {
    m_dgnPlatformIntegrations.push_back(integration);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformIntegrationList::Clear ()
    {
    m_dgnPlatformIntegrations.clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformIntegrationPtr DgnPlatformIntegrationList::At (size_type n)
    {
    return m_dgnPlatformIntegrations[n];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformIntegrationList::Remove(DgnPlatformIntegrationPtr integration)
    {
    DgnPlatformIntegrationList::iterator found = std::find (m_dgnPlatformIntegrations.begin(), m_dgnPlatformIntegrations.end(), integration);
    if (found == m_dgnPlatformIntegrations.end())
        return false;
    m_dgnPlatformIntegrations.erase(found);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/11
+---------------+---------------+---------------+---------------+---------------+------*/

DgnPlatformIntegrationListPtr IntegrationManager::LocateIntegrations
    (
    LocateIntegrationStatus& status
    )
    {
    HKEY    myHKey;
    WCHAR   subkeyName[MAX_PATH] = {0};

    wsprintf (subkeyName, L"Software\\Bentley\\Integrations\\%ls", L"DgnPlatform3");
    status = LOCATEINTEGRATION_STATUS_NoneRegistered;

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, subkeyName, 0, KEY_READ, &myHKey))
        return NULL;
                                                   
    
    WCHAR    achKey[MAX_KEY_LENGTH] = {0};   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    WCHAR    achClass[MAX_PATH] = {0};  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
 
    if (ERROR_SUCCESS != RegQueryInfoKey (myHKey, achClass, &cchClassName, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass,
                                NULL, NULL,  NULL, &cbSecurityDescriptor, &ftLastWriteTime) || 1 > cSubKeys)
        {
        RegCloseKey (myHKey);
        return NULL;
        }

    DgnPlatformIntegrationListP integrationList = new DgnPlatformIntegrationList ();

    DWORD i; 
    for (i=0; i<cSubKeys; i++)
        {
        cbName = MAX_KEY_LENGTH;
        if (ERROR_SUCCESS != RegEnumKeyEx (myHKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime))
            continue;
        HKEY subHKey;
        if (ERROR_SUCCESS != RegOpenKeyEx (myHKey, achKey, 0, KEY_READ, &subHKey))
            continue;
        WCHAR programName[MAX_PATH] = {0};
        DWORD programNameSize = MAX_PATH;
        RegQueryValueEx (subHKey, L"ProgramName", NULL, NULL, reinterpret_cast<LPBYTE>(programName), &programNameSize);
        

        WCHAR versionStr[MAX_PATH] = {0};
        DWORD versionSize = MAX_PATH;
        RegQueryValueEx (subHKey, L"Version", NULL, NULL, reinterpret_cast<LPBYTE>(&versionStr), &versionSize);

        WCHAR pathStr[MAX_PATH] = {0};
        DWORD pathSize = MAX_PATH;
        RegQueryValueEx (subHKey, L"Path", NULL, NULL, reinterpret_cast<LPBYTE>(&pathStr), &pathSize);

        RegCloseKey (subHKey);

        DgnPlatformIntegrationP integration = new DgnPlatformIntegration (achKey, programName, versionStr, pathStr);
        integrationList->Add (integration);
        }

    RegCloseKey (myHKey);
    status = LOCATEINTEGRATION_STATUS_Found;

    return integrationList;
    }
   
// DgnPlatformIntegrationManager
