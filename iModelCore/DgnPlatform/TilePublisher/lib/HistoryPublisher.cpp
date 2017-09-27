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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr  acquireTemporaryBriefcase(BeFileNameR tempDbName, ClientPtr client, Utf8StringCR projectName, Utf8StringCR repositoryName)
    {
    BeFileName      tempPath;
    BentleyStatus   status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPath, L"TempBriefcase");

    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempDbName.BuildName(nullptr, tempPath.c_str(), WString(repositoryName.c_str(), false).c_str(), L".ibim");

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetHistoryPublisher::PublishTilesetWithHistory(PublisherParamsR params)
    {
    auto client = doSignIn(params);

    if (!client.IsValid())
        return Status::CantConnectToIModelHub;

    BeFileName  tempDbName;
    auto        tempDb = acquireTemporaryBriefcase(tempDbName, client, params.GetProjectName(), params.GetRepositoryName());

    if (!tempDb.IsValid())
        return Status::CantAcquireBriefcase;

    DgnViewIdSet    viewsToPublish;
    DgnViewId       defaultView = params.GetViewIds(viewsToPublish, *tempDb);

    if (!defaultView.IsValid())
        return Status::CantFindDefaultView;

    bvector<ChangeSetInfoPtr>   allChangeSets;
    bvector<FileInfoPtr>        masterFiles;

    // Get the information of all changesets present in the server
    if (SUCCESS != VersionSelector::GetRevisions(client, tempDb.get(), allChangeSets, masterFiles))
        return Status::CantExtractHistory;
    
    for (size_t i=allChangeSets.size()-2; i>=0; i++)
        {
        // Select the first one as an example and get the ID
        Utf8String changesetId = allChangeSets.at(i)->GetId();

        // Generate a change summary to compare to the target changeset ID
        VersionCompareChangeSummaryPtr changeSummary = VersionCompareChangeSummary::Generate(client, *tempDb, changesetId);
        if (!changeSummary.IsValid())
            return Status::CantExtractHistory;

        bvector<DgnElementId>   elementIds;
        DgnElementIdSet         addedOrModifiedIds, deletedIds;
        bvector<ECClassId>      ecclassIds;
        bvector<DbOpcode>       opCodes;
        // Get the IDs, ECClass IDs and Opcodes (change type) of the elements that were affected by the changesets
        if (SUCCESS == changeSummary->GetChangedElements(elementIds, ecclassIds, opCodes) && !elementIds.empty())
            {
            for (size_t i=0; i<elementIds.size(); i++)
                {
                switch(opCodes.at(i))
                    {
                    case DbOpcode::Delete:
                        deletedIds.insert(elementIds.at(i));
                        break;

                    case DbOpcode::Insert:
                        addedOrModifiedIds.insert(elementIds.at(i));
                        break;

                    case DbOpcode::Update:
                        deletedIds.insert(elementIds.at(i));
                        addedOrModifiedIds.insert(elementIds.at(i));
                        break;
                    }
                }

            if (!addedOrModifiedIds.empty())
                {
                TilesetRevisionPublisher  revisionPublisher(*tempDb, params);

                revisionPublisher.PublishRevision(addedOrModifiedIds, deletedIds, i, params);
                }
            // Write JSON with revision information (description?) and deleted Ids.
            }

        // Roll to previous revision.
        VersionSelector::RollTemporaryDb(client, tempDb.get(), tempDb.get(), changesetId, Utf8String(tempDbName));
        }

    TilesetPublisher  publisher(*tempDb, params, viewsToPublish, defaultView);

    publisher.Publish(params);


    return Status::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetRevisionPublisher::PublishRevision(DgnElementIdSet const&  addedIds, DgnElementIdSet const& removedIds, size_t index, PublisherParamsR params)
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
    


    if (!addedIds.empty())
        {
        DgnModelIdSet               modelIds;
        ProgressMeter               progressMonitor(*this);
        RevisionCollectionFilter    collectionFilter(addedIds);
        TileGenerator               tileGenerator(m_db, &collectionFilter, &progressMonitor);
    
        for (auto& addedId : addedIds)
            modelIds.insert(m_db.Elements().Get<DgnElement>(addedId)->GetModelId());

        auto generateStatus = tileGenerator.GenerateTiles(*this, modelIds, params.GetTolerance(), params.SurfacesOnly(), s_maxPointsPerTile);
        }

    return PublisherContext::Status::Success;
    }

