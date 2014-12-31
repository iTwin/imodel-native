/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SqlGenerator.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************* SqlSelect ****************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect::SqlSelect() 
    : m_selectDistinctRows (false), m_limitRowCount (0), m_limitOffset (0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect::SqlSelect (const SqlSelect& select)
    {
    CopyFrom (select);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::operator= (const SqlSelect& select)
    {
    if (this == &select)
        return *this;
    CopyFrom (select);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SqlSelect::CopyFrom (const SqlSelect& select)
    {
    m_selectExprs           = select.m_selectExprs;
    m_fromExprs             = select.m_fromExprs;
    m_whereExprs            = select.m_whereExprs;
    m_groupByExprs          = select.m_groupByExprs;
    m_havingExprs           = select.m_havingExprs;
    m_orderByExprs          = select.m_orderByExprs;
    m_joinOnExprs           = select.m_joinOnExprs;
    m_selectDistinctRows    = select.m_selectDistinctRows;
    m_limitRowCount         = select.m_limitRowCount;
    m_limitOffset           = select.m_limitOffset;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddSelect (Utf8CP expr) 
    {
    m_selectExprs.push_back (expr); 
    return *this; 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddSelect (Utf8CP table, Utf8CP column)
    {
    return AddSelect (SqlPrintfString ("[%s].[%s]", table, column)); 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::SetDistinct (bool selectDistinctRows) 
    {
    m_selectDistinctRows = selectDistinctRows;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::ReplaceSelectExpression (int columnIndex, Utf8CP expr)
    {
    m_selectExprs[columnIndex] = expr;
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddFrom (Utf8CP expr) 
    { 
    m_fromExprs.push_back (expr);
    return *this; 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddFrom (Utf8CP table, Utf8CP alias)
    {
    return AddFrom (SqlPrintfString ("[%s] [%s]", table, alias)); 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::ClearFrom () 
    { 
    m_fromExprs.clear(); 
    return *this; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddJoinOn (Utf8CP table, Utf8CP tableAlias, Utf8CP onExpression)
    {
    m_joinOnExprs.push_back (SqlPrintfString ("[%s] [%s] ON %s", table, tableAlias, onExpression).GetUtf8CP());
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddWhere (Utf8CP expr) 
    { 
    m_whereExprs.push_back (expr); 
    return *this; 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddWhere (Utf8CP table, Utf8CP column, Utf8CP expr)
    {
    return AddWhere (SqlPrintfString ("[%s].[%s] %s", table, column, expr)); 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddWhere (Utf8CP column, Utf8CP expr)
    {
    return AddWhere (SqlPrintfString ("[%s] %s", column, expr)); 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddOrderBy (Utf8CP expr) 
    { 
    m_orderByExprs.push_back (expr);
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddOrderBy (Utf8CP table, Utf8CP column)
    {
    return AddOrderBy (SqlPrintfString ("[%s].[%s]", table, column)); 
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddGroupBy (Utf8CP expr) 
    { 
    m_groupByExprs.push_back (expr);
    return *this; 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::AddHaving  (Utf8CP expr) 
    { 
    m_havingExprs.push_back (expr); 
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::SetLimit (int rows, int offset) 
    { 
    m_limitRowCount = rows;
    m_limitOffset = offset;
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlSelect& SqlSelect::SetLimit (int rows) 
    {
    return SetLimit (rows, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void appendExpressions (Utf8String& sql, const bvector<Utf8String>& strings, Utf8CP separator, bool padWithBraces)
    {
    for (bvector<Utf8String>::const_iterator iter=strings.begin(); iter != strings.end(); iter++)
        {
        Utf8StringCR token = *iter;
        if (padWithBraces)
            sql.append ("(");
        sql.append (token.c_str());
        if (padWithBraces)
            sql.append (")");
        if (iter != (strings.end() - 1))
            sql.append (separator);
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SqlSelect::GetSql (Utf8String& sql) const
    {
    sql = "SELECT ";
    if (m_selectDistinctRows)
        sql.append ("DISTINCT ");

    if (m_selectExprs.size() == 0)
        sql.append ("*");
    else
        appendExpressions (sql, m_selectExprs, ", ", false);

    sql.append (" FROM ");
    if (m_fromExprs.size() == 0)
        {
        return false;
        }
    else
        appendExpressions (sql, m_fromExprs, ", ", false);
        
    if (m_joinOnExprs.size() > 0)
        {
        for (bvector<Utf8String>::const_iterator iter=m_joinOnExprs.begin(); iter != m_joinOnExprs.end(); iter++)
            {
            sql.append (" JOIN ");
            sql.append (*iter);
            }
        }

    if (m_whereExprs.size() > 0)
        {
        sql.append (" WHERE ");
        appendExpressions (sql, m_whereExprs, " AND ", true);
        }

    if (m_groupByExprs.size() > 0)
        {
        sql.append (" GROUP BY ");
        appendExpressions (sql, m_groupByExprs, ", ", false);
        }

    if (m_havingExprs.size() > 0)
        {
        sql.append (" HAVING ");
        appendExpressions (sql, m_groupByExprs, " AND ", true);
        }

    if (m_orderByExprs.size() > 0)
        {
        sql.append (" ORDER BY ");
        appendExpressions (sql, m_orderByExprs, " AND ", false);
        }

    if (m_limitRowCount > 0)
        {
        if (m_limitOffset > 0)
            sql.append (SqlPrintfString (" LIMIT %d OFFSET %d", m_limitRowCount, m_limitOffset));
        else
            sql.append (SqlPrintfString (" LIMIT %d", m_limitRowCount));
        }
    return true;
    }

//************* SqlDelete ****************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlDelete::SqlDelete() 
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlDelete& SqlDelete::SetTable (Utf8CP table) 
    {
    m_table = table; 
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlDelete& SqlDelete::AddWhere (Utf8CP expr)
    {
    m_whereExprs.push_back (expr); 
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SqlDelete& SqlDelete::AddWhere (Utf8CP column, Utf8CP expr)
    {
    return AddWhere (SqlPrintfString ("[%s] %s", column, expr)); 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SqlDelete::GetSql (Utf8String& sql) const
    {
    sql = "DELETE FROM ";

    if (Utf8String::IsNullOrEmpty (m_table.c_str ()))
        return false;

    sql.append (m_table); 
    if (m_whereExprs.size() > 0)
        {
        sql.append (" WHERE ");
        appendExpressions (sql, m_whereExprs, " AND ", true);
        }

    return true;
    }

//************* SqlUpdate ****************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlUpdate::SqlUpdate() 
    : m_action (SQLUPDATE_None)
    {}


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlUpdate& SqlUpdate::SetTable (Utf8CP table) 
    {
    m_table = table; 
    return *this;
    } 

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlUpdate& SqlUpdate::AddUpdateColumn (Utf8CP column, Utf8CP val) 
    { 
    Utf8String sql;
    m_updateColumns.push_back ((Utf8String) SqlPrintfString ("[%s] = %s", column, val));
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlUpdate& SqlUpdate::AddWhere (Utf8CP expr)
    {
    m_whereExprs.push_back (expr); 
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlUpdate& SqlUpdate::AddWhere (Utf8CP column, Utf8CP expr) 
    {
    return AddWhere (SqlPrintfString ("[%s] %s", column, expr)); 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlUpdate& SqlUpdate::SetOnConflict (SqlUpdateActions action) 
    {
    m_action = action;
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
bool SqlUpdate::GetSql (Utf8String& sql) const
    {
    sql = "UPDATE ";

    if (Utf8String::IsNullOrEmpty (m_table.c_str ()))
        return false;

    switch (m_action)
        {
        case SQLUPDATE_None: break;
        case SQLUPDATE_Rollback:
            sql.append ("OR ROLLBACK "); break;
        case SQLUPDATE_Abort:
            sql.append ("OR ABORT "); break;
        case SQLUPDATE_Replace:
            sql.append ("OR REPLACE "); break;
        case SQLUPDATE_Fail:
            sql.append ("OR FAIL "); break;
        case SQLUPDATE_Ignore:
            sql.append ("OR IGNORE "); break;
        default:
            return false;
        }

    sql.append (m_table); 
    sql.append (" ");

    if (m_updateColumns.size() == 0)
        return false;

    sql.append ("SET ");
    appendExpressions (sql, m_updateColumns, ", ", false);

    if (m_whereExprs.size() > 0)
        {
        sql.append (" WHERE ");
        appendExpressions (sql, m_whereExprs, " AND ", true);
        }

    return true;
    }

//************* SqlInsert ****************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlInsert::SqlInsert() 
    : m_action (SQLINSERT_None) 
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlInsert& SqlInsert::SetTable (Utf8CP table) 
    {
    m_table = table; 
    return *this;
    } 

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlInsert& SqlInsert::AddInsertColumn (Utf8CP column, Utf8CP value) 
    { 
    Utf8String sql;
    m_insertColumns.push_back ((Utf8String) SqlPrintfString ("[%s]", column));
    m_insertValues. push_back ((Utf8String) SqlPrintfString ("%s", value));
    return *this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
SqlInsert& SqlInsert::SetOnConflict (SqlInsertActions action) 
    {
    m_action = action;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
int SqlInsert::GetParameterCount()
    {
    int nParameters = 0;
    for (Utf8StringCR val : m_insertValues)
        if (val.Equals("?"))
            nParameters++;

    return nParameters;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/    
bool SqlInsert::GetSql (Utf8String& sql) const
    {
    sql = "INSERT INTO ";

    if (Utf8String::IsNullOrEmpty (m_table.c_str ()))
        return false;

    switch (m_action)
        {
        case SQLINSERT_None: break;
        case SQLINSERT_Rollback:
            sql.append ("OR ROLLBACK "); break;
        case SQLINSERT_Abort:
            sql.append ("OR ABORT "); break;
        case SQLINSERT_Replace:
            sql.append ("OR REPLACE "); break;
        case SQLINSERT_Fail:
            sql.append ("OR FAIL "); break;
        case SQLINSERT_Ignore:
            sql.append ("OR IGNORE "); break;
        default:
            return false;
        }

    sql.append (m_table); 
    sql.append (" (");

    if (m_insertColumns.size() == 0)
        return false;

    appendExpressions (sql, m_insertColumns, ", ", false);

    sql.append (") VALUES (");

    appendExpressions (sql, m_insertValues, ", ", false);

    sql.append (")");

    return true;
    }  

END_BENTLEY_SQLITE_EC_NAMESPACE
