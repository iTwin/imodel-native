/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnViewDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnViews::Insert(View& entry)
    {
    entry.m_viewId.Invalidate();
    m_dgndb.GetNextRepositoryBasedId(entry.m_viewId, DGN_TABLE(DGN_CLASSNAME_View), "Id");

    ViewHandlerP handler = ViewHandler::FindHandler(m_dgndb, entry.m_classId);
    if (nullptr == handler)
        return BE_SQLITE_ERROR_BadDbSchema;

    Statement stmt;
    stmt.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_View) " (Id,ViewType,Name,Descr,Source,BaseModel,ECClassId) VALUES(?,?,?,?,?,?,?)");

    int col=1;
    stmt.BindId(col++, entry.m_viewId);
    stmt.BindInt(col++,(int)entry.m_viewType);
    stmt.BindText(col++, entry.m_name, Statement::MakeCopy::No);
    stmt.BindText(col++, entry.GetDescription(), Statement::MakeCopy::No);
    stmt.BindInt(col++,(int)entry.m_viewSource);
    stmt.BindId(col++, entry.m_baseModelId);
    stmt.BindId(col++, entry.m_classId);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnViews::Update(View const& entry)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_View) " SET ViewType=?,Name=?,Descr=?,Source=?,BaseModel=?,ECClassId=? WHERE Id=?");

    int col=1;
    stmt.BindInt(col++,(int)entry.m_viewType);
    stmt.BindText(col++, entry.m_name, Statement::MakeCopy::No);
    stmt.BindText(col++, entry.GetDescription(), Statement::MakeCopy::No);
    stmt.BindInt(col++,(int)entry.m_viewSource);
    stmt.BindId(col++, entry.m_baseModelId);
    stmt.BindId(col++, entry.m_classId);
    stmt.BindId(col++, entry.m_viewId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnViews::Delete(DgnViewId viewId)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_View) " WHERE Id=?");
    stmt.BindId(1, viewId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViews::View DgnViews::QueryView(DgnViewId id) const
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT Name,ViewType,Descr,Source,BaseModel,ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_View) " WHERE Id=?");
    stmt.BindId(1, id);

    DgnViews::View entry;
    if (BE_SQLITE_ROW == stmt.Step()) 
        {
        entry.m_viewId = id;
        int col=0;
        entry.m_name.assign(stmt.GetValueText(col++));
        entry.m_viewType =(DgnViewType) stmt.GetValueInt(col++);
        entry.m_description.AssignOrClear(stmt.GetValueText(col++));
        entry.m_viewSource =(DgnViewSource) stmt.GetValueInt(col++);
        entry.m_baseModelId = stmt.GetValueId<DgnModelId>(col++);
        entry.m_classId = stmt.GetValueId<DgnClassId>(col++);
        }

    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnViews::GetUniqueViewName(Utf8CP baseName)
    {
    Utf8String tmpStr(baseName);

    if (!tmpStr.empty() && !m_dgndb.Views().QueryViewId(tmpStr.c_str()).IsValid())
        return tmpStr;

    bool addDash = !tmpStr.empty();
    int index = 0;
    size_t lastDash = tmpStr.find_last_of('-');
    if (lastDash != Utf8String::npos)
        {
        if (BE_STRING_UTILITIES_UTF8_SSCANF(&tmpStr[lastDash], "-%d", &index) == 1)
            addDash = false;
        else
            index = 0;
        }

    Utf8String uniqueViewName;
    do  {
        uniqueViewName.assign(tmpStr);
        if (addDash)
            uniqueViewName.append("-");
        uniqueViewName.append(Utf8PrintfString("%d", ++index));
        } while (m_dgndb.Views().QueryViewId(uniqueViewName.c_str()).IsValid());

    return uniqueViewName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnViews::QueryViewId(Utf8CP viewName) const
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_View) " WHERE Name=?");
    stmt.BindText(1, viewName, Statement::MakeCopy::No);

    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueId<DgnViewId>(0) : DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnViews::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_View) " WHERE (0 != (ViewType & ?1))", true);

    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());
    sql.BindInt(1, m_typeMask);

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViews::Iterator::Entry DgnViews::Iterator::begin() const   
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,ViewType,Name,Descr,Source,BaseModel,ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_View) " WHERE (0 != (ViewType & ?1))", true);

        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_stmt->BindInt(1, m_typeMask);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnViewId       DgnViews::Iterator::Entry::GetDgnViewId() const     {Verify(); return m_sql->GetValueId<DgnViewId>(0);}
DgnViewType     DgnViews::Iterator::Entry::GetDgnViewType() const   {Verify(); return (DgnViewType)(m_sql->GetValueInt(1));}
Utf8CP          DgnViews::Iterator::Entry::GetName() const          {Verify(); return m_sql->GetValueText(2);}
Utf8CP          DgnViews::Iterator::Entry::GetDescription() const   {Verify(); return m_sql->GetValueText(3);}
DgnViewSource   DgnViews::Iterator::Entry::GetDgnViewSource() const {Verify(); return (DgnViewSource)(m_sql->GetValueInt(4));}
DgnModelId      DgnViews::Iterator::Entry::GetBaseModelId() const   {Verify(); return m_sql->GetValueId<DgnModelId>(5);}
DgnClassId      DgnViews::Iterator::Entry::GetClassId() const       {Verify(); return m_sql->GetValueId<DgnClassId>(6);}
