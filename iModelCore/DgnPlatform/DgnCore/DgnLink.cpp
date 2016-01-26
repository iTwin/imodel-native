/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnLink.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnLink.h>

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// Used in persistence; do not change values.
// @bsiclass
//=======================================================================================
#define DATA_KEY_Url "url"
#define DATA_KEY_DmsPath "dmspath"
#define DATA_KEY_PortablePath "portablepath"
#define DATA_KEY_LastKnownLocalPath "lastknownlocalpath"
#define DATA_KEY_EmbeddedDocumentName "embeddeddocumentname"
#define DATA_KEY_ViewId "viewid"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinkPtr DgnLink::Create(DgnDbR db) {return new DgnLink(db);}
DgnLink::DgnLink(DgnDbR db) :
    T_Super(),
    m_db(&db),
    m_type(DgnLinkType::Invalid),
    m_data(Json::objectValue)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinkPtr DgnLink::Clone() const {return new DgnLink(*this);}
DgnLink::DgnLink(DgnLinkCR rhs) : T_Super(rhs) {CopyFrom(*this);}
DgnLinkR DgnLink::operator=(DgnLinkCR rhs) {T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void DgnLink::CopyFrom(DgnLinkCR rhs)
    {
    m_db = rhs.m_db;
    m_id = rhs.m_id;
    m_type = rhs.m_type;
    m_displayLabel = rhs.m_displayLabel;
    m_data = rhs.m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinkId DgnLink::GetId() const {return m_id;}
void DgnLink::SetId(DgnLinkId value) {m_id = value;}
DgnLinkType DgnLink::GetType() const {return m_type;}
Utf8StringCR DgnLink::GetDisplayLabel() const {return m_displayLabel;}
void DgnLink::SetDisplayLabel(Utf8CP value) {m_displayLabel.AssignOrClear(value);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
void DgnLink::GetStringFromJson(Utf8StringR value, Utf8CP key) const
    {
    Json::Value jValue = m_data[key];
    if (jValue.isString())
        value = jValue.asCString();
    else
        value.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLink::GetUrl(Utf8StringR value) const
    {
    PRECONDITION(DgnLinkType::Url == m_type, ERROR);
    
    GetStringFromJson(value, DATA_KEY_Url);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
void DgnLink::SetUrl(Utf8CP value)
    {
    m_type = DgnLinkType::Url;
    m_data.clear();
    
    if (!Utf8String::IsNullOrEmpty(value))
        m_data[DATA_KEY_Url] = Json::Value(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLink::GetExternalFilePaths(Utf8StringP dmsPath, Utf8StringP portablePath, Utf8StringP lastKnownLocalPath) const
    {
    PRECONDITION(DgnLinkType::ExternalFile == m_type, ERROR);

    if (dmsPath)
        GetStringFromJson(*dmsPath, DATA_KEY_DmsPath);

    if (portablePath)
        GetStringFromJson(*portablePath, DATA_KEY_PortablePath);

    if (lastKnownLocalPath)
        GetStringFromJson(*lastKnownLocalPath, DATA_KEY_LastKnownLocalPath);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
void DgnLink::SetExternalFilePaths(Utf8CP dmsPath, Utf8CP portablePath, Utf8CP lastKnownLocalPath)
    {
    m_type = DgnLinkType::ExternalFile;
    m_data.clear();

    if (!Utf8String::IsNullOrEmpty(dmsPath))
        m_data[DATA_KEY_DmsPath] = Json::Value(dmsPath);

    if (!Utf8String::IsNullOrEmpty(portablePath))
        m_data[DATA_KEY_PortablePath] = Json::Value(portablePath);

    if (!Utf8String::IsNullOrEmpty(lastKnownLocalPath))
        m_data[DATA_KEY_LastKnownLocalPath] = Json::Value(lastKnownLocalPath);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLink::GetEmbeddedDocumentName(Utf8StringR value) const
    {
    PRECONDITION(DgnLinkType::EmbeddedFile == m_type, ERROR);

    GetStringFromJson(value, DATA_KEY_EmbeddedDocumentName);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
void DgnLink::SetEmbeddedDocumentName(Utf8CP value)
    {
    m_type = DgnLinkType::EmbeddedFile;
    m_data.clear();

    if (!Utf8String::IsNullOrEmpty(value))
        m_data[DATA_KEY_EmbeddedDocumentName] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLink::GetViewId(DgnViewId& value) const
    {
    PRECONDITION(DgnLinkType::View == m_type, ERROR);

    value = DgnViewId(BeJsonUtilities::UInt64FromValue(m_data[DATA_KEY_ViewId]));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
void DgnLink::SetViewId(DgnViewId value)
    {
    m_type = DgnLinkType::View;
    m_data.clear();

    if (value.IsValid())
        m_data[DATA_KEY_ViewId] = value.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinkId DgnLinks::Iterator::Entry::GetId() const { Verify(); return m_sql->GetValueId<DgnLinkId>(0); }
DgnLinkType DgnLinks::Iterator::Entry::GetType() const { Verify(); return (DgnLinkType)m_sql->GetValueInt(1); }
Utf8CP DgnLinks::Iterator::Entry::GetDisplayLabel() const { Verify(); return m_sql->GetValueText(2); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinks::Iterator::const_iterator DgnLinks::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,Type,DisplayLabel FROM " DGN_TABLE(DGN_CLASSNAME_Link), false);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
size_t DgnLinks::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Link), false);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinkId DgnLinks::OnElementIterator::Entry::GetId() const { Verify(); return m_sql->GetValueId<DgnLinkId>(0); }
DgnLinkType DgnLinks::OnElementIterator::Entry::GetType() const { Verify(); return (DgnLinkType)m_sql->GetValueInt(1); }
Utf8CP DgnLinks::OnElementIterator::Entry::GetDisplayLabel() const { Verify(); return m_sql->GetValueText(2); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinks::OnElementIterator::const_iterator DgnLinks::OnElementIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString ("SELECT Link.Id,Link.Type,Link.DisplayLabel FROM " DGN_TABLE (DGN_CLASSNAME_Link) " Link INNER JOIN " DGN_TABLE (DGN_RELNAME_ElementHasLinks) " Rel ON Link.Id=Rel.LinkId INNER JOIN " DGN_TABLE (DGN_CLASSNAME_Element) " Elm ON Elm.Id=Rel.ElementId WHERE Elm.Id=?", true);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_stmt->BindId(1, m_elementId);
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
size_t DgnLinks::OnElementIterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString ("SELECT COUNT(*) FROM " DGN_TABLE (DGN_RELNAME_ElementHasLinks) " Rel INNER JOIN " DGN_TABLE (DGN_CLASSNAME_Element) " Elm ON Elm.Id = Rel.ElementId WHERE Elm.Id=? ", true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    query.BindId(1, m_elementId);
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
ECInstanceKey DgnLinks::ReferencesLinkIterator::Entry::GetECInstanceKey() const { Verify(); return ECInstanceKey((ECClassId)m_sql->GetValueInt64(0), m_sql->GetValueId<ECInstanceId>(1)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinks::ReferencesLinkIterator::const_iterator DgnLinks::ReferencesLinkIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString ("SELECT Elm.ECClassId,Elm.Id FROM " DGN_TABLE (DGN_RELNAME_ElementHasLinks) " Rel INNER JOIN " DGN_TABLE (DGN_CLASSNAME_Element) " Elm ON Elm.Id = Rel.ElementId WHERE Rel.LinkId=?", true);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_stmt->BindId(1, m_linkId);
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
size_t DgnLinks::ReferencesLinkIterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE(DGN_RELNAME_ElementHasLinks) " WHERE LinkId=?", true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    query.BindId(1, m_linkId);
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
DgnLinkPtr DgnLinks::QueryById(DgnLinkId id) const
    {
    PRECONDITION(id.IsValid(), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Type,DisplayLabel,Data FROM " DGN_TABLE(DGN_CLASSNAME_Link) " WHERE Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    DgnLinkPtr link = DgnLink::Create(m_dgndb);

    link->m_type = (DgnLinkType)query.GetValueInt(0);
    link->SetId(id);
    link->SetDisplayLabel(query.GetValueText(1));
    
    Utf8CP jsonStart = (Utf8CP)query.GetValueBlob(2);
    Utf8CP jsonEnd = jsonStart + query.GetColumnBytes(2);
    Json::Reader reader;
    POSTCONDITION(reader.parse(jsonStart, jsonEnd, link->m_data, false), nullptr);
    
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinks::Update(DgnLinkCR link)
    {
    PRECONDITION(link.GetId().IsValid(), ERROR);

    Utf8String data = Json::FastWriter::ToString(link.m_data);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Link) " SET Type=?,DisplayLabel=?,Data=? WHERE Id=?");
    update.BindInt(1, (int)link.GetType());
    update.BindText(2, link.GetDisplayLabel().c_str(), Statement::MakeCopy::No);
    update.BindBlob(3, (void const*)&data[0], (int)(data.size() + 1), Statement::MakeCopy::No);
    update.BindId(4, link.GetId());

    POSTCONDITION(BE_SQLITE_DONE == update.Step(), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinks::InsertOnElement(DgnElementId elementId, DgnLinkR link)
    {
    // Don't assert to ensure an invalid ID.
    Utf8String data = Json::FastWriter::ToString(link.m_data);

    DgnLinkId nextId(m_dgndb, DGN_TABLE(DGN_CLASSNAME_Link), "Id");

    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Link) " (Id,Type,DisplayLabel,Data) VALUES (?,?,?,?)");
    insert.BindId(1, nextId);
    insert.BindInt(2, (int)link.GetType());
    insert.BindText(3, link.GetDisplayLabel().c_str(), Statement::MakeCopy::No);
    insert.BindBlob(4, (void const*)&data[0], (int)(data.size() + 1), Statement::MakeCopy::No);

    POSTCONDITION(BE_SQLITE_DONE == insert.Step(), ERROR);

    link.SetId(nextId);

    return InsertOnElement(elementId, link.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinks::InsertOnElement(DgnElementId elementId, DgnLinkId linkId)
    {
    PRECONDITION(elementId.IsValid(), ERROR);
    PRECONDITION(linkId.IsValid(), ERROR);

    DgnLinkId nextId(m_dgndb, DGN_TABLE(DGN_RELNAME_ElementHasLinks), "Id");

    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_RELNAME_ElementHasLinks) " (Id,ElementId,LinkId) VALUES (?,?,?)");
    insert.BindId(1, nextId);
    insert.BindId(2, elementId);
    insert.BindId(3, linkId);

    POSTCONDITION(BE_SQLITE_DONE == insert.Step(), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinks::DeleteFromElement(DgnElementId elementId, DgnLinkId linkId)
    {
    PRECONDITION(elementId.IsValid(), ERROR);
    PRECONDITION(linkId.IsValid(), ERROR);
    
    Statement del;
    del.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_RELNAME_ElementHasLinks) " WHERE ElementId=? AND LinkId=?");
    del.BindId(1, elementId);
    del.BindId(2, linkId);
    
    POSTCONDITION(BE_SQLITE_DONE == del.Step(), ERROR);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
void DgnLinks::PurgeUnused()
    {
    Statement del;
    del.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Link) " WHERE Id NOT IN (SELECT LinkId FROM " DGN_TABLE(DGN_RELNAME_ElementHasLinks) ")");

    EXPECTED_CONDITION(BE_SQLITE_DONE == del.Step());
    }
