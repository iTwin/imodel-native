/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Registry/iModelBridgeRegistry.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif
#include <iModelBridge/iModelBridgeRegistry.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeGetProcAddress.h>
#include <Logging/bentleylogging.h>
#include <BeXml/BeXml.h>
#include "cpl_spawn.h"
#include <cxxopts/cxxopts.hpp>

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
static bset<BeFileName> s_badAffinityCalculators;
static ProfileVersion s_schemaVer(1,0,0,0);
static BeSQLite::PropertySpec s_schemaVerPropSpec("SchemaVersion", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeRegistryBase::OpenOrCreateStateDb()
    {
    BeFileName tempDir;
    Desktop::FileSystem::BeGetTempPath(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    if (m_stateFileName.DoesPathExist())
        {
        MUSTBEOK(m_stateDb.OpenBeSQLiteDb(m_stateFileName, Db::OpenParams(Db::OpenMode::ReadWrite)));

        // Double-check that this really is a fwk registry db
        Utf8String propStr;
        if (BE_SQLITE_ROW != m_stateDb.QueryProperty(propStr, s_schemaVerPropSpec))
            {
            LOG.fatalv(L"%ls - this is an invalid fwk registry db. Re-creating ...", m_stateFileName.c_str());
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

        // WARNING: Do not change the name or layout of the DocumentProperties - Bentley Automation Services assumes the following definition:
        MUSTBEOK(m_stateDb.CreateTable("DocumentProperties", "LocalFilePath TEXT NOT NULL UNIQUE COLLATE NoCase,\
                                                                DocGuid TEXT UNIQUE COLLATE NoCase,\
                                                                DesktopURN TEXT,\
                                                                WebURN TEXT,\
                                                                AttributesJSON TEXT,\
                                                                ChangeHistoryJSON TEXT,\
                                                                SpatialRootTransformJSON TEXT"));

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
iModelBridgeRegistryBase::iModelBridgeRegistryBase(BeFileNameCR stagingDir, BeFileNameCR dbname)
    {
    m_stagingDir = stagingDir;
    m_stateFileName = dbname;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeRegistryBase::MakeDbName(BeFileNameCR stagingDir, Utf8StringCR iModelName)
    {
    BeFileName dbName = stagingDir;
    dbName.AppendToPath(WString(iModelName.c_str(), true).c_str());
    dbName.append(L".fwk-registry.db");
    return dbName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeRegistryBase::~iModelBridgeRegistryBase()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeRegistryBase::QueryBridgeLibraryPathByName(uint64_t* rowid, WStringCR bridgeName)
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
BentleyStatus iModelBridgeRegistryBase::QueryBridgeAssignedToDocument(BeFileNameR libPath, WStringR name, BeFileNameCR docName)
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
static CPLSpawnedProcess* findOrStartAffinityCalculator(BeFileNameCR affinityLibraryPath, bool forceNew)
    {
    if (forceNew)
        s_affinityCalculators.erase(affinityLibraryPath);

    auto iFound = s_affinityCalculators.find(affinityLibraryPath);
    if (iFound != s_affinityCalculators.end())
        return iFound->second;

    auto calcexe = Desktop::FileSystem::GetExecutableDir();
    calcexe.AppendToPath(L"iModelBridgeGetAffinityHost/");
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
BentleyStatus callAffinityFunc(Utf8String& line0, Utf8String& line1, CPLSpawnedProcess* calc, BeFileNameCR filePath, BeFileNameCR affinityLibraryPath)
    {
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
    if ((BSISUCCESS != CPLPipeReadLine(line0, responseHandle))
     || (BSISUCCESS != CPLPipeReadLine(line1, responseHandle)))
        {
        LOG.errorv(L"%ls - has crashed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityLibraryPath, BeFileNameCR filePath)
    {
    auto calc = findOrStartAffinityCalculator(affinityLibraryPath, false);
    if (nullptr == calc)
        {
        LOG.errorv(L"%ls - not found or could not be executed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    Utf8String line0, line1;
    if (BSISUCCESS != callAffinityFunc(line0, line1, calc, filePath, affinityLibraryPath))
        {
        if (s_badAffinityCalculators.find(affinityLibraryPath) != s_badAffinityCalculators.end())
            {
            return BSIERROR;
            }

        LOG.errorv(L"%ls - Attempting to restart", affinityLibraryPath.c_str());
        auto calc = findOrStartAffinityCalculator(affinityLibraryPath, false);
        if (nullptr == calc)
            {
            LOG.errorv(L"%ls - Restart failed", affinityLibraryPath.c_str());
            s_badAffinityCalculators.insert(affinityLibraryPath);
            return BSIERROR;
            }

        if (BSISUCCESS != callAffinityFunc(line0, line1, calc, filePath, affinityLibraryPath))
            {
            s_badAffinityCalculators.insert(affinityLibraryPath);
            return BSIERROR;
            }
        }

    int v = 0;
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
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       iModelBridgeRegistryBase::ComputeBridgeAffinityInParentContext(iModelBridgeWithAffinity& bridgeAffinity, bool thisBridgeisPP, WStringCR parent)
    {
    //Do nothing for no affinity
    if (iModelBridgeAffinityLevel::None == bridgeAffinity.m_affinity)
        return SUCCESS;
    //Do nothing if there is no parent
    if (parent.empty())
        return SUCCESS;

    //Do nothing for same parent
    if (0 == bridgeAffinity.m_bridgeRegSubKey.CompareToI(parent))
        return SUCCESS;

    //We have a dwg bridge file. But parent is not dwg bridge. Lets ignore its affinity
    if (0 == bridgeAffinity.m_bridgeRegSubKey.CompareToI(L"RealDWGBridge"))
        {
        bridgeAffinity.m_bridgeRegSubKey = parent;
        return SUCCESS;
        }
    
    //We have a revit bridge file. But parent is not revit bridge. Lets ignore its affinity
    if (0 == bridgeAffinity.m_bridgeRegSubKey.CompareToI(L"RevitBridge"))
        {
        bridgeAffinity.m_affinity = iModelBridgeAffinityLevel::None;
        return SUCCESS;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::SearchForBridgeToAssignToDocument(WStringR bridgeName, BeFileNameCR sourceFilePath, WStringCR parentBridgeName)
    {
    BeFileName a; 
    if (BSISUCCESS == QueryBridgeAssignedToDocument(a, bridgeName, sourceFilePath)) // If we have already assigned a bridge to this document, then stick with that.
        return BSISUCCESS;

    LOG.tracev(L"SearchForBridgeToAssignToDocument %ls", sourceFilePath.c_str());

    iModelBridgeWithAffinity bestBridge;

    auto iterateInstalled = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath, AffinityLibraryPath, IsPowerPlatformBased FROM fwk_InstalledBridges");
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

        bool isPPBased = iterateInstalled->GetValueBoolean(2);
        if (BSISUCCESS != ComputeBridgeAffinityInParentContext(thisBridge, isPPBased, parentBridgeName))
            continue;

        LOG.tracev(L"%ls -> (%ls,%d)", affinityLibraryPath.c_str(), thisBridge.m_bridgeRegSubKey.c_str(), (int)thisBridge.m_affinity);

        if (!thisBridge.m_bridgeRegSubKey.empty() && thisBridge.m_affinity > bestBridge.m_affinity)
            bestBridge = thisBridge;
        }

    if ((bestBridge.m_bridgeRegSubKey.empty()) || (bestBridge.m_affinity == iModelBridgeAffinityLevel::None))
        {
        LOG.warningv(L"%ls - no installed bridge can convert this document", sourceFilePath.c_str());
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

    EnsureDocumentPropertiesFor(sourceFilePath, nullptr);
    bridgeName = bestBridge.m_bridgeRegSubKey;
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeRegistryBase::_IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey)
    {
    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir

    auto findBridgeForDoc = m_stateDb.GetCachedStatement("SELECT b.ROWID BridgeLibraryPath FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (b.ROWID = a.Bridge) AND (a.SourceFile=?) AND (b.Name=?)");
    findBridgeForDoc->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    findBridgeForDoc->BindText(2, Utf8String(bridgeRegSubKey), Statement::MakeCopy::Yes);
    return BE_SQLITE_ROW == findBridgeForDoc->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::_AssignFileToBridge(BeFileNameCR sourceFilePath, wchar_t const* bridgeRegSubKey, BeGuidCP guid)
    {
    auto findBridgeForDoc = m_stateDb.GetCachedStatement("SELECT b.ROWID BridgeLibraryPath FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (b.ROWID = a.Bridge) AND (a.SourceFile=?)");
    findBridgeForDoc->BindText(1, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW == findBridgeForDoc->Step())
        {
        LOG.errorv(L"File %ls cannot be assigned to %ls . Since it already has an assignment", sourceFilePath.c_str(), bridgeRegSubKey);
        return ERROR;
        }

    uint64_t bestBridgeRowid = 0;
    QueryBridgeLibraryPathByName(&bestBridgeRowid, bridgeRegSubKey);
    if (0 == bestBridgeRowid)
        {
        LOG.errorv(L"Bridge %ls cannot be found in the list of installed bridges.", bridgeRegSubKey);
        return ERROR;
        }

    auto insertAssignment = m_stateDb.GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge) VALUES(?,?)");
    insertAssignment->BindText(1, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
    insertAssignment->BindInt64(2, bestBridgeRowid);
    auto rc = insertAssignment->Step();
    if (BE_SQLITE_DONE != rc)
        {
        LOG.errorv(L"File %ls cannot be assigned to %ls . Error inserting into the database", sourceFilePath.c_str(), bridgeRegSubKey);
        return ERROR;
        }

    LOG.tracev(L"File %ls assigned to %ls .", sourceFilePath.c_str(), bridgeRegSubKey);

    //!Lets insert default document properties for this 
    EnsureDocumentPropertiesFor(sourceFilePath, guid);

    m_stateDb.SaveChanges();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::_QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey)
    {
    auto stmt = m_stateDb.GetCachedStatement("SELECT a.SourceFile FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (b.ROWID = a.Bridge) AND (b.Name=?)");
    stmt->BindText(1, Utf8String(bridgeRegSubKey), Statement::MakeCopy::Yes);
    while (BE_SQLITE_ROW == stmt->Step())
        fns.push_back(BeFileName(stmt->GetValueText(0), true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR topDirIn)
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
            {
            WString bridgeName;
            SearchForBridgeToAssignToDocument(bridgeName, entryName, L"");
            }
        else
            SearchForBridgesToAssignToDocumentsInDir(entryName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeRegistryBase::SearchForBridgesToAssignToFile(BeFileNameCR fileName, WStringCR parentBridgeName, bset<BeFileName>& currentContext)
    {
    if (fileName.empty())
        return ERROR;

    //Assign the incoming file
    WString bridgeName;
    //Skip already visited files to prevent circural reference cases
    if (currentContext.end() != currentContext.find(fileName))
        return BSISUCCESS;
    else
        currentContext.insert(fileName);

    SearchForBridgeToAssignToDocument(bridgeName, fileName, parentBridgeName);

    BASFileLocator locator;
    IDmsFileLocator::DmsInfo info;
    if (SUCCESS != locator.GetDocumentInfo(info, fileName))
        return ERROR;

    for (auto refInfo : info.m_refs)
        {
        IDmsFileLocator::DmsInfo refDocInfo;
        if (SUCCESS == locator.GetDocumentInfo(refDocInfo, refInfo.m_id, m_stagingDir))
            SearchForBridgesToAssignToFile(refDocInfo.m_fileName, bridgeName, currentContext);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::SearchForBridgesToAssignToDocuments()
    {
    if (!m_masterFilePath.empty())
        {
        bset<BeFileName> currentContext;
        SearchForBridgesToAssignToFile(m_masterFilePath, L"", currentContext);
        }
    else
        {
        // There are no documents in the staging directory itself. The docs are all in subdirectories (where the
        // subdir name corresponds to the PW doc GUID).
        BeFileName entryName;
        bool        isDir;
        for (BeDirectoryIterator dirs(m_stagingDir); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
            {
            if (isDir)
                SearchForBridgesToAssignToDocumentsInDir(entryName);
            }
        }

    killAllAffinityCalculators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::WriteBridgesFile()
    {
    BeFileName bridgesFileName(m_stateFileName.GetDirectoryName());
    bridgesFileName.AppendToPath(L"bridges.txt");
    BeFileStatus status;
    BeTextFilePtr bridgesFile = BeTextFile::Open(status, bridgesFileName.c_str(), TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
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
bool iModelBridgeRegistryBase::QueryAnyInstalledBridges()
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
void iModelBridgeRegistry::_DiscoverInstalledBridges()
    {
    LOG.tracev(L"DiscoverInstalledBridges");

    HKEY iModelBridgesKey;
    long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Bentley\\iModelBridges", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &iModelBridgesKey);
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
        if (ERROR_SUCCESS != RegOpenKeyExW(iModelBridgesKey, iModelBridgeName, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &subKey))
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
void iModelBridgeRegistry::_DiscoverInstalledBridges()
    {
    BeAssert(false && "TBD");
    }

BentleyStatus iModelBridgeRegistry::_FindBridgeInRegistry(BeFileNameR bridgeLibrary, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)
    {
    BeAssert(false && "TBD");
    return BSIERROR;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus readEntireFile(Utf8StringR contents, BeFileNameCR fileName)
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
        contents.assign((Utf8CP) (bytes.data() + 3));
    else
        contents.assign((char*) bytes.data());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignCxxOptsResult(cxxopts::ParseResult& result, CharCP option, BeFileNameR fileNameResult)
    {
    if (result.count(option) > 0)
        {
        WString fileName(result[option].as<std::string>().c_str(), true);
        fileName.DropQuotes();
        BeFileName::FixPathName(fileNameResult, fileName.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void  ParseCxxOptsResult(cxxopts::ParseResult& result, iModelBridgeRegistryBase::AssignCmdLineArgs& args)
    {
    AssignCxxOptsResult(result, "fwk-input", args.m_inputFileName);
    AssignCxxOptsResult(result, "fwk-staging-dir", args.m_stagingDir);
    AssignCxxOptsResult(result, "registry-dir", args.m_registryDir);
    AssignCxxOptsResult(result, "fwk-logging-config-file", args.m_loggingConfigFileName);

    if (result.count("server-repository") > 0)
        {
        args.m_repositoryName = result["server-repository"].as<std::string>().c_str();
        args.m_repositoryName.DropQuotes();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static cxxopts::Options GetCmdLineOptions()
    {
    cxxopts::Options options("iModelBridgeAssign", "iModelBridgeAssign creates an SQL database where bridge Assignments are stored");
    options.add_options()
        ("o,options-file", "FileName containing command line options", cxxopts::value<std::string>(),"<Optional>")
        ("f,fwk-input", "The input Master model for assignment scanning", cxxopts::value<std::string>(),"eg.\"Mastermodel.dgn\"")
        ("d,fwk-staging-dir", "Optional input staging directory for assignment scanning. Ignored if --fwk-input is specified.", cxxopts::value<std::string>(),"eg.\"c:\\BAS_WORKDIR\\1\"")
        ("s,server-repository", "The iModel for the conversion job.", cxxopts::value<std::string>(),"<Required>")
        ("r,registry-dir", "The directory to store assignments.", cxxopts::value<std::string>(),"<Optional>")
        ("l,fwk-logging-config-file", "The configuration file to be used for logging.", cxxopts::value<std::string>(), "<Optional>")
        ("h,help", "Print help")
        ;
    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistryBase::AssignCmdLineArgs::ParseCommandLine(int argc, WCharCP argv[])
    {
    std::vector<Utf8String> args;

    auto argPtrs = new char*[argc];

    for (int index = 0; index < argc; ++index)
        {
        Utf8String val(argv[index]);
        args.push_back(val);
        }

    for (int index =0; index <argc; ++index)
        argPtrs[index] = &args[index][0];

    cxxopts::Options options = GetCmdLineOptions();
    auto result = options.parse(argc, argPtrs);
    
    
    if (result.count("options-file") > 0)
        {
        Utf8String contents;
        BeFileName optionsFileName;
        AssignCxxOptsResult(result, "options-file", optionsFileName);
        readEntireFile(contents, optionsFileName);

        if (!contents.empty())
        {
            bvector<Utf8String> fileOptions;
            BeStringUtilities::Split(contents.c_str(), "\r\n", NULL, fileOptions);
            for (Utf8StringCR opt : fileOptions)
            {
                if (opt.empty())
                    continue;

                args.push_back(opt);
            }
            delete[]argPtrs;
            argc = (int) args.size();
            argPtrs = new char*[argc];
            
            for (int index = 0; index < argc; ++index)
                argPtrs[index] = &args[index][0];
            auto finalResult = options.parse(argc, argPtrs);
            ParseCxxOptsResult(finalResult, *this);
        }
        else
            ParseCxxOptsResult(result, *this);
        }
    else
        ParseCxxOptsResult(result, *this);

    if (result.count("help"))
        {
        std::cout << options.help({ "", "Group" }) << std::endl;
        exit(0);
        }

    delete []argPtrs;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void initLoggingForAssignMain(BeFileNameCR loggingConfigFileName)
    {
    if (!loggingConfigFileName.empty() && loggingConfigFileName.DoesPathExist())
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, loggingConfigFileName);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        return;
        }

    fprintf(stderr, "Logging.config.xml not specified. Activating default logging using console provider.\n");
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridgeRegistry", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridge", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridgeFwk", NativeLogging::LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistryBase::AssignMain(int argc, WCharCP argv[])
    {
    iModelBridgeRegistryUtils::InitCrt(false);

    AssignCmdLineArgs args;
    if (BSISUCCESS != args.ParseCommandLine(argc, argv))
        return -1;

    initLoggingForAssignMain(args.m_loggingConfigFileName);


    if (args.m_stagingDir.empty() || !args.m_stagingDir.DoesPathExist())
        {
        LOG.fatal(L"Staging directory specified must exist.");
        fprintf(stderr, GetCmdLineOptions().help({ "" }).c_str());
        return -1;
        }
     if (args.m_repositoryName.empty())
        {
        LOG.fatal(L"iModel name should be specified.");
        fprintf(stderr, GetCmdLineOptions().help({ "" }).c_str());
        return -1;
        }

    initLoggingForAssignMain(args.m_loggingConfigFileName);

    BeFileName dbDir = !args.m_registryDir.empty() ? args.m_registryDir : args.m_stagingDir;
    auto dbname = MakeDbName(dbDir, args.m_repositoryName);
    Dgn::iModelBridgeRegistry app(args.m_stagingDir, dbname);

    auto dbres = app.OpenOrCreateStateDb();
    if (BE_SQLITE_OK != dbres)
        return RETURN_STATUS_LOCAL_ERROR;

    app._DiscoverInstalledBridges();
    app.m_masterFilePath = args.m_inputFileName;
    app.SearchForBridgesToAssignToDocuments();
    app.m_stateDb.SaveChanges();

    app.WriteBridgesFile();

    app.m_stateDb.CloseDb();

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<iModelBridgeRegistry> iModelBridgeRegistry::OpenForFwk(BeSQLite::DbResult& res, BeFileNameCR stagingDir, Utf8StringCR iModelName)
    {
    // Staging dir is probably a subdirectory of the main job work directory. The registry db is stored in the main job work dir.
    // Look up the parent directory chain until we find it.
    auto jobWorkDir = stagingDir;
    BeFileName dbname;
    while (!jobWorkDir.empty())
        {
        dbname = MakeDbName(jobWorkDir, iModelName);
        if (dbname.DoesPathExist())
            break;
        jobWorkDir.PopDir();
        }

    if (!dbname.DoesPathExist())
        {
        LOG.errorv(L"%ls - cannot find fwk registry db in this directory or any of its parents. Creating an empty db.", stagingDir.c_str());
        dbname = MakeDbName(stagingDir, iModelName);
        }

    RefCountedPtr<iModelBridgeRegistry> reg = new iModelBridgeRegistry(stagingDir, dbname);
    if (BE_SQLITE_OK != (res = reg->OpenOrCreateStateDb()))
        return nullptr;

    return reg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            iModelBridgeRegistryBase::SetDocumentProperties(iModelBridgeDocumentProperties& docProps, BeFileNameCR fn)
    {
    auto stmt = m_stateDb.GetCachedStatement("INSERT OR REPLACE INTO DocumentProperties (LocalFilePath,DocGuid,DesktopURN,WebURN,AttributesJSON,SpatialRootTransformJSON) VALUES(?,?,?, ?, ?, ?)");
    stmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    stmt->BindText(2, docProps.m_docGuid, Statement::MakeCopy::Yes);
    stmt->BindText(3, docProps.m_desktopURN, Statement::MakeCopy::Yes);
    stmt->BindText(4, docProps.m_webURN, Statement::MakeCopy::Yes);
    stmt->BindText(5, docProps.m_attributesJSON, Statement::MakeCopy::Yes);
    stmt->BindText(6, docProps.m_spatialRootTransformJSON, Statement::MakeCopy::Yes);
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::EnsureDocumentPropertiesFor(BeFileNameCR fn, BeGuidCP guid)
    {
    iModelBridgeDocumentProperties _props;
    if (BSISUCCESS == _GetDocumentProperties(_props, fn))
        return;

#define TEST_REGISTRY_FAKE_GUIDS
#ifdef TEST_REGISTRY_FAKE_GUIDS
    BeGuid testGuid;
    if (guid)
        testGuid = *guid;
    else
        testGuid.Create();

    auto stmt = m_stateDb.GetCachedStatement("INSERT INTO DocumentProperties (LocalFilePath,DocGuid,DesktopURN,WebURN,AttributesJSON,SpatialRootTransformJSON) VALUES(?,?,'', '', '', '')");
    stmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    stmt->BindText(2, testGuid.ToString(), Statement::MakeCopy::Yes);
#else
    auto stmt = m_stateDb.GetCachedStatement("INSERT INTO DocumentProperties (LocalFilePath) VALUES(?)");
    stmt->BindText(1, Utf8String(fn).c_str(), Statement::MakeCopy::Yes);
#endif
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::_GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn)
    {
    if (!m_stateDb.TableExists("DocumentProperties"))
        return BSIERROR;

    //                                               0         1           2       3              4                         5
    auto stmt = m_stateDb.GetCachedStatement("SELECT docGuid, DesktopURN, WebURN, AttributesJSON, SpatialRootTransformJSON, ChangeHistoryJSON FROM DocumentProperties WHERE (LocalFilePath=?)");
    stmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BSIERROR;

    props.m_docGuid        = stmt->GetValueText(0);
    props.m_desktopURN     = stmt->GetValueText(1);
    props.m_webURN         = stmt->GetValueText(2);
    props.m_attributesJSON = stmt->GetValueText(3);
    props.m_spatialRootTransformJSON = stmt->GetValueText(4);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::_GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFileName, BeGuid const& docGuid)
    {
    if (!m_stateDb.TableExists("DocumentProperties"))
        return BSIERROR;

    //                                               0               1           2       3              4                          5
    auto stmt = m_stateDb.GetCachedStatement("SELECT LocalFilePath, DesktopURN, WebURN, AttributesJSON, SpatialRootTransformJSON, ChangeHistoryJSON FROM DocumentProperties WHERE (docGuid=?)");
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
    props.m_attributesJSON      = stmt->GetValueText(3);
    props.m_spatialRootTransformJSON = stmt->GetValueText(4);
    props.m_changeHistoryJSON   = stmt->GetValueText(5);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeRegistry::iModelBridgeRegistry(BeFileNameCR stagingDir, BeFileNameCR dbName)
    :iModelBridgeRegistryBase(stagingDir, dbName)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeRegistryBase::RemoveFileAssignment(BeFileNameCR fn)
    {
    CachedStatementPtr docStmt;
    m_stateDb.GetCachedStatement(docStmt, "DELETE FROM DocumentProperties WHERE (LocalFilePath=?)");
    docStmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    if (docStmt->Step() != BE_SQLITE_DONE)
        return BSIERROR;

    CachedStatementPtr bridgeStmt;
    m_stateDb.GetCachedStatement(bridgeStmt, "DELETE FROM fwk_BridgeAssignments WHERE (SourceFile = ?)");
    bridgeStmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    if (bridgeStmt->Step() != BE_SQLITE_DONE)
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BASFileLocator::BASFileLocator()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BASFileLocator::GetDocumentInfoFromPrp(DmsInfo& info, BeFileNameCR fileName)
    {
    if (!fileName.DoesPathExist())
        {
        LOG.errorv(L"%ls - Doc.prp file does not exist.", fileName.c_str());
        return BSIERROR;
        }

    BeXmlStatus status = BEXML_Success;   
    WString errmsg;
    BeXmlReaderPtr  reader = BeXmlReader::CreateAndReadFromFile(status, fileName.c_str(), &errmsg);
    if (!reader.IsValid() || status != BEXML_Success)
        {
        LOG.errorv(L"%ls - Error opening prp file as xml. Msg: %ls", fileName.c_str(), errmsg.c_str());
        return  BSIERROR;
        }

    BeFileName directoryName = fileName.GetDirectoryName();
    bool    foundPwDoc = false;
    IBeXmlReader::ReadResult result;
    while ((result = reader->Read()) == IBeXmlReader::READ_RESULT_Success)
        {
        if (reader->GetCurrentNodeType() != IBeXmlReader::NODE_TYPE_Element)
            continue;

        Utf8String  nodeName;
        if (BEXML_Success != reader->GetCurrentNodeName(nodeName))
            break;

        // only care about node "PwDoc"
        if (nodeName.EqualsI("PwDoc"))
            foundPwDoc = true;

        Utf8String  key, value;
        while (BEXML_Success == reader->ReadToNextAttribute(&key, &value))
            {
            if (key.EqualsI("VaultID"))
                info.m_id.first = atoi(value.c_str());
            else if (key.EqualsI("DocumentID"))
                info.m_id.second = atoi(value.c_str());
            else if (key.EqualsI("FileName"))
                {
                BeFileName docName(directoryName);
                docName.AppendToPath(WString(value.c_str(), true).c_str());
                info.m_fileName = docName;
                }
            }
        if (foundPwDoc)
            return BSISUCCESS;
        }

    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BASFileLocator::GetReferenceInfoFromPrp(DmsInfo& info, BeFileNameCR fileName)
    {
    BeXmlStatus status = BEXML_Success;

    WString errmsg;
    BeXmlReaderPtr  reader = BeXmlReader::CreateAndReadFromFile(status, fileName.c_str(), &errmsg);
    if (!reader.IsValid() || status != BEXML_Success)
        {
        LOG.errorv(L"%ls - Error opening prp file as xml. Msg: %ls", fileName.c_str(), errmsg.c_str());
        return  BSIERROR;
        }

    bool    foundReferencesNode = false;
    IBeXmlReader::ReadResult result;
    while ((result = reader->Read()) == IBeXmlReader::READ_RESULT_Success)
        {
        if (reader->GetCurrentNodeType() != IBeXmlReader::NODE_TYPE_Element)
            continue;

        Utf8String  nodeName;
        if (BEXML_Success != reader->GetCurrentNodeName(nodeName))
            break;

        // only care about node "References"
        if (nodeName.EqualsI("References"))
            {
            foundReferencesNode = true;
            continue;
            }
        // walk through "Reference" nodes and parse and check their attributes:
        if (foundReferencesNode && nodeName.EqualsI("Reference"))
            {
            ReferenceInfo refInfo;
            Utf8String  key, value;
            while (BEXML_Success == reader->ReadToNextAttribute(&key, &value))
                {
                if (key.EqualsI("VaultID"))
                    refInfo.m_id.first = atoi(value.c_str());
                else if (key.EqualsI("DocID"))
                    refInfo.m_id.second = atoi(value.c_str());
                }
            info.m_refs.push_back(refInfo);
            }
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BASFileLocator::ReadMetaDataFromDmsDir(DmsInfo& info, BeFileNameCR dirName)
    {
    BeFileName docPrpFile = dirName;
    docPrpFile.AppendToPath(L"doc.prp");
    if (SUCCESS != GetDocumentInfoFromPrp(info, docPrpFile))
        return BSIERROR;

    BeFileName refPrpFile = dirName;
    refPrpFile.AppendToPath(L"refs.prp");
    if (SUCCESS != GetReferenceInfoFromPrp(info, refPrpFile))
        return BSIERROR;

    m_dmsInfoCache[info.m_id] = info;
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BASFileLocator::GetDocumentInfo(DmsInfo& info, BeFileNameCR fileName)
    {
    //Seach the cache for existing info
    for (auto iter : m_dmsInfoCache)
        {
        if (0 == iter.second.m_fileName.CompareToI(fileName))
            {
            LOG.tracev(L"%ls - Found DmsInfo from memory cache.", fileName.c_str());
            info = iter.second;
            return BSISUCCESS;
            }
        }

    return ReadMetaDataFromDmsDir(info, fileName.GetDirectoryName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IDmsFileLocator::DmsInfo::DmsInfo()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            IDmsFileLocator::DmsInfo::Clear()
    {
    m_id.first = 0;
    m_id.second = 0;
    m_fileName.clear();
    m_refs.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IDmsFileLocator::ReferenceInfo::ReferenceInfo()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            IDmsFileLocator::ReferenceInfo::Clear()
    {
    m_id.first = 0;
    m_id.second = 0;
    m_referenceModelID = 0;
    m_nestDepth = 0;
    m_referenceType = 0;
    m_dmsFlags = 0;
    m_elementId = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  BASFileLocator::GetDocumentInfo(DmsInfo& info, T_DocumentId const& docId, BeFileNameCR dmsFolder)
    {
    auto iter = m_dmsInfoCache.find(docId);
    if (m_dmsInfoCache.end() != iter)
        {
        info = iter->second;
        LOG.tracev(L"%ls - Found DmsInfo from memory cache.", iter->second.m_fileName.c_str());
        return BSISUCCESS;
        }

    BeFileName dmsDirName(dmsFolder);
    WString folderName;
    folderName.Sprintf(L"%d_%d", docId.first, docId.second);
    dmsDirName.AppendToPath(folderName.c_str());

    return ReadMetaDataFromDmsDir(info, dmsDirName);
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