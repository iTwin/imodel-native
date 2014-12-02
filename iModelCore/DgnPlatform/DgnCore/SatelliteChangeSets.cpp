/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SatelliteChangeSets.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnCore/SatelliteChangeSets.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Changes"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc = stmt; if (rc != RESULT) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

static bool s_traceUpdate = false;

static SchemaVersion s_currentVersion (0, 1, 0, 0);

USING_NAMESPACE_BENTLEY_SQLITE

namespace {
/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      07/14
+===============+===============+===============+===============+===============+======*/
struct SyncInfoProjectUpdaterChangeSet : ChangeSet
    {
    Db& m_db;

    SyncInfoProjectUpdaterChangeSet (Db& db) : m_db(db) {;}

    virtual ConflictResolution _OnConflict (ConflictCause cause, Changes::Change iter) override 
        {
        LOG.tracev ("Conflict \"%s\"\n", ChangeSet::InterpretConflictCause(cause).c_str());
        
        if (s_traceUpdate)
            iter.Dump (m_db);
        
        // The "data" conflict indicates that the current state of the row to be updated is different from 
        //  the row's state before this change was applied. That can only happen if:
        //  a) there are multiple users, and a second user changed the row independently, 
        //  b) the project has been regenerated since this changeset was created.
        //  c) this changeset is being applied out of order.
        // The "notfound" conflict indicates that the row is already gone. That indicates that
        //  somebody is confused and is applying changesets twice or out of order or something like that.
        // In all cases, applying this changeset is likely to corrupt the database from the application's
        //  point of view, so don't allow it.
        //return CONFLICT_RESOLUTION_Abort;
        return CONFLICT_RESOLUTION_Skip;
        }
    };


}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String uint64ToString (UInt64 value) {return Utf8PrintfString("%lld",value);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt64 uint64FromString (Utf8StringCR str) 
    {
    UInt64 value;
    if (BeStringUtilities::ParseUInt64 (value, str.c_str()) != BSISUCCESS)
        return 0;
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus expandData (bvector<byte>& buffer, void const* compresseddata, Int32 compressedsize, SatelliteChangeSets::Compressed compressOption)
    {
    LzmaOutToBvectorStream  bvectorWriter (buffer);

    BeFileLzmaInFromMemory memoryReader (compresseddata, (UInt32)compressedsize);

    LzmaDecoder decoder;
    ZipErrors result = decoder.Uncompress (bvectorWriter, memoryReader, true, NULL);

    return  ZIP_SUCCESS == result ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus compressData (bvector<byte>& buffer, void const* data, Int32 size, SatelliteChangeSets::Compressed compressOption)
    {
    LzmaOutToBvectorStream  bvectorWriter (buffer);
    
    BeFileLzmaInFromMemory memoryReader (data, (UInt32)size);
    
    LzmaEncoder encoder ((UInt32)size);
    ZipErrors result = encoder.Compress (bvectorWriter, memoryReader, NULL, false);
    
    return  ZIP_SUCCESS == result ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::SavePropertyString (PropertySpecCR spec, Utf8StringCR stringData, UInt64 id, UInt64 subId)
    {
    Statement stmt;
    auto rc = stmt.Prepare (*m_project, "INSERT OR REPLACE INTO " CHANGES_TABLE_Property " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    int col=1;
    stmt.BindText (col++, spec.GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText (col++, spec.GetName(), Statement::MAKE_COPY_No);
    stmt.BindInt64 (col++, id);
    stmt.BindInt64 (col++, subId);
    stmt.BindInt (col++, 0);
    stmt.BindText (col++, stringData, Statement::MAKE_COPY_No);
    rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::QueryProperty (Utf8StringR value, PropertySpecCR spec, UInt64 id, UInt64 subId) const
    {
    Statement stmt;
    DbResult rc = stmt.Prepare (*m_project, "SELECT StrData FROM " CHANGES_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
    if (BE_SQLITE_OK != rc)
        return rc;

    int col = 1;
    stmt.BindText (col++, spec.GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText (col++, spec.GetName(), Statement::MAKE_COPY_No);
    stmt.BindInt64 (col++, id);
    stmt.BindInt64 (col++, subId);
    rc = stmt.Step();
    if (BE_SQLITE_ROW != rc)
        return rc;

    value.AssignOrClear (stmt.GetValueText(0));
    return  BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       08/14
+---------------+---------------+---------------+---------------+---------------+------*/
SatelliteChangeSets::SatelliteChangeSets () 
    : m_lastError(BE_SQLITE_OK), m_isValid(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       08/14
+---------------+---------------+---------------+---------------+---------------+------*/
SatelliteChangeSets::~SatelliteChangeSets() 
    {
    if (m_project != NULL)
        m_project->DetachDb (CHANGES_ATTACH_ALIAS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       08/14
+---------------+---------------+---------------+---------------+---------------+------*/
SatelliteChangeSets::ChangeSetInfo::ChangeSetInfo() 
    :
    m_sequenceNumber(0),
    m_type(PatchSet)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       08/14
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8 SatelliteChangeSets::ChangeSetInfo::GetTypeAsInt() const {return (m_type==PatchSet)? 0: 1;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       08/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::ChangeSetInfo::SetTypeFromInt (UInt8 type) 
    {
    if (type == 0)
        m_type = PatchSet;
    else if (type == 1)
        m_type = ChangeSet;
    else
        {
        BeDataAssert(false && "invalid type identifier");
        return BSIERROR;    // Detect newer version above!
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::CreateTable (Db& db)
    {
    return db.CreateTable (CHANGES_TABLE_ChangeSet, "SequenceNumber INTEGER PRIMARY KEY, Type INT, Description CHAR, Time DATETIME, Data BLOB, Compressed INT");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::Insert (Db& db) const
    {
    Statement stmt;
    stmt.Prepare (db, "INSERT INTO " CHANGES_TABLE_ChangeSet " (SequenceNumber,Type,Description,Time) VALUES (?,?,?,?)");
    int col = 1;
    stmt.BindInt64(col++, m_sequenceNumber);                                        // SequenceNumber
    stmt.BindInt  (col++, GetTypeAsInt());                                          // Type
    stmt.BindText (col++, m_description, Statement::MAKE_COPY_No);                  // Description
    stmt.BindText (col++, Utf8String(m_time.ToString()), Statement::MAKE_COPY_Yes); // Time
    return stmt.Step();
    }

#define CHANGSETINFO_COLS "SequenceNumber,Type,Description,Time"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::Step (Statement& stmt)
    {
    auto result = stmt.Step();
    if (BE_SQLITE_ROW != result)
        return result;

    int col = 0;
    m_sequenceNumber = stmt.GetValueInt64   (col++);    // SequenceNumber
    SetTypeFromInt ((UInt8) stmt.GetValueInt(col++));   // Type
    m_description   = stmt.GetValueText     (col++);    // Description
    DateTime::FromString (m_time, stmt.GetValueText (col++));    // Time

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::Prepare (Statement& stmt, SatelliteChangeSets& db)
    {
    stmt.Finalize();
    return stmt.Prepare ((*db.m_project), "SELECT " CHANGSETINFO_COLS " FROM " CHANGES_TABLE_ChangeSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::PrepareFindBySequenceNumber (Statement& stmt, SatelliteChangeSets& db, UInt64 cid)
    {
    stmt.Finalize();
    auto result = stmt.Prepare ((*db.m_project), "SELECT " CHANGSETINFO_COLS " FROM " CHANGES_TABLE_ChangeSet" WHERE (SequenceNumber=?)");
    stmt.BindInt64 (1, cid);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::PrepareFindLater (Statement& stmt, SatelliteChangeSets& db, UInt64 cid)
    {
    stmt.Finalize();
    auto result = stmt.Prepare ((*db.m_project), "SELECT " CHANGSETINFO_COLS " FROM " CHANGES_TABLE_ChangeSet" WHERE (SequenceNumber>?)");
    stmt.BindInt64 (1, cid);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::StepFindBySequenceNumber (Statement& stmt)
    {
    return Step(stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::ChangeSetInfo::StepFindLater (Statement& stmt)
    {
    return Step(stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::CreateTables()
    {
    return ChangeSetInfo::CreateTable (*m_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SatelliteChangeSets::GetChangeSetFileName (Db& project, UInt64 csid, bool useProjectGuid)
    {
    BeFileName dataFileName;
    if (useProjectGuid)
        dataFileName.SetNameA (project.GetDbGuid().ToString().c_str());
    else
        dataFileName.SetNameA (project.GetDbFileName());
    dataFileName.append (WPrintfString(L".%lld.changes",csid));
    return dataFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime toUtc (DateTime const& timeIn)
    {
    DateTime time;
    
    if (DateTime::Compare (timeIn, time) == DateTime::CompareResult::Equals)
        return timeIn;

    if (timeIn.GetInfo().GetKind () != DateTime::Kind::Local)
        return timeIn;
        
    timeIn.ToUtc (time);
    return time;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::InsertChangeSet (ChangeSetInfo& csinfo, UInt64 csid, Compressed compressOption, void const* data, Int32 datasize, ChangeSetType type, Utf8StringCR desc, DateTime const& timeIn)
    {
    Db& db = *m_project;

    //  Fill out the new ChangeSetInfo to insert
    csinfo.m_sequenceNumber = csid;
    csinfo.m_type = type;
    csinfo.m_description = desc;
    csinfo.m_time = toUtc(timeIn);

    // Insert ChangeSet row
    if (csinfo.Insert (db) != BE_SQLITE_DONE)
        return BSIERROR;

    // store the changeset data (optionally compressed) in a column of the new row
    void const* dataToStore = data;
    Int32 dataToStoreSize = datasize;
    bvector<byte> compressedData;
    if (compressOption != COMPRESSED_No)
        {
        compressData (compressedData, data, datasize, compressOption);
        dataToStore = &compressedData[0];
        dataToStoreSize = (Int32)compressedData.size();
        }

    Statement stmt;
    stmt.Prepare (db, "UPDATE " CHANGES_TABLE_ChangeSet " SET Data=?, Compressed=? WHERE (SequenceNumber=?)");
    stmt.BindBlob (1, dataToStore, dataToStoreSize, Statement::MAKE_COPY_No);
    stmt.BindInt (2, compressOption);
    stmt.BindInt64 (3, csinfo.m_sequenceNumber);
    MUSTBEDONE (stmt.Step());

    // Update my range
    UpdateChangeSetSequenceNumberRange (csid);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::ExtractChangeSetBySequenceNumber (BeSQLite::ChangeSet& cset, UInt64 cid)
    {
    Statement stmt;
    stmt.Prepare (*m_project, "SELECT Data,length(Data),Compressed FROM " CHANGES_TABLE_ChangeSet " WHERE (SequenceNumber=?)");
    stmt.BindInt64 (1, cid);
    if (stmt.Step() != BE_SQLITE_ROW)
        return BSIERROR;

    void const* data = stmt.GetValueBlob(0);
    int datasize = stmt.GetValueInt(1);
    Compressed compressed = (Compressed)(stmt.GetValueInt(2));

    bvector<byte> expanded;
    if (compressed != COMPRESSED_No)
        {
        if (expandData (expanded, data, datasize, compressed) != BSISUCCESS)
            return BSIERROR;
        data = &expanded[0];
        datasize = (Int32)expanded.size();
        }

    MUSTBEOK (cset.FromData (datasize, data, false));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::OnFatalError (UInt64 lastCsid)
    {
    Statement stmt;
    stmt.Prepare (*m_project, "DELETE FROM " CHANGES_TABLE_ChangeSet " WHERE (SequenceNumber=?)");
    stmt.BindInt64 (1, lastCsid);
    MUSTBEROW (stmt.Step());

    m_project->SaveChanges();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::FindChangeSetBySequenceNumber (ChangeSetInfo& csinfo, UInt64 cid)
    {
    Statement stmt;
    ChangeSetInfo::PrepareFindBySequenceNumber (stmt, *this, cid);
    if (csinfo.StepFindBySequenceNumber (stmt) != BE_SQLITE_ROW)
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::DetectChangeSets (T_ChangesFileDictionary& csfiles, ChangeSetRange& range, Db& project, bvector<BeFileName> const& candidateFiles)
    {
    auto projectGuid = project.GetDbGuid();

    ProjectChangeSetProperties targetProjectProperties;
    targetProjectProperties.GetChangeSetProperties (project);

    range.m_earliest = UINT64_MAX;
    range.m_latest = 0;
    for (auto const& csFileName : candidateFiles)
        {
        SatelliteChangeSets csfile;
        
        if (csfile.AttachToProject (project, csFileName) != BSISUCCESS)             // WARNING - Attaching a db commits the current txn!
            continue;

        //  Only select changes files that apply to this project!
        if (csfile.GetTargetProjectGuid() != projectGuid)
            continue;

        //  Only select changes that we haven't seen before.
        auto endsWith = csfile.GetLastChangeSetSequenceNumber();
        if (endsWith <= targetProjectProperties.m_latestChangeSetId)
            continue;

        auto startsWith = csfile.GetFirstChangeSetSequenceNumber();

        if (csfiles.find (startsWith) != csfiles.end())
            {
            LOG.errorv (L"DetectChangeSets - %ls - duplicate changeset sequence numbers found. Something is badly wrong!", csFileName.c_str());
            return BSIERROR;
            }

        csfiles[startsWith] = csFileName;

        if (startsWith < range.m_earliest)
            range.m_earliest = startsWith;

        if (endsWith > range.m_latest)
            range.m_latest = endsWith;
        }                                                                       // WARNING - ~SatelliteChangeSets detaches, which commits the current txn!

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::ApplyChangeSets (UInt32& nChangesApplied, Db& project, bvector<BeFileName> const& csfilesIn, IApplyChangeSet& applier)
    {
    nChangesApplied = 0;

    if (!project.IsDbOpen() || project.IsReadonly())
        {
        LOG.errorv ("ApplyChangeSets - project %s is not open for read-write", project.GetDbFileName());
        return BSIERROR;
        }

    //  ----------------------------------------
    //  Get the latest changeset known to the project
    //  ----------------------------------------
    ProjectChangeSetProperties targetProjectProperties;
    targetProjectProperties.GetChangeSetProperties(project);

    //  ----------------------------------------
    // Pass 1: Detect the changes files that apply to this project and that come after targetProjectProperties.m_sequenceNumber
    //  ----------------------------------------
    T_ChangesFileDictionary dictionary;
    ChangeSetRange range;
    if (DetectChangeSets (dictionary, range, project, csfilesIn) != BSISUCCESS)
        return BSIERROR;

    //  ----------------------------------------
    //  Quick return if no applicable changes found
    //  ----------------------------------------
    if (dictionary.empty())
        return BSISUCCESS;

    //  ----------------------------------------
    //  Integrity checks
    //  ----------------------------------------
    // The first new change must come right after the last known change.
    if (range.m_earliest != (targetProjectProperties.m_latestChangeSetId+1))
        {
        LOG.errorv ("ApplyChangeSets - project %s - LatestChangeSetId %lld + 1 is not in the range to be applied %lld-%lld.", project.GetDbFileName(), targetProjectProperties.m_latestChangeSetId, range.m_earliest, range.m_latest);
        return BSIERROR;
        }

    //  ----------------------------------------
    // Pass 2: Apply relevant changesets in sequence
    //  ----------------------------------------
    ProjectChangeSetProperties csprops;
    
    UInt64 endOfPrevious = UINT64_MAX;
    BeFileName nameOfPrevious;

    for (auto const& record : dictionary)
        {
        auto const& csFileName = record.second;

        SatelliteChangeSets csfile;
        if (csfile.AttachToProject (project, csFileName) != BSISUCCESS)        // WARNING - Attaching a db commits the current txn!
            continue;

        if (endOfPrevious != UINT64_MAX)
            {
            if ((endOfPrevious+1) != csfile.GetFirstChangeSetSequenceNumber())
                {
                // The set of changes to apply must not skip any sequence numbers
                LOG.errorv (L"ApplyChangeSets - A .changes file is missing. [%ls] does not lead directly to [%ls]. Stopping.", nameOfPrevious.c_str(), csFileName.c_str());
                break;
                }
            }

        endOfPrevious = csfile.GetLastChangeSetSequenceNumber();
        nameOfPrevious = csFileName;

        csprops.m_latestChangeSetId = csfile.GetLastChangeSetSequenceNumber();
        csfile.GetExpirationDate (csprops.m_expirationDate);

        ChangeSetInfo info;
        BeSQLite::Statement stmt;
        ChangeSetInfo::Prepare (stmt, csfile);
        while (info.Step (stmt) == BE_SQLITE_ROW)
            {
            SyncInfoProjectUpdaterChangeSet changeSet (*csfile.m_project);
            csfile.ExtractChangeSetBySequenceNumber (changeSet, info.m_sequenceNumber);

            if (s_traceUpdate)
                changeSet.Dump (project);

            DbResult applyResult = applier.ApplyChangeSet (project, changeSet);
            if (applyResult != BE_SQLITE_OK)
                {
                BeAssert(false);
                LOG.errorv ("ApplyChangeSets - changeset  %ls ApplyChanges failed with code %s", csFileName.c_str(), BeSQLite::Db::InterpretDbResult(applyResult));
                project.AbandonChanges();
                return BSIERROR;
                }

            // commit as we go along. This keeps down memory usage, and it allows us to re-start with first un-applied chagneset.
            csprops.m_latestChangeSetId = info.m_sequenceNumber;
            if (csprops.SetChangeSetProperties (project) != BSISUCCESS)
                {
                project.AbandonChanges();
                return BSIERROR;
                }

            auto saveResult = project.SaveChanges();
            if (saveResult != BE_SQLITE_OK)
                {
                LOG.errorv ("ApplyChangeSets - project %s - SaveChanges failed with code %s.", project.GetDbFileName(), BeSQLite::Db::InterpretDbResult(saveResult));
                project.AbandonChanges();
                return BSIERROR;
                }

            ++nChangesApplied;
            }
        }                                                               // WARNING - ~SatelliteChangeSets detaches, which commits the current txn!

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::ApplyChangeSets (UInt32& nChangesApplied, Db& project, T_ChangesFileDictionary const& csfilesIn, IApplyChangeSet& applier)
    {
    bvector<BeFileName> files;
    for (auto const& entry : csfilesIn)
        files.push_back (entry.second);
    return ApplyChangeSets (nChangesApplied, project, files, applier);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
BentleyStatus SatelliteChangeSets::PerformVersionChecks()
    {
    //  ---------------------------------------------------------------------
    //  Look at the stored version and see if we have to upgrade
    //  ---------------------------------------------------------------------
    Utf8String versionString;
    MUSTBEROW (QueryProperty (versionString, ChangesProperty::SchemaVersion()));

    SchemaVersion storedVersion (0,0,0,0);
    storedVersion.FromJson (versionString.c_str());
        
    if (storedVersion.CompareTo (s_currentVersion) == 0)
        return BSISUCCESS;

    if (storedVersion.CompareTo (s_currentVersion) > 0)
        { // version is too new!
        LOG.errorv ("compatibility error - storedVersion=%s > currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return BSIERROR;
        }

    //  ---------------------------------------------------------------------
    //  Upgrade
    //  ---------------------------------------------------------------------

    //      when we change the syncInfo schema, add upgrade steps here ...
    
    //  Upgraded. Update the stored version.
    MUSTBEOK (SavePropertyString (ChangesProperty::SchemaVersion(), s_currentVersion.ToJson().c_str()));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Db* SatelliteChangeSets::GetProject()
    {
    return m_project;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::CreateEmptyFile (BeFileNameCR dbNameIn, bool deleteIfExists)
    {
    BeFileName dbName (dbNameIn);
    if (dbName.GetExtension().empty())
        dbName.append (L".changes");

    if (deleteIfExists)
        dbName.BeDeleteFile();

    Db bootStrapDb;
    auto rc = bootStrapDb.CreateNewDb (Utf8String(dbName).c_str());
    if (rc != BE_SQLITE_OK)
        {
        LOG.errorv ("%s - cannot create. Error code=%s", Utf8String(dbName).c_str(), Db::InterpretDbResult(rc));
        return BSIERROR;
        }

    bootStrapDb.CloseDb();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::OnAttach (Db& targetProject, BeSQLite::SchemaVersion const& targetSchemaVersion)
    {
    m_project = &targetProject;

    if (!m_project->TableExists (CHANGES_TABLE_DIRECT_ChangeSet))
        {
        //  -----------------------------------------
        //  We are creating a new .changes file
        //  -----------------------------------------
        Utf8String currentProjectDbSchemaVersion;
        m_project->QueryProperty (currentProjectDbSchemaVersion, Properties::SchemaVersion());

        MUSTBEOK (SavePropertyString (ChangesProperty::SchemaVersion(), s_currentVersion.ToJson()));
        MUSTBEOK (SavePropertyString (Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToUtf8String()));
        MUSTBEOK (SavePropertyString (ChangesProperty::ProjectGUID(), m_project->GetDbGuid().ToString()));
        MUSTBEOK (SavePropertyString (ChangesProperty::ProjectDbSchemaVersion(), currentProjectDbSchemaVersion));
        MUSTBEOK (SavePropertyString (ChangesProperty::ProjectSchemaVersion(), targetSchemaVersion.ToJson()));
        // *** WIP_CONVERTER - I'd like to save project's last save time

        MUSTBEOK (CreateTables());
    
        SetValid (true);

        return BSISUCCESS;
        }

    //  -----------------------------------------
    //  We are opening an existing .changes file
    //  -----------------------------------------
    if (PerformVersionChecks() != BSISUCCESS)
        return BSIERROR;

    if (GetTargetProjectGuid() != m_project->GetDbGuid())
        {
        LOG.errorv ("compatibility error - storedProjectGUID=%s, specifiedProjectGUID=%s", GetTargetProjectGuid().ToString().c_str(), m_project->GetDbGuid().ToString().c_str());
        return BSIERROR;
        }

    Utf8String savedProjectDbSchemaVersion, currentProjectDbSchemaVersion;
    if (QueryProperty (savedProjectDbSchemaVersion, ChangesProperty::ProjectDbSchemaVersion()) != BE_SQLITE_ROW
        || m_project->QueryProperty (currentProjectDbSchemaVersion, Properties::SchemaVersion()) !=  BE_SQLITE_ROW
        || !savedProjectDbSchemaVersion.Equals (currentProjectDbSchemaVersion))
        {
        LOG.warningv ("compatibility error. ProjectDbSchemaVersion=%s does not match project SchemaVersion=%s.",
                                savedProjectDbSchemaVersion.c_str(), currentProjectDbSchemaVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away changes whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    if (targetSchemaVersion.GetInt64 (SchemaVersion::VERSION_All) != 0)
        {
        Utf8String savedProjectSchemaVersion;
        QueryProperty (savedProjectSchemaVersion, ChangesProperty::ProjectSchemaVersion());
        if (targetSchemaVersion.ToJson() != savedProjectSchemaVersion)
            {
            LOG.warningv ("compatibility error - savedProjectSchemaVersion=%s, specifiedProjectSchemaVersion=%s", savedProjectSchemaVersion.c_str(), targetSchemaVersion.ToJson().c_str());
            // *** WIP_CONVERTER - Do we really have to throw away changes whenever we make a trivial schema change?
            return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
            }
        }

    SetValid (true);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::AttachToProject (Db& targetProject, BeFileNameCR dbName)
    {
    m_dbFileName = dbName;
    MUSTBEOK (targetProject.AttachDb (Utf8String(dbName).c_str(), CHANGES_ATTACH_ALIAS));
    return OnAttach (targetProject, SchemaVersion(0,0,0,0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeDbGuid SatelliteChangeSets::GetTargetProjectGuid()
    {
    Utf8String projectGuidStr;
    QueryProperty (projectGuidStr, ChangesProperty::ProjectGUID());
    BeDbGuid guid;
    guid.FromString (projectGuidStr.c_str());
    return guid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64 SatelliteChangeSets::GetFirstChangeSetSequenceNumber()
    {
    Utf8String str;
    QueryProperty (str, ChangesProperty::FirstChangeSetSequenceNumber());
    return uint64FromString (str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64 SatelliteChangeSets::GetLastChangeSetSequenceNumber()
    {
    Utf8String str;
    QueryProperty (str, ChangesProperty::LastChangeSetSequenceNumber());
    return uint64FromString (str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::UpdateChangeSetSequenceNumberRange (UInt64 csid)
    {
    if (csid < GetFirstChangeSetSequenceNumber() || 0==GetFirstChangeSetSequenceNumber())
        SavePropertyString (ChangesProperty::FirstChangeSetSequenceNumber(), uint64ToString(csid));
    if (csid > GetLastChangeSetSequenceNumber())
        SavePropertyString (ChangesProperty::LastChangeSetSequenceNumber(), uint64ToString(csid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::SetExpirationDate (DateTime const& expirationDate)
    {
    MUSTBEOK (SavePropertyString (Properties::ExpirationDate(), expirationDate.ToUtf8String()));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::GetExpirationDate (DateTime& dt)
    {
    Utf8String str;
    QueryProperty (str, Properties::ExpirationDate());
    DateTime::FromString (dt, str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::SetLastError (BeSQLite::DbResult rc)
    {
    m_lastError = rc;
    m_lastErrorDescription = m_project->GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::GetLastError (BeSQLite::DbResult& result, Utf8String& descr)
    {
    result = m_lastError;
    descr = m_lastErrorDescription;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::SaveChanges()
    {
    if (!m_project->IsDbOpen())
        return BSISUCCESS;

    MUSTBEOK (m_project->SaveChanges());
    
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::AbandonChanges()
    {
    m_project->AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::Close()
    {
    m_project->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ProjectChangeSetProperties::SetChangeSetProperties (Db& project)
    {
    auto saveResult = project.SavePropertyString (Properties::LatestChangeSetId(), uint64ToString(m_latestChangeSetId));
    if (saveResult != BE_SQLITE_OK)
        {
        LOG.errorv ("ApplyChangeSets - project %s - save LatestChangeSetId failed with code %s.", project.GetDbFileName(), BeSQLite::Db::InterpretDbResult(saveResult));
        return BSIERROR;
        }

    saveResult = project.SavePropertyString (Properties::ExpirationDate(), m_expirationDate.ToUtf8String());
    if (saveResult != BE_SQLITE_OK)
        {
        LOG.errorv ("ApplyChangeSets - project %s - save ExpirationDate failed with code %s.", project.GetDbFileName(), BeSQLite::Db::InterpretDbResult(saveResult));
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ProjectChangeSetProperties::GetChangeSetProperties (Db& project)
    {
    Utf8String str;
    if (project.QueryProperty (str, Properties::LatestChangeSetId()) != BeSQLite::BE_SQLITE_ROW)
        return BSIERROR;
    m_latestChangeSetId = uint64FromString (str);

    Utf8String expirationDateString;
    project.QueryProperty (expirationDateString, Properties::ExpirationDate());
    DateTime::FromString (m_expirationDate, expirationDateString.c_str());

    return BSISUCCESS;
    }
