/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnViewDb.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnViews::InsertView (View& entry)
    {
    entry.m_viewId.Invalidate();
    m_project.GetNextRepositoryBasedId (entry.m_viewId, DGN_TABLE_View, "Id");

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_View " (Id,ViewType,Name,Descr,SelectorId,Source,BaseModel,SubType) VALUES(?,?,?,?,?,?,?,?)");
    stmt.BindId(1, entry.m_viewId);
    stmt.BindInt(2, entry.m_viewType);
    stmt.BindText(3, entry.m_name, Statement::MAKE_COPY_No);
    stmt.BindText(4, entry.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindId(5, entry.m_selectorId);
    stmt.BindInt(6, entry.m_viewSource);
    stmt.BindId(7, entry.m_baseModelId);
    stmt.BindText(8, entry.m_viewSubType, Statement::MAKE_COPY_No);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnViews::UpdateView(View const& entry)
    {
    Statement stmt;
    stmt.Prepare (m_project, "UPDATE " DGN_TABLE_View " SET ViewType=?,Name=?,Descr=?,SelectorId=?,Source=?,BaseModel=?,SubType=? WHERE Id=?");
    stmt.BindInt (1, entry.m_viewType);
    stmt.BindText (2, entry.m_name, Statement::MAKE_COPY_No);
    stmt.BindText (3, entry.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindId (4, entry.m_selectorId);
    stmt.BindInt (5, entry.m_viewSource);
    stmt.BindId (6, entry.m_baseModelId);
    stmt.BindText(7, entry.m_viewSubType, Statement::MAKE_COPY_No);
    stmt.BindId (8, entry.m_viewId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnViews::DeleteView(DgnViewId viewId)
    {
    Statement stmt;
    stmt.Prepare(m_project, "DELETE FROM " DGN_TABLE_View " WHERE Id=?");
    stmt.BindId(1, viewId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModelSelectors::InsertSelector(Selector& entry)
    {
    entry.m_selectorId.Invalidate();
    m_project.GetNextRepositoryBasedId (entry.m_selectorId, DGN_TABLE_ModelSelector, "Id");

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_ModelSelector " (Id,Name,Descr,Flags) VALUES(?,?,?,?)");
    stmt.BindId (1, entry.m_selectorId);
    stmt.BindText (2, entry.m_name, Statement::MAKE_COPY_No);
    stmt.BindText (3, entry.GetDescription(), Statement::MAKE_COPY_No);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModelSelectors::DeleteSelector(DgnModelSelectorId id)
    {
    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_ModelSelector " WHERE Id=?");
    stmt.BindId(1, id);
    return stmt.Step();
    }

#define GET_SELECTOR_SUBQUERY "(SELECT Id FROM " DGN_TABLE_KeyStr " WHERE Name=\"" DGN_SETTYPE_ModelSelector "\")"
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2011
//--------------+------------------------------------------------------------------------
DbResult DgnModelSelectors::AddModelToSelector (DgnModelSelectorId selectorId, DgnModelId modelId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_SetEntry " (SetType,SetId,EntryId) VALUES(" GET_SELECTOR_SUBQUERY ",?,?)");
    stmt.BindId(1, selectorId);
    stmt.BindId(2, modelId);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModelSelectors::RemoveModelFromSelector (DgnModelSelectorId selectorId, DgnModelId modelId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_SetEntry " WHERE SetType=" GET_SELECTOR_SUBQUERY " AND SetId=? AND EntryId=?");
    stmt.BindId(1, selectorId);
    stmt.BindId(2, modelId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelSelectors::Selector DgnModelSelectors::QuerySelectorById (DgnModelSelectorId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Name,Descr,Flags FROM " DGN_TABLE_ModelSelector " WHERE Id=?");
    stmt.BindId (1, id);

    Selector entry;
    if (BE_SQLITE_ROW == stmt.Step()) 
        {
        entry.m_selectorId = id;
        entry.m_name.assign(stmt.GetValueText(0));
        entry.m_description.AssignOrClear(stmt.GetValueText(1));
        }

    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelSelectorId DgnModelSelectors::QuerySelectorId (Utf8CP selectorName) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_ModelSelector " WHERE Name=?");
    stmt.BindText (1, selectorName, Statement::MAKE_COPY_No);

    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueId<DgnModelSelectorId>(0) : DgnModelSelectorId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViews::View DgnViews::QueryViewById (DgnViewId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Name,ViewType,Descr,SelectorId,Source,BaseModel,SubType FROM " DGN_TABLE_View " WHERE Id=?");
    stmt.BindId (1, id);

    DgnViews::View entry;
    if (BE_SQLITE_ROW == stmt.Step()) 
        {
        entry.m_viewId = id;
        entry.m_name.assign(stmt.GetValueText(0));
        entry.m_viewType = (DgnViewType) stmt.GetValueInt(1);
        entry.m_description.AssignOrClear(stmt.GetValueText(2));
        entry.m_selectorId = stmt.GetValueId<DgnModelSelectorId>(3);
        entry.m_viewSource = (DgnViewSource) stmt.GetValueInt(4);
        entry.m_baseModelId = DgnModelId (stmt.GetValueInt(5));
        entry.m_viewSubType.AssignOrClear(stmt.GetValueText(6));
        }

    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnViews::QueryViewId (Utf8CP viewName) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_View " WHERE Name=?");
    stmt.BindText (1, viewName, Statement::MAKE_COPY_No);

    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueId<DgnViewId>(0) : DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnViews::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_View " WHERE (0 != (ViewType & ?1))", true);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());
    sql.BindInt (1, m_typeMask);

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViews::Iterator::Entry DgnViews::Iterator::begin () const   
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,ViewType,Name,Descr,SelectorId,Source,BaseModel,SubType FROM " DGN_TABLE_View " WHERE (0 != (ViewType & ?1))", true);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_stmt->BindInt (1, m_typeMask);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnViewId          DgnViews::Iterator::Entry::GetDgnViewId() const     {Verify(); return m_sql->GetValueId<DgnViewId>(0);}
DgnViewType        DgnViews::Iterator::Entry::GetDgnViewType() const   {Verify(); return (DgnViewType)(m_sql->GetValueInt(1));}
Utf8CP             DgnViews::Iterator::Entry::GetViewSubType() const   {Verify(); return m_sql->GetValueText(7);}
DgnViewSource      DgnViews::Iterator::Entry::GetDgnViewSource() const {Verify(); return (DgnViewSource)(m_sql->GetValueInt(5));}
Utf8CP             DgnViews::Iterator::Entry::GetName() const          {Verify(); return m_sql->GetValueText(2);}
Utf8CP             DgnViews::Iterator::Entry::GetDescription() const   {Verify(); return m_sql->GetValueText(3);}
DgnModelSelectorId DgnViews::Iterator::Entry::GetSelectorId() const    {Verify(); return m_sql->GetValueId<DgnModelSelectorId>(4);}
DgnModelId         DgnViews::Iterator::Entry::GetBaseModelId() const   {Verify(); return m_sql->GetValueId<DgnModelId>(6);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnModelSelectors::Iterator::QueryCount () const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_ModelSelector);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelSelectors::Iterator::Entry DgnModelSelectors::Iterator::begin () const   
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Descr FROM " DGN_TABLE_ModelSelector);
        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelSelectorId DgnModelSelectors::Iterator::Entry::GetId() const {return m_sql->GetValueId<DgnModelSelectorId>(0);}
Utf8CP             DgnModelSelectors::Iterator::Entry::GetName() const {return m_sql->GetValueText(1);}
Utf8CP             DgnModelSelectors::Iterator::Entry::GetDescription() const {return m_sql->GetValueText(2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelSelection::ExplicitModelsIterator DgnModelSelection::FindExplicitModel(DgnModelId id)
    {
    return std::find (m_explicitModels.begin(), m_explicitModels.end(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModelSelection::HasExplicitModel(DgnModelId id)
    {
    return FindExplicitModel(id) != m_explicitModels.end();
    }

DgnModelSelection::const_iterator DgnModelSelection::begin() const {return m_explicitModels.begin();}
DgnModelSelection::const_iterator DgnModelSelection::end() const {return m_explicitModels.end();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModelSelection::AddExplicitModel(DgnModelId modelId)
    {
    if (HasExplicitModel(modelId))
        return  ERROR;

    m_project.ModelSelectors().AddModelToSelector (m_id, modelId);
    m_explicitModels.push_back(modelId);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModelSelection::DropExplicitModel(DgnModelId modelId)
    {
    auto it = FindExplicitModel(modelId);
    if (it == m_explicitModels.end())
        return  ERROR;

    m_project.ModelSelectors().RemoveModelFromSelector (m_id, modelId);
    m_explicitModels.erase(it);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModelSelection::Load ()
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT EntryId FROM " DGN_TABLE_SetEntry " WHERE SetType=" GET_SELECTOR_SUBQUERY " AND SetId=?");
    stmt.BindId (1, m_id);

    while (BE_SQLITE_ROW == stmt.Step())
        m_explicitModels.push_back (stmt.GetValueId<DgnModelId>(0));

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSessions::InsertSession (Row& entry)
    {
    entry.m_sessionId.Invalidate();
    m_project.GetNextRepositoryBasedId (entry.m_sessionId, DGN_TABLE_Session, "Id");

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_Session " (Id,Name,Descr) VALUES(?,?,?)");
    stmt.BindId         (1, entry.m_sessionId);
    stmt.BindText (2, entry.m_name, Statement::MAKE_COPY_No);
    stmt.BindText     (3, entry.GetDescription(), Statement::MAKE_COPY_No);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSessions::UpdateSession (DgnSessions::Row const& entry)
    {
    Statement stmt;
    stmt.Prepare (m_project, "UPDATE " DGN_TABLE_Session " SET Name=?,Descr=? WHERE Id=?");
    stmt.BindText (1, entry.m_name, Statement::MAKE_COPY_No);
    stmt.BindText     (2, entry.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindId         (3, entry.m_sessionId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSessions::DeleteSession (DgnSessionId sessionId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_Session " WHERE Id=?");
    stmt.BindId(1, sessionId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSessions::Row DgnSessions::QuerySessionById (DgnSessionId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Name,Descr FROM " DGN_TABLE_Session " WHERE Id=?");
    stmt.BindId (1, id);

    DgnSessions::Row entry;
    if (BE_SQLITE_ROW == stmt.Step()) 
        {
        entry.m_sessionId = id;
        entry.m_name.assign(stmt.GetValueText(0));
        entry.m_description.assign(stmt.GetValueText(1));
        }

    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSessionId DgnSessions::QuerySessionId (Utf8CP sessionName) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_Session " WHERE Name=?");
    stmt.BindText (1, sessionName, Statement::MAKE_COPY_No);

    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueId<DgnSessionId>(0) : DgnSessionId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSessions::Iterator::Entry DgnSessions::Iterator::begin () const   
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Descr FROM " DGN_TABLE_Session);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
size_t DgnSessions::Iterator::QueryCount () const
    {
    Utf8String sqlString = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE_Session);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str ());

    return ((BE_SQLITE_ROW != sql.Step ()) ? 0 : sql.GetValueInt (0));
    }

DgnSessionId DgnSessions::Iterator::Entry::GetSessionId() const {Verify(); return m_sql->GetValueId<DgnSessionId>(0);}
Utf8CP DgnSessions::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP DgnSessions::Iterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(2);}

