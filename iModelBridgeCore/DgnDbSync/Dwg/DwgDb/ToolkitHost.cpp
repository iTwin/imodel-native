/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/ToolkitHost.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

#ifdef DWGTOOLKIT_OpenDwg
static OdStaticRxObject<DwgToolkitHost> s_dwgToolkitHostInstance;
#elif DWGTOOLKIT_RealDwg
static DwgToolkitHost*                  s_dwgToolkitHostInstance = nullptr;
#endif
static bool                             s_toolkitInitialized = false;


#ifdef DWGTOOLKIT_OpenDwg
void DwgDbProgressMeter::start (const OdString& displayString) { if(nullptr!=m_appMeter) m_appMeter->_Start(WString(displayString.c_str())); }
#elif defined (DWGTOOLKIT_RealDwg)
void DwgDbProgressMeter::start (const ACHAR* displayString) { if(nullptr!=m_appMeter) m_appMeter->_Start(nullptr == displayString ? WString() : WString(displayString)); }
#endif
void DwgDbProgressMeter::stop () { if(nullptr!=m_appMeter) m_appMeter->_Stop(); }
void DwgDbProgressMeter::meterProgress () { if(nullptr!=m_appMeter) m_appMeter->_Progress(); }
void DwgDbProgressMeter::setLimit (int max) { if(nullptr!=m_appMeter) m_appMeter->_SetLimit(max); }


#if defined (DWGTOOLKIT_OpenDwg)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
OdDbHostAppProgressMeter::~OdDbHostAppProgressMeter ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
OdString    DwgToolkitHost::findFile(const OdString& filenameIn, OdDbBaseDatabase* dwg, FindFileHint hint)
    {
    if (nullptr != m_appHost)
        {
        WString   found;
        if (DwgDbStatus::Success == m_appHost->_FindFile(found, reinterpret_cast<WCharCP>(filenameIn.c_str()), static_cast<DwgDbDatabaseP>(dwg), static_cast<AcadFileType>(hint)))
            return  found.c_str();
        }

    return  OdString::kEmpty;
    }

void        DwgToolkitHost::warning (const OdString& message) { if (nullptr != m_appHost) m_appHost->_Alert(reinterpret_cast<WCharCP>(message.c_str())); }
void        DwgToolkitHost::warning (const char* warnVisGroup, OdWarning warningOb) { if (nullptr != m_appHost) m_appHost->_Alert(L"...implement warning!"); }
void        DwgToolkitHost::warning (const char* warnVisGroup, OdWarning warningOb, OdDbObjectId objectId) { if (nullptr != m_appHost) m_appHost->_Alert(L"...implement warning!"); }
void        DwgToolkitHost::warning (OdWarning code) { if (nullptr != m_appHost) m_appHost->_Alert(reinterpret_cast<WCharCP>(this->getErrorDescription(code).c_str())); }
OdString    DwgToolkitHost::getErrorDescription (unsigned int code) { return  OdString(L"implement getErrorDescriprion!!!"); }

OdDbHostAppProgressMeter*   DwgToolkitHost::newProgressMeter ()
    {
    if (nullptr != m_workingProgressMeter)
        return new DwgDbProgressMeter (m_workingProgressMeter->GetApplicationProgressMeter());
    else
        return new DwgDbProgressMeter ();
    }

