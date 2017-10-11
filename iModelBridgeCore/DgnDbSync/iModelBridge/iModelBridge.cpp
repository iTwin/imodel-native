/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridge.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

static L10NLookup* s_bridgeL10NLookup = NULL;

// Helper class to ensure that bridge book mark functions are called
struct CallBookmarkFunctions
    {
    BentleyStatus m_bstatus;
    BentleyStatus m_sstatus = BSIERROR;
    iModelBridge& m_bridge;
    BentleyStatus m_updateStatus = BSIERROR;
    CallBookmarkFunctions(iModelBridge& bridge, DgnDbR db) : m_bridge(bridge)
        {
        m_bstatus = m_bridge._OnConvertToBim(db);
        if (BSISUCCESS == m_bstatus)
            m_sstatus = m_bridge._OpenSource();
        }
    ~CallBookmarkFunctions()
        {
        if (BSISUCCESS == m_bstatus)
            {
            if (BSISUCCESS == m_sstatus)
                m_bridge._CloseSource(m_updateStatus);
            m_bridge._OnConvertedToBim(m_updateStatus);
            }
        }

    bool IsBridgeReady() const {return (BSISUCCESS==m_bstatus) && (BSISUCCESS==m_sstatus);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge::Params::SetReportFileName()
    {
    m_reportFileName = GetBriefcaseName();
    m_reportFileName.append(L"-issues");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void queryAllModels(bvector<DgnModelId>& models, DgnDbR db)
    {
    BeSQLite::Statement stmt;
    stmt.Prepare(db, "SELECT Id from bis_Model");
    while (BeSQLite::BE_SQLITE_ROW == stmt.Step())
        models.push_back(stmt.GetValueId<DgnModelId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr iModelBridge::DoCreateDgnDb(bvector<DgnModelId>& jobModels, Utf8CP rootSubjectDescription)
    {
    BeAssert(!_GetParams().GetInputFileName().empty());
    BeAssert(!_GetParams().GetBriefcaseName().empty());

    CreateDgnDbParams createProjectParams;
    if (nullptr != rootSubjectDescription)
        createProjectParams.SetRootSubjectDescription(rootSubjectDescription);

    createProjectParams.SetRootSubjectName("TBD"); // WIP_BRIDGE

    // Create the DgnDb file. All currently registered domain schemas are imported.
    BeSQLite::DbResult createStatus;
    auto db = DgnDb::CreateDgnDb(&createStatus, _GetParams().GetBriefcaseName(), createProjectParams);
    if (!db.IsValid())
        {
        LOG.fatalv(L"Failed to create repository [%s] with error %x", createStatus, _GetParams().GetBriefcaseName().c_str());
        return nullptr;
        }

    bvector<DgnModelId> baseModels;
    queryAllModels(baseModels, *db);

    _GetParams().SetIsCreatingNewDgnDb(true);
    _GetParams().SetIsUpdating(false);

    _DeleteSyncInfo(); // Make sure that there is no old syncinfo file hanging around

    // Call bridge _OnConvertToBim and _OpenSource
    CallBookmarkFunctions bookMarkFunctions(*this, *db);
    if (!bookMarkFunctions.IsBridgeReady())
        {
        LOG.fatalv("Bridge not ready to populate new repository");
        return nullptr;
        }

    auto jobsubj = _InitializeJob();
    if (!jobsubj.IsValid())
        {
        LOG.fatalv("Failed to create job structure");
        return nullptr;
        }

    if (BSISUCCESS != _ConvertToBim(*jobsubj))
        {
        LOG.fatalv("Failed to populate new repository");
        return nullptr;
        }

    bookMarkFunctions.m_updateStatus = BSISUCCESS;

    bvector<DgnModelId> models;
    queryAllModels(models, *db);
    for (auto modelId : models)
        {
        if (baseModels.end() == std::find(baseModels.begin(), baseModels.end(), modelId))
            jobModels.push_back(modelId);
        }

    return db;
    // Call bridge's _CloseSource and _OnConvertedToBim
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr iModelBridge::OpenBim(BeSQLite::DbResult& dbres, bool& madeSchemaChanges, bool& hasDynamicSchemaChange)
    {
    madeSchemaChanges = false;

    //  Common case: Just open the BIM
    auto db = DgnDb::OpenDgnDb(&dbres, _GetParams().GetBriefcaseName(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    if (db.IsValid())
        {
        hasDynamicSchemaChange = _UpgradeDynamicSchema(*db);
        if (!hasDynamicSchemaChange)
            return db;// Common case

        db->SaveChanges();
        db->CloseDb();
        db = nullptr;
        return DgnDb::OpenDgnDb(&dbres, _GetParams().GetBriefcaseName(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        }

    if (BeSQLite::BE_SQLITE_ERROR_SchemaUpgradeRequired != dbres)
        return nullptr;

    // We must do a schema upgrade.
    // Probably, the bridge registered some required domains, and they must be imported
    DgnDb::OpenParams oparams(DgnDb::OpenMode::ReadWrite);
    oparams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains();
    db = DgnDb::OpenDgnDb(&dbres, _GetParams().GetBriefcaseName(), oparams);
    if (!db.IsValid())
        return nullptr;
    dbres = db->SaveChanges();
    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        BeAssert(false);
        LOG.fatalv("Failed to save results of importing domain schemas");
        return nullptr;
        }

    madeSchemaChanges = true;

    // ... close and re-open, so that the side-effects of the schema changes are reflected in the open DgnDb.
    db->CloseDb();
    db = nullptr;

    return DgnDb::OpenDgnDb(&dbres, _GetParams().GetBriefcaseName(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::DoConvertToExistingBim(DgnDbR db)
    {
    BeAssert(!_GetParams().GetInputFileName().empty());

    _GetParams().SetIsCreatingNewDgnDb(false);
    _GetParams().SetIsUpdating(true);

    // ***
    // ***
    // *** DO NOT CHANGE THE ORDER OF THE STEPS BELOW
    // *** Talk to Sam Wilson if you need to make a change.
    // ***
    // ***

    // NB: _OnConvertBim must be called before we start "bulk insert" mode.
    //      That is because it often does things like Db::AttachDb and create temp table,
    //		which need to commit the txn. We cannot commit while in bulk insert mode.

    // Call bridge _OnConvertToBim and _OpenSource
    CallBookmarkFunctions bookMarkFunctions(*this, db);
    if (!bookMarkFunctions.IsBridgeReady())
        {
        LOG.fatalv("Bridge not ready to update briefcase");
        return BSIERROR;
        }

    //  go into bulk import mode. (Note that any locks and codes required by _OnBimOpen would have to have been acquired immediately, the normal way.)
    db.BriefcaseManager().StartBulkOperation();

    SubjectCPtr jobsubj = _FindJob();
    if (!jobsubj.IsValid())
        {
        _GetParams().SetIsUpdating(false);
        jobsubj = _InitializeJob();    // this is probably the first time that this bridge has tried to convert this input file into this iModel
        if (!jobsubj.IsValid())
            {
            LOG.fatalv("Failed to create job structure");
            return BSIERROR;
            }
        }

    if (BSISUCCESS != _ConvertToBim(*jobsubj))
        {
        LOG.fatalv("_ConvertToBim failed");
        return BSIERROR;
        }

    // Must either succeed in getting all required locks and codes ... or abort the whole txn.
    auto response = db.BriefcaseManager().EndBulkOperation();
    if (RepositoryStatus::Success != response.Result())
        {
        LOG.fatalv("Failed to acquire locks and/or codes with error %x", response.Result());
        return BSIERROR;
        }

    bookMarkFunctions.m_updateStatus = BSISUCCESS;

    return BSISUCCESS;
    // Call bridge's _CloseSource and _OnConvertedToBim
    }

// *******************
/// Command-line parsing utilities

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus parseGeoPointAndAzimuth(iModelBridge::GCSDefinition& gcs, Utf8StringCR u)
    {
    auto parser = AngleParser::Create();
    parser->SetAngleMode(AngleMode::DegMinSec);

    size_t start=0, end;

    if ((end = u.find(',')) == Utf8String::npos)
        return BSIERROR;

    if (BSISUCCESS != parser->ToValue(gcs.m_geoPoint.latitude, u.substr(start, end-start).c_str()))
        return BSIERROR;

    bool firstValueIsLongitude = (Utf8String::npos != u.substr(start, end-start).find_first_of("ewEW"));

    start = end + 1;

    if ((end = u.find(',', start)) == Utf8String::npos)
        return BSIERROR;

    if (BSISUCCESS != parser->ToValue(gcs.m_geoPoint.longitude, u.substr(start, end-start).c_str()))
        return BSIERROR;

    bool secondValueIsLatitude = (Utf8String::npos != u.substr(start, end-start).find_first_of("nsNS"));

    start = end + 1;

    if (BSISUCCESS != parser->ToValue(gcs.m_azimuthAngle, u.substr(start).c_str()))
        return BSIERROR;

    if (firstValueIsLongitude || secondValueIsLatitude)
        {
        std::swap(gcs.m_geoPoint.longitude, gcs.m_geoPoint.latitude);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::Params::ParseGCSCalculationMethod(GCSCalculationMethod& cm, Utf8StringCR value)
    {
    if (value.EqualsI("default"))
        {
        cm = GCSCalculationMethod::UseDefault;
        return BSISUCCESS;
        }
    if (value.EqualsI("reproject"))
        {
        cm = GCSCalculationMethod::UseReprojection;
        return BSISUCCESS;
        }
    if (value.EqualsI("transform"))
        {
        cm = GCSCalculationMethod::UseGcsTransform;
        return BSISUCCESS;
        }
    if (value.EqualsI("transformscaled"))
        {
        cm = GCSCalculationMethod::UseGcsTransformWithScaling;
        return BSISUCCESS;
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::Params::ParseGcsSpec(GCSDefinition& gcs, Utf8StringCR gcsParms)
    {
    if (gcsParms.empty())
        {
        fprintf(stderr, "expected at least coordinate system key name");
        return BSIERROR;
        }

    if (isalpha(gcsParms[0]))
        gcs.m_coordSysKeyName = gcsParms;
    else
        {
        gcs.m_coordSysKeyName.clear();
        if (BSISUCCESS != parseGeoPointAndAzimuth(gcs, gcsParms)
            && (3 != sscanf(gcsParms.c_str(), "%lf,%lf,%lf", &gcs.m_geoPoint.latitude, &gcs.m_geoPoint.longitude, &gcs.m_azimuthAngle)))
            {
            fprintf(stderr, "%s - invalid GCS values - expected latitude,longitude,azimuthangle\n", gcsParms.c_str());
            return BSIERROR;
            }
        }

    gcs.m_geoPoint.elevation = 0.0;
    gcs.m_originUors.Init(0,0,0);
    gcs.m_isValid = true;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr iModelBridge::GCSDefinition::CreateGcs(DgnDbR db)
    {
    if (!m_isValid)
        return nullptr;

    if (!m_coordSysKeyName.empty() && !m_coordSysKeyName.Equals("AZMEA"))
        return DgnGCS::CreateGCS(WString(m_coordSysKeyName.c_str(), BentleyCharEncoding::Utf8).c_str(), db);

    auto gcs = DgnGCS::CreateGCS(db);
    if (BSISUCCESS != gcs->InitAzimuthalEqualArea(NULL, L"WGS84", L"METER", m_geoPoint.longitude, m_geoPoint.latitude, m_azimuthAngle, 1.0, m_originUors.x, m_originUors.y, 0))
        {
        BeAssert(false && "missing basegeocoord dat files?");
        return nullptr;
        }

    return gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
WString iModelBridge::GetArgValueW(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return argValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelBridge::GetArgValue(WCharCP arg)
    {
    return Utf8String(GetArgValueW(arg));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::Params::Validate()
    {
    if (m_briefcaseName.empty())
        {
        fprintf(stderr, "Missing output briefcase name\n");
        return BentleyStatus::ERROR;
        }

    if (m_inputFileName.empty())
        {
        fprintf(stderr, "Missing input file name\n");
        return BentleyStatus::ERROR;
        }

    if (m_assetsDir.empty())
        {
        fprintf(stderr, "Missing assets directory name\n");
        return BentleyStatus::ERROR;
        }

    if (!m_assetsDir.DoesPathExist() || !m_assetsDir.IsDirectory())
        {
        fwprintf(stderr, L"%ls: invalid assets directory name\n", m_assetsDir.c_str());
        return BentleyStatus::ERROR;
        }

    if (m_libraryDir.empty())
        {
        fprintf(stderr, "Missing library directory name\n");
        return BentleyStatus::ERROR;
        }

    if (!m_libraryDir.DoesPathExist() || !m_libraryDir.IsDirectory())
        {
        fwprintf(stderr, L"%ls: invalid bridge library directory name\n", m_libraryDir.c_str());
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson              07/17
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridge::L10N::Initialize(BeSQLite::L10N::SqlangFiles const & bridgeSqlangFiles)
    {
    if (NULL != s_bridgeL10NLookup)
        {
        // should only call initialize once
        BeAssert(false);
        return BSISUCCESS;
        }

    if (!bridgeSqlangFiles.m_default.DoesPathExist())
        {
        // invalid last resort sqlang database
        BeAssert(false);
        return BSIERROR;
        }

    s_bridgeL10NLookup = new L10NLookup(bridgeSqlangFiles);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson              07/17
//---------------------------------------------------------------------------------------
Utf8String iModelBridge::L10N::GetString(BeSQLite::L10N::NameSpace scope, BeSQLite::L10N::StringId name)
    {
    bool hasString = false;

    if (NULL != s_bridgeL10NLookup)
        {
        Utf8String appString = s_bridgeL10NLookup->GetString(scope, name, &hasString);
        if (!appString.empty() || hasString)
            return appString;
        }

    // no bridge string found, search platform strings
    return BeSQLite::L10N::GetString(scope, name, &hasString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson              08/17
//---------------------------------------------------------------------------------------
bool iModelBridge::Params::IsFileAssignedToBridge(BeFileNameCR fn) const
    {
    if (nullptr == m_assignmentChecker) // if there is no checker assigned, then assume that this is a standalone converter. It converts everything fed to it.
        return true;
    return m_assignmentChecker->_IsFileAssignedToBridge(fn, m_thisBridgeRegSubKey.c_str());
    }

#ifdef WIP_WIP_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
LinkModelPtr iModelBridge::GetRepositoryLinkModel(DgnDbR db, bool createIfNecessary)
    {
    Utf8String partitionName = "RepositoryLinksPartition"; //iModelBridge::L10N.GetString(iModelBridge::L10N::??::RepositoryLinksPartitionName());    TODO
    DgnCode partitionCode = LinkPartition::CreateCode(*db.Elements().GetRootSubject(), partitionName.c_str());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    if (partitionId.IsValid())
        return LinkModel::Get(db, DgnModelId(partitionId.GetValue()));

    if (!createIfNecessary)
        return nullptr;

    LinkPartitionPtr ed = LinkPartition::Create(*db.Elements().GetRootSubject(), partitionName.c_str());
    LinkPartitionCPtr partition = ed->InsertT<LinkPartition>();
    if (!partition.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }
    auto lm = LinkModel::Create(LinkModel::CreateParams(db, partition->GetElementId()));
    if (lm->Insert() != DgnDbStatus::Success)
        {
        BeAssert(false);
        return nullptr;
        }
    return lm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId iModelBridge::FindOrCreateRepositoryLink(DgnDbR db, BeFileNameCR localFileName, Utf8StringCR defaultCode, Utf8StringCR defaultURN, bool createIfNecessary)
    {
    auto lmodel = GetRepositoryLinkModel(db, createIfNecessary);
    if (!lmodel.IsValid())
        return DgnElementId();

    iModelBridgeDocumentProperties docProps;

    // Get the document's properties 
    
    // Prefer to get the properties assigned by ProjectWise, if possible.
    if (nullptr != _GetParams().GetAssignmentChecker())
        _GetParams().GetAssignmentChecker()->_GetDocumentProperties(docProps, BeFileName(file.GetFileName().c_str())); 

    if (docProps.m_docGUID.empty())
        {
        docProps.m_webURN = defaultURN;
        DgnCode code = RepositoryLink::CreateUniqueCode(*lmodel, defaultCode.c_str());   // Make sure the fake GUID is really unique
        docProps.m_docGUID = code.GetValueUtf8CP();
        }

    //  Make the RepositoryLink, using the GUID as its code, and the WebURN as its URI
    auto rlink = RepositoryLink::Create(*lmodel, docProps.m_webURN.c_str(), docProps.m_docGUID.c_str());

    auto rlinkPersist = rlink->Insert();
    if (!rlinkPersist.IsValid())
        {
        BeAssert(false);
        return DgnElementId();
        }

    return rlinkPersist->GetElementId();
    }
#endif
