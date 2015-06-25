/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SatelliteChangeSets.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

#include <DgnPlatform/DgnCore/SatelliteChangeSets.h>
#include <Bentley/BeDirectoryIterator.h>

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc = stmt; if (rc != RESULT) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

BEGIN_UNNAMED_NAMESPACE

bool s_traceUpdate = false;
SchemaVersion s_currentVersion(2, 0, 0, 0);

/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      07/14
+===============+===============+===============+===============+===============+======*/
struct SyncInfoChangeSet : ChangeSet
{
    Db& m_db;
    bool m_isPatchSet;

    SyncInfoChangeSet(Db& db, bool ispatch) : m_db(db), m_isPatchSet(ispatch) {}

    virtual ConflictResolution _OnConflict(ConflictCause cause, Changes::Change iter) override 
        {
        LOG.tracev("Conflict \"%s\"\n", ChangeSet::InterpretConflictCause(cause).c_str());
        
        if (s_traceUpdate)
            iter.Dump(m_db, m_isPatchSet, 0);
        
        // The "data" conflict indicates that the current state of the row to be updated is different from 
        //  the row's state before this change was applied. That can only happen if:
        //  a) there are multiple users, and a second user changed the row independently, 
        //  b) the project has been regenerated since this changeset was created.
        //  c) this changeset is being applied out of order.
        // The "notfound" conflict indicates that the row is already gone. That indicates that
        //  somebody is confused and is applying changesets twice or out of order or something like that.
        // In all cases, applying this changeset is likely to corrupt the database from the application's
        //  point of view, so don't allow it.
        return ConflictResolution::Skip;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String uint64ToString(uint64_t value) {return Utf8PrintfString("%lld",value);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t uint64FromString(Utf8StringCR str) 
    {
    uint64_t value;
    return (BSISUCCESS != BeStringUtilities::ParseUInt64(value, str.c_str())) ? 0 : value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus expandData(bvector<Byte>& buffer, void const* compresseddata, int32_t compressedsize, SatelliteChangeSets::Compressed compressOption)
    {
    LzmaOutToBvectorStream  bvectorWriter(buffer);

    BeFileLzmaInFromMemory memoryReader(compresseddata, (uint32_t)compressedsize);

    LzmaDecoder decoder;
    ZipErrors result = decoder.Uncompress(bvectorWriter, memoryReader, true, NULL);

    return  ZIP_SUCCESS == result ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus compressData(bvector<Byte>& buffer, void const* data, int32_t size, SatelliteChangeSets::Compressed compressOption)
    {
    LzmaOutToBvectorStream  bvectorWriter(buffer);
    
    BeFileLzmaInFromMemory memoryReader(data, (uint32_t)size);
    
    LzmaEncoder encoder((uint32_t)size);
    ZipErrors result = encoder.Compress(bvectorWriter, memoryReader, NULL, false);
    
    return  ZIP_SUCCESS == result ? BSISUCCESS : BSIERROR;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::SavePropertyString(PropertySpecCR spec, Utf8StringCR stringData, uint64_t id, uint64_t subId)
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(*m_dgndb, "INSERT OR REPLACE INTO " CHANGESET_ATTACH(BEDB_TABLE_Property) " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    int col=1;
    stmt.BindText(col++, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(col++, spec.GetName(), Statement::MakeCopy::No);
    stmt.BindInt64(col++, id);
    stmt.BindInt64(col++, subId);
    stmt.BindInt(col++, 0);
    stmt.BindText(col++, stringData, Statement::MakeCopy::No);
    rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(*m_dgndb, "SELECT StrData FROM " CHANGESET_ATTACH(BEDB_TABLE_Property) " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
    if (BE_SQLITE_OK != rc)
        return rc;

    int col = 1;
    stmt.BindText(col++, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(col++, spec.GetName(), Statement::MakeCopy::No);
    stmt.BindInt64(col++, id);
    stmt.BindInt64(col++, subId);
    rc = stmt.Step();
    if (BE_SQLITE_ROW != rc)
        return rc;

    value.AssignOrClear(stmt.GetValueText(0));
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       08/14
+---------------+---------------+---------------+---------------+---------------+------*/
SatelliteChangeSets::~SatelliteChangeSets() 
    {
    if (nullptr != m_dgndb)
        m_dgndb->DetachDb(CHANGES_ATTACH_ALIAS);
    }

#define CHANGSETINFO_COLS "SequenceNumber,Type,Description,Time"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SatelliteChangeSets::ChangeSetInfo::ChangeSetInfo(Statement& stmt)
    {
    if (BE_SQLITE_ROW != stmt.Step())
        {
        m_isValid = false;
        return;
        }

    m_isValid = true;

    int col = 0;
    m_sequenceNumber = stmt.GetValueInt64(col++);    // SequenceNumber
    m_type = (ChangeSetType) stmt.GetValueInt(col++);   // Type
    m_description   = stmt.GetValueText(col++);    // Description
    DateTime::FromString(m_time, stmt.GetValueText(col++));    // Time
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SatelliteChangeSets::CreateTables()
    {
    return m_dgndb->CreateTable(CHANGESET_ATTACH(CHANGESET_Table), "SequenceNumber INTEGER PRIMARY KEY,Type INT,Description CHAR,Time DATETIME,Data BLOB,Compressed INT");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SatelliteChangeSets::GetChangeSetFileName(Db& db, uint64_t csid, bool useGuid)
    {
    BeFileName dataFileName;
    if (useGuid)
        dataFileName.SetNameUtf8(db.GetDbGuid().ToString().c_str());
    else
        dataFileName.SetNameUtf8(db.GetDbFileName());

    dataFileName.append(WPrintfString(L".%lld.changes", csid));
    return dataFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime toUtc(DateTime const& timeIn)
    {
    if (!timeIn.IsValid())
        return DateTime::GetCurrentTimeUtc();

    if (timeIn.GetInfo().GetKind() != DateTime::Kind::Local)
        return timeIn;

    DateTime time;
    timeIn.ToUtc(time);
    return time;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::InsertChangeSet(ChangeSetInfo const &infoIn, Compressed compressOption, void const* data, int32_t datasize)
    {
    ChangeSetInfo info(infoIn);
    info.m_time = toUtc(info.m_time);

    // store the changeset data (optionally compressed) in a column of the new row
    void const* dataToStore = data;
    int32_t dataToStoreSize = datasize;
    bvector<Byte> compressedData;
    if (compressOption != Compressed::No)
        {
        compressData(compressedData, data, datasize, compressOption);
        dataToStore = &compressedData[0];
        dataToStoreSize = (int32_t)compressedData.size();
        }

    Statement stmt;
    stmt.Prepare(*m_dgndb, "INSERT INTO " CHANGESET_ATTACH(CHANGESET_Table) " (SequenceNumber,Type,Description,Time,Data,Compressed) VALUES (?,?,?,?,?,?)");
    int col = 1;
    stmt.BindInt64(col++, info.m_sequenceNumber);   
    stmt.BindInt(col++, (int) info.m_type);         
    stmt.BindText(col++, info.m_description, Statement::MakeCopy::No); 
    stmt.BindText(col++, Utf8String(info.m_time.ToString()), Statement::MakeCopy::Yes); 
    stmt.BindBlob(col++, dataToStore, dataToStoreSize, Statement::MakeCopy::No);
    stmt.BindInt(col++, (int) compressOption);

    auto rc = stmt.Step();
    if (BE_SQLITE_DONE != rc)
        return BSIERROR;

    // Update my range
    UpdateSequenceNumberRange(info.m_sequenceNumber);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::ExtractChangeSetBySequenceNumber(BeSQLite::ChangeSet& cset, uint64_t sequenceNumber)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "SELECT Data,length(Data),Compressed FROM " CHANGESET_ATTACH(CHANGESET_Table) " WHERE (SequenceNumber=?)");
    stmt.BindInt64(1, sequenceNumber);
    if (stmt.Step() != BE_SQLITE_ROW)
        return BSIERROR;

    void const* data = stmt.GetValueBlob(0);
    int datasize = stmt.GetValueInt(1);
    Compressed compressed = (Compressed)(stmt.GetValueInt(2));

    bvector<Byte> expanded;
    if (compressed != Compressed::No)
        {
        if (expandData(expanded, data, datasize, compressed) != BSISUCCESS)
            return BSIERROR;
        data = &expanded[0];
        datasize = (int32_t)expanded.size();
        }

    MUSTBEOK (cset.FromData(datasize, data, false));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus extractChangeSetBySequenceNumberDirect(BeSQLite::ChangeSet& cset, Db& db, uint64_t sequenceNumber)
    {
    Statement stmt;
    stmt.Prepare(db, "SELECT Data,length(Data),Compressed FROM " CHANGESET_Table " WHERE (SequenceNumber=?)");
    stmt.BindInt64(1, sequenceNumber);
    if (stmt.Step() != BE_SQLITE_ROW)
        return BSIERROR;

    void const* data = stmt.GetValueBlob(0);
    int datasize = stmt.GetValueInt(1);
    SatelliteChangeSets::Compressed compressed = (SatelliteChangeSets::Compressed)(stmt.GetValueInt(2));

    bvector<Byte> expanded;
    if (compressed != SatelliteChangeSets::Compressed::No)
        {
        if (expandData(expanded, data, datasize, compressed) != BSISUCCESS)
            return BSIERROR;
        data = &expanded[0];
        datasize = (int32_t)expanded.size();
        }

    DbResult rc = cset.FromData(datasize, data, false);
    return (rc == BE_SQLITE_OK)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::OnFatalError(uint64_t lastCsid)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "DELETE FROM " CHANGESET_ATTACH(CHANGESET_Table) " WHERE (SequenceNumber=?)");
    stmt.BindInt64(1, lastCsid);
    MUSTBEROW (stmt.Step());

    m_dgndb->SaveChanges();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SatelliteChangeSets::ChangeSetInfo SatelliteChangeSets::FindChangeSetBySequenceNumber(uint64_t sequenceNumber)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "SELECT " CHANGSETINFO_COLS " FROM " CHANGESET_ATTACH(CHANGESET_Table) " WHERE (SequenceNumber=?)");
    stmt.BindInt64(1, sequenceNumber);

    return ChangeSetInfo(stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::DetectChangeSets(T_ChangesFileDictionary& csfiles, ChangeSetRange& range, Db& db, bvector<BeFileName> const& candidateFiles)
    {
    auto dbGuid = db.GetDbGuid();

    ChangeSetProperties targetProperties;
    targetProperties.LoadFromDb(db);

    range.m_earliest = UINT64_MAX;
    range.m_latest = 0;
    for (auto const& csFileName : candidateFiles)
        {
        SatelliteChangeSets csfile;
        
        if (BSISUCCESS != csfile.AttachToDb(db, csFileName))    // WARNING - Attaching a db commits the current txn!
            continue;

        //  Only select changes files that apply to this project!
        if (dbGuid != csfile.GetTargetDbGuid())
            continue;

        //  Only select changes that we haven't seen before.
        uint64_t endsWith = csfile.GetLastSequenceNumber();
        if (endsWith <= targetProperties.m_latestChangeSetId)
            continue;

        uint64_t startsWith = csfile.GetFirstSequenceNumber();

        if (csfiles.find(startsWith) != csfiles.end())
            {
            LOG.errorv(L"DetectChangeSets - %ls - duplicate changeset sequence numbers found. Something is badly wrong!", csFileName.c_str());
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
BentleyStatus SatelliteChangeSets::ApplyChangeSets(uint32_t& nChangesApplied, Db& db, bvector<BeFileName> const& csfilesIn)
    {
    nChangesApplied = 0;

    if (!db.IsDbOpen() || db.IsReadonly())
        {
        LOG.errorv("ApplyChangeSets - Db %s is not open for read-write", db.GetDbFileName());
        return BSIERROR;
        }

    //  Get the latest changeset known to the project
    ChangeSetProperties targetProperties;
    targetProperties.LoadFromDb(db);

    // Pass 1: Detect the changes files that apply to this project and that come after targetProjectProperties.m_sequenceNumber
    T_ChangesFileDictionary dictionary;
    ChangeSetRange range;
    if (DetectChangeSets(dictionary, range, db, csfilesIn) != BSISUCCESS)
        return BSIERROR;

    if (dictionary.empty()) //  Quick return if no applicable changes found
        return BSISUCCESS;

    //  Integrity checks
    // The first new change must come right after the last known change.
    if (range.m_earliest != (targetProperties.m_latestChangeSetId+1))
        {
        LOG.errorv("ApplyChangeSets - Db %s - LatestChangeSetId %lld + 1 is not in the range to be applied %lld-%lld.", db.GetDbFileName(), targetProperties.m_latestChangeSetId, range.m_earliest, range.m_latest);
        return BSIERROR;
        }

    // Pass 2: Apply relevant changesets in sequence
    ChangeSetProperties csprops;
    
    uint64_t endOfPrevious = UINT64_MAX;
    BeFileName nameOfPrevious;

    for (auto const& record : dictionary)
        {
        auto const& csFileName = record.second;

        SatelliteChangeSets csfile;
        if (SUCCESS != csfile.AttachToDb(db, csFileName))        // WARNING - Attaching a db commits the current txn!
            continue;

        if (endOfPrevious != UINT64_MAX)
            {
            if ((endOfPrevious+1) != csfile.GetFirstSequenceNumber())
                {
                // The set of changes to apply must not skip any sequence numbers
                LOG.errorv(L"ApplyChangeSets - A .changes file is missing. [%ls] does not lead directly to [%ls]. Stopping.", nameOfPrevious.c_str(), csFileName.c_str());
                break;
                }
            }

        endOfPrevious = csfile.GetLastSequenceNumber();
        nameOfPrevious = csFileName;

        csprops.m_latestChangeSetId = csfile.GetLastSequenceNumber();
        csfile.GetExpirationDate(csprops.m_expirationDate);

        BeSQLite::Statement stmt;
        stmt.Prepare(db, "SELECT " CHANGSETINFO_COLS " FROM " CHANGESET_ATTACH(CHANGESET_Table));

        while (true)
            {
            ChangeSetInfo info(stmt);
            if (!info.IsValid())
                break;

            SyncInfoChangeSet changeSet(db, (ChangeSetType::Patch==info.m_type));
            csfile.ExtractChangeSetBySequenceNumber(changeSet, info.m_sequenceNumber);

            if (s_traceUpdate)
                changeSet.Dump("", db, (ChangeSetType::Patch==info.m_type), 0);

            DbResult applyResult = changeSet.ApplyChanges(db);
            if (applyResult != BE_SQLITE_OK)
                {
                BeAssert(false);
                LOG.errorv("ApplyChangeSets - changeset  %ls ApplyChanges failed with code %s", csFileName.c_str(), BeSQLite::Db::InterpretDbResult(applyResult));
                db.AbandonChanges();
                return BSIERROR;
                }

            // commit as we go along. This keeps down memory usage, and it allows us to re-start with first un-applied chagneset.
            csprops.m_latestChangeSetId = info.m_sequenceNumber;
            if (SUCCESS != csprops.SaveToDb(db))
                {
                db.AbandonChanges();
                return BSIERROR;
                }

            auto saveResult = db.SaveChanges();
            if (saveResult != BE_SQLITE_OK)
                {
                LOG.errorv("ApplyChangeSets - Db %s - SaveChanges failed with code %s.", db.GetDbFileName(), BeSQLite::Db::InterpretDbResult(saveResult));
                db.AbandonChanges();
                return BSIERROR;
                }

            ++nChangesApplied;
            }
        }  // WARNING - ~SatelliteChangeSets detaches, which commits the current txn!

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
-+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::ApplyChangeSets(uint32_t& nChangesApplied, Db& project, T_ChangesFileDictionary const& csfilesIn)
    {
    bvector<BeFileName> files;
    for (auto const& entry : csfilesIn)
        files.push_back(entry.second);
    return ApplyChangeSets(nChangesApplied, project, files);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::Dump(BeFileNameCR csfileName, Db& db, int detailLevel)
    {
    Db csfile;
    if (BE_SQLITE_OK != csfile.OpenBeSQLiteDb(csfileName, Db::OpenParams(Db::OpenMode::OPEN_Readonly)))
        return BSIERROR;

    BeSQLite::Statement stmt;
    stmt.Prepare(csfile, "SELECT " CHANGSETINFO_COLS " FROM " CHANGESET_Table);

    while (true)
        {
        ChangeSetInfo info(stmt);
        if (!info.IsValid())
            break;

        SyncInfoChangeSet changeSet(db, (ChangeSetType::Patch==info.m_type));
        extractChangeSetBySequenceNumberDirect(changeSet, csfile, info.m_sequenceNumber);

        printf("\n\n--Changeset SequenceNumber=%" PRIu64 " type=%d desc=[%s] time=%s size=%d---\n", info.m_sequenceNumber, (int)info.m_type, info.m_description.c_str(), Utf8String(info.m_time.ToString()).c_str(), (int)changeSet.GetSize());

        changeSet.Dump("", db, (ChangeSetType::Patch==info.m_type), detailLevel);
        }

    return BSISUCCESS;
    } 

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
BentleyStatus SatelliteChangeSets::PerformVersionChecks()
    {
    // Look at the stored version and see if we have to upgrade
    Utf8String versionString;
    MUSTBEROW (QueryProperty(versionString, Property::SchemaVersion()));

    SchemaVersion storedVersion(versionString.c_str());
        
    if (storedVersion.CompareTo(s_currentVersion) == 0)
        return BSISUCCESS;

    if (storedVersion.CompareTo(s_currentVersion) > 0)
        { // version is too new!
        LOG.errorv("compatibility error - storedVersion=%s > currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return BSIERROR;
        }

    //  Upgrade
    //      when we change the syncInfo schema, add upgrade steps here ...
    
    //  Upgraded. Update the stored version.
    MUSTBEOK (SavePropertyString(Property::SchemaVersion(), s_currentVersion.ToJson().c_str()));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::CreateEmptyFile(BeFileNameCR dbNameIn, bool deleteIfExists)
    {
    BeFileName dbName(dbNameIn);
    if (dbName.GetExtension().empty())
        dbName.append(L".changes");

    if (deleteIfExists)
        dbName.BeDeleteFile();

    Db bootStrapDb;
    auto rc = bootStrapDb.CreateNewDb(Utf8String(dbName).c_str());
    if (rc != BE_SQLITE_OK)
        {
        LOG.errorv("%s - cannot create. Error code=%s", Utf8String(dbName).c_str(), Db::InterpretDbResult(rc));
        return BSIERROR;
        }

    bootStrapDb.CloseDb();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::OnAttach(Db& targetProject, BeSQLite::SchemaVersion const& targetSchemaVersion)
    {
    m_dgndb = &targetProject;

    if (!targetProject.TableExists(CHANGESET_ATTACH(CHANGESET_Table)))
        {
        //  We are creating a new .changes file
        Utf8String currentProjectDbSchemaVersion;
        targetProject.QueryProperty(currentProjectDbSchemaVersion, Properties::SchemaVersion());

        MUSTBEOK (SavePropertyString(Property::SchemaVersion(), s_currentVersion.ToJson()));
        MUSTBEOK (SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToUtf8String()));
        MUSTBEOK (SavePropertyString(Property::DgnDbGuid(), targetProject.GetDbGuid().ToString()));
        MUSTBEOK (SavePropertyString(Property::DgnDbSchemaVersion(), currentProjectDbSchemaVersion));
        MUSTBEOK (SavePropertyString(Property::DbSchemaVersion(), targetSchemaVersion.ToJson()));
        // *** WIP_CONVERTER - I'd like to save project's last save time

        MUSTBEOK (CreateTables());
    
        SetValid(true);
        return BSISUCCESS;
        }

    //  We are opening an existing .changes file
    if (SUCCESS != PerformVersionChecks())
        return BSIERROR;

    if (GetTargetDbGuid() != targetProject.GetDbGuid())
        {
        LOG.errorv("compatibility error - storedDbGuid=%s, specifiedDbGuid=%s", GetTargetDbGuid().ToString().c_str(), targetProject.GetDbGuid().ToString().c_str());
        return BSIERROR;
        }

    Utf8String savedProjectDbSchemaVersion, currentProjectDbSchemaVersion;
    if (QueryProperty(savedProjectDbSchemaVersion, Property::DgnDbSchemaVersion()) != BE_SQLITE_ROW
        || m_dgndb->QueryProperty(currentProjectDbSchemaVersion, Properties::SchemaVersion()) !=  BE_SQLITE_ROW
        || !savedProjectDbSchemaVersion.Equals(currentProjectDbSchemaVersion))
        {
        LOG.warningv("compatibility error. ProjectDbSchemaVersion=%s does not match project SchemaVersion=%s.",
                                savedProjectDbSchemaVersion.c_str(), currentProjectDbSchemaVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away changes whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    if (targetSchemaVersion.GetInt64(SchemaVersion::VERSION_All) != 0)
        {
        Utf8String savedProjectSchemaVersion;
        QueryProperty(savedProjectSchemaVersion, Property::DgnDbSchemaVersion());
        if (targetSchemaVersion.ToJson() != savedProjectSchemaVersion)
            {
            LOG.warningv("compatibility error - savedProjectSchemaVersion=%s, specifiedProjectSchemaVersion=%s", savedProjectSchemaVersion.c_str(), targetSchemaVersion.ToJson().c_str());
            // *** WIP_CONVERTER - Do we really have to throw away changes whenever we make a trivial schema change?
            return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
            }
        }

    SetValid(true);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::AttachToDb(Db& targetProject, BeFileNameCR dbName)
    {
    m_dbFileName = dbName;
    DbResult rc = targetProject.AttachDb(Utf8String(dbName).c_str(), CHANGES_ATTACH_ALIAS);
    if (BE_SQLITE_OK != rc)
        return BSIERROR;
    return OnAttach(targetProject, SchemaVersion(0,0,0,0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeDbGuid SatelliteChangeSets::GetTargetDbGuid()
    {
    Utf8String projectGuidStr;
    QueryProperty(projectGuidStr, Property::DgnDbGuid());
    BeDbGuid guid;
    guid.FromString(projectGuidStr.c_str());
    return guid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t SatelliteChangeSets::GetFirstSequenceNumber()
    {
    Utf8String str;
    QueryProperty(str, Property::FirstSequenceNumber());
    return uint64FromString(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t SatelliteChangeSets::GetLastSequenceNumber()
    {
    Utf8String str;
    QueryProperty(str, Property::LastSequenceNumber());
    return uint64FromString(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::UpdateSequenceNumberRange(uint64_t csid)
    {
    if (csid < GetFirstSequenceNumber() || 0==GetFirstSequenceNumber())
        SavePropertyString(Property::FirstSequenceNumber(), uint64ToString(csid));
    if (csid > GetLastSequenceNumber())
        SavePropertyString(Property::LastSequenceNumber(), uint64ToString(csid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::SetExpirationDate(DateTime const& expirationDate)
    {
    MUSTBEOK (SavePropertyString(Properties::ExpirationDate(), expirationDate.ToUtf8String()));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::GetExpirationDate(DateTime& dt)
    {
    Utf8String str;
    QueryProperty(str, Properties::ExpirationDate());
    DateTime::FromString(dt, str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::SetLastError(BeSQLite::DbResult rc)
    {
    m_lastError = rc;
    m_lastErrorDescription = m_dgndb->GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::GetLastError(BeSQLite::DbResult& result, Utf8String& descr)
    {
    result = m_lastError;
    descr = m_lastErrorDescription;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SatelliteChangeSets::SaveChanges()
    {
    if (!m_dgndb->IsDbOpen())
        return BSISUCCESS;

    MUSTBEOK(m_dgndb->SaveChanges());
    
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::AbandonChanges()
    {
    m_dgndb->AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SatelliteChangeSets::Close()
    {
    m_dgndb->CloseDb();
    }

#define LAST_CHANGESET_ID_STR "changeset_lastId"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeSetProperties::SaveToDb(Db& db)
    {
    db.SaveRepositoryLocalValue(LAST_CHANGESET_ID_STR, uint64ToString(m_latestChangeSetId));
    if (m_expirationDate.IsValid())
        {
        auto saveResult = db.SavePropertyString(Properties::ExpirationDate(), m_expirationDate.ToUtf8String());
        if (saveResult != BE_SQLITE_OK)
            {
            LOG.errorv("ApplyChangeSets - Db %s - save ExpirationDate failed with code %s.", db.GetDbFileName(), Db::InterpretDbResult(saveResult));
            return BSIERROR;
            }
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeSetProperties::LoadFromDb(Db& db)
    {
    Utf8String str;
    DbResult rc = db.QueryRepositoryLocalValue(LAST_CHANGESET_ID_STR, str);
    m_latestChangeSetId = (BE_SQLITE_ROW == rc) ? uint64FromString(str) : 0;

    Utf8String expirationDateString;
    db.QueryProperty(expirationDateString, Properties::ExpirationDate());
    DateTime::FromString(m_expirationDate, expirationDateString.c_str());

    return BSISUCCESS;
    }
