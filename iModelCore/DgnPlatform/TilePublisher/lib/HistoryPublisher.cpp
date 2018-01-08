/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/HistoryPublisher.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/CesiumPublisher.h>
#include <VersionCompare/VersionCompare.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include "Constants.h"

USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUM
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_VERSIONCOMPARE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC


//=======================================================================================
// @bsistruct                                                   Diego.Pinate    09/17
//=======================================================================================
struct CompareChangeSet : BentleyApi::BeSQLite::ChangeSet
    {
    /*---------------------------------------------------------------------------------**//**
     * @bsimethod                                                    Diego.Pinate    03/17
     +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override
        {
        Utf8CP tableName = nullptr;
        int nCols, indirect;
        DbOpcode opcode;
        DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(result == BE_SQLITE_OK);
        UNUSED_VARIABLE(result);

        if (cause == ChangeSet::ConflictCause::NotFound && opcode == DbOpcode::Delete) // a delete that is already gone.
            return ChangeSet::ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.

        return ChangeSet::ConflictResolution::Replace;
        }
    };  // CompareChangeSet




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServiceLocalState : public IJsonLocalState
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap()
            {
            return m_map;
            }
        //! Saves the Utf8String value in the local state.
        //! @note The nameSpace and key pair must be unique.
        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);

            if (value == "null")
                {
                m_map.removeMember(identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };
        //! Returns a stored Utf8String from the local state.
        //! @note The nameSpace and key pair uniquely identifies the value.
        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);
            return m_map.isMember(identifier) ? m_map[identifier].asCString() : "null";
            };
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr copyToTemporaryBriefcase (BeFileNameR tempDbName, BeFileNameCR inputDbName)
    {
    BeFileName      tempPath;
    WString         fileName;
    BentleyStatus   status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPath, L"TempBriefcase");

    BeAssert(SUCCESS == status && "Cannot get temporary directory");

    tempDbName.BuildName(nullptr, tempPath.c_str(), inputDbName.GetFileNameAndExtension().c_str(), nullptr);

    // Try to create temporary file by copying base bim file
    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(inputDbName, tempDbName))
        {
        BeAssert(false && "unable to copy temporary DB.");
        return nullptr;
        }
    return DgnDb::OpenDgnDb(nullptr, tempDbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    }



//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct TilesetRevisionPublisher : TilesetPublisher
{
    DEFINE_T_SUPER(TilesetPublisher);

private:
    bool        m_preview;
    int         m_index;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetRevisionPublisher(DgnDbR db, PublisherParamsCR params, AxisAlignedBox3dCR projectExtents, WStringCR revisionName, int index, bool preview) : m_preview(preview), m_index(index),
    TilesetPublisher(db, DgnViewIdSet(), DgnViewId(), projectExtents, params.GetOutputDirectory(), params.GetTilesetName(), params.GetGeoLocation(), 5,
            params.GetDepth(), params.SurfacesOnly(), params.WantVerboseStatistics(), params.GetTextureMode(), params.WantProgressOutput(), params.GetGlobeMode())
    {
    m_dataDir.AppendToPath(L"History").AppendSeparator();
    m_dataDir.AppendToPath(revisionName.c_str()).AppendSeparator();
    m_dataDir.AppendToPath(preview ? L"Pre" : L"Post").AppendSeparator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublishRevision(DgnModelIdSet const& modelIds, DgnElementIdSet const& elementIds, PublisherParamsCR params)
    {
    struct RevisionCollectionFilter : ITileCollectionFilter
        {
        DgnElementIdSet const& m_ids;

        RevisionCollectionFilter(DgnElementIdSet const& ids) : m_ids(ids) {}

        virtual bool _AcceptElement(DgnElementId elementId) const override { return m_ids.Contains(elementId); }
        };

    auto status = InitializeDirectories(GetDataDirectory());
    if (Status::Success != status)
        return status;


    if (!elementIds.empty())                                                                                                        {
        ProgressMeter               progressMonitor(*this);
        RevisionCollectionFilter    collectionFilter(elementIds);
        TileGenerator               tileGenerator(m_db, &collectionFilter, &progressMonitor);

        auto generateStatus = tileGenerator.GenerateTiles(*this, modelIds, params.GetTolerance(), params.SurfacesOnly(), s_maxPointsPerTile);
        }

    return PublisherContext::Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AddBatchTableAttributes (Json::Value& json, FeatureAttributesMapCR attrs) override
    {
    T_Super::_AddBatchTableAttributes(json, attrs);

    Json::Value revisions = Json::arrayValue;

    for (size_t i=0; i<attrs.size(); i++)
        revisions.append(m_index);

    json[m_preview ? "preRevision" : "revision"] = std::move(revisions);
    }

};  // TilesetRevisionPublisher


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct     HistoryPublisher
{

    ClientPtr                               m_client;
    iModelConnectionPtr                     m_connection;
    DgnDbPtr                                m_tempDb;
    BeFileName                              m_tempDbName;
    bvector<DgnRevisionPtr>                 m_changeSets;
    bvector<ChangeSetInfoPtr>               m_changeSetInfos;
    PublisherParamsCR                       m_params;
    TilesetPublisher&                       m_tipPublisher;

    HistoryPublisher(PublisherParamsCR params, TilesetPublisher& tipPublisher) : m_params(params), m_tipPublisher(tipPublisher) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelIdSet   GetElementModelIds(DgnElementIdSet const& elementIds)
    {
    DgnModelIdSet       modelIds;

    for (auto& elementId : elementIds)
        {
        auto modelId = m_tempDb->Elements().Get<DgnElement>(elementId)->GetModelId();

        if (modelIds.find(modelId) == modelIds.end())
            {
            auto model = m_tempDb->Models().GetModel(modelId);

            if (nullptr != model->ToGeometricModelP())
                modelIds.insert(modelId);
            }
        }
    return modelIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status Initialize()
    {
    m_client = m_params.GetClient();    

    if (!m_client.IsValid())
        return TilesetPublisher::Status::CantConnectToIModelHub;

    DgnDbPtr    db;

    if (HistoryMode::OmitHistory == m_params.GetHistoryMode())
        {
        db = &m_tipPublisher.GetDgnDb();
        }
    else
        {
        m_tempDb = copyToTemporaryBriefcase(m_tempDbName, m_params.GetInputFileName());

        if (!m_tempDb.IsValid())
            return TilesetPublisher::Status::CantAcquireBriefcase;

        db = m_tempDb;
        }

    m_connection = VersionCompareUtilities::GetiModelConnection(db.get(), m_client);

    if (!m_connection.IsValid())
        return TilesetPublisher::Status::CantConnectToIModelHub;

    bool        isBackwardsRoll;
    Utf8String  filename(db->GetFileName().GetFileNameWithoutExtension());

    VersionSelector::GetChangeSetsToApply(m_changeSets, m_changeSetInfos, isBackwardsRoll, m_client, db.get(), "MasterFile", filename);

    return m_changeSets.empty() ? TilesetPublisher::Status::NoHistoryFound : TilesetPublisher::Status::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void  WriteChangeSetInfoJson(Json::Value& revisionJson, ChangeSetInfoCR changeSetInfo)
    {
    double      julianDay;

    if (SUCCESS == changeSetInfo.GetPushDate().ToJulianDay(julianDay))
        revisionJson["Date"] = julianDay;

    UserInfoTaskPtr task = m_connection->GetUserInfoManager().QueryUserInfoById(changeSetInfo.GetUserCreated());

    if (task->GetResult().IsSuccess())
        {
        auto            userInfo = task->GetResult().GetValue();
        Utf8String      userName = userInfo->GetName() + Utf8String(" ") + userInfo->GetSurname();

        revisionJson["User"] = userName;
        }

    revisionJson["Description"] = changeSetInfo.GetDescription();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GetChanges(DgnElementIdSet& addedOrModifiedIds, DgnElementIdSet& deletedOrModifiedIds, VersionCompareChangeSummary& changeSummary)
    {
    bvector<DgnElementId>               elementIds;
    bvector<BentleyApi::ECN::ECClassId> ecClassIds;
    bvector<DbOpcode>                   opCodes;


    changeSummary.GetChangedElements(elementIds, ecClassIds, opCodes);

    Json::Value         revisionElementsJson = Json::objectValue;

    for (size_t j=0; j<elementIds.size(); j++)
        {
        auto                elementId = elementIds.at(j);
        Json::Value         elementJson = Json::objectValue;
        auto const&         element = m_tempDb->Elements().Get<DgnElement>(elementId);

        elementJson["op"] = (int) opCodes.at(j);

        switch(opCodes.at(j))
            {
            case DbOpcode::Delete:
                deletedOrModifiedIds.insert(elementId);
                break;

            case DbOpcode::Insert:
                addedOrModifiedIds.insert(elementId);
                break;

            case DbOpcode::Update:
                {
                deletedOrModifiedIds.insert(elementId);
                addedOrModifiedIds.insert(elementId);

                Json::Value propertyData;
                changeSummary.GetPropertyContentComparison(elementId, ecClassIds.at(j), true, propertyData);

                Json::Value displayValues = propertyData["ContentSet"][0]["DisplayValues"];
                bvector<Utf8String> propertyNames = displayValues.getMemberNames();
                static Utf8String s_opCodeCompare("__ver_compare___Opcode");

                for (auto const& propertyName : propertyNames)
                    {
                    if (propertyName != s_opCodeCompare)
                        {
                        Json::Value     propertyChangeJson;

                        propertyChangeJson["name"] = propertyName;
#ifdef PROPERTY_CHANGE_VALUES
                        propertyChangeJson["pre"]  =  displayValues[propertyName]["Target"].asString();
                        propertyChangeJson["post"] =  displayValues[propertyName]["Current"].asString();
#endif

                        elementJson["prop"] = propertyChangeJson;
                        }
                    }
                break;
                }
            }
        revisionElementsJson[elementId.ToString()] = std::move(elementJson);
        }
    return revisionElementsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status PublishChangeSets(Json::Value& revisionsJson, TilesetPublisher& tipPublisher)
    {
    AxisAlignedBox3d        projectExtents = tipPublisher.GetProjectExtents();

    LOG.infov ("Publishing History with %d Revisions\n", (int) m_changeSets.size());

    revisionsJson = Json::objectValue;

    for (int i = 0; i<m_changeSets.size(); i++)
        {
        bvector<DgnRevisionPtr>             thisRevisions(1, m_changeSets.at(i));
        int                                 revisionIndex = m_changeSets.size() - i - 1;
        WString                             revisionName = WPrintfString(L"Revision_%d", revisionIndex);
        WString                             outputFileName = tipPublisher.GetDataDirectory() + L"History\\" + revisionName + L"\\Revision_Appdata.json";
        Utf8String                          outputRelativePath = Utf8String(outputFileName).c_str() + tipPublisher.GetOutputDirectory().size();
        Json::Value                         revisionJson = VersionSelector::WriteRevisionToJson(*m_changeSets.at(i));

        outputRelativePath.ReplaceAll("\\", "//");
        revisionJson["url"] = outputRelativePath;

        WriteChangeSetInfoJson(revisionJson, *m_changeSetInfos.at(i));
        revisionsJson[Utf8PrintfString("%d", revisionIndex).c_str()] = revisionJson;


        // Don't recreate revision if it already exists.
        if (BeFileName::DoesPathExist(outputFileName.c_str()) ||
            HistoryMode::OmitHistory == m_params.GetHistoryMode())
            continue;

        VersionCompareChangeSummaryPtr      changeSummary = VersionCompareChangeSummary::Generate(*m_tempDb, thisRevisions, true);
        DgnElementIdSet                     addedOrModifiedIds, deletedOrModifiedIds;
        Json::Value                         revisionElementsJson = GetChanges (addedOrModifiedIds, deletedOrModifiedIds, *changeSummary);
        DgnModelIdSet                       prevModelIds, postModelIds;
        auto                                prevModelIterator = changeSummary->GetTargetDb()->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel));
        auto                                postModelIterator = m_tempDb->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel));

        for (auto& model : prevModelIterator)
            prevModelIds.insert(model.GetModelId());

        for (auto& model :postModelIterator)
            postModelIds.insert(model.GetModelId());

        if (addedOrModifiedIds.empty() && deletedOrModifiedIds.empty() && prevModelIds == postModelIds)
            continue;           // Skip empty revisions.


        if (!addedOrModifiedIds.empty())
            {
            TilesetRevisionPublisher    revisionPublisher(*m_tempDb, m_params, projectExtents, revisionName, revisionIndex, false);
            DgnModelIdSet               modelIds = GetElementModelIds(addedOrModifiedIds);

            for (auto& modelId : postModelIds)
                if (prevModelIds.find(modelId) == prevModelIds.end())
                    modelIds.insert(modelId);

            if (!modelIds.empty())
                revisionPublisher.PublishRevision(modelIds, addedOrModifiedIds, m_params);

            revisionJson["postModels"] = revisionPublisher.GetModelsJson(modelIds);
            }

        // Roll to previous revision.
        Utf8String          rollTo = (i == m_changeSets.size() - 1) ? "MasterFile" : m_changeSets.at(i+1)->GetId();
        VersionSelector::RollTemporaryDb(m_client, m_tempDb.get(), m_tempDb.get(), rollTo, Utf8String(m_tempDbName));

        if (!deletedOrModifiedIds.empty())
            {
            TilesetRevisionPublisher    revisionPublisher(*m_tempDb, m_params, projectExtents, revisionName, revisionIndex, true);
            DgnModelIdSet               modelIds = GetElementModelIds(deletedOrModifiedIds);

            if (!modelIds.empty())
                revisionPublisher.PublishRevision(modelIds, deletedOrModifiedIds, m_params);

            revisionJson["preModels"] = revisionPublisher.GetModelsJson(modelIds);
            }

        revisionJson["name"] = Utf8PrintfString("1.%d", revisionIndex + 1);
        revisionJson["elements"] = std::move(revisionElementsJson);
        revisionJson.removeMember("url");

        TileUtil::WriteJsonToFile (outputFileName.c_str(), revisionJson);
        }
    return TilesetPublisher::Status::Success;
    }

};  // HistoryPublisher

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetHistoryPublisher::PublishHistory(Json::Value& revisionsJson, PublisherParamsCR params, TilesetPublisher& tipPublisher)
    {
    TilesetPublisher::Status    status;
    HistoryPublisher            publisher(params, tipPublisher);

    if (Status::Success != (status = publisher.Initialize()))
        return status;

    return publisher.PublishChangeSets(revisionsJson, tipPublisher);
    }