#elif defined (DWGTOOLKIT_RealDwg)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Acad::ErrorStatus   DwgToolkitHost::findFile(ACHAR * fileOut, int nChars, const ACHAR* fileIn, AcDbDatabase* dwg, AcDbHostApplicationServices::FindFileHint hint)
    {
    if (nullptr != m_appHost)
        {
        WString     found;
        if (DwgDbStatus::Success == m_appHost->_FindFile(found, fileIn, dynamic_cast<DwgDbDatabaseP>(dwg), static_cast<AcadFileType>(hint)) &&
            static_cast<int>(found.length()) <= nChars)
            {
            wcsncpy (fileOut, found.c_str(), nChars);
            return  Acad::eOk;
            }
        }
    return  Acad::eFileNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgToolkitHost::fatalError (const ACHAR* format, ...)
    { 
    if (nullptr != m_appHost)
        {
        va_list  varArgs;

        va_start (varArgs, format);
        m_appHost->_FatalError (format, varArgs);
        va_end (varArgs);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgToolkitHost::notifyCorruptDrawingFoundOnOpen (AcDbObjectId id, Acad::ErrorStatus es)
    {
    if (nullptr != m_appHost)
        {
        WString     message;
        message.Sprintf (L"DWG file corrupted! ObjectHandle=%I64d [%ls]\n", static_cast<uint64_t>(id.handle()), acadErrorStatusText(es));
        m_appHost->_Alert (message.c_str());
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Acad::ErrorStatus   DwgToolkitHost::getLocalRootFolder (const wchar_t*& folder)
    {
    Acad::ErrorStatus status = Acad::eOk;
#ifdef _MSC_VER
    static wchar_t  s_moduleName[MAX_PATH] = L"\0"; //MDI SAFE
    if (0 == s_moduleName[0])
        {
        if (::GetModuleFileName (nullptr, s_moduleName, MAX_PATH) != 0)
            status = Acad::eRegistryAccessError;
        }
#endif
    folder = s_moduleName;
    return status;
    }

Acad::ErrorStatus   DwgToolkitHost::getRoamableRootFolder (const wchar_t*& folder) { return this->getLocalRootFolder(folder); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean      DwgToolkitHost::isRemoteFile (const ACHAR* localFile, ACHAR* url, size_t urlLen) const
    {
    auto entry = m_localToUrlMap.find (localFile);
    if (entry != m_localToUrlMap.end())
        {
        if (urlLen > 0)
            ::wcsncpy (url, entry->second.c_str(), urlLen);
        }
    return  false;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Acad::ErrorStatus   DwgToolkitHost::getRemoteFile (const ACHAR* url, ACHAR* local, size_t localLen, Adesk::Boolean ignoreCache) const
    {
    if (nullptr == url || nullptr == local)
        return  Acad::eInetNotAnURL;

    WString inFile(url), outFile;

    DwgDbStatus status = this->DownloadOrGetCachedFile (outFile, inFile, ignoreCache);

    if (DwgDbStatus::Success == status)
        {
        size_t  size2copy = MIN(outFile.size(), localLen);
        if (size2copy > 0)
            ::wcsncpy (local, outFile.c_str(), size2copy);
        // tell RealDWG about the success
        return Acad::eInetOk;
        }

    // tell RealDWG about the failure:
    return Acad::eInetFileGenericError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean      DwgToolkitHost::isURL (const wchar_t* url) const
    {
#ifdef _MSC_VER
    return ::PathIsURLW(url) ? Adesk::kTrue : Adesk::kFalse;
#else
    return  Adesk::kFalse;
#endif
    }

const ACHAR*    DwgToolkitHost::getEnv (const ACHAR* var) { return T_Super::getEnv(var); }
void            DwgToolkitHost::alert (const ACHAR* message) const { if(nullptr!=m_appHost) m_appHost->_Alert(message); }
void            DwgToolkitHost::displayChar (ACHAR c) const { if(nullptr!=m_appHost) m_appHost->_Message(static_cast<WCharCP>(&c), 1); }
void            DwgToolkitHost::displayString (const ACHAR* chars, int count) const { if(nullptr!=m_appHost) m_appHost->_Message(static_cast<WCharCP>(chars), count); }
Adesk::Boolean  DwgToolkitHost::readyToDisplayMessages () { return Adesk::kTrue; }
const ACHAR*    DwgToolkitHost::program () { return L"AcDb"; }
const ACHAR*    DwgToolkitHost::product () { return nullptr!=m_appHost ? m_appHost->_Product() : L"ObjectDBX"; }
const ProdIdCode DwgToolkitHost::prodcode () { return ProdIdCode::kProd_AcDb; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
#if VendorVersion <= 2016
LCID            DwgToolkitHost::getRegistryProductLCID ()
    {
    return nullptr != m_appHost ? m_appHost->_GetRegistryProductLCID() : 0x0409;
    }
#else
AcLocale        DwgToolkitHost::getProductLocale ()
    {
    if (nullptr == m_appHost)
        return  AcLocale (L"en", L"us");
    else
        return  AcLocale (m_appHost->_GetRegistryProductLCID());
    }
#endif  // VendorVersion

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
const ACHAR*    DwgToolkitHost::getMachineRegistryProductRootKey ()
    {
    const ACHAR*    regkey = nullptr == m_appHost ? nullptr : m_appHost->_GetRegistryProductRootKey (IDwgDbHost::RootRegistry::Machine);

    if (nullptr == regkey || 0 == regkey[0])
        regkey = T_Super::getMachineRegistryProductRootKey ();

    return regkey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
const ACHAR*    DwgToolkitHost::getUserRegistryProductRootKey ()
    {
    const ACHAR*    regkey = nullptr == m_appHost ? nullptr : m_appHost->_GetRegistryProductRootKey (IDwgDbHost::RootRegistry::User);

    if (nullptr == regkey || 0 == regkey[0])
        regkey = T_Super::getUserRegistryProductRootKey ();

    return  regkey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgToolkitHost::getPassword (const ACHAR* dwgName, PasswordOptions options, wchar_t* password, const size_t size)
    {
    if (nullptr != m_appHost)
        return m_appHost->_GetPassword(dwgName, static_cast<PasswordChoice>(options), password, size);
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t*  DwgToolkitHost::getAlternateFontName () const
    {
    if (nullptr != m_appHost)
        {
        m_alternateFontName.clear ();

        if (m_appHost->_GetAlternateFontName(m_alternateFontName))
            return m_alternateFontName.c_str();
        }
    return  nullptr;
    }

#ifdef _MSC_VER
Acad::ErrorStatus   DwgToolkitHost::getNewOleClientItem (COleClientItem*& pItem)
    {
    // WIP - support OLE
    return  Acad::eNotImplementedYet;
    }
Acad::ErrorStatus   DwgToolkitHost::serializeOleItem(COleClientItem* pItem, CArchive*)
    {
    // WIP - support OLE
    return  Acad::eNotImplementedYet;
    }
#endif

/*---------------------------------------------------------------------------------**//**
In cases when AcDb.dll needs to create a new progress meter, it calls this method; in 
other cases(most times) we have to explicitly set a global progress meter for RealDWG 
via setWorkingProgressMeter before we read a DWG file.
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
AcDbHostApplicationProgressMeter*   DwgToolkitHost::newProgressMeter ()
    {
    if (nullptr != m_workingProgressMeter)
        return new DwgDbProgressMeter (m_workingProgressMeter->GetApplicationProgressMeter());
    else
        return new DwgDbProgressMeter ();
    }

#endif  // DWGTOOLKIT_Open/RealDwg


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgToolkitHost::~DwgToolkitHost ()
    {
    m_localToUrlMap.clear ();
    m_alternateFontName.clear ();

    if (nullptr != m_workingProgressMeter)
        delete m_workingProgressMeter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgToolkitHost::Initialize ()
    {
    m_localToUrlMap.clear();
    m_alternateFontName.clear();
    m_toolkitRegistryRootKey = nullptr;
    m_progressPosition = 0;
    m_progressLimit = 100;
    m_appHost = nullptr;
    m_workingProgressMeter = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgToolkitHost::SetApplicationHost (IDwgDbHost& appHost)
    {
    // set application host in the toolkit host
    m_appHost = &appHost;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbProgressMeter*     DwgToolkitHost::NewWorkingProgressMeter (IDwgDbProgressMeter* appMeter)
    {
    // reset application progress meter
    if (nullptr != m_workingProgressMeter)
        delete m_workingProgressMeter;

    if (nullptr != appMeter)
        m_workingProgressMeter = new DwgDbProgressMeter (appMeter);
    else
        m_workingProgressMeter = nullptr;

    return  m_workingProgressMeter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgToolkitHost::DownloadOrGetCachedFile (WStringR local, WStringCR url, bool ignoreCache) const
    {
#ifdef _MSC_VER
    DWORD   winError = ERROR_FILE_NOT_FOUND;
    if (!ignoreCache)
        {
        // if we have previsously downloaded & cached this file, use it:
        if (this->FindCachedLocalFile(local, url))
            return  DwgDbStatus::Success;
            
        // look the file up from the OS's cache:
        DWORD   size = 0;
        if (::GetUrlCacheEntryInfo(url.c_str(), nullptr, &size))
            return DwgDbStatus::UrlCacheError;

        winError = ::GetLastError();
        if (winError == ERROR_INSUFFICIENT_BUFFER)
            {
            INTERNET_CACHE_ENTRY_INFO* cacheEntry = (INTERNET_CACHE_ENTRY_INFO*)::malloc(size);
            if (::GetUrlCacheEntryInfo(url.c_str(), cacheEntry, &size))
                {
                // cacheEntry->lpszLocalFileName might be a .htm file telling us that the file has been moved!
                bool isMoved = false;
                if (cacheEntry->dwHeaderInfoSize > 0 && WString(cacheEntry->lpHeaderInfo).Contains(L"Moved Permanently"))
                    isMoved = true;

                bool isFound = false;
                if (!isMoved && BeFileName::DoesPathExist(cacheEntry->lpszLocalFileName))
                    {
                    // found the cache file - return it:
                    local.assign (cacheEntry->lpszLocalFileName);
                    // but also save it to our list for future lookup:
                    m_localToUrlMap.Insert (local, url);
                    isFound = true;
                    }

                ::free(cacheEntry);
                if (isFound)
                    return DwgDbStatus::Success;

                winError = ERROR_FILE_NOT_FOUND;
                }
            else
                {
                winError = ::GetLastError();
                }
            }
        }

    if (winError == ERROR_FILE_NOT_FOUND)
        {
        // download and cache the URL file
        static DWORD    localSize = 2048;
        LPTSTR  localFile = static_cast<LPTSTR> (::calloc(1, localSize));

        HRESULT result = ::URLDownloadToCacheFile (nullptr, url.c_str(), localFile, localSize, 0, nullptr);

        if (SUCCEEDED(result))
            {
            // return the downloaded cache file:
            local.assign (localFile);
            // save the cache file for future lookup:
            m_localToUrlMap.Insert (local, url);
            }
        ::free (localFile);

        if (SUCCEEDED(result))
            return  DwgDbStatus::Success;
        }
#endif // _MSC_VER
    return DwgDbStatus::UrlCacheError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgToolkitHost::FindCachedLocalFile (WStringR cached, WStringCR url) const
    {
    auto found = std::find_if (m_localToUrlMap.begin(), m_localToUrlMap.end(), [&](bpair<WString,WString> const& entry){ return entry.second.EqualsI(url.c_str()); });
    if (found != m_localToUrlMap.end())
        {
        cached.assign (found->first.c_str());
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgToolkitHost::Warn (WStringCR message) const
    {
    if (nullptr != m_appHost && !message.empty())
        m_appHost->_Alert (message.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgToolkitHost& DwgToolkitHost::GetHost ()
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  s_dwgToolkitHostInstance;
#elif DWGTOOLKIT_RealDwg
    return  *s_dwgToolkitHostInstance;
#endif
    }

#if DWGTOOLKIT_RealDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static long     GetValidLocale (IDwgDbHost& appHost)
    {
    // default to English
    long        lcid = AcLocale(L"en", L"us");

#ifdef _MSC_VER
    wchar_t*    buffer = _wgetcwd (nullptr, 0);
    if (nullptr == buffer)
        return  lcid;

    WString     path(buffer);
    free (buffer);
    
    lcid = (long)appHost._GetRegistryProductLCID ();
    switch (lcid)
        {
        case 0x0804: path += L"\\zh-CN";     break;  // Simplified Chinese
        case 0x0404: path += L"\\zh-TW";     break;  // Chinese Traditional
        case 0x0405: path += L"\\cs-CZ";     break;  // Czech
        case 0x0407: path += L"\\de-DE";     break;  // German
        case 0x040A: path += L"\\es-ES";     break;  // Spanish
        case 0x040C: path += L"\\fr-FR";     break;  // French
        case 0x040E: path += L"\\hu-HU";     break;  // Hungarian
        case 0x0410: path += L"\\it-IT";     break;  // Italian
        case 0x0411: path += L"\\ja-JP";     break;  // Japanese
        case 0x0412: path += L"\\ko-KR";     break;  // Korean
        case 0x0415: path += L"\\pl-PL";     break;  // Polish
        case 0x0416: path += L"\\pt-BR";     break;  // Portuguese (Brazil)
        case 0x0419: path += L"\\ru-RU";     break;  // Russian
        default:
            lcid = 0x0409;
        }

    // if the locale sub-folder exists, assume it to be valid; otherwise default to English:
    if (0x0409 != lcid && -1 == _waccess(path.c_str(), 0))
        lcid = AcLocale(L"en", L"us");
#endif  // _MSC_VER

    return  lcid;
    }
#endif  // DWGTOOLKIT_RealDwg

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            IDwgDbHost::InitializeToolkit (IDwgDbHost& appHost)
    {
#ifdef DWGTOOLKIT_OpenDwg
    if (!s_toolkitInitialized)
        {
        // never want to recompute dimensions
        s_dwgToolkitHostInstance.setRecomputeDimBlocksRequired (false);

        odInitialize (&s_dwgToolkitHostInstance);

#if defined(_MSC_VER) && (DWGDB_ToolkitMajorRelease < 19)
        ::odrxInitWinNTCrypt ();
#endif
        // OpenDWG has no restrictions on OdDb classes to be sub-classed:
        RegisterDwgDbObjectExtensions (true);
        RegisterDwgDbObjectExtensions (false);

        s_dwgToolkitHostInstance.SetApplicationHost (appHost);
        s_toolkitInitialized = true;
        }

#elif DWGTOOLKIT_RealDwg

    if (!s_toolkitInitialized)
        {
        s_dwgToolkitHostInstance = new DwgToolkitHost ();

        Acad::ErrorStatus   status = acdbSetHostApplicationServices (s_dwgToolkitHostInstance);
        if (Acad::eOk != status)
            BeAssert (false && L"Failed setting up RealDWG host!!");

        // RealDWG has restrictions on some AcDb classes to be sub-classed - register these prior to adding the hostApp:
        RegisterDwgDbObjectExtensions (true);

        // set app host for the toolkit host - do it before acdbValidateSetup; otherwise RealDWG does not check for registries!
        s_dwgToolkitHostInstance->SetApplicationHost (appHost);

        long        lcid = GetValidLocale (appHost);
        status = acdbValidateSetup (lcid);
        if (Acad::eOk != status)
            BeAssert (false && L"Failed validating RealDWG locale!!!");

        // add all other DwgDb classes
        RegisterDwgDbObjectExtensions (false);

        s_toolkitInitialized = true;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            IDwgDbHost::TerminateToolkit ()
    {
    s_toolkitInitialized = false;

    UnRegisterDwgDbObjectExtensions ();

#if DWGTOOLKIT_OpenDwg

#if defined(_MSC_VER) && (DWGDB_ToolkitMajorRelease < 19)
    ::odrxUninitWinNTCrypt ();
#endif

    try
        {
        odUninitialize ();
        }
    catch (OdError& error)
        {
        std::wcout << L"OpenDWG exception: " << reinterpret_cast<wchar_t const*>(error.description().c_str()) << std::endl;
        if (OdResult::eExtendedError != error.code())
            BeAssert (false && L"OpenDWG exception thrown from uninitialization!");
        }

#elif DWGTOOLKIT_RealDwg

    acdbCleanUp ();

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            IDwgDbHost::SetWorkingDatabase (DwgDbDatabaseP dwg)
    {
#if DWGTOOLKIT_RealDwg
    return  DwgToolkitHost::GetHost().setWorkingDatabase (dwg);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            IDwgDbHost::SetWorkingProgressMeter (IDwgDbProgressMeter* appMeter)
    {
    DwgDbProgressMeter* newMeter = DwgToolkitHost::GetHost().NewWorkingProgressMeter (appMeter);

#if DWGTOOLKIT_RealDwg
    /*-----------------------------------------------------------------------------------
    In cases when AcDb.dll needs to create a new progress meter, it calls the "new" method
    in other cases(most times) we have to explicitly set a global progress meter for RealDWG
    via setWorkingProgressMeter before we read a DWG file.
    +---------------+---------------+---------------+---------------+------------------*/
    DwgToolkitHost::GetHost().setWorkingProgressMeter (newMeter);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IDwgDbHost::IsDxfFile (WStringCR filename)
    {
    if (!filename.EndsWithI(L"dxf") && !filename.EndsWithI(L"dxb"))
        return  false;

#ifdef _MSC_VER
    int         handle = 0;
    ::_wsopen_s (&handle, filename.c_str(), _O_BINARY | _O_TEXT, _SH_DENYNO, _O_RDONLY);
    if (-1 == handle)
        return false;

    char        headerData[4097] = { 0 };
    int         headerSize = ::_read (handle, headerData, sizeof(headerData)-1);
    if (headerSize < 0)
        headerSize = 0;
    headerData[headerSize] = 0;
    ::_close (handle);

#else
    #error read & check file for DXF!!
    return  false;
#endif
    // a complete DXF file should contain $ACADVER followed by a version numer AC10xx in file header:
    CharCP      asciiString = strstr (headerData, "$ACADVER");
    if (nullptr != asciiString && nullptr != (asciiString = strstr(asciiString, "AC10")))
        return  true;

    /*-----------------------------------------------------------------------------------
    Ideally we want to see at least SECTION and ENTITIES to appear in the string, as the pair
    make a good telltale for a DXF file.  Unfortunately, some crapy DXF files have large data 
    prior to reaching to the ENTITIES section!  There is no point to open entire file just to
    check ENTITIES, because the file can be huge.  TFS's 346565, 633683.
    -----------------------------------------------------------------------------------*/
    if (nullptr != (asciiString = strstr(headerData, "SECTION")))
        return true;

    // a DXB file must start with "AutoCAD Binary DXF":
    if (0 == strncmp(headerData, "AutoCAD Binary DXF", 18))
        return  true;

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IDwgDbHost::LoadObjectEnabler (WStringCR moduleName)
    {
    bool    loaded = false;
#if DWGTOOLKIT_OpenDwg
    OdRxModule* app = OdRxSystemServices::loadModuleLib (OdString(moduleName.c_str()), true);
    if (nullptr != app)
        {
        app->initApp ();
        loaded = true;
        }

#elif DWGTOOLKIT_RealDwg

    loaded = acrxLoadModule (moduleName.c_str(), true);
#endif
    return  loaded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IDwgDbHost::GetCachedLocalFile (WStringR local, WStringCR url)
    {
    DwgToolkitHost& host = DwgToolkitHost::GetHost ();
    // first look the URL up in our saved list
    if (host.FindCachedLocalFile(local, url))
        return  true;
    // try OS's cache - will download & cache as necessary:
    return  host.DownloadOrGetCachedFile(local, url) == DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbDatabasePtr    IDwgDbHost::ReadFile (WStringCR filename, bool convCodepage, bool partialLoad, FileShareMode sharemode, WStringCR password)
    {
    if (filename.empty())
        return  DwgDbDatabasePtr();
        
#if DWGTOOLKIT_OpenDwg

    DwgToolkitHost& host = DwgToolkitHost::GetHost ();

    try
        {
        OdString        name(filename.c_str());
        OdString        passwd(password.c_str());
        OdDbDatabasePtr oddwg = host.readFile (name, convCodepage, partialLoad, static_cast<Oda::FileShareMode>(sharemode), passwd);

        if (!oddwg.isNull())
            return  oddwg;
        }
    catch (OdError&  odError)
	{
        this->_Alert (reinterpret_cast<WCharCP>(odError.description().c_str()));
        return  DwgDbDatabasePtr();
        }
    catch (...)
        {
        this->_Alert (L"Failed reading file...\n");
        return  DwgDbDatabasePtr();
        }

#elif DWGTOOLKIT_RealDwg

    // create database with neither default data nor association to a document:
    DwgDbDatabasePtr    dwg = new DwgDbDatabase (false, true);
    if (dwg.IsValid())
        {
#if VendorVersion <= 2016
         int             acmode;
         switch (sharemode)
            {
            case FileShareMode::DenyReadWrite:  acmode = _SH_DENYRW;    break;
            case FileShareMode::DenyWrite:      acmode = _SH_DENYWR;    break;
            case FileShareMode::DenyRead:       acmode = _SH_DENYRW;    break;
            default:
            case FileShareMode::DenyNo:         acmode = _SH_DENYNO;    break;
            }
#else
        AcDbDatabase::OpenMode  acmode;
        switch (sharemode)
            {
            case FileShareMode::DenyReadWrite:  acmode = AcDbDatabase::kForReadAndWriteNoShare;     break;
            case FileShareMode::DenyWrite:      acmode = AcDbDatabase::kForReadAndReadShare;        break;
            case FileShareMode::DenyRead:       acmode = AcDbDatabase::kForReadAndWriteNoShare;     break;
            default:
            case FileShareMode::DenyNo:         acmode = AcDbDatabase::kForReadAndAllShare;         break;
            }
#endif
    
        DwgToolkitHost::GetHost().setWorkingDatabase (dwg.get());
        
        const wchar_t*      pPassword = password.empty() ? nullptr : password.c_str();
        Acad::ErrorStatus   status = Acad::eOk;

#ifndef NDEBUG
        class LinkerReactor : public AcRxDLinkerReactor
        {
        private:
            IDwgDbHost&     m_host;
        public:
            explicit LinkerReactor (IDwgDbHost& h) : m_host(h) {}
            virtual void rxAppWillBeLoaded (const ACHAR* moduleName) override { m_host._DebugPrintf (L"Loading %ls", moduleName); }
            virtual void rxAppLoadAborted (const ACHAR* moduleName) override { m_host._DebugPrintf (L"Failed loading %ls", moduleName); }
            virtual void rxAppLoaded (const ACHAR* moduleName) override { m_host._DebugPrintf (L"Successfully loaded %ls", moduleName); }
        };  // LinkerReactor

        AcRxDynamicLinker*  dynamicLinker = AcRxDynamicLinker::cast (acrxSysRegistry()->at(ACRX_DYNAMIC_LINKER));
        LinkerReactor*      linkReactor = new LinkerReactor (*this);
        if (nullptr != dynamicLinker && nullptr != linkReactor)
            dynamicLinker->addReactor (linkReactor);

        // track xref events
        class XrefEventReactor : public AcRxEventReactor
        {
        private:
            IDwgDbHost&     m_host;
        public:
            explicit XrefEventReactor (IDwgDbHost& h) : m_host(h) {}
            virtual void beginRestore (AcDbDatabase* to, const ACHAR* name, AcDbDatabase* from) override
                {
                m_host._DebugPrintf (L"Restoring AcDbDatabase[%ls] from 0x%x to 0x%x", nullptr == name ? L"null" : name, from, to);
                }
            virtual void databaseConstructed (AcDbDatabase* db) override
                { 
                m_host._DebugPrintf (L"Created an xRef AcDbDatabase 0x%x", db);
                }
            virtual void databaseToBeDestroyed (AcDbDatabase* db) override
                {
                const ACHAR* name = nullptr;
                if (Acad::eOk != db->getFilename(name))
                    name = nullptr;
                m_host._DebugPrintf (L"Destroying AcDbDatabase 0x%x [%ls]", db, nullptr == name ? L"null" : name);
                }
        };  // XrefEventReactor

        AcRxEvent* arxEvent = AcRxEvent::cast (acrxSysRegistry()->at(ACRX_EVENT_OBJ));
        XrefEventReactor* xrefReactor = new XrefEventReactor (*this);
        arxEvent->addReactor (xrefReactor);
#endif  // NDEBUG

        try
            {
            if (IDwgDbHost::IsDxfFile(filename))
                status = dwg->dxfIn (filename.c_str(), nullptr);
            else
                status = dwg->readDwgFile (filename.c_str(), acmode, convCodepage, pPassword);
            }
        catch (...)
            {
            this->_DebugPrintf (L"Exception thrown from AcDbDatabase::readDwgFile");
            }

        switch (status)
            {
            case Acad::eOk:
                break;
            case Acad::eUndefineShapeName:
            case Acad::eMissingDxfField:
                status = Acad::eOk;
                break;
            case Acad::eNLSFileNotAvailable:
            default:
                {
                AcString    msg = AcString(L"Error reading file...") + acadErrorStatusText(status);
                this->_Alert (msg.kwszPtr());
                }
            }

#ifndef NDEBUG
        if (nullptr != dynamicLinker && nullptr != linkReactor)
            dynamicLinker->removeReactor (linkReactor);
        if (nullptr != linkReactor)
            delete linkReactor;
        if (nullptr != arxEvent && nullptr != xrefReactor)
            arxEvent->removeReactor (xrefReactor);
        if (nullptr != xrefReactor)
            delete xrefReactor;
#endif  // NDEBUG

        if (Acad::eOk == status)
            return  dwg;
        }
#endif

    return  DwgDbDatabasePtr();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbLayoutManagerPtr   IDwgDbHost::GetLayoutManager () const
    {
    return new DwgDbLayoutManager (DwgToolkitHost::GetHost().layoutManager());
    }
