/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridge.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <BeSQLite/L10N.h>
#include <Bentley/BeTextFile.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <Bentley/BeNumerical.h>
#include "iModelBridgeHelpers.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

static L10NLookup* s_bridgeL10NLookup = NULL;

// Helper class to ensure that bridge _CloseSource function is called
struct CallCloseSource
    {
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;
    bool m_closeOnErrorOnly;

    CallCloseSource(iModelBridge& bridge, bool closeOnErrorOnly) : m_bridge(bridge), m_closeOnErrorOnly(closeOnErrorOnly) {}

    ~CallCloseSource()
        {
        if (m_closeOnErrorOnly && (BSISUCCESS == m_status)) // if we should only close in case of error and there is no error
            return;                                         //  don't close
        m_bridge._CloseSource(m_status);
        }
    };

// Helper class to ensure that bridge _Converted function is called
struct CallOnBimClose
    {
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;
    bool m_closeOnErrorOnly;

    CallOnBimClose(iModelBridge& bridge, bool closeOnErrorOnly) : m_bridge(bridge), m_closeOnErrorOnly(closeOnErrorOnly) {}

    ~CallOnBimClose() 
        {
        if (m_closeOnErrorOnly && (BSISUCCESS == m_status)) // if we should only close in case of error and there is no error
            return;                                         //  don't close
        m_bridge._OnCloseBim(m_status);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::Params::Params()
    :m_dmsSupport(NULL)
    {
    m_spatialDataTransform.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridge::ComputeReportFileName(BeFileNameCR bcName)
    {
    BeFileName reportFileName(bcName);
    reportFileName.append(L"-issues");
    return reportFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge::Params::SetReportFileName()
    {
    m_reportFileName = ComputeReportFileName(GetBriefcaseName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge::ReportIssue(WStringCR msg)
    {
    BeFileStatus status;
    auto tf = BeTextFile::Open(status, _GetParams().GetReportFileName().c_str(), TextFileOpenType::Append, TextFileOptions::None);
    if (!tf.IsValid())
        {
        BeAssert(false);
        return;
        }
    tf->PutLine(msg.c_str(), true);
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

    createProjectParams.SetRootSubjectName(_GetParams().GetBridgeRegSubKeyUtf8().c_str());

    // Create the DgnDb file. All currently registered domain schemas are imported.
    BeSQLite::DbResult createStatus;
    auto db = DgnDb::CreateDgnDb(&createStatus, _GetParams().GetBriefcaseName(), createProjectParams);
    if (!db.IsValid())
        {
        LOG.fatalv(L"Failed to create repository [%s] with error %x", _GetParams().GetBriefcaseName().c_str(), createStatus);
        return nullptr;
        }

    if (nullptr != rootSubjectDescription)
        db->SavePropertyString(DgnProjectProperty::Description(), rootSubjectDescription);

    bvector<DgnModelId> baseModels;
    queryAllModels(baseModels, *db);

    _GetParams().SetIsCreatingNewDgnDb(true);
    _GetParams().SetIsUpdating(false);

    iModelBridgeCallOpenCloseFunctions callCloseOnReturn(*this, *db);
    if (!callCloseOnReturn.IsReady())
        {
        LOG.fatalv("Bridge is not ready or could not open source file");
        return nullptr;
        }

    db->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                       // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.

    // Tell the bridge to generate schemas
    _MakeSchemaChanges();

    auto jobsubj = _InitializeJob();
    if (!jobsubj.IsValid())
        {
        LOG.fatalv("Failed to create job structure");
        return nullptr;
        }

    _GetParams().SetJobSubjectId(jobsubj->GetElementId());

    if (BSISUCCESS != _ConvertToBim(*jobsubj))
        {
        LOG.fatalv("Failed to populate new repository");
        return nullptr;
        }

    callCloseOnReturn.m_status = BSISUCCESS;

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
DgnDbPtr iModelBridge::OpenBimAndMergeSchemaChanges(BeSQLite::DbResult& dbres, bool& madeSchemaChanges, BeFileNameCR dbName)
    {
    // Try to open the BIM without permitting schema changes. That's the common case, and that's the only way we have
    // of detecting the case where we do have domain schema changes (by looking for an error result).

    // (Note that OpenDgnDb will also merge in any pending schema changes that were recently pulled from iModelHub.)

    madeSchemaChanges = false;
    auto db = DgnDb::OpenDgnDb(&dbres, dbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    if (!db.IsValid())
        {
        if (BeSQLite::BE_SQLITE_ERROR_SchemaUpgradeRequired != dbres)
            return nullptr;

        // We must do a schema upgrade.
        // Probably, the bridge registered some required domains, and they must be imported
        DgnDb::OpenParams oparams(DgnDb::OpenMode::ReadWrite);
        oparams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
        db = DgnDb::OpenDgnDb(&dbres, dbName, oparams);
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
        }

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::DoConvertToExistingBim(DgnDbR db, bool detectDeletedFiles)
    {
    bool haveInputFile = !_GetParams().GetInputFileName().empty();

    _GetParams().SetIsCreatingNewDgnDb(false);
    _GetParams().SetIsUpdating(true);

    db.BriefcaseManager().StartBulkOperation();

    if (haveInputFile)
        {
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

        _GetParams().SetJobSubjectId(jobsubj->GetElementId());

        if (BSISUCCESS != _ConvertToBim(*jobsubj))
            {
            LOG.fatalv("_ConvertToBim failed");
            return BSIERROR; // caller must call abandon changes
            }
        }

    if (detectDeletedFiles)
        _DetectDeletedDocuments();

    // Must either succeed in getting all required locks and codes ... or abort the whole txn.
    auto response = db.BriefcaseManager().EndBulkOperation();
    if (RepositoryStatus::Success != response.Result())
        {
        LOG.fatalv("Failed to acquire locks and/or codes with error %x", response.Result());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

// *******************
/// Command-line parsing utilities


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::Params::GCSCalculationMethodFromString(GCSCalculationMethod& cm, Utf8StringCR value)
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
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelBridge::Params::GCSCalculationMethodToString(GCSCalculationMethod const& cm)
    {
    switch(cm)
        {
        case GCSCalculationMethod::UseDefault: return "default";
        case GCSCalculationMethod::UseReprojection: return "reproject";
        case GCSCalculationMethod::UseGcsTransform: return "transform";
        case GCSCalculationMethod::UseGcsTransformWithScaling: return "transformscaled";
        }
    BeAssert(false && "unrecognized GCSCalculationMethod -- keep this function consistent with the enum definition!");
    return "";
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      11/17
//---------------------------------------------------------------------------------------
void iModelBridge::Params::SetGcsJson(JsonValueR json, GCSDefinition const& gcsDef, GCSCalculationMethod const& gcsCalculationMethod)
    {
    if (!gcsDef.m_isValid)
        {
        BeAssert(false);
        }

    if (gcsCalculationMethod != GCSCalculationMethod::UseDefault)
        json[json_gcs()]["gcsCalculationMethod"] = GCSCalculationMethodToString(gcsCalculationMethod);

    if (!gcsDef.m_coordSysKeyName.empty())
        {
        auto& member = json[json_gcs()]["coordinateSystemKeyName"];
        member["key"] = gcsDef.m_coordSysKeyName;
        return;
        }

    auto& member = json[json_gcs()]["azmea"];
    JsonUtils::DPoint3dToJson(member["sourceOrigin"], gcsDef.m_originUors);
    member["azimuthAngle"] = gcsDef.m_azimuthAngle;   // The angle, clockwise from true north in decimal degrees, of the rotation to be applied.
    auto& geoPoint = member["geoPoint"];
    geoPoint["latitude"] = gcsDef.m_geoPoint.latitude;
    geoPoint["longitude"] = gcsDef.m_geoPoint.longitude;
    geoPoint["elevation"] = gcsDef.m_geoPoint.elevation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      11/17
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridge::Params::ParseGcsJson(GCSDefinition& gcsDef, GCSCalculationMethod& gcsCalculationMethod, JsonValueCR json)
    {
    if (!json.isMember("gcsCalculationMethod"))
        gcsCalculationMethod = GCSCalculationMethod::UseDefault;
    else
        {
        auto const& member = json["gcsCalculationMethod"];
        if (BSISUCCESS != GCSCalculationMethodFromString(gcsCalculationMethod, member.asCString()))
            return BSIERROR;
        }

    if (json.isMember("coordinateSystemKeyName"))
        {
        gcsDef.m_coordSysKeyName = json["coordinateSystemKeyName"].asCString();
        return BSISUCCESS;
        }
    
    if (json.isMember("azmea"))
        {
        auto const& member = json["azmea"];
        gcsDef.m_azimuthAngle = member["azimuthAngle"].asDouble();
        auto const& geoPoint = member["geoPoint"];
        gcsDef.m_geoPoint.latitude = geoPoint["latitude"].asDouble();
        gcsDef.m_geoPoint.longitude = geoPoint["longitude"].asDouble();
        gcsDef.m_geoPoint.elevation = geoPoint["elevation"].asDouble();
        return BSISUCCESS;
        }
    
    BeAssert(false);
    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      11/17
//---------------------------------------------------------------------------------------
void iModelBridge::Params::SetTransformJson(JsonValueR json, TransformCR transform)
    {
    auto& member = json[json_transform()]["transform"];
    JsonUtils::TransformToJson(member, transform);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      11/17
//---------------------------------------------------------------------------------------
void iModelBridge::Params::SetOffsetJson(JsonValueR json, DPoint3dCR offset, AngleInDegrees azimuthAngle)
    {
    auto& member = json[json_transform()]["offsetAndAngle"];
    JsonUtils::DPoint3dToJson(member["offset"], offset);
    member["angle"] = azimuthAngle.Degrees();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::Params::ParseTransformJson(Transform& trans, JsonValueCR json)
    {
    if (json.isMember("transform"))
        {
        JsonUtils::TransformFromJson(trans, json["transform"]);
        return BSISUCCESS;
        }

    if (json.isMember("offsetAndAngle"))
        {
        auto& member = json["offsetAndAngle"];
        DPoint3d sourceOrigin;
        JsonUtils::DPoint3dFromJson(sourceOrigin, member["sourceOrigin"]);
        DPoint3d offset;
        JsonUtils::DPoint3dFromJson(offset, member["offset"]);
        double rdeg = 0;
        if (member.isMember("angle"))
            rdeg = member["angle"].asDouble();
        double rrad = Angle::DegreesToRadians(rdeg);
        auto zAxis = DRay3d::FromOriginAndVector(DPoint3d::FromZero(), DVec3d::From(0, 0, 1));
        trans = Transform::FromAxisAndRotationAngle(zAxis, rrad);
        trans.SetTranslation(offset);
        return BSISUCCESS;
        }

    BeAssert(false && "malformed iModelBridge transform JSON data");

    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      10/17
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridge::Params::ParseJsonArgs(JsonValueCR obj, bool isForInputGcs)
    {
    for (auto const& propName : obj.getMemberNames())
        {
        if (propName.EqualsI(json_transform()))
            {
            if (obj.isMember(json_gcs()))
                {
                BeAssert(false);
                fprintf(stderr, "Specify transform or GCS, but not both\n");
                return BSIERROR;
                }

            if (BSISUCCESS != ParseTransformJson(m_spatialDataTransform, obj[json_transform()]))
                return BSIERROR;
            }
        else if (propName.EqualsI(json_gcs()))
            {
            if (obj.isMember(json_transform()))
                {
                BeAssert(false);
                fprintf(stderr, "Specify transform or GCS, but not both\n");
                return BSIERROR;
                }

            BentleyStatus status;
            if (isForInputGcs)
                status = ParseGcsJson(m_inputGcs, m_gcsCalculationMethod, obj[json_gcs()]);
            else
                {
                GCSCalculationMethod ignore;
                status = ParseGcsJson(m_outputGcs, ignore, obj[json_gcs()]);
                }

            if (BSISUCCESS != status)
                return status;
            }
        else
            {
            BeAssert(false);
            fprintf(stderr, "%s - unrecognized JSON value\n", propName.c_str());
            return BSIERROR;
            }
        }

    return BSISUCCESS;
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
void iModelBridge::L10N::Terminate()
    {
    if (s_bridgeL10NLookup)
        delete s_bridgeL10NLookup;
    s_bridgeL10NLookup = nullptr;
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
    if (nullptr == m_documentPropertiesAccessor) // if there is no checker assigned, then assume that this is a standalone converter. It converts everything fed to it.
        return true;
    return m_documentPropertiesAccessor->_IsFileAssignedToBridge(fn, m_thisBridgeRegSubKey.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson              08/17
//---------------------------------------------------------------------------------------
bool iModelBridge::Params::IsDocumentAssignedToJob(Utf8StringCR docId) const
    {
    if (nullptr == m_documentPropertiesAccessor) // if there is no checker assigned, then assume that this is a standalone converter. It converts everything fed to it.
        {
        BeAssert(false);
        return true;
        }

    iModelBridgeDocumentProperties docpropsdontcare;

    BeGuid docGuid;
    docGuid.FromString(docId.c_str());
    if (docGuid.IsValid())
        {
        // always prefer to check documents by GUID, if available.
        BeFileName localfilepathdontcare;
        return BSISUCCESS == m_documentPropertiesAccessor->_GetDocumentPropertiesByGuid(docpropsdontcare, localfilepathdontcare, docGuid);
        }

    // Assume docId is a localFilePath
    return BSISUCCESS == m_documentPropertiesAccessor->_GetDocumentProperties(docpropsdontcare, BeFileName(docId.c_str(), true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::BeGuid iModelBridge::Params::QueryDocumentGuid(BeFileNameCR localFileName) const
    {
    if (nullptr == m_documentPropertiesAccessor)
        return BeGuid();

    iModelBridgeDocumentProperties docProps;
    m_documentPropertiesAccessor->_GetDocumentProperties(docProps, localFileName); 
    BeGuid docGuid;
    docGuid.FromString(docProps.m_docGuid.c_str());
    return docGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
SHA1 iModelBridge::ComputeRepositoryLinkHash(RepositoryLinkCR el)
    {
    SHA1 sha1;
    sha1(el.GetDocumentProperties().ToString());
    BeGuid guid = el.GetRepositoryGuid();
    sha1(&guid, sizeof(guid));
    sha1(el.GetUrl());
    return sha1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge::GetRepositoryLinkInfo(DgnCode& code, iModelBridgeDocumentProperties& docProps, DgnDbR db, Params const& params, 
                                                BeFileNameCR localFileName, Utf8StringCR defaultCode, Utf8StringCR defaultURN, InformationModelR lmodel)
    {
    Utf8String codeStr(defaultCode);
    docProps.m_desktopURN = defaultURN;

    // Prefer to get the properties assigned by ProjectWise, if possible.
    if (nullptr != params.GetDocumentPropertiesAccessor())
        params.GetDocumentPropertiesAccessor()->_GetDocumentProperties(docProps, localFileName); 

    if (!docProps.m_docGuid.empty())
        codeStr = docProps.m_docGuid; // Use the GUID as the code, if we have it.

    if (codeStr.empty())
        codeStr = Utf8String(localFileName);

    if (docProps.m_desktopURN.empty())
        docProps.m_desktopURN = Utf8String(localFileName);

    code = RepositoryLink::CreateCode(lmodel, codeStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkPtr iModelBridge::MakeRepositoryLink(DgnDbR db, Params const& params, BeFileNameCR localFileName, Utf8StringCR defaultCode, Utf8StringCR defaultURN)
    {
    auto lmodel = db.GetRepositoryModel();
    if (!lmodel.IsValid())
        return nullptr;

    DgnCode code;
    iModelBridgeDocumentProperties docProps;
    GetRepositoryLinkInfo(code, docProps, db, params, localFileName, defaultCode, defaultURN, *lmodel);

    RepositoryLinkCPtr rlinkPersist = db.Elements().Get<RepositoryLink>(db.Elements().QueryElementIdByCode(code));
    
    RepositoryLinkPtr rlink;
    if (rlinkPersist.IsValid())
        rlink = rlinkPersist->MakeCopy<RepositoryLink>();
    else
        rlink = RepositoryLink::Create(*lmodel, "", code.GetValue().GetUtf8CP());

    rlink->SetUrl(docProps.m_desktopURN.c_str());
    rlink->SetDescription("");
    WString relFileName;
    BeFileName::FindRelativePath(relFileName, localFileName.c_str(), params.GetInputFileName().GetDirectoryName().c_str());
    rlink->SetUserLabel(Utf8String(relFileName).c_str());

    if (!docProps.m_docGuid.empty())
        {
        BeGuid beguid;
        if (SUCCESS == beguid.FromString(docProps.m_docGuid.c_str()))
            rlink->SetRepositoryGuid(beguid);
        }

    if (!docProps.m_desktopURN.empty() || !docProps.m_attributesJSON.empty())
        {
        Json::Value jsonValue = Json::objectValue;
        jsonValue["desktopURN"] = docProps.m_desktopURN;
        jsonValue["webURN"] = docProps.m_webURN;
        jsonValue["attributes"] = Json::Value::From(docProps.m_attributesJSON);
        rlink->SetDocumentProperties(jsonValue);
        }

    return rlink;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
DgnDbStatus iModelBridge::InsertLinkTableRelationship(DgnDbR db, Utf8CP relClassName, DgnElementId source, DgnElementId target, Utf8CP schemaName)
    {
    auto relClass = db.Schemas().GetClass(schemaName, relClassName);
    if (nullptr == relClass || nullptr == relClass->GetRelationshipClassCP())
        {
        BeAssert(false);
        return DgnDbStatus::NotFound;
        }
    EC::ECInstanceKey relKey;
    auto status = db.InsertLinkTableRelationship(relKey, *relClass->GetRelationshipClassCP(), source, target);
    BeAssert(BE_SQLITE_OK == status);
    return (BE_SQLITE_OK == status)? DgnDbStatus::Success: DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Transform iModelBridge::GetSpatialDataTransform(Params const& params, SubjectCR jobSubject)
    {
    Transform jobTrans = params.GetSpatialDataTransform();
    
    // Report the jobTrans in a property of the JobSubject. 
    // Note that we NOT getting the transform from the JobSubject. We are SETTING
    // the property on the JobSubject, so that the user and apps can see what the 
    // bridge configuration transform is.
    Transform jobSubjectTransform;
    auto matrixTolerance = Angle::TinyAngle();
    auto pointTolerance = 10 * BentleyApi::BeNumerical::NextafterDelta(jobTrans.ColumnXMagnitude());
    if ((BSISUCCESS != JobSubjectUtils::GetTransform(jobSubjectTransform, jobSubject)) || !jobSubjectTransform.IsEqual(jobTrans, matrixTolerance, pointTolerance))
        {
        auto jobSubjectED = jobSubject.MakeCopy<Subject>();
        JobSubjectUtils::SetTransform(*jobSubjectED, jobTrans);
        jobSubjectED->Update();
        }

    return jobTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      iModelBridge::ComputeJobSubjectName(DgnDbCR db, Params const& params, Utf8StringCR bridgeSpecificSuffix)
    {
    // Use the document GUID, if available, to ensure a unique Job subject name.
    Utf8String docIdStr;
    auto docGuid = params.QueryDocumentGuid(params.GetInputFileName());
    if (docGuid.IsValid())
        docIdStr = docGuid.ToString();
    else
        docIdStr = Utf8String(params.GetInputFileName());

    Utf8String jobName(params.GetBridgeRegSubKey());
    jobName.append(":");
    jobName.append(docIdStr.c_str());
    if (!bridgeSpecificSuffix.empty())
        {
        jobName.append(", ");
        jobName.append(bridgeSpecificSuffix);
        }

    DgnCode code = Subject::CreateCode(*db.Elements().GetRootSubject(), jobName.c_str());
    int i = 0;
    while (db.Elements().QueryElementIdByCode(code).IsValid())
        {
        Utf8String uniqueJobName(jobName);
        uniqueJobName.append(Utf8PrintfString("%d", ++i).c_str());
        code = Subject::CreateCode(*db.Elements().GetRootSubject(), uniqueJobName.c_str());
        }
    jobName = code.GetValueUtf8();
    return jobName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Http::IHttpHeaderProviderPtr iModelBridge::Params::GetDefaultHeaderProvider() const
    {
    if (m_jobRunCorrelationId.empty())
        return nullptr;

    Http::HttpRequestHeaders headers;
    headers.SetValue("X-Correlation-ID", m_jobRunCorrelationId.c_str());
    return Http::HttpHeaderProvider::Create(headers);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ClientInfoPtr iModelBridge::Params::GetClientInfo() const
    {
    if (nullptr != m_clientInfo)
        return m_clientInfo;
    //Else provide a dummy default
    static Utf8CP s_productId = "1654"; // Navigator Desktop
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    WebServices::ClientInfoPtr clientInfo = WebServices::ClientInfoPtr(
        new WebServices::ClientInfo("Bentley-Test", BeVersion(1, 0), "{41FE7A91-A984-432D-ABCF-9B860A8D5360}", "TestDeviceId", "TestSystem", s_productId, GetDefaultHeaderProvider()));
    return clientInfo;
    }

struct MemoryUsageAppData : DgnDb::AppData
    {
    int m_rowsChanged{};
    static Key const& GetKey() { static Key s_key; return s_key; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::SaveChangesToConserveMemory(DgnDbR db, Utf8CP commitComment, int maxRowsChangedPerTxn)
    {
    MemoryUsageAppData& lastCheck = *(MemoryUsageAppData*)db.FindOrAddAppData(MemoryUsageAppData::GetKey(), []() { return new MemoryUsageAppData(); });

    int rowsChanged = db.GetTotalModifiedRowCount();
    if ((rowsChanged - lastCheck.m_rowsChanged) <= maxRowsChangedPerTxn)
        return BSISUCCESS;

    lastCheck.m_rowsChanged = rowsChanged;

    auto status = db.SaveChanges(commitComment);
    if (BE_SQLITE_OK != status)
        return BSIERROR;

    db.BriefcaseManager().StartBulkOperation();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::SaveChanges(DgnDbR db, Utf8CP commitComment)
    {
    MemoryUsageAppData& lastCheck = *(MemoryUsageAppData*)db.FindOrAddAppData(MemoryUsageAppData::GetKey(), []() { return new MemoryUsageAppData(); });

    lastCheck.m_rowsChanged = db.GetTotalModifiedRowCount();

    auto status = db.SaveChanges(commitComment);
    if (BE_SQLITE_OK != status)
        return BSIERROR;

    db.BriefcaseManager().StartBulkOperation();
    return BSISUCCESS;
    }