/*--------------------------------------------------------------------------------------+
|
|     $Source: SQLite/IntVirtualTable.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/BeSQLite.h>
#include "sqlite3.h"

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/13
//=======================================================================================
struct IntVTModule : sqlite3_vtab
{
    sqlite3*    m_db;
    int         m_nCursor;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/13
//=======================================================================================
struct IntVtCursor : sqlite3_vtab_cursor 
{
    IntVTModule*                m_module;
    IntVirtualTable*            m_table;
    IntVirtualTable::Iterator*  m_iter;

    void Clear();
    uint64_t GetValue() const {return m_iter->_GetValue();}
    uint64_t GetRowId() const {return m_iter->_GetRowId();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtDisconnect(sqlite3_vtab *pVtab)
    {
    IntVTModule *p = (IntVTModule*)pVtab;
    assert( p->m_nCursor==0 );
    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtConnect(sqlite3* db, void*, int argc, Utf8CP const* argv, sqlite3_vtab** ppVtab, char** pzErr)
    {
    IntVTModule* pNew = (IntVTModule*) sqlite3_malloc (sizeof(IntVTModule));
    memset(pNew, 0, sizeof(*pNew));

    pNew->m_db = db;
    int rc = sqlite3_declare_vtab(db,"CREATE TABLE x(id,address HIDDEN)");
    BeAssert (rc==SQLITE_OK);

    *ppVtab = pNew;
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtOpen (sqlite3_vtab* pVTab, sqlite3_vtab_cursor** ppCursor)
    {
    IntVtCursor* cursor = (IntVtCursor*) sqlite3_malloc(sizeof(IntVtCursor));
    memset(cursor, 0, sizeof(*cursor));

    cursor->m_module = (IntVTModule*) pVTab;
    cursor->m_module->m_nCursor++;

    *ppCursor = cursor;
    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void IntVtCursor::Clear()
    {
    if (m_iter)
        {
        m_table->_DeleteIterator (m_iter);
        m_iter = 0;
        }

    m_module->m_nCursor--;
    sqlite3_free(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtClose(sqlite3_vtab_cursor* cursor)
    {
    if (cursor)
        ((IntVtCursor*) cursor)->Clear();

    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtNext(sqlite3_vtab_cursor* cur)
    {
    IntVtCursor* cursor = (IntVtCursor*) cur;
    cursor->m_table->_Increment(*cursor->m_iter);
    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtFilter(sqlite3_vtab_cursor* pVtabCursor, int idxNum, Utf8CP idxStr, int argc, sqlite3_value **argv)
    {
    IntVtCursor *cursor = (IntVtCursor*) pVtabCursor;
#if defined (intVt_TESTING)
    IntVTModule *pVtab = pCur->pVtab;
  sqlite3_int64 iRoot;

  int mxGen = 999999999;  
  char *zSql;
  sqlite3_stmt *pStmt;
  closure_avl *pAvl;
  int rc = SQLITE_OK;
  Utf8CP zTableName = pVtab->zTableName;
  Utf8CP zIdColumn = pVtab->zIdColumn;
  Utf8CP zParentColumn = pVtab->zParentColumn;
  closure_queue sQueue;

  (void)idxStr;  /* Unused parameter */
  (void)argc;    /* Unused parameter */
  closureClearCursor(pCur);
  memset(&sQueue, 0, sizeof(sQueue));
  if( (idxNum & 1)==0 ){
    /* No root=$root in the WHERE clause.  Return an empty set */
    return SQLITE_OK;
  }
  iRoot = sqlite3_value_int64(argv[0]);
  if( (idxNum & 0x000f0)!=0 ){
    mxGen = sqlite3_value_int(argv[(idxNum>>4)&0x0f]);
    if( (idxNum & 0x00002)!=0 ) mxGen--;
  }
  if( (idxNum & 0x00f00)!=0 ){
    zTableName = (const char*)sqlite3_value_text(argv[(idxNum>>8)&0x0f]);
    pCur->zTableName = sqlite3_mprintf("%s", zTableName);
  }
  if( (idxNum & 0x0f000)!=0 ){
    zIdColumn = (const char*)sqlite3_value_text(argv[(idxNum>>12)&0x0f]);
    pCur->zIdColumn = sqlite3_mprintf("%s", zIdColumn);
  }
  if( (idxNum & 0x0f0000)!=0 ){
    zParentColumn = (const char*)sqlite3_value_text(argv[(idxNum>>16)&0x0f]);
    pCur->zParentColumn = sqlite3_mprintf("%s", zParentColumn);
  }

  zSql = sqlite3_mprintf(
       "SELECT \"%w\".\"%w\" FROM \"%w\" WHERE \"%w\".\"%w\"=?1",
       zTableName, zIdColumn, zTableName, zTableName, zParentColumn);
  if( zSql==0 ){
    return SQLITE_NOMEM;
  }else{
    rc = sqlite3_prepare_v2(pVtab->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
     if( rc ){
      sqlite3_free(pVtab->base.zErrMsg);
      pVtab->base.zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(pVtab->db));
      return rc;
    }
  }
  if( rc==SQLITE_OK ){
    rc = closureInsertNode(&sQueue, pCur, iRoot, 0);
  }
  while( (pAvl = queuePull(&sQueue))!=0 ){
    if( pAvl->iGeneration>=mxGen ) continue;
    sqlite3_bind_int64(pStmt, 1, pAvl->id);
    while( rc==SQLITE_OK && sqlite3_step(pStmt)==SQLITE_ROW ){
      if( sqlite3_column_type(pStmt,0)==SQLITE_INTEGER ){
        sqlite3_int64 iNew = sqlite3_column_int64(pStmt, 0);
        if( closureAvlSearch(pCur->pClosure, iNew)==0 ){
          rc = closureInsertNode(&sQueue, pCur, iNew, pAvl->iGeneration+1);
        }
      }
    }
    sqlite3_reset(pStmt);
  }
  sqlite3_finalize(pStmt);
  if( rc==SQLITE_OK ){
    pCur->pCurrent = closureAvlFirst(pCur->pClosure);
  }
