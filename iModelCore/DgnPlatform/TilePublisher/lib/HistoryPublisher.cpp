/*--------------------------------------------------------------------------------------+                  
|
|     $Source: TilePublisher/lib/HistoryPublisher.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/CesiumPublisher.h>
#include <VersionCompare/VersionCompare.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>
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
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void getChangedGeometricElements(bvector<DgnElementId>& elementIds, bvector<DbOpcode>& opcodes, DgnDbPtr db, DgnRevisionPtr changeset)
    {
    ChangeSummary summary(*db);
    ChangeGroup changeGrp;
    BeFileNameCR changesFile = changeset->GetRevisionChangesFile();
    RevisionChangesFileReader stream(changesFile, *db);
    stream.ToChangeGroup(changeGrp);
    CompareChangeSet set;
    set.FromChangeGroup(changeGrp);
    summary.FromChangeSet(set);

    ECClassCP geomClass = db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_GeometricElement);
    bmap<ECInstanceId, ChangeSummary::Instance> changedElements;
    summary.QueryByClass(changedElements, geomClass->GetId());

    for (auto changedElement : changedElements)
        {
        DgnElementId elementId (changedElement.first.GetValue());
        DbOpcode opcode = changedElement.second.GetDbOpcode();
        elementIds.push_back(elementId);
        opcodes.push_back(opcode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getAllChangeSets(bvector<DgnRevisionPtr>& changesets, DgnDbR db, ClientPtr client)
    {
    bool isBackwardsRoll;
    Utf8String filename(db.GetFileName().GetFileNameWithoutExtension());
    Utf8String firstChangesetId = VersionSelector::GetFileFirstChangeSet(client, &db, filename);

    return VersionSelector::GetChangeSetsToApply(changesets, isBackwardsRoll, client, &db, firstChangesetId, filename);
    }

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
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static ServiceLocalState* getLocalState()
    {
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    ServiceLocalState* s_localState = new ServiceLocalState;
    return s_localState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static WebServices::ClientInfoPtr getClientInfo()
    {
    static Utf8CP s_productId = "1654"; // Navigator Desktop
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    static WebServices::ClientInfoPtr s_clientInfo = WebServices::ClientInfoPtr(
        new WebServices::ClientInfo("Bentley-Test", BeVersion(1, 0), "{41FE7A91-A984-432D-ABCF-9B860A8D5360}", "TestDeviceId", "TestSystem", s_productId));
    return s_clientInfo;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static ClientPtr   doSignIn(PublisherParams const& params)
    {
    Credentials                 credentials;
    UrlProvider::Environment    environment = WebServices::UrlProvider::Environment::Qa;

    credentials.SetUsername(params.GetUser());
    credentials.SetPassword(params.GetPassword());

    Http::HttpClient::Initialize(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    iModel::Hub::ClientHelper::Initialize(getClientInfo(), getLocalState());
    UrlProvider::Initialize(environment, UrlProvider::DefaultTimeout, getLocalState());

    Tasks::AsyncError error;        
    return iModel::Hub::ClientHelper::GetInstance()->SignInWithCredentials(&error, credentials);
    }

#ifdef NOT_NOW
// This acquires a temporary briefcase at TIP - We're not using it now as we are always working from 
// an existing briefcase.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr  acquireTemporaryBriefcase(BeFileNameR tempDbName, ClientPtr client, Utf8StringCR projectName, Utf8StringCR repositoryName)
    {
    BeFileName      tempPath;
    BentleyStatus   status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPath, L"TempBriefcase");

    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempDbName.BuildName(nullptr, tempPath.c_str(), WString(repositoryName.c_str(), false).c_str(), L".ibim");

    DgnDbPtr    existingBriefcase = DgnDb::OpenDgnDb(nullptr, tempDbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    if (existingBriefcase.IsValid())
        return existingBriefcase;

    WebServices::WSError wserror;
    auto projectId = iModel::Hub::ClientHelper::GetInstance()->QueryProjectId(&wserror, projectName);

    auto getIModelResult = client->GetiModelByName(projectId, repositoryName)->GetResult();

    if (!getIModelResult.IsSuccess())
        return nullptr;

    if (BeFileName::DoesPathExist(tempDbName.c_str())) 
        BeFileName::BeDeleteFile(tempDbName.c_str()); 

    auto acquireBriefcaseResult = client->AcquireBriefcase(*getIModelResult.GetValue(), tempDbName)->GetResult();

    if (!acquireBriefcaseResult.IsSuccess())
        return nullptr;
    
    return DgnDb::OpenDgnDb(nullptr, tempDbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    }
#endif
// This acquires a temporary briefcase at TIP - We're not using it now as we are always working from 
// an existing briefcase.
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
private:
    WString     m_revisionName;

public:    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetRevisionPublisher(DgnDbR db, PublisherParamsR params, int index) : 
    TilesetPublisher(db, DgnViewIdSet(), DgnViewId(), params.GetOutputDirectory(), params.GetTilesetName(), params.GetGeoLocation(), 5,
            params.GetDepth(), params.SurfacesOnly(), params.WantVerboseStatistics(), params.GetTextureMode(), params.WantProgressOutput()) 
    { 
    m_revisionName = WPrintfString(L"Revision_%d", index);
    m_revisionIndex = index;
    m_dataDir.AppendToPath(m_revisionName.c_str()).AppendSeparator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublishRevision(DgnModelIdSet const& modelIds, DgnElementIdSet const& elementIds, PublisherParamsR params)
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
    

    if (!elementIds.empty())
        {
        ProgressMeter               progressMonitor(*this);
        RevisionCollectionFilter    collectionFilter(elementIds);
        TileGenerator               tileGenerator(m_db, &collectionFilter, &progressMonitor);
    
        auto generateStatus = tileGenerator.GenerateTiles(*this, modelIds, params.GetTolerance(), params.SurfacesOnly(), s_maxPointsPerTile);
        }

    return PublisherContext::Status::Success;
    }

   
};  // TilesetRevisionPublisher

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetHistoryPublisher::PublishTilesetWithHistory(PublisherParamsR params)
    {
    auto client = doSignIn(params);

    if (!client.IsValid())
        return Status::CantConnectToIModelHub;

    BeFileName  tempDbName;
    auto        tempDb = copyToTemporaryBriefcase(tempDbName, params.GetInputFileName());

    if (!tempDb.IsValid())
        return Status::CantAcquireBriefcase;

    DgnViewIdSet    viewsToPublish;
    DgnViewId       defaultView = params.GetViewIds(viewsToPublish, *tempDb);

    if (!defaultView.IsValid())
        return Status::CantFindDefaultView;

    bvector<DgnRevisionPtr> changeSets;
    std::list<Json::Value>  revisionJsons;

    getAllChangeSets(changeSets, *tempDb, client);

    TilesetPublisher  tilesetPublisher(*tempDb, params, viewsToPublish, defaultView);

    tilesetPublisher.InitializeDirectories(tilesetPublisher.GetDataDirectory());

    for (int i = 0; i<changeSets.size(); i++)
        {
        bvector<DgnElementId>   elementIds;
        bvector<DbOpcode>       opCodes;
             
        getChangedGeometricElements(elementIds, opCodes, tempDb, changeSets.at(i)); 

        if (!elementIds.empty())
            {
            Json::Value         revisionElementsJson = Json::objectValue;
            DgnElementIdSet     addedOrModifiedIds, deletedIds;

            for (size_t j=0; j<elementIds.size(); j++)
                {
                auto elementId = elementIds.at(j);

                switch(opCodes.at(j))
                    {
                    case DbOpcode::Delete:
                        deletedIds.insert(elementId);
                        break;

                    case DbOpcode::Insert:
                        addedOrModifiedIds.insert(elementId);
                        break;

                    case DbOpcode::Update:
                        deletedIds.insert(elementId);
                        addedOrModifiedIds.insert(elementId);
                        break;
                    }
                revisionElementsJson[elementId.ToString()] = (int) opCodes.at(j);
                }

            Json::Value     revisionJson = VersionSelector::WriteRevisionToJson(*changeSets.at(i));

            if (!addedOrModifiedIds.empty())
                {
                TilesetRevisionPublisher    revisionPublisher(*tempDb, params, changeSets.size() - i);
                DgnModelIdSet               modelIds;

                for (auto& elementId : addedOrModifiedIds)
                    modelIds.insert(tempDb->Elements().Get<DgnElement>(elementId)->GetModelId());

                revisionPublisher.PublishRevision(modelIds, addedOrModifiedIds, params);
                revisionJson["models"] = revisionPublisher.GetModelsJson(modelIds);
                }
            Utf8String  revisionName = Utf8PrintfString("Revision: %d", changeSets.size() - i);        // Needs work - Get real string from for name - they seem to be empty or nonsense now.

            revisionJson["name"] = revisionName;
            revisionJson["elements"] = std::move(revisionElementsJson);
            revisionJsons.push_front(revisionJson);
            }

        // Roll to previous revision.
        VersionSelector::RollTemporaryDb(client, tempDb.get(), tempDb.get(), changeSets.at(i)->GetId(), Utf8String(tempDbName));
        }
    Json::Value     revisionsJson;
    for (auto& revisionJson : revisionJsons)
        revisionsJson.append(revisionJson);

    // Needs work - This publishes in initial state and doesn't include changes to views/categories etc.
    tilesetPublisher.SetRevisionsJson(std::move(revisionsJson));
    tilesetPublisher.Publish(params, false);

    return Status::Success;
    }

