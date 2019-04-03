/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeSyncInfoFile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <Logging/bentleylogging.h>
#include <GeomJsonWireFormat/JsonUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING


#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define MUSTBEDBRESULTRC(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return rc;}}
#define MUSTBEOKRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_OK)
#define MUSTBEROWRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_ROW)
#define MUSTBEDONERC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_DONE)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::AttachToBIM(DgnDbR db, bool createIfNecessary)
    {
    m_bim = &db;

    m_isAttached = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::DetachFromBIM()
    {
    if (!m_bim.IsValid())
        return;

    m_bim = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::~iModelBridgeSyncInfoFile()
    {
    DetachFromBIM();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::SetLastError(BeSQLite::DbResult rc)
    {
    m_lastError = rc;
    m_lastErrorDescription = GetDgnDb().GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::GetLastError(BeSQLite::DbResult& result, Utf8String& descr)
    {
    result = m_lastError;
    descr = m_lastErrorDescription;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeWithSyncInfoBase::_OnOpenBim(DgnDbR db)
    {
    if (BentleyStatus::SUCCESS != T_Super::_OnOpenBim(db))
        return BentleyStatus::ERROR;

    // Note that I must attach my syncinfo in _OnConvertToBim -- outside of the bulk update txn -- I must not wait until _ConvertToBim.
    return m_syncInfo.AttachToBIM(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeWithSyncInfoBase::_OnCloseBim(BentleyStatus status, iModelBridge::ClosePurpose purpose)
    {
    m_syncInfo.DetachFromBIM();
    T_Super::_OnCloseBim(status, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::ConversionResults iModelBridgeWithSyncInfoBase::RecordDocument(iModelBridgeSyncInfoFile::ChangeDetector& changeDetector,
                                                                     BeFileNameCR fileNameIn, iModelBridgeSyncInfoFile::SourceState const* sstateIn,
                                                                     Utf8CP kind, iModelBridgeSyncInfoFile::ROWID srid, Utf8StringCR knownUrn)
    {
    // Get the identity of the document
    BeFileName fileName(fileNameIn);
    if (fileName.empty())
        fileName = _GetParams().GetInputFileName();

    Utf8String urn(knownUrn);
    if (urn.empty())
        urn = GetParamsCR().QueryDocumentURN(fileName);

    // Make a RepositoryLink to represent the document
    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = MakeRepositoryLink(GetDgnDbR(), _GetParams(), fileName, "", urn);
    if (!results.m_element.IsValid())
        {
        BeAssert(false);
        return results;
        }

    //  Compute the state of the document
    time_t lmt = 0;
    if (sstateIn)
        lmt = static_cast<time_t>(sstateIn->GetLastModifiedTime());
    else
        {
        if (BeFileNameStatus::Success == BeFileName::GetFileTime(nullptr, nullptr, &lmt, fileName)) // (may not really be a disk file)
            iModelBridgeSyncInfoFile::SourceState sstate((double)lmt, "");
        }

    auto sha1 = ComputeRepositoryLinkHash(*(RepositoryLink*)results.m_element.get());   // The source state is based on the properties of the RepositoryLink
    if (sstateIn)
        sha1(sstateIn->GetHash());   // if the caller passed in a hash of the file's contents, then include that in the source state hash
    else
        sha1(&lmt, sizeof(lmt)); // otherwise, make the hash depend on the last modified time, as a proxy for the file's contents.

    iModelBridgeSyncInfoFile::SourceState sstate(static_cast<double>(lmt), sha1.GetHashString());

    //  Write the item to syncinfo, and write the RepositoryLink Element to the BIM
    DocSourceItem docItem(results.m_element->GetCode(), sstate);

    auto change = changeDetector._DetectChange(srid, kind, docItem);
    changeDetector._UpdateBimAndSyncInfo(results, change);
 
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged != change.GetChangeType())
        {
        LOG.infov(L"[%ls] - document recorded in syncinfo with id=[%ls], rowid=%ld", fileName.c_str(), WString(docItem._GetId().c_str(), true).c_str(), results.m_syncInfoRecord.GetROWID());
        }

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeWithSyncInfoBase::DetectDeletedDocuments(Utf8CP kind, iModelBridgeSyncInfoFile::ROWID srid)
    {
    if (!_GetParams().IsUpdating())
        return BSISUCCESS;

    ExternalSourceAspectIterator<iModelExternalSourceAspect> files(GetDgnDbR(), GetDgnDbR().Elements().GetRootSubjectId(), kind, "");
    for (auto file = files.begin(); file != files.end(); ++file)
        {
        Utf8String docId = file->GetIdentifier();
        if (IsDocumentInRegistry(docId)) // If the document is in the document registry at all, that means that a) the document exists, and b) the job scheduler assigned the document to this bridge.
            continue;                    // So, if the doc exists and is still assigned to me, then don't delete it.

        _OnDocumentDeleted(docId, file->GetScope().GetValue());
        }

    return BSISUCCESS;
    }

//=======================================================================================
// The "hash" of this item is the JSON representation of a 3x4 Transform
// @bsiclass                                    BentleySystems
//=======================================================================================
struct JobTransformSourceItem : iModelBridgeSyncInfoFile::ISourceItem
    {
    Transform m_trans;
    Utf8String m_id;

    JobTransformSourceItem(Utf8StringCR id, TransformCR t) : m_id(id), m_trans(t) {;}

    Utf8String _GetId() override {return m_id;}
    double _GetLastModifiedTime() override {return 0.0;}
    Utf8String _GetHash() override {Json::Value json; JsonUtils::TransformToJson(json, m_trans); return json.ToString();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeWithSyncInfoBase::DetectSpatialDataTransformChange(TransformR newTrans, TransformR oldTrans,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ROWID srid, Utf8CP kind, Utf8StringCR id)
    {
    newTrans = GetParamsCR().GetSpatialDataTransform();

    if (BSISUCCESS != JobSubjectUtils::GetTransform(oldTrans, *GetJobSubject()))
        oldTrans.InitIdentity();

    return !AreTransformsEqual(newTrans, oldTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelExternalSourceAspect::AddAspect(DgnElementR el)
    {
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    return DgnElement::GenericMultiAspect::AddAspect(el, *m_instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr iModelExternalSourceAspect::CreateInstance(DgnElementId scope, Utf8CP kind, Utf8StringCR sourceId, SourceState const* ss, ECN::ECClassCR aspectClass) 
    {
    auto instance = aspectClass.GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue(XTRN_SRC_ASPCT_Scope, ECN::ECValue(scope));
    instance->SetValue(XTRN_SRC_ASPCT_Identifier, ECN::ECValue(sourceId.c_str()));
    instance->SetValue(XTRN_SRC_ASPCT_Kind, ECN::ECValue(kind));
    if (ss)
        SetSourceState(*instance, *ss);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP iModelExternalSourceAspect::GetAspectClass(DgnDbR db)
    {
    return db.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_ExternalSourceAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect::ElementAndAspectId iModelExternalSourceAspect::FindElementBySourceId(DgnDbR db, DgnElementId scopeId, Utf8CP kind, Utf8StringCR sourceId)
    {
    auto sel = db.GetPreparedECSqlStatement("SELECT Element.Id, ECInstanceId from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Scope.Id=? AND Kind=? AND Identifier=?)");
    sel->BindId(1, scopeId);
    sel->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sel->BindText(3, sourceId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    ElementAndAspectId res;
    if (BE_SQLITE_ROW != sel->Step())
        return res;
    res.elementId = sel->GetValueId<DgnElementId>(0);
    res.aspectId = sel->GetValueId<BeSQLite::EC::ECInstanceId>(1);
    return res;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect::ElementAndAspectId iModelExternalSourceAspect::FindElementByHash(DgnDbR db, DgnElementId scopeId, Utf8CP kind, Utf8StringCR hash)
    {
    auto sel = db.GetPreparedECSqlStatement("SELECT Element.Id, ECInstanceId from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Scope.Id=? AND Kind=? AND Checksum=?)");
    sel->BindId(1, scopeId);
    sel->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sel->BindText(3, hash.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    ElementAndAspectId res;
    if (BE_SQLITE_ROW != sel->Step())
        return res;
    res.elementId = sel->GetValueId<DgnElementId>(0);
    res.aspectId = sel->GetValueId<BeSQLite::EC::ECInstanceId>(1);
    return res;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<iModelExternalSourceAspect> iModelExternalSourceAspect::GetAll(DgnElementCR el, ECN::ECClassCP aspectClass)
    {
    bvector<iModelExternalSourceAspect> aspects;
    if (!aspectClass)
        aspectClass = GetAspectClass(el.GetDgnDb());
    if (nullptr == aspectClass)
        return aspects;
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Element.Id=?)");
    sel->BindId(1, el.GetElementId());
    while (BE_SQLITE_ROW == sel->Step())
        {
        auto aspectid = sel->GetValueId<BeSQLite::EC::ECInstanceId>(0);
        auto aspect = GetAspect(el, aspectid);
        aspects.push_back(aspect);
        }
    return aspects;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<iModelExternalSourceAspect> iModelExternalSourceAspect::GetAllByKind(DgnElementCR el, Utf8CP kind, ECN::ECClassCP aspectClass)
    {
    bvector<iModelExternalSourceAspect> aspects;
    if (!aspectClass)
        aspectClass = GetAspectClass(el.GetDgnDb());
    if (nullptr == aspectClass)
        return aspects;
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Element.Id=? AND Kind=?)");
    sel->BindId(1, el.GetElementId());
    sel->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    while (BE_SQLITE_ROW == sel->Step())
        {
        auto aspectid = sel->GetValueId<BeSQLite::EC::ECInstanceId>(0);
        auto aspect = GetAspect(el, aspectid);
        aspects.push_back(aspect);
        }
    return aspects;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelExternalSourceAspect::GetDumpHeaders(bool includeProperties, bool includeSourceState)
    {
    Utf8PrintfString str("%-20.20s %8.8s %-32.32s %-32.32s", "Kind", "Scope", " ", "Identifier");
    if (includeSourceState)
        {
        // TBD
        }
    if (includeProperties)
        {
        str.append("\tProperties");
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelExternalSourceAspect::FormatForDump(DgnDbR db, bool includeProperties, bool includeSourceState) const
    {
    auto scopeEl = db.Elements().GetElement(GetScope());
    auto scopeClass = scopeEl.IsValid()? scopeEl->GetElementClass()->GetFullName(): "?";
    Utf8PrintfString str("%-20s %8.0llx %-32s %-32s", GetKind().c_str(), GetScope().GetValueUnchecked(), scopeClass, GetIdentifier().c_str());
    if (includeSourceState)
        {
        // TBD
        }
    if (includeProperties)
        {
        ECN::ECValue props;
        BeAssert(m_instance.IsValid());
        if (m_instance.IsNull())
            LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
        if (ECN::ECObjectsStatus::Success == m_instance->GetValue(props, XTRN_SRC_ASPCT_JsonProperties) && !props.IsNull() && props.IsString())
            str.append("\t").append(props.GetUtf8CP());
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void outDump(ILogger* logger, NativeLogging::SEVERITY sev, Utf8StringCR msg)
    {
    if (nullptr == logger)
        {
        printf("%s\n", msg.c_str());
        return;
        }
    logger->message(sev, msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelExternalSourceAspect::Dump(DgnElementCR el, Utf8CP loggingCategory, NativeLogging::SEVERITY sev, bool includeProperties, bool includeSourceState)
    {
    auto aspects = GetAll(el, nullptr);
//    if (aspects.empty())
//        return;

    ILogger* logger = loggingCategory? LoggingManager::GetLogger(loggingCategory): nullptr;

    GeometrySource const* geom = el.ToGeometrySource();
    Utf8String catid;
    if (nullptr != geom)
        catid.Sprintf("Category: 0x%llx", geom->GetCategoryId().GetValue());

    Utf8String parent;
    if (el.GetParentId().IsValid())
        parent.Sprintf("Parent: 0x%llx", el.GetDgnDb().Elements().GetElement(el.GetParentId())->GetElementId().GetValue());

    outDump(logger, sev, Utf8PrintfString("Element ID: 0x%llx Class: %s Code: %s %s %s", el.GetElementId().GetValue(), el.GetElementClass()->GetFullName(), el.GetCode().GetValueUtf8CP(), catid.c_str(), parent.c_str()));

    if (aspects.empty())
        return;

    outDump(logger, sev, GetDumpHeaders(includeProperties, includeSourceState));
    for (auto const& aspect : aspects)
        {
        auto str = aspect.FormatForDump(el.GetDgnDb(), includeProperties, includeSourceState);
        outDump(logger, sev, str);
        }
    outDump(logger, sev, "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect iModelExternalSourceAspect::GetAspect(DgnElementCR el, BeSQLite::EC::ECInstanceId aspectid, ECN::ECClassCP aspectClass)
    {
    if (!aspectClass)
        aspectClass = GetAspectClass(el.GetDgnDb());
    if (nullptr == aspectClass)
        return iModelExternalSourceAspect();
    auto instance = DgnElement::GenericMultiAspect::GetAspect (el, *aspectClass, aspectid); // Get read-only copy of the aspect.
    if (nullptr == instance)
        return iModelExternalSourceAspect();
    return iModelExternalSourceAspect(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect iModelExternalSourceAspect::GetAspectForEdit(DgnElementR el, BeSQLite::EC::ECInstanceId aspectid, ECN::ECClassCP aspectClass)
    {
    if (!aspectClass)
        aspectClass = GetAspectClass(el.GetDgnDb());
    if (nullptr == aspectClass)
        return iModelExternalSourceAspect();
    auto instance = DgnElement::GenericMultiAspect::GetAspectP(el, *aspectClass, aspectid);    // NB: Call GetAspectP, not GetAspect! GetAspectP sets the aspect's dirty flag, which tells its _OnUpdate method to write out changes.
    if (nullptr == instance)
        return iModelExternalSourceAspect();
    return iModelExternalSourceAspect(instance);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect iModelExternalSourceAspect::GetAspectBySourceId(DgnDbR db, DgnElementId scopeId, Utf8CP kind, Utf8StringCR sourceId)
    {
    auto found = FindElementBySourceId(db, scopeId, kind, sourceId);
    if (!found.elementId.IsValid())
        return iModelExternalSourceAspect();

    auto el = db.Elements().GetElement(found.elementId);
    if (!el.IsValid())
        {
        BeAssert(false);
        return iModelExternalSourceAspect();
        }
    return GetAspect(*el, found.aspectId, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId iModelExternalSourceAspect::GetScope() const
    {
    ECN::ECValue v;
    BeAssert(m_instance.IsValid());
    m_instance->GetValue(v, XTRN_SRC_ASPCT_Scope);
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    return v.GetNavigationInfo().GetId<DgnElementId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId iModelExternalSourceAspect::GetElementId() const
    {
    ECN::ECValue v;
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    m_instance->GetValue(v, "Element");
    return v.GetNavigationInfo().GetId<DgnElementId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelExternalSourceAspect::GetIdentifier() const
    {
    ECN::ECValue v;
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    m_instance->GetValue(v, XTRN_SRC_ASPCT_Identifier);
    return v.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelExternalSourceAspect::GetKind() const 
    {
    ECN::ECValue v;
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    m_instance->GetValue(v, XTRN_SRC_ASPCT_Kind);
    return v.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect::SourceState iModelExternalSourceAspect::GetSourceState() const
    {
    SourceState ss;

    ECN::ECValue v;
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    if ((ECN::ECObjectsStatus::Success == m_instance->GetValue(v, XTRN_SRC_ASPCT_Checksum)) && !v.IsNull())
        {
        ss.m_checksum = v.GetUtf8CP();
        }

    if ((ECN::ECObjectsStatus::Success == m_instance->GetValue(v, XTRN_SRC_ASPCT_Version)) && !v.IsNull())
        {
        ss.m_version = v.GetUtf8CP();
        }

    return ss;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelExternalSourceAspect::SetProperties(rapidjson::Document const& json)
    {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);

    ECN::ECValue props(buffer.GetString());
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    m_instance->SetValue(XTRN_SRC_ASPCT_JsonProperties, props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document iModelExternalSourceAspect::GetProperties() const
    {
    rapidjson::Document json(rapidjson::kObjectType);
    ECN::ECValue props;
    BeAssert(m_instance.IsValid());
    if (m_instance.IsNull())
        LOG.fatal(L"iModelExternalSourceAspect::m_instance==nullptr");
    if (ECN::ECObjectsStatus::Success != m_instance->GetValue(props, XTRN_SRC_ASPCT_JsonProperties) || !props.IsString() || props.IsNull())
        return json;
    json.Parse(props.GetUtf8CP());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelExternalSourceAspect::SetSourceState(ECN::IECInstanceR instance, SourceState const& ss)
    {
    instance.SetValue(XTRN_SRC_ASPCT_Checksum, ECN::ECValue(ss.m_checksum.c_str()));
    instance.SetValue(XTRN_SRC_ASPCT_Version, ECN::ECValue(ss.m_version.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelExternalSourceAspect::SourceState iModelBridgeSyncInfoFile::SourceState::GetAspectState() const
    {
    iModelExternalSourceAspect::SourceState state;
    iModelExternalSourceAspect::DoubleToString(state.m_version, m_lmt);
    state.m_checksum = m_hash;
    return state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::SourceState::SourceState(iModelExternalSourceAspect::SourceState aspectState)
    {
    m_hash = aspectState.m_checksum;
    m_lmt = iModelExternalSourceAspect::DoubleFromString(aspectState.m_version.c_str());
    }

#ifdef COMMENT_OUT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename XSAT>
ExternalSourceAspectIterator::ExternalSourceAspectIterator(DgnDbR db, DgnElementId scope, Utf8CP kind, Utf8StringCR wh) : m_db(db))
    {
    Utf8String ecsql("SELECT ECInstanceId, Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE ( (Scope.Id=?) AND (Kind=?)");
    if (!wh.empty())
        ecsql.append(" AND (").append(wh).append(" )");
    ecsql.append(" )");

    m_stmt = db.GetPreparedECSqlStatement(ecsql.c_str());

    if (!m_stmt.IsValid())
        return;

    m_stmt->BindId(1, scope);
    m_stmt->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename XSAT>
BeSQLite::DbResult ExternalSourceAspectIterator::Step() const
    {
    if (!m_stmt.IsValid())
        return BE_SQLITE_DONE;

    auto rc = m_stmt->Step();
    if (BE_SQLITE_ROW !=  rc)
        {
        m_aspect.Invalidate();
        return rc;
        }

    auto el = m_db.Elements().GetElement(m_stmt->GetValueId<DgnElementId>(1));
    if (!el.IsValid())
        {
        BeAssert(false);
        m_aspect.Invalidate();
        return BE_SQLITE_ERROR;
        }
    m_aspect = iModelExternalSourceAspect::GetAspect(*el, m_stmt->GetValueId<BeSQLite::EC::ECInstanceId>(0));
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename XSAT>
void ExternalSourceAspectIterator::Entry::Step()
    {
    if (m_it) 
        {
        if (BeSQLite::BE_SQLITE_ROW != m_it->Step()) 
            m_it = nullptr;
        }
    }
#endif