#endif

  return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtColumn(sqlite3_vtab_cursor* cur, sqlite3_context* ctx, int i)
    {
    IntVtCursor* cursor = (IntVtCursor*)cur;
    switch (i)
        {
        case 1:  // the "id" column
            sqlite3_result_int64(ctx, cursor->GetValue());
            break;

        default:
            sqlite3_result_null(ctx);

        break;
        }

    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtRowid(sqlite3_vtab_cursor* cur, sqlite_int64* pRowid)
    {
    *pRowid = ((IntVtCursor*)cur)->GetRowId();
    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtEof(sqlite3_vtab_cursor* cur)
    {
    IntVtCursor* cursor = (IntVtCursor*)cur;
    return cursor->m_table->_IsAtEnd(*cursor->m_iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int intVtBestIndex (sqlite3_vtab* pTab, sqlite3_index_info* pIdxInfo)
    {
#if defined (intVt_TESTING)
  int iPlan = 0;
  int i;
  int idx = 1;
  int seenMatch = 0;
  const struct sqlite3_index_constraint *pConstraint;
  IntVTModule *pVtab = (IntVTModule*)pTab;
  double rCost = 10000000.0;

  pConstraint = pIdxInfo->aConstraint;
  for(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    if( pConstraint->iColumn==CLOSURE_COL_ROOT
     && pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      seenMatch = 1;
    }
    if( pConstraint->usable==0 ) continue;
    if( (iPlan & 1)==0
     && pConstraint->iColumn==CLOSURE_COL_ROOT
     && pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ
    ){
      iPlan |= 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      rCost /= 100.0;
    }
    if( (iPlan & 0x0000f0)==0
     && pConstraint->iColumn==CLOSURE_COL_DEPTH
     && (pConstraint->op==SQLITE_INDEX_CONSTRAINT_LT
           || pConstraint->op==SQLITE_INDEX_CONSTRAINT_LE
           || pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ)
    ){
      iPlan |= idx<<4;
      pIdxInfo->aConstraintUsage[i].argvIndex = ++idx;
      if( pConstraint->op==SQLITE_INDEX_CONSTRAINT_LT ) iPlan |= 0x000002;
      rCost /= 5.0;
    }
    if( (iPlan & 0x000f00)==0
     && pConstraint->iColumn==CLOSURE_COL_TABLENAME
     && pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ
    ){
      iPlan |= idx<<8;
      pIdxInfo->aConstraintUsage[i].argvIndex = ++idx;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      rCost /= 5.0;
    }
    if( (iPlan & 0x00f000)==0
     && pConstraint->iColumn==CLOSURE_COL_IDCOLUMN
     && pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ
    ){
      iPlan |= idx<<12;
      pIdxInfo->aConstraintUsage[i].argvIndex = ++idx;
      pIdxInfo->aConstraintUsage[i].omit = 1;
    }
    if( (iPlan & 0x0f0000)==0
     && pConstraint->iColumn==CLOSURE_COL_PARENTCOLUMN
     && pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ
    ){
      iPlan |= idx<<16;
      pIdxInfo->aConstraintUsage[i].argvIndex = ++idx;
      pIdxInfo->aConstraintUsage[i].omit = 1;
    }
  }
  if( (pVtab->zTableName==0    && (iPlan & 0x000f00)==0)
   || (pVtab->zIdColumn==0     && (iPlan & 0x00f000)==0)
   || (pVtab->zParentColumn==0 && (iPlan & 0x0f0000)==0)
  ){
    /* All of tablename, idcolumn, and parentcolumn must be specified
    ** in either the CREATE VIRTUAL TABLE or in the WHERE clause constraints
    ** or else the result is an empty set. */
    iPlan = 0;
  }
  pIdxInfo->idxNum = iPlan;
  if( pIdxInfo->nOrderBy==1
   && pIdxInfo->aOrderBy[0].iColumn==CLOSURE_COL_ID
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  if( seenMatch && (iPlan&1)==0 ) rCost *= 1e30;
  pIdxInfo->estimatedCost = rCost;
#endif

    pIdxInfo->estimatedCost = 10;
    return SQLITE_OK;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/13
//=======================================================================================
static sqlite3_module intVtVtModule =
{
    0,                      /* iVersion */
    intVtConnect,           /* xCreate */
    intVtConnect,           /* xConnect */
    intVtBestIndex,         /* xBestIndex */
    intVtDisconnect,        /* xDisconnect */
    intVtDisconnect,        /* xDestroy */
    intVtOpen,              /* xOpen - open a cursor */
    intVtClose,             /* xClose - close a cursor */
    intVtFilter,            /* xFilter - configure scan constraints */
    intVtNext,              /* xNext - advance a cursor */
    intVtEof,               /* xEof - check for end of scan */
    intVtColumn,            /* xColumn - read data */
    intVtRowid,             /* xRowid - read data */
    0,                      /* xUpdate */
    0,                      /* xBegin */
    0,                      /* xSync */
    0,                      /* xCommit */
    0,                      /* xRollback */
    0,                      /* xFindMethod */
    0,                      /* xRename */
    0,                      /* xSavepoint */
    0,                      /* xRelease */
    0                       /* xRollbackTo */
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" int init_intVirtualTable(sqlite3* db,  char**, void const*)
    {
    return sqlite3_create_module(db, "int_virtual", &intVtVtModule, 0);
    }
