/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SqlGenerator.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=================================================================================
//! @deprecated Only used by deprecated ECDbStatement
//* @bsiclass                                                     Affan.Khan       04/2012
//+===============+===============+===============+===============+===============+======
struct SqlBuilder
{
public:
    virtual bool GetSql(Utf8String& sql) const=0;
};

//=================================================================================
//! @deprecated Only used by deprecated ECDbStatement
//* @bsiclass                                                     Affan.Khan       04/2012
//+===============+===============+===============+===============+===============+======
struct SqlSelect : SqlBuilder
{
private:
   bvector<Utf8String>  m_selectExprs;
   bvector<Utf8String>  m_fromExprs;
   bvector<Utf8String>  m_whereExprs;
   bvector<Utf8String>  m_groupByExprs;
   bvector<Utf8String>  m_havingExprs;
   bvector<Utf8String>  m_orderByExprs;
   bvector<Utf8String>  m_joinOnExprs;
   bool                 m_selectDistinctRows;
   int                  m_limitRowCount;
   int                  m_limitOffset;

private:
    void        CopyFrom         (const SqlSelect& select);

public:
                SqlSelect        ();
                SqlSelect        (const SqlSelect& select);
    SqlSelect&  operator=        (const SqlSelect& select);
    SqlSelect&  AddSelect (Utf8CP expr);
    SqlSelect&  AddSelect (Utf8CP table, Utf8CP column);
    SqlSelect&  SetDistinct (bool selectDistinctRows);
    SqlSelect&  ReplaceSelectExpression (int columnIndex, Utf8CP expr);
    SqlSelect&  AddFrom (Utf8CP expr);
    SqlSelect&  AddFrom (Utf8CP expr, Utf8CP alias);
    SqlSelect&  ClearFrom ();
    SqlSelect&  AddJoinOn (Utf8CP table, Utf8CP tableAlias, Utf8CP onExpression);
    SqlSelect&  AddWhere (Utf8CP expr);
    SqlSelect&  AddWhere (Utf8CP table, Utf8CP column, Utf8CP expr);
    SqlSelect&  AddWhere (Utf8CP column, Utf8CP expr);
    SqlSelect&  AddGroupBy (Utf8CP expr);
    SqlSelect&  AddHaving (Utf8CP expr);
    SqlSelect&  AddOrderBy (Utf8CP expr);
    SqlSelect&  AddOrderBy (Utf8CP table, Utf8CP column);
    SqlSelect&  SetLimit (int rows);
    SqlSelect&  SetLimit (int rows, int offset);
    bool        GetSql           (Utf8String& sql) const;
};

//=================================================================================
//! @deprecated Only used by deprecated ECDbStatement
//* @bsiclass                                                     Affan.Khan       04/2012
//+===============+===============+===============+===============+===============+======
struct SqlDelete: SqlBuilder
{
private:
   bvector<Utf8String>  m_whereExprs;
   Utf8String           m_table;
public:
                SqlDelete();
    SqlDelete&  SetTable    (Utf8CP table);
    SqlDelete&  AddWhere    (Utf8CP expr);
    SqlDelete&  AddWhere    (Utf8CP column, Utf8CP expr);
    bool        GetSql      (Utf8String& sql) const;
};

//=================================================================================
//! @deprecated Only used by deprecated ECDbStatement
//* @bsiclass                                                     Affan.Khan       04/2012
//+===============+===============+===============+===============+===============+======
struct SqlUpdate: SqlBuilder
{
private:
   bvector<Utf8String>  m_whereExprs;
   bvector<Utf8String>  m_updateColumns;
   Utf8String           m_table;
   SqlUpdateActions     m_action;
public:
    SqlUpdate();
    SqlUpdate&  SetTable        (Utf8CP table);
    SqlUpdate&  AddUpdateColumn (Utf8CP column, Utf8CP value);
    SqlUpdate&  AddWhere        (Utf8CP expr);
    SqlUpdate&  AddWhere        (Utf8CP column, Utf8CP expr);
    bool        GetSql          (Utf8String& sql) const;
    SqlUpdate&  SetOnConflict   (SqlUpdateActions action);
};

//=================================================================================
//! @deprecated Only used by deprecated ECDbStatement
//* @bsiclass                                                     Affan.Khan       04/2012
//+===============+===============+===============+===============+===============+======
struct SqlInsert: SqlBuilder
{
private:
   bvector<Utf8String>  m_insertColumns;
   bvector<Utf8String>  m_insertValues;
   Utf8String           m_table;
   SqlInsertActions     m_action;
public:
    SqlInsert();
    SqlInsert&  SetTable        (Utf8CP table);
    SqlInsert&  AddInsertColumn (Utf8CP column, Utf8CP value);
    SqlInsert&  SetOnConflict   (SqlInsertActions action);
    //! Returns the number of parameters to be bound
    int GetParameterCount();
    bool        GetSql          (Utf8String& sql) const;    
    };
        
END_BENTLEY_SQLITE_EC_NAMESPACE

