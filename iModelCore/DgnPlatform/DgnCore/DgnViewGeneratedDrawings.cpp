/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnViewGeneratedDrawings.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnViewGeneratedDrawings::QuerySourceView (DgnModelId modelId) const
    {
    BeSQLite::Statement stmt;
    stmt.Prepare (m_project, "SELECT ViewId FROM " DGN_TABLE_GeneratedModel " WHERE ModelId=?");
    stmt.BindId (1, modelId);
    if (stmt.Step() == BeSQLite::BE_SQLITE_ROW)
        return stmt.GetValueId<DgnViewId> (0);

    return DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnViewGeneratedDrawings::Insert (DgnModelId modelId, DgnViewId viewId) const
    {
    BeSQLite::Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_GeneratedModel " (ViewId,ModelId) VALUES(?,?)");
    stmt.BindId (1, viewId);
    stmt.BindId (2, modelId);
    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
template <typename LISTTYPE>
static Utf8String formatIdList (LISTTYPE const& list)
    {
    Utf8String str;
    char const* comma = "";
    for (auto id : list)
        {
        str.append (Utf8PrintfString("%s%llu", comma, id.GetValue()));
        comma = ",";
        }
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
//static Utf8String formatIdStmt (BeSQLite::Statement& stmt)
//    {
//    Utf8String str;
//    char const* comma = "";
//    while (stmt.Step() == BeSQLite::BE_SQLITE_ROW)
//        {
//        str.append (Utf8PrintfString("%s%lld", comma, stmt.GetValueInt64(0)));
//        comma = ",";
//        }
//    return str;
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
bvector<DgnModelId> DgnViewGeneratedDrawings::QueryGeneratedModels (DgnViewId viewId) const
    {
    bvector<DgnModelId> models;

    BeSQLite::Statement dmodelsStmt;
    if (dmodelsStmt.Prepare (m_project, " SELECT ModelId FROM " DGN_TABLE_GeneratedModel " WHERE ViewId=?") != BeSQLite::BE_SQLITE_OK)
        return models;

    dmodelsStmt.BindId (1, viewId);
    
    while (dmodelsStmt.Step() == BE_SQLITE_ROW)
        models.push_back (dmodelsStmt.GetValueId<DgnModelId>(0));

    return models;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
BeSQLite::DbResult DgnViewGeneratedDrawings::QueryGeneratedModels (BeSQLite::Statement& dmodelsSmt, bset<DgnModelId> const& pmodels, char const* cols) const
    {
    auto pmodelsStr = formatIdList (pmodels);
    if (pmodelsStr.empty())
        return BeSQLite::BE_SQLITE_NOTFOUND;

    //  This selects all models that were generated from one of the models in 'pmodels'
    Utf8PrintfString sql (
    " SELECT G.ModelId FROM " DGN_TABLE_GeneratedModel " AS G"        // I want each model generated 
    "  JOIN " DGN_TABLE_View " AS V ON V.[Id]=G.ViewId"                   // by a view, where
    "  WHERE V.BaseModel IN (%s)",                                               // the view targets some model in pmodels
    cols, pmodelsStr.c_str());

    return dmodelsSmt.Prepare (m_project, sql.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
BeSQLite::DbResult DgnViewGeneratedDrawings::QueryViewsOfGeneratedModels (BeSQLite::Statement& dviewsSmt, bset<DgnModelId> const& pmodels, char const* cols) const
    {
    auto pmodelsStr = formatIdList (pmodels);
    if (pmodelsStr.empty())
        return BeSQLite::BE_SQLITE_NOTFOUND;

    //  This selects all models that were generated from one of the models in 'pmodels'
    Utf8PrintfString sql (
"SELECT %s FROM " DGN_TABLE_View " AS V2"                        // I want each view that
" WHERE V2.BaseModel IN "                                               // targets a model in this set of models:
    " ("
    " SELECT G.ModelId FROM " DGN_TABLE_GeneratedModel " AS G"        // I want each model generated 
    "  JOIN " DGN_TABLE_View " AS V ON V.[Id]=G.ViewId"                   // by a view, where
    "  WHERE V.BaseModel IN (%s)"                                                // the view targets some model in pmodels
    " )", cols, pmodelsStr.c_str());

    return dviewsSmt.Prepare (m_project, sql.c_str());
    }