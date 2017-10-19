/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Registry/iModelBridgeRegistry.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif
#include "iModelBridgeRegistry.h"
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeGetProcAddress.h>
#include <Logging/bentleylogging.h>
#include "cpl_spawn.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#define RETURN_STATUS_SUCCESS           0
#define RETURN_STATUS_USAGE_ERROR       1
#define RETURN_STATUS_CONVERTER_ERROR   2
#define RETURN_STATUS_SERVER_ERROR      3
#define RETURN_STATUS_LOCAL_ERROR       4

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {return rc;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#ifdef _WIN32
#define EXE_EXT L".exe"
#else
#define EXE_EXT L""
#endif

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridgeRegistry"))

USING_NAMESPACE_BENTLEY_DGN

static bmap<BeFileName, CPLSpawnedProcess*> s_affinityCalculators;
static ProfileVersion s_schemaVer(1,0,0,0);
static BeSQLite::PropertySpec s_schemaVerPropSpec("SchemaVersion", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeRegistry::OpenOrCreateStateDb()
    {
    BeFileName tempDir;
    Desktop::FileSystem::BeGetTempPath(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    if (m_stateFileName.DoesPathExist())
        {
        MUSTBEOK(m_stateDb.OpenBeSQLiteDb(m_stateFileName, Db::OpenParams(Db::OpenMode::ReadWrite)));

        // Double-check that this really is a fwk state db
        Utf8String propStr;
        if (BE_SQLITE_ROW != m_stateDb.QueryProperty(propStr, s_schemaVerPropSpec))
            {
            LOG.fatalv(L"%ls - this is an invalid fwk state db. Re-creating ...", m_stateFileName.c_str());
            m_stateDb.CloseDb();
            m_stateFileName.BeDeleteFile();
            }
        }

    if (!m_stateFileName.DoesPathExist())
        {
        MUSTBEOK(m_stateDb.CreateNewDb(m_stateFileName));
        MUSTBEOK(m_stateDb.CreateTable("fwk_InstalledBridges", "Name TEXT NOT NULL UNIQUE COLLATE NoCase, \
                                                                BridgeLibraryPath TEXT UNIQUE COLLATE NoCase, \
                                                                AffinityLibraryPath TEXT UNIQUE COLLATE NoCase, \
                                                                IsPowerPlatformBased BOOLEAN \
                                                                "));
        MUSTBEOK(m_stateDb.CreateTable("fwk_BridgeAssignments", "SourceFile TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY, \
                                                                 Bridge BIGINT"));  // Bridge --foreign key--> fwk_InstalledBridges

        MUSTBEOK(m_stateDb.SavePropertyString(s_schemaVerPropSpec, s_schemaVer.ToJson()));

        MUSTBEOK(m_stateDb.SaveChanges());
        }
    else
        {
        Utf8String propStr;
        MUSTBEROW(m_stateDb.QueryProperty(propStr, s_schemaVerPropSpec));

        ProfileVersion ver(propStr.c_str());
        if (ver.IsEmpty() || ver.GetMajor() != s_schemaVer.GetMajor())
            {
            LOG.fatalv(L"%ls - version %ls is too old", m_stateFileName.c_str(), WString(ver.ToString().c_str(), true).c_str());
            return BE_SQLITE_ERROR_ProfileTooOld;
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeRegistry::iModelBridgeRegistry(BeFileNameCR stagingDir, Utf8StringCR iModelName)
    {
    m_stagingDir = stagingDir;
    m_stateFileName = stagingDir;
    m_stateFileName.AppendToPath(WString(iModelName.c_str(), true).c_str());
    m_stateFileName.append(L".fwk-registry.db");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeRegistry::~iModelBridgeRegistry()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeRegistry::QueryBridgeLibraryPathByName(uint64_t* rowid, WStringCR bridgeName)
    {
    auto stmt = m_stateDb.GetCachedStatement("SELECT ROWID, BridgeLibraryPath FROM fwk_InstalledBridges WHERE Name=?");
    stmt->BindText(1, Utf8String(bridgeName), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BeFileName();

    if (nullptr != rowid)
        *rowid = stmt->GetValueInt64(0);
    return BeFileName(stmt->GetValueText(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::QueryBridgeAssignedToDocument(BeFileNameR libPath, WStringR name, BeFileNameCR docName)
    {
    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir

    auto stmt = m_stateDb.GetCachedStatement("SELECT b.BridgeLibraryPath, b.Name FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (a.SourceFile=?) AND (b.ROWID = a.Bridge)");
    stmt->BindText(1, Utf8String(docName), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BSIERROR;

    libPath.SetNameUtf8(stmt->GetValueText(0));
    name.AssignUtf8(stmt->GetValueText(1));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static CPLSpawnedProcess* findOrStartAffinityCalculator(BeFileNameCR affinityLibraryPath)
    {
    auto iFound = s_affinityCalculators.find(affinityLibraryPath);
    if (iFound != s_affinityCalculators.end())
        return iFound->second;

    auto calcexe = Desktop::FileSystem::GetExecutableDir();
    calcexe.AppendToPath(L"iModelBridgeGetAffinityHost" EXE_EXT);
    BeAssert(calcexe.DoesPathExist());

    Utf8String calcexeU(calcexe);
    Utf8String libU(affinityLibraryPath);
    libU.AddQuotes();
    const char* callData[] = { calcexeU.c_str(), libU.c_str(), nullptr };

    auto proc = CPLSpawnAsync(NULL, callData, true, true, false, nullptr);
    if (nullptr == proc)
        return nullptr;

    return s_affinityCalculators[affinityLibraryPath] = proc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void killAllAffinityCalculators()
    {
    for (auto i = s_affinityCalculators.begin(); i != s_affinityCalculators.end(); ++i)
        {
        CPLSpawnAsyncFinish(i->second, false, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityLibraryPath, BeFileNameCR filePath)
    {
    auto calc = findOrStartAffinityCalculator(affinityLibraryPath);
    if (nullptr == calc)
        {
        LOG.errorv(L"%ls - not found or could not be executed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    Utf8String filePathU(filePath);
    filePathU.AddQuotes();
    filePathU.append("\n");
    if (!CPLPipeWrite(CPLSpawnAsyncGetOutputFileHandle(calc), filePathU.c_str(), (int)filePathU.size()))
        {
        LOG.errorv(L"%ls - has crashed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    auto responseHandle = CPLSpawnAsyncGetInputFileHandle(calc);

    // I expect exactly two lines of text: [0]affinity (integer) [1]bridge name (string)
    Utf8String line0, line1;
    if ((BSISUCCESS != CPLPipeReadLine(line0, responseHandle))
     || (BSISUCCESS != CPLPipeReadLine(line1, responseHandle)))
        {
        LOG.errorv(L"%ls - has crashed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    int v;
    if (1 != sscanf(line0.c_str(), "%d", &v))
        {
        LOG.errorv(L"%ls - \"%ls\" is an invalid affinity value?!", affinityLibraryPath.c_str(), WString(line0.c_str(), true).c_str());
        return BSIERROR;
        }

    affinity.m_affinity = (iModelBridgeAffinityLevel)v;
    affinity.m_bridgeRegSubKey.AssignUtf8(line1.c_str());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::SearchForBridgeToAssignToDocument(BeFileNameCR sourceFilePath)
    {
    BeFileName a; WString b;
    if (BSISUCCESS == QueryBridgeAssignedToDocument(a, b, sourceFilePath)) // If we have already assigned a bridge to this document, then stick with that.
        return BSISUCCESS;

    LOG.tracev(L"SearchForBridgeToAssignToDocument %ls", sourceFilePath.c_str());

    iModelBridgeWithAffinity bestBridge;

    auto iterateInstalled = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath, AffinityLibraryPath FROM fwk_InstalledBridges");
    while (BE_SQLITE_ROW == iterateInstalled->Step())
        {
        BeFileName bridgePath(iterateInstalled->GetValueText(0));
        BeFileName affinityLibraryPath(iterateInstalled->GetValueText(1));
        if (affinityLibraryPath.empty())
            affinityLibraryPath = bridgePath;
        if (affinityLibraryPath.empty())
            continue;

        iModelBridgeWithAffinity thisBridge;
        if (BSISUCCESS != ComputeBridgeAffinityToDocument(thisBridge, affinityLibraryPath, sourceFilePath))
            continue;

        LOG.tracev(L"%ls -> (%ls,%d)", affinityLibraryPath.c_str(), thisBridge.m_bridgeRegSubKey.c_str(), (int)thisBridge.m_affinity);

        if (thisBridge.m_affinity > bestBridge.m_affinity)
            bestBridge = thisBridge;
        }

    if ((bestBridge.m_bridgeRegSubKey.empty()) || (bestBridge.m_affinity == iModelBridgeAffinityLevel::None))
        {
        LOG.infov(L"%ls - no installed bridge can convert this document", sourceFilePath.c_str());
        return BSIERROR;
        }

    uint64_t bestBridgeRowid = 0;
    QueryBridgeLibraryPathByName(&bestBridgeRowid, bestBridge.m_bridgeRegSubKey);
    BeAssert(0 != bestBridgeRowid);

    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir

    auto insertAssignment = m_stateDb.GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge) VALUES(?,?)");
    insertAssignment->BindText(1, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
    insertAssignment->BindInt64(2, bestBridgeRowid);
    auto rc = insertAssignment->Step();
    BeAssert(BE_SQLITE_DONE == rc);

    LOG.tracev(L"%ls := (%ls,%d)", sourceFilePath.c_str(), bestBridge.m_bridgeRegSubKey.c_str(), (int)bestBridge.m_affinity);
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeRegistry::_IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey)
    {
    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir

    auto findBridgeForDoc = m_stateDb.GetCachedStatement("SELECT b.ROWID BridgeLibraryPath FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (b.ROWID = a.Bridge) AND (a.SourceFile=?) AND (b.Name=?)");
    findBridgeForDoc->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    findBridgeForDoc->BindText(2, Utf8String(bridgeRegSubKey), Statement::MakeCopy::Yes);
    return BE_SQLITE_ROW == findBridgeForDoc->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistry::SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR topDirIn)
    {
    BeFileName topDir(topDirIn);
    topDir.AppendSeparator();

    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs (topDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (entryName.GetBaseName().EndsWithI(L".prp"))         // Filter out some control files that we know we should ignore
            continue;

        // *** NEEDS WORK: Should probably store relative paths

        if (!isDir)
            SearchForBridgeToAssignToDocument(entryName);
        else
            SearchForBridgesToAssignToDocumentsInDir(entryName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistry::SearchForBridgesToAssignToDocuments()
    {
    // There are no documents in the staging directory itself. The docs are all in subdirectories (where the
    // subdir name corresponds to the PW doc GUID).
    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs (m_stagingDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (isDir)
            SearchForBridgesToAssignToDocumentsInDir(entryName);
        }

    killAllAffinityCalculators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::WriteBridgesFile()
    {
    BeFileName bridgesFileName(m_stagingDir);
    bridgesFileName.AppendToPath(L"bridges.txt");
    BeFileStatus status;
    BeTextFilePtr bridgesFile = BeTextFile::Open(status, bridgesFileName.c_str(), TextFileOpenType::Write, TextFileOptions::KeepNewLine);
    if (!bridgesFile.IsValid())
        {
        LOG.fatalv(L"%ls - error writing bridges file", bridgesFileName.c_str());
        return BSIERROR;
        }

    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir - turn it back into an abs dir for .txt file?

    auto stmt = m_stateDb.GetCachedStatement("SELECT DISTINCT b.Name, b.IsPowerPlatformBased, a.SourceFile FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (b.ROWID = a.Bridge)");
    while (BE_SQLITE_ROW == stmt->Step())
        bridgesFile->PrintfTo(false, L"%ls;%ls;%d\n", WString(stmt->GetValueText(0), true).c_str(), WString(stmt->GetValueText(2), true).c_str(), stmt->GetValueBoolean(1));

    bridgesFile->Close();
    bridgesFile = nullptr;
    return BSISUCCESS;
    }

#ifdef _WIN32
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
LONG GetStringRegKey(WStringR strValue, HKEY hKey, WStringCR strValueName, WStringCR strDefaultValue)
    {
    strValue = strDefaultValue;
    WCHAR szBuffer[2048];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
        {
        strValue = szBuffer;
        }
    return nError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeRegistry::QueryAnyInstalledBridges()
    {
    auto stmt = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath FROM fwk_InstalledBridges");
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (!Utf8String::IsNullOrEmpty(stmt->GetValueText(0)))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistry::DiscoverInstalledBridges()
    {
    LOG.tracev(L"DiscoverInstalledBridges");

    HKEY iModelBridgesKey;
    long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Bentley\\iModelBridges", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &iModelBridgesKey);
    if (result != ERROR_SUCCESS)
        {
        LOG.info(L"HKLM\\Software\\Bentley\\iModelBridges not found -- no iModelBridges are installed on this machine.");
        return;
        }

    wchar_t iModelBridgeName[1024];
    DWORD iModelBridgeNameLen = _countof(iModelBridgeName);
    DWORD index = 0;
    while ( ERROR_SUCCESS == RegEnumKeyExW(iModelBridgesKey, index, iModelBridgeName, &iModelBridgeNameLen, NULL, NULL, NULL, NULL) )
        {
        // iModelBridgeName buffer is contains key iModelBridgeName here
        HKEY subKey;
        if (ERROR_SUCCESS != RegOpenKeyExW(iModelBridgesKey, iModelBridgeName, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &subKey))
            continue;

        auto stmt = m_stateDb.GetCachedStatement("INSERT INTO fwk_InstalledBridges (Name,BridgeLibraryPath,AffinityLibraryPath,IsPowerPlatformBased) VALUES(?,?,?,?)");

        BeFileName bridgeLibraryPath, affinityLibraryPath;
        WString isPowerPlatformBased;
        GetStringRegKey(bridgeLibraryPath, subKey, L"BridgeLibraryPath", L"");
        GetStringRegKey(affinityLibraryPath, subKey, L"AffinityLibraryPath", L"");
        GetStringRegKey(isPowerPlatformBased, subKey, L"IsPowerPlatformBased", L"");

        stmt->BindText(1, Utf8String(iModelBridgeName).c_str(), Statement::MakeCopy::Yes);

        if (!bridgeLibraryPath.empty())
            stmt->BindText(2, Utf8String(bridgeLibraryPath).c_str(), Statement::MakeCopy::Yes);
        else
            stmt->BindNull(2);

        if (!affinityLibraryPath.empty())
            stmt->BindText(3, Utf8String(affinityLibraryPath).c_str(), Statement::MakeCopy::Yes);
        else
            stmt->BindNull(3);

        bool isPPBased = false;
        if (!isPowerPlatformBased.empty())
            {
            isPPBased= 0 == isPowerPlatformBased.CompareToI(L"True");
            stmt->BindBoolean(4, isPPBased);
            }
        else
            stmt->BindNull(4);

        stmt->Step(); // may fail with constraint if bridge library is already registered.

        // Get ready for the next subkey
        iModelBridgeNameLen = _countof(iModelBridgeName); // restore iModelBridgeNameLen after it is set to key's actual length by RegEnumKeyExW
        ++index; // increment subkey index

        LOG.tracev(L"%ls -> %ls %ls %d", iModelBridgeName, bridgeLibraryPath.c_str(), affinityLibraryPath.c_str(), isPPBased);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::_FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)
    {
    WString bridgePath(L"Software\\Bentley\\iModelBridges");
    bridgePath.append(L"\\");
    bridgePath.append(bridgeName.c_str());

    HKEY iModelBridgeKey;
    long result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, bridgePath.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &iModelBridgeKey);
    if (result != ERROR_SUCCESS)
        return BSIERROR;

    GetStringRegKey(bridgeLibraryPath, iModelBridgeKey, L"BridgeLibraryPath", L"");
    GetStringRegKey(bridgeAssetsDir, iModelBridgeKey, L"BridgeAssetsDir", L"");

    return BSISUCCESS;
    }

#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistry::DiscoverInstalledBridges()
    {
    BeAssert(false && "TBD");
    }

BentleyStatus iModelBridgeRegistry::_FindBridgeInRegistry(BeFileNameR bridgeLibrary, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)
    {
    BeAssert(false && "TBD");
    return BSIERROR;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2012
//---------------------------------------------------------------------------------------
static void justLogAssertionFailures(WCharCP message, WCharCP file, uint32_t line, BeAssertFunctions::AssertType atype)
    {
    WPrintfString str(L"ASSERT: (%ls) @ %ls:%u\n", message, file, line);
    LOG.error(str.c_str());
    //::OutputDebugStringW (str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistry::InitCrt(bool quietAsserts)
    {
#ifdef NDEBUG
    quietAsserts = true; // we never allow disruptive asserts in a production program
#endif

    if (quietAsserts)
        BeAssertFunctions::SetBeAssertHandler(justLogAssertionFailures);

#ifdef _WIN32
    if (quietAsserts)
        _set_error_mode(_OUT_TO_STDERR);
    else
        _set_error_mode(_OUT_TO_MSGBOX);

    #if defined (UNICODE_OUTPUT_FOR_TESTING)
        // turning this on makes it so we can show unicode characters, but screws up piped output for programs like python.
        _setmode(_fileno(stdout), _O_U16TEXT);  // so we can output any and all unicode to the console
        _setmode(_fileno(stderr), _O_U16TEXT);  // so we can output any and all unicode to the console
    #endif

    // FOR THE CONSOLE PUBLISHER ONLY! "Gui" publishers won't have any console output and won't need this.
    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is
    // based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale.
    // However, the success of this code is dependent on two things:
    //      1) The narrow encoding must support the wide character being converted.
    //      2) The font/gui must support the rendering of that character.
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console."
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale(LC_CTYPE, "");
#else
    // unix-specific CRT init
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void* iModelBridgeRegistry::GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName)
    {
    BeFileName pathname(BeFileName::FileNameParts::DevAndDir, bridgeDllName);

    BeGetProcAddress::SetLibrarySearchPath(pathname);
    auto hinst = BeGetProcAddress::LoadLibrary(bridgeDllName);
    if (!hinst)
        {
        LOG.fatalv(L"%ls: not found or could not be loaded", bridgeDllName.c_str());
        return nullptr;
        }

    return BeGetProcAddress::GetProcAddress (hinst, funcName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistry::ComputeAffinityMain(int argc, WCharCP argv[])
    {
    iModelBridgeRegistry::InitCrt(false);

    if (argc != 2)
        {
        fprintf(stderr, "syntax: iModelBridgeGetAffinityHost affinityLibraryName\n");
        return -1;
        }

    BeFileName affinityLibraryPath(argv[1]);

    auto getAffinity = (T_iModelBridge_getAffinity*)GetBridgeFunction(affinityLibraryPath, "iModelBridge_getAffinity");
    if (nullptr == getAffinity)
        {
        LOG.errorv(L"%ls - does not export the iModelBridge_getAffinity function. That is probably an error.", affinityLibraryPath.c_str());
        return -1;
        }

    char filePathUtf8[MAX_PATH*2];
    while (fgets(filePathUtf8, sizeof(filePathUtf8), stdin))
        {
        BeFileName filePath(filePathUtf8, true);
        filePath.RemoveQuotes();
        iModelBridgeWithAffinity bridge;
        bridge.m_bridgeRegSubKey = L"dummy";    // *** TRICKY: See comment below "Don't pass empty string to bridge"
        getAffinity(bridge, affinityLibraryPath, filePath);
        fprintf(stdout, "%d\n%s\n", (int)bridge.m_affinity, Utf8String(bridge.m_bridgeRegSubKey).c_str());
        fflush(stdout);
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus readEntireFile(WStringR contents, BeFileNameCR fileName)
    {
    BeFile errfile;
    if (BeFileStatus::Success != errfile.Open(fileName.c_str(), BeFileAccess::Read))
        return BSIERROR;

    bvector<Byte> bytes;
    if (BeFileStatus::Success != errfile.ReadEntireFile(bytes))
        return BSIERROR;

    if (bytes.empty())
        return BSISUCCESS;

    bytes.push_back('\0'); // End of stream

    const Byte utf8BOM[] = {0xef, 0xbb, 0xbf};
    if (bytes[0] == utf8BOM[0] || bytes[1] == utf8BOM[1] || bytes[2] == utf8BOM[2])
        contents.AssignUtf8((Utf8CP) (bytes.data() + 3));
    else
        contents.AssignA((char*) bytes.data());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static WString getArgValueW(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return argValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getArgValue(WCharCP arg)
    {
    return Utf8String(getArgValueW(arg));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistry::AssignCmdLineArgs::ParseCommandLine(int argc, WCharCP argv[])
    {
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg][0] == '@')
            {
            BeFileName rspFileName(argv[iArg]+1);
            WString wargs;
            if (BSISUCCESS != readEntireFile(wargs, rspFileName))
                {
                fwprintf(stderr, L"%ls - response file not found\n", rspFileName.c_str());
                return BSIERROR;
                }

            bvector<WString> strings;
            bvector<WCharCP> ptrs;
            BeStringUtilities::ParseArguments(strings, wargs.c_str(), L"\n\r");

            if (!strings.empty())
                {
                ptrs.push_back(argv[0]);
                for (auto const& str: strings)
                    {
                    if (!str.empty())
                        ptrs.push_back(str.c_str());
                    }

                if (BSISUCCESS != ParseCommandLine((int)ptrs.size(), &ptrs.front()))
                    return BSIERROR;
                }

            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-staging-dir="))
            {
            if (!m_stagingDir.empty())
                {
                fwprintf(stderr, L"The --fwk-staging-dir= option may appear only once.\n");
                return BSIERROR;
                }
            m_stagingDir.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-repository="))
            {
            m_repositoryName = getArgValue(argv[iArg]);
            continue;
            }

        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized assign argument\n", argv[iArg]);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistry::AssignMain(int argc, WCharCP argv[])
    {
    iModelBridgeRegistry::InitCrt(false);

    AssignCmdLineArgs args;
    if (BSISUCCESS != args.ParseCommandLine(argc, argv))
        return -1;

    if (args.m_stagingDir.empty() || !args.m_stagingDir.DoesPathExist()
     || args.m_repositoryName.empty())
        {
        fwprintf(stderr, L"syntax: %ls --server-repository=<reponame> --fwk-staging-dir=<dirname>\n", argv[0]);
        return -1;
        }

    Dgn::iModelBridgeRegistry app(args.m_stagingDir, args.m_repositoryName);

    auto dbres = app.OpenOrCreateStateDb();
    if (BE_SQLITE_OK != dbres)
        return RETURN_STATUS_LOCAL_ERROR;

    app.DiscoverInstalledBridges();
    app.SearchForBridgesToAssignToDocuments();
    app.m_stateDb.SaveChanges();

    app.WriteBridgesFile();

    app.m_stateDb.CloseDb();

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<iModelBridgeRegistry> iModelBridgeRegistry::OpenForFwk(BeFileNameCR stagingDir, Utf8StringCR iModelName)
    {
    RefCountedPtr<iModelBridgeRegistry> reg = new iModelBridgeRegistry(stagingDir, iModelName);
    if (BE_SQLITE_OK != reg->OpenOrCreateStateDb())
        return nullptr;
    return reg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::_GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn)
    {
    if (!m_stateDb.TableExists("DocumentProperties"))
        return BSIERROR;

    //                                               0         1           2       3
    auto stmt = m_stateDb.GetCachedStatement("SELECT docGuid, DesktopURN, WebURN, OtherPropertiesJSON FROM DocumentProperties WHERE (LocalFilePath=?)");
    stmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BSIERROR;

    props.m_docGuid             = stmt->GetValueText(0);
    props.m_desktopURN          = stmt->GetValueText(1);
    props.m_webURN              = stmt->GetValueText(2);
    props.m_otherPropertiesJSON = stmt->GetValueText(3);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistry::_GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFileName, BeGuid const& docGuid)
    {
    if (!m_stateDb.TableExists("DocumentProperties"))
        return BSIERROR;

    //                                               0               1           2       3
    auto stmt = m_stateDb.GetCachedStatement("SELECT LocalFilePath, DesktopURN, WebURN, OtherPropertiesJSON FROM DocumentProperties WHERE (docGuid=?)");
#ifdef WIP_GUID_BINARY
    stmt->BindGuid(1, docGuid);
#else
    auto guidstr = docGuid.ToString();
    guidstr.ToLower();
    stmt->BindText(1, guidstr.c_str(), Statement::MakeCopy::No);
#endif
    if (BE_SQLITE_ROW != stmt->Step())
        return BSIERROR;

    props.m_docGuid             = docGuid.ToString();
    localFileName    = BeFileName(stmt->GetValueText(0), true);
    props.m_desktopURN          = stmt->GetValueText(1);
    props.m_webURN              = stmt->GetValueText(2);
    props.m_otherPropertiesJSON = stmt->GetValueText(3);
    return BSISUCCESS;
    }

/*
"Don't pass empty string to bridge"

This is very tricky. An empty string is initialized with pointer to a static "null string" buffer object. 
The Bstdcxx::basic_string code for assigning, reallocating, and destroying a string recognizes the pointer to
this special object and does the right thing. 

iModelBridgeRegistry is compiled into a standalone program called iModelBridgeGetAffinityHost.exe, which statically 
links with the bentley libraries. That program loads the bridge dll dynamically, as shown above, and invokes the 
bridge's getaffinity function. The affinity function assigns a value to a string in the iModelBridgeWithAffinity object 
that is created by and passed in by the iModelBridgeGetAffinityHost.exe program. So, this leads to a problem. 
iModelBridgeGetAffinityHost.exe constructs the string to point to the special "null string" buffer object in the
iModelBridgeGetAffinityHost.exe program. The the bridge dll links with the dynamic bentley library, and so its copy 
of the Bstdcxx::basic_string code will be checking for the special null string object that is specific to the dynamic
bentley library and will not recognize the one set up by the iModelBridgeGetAffinityHost.exe program.

We work around this by not passing an empty string to the bridge's getaffinity program. That way, the assignment
in the bridge will indeed trigger dynamic memory allocation, which will be freed in the iModelBridgeGetAffinityHost.exe
program. That does not pose a problem, as the EXE determines the CRT that is used by both.
*/