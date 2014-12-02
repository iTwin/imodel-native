/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnLinkTable.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnHandlers/DgnLinkTable.h>
#include <DgnPlatform/DgnHandlers/DgnECPersistence.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC

// DgnLinkEntry sub-classes are expected to prepare their select statement such that the following base class values are selected first, in this order:
//  0: LinkID
//  1: ElementID
//  2: Ordinal
//  3: DisplayLabel

// As a convenience, this macro selects the base class fields in the expected order, and allows for basic extension.
#define LINK_SELECT_SQL(ADDITIONAL_FIELDS) "SELECT LinkId,ElementId,Ordinal,DisplayLabel" ADDITIONAL_FIELDS " FROM " DGN_TABLE_Link " WHERE LinkId=?"
#define LINK_SELECT_INDEX_LinkID 0
#define LINK_SELECT_INDEX_ElementID 1
#define LINK_SELECT_INDEX_Ordinal 2
#define LINK_SELECT_INDEX_DisplayLabel 3
#define LINK_SELECT_INDEX_LAST 3

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2013
//---------------------------------------------------------------------------------------
DgnLinkTable DgnProject::DgnLinks()
    {
    return DgnLinkTable(*this);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2013
//---------------------------------------------------------------------------------------
static ECSchemaP getDgnECSchema(ECDbR db)
    {
    ECSchemaP dgnSchema;
    
    if (!EXPECTED_CONDITION(SUCCESS == db.GetEC().GetSchemaManager().GetECSchema(dgnSchema, DGNECSCHEMA_SchemaName)))
        return NULL;
    
    POSTCONDITION(NULL != dgnSchema, NULL)
    
    return dgnSchema;        
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntry::DgnLinkEntry(DgnProjectR project, DgnLinkId linkID, DgnLinkType linkType) : m_project(&project), m_id(linkID), m_type(linkType)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
bool DgnLinkTable::DgnLinkEntry::EnsureSelect() const
    {
    if (NULL != m_select.GetSqlStatementP())
        return true;
    
    return (SUCCESS == _PrepareSelect());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkId const& DgnLinkTable::DgnLinkEntry::GetId() const
    {
    return m_id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkType const& DgnLinkTable::DgnLinkEntry::GetLinkType() const
    {
    return m_type;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
ElementId DgnLinkTable::DgnLinkEntry::GetElementId() const
    {
    if (!EXPECTED_CONDITION(EnsureSelect())
        || !EXPECTED_CONDITION(!m_select.IsColumnNull(LINK_SELECT_INDEX_ElementID))
        || !EXPECTED_CONDITION(BE_SQLITE_INTEGER == m_select.GetColumnType(LINK_SELECT_INDEX_ElementID)))
        return ElementId();
    
    ElementId elementID (m_select.GetValueInt64(LINK_SELECT_INDEX_ElementID));
    POSTCONDITION(elementID.IsValid(), ElementId())
    return elementID;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
Utf8String DgnLinkTable::DgnLinkEntry::GetDisplayLabel() const
    {
    Utf8String displayLabel;
    
    if (!EXPECTED_CONDITION(EnsureSelect())
        || !EXPECTED_CONDITION(!m_select.IsColumnNull(LINK_SELECT_INDEX_DisplayLabel))
        || !EXPECTED_CONDITION(BE_SQLITE_TEXT == m_select.GetColumnType(LINK_SELECT_INDEX_DisplayLabel)))
        return displayLabel;
    
    displayLabel.AssignOrClear(m_select.GetValueText(LINK_SELECT_INDEX_DisplayLabel));

    POSTCONDITION(!displayLabel.empty(), displayLabel)

    return displayLabel;
    }
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::UrlDgnLinkEntry::UrlDgnLinkEntry(DgnProjectR project, DgnLinkId linkID) :
    T_Super(project, linkID, DgnLinkType::Url)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
ECClassP DgnLinkTable::UrlDgnLinkEntry::GetECClassP(ECDbR db, ECSchemaP dgnSchema)
    {
    PRECONDITION((NULL != dgnSchema) || (NULL != (dgnSchema = getDgnECSchema(db))), NULL)
    
    ECClassP urlLinkClass = dgnSchema->GetClassP(DGNECSCHEMA_CLASSNAME_UrlDgnLink);
    
    POSTCONDITION(NULL != urlLinkClass, NULL)

    return urlLinkClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinkTable::UrlDgnLinkEntry::_PrepareSelect() const
    {
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.Prepare(*m_project, LINK_SELECT_SQL(",Url"))))
        return ERROR;
    
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.BindId(1, m_id)))
        return ERROR;
    
    POSTCONDITION(BE_SQLITE_ROW == m_select.Step(), ERROR)

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::UrlDgnLinkEntry::Create(DgnProjectR project, DgnLinkId linkID)
    {
    return new UrlDgnLinkEntry(project, linkID);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
Utf8String DgnLinkTable::UrlDgnLinkEntry::GetUrl() const
    {
    int displayLabelColumnIndex = (LINK_SELECT_INDEX_LAST + 1);
    Utf8String url;
    
    if (!EXPECTED_CONDITION(EnsureSelect())
        || !EXPECTED_CONDITION(!m_select.IsColumnNull(displayLabelColumnIndex))
        || !EXPECTED_CONDITION(BE_SQLITE_TEXT == m_select.GetColumnType(displayLabelColumnIndex)))
        return url;

    url.AssignOrClear(m_select.GetValueText(displayLabelColumnIndex));

    POSTCONDITION(!url.empty(), url)

    return url;
    }
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/2014
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnViewLinkEntry::DgnViewLinkEntry(DgnProjectR project, DgnLinkId linkID) :
    T_Super(project, linkID, DgnLinkType::View)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/2014
//---------------------------------------------------------------------------------------
ECClassP DgnLinkTable::DgnViewLinkEntry::GetECClassP(ECDbR db, ECSchemaP dgnSchema)
    {
    PRECONDITION((NULL != dgnSchema) || (NULL != (dgnSchema = getDgnECSchema(db))), NULL)
    
    ECClassP linkClass = dgnSchema->GetClassP(DGNECSCHEMA_CLASSNAME_DgnViewLink);
    
    POSTCONDITION(NULL != linkClass, NULL)

    return linkClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinkTable::DgnViewLinkEntry::_PrepareSelect() const
    {
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.Prepare(*m_project, LINK_SELECT_SQL(",ViewId"))))
        return ERROR;
    
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.BindId(1, m_id)))
        return ERROR;
    
    POSTCONDITION(BE_SQLITE_ROW == m_select.Step(), ERROR)

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/2014
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::DgnViewLinkEntry::Create(DgnProjectR project, DgnLinkId linkID)
    {
    return new DgnViewLinkEntry(project, linkID);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/2014
//---------------------------------------------------------------------------------------
DgnViewId DgnLinkTable::DgnViewLinkEntry::GetViewId() const
    {
    int viewIdColumnIndex = (LINK_SELECT_INDEX_LAST + 1);
    DgnViewId viewId;
    
    if (!EXPECTED_CONDITION(EnsureSelect())
        || !EXPECTED_CONDITION(!m_select.IsColumnNull(viewIdColumnIndex))
        || !EXPECTED_CONDITION(BE_SQLITE_INTEGER == m_select.GetColumnType(viewIdColumnIndex)))
        return viewId;

    viewId = m_select.GetValueId<DgnViewId> (viewIdColumnIndex);

    POSTCONDITION(viewId.IsValid(), viewId)

    return viewId;
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Utf8CP DgnLinkTable::ExternalFileDgnLinkEntry::PATH_STRING_KEY_DmsPath = "DmsPath";
Utf8CP DgnLinkTable::ExternalFileDgnLinkEntry::PATH_STRING_KEY_PortablePath = "PortablePath";
Utf8CP DgnLinkTable::ExternalFileDgnLinkEntry::PATH_STRING_KEY_FullPath = "FullPath";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::ExternalFileDgnLinkEntry::ExternalFileDgnLinkEntry(DgnProjectR project, DgnLinkId linkID) :
    T_Super(project, linkID, DgnLinkType::ExternalFile)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
ECClassP DgnLinkTable::ExternalFileDgnLinkEntry::GetECClassP(ECDbR db, ECSchemaP dgnSchema)
    {
    PRECONDITION((NULL != dgnSchema) || (NULL != (dgnSchema = getDgnECSchema(db))), NULL)
    
    ECClassP fileLinkClass = dgnSchema->GetClassP(DGNECSCHEMA_CLASSNAME_ExternalFileDgnLink);
    
    POSTCONDITION(NULL != fileLinkClass, NULL)

    return fileLinkClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinkTable::ExternalFileDgnLinkEntry::_PrepareSelect() const
    {
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.Prepare(*m_project, LINK_SELECT_SQL(",PathStrings"))))
        return ERROR;
    
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.BindId(1, m_id)))
        return ERROR;
    
    POSTCONDITION(BE_SQLITE_ROW == m_select.Step(), ERROR)
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::ExternalFileDgnLinkEntry::Create(DgnProjectR project, DgnLinkId linkID)
    {
    return new ExternalFileDgnLinkEntry(project, linkID);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::ExternalFileDgnLinkEntry::PathStringMap DgnLinkTable::ExternalFileDgnLinkEntry::GetPathStrings() const
    {
    PathStringMap pathStringsMap;

    int pathStringsColumnIndex = (LINK_SELECT_INDEX_LAST + 1);
    Utf8String pathStringsStr;
    
    if (!EXPECTED_CONDITION(EnsureSelect())
        || !EXPECTED_CONDITION(!m_select.IsColumnNull(pathStringsColumnIndex))
        || !EXPECTED_CONDITION(BE_SQLITE_TEXT == m_select.GetColumnType(pathStringsColumnIndex)))
        return pathStringsMap;

    pathStringsStr.AssignOrClear(m_select.GetValueText(pathStringsColumnIndex));
    if (UNEXPECTED_CONDITION(pathStringsStr.empty()))
        return pathStringsMap;

    Json::Value pathStringsJson;
    if (UNEXPECTED_CONDITION(!Json::Reader::Parse(pathStringsStr, pathStringsJson, false) || pathStringsJson.isNull()))
        return pathStringsMap;
    
    for (auto pathStringKey : pathStringsJson.getMemberNames())
        pathStringsMap[pathStringKey] = pathStringsJson[pathStringKey].asString();

    POSTCONDITION(!pathStringsMap.empty(), pathStringsMap)

    return pathStringsMap;
    }
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EmbeddedFileDgnLinkEntry::EmbeddedFileDgnLinkEntry(DgnProjectR project, DgnLinkId linkID) :
    T_Super(project, linkID, DgnLinkType::EmbeddedFile)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
ECClassP DgnLinkTable::EmbeddedFileDgnLinkEntry::GetECClassP(ECDbR db, ECSchemaP dgnSchema)
    {
    PRECONDITION((NULL != dgnSchema) || (NULL != (dgnSchema = getDgnECSchema(db))), NULL)
    
    ECClassP embeddedFileLinkClass = dgnSchema->GetClassP(DGNECSCHEMA_CLASSNAME_EmbeddedFileDgnLink);
    
    POSTCONDITION(NULL != embeddedFileLinkClass, NULL)

    return embeddedFileLinkClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinkTable::EmbeddedFileDgnLinkEntry::_PrepareSelect() const
    {
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.Prepare(*m_project, LINK_SELECT_SQL(",DocumentName"))))
        return ERROR;
    
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_select.BindId(1, m_id)))
        return ERROR;
    
    POSTCONDITION(BE_SQLITE_ROW == m_select.Step(), ERROR)
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::EmbeddedFileDgnLinkEntry::Create(DgnProjectR project, DgnLinkId linkID)
    {
    return new EmbeddedFileDgnLinkEntry(project, linkID);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
Utf8String DgnLinkTable::EmbeddedFileDgnLinkEntry::GetDocumentName() const
    {
    int displayLabelColumnIndex = (LINK_SELECT_INDEX_LAST + 1);
    Utf8String documentName;
    
    if (!EXPECTED_CONDITION(EnsureSelect())
        || !EXPECTED_CONDITION(!m_select.IsColumnNull(displayLabelColumnIndex))
        || !EXPECTED_CONDITION(BE_SQLITE_TEXT == m_select.GetColumnType(displayLabelColumnIndex)))
        return documentName;

    documentName.AssignOrClear(m_select.GetValueText(displayLabelColumnIndex));

    POSTCONDITION(!documentName.empty(), documentName)

    return documentName;
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EntryFactory::EntryFactory(DgnProjectR project, BeSQLiteStatementP sql, bool isValid) :
    T_Super(sql, isValid),
    m_project(&project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkType DgnLinkTable::EntryFactory::GetLinkType() const
    {
    PRECONDITION(!m_sql->IsColumnNull(1), DgnLinkType::Invalid)
    PRECONDITION(BE_SQLITE_INTEGER == m_sql->GetColumnType(1), DgnLinkType::Invalid)
    
    ECClassId classID = m_sql->GetValueInt64(1);

    ECSchemaP dgnSchema = getDgnECSchema(*m_project);
    
    PRECONDITION(NULL != dgnSchema, DgnLinkType::Invalid)

    // UrlDgnLinkEntry?
    ECClassP urlLinkClass = UrlDgnLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != urlLinkClass))
        return DgnLinkType::Invalid;
    
    ECClassId urlLinkClassID = urlLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != urlLinkClassID))
        return DgnLinkType::Invalid;

    if (urlLinkClassID == classID)
        return DgnLinkType::Url;
        
    // DgnViewLinkEntry?
    ECClassP viewLinkClass = DgnViewLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != viewLinkClass))
        return DgnLinkType::Invalid;
    
    ECClassId viewLinkClassID = viewLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != viewLinkClassID))
        return DgnLinkType::Invalid;

    if (viewLinkClassID == classID)
        return DgnLinkType::View;
        
    // ExternalFileDgnLinkEntry?
    ECClassP fileLinkClass = ExternalFileDgnLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != fileLinkClass))
        return DgnLinkType::Invalid;

    ECClassId fileLinkClassID = fileLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != fileLinkClassID))
        return DgnLinkType::Invalid;

    if (fileLinkClassID == classID)
        return DgnLinkType::ExternalFile;
        
    // EmbeddedFileDgnLinkEntry?
    ECClassP embeddedFileLinkClass = EmbeddedFileDgnLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != embeddedFileLinkClass))
        return DgnLinkType::Invalid;

    ECClassId embeddedFileLinkClassID = embeddedFileLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != embeddedFileLinkClassID))
        return DgnLinkType::Invalid;

    if (embeddedFileLinkClassID == classID)
        return DgnLinkType::EmbeddedFile;

    POSTCONDITION(false, DgnLinkType::Invalid)
    return DgnLinkType::Invalid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::EntryFactory::CreateEntry() const
    {
    PRECONDITION(!m_sql->IsColumnNull(0), NULL)
    PRECONDITION(BE_SQLITE_INTEGER == m_sql->GetColumnType(0), NULL)
    
    DgnLinkId linkID = m_sql->GetValueId<DgnLinkId>(0);
    
    PRECONDITION(!m_sql->IsColumnNull(1), NULL)
    PRECONDITION(BE_SQLITE_INTEGER == m_sql->GetColumnType(1), NULL)
    
    ECClassId classID = m_sql->GetValueInt64(1);

    ECSchemaP dgnSchema = getDgnECSchema(*m_project);
    
    PRECONDITION(NULL != dgnSchema, NULL)

    // UrlDgnLinkEntry?
    ECClassP urlLinkClass = UrlDgnLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != urlLinkClass))
        return NULL;
    
    ECClassId urlLinkClassID = urlLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != urlLinkClassID))
        return NULL;

    if (urlLinkClassID == classID)
        {
        DgnLinkEntryPtr link = UrlDgnLinkEntry::Create(*m_project, linkID);
        POSTCONDITION(link.IsValid(), NULL)
        return link;
        }
        
    // DgnViewLinkEntry?
    ECClassP viewLinkClass = DgnViewLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != viewLinkClass))
        return NULL;
    
    ECClassId viewLinkClassID = viewLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != viewLinkClassID))
        return NULL;

    if (viewLinkClassID == classID)
        {
        DgnLinkEntryPtr link = DgnViewLinkEntry::Create(*m_project, linkID);
        POSTCONDITION(link.IsValid(), NULL)
        return link;
        }
        
    // ExternalFileDgnLinkEntry?
    ECClassP externalFileLinkClass = ExternalFileDgnLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != externalFileLinkClass))
        return NULL;

    ECClassId fileLinkClassID = externalFileLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != fileLinkClassID))
        return NULL;

    if (fileLinkClassID == classID)
        {
        DgnLinkEntryPtr link = ExternalFileDgnLinkEntry::Create(*m_project, linkID);
        POSTCONDITION(link.IsValid(), NULL)
        return link;
        }
        
    // EmbeddedFileDgnLinkEntry?
    ECClassP embeddedFileLinkClass = EmbeddedFileDgnLinkEntry::GetECClassP(*m_project, dgnSchema);
    if (!EXPECTED_CONDITION(NULL != embeddedFileLinkClass))
        return NULL;

    ECClassId embeddedFileLinkClassID = embeddedFileLinkClass->GetId();
    if (!EXPECTED_CONDITION(0 != embeddedFileLinkClassID))
        return NULL;

    if (embeddedFileLinkClassID == classID)
        {
        DgnLinkEntryPtr link = EmbeddedFileDgnLinkEntry::Create(*m_project, linkID);
        POSTCONDITION(link.IsValid(), NULL)
        return link;
        }

    POSTCONDITION(false, NULL)
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EntryFactory const& DgnLinkTable::EntryFactory::operator*() const
    {
    return *this;
    }
            
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::OnElementIterator::OnElementIterator(DgnProjectR project, ElementId elementID) :
    T_Super(project),
    m_project(&project),
    m_elementID(elementID)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EntryFactory DgnLinkTable::OnElementIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = "SELECT LinkId,ECClassId FROM " DGN_TABLE_Link " WHERE ElementId=? ORDER BY Ordinal ASC";
        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        
        m_stmt->BindId(1, m_elementID);
        }
    else
        {
        m_stmt->Reset();
        }

    return EntryFactory (*m_project, m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EntryFactory DgnLinkTable::OnElementIterator::end() const
    {
    return EntryFactory(*m_project, NULL, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
size_t DgnLinkTable::OnElementIterator::QueryCount() const
    {
    Utf8String sqlString = "SELECT COUNT(*) FROM " DGN_TABLE_Link " WHERE ElementId=?";

    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());
    sql.BindId(1, m_elementID);
        
    if (!EXPECTED_CONDITION(BE_SQLITE_ROW == sql.Step()))
        return 0;
    
    return (size_t)sql.GetValueInt(0);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::InProjectIterator::InProjectIterator(DgnProjectR project) :
    T_Super(project),
    m_project(&project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EntryFactory DgnLinkTable::InProjectIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = "SELECT LinkId,ECClassId FROM " DGN_TABLE_Link " ORDER BY ElementId ASC,Ordinal ASC";
        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        }
    else
        {
        m_stmt->Reset();
        }

    return EntryFactory(*m_project, m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::EntryFactory DgnLinkTable::InProjectIterator::end() const
    {
    return EntryFactory(*m_project, NULL, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2013
//---------------------------------------------------------------------------------------
size_t DgnLinkTable::InProjectIterator::QueryCount() const
    {
    Utf8String sqlString = "SELECT COUNT(*) FROM " DGN_TABLE_Link;

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());
    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkTable(DgnProjectR project) :
    T_Super(project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
static BentleyStatus resolveOrdinal (Int32& ordinal, DgnProjectR project, ElementId elementID, DgnLinkId const* insertBefore)
    {
    //...............................................................................................................................................
    // Just adding to the end?
    if (NULL == insertBefore)
        {
        Statement countStatement;
        if (!EXPECTED_CONDITION(BE_SQLITE_OK == countStatement.Prepare(project, "SELECT COUNT(*) FROM " DGN_TABLE_Link " WHERE ElementId=?")))
            return ERROR;
        
        if (!EXPECTED_CONDITION(BE_SQLITE_OK == countStatement.BindId(1, elementID)))
            return ERROR;
        
        if (!EXPECTED_CONDITION(BE_SQLITE_ROW == countStatement.Step()))
            return ERROR;
        
        if (!EXPECTED_CONDITION(!countStatement.IsColumnNull(0))
            || !EXPECTED_CONDITION(BE_SQLITE_INTEGER == countStatement.GetColumnType(0)))
            return ERROR;
        
        ordinal = countStatement.GetValueInt(0);

        POSTCONDITION(BE_SQLITE_DONE == countStatement.Step(), ERROR)

        return SUCCESS;
        }
    
    //...............................................................................................................................................
    // Get the existing ordinal.
    Statement existingOrdinalStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == existingOrdinalStatement.Prepare(project, "SELECT Ordinal FROM " DGN_TABLE_Link " WHERE LinkId=? AND ElementId=?")))
        return ERROR;
    
    int bindIndex = 1;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == existingOrdinalStatement.BindId(bindIndex++, *insertBefore)))
        return ERROR;
    
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == existingOrdinalStatement.BindId(bindIndex++, elementID)))
        return ERROR;
        
    if (!EXPECTED_CONDITION(BE_SQLITE_ROW == existingOrdinalStatement.Step()))
        return ERROR;
    
    if (!EXPECTED_CONDITION(!existingOrdinalStatement.IsColumnNull(0))
        || !EXPECTED_CONDITION(BE_SQLITE_INTEGER == existingOrdinalStatement.GetColumnType(0)))
        return ERROR;
        
    Int32 existingOrdinal = existingOrdinalStatement.GetValueInt(0);
    
    POSTCONDITION(BE_SQLITE_DONE == existingOrdinalStatement.Step(), ERROR)

    //...............................................................................................................................................
    // Make a hole for it. Not attempting to compact or be clever here.
    Statement updateStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == updateStatement.Prepare(project, "UPDATE " DGN_TABLE_Link " SET Ordinal=Ordinal+1 WHERE ElementId=? AND Ordinal>=?")))
        return ERROR;
        
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == updateStatement.BindId(1, elementID)))
        return ERROR;
    
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == updateStatement.BindInt(2, existingOrdinal)))
        return ERROR;
        
    POSTCONDITION(BE_SQLITE_DONE == updateStatement.Step(), ERROR)

    ordinal = existingOrdinal;
        
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::AttachUrlLink(ElementId elementID, Utf8CP displayLabel, Utf8CP url, DgnLinkId const* insertBefore)
    {
    PRECONDITION(elementID.IsValid(), NULL)
    PRECONDITION(!Utf8String::IsNullOrEmpty(displayLabel), NULL)
    PRECONDITION(!Utf8String::IsNullOrEmpty(url), NULL)
    
    //...............................................................................................................................................
    ECClassP urlLinkClass = UrlDgnLinkEntry::GetECClassP(m_project);
    PRECONDITION(NULL != urlLinkClass, NULL)

    ECClassId urlLinkClassID = urlLinkClass->GetId();
    PRECONDITION(0 != urlLinkClassID, NULL)

    Int32 ordinal;
    if (!EXPECTED_CONDITION(SUCCESS == resolveOrdinal(ordinal, m_project, elementID, insertBefore)))
        return NULL;

    DgnLinkId linkID;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_project.GetNextRepositoryBasedId(linkID, DGN_TABLE_Link, "LinkId")))
        return NULL;
    
    //...............................................................................................................................................
    Statement insertStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.Prepare(m_project, "INSERT INTO " DGN_TABLE_Link " (LinkId,ElementId,ECClassId,Ordinal,DisplayLabel,Url) VALUES (?,?,?,?,?,?)")))
        return NULL;

    int bindIndex = 1;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, linkID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, elementID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt64(bindIndex++, urlLinkClassID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt(bindIndex++, ordinal)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, displayLabel, Statement::MAKE_COPY_No)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, url, Statement::MAKE_COPY_No)))
        return NULL;
        
    POSTCONDITION(BE_SQLITE_DONE == insertStatement.Step(), NULL)

    //...............................................................................................................................................
    DgnLinkEntryPtr link = UrlDgnLinkEntry::Create(m_project, linkID);

    POSTCONDITION(link.IsValid(), NULL)
    
    //...............................................................................................................................................
    PersistentElementRefPtr elemRef = m_project.Models().GetElementById(elementID);
    if (UNEXPECTED_CONDITION(!elemRef.IsValid()))
        return link;

#if defined (NEEDS_WORK_DGNITEM)
    POSTCONDITION(SUCCESS == DgnECPersistence::AddSecondaryInstanceOnElement (elemRef->GetElementId(), ECInstanceKey(urlLinkClassID, ECInstanceId (linkID.GetValue())), m_project), link);
#endif

    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::AttachDgnViewLink(ElementId elementID, Utf8CP displayLabel, DgnViewId viewId, DgnLinkId const* insertBefore)
    {
    PRECONDITION(elementID.IsValid(), NULL)
    PRECONDITION(!Utf8String::IsNullOrEmpty(displayLabel), NULL)
    PRECONDITION(viewId.IsValid(), NULL)
    
    //...............................................................................................................................................
    ECClassP viewIdLinkClass = DgnViewLinkEntry::GetECClassP(m_project);
    PRECONDITION(NULL != viewIdLinkClass, NULL)

    ECClassId viewIdLinkClassID = viewIdLinkClass->GetId();
    PRECONDITION(0 != viewIdLinkClassID, NULL)

    Int32 ordinal;
    if (!EXPECTED_CONDITION(SUCCESS == resolveOrdinal(ordinal, m_project, elementID, insertBefore)))
        return NULL;

    DgnLinkId linkID;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_project.GetNextRepositoryBasedId(linkID, DGN_TABLE_Link, "LinkId")))
        return NULL;
    
    //...............................................................................................................................................
    Statement insertStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.Prepare(m_project, "INSERT INTO " DGN_TABLE_Link " (LinkId,ElementId,ECClassId,Ordinal,DisplayLabel,ViewId) VALUES (?,?,?,?,?,?)")))
        return NULL;

    int bindIndex = 1;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, linkID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, elementID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt64(bindIndex++, viewIdLinkClassID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt(bindIndex++, ordinal)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, displayLabel, Statement::MAKE_COPY_No)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, viewId)))
        return NULL;
        
    POSTCONDITION(BE_SQLITE_DONE == insertStatement.Step(), NULL)

    //...............................................................................................................................................
    DgnLinkEntryPtr link = DgnViewLinkEntry::Create(m_project, linkID);

    POSTCONDITION(link.IsValid(), NULL)
    
    //...............................................................................................................................................
    PersistentElementRefPtr elemRef = m_project.Models().GetElementById(elementID);
    if (UNEXPECTED_CONDITION(!elemRef.IsValid()))
        return link;

#if defined (NEEDS_WORK_DGNITEM)
    POSTCONDITION(SUCCESS == DgnECPersistence::AddSecondaryInstanceOnElement (elemRef->GetElementId(), ECInstanceKey(viewIdLinkClassID, ECInstanceId (linkID.GetValue())), m_project), link);
#endif

    return link;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::AttachExternalFileLink(ElementId elementID, Utf8CP displayLabel, ExternalFileDgnLinkEntry::PathStringMap const& pathStringsMap, DgnLinkId const* insertBefore)
    {
    PRECONDITION(elementID.IsValid(), NULL)
    PRECONDITION(!Utf8String::IsNullOrEmpty(displayLabel), NULL)
    PRECONDITION(!pathStringsMap.empty(), NULL)
    
    //...............................................................................................................................................
    ECClassP fileLinkClass = ExternalFileDgnLinkEntry::GetECClassP(m_project);
    PRECONDITION(NULL != fileLinkClass, NULL)

    ECClassId fileLinkClassID = fileLinkClass->GetId();
    PRECONDITION(0 != fileLinkClassID, NULL)
        
    Int32 ordinal;
    if (!EXPECTED_CONDITION(SUCCESS == resolveOrdinal(ordinal, m_project, elementID, insertBefore)))
        return NULL;

    DgnLinkId linkID;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_project.GetNextRepositoryBasedId(linkID, DGN_TABLE_Link, "LinkId")))
        return NULL;
    
    //...............................................................................................................................................
    Json::Value pathStringsJson(Json::objectValue);
    for (auto pathString : pathStringsMap)
        pathStringsJson[pathString.first.c_str()] = pathString.second.c_str();

    Utf8String pathStringsStr = Json::FastWriter::ToString(pathStringsJson);

    if (UNEXPECTED_CONDITION(pathStringsStr.empty()))
        return NULL;

    //...............................................................................................................................................
    Statement insertStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.Prepare(m_project, "INSERT INTO " DGN_TABLE_Link " (LinkId,ElementId,ECClassId,Ordinal,DisplayLabel,PathStrings) VALUES (?,?,?,?,?,?)")))
        return NULL;

    int bindIndex = 1;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, linkID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, elementID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt64(bindIndex++, fileLinkClassID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt(bindIndex++, ordinal)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, displayLabel, Statement::MAKE_COPY_No)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, pathStringsStr.c_str(), Statement::MAKE_COPY_No)))
        return NULL;
        
    POSTCONDITION(BE_SQLITE_DONE == insertStatement.Step(), NULL)

    //...............................................................................................................................................
    DgnLinkEntryPtr link = ExternalFileDgnLinkEntry::Create(m_project, linkID);

    POSTCONDITION(link.IsValid(), NULL)
    
    //...............................................................................................................................................
    PersistentElementRefPtr elemRef = m_project.Models().GetElementById(elementID);
    if (UNEXPECTED_CONDITION(!elemRef.IsValid()))
        return link;

#if defined (NEEDS_WORK_DGNITEM)
    POSTCONDITION(SUCCESS == DgnECPersistence::AddSecondaryInstanceOnElement (elemRef->GetElementId(), ECInstanceKey(fileLinkClassID, ECInstanceId (linkID.GetValue())), m_project), link);
#endif
    
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
DgnLinkTable::DgnLinkEntryPtr DgnLinkTable::AttachEmbeddedFileLink(ElementId elementID, Utf8CP displayLabel, Utf8CP documentName, DgnLinkId const* insertBefore)
    {
    PRECONDITION(elementID.IsValid(), NULL)
    PRECONDITION(!Utf8String::IsNullOrEmpty(displayLabel), NULL)
    PRECONDITION(!Utf8String::IsNullOrEmpty(documentName), NULL)
    
    //...............................................................................................................................................
    ECClassP embeddedFileLinkClass = EmbeddedFileDgnLinkEntry::GetECClassP(m_project);
    PRECONDITION(NULL != embeddedFileLinkClass, NULL)

    ECClassId embeddedFileLinkClassID = embeddedFileLinkClass->GetId();
    PRECONDITION(0 != embeddedFileLinkClassID, NULL)
        
    Int32 ordinal;
    if (!EXPECTED_CONDITION(SUCCESS == resolveOrdinal(ordinal, m_project, elementID, insertBefore)))
        return NULL;

    DgnLinkId linkID;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == m_project.GetNextRepositoryBasedId(linkID, DGN_TABLE_Link, "LinkId")))
        return NULL;
    
    //...............................................................................................................................................
    Statement insertStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.Prepare(m_project, "INSERT INTO " DGN_TABLE_Link " (LinkId,ElementId,ECClassId,Ordinal,DisplayLabel,DocumentName) VALUES (?,?,?,?,?,?)")))
        return NULL;

    int bindIndex = 1;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, linkID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindId(bindIndex++, elementID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt64(bindIndex++, embeddedFileLinkClassID)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindInt(bindIndex++, ordinal)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, displayLabel, Statement::MAKE_COPY_No)))
        return NULL;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == insertStatement.BindText(bindIndex++, documentName, Statement::MAKE_COPY_No)))
        return NULL;
        
    POSTCONDITION(BE_SQLITE_DONE == insertStatement.Step(), NULL)

    //...............................................................................................................................................
    DgnLinkEntryPtr link = EmbeddedFileDgnLinkEntry::Create(m_project, linkID);

    POSTCONDITION(link.IsValid(), NULL)
    
    //...............................................................................................................................................
    PersistentElementRefPtr elemRef = m_project.Models().GetElementById(elementID);
    if (UNEXPECTED_CONDITION(!elemRef.IsValid()))
        return link;

