/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif
#include <iModelBridge/iModelBridgeRegistry.h>
#include <iModelBridge/iModelBridge.h>
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

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {LOG.fatalv("%s", BeSQLite::Db::InterpretDbResult(rc)); return rc;}}
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
static ProfileVersion s_schemaVer(1,1,0,0);
static BeSQLite::PropertySpec s_schemaVerPropSpec("SchemaVersion", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Wouter.Rombouts                 10/19
+---------------+---------------+---------------+---------------+---------------+------*/
int RegistryBusyHandler::_OnBusy(int count) const
    {
    //VSTS#184256. On some slow BAS machines the 5 sec default treshold is not enough.

    if ((count < 5) || !(count % 5))
        {
        if (count > 15)
            {
            LOG.errorv(L"Waiting for SQLITE_BUSY to yield. (t=%d s)", count);
            }
        else
            {
            LOG.tracev(L"Waiting for SQLITE_BUSY to yield. (t=%d s)", count);
            }
        }

    if (count > 600)
        {
        return 0;
        }

    BeThreadUtilities::BeSleep(1000);

    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeRegistryBase::OpenOrCreateStateDb()
    {
    BeFileName tempDir;
    Desktop::FileSystem::BeGetTempPath(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    m_retry = new RegistryBusyHandler();

    if (m_stateFileName.DoesPathExist())
        {
        LOG.infov(L"Opening registry file %ls", m_stateFileName.c_str());
        MUSTBEOK(m_stateDb.OpenBeSQLiteDb(m_stateFileName, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes, m_retry.get())));

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
        LOG.infov(L"Creating registry file %ls", m_stateFileName.c_str());
        MUSTBEOK(m_stateDb.CreateNewDb(m_stateFileName, BeGuid(), BeSQLite::Db::CreateParams (BeSQLite::Db::PageSize::PAGESIZE_4K, BeSQLite::Db::Encoding::Utf8, true, DefaultTxn::Yes, m_retry.get())));
        MUSTBEOK(m_stateDb.CreateTable("fwk_InstalledBridges", "Name TEXT NOT NULL UNIQUE COLLATE NoCase, \
                                                                BridgeLibraryPath TEXT UNIQUE COLLATE NoCase, \
                                                                AffinityLibraryPath TEXT UNIQUE COLLATE NoCase, \
                                                                IsPowerPlatformBased BOOLEAN, \
                                                                BridgeAssetsDir TEXT COLLATE NoCase \
                                                                "));
        MUSTBEOK(m_stateDb.CreateTable("fwk_BridgeAssignments", "SourceFile TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY, \
                                                                 Bridge BIGINT, \
                                                                 Affinity INT"));

        // WARNING: Do not change the name or layout of the DocumentProperties - Bentley Automation Services assumes the following definition:
        MUSTBEOK(m_stateDb.CreateTable("DocumentProperties", "LocalFilePath TEXT NOT NULL UNIQUE COLLATE NoCase,\
                                                                DocGuid TEXT UNIQUE COLLATE NoCase,\
                                                                DesktopURN TEXT,\
                                                                WebURN TEXT,\
                                                                AttributesJSON TEXT,\
                                                                ChangeHistoryJSON TEXT,\
                                                                SpatialRootTransformJSON TEXT"));
        MUSTBEOK(m_stateDb.CreateTable("Recommendations", "LocalFilePath TEXT NOT NULL COLLATE NoCase, Bridge TEXT NOT NULL COLLATE NoCase"));
        MUSTBEOK(m_stateDb.CreateTable("DiscloseNotSupported", "Bridge TEXT NOT NULL UNIQUE COLLATE NoCase, FOREIGN KEY(Bridge) REFERENCES fwk_InstalledBridges(Name)"));

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

        if ((ver.GetMinor() < s_schemaVer.GetMinor()) && !m_stateDb.IsReadonly())
            {
            MUSTBEOK(UpgradeMinorVersion(ver));
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeRegistryBase::UpgradeMinorVersion(ProfileVersion const& storedVer)
    {
    BeAssert(storedVer.GetMajor() == s_schemaVer.GetMajor());

    auto ver = storedVer;

    if (ver.GetMinor() == 0)
        {
        // 1.0 -> 1.1
        auto stmt = m_stateDb.GetCachedStatement("alter table fwk_BridgeAssignments add Affinity INT");
        MUSTBEDONE(stmt->Step());

        stmt = m_stateDb.GetCachedStatement("alter table fwk_InstalledBridges add BridgeAssetsDir TEXT COLLATE NoCase");
        MUSTBEDONE(stmt->Step());

        stmt = nullptr;

        // Note: Don't bother inserting default values for new columns. The caller obviously doesn't know about these columns and so will not query them or use their values.

        MUSTBEOK(m_stateDb.CreateTable("Recommendations", "LocalFilePath TEXT NOT NULL COLLATE NoCase, Bridge TEXT NOT NULL COLLATE NoCase"));
        MUSTBEOK(m_stateDb.CreateTable("DiscloseNotSupported", "Bridge TEXT NOT NULL UNIQUE COLLATE NoCase, FOREIGN KEY(Bridge) REFERENCES fwk_InstalledBridges(Name)"));

        MUSTBEOK(m_stateDb.SavePropertyString(s_schemaVerPropSpec, s_schemaVer.ToJson()));

        MUSTBEOK(m_stateDb.SaveChanges());
        }

    // When we go to 1.2, add incremental update logic here

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
BentleyStatus iModelBridgeRegistryBase::QueryBridgeAssignedToDocument(iModelBridgeWithAffinity* ba, BeFileNameCR docName)
    {
    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir

    auto stmt = m_stateDb.GetCachedStatement("SELECT b.Name, a.Affinity FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (a.SourceFile=?) AND (b.ROWID = a.Bridge)");
    stmt->BindText(1, Utf8String(docName), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BSIERROR;

    if (ba != nullptr)
        {
        ba->m_bridgeRegSubKey = WString(stmt->GetValueText(0), true);
        ba->m_affinity = (iModelBridgeAffinityLevel)stmt->GetValueInt(1);
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void GetAffinityHostExePath(BeFileNameR calcExe, BeFileNameCR affinityLibraryPath)
    {
    calcExe = affinityLibraryPath.GetDirectoryName();
    calcExe.AppendToPath(L"iModelBridgeGetAffinityHost" EXE_EXT);
    if (calcExe.DoesPathExist())
        return;

    calcExe = Desktop::FileSystem::GetExecutableDir();
    calcExe.AppendToPath(L"iModelBridgeGetAffinityHost/");
    calcExe.AppendToPath(L"iModelBridgeGetAffinityHost" EXE_EXT);
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

    BeFileName calcexe;
    GetAffinityHostExePath(calcexe, affinityLibraryPath);
    if (!calcexe.DoesPathExist())
        return nullptr;

    WString libU(affinityLibraryPath); 
    libU.AddQuotes();
    const wchar_t* callData[] = { calcexe.c_str(), libU.c_str(), nullptr };

    auto proc = CPLSpawnAsync(NULL, callData, true, true, false);
    if (nullptr == proc)
        return nullptr;

    return s_affinityCalculators[affinityLibraryPath] = proc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static int callDiscloseFilesAndAffinities(BeFileNameCR outputFileName, BeFileNameCR affinityLibraryPath, BeFileNameCR assetsPath, BeFileNameCR sourceFileName, WStringCR bridgeName)
    {
    BeFileName calcexe;
    GetAffinityHostExePath(calcexe, affinityLibraryPath);
    if (!calcexe.DoesPathExist())
        return -1;

    WString libU(affinityLibraryPath);
    libU.AddQuotes();
    WString outputNameU(outputFileName);
    outputNameU.AddQuotes();
    WString assetsPathU(assetsPath);
    assetsPathU.AddQuotes();
    WString sourceFileNameU(sourceFileName);
    sourceFileNameU.AddQuotes();    
    WString bridgeNameU(bridgeName.c_str());
    bridgeNameU.AddQuotes();
    const wchar_t* callData[] = { calcexe.c_str(), outputNameU.c_str(), libU.c_str(), assetsPathU.c_str(), sourceFileNameU.c_str(), bridgeNameU.c_str(), nullptr };

    LOG.tracev(L"%ls <-discloseFilesAndAffinities- (%ls,%ls,%ls)", outputFileName.c_str(), affinityLibraryPath.c_str(), bridgeName.c_str(), sourceFileName.c_str());

    auto proc = CPLSpawnAsync(NULL, callData, false, false, false); // don't create any pipes, as we won't write to the process, and we won't read from it. We just expect it to create a file on disk.
    if (nullptr == proc)
        return -1;

    return CPLSpawnAsyncFinish(proc, true, false);
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
    s_affinityCalculators.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus callAffinityFunc(WString& line0, WString& line1, CPLSpawnedProcess* calc, BeFileNameCR filePath, BeFileNameCR affinityLibraryPath)
    {
    Utf8String filePathU(filePath);
    filePathU.AddQuotes();
    filePathU.append("\n");
    if (!CPLPipeWrite(CPLSpawnAsyncGetOutputFileHandle(calc), filePathU.c_str(), (int)filePathU.size()))
        {
        LOG.errorv(L"%ls - has crashed on %ls", affinityLibraryPath.c_str(), filePath.c_str());
        return BSIERROR;
        }

    auto responseHandle = CPLSpawnAsyncGetInputFileHandle(calc);

    // I expect exactly two lines of text: [0]affinity (integer) [1]bridge name (string)
    if ((BSISUCCESS != CPLPipeReadLine(line0, responseHandle))
     || (BSISUCCESS != CPLPipeReadLine(line1, responseHandle)))
        {
        LOG.errorv(L"%ls - has crashed on %ls", affinityLibraryPath.c_str(), filePath.c_str());
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

    WString line0, line1;
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
    if (1 != WString::Swscanf_safe(line0.c_str(), L"%d", &v))
        {
        LOG.errorv(L"%ls - \"%ls\" is an invalid affinity value?!", affinityLibraryPath.c_str(), WString(line0.c_str(), true).c_str());
        return BSIERROR;
        }

    affinity.m_affinity = (iModelBridgeAffinityLevel)v;
    affinity.m_bridgeRegSubKey = line1.c_str();
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

    //We have a ifc bridge file. But parent is not ifc bridge. Lets ignore its affinity
    if (0 == bridgeAffinity.m_bridgeRegSubKey.CompareToI(L"IFCBridge"))
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

    // if we have a C3D bridge file, but the parent is not C3D, ignore its affinity
    if (0 == bridgeAffinity.m_bridgeRegSubKey.CompareToI(L"C3dBridge"))
        {
        bridgeAffinity.m_affinity = iModelBridgeAffinityLevel::None;
        return SUCCESS;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/20
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::RecommendBridge(BeFileNameCR doc, bvector<WString> recommendedBridges)
    {
    if (recommendedBridges.empty() || !m_stateDb.TableExists("Recommendations"))
        return;
    
    for (auto const& bridge : recommendedBridges)
        {
        auto stmt = m_stateDb.GetCachedStatement("INSERT INTO Recommendations (LocalFilePath,Bridge) VALUES(?,?)");
        stmt->BindText(1, Utf8String(doc.c_str()), BeSQLite::Statement::MakeCopy::Yes);      
        stmt->BindText(2, Utf8String(bridge.c_str()), BeSQLite::Statement::MakeCopy::Yes);      
        auto rc = stmt->Step();
        if (rc != BE_SQLITE_DONE)
            {
            LOG.errorv(L"Insert recommendation %ls -> %ls failed with %lx", bridge.c_str(), doc.c_str(), rc);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/20
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::BridgeDoesNotDiscloseFiles(Utf8StringCR bridgeRegSubKey)
    {
    LOG.tracev("%s -> BridgeDoesNotDiscloseFiles", bridgeRegSubKey.c_str());
    
    auto stmt = m_stateDb.GetCachedStatement("INSERT OR IGNORE INTO DiscloseNotSupported (Bridge) VALUES(?)");
    stmt->BindText(1, bridgeRegSubKey, BeSQLite::Statement::MakeCopy::No);      
    auto rc = stmt->Step();
    if (rc != BE_SQLITE_DONE)
        {
        LOG.errorv(L"Insert BridgeDoesNotDiscloseFiles %s failed with %lx", bridgeRegSubKey.c_str(), rc);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::SearchForBridgeToAssignToDocument(WStringR bridgeName, BeFileNameCR sourceFilePath, WStringCR parentBridgeName)
    {
    ++m_processedFileCount;

    bool hasExistingAssignment = false;
    iModelBridgeWithAffinity bestBridge;
    if (BSISUCCESS == QueryBridgeAssignedToDocument(&bestBridge, sourceFilePath)) // If we have already assigned a bridge to this document
        {
        if (!m_updateAssignments) // and if we are not updating
            {
            bridgeName = bestBridge.m_bridgeRegSubKey;
            return BSISUCCESS;
            }
        hasExistingAssignment = true;
        }

    LOG.tracev(L"SearchForBridgeToAssignToDocument %ls", sourceFilePath.c_str());

    bvector<WString> recommendedBridges;

    auto iterateInstalled = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath, AffinityLibraryPath, IsPowerPlatformBased, Name FROM fwk_InstalledBridges");
    while (BE_SQLITE_ROW == iterateInstalled->Step())
        {
        if (!m_bridgeFilter.empty())
            {
            Utf8String bridge(iterateInstalled->GetValueText(3));
            bridge.ToLower();
            if (m_bridgeFilter.find(bridge) == m_bridgeFilter.end())
                continue;
            }

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
        if (!thisBridge.m_bridgeRegSubKey.empty() && thisBridge.m_affinity != iModelBridgeAffinityLevel::None)
            recommendedBridges.push_back(thisBridge.m_bridgeRegSubKey);

        if (thisBridge.m_bridgeRegSubKey.empty())
            continue;

        if (thisBridge.m_affinity > bestBridge.m_affinity)
            {
            bestBridge = thisBridge;
            continue;
            }

        if (thisBridge.m_affinity == bestBridge.m_affinity)
            {
            if (!parentBridgeName.empty() &&  0 == thisBridge.m_bridgeRegSubKey.CompareToI(parentBridgeName))
                {
                bestBridge = thisBridge;
                continue;
                }
             if (0 == thisBridge.m_bridgeRegSubKey.CompareToI(L"iModelBridgeForMstn"))
                {
                bestBridge = thisBridge;
                continue;
                }
            }
        }

    if ((bestBridge.m_bridgeRegSubKey.empty()) || (bestBridge.m_affinity == iModelBridgeAffinityLevel::None))
        {
        LOG.tracev(L"%ls - no installed bridge can convert this document", sourceFilePath.c_str());
        RecommendBridge(sourceFilePath, recommendedBridges);
        return BSIERROR;
        }

    uint64_t bestBridgeRowid = 0;
    QueryBridgeLibraryPathByName(&bestBridgeRowid, bestBridge.m_bridgeRegSubKey);
    if (0 == bestBridgeRowid)
        {
        LOG.tracev(L"%ls - no installed bridge can convert this document", sourceFilePath.c_str());
        RecommendBridge(sourceFilePath, recommendedBridges);
        return BSIERROR;
        }

    // *** NEEDS WORK: SourceFile should be a relative path, relative to m_fwkAssetsDir
    if (!hasExistingAssignment)
        {
        auto insert = m_stateDb.GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge,Affinity) VALUES(?,?,?)");
        insert->BindText(1, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
        insert->BindInt64(2, bestBridgeRowid);
        insert->BindInt(3, bestBridge.m_affinity);
        auto rc = insert->Step();
        BeAssert(BE_SQLITE_DONE == rc);
        LOG.tracev(L"%ls := (%ls,%d)", sourceFilePath.c_str(), bestBridge.m_bridgeRegSubKey.c_str(), (int)bestBridge.m_affinity);
        }
    else
        {
        auto update = m_stateDb.GetCachedStatement("UPDATE fwk_BridgeAssignments SET Bridge=?, Affinity=? WHERE SourceFile=?");
        update->BindInt64(1, bestBridgeRowid);
        update->BindInt(2, bestBridge.m_affinity);
        update->BindText(3, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
        auto rc = update->Step();
        BeAssert(BeSQLite::BE_SQLITE_DONE == rc);
        LOG.tracev(L"%ls := (%ls,%d) (update)", sourceFilePath.c_str(), bestBridge.m_bridgeRegSubKey.c_str(), (int)bestBridge.m_affinity);
        }

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

    auto insertAssignment = m_stateDb.GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge,Affinity) VALUES(?,?,?)");
    if (!insertAssignment.IsValid())
        {
        LOG.errorv(L"%ls - incompatible schema", m_stateFileName.c_str());
        return BSIERROR;
        }
    insertAssignment->BindText(1, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
    insertAssignment->BindInt64(2, bestBridgeRowid);
    insertAssignment->BindInt64(3, iModelBridgeAffinityLevel::ExactMatch);
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
* @bsimethod                                    Sam.Wilson                      03/20
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t countFilesInDirs(BeFileNameCR topDirIn)
    {
    uint32_t count = 0;
    BeFileName topDir(topDirIn);
    topDir.AppendSeparator();

    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs (topDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (entryName.GetBaseName().EndsWithI(L".prp"))         // Filter out some control files that we know we should ignore
            continue;

        if (!isDir)
            ++count;
        else
            count += countFilesInDirs(entryName);
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/20
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::PrintProgress(BeFileNameCR fileName)
    {
    if (!m_writeProgressToStdout || m_totalFileCount == 0)
        return;

    auto timeNow = BeTimeUtilities::QueryMillisecondsCounterUInt32();
    if ((timeNow - m_lastWriteToProgress) < m_writeProgressInterval)
        return;

    m_lastWriteToProgress = timeNow;

    Json::Value json(Json::objectValue);
    // json["jobRequestId"] =
    // json["jobRunCorrelationId"] =
    json["messageType"] = "progress";
    json["phase"] = "Assignments";
    json["step"] = Utf8String(fileName.c_str());
    json["phasesPct"] = 0;
    json["stepsPct"] =  (m_processedFileCount == 0)?              0 : 
                        (m_processedFileCount >= m_totalFileCount)? 100: 
                        ((100 * (m_processedFileCount-1)) / m_totalFileCount);
    json["phaseCount"] = 1;
    json["stepCount"] = m_totalFileCount;
    json["lastUpdateTime"] = timeNow;
    printf("%s\n", json.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR topDirIn)
    {
    LOG.tracev(L"SearchForBridgesToAssignToDocumentsInDir %ls", topDirIn.c_str());

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

            PrintProgress(entryName);
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
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeRegistryBase::DiscloseFilesAndComputeAssignments()
    {
    if (m_masterFilePath.empty() || m_affinityDbName.empty())
        {
        BeAssert(false);
        return BSIERROR;
        }

    LOG.tracev(L"DiscloseFilesAndComputeAssignments %ls", m_masterFilePath.c_str());

    BeFileName affinityDbName(m_affinityDbName);
    WString dir,basename;
    affinityDbName.ParseName(nullptr, &dir, &basename, nullptr);
    if (dir.empty())
        {
        if (m_registryDir.empty())
            {
            LOG.fatalv(L"If you do not supply a full path to the --affinity-db-name, then you must supply a value for the --registry-dir argument.");
            return BSIERROR;
            }
        affinityDbName = m_registryDir;
        affinityDbName.AppendToPath(m_affinityDbName);
        }

    if (true)
        {
        if (!affinityDbName.DoesPathExist())
            iModelBridgeAffinityDb::Create(affinityDbName);
         }

    auto iterateInstalled = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath, AffinityLibraryPath, bridgeAssetsDir, Name FROM fwk_InstalledBridges");
    while (BE_SQLITE_ROW == iterateInstalled->Step())
        {
        BeFileName bridgePath(iterateInstalled->GetValueText(0));
        BeFileName affinityLibraryPath(iterateInstalled->GetValueText(1));
        if (affinityLibraryPath.empty())
            affinityLibraryPath = bridgePath;
        if (affinityLibraryPath.empty())
            continue;
        BeFileName bridgeAssetsDir(iterateInstalled->GetValueText(2));
        if (bridgeAssetsDir.empty())
            {
            bridgeAssetsDir = affinityLibraryPath.GetDirectoryName();
            bridgeAssetsDir.AppendToPath(L"Assets");
            }
        Utf8String bridgeId(iterateInstalled->GetValueText(3));

        auto exitCode = callDiscloseFilesAndAffinities(affinityDbName, affinityLibraryPath, bridgeAssetsDir, m_masterFilePath, WString(bridgeId.c_str(), true));
        if (0 != exitCode)
            BridgeDoesNotDiscloseFiles(bridgeId);
        }

    auto affinityDb = iModelBridgeAffinityDb::Open(affinityDbName);
    if (!affinityDb.IsValid())
        {
        LOG.errorv(L"%ls - not found or could not be opened", affinityDbName.c_str());
        return BSIERROR;
        }

    if (true)
        {
        auto allSourceFiles = affinityDb->GetDb().GetCachedStatement("select filename from File");
        while (BE_SQLITE_ROW == allSourceFiles->Step())
            {
            EnsureDocumentPropertiesFor(BeFileName(allSourceFiles->GetValueText(0), true), nullptr);
            }
        EnsureDocumentPropertiesFor(m_masterFilePath, nullptr); // If the affinity function didn't report it, we must record the file, even if we don't report any affinity to it.
        }

    affinityDb->ComputeAssignments([this](BeFileNameCR filePath, Utf8StringCR bridgeRegSubKeyA, iModelBridgeAffinityLevel affinity)
        {
        WString bridgeRegSubKey(bridgeRegSubKeyA.c_str(), true);

        if (BSISUCCESS == QueryBridgeAssignedToDocument(nullptr, filePath)) // If we have already assigned a bridge to this document, then stick with that.
            return;

        uint64_t bestBridgeRowid = 0;
        QueryBridgeLibraryPathByName(&bestBridgeRowid, bridgeRegSubKey);
        if (0 == bestBridgeRowid)
            {
            LOG.tracev(L"%ls - no installed bridge can convert this document", filePath.c_str());
            bvector<WString> recommendedBridges = { bridgeRegSubKey };
            RecommendBridge(filePath, recommendedBridges);
            return;
            }

        auto insertAssignment = m_stateDb.GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge,Affinity) VALUES(?,?,?)");
        insertAssignment->BindText(1, Utf8String(filePath), Statement::MakeCopy::Yes);
        insertAssignment->BindInt64(2, bestBridgeRowid);
        insertAssignment->BindInt64(3, affinity);
        auto rc = insertAssignment->Step();
        BeAssert(BE_SQLITE_DONE == rc);

        LOG.tracev(L"%ls := (%ls,%d)", filePath.c_str(), bridgeRegSubKey.c_str(), (int)affinity);
        });

    // In some cases, when a bridge has an affinity to the masterfile, we assert its priority in handling certain types of reference files.
    auto masterfileRowId = affinityDb->FindFile(m_masterFilePath);
    bset<int64_t> parentFileRowIdsSeen;
    iModelBridgeWithAffinity assignment;
    if (BSISUCCESS == QueryBridgeAssignedToDocument(&assignment, m_masterFilePath))
        {
        UpdateAssignmentsOfReferences(*affinityDb, parentFileRowIdsSeen, masterfileRowId, assignment.m_bridgeRegSubKey);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::UpdateAssignmentsOfReferences(iModelBridgeAffinityDb& db, bset<int64_t>& parentFileRowIdsSeen, int64_t parentFileRowId, WStringCR parentBridgeRegSubKey)
    {
    if (!parentFileRowIdsSeen.insert(parentFileRowId).second)
        return; // we've already processed the attachments to this file.

    db.QueryAttachmentsToFile(parentFileRowId, [&](int64_t childFileRowId) {
        auto childFileName = db.GetFilename(childFileRowId);
        
        iModelBridgeWithAffinity assignment;
        if (BSISUCCESS == QueryBridgeAssignedToDocument(&assignment, childFileName))
            {
            auto originalBridge = assignment.m_bridgeRegSubKey;
            ComputeBridgeAffinityInParentContext(assignment, false, parentBridgeRegSubKey); // (*may* reassign assignment.m_bridgeRegSubKey to parentBridgeRegSubKey)
            if (!assignment.m_bridgeRegSubKey.EqualsI(originalBridge))
                {
                uint64_t newAssigedBridgeRowId;
                QueryBridgeLibraryPathByName(&newAssigedBridgeRowId, assignment.m_bridgeRegSubKey);

                auto update = m_stateDb.GetCachedStatement("UPDATE fwk_BridgeAssignments SET Bridge=? WHERE SourceFile=?");
                update->BindInt64(1, newAssigedBridgeRowId);
                update->BindText(2, Utf8String(childFileName), Statement::MakeCopy::Yes);
                auto rc = update->Step();
                BeAssert(BeSQLite::BE_SQLITE_DONE == rc);
                }
            }
        UpdateAssignmentsOfReferences(db, parentFileRowIdsSeen, childFileRowId, parentBridgeRegSubKey);
    });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryBase::SearchForBridgesToAssignToDocuments()
    {
    if (!m_masterFilePath.empty())
        {
        if (!m_affinityDbName.empty())
            {
            DiscloseFilesAndComputeAssignments();
            }
        else
            {
            LOG.tracev(L"SearchForBridgesToAssignToDocuments - file=%ls", m_masterFilePath.c_str());
            bset<BeFileName> currentContext;
            SearchForBridgesToAssignToFile(m_masterFilePath, L"", currentContext);
            }
        }
    else if (m_searchForFilesInStagingDir)
        {
        LOG.tracev(L"SearchForBridgesToAssignToDocuments - search in %ls", m_stagingDir.c_str());
        if (m_writeProgressToStdout)
            {
            m_totalFileCount = countFilesInDirs(m_stagingDir);
            }

        SearchForBridgesToAssignToDocumentsInDir(m_stagingDir);
        }
    else
        {
        LOG.tracev(L"SearchForBridgesToAssignToDocuments - search below %ls", m_stagingDir.c_str());
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
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void getVersionInfoAsString(WString& version, BeFileNameCR dllFileName)
    {
    DWORD  verHandle = NULL;
    UINT   size      = 0;
    LPBYTE lpBuffer  = NULL;
    DWORD  verSize   = GetFileVersionInfoSizeW(dllFileName.c_str(), &verHandle);

    if (verSize != NULL)
        {
        LPSTR verData = new char[verSize];

        if (GetFileVersionInfoW(dllFileName.c_str(), verHandle, verSize, verData))
            {
            if (VerQueryValueW(verData, L"\\", (VOID FAR* FAR*)&lpBuffer, &size))
                {
                if (size)
                    {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                        {

                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS
                        version.Sprintf(L"%d.%d.%d.%d",
                            ( verInfo->dwFileVersionMS >> 16 ) & 0xffff,
                            ( verInfo->dwFileVersionMS >>  0 ) & 0xffff,
                            ( verInfo->dwFileVersionLS >> 16 ) & 0xffff,
                            ( verInfo->dwFileVersionLS >>  0 ) & 0xffff
                            );
                        }
                    }
                }
            }
        delete[] verData;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeRegistryBase::WriteBridgesFile()
    {
    bset<WString> bridgeNames;
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
            {
            WString bridgeName(stmt->GetValueText(0), true);
            bool isPowerPlatformBased = stmt->GetValueBoolean(1);
            BeFileName sourceFile(stmt->GetValueText(2), true);

            bridgeNames.insert(bridgeName);

            bridgesFile->PrintfTo(false, L"%ls;%ls;%d\n", bridgeName.c_str(), sourceFile.c_str(), isPowerPlatformBased);
            }

        bridgesFile->Close();
        bridgesFile = nullptr;
        }

    if (true)
        {
        BeFileName fileName = BeFileName(m_stateFileName.GetDirectoryName());
        fileName.AppendToPath(L"bridgeDetails.txt");
        BeFileStatus status;
        BeTextFilePtr file = BeTextFile::Open(status, fileName.c_str(), TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
        if (!file.IsValid())
            {
            LOG.fatalv(L"%ls - error writing bridge details file", fileName.c_str());
            return BSIERROR;
            }

        for (auto const& bridgeName: bridgeNames)
            {
            BeFileName bridgeDllFilename = QueryBridgeLibraryPathByName(nullptr, bridgeName);

            WString version;
            getVersionInfoAsString(version, bridgeDllFilename);

            file->PrintfTo(false, L"%ls;%ls;%ls\n", bridgeName.c_str(), bridgeDllFilename.c_str(), version.c_str());
            }

        file->Close();
        file = nullptr;
        }


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

        auto stmt = m_stateDb.GetCachedStatement("INSERT INTO fwk_InstalledBridges (Name,BridgeLibraryPath,AffinityLibraryPath,IsPowerPlatformBased,BridgeAssetsDir) VALUES(?,?,?,?,?)");

        BeFileName bridgeLibraryPath, affinityLibraryPath, bridgeAssetsDir;
        WString isPowerPlatformBased;
        GetStringRegKey(bridgeLibraryPath, subKey, L"BridgeLibraryPath", L"");
        GetStringRegKey(affinityLibraryPath, subKey, L"AffinityLibraryPath", L"");
        GetStringRegKey(isPowerPlatformBased, subKey, L"IsPowerPlatformBased", L"");
        GetStringRegKey(bridgeAssetsDir, subKey, L"BridgeAssetsDir", L"");

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

        if (!bridgeAssetsDir.empty())
            stmt->BindText(5, Utf8String(bridgeAssetsDir).c_str(), Statement::MakeCopy::Yes);
        else
            stmt->BindNull(5);

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
    long result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, bridgePath.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &iModelBridgeKey);
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
    AssignCxxOptsResult(result, "affinity-db-name", args.m_affinityDbName);
    AssignCxxOptsResult(result, "fwk-logging-config-file", args.m_loggingConfigFileName);
    
    if (result.count("bridge-filter") > 0)
        {
        Utf8String filter = result["bridge-filter"].as<std::string>().c_str();
        filter.Trim();
        filter.DropQuotes();
        size_t offset = 0;
        Utf8String bridge;
        while ((offset = filter.GetNextToken (bridge, ":", offset)) != Utf8String::npos)
            {
            bridge.Trim();
            bridge.ToLower();
            args.m_bridgeFilter.insert(bridge);
            }
        }

    if (result.count("update-assignments") > 0)
        args.m_updateAssignments = result["update-assignments"].as<bool>();

    if (result.count("fwk-search-in-staging-dir") > 0)
        args.m_searchForFilesInStagingDir = result["fwk-search-in-staging-dir"].as<bool>();

    if (result.count("no-bridge-search") > 0)
        args.m_noBridgeSearch = result["no-bridge-search"].as<bool>();

    if (result.count("write-progress-to-stdout") > 0)
        args.m_writeProgressToStdout = result["write-progress-to-stdout"].as<bool>();

    if (result.count("write-progress-interval") > 0)
        args.m_writeProgressInterval = result["write-progress-interval"].as<uint32_t>();

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
        ("a,affinity-db-name", "The name of the affinity db to create within the registry-dir.", cxxopts::value<std::string>(),"<Optional>")
        ("l,fwk-logging-config-file", "The configuration file to be used for logging.", cxxopts::value<std::string>(), "<Optional>")
        ("x,fwk-search-in-staging-dir", "Search for files in fwk-staging-dir.", cxxopts::value<bool>(), "<Optional>")
        ("bridge-filter", "Consider only the specified bridges. Separate bridge names by colons.", cxxopts::value<std::string>(), "<Optional>")
        ("no-bridge-search", "Don't search for globally installed bridges.", cxxopts::value<bool>(), "<Optional>")
        ("write-progress-to-stdout", "Write progress messages to stdout. Defaults to false.", cxxopts::value<bool>(), "<Optional>")
        ("write-progress-interval", "Time between progress messages in milliseconds. Defaults to 500.", cxxopts::value<uint32_t>(), "<Optional>")
        ("update-assignments", "Update existing bridge-file assignments with better ones? Otherwise, stick with existing assignments.", cxxopts::value<bool>(), "<Optional>")
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

        if (BSISUCCESS != readEntireFile(contents, optionsFileName))
            {
            WString escapedName = optionsFileName;
            escapedName.ReplaceAll(L"\\", L"\\\\");
            std::wcerr << L"Cannot open or read options file '" << escapedName.c_str() << L"'" << std::endl;
            }

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
        LOG.fatal(WPrintfString(L"Staging directory '%ls' specified must exist.", args.m_stagingDir.c_str()).c_str());
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

    return app.RunAssign(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistryBase::RunAssign(AssignCmdLineArgs& args)
    {
    m_searchForFilesInStagingDir = args.m_searchForFilesInStagingDir;
    m_writeProgressToStdout = args.m_writeProgressToStdout;
    m_writeProgressInterval = args.m_writeProgressInterval;
    m_affinityDbName = args.m_affinityDbName;
    m_bridgeFilter = args.m_bridgeFilter;
    m_registryDir = args.m_registryDir;
    m_updateAssignments = args.m_updateAssignments;

    if (!args.m_noBridgeSearch)
        _DiscoverInstalledBridges();
    
    m_masterFilePath = args.m_inputFileName;
    SearchForBridgesToAssignToDocuments();
    m_stateDb.SaveChanges();

    WriteBridgesFile();

    m_stateDb.CloseDb();

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

    BeGuid testGuid;
    if (guid)
        testGuid = *guid;
#ifdef TEST_REGISTRY_FAKE_GUIDS
    else
        testGuid.Create();
#endif

    auto stmt = m_stateDb.GetCachedStatement("INSERT INTO DocumentProperties (LocalFilePath,DocGuid,DesktopURN,WebURN,AttributesJSON,SpatialRootTransformJSON) VALUES(?,?,'', '', '', '')");
    stmt->BindText(1, Utf8String(fn), Statement::MakeCopy::Yes);
    if (testGuid.IsValid())
        stmt->BindText(2, testGuid.ToString(), Statement::MakeCopy::Yes);
    else
        {
        stmt->BindNull(2);
        LOG.tracev(L"no guid for %ls", fn.c_str());
        }
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