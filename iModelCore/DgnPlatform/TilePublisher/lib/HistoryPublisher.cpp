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

    return VersionSelector::GetChangeSetsToApply(changesets, isBackwardsRoll, client, &db, "MasterFile", filename);
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
    WebServices::UrlProvider::Environment   urlEnvironment = WebServices::UrlProvider::Environment::Qa;  


    if (params.GetEnvironment().StartsWithI("Dev"))
        urlEnvironment =  urlEnvironment = WebServices::UrlProvider::Environment::Dev;
    else if (0 == params.GetEnvironment().CompareToI("Release"))
        urlEnvironment = WebServices::UrlProvider::Environment::Release; 
        
        
    credentials.SetUsername(params.GetUser());
    credentials.SetPassword(params.GetPassword());

    Http::HttpClient::Initialize(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    iModel::Hub::ClientHelper::Initialize(getClientInfo(), getLocalState());
    UrlProvider::Initialize(urlEnvironment, UrlProvider::DefaultTimeout, getLocalState());

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

#ifdef NOTNOW_BRIEFCASE_IS_ROLLED_BACK
    DgnDbPtr    existingBriefcase = DgnDb::OpenDgnDb(nullptr, tempDbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    if (existingBriefcase.IsValid())
        return existingBriefcase;
#endif

    WebServices::WSError wserror;
    auto projectId = iModel::Hub::ClientHelper::GetInstance()->QueryProjectId(&wserror, projectName);

    Utf8String      repositoryNameNoSpaces = repositoryName;

    repositoryNameNoSpaces.ReplaceAll(" ", "%20");
    auto getIModelResult = client->GetiModelByName(projectId, repositoryNameNoSpaces)->GetResult();

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

#define FABRICATE_FAKE_REVISION_DATA
#ifdef FABRICATE_FAKE_REVISION_DATA
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void fabricateFakeRevisionData(Json::Value& revisionJson)
    {
    static          double s_time = 0.0;
    static          double s_fakeTimeInterval = 2.0;     
    static char*    s_fakeUsers[] = {"John Braun", "Charles Johnson", "Sharon Kowalski" };
    static char*    s_fakeDescriptions[] = {"Add HVAC", "Modify roof", "Add plumbing", "Rework HVAC" };
    static int      s_fakeUserIndex, s_fakeDescriptionIndex;
    DateTime::Info  info;

    if (0.0 == s_time)
        DateTime::GetCurrentTime().ToJulianDay(s_time);


    if (!revisionJson.isMember("Date"))
        revisionJson["Date"] = s_time;

    if (0 == revisionJson["User"].asString().size())
        revisionJson["User"] = s_fakeUsers[s_fakeUserIndex++ % 3];

    s_time -= s_fakeTimeInterval;
    }
#endif


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct TilesetRevisionPublisher : TilesetPublisher
{
    DEFINE_T_SUPER(TilesetPublisher);
    
private:
    WString     m_revisionName;
    bool        m_preview;
    int         m_index;

public:    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetRevisionPublisher(DgnDbR db, PublisherParamsR params, int index, bool preview) : m_preview(preview), m_index(index),
    TilesetPublisher(db, DgnViewIdSet(), DgnViewId(), params.GetOutputDirectory(), params.GetTilesetName(), params.GetGeoLocation(), 5,
            params.GetDepth(), params.SurfacesOnly(), params.WantVerboseStatistics(), params.GetTextureMode(), params.WantProgressOutput()) 
    { 
    m_revisionName = WPrintfString(L"Revision_%d", index);
    if (preview)
        m_revisionName += L"_Preview";

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
struct BaselinePublisher : TilesetPublisher
{
    Json::Value             m_json;
    DgnElementIdSet         m_allModelSelectors;
    T_CategorySelectorMap   m_allCategorySelectors;
    DgnElementIdSet         m_allDisplayStyles;
    DgnModelIdSet           m_all2dModelIds;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BaselinePublisher(DgnDbR db, PublisherParamsR params,  DgnViewIdSet const& viewIds, DgnViewId defaultViewId) : 
    TilesetPublisher(db, viewIds, defaultViewId, params.GetOutputDirectory(), params.GetTilesetName(), params.GetGeoLocation(), 5,
            params.GetDepth(), params.SurfacesOnly(), params.WantVerboseStatistics(), params.GetTextureMode(), params.WantProgressOutput()) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status Initialize()
    {
    Status          status;

    Utf8String rootNameUtf8(m_rootName.c_str());
    m_json["name"] = rootNameUtf8;

    // TODO - Set ground point.

    
    ExtractViewSelectors (m_defaultViewId, m_allModelSelectors, m_allCategorySelectors, m_allDisplayStyles, m_all2dModelIds);

    if (!m_defaultViewId.IsValid())
        return Status::NoGeometry;

    m_json["views"] = GetViewDefinitionsJson();
    m_json["defaultView"] = m_defaultViewId.ToString();

    WriteCategoriesJson(m_json, m_allCategorySelectors, false);
    m_json["displayStyles"] = GetDisplayStylesJson(m_allDisplayStyles);

    AxisAlignedBox3d projectExtents = m_projectExtents;
    Transform::FromProduct(m_spatialToEcef, m_dbToTile).Multiply(projectExtents, projectExtents);
    m_json["projectExtents"] = RangeToJson(projectExtents);
    m_json["projectTransform"] = TransformToJson(m_spatialToEcef);
    m_json["projectOrigin"] = PointToJson(m_projectExtents.GetCenter());
    
    return InitializeDirectories(GetDataDirectory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status Publish(PublisherParamsR params, Json::Value&& revisions)
    {
    DRange3d      range;
    ProgressMeter progressMeter(*this);
    TileGenerator generator (GetDgnDb(), nullptr, &progressMeter);                                                                                                                     

    m_generator = &generator;
    auto status = PublishViewModels(generator, range, params.GetTolerance(), params.SurfacesOnly(), progressMeter);
    m_generator = nullptr;

    if (Status::Success != status)
        {
        CleanDirectories(GetDataDirectory());
        return Status::Success != m_acceptTileStatus ? m_acceptTileStatus : status;
        }

    OutputStatistics(generator.GetStatistics());
    WriteModelsJson(m_json, m_allModelSelectors, m_all2dModelIds);

    Json::Value viewerOptions = params.GetViewerOptions();

    // If we are displaying "in place" but don't have a real geographic location - default to natural earth.
    if (!IsGeolocated() && viewerOptions["imageryProvider"].isNull())
        viewerOptions["imageryProvider"] = "NaturalEarth";

    m_json["viewerOptions"] = viewerOptions;
    m_json["revisions"] = std::move(revisions);

    if (Status::Success != (status = WriteAppJson (m_json)) ||
        Status::Success != (status = WriteHtmlFile()))
        return  status;

    return WriteScripts ();

    }

};  // Baseline Publisher


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetHistoryPublisher::PublishTilesetWithHistory(PublisherParamsR params)
    {
    auto client = doSignIn(params);
    
    if (!client.IsValid())
        return Status::CantConnectToIModelHub;

    BeFileName  tempDbName;
    DgnDbPtr    tempDb;

    if (!params.GetInputFileName().empty())
        tempDb = copyToTemporaryBriefcase(tempDbName, params.GetInputFileName());
    else
        tempDb = acquireTemporaryBriefcase (tempDbName, client, params.GetProject(), params.GetRepository());

    if (!tempDb.IsValid())
        return Status::CantAcquireBriefcase;

    DgnViewIdSet    viewsToPublish;
    DgnViewId       defaultView = params.GetViewIds(viewsToPublish, *tempDb);

    if (!defaultView.IsValid())
        return Status::CantFindDefaultView;

    bvector<DgnRevisionPtr> changeSets;
    std::list<Json::Value>  revisionJsons;

    getAllChangeSets(changeSets, *tempDb, client);

    printf ("Publishing History with %d Revisions", (int) changeSets.size());
    BaselinePublisher  baselinePublisher(*tempDb, params, viewsToPublish, defaultView);

    PublisherContext::Status    status;

    if (Status::Success != (status = baselinePublisher.Initialize()))
        return status;

    for (int i = 0; i<changeSets.size(); i++)
        {
        bvector<DgnElementId>   elementIds;
        bvector<DbOpcode>       opCodes;
             
        getChangedGeometricElements(elementIds, opCodes, tempDb, changeSets.at(i)); 

        printf ("Revision: %d Contains %d changed elements\n", i, (int) elementIds.size());
        Json::Value         revisionElementsJson = Json::objectValue;
        DgnElementIdSet     addedOrModifiedIds, deletedOrModifiedIds;

        for (size_t j=0; j<elementIds.size(); j++)
            {
            auto elementId = elementIds.at(j);

            switch(opCodes.at(j))
                {
                case DbOpcode::Delete:
                    deletedOrModifiedIds.insert(elementId);
                    break;

                case DbOpcode::Insert:
                    addedOrModifiedIds.insert(elementId);
                    break;

                case DbOpcode::Update:
                    deletedOrModifiedIds.insert(elementId);
                    addedOrModifiedIds.insert(elementId);
                    break;
                }
            revisionElementsJson[elementId.ToString()] = (int) opCodes.at(j);
            }

        Json::Value     revisionJson = VersionSelector::WriteRevisionToJson(*changeSets.at(i));

#ifdef FABRICATE_FAKE_REVISION_DATA
        fabricateFakeRevisionData(revisionJson);
#endif

        if (!addedOrModifiedIds.empty())
            {
            TilesetRevisionPublisher    revisionPublisher(*tempDb, params, changeSets.size() - i - 1, false);
            DgnModelIdSet               modelIds;

            for (auto& elementId : addedOrModifiedIds)
                modelIds.insert(tempDb->Elements().Get<DgnElement>(elementId)->GetModelId());

            revisionPublisher.PublishRevision(modelIds, addedOrModifiedIds, params);
            revisionJson["postModels"] = revisionPublisher.GetModelsJson(modelIds);
            }

        // Roll to previous revision.
        Utf8String          rollTo = (i == changeSets.size() - 1) ? "MasterFile" : changeSets.at(i+1)->GetId();
        VersionSelector::RollTemporaryDb(client, tempDb.get(), tempDb.get(), rollTo, Utf8String(tempDbName));

        if (!deletedOrModifiedIds.empty())
            {
            TilesetRevisionPublisher    revisionPublisher(*tempDb, params, changeSets.size() - i - 1, true);
            DgnModelIdSet               modelIds;

            for (auto& elementId : deletedOrModifiedIds)
                modelIds.insert(tempDb->Elements().Get<DgnElement>(elementId)->GetModelId());

            revisionPublisher.PublishRevision(modelIds, deletedOrModifiedIds, params);
            revisionJson["preModels"] = revisionPublisher.GetModelsJson(modelIds);
            }
            
        Utf8String  revisionName = Utf8PrintfString("Revision: %d", changeSets.size() - i);        // Needs work - Get real string from for name - they seem to be empty or nonsense now.

        revisionJson["name"] = revisionName;
        revisionJson["elements"] = std::move(revisionElementsJson);
        revisionJsons.push_front(revisionJson);
        }
    Json::Value     revisionsJson;
    for (auto& revisionJson : revisionJsons)
        revisionsJson.append(revisionJson);

    return baselinePublisher.Publish(params, std::move(revisionsJson));
    }

