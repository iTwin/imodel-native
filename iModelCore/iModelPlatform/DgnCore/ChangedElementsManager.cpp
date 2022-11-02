/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ChangedElementsManager.h>
#include <ECDb/ECDbApi.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/RuleSetLocater.h>

#define CHANGE_PROPSPEC_NAMESPACE "ec_ChangedElements"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_ECPRESENTATION

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECPresentationManager* ChangedElementsManager::CreatePresentationManager()
    {
    ECPresentationManager::Paths paths(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory(),
        T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName());
    ECPresentationManager::Params params(paths);
    return new ECPresentationManager(params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedElementsManager::SetPresentationRulesetDirectory(Utf8String rulesetDir) {
    m_rulesetDirectory = rulesetDir;
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(rulesetDir.c_str());
    m_presentationManager->GetLocaters().RegisterLocater(*locater);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedElementsManager::~ChangedElementsManager()
    {
    DELETE_AND_CLEAR(m_presentationManager);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    r = AddMetadataToChangeCacheFile(cacheDb);
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
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::AddMetadataToChangeCacheFile(ECDb& cacheFile) const
    {
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, m_dbFilename.c_str());
    return cacheFile.SavePropertyString(PropertySpec("ECDbPath", CHANGE_PROPSPEC_NAMESPACE), fileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ChangedElementsManager::HasChangeset(ECDbR cacheDb, DgnRevisionPtr revision)
    {
    return IsProcessed(cacheDb, revision->GetChangesetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ChangedElementsManager::IsProcessed(ECDbR cacheDb, Utf8String changesetId)
    {
    ECSqlStatement stmt;
    stmt.Prepare(cacheDb, "SELECT 1 FROM chems.Changeset WHERE ChangesetId=?");
    stmt.BindText(1, changesetId.c_str(), IECSqlBinder::MakeCopy::Yes);
    return stmt.Step() == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::InsertEntries(ECDbR cacheDb, DgnRevisionPtr revision, bvector<ChangedElement> const& elements)
    {
    // Check if we already have this changeset in cache, if so, return success
    if (HasChangeset(cacheDb, revision))
        return BE_SQLITE_OK;

    // TODO: Maintain statements so that we don't need to re-prepare them
	// Insert an entry for the revision/changeset
    ECSqlStatement stmt;
    stmt.Prepare(cacheDb, "INSERT INTO chems.Changeset (ChangesetId, ParentId, PushDate) VALUES (?, ?, ?)");
    stmt.BindText(1, revision->GetChangesetId().c_str(), IECSqlBinder::MakeCopy::Yes);
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
    dataStmt.Prepare(cacheDb, "INSERT INTO chems.InstanceChange (ChangedInstance.Id, ChangedInstance.ClassId, OpCode, ModelId, BBoxLow, BBoxHigh, ChangesType, Properties, PropertyOldChecksums, PropertyNewChecksums, Changeset.Id, Parent.Id, Parent.ClassId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    for (ChangedElement const& element : elements)
        {
        dataStmt.BindId(1, element.m_elementId);
        dataStmt.BindId(2, element.m_classId);
        switch (element.m_opcode)
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
        dataStmt.BindId(4, element.m_modelId);

        // Bind axis aligned box
        dataStmt.BindPoint3d(5, element.m_bbox.low);
        dataStmt.BindPoint3d(6, element.m_bbox.high);

        // Changes type
        dataStmt.BindInt(7, element.m_changes.m_flags);

        // Changed Properties
        for (Utf8String const& prop : element.m_changes.m_properties)
            {
            // Bind property accessor
            IECSqlBinder& arrayBinder = dataStmt.GetBinder(8).AddArrayElement();
            arrayBinder.BindText(prop.c_str(), IECSqlBinder::MakeCopy::No);
            // Bind property checksums
            auto const& pair = element.m_changes.m_propertyChecksums.find(prop);
            IECSqlBinder& arrayOldChecksumBinder = dataStmt.GetBinder(9).AddArrayElement();
            arrayOldChecksumBinder.BindInt(pair->second.m_oldValue);
            IECSqlBinder& arrayNewChecksumBinder = dataStmt.GetBinder(10).AddArrayElement();
            arrayNewChecksumBinder.BindInt(pair->second.m_newValue);
            }

        // Navigation property
        dataStmt.BindId(11, key.GetInstanceId());

        // Parent Key
        dataStmt.BindId(12, element.m_parentKey.GetInstanceId());
        dataStmt.BindId(13, element.m_parentKey.GetClassId());

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
    bmap<DgnModelId, AxisAlignedBox3d> changedModels = ChangedElementsManager::ComputeChangedModels(elements);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName    ChangedElementsManager::CloneDb(BeFileNameCR dbFilename)
    {
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck));

    BeFileName tempFilename;
    if (m_tempLocation.IsEmpty())
        T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFilename, L"ChangedElementsManager");
    else
        tempFilename = m_tempLocation;

    WString name = WString(L"Temp_") + dbFilename.GetFileNameWithoutExtension();
    tempFilename.AppendToPath(name.c_str());
    tempFilename.AppendExtension(L"bim");

    // Try to create temporary file by copying base bim file
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(dbFilename, tempFilename);

    // If we get any error besides already exists, return error, something went wrong
    if (BeFileNameStatus::Success != fileStatus && BeFileNameStatus::AlreadyExists != fileStatus)
        {
        BeAssert(false && "File should have been cleaned before clone");
        }

    return tempFilename;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
bmap<DgnModelId, AxisAlignedBox3d> ChangedElementsManager::ComputeChangedModels(bvector<ChangedElement> const& elements)
    {
    // Compute union of ranges then generate entries in changedModels JSON object
    bmap<DgnModelId, AxisAlignedBox3d> map;
    for (ChangedElement const& element : elements)
        {
        if (map.find(element.m_modelId) == map.end())
            {
            map.Insert(element.m_modelId, element.m_bbox);
            continue;
            }

        AxisAlignedBox3d foundBox = map[element.m_modelId];
        AxisAlignedBox3d bboxUnion(DRange3d::FromUnion(foundBox, element.m_bbox));
        map[element.m_modelId] = bboxUnion;
        }

    return map;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedElementsManager::ChangedElementsToJSON(BeJsValue val, ChangedElementsMap const& changedElements)
    {
    val[ChangedElementsManager::json_changedElements()].toObject();
    val[ChangedElementsManager::json_changedModels()].toObject();
    auto celems = val[ChangedElementsManager::json_changedElements()];
    auto cmodels = val[ChangedElementsManager::json_changedModels()];

    auto elements = celems[ChangedElementsManager::json_elements()];
    auto classIds = celems[ChangedElementsManager::json_classIds()];
    auto opcodes = celems[ChangedElementsManager::json_opcodes()];
    auto elModelIds = celems[ChangedElementsManager::json_modelIds()];
    auto type = celems[ChangedElementsManager::json_type()];
    auto properties = celems[ChangedElementsManager::json_properties()];
    auto oldChecksums = celems[ChangedElementsManager::json_oldChecksums()];
    auto newChecksums = celems[ChangedElementsManager::json_newChecksums()];
    auto parentIds = celems[ChangedElementsManager::json_parentIds()];
    auto parentClassIds = celems[ChangedElementsManager::json_parentClassIds()];

    bvector<DgnModelId> modelIds;
    bvector<AxisAlignedBox3d> bboxes;
    // Add all the changed elements entries
    int index = 0;
    for (auto pair : changedElements)
        {
        elements[index] = pair.first.GetInstanceId();
        classIds[index] = pair.second.m_ecclassId;
        opcodes[index] = (int)pair.second.m_opcode;
        elModelIds[index] = pair.second.m_modelId;
        type[index] = (int)pair.second.m_changes.m_flags;
        auto propArray = properties[index];
        auto oldCsArray = oldChecksums[index];
        auto newCsArray = newChecksums[index];
        propArray.SetEmptyArray();
        oldCsArray.SetEmptyArray();
        newCsArray.SetEmptyArray();
        for (Utf8String const& prop : pair.second.m_changes.m_properties)
            {
            propArray.appendValue() = prop;
            auto cs = pair.second.m_changes.m_propertyChecksums.find(prop);
            if (cs != pair.second.m_changes.m_propertyChecksums.end())
                {
                oldCsArray.appendValue() = cs->second.m_oldValue;
                newCsArray.appendValue() = cs->second.m_newValue;
                }
            }

        parentIds[index] = pair.second.m_parentKey.GetInstanceId();
        parentClassIds[index] = pair.second.m_parentKey.GetClassId();
        index++;

        modelIds.push_back(pair.second.m_modelId);
        bboxes.push_back(pair.second.m_bbox);
        }

    // Compute union of ranges then generate entries in changedModels JSON object
    bmap<DgnModelId, AxisAlignedBox3d> map = ChangedElementsManager::ComputeChangedModels(changedElements);
    // Set it in the JSON object
    auto modelModelIds = cmodels[ChangedElementsManager::json_modelIds()];
    auto modelBboxes = cmodels[ChangedElementsManager::json_bboxes()];
    index = 0;
    for (auto pair : map)
        {
        modelModelIds[index] = pair.first;
        pair.second.ToJson(modelBboxes[index++]);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangedElementsManager::ProcessChangesets(ECDbR cacheDb, Utf8String rulesetId, bvector<DgnRevisionPtr> const& revisions)
    {
    bool multiProcessing = revisions.size() > 1;
    // Clone briefcase so that we may roll it if we have multiple changesets to process
    BeFileName dbFilename = multiProcessing ? CloneDb(m_dbFilename) : m_dbFilename;

    bvector<DgnRevisionPtr> processedRevisions;
    for (DgnRevisionPtr rev : revisions)
        processedRevisions.push_back(rev);

    // We always have to process from newest to oldest
    DateTime firstDate = (*revisions[0]).GetDateTime();
    DateTime lastDate = (*revisions[revisions.size() - 1]).GetDateTime();
	// Process forwards
    if (DateTime::Compare(firstDate, lastDate) != DateTime::CompareResult::EarlierThan)
        std::reverse(processedRevisions.begin(), processedRevisions.end());

    // Use version compare change summary to generate the changed elements list
    for (DgnRevisionPtr revision : processedRevisions)
        {
        // Generate a summary for each revision
        bvector<DgnRevisionPtr> currentRevisions;
        currentRevisions.push_back(revision);

        SummaryOptions options;
        options.filterSpatial = m_filterSpatial;
        options.tempLocation = m_tempLocation;
        options.presentationManager = m_presentationManager;
        options.wantParents = m_wantParents;
        options.wantBriefcaseRoll = multiProcessing || m_wantBriefcaseRoll;
        options.wantPropertyChecksums = m_wantPropertyChecksums;
        options.wantRelationshipCaching = m_wantRelationshipCaching;
        options.relationshipCacheSize = m_relationshipCacheSize;
        options.wantChunkTraversal = m_wantChunkTraversal;
        VersionCompareChangeSummaryPtr summary = VersionCompareChangeSummary::Generate(dbFilename, currentRevisions, options);
        if (!summary.IsValid())
            {
            LOG.errorv(L"Could not generate change summary for revision");
            return BE_SQLITE_ERROR;
            }

        // Get changed elements
        bvector<ChangedElement> elements;
        if (SUCCESS != summary->GetChangedElements(elements))
            {
            LOG.errorv(L"Problem getting changed elements");
            continue;
            }

        // Insert data into the cache
        if (BE_SQLITE_OK != InsertEntries(cacheDb, revision, elements))
            {
            LOG.errorv(L"Could not insert entries into cache");
            return BE_SQLITE_ERROR;
            }

        // Release summary to clean statement cache
        summary = nullptr;
        }

    // If processing multiple changesets, delete our temporary cloned iModel
    if (multiProcessing)
        BeFileName::BeDeleteFile(dbFilename.GetName());

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    stmt.Prepare(cacheDb, "SELECT ECInstanceId FROM chems.Changeset WHERE PushDate >= ? AND PushDate <= ? ORDER BY PushDate ASC");
	bool startTimeIsLater = (DateTime::Compare(startTime, endTime) == DateTime::CompareResult::LaterThan);
	stmt.BindDateTime(1, startTimeIsLater ? endTime : startTime);
	stmt.BindDateTime(2, startTimeIsLater ? startTime : endTime);


    // Clear the map before populating it
    changedElementsMap.clear();

    // Select data for the elements
    ECSqlStatement elementStmt;
    elementStmt.Prepare(cacheDb, "SELECT ChangedInstance.Id, ChangedInstance.ClassId, OpCode, ModelId, BBoxLow, BBoxHigh, ChangesType, Properties, PropertyOldChecksums, PropertyNewChecksums, Parent.Id, Parent.ClassId FROM chems.InstanceChange WHERE Changeset.Id=?");
    // bvector<ECInstanceId> changesetIds;
    while(stmt.Step() == BE_SQLITE_ROW)
        {
        ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);

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
            int changesType         = elementStmt.GetValueInt(6);
            bvector<Utf8String> props;
            bvector<uint32_t> oldChecksums;
            bvector<uint32_t> newChecksums;
            for (IECSqlValue const& property : elementStmt.GetValue(7).GetArrayIterable())
                props.push_back(Utf8String(property.GetText()));
            for (IECSqlValue const& checksum : elementStmt.GetValue(8).GetArrayIterable())
                oldChecksums.push_back(checksum.GetInt());
            for (IECSqlValue const& checksum : elementStmt.GetValue(9).GetArrayIterable())
                newChecksums.push_back(checksum.GetInt());
            ECInstanceId parentId   = elementStmt.GetValueId<ECInstanceId>(10);
            ECClassId parentClassId = elementStmt.GetValueId<ECClassId>(11);

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
            ChangedElementRecord info(dbOpcode, classId, modelId, AxisAlignedBox3d(bboxLow, bboxHigh), changesType, props, oldChecksums, newChecksums, ECInstanceKey(parentClassId, parentId));
            if (changedElementsMap.find(elemKey) != changedElementsMap.end())
                changedElementsMap[elemKey].AccumulateChange(info);
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
// @bsimethod
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
