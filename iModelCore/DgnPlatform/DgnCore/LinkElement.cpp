/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LinkElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/LinkElement.h>

#define URLLINK_Url "Url"
#define URLLINK_Label "Label"
#define URLLINK_Description "Descr"

#define EMBEDDEDFILELINK_Name "Name"
#define EMBEDDEDFILELINK_UserLabel "UserLabel"
#define EMBEDDEDFILELINK_Description "Descr"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ModelHandler
{
    HANDLER_DEFINE_MEMBERS(Link)
};

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(UrlLinkHandler)
    HANDLER_DEFINE_MEMBERS(RepositoryLinkHandler)
    HANDLER_DEFINE_MEMBERS(EmbeddedFileLinkHandler)
};

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
    LinkModelP linkModel = dynamic_cast<LinkModelP> (GetModel().get());
    if (nullptr == linkModel)
        {
        BeAssert(false && "Can insert LinkElement only in a LinkModel");
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
    CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql);
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

    Utf8CP ecSql = "DELETE FROM ONLY " BIS_SCHEMA(BIS_REL_ElementHasLinks) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?";
    CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql);
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
UrlLink::CreateParams::CreateParams(LinkModelR linkModel, Utf8CP url /*= nullptr*/, Utf8CP label /*= nullptr*/, Utf8CP description /*= nullptr*/) : CreateParams(Dgn::DgnElement::CreateParams(linkModel.GetDgnDb(), linkModel.GetModelId(), UrlLink::QueryClassId(linkModel.GetDgnDb()), DgnCode(), label), url, description)
    {
    BeAssert(linkModel.GetModelId().IsValid() && "Creating a link requires a persisted link model");
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
DgnDbStatus UrlLink::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
DgnDbStatus UrlLink::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnDbStatus UrlLink::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(URLLINK_Url), m_url.empty() ? nullptr : m_url.c_str(), IECSqlBinder::MakeCopy::No) ||
        ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(URLLINK_Description), m_description.empty() ? nullptr : m_description.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
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
        whereClause.empty() ? whereClause.append(URLLINK_Label "=?") : whereClause.append(" AND " URLLINK_Label "=?");

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
DgnDbStatus EmbeddedFileLink::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
DgnDbStatus EmbeddedFileLink::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
DgnDbStatus EmbeddedFileLink::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(EMBEDDEDFILELINK_Name), m_name.empty() ? nullptr : m_name.c_str(), IECSqlBinder::MakeCopy::No) ||
        ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(EMBEDDEDFILELINK_Description), m_name.empty() ? nullptr : m_description.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
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

END_BENTLEY_DGN_NAMESPACE
