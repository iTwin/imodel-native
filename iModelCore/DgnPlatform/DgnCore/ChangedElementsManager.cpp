/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ChangedElementsManager.h>
#include <ECDb/ECDbApi.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>

#define CHANGE_PROPSPEC_NAMESPACE "ec_ChangedElements"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_ECPRESENTATION

//---------------------------------------------------------------------------------------
// @bsimethod                                              Grigas.Petraitis     09/2019
//---------------------------------------------------------------------------------------
IECPresentationManager* ChangedElementsManager::CreatePresentationManager()
    {
    RulesDrivenECPresentationManager::Paths paths(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory(),
        T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName());
    RulesDrivenECPresentationManager::Params params(paths);
    return new RulesDrivenECPresentationManager(params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     01/2020
//---------------------------------------------------------------------------------------
void ChangedElementsManager::SetPresentationRulesetDirectory(Utf8String rulesetDir) {
    m_rulesetDirectory = rulesetDir;
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(rulesetDir.c_str());
    ((RulesDrivenECPresentationManager*)m_presentationManager)->GetLocaters().RegisterLocater(*locater);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
ChangedElementsManager::~ChangedElementsManager()
    {
    DELETE_AND_CLEAR(m_presentationManager);
    m_db = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::CreateChangedElementsCache(ECDbR cacheDb, BeFileNameCR cacheFilePath)
    {
    if (cacheFilePath.DoesPathExist())
        {
        LOG.errorv(L"Failed to create Changed Elements cache. The file already exists.");
        return BE_SQLITE_ERROR;
        }

    BeAssert(!cacheDb.IsDbOpen());
    DbResult r = cacheDb.CreateNewDb(cacheFilePath);
    if (BE_SQLITE_OK != r)
        {
        LOG.errorv(L"Failed to create Changed Elements cache.");
        return r;
        }

    //import ChangedElements schema
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(cacheDb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"Dgn");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    ECN::SchemaKey schemaKey(ECSCHEMA_ChangedElements, 1, 0, 0);
    if (context->LocateSchema(schemaKey, ECN::SchemaMatchType::LatestWriteCompatible) == nullptr)
        {
        LOG.errorv(L"Failed to create new Change Cache file. Could not locate schema at '%s'", ecdbStandardSchemasFolder.GetName());
        return BE_SQLITE_ERROR;
        }

    if (SUCCESS != cacheDb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        LOG.errorv(L"Failed to create new Change Cache file. Could not import schemas");
        cacheDb.AbandonChanges();
        return BE_SQLITE_ERROR;
        }

    r = AddMetadataToChangeCacheFile(cacheDb, *m_db);
    if (BE_SQLITE_OK != r)
        {
        LOG.errorv(L"Failed to create new Change Cache file. Could not add metadata to file");
        cacheDb.AbandonChanges();
        return r;
        }

    r = cacheDb.SaveChanges();
    if (BE_SQLITE_OK != r)
        {
        LOG.errorv(L"Failed to create new Change Cache file. Could not commit changes");
        return r;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::AddMetadataToChangeCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const
    {
    BeGuid primaryFileGuid = m_db->GetDbGuid();
    if (primaryFileGuid.IsValid())
        {
        const DbResult r = cacheFile.SavePropertyString(PropertySpec("ECDbGuid", CHANGE_PROPSPEC_NAMESPACE), primaryFileGuid.ToString().c_str());
        if (BE_SQLITE_OK != r)
            return r;
        }
    else
        {
        const DbResult r = cacheFile.SavePropertyString(PropertySpec("ECDbPath", CHANGE_PROPSPEC_NAMESPACE), m_db->GetDbFileName());
        if (BE_SQLITE_OK != r)
            return r;
        }

    return cacheFile.SavePropertyString(PropertySpec("ECDbSchemaVersion", CHANGE_PROPSPEC_NAMESPACE), primaryECDb.GetECDbProfileVersion().ToJson().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
bool ChangedElementsManager::HasChangeset(ECDbR cacheDb, DgnRevisionPtr revision)
    {
    return IsProcessed(cacheDb, revision->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
bool ChangedElementsManager::IsProcessed(ECDbR cacheDb, Utf8String changesetId)
    {
    ECSqlStatement stmt;
    stmt.Prepare(cacheDb, "SELECT 1 FROM chems.Changeset WHERE ChangesetId=?");
    stmt.BindText(1, changesetId.c_str(), IECSqlBinder::MakeCopy::Yes);
    return stmt.Step() == BE_SQLITE_ROW;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::InsertEntries(ECDbR cacheDb, DgnRevisionPtr revision, bvector<DgnElementId> const& elementIds, bvector<ECClassId> const& classIds, bvector<BeSQLite::DbOpcode> const& opcodes, bvector<DgnModelId> const& modelIds, bvector<AxisAlignedBox3d> const& bboxes)
    {
    // Check if we already have this changeset in cache, if so, return success
    if (HasChangeset(cacheDb, revision))
        return BE_SQLITE_OK;

    // TODO: Maintain statements so that we don't need to re-prepare them
	// Insert an entry for the revision/changeset
    ECSqlStatement stmt;
    stmt.Prepare(cacheDb, "INSERT INTO chems.Changeset (ChangesetId, ParentId, PushDate) VALUES (?, ?, ?)");
    stmt.BindText(1, revision->GetId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, revision->GetParentId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindDateTime(3, revision->GetDateTime());
    ECInstanceKey key;
    if (BE_SQLITE_DONE != stmt.Step(key))
        {
        BeAssert(false && "Problem inserting changeset into Cache");
        cacheDb.AbandonChanges();
        return BE_SQLITE_ERROR;
        }

    // Insert element changed in revision
    ECSqlStatement dataStmt;
    dataStmt.Prepare(cacheDb, "INSERT INTO chems.InstanceChange (ChangedInstance.Id, ChangedInstance.ClassId, OpCode, ModelId, BBoxLow, BBoxHigh, Changeset.Id) VALUES (?, ?, ?, ?, ?, ?, ?)");
    for (int i = 0; i < elementIds.size(); ++i)
        {
        dataStmt.BindId(1, elementIds[i]);
        dataStmt.BindId(2, classIds[i]);
        switch (opcodes[i])
            {
            case DbOpcode::Insert:
                dataStmt.BindInt(3, OPC_INSERT);
                break;
            case DbOpcode::Delete:
                dataStmt.BindInt(3, OPC_DELETE);
                break;
            case DbOpcode::Update:
                dataStmt.BindInt(3, OPC_UPDATE);
                break;
            }

        // Bind model Id
        dataStmt.BindId(4, modelIds[i]);

        // Bind axis aligned box
        dataStmt.BindPoint3d(5, bboxes[i].low);
        dataStmt.BindPoint3d(6, bboxes[i].high);

        // Navigation property
        dataStmt.BindId(7, key.GetInstanceId());

        if (BE_SQLITE_DONE != dataStmt.Step())
            {
            LOG.errorv(L"Problem inserting change into cache");
            cacheDb.AbandonChanges();
            return BE_SQLITE_ERROR;
            }

        dataStmt.Reset();
        dataStmt.ClearBindings();
        }

    dataStmt.Finalize();

    // Insert changed models data
    ECSqlStatement modelStmt;
    modelStmt.Prepare(cacheDb, "INSERT INTO chems.ModelChange (ModelId, BBoxLow, BBoxHigh, Changeset.Id) VALUES (?, ?, ?, ?)");
    bmap<DgnModelId, AxisAlignedBox3d> changedModels = ChangedElementsManager::ComputeChangedModels(modelIds, bboxes);
    for (auto changedModel : changedModels)
        {
        modelStmt.BindId(1, changedModel.first);
        modelStmt.BindPoint3d(2, changedModel.second.low);
        modelStmt.BindPoint3d(3, changedModel.second.high);
        modelStmt.BindId(4, key.GetInstanceId());

        if (BE_SQLITE_DONE != modelStmt.Step())
            {
            LOG.errorv(L"Problem inserting model change into cache");
            cacheDb.AbandonChanges();
            return BE_SQLITE_ERROR;
            }

        modelStmt.Reset();
        modelStmt.ClearBindings();
        }

    modelStmt.Finalize();

    return cacheDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr    ChangedElementsManager::CloneDb(DgnDbR db)
    {
    BeSQLite::DbResult result;
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck));

    BeFileName tempFilename;
    if (m_tempLocation.IsEmpty())
        T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFilename, L"ChangedElementsManager");
    else
        tempFilename = m_tempLocation;

    WString name = WString(L"Temp_") + db.GetFileName().GetFileNameWithoutExtension();
    tempFilename.AppendToPath(name.c_str());
    tempFilename.AppendExtension(L"bim");

    // Try to create temporary file by copying base bim file
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(db.GetFileName(), tempFilename);

    // If we get any error besides already exists, return error, something went wrong
    if (BeFileNameStatus::Success != fileStatus && BeFileNameStatus::AlreadyExists != fileStatus)
        return nullptr;

    // If the file was copied before, act on it instead of re-copying each time we compare revisions
    // Open the target db using the temporary filename
    DgnDbPtr targetDb = DgnDb::OpenDgnDb(&result, tempFilename, params);
    if (!targetDb.IsValid())
        return nullptr;

    return targetDb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     8/2019
//---------------------------------------------------------------------------------------
bmap<DgnModelId, AxisAlignedBox3d> ChangedElementsManager::ComputeChangedModels(ChangedElementsMap const& changedElements)
    {
    // Compute union of ranges then generate entries in changedModels JSON object
    bmap<DgnModelId, AxisAlignedBox3d> map;
    for (auto pair : changedElements)
        {
        DgnModelId modelId = pair.second.m_modelId;
        AxisAlignedBox3d bbox = pair.second.m_bbox;
        if (map.find(modelId) == map.end())
            {
            map.Insert(modelId, bbox);
            continue;
            }

        AxisAlignedBox3d foundBox = map[modelId];
        AxisAlignedBox3d bboxUnion(DRange3d::FromUnion(foundBox, bbox));
        map[modelId] = bboxUnion;
        }

    return map;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     8/2019
//---------------------------------------------------------------------------------------
bmap<DgnModelId, AxisAlignedBox3d> ChangedElementsManager::ComputeChangedModels(bvector<DgnModelId> const& modelIds, bvector<AxisAlignedBox3d> const& bboxes)
    {
    // Compute union of ranges then generate entries in changedModels JSON object
    bmap<DgnModelId, AxisAlignedBox3d> map;
    for (int i = 0; i < modelIds.size(); ++i)
        {
        DgnModelId modelId = modelIds[i];
        AxisAlignedBox3d bbox = bboxes[i];
        if (map.find(modelId) == map.end())
            {
            map.Insert(modelId, bbox);
            continue;
            }

        AxisAlignedBox3d foundBox = map[modelId];
        AxisAlignedBox3d bboxUnion(DRange3d::FromUnion(foundBox, bbox));
        map[modelId] = bboxUnion;
        }

    return map;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
void ChangedElementsManager::ChangedElementsToJSON(JsonValueR val, ChangedElementsMap const& changedElements)
    {
    val[ChangedElementsManager::json_changedElements()] = Json::objectValue;
    val[ChangedElementsManager::json_changedModels()] = Json::objectValue;
    auto& celems = val[ChangedElementsManager::json_changedElements()];
    auto& cmodels = val[ChangedElementsManager::json_changedModels()];

    celems[ChangedElementsManager::json_elements()] = Json::arrayValue;
    celems[ChangedElementsManager::json_classIds()] = Json::arrayValue;
    celems[ChangedElementsManager::json_opcodes()] = Json::arrayValue;
    celems[ChangedElementsManager::json_modelIds()] = Json::arrayValue;

    cmodels[ChangedElementsManager::json_modelIds()] = Json::arrayValue;
    cmodels[ChangedElementsManager::json_bboxes()] = Json::arrayValue;

    bvector<DgnModelId> modelIds;
    bvector<AxisAlignedBox3d> bboxes;
    // Add all the changed elements entries
    for (auto pair : changedElements)
        {
        celems[ChangedElementsManager::json_elements()].append(pair.first.GetInstanceId().ToHexStr());
        celems[ChangedElementsManager::json_classIds()].append(pair.second.m_ecclassId.ToHexStr());
        celems[ChangedElementsManager::json_opcodes()].append((int)pair.second.m_opcode);
        celems[ChangedElementsManager::json_modelIds()].append(pair.second.m_modelId.ToHexStr());
        modelIds.push_back(pair.second.m_modelId);
        bboxes.push_back(pair.second.m_bbox);
        }

    // Compute union of ranges then generate entries in changedModels JSON object
    bmap<DgnModelId, AxisAlignedBox3d> map = ChangedElementsManager::ComputeChangedModels(changedElements);
    // Set it in the JSON object
    for (auto pair : map)
        {
        cmodels[ChangedElementsManager::json_modelIds()].append(pair.first.ToHexStr());
        Json::Value bboxJson;
        pair.second.ToJson(bboxJson);
        cmodels[ChangedElementsManager::json_bboxes()].append(bboxJson);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::ProcessChangesets(ECDbR cacheDb, Utf8String rulesetId, bvector<DgnRevisionPtr> const& revisions)
    {
    // Clone briefcase so that we may roll it backwards during processing
    DgnDbPtr db = CloneDb(*m_db);

    bvector<DgnRevisionPtr> processedRevisions;
    for (DgnRevisionPtr rev : revisions)
        processedRevisions.push_back(rev);

    // We always have to process from newest to oldest
    DateTime firstDate = (*revisions[0]).GetDateTime();
    DateTime lastDate = (*revisions[revisions.size() - 1]).GetDateTime();
    if (DateTime::Compare(firstDate, lastDate) == DateTime::CompareResult::EarlierThan)
        std::reverse(processedRevisions.begin(), processedRevisions.end());

    // Use version compare change summary to generate the changed elements list
    for (DgnRevisionPtr revision : processedRevisions)
        {
        // Generate a summary for each revision
        bvector<DgnRevisionPtr> currentRevisions;
        currentRevisions.push_back(revision);
        // Process going backwards
        VersionCompareChangeSummaryPtr summary = VersionCompareChangeSummary::Generate(*db, currentRevisions, *m_presentationManager, rulesetId, true, m_filterSpatial, true);
        if (!summary.IsValid())
            {
            LOG.errorv(L"Could not generate change summary for revision");
            return BE_SQLITE_ERROR;
            }

        // Get changed elements
        bvector<DgnElementId> elementIds;
        bvector<ECClassId> classIds;
        bvector<BeSQLite::DbOpcode> opcodes;
        bvector<DgnModelId> modelIds;
        bvector<AxisAlignedBox3d> bboxes;
        if (SUCCESS != summary->GetChangedElements(elementIds, classIds, opcodes, modelIds, bboxes))
            {
            LOG.errorv(L"Problem getting changed elements");
            continue;
            }

        // Insert data into the cache
        if (BE_SQLITE_OK != InsertEntries(cacheDb, revision, elementIds, classIds, opcodes, modelIds, bboxes))
            {
            LOG.errorv(L"Could not insert entries into cache");
            return BE_SQLITE_ERROR;
            }

        // Roll Db
        // Close db
        BeFileName filename = db->GetFileName();
        db->CloseDb();
        // Re-open to apply changesets
        BeSQLite::DbResult result;
        DgnDb::OpenParams params(Db::OpenMode::ReadWrite);
        bvector<DgnRevisionCP> changesetsCP;
        changesetsCP.push_back(revision.get());
        params.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(changesetsCP, RevisionProcessOption::Reverse);
        params.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
        db = DgnDb::OpenDgnDb(&result, filename, params);
        BeAssert(result == BeSQLite::BE_SQLITE_OK && db.IsValid());
        }

    // Delete file and cleanup
    BeFileName filename = db->GetFileName();
    db->CloseDb();
    BeFileName::BeDeleteFile(filename.GetName());

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     12/2018
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::GetChangedElements(ECDbR cacheDb, ChangedElementsMap& changedElementsMap, Utf8String startChangesetId, Utf8String endChangesetId)
    {
    // Find changesets in range
    ECSqlStatement stmt;
    stmt.Prepare(cacheDb, "SELECT PushDate FROM chems.Changeset WHERE ChangesetId=?");
    stmt.BindText(1, startChangesetId.c_str(), IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt.Step())
        return BE_SQLITE_ERROR;

    // Check if null
    if (stmt.IsValueNull(0))
        {
        LOG.errorv("PushDate is null on changeset entry");
        return BE_SQLITE_ERROR;
        }

    DateTime startTime = stmt.GetValueDateTime(0);
    stmt.Reset();
    stmt.ClearBindings();

    stmt.BindText(1, endChangesetId.c_str(), IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt.Step())
        return BE_SQLITE_ERROR;

    // Check if null
    if (stmt.IsValueNull(0))
        {
        LOG.errorv("PushDate is null on changeset entry");
        return BE_SQLITE_ERROR;
        }

    DateTime endTime = stmt.GetValueDateTime(0);
    stmt.Reset();
    stmt.Finalize();

    // Find all changesets in range and order them
    stmt.Prepare(cacheDb, "SELECT ECInstanceId FROM chems.Changeset WHERE PushDate >= ? AND PushDate <= ? ORDER BY PushDate DESC");
    stmt.BindDateTime(1, startTime);
    stmt.BindDateTime(2, endTime);

    // Clear the map before populating it
    changedElementsMap.clear();

    // Select data for the elements
    ECSqlStatement elementStmt;
    elementStmt.Prepare(cacheDb, "SELECT ChangedInstance.Id, ChangedInstance.ClassId, OpCode, ModelId, BBoxLow, BBoxHigh FROM chems.InstanceChange WHERE Changeset.Id=?");
    // bvector<ECInstanceId> changesetIds;
    while(stmt.Step() == BE_SQLITE_ROW)
        {
        ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
        // changesetIds.push_back(stmt.GetValueId<ECInstanceId>(0));

        elementStmt.BindId(1, id);
        while(elementStmt.Step() == BE_SQLITE_ROW)
            {
            // Process elements and accumulate change
            ECInstanceId elementId  = elementStmt.GetValueId<ECInstanceId>(0);
            ECClassId classId       = elementStmt.GetValueId<ECClassId>(1);
            int opcode              = elementStmt.GetValueInt(2);
            DgnModelId modelId      = elementStmt.GetValueId<DgnModelId>(3);
            DPoint3d bboxLow        = elementStmt.GetValuePoint3d(4);
            DPoint3d bboxHigh       = elementStmt.GetValuePoint3d(5);

            DbOpcode dbOpcode = DbOpcode::Insert;
            switch(opcode)
                {
                case OPC_INSERT:
                    dbOpcode = DbOpcode::Insert;
                    break;
                case OPC_DELETE:
                    dbOpcode = DbOpcode::Delete;
                    break;
                case OPC_UPDATE:
                    dbOpcode = DbOpcode::Update;
                    break;
                default:
                    BeAssert(false && "Unknown opcode");
                }

            ECInstanceKey elemKey(classId, elementId);
            VersionCompareChangeSummary::SummaryElementInfo info(dbOpcode, classId, modelId, AxisAlignedBox3d(bboxLow, bboxHigh));
            if (changedElementsMap.find(elemKey) != changedElementsMap.end())
                changedElementsMap[elemKey].AccumulateChange(info, true);
            else
                changedElementsMap[elemKey] = info;

            // May have become invalid in case of wrong operations
            if (!changedElementsMap[elemKey].IsValid())
                changedElementsMap.erase(elemKey);
            }

        elementStmt.Reset();
        elementStmt.ClearBindings();
        }

    stmt.Finalize();
    elementStmt.Finalize();

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Diego.Pinate     8/2019
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::GetChangedModels(ECDbR cacheDb, bmap<DgnModelId, AxisAlignedBox3d>& changedModels, Utf8String startChangesetId, Utf8String endChangesetId)
    {
    // Find changesets in range
    ECSqlStatement stmt;
    stmt.Prepare(cacheDb, "SELECT PushDate FROM chems.Changeset WHERE ChangesetId=?");
    stmt.BindText(1, startChangesetId.c_str(), IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt.Step())
        return BE_SQLITE_ERROR;

    // Check if null
    if (stmt.IsValueNull(0))
        {
        LOG.errorv("PushDate is null on changeset entry");
        return BE_SQLITE_ERROR;
        }

    DateTime startTime = stmt.GetValueDateTime(0);
    stmt.Reset();
    stmt.ClearBindings();

    stmt.BindText(1, endChangesetId.c_str(), IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt.Step())
        return BE_SQLITE_ERROR;

    // Check if null
    if (stmt.IsValueNull(0))
        {
        LOG.errorv("PushDate is null on changeset entry");
        return BE_SQLITE_ERROR;
        }

    DateTime endTime = stmt.GetValueDateTime(0);
    stmt.Reset();
    stmt.Finalize();

    // Find all changesets in range and order them
    stmt.Prepare(cacheDb, "SELECT ECInstanceId FROM chems.Changeset WHERE PushDate >= ? AND PushDate <= ? ORDER BY PushDate DESC");
    stmt.BindDateTime(1, startTime);
    stmt.BindDateTime(2, endTime);

    // Clear the map before populating it
    changedModels.clear();

    // Select data for the elements
    ECSqlStatement elementStmt;
    elementStmt.Prepare(cacheDb, "SELECT ModelId, BBoxLow, BBoxHigh FROM chems.ModelChange WHERE Changeset.Id=?");
    // bvector<ECInstanceId> changesetIds;
    while(stmt.Step() == BE_SQLITE_ROW)
        {
        ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
        // changesetIds.push_back(stmt.GetValueId<ECInstanceId>(0));

        elementStmt.BindId(1, id);
        while(elementStmt.Step() == BE_SQLITE_ROW)
            {
            // Process elements and accumulate change
            DgnModelId modelId      = elementStmt.GetValueId<DgnModelId>(0);
            DPoint3d bboxLow        = elementStmt.GetValuePoint3d(1);
            DPoint3d bboxHigh       = elementStmt.GetValuePoint3d(2);

            // Insert the changed models into the map if not found
            if (changedModels.find(modelId) == changedModels.end())
                {
                changedModels.Insert(modelId, AxisAlignedBox3d(bboxLow, bboxHigh));
                continue;
                }

            // If found, accumulate change by doing union of ranges
            AxisAlignedBox3d current = changedModels[modelId];
            AxisAlignedBox3d bboxUnion = AxisAlignedBox3d(DRange3d::FromUnion(current, AxisAlignedBox3d(bboxLow, bboxHigh)));
            changedModels[modelId] = bboxUnion;
            }

        elementStmt.Reset();
        elementStmt.ClearBindings();
        }

    stmt.Finalize();
    elementStmt.Finalize();

    return BE_SQLITE_OK;
    }