#if defined (NEEDS_WORK_DGNITEM)
    POSTCONDITION(SUCCESS == DgnECPersistence::AddSecondaryInstanceOnElement (elemRef->GetElementId(), ECInstanceKey(embeddedFileLinkClassID, ECInstanceId (linkID.GetValue())), m_project), link);
#endif
    
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnLinkTable::DeleteLink(DgnLinkId linkID)
    {
    PRECONDITION(linkID.IsValid(), ERROR)
    
    //...............................................................................................................................................
    Statement linkQuerySql;
    if (UNEXPECTED_CONDITION(BE_SQLITE_OK != linkQuerySql.Prepare(m_project, "SELECT ElementId,ECClassId FROM " DGN_TABLE_Link " WHERE LinkId=?")))
        return ERROR;

    if (UNEXPECTED_CONDITION(BE_SQLITE_OK != linkQuerySql.BindId(1, linkID)))
        return ERROR;
        
    if (UNEXPECTED_CONDITION(BE_SQLITE_ROW != linkQuerySql.Step()))
        return ERROR;
    
    if (UNEXPECTED_CONDITION(linkQuerySql.IsColumnNull(0))
        || UNEXPECTED_CONDITION(BE_SQLITE_INTEGER != linkQuerySql.GetColumnType(0)))
        return ERROR;
    
    ElementId elementID(linkQuerySql.GetValueId<ElementId>(0));
    if (UNEXPECTED_CONDITION(!elementID.IsValid()))
        return ERROR;
    
    if (UNEXPECTED_CONDITION(linkQuerySql.IsColumnNull(1))
        || UNEXPECTED_CONDITION(BE_SQLITE_INTEGER != linkQuerySql.GetColumnType(1)))
        return ERROR;
    
    ECClassId classID = linkQuerySql.GetValueInt64(1);
    if (UNEXPECTED_CONDITION(0 == classID))
        return ERROR;

    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != linkQuerySql.Step()))
        return ERROR;
    
    //...............................................................................................................................................
    PersistentElementRefPtr elemRef = m_project.Models().GetElementById(elementID);
    if (UNEXPECTED_CONDITION(!elemRef.IsValid()))
        return ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    if (UNEXPECTED_CONDITION(SUCCESS != DgnECPersistence::RemoveSecondaryInstanceOnElement (elemRef->GetElementId(), ECInstanceKey(classID, ECInstanceId (linkID.GetValue())), m_project)))
        return ERROR;
#endif
    
    //...............................................................................................................................................
    Statement deleteStatement;
    if (!EXPECTED_CONDITION(BE_SQLITE_OK == deleteStatement.Prepare(m_project, "DELETE FROM " DGN_TABLE_Link " WHERE LinkId=?")))
        return ERROR;

    int bindIndex = 1;

    if (!EXPECTED_CONDITION(BE_SQLITE_OK == deleteStatement.BindId(bindIndex++, linkID)))
        return ERROR;

    POSTCONDITION(BE_SQLITE_DONE == deleteStatement.Step(), ERROR)

    return SUCCESS;
    }
