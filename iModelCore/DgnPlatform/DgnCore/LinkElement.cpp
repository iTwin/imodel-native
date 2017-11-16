/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LinkElement.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/LinkElement.h>

#define URLLINK_Url "Url"
#define URLLINK_UserLabel "UserLabel"
#define URLLINK_Description "Description"

#define REPOLINK_RepositoryGuid "RepositoryGuid"

#define EMBEDDEDFILELINK_Name "Name"
#define EMBEDDEDFILELINK_UserLabel "UserLabel"
#define EMBEDDEDFILELINK_Description "Description"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ModelHandler
{
    HANDLER_DEFINE_MEMBERS(Link)
};

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(LinkPartitionHandler)
    HANDLER_DEFINE_MEMBERS(UrlLinkHandler)
    HANDLER_DEFINE_MEMBERS(RepositoryLinkHandler)
    HANDLER_DEFINE_MEMBERS(EmbeddedFileLinkHandler)
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinkPartition::_OnSubModelInsert(DgnModelCR model) const 
    {
    if (nullptr == dynamic_cast<LinkModelCP>(&model))
        {
        BeAssert(false && "A LinkPartition can only be modeled by a LinkModel");
        return DgnDbStatus::ElementBlockedChange;
        }

    return T_Super::_OnSubModelInsert(model);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2016
//---------------------------------------------------------------------------------------
LinkPartitionPtr LinkPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::LinkPartitionHandler::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    LinkPartitionPtr partition = new LinkPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2016
//---------------------------------------------------------------------------------------
LinkPartitionCPtr LinkPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    LinkPartitionPtr partition = Create(parentSubject, name, description);
    if (!partition.IsValid())
        return nullptr;

    return parentSubject.GetDgnDb().Elements().Insert<LinkPartition>(*partition);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinkModel::_OnInsertElement(DgnElementR el)
    {
    DgnDbStatus status = T_Super::_OnInsertElement(el);
    if (DgnDbStatus::Success != status)
        return status;

    LinkElementCP linkElement = dynamic_cast<LinkElementCP> (&el);
    if (linkElement == nullptr)
        return DgnDbStatus::WrongModel;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinkElement::_OnInsert()
    {
    if (nullptr == GetModel()->ToInformationModel())
        {
        BeAssert(false && "Can insert LinkElement only in an InformationModel");
        return DgnDbStatus::WrongModel;
        }

    return T_Super::_OnInsert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkElement::AddToSource(DgnElementId sourceElementId) const
    {
    return AddToSource(GetDgnDb(), GetElementId(), sourceElementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
// static 
BentleyStatus LinkElement::AddToSource(DgnDbR dgndb, DgnElementId linkId, DgnElementId sourceElementId)
    {
    if (!sourceElementId.IsValid())
        {
        BeAssert(false && "Invalid source element");
        return ERROR;
        }

    if (!linkId.IsValid())
        {
        BeAssert(false && "Cannot call AddToSource without inserting the link into the Db.");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BIS_SCHEMA(BIS_REL_ElementHasLinks) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr stmt = dgndb.GetNonSelectPreparedECSqlStatement(ecSql, dgndb.GetECCrudWriteToken());
    BeAssert(stmt.IsValid());

    stmt->BindId(1, sourceElementId);
    stmt->BindId(2, linkId);

    DbResult stepStatus = stmt->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
bool LinkElement::IsFromSource(DgnElementId sourceElementId) const
    {
    return IsFromSource(GetDgnDb(), GetElementId(), sourceElementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
// static 
bool LinkElement::IsFromSource(DgnDbR dgndb, DgnElementId linkId, DgnElementId sourceElementId)
    {
    if (!sourceElementId.IsValid())
        {
        BeAssert(false && "Invalid source element");
        return false;
        }

    if (!linkId.IsValid())
        {
        BeAssert(false && "Cannot call AddToSource without inserting the link into the Db.");
        return false;
        }

    Utf8CP ecSql = "SELECT * FROM ONLY " BIS_SCHEMA(BIS_REL_ElementHasLinks) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?";
    CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(stmt.IsValid());

    stmt->BindId(1, sourceElementId);
    stmt->BindId(2, linkId);

    return (stmt->Step() == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkElement::RemoveFromSource(DgnElementId sourceElementId) const
    {
    return RemoveFromSource(GetDgnDb(), GetElementId(), sourceElementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
// static 
BentleyStatus LinkElement::RemoveFromSource(DgnDbR dgndb, DgnElementId linkId, DgnElementId sourceElementId)
    {
    if (!sourceElementId.IsValid())
        {
        BeAssert(false && "Invalid source element");
        return ERROR;
        }

    if (!linkId.IsValid())
        {
        BeAssert(false && "Cannot (and need not) call RemoveFromSource for a link that hasn't even been inserted into the Db.");
        return ERROR;
        }

    return (BE_SQLITE_OK == dgndb.DeleteLinkTableRelationships(BIS_SCHEMA(BIS_REL_ElementHasLinks), sourceElementId, linkId))? BSISUCCESS: BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnElementIdSet LinkElement::QuerySources()
    {
    Utf8CP ecSql = "SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementHasLinks) " rel WHERE rel.TargetECInstanceId=?";

    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(GetDgnDb(), ecSql);
    BeAssert(status == ECSqlStatus::Success);
    UNUSED_VARIABLE(status);

    stmt.BindId(1, GetElementId());

    DgnElementIdSet idSet;
    DbResult result;
    while ((result = stmt.Step()) == BE_SQLITE_ROW)
        idSet.insert(stmt.GetValueId<DgnElementId>(0));
    return idSet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
UrlLink::CreateParams::CreateParams(InformationModelR linkModel, Utf8CP url /*= nullptr*/, Utf8CP label /*= nullptr*/, Utf8CP description /*= nullptr*/) : CreateParams(Dgn::DgnElement::CreateParams(linkModel.GetDgnDb(), linkModel.GetModelId(), UrlLink::QueryClassId(linkModel.GetDgnDb()), DgnCode(), label), url, description)
    {
    BeAssert(linkModel.GetModelId().IsValid() && "Creating a link requires a persisted InformationModel");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
UrlLinkCPtr UrlLink::Insert()
    {
    UrlLinkCPtr link = GetDgnDb().Elements().Insert<UrlLink>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
UrlLinkCPtr UrlLink::Update()
    {
    UrlLinkCPtr link = GetDgnDb().Elements().Update<UrlLink>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
void UrlLink::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindText(stmt.GetParameterIndex(URLLINK_Url), m_url.empty() ? nullptr : m_url.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindText(stmt.GetParameterIndex(URLLINK_Description), m_description.empty() ? nullptr : m_description.c_str(), IECSqlBinder::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnDbStatus UrlLink::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_url = stmt.GetValueText(params.GetSelectIndex(URLLINK_Url));
    m_description = stmt.GetValueText(params.GetSelectIndex(URLLINK_Description));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void UrlLink::_ToJson(JsonValueR out, JsonValueCR opts) const 
    {
    T_Super::_ToJson(out, opts);
    out[json_url()] = m_url;
    out[json_description()] = m_description;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void UrlLink::_FromJson(JsonValueR val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_url()))
        m_url = val[json_url()].asString();

    if (val.isMember(json_description()))
        m_description = val[json_description()].asString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
void UrlLink::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    UrlLinkCP otherLink = dynamic_cast<UrlLinkCP> (&other);
    if (otherLink)
        {
        m_url = otherLink->m_url;
        m_description = otherLink->m_description;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
// static
DgnElementIdSet UrlLink::Query(DgnDbCR dgndb, Utf8CP url, Utf8CP label /*= nullptr*/, Utf8CP description /*= nullptr*/, int limitCount /*= -1*/)
    {
    Utf8String ecSql = "SELECT ECInstanceId FROM ONLY " BIS_SCHEMA(BIS_CLASS_UrlLink);

    Utf8String whereClause;

    if (url)
        whereClause.append(URLLINK_Url "=?");
        
    if (label)
        whereClause.empty() ? whereClause.append(URLLINK_UserLabel "=?") : whereClause.append(" AND " URLLINK_UserLabel "=?");

    if (description) 
        whereClause.empty() ? whereClause.append(URLLINK_Description "=?") : whereClause.append(" AND " URLLINK_Description " =?");
        
    if (!whereClause.empty())
        {
        ecSql.append(" WHERE ");
        ecSql.append(whereClause);
        }

    if (limitCount > 0)
        {
        Utf8PrintfString limitClause(" LIMIT %d", limitCount);
        ecSql.append(limitClause);
        }

    BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
    BeAssert(stmt.IsValid());

    int bindIndex = 1;
    if (url)
        stmt->BindText(bindIndex++, (0 == *url) ? nullptr : url, IECSqlBinder::MakeCopy::No);

    if (label)
        stmt->BindText(bindIndex++, (0 == *label) ? nullptr : label, IECSqlBinder::MakeCopy::No);

    if (description)
        stmt->BindText(bindIndex++, (0 == *description) ? nullptr : description, IECSqlBinder::MakeCopy::No);

    return CollectElementIds(*stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Shaun.Sewall                       11/2016
//---------------------------------------------------------------------------------------
DgnCode RepositoryLink::CreateCode(InformationModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_LinkElement, *model.GetModeledElement(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RepositoryLink::CreateUniqueCode(InformationModelCR model, Utf8CP baseName)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCode code = CreateCode(model, baseName);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int counter=1;
    do  {
        Utf8PrintfString name("%s-%d", baseName, counter);
        code = CreateCode(model, name.c_str());
        counter++;
        } while (db.Elements().QueryElementIdByCode(code).IsValid());

    return code;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Shaun.Sewall                       09/2016
//---------------------------------------------------------------------------------------
RepositoryLinkPtr RepositoryLink::Create(InformationModelR model, Utf8CP url, Utf8CP name, Utf8CP description)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::RepositoryLinkHandler::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    CreateParams createParams(model, url, name, description);
    createParams.m_classId = classId;
    createParams.SetCode(CreateCode(model, name));
    return new RepositoryLink(createParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    10/2017
//---------------------------------------------------------------------------------------
void RepositoryLink::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    auto idx = stmt.GetParameterIndex(REPOLINK_RepositoryGuid);
    if (!m_repositoryGuid.IsValid())
        stmt.BindNull(idx);
    else
        stmt.BindGuid(idx, m_repositoryGuid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    10/2017
//---------------------------------------------------------------------------------------
DgnDbStatus RepositoryLink::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_repositoryGuid = stmt.GetValueGuid(params.GetSelectIndex(REPOLINK_RepositoryGuid));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    10/2017
//---------------------------------------------------------------------------------------
void RepositoryLink::_ToJson(JsonValueR out, JsonValueCR opts) const 
    {
    T_Super::_ToJson(out, opts);
    if (m_repositoryGuid.IsValid())
        out[json_repositoryGuid()] = m_repositoryGuid.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    10/2017
//---------------------------------------------------------------------------------------
void RepositoryLink::_FromJson(JsonValueR val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_repositoryGuid()))
        m_repositoryGuid.FromString(val[json_repositoryGuid()].asString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    10/2017
//---------------------------------------------------------------------------------------
void RepositoryLink::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    RepositoryLinkCP otherLink = dynamic_cast<RepositoryLinkCP> (&other);
    if (otherLink)
        {
        m_repositoryGuid = otherLink->m_repositoryGuid;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
EmbeddedFileLink::CreateParams::CreateParams(LinkModelR linkModel, Utf8CP name, Utf8CP label /*=nullptr*/, Utf8CP description /*= nullptr*/) : CreateParams(Dgn::DgnElement::CreateParams(linkModel.GetDgnDb(), linkModel.GetModelId(), EmbeddedFileLink::QueryClassId(linkModel.GetDgnDb()), DgnCode(), label), name, description)
    {
    BeAssert(linkModel.GetModelId().IsValid() && "Creating a link requires a persisted link model");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
EmbeddedFileLinkCPtr EmbeddedFileLink::Insert()
    {
    EmbeddedFileLinkCPtr link = GetDgnDb().Elements().Insert<EmbeddedFileLink>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
EmbeddedFileLinkCPtr EmbeddedFileLink::Update()
    {
    EmbeddedFileLinkCPtr link = GetDgnDb().Elements().Update<EmbeddedFileLink>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
void EmbeddedFileLink::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindText(stmt.GetParameterIndex(EMBEDDEDFILELINK_Name), m_name.empty() ? nullptr : m_name.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindText(stmt.GetParameterIndex(EMBEDDEDFILELINK_Description), m_name.empty() ? nullptr : m_description.c_str(), IECSqlBinder::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnDbStatus EmbeddedFileLink::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_name = stmt.GetValueText(params.GetSelectIndex(EMBEDDEDFILELINK_Name));
    m_description = stmt.GetValueText(params.GetSelectIndex(EMBEDDEDFILELINK_Description));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void EmbeddedFileLink::_ToJson(JsonValueR out, JsonValueCR opts) const 
    {
    T_Super::_ToJson(out, opts);
    out[json_name()] = m_name;
    out[json_description()] = m_description;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void EmbeddedFileLink::_FromJson(JsonValueR val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_name()))
        m_name = val[json_name()].asString();

    if (val.isMember(json_description()))
        m_description = val[json_description()].asString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
void EmbeddedFileLink::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    EmbeddedFileLinkCP otherLink = dynamic_cast<EmbeddedFileLinkCP> (&other);
    if (otherLink)
        {
        m_name = otherLink->m_name;
        m_description = otherLink->m_description;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
// static
DgnElementIdSet EmbeddedFileLink::Query(DgnDbCR dgndb, Utf8CP name /* = nullptr */, Utf8CP label /*= nullptr*/, Utf8CP description /* = nullptr */, int limitCount /*= -1*/)
    {
    Utf8String ecSql = "SELECT ECInstanceId FROM ONLY " BIS_SCHEMA(BIS_CLASS_EmbeddedFileLink);

    Utf8String whereClause;

    if (name)
        whereClause.append(EMBEDDEDFILELINK_Name "=?");

    if (label)
        whereClause.empty() ? whereClause.append(EMBEDDEDFILELINK_UserLabel "=?") : whereClause.append(" AND " EMBEDDEDFILELINK_UserLabel "=?");

    if (description)
        whereClause.empty() ? whereClause.append(EMBEDDEDFILELINK_Description "=?") : whereClause.append(" AND " EMBEDDEDFILELINK_Description " =?");

    if (!whereClause.empty())
        {
        ecSql.append(" WHERE ");
        ecSql.append(whereClause);
        }

    if (limitCount > 0)
        {
        Utf8PrintfString limitClause(" LIMIT %d", limitCount);
        ecSql.append(limitClause);
        }

    BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
    BeAssert(stmt.IsValid());

    int bindIndex = 1;
    if (name)
        stmt->BindText(bindIndex++, (0 == *name) ? nullptr : name, IECSqlBinder::MakeCopy::No);
    
    if (label)
        stmt->BindText(bindIndex++, (0 == *label) ? nullptr : label, IECSqlBinder::MakeCopy::No);

    if (description)
        stmt->BindText(bindIndex++, (0 == *description) ? nullptr : description, IECSqlBinder::MakeCopy::No);

    return CollectElementIds(*stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkElement::DoRemoveAllFromSource(DgnDbR dgndb, DgnElementId sourceElementId, Utf8CP schemaName, Utf8CP className, DgnElementIdSet const& removeLinkIds)
    {
    BeAssert(sourceElementId.IsValid());

    // Note: We have to work around the ECSql limitation of not allowing JOIN clauses with DELETE statements. 

    Utf8CP ecSqlFmt = "DELETE FROM ONLY " BIS_SCHEMA(BIS_REL_ElementHasLinks) " WHERE InVirtualSet(?, TargetECInstanceId)";
    Utf8PrintfString ecSql(ecSqlFmt, schemaName, className);

    BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetNonSelectPreparedECSqlStatement(ecSql.c_str(), dgndb.GetECCrudWriteToken());
    BeAssert(stmt.IsValid());

    stmt->BindInt64(1, (int64_t) &removeLinkIds);

    BeSQLite::DbResult stepStatus = stmt->Step();
    if (BeSQLite::DbResult::BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkElement::DoPurgeOrphaned(DgnDbCR dgndb, Utf8CP schemaName, Utf8CP className, DgnElementIdSet const& unusedIds)
    {
    // Note: We have to work around the ECSql limitation of not allowing JOIN clauses with DELETE statements. 

    if (unusedIds.empty())
        return SUCCESS;

    Utf8CP ecSqlFmt = "DELETE FROM ONLY %s.%s WHERE InVirtualSet(?, ECInstanceId)";
    Utf8PrintfString ecSql(ecSqlFmt, schemaName, className);

    BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetNonSelectPreparedECSqlStatement(ecSql.c_str(), dgndb.GetECCrudWriteToken());
    BeAssert(stmt.IsValid());

    stmt->BindInt64(1, (int64_t) &unusedIds);

    BeSQLite::DbResult stepStatus = stmt->Step();
    if (BeSQLite::DbResult::BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                             02/17
//---------------------------------------------------------------------------------------
void dgn_ElementHandler::UrlLinkHandler::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);
    
    params.RegisterPropertyAccessors(layout, URLLINK_Description,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (UrlLink&) elIn;
            value.SetUtf8CP(el.GetDescription());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (UrlLink&) elIn;
            el.SetDescription(value.ToString().c_str());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, URLLINK_Url,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (UrlLink&) elIn;
            value.SetUtf8CP(el.GetUrl());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (UrlLink&) elIn;
            el.SetUrl(value.ToString().c_str());
            return DgnDbStatus::Success;
            });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                             02/17
//---------------------------------------------------------------------------------------
void dgn_ElementHandler::EmbeddedFileLinkHandler::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);
    
    params.RegisterPropertyAccessors(layout, EMBEDDEDFILELINK_Description,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (EmbeddedFileLink&) elIn;
            value.SetUtf8CP(el.GetDescription());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (EmbeddedFileLink&) elIn;
            el.SetDescription(value.ToString().c_str());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, EMBEDDEDFILELINK_Name,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (EmbeddedFileLink&) elIn;
            value.SetUtf8CP(el.GetName());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (EmbeddedFileLink&) elIn;
            el.SetName(value.ToString().c_str());
            return DgnDbStatus::Success;
            });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                             02/17
//---------------------------------------------------------------------------------------
void dgn_ElementHandler::RepositoryLinkHandler::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);
    
    params.RegisterPropertyAccessors(layout, REPOLINK_RepositoryGuid,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (RepositoryLink&) elIn;
            auto beguid = el.GetRepositoryGuid();
            value.SetBinary(beguid.m_guid.b, sizeof(BeGuid), true);
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsBinary())
                return DgnDbStatus::BadArg;
            auto& el = (RepositoryLink&) elIn;
            size_t sz;
            auto bytes = value.GetBinary(sz);
            if (sz != sizeof(BeGuid))
                return DgnDbStatus::BadArg;
            BeGuid guid;
            memcpy (guid.m_guid.b, bytes, sizeof(BeGuid));
            el.SetRepositoryGuid(guid);
            return DgnDbStatus::Success;
            });

    }
END_BENTLEY_DGN_NAMESPACE
