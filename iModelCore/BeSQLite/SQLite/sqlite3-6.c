/************** Begin file fts3_write.c **************************************/
/*
** 2009 Oct 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file is part of the SQLite FTS3 extension module. Specifically,
** this file contains code to insert, update and delete rows from FTS3
** tables. It also contains code to merge FTS3 b-tree segments. Some
** of the sub-routines used to merge segments are also used by the query 
** code in fts3.c.
*/

/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <string.h> */
/* #include <assert.h> */
/* #include <stdlib.h> */


#define FTS_MAX_APPENDABLE_HEIGHT 16

/*
** When full-text index nodes are loaded from disk, the buffer that they
** are loaded into has the following number of bytes of padding at the end 
** of it. i.e. if a full-text index node is 900 bytes in size, then a buffer
** of 920 bytes is allocated for it.
**
** This means that if we have a pointer into a buffer containing node data,
** it is always safe to read up to two varints from it without risking an
** overread, even if the node data is corrupted.
*/
#define FTS3_NODE_PADDING (FTS3_VARINT_MAX*2)

/*
** Under certain circumstances, b-tree nodes (doclists) can be loaded into
** memory incrementally instead of all at once. This can be a big performance
** win (reduced IO and CPU) if SQLite stops calling the virtual table xNext()
** method before retrieving all query results (as may happen, for example,
** if a query has a LIMIT clause).
**
** Incremental loading is used for b-tree nodes FTS3_NODE_CHUNK_THRESHOLD 
** bytes and larger. Nodes are loaded in chunks of FTS3_NODE_CHUNKSIZE bytes.
** The code is written so that the hard lower-limit for each of these values 
** is 1. Clearly such small values would be inefficient, but can be useful 
** for testing purposes.
**
** If this module is built with SQLITE_TEST defined, these constants may
** be overridden at runtime for testing purposes. File fts3_test.c contains
** a Tcl interface to read and write the values.
*/
#ifdef SQLITE_TEST
int test_fts3_node_chunksize = (4*1024);
int test_fts3_node_chunk_threshold = (4*1024)*4;
# define FTS3_NODE_CHUNKSIZE       test_fts3_node_chunksize
# define FTS3_NODE_CHUNK_THRESHOLD test_fts3_node_chunk_threshold
#else
# define FTS3_NODE_CHUNKSIZE (4*1024) 
# define FTS3_NODE_CHUNK_THRESHOLD (FTS3_NODE_CHUNKSIZE*4)
#endif

/*
** The two values that may be meaningfully bound to the :1 parameter in
** statements SQL_REPLACE_STAT and SQL_SELECT_STAT.
*/
#define FTS_STAT_DOCTOTAL      0
#define FTS_STAT_INCRMERGEHINT 1
#define FTS_STAT_AUTOINCRMERGE 2

/*
** If FTS_LOG_MERGES is defined, call sqlite3_log() to report each automatic
** and incremental merge operation that takes place. This is used for 
** debugging FTS only, it should not usually be turned on in production
** systems.
*/
#ifdef FTS3_LOG_MERGES
static void fts3LogMerge(int nMerge, sqlite3_int64 iAbsLevel){
  sqlite3_log(SQLITE_OK, "%d-way merge from level %d", nMerge, (int)iAbsLevel);
}
#else
#define fts3LogMerge(x, y)
#endif


typedef struct PendingList PendingList;
typedef struct SegmentNode SegmentNode;
typedef struct SegmentWriter SegmentWriter;

/*
** An instance of the following data structure is used to build doclists
** incrementally. See function fts3PendingListAppend() for details.
*/
struct PendingList {
  int nData;
  char *aData;
  int nSpace;
  sqlite3_int64 iLastDocid;
  sqlite3_int64 iLastCol;
  sqlite3_int64 iLastPos;
};


/*
** Each cursor has a (possibly empty) linked list of the following objects.
*/
struct Fts3DeferredToken {
  Fts3PhraseToken *pToken;        /* Pointer to corresponding expr token */
  int iCol;                       /* Column token must occur in */
  Fts3DeferredToken *pNext;       /* Next in list of deferred tokens */
  PendingList *pList;             /* Doclist is assembled here */
};

/*
** An instance of this structure is used to iterate through the terms on
** a contiguous set of segment b-tree leaf nodes. Although the details of
** this structure are only manipulated by code in this file, opaque handles
** of type Fts3SegReader* are also used by code in fts3.c to iterate through
** terms when querying the full-text index. See functions:
**
**   sqlite3Fts3SegReaderNew()
**   sqlite3Fts3SegReaderFree()
**   sqlite3Fts3SegReaderIterate()
**
** Methods used to manipulate Fts3SegReader structures:
**
**   fts3SegReaderNext()
**   fts3SegReaderFirstDocid()
**   fts3SegReaderNextDocid()
*/
struct Fts3SegReader {
  int iIdx;                       /* Index within level, or 0x7FFFFFFF for PT */
  u8 bLookup;                     /* True for a lookup only */
  u8 rootOnly;                    /* True for a root-only reader */

  sqlite3_int64 iStartBlock;      /* Rowid of first leaf block to traverse */
  sqlite3_int64 iLeafEndBlock;    /* Rowid of final leaf block to traverse */
  sqlite3_int64 iEndBlock;        /* Rowid of final block in segment (or 0) */
  sqlite3_int64 iCurrentBlock;    /* Current leaf block (or 0) */

  char *aNode;                    /* Pointer to node data (or NULL) */
  int nNode;                      /* Size of buffer at aNode (or 0) */
  int nPopulate;                  /* If >0, bytes of buffer aNode[] loaded */
  sqlite3_blob *pBlob;            /* If not NULL, blob handle to read node */

  Fts3HashElem **ppNextElem;

  /* Variables set by fts3SegReaderNext(). These may be read directly
  ** by the caller. They are valid from the time SegmentReaderNew() returns
  ** until SegmentReaderNext() returns something other than SQLITE_OK
  ** (i.e. SQLITE_DONE).
  */
  int nTerm;                      /* Number of bytes in current term */
  char *zTerm;                    /* Pointer to current term */
  int nTermAlloc;                 /* Allocated size of zTerm buffer */
  char *aDoclist;                 /* Pointer to doclist of current entry */
  int nDoclist;                   /* Size of doclist in current entry */

  /* The following variables are used by fts3SegReaderNextDocid() to iterate 
  ** through the current doclist (aDoclist/nDoclist).
  */
  char *pOffsetList;
  int nOffsetList;                /* For descending pending seg-readers only */
  sqlite3_int64 iDocid;
};

#define fts3SegReaderIsPending(p) ((p)->ppNextElem!=0)
#define fts3SegReaderIsRootOnly(p) ((p)->rootOnly!=0)

/*
** An instance of this structure is used to create a segment b-tree in the
** database. The internal details of this type are only accessed by the
** following functions:
**
**   fts3SegWriterAdd()
**   fts3SegWriterFlush()
**   fts3SegWriterFree()
*/
struct SegmentWriter {
  SegmentNode *pTree;             /* Pointer to interior tree structure */
  sqlite3_int64 iFirst;           /* First slot in %_segments written */
  sqlite3_int64 iFree;            /* Next free slot in %_segments */
  char *zTerm;                    /* Pointer to previous term buffer */
  int nTerm;                      /* Number of bytes in zTerm */
  int nMalloc;                    /* Size of malloc'd buffer at zMalloc */
  char *zMalloc;                  /* Malloc'd space (possibly) used for zTerm */
  int nSize;                      /* Size of allocation at aData */
  int nData;                      /* Bytes of data in aData */
  char *aData;                    /* Pointer to block from malloc() */
  i64 nLeafData;                  /* Number of bytes of leaf data written */
};

/*
** Type SegmentNode is used by the following three functions to create
** the interior part of the segment b+-tree structures (everything except
** the leaf nodes). These functions and type are only ever used by code
** within the fts3SegWriterXXX() family of functions described above.
**
**   fts3NodeAddTerm()
**   fts3NodeWrite()
**   fts3NodeFree()
**
** When a b+tree is written to the database (either as a result of a merge
** or the pending-terms table being flushed), leaves are written into the 
** database file as soon as they are completely populated. The interior of
** the tree is assembled in memory and written out only once all leaves have
** been populated and stored. This is Ok, as the b+-tree fanout is usually
** very large, meaning that the interior of the tree consumes relatively 
** little memory.
*/
struct SegmentNode {
  SegmentNode *pParent;           /* Parent node (or NULL for root node) */
  SegmentNode *pRight;            /* Pointer to right-sibling */
  SegmentNode *pLeftmost;         /* Pointer to left-most node of this depth */
  int nEntry;                     /* Number of terms written to node so far */
  char *zTerm;                    /* Pointer to previous term buffer */
  int nTerm;                      /* Number of bytes in zTerm */
  int nMalloc;                    /* Size of malloc'd buffer at zMalloc */
  char *zMalloc;                  /* Malloc'd space (possibly) used for zTerm */
  int nData;                      /* Bytes of valid data so far */
  char *aData;                    /* Node data */
};

/*
** Valid values for the second argument to fts3SqlStmt().
*/
#define SQL_DELETE_CONTENT             0
#define SQL_IS_EMPTY                   1
#define SQL_DELETE_ALL_CONTENT         2 
#define SQL_DELETE_ALL_SEGMENTS        3
#define SQL_DELETE_ALL_SEGDIR          4
#define SQL_DELETE_ALL_DOCSIZE         5
#define SQL_DELETE_ALL_STAT            6
#define SQL_SELECT_CONTENT_BY_ROWID    7
#define SQL_NEXT_SEGMENT_INDEX         8
#define SQL_INSERT_SEGMENTS            9
#define SQL_NEXT_SEGMENTS_ID          10
#define SQL_INSERT_SEGDIR             11
#define SQL_SELECT_LEVEL              12
#define SQL_SELECT_LEVEL_RANGE        13
#define SQL_SELECT_LEVEL_COUNT        14
#define SQL_SELECT_SEGDIR_MAX_LEVEL   15
#define SQL_DELETE_SEGDIR_LEVEL       16
#define SQL_DELETE_SEGMENTS_RANGE     17
#define SQL_CONTENT_INSERT            18
#define SQL_DELETE_DOCSIZE            19
#define SQL_REPLACE_DOCSIZE           20
#define SQL_SELECT_DOCSIZE            21
#define SQL_SELECT_STAT               22
#define SQL_REPLACE_STAT              23

#define SQL_SELECT_ALL_PREFIX_LEVEL   24
#define SQL_DELETE_ALL_TERMS_SEGDIR   25
#define SQL_DELETE_SEGDIR_RANGE       26
#define SQL_SELECT_ALL_LANGID         27
#define SQL_FIND_MERGE_LEVEL          28
#define SQL_MAX_LEAF_NODE_ESTIMATE    29
#define SQL_DELETE_SEGDIR_ENTRY       30
#define SQL_SHIFT_SEGDIR_ENTRY        31
#define SQL_SELECT_SEGDIR             32
#define SQL_CHOMP_SEGDIR              33
#define SQL_SEGMENT_IS_APPENDABLE     34
#define SQL_SELECT_INDEXES            35
#define SQL_SELECT_MXLEVEL            36

#define SQL_SELECT_LEVEL_RANGE2       37
#define SQL_UPDATE_LEVEL_IDX          38
#define SQL_UPDATE_LEVEL              39

/*
** This function is used to obtain an SQLite prepared statement handle
** for the statement identified by the second argument. If successful,
** *pp is set to the requested statement handle and SQLITE_OK returned.
** Otherwise, an SQLite error code is returned and *pp is set to 0.
**
** If argument apVal is not NULL, then it must point to an array with
** at least as many entries as the requested statement has bound 
** parameters. The values are bound to the statements parameters before
** returning.
*/
static int fts3SqlStmt(
  Fts3Table *p,                   /* Virtual table handle */
  int eStmt,                      /* One of the SQL_XXX constants above */
  sqlite3_stmt **pp,              /* OUT: Statement handle */
  sqlite3_value **apVal           /* Values to bind to statement */
){
  const char *azSql[] = {
/* 0  */  "DELETE FROM %Q.'%q_content' WHERE rowid = ?",
/* 1  */  "SELECT NOT EXISTS(SELECT docid FROM %Q.'%q_content' WHERE rowid!=?)",
/* 2  */  "DELETE FROM %Q.'%q_content'",
/* 3  */  "DELETE FROM %Q.'%q_segments'",
/* 4  */  "DELETE FROM %Q.'%q_segdir'",
/* 5  */  "DELETE FROM %Q.'%q_docsize'",
/* 6  */  "DELETE FROM %Q.'%q_stat'",
/* 7  */  "SELECT %s WHERE rowid=?",
/* 8  */  "SELECT (SELECT max(idx) FROM %Q.'%q_segdir' WHERE level = ?) + 1",
/* 9  */  "REPLACE INTO %Q.'%q_segments'(blockid, block) VALUES(?, ?)",
/* 10 */  "SELECT coalesce((SELECT max(blockid) FROM %Q.'%q_segments') + 1, 1)",
/* 11 */  "REPLACE INTO %Q.'%q_segdir' VALUES(?,?,?,?,?,?)",

          /* Return segments in order from oldest to newest.*/ 
/* 12 */  "SELECT idx, start_block, leaves_end_block, end_block, root "
            "FROM %Q.'%q_segdir' WHERE level = ? ORDER BY idx ASC",
/* 13 */  "SELECT idx, start_block, leaves_end_block, end_block, root "
            "FROM %Q.'%q_segdir' WHERE level BETWEEN ? AND ?"
            "ORDER BY level DESC, idx ASC",

/* 14 */  "SELECT count(*) FROM %Q.'%q_segdir' WHERE level = ?",
/* 15 */  "SELECT max(level) FROM %Q.'%q_segdir' WHERE level BETWEEN ? AND ?",

/* 16 */  "DELETE FROM %Q.'%q_segdir' WHERE level = ?",
/* 17 */  "DELETE FROM %Q.'%q_segments' WHERE blockid BETWEEN ? AND ?",
/* 18 */  "INSERT INTO %Q.'%q_content' VALUES(%s)",
/* 19 */  "DELETE FROM %Q.'%q_docsize' WHERE docid = ?",
/* 20 */  "REPLACE INTO %Q.'%q_docsize' VALUES(?,?)",
/* 21 */  "SELECT size FROM %Q.'%q_docsize' WHERE docid=?",
/* 22 */  "SELECT value FROM %Q.'%q_stat' WHERE id=?",
/* 23 */  "REPLACE INTO %Q.'%q_stat' VALUES(?,?)",
/* 24 */  "",
/* 25 */  "",

/* 26 */ "DELETE FROM %Q.'%q_segdir' WHERE level BETWEEN ? AND ?",
/* 27 */ "SELECT ? UNION SELECT level / (1024 * ?) FROM %Q.'%q_segdir'",

/* This statement is used to determine which level to read the input from
** when performing an incremental merge. It returns the absolute level number
** of the oldest level in the db that contains at least ? segments. Or,
** if no level in the FTS index contains more than ? segments, the statement
** returns zero rows.  */
/* 28 */ "SELECT level FROM %Q.'%q_segdir' GROUP BY level HAVING count(*)>=?"
         "  ORDER BY (level %% 1024) ASC LIMIT 1",

/* Estimate the upper limit on the number of leaf nodes in a new segment
** created by merging the oldest :2 segments from absolute level :1. See 
** function sqlite3Fts3Incrmerge() for details.  */
/* 29 */ "SELECT 2 * total(1 + leaves_end_block - start_block) "
         "  FROM %Q.'%q_segdir' WHERE level = ? AND idx < ?",

/* SQL_DELETE_SEGDIR_ENTRY
**   Delete the %_segdir entry on absolute level :1 with index :2.  */
/* 30 */ "DELETE FROM %Q.'%q_segdir' WHERE level = ? AND idx = ?",

/* SQL_SHIFT_SEGDIR_ENTRY
**   Modify the idx value for the segment with idx=:3 on absolute level :2
**   to :1.  */
/* 31 */ "UPDATE %Q.'%q_segdir' SET idx = ? WHERE level=? AND idx=?",

/* SQL_SELECT_SEGDIR
**   Read a single entry from the %_segdir table. The entry from absolute 
**   level :1 with index value :2.  */
/* 32 */  "SELECT idx, start_block, leaves_end_block, end_block, root "
            "FROM %Q.'%q_segdir' WHERE level = ? AND idx = ?",

/* SQL_CHOMP_SEGDIR
**   Update the start_block (:1) and root (:2) fields of the %_segdir
**   entry located on absolute level :3 with index :4.  */
/* 33 */  "UPDATE %Q.'%q_segdir' SET start_block = ?, root = ?"
            "WHERE level = ? AND idx = ?",

/* SQL_SEGMENT_IS_APPENDABLE
**   Return a single row if the segment with end_block=? is appendable. Or
**   no rows otherwise.  */
/* 34 */  "SELECT 1 FROM %Q.'%q_segments' WHERE blockid=? AND block IS NULL",

/* SQL_SELECT_INDEXES
**   Return the list of valid segment indexes for absolute level ?  */
/* 35 */  "SELECT idx FROM %Q.'%q_segdir' WHERE level=? ORDER BY 1 ASC",

/* SQL_SELECT_MXLEVEL
**   Return the largest relative level in the FTS index or indexes.  */
/* 36 */  "SELECT max( level %% 1024 ) FROM %Q.'%q_segdir'",

          /* Return segments in order from oldest to newest.*/ 
/* 37 */  "SELECT level, idx, end_block "
            "FROM %Q.'%q_segdir' WHERE level BETWEEN ? AND ? "
            "ORDER BY level DESC, idx ASC",

          /* Update statements used while promoting segments */
/* 38 */  "UPDATE OR FAIL %Q.'%q_segdir' SET level=-1,idx=? "
            "WHERE level=? AND idx=?",
/* 39 */  "UPDATE OR FAIL %Q.'%q_segdir' SET level=? WHERE level=-1"

  };
  int rc = SQLITE_OK;
  sqlite3_stmt *pStmt;

  assert( SizeofArray(azSql)==SizeofArray(p->aStmt) );
  assert( eStmt<SizeofArray(azSql) && eStmt>=0 );
  
  pStmt = p->aStmt[eStmt];
  if( !pStmt ){
    char *zSql;
    if( eStmt==SQL_CONTENT_INSERT ){
      zSql = sqlite3_mprintf(azSql[eStmt], p->zDb, p->zName, p->zWriteExprlist);
    }else if( eStmt==SQL_SELECT_CONTENT_BY_ROWID ){
      zSql = sqlite3_mprintf(azSql[eStmt], p->zReadExprlist);
    }else{
      zSql = sqlite3_mprintf(azSql[eStmt], p->zDb, p->zName);
    }
    if( !zSql ){
      rc = SQLITE_NOMEM;
    }else{
      rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, NULL);
      sqlite3_free(zSql);
      assert( rc==SQLITE_OK || pStmt==0 );
      p->aStmt[eStmt] = pStmt;
    }
  }
  if( apVal ){
    int i;
    int nParam = sqlite3_bind_parameter_count(pStmt);
    for(i=0; rc==SQLITE_OK && i<nParam; i++){
      rc = sqlite3_bind_value(pStmt, i+1, apVal[i]);
    }
  }
  *pp = pStmt;
  return rc;
}


static int fts3SelectDocsize(
  Fts3Table *pTab,                /* FTS3 table handle */
  sqlite3_int64 iDocid,           /* Docid to bind for SQL_SELECT_DOCSIZE */
  sqlite3_stmt **ppStmt           /* OUT: Statement handle */
){
  sqlite3_stmt *pStmt = 0;        /* Statement requested from fts3SqlStmt() */
  int rc;                         /* Return code */

  rc = fts3SqlStmt(pTab, SQL_SELECT_DOCSIZE, &pStmt, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pStmt, 1, iDocid);
    rc = sqlite3_step(pStmt);
    if( rc!=SQLITE_ROW || sqlite3_column_type(pStmt, 0)!=SQLITE_BLOB ){
      rc = sqlite3_reset(pStmt);
      if( rc==SQLITE_OK ) rc = FTS_CORRUPT_VTAB;
      pStmt = 0;
    }else{
      rc = SQLITE_OK;
    }
  }

  *ppStmt = pStmt;
  return rc;
}

SQLITE_PRIVATE int sqlite3Fts3SelectDoctotal(
  Fts3Table *pTab,                /* Fts3 table handle */
  sqlite3_stmt **ppStmt           /* OUT: Statement handle */
){
  sqlite3_stmt *pStmt = 0;
  int rc;
  rc = fts3SqlStmt(pTab, SQL_SELECT_STAT, &pStmt, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int(pStmt, 1, FTS_STAT_DOCTOTAL);
    if( sqlite3_step(pStmt)!=SQLITE_ROW
     || sqlite3_column_type(pStmt, 0)!=SQLITE_BLOB
    ){
      rc = sqlite3_reset(pStmt);
      if( rc==SQLITE_OK ) rc = FTS_CORRUPT_VTAB;
      pStmt = 0;
    }
  }
  *ppStmt = pStmt;
  return rc;
}

SQLITE_PRIVATE int sqlite3Fts3SelectDocsize(
  Fts3Table *pTab,                /* Fts3 table handle */
  sqlite3_int64 iDocid,           /* Docid to read size data for */
  sqlite3_stmt **ppStmt           /* OUT: Statement handle */
){
  return fts3SelectDocsize(pTab, iDocid, ppStmt);
}

/*
** Similar to fts3SqlStmt(). Except, after binding the parameters in
** array apVal[] to the SQL statement identified by eStmt, the statement
** is executed.
**
** Returns SQLITE_OK if the statement is successfully executed, or an
** SQLite error code otherwise.
*/
static void fts3SqlExec(
  int *pRC,                /* Result code */
  Fts3Table *p,            /* The FTS3 table */
  int eStmt,               /* Index of statement to evaluate */
  sqlite3_value **apVal    /* Parameters to bind */
){
  sqlite3_stmt *pStmt;
  int rc;
  if( *pRC ) return;
  rc = fts3SqlStmt(p, eStmt, &pStmt, apVal); 
  if( rc==SQLITE_OK ){
    sqlite3_step(pStmt);
    rc = sqlite3_reset(pStmt);
  }
  *pRC = rc;
}


/*
** This function ensures that the caller has obtained an exclusive 
** shared-cache table-lock on the %_segdir table. This is required before 
** writing data to the fts3 table. If this lock is not acquired first, then
** the caller may end up attempting to take this lock as part of committing
** a transaction, causing SQLite to return SQLITE_LOCKED or 
** LOCKED_SHAREDCACHEto a COMMIT command.
**
** It is best to avoid this because if FTS3 returns any error when 
** committing a transaction, the whole transaction will be rolled back. 
** And this is not what users expect when they get SQLITE_LOCKED_SHAREDCACHE. 
** It can still happen if the user locks the underlying tables directly 
** instead of accessing them via FTS.
*/
static int fts3Writelock(Fts3Table *p){
  int rc = SQLITE_OK;
  
  if( p->nPendingData==0 ){
    sqlite3_stmt *pStmt;
    rc = fts3SqlStmt(p, SQL_DELETE_SEGDIR_LEVEL, &pStmt, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_null(pStmt, 1);
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
    }
  }

  return rc;
}

/*
** FTS maintains a separate indexes for each language-id (a 32-bit integer).
** Within each language id, a separate index is maintained to store the
** document terms, and each configured prefix size (configured the FTS 
** "prefix=" option). And each index consists of multiple levels ("relative
** levels").
**
** All three of these values (the language id, the specific index and the
** level within the index) are encoded in 64-bit integer values stored
** in the %_segdir table on disk. This function is used to convert three
** separate component values into the single 64-bit integer value that
** can be used to query the %_segdir table.
**
** Specifically, each language-id/index combination is allocated 1024 
** 64-bit integer level values ("absolute levels"). The main terms index
** for language-id 0 is allocate values 0-1023. The first prefix index
** (if any) for language-id 0 is allocated values 1024-2047. And so on.
** Language 1 indexes are allocated immediately following language 0.
**
** So, for a system with nPrefix prefix indexes configured, the block of
** absolute levels that corresponds to language-id iLangid and index 
** iIndex starts at absolute level ((iLangid * (nPrefix+1) + iIndex) * 1024).
*/
static sqlite3_int64 getAbsoluteLevel(
  Fts3Table *p,                   /* FTS3 table handle */
  int iLangid,                    /* Language id */
  int iIndex,                     /* Index in p->aIndex[] */
  int iLevel                      /* Level of segments */
){
  sqlite3_int64 iBase;            /* First absolute level for iLangid/iIndex */
  assert( iLangid>=0 );
  assert( p->nIndex>0 );
  assert( iIndex>=0 && iIndex<p->nIndex );

  iBase = ((sqlite3_int64)iLangid * p->nIndex + iIndex) * FTS3_SEGDIR_MAXLEVEL;
  return iBase + iLevel;
}

/*
** Set *ppStmt to a statement handle that may be used to iterate through
** all rows in the %_segdir table, from oldest to newest. If successful,
** return SQLITE_OK. If an error occurs while preparing the statement, 
** return an SQLite error code.
**
** There is only ever one instance of this SQL statement compiled for
** each FTS3 table.
**
** The statement returns the following columns from the %_segdir table:
**
**   0: idx
**   1: start_block
**   2: leaves_end_block
**   3: end_block
**   4: root
*/
SQLITE_PRIVATE int sqlite3Fts3AllSegdirs(
  Fts3Table *p,                   /* FTS3 table */
  int iLangid,                    /* Language being queried */
  int iIndex,                     /* Index for p->aIndex[] */
  int iLevel,                     /* Level to select (relative level) */
  sqlite3_stmt **ppStmt           /* OUT: Compiled statement */
){
  int rc;
  sqlite3_stmt *pStmt = 0;

  assert( iLevel==FTS3_SEGCURSOR_ALL || iLevel>=0 );
  assert( iLevel<FTS3_SEGDIR_MAXLEVEL );
  assert( iIndex>=0 && iIndex<p->nIndex );

  if( iLevel<0 ){
    /* "SELECT * FROM %_segdir WHERE level BETWEEN ? AND ? ORDER BY ..." */
    rc = fts3SqlStmt(p, SQL_SELECT_LEVEL_RANGE, &pStmt, 0);
    if( rc==SQLITE_OK ){ 
      sqlite3_bind_int64(pStmt, 1, getAbsoluteLevel(p, iLangid, iIndex, 0));
      sqlite3_bind_int64(pStmt, 2, 
          getAbsoluteLevel(p, iLangid, iIndex, FTS3_SEGDIR_MAXLEVEL-1)
      );
    }
  }else{
    /* "SELECT * FROM %_segdir WHERE level = ? ORDER BY ..." */
    rc = fts3SqlStmt(p, SQL_SELECT_LEVEL, &pStmt, 0);
    if( rc==SQLITE_OK ){ 
      sqlite3_bind_int64(pStmt, 1, getAbsoluteLevel(p, iLangid, iIndex,iLevel));
    }
  }
  *ppStmt = pStmt;
  return rc;
}


/*
** Append a single varint to a PendingList buffer. SQLITE_OK is returned
** if successful, or an SQLite error code otherwise.
**
** This function also serves to allocate the PendingList structure itself.
** For example, to create a new PendingList structure containing two
** varints:
**
**   PendingList *p = 0;
**   fts3PendingListAppendVarint(&p, 1);
**   fts3PendingListAppendVarint(&p, 2);
*/
static int fts3PendingListAppendVarint(
  PendingList **pp,               /* IN/OUT: Pointer to PendingList struct */
  sqlite3_int64 i                 /* Value to append to data */
){
  PendingList *p = *pp;

  /* Allocate or grow the PendingList as required. */
  if( !p ){
    p = sqlite3_malloc(sizeof(*p) + 100);
    if( !p ){
      return SQLITE_NOMEM;
    }
    p->nSpace = 100;
    p->aData = (char *)&p[1];
    p->nData = 0;
  }
  else if( p->nData+FTS3_VARINT_MAX+1>p->nSpace ){
    int nNew = p->nSpace * 2;
    p = sqlite3_realloc(p, sizeof(*p) + nNew);
    if( !p ){
      sqlite3_free(*pp);
      *pp = 0;
      return SQLITE_NOMEM;
    }
    p->nSpace = nNew;
    p->aData = (char *)&p[1];
  }

  /* Append the new serialized varint to the end of the list. */
  p->nData += sqlite3Fts3PutVarint(&p->aData[p->nData], i);
  p->aData[p->nData] = '\0';
  *pp = p;
  return SQLITE_OK;
}

/*
** Add a docid/column/position entry to a PendingList structure. Non-zero
** is returned if the structure is sqlite3_realloced as part of adding
** the entry. Otherwise, zero.
**
** If an OOM error occurs, *pRc is set to SQLITE_NOMEM before returning.
** Zero is always returned in this case. Otherwise, if no OOM error occurs,
** it is set to SQLITE_OK.
*/
static int fts3PendingListAppend(
  PendingList **pp,               /* IN/OUT: PendingList structure */
  sqlite3_int64 iDocid,           /* Docid for entry to add */
  sqlite3_int64 iCol,             /* Column for entry to add */
  sqlite3_int64 iPos,             /* Position of term for entry to add */
  int *pRc                        /* OUT: Return code */
){
  PendingList *p = *pp;
  int rc = SQLITE_OK;

  assert( !p || p->iLastDocid<=iDocid );

  if( !p || p->iLastDocid!=iDocid ){
    sqlite3_int64 iDelta = iDocid - (p ? p->iLastDocid : 0);
    if( p ){
      assert( p->nData<p->nSpace );
      assert( p->aData[p->nData]==0 );
      p->nData++;
    }
    if( SQLITE_OK!=(rc = fts3PendingListAppendVarint(&p, iDelta)) ){
      goto pendinglistappend_out;
    }
    p->iLastCol = -1;
    p->iLastPos = 0;
    p->iLastDocid = iDocid;
  }
  if( iCol>0 && p->iLastCol!=iCol ){
    if( SQLITE_OK!=(rc = fts3PendingListAppendVarint(&p, 1))
     || SQLITE_OK!=(rc = fts3PendingListAppendVarint(&p, iCol))
    ){
      goto pendinglistappend_out;
    }
    p->iLastCol = iCol;
    p->iLastPos = 0;
  }
  if( iCol>=0 ){
    assert( iPos>p->iLastPos || (iPos==0 && p->iLastPos==0) );
    rc = fts3PendingListAppendVarint(&p, 2+iPos-p->iLastPos);
    if( rc==SQLITE_OK ){
      p->iLastPos = iPos;
    }
  }

 pendinglistappend_out:
  *pRc = rc;
  if( p!=*pp ){
    *pp = p;
    return 1;
  }
  return 0;
}

/*
** Free a PendingList object allocated by fts3PendingListAppend().
*/
static void fts3PendingListDelete(PendingList *pList){
  sqlite3_free(pList);
}

/*
** Add an entry to one of the pending-terms hash tables.
*/
static int fts3PendingTermsAddOne(
  Fts3Table *p,
  int iCol,
  int iPos,
  Fts3Hash *pHash,                /* Pending terms hash table to add entry to */
  const char *zToken,
  int nToken
){
  PendingList *pList;
  int rc = SQLITE_OK;

  pList = (PendingList *)fts3HashFind(pHash, zToken, nToken);
  if( pList ){
    p->nPendingData -= (pList->nData + nToken + sizeof(Fts3HashElem));
  }
  if( fts3PendingListAppend(&pList, p->iPrevDocid, iCol, iPos, &rc) ){
    if( pList==fts3HashInsert(pHash, zToken, nToken, pList) ){
      /* Malloc failed while inserting the new entry. This can only 
      ** happen if there was no previous entry for this token.
      */
      assert( 0==fts3HashFind(pHash, zToken, nToken) );
      sqlite3_free(pList);
      rc = SQLITE_NOMEM;
    }
  }
  if( rc==SQLITE_OK ){
    p->nPendingData += (pList->nData + nToken + sizeof(Fts3HashElem));
  }
  return rc;
}

/*
** Tokenize the nul-terminated string zText and add all tokens to the
** pending-terms hash-table. The docid used is that currently stored in
** p->iPrevDocid, and the column is specified by argument iCol.
**
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code.
*/
static int fts3PendingTermsAdd(
  Fts3Table *p,                   /* Table into which text will be inserted */
  int iLangid,                    /* Language id to use */
  const char *zText,              /* Text of document to be inserted */
  int iCol,                       /* Column into which text is being inserted */
  u32 *pnWord                     /* IN/OUT: Incr. by number tokens inserted */
){
  int rc;
  int iStart = 0;
  int iEnd = 0;
  int iPos = 0;
  int nWord = 0;

  char const *zToken;
  int nToken = 0;

  sqlite3_tokenizer *pTokenizer = p->pTokenizer;
  sqlite3_tokenizer_module const *pModule = pTokenizer->pModule;
  sqlite3_tokenizer_cursor *pCsr;
  int (*xNext)(sqlite3_tokenizer_cursor *pCursor,
      const char**,int*,int*,int*,int*);

  assert( pTokenizer && pModule );

  /* If the user has inserted a NULL value, this function may be called with
  ** zText==0. In this case, add zero token entries to the hash table and 
  ** return early. */
  if( zText==0 ){
    *pnWord = 0;
    return SQLITE_OK;
  }

  rc = sqlite3Fts3OpenTokenizer(pTokenizer, iLangid, zText, -1, &pCsr);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  xNext = pModule->xNext;
  while( SQLITE_OK==rc
      && SQLITE_OK==(rc = xNext(pCsr, &zToken, &nToken, &iStart, &iEnd, &iPos))
  ){
    int i;
    if( iPos>=nWord ) nWord = iPos+1;

    /* Positions cannot be negative; we use -1 as a terminator internally.
    ** Tokens must have a non-zero length.
    */
    if( iPos<0 || !zToken || nToken<=0 ){
      rc = SQLITE_ERROR;
      break;
    }

    /* Add the term to the terms index */
    rc = fts3PendingTermsAddOne(
        p, iCol, iPos, &p->aIndex[0].hPending, zToken, nToken
    );
    
    /* Add the term to each of the prefix indexes that it is not too 
    ** short for. */
    for(i=1; rc==SQLITE_OK && i<p->nIndex; i++){
      struct Fts3Index *pIndex = &p->aIndex[i];
      if( nToken<pIndex->nPrefix ) continue;
      rc = fts3PendingTermsAddOne(
          p, iCol, iPos, &pIndex->hPending, zToken, pIndex->nPrefix
      );
    }
  }

  pModule->xClose(pCsr);
  *pnWord += nWord;
  return (rc==SQLITE_DONE ? SQLITE_OK : rc);
}

/* 
** Calling this function indicates that subsequent calls to 
** fts3PendingTermsAdd() are to add term/position-list pairs for the
** contents of the document with docid iDocid.
*/
static int fts3PendingTermsDocid(
  Fts3Table *p,                   /* Full-text table handle */
  int iLangid,                    /* Language id of row being written */
  sqlite_int64 iDocid             /* Docid of row being written */
){
  assert( iLangid>=0 );

  /* TODO(shess) Explore whether partially flushing the buffer on
  ** forced-flush would provide better performance.  I suspect that if
  ** we ordered the doclists by size and flushed the largest until the
  ** buffer was half empty, that would let the less frequent terms
  ** generate longer doclists.
  */
  if( iDocid<=p->iPrevDocid 
   || p->iPrevLangid!=iLangid
   || p->nPendingData>p->nMaxPendingData 
  ){
    int rc = sqlite3Fts3PendingTermsFlush(p);
    if( rc!=SQLITE_OK ) return rc;
  }
  p->iPrevDocid = iDocid;
  p->iPrevLangid = iLangid;
  return SQLITE_OK;
}

/*
** Discard the contents of the pending-terms hash tables. 
*/
SQLITE_PRIVATE void sqlite3Fts3PendingTermsClear(Fts3Table *p){
  int i;
  for(i=0; i<p->nIndex; i++){
    Fts3HashElem *pElem;
    Fts3Hash *pHash = &p->aIndex[i].hPending;
    for(pElem=fts3HashFirst(pHash); pElem; pElem=fts3HashNext(pElem)){
      PendingList *pList = (PendingList *)fts3HashData(pElem);
      fts3PendingListDelete(pList);
    }
    fts3HashClear(pHash);
  }
  p->nPendingData = 0;
}

/*
** This function is called by the xUpdate() method as part of an INSERT
** operation. It adds entries for each term in the new record to the
** pendingTerms hash table.
**
** Argument apVal is the same as the similarly named argument passed to
** fts3InsertData(). Parameter iDocid is the docid of the new row.
*/
static int fts3InsertTerms(
  Fts3Table *p, 
  int iLangid, 
  sqlite3_value **apVal, 
  u32 *aSz
){
  int i;                          /* Iterator variable */
  for(i=2; i<p->nColumn+2; i++){
    int iCol = i-2;
    if( p->abNotindexed[iCol]==0 ){
      const char *zText = (const char *)sqlite3_value_text(apVal[i]);
      int rc = fts3PendingTermsAdd(p, iLangid, zText, iCol, &aSz[iCol]);
      if( rc!=SQLITE_OK ){
        return rc;
      }
      aSz[p->nColumn] += sqlite3_value_bytes(apVal[i]);
    }
  }
  return SQLITE_OK;
}

/*
** This function is called by the xUpdate() method for an INSERT operation.
** The apVal parameter is passed a copy of the apVal argument passed by
** SQLite to the xUpdate() method. i.e:
**
**   apVal[0]                Not used for INSERT.
**   apVal[1]                rowid
**   apVal[2]                Left-most user-defined column
**   ...
**   apVal[p->nColumn+1]     Right-most user-defined column
**   apVal[p->nColumn+2]     Hidden column with same name as table
**   apVal[p->nColumn+3]     Hidden "docid" column (alias for rowid)
**   apVal[p->nColumn+4]     Hidden languageid column
*/
static int fts3InsertData(
  Fts3Table *p,                   /* Full-text table */
  sqlite3_value **apVal,          /* Array of values to insert */
  sqlite3_int64 *piDocid          /* OUT: Docid for row just inserted */
){
  int rc;                         /* Return code */
  sqlite3_stmt *pContentInsert;   /* INSERT INTO %_content VALUES(...) */

  if( p->zContentTbl ){
    sqlite3_value *pRowid = apVal[p->nColumn+3];
    if( sqlite3_value_type(pRowid)==SQLITE_NULL ){
      pRowid = apVal[1];
    }
    if( sqlite3_value_type(pRowid)!=SQLITE_INTEGER ){
      return SQLITE_CONSTRAINT;
    }
    *piDocid = sqlite3_value_int64(pRowid);
    return SQLITE_OK;
  }

  /* Locate the statement handle used to insert data into the %_content
  ** table. The SQL for this statement is:
  **
  **   INSERT INTO %_content VALUES(?, ?, ?, ...)
  **
  ** The statement features N '?' variables, where N is the number of user
  ** defined columns in the FTS3 table, plus one for the docid field.
  */
  rc = fts3SqlStmt(p, SQL_CONTENT_INSERT, &pContentInsert, &apVal[1]);
  if( rc==SQLITE_OK && p->zLanguageid ){
    rc = sqlite3_bind_int(
        pContentInsert, p->nColumn+2, 
        sqlite3_value_int(apVal[p->nColumn+4])
    );
  }
  if( rc!=SQLITE_OK ) return rc;

  /* There is a quirk here. The users INSERT statement may have specified
  ** a value for the "rowid" field, for the "docid" field, or for both.
  ** Which is a problem, since "rowid" and "docid" are aliases for the
  ** same value. For example:
  **
  **   INSERT INTO fts3tbl(rowid, docid) VALUES(1, 2);
  **
  ** In FTS3, this is an error. It is an error to specify non-NULL values
  ** for both docid and some other rowid alias.
  */
  if( SQLITE_NULL!=sqlite3_value_type(apVal[3+p->nColumn]) ){
    if( SQLITE_NULL==sqlite3_value_type(apVal[0])
     && SQLITE_NULL!=sqlite3_value_type(apVal[1])
    ){
      /* A rowid/docid conflict. */
      return SQLITE_ERROR;
    }
    rc = sqlite3_bind_value(pContentInsert, 1, apVal[3+p->nColumn]);
    if( rc!=SQLITE_OK ) return rc;
  }

  /* Execute the statement to insert the record. Set *piDocid to the 
  ** new docid value. 
  */
  sqlite3_step(pContentInsert);
  rc = sqlite3_reset(pContentInsert);

  *piDocid = sqlite3_last_insert_rowid(p->db);
  return rc;
}



/*
** Remove all data from the FTS3 table. Clear the hash table containing
** pending terms.
*/
static int fts3DeleteAll(Fts3Table *p, int bContent){
  int rc = SQLITE_OK;             /* Return code */

  /* Discard the contents of the pending-terms hash table. */
  sqlite3Fts3PendingTermsClear(p);

  /* Delete everything from the shadow tables. Except, leave %_content as
  ** is if bContent is false.  */
  assert( p->zContentTbl==0 || bContent==0 );
  if( bContent ) fts3SqlExec(&rc, p, SQL_DELETE_ALL_CONTENT, 0);
  fts3SqlExec(&rc, p, SQL_DELETE_ALL_SEGMENTS, 0);
  fts3SqlExec(&rc, p, SQL_DELETE_ALL_SEGDIR, 0);
  if( p->bHasDocsize ){
    fts3SqlExec(&rc, p, SQL_DELETE_ALL_DOCSIZE, 0);
  }
  if( p->bHasStat ){
    fts3SqlExec(&rc, p, SQL_DELETE_ALL_STAT, 0);
  }
  return rc;
}

/*
**
*/
static int langidFromSelect(Fts3Table *p, sqlite3_stmt *pSelect){
  int iLangid = 0;
  if( p->zLanguageid ) iLangid = sqlite3_column_int(pSelect, p->nColumn+1);
  return iLangid;
}

/*
** The first element in the apVal[] array is assumed to contain the docid
** (an integer) of a row about to be deleted. Remove all terms from the
** full-text index.
*/
static void fts3DeleteTerms( 
  int *pRC,               /* Result code */
  Fts3Table *p,           /* The FTS table to delete from */
  sqlite3_value *pRowid,  /* The docid to be deleted */
  u32 *aSz,               /* Sizes of deleted document written here */
  int *pbFound            /* OUT: Set to true if row really does exist */
){
  int rc;
  sqlite3_stmt *pSelect;

  assert( *pbFound==0 );
  if( *pRC ) return;
  rc = fts3SqlStmt(p, SQL_SELECT_CONTENT_BY_ROWID, &pSelect, &pRowid);
  if( rc==SQLITE_OK ){
    if( SQLITE_ROW==sqlite3_step(pSelect) ){
      int i;
      int iLangid = langidFromSelect(p, pSelect);
      rc = fts3PendingTermsDocid(p, iLangid, sqlite3_column_int64(pSelect, 0));
      for(i=1; rc==SQLITE_OK && i<=p->nColumn; i++){
        int iCol = i-1;
        if( p->abNotindexed[iCol]==0 ){
          const char *zText = (const char *)sqlite3_column_text(pSelect, i);
          rc = fts3PendingTermsAdd(p, iLangid, zText, -1, &aSz[iCol]);
          aSz[p->nColumn] += sqlite3_column_bytes(pSelect, i);
        }
      }
      if( rc!=SQLITE_OK ){
        sqlite3_reset(pSelect);
        *pRC = rc;
        return;
      }
      *pbFound = 1;
    }
    rc = sqlite3_reset(pSelect);
  }else{
    sqlite3_reset(pSelect);
  }
  *pRC = rc;
}

/*
** Forward declaration to account for the circular dependency between
** functions fts3SegmentMerge() and fts3AllocateSegdirIdx().
*/
static int fts3SegmentMerge(Fts3Table *, int, int, int);

/* 
** This function allocates a new level iLevel index in the segdir table.
** Usually, indexes are allocated within a level sequentially starting
** with 0, so the allocated index is one greater than the value returned
** by:
**
**   SELECT max(idx) FROM %_segdir WHERE level = :iLevel
**
** However, if there are already FTS3_MERGE_COUNT indexes at the requested
** level, they are merged into a single level (iLevel+1) segment and the 
** allocated index is 0.
**
** If successful, *piIdx is set to the allocated index slot and SQLITE_OK
** returned. Otherwise, an SQLite error code is returned.
*/
static int fts3AllocateSegdirIdx(
  Fts3Table *p, 
  int iLangid,                    /* Language id */
  int iIndex,                     /* Index for p->aIndex */
  int iLevel, 
  int *piIdx
){
  int rc;                         /* Return Code */
  sqlite3_stmt *pNextIdx;         /* Query for next idx at level iLevel */
  int iNext = 0;                  /* Result of query pNextIdx */

  assert( iLangid>=0 );
  assert( p->nIndex>=1 );

  /* Set variable iNext to the next available segdir index at level iLevel. */
  rc = fts3SqlStmt(p, SQL_NEXT_SEGMENT_INDEX, &pNextIdx, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(
        pNextIdx, 1, getAbsoluteLevel(p, iLangid, iIndex, iLevel)
    );
    if( SQLITE_ROW==sqlite3_step(pNextIdx) ){
      iNext = sqlite3_column_int(pNextIdx, 0);
    }
    rc = sqlite3_reset(pNextIdx);
  }

  if( rc==SQLITE_OK ){
    /* If iNext is FTS3_MERGE_COUNT, indicating that level iLevel is already
    ** full, merge all segments in level iLevel into a single iLevel+1
    ** segment and allocate (newly freed) index 0 at level iLevel. Otherwise,
    ** if iNext is less than FTS3_MERGE_COUNT, allocate index iNext.
    */
    if( iNext>=FTS3_MERGE_COUNT ){
      fts3LogMerge(16, getAbsoluteLevel(p, iLangid, iIndex, iLevel));
      rc = fts3SegmentMerge(p, iLangid, iIndex, iLevel);
      *piIdx = 0;
    }else{
      *piIdx = iNext;
    }
  }

  return rc;
}

/*
** The %_segments table is declared as follows:
**
**   CREATE TABLE %_segments(blockid INTEGER PRIMARY KEY, block BLOB)
**
** This function reads data from a single row of the %_segments table. The
** specific row is identified by the iBlockid parameter. If paBlob is not
** NULL, then a buffer is allocated using sqlite3_malloc() and populated
** with the contents of the blob stored in the "block" column of the 
** identified table row is. Whether or not paBlob is NULL, *pnBlob is set
** to the size of the blob in bytes before returning.
**
** If an error occurs, or the table does not contain the specified row,
** an SQLite error code is returned. Otherwise, SQLITE_OK is returned. If
** paBlob is non-NULL, then it is the responsibility of the caller to
** eventually free the returned buffer.
**
** This function may leave an open sqlite3_blob* handle in the
** Fts3Table.pSegments variable. This handle is reused by subsequent calls
** to this function. The handle may be closed by calling the
** sqlite3Fts3SegmentsClose() function. Reusing a blob handle is a handy
** performance improvement, but the blob handle should always be closed
** before control is returned to the user (to prevent a lock being held
** on the database file for longer than necessary). Thus, any virtual table
** method (xFilter etc.) that may directly or indirectly call this function
** must call sqlite3Fts3SegmentsClose() before returning.
*/
SQLITE_PRIVATE int sqlite3Fts3ReadBlock(
  Fts3Table *p,                   /* FTS3 table handle */
  sqlite3_int64 iBlockid,         /* Access the row with blockid=$iBlockid */
  char **paBlob,                  /* OUT: Blob data in malloc'd buffer */
  int *pnBlob,                    /* OUT: Size of blob data */
  int *pnLoad                     /* OUT: Bytes actually loaded */
){
  int rc;                         /* Return code */

  /* pnBlob must be non-NULL. paBlob may be NULL or non-NULL. */
  assert( pnBlob );

  if( p->pSegments ){
    rc = sqlite3_blob_reopen(p->pSegments, iBlockid);
  }else{
    if( 0==p->zSegmentsTbl ){
      p->zSegmentsTbl = sqlite3_mprintf("%s_segments", p->zName);
      if( 0==p->zSegmentsTbl ) return SQLITE_NOMEM;
    }
    rc = sqlite3_blob_open(
       p->db, p->zDb, p->zSegmentsTbl, "block", iBlockid, 0, &p->pSegments
    );
  }

  if( rc==SQLITE_OK ){
    int nByte = sqlite3_blob_bytes(p->pSegments);
    *pnBlob = nByte;
    if( paBlob ){
      char *aByte = sqlite3_malloc(nByte + FTS3_NODE_PADDING);
      if( !aByte ){
        rc = SQLITE_NOMEM;
      }else{
        if( pnLoad && nByte>(FTS3_NODE_CHUNK_THRESHOLD) ){
          nByte = FTS3_NODE_CHUNKSIZE;
          *pnLoad = nByte;
        }
        rc = sqlite3_blob_read(p->pSegments, aByte, nByte, 0);
        memset(&aByte[nByte], 0, FTS3_NODE_PADDING);
        if( rc!=SQLITE_OK ){
          sqlite3_free(aByte);
          aByte = 0;
        }
      }
      *paBlob = aByte;
    }
  }

  return rc;
}

/*
** Close the blob handle at p->pSegments, if it is open. See comments above
** the sqlite3Fts3ReadBlock() function for details.
*/
SQLITE_PRIVATE void sqlite3Fts3SegmentsClose(Fts3Table *p){
  sqlite3_blob_close(p->pSegments);
  p->pSegments = 0;
}
    
static int fts3SegReaderIncrRead(Fts3SegReader *pReader){
  int nRead;                      /* Number of bytes to read */
  int rc;                         /* Return code */

  nRead = MIN(pReader->nNode - pReader->nPopulate, FTS3_NODE_CHUNKSIZE);
  rc = sqlite3_blob_read(
      pReader->pBlob, 
      &pReader->aNode[pReader->nPopulate],
      nRead,
      pReader->nPopulate
  );

  if( rc==SQLITE_OK ){
    pReader->nPopulate += nRead;
    memset(&pReader->aNode[pReader->nPopulate], 0, FTS3_NODE_PADDING);
    if( pReader->nPopulate==pReader->nNode ){
      sqlite3_blob_close(pReader->pBlob);
      pReader->pBlob = 0;
      pReader->nPopulate = 0;
    }
  }
  return rc;
}

static int fts3SegReaderRequire(Fts3SegReader *pReader, char *pFrom, int nByte){
  int rc = SQLITE_OK;
  assert( !pReader->pBlob 
       || (pFrom>=pReader->aNode && pFrom<&pReader->aNode[pReader->nNode])
  );
  while( pReader->pBlob && rc==SQLITE_OK 
     &&  (pFrom - pReader->aNode + nByte)>pReader->nPopulate
  ){
    rc = fts3SegReaderIncrRead(pReader);
  }
  return rc;
}

/*
** Set an Fts3SegReader cursor to point at EOF.
*/
static void fts3SegReaderSetEof(Fts3SegReader *pSeg){
  if( !fts3SegReaderIsRootOnly(pSeg) ){
    sqlite3_free(pSeg->aNode);
    sqlite3_blob_close(pSeg->pBlob);
    pSeg->pBlob = 0;
  }
  pSeg->aNode = 0;
}

/*
** Move the iterator passed as the first argument to the next term in the
** segment. If successful, SQLITE_OK is returned. If there is no next term,
** SQLITE_DONE. Otherwise, an SQLite error code.
*/
static int fts3SegReaderNext(
  Fts3Table *p, 
  Fts3SegReader *pReader,
  int bIncr
){
  int rc;                         /* Return code of various sub-routines */
  char *pNext;                    /* Cursor variable */
  int nPrefix;                    /* Number of bytes in term prefix */
  int nSuffix;                    /* Number of bytes in term suffix */

  if( !pReader->aDoclist ){
    pNext = pReader->aNode;
  }else{
    pNext = &pReader->aDoclist[pReader->nDoclist];
  }

  if( !pNext || pNext>=&pReader->aNode[pReader->nNode] ){

    if( fts3SegReaderIsPending(pReader) ){
      Fts3HashElem *pElem = *(pReader->ppNextElem);
      if( pElem==0 ){
        pReader->aNode = 0;
      }else{
        PendingList *pList = (PendingList *)fts3HashData(pElem);
        pReader->zTerm = (char *)fts3HashKey(pElem);
        pReader->nTerm = fts3HashKeysize(pElem);
        pReader->nNode = pReader->nDoclist = pList->nData + 1;
        pReader->aNode = pReader->aDoclist = pList->aData;
        pReader->ppNextElem++;
        assert( pReader->aNode );
      }
      return SQLITE_OK;
    }

    fts3SegReaderSetEof(pReader);

    /* If iCurrentBlock>=iLeafEndBlock, this is an EOF condition. All leaf 
    ** blocks have already been traversed.  */
    assert( pReader->iCurrentBlock<=pReader->iLeafEndBlock );
    if( pReader->iCurrentBlock>=pReader->iLeafEndBlock ){
      return SQLITE_OK;
    }

    rc = sqlite3Fts3ReadBlock(
        p, ++pReader->iCurrentBlock, &pReader->aNode, &pReader->nNode, 
        (bIncr ? &pReader->nPopulate : 0)
    );
    if( rc!=SQLITE_OK ) return rc;
    assert( pReader->pBlob==0 );
    if( bIncr && pReader->nPopulate<pReader->nNode ){
      pReader->pBlob = p->pSegments;
      p->pSegments = 0;
    }
    pNext = pReader->aNode;
  }

  assert( !fts3SegReaderIsPending(pReader) );

  rc = fts3SegReaderRequire(pReader, pNext, FTS3_VARINT_MAX*2);
  if( rc!=SQLITE_OK ) return rc;
  
  /* Because of the FTS3_NODE_PADDING bytes of padding, the following is 
  ** safe (no risk of overread) even if the node data is corrupted. */
  pNext += fts3GetVarint32(pNext, &nPrefix);
  pNext += fts3GetVarint32(pNext, &nSuffix);
  if( nPrefix<0 || nSuffix<=0 
   || &pNext[nSuffix]>&pReader->aNode[pReader->nNode] 
  ){
    return FTS_CORRUPT_VTAB;
  }

  if( nPrefix+nSuffix>pReader->nTermAlloc ){
    int nNew = (nPrefix+nSuffix)*2;
    char *zNew = sqlite3_realloc(pReader->zTerm, nNew);
    if( !zNew ){
      return SQLITE_NOMEM;
    }
    pReader->zTerm = zNew;
    pReader->nTermAlloc = nNew;
  }

  rc = fts3SegReaderRequire(pReader, pNext, nSuffix+FTS3_VARINT_MAX);
  if( rc!=SQLITE_OK ) return rc;

  memcpy(&pReader->zTerm[nPrefix], pNext, nSuffix);
  pReader->nTerm = nPrefix+nSuffix;
  pNext += nSuffix;
  pNext += fts3GetVarint32(pNext, &pReader->nDoclist);
  pReader->aDoclist = pNext;
  pReader->pOffsetList = 0;

  /* Check that the doclist does not appear to extend past the end of the
  ** b-tree node. And that the final byte of the doclist is 0x00. If either 
  ** of these statements is untrue, then the data structure is corrupt.
  */
  if( &pReader->aDoclist[pReader->nDoclist]>&pReader->aNode[pReader->nNode] 
   || (pReader->nPopulate==0 && pReader->aDoclist[pReader->nDoclist-1])
  ){
    return FTS_CORRUPT_VTAB;
  }
  return SQLITE_OK;
}

/*
** Set the SegReader to point to the first docid in the doclist associated
** with the current term.
*/
static int fts3SegReaderFirstDocid(Fts3Table *pTab, Fts3SegReader *pReader){
  int rc = SQLITE_OK;
  assert( pReader->aDoclist );
  assert( !pReader->pOffsetList );
  if( pTab->bDescIdx && fts3SegReaderIsPending(pReader) ){
    u8 bEof = 0;
    pReader->iDocid = 0;
    pReader->nOffsetList = 0;
    sqlite3Fts3DoclistPrev(0,
        pReader->aDoclist, pReader->nDoclist, &pReader->pOffsetList, 
        &pReader->iDocid, &pReader->nOffsetList, &bEof
    );
  }else{
    rc = fts3SegReaderRequire(pReader, pReader->aDoclist, FTS3_VARINT_MAX);
    if( rc==SQLITE_OK ){
      int n = sqlite3Fts3GetVarint(pReader->aDoclist, &pReader->iDocid);
      pReader->pOffsetList = &pReader->aDoclist[n];
    }
  }
  return rc;
}

/*
** Advance the SegReader to point to the next docid in the doclist
** associated with the current term.
** 
** If arguments ppOffsetList and pnOffsetList are not NULL, then 
** *ppOffsetList is set to point to the first column-offset list
** in the doclist entry (i.e. immediately past the docid varint).
** *pnOffsetList is set to the length of the set of column-offset
** lists, not including the nul-terminator byte. For example:
*/
static int fts3SegReaderNextDocid(
  Fts3Table *pTab,
  Fts3SegReader *pReader,         /* Reader to advance to next docid */
  char **ppOffsetList,            /* OUT: Pointer to current position-list */
  int *pnOffsetList               /* OUT: Length of *ppOffsetList in bytes */
){
  int rc = SQLITE_OK;
  char *p = pReader->pOffsetList;
  char c = 0;

  assert( p );

  if( pTab->bDescIdx && fts3SegReaderIsPending(pReader) ){
    /* A pending-terms seg-reader for an FTS4 table that uses order=desc.
    ** Pending-terms doclists are always built up in ascending order, so
    ** we have to iterate through them backwards here. */
    u8 bEof = 0;
    if( ppOffsetList ){
      *ppOffsetList = pReader->pOffsetList;
      *pnOffsetList = pReader->nOffsetList - 1;
    }
    sqlite3Fts3DoclistPrev(0,
        pReader->aDoclist, pReader->nDoclist, &p, &pReader->iDocid,
        &pReader->nOffsetList, &bEof
    );
    if( bEof ){
      pReader->pOffsetList = 0;
    }else{
      pReader->pOffsetList = p;
    }
  }else{
    char *pEnd = &pReader->aDoclist[pReader->nDoclist];

    /* Pointer p currently points at the first byte of an offset list. The
    ** following block advances it to point one byte past the end of
    ** the same offset list. */
    while( 1 ){
  
      /* The following line of code (and the "p++" below the while() loop) is
      ** normally all that is required to move pointer p to the desired 
      ** position. The exception is if this node is being loaded from disk
      ** incrementally and pointer "p" now points to the first byte past
      ** the populated part of pReader->aNode[].
      */
      while( *p | c ) c = *p++ & 0x80;
      assert( *p==0 );
  
      if( pReader->pBlob==0 || p<&pReader->aNode[pReader->nPopulate] ) break;
      rc = fts3SegReaderIncrRead(pReader);
      if( rc!=SQLITE_OK ) return rc;
    }
    p++;
  
    /* If required, populate the output variables with a pointer to and the
    ** size of the previous offset-list.
    */
    if( ppOffsetList ){
      *ppOffsetList = pReader->pOffsetList;
      *pnOffsetList = (int)(p - pReader->pOffsetList - 1);
    }

    /* List may have been edited in place by fts3EvalNearTrim() */
    while( p<pEnd && *p==0 ) p++;
  
    /* If there are no more entries in the doclist, set pOffsetList to
    ** NULL. Otherwise, set Fts3SegReader.iDocid to the next docid and
    ** Fts3SegReader.pOffsetList to point to the next offset list before
    ** returning.
    */
    if( p>=pEnd ){
      pReader->pOffsetList = 0;
    }else{
      rc = fts3SegReaderRequire(pReader, p, FTS3_VARINT_MAX);
      if( rc==SQLITE_OK ){
        sqlite3_int64 iDelta;
        pReader->pOffsetList = p + sqlite3Fts3GetVarint(p, &iDelta);
        if( pTab->bDescIdx ){
          pReader->iDocid -= iDelta;
        }else{
          pReader->iDocid += iDelta;
        }
      }
    }
  }

  return SQLITE_OK;
}


SQLITE_PRIVATE int sqlite3Fts3MsrOvfl(
  Fts3Cursor *pCsr, 
  Fts3MultiSegReader *pMsr,
  int *pnOvfl
){
  Fts3Table *p = (Fts3Table*)pCsr->base.pVtab;
  int nOvfl = 0;
  int ii;
  int rc = SQLITE_OK;
  int pgsz = p->nPgsz;

  assert( p->bFts4 );
  assert( pgsz>0 );

  for(ii=0; rc==SQLITE_OK && ii<pMsr->nSegment; ii++){
    Fts3SegReader *pReader = pMsr->apSegment[ii];
    if( !fts3SegReaderIsPending(pReader) 
     && !fts3SegReaderIsRootOnly(pReader) 
    ){
      sqlite3_int64 jj;
      for(jj=pReader->iStartBlock; jj<=pReader->iLeafEndBlock; jj++){
        int nBlob;
        rc = sqlite3Fts3ReadBlock(p, jj, 0, &nBlob, 0);
        if( rc!=SQLITE_OK ) break;
        if( (nBlob+35)>pgsz ){
          nOvfl += (nBlob + 34)/pgsz;
        }
      }
    }
  }
  *pnOvfl = nOvfl;
  return rc;
}

/*
** Free all allocations associated with the iterator passed as the 
** second argument.
*/
SQLITE_PRIVATE void sqlite3Fts3SegReaderFree(Fts3SegReader *pReader){
  if( pReader && !fts3SegReaderIsPending(pReader) ){
    sqlite3_free(pReader->zTerm);
    if( !fts3SegReaderIsRootOnly(pReader) ){
      sqlite3_free(pReader->aNode);
      sqlite3_blob_close(pReader->pBlob);
    }
  }
  sqlite3_free(pReader);
}

/*
** Allocate a new SegReader object.
*/
SQLITE_PRIVATE int sqlite3Fts3SegReaderNew(
  int iAge,                       /* Segment "age". */
  int bLookup,                    /* True for a lookup only */
  sqlite3_int64 iStartLeaf,       /* First leaf to traverse */
  sqlite3_int64 iEndLeaf,         /* Final leaf to traverse */
  sqlite3_int64 iEndBlock,        /* Final block of segment */
  const char *zRoot,              /* Buffer containing root node */
  int nRoot,                      /* Size of buffer containing root node */
  Fts3SegReader **ppReader        /* OUT: Allocated Fts3SegReader */
){
  Fts3SegReader *pReader;         /* Newly allocated SegReader object */
  int nExtra = 0;                 /* Bytes to allocate segment root node */

  assert( iStartLeaf<=iEndLeaf );
  if( iStartLeaf==0 ){
    nExtra = nRoot + FTS3_NODE_PADDING;
  }

  pReader = (Fts3SegReader *)sqlite3_malloc(sizeof(Fts3SegReader) + nExtra);
  if( !pReader ){
    return SQLITE_NOMEM;
  }
  memset(pReader, 0, sizeof(Fts3SegReader));
  pReader->iIdx = iAge;
  pReader->bLookup = bLookup!=0;
  pReader->iStartBlock = iStartLeaf;
  pReader->iLeafEndBlock = iEndLeaf;
  pReader->iEndBlock = iEndBlock;

  if( nExtra ){
    /* The entire segment is stored in the root node. */
    pReader->aNode = (char *)&pReader[1];
    pReader->rootOnly = 1;
    pReader->nNode = nRoot;
    memcpy(pReader->aNode, zRoot, nRoot);
    memset(&pReader->aNode[nRoot], 0, FTS3_NODE_PADDING);
  }else{
    pReader->iCurrentBlock = iStartLeaf-1;
  }
  *ppReader = pReader;
  return SQLITE_OK;
}

/*
** This is a comparison function used as a qsort() callback when sorting
** an array of pending terms by term. This occurs as part of flushing
** the contents of the pending-terms hash table to the database.
*/
static int SQLITE_CDECL fts3CompareElemByTerm(
  const void *lhs,
  const void *rhs
){
  char *z1 = fts3HashKey(*(Fts3HashElem **)lhs);
  char *z2 = fts3HashKey(*(Fts3HashElem **)rhs);
  int n1 = fts3HashKeysize(*(Fts3HashElem **)lhs);
  int n2 = fts3HashKeysize(*(Fts3HashElem **)rhs);

  int n = (n1<n2 ? n1 : n2);
  int c = memcmp(z1, z2, n);
  if( c==0 ){
    c = n1 - n2;
  }
  return c;
}

/*
** This function is used to allocate an Fts3SegReader that iterates through
** a subset of the terms stored in the Fts3Table.pendingTerms array.
**
** If the isPrefixIter parameter is zero, then the returned SegReader iterates
** through each term in the pending-terms table. Or, if isPrefixIter is
** non-zero, it iterates through each term and its prefixes. For example, if
** the pending terms hash table contains the terms "sqlite", "mysql" and
** "firebird", then the iterator visits the following 'terms' (in the order
** shown):
**
**   f fi fir fire fireb firebi firebir firebird
**   m my mys mysq mysql
**   s sq sql sqli sqlit sqlite
**
** Whereas if isPrefixIter is zero, the terms visited are:
**
**   firebird mysql sqlite
*/
SQLITE_PRIVATE int sqlite3Fts3SegReaderPending(
  Fts3Table *p,                   /* Virtual table handle */
  int iIndex,                     /* Index for p->aIndex */
  const char *zTerm,              /* Term to search for */
  int nTerm,                      /* Size of buffer zTerm */
  int bPrefix,                    /* True for a prefix iterator */
  Fts3SegReader **ppReader        /* OUT: SegReader for pending-terms */
){
  Fts3SegReader *pReader = 0;     /* Fts3SegReader object to return */
  Fts3HashElem *pE;               /* Iterator variable */
  Fts3HashElem **aElem = 0;       /* Array of term hash entries to scan */
  int nElem = 0;                  /* Size of array at aElem */
  int rc = SQLITE_OK;             /* Return Code */
  Fts3Hash *pHash;

  pHash = &p->aIndex[iIndex].hPending;
  if( bPrefix ){
    int nAlloc = 0;               /* Size of allocated array at aElem */

    for(pE=fts3HashFirst(pHash); pE; pE=fts3HashNext(pE)){
      char *zKey = (char *)fts3HashKey(pE);
      int nKey = fts3HashKeysize(pE);
      if( nTerm==0 || (nKey>=nTerm && 0==memcmp(zKey, zTerm, nTerm)) ){
        if( nElem==nAlloc ){
          Fts3HashElem **aElem2;
          nAlloc += 16;
          aElem2 = (Fts3HashElem **)sqlite3_realloc(
              aElem, nAlloc*sizeof(Fts3HashElem *)
          );
          if( !aElem2 ){
            rc = SQLITE_NOMEM;
            nElem = 0;
            break;
          }
          aElem = aElem2;
        }

        aElem[nElem++] = pE;
      }
    }

    /* If more than one term matches the prefix, sort the Fts3HashElem
    ** objects in term order using qsort(). This uses the same comparison
    ** callback as is used when flushing terms to disk.
    */
    if( nElem>1 ){
      qsort(aElem, nElem, sizeof(Fts3HashElem *), fts3CompareElemByTerm);
    }

  }else{
    /* The query is a simple term lookup that matches at most one term in
    ** the index. All that is required is a straight hash-lookup. 
    **
    ** Because the stack address of pE may be accessed via the aElem pointer
    ** below, the "Fts3HashElem *pE" must be declared so that it is valid
    ** within this entire function, not just this "else{...}" block.
    */
    pE = fts3HashFindElem(pHash, zTerm, nTerm);
    if( pE ){
      aElem = &pE;
      nElem = 1;
    }
  }

  if( nElem>0 ){
    int nByte = sizeof(Fts3SegReader) + (nElem+1)*sizeof(Fts3HashElem *);
    pReader = (Fts3SegReader *)sqlite3_malloc(nByte);
    if( !pReader ){
      rc = SQLITE_NOMEM;
    }else{
      memset(pReader, 0, nByte);
      pReader->iIdx = 0x7FFFFFFF;
      pReader->ppNextElem = (Fts3HashElem **)&pReader[1];
      memcpy(pReader->ppNextElem, aElem, nElem*sizeof(Fts3HashElem *));
    }
  }

  if( bPrefix ){
    sqlite3_free(aElem);
  }
  *ppReader = pReader;
  return rc;
}

/*
** Compare the entries pointed to by two Fts3SegReader structures. 
** Comparison is as follows:
**
**   1) EOF is greater than not EOF.
**
**   2) The current terms (if any) are compared using memcmp(). If one
**      term is a prefix of another, the longer term is considered the
**      larger.
**
**   3) By segment age. An older segment is considered larger.
*/
static int fts3SegReaderCmp(Fts3SegReader *pLhs, Fts3SegReader *pRhs){
  int rc;
  if( pLhs->aNode && pRhs->aNode ){
    int rc2 = pLhs->nTerm - pRhs->nTerm;
    if( rc2<0 ){
      rc = memcmp(pLhs->zTerm, pRhs->zTerm, pLhs->nTerm);
    }else{
      rc = memcmp(pLhs->zTerm, pRhs->zTerm, pRhs->nTerm);
    }
    if( rc==0 ){
      rc = rc2;
    }
  }else{
    rc = (pLhs->aNode==0) - (pRhs->aNode==0);
  }
  if( rc==0 ){
    rc = pRhs->iIdx - pLhs->iIdx;
  }
  assert( rc!=0 );
  return rc;
}

/*
** A different comparison function for SegReader structures. In this
** version, it is assumed that each SegReader points to an entry in
** a doclist for identical terms. Comparison is made as follows:
**
**   1) EOF (end of doclist in this case) is greater than not EOF.
**
**   2) By current docid.
**
**   3) By segment age. An older segment is considered larger.
*/
static int fts3SegReaderDoclistCmp(Fts3SegReader *pLhs, Fts3SegReader *pRhs){
  int rc = (pLhs->pOffsetList==0)-(pRhs->pOffsetList==0);
  if( rc==0 ){
    if( pLhs->iDocid==pRhs->iDocid ){
      rc = pRhs->iIdx - pLhs->iIdx;
    }else{
      rc = (pLhs->iDocid > pRhs->iDocid) ? 1 : -1;
    }
  }
  assert( pLhs->aNode && pRhs->aNode );
  return rc;
}
static int fts3SegReaderDoclistCmpRev(Fts3SegReader *pLhs, Fts3SegReader *pRhs){
  int rc = (pLhs->pOffsetList==0)-(pRhs->pOffsetList==0);
  if( rc==0 ){
    if( pLhs->iDocid==pRhs->iDocid ){
      rc = pRhs->iIdx - pLhs->iIdx;
    }else{
      rc = (pLhs->iDocid < pRhs->iDocid) ? 1 : -1;
    }
  }
  assert( pLhs->aNode && pRhs->aNode );
  return rc;
}

/*
** Compare the term that the Fts3SegReader object passed as the first argument
** points to with the term specified by arguments zTerm and nTerm. 
**
** If the pSeg iterator is already at EOF, return 0. Otherwise, return
** -ve if the pSeg term is less than zTerm/nTerm, 0 if the two terms are
** equal, or +ve if the pSeg term is greater than zTerm/nTerm.
*/
static int fts3SegReaderTermCmp(
  Fts3SegReader *pSeg,            /* Segment reader object */
  const char *zTerm,              /* Term to compare to */
  int nTerm                       /* Size of term zTerm in bytes */
){
  int res = 0;
  if( pSeg->aNode ){
    if( pSeg->nTerm>nTerm ){
      res = memcmp(pSeg->zTerm, zTerm, nTerm);
    }else{
      res = memcmp(pSeg->zTerm, zTerm, pSeg->nTerm);
    }
    if( res==0 ){
      res = pSeg->nTerm-nTerm;
    }
  }
  return res;
}

/*
** Argument apSegment is an array of nSegment elements. It is known that
** the final (nSegment-nSuspect) members are already in sorted order
** (according to the comparison function provided). This function shuffles
** the array around until all entries are in sorted order.
*/
static void fts3SegReaderSort(
  Fts3SegReader **apSegment,                     /* Array to sort entries of */
  int nSegment,                                  /* Size of apSegment array */
  int nSuspect,                                  /* Unsorted entry count */
  int (*xCmp)(Fts3SegReader *, Fts3SegReader *)  /* Comparison function */
){
  int i;                          /* Iterator variable */

  assert( nSuspect<=nSegment );

  if( nSuspect==nSegment ) nSuspect--;
  for(i=nSuspect-1; i>=0; i--){
    int j;
    for(j=i; j<(nSegment-1); j++){
      Fts3SegReader *pTmp;
      if( xCmp(apSegment[j], apSegment[j+1])<0 ) break;
      pTmp = apSegment[j+1];
      apSegment[j+1] = apSegment[j];
      apSegment[j] = pTmp;
    }
  }

#ifndef NDEBUG
  /* Check that the list really is sorted now. */
  for(i=0; i<(nSuspect-1); i++){
    assert( xCmp(apSegment[i], apSegment[i+1])<0 );
  }
#endif
}

/* 
** Insert a record into the %_segments table.
*/
static int fts3WriteSegment(
  Fts3Table *p,                   /* Virtual table handle */
  sqlite3_int64 iBlock,           /* Block id for new block */
  char *z,                        /* Pointer to buffer containing block data */
  int n                           /* Size of buffer z in bytes */
){
  sqlite3_stmt *pStmt;
  int rc = fts3SqlStmt(p, SQL_INSERT_SEGMENTS, &pStmt, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pStmt, 1, iBlock);
    sqlite3_bind_blob(pStmt, 2, z, n, SQLITE_STATIC);
    sqlite3_step(pStmt);
    rc = sqlite3_reset(pStmt);
  }
  return rc;
}

/*
** Find the largest relative level number in the table. If successful, set
** *pnMax to this value and return SQLITE_OK. Otherwise, if an error occurs,
** set *pnMax to zero and return an SQLite error code.
*/
SQLITE_PRIVATE int sqlite3Fts3MaxLevel(Fts3Table *p, int *pnMax){
  int rc;
  int mxLevel = 0;
  sqlite3_stmt *pStmt = 0;

  rc = fts3SqlStmt(p, SQL_SELECT_MXLEVEL, &pStmt, 0);
  if( rc==SQLITE_OK ){
    if( SQLITE_ROW==sqlite3_step(pStmt) ){
      mxLevel = sqlite3_column_int(pStmt, 0);
    }
    rc = sqlite3_reset(pStmt);
  }
  *pnMax = mxLevel;
  return rc;
}

/* 
** Insert a record into the %_segdir table.
*/
static int fts3WriteSegdir(
  Fts3Table *p,                   /* Virtual table handle */
  sqlite3_int64 iLevel,           /* Value for "level" field (absolute level) */
  int iIdx,                       /* Value for "idx" field */
  sqlite3_int64 iStartBlock,      /* Value for "start_block" field */
  sqlite3_int64 iLeafEndBlock,    /* Value for "leaves_end_block" field */
  sqlite3_int64 iEndBlock,        /* Value for "end_block" field */
  sqlite3_int64 nLeafData,        /* Bytes of leaf data in segment */
  char *zRoot,                    /* Blob value for "root" field */
  int nRoot                       /* Number of bytes in buffer zRoot */
){
  sqlite3_stmt *pStmt;
  int rc = fts3SqlStmt(p, SQL_INSERT_SEGDIR, &pStmt, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pStmt, 1, iLevel);
    sqlite3_bind_int(pStmt, 2, iIdx);
    sqlite3_bind_int64(pStmt, 3, iStartBlock);
    sqlite3_bind_int64(pStmt, 4, iLeafEndBlock);
    if( nLeafData==0 ){
      sqlite3_bind_int64(pStmt, 5, iEndBlock);
    }else{
      char *zEnd = sqlite3_mprintf("%lld %lld", iEndBlock, nLeafData);
      if( !zEnd ) return SQLITE_NOMEM;
      sqlite3_bind_text(pStmt, 5, zEnd, -1, sqlite3_free);
    }
    sqlite3_bind_blob(pStmt, 6, zRoot, nRoot, SQLITE_STATIC);
    sqlite3_step(pStmt);
    rc = sqlite3_reset(pStmt);
  }
  return rc;
}

/*
** Return the size of the common prefix (if any) shared by zPrev and
** zNext, in bytes. For example, 
**
**   fts3PrefixCompress("abc", 3, "abcdef", 6)   // returns 3
**   fts3PrefixCompress("abX", 3, "abcdef", 6)   // returns 2
**   fts3PrefixCompress("abX", 3, "Xbcdef", 6)   // returns 0
*/
static int fts3PrefixCompress(
  const char *zPrev,              /* Buffer containing previous term */
  int nPrev,                      /* Size of buffer zPrev in bytes */
  const char *zNext,              /* Buffer containing next term */
  int nNext                       /* Size of buffer zNext in bytes */
){
  int n;
  UNUSED_PARAMETER(nNext);
  for(n=0; n<nPrev && zPrev[n]==zNext[n]; n++);
  return n;
}

/*
** Add term zTerm to the SegmentNode. It is guaranteed that zTerm is larger
** (according to memcmp) than the previous term.
*/
static int fts3NodeAddTerm(
  Fts3Table *p,                   /* Virtual table handle */
  SegmentNode **ppTree,           /* IN/OUT: SegmentNode handle */ 
  int isCopyTerm,                 /* True if zTerm/nTerm is transient */
  const char *zTerm,              /* Pointer to buffer containing term */
  int nTerm                       /* Size of term in bytes */
){
  SegmentNode *pTree = *ppTree;
  int rc;
  SegmentNode *pNew;

  /* First try to append the term to the current node. Return early if 
  ** this is possible.
  */
  if( pTree ){
    int nData = pTree->nData;     /* Current size of node in bytes */
    int nReq = nData;             /* Required space after adding zTerm */
    int nPrefix;                  /* Number of bytes of prefix compression */
    int nSuffix;                  /* Suffix length */

    nPrefix = fts3PrefixCompress(pTree->zTerm, pTree->nTerm, zTerm, nTerm);
    nSuffix = nTerm-nPrefix;

    nReq += sqlite3Fts3VarintLen(nPrefix)+sqlite3Fts3VarintLen(nSuffix)+nSuffix;
    if( nReq<=p->nNodeSize || !pTree->zTerm ){

      if( nReq>p->nNodeSize ){
        /* An unusual case: this is the first term to be added to the node
        ** and the static node buffer (p->nNodeSize bytes) is not large
        ** enough. Use a separately malloced buffer instead This wastes
        ** p->nNodeSize bytes, but since this scenario only comes about when
        ** the database contain two terms that share a prefix of almost 2KB, 
        ** this is not expected to be a serious problem. 
        */
        assert( pTree->aData==(char *)&pTree[1] );
        pTree->aData = (char *)sqlite3_malloc(nReq);
        if( !pTree->aData ){
          return SQLITE_NOMEM;
        }
      }

      if( pTree->zTerm ){
        /* There is no prefix-length field for first term in a node */
        nData += sqlite3Fts3PutVarint(&pTree->aData[nData], nPrefix);
      }

      nData += sqlite3Fts3PutVarint(&pTree->aData[nData], nSuffix);
      memcpy(&pTree->aData[nData], &zTerm[nPrefix], nSuffix);
      pTree->nData = nData + nSuffix;
      pTree->nEntry++;

      if( isCopyTerm ){
        if( pTree->nMalloc<nTerm ){
          char *zNew = sqlite3_realloc(pTree->zMalloc, nTerm*2);
          if( !zNew ){
            return SQLITE_NOMEM;
          }
          pTree->nMalloc = nTerm*2;
          pTree->zMalloc = zNew;
        }
        pTree->zTerm = pTree->zMalloc;
        memcpy(pTree->zTerm, zTerm, nTerm);
        pTree->nTerm = nTerm;
      }else{
        pTree->zTerm = (char *)zTerm;
        pTree->nTerm = nTerm;
      }
      return SQLITE_OK;
    }
  }

  /* If control flows to here, it was not possible to append zTerm to the
  ** current node. Create a new node (a right-sibling of the current node).
  ** If this is the first node in the tree, the term is added to it.
  **
  ** Otherwise, the term is not added to the new node, it is left empty for
  ** now. Instead, the term is inserted into the parent of pTree. If pTree 
  ** has no parent, one is created here.
  */
  pNew = (SegmentNode *)sqlite3_malloc(sizeof(SegmentNode) + p->nNodeSize);
  if( !pNew ){
    return SQLITE_NOMEM;
  }
  memset(pNew, 0, sizeof(SegmentNode));
  pNew->nData = 1 + FTS3_VARINT_MAX;
  pNew->aData = (char *)&pNew[1];

  if( pTree ){
    SegmentNode *pParent = pTree->pParent;
    rc = fts3NodeAddTerm(p, &pParent, isCopyTerm, zTerm, nTerm);
    if( pTree->pParent==0 ){
      pTree->pParent = pParent;
    }
    pTree->pRight = pNew;
    pNew->pLeftmost = pTree->pLeftmost;
    pNew->pParent = pParent;
    pNew->zMalloc = pTree->zMalloc;
    pNew->nMalloc = pTree->nMalloc;
    pTree->zMalloc = 0;
  }else{
    pNew->pLeftmost = pNew;
    rc = fts3NodeAddTerm(p, &pNew, isCopyTerm, zTerm, nTerm); 
  }

  *ppTree = pNew;
  return rc;
}

/*
** Helper function for fts3NodeWrite().
*/
static int fts3TreeFinishNode(
  SegmentNode *pTree, 
  int iHeight, 
  sqlite3_int64 iLeftChild
){
  int nStart;
  assert( iHeight>=1 && iHeight<128 );
  nStart = FTS3_VARINT_MAX - sqlite3Fts3VarintLen(iLeftChild);
  pTree->aData[nStart] = (char)iHeight;
  sqlite3Fts3PutVarint(&pTree->aData[nStart+1], iLeftChild);
  return nStart;
}

/*
** Write the buffer for the segment node pTree and all of its peers to the
** database. Then call this function recursively to write the parent of 
** pTree and its peers to the database. 
**
** Except, if pTree is a root node, do not write it to the database. Instead,
** set output variables *paRoot and *pnRoot to contain the root node.
**
** If successful, SQLITE_OK is returned and output variable *piLast is
** set to the largest blockid written to the database (or zero if no
** blocks were written to the db). Otherwise, an SQLite error code is 
** returned.
*/
static int fts3NodeWrite(
  Fts3Table *p,                   /* Virtual table handle */
  SegmentNode *pTree,             /* SegmentNode handle */
  int iHeight,                    /* Height of this node in tree */
  sqlite3_int64 iLeaf,            /* Block id of first leaf node */
  sqlite3_int64 iFree,            /* Block id of next free slot in %_segments */
  sqlite3_int64 *piLast,          /* OUT: Block id of last entry written */
  char **paRoot,                  /* OUT: Data for root node */
  int *pnRoot                     /* OUT: Size of root node in bytes */
){
  int rc = SQLITE_OK;

  if( !pTree->pParent ){
    /* Root node of the tree. */
    int nStart = fts3TreeFinishNode(pTree, iHeight, iLeaf);
    *piLast = iFree-1;
    *pnRoot = pTree->nData - nStart;
    *paRoot = &pTree->aData[nStart];
  }else{
    SegmentNode *pIter;
    sqlite3_int64 iNextFree = iFree;
    sqlite3_int64 iNextLeaf = iLeaf;
    for(pIter=pTree->pLeftmost; pIter && rc==SQLITE_OK; pIter=pIter->pRight){
      int nStart = fts3TreeFinishNode(pIter, iHeight, iNextLeaf);
      int nWrite = pIter->nData - nStart;
  
      rc = fts3WriteSegment(p, iNextFree, &pIter->aData[nStart], nWrite);
      iNextFree++;
      iNextLeaf += (pIter->nEntry+1);
    }
    if( rc==SQLITE_OK ){
      assert( iNextLeaf==iFree );
      rc = fts3NodeWrite(
          p, pTree->pParent, iHeight+1, iFree, iNextFree, piLast, paRoot, pnRoot
      );
    }
  }

  return rc;
}

/*
** Free all memory allocations associated with the tree pTree.
*/
static void fts3NodeFree(SegmentNode *pTree){
  if( pTree ){
    SegmentNode *p = pTree->pLeftmost;
    fts3NodeFree(p->pParent);
    while( p ){
      SegmentNode *pRight = p->pRight;
      if( p->aData!=(char *)&p[1] ){
        sqlite3_free(p->aData);
      }
      assert( pRight==0 || p->zMalloc==0 );
      sqlite3_free(p->zMalloc);
      sqlite3_free(p);
      p = pRight;
    }
  }
}

/*
** Add a term to the segment being constructed by the SegmentWriter object
** *ppWriter. When adding the first term to a segment, *ppWriter should
** be passed NULL. This function will allocate a new SegmentWriter object
** and return it via the input/output variable *ppWriter in this case.
**
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code.
*/
static int fts3SegWriterAdd(
  Fts3Table *p,                   /* Virtual table handle */
  SegmentWriter **ppWriter,       /* IN/OUT: SegmentWriter handle */ 
  int isCopyTerm,                 /* True if buffer zTerm must be copied */
  const char *zTerm,              /* Pointer to buffer containing term */
  int nTerm,                      /* Size of term in bytes */
  const char *aDoclist,           /* Pointer to buffer containing doclist */
  int nDoclist                    /* Size of doclist in bytes */
){
  int nPrefix;                    /* Size of term prefix in bytes */
  int nSuffix;                    /* Size of term suffix in bytes */
  int nReq;                       /* Number of bytes required on leaf page */
  int nData;
  SegmentWriter *pWriter = *ppWriter;

  if( !pWriter ){
    int rc;
    sqlite3_stmt *pStmt;

    /* Allocate the SegmentWriter structure */
    pWriter = (SegmentWriter *)sqlite3_malloc(sizeof(SegmentWriter));
    if( !pWriter ) return SQLITE_NOMEM;
    memset(pWriter, 0, sizeof(SegmentWriter));
    *ppWriter = pWriter;

    /* Allocate a buffer in which to accumulate data */
    pWriter->aData = (char *)sqlite3_malloc(p->nNodeSize);
    if( !pWriter->aData ) return SQLITE_NOMEM;
    pWriter->nSize = p->nNodeSize;

    /* Find the next free blockid in the %_segments table */
    rc = fts3SqlStmt(p, SQL_NEXT_SEGMENTS_ID, &pStmt, 0);
    if( rc!=SQLITE_OK ) return rc;
    if( SQLITE_ROW==sqlite3_step(pStmt) ){
      pWriter->iFree = sqlite3_column_int64(pStmt, 0);
      pWriter->iFirst = pWriter->iFree;
    }
    rc = sqlite3_reset(pStmt);
    if( rc!=SQLITE_OK ) return rc;
  }
  nData = pWriter->nData;

  nPrefix = fts3PrefixCompress(pWriter->zTerm, pWriter->nTerm, zTerm, nTerm);
  nSuffix = nTerm-nPrefix;

  /* Figure out how many bytes are required by this new entry */
  nReq = sqlite3Fts3VarintLen(nPrefix) +    /* varint containing prefix size */
    sqlite3Fts3VarintLen(nSuffix) +         /* varint containing suffix size */
    nSuffix +                               /* Term suffix */
    sqlite3Fts3VarintLen(nDoclist) +        /* Size of doclist */
    nDoclist;                               /* Doclist data */

  if( nData>0 && nData+nReq>p->nNodeSize ){
    int rc;

    /* The current leaf node is full. Write it out to the database. */
    rc = fts3WriteSegment(p, pWriter->iFree++, pWriter->aData, nData);
    if( rc!=SQLITE_OK ) return rc;
    p->nLeafAdd++;

    /* Add the current term to the interior node tree. The term added to
    ** the interior tree must:
    **
    **   a) be greater than the largest term on the leaf node just written
    **      to the database (still available in pWriter->zTerm), and
    **
    **   b) be less than or equal to the term about to be added to the new
    **      leaf node (zTerm/nTerm).
    **
    ** In other words, it must be the prefix of zTerm 1 byte longer than
    ** the common prefix (if any) of zTerm and pWriter->zTerm.
    */
    assert( nPrefix<nTerm );
    rc = fts3NodeAddTerm(p, &pWriter->pTree, isCopyTerm, zTerm, nPrefix+1);
    if( rc!=SQLITE_OK ) return rc;

    nData = 0;
    pWriter->nTerm = 0;

    nPrefix = 0;
    nSuffix = nTerm;
    nReq = 1 +                              /* varint containing prefix size */
      sqlite3Fts3VarintLen(nTerm) +         /* varint containing suffix size */
      nTerm +                               /* Term suffix */
      sqlite3Fts3VarintLen(nDoclist) +      /* Size of doclist */
      nDoclist;                             /* Doclist data */
  }

  /* Increase the total number of bytes written to account for the new entry. */
  pWriter->nLeafData += nReq;

  /* If the buffer currently allocated is too small for this entry, realloc
  ** the buffer to make it large enough.
  */
  if( nReq>pWriter->nSize ){
    char *aNew = sqlite3_realloc(pWriter->aData, nReq);
    if( !aNew ) return SQLITE_NOMEM;
    pWriter->aData = aNew;
    pWriter->nSize = nReq;
  }
  assert( nData+nReq<=pWriter->nSize );

  /* Append the prefix-compressed term and doclist to the buffer. */
  nData += sqlite3Fts3PutVarint(&pWriter->aData[nData], nPrefix);
  nData += sqlite3Fts3PutVarint(&pWriter->aData[nData], nSuffix);
  memcpy(&pWriter->aData[nData], &zTerm[nPrefix], nSuffix);
  nData += nSuffix;
  nData += sqlite3Fts3PutVarint(&pWriter->aData[nData], nDoclist);
  memcpy(&pWriter->aData[nData], aDoclist, nDoclist);
  pWriter->nData = nData + nDoclist;

  /* Save the current term so that it can be used to prefix-compress the next.
  ** If the isCopyTerm parameter is true, then the buffer pointed to by
  ** zTerm is transient, so take a copy of the term data. Otherwise, just
  ** store a copy of the pointer.
  */
  if( isCopyTerm ){
    if( nTerm>pWriter->nMalloc ){
      char *zNew = sqlite3_realloc(pWriter->zMalloc, nTerm*2);
      if( !zNew ){
        return SQLITE_NOMEM;
      }
      pWriter->nMalloc = nTerm*2;
      pWriter->zMalloc = zNew;
      pWriter->zTerm = zNew;
    }
    assert( pWriter->zTerm==pWriter->zMalloc );
    memcpy(pWriter->zTerm, zTerm, nTerm);
  }else{
    pWriter->zTerm = (char *)zTerm;
  }
  pWriter->nTerm = nTerm;

  return SQLITE_OK;
}

/*
** Flush all data associated with the SegmentWriter object pWriter to the
** database. This function must be called after all terms have been added
** to the segment using fts3SegWriterAdd(). If successful, SQLITE_OK is
** returned. Otherwise, an SQLite error code.
*/
static int fts3SegWriterFlush(
  Fts3Table *p,                   /* Virtual table handle */
  SegmentWriter *pWriter,         /* SegmentWriter to flush to the db */
  sqlite3_int64 iLevel,           /* Value for 'level' column of %_segdir */
  int iIdx                        /* Value for 'idx' column of %_segdir */
){
  int rc;                         /* Return code */
  if( pWriter->pTree ){
    sqlite3_int64 iLast = 0;      /* Largest block id written to database */
    sqlite3_int64 iLastLeaf;      /* Largest leaf block id written to db */
    char *zRoot = NULL;           /* Pointer to buffer containing root node */
    int nRoot = 0;                /* Size of buffer zRoot */

    iLastLeaf = pWriter->iFree;
    rc = fts3WriteSegment(p, pWriter->iFree++, pWriter->aData, pWriter->nData);
    if( rc==SQLITE_OK ){
      rc = fts3NodeWrite(p, pWriter->pTree, 1,
          pWriter->iFirst, pWriter->iFree, &iLast, &zRoot, &nRoot);
    }
    if( rc==SQLITE_OK ){
      rc = fts3WriteSegdir(p, iLevel, iIdx, 
          pWriter->iFirst, iLastLeaf, iLast, pWriter->nLeafData, zRoot, nRoot);
    }
  }else{
    /* The entire tree fits on the root node. Write it to the segdir table. */
    rc = fts3WriteSegdir(p, iLevel, iIdx, 
        0, 0, 0, pWriter->nLeafData, pWriter->aData, pWriter->nData);
  }
  p->nLeafAdd++;
  return rc;
}

/*
** Release all memory held by the SegmentWriter object passed as the 
** first argument.
*/
static void fts3SegWriterFree(SegmentWriter *pWriter){
  if( pWriter ){
    sqlite3_free(pWriter->aData);
    sqlite3_free(pWriter->zMalloc);
    fts3NodeFree(pWriter->pTree);
    sqlite3_free(pWriter);
  }
}

/*
** The first value in the apVal[] array is assumed to contain an integer.
** This function tests if there exist any documents with docid values that
** are different from that integer. i.e. if deleting the document with docid
** pRowid would mean the FTS3 table were empty.
**
** If successful, *pisEmpty is set to true if the table is empty except for
** document pRowid, or false otherwise, and SQLITE_OK is returned. If an
** error occurs, an SQLite error code is returned.
*/
static int fts3IsEmpty(Fts3Table *p, sqlite3_value *pRowid, int *pisEmpty){
  sqlite3_stmt *pStmt;
  int rc;
  if( p->zContentTbl ){
    /* If using the content=xxx option, assume the table is never empty */
    *pisEmpty = 0;
    rc = SQLITE_OK;
  }else{
    rc = fts3SqlStmt(p, SQL_IS_EMPTY, &pStmt, &pRowid);
    if( rc==SQLITE_OK ){
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        *pisEmpty = sqlite3_column_int(pStmt, 0);
      }
      rc = sqlite3_reset(pStmt);
    }
  }
  return rc;
}

/*
** Set *pnMax to the largest segment level in the database for the index
** iIndex.
**
** Segment levels are stored in the 'level' column of the %_segdir table.
**
** Return SQLITE_OK if successful, or an SQLite error code if not.
*/
static int fts3SegmentMaxLevel(
  Fts3Table *p, 
  int iLangid,
  int iIndex, 
  sqlite3_int64 *pnMax
){
  sqlite3_stmt *pStmt;
  int rc;
  assert( iIndex>=0 && iIndex<p->nIndex );

  /* Set pStmt to the compiled version of:
  **
  **   SELECT max(level) FROM %Q.'%q_segdir' WHERE level BETWEEN ? AND ?
  **
  ** (1024 is actually the value of macro FTS3_SEGDIR_PREFIXLEVEL_STR).
  */
  rc = fts3SqlStmt(p, SQL_SELECT_SEGDIR_MAX_LEVEL, &pStmt, 0);
  if( rc!=SQLITE_OK ) return rc;
  sqlite3_bind_int64(pStmt, 1, getAbsoluteLevel(p, iLangid, iIndex, 0));
  sqlite3_bind_int64(pStmt, 2, 
      getAbsoluteLevel(p, iLangid, iIndex, FTS3_SEGDIR_MAXLEVEL-1)
  );
  if( SQLITE_ROW==sqlite3_step(pStmt) ){
    *pnMax = sqlite3_column_int64(pStmt, 0);
  }
  return sqlite3_reset(pStmt);
}

/*
** iAbsLevel is an absolute level that may be assumed to exist within
** the database. This function checks if it is the largest level number
** within its index. Assuming no error occurs, *pbMax is set to 1 if
** iAbsLevel is indeed the largest level, or 0 otherwise, and SQLITE_OK
** is returned. If an error occurs, an error code is returned and the
** final value of *pbMax is undefined.
*/
static int fts3SegmentIsMaxLevel(Fts3Table *p, i64 iAbsLevel, int *pbMax){

  /* Set pStmt to the compiled version of:
  **
  **   SELECT max(level) FROM %Q.'%q_segdir' WHERE level BETWEEN ? AND ?
  **
  ** (1024 is actually the value of macro FTS3_SEGDIR_PREFIXLEVEL_STR).
  */
  sqlite3_stmt *pStmt;
  int rc = fts3SqlStmt(p, SQL_SELECT_SEGDIR_MAX_LEVEL, &pStmt, 0);
  if( rc!=SQLITE_OK ) return rc;
  sqlite3_bind_int64(pStmt, 1, iAbsLevel+1);
  sqlite3_bind_int64(pStmt, 2, 
      ((iAbsLevel/FTS3_SEGDIR_MAXLEVEL)+1) * FTS3_SEGDIR_MAXLEVEL
  );

  *pbMax = 0;
  if( SQLITE_ROW==sqlite3_step(pStmt) ){
    *pbMax = sqlite3_column_type(pStmt, 0)==SQLITE_NULL;
  }
  return sqlite3_reset(pStmt);
}

/*
** Delete all entries in the %_segments table associated with the segment
** opened with seg-reader pSeg. This function does not affect the contents
** of the %_segdir table.
*/
static int fts3DeleteSegment(
  Fts3Table *p,                   /* FTS table handle */
  Fts3SegReader *pSeg             /* Segment to delete */
){
  int rc = SQLITE_OK;             /* Return code */
  if( pSeg->iStartBlock ){
    sqlite3_stmt *pDelete;        /* SQL statement to delete rows */
    rc = fts3SqlStmt(p, SQL_DELETE_SEGMENTS_RANGE, &pDelete, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int64(pDelete, 1, pSeg->iStartBlock);
      sqlite3_bind_int64(pDelete, 2, pSeg->iEndBlock);
      sqlite3_step(pDelete);
      rc = sqlite3_reset(pDelete);
    }
  }
  return rc;
}

/*
** This function is used after merging multiple segments into a single large
** segment to delete the old, now redundant, segment b-trees. Specifically,
** it:
** 
**   1) Deletes all %_segments entries for the segments associated with 
**      each of the SegReader objects in the array passed as the third 
**      argument, and
**
**   2) deletes all %_segdir entries with level iLevel, or all %_segdir
**      entries regardless of level if (iLevel<0).
**
** SQLITE_OK is returned if successful, otherwise an SQLite error code.
*/
static int fts3DeleteSegdir(
  Fts3Table *p,                   /* Virtual table handle */
  int iLangid,                    /* Language id */
  int iIndex,                     /* Index for p->aIndex */
  int iLevel,                     /* Level of %_segdir entries to delete */
  Fts3SegReader **apSegment,      /* Array of SegReader objects */
  int nReader                     /* Size of array apSegment */
){
  int rc = SQLITE_OK;             /* Return Code */
  int i;                          /* Iterator variable */
  sqlite3_stmt *pDelete = 0;      /* SQL statement to delete rows */

  for(i=0; rc==SQLITE_OK && i<nReader; i++){
    rc = fts3DeleteSegment(p, apSegment[i]);
  }
  if( rc!=SQLITE_OK ){
    return rc;
  }

  assert( iLevel>=0 || iLevel==FTS3_SEGCURSOR_ALL );
  if( iLevel==FTS3_SEGCURSOR_ALL ){
    rc = fts3SqlStmt(p, SQL_DELETE_SEGDIR_RANGE, &pDelete, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int64(pDelete, 1, getAbsoluteLevel(p, iLangid, iIndex, 0));
      sqlite3_bind_int64(pDelete, 2, 
          getAbsoluteLevel(p, iLangid, iIndex, FTS3_SEGDIR_MAXLEVEL-1)
      );
    }
  }else{
    rc = fts3SqlStmt(p, SQL_DELETE_SEGDIR_LEVEL, &pDelete, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int64(
          pDelete, 1, getAbsoluteLevel(p, iLangid, iIndex, iLevel)
      );
    }
  }

  if( rc==SQLITE_OK ){
    sqlite3_step(pDelete);
    rc = sqlite3_reset(pDelete);
  }

  return rc;
}

/*
** When this function is called, buffer *ppList (size *pnList bytes) contains 
** a position list that may (or may not) feature multiple columns. This
** function adjusts the pointer *ppList and the length *pnList so that they
** identify the subset of the position list that corresponds to column iCol.
**
** If there are no entries in the input position list for column iCol, then
** *pnList is set to zero before returning.
**
** If parameter bZero is non-zero, then any part of the input list following
** the end of the output list is zeroed before returning.
*/
static void fts3ColumnFilter(
  int iCol,                       /* Column to filter on */
  int bZero,                      /* Zero out anything following *ppList */
  char **ppList,                  /* IN/OUT: Pointer to position list */
  int *pnList                     /* IN/OUT: Size of buffer *ppList in bytes */
){
  char *pList = *ppList;
  int nList = *pnList;
  char *pEnd = &pList[nList];
  int iCurrent = 0;
  char *p = pList;

  assert( iCol>=0 );
  while( 1 ){
    char c = 0;
    while( p<pEnd && (c | *p)&0xFE ) c = *p++ & 0x80;
  
    if( iCol==iCurrent ){
      nList = (int)(p - pList);
      break;
    }

    nList -= (int)(p - pList);
    pList = p;
    if( nList==0 ){
      break;
    }
    p = &pList[1];
    p += fts3GetVarint32(p, &iCurrent);
  }

  if( bZero && &pList[nList]!=pEnd ){
    memset(&pList[nList], 0, pEnd - &pList[nList]);
  }
  *ppList = pList;
  *pnList = nList;
}

/*
** Cache data in the Fts3MultiSegReader.aBuffer[] buffer (overwriting any
** existing data). Grow the buffer if required.
**
** If successful, return SQLITE_OK. Otherwise, if an OOM error is encountered
** trying to resize the buffer, return SQLITE_NOMEM.
*/
static int fts3MsrBufferData(
  Fts3MultiSegReader *pMsr,       /* Multi-segment-reader handle */
  char *pList,
  int nList
){
  if( nList>pMsr->nBuffer ){
    char *pNew;
    pMsr->nBuffer = nList*2;
    pNew = (char *)sqlite3_realloc(pMsr->aBuffer, pMsr->nBuffer);
    if( !pNew ) return SQLITE_NOMEM;
    pMsr->aBuffer = pNew;
  }

  memcpy(pMsr->aBuffer, pList, nList);
  return SQLITE_OK;
}

SQLITE_PRIVATE int sqlite3Fts3MsrIncrNext(
  Fts3Table *p,                   /* Virtual table handle */
  Fts3MultiSegReader *pMsr,       /* Multi-segment-reader handle */
  sqlite3_int64 *piDocid,         /* OUT: Docid value */
  char **paPoslist,               /* OUT: Pointer to position list */
  int *pnPoslist                  /* OUT: Size of position list in bytes */
){
  int nMerge = pMsr->nAdvance;
  Fts3SegReader **apSegment = pMsr->apSegment;
  int (*xCmp)(Fts3SegReader *, Fts3SegReader *) = (
    p->bDescIdx ? fts3SegReaderDoclistCmpRev : fts3SegReaderDoclistCmp
  );

  if( nMerge==0 ){
    *paPoslist = 0;
    return SQLITE_OK;
  }

  while( 1 ){
    Fts3SegReader *pSeg;
    pSeg = pMsr->apSegment[0];

    if( pSeg->pOffsetList==0 ){
      *paPoslist = 0;
      break;
    }else{
      int rc;
      char *pList;
      int nList;
      int j;
      sqlite3_int64 iDocid = apSegment[0]->iDocid;

      rc = fts3SegReaderNextDocid(p, apSegment[0], &pList, &nList);
      j = 1;
      while( rc==SQLITE_OK 
        && j<nMerge
        && apSegment[j]->pOffsetList
        && apSegment[j]->iDocid==iDocid
      ){
        rc = fts3SegReaderNextDocid(p, apSegment[j], 0, 0);
        j++;
      }
      if( rc!=SQLITE_OK ) return rc;
      fts3SegReaderSort(pMsr->apSegment, nMerge, j, xCmp);

      if( nList>0 && fts3SegReaderIsPending(apSegment[0]) ){
        rc = fts3MsrBufferData(pMsr, pList, nList+1);
        if( rc!=SQLITE_OK ) return rc;
        assert( (pMsr->aBuffer[nList] & 0xFE)==0x00 );
        pList = pMsr->aBuffer;
      }

      if( pMsr->iColFilter>=0 ){
        fts3ColumnFilter(pMsr->iColFilter, 1, &pList, &nList);
      }

      if( nList>0 ){
        *paPoslist = pList;
        *piDocid = iDocid;
        *pnPoslist = nList;
        break;
      }
    }
  }

  return SQLITE_OK;
}

static int fts3SegReaderStart(
  Fts3Table *p,                   /* Virtual table handle */
  Fts3MultiSegReader *pCsr,       /* Cursor object */
  const char *zTerm,              /* Term searched for (or NULL) */
  int nTerm                       /* Length of zTerm in bytes */
){
  int i;
  int nSeg = pCsr->nSegment;

  /* If the Fts3SegFilter defines a specific term (or term prefix) to search 
  ** for, then advance each segment iterator until it points to a term of
  ** equal or greater value than the specified term. This prevents many
  ** unnecessary merge/sort operations for the case where single segment
  ** b-tree leaf nodes contain more than one term.
  */
  for(i=0; pCsr->bRestart==0 && i<pCsr->nSegment; i++){
    int res = 0;
    Fts3SegReader *pSeg = pCsr->apSegment[i];
    do {
      int rc = fts3SegReaderNext(p, pSeg, 0);
      if( rc!=SQLITE_OK ) return rc;
    }while( zTerm && (res = fts3SegReaderTermCmp(pSeg, zTerm, nTerm))<0 );

    if( pSeg->bLookup && res!=0 ){
      fts3SegReaderSetEof(pSeg);
    }
  }
  fts3SegReaderSort(pCsr->apSegment, nSeg, nSeg, fts3SegReaderCmp);

  return SQLITE_OK;
}

SQLITE_PRIVATE int sqlite3Fts3SegReaderStart(
  Fts3Table *p,                   /* Virtual table handle */
  Fts3MultiSegReader *pCsr,       /* Cursor object */
  Fts3SegFilter *pFilter          /* Restrictions on range of iteration */
){
  pCsr->pFilter = pFilter;
  return fts3SegReaderStart(p, pCsr, pFilter->zTerm, pFilter->nTerm);
}

SQLITE_PRIVATE int sqlite3Fts3MsrIncrStart(
  Fts3Table *p,                   /* Virtual table handle */
  Fts3MultiSegReader *pCsr,       /* Cursor object */
  int iCol,                       /* Column to match on. */
  const char *zTerm,              /* Term to iterate through a doclist for */
  int nTerm                       /* Number of bytes in zTerm */
){
  int i;
  int rc;
  int nSegment = pCsr->nSegment;
  int (*xCmp)(Fts3SegReader *, Fts3SegReader *) = (
    p->bDescIdx ? fts3SegReaderDoclistCmpRev : fts3SegReaderDoclistCmp
  );

  assert( pCsr->pFilter==0 );
  assert( zTerm && nTerm>0 );

  /* Advance each segment iterator until it points to the term zTerm/nTerm. */
  rc = fts3SegReaderStart(p, pCsr, zTerm, nTerm);
  if( rc!=SQLITE_OK ) return rc;

  /* Determine how many of the segments actually point to zTerm/nTerm. */
  for(i=0; i<nSegment; i++){
    Fts3SegReader *pSeg = pCsr->apSegment[i];
    if( !pSeg->aNode || fts3SegReaderTermCmp(pSeg, zTerm, nTerm) ){
      break;
    }
  }
  pCsr->nAdvance = i;

  /* Advance each of the segments to point to the first docid. */
  for(i=0; i<pCsr->nAdvance; i++){
    rc = fts3SegReaderFirstDocid(p, pCsr->apSegment[i]);
    if( rc!=SQLITE_OK ) return rc;
  }
  fts3SegReaderSort(pCsr->apSegment, i, i, xCmp);

  assert( iCol<0 || iCol<p->nColumn );
  pCsr->iColFilter = iCol;

  return SQLITE_OK;
}

/*
** This function is called on a MultiSegReader that has been started using
** sqlite3Fts3MsrIncrStart(). One or more calls to MsrIncrNext() may also
** have been made. Calling this function puts the MultiSegReader in such
** a state that if the next two calls are:
**
**   sqlite3Fts3SegReaderStart()
**   sqlite3Fts3SegReaderStep()
**
** then the entire doclist for the term is available in 
** MultiSegReader.aDoclist/nDoclist.
*/
SQLITE_PRIVATE int sqlite3Fts3MsrIncrRestart(Fts3MultiSegReader *pCsr){
  int i;                          /* Used to iterate through segment-readers */

  assert( pCsr->zTerm==0 );
  assert( pCsr->nTerm==0 );
  assert( pCsr->aDoclist==0 );
  assert( pCsr->nDoclist==0 );

  pCsr->nAdvance = 0;
  pCsr->bRestart = 1;
  for(i=0; i<pCsr->nSegment; i++){
    pCsr->apSegment[i]->pOffsetList = 0;
    pCsr->apSegment[i]->nOffsetList = 0;
    pCsr->apSegment[i]->iDocid = 0;
  }

  return SQLITE_OK;
}


SQLITE_PRIVATE int sqlite3Fts3SegReaderStep(
  Fts3Table *p,                   /* Virtual table handle */
  Fts3MultiSegReader *pCsr        /* Cursor object */
){
  int rc = SQLITE_OK;

  int isIgnoreEmpty =  (pCsr->pFilter->flags & FTS3_SEGMENT_IGNORE_EMPTY);
  int isRequirePos =   (pCsr->pFilter->flags & FTS3_SEGMENT_REQUIRE_POS);
  int isColFilter =    (pCsr->pFilter->flags & FTS3_SEGMENT_COLUMN_FILTER);
  int isPrefix =       (pCsr->pFilter->flags & FTS3_SEGMENT_PREFIX);
  int isScan =         (pCsr->pFilter->flags & FTS3_SEGMENT_SCAN);
  int isFirst =        (pCsr->pFilter->flags & FTS3_SEGMENT_FIRST);

  Fts3SegReader **apSegment = pCsr->apSegment;
  int nSegment = pCsr->nSegment;
  Fts3SegFilter *pFilter = pCsr->pFilter;
  int (*xCmp)(Fts3SegReader *, Fts3SegReader *) = (
    p->bDescIdx ? fts3SegReaderDoclistCmpRev : fts3SegReaderDoclistCmp
  );

  if( pCsr->nSegment==0 ) return SQLITE_OK;

  do {
    int nMerge;
    int i;
  
    /* Advance the first pCsr->nAdvance entries in the apSegment[] array
    ** forward. Then sort the list in order of current term again.  
    */
    for(i=0; i<pCsr->nAdvance; i++){
      Fts3SegReader *pSeg = apSegment[i];
      if( pSeg->bLookup ){
        fts3SegReaderSetEof(pSeg);
      }else{
        rc = fts3SegReaderNext(p, pSeg, 0);
      }
      if( rc!=SQLITE_OK ) return rc;
    }
    fts3SegReaderSort(apSegment, nSegment, pCsr->nAdvance, fts3SegReaderCmp);
    pCsr->nAdvance = 0;

    /* If all the seg-readers are at EOF, we're finished. return SQLITE_OK. */
    assert( rc==SQLITE_OK );
    if( apSegment[0]->aNode==0 ) break;

    pCsr->nTerm = apSegment[0]->nTerm;
    pCsr->zTerm = apSegment[0]->zTerm;

    /* If this is a prefix-search, and if the term that apSegment[0] points
    ** to does not share a suffix with pFilter->zTerm/nTerm, then all 
    ** required callbacks have been made. In this case exit early.
    **
    ** Similarly, if this is a search for an exact match, and the first term
    ** of segment apSegment[0] is not a match, exit early.
    */
    if( pFilter->zTerm && !isScan ){
      if( pCsr->nTerm<pFilter->nTerm 
       || (!isPrefix && pCsr->nTerm>pFilter->nTerm)
       || memcmp(pCsr->zTerm, pFilter->zTerm, pFilter->nTerm) 
      ){
        break;
      }
    }

    nMerge = 1;
    while( nMerge<nSegment 
        && apSegment[nMerge]->aNode
        && apSegment[nMerge]->nTerm==pCsr->nTerm 
        && 0==memcmp(pCsr->zTerm, apSegment[nMerge]->zTerm, pCsr->nTerm)
    ){
      nMerge++;
    }

    assert( isIgnoreEmpty || (isRequirePos && !isColFilter) );
    if( nMerge==1 
     && !isIgnoreEmpty 
     && !isFirst 
     && (p->bDescIdx==0 || fts3SegReaderIsPending(apSegment[0])==0)
    ){
      pCsr->nDoclist = apSegment[0]->nDoclist;
      if( fts3SegReaderIsPending(apSegment[0]) ){
        rc = fts3MsrBufferData(pCsr, apSegment[0]->aDoclist, pCsr->nDoclist);
        pCsr->aDoclist = pCsr->aBuffer;
      }else{
        pCsr->aDoclist = apSegment[0]->aDoclist;
      }
      if( rc==SQLITE_OK ) rc = SQLITE_ROW;
    }else{
      int nDoclist = 0;           /* Size of doclist */
      sqlite3_int64 iPrev = 0;    /* Previous docid stored in doclist */

      /* The current term of the first nMerge entries in the array
      ** of Fts3SegReader objects is the same. The doclists must be merged
      ** and a single term returned with the merged doclist.
      */
      for(i=0; i<nMerge; i++){
        fts3SegReaderFirstDocid(p, apSegment[i]);
      }
      fts3SegReaderSort(apSegment, nMerge, nMerge, xCmp);
      while( apSegment[0]->pOffsetList ){
        int j;                    /* Number of segments that share a docid */
        char *pList = 0;
        int nList = 0;
        int nByte;
        sqlite3_int64 iDocid = apSegment[0]->iDocid;
        fts3SegReaderNextDocid(p, apSegment[0], &pList, &nList);
        j = 1;
        while( j<nMerge
            && apSegment[j]->pOffsetList
            && apSegment[j]->iDocid==iDocid
        ){
          fts3SegReaderNextDocid(p, apSegment[j], 0, 0);
          j++;
        }

        if( isColFilter ){
          fts3ColumnFilter(pFilter->iCol, 0, &pList, &nList);
        }

        if( !isIgnoreEmpty || nList>0 ){

          /* Calculate the 'docid' delta value to write into the merged 
          ** doclist. */
          sqlite3_int64 iDelta;
          if( p->bDescIdx && nDoclist>0 ){
            iDelta = iPrev - iDocid;
          }else{
            iDelta = iDocid - iPrev;
          }
          assert( iDelta>0 || (nDoclist==0 && iDelta==iDocid) );
          assert( nDoclist>0 || iDelta==iDocid );

          nByte = sqlite3Fts3VarintLen(iDelta) + (isRequirePos?nList+1:0);
          if( nDoclist+nByte>pCsr->nBuffer ){
            char *aNew;
            pCsr->nBuffer = (nDoclist+nByte)*2;
            aNew = sqlite3_realloc(pCsr->aBuffer, pCsr->nBuffer);
            if( !aNew ){
              return SQLITE_NOMEM;
            }
            pCsr->aBuffer = aNew;
          }

          if( isFirst ){
            char *a = &pCsr->aBuffer[nDoclist];
            int nWrite;
           
            nWrite = sqlite3Fts3FirstFilter(iDelta, pList, nList, a);
            if( nWrite ){
              iPrev = iDocid;
              nDoclist += nWrite;
            }
          }else{
            nDoclist += sqlite3Fts3PutVarint(&pCsr->aBuffer[nDoclist], iDelta);
            iPrev = iDocid;
            if( isRequirePos ){
              memcpy(&pCsr->aBuffer[nDoclist], pList, nList);
              nDoclist += nList;
              pCsr->aBuffer[nDoclist++] = '\0';
            }
          }
        }

        fts3SegReaderSort(apSegment, nMerge, j, xCmp);
      }
      if( nDoclist>0 ){
        pCsr->aDoclist = pCsr->aBuffer;
        pCsr->nDoclist = nDoclist;
        rc = SQLITE_ROW;
      }
    }
    pCsr->nAdvance = nMerge;
  }while( rc==SQLITE_OK );

  return rc;
}


SQLITE_PRIVATE void sqlite3Fts3SegReaderFinish(
  Fts3MultiSegReader *pCsr       /* Cursor object */
){
  if( pCsr ){
    int i;
    for(i=0; i<pCsr->nSegment; i++){
      sqlite3Fts3SegReaderFree(pCsr->apSegment[i]);
    }
    sqlite3_free(pCsr->apSegment);
    sqlite3_free(pCsr->aBuffer);

    pCsr->nSegment = 0;
    pCsr->apSegment = 0;
    pCsr->aBuffer = 0;
  }
}

/*
** Decode the "end_block" field, selected by column iCol of the SELECT 
** statement passed as the first argument. 
**
** The "end_block" field may contain either an integer, or a text field
** containing the text representation of two non-negative integers separated 
** by one or more space (0x20) characters. In the first case, set *piEndBlock 
** to the integer value and *pnByte to zero before returning. In the second, 
** set *piEndBlock to the first value and *pnByte to the second.
*/
static void fts3ReadEndBlockField(
  sqlite3_stmt *pStmt, 
  int iCol, 
  i64 *piEndBlock,
  i64 *pnByte
){
  const unsigned char *zText = sqlite3_column_text(pStmt, iCol);
  if( zText ){
    int i;
    int iMul = 1;
    i64 iVal = 0;
    for(i=0; zText[i]>='0' && zText[i]<='9'; i++){
      iVal = iVal*10 + (zText[i] - '0');
    }
    *piEndBlock = iVal;
    while( zText[i]==' ' ) i++;
    iVal = 0;
    if( zText[i]=='-' ){
      i++;
      iMul = -1;
    }
    for(/* no-op */; zText[i]>='0' && zText[i]<='9'; i++){
      iVal = iVal*10 + (zText[i] - '0');
    }
    *pnByte = (iVal * (i64)iMul);
  }
}


/*
** A segment of size nByte bytes has just been written to absolute level
** iAbsLevel. Promote any segments that should be promoted as a result.
*/
static int fts3PromoteSegments(
  Fts3Table *p,                   /* FTS table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level just updated */
  sqlite3_int64 nByte             /* Size of new segment at iAbsLevel */
){
  int rc = SQLITE_OK;
  sqlite3_stmt *pRange;

  rc = fts3SqlStmt(p, SQL_SELECT_LEVEL_RANGE2, &pRange, 0);

  if( rc==SQLITE_OK ){
    int bOk = 0;
    i64 iLast = (iAbsLevel/FTS3_SEGDIR_MAXLEVEL + 1) * FTS3_SEGDIR_MAXLEVEL - 1;
    i64 nLimit = (nByte*3)/2;

    /* Loop through all entries in the %_segdir table corresponding to 
    ** segments in this index on levels greater than iAbsLevel. If there is
    ** at least one such segment, and it is possible to determine that all 
    ** such segments are smaller than nLimit bytes in size, they will be 
    ** promoted to level iAbsLevel.  */
    sqlite3_bind_int64(pRange, 1, iAbsLevel+1);
    sqlite3_bind_int64(pRange, 2, iLast);
    while( SQLITE_ROW==sqlite3_step(pRange) ){
      i64 nSize = 0, dummy;
      fts3ReadEndBlockField(pRange, 2, &dummy, &nSize);
      if( nSize<=0 || nSize>nLimit ){
        /* If nSize==0, then the %_segdir.end_block field does not not 
        ** contain a size value. This happens if it was written by an
        ** old version of FTS. In this case it is not possible to determine
        ** the size of the segment, and so segment promotion does not
        ** take place.  */
        bOk = 0;
        break;
      }
      bOk = 1;
    }
    rc = sqlite3_reset(pRange);

    if( bOk ){
      int iIdx = 0;
      sqlite3_stmt *pUpdate1 = 0;
      sqlite3_stmt *pUpdate2 = 0;

      if( rc==SQLITE_OK ){
        rc = fts3SqlStmt(p, SQL_UPDATE_LEVEL_IDX, &pUpdate1, 0);
      }
      if( rc==SQLITE_OK ){
        rc = fts3SqlStmt(p, SQL_UPDATE_LEVEL, &pUpdate2, 0);
      }

      if( rc==SQLITE_OK ){

        /* Loop through all %_segdir entries for segments in this index with
        ** levels equal to or greater than iAbsLevel. As each entry is visited,
        ** updated it to set (level = -1) and (idx = N), where N is 0 for the
        ** oldest segment in the range, 1 for the next oldest, and so on.
        **
        ** In other words, move all segments being promoted to level -1,
        ** setting the "idx" fields as appropriate to keep them in the same
        ** order. The contents of level -1 (which is never used, except
        ** transiently here), will be moved back to level iAbsLevel below.  */
        sqlite3_bind_int64(pRange, 1, iAbsLevel);
        while( SQLITE_ROW==sqlite3_step(pRange) ){
          sqlite3_bind_int(pUpdate1, 1, iIdx++);
          sqlite3_bind_int(pUpdate1, 2, sqlite3_column_int(pRange, 0));
          sqlite3_bind_int(pUpdate1, 3, sqlite3_column_int(pRange, 1));
          sqlite3_step(pUpdate1);
          rc = sqlite3_reset(pUpdate1);
          if( rc!=SQLITE_OK ){
            sqlite3_reset(pRange);
            break;
          }
        }
      }
      if( rc==SQLITE_OK ){
        rc = sqlite3_reset(pRange);
      }

      /* Move level -1 to level iAbsLevel */
      if( rc==SQLITE_OK ){
        sqlite3_bind_int64(pUpdate2, 1, iAbsLevel);
        sqlite3_step(pUpdate2);
        rc = sqlite3_reset(pUpdate2);
      }
    }
  }


  return rc;
}

/*
** Merge all level iLevel segments in the database into a single 
** iLevel+1 segment. Or, if iLevel<0, merge all segments into a
** single segment with a level equal to the numerically largest level 
** currently present in the database.
**
** If this function is called with iLevel<0, but there is only one
** segment in the database, SQLITE_DONE is returned immediately. 
** Otherwise, if successful, SQLITE_OK is returned. If an error occurs, 
** an SQLite error code is returned.
*/
static int fts3SegmentMerge(
  Fts3Table *p, 
  int iLangid,                    /* Language id to merge */
  int iIndex,                     /* Index in p->aIndex[] to merge */
  int iLevel                      /* Level to merge */
){
  int rc;                         /* Return code */
  int iIdx = 0;                   /* Index of new segment */
  sqlite3_int64 iNewLevel = 0;    /* Level/index to create new segment at */
  SegmentWriter *pWriter = 0;     /* Used to write the new, merged, segment */
  Fts3SegFilter filter;           /* Segment term filter condition */
  Fts3MultiSegReader csr;         /* Cursor to iterate through level(s) */
  int bIgnoreEmpty = 0;           /* True to ignore empty segments */
  i64 iMaxLevel = 0;              /* Max level number for this index/langid */

  assert( iLevel==FTS3_SEGCURSOR_ALL
       || iLevel==FTS3_SEGCURSOR_PENDING
       || iLevel>=0
  );
  assert( iLevel<FTS3_SEGDIR_MAXLEVEL );
  assert( iIndex>=0 && iIndex<p->nIndex );

  rc = sqlite3Fts3SegReaderCursor(p, iLangid, iIndex, iLevel, 0, 0, 1, 0, &csr);
  if( rc!=SQLITE_OK || csr.nSegment==0 ) goto finished;

  if( iLevel!=FTS3_SEGCURSOR_PENDING ){
    rc = fts3SegmentMaxLevel(p, iLangid, iIndex, &iMaxLevel);
    if( rc!=SQLITE_OK ) goto finished;
  }

  if( iLevel==FTS3_SEGCURSOR_ALL ){
    /* This call is to merge all segments in the database to a single
    ** segment. The level of the new segment is equal to the numerically
    ** greatest segment level currently present in the database for this
    ** index. The idx of the new segment is always 0.  */
    if( csr.nSegment==1 ){
      rc = SQLITE_DONE;
      goto finished;
    }
    iNewLevel = iMaxLevel;
    bIgnoreEmpty = 1;

  }else{
    /* This call is to merge all segments at level iLevel. find the next
    ** available segment index at level iLevel+1. The call to
    ** fts3AllocateSegdirIdx() will merge the segments at level iLevel+1 to 
    ** a single iLevel+2 segment if necessary.  */
    assert( FTS3_SEGCURSOR_PENDING==-1 );
    iNewLevel = getAbsoluteLevel(p, iLangid, iIndex, iLevel+1);
    rc = fts3AllocateSegdirIdx(p, iLangid, iIndex, iLevel+1, &iIdx);
    bIgnoreEmpty = (iLevel!=FTS3_SEGCURSOR_PENDING) && (iNewLevel>iMaxLevel);
  }
  if( rc!=SQLITE_OK ) goto finished;

  assert( csr.nSegment>0 );
  assert( iNewLevel>=getAbsoluteLevel(p, iLangid, iIndex, 0) );
  assert( iNewLevel<getAbsoluteLevel(p, iLangid, iIndex,FTS3_SEGDIR_MAXLEVEL) );

  memset(&filter, 0, sizeof(Fts3SegFilter));
  filter.flags = FTS3_SEGMENT_REQUIRE_POS;
  filter.flags |= (bIgnoreEmpty ? FTS3_SEGMENT_IGNORE_EMPTY : 0);

  rc = sqlite3Fts3SegReaderStart(p, &csr, &filter);
  while( SQLITE_OK==rc ){
    rc = sqlite3Fts3SegReaderStep(p, &csr);
    if( rc!=SQLITE_ROW ) break;
    rc = fts3SegWriterAdd(p, &pWriter, 1, 
        csr.zTerm, csr.nTerm, csr.aDoclist, csr.nDoclist);
  }
  if( rc!=SQLITE_OK ) goto finished;
  assert( pWriter || bIgnoreEmpty );

  if( iLevel!=FTS3_SEGCURSOR_PENDING ){
    rc = fts3DeleteSegdir(
        p, iLangid, iIndex, iLevel, csr.apSegment, csr.nSegment
    );
    if( rc!=SQLITE_OK ) goto finished;
  }
  if( pWriter ){
    rc = fts3SegWriterFlush(p, pWriter, iNewLevel, iIdx);
    if( rc==SQLITE_OK ){
      if( iLevel==FTS3_SEGCURSOR_PENDING || iNewLevel<iMaxLevel ){
        rc = fts3PromoteSegments(p, iNewLevel, pWriter->nLeafData);
      }
    }
  }

 finished:
  fts3SegWriterFree(pWriter);
  sqlite3Fts3SegReaderFinish(&csr);
  return rc;
}


/* 
** Flush the contents of pendingTerms to level 0 segments. 
*/
SQLITE_PRIVATE int sqlite3Fts3PendingTermsFlush(Fts3Table *p){
  int rc = SQLITE_OK;
  int i;
        
  for(i=0; rc==SQLITE_OK && i<p->nIndex; i++){
    rc = fts3SegmentMerge(p, p->iPrevLangid, i, FTS3_SEGCURSOR_PENDING);
    if( rc==SQLITE_DONE ) rc = SQLITE_OK;
  }
  sqlite3Fts3PendingTermsClear(p);

  /* Determine the auto-incr-merge setting if unknown.  If enabled,
  ** estimate the number of leaf blocks of content to be written
  */
  if( rc==SQLITE_OK && p->bHasStat
   && p->nAutoincrmerge==0xff && p->nLeafAdd>0
  ){
    sqlite3_stmt *pStmt = 0;
    rc = fts3SqlStmt(p, SQL_SELECT_STAT, &pStmt, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int(pStmt, 1, FTS_STAT_AUTOINCRMERGE);
      rc = sqlite3_step(pStmt);
      if( rc==SQLITE_ROW ){
        p->nAutoincrmerge = sqlite3_column_int(pStmt, 0);
        if( p->nAutoincrmerge==1 ) p->nAutoincrmerge = 8;
      }else if( rc==SQLITE_DONE ){
        p->nAutoincrmerge = 0;
      }
      rc = sqlite3_reset(pStmt);
    }
  }
  return rc;
}

/*
** Encode N integers as varints into a blob.
*/
static void fts3EncodeIntArray(
  int N,             /* The number of integers to encode */
  u32 *a,            /* The integer values */
  char *zBuf,        /* Write the BLOB here */
  int *pNBuf         /* Write number of bytes if zBuf[] used here */
){
  int i, j;
  for(i=j=0; i<N; i++){
    j += sqlite3Fts3PutVarint(&zBuf[j], (sqlite3_int64)a[i]);
  }
  *pNBuf = j;
}

/*
** Decode a blob of varints into N integers
*/
static void fts3DecodeIntArray(
  int N,             /* The number of integers to decode */
  u32 *a,            /* Write the integer values */
  const char *zBuf,  /* The BLOB containing the varints */
  int nBuf           /* size of the BLOB */
){
  int i, j;
  UNUSED_PARAMETER(nBuf);
  for(i=j=0; i<N; i++){
    sqlite3_int64 x;
    j += sqlite3Fts3GetVarint(&zBuf[j], &x);
    assert(j<=nBuf);
    a[i] = (u32)(x & 0xffffffff);
  }
}

/*
** Insert the sizes (in tokens) for each column of the document
** with docid equal to p->iPrevDocid.  The sizes are encoded as
** a blob of varints.
*/
static void fts3InsertDocsize(
  int *pRC,                       /* Result code */
  Fts3Table *p,                   /* Table into which to insert */
  u32 *aSz                        /* Sizes of each column, in tokens */
){
  char *pBlob;             /* The BLOB encoding of the document size */
  int nBlob;               /* Number of bytes in the BLOB */
  sqlite3_stmt *pStmt;     /* Statement used to insert the encoding */
  int rc;                  /* Result code from subfunctions */

  if( *pRC ) return;
  pBlob = sqlite3_malloc( 10*p->nColumn );
  if( pBlob==0 ){
    *pRC = SQLITE_NOMEM;
    return;
  }
  fts3EncodeIntArray(p->nColumn, aSz, pBlob, &nBlob);
  rc = fts3SqlStmt(p, SQL_REPLACE_DOCSIZE, &pStmt, 0);
  if( rc ){
    sqlite3_free(pBlob);
    *pRC = rc;
    return;
  }
  sqlite3_bind_int64(pStmt, 1, p->iPrevDocid);
  sqlite3_bind_blob(pStmt, 2, pBlob, nBlob, sqlite3_free);
  sqlite3_step(pStmt);
  *pRC = sqlite3_reset(pStmt);
}

/*
** Record 0 of the %_stat table contains a blob consisting of N varints,
** where N is the number of user defined columns in the fts3 table plus
** two. If nCol is the number of user defined columns, then values of the 
** varints are set as follows:
**
**   Varint 0:       Total number of rows in the table.
**
**   Varint 1..nCol: For each column, the total number of tokens stored in
**                   the column for all rows of the table.
**
**   Varint 1+nCol:  The total size, in bytes, of all text values in all
**                   columns of all rows of the table.
**
*/
static void fts3UpdateDocTotals(
  int *pRC,                       /* The result code */
  Fts3Table *p,                   /* Table being updated */
  u32 *aSzIns,                    /* Size increases */
  u32 *aSzDel,                    /* Size decreases */
  int nChng                       /* Change in the number of documents */
){
  char *pBlob;             /* Storage for BLOB written into %_stat */
  int nBlob;               /* Size of BLOB written into %_stat */
  u32 *a;                  /* Array of integers that becomes the BLOB */
  sqlite3_stmt *pStmt;     /* Statement for reading and writing */
  int i;                   /* Loop counter */
  int rc;                  /* Result code from subfunctions */

  const int nStat = p->nColumn+2;

  if( *pRC ) return;
  a = sqlite3_malloc( (sizeof(u32)+10)*nStat );
  if( a==0 ){
    *pRC = SQLITE_NOMEM;
    return;
  }
  pBlob = (char*)&a[nStat];
  rc = fts3SqlStmt(p, SQL_SELECT_STAT, &pStmt, 0);
  if( rc ){
    sqlite3_free(a);
    *pRC = rc;
    return;
  }
  sqlite3_bind_int(pStmt, 1, FTS_STAT_DOCTOTAL);
  if( sqlite3_step(pStmt)==SQLITE_ROW ){
    fts3DecodeIntArray(nStat, a,
         sqlite3_column_blob(pStmt, 0),
         sqlite3_column_bytes(pStmt, 0));
  }else{
    memset(a, 0, sizeof(u32)*(nStat) );
  }
  rc = sqlite3_reset(pStmt);
  if( rc!=SQLITE_OK ){
    sqlite3_free(a);
    *pRC = rc;
    return;
  }
  if( nChng<0 && a[0]<(u32)(-nChng) ){
    a[0] = 0;
  }else{
    a[0] += nChng;
  }
  for(i=0; i<p->nColumn+1; i++){
    u32 x = a[i+1];
    if( x+aSzIns[i] < aSzDel[i] ){
      x = 0;
    }else{
      x = x + aSzIns[i] - aSzDel[i];
    }
    a[i+1] = x;
  }
  fts3EncodeIntArray(nStat, a, pBlob, &nBlob);
  rc = fts3SqlStmt(p, SQL_REPLACE_STAT, &pStmt, 0);
  if( rc ){
    sqlite3_free(a);
    *pRC = rc;
    return;
  }
  sqlite3_bind_int(pStmt, 1, FTS_STAT_DOCTOTAL);
  sqlite3_bind_blob(pStmt, 2, pBlob, nBlob, SQLITE_STATIC);
  sqlite3_step(pStmt);
  *pRC = sqlite3_reset(pStmt);
  sqlite3_free(a);
}

/*
** Merge the entire database so that there is one segment for each 
** iIndex/iLangid combination.
*/
static int fts3DoOptimize(Fts3Table *p, int bReturnDone){
  int bSeenDone = 0;
  int rc;
  sqlite3_stmt *pAllLangid = 0;

  rc = fts3SqlStmt(p, SQL_SELECT_ALL_LANGID, &pAllLangid, 0);
  if( rc==SQLITE_OK ){
    int rc2;
    sqlite3_bind_int(pAllLangid, 1, p->iPrevLangid);
    sqlite3_bind_int(pAllLangid, 2, p->nIndex);
    while( sqlite3_step(pAllLangid)==SQLITE_ROW ){
      int i;
      int iLangid = sqlite3_column_int(pAllLangid, 0);
      for(i=0; rc==SQLITE_OK && i<p->nIndex; i++){
        rc = fts3SegmentMerge(p, iLangid, i, FTS3_SEGCURSOR_ALL);
        if( rc==SQLITE_DONE ){
          bSeenDone = 1;
          rc = SQLITE_OK;
        }
      }
    }
    rc2 = sqlite3_reset(pAllLangid);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  sqlite3Fts3SegmentsClose(p);
  sqlite3Fts3PendingTermsClear(p);

  return (rc==SQLITE_OK && bReturnDone && bSeenDone) ? SQLITE_DONE : rc;
}

/*
** This function is called when the user executes the following statement:
**
**     INSERT INTO <tbl>(<tbl>) VALUES('rebuild');
**
** The entire FTS index is discarded and rebuilt. If the table is one 
** created using the content=xxx option, then the new index is based on
** the current contents of the xxx table. Otherwise, it is rebuilt based
** on the contents of the %_content table.
*/
static int fts3DoRebuild(Fts3Table *p){
  int rc;                         /* Return Code */

  rc = fts3DeleteAll(p, 0);
  if( rc==SQLITE_OK ){
    u32 *aSz = 0;
    u32 *aSzIns = 0;
    u32 *aSzDel = 0;
    sqlite3_stmt *pStmt = 0;
    int nEntry = 0;

    /* Compose and prepare an SQL statement to loop through the content table */
    char *zSql = sqlite3_mprintf("SELECT %s" , p->zReadExprlist);
    if( !zSql ){
      rc = SQLITE_NOMEM;
    }else{
      rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
      sqlite3_free(zSql);
    }

    if( rc==SQLITE_OK ){
      int nByte = sizeof(u32) * (p->nColumn+1)*3;
      aSz = (u32 *)sqlite3_malloc(nByte);
      if( aSz==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memset(aSz, 0, nByte);
        aSzIns = &aSz[p->nColumn+1];
        aSzDel = &aSzIns[p->nColumn+1];
      }
    }

    while( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
      int iCol;
      int iLangid = langidFromSelect(p, pStmt);
      rc = fts3PendingTermsDocid(p, iLangid, sqlite3_column_int64(pStmt, 0));
      memset(aSz, 0, sizeof(aSz[0]) * (p->nColumn+1));
      for(iCol=0; rc==SQLITE_OK && iCol<p->nColumn; iCol++){
        if( p->abNotindexed[iCol]==0 ){
          const char *z = (const char *) sqlite3_column_text(pStmt, iCol+1);
          rc = fts3PendingTermsAdd(p, iLangid, z, iCol, &aSz[iCol]);
          aSz[p->nColumn] += sqlite3_column_bytes(pStmt, iCol+1);
        }
      }
      if( p->bHasDocsize ){
        fts3InsertDocsize(&rc, p, aSz);
      }
      if( rc!=SQLITE_OK ){
        sqlite3_finalize(pStmt);
        pStmt = 0;
      }else{
        nEntry++;
        for(iCol=0; iCol<=p->nColumn; iCol++){
          aSzIns[iCol] += aSz[iCol];
        }
      }
    }
    if( p->bFts4 ){
      fts3UpdateDocTotals(&rc, p, aSzIns, aSzDel, nEntry);
    }
    sqlite3_free(aSz);

    if( pStmt ){
      int rc2 = sqlite3_finalize(pStmt);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }

  return rc;
}


/*
** This function opens a cursor used to read the input data for an 
** incremental merge operation. Specifically, it opens a cursor to scan
** the oldest nSeg segments (idx=0 through idx=(nSeg-1)) in absolute 
** level iAbsLevel.
*/
static int fts3IncrmergeCsr(
  Fts3Table *p,                   /* FTS3 table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level to open */
  int nSeg,                       /* Number of segments to merge */
  Fts3MultiSegReader *pCsr        /* Cursor object to populate */
){
  int rc;                         /* Return Code */
  sqlite3_stmt *pStmt = 0;        /* Statement used to read %_segdir entry */  
  int nByte;                      /* Bytes allocated at pCsr->apSegment[] */

  /* Allocate space for the Fts3MultiSegReader.aCsr[] array */
  memset(pCsr, 0, sizeof(*pCsr));
  nByte = sizeof(Fts3SegReader *) * nSeg;
  pCsr->apSegment = (Fts3SegReader **)sqlite3_malloc(nByte);

  if( pCsr->apSegment==0 ){
    rc = SQLITE_NOMEM;
  }else{
    memset(pCsr->apSegment, 0, nByte);
    rc = fts3SqlStmt(p, SQL_SELECT_LEVEL, &pStmt, 0);
  }
  if( rc==SQLITE_OK ){
    int i;
    int rc2;
    sqlite3_bind_int64(pStmt, 1, iAbsLevel);
    assert( pCsr->nSegment==0 );
    for(i=0; rc==SQLITE_OK && sqlite3_step(pStmt)==SQLITE_ROW && i<nSeg; i++){
      rc = sqlite3Fts3SegReaderNew(i, 0,
          sqlite3_column_int64(pStmt, 1),        /* segdir.start_block */
          sqlite3_column_int64(pStmt, 2),        /* segdir.leaves_end_block */
          sqlite3_column_int64(pStmt, 3),        /* segdir.end_block */
          sqlite3_column_blob(pStmt, 4),         /* segdir.root */
          sqlite3_column_bytes(pStmt, 4),        /* segdir.root */
          &pCsr->apSegment[i]
      );
      pCsr->nSegment++;
    }
    rc2 = sqlite3_reset(pStmt);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  return rc;
}

typedef struct IncrmergeWriter IncrmergeWriter;
typedef struct NodeWriter NodeWriter;
typedef struct Blob Blob;
typedef struct NodeReader NodeReader;

/*
** An instance of the following structure is used as a dynamic buffer
** to build up nodes or other blobs of data in.
**
** The function blobGrowBuffer() is used to extend the allocation.
*/
struct Blob {
  char *a;                        /* Pointer to allocation */
  int n;                          /* Number of valid bytes of data in a[] */
  int nAlloc;                     /* Allocated size of a[] (nAlloc>=n) */
};

/*
** This structure is used to build up buffers containing segment b-tree 
** nodes (blocks).
*/
struct NodeWriter {
  sqlite3_int64 iBlock;           /* Current block id */
  Blob key;                       /* Last key written to the current block */
  Blob block;                     /* Current block image */
};

/*
** An object of this type contains the state required to create or append
** to an appendable b-tree segment.
*/
struct IncrmergeWriter {
  int nLeafEst;                   /* Space allocated for leaf blocks */
  int nWork;                      /* Number of leaf pages flushed */
  sqlite3_int64 iAbsLevel;        /* Absolute level of input segments */
  int iIdx;                       /* Index of *output* segment in iAbsLevel+1 */
  sqlite3_int64 iStart;           /* Block number of first allocated block */
  sqlite3_int64 iEnd;             /* Block number of last allocated block */
  sqlite3_int64 nLeafData;        /* Bytes of leaf page data so far */
  u8 bNoLeafData;                 /* If true, store 0 for segment size */
  NodeWriter aNodeWriter[FTS_MAX_APPENDABLE_HEIGHT];
};

/*
** An object of the following type is used to read data from a single
** FTS segment node. See the following functions:
**
**     nodeReaderInit()
**     nodeReaderNext()
**     nodeReaderRelease()
*/
struct NodeReader {
  const char *aNode;
  int nNode;
  int iOff;                       /* Current offset within aNode[] */

  /* Output variables. Containing the current node entry. */
  sqlite3_int64 iChild;           /* Pointer to child node */
  Blob term;                      /* Current term */
  const char *aDoclist;           /* Pointer to doclist */
  int nDoclist;                   /* Size of doclist in bytes */
};

/*
** If *pRc is not SQLITE_OK when this function is called, it is a no-op.
** Otherwise, if the allocation at pBlob->a is not already at least nMin
** bytes in size, extend (realloc) it to be so.
**
** If an OOM error occurs, set *pRc to SQLITE_NOMEM and leave pBlob->a
** unmodified. Otherwise, if the allocation succeeds, update pBlob->nAlloc
** to reflect the new size of the pBlob->a[] buffer.
*/
static void blobGrowBuffer(Blob *pBlob, int nMin, int *pRc){
  if( *pRc==SQLITE_OK && nMin>pBlob->nAlloc ){
    int nAlloc = nMin;
    char *a = (char *)sqlite3_realloc(pBlob->a, nAlloc);
    if( a ){
      pBlob->nAlloc = nAlloc;
      pBlob->a = a;
    }else{
      *pRc = SQLITE_NOMEM;
    }
  }
}

/*
** Attempt to advance the node-reader object passed as the first argument to
** the next entry on the node. 
**
** Return an error code if an error occurs (SQLITE_NOMEM is possible). 
** Otherwise return SQLITE_OK. If there is no next entry on the node
** (e.g. because the current entry is the last) set NodeReader->aNode to
** NULL to indicate EOF. Otherwise, populate the NodeReader structure output 
** variables for the new entry.
*/
static int nodeReaderNext(NodeReader *p){
  int bFirst = (p->term.n==0);    /* True for first term on the node */
  int nPrefix = 0;                /* Bytes to copy from previous term */
  int nSuffix = 0;                /* Bytes to append to the prefix */
  int rc = SQLITE_OK;             /* Return code */

  assert( p->aNode );
  if( p->iChild && bFirst==0 ) p->iChild++;
  if( p->iOff>=p->nNode ){
    /* EOF */
    p->aNode = 0;
  }else{
    if( bFirst==0 ){
      p->iOff += fts3GetVarint32(&p->aNode[p->iOff], &nPrefix);
    }
    p->iOff += fts3GetVarint32(&p->aNode[p->iOff], &nSuffix);

    blobGrowBuffer(&p->term, nPrefix+nSuffix, &rc);
    if( rc==SQLITE_OK ){
      memcpy(&p->term.a[nPrefix], &p->aNode[p->iOff], nSuffix);
      p->term.n = nPrefix+nSuffix;
      p->iOff += nSuffix;
      if( p->iChild==0 ){
        p->iOff += fts3GetVarint32(&p->aNode[p->iOff], &p->nDoclist);
        p->aDoclist = &p->aNode[p->iOff];
        p->iOff += p->nDoclist;
      }
    }
  }

  assert( p->iOff<=p->nNode );

  return rc;
}

/*
** Release all dynamic resources held by node-reader object *p.
*/
static void nodeReaderRelease(NodeReader *p){
  sqlite3_free(p->term.a);
}

/*
** Initialize a node-reader object to read the node in buffer aNode/nNode.
**
** If successful, SQLITE_OK is returned and the NodeReader object set to 
** point to the first entry on the node (if any). Otherwise, an SQLite
** error code is returned.
*/
static int nodeReaderInit(NodeReader *p, const char *aNode, int nNode){
  memset(p, 0, sizeof(NodeReader));
  p->aNode = aNode;
  p->nNode = nNode;

  /* Figure out if this is a leaf or an internal node. */
  if( p->aNode[0] ){
    /* An internal node. */
    p->iOff = 1 + sqlite3Fts3GetVarint(&p->aNode[1], &p->iChild);
  }else{
    p->iOff = 1;
  }

  return nodeReaderNext(p);
}

/*
** This function is called while writing an FTS segment each time a leaf o
** node is finished and written to disk. The key (zTerm/nTerm) is guaranteed
** to be greater than the largest key on the node just written, but smaller
** than or equal to the first key that will be written to the next leaf
** node.
**
** The block id of the leaf node just written to disk may be found in
** (pWriter->aNodeWriter[0].iBlock) when this function is called.
*/
static int fts3IncrmergePush(
  Fts3Table *p,                   /* Fts3 table handle */
  IncrmergeWriter *pWriter,       /* Writer object */
  const char *zTerm,              /* Term to write to internal node */
  int nTerm                       /* Bytes at zTerm */
){
  sqlite3_int64 iPtr = pWriter->aNodeWriter[0].iBlock;
  int iLayer;

  assert( nTerm>0 );
  for(iLayer=1; ALWAYS(iLayer<FTS_MAX_APPENDABLE_HEIGHT); iLayer++){
    sqlite3_int64 iNextPtr = 0;
    NodeWriter *pNode = &pWriter->aNodeWriter[iLayer];
    int rc = SQLITE_OK;
    int nPrefix;
    int nSuffix;
    int nSpace;

    /* Figure out how much space the key will consume if it is written to
    ** the current node of layer iLayer. Due to the prefix compression, 
    ** the space required changes depending on which node the key is to
    ** be added to.  */
    nPrefix = fts3PrefixCompress(pNode->key.a, pNode->key.n, zTerm, nTerm);
    nSuffix = nTerm - nPrefix;
    nSpace  = sqlite3Fts3VarintLen(nPrefix);
    nSpace += sqlite3Fts3VarintLen(nSuffix) + nSuffix;

    if( pNode->key.n==0 || (pNode->block.n + nSpace)<=p->nNodeSize ){ 
      /* If the current node of layer iLayer contains zero keys, or if adding
      ** the key to it will not cause it to grow to larger than nNodeSize 
      ** bytes in size, write the key here.  */

      Blob *pBlk = &pNode->block;
      if( pBlk->n==0 ){
        blobGrowBuffer(pBlk, p->nNodeSize, &rc);
        if( rc==SQLITE_OK ){
          pBlk->a[0] = (char)iLayer;
          pBlk->n = 1 + sqlite3Fts3PutVarint(&pBlk->a[1], iPtr);
        }
      }
      blobGrowBuffer(pBlk, pBlk->n + nSpace, &rc);
      blobGrowBuffer(&pNode->key, nTerm, &rc);

      if( rc==SQLITE_OK ){
        if( pNode->key.n ){
          pBlk->n += sqlite3Fts3PutVarint(&pBlk->a[pBlk->n], nPrefix);
        }
        pBlk->n += sqlite3Fts3PutVarint(&pBlk->a[pBlk->n], nSuffix);
        memcpy(&pBlk->a[pBlk->n], &zTerm[nPrefix], nSuffix);
        pBlk->n += nSuffix;

        memcpy(pNode->key.a, zTerm, nTerm);
        pNode->key.n = nTerm;
      }
    }else{
      /* Otherwise, flush the current node of layer iLayer to disk.
      ** Then allocate a new, empty sibling node. The key will be written
      ** into the parent of this node. */
      rc = fts3WriteSegment(p, pNode->iBlock, pNode->block.a, pNode->block.n);

      assert( pNode->block.nAlloc>=p->nNodeSize );
      pNode->block.a[0] = (char)iLayer;
      pNode->block.n = 1 + sqlite3Fts3PutVarint(&pNode->block.a[1], iPtr+1);

      iNextPtr = pNode->iBlock;
      pNode->iBlock++;
      pNode->key.n = 0;
    }

    if( rc!=SQLITE_OK || iNextPtr==0 ) return rc;
    iPtr = iNextPtr;
  }

  assert( 0 );
  return 0;
}

/*
** Append a term and (optionally) doclist to the FTS segment node currently
** stored in blob *pNode. The node need not contain any terms, but the
** header must be written before this function is called.
**
** A node header is a single 0x00 byte for a leaf node, or a height varint
** followed by the left-hand-child varint for an internal node.
**
** The term to be appended is passed via arguments zTerm/nTerm. For a 
** leaf node, the doclist is passed as aDoclist/nDoclist. For an internal
** node, both aDoclist and nDoclist must be passed 0.
**
** If the size of the value in blob pPrev is zero, then this is the first
** term written to the node. Otherwise, pPrev contains a copy of the 
** previous term. Before this function returns, it is updated to contain a
** copy of zTerm/nTerm.
**
** It is assumed that the buffer associated with pNode is already large
** enough to accommodate the new entry. The buffer associated with pPrev
** is extended by this function if requrired.
**
** If an error (i.e. OOM condition) occurs, an SQLite error code is
** returned. Otherwise, SQLITE_OK.
*/
static int fts3AppendToNode(
  Blob *pNode,                    /* Current node image to append to */
  Blob *pPrev,                    /* Buffer containing previous term written */
  const char *zTerm,              /* New term to write */
  int nTerm,                      /* Size of zTerm in bytes */
  const char *aDoclist,           /* Doclist (or NULL) to write */
  int nDoclist                    /* Size of aDoclist in bytes */ 
){
  int rc = SQLITE_OK;             /* Return code */
  int bFirst = (pPrev->n==0);     /* True if this is the first term written */
  int nPrefix;                    /* Size of term prefix in bytes */
  int nSuffix;                    /* Size of term suffix in bytes */

  /* Node must have already been started. There must be a doclist for a
  ** leaf node, and there must not be a doclist for an internal node.  */
  assert( pNode->n>0 );
  assert( (pNode->a[0]=='\0')==(aDoclist!=0) );

  blobGrowBuffer(pPrev, nTerm, &rc);
  if( rc!=SQLITE_OK ) return rc;

  nPrefix = fts3PrefixCompress(pPrev->a, pPrev->n, zTerm, nTerm);
  nSuffix = nTerm - nPrefix;
  memcpy(pPrev->a, zTerm, nTerm);
  pPrev->n = nTerm;

  if( bFirst==0 ){
    pNode->n += sqlite3Fts3PutVarint(&pNode->a[pNode->n], nPrefix);
  }
  pNode->n += sqlite3Fts3PutVarint(&pNode->a[pNode->n], nSuffix);
  memcpy(&pNode->a[pNode->n], &zTerm[nPrefix], nSuffix);
  pNode->n += nSuffix;

  if( aDoclist ){
    pNode->n += sqlite3Fts3PutVarint(&pNode->a[pNode->n], nDoclist);
    memcpy(&pNode->a[pNode->n], aDoclist, nDoclist);
    pNode->n += nDoclist;
  }

  assert( pNode->n<=pNode->nAlloc );

  return SQLITE_OK;
}

/*
** Append the current term and doclist pointed to by cursor pCsr to the
** appendable b-tree segment opened for writing by pWriter.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
static int fts3IncrmergeAppend(
  Fts3Table *p,                   /* Fts3 table handle */
  IncrmergeWriter *pWriter,       /* Writer object */
  Fts3MultiSegReader *pCsr        /* Cursor containing term and doclist */
){
  const char *zTerm = pCsr->zTerm;
  int nTerm = pCsr->nTerm;
  const char *aDoclist = pCsr->aDoclist;
  int nDoclist = pCsr->nDoclist;
  int rc = SQLITE_OK;           /* Return code */
  int nSpace;                   /* Total space in bytes required on leaf */
  int nPrefix;                  /* Size of prefix shared with previous term */
  int nSuffix;                  /* Size of suffix (nTerm - nPrefix) */
  NodeWriter *pLeaf;            /* Object used to write leaf nodes */

  pLeaf = &pWriter->aNodeWriter[0];
  nPrefix = fts3PrefixCompress(pLeaf->key.a, pLeaf->key.n, zTerm, nTerm);
  nSuffix = nTerm - nPrefix;

  nSpace  = sqlite3Fts3VarintLen(nPrefix);
  nSpace += sqlite3Fts3VarintLen(nSuffix) + nSuffix;
  nSpace += sqlite3Fts3VarintLen(nDoclist) + nDoclist;

  /* If the current block is not empty, and if adding this term/doclist
  ** to the current block would make it larger than Fts3Table.nNodeSize
  ** bytes, write this block out to the database. */
  if( pLeaf->block.n>0 && (pLeaf->block.n + nSpace)>p->nNodeSize ){
    rc = fts3WriteSegment(p, pLeaf->iBlock, pLeaf->block.a, pLeaf->block.n);
    pWriter->nWork++;

    /* Add the current term to the parent node. The term added to the 
    ** parent must:
    **
    **   a) be greater than the largest term on the leaf node just written
    **      to the database (still available in pLeaf->key), and
    **
    **   b) be less than or equal to the term about to be added to the new
    **      leaf node (zTerm/nTerm).
    **
    ** In other words, it must be the prefix of zTerm 1 byte longer than
    ** the common prefix (if any) of zTerm and pWriter->zTerm.
    */
    if( rc==SQLITE_OK ){
      rc = fts3IncrmergePush(p, pWriter, zTerm, nPrefix+1);
    }

    /* Advance to the next output block */
    pLeaf->iBlock++;
    pLeaf->key.n = 0;
    pLeaf->block.n = 0;

    nSuffix = nTerm;
    nSpace  = 1;
    nSpace += sqlite3Fts3VarintLen(nSuffix) + nSuffix;
    nSpace += sqlite3Fts3VarintLen(nDoclist) + nDoclist;
  }

  pWriter->nLeafData += nSpace;
  blobGrowBuffer(&pLeaf->block, pLeaf->block.n + nSpace, &rc);
  if( rc==SQLITE_OK ){
    if( pLeaf->block.n==0 ){
      pLeaf->block.n = 1;
      pLeaf->block.a[0] = '\0';
    }
    rc = fts3AppendToNode(
        &pLeaf->block, &pLeaf->key, zTerm, nTerm, aDoclist, nDoclist
    );
  }

  return rc;
}

/*
** This function is called to release all dynamic resources held by the
** merge-writer object pWriter, and if no error has occurred, to flush
** all outstanding node buffers held by pWriter to disk.
**
** If *pRc is not SQLITE_OK when this function is called, then no attempt
** is made to write any data to disk. Instead, this function serves only
** to release outstanding resources.
**
** Otherwise, if *pRc is initially SQLITE_OK and an error occurs while
** flushing buffers to disk, *pRc is set to an SQLite error code before
** returning.
*/
static void fts3IncrmergeRelease(
  Fts3Table *p,                   /* FTS3 table handle */
  IncrmergeWriter *pWriter,       /* Merge-writer object */
  int *pRc                        /* IN/OUT: Error code */
){
  int i;                          /* Used to iterate through non-root layers */
  int iRoot;                      /* Index of root in pWriter->aNodeWriter */
  NodeWriter *pRoot;              /* NodeWriter for root node */
  int rc = *pRc;                  /* Error code */

  /* Set iRoot to the index in pWriter->aNodeWriter[] of the output segment 
  ** root node. If the segment fits entirely on a single leaf node, iRoot
  ** will be set to 0. If the root node is the parent of the leaves, iRoot
  ** will be 1. And so on.  */
  for(iRoot=FTS_MAX_APPENDABLE_HEIGHT-1; iRoot>=0; iRoot--){
    NodeWriter *pNode = &pWriter->aNodeWriter[iRoot];
    if( pNode->block.n>0 ) break;
    assert( *pRc || pNode->block.nAlloc==0 );
    assert( *pRc || pNode->key.nAlloc==0 );
    sqlite3_free(pNode->block.a);
    sqlite3_free(pNode->key.a);
  }

  /* Empty output segment. This is a no-op. */
  if( iRoot<0 ) return;

  /* The entire output segment fits on a single node. Normally, this means
  ** the node would be stored as a blob in the "root" column of the %_segdir
  ** table. However, this is not permitted in this case. The problem is that 
  ** space has already been reserved in the %_segments table, and so the 
  ** start_block and end_block fields of the %_segdir table must be populated. 
  ** And, by design or by accident, released versions of FTS cannot handle 
  ** segments that fit entirely on the root node with start_block!=0.
  **
  ** Instead, create a synthetic root node that contains nothing but a 
  ** pointer to the single content node. So that the segment consists of a
  ** single leaf and a single interior (root) node.
  **
  ** Todo: Better might be to defer allocating space in the %_segments 
  ** table until we are sure it is needed.
  */
  if( iRoot==0 ){
    Blob *pBlock = &pWriter->aNodeWriter[1].block;
    blobGrowBuffer(pBlock, 1 + FTS3_VARINT_MAX, &rc);
    if( rc==SQLITE_OK ){
      pBlock->a[0] = 0x01;
      pBlock->n = 1 + sqlite3Fts3PutVarint(
          &pBlock->a[1], pWriter->aNodeWriter[0].iBlock
      );
    }
    iRoot = 1;
  }
  pRoot = &pWriter->aNodeWriter[iRoot];

  /* Flush all currently outstanding nodes to disk. */
  for(i=0; i<iRoot; i++){
    NodeWriter *pNode = &pWriter->aNodeWriter[i];
    if( pNode->block.n>0 && rc==SQLITE_OK ){
      rc = fts3WriteSegment(p, pNode->iBlock, pNode->block.a, pNode->block.n);
    }
    sqlite3_free(pNode->block.a);
    sqlite3_free(pNode->key.a);
  }

  /* Write the %_segdir record. */
  if( rc==SQLITE_OK ){
    rc = fts3WriteSegdir(p, 
        pWriter->iAbsLevel+1,               /* level */
        pWriter->iIdx,                      /* idx */
        pWriter->iStart,                    /* start_block */
        pWriter->aNodeWriter[0].iBlock,     /* leaves_end_block */
        pWriter->iEnd,                      /* end_block */
        (pWriter->bNoLeafData==0 ? pWriter->nLeafData : 0),   /* end_block */
        pRoot->block.a, pRoot->block.n      /* root */
    );
  }
  sqlite3_free(pRoot->block.a);
  sqlite3_free(pRoot->key.a);

  *pRc = rc;
}

/*
** Compare the term in buffer zLhs (size in bytes nLhs) with that in
** zRhs (size in bytes nRhs) using memcmp. If one term is a prefix of
** the other, it is considered to be smaller than the other.
**
** Return -ve if zLhs is smaller than zRhs, 0 if it is equal, or +ve
** if it is greater.
*/
static int fts3TermCmp(
  const char *zLhs, int nLhs,     /* LHS of comparison */
  const char *zRhs, int nRhs      /* RHS of comparison */
){
  int nCmp = MIN(nLhs, nRhs);
  int res;

  res = memcmp(zLhs, zRhs, nCmp);
  if( res==0 ) res = nLhs - nRhs;

  return res;
}


/*
** Query to see if the entry in the %_segments table with blockid iEnd is 
** NULL. If no error occurs and the entry is NULL, set *pbRes 1 before
** returning. Otherwise, set *pbRes to 0. 
**
** Or, if an error occurs while querying the database, return an SQLite 
** error code. The final value of *pbRes is undefined in this case.
**
** This is used to test if a segment is an "appendable" segment. If it
** is, then a NULL entry has been inserted into the %_segments table
** with blockid %_segdir.end_block.
*/
static int fts3IsAppendable(Fts3Table *p, sqlite3_int64 iEnd, int *pbRes){
  int bRes = 0;                   /* Result to set *pbRes to */
  sqlite3_stmt *pCheck = 0;       /* Statement to query database with */
  int rc;                         /* Return code */

  rc = fts3SqlStmt(p, SQL_SEGMENT_IS_APPENDABLE, &pCheck, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pCheck, 1, iEnd);
    if( SQLITE_ROW==sqlite3_step(pCheck) ) bRes = 1;
    rc = sqlite3_reset(pCheck);
  }
  
  *pbRes = bRes;
  return rc;
}

/*
** This function is called when initializing an incremental-merge operation.
** It checks if the existing segment with index value iIdx at absolute level 
** (iAbsLevel+1) can be appended to by the incremental merge. If it can, the
** merge-writer object *pWriter is initialized to write to it.
**
** An existing segment can be appended to by an incremental merge if:
**
**   * It was initially created as an appendable segment (with all required
**     space pre-allocated), and
**
**   * The first key read from the input (arguments zKey and nKey) is 
**     greater than the largest key currently stored in the potential
**     output segment.
*/
static int fts3IncrmergeLoad(
  Fts3Table *p,                   /* Fts3 table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level of input segments */
  int iIdx,                       /* Index of candidate output segment */
  const char *zKey,               /* First key to write */
  int nKey,                       /* Number of bytes in nKey */
  IncrmergeWriter *pWriter        /* Populate this object */
){
  int rc;                         /* Return code */
  sqlite3_stmt *pSelect = 0;      /* SELECT to read %_segdir entry */

  rc = fts3SqlStmt(p, SQL_SELECT_SEGDIR, &pSelect, 0);
  if( rc==SQLITE_OK ){
    sqlite3_int64 iStart = 0;     /* Value of %_segdir.start_block */
    sqlite3_int64 iLeafEnd = 0;   /* Value of %_segdir.leaves_end_block */
    sqlite3_int64 iEnd = 0;       /* Value of %_segdir.end_block */
    const char *aRoot = 0;        /* Pointer to %_segdir.root buffer */
    int nRoot = 0;                /* Size of aRoot[] in bytes */
    int rc2;                      /* Return code from sqlite3_reset() */
    int bAppendable = 0;          /* Set to true if segment is appendable */

    /* Read the %_segdir entry for index iIdx absolute level (iAbsLevel+1) */
    sqlite3_bind_int64(pSelect, 1, iAbsLevel+1);
    sqlite3_bind_int(pSelect, 2, iIdx);
    if( sqlite3_step(pSelect)==SQLITE_ROW ){
      iStart = sqlite3_column_int64(pSelect, 1);
      iLeafEnd = sqlite3_column_int64(pSelect, 2);
      fts3ReadEndBlockField(pSelect, 3, &iEnd, &pWriter->nLeafData);
      if( pWriter->nLeafData<0 ){
        pWriter->nLeafData = pWriter->nLeafData * -1;
      }
      pWriter->bNoLeafData = (pWriter->nLeafData==0);
      nRoot = sqlite3_column_bytes(pSelect, 4);
      aRoot = sqlite3_column_blob(pSelect, 4);
    }else{
      return sqlite3_reset(pSelect);
    }

    /* Check for the zero-length marker in the %_segments table */
    rc = fts3IsAppendable(p, iEnd, &bAppendable);

    /* Check that zKey/nKey is larger than the largest key the candidate */
    if( rc==SQLITE_OK && bAppendable ){
      char *aLeaf = 0;
      int nLeaf = 0;

      rc = sqlite3Fts3ReadBlock(p, iLeafEnd, &aLeaf, &nLeaf, 0);
      if( rc==SQLITE_OK ){
        NodeReader reader;
        for(rc = nodeReaderInit(&reader, aLeaf, nLeaf);
            rc==SQLITE_OK && reader.aNode;
            rc = nodeReaderNext(&reader)
        ){
          assert( reader.aNode );
        }
        if( fts3TermCmp(zKey, nKey, reader.term.a, reader.term.n)<=0 ){
          bAppendable = 0;
        }
        nodeReaderRelease(&reader);
      }
      sqlite3_free(aLeaf);
    }

    if( rc==SQLITE_OK && bAppendable ){
      /* It is possible to append to this segment. Set up the IncrmergeWriter
      ** object to do so.  */
      int i;
      int nHeight = (int)aRoot[0];
      NodeWriter *pNode;

      pWriter->nLeafEst = (int)((iEnd - iStart) + 1)/FTS_MAX_APPENDABLE_HEIGHT;
      pWriter->iStart = iStart;
      pWriter->iEnd = iEnd;
      pWriter->iAbsLevel = iAbsLevel;
      pWriter->iIdx = iIdx;

      for(i=nHeight+1; i<FTS_MAX_APPENDABLE_HEIGHT; i++){
        pWriter->aNodeWriter[i].iBlock = pWriter->iStart + i*pWriter->nLeafEst;
      }

      pNode = &pWriter->aNodeWriter[nHeight];
      pNode->iBlock = pWriter->iStart + pWriter->nLeafEst*nHeight;
      blobGrowBuffer(&pNode->block, MAX(nRoot, p->nNodeSize), &rc);
      if( rc==SQLITE_OK ){
        memcpy(pNode->block.a, aRoot, nRoot);
        pNode->block.n = nRoot;
      }

      for(i=nHeight; i>=0 && rc==SQLITE_OK; i--){
        NodeReader reader;
        pNode = &pWriter->aNodeWriter[i];

        rc = nodeReaderInit(&reader, pNode->block.a, pNode->block.n);
        while( reader.aNode && rc==SQLITE_OK ) rc = nodeReaderNext(&reader);
        blobGrowBuffer(&pNode->key, reader.term.n, &rc);
        if( rc==SQLITE_OK ){
          memcpy(pNode->key.a, reader.term.a, reader.term.n);
          pNode->key.n = reader.term.n;
          if( i>0 ){
            char *aBlock = 0;
            int nBlock = 0;
            pNode = &pWriter->aNodeWriter[i-1];
            pNode->iBlock = reader.iChild;
            rc = sqlite3Fts3ReadBlock(p, reader.iChild, &aBlock, &nBlock, 0);
            blobGrowBuffer(&pNode->block, MAX(nBlock, p->nNodeSize), &rc);
            if( rc==SQLITE_OK ){
              memcpy(pNode->block.a, aBlock, nBlock);
              pNode->block.n = nBlock;
            }
            sqlite3_free(aBlock);
          }
        }
        nodeReaderRelease(&reader);
      }
    }

    rc2 = sqlite3_reset(pSelect);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  return rc;
}

/*
** Determine the largest segment index value that exists within absolute
** level iAbsLevel+1. If no error occurs, set *piIdx to this value plus
** one before returning SQLITE_OK. Or, if there are no segments at all 
** within level iAbsLevel, set *piIdx to zero.
**
** If an error occurs, return an SQLite error code. The final value of
** *piIdx is undefined in this case.
*/
static int fts3IncrmergeOutputIdx( 
  Fts3Table *p,                   /* FTS Table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute index of input segments */
  int *piIdx                      /* OUT: Next free index at iAbsLevel+1 */
){
  int rc;
  sqlite3_stmt *pOutputIdx = 0;   /* SQL used to find output index */

  rc = fts3SqlStmt(p, SQL_NEXT_SEGMENT_INDEX, &pOutputIdx, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pOutputIdx, 1, iAbsLevel+1);
    sqlite3_step(pOutputIdx);
    *piIdx = sqlite3_column_int(pOutputIdx, 0);
    rc = sqlite3_reset(pOutputIdx);
  }

  return rc;
}

/* 
** Allocate an appendable output segment on absolute level iAbsLevel+1
** with idx value iIdx.
**
** In the %_segdir table, a segment is defined by the values in three
** columns:
**
**     start_block
**     leaves_end_block
**     end_block
**
** When an appendable segment is allocated, it is estimated that the
** maximum number of leaf blocks that may be required is the sum of the
** number of leaf blocks consumed by the input segments, plus the number
** of input segments, multiplied by two. This value is stored in stack 
** variable nLeafEst.
**
** A total of 16*nLeafEst blocks are allocated when an appendable segment
** is created ((1 + end_block - start_block)==16*nLeafEst). The contiguous
** array of leaf nodes starts at the first block allocated. The array
** of interior nodes that are parents of the leaf nodes start at block
** (start_block + (1 + end_block - start_block) / 16). And so on.
**
** In the actual code below, the value "16" is replaced with the 
** pre-processor macro FTS_MAX_APPENDABLE_HEIGHT.
*/
static int fts3IncrmergeWriter( 
  Fts3Table *p,                   /* Fts3 table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level of input segments */
  int iIdx,                       /* Index of new output segment */
  Fts3MultiSegReader *pCsr,       /* Cursor that data will be read from */
  IncrmergeWriter *pWriter        /* Populate this object */
){
  int rc;                         /* Return Code */
  int i;                          /* Iterator variable */
  int nLeafEst = 0;               /* Blocks allocated for leaf nodes */
  sqlite3_stmt *pLeafEst = 0;     /* SQL used to determine nLeafEst */
  sqlite3_stmt *pFirstBlock = 0;  /* SQL used to determine first block */

  /* Calculate nLeafEst. */
  rc = fts3SqlStmt(p, SQL_MAX_LEAF_NODE_ESTIMATE, &pLeafEst, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pLeafEst, 1, iAbsLevel);
    sqlite3_bind_int64(pLeafEst, 2, pCsr->nSegment);
    if( SQLITE_ROW==sqlite3_step(pLeafEst) ){
      nLeafEst = sqlite3_column_int(pLeafEst, 0);
    }
    rc = sqlite3_reset(pLeafEst);
  }
  if( rc!=SQLITE_OK ) return rc;

  /* Calculate the first block to use in the output segment */
  rc = fts3SqlStmt(p, SQL_NEXT_SEGMENTS_ID, &pFirstBlock, 0);
  if( rc==SQLITE_OK ){
    if( SQLITE_ROW==sqlite3_step(pFirstBlock) ){
      pWriter->iStart = sqlite3_column_int64(pFirstBlock, 0);
      pWriter->iEnd = pWriter->iStart - 1;
      pWriter->iEnd += nLeafEst * FTS_MAX_APPENDABLE_HEIGHT;
    }
    rc = sqlite3_reset(pFirstBlock);
  }
  if( rc!=SQLITE_OK ) return rc;

  /* Insert the marker in the %_segments table to make sure nobody tries
  ** to steal the space just allocated. This is also used to identify 
  ** appendable segments.  */
  rc = fts3WriteSegment(p, pWriter->iEnd, 0, 0);
  if( rc!=SQLITE_OK ) return rc;

  pWriter->iAbsLevel = iAbsLevel;
  pWriter->nLeafEst = nLeafEst;
  pWriter->iIdx = iIdx;

  /* Set up the array of NodeWriter objects */
  for(i=0; i<FTS_MAX_APPENDABLE_HEIGHT; i++){
    pWriter->aNodeWriter[i].iBlock = pWriter->iStart + i*pWriter->nLeafEst;
  }
  return SQLITE_OK;
}

/*
** Remove an entry from the %_segdir table. This involves running the 
** following two statements:
**
**   DELETE FROM %_segdir WHERE level = :iAbsLevel AND idx = :iIdx
**   UPDATE %_segdir SET idx = idx - 1 WHERE level = :iAbsLevel AND idx > :iIdx
**
** The DELETE statement removes the specific %_segdir level. The UPDATE 
** statement ensures that the remaining segments have contiguously allocated
** idx values.
*/
static int fts3RemoveSegdirEntry(
  Fts3Table *p,                   /* FTS3 table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level to delete from */
  int iIdx                        /* Index of %_segdir entry to delete */
){
  int rc;                         /* Return code */
  sqlite3_stmt *pDelete = 0;      /* DELETE statement */

  rc = fts3SqlStmt(p, SQL_DELETE_SEGDIR_ENTRY, &pDelete, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pDelete, 1, iAbsLevel);
    sqlite3_bind_int(pDelete, 2, iIdx);
    sqlite3_step(pDelete);
    rc = sqlite3_reset(pDelete);
  }

  return rc;
}

/*
** One or more segments have just been removed from absolute level iAbsLevel.
** Update the 'idx' values of the remaining segments in the level so that
** the idx values are a contiguous sequence starting from 0.
*/
static int fts3RepackSegdirLevel(
  Fts3Table *p,                   /* FTS3 table handle */
  sqlite3_int64 iAbsLevel         /* Absolute level to repack */
){
  int rc;                         /* Return code */
  int *aIdx = 0;                  /* Array of remaining idx values */
  int nIdx = 0;                   /* Valid entries in aIdx[] */
  int nAlloc = 0;                 /* Allocated size of aIdx[] */
  int i;                          /* Iterator variable */
  sqlite3_stmt *pSelect = 0;      /* Select statement to read idx values */
  sqlite3_stmt *pUpdate = 0;      /* Update statement to modify idx values */

  rc = fts3SqlStmt(p, SQL_SELECT_INDEXES, &pSelect, 0);
  if( rc==SQLITE_OK ){
    int rc2;
    sqlite3_bind_int64(pSelect, 1, iAbsLevel);
    while( SQLITE_ROW==sqlite3_step(pSelect) ){
      if( nIdx>=nAlloc ){
        int *aNew;
        nAlloc += 16;
        aNew = sqlite3_realloc(aIdx, nAlloc*sizeof(int));
        if( !aNew ){
          rc = SQLITE_NOMEM;
          break;
        }
        aIdx = aNew;
      }
      aIdx[nIdx++] = sqlite3_column_int(pSelect, 0);
    }
    rc2 = sqlite3_reset(pSelect);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  if( rc==SQLITE_OK ){
    rc = fts3SqlStmt(p, SQL_SHIFT_SEGDIR_ENTRY, &pUpdate, 0);
  }
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pUpdate, 2, iAbsLevel);
  }

  assert( p->bIgnoreSavepoint==0 );
  p->bIgnoreSavepoint = 1;
  for(i=0; rc==SQLITE_OK && i<nIdx; i++){
    if( aIdx[i]!=i ){
      sqlite3_bind_int(pUpdate, 3, aIdx[i]);
      sqlite3_bind_int(pUpdate, 1, i);
      sqlite3_step(pUpdate);
      rc = sqlite3_reset(pUpdate);
    }
  }
  p->bIgnoreSavepoint = 0;

  sqlite3_free(aIdx);
  return rc;
}

static void fts3StartNode(Blob *pNode, int iHeight, sqlite3_int64 iChild){
  pNode->a[0] = (char)iHeight;
  if( iChild ){
    assert( pNode->nAlloc>=1+sqlite3Fts3VarintLen(iChild) );
    pNode->n = 1 + sqlite3Fts3PutVarint(&pNode->a[1], iChild);
  }else{
    assert( pNode->nAlloc>=1 );
    pNode->n = 1;
  }
}

/*
** The first two arguments are a pointer to and the size of a segment b-tree
** node. The node may be a leaf or an internal node.
**
** This function creates a new node image in blob object *pNew by copying
** all terms that are greater than or equal to zTerm/nTerm (for leaf nodes)
** or greater than zTerm/nTerm (for internal nodes) from aNode/nNode.
*/
static int fts3TruncateNode(
  const char *aNode,              /* Current node image */
  int nNode,                      /* Size of aNode in bytes */
  Blob *pNew,                     /* OUT: Write new node image here */
  const char *zTerm,              /* Omit all terms smaller than this */
  int nTerm,                      /* Size of zTerm in bytes */
  sqlite3_int64 *piBlock          /* OUT: Block number in next layer down */
){
  NodeReader reader;              /* Reader object */
  Blob prev = {0, 0, 0};          /* Previous term written to new node */
  int rc = SQLITE_OK;             /* Return code */
  int bLeaf = aNode[0]=='\0';     /* True for a leaf node */

  /* Allocate required output space */
  blobGrowBuffer(pNew, nNode, &rc);
  if( rc!=SQLITE_OK ) return rc;
  pNew->n = 0;

  /* Populate new node buffer */
  for(rc = nodeReaderInit(&reader, aNode, nNode); 
      rc==SQLITE_OK && reader.aNode; 
      rc = nodeReaderNext(&reader)
  ){
    if( pNew->n==0 ){
      int res = fts3TermCmp(reader.term.a, reader.term.n, zTerm, nTerm);
      if( res<0 || (bLeaf==0 && res==0) ) continue;
      fts3StartNode(pNew, (int)aNode[0], reader.iChild);
      *piBlock = reader.iChild;
    }
    rc = fts3AppendToNode(
        pNew, &prev, reader.term.a, reader.term.n,
        reader.aDoclist, reader.nDoclist
    );
    if( rc!=SQLITE_OK ) break;
  }
  if( pNew->n==0 ){
    fts3StartNode(pNew, (int)aNode[0], reader.iChild);
    *piBlock = reader.iChild;
  }
  assert( pNew->n<=pNew->nAlloc );

  nodeReaderRelease(&reader);
  sqlite3_free(prev.a);
  return rc;
}

/*
** Remove all terms smaller than zTerm/nTerm from segment iIdx in absolute 
** level iAbsLevel. This may involve deleting entries from the %_segments
** table, and modifying existing entries in both the %_segments and %_segdir
** tables.
**
** SQLITE_OK is returned if the segment is updated successfully. Or an
** SQLite error code otherwise.
*/
static int fts3TruncateSegment(
  Fts3Table *p,                   /* FTS3 table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level of segment to modify */
  int iIdx,                       /* Index within level of segment to modify */
  const char *zTerm,              /* Remove terms smaller than this */
  int nTerm                      /* Number of bytes in buffer zTerm */
){
  int rc = SQLITE_OK;             /* Return code */
  Blob root = {0,0,0};            /* New root page image */
  Blob block = {0,0,0};           /* Buffer used for any other block */
  sqlite3_int64 iBlock = 0;       /* Block id */
  sqlite3_int64 iNewStart = 0;    /* New value for iStartBlock */
  sqlite3_int64 iOldStart = 0;    /* Old value for iStartBlock */
  sqlite3_stmt *pFetch = 0;       /* Statement used to fetch segdir */

  rc = fts3SqlStmt(p, SQL_SELECT_SEGDIR, &pFetch, 0);
  if( rc==SQLITE_OK ){
    int rc2;                      /* sqlite3_reset() return code */
    sqlite3_bind_int64(pFetch, 1, iAbsLevel);
    sqlite3_bind_int(pFetch, 2, iIdx);
    if( SQLITE_ROW==sqlite3_step(pFetch) ){
      const char *aRoot = sqlite3_column_blob(pFetch, 4);
      int nRoot = sqlite3_column_bytes(pFetch, 4);
      iOldStart = sqlite3_column_int64(pFetch, 1);
      rc = fts3TruncateNode(aRoot, nRoot, &root, zTerm, nTerm, &iBlock);
    }
    rc2 = sqlite3_reset(pFetch);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  while( rc==SQLITE_OK && iBlock ){
    char *aBlock = 0;
    int nBlock = 0;
    iNewStart = iBlock;

    rc = sqlite3Fts3ReadBlock(p, iBlock, &aBlock, &nBlock, 0);
    if( rc==SQLITE_OK ){
      rc = fts3TruncateNode(aBlock, nBlock, &block, zTerm, nTerm, &iBlock);
    }
    if( rc==SQLITE_OK ){
      rc = fts3WriteSegment(p, iNewStart, block.a, block.n);
    }
    sqlite3_free(aBlock);
  }

  /* Variable iNewStart now contains the first valid leaf node. */
  if( rc==SQLITE_OK && iNewStart ){
    sqlite3_stmt *pDel = 0;
    rc = fts3SqlStmt(p, SQL_DELETE_SEGMENTS_RANGE, &pDel, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int64(pDel, 1, iOldStart);
      sqlite3_bind_int64(pDel, 2, iNewStart-1);
      sqlite3_step(pDel);
      rc = sqlite3_reset(pDel);
    }
  }

  if( rc==SQLITE_OK ){
    sqlite3_stmt *pChomp = 0;
    rc = fts3SqlStmt(p, SQL_CHOMP_SEGDIR, &pChomp, 0);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int64(pChomp, 1, iNewStart);
      sqlite3_bind_blob(pChomp, 2, root.a, root.n, SQLITE_STATIC);
      sqlite3_bind_int64(pChomp, 3, iAbsLevel);
      sqlite3_bind_int(pChomp, 4, iIdx);
      sqlite3_step(pChomp);
      rc = sqlite3_reset(pChomp);
    }
  }

  sqlite3_free(root.a);
  sqlite3_free(block.a);
  return rc;
}

/*
** This function is called after an incrmental-merge operation has run to
** merge (or partially merge) two or more segments from absolute level
** iAbsLevel.
**
** Each input segment is either removed from the db completely (if all of
** its data was copied to the output segment by the incrmerge operation)
** or modified in place so that it no longer contains those entries that
** have been duplicated in the output segment.
*/
static int fts3IncrmergeChomp(
  Fts3Table *p,                   /* FTS table handle */
  sqlite3_int64 iAbsLevel,        /* Absolute level containing segments */
  Fts3MultiSegReader *pCsr,       /* Chomp all segments opened by this cursor */
  int *pnRem                      /* Number of segments not deleted */
){
  int i;
  int nRem = 0;
  int rc = SQLITE_OK;

  for(i=pCsr->nSegment-1; i>=0 && rc==SQLITE_OK; i--){
    Fts3SegReader *pSeg = 0;
    int j;

    /* Find the Fts3SegReader object with Fts3SegReader.iIdx==i. It is hiding
    ** somewhere in the pCsr->apSegment[] array.  */
    for(j=0; ALWAYS(j<pCsr->nSegment); j++){
      pSeg = pCsr->apSegment[j];
      if( pSeg->iIdx==i ) break;
    }
    assert( j<pCsr->nSegment && pSeg->iIdx==i );

    if( pSeg->aNode==0 ){
      /* Seg-reader is at EOF. Remove the entire input segment. */
      rc = fts3DeleteSegment(p, pSeg);
      if( rc==SQLITE_OK ){
        rc = fts3RemoveSegdirEntry(p, iAbsLevel, pSeg->iIdx);
      }
      *pnRem = 0;
    }else{
      /* The incremental merge did not copy all the data from this 
      ** segment to the upper level. The segment is modified in place
      ** so that it contains no keys smaller than zTerm/nTerm. */ 
      const char *zTerm = pSeg->zTerm;
      int nTerm = pSeg->nTerm;
      rc = fts3TruncateSegment(p, iAbsLevel, pSeg->iIdx, zTerm, nTerm);
      nRem++;
    }
  }

  if( rc==SQLITE_OK && nRem!=pCsr->nSegment ){
    rc = fts3RepackSegdirLevel(p, iAbsLevel);
  }

  *pnRem = nRem;
  return rc;
}

/*
** Store an incr-merge hint in the database.
*/
static int fts3IncrmergeHintStore(Fts3Table *p, Blob *pHint){
  sqlite3_stmt *pReplace = 0;
  int rc;                         /* Return code */

  rc = fts3SqlStmt(p, SQL_REPLACE_STAT, &pReplace, 0);
  if( rc==SQLITE_OK ){
    sqlite3_bind_int(pReplace, 1, FTS_STAT_INCRMERGEHINT);
    sqlite3_bind_blob(pReplace, 2, pHint->a, pHint->n, SQLITE_STATIC);
    sqlite3_step(pReplace);
    rc = sqlite3_reset(pReplace);
  }

  return rc;
}

/*
** Load an incr-merge hint from the database. The incr-merge hint, if one 
** exists, is stored in the rowid==1 row of the %_stat table.
**
** If successful, populate blob *pHint with the value read from the %_stat
** table and return SQLITE_OK. Otherwise, if an error occurs, return an
** SQLite error code.
*/
static int fts3IncrmergeHintLoad(Fts3Table *p, Blob *pHint){
  sqlite3_stmt *pSelect = 0;
  int rc;

  pHint->n = 0;
  rc = fts3SqlStmt(p, SQL_SELECT_STAT, &pSelect, 0);
  if( rc==SQLITE_OK ){
    int rc2;
    sqlite3_bind_int(pSelect, 1, FTS_STAT_INCRMERGEHINT);
    if( SQLITE_ROW==sqlite3_step(pSelect) ){
      const char *aHint = sqlite3_column_blob(pSelect, 0);
      int nHint = sqlite3_column_bytes(pSelect, 0);
      if( aHint ){
        blobGrowBuffer(pHint, nHint, &rc);
        if( rc==SQLITE_OK ){
          memcpy(pHint->a, aHint, nHint);
          pHint->n = nHint;
        }
      }
    }
    rc2 = sqlite3_reset(pSelect);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  return rc;
}

/*
** If *pRc is not SQLITE_OK when this function is called, it is a no-op.
** Otherwise, append an entry to the hint stored in blob *pHint. Each entry
** consists of two varints, the absolute level number of the input segments 
** and the number of input segments.
**
** If successful, leave *pRc set to SQLITE_OK and return. If an error occurs,
** set *pRc to an SQLite error code before returning.
*/
static void fts3IncrmergeHintPush(
  Blob *pHint,                    /* Hint blob to append to */
  i64 iAbsLevel,                  /* First varint to store in hint */
  int nInput,                     /* Second varint to store in hint */
  int *pRc                        /* IN/OUT: Error code */
){
  blobGrowBuffer(pHint, pHint->n + 2*FTS3_VARINT_MAX, pRc);
  if( *pRc==SQLITE_OK ){
    pHint->n += sqlite3Fts3PutVarint(&pHint->a[pHint->n], iAbsLevel);
    pHint->n += sqlite3Fts3PutVarint(&pHint->a[pHint->n], (i64)nInput);
  }
}

/*
** Read the last entry (most recently pushed) from the hint blob *pHint
** and then remove the entry. Write the two values read to *piAbsLevel and 
** *pnInput before returning.
**
** If no error occurs, return SQLITE_OK. If the hint blob in *pHint does
** not contain at least two valid varints, return SQLITE_CORRUPT_VTAB.
*/
static int fts3IncrmergeHintPop(Blob *pHint, i64 *piAbsLevel, int *pnInput){
  const int nHint = pHint->n;
  int i;

  i = pHint->n-2;
  while( i>0 && (pHint->a[i-1] & 0x80) ) i--;
  while( i>0 && (pHint->a[i-1] & 0x80) ) i--;

  pHint->n = i;
  i += sqlite3Fts3GetVarint(&pHint->a[i], piAbsLevel);
  i += fts3GetVarint32(&pHint->a[i], pnInput);
  if( i!=nHint ) return FTS_CORRUPT_VTAB;

  return SQLITE_OK;
}


/*
** Attempt an incremental merge that writes nMerge leaf blocks.
**
** Incremental merges happen nMin segments at a time. The segments 
** to be merged are the nMin oldest segments (the ones with the smallest 
** values for the _segdir.idx field) in the highest level that contains 
** at least nMin segments. Multiple merges might occur in an attempt to 
** write the quota of nMerge leaf blocks.
*/
SQLITE_PRIVATE int sqlite3Fts3Incrmerge(Fts3Table *p, int nMerge, int nMin){
  int rc;                         /* Return code */
  int nRem = nMerge;              /* Number of leaf pages yet to  be written */
  Fts3MultiSegReader *pCsr;       /* Cursor used to read input data */
  Fts3SegFilter *pFilter;         /* Filter used with cursor pCsr */
  IncrmergeWriter *pWriter;       /* Writer object */
  int nSeg = 0;                   /* Number of input segments */
  sqlite3_int64 iAbsLevel = 0;    /* Absolute level number to work on */
  Blob hint = {0, 0, 0};          /* Hint read from %_stat table */
  int bDirtyHint = 0;             /* True if blob 'hint' has been modified */

  /* Allocate space for the cursor, filter and writer objects */
  const int nAlloc = sizeof(*pCsr) + sizeof(*pFilter) + sizeof(*pWriter);
  pWriter = (IncrmergeWriter *)sqlite3_malloc(nAlloc);
  if( !pWriter ) return SQLITE_NOMEM;
  pFilter = (Fts3SegFilter *)&pWriter[1];
  pCsr = (Fts3MultiSegReader *)&pFilter[1];

  rc = fts3IncrmergeHintLoad(p, &hint);
  while( rc==SQLITE_OK && nRem>0 ){
    const i64 nMod = FTS3_SEGDIR_MAXLEVEL * p->nIndex;
    sqlite3_stmt *pFindLevel = 0; /* SQL used to determine iAbsLevel */
    int bUseHint = 0;             /* True if attempting to append */
    int iIdx = 0;                 /* Largest idx in level (iAbsLevel+1) */

    /* Search the %_segdir table for the absolute level with the smallest
    ** relative level number that contains at least nMin segments, if any.
    ** If one is found, set iAbsLevel to the absolute level number and
    ** nSeg to nMin. If no level with at least nMin segments can be found, 
    ** set nSeg to -1.
    */
    rc = fts3SqlStmt(p, SQL_FIND_MERGE_LEVEL, &pFindLevel, 0);
    sqlite3_bind_int(pFindLevel, 1, nMin);
    if( sqlite3_step(pFindLevel)==SQLITE_ROW ){
      iAbsLevel = sqlite3_column_int64(pFindLevel, 0);
      nSeg = nMin;
    }else{
      nSeg = -1;
    }
    rc = sqlite3_reset(pFindLevel);

    /* If the hint read from the %_stat table is not empty, check if the
    ** last entry in it specifies a relative level smaller than or equal
    ** to the level identified by the block above (if any). If so, this 
    ** iteration of the loop will work on merging at the hinted level.
    */
    if( rc==SQLITE_OK && hint.n ){
      int nHint = hint.n;
      sqlite3_int64 iHintAbsLevel = 0;      /* Hint level */
      int nHintSeg = 0;                     /* Hint number of segments */

      rc = fts3IncrmergeHintPop(&hint, &iHintAbsLevel, &nHintSeg);
      if( nSeg<0 || (iAbsLevel % nMod) >= (iHintAbsLevel % nMod) ){
        iAbsLevel = iHintAbsLevel;
        nSeg = nHintSeg;
        bUseHint = 1;
        bDirtyHint = 1;
      }else{
        /* This undoes the effect of the HintPop() above - so that no entry
        ** is removed from the hint blob.  */
        hint.n = nHint;
      }
    }

    /* If nSeg is less that zero, then there is no level with at least
    ** nMin segments and no hint in the %_stat table. No work to do.
    ** Exit early in this case.  */
    if( nSeg<0 ) break;

    /* Open a cursor to iterate through the contents of the oldest nSeg 
    ** indexes of absolute level iAbsLevel. If this cursor is opened using 
    ** the 'hint' parameters, it is possible that there are less than nSeg
    ** segments available in level iAbsLevel. In this case, no work is
    ** done on iAbsLevel - fall through to the next iteration of the loop 
    ** to start work on some other level.  */
    memset(pWriter, 0, nAlloc);
    pFilter->flags = FTS3_SEGMENT_REQUIRE_POS;

    if( rc==SQLITE_OK ){
      rc = fts3IncrmergeOutputIdx(p, iAbsLevel, &iIdx);
      assert( bUseHint==1 || bUseHint==0 );
      if( iIdx==0 || (bUseHint && iIdx==1) ){
        int bIgnore = 0;
        rc = fts3SegmentIsMaxLevel(p, iAbsLevel+1, &bIgnore);
        if( bIgnore ){
          pFilter->flags |= FTS3_SEGMENT_IGNORE_EMPTY;
        }
      }
    }

    if( rc==SQLITE_OK ){
      rc = fts3IncrmergeCsr(p, iAbsLevel, nSeg, pCsr);
    }
    if( SQLITE_OK==rc && pCsr->nSegment==nSeg
     && SQLITE_OK==(rc = sqlite3Fts3SegReaderStart(p, pCsr, pFilter))
     && SQLITE_ROW==(rc = sqlite3Fts3SegReaderStep(p, pCsr))
    ){
      if( bUseHint && iIdx>0 ){
        const char *zKey = pCsr->zTerm;
        int nKey = pCsr->nTerm;
        rc = fts3IncrmergeLoad(p, iAbsLevel, iIdx-1, zKey, nKey, pWriter);
      }else{
        rc = fts3IncrmergeWriter(p, iAbsLevel, iIdx, pCsr, pWriter);
      }

      if( rc==SQLITE_OK && pWriter->nLeafEst ){
        fts3LogMerge(nSeg, iAbsLevel);
        do {
          rc = fts3IncrmergeAppend(p, pWriter, pCsr);
          if( rc==SQLITE_OK ) rc = sqlite3Fts3SegReaderStep(p, pCsr);
          if( pWriter->nWork>=nRem && rc==SQLITE_ROW ) rc = SQLITE_OK;
        }while( rc==SQLITE_ROW );

        /* Update or delete the input segments */
        if( rc==SQLITE_OK ){
          nRem -= (1 + pWriter->nWork);
          rc = fts3IncrmergeChomp(p, iAbsLevel, pCsr, &nSeg);
          if( nSeg!=0 ){
            bDirtyHint = 1;
            fts3IncrmergeHintPush(&hint, iAbsLevel, nSeg, &rc);
          }
        }
      }

      if( nSeg!=0 ){
        pWriter->nLeafData = pWriter->nLeafData * -1;
      }
      fts3IncrmergeRelease(p, pWriter, &rc);
      if( nSeg==0 && pWriter->bNoLeafData==0 ){
        fts3PromoteSegments(p, iAbsLevel+1, pWriter->nLeafData);
      }
    }

    sqlite3Fts3SegReaderFinish(pCsr);
  }

  /* Write the hint values into the %_stat table for the next incr-merger */
  if( bDirtyHint && rc==SQLITE_OK ){
    rc = fts3IncrmergeHintStore(p, &hint);
  }

  sqlite3_free(pWriter);
  sqlite3_free(hint.a);
  return rc;
}

/*
** Convert the text beginning at *pz into an integer and return
** its value.  Advance *pz to point to the first character past
** the integer.
*/
static int fts3Getint(const char **pz){
  const char *z = *pz;
  int i = 0;
  while( (*z)>='0' && (*z)<='9' ) i = 10*i + *(z++) - '0';
  *pz = z;
  return i;
}

/*
** Process statements of the form:
**
**    INSERT INTO table(table) VALUES('merge=A,B');
**
** A and B are integers that decode to be the number of leaf pages
** written for the merge, and the minimum number of segments on a level
** before it will be selected for a merge, respectively.
*/
static int fts3DoIncrmerge(
  Fts3Table *p,                   /* FTS3 table handle */
  const char *zParam              /* Nul-terminated string containing "A,B" */
){
  int rc;
  int nMin = (FTS3_MERGE_COUNT / 2);
  int nMerge = 0;
  const char *z = zParam;

  /* Read the first integer value */
  nMerge = fts3Getint(&z);

  /* If the first integer value is followed by a ',',  read the second
  ** integer value. */
  if( z[0]==',' && z[1]!='\0' ){
    z++;
    nMin = fts3Getint(&z);
  }

  if( z[0]!='\0' || nMin<2 ){
    rc = SQLITE_ERROR;
  }else{
    rc = SQLITE_OK;
    if( !p->bHasStat ){
      assert( p->bFts4==0 );
      sqlite3Fts3CreateStatTable(&rc, p);
    }
    if( rc==SQLITE_OK ){
      rc = sqlite3Fts3Incrmerge(p, nMerge, nMin);
    }
    sqlite3Fts3SegmentsClose(p);
  }
  return rc;
}

/*
** Process statements of the form:
**
**    INSERT INTO table(table) VALUES('automerge=X');
**
** where X is an integer.  X==0 means to turn automerge off.  X!=0 means
** turn it on.  The setting is persistent.
*/
static int fts3DoAutoincrmerge(
  Fts3Table *p,                   /* FTS3 table handle */
  const char *zParam              /* Nul-terminated string containing boolean */
){
  int rc = SQLITE_OK;
  sqlite3_stmt *pStmt = 0;
  p->nAutoincrmerge = fts3Getint(&zParam);
  if( p->nAutoincrmerge==1 || p->nAutoincrmerge>FTS3_MERGE_COUNT ){
    p->nAutoincrmerge = 8;
  }
  if( !p->bHasStat ){
    assert( p->bFts4==0 );
    sqlite3Fts3CreateStatTable(&rc, p);
    if( rc ) return rc;
  }
  rc = fts3SqlStmt(p, SQL_REPLACE_STAT, &pStmt, 0);
  if( rc ) return rc;
  sqlite3_bind_int(pStmt, 1, FTS_STAT_AUTOINCRMERGE);
  sqlite3_bind_int(pStmt, 2, p->nAutoincrmerge);
  sqlite3_step(pStmt);
  rc = sqlite3_reset(pStmt);
  return rc;
}

/*
** Return a 64-bit checksum for the FTS index entry specified by the
** arguments to this function.
*/
static u64 fts3ChecksumEntry(
  const char *zTerm,              /* Pointer to buffer containing term */
  int nTerm,                      /* Size of zTerm in bytes */
  int iLangid,                    /* Language id for current row */
  int iIndex,                     /* Index (0..Fts3Table.nIndex-1) */
  i64 iDocid,                     /* Docid for current row. */
  int iCol,                       /* Column number */
  int iPos                        /* Position */
){
  int i;
  u64 ret = (u64)iDocid;

  ret += (ret<<3) + iLangid;
  ret += (ret<<3) + iIndex;
  ret += (ret<<3) + iCol;
  ret += (ret<<3) + iPos;
  for(i=0; i<nTerm; i++) ret += (ret<<3) + zTerm[i];

  return ret;
}

/*
** Return a checksum of all entries in the FTS index that correspond to
** language id iLangid. The checksum is calculated by XORing the checksums
** of each individual entry (see fts3ChecksumEntry()) together.
**
** If successful, the checksum value is returned and *pRc set to SQLITE_OK.
** Otherwise, if an error occurs, *pRc is set to an SQLite error code. The
** return value is undefined in this case.
*/
static u64 fts3ChecksumIndex(
  Fts3Table *p,                   /* FTS3 table handle */
  int iLangid,                    /* Language id to return cksum for */
  int iIndex,                     /* Index to cksum (0..p->nIndex-1) */
  int *pRc                        /* OUT: Return code */
){
  Fts3SegFilter filter;
  Fts3MultiSegReader csr;
  int rc;
  u64 cksum = 0;

  assert( *pRc==SQLITE_OK );

  memset(&filter, 0, sizeof(filter));
  memset(&csr, 0, sizeof(csr));
  filter.flags =  FTS3_SEGMENT_REQUIRE_POS|FTS3_SEGMENT_IGNORE_EMPTY;
  filter.flags |= FTS3_SEGMENT_SCAN;

  rc = sqlite3Fts3SegReaderCursor(
      p, iLangid, iIndex, FTS3_SEGCURSOR_ALL, 0, 0, 0, 1,&csr
  );
  if( rc==SQLITE_OK ){
    rc = sqlite3Fts3SegReaderStart(p, &csr, &filter);
  }

  if( rc==SQLITE_OK ){
    while( SQLITE_ROW==(rc = sqlite3Fts3SegReaderStep(p, &csr)) ){
      char *pCsr = csr.aDoclist;
      char *pEnd = &pCsr[csr.nDoclist];

      i64 iDocid = 0;
      i64 iCol = 0;
      i64 iPos = 0;

      pCsr += sqlite3Fts3GetVarint(pCsr, &iDocid);
      while( pCsr<pEnd ){
        i64 iVal = 0;
        pCsr += sqlite3Fts3GetVarint(pCsr, &iVal);
        if( pCsr<pEnd ){
          if( iVal==0 || iVal==1 ){
            iCol = 0;
            iPos = 0;
            if( iVal ){
              pCsr += sqlite3Fts3GetVarint(pCsr, &iCol);
            }else{
              pCsr += sqlite3Fts3GetVarint(pCsr, &iVal);
              iDocid += iVal;
            }
          }else{
            iPos += (iVal - 2);
            cksum = cksum ^ fts3ChecksumEntry(
                csr.zTerm, csr.nTerm, iLangid, iIndex, iDocid,
                (int)iCol, (int)iPos
            );
          }
        }
      }
    }
  }
  sqlite3Fts3SegReaderFinish(&csr);

  *pRc = rc;
  return cksum;
}

/*
** Check if the contents of the FTS index match the current contents of the
** content table. If no error occurs and the contents do match, set *pbOk
** to true and return SQLITE_OK. Or if the contents do not match, set *pbOk
** to false before returning.
**
** If an error occurs (e.g. an OOM or IO error), return an SQLite error 
** code. The final value of *pbOk is undefined in this case.
*/
static int fts3IntegrityCheck(Fts3Table *p, int *pbOk){
  int rc = SQLITE_OK;             /* Return code */
  u64 cksum1 = 0;                 /* Checksum based on FTS index contents */
  u64 cksum2 = 0;                 /* Checksum based on %_content contents */
  sqlite3_stmt *pAllLangid = 0;   /* Statement to return all language-ids */

  /* This block calculates the checksum according to the FTS index. */
  rc = fts3SqlStmt(p, SQL_SELECT_ALL_LANGID, &pAllLangid, 0);
  if( rc==SQLITE_OK ){
    int rc2;
    sqlite3_bind_int(pAllLangid, 1, p->iPrevLangid);
    sqlite3_bind_int(pAllLangid, 2, p->nIndex);
    while( rc==SQLITE_OK && sqlite3_step(pAllLangid)==SQLITE_ROW ){
      int iLangid = sqlite3_column_int(pAllLangid, 0);
      int i;
      for(i=0; i<p->nIndex; i++){
        cksum1 = cksum1 ^ fts3ChecksumIndex(p, iLangid, i, &rc);
      }
    }
    rc2 = sqlite3_reset(pAllLangid);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  /* This block calculates the checksum according to the %_content table */
  if( rc==SQLITE_OK ){
    sqlite3_tokenizer_module const *pModule = p->pTokenizer->pModule;
    sqlite3_stmt *pStmt = 0;
    char *zSql;
   
    zSql = sqlite3_mprintf("SELECT %s" , p->zReadExprlist);
    if( !zSql ){
      rc = SQLITE_NOMEM;
    }else{
      rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
      sqlite3_free(zSql);
    }

    while( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
      i64 iDocid = sqlite3_column_int64(pStmt, 0);
      int iLang = langidFromSelect(p, pStmt);
      int iCol;

      for(iCol=0; rc==SQLITE_OK && iCol<p->nColumn; iCol++){
        if( p->abNotindexed[iCol]==0 ){
          const char *zText = (const char *)sqlite3_column_text(pStmt, iCol+1);
          int nText = sqlite3_column_bytes(pStmt, iCol+1);
          sqlite3_tokenizer_cursor *pT = 0;

          rc = sqlite3Fts3OpenTokenizer(p->pTokenizer, iLang, zText, nText,&pT);
          while( rc==SQLITE_OK ){
            char const *zToken;       /* Buffer containing token */
            int nToken = 0;           /* Number of bytes in token */
            int iDum1 = 0, iDum2 = 0; /* Dummy variables */
            int iPos = 0;             /* Position of token in zText */

            rc = pModule->xNext(pT, &zToken, &nToken, &iDum1, &iDum2, &iPos);
            if( rc==SQLITE_OK ){
              int i;
              cksum2 = cksum2 ^ fts3ChecksumEntry(
                  zToken, nToken, iLang, 0, iDocid, iCol, iPos
              );
              for(i=1; i<p->nIndex; i++){
                if( p->aIndex[i].nPrefix<=nToken ){
                  cksum2 = cksum2 ^ fts3ChecksumEntry(
                      zToken, p->aIndex[i].nPrefix, iLang, i, iDocid, iCol, iPos
                  );
                }
              }
            }
          }
          if( pT ) pModule->xClose(pT);
          if( rc==SQLITE_DONE ) rc = SQLITE_OK;
        }
      }
    }

    sqlite3_finalize(pStmt);
  }

  *pbOk = (cksum1==cksum2);
  return rc;
}

/*
** Run the integrity-check. If no error occurs and the current contents of
** the FTS index are correct, return SQLITE_OK. Or, if the contents of the
** FTS index are incorrect, return SQLITE_CORRUPT_VTAB.
**
** Or, if an error (e.g. an OOM or IO error) occurs, return an SQLite 
** error code.
**
** The integrity-check works as follows. For each token and indexed token
** prefix in the document set, a 64-bit checksum is calculated (by code
** in fts3ChecksumEntry()) based on the following:
**
**     + The index number (0 for the main index, 1 for the first prefix
**       index etc.),
**     + The token (or token prefix) text itself, 
**     + The language-id of the row it appears in,
**     + The docid of the row it appears in,
**     + The column it appears in, and
**     + The tokens position within that column.
**
** The checksums for all entries in the index are XORed together to create
** a single checksum for the entire index.
**
** The integrity-check code calculates the same checksum in two ways:
**
**     1. By scanning the contents of the FTS index, and 
**     2. By scanning and tokenizing the content table.
**
** If the two checksums are identical, the integrity-check is deemed to have
** passed.
*/
static int fts3DoIntegrityCheck(
  Fts3Table *p                    /* FTS3 table handle */
){
  int rc;
  int bOk = 0;
  rc = fts3IntegrityCheck(p, &bOk);
  if( rc==SQLITE_OK && bOk==0 ) rc = FTS_CORRUPT_VTAB;
  return rc;
}

/*
** Handle a 'special' INSERT of the form:
**
**   "INSERT INTO tbl(tbl) VALUES(<expr>)"
**
** Argument pVal contains the result of <expr>. Currently the only 
** meaningful value to insert is the text 'optimize'.
*/
static int fts3SpecialInsert(Fts3Table *p, sqlite3_value *pVal){
  int rc;                         /* Return Code */
  const char *zVal = (const char *)sqlite3_value_text(pVal);
  int nVal = sqlite3_value_bytes(pVal);

  if( !zVal ){
    return SQLITE_NOMEM;
  }else if( nVal==8 && 0==sqlite3_strnicmp(zVal, "optimize", 8) ){
    rc = fts3DoOptimize(p, 0);
  }else if( nVal==7 && 0==sqlite3_strnicmp(zVal, "rebuild", 7) ){
    rc = fts3DoRebuild(p);
  }else if( nVal==15 && 0==sqlite3_strnicmp(zVal, "integrity-check", 15) ){
    rc = fts3DoIntegrityCheck(p);
  }else if( nVal>6 && 0==sqlite3_strnicmp(zVal, "merge=", 6) ){
    rc = fts3DoIncrmerge(p, &zVal[6]);
  }else if( nVal>10 && 0==sqlite3_strnicmp(zVal, "automerge=", 10) ){
    rc = fts3DoAutoincrmerge(p, &zVal[10]);
#ifdef SQLITE_TEST
  }else if( nVal>9 && 0==sqlite3_strnicmp(zVal, "nodesize=", 9) ){
    p->nNodeSize = atoi(&zVal[9]);
    rc = SQLITE_OK;
  }else if( nVal>11 && 0==sqlite3_strnicmp(zVal, "maxpending=", 9) ){
    p->nMaxPendingData = atoi(&zVal[11]);
    rc = SQLITE_OK;
  }else if( nVal>21 && 0==sqlite3_strnicmp(zVal, "test-no-incr-doclist=", 21) ){
    p->bNoIncrDoclist = atoi(&zVal[21]);
    rc = SQLITE_OK;
#endif
  }else{
    rc = SQLITE_ERROR;
  }

  return rc;
}

#ifndef SQLITE_DISABLE_FTS4_DEFERRED
/*
** Delete all cached deferred doclists. Deferred doclists are cached
** (allocated) by the sqlite3Fts3CacheDeferredDoclists() function.
*/
SQLITE_PRIVATE void sqlite3Fts3FreeDeferredDoclists(Fts3Cursor *pCsr){
  Fts3DeferredToken *pDef;
  for(pDef=pCsr->pDeferred; pDef; pDef=pDef->pNext){
    fts3PendingListDelete(pDef->pList);
    pDef->pList = 0;
  }
}

/*
** Free all entries in the pCsr->pDeffered list. Entries are added to 
** this list using sqlite3Fts3DeferToken().
*/
SQLITE_PRIVATE void sqlite3Fts3FreeDeferredTokens(Fts3Cursor *pCsr){
  Fts3DeferredToken *pDef;
  Fts3DeferredToken *pNext;
  for(pDef=pCsr->pDeferred; pDef; pDef=pNext){
    pNext = pDef->pNext;
    fts3PendingListDelete(pDef->pList);
    sqlite3_free(pDef);
  }
  pCsr->pDeferred = 0;
}

/*
** Generate deferred-doclists for all tokens in the pCsr->pDeferred list
** based on the row that pCsr currently points to.
**
** A deferred-doclist is like any other doclist with position information
** included, except that it only contains entries for a single row of the
** table, not for all rows.
*/
SQLITE_PRIVATE int sqlite3Fts3CacheDeferredDoclists(Fts3Cursor *pCsr){
  int rc = SQLITE_OK;             /* Return code */
  if( pCsr->pDeferred ){
    int i;                        /* Used to iterate through table columns */
    sqlite3_int64 iDocid;         /* Docid of the row pCsr points to */
    Fts3DeferredToken *pDef;      /* Used to iterate through deferred tokens */
  
    Fts3Table *p = (Fts3Table *)pCsr->base.pVtab;
    sqlite3_tokenizer *pT = p->pTokenizer;
    sqlite3_tokenizer_module const *pModule = pT->pModule;
   
    assert( pCsr->isRequireSeek==0 );
    iDocid = sqlite3_column_int64(pCsr->pStmt, 0);
  
    for(i=0; i<p->nColumn && rc==SQLITE_OK; i++){
      if( p->abNotindexed[i]==0 ){
        const char *zText = (const char *)sqlite3_column_text(pCsr->pStmt, i+1);
        sqlite3_tokenizer_cursor *pTC = 0;

        rc = sqlite3Fts3OpenTokenizer(pT, pCsr->iLangid, zText, -1, &pTC);
        while( rc==SQLITE_OK ){
          char const *zToken;       /* Buffer containing token */
          int nToken = 0;           /* Number of bytes in token */
          int iDum1 = 0, iDum2 = 0; /* Dummy variables */
          int iPos = 0;             /* Position of token in zText */

          rc = pModule->xNext(pTC, &zToken, &nToken, &iDum1, &iDum2, &iPos);
          for(pDef=pCsr->pDeferred; pDef && rc==SQLITE_OK; pDef=pDef->pNext){
            Fts3PhraseToken *pPT = pDef->pToken;
            if( (pDef->iCol>=p->nColumn || pDef->iCol==i)
                && (pPT->bFirst==0 || iPos==0)
                && (pPT->n==nToken || (pPT->isPrefix && pPT->n<nToken))
                && (0==memcmp(zToken, pPT->z, pPT->n))
              ){
              fts3PendingListAppend(&pDef->pList, iDocid, i, iPos, &rc);
            }
          }
        }
        if( pTC ) pModule->xClose(pTC);
        if( rc==SQLITE_DONE ) rc = SQLITE_OK;
      }
    }

    for(pDef=pCsr->pDeferred; pDef && rc==SQLITE_OK; pDef=pDef->pNext){
      if( pDef->pList ){
        rc = fts3PendingListAppendVarint(&pDef->pList, 0);
      }
    }
  }

  return rc;
}

SQLITE_PRIVATE int sqlite3Fts3DeferredTokenList(
  Fts3DeferredToken *p, 
  char **ppData, 
  int *pnData
){
  char *pRet;
  int nSkip;
  sqlite3_int64 dummy;

  *ppData = 0;
  *pnData = 0;

  if( p->pList==0 ){
    return SQLITE_OK;
  }

  pRet = (char *)sqlite3_malloc(p->pList->nData);
  if( !pRet ) return SQLITE_NOMEM;

  nSkip = sqlite3Fts3GetVarint(p->pList->aData, &dummy);
  *pnData = p->pList->nData - nSkip;
  *ppData = pRet;
  
  memcpy(pRet, &p->pList->aData[nSkip], *pnData);
  return SQLITE_OK;
}

/*
** Add an entry for token pToken to the pCsr->pDeferred list.
*/
SQLITE_PRIVATE int sqlite3Fts3DeferToken(
  Fts3Cursor *pCsr,               /* Fts3 table cursor */
  Fts3PhraseToken *pToken,        /* Token to defer */
  int iCol                        /* Column that token must appear in (or -1) */
){
  Fts3DeferredToken *pDeferred;
  pDeferred = sqlite3_malloc(sizeof(*pDeferred));
  if( !pDeferred ){
    return SQLITE_NOMEM;
  }
  memset(pDeferred, 0, sizeof(*pDeferred));
  pDeferred->pToken = pToken;
  pDeferred->pNext = pCsr->pDeferred; 
  pDeferred->iCol = iCol;
  pCsr->pDeferred = pDeferred;

  assert( pToken->pDeferred==0 );
  pToken->pDeferred = pDeferred;

  return SQLITE_OK;
}
#endif

/*
** SQLite value pRowid contains the rowid of a row that may or may not be
** present in the FTS3 table. If it is, delete it and adjust the contents
** of subsiduary data structures accordingly.
*/
static int fts3DeleteByRowid(
  Fts3Table *p, 
  sqlite3_value *pRowid, 
  int *pnChng,                    /* IN/OUT: Decrement if row is deleted */
  u32 *aSzDel
){
  int rc = SQLITE_OK;             /* Return code */
  int bFound = 0;                 /* True if *pRowid really is in the table */

  fts3DeleteTerms(&rc, p, pRowid, aSzDel, &bFound);
  if( bFound && rc==SQLITE_OK ){
    int isEmpty = 0;              /* Deleting *pRowid leaves the table empty */
    rc = fts3IsEmpty(p, pRowid, &isEmpty);
    if( rc==SQLITE_OK ){
      if( isEmpty ){
        /* Deleting this row means the whole table is empty. In this case
        ** delete the contents of all three tables and throw away any
        ** data in the pendingTerms hash table.  */
        rc = fts3DeleteAll(p, 1);
        *pnChng = 0;
        memset(aSzDel, 0, sizeof(u32) * (p->nColumn+1) * 2);
      }else{
        *pnChng = *pnChng - 1;
        if( p->zContentTbl==0 ){
          fts3SqlExec(&rc, p, SQL_DELETE_CONTENT, &pRowid);
        }
        if( p->bHasDocsize ){
          fts3SqlExec(&rc, p, SQL_DELETE_DOCSIZE, &pRowid);
        }
      }
    }
  }

  return rc;
}

/*
** This function does the work for the xUpdate method of FTS3 virtual
** tables. The schema of the virtual table being:
**
**     CREATE TABLE <table name>( 
**       <user columns>,
**       <table name> HIDDEN, 
**       docid HIDDEN, 
**       <langid> HIDDEN
**     );
**
** 
*/
SQLITE_PRIVATE int sqlite3Fts3UpdateMethod(
  sqlite3_vtab *pVtab,            /* FTS3 vtab object */
  int nArg,                       /* Size of argument array */
  sqlite3_value **apVal,          /* Array of arguments */
  sqlite_int64 *pRowid            /* OUT: The affected (or effected) rowid */
){
  Fts3Table *p = (Fts3Table *)pVtab;
  int rc = SQLITE_OK;             /* Return Code */
  int isRemove = 0;               /* True for an UPDATE or DELETE */
  u32 *aSzIns = 0;                /* Sizes of inserted documents */
  u32 *aSzDel = 0;                /* Sizes of deleted documents */
  int nChng = 0;                  /* Net change in number of documents */
  int bInsertDone = 0;

  /* At this point it must be known if the %_stat table exists or not.
  ** So bHasStat may not be 2.  */
  assert( p->bHasStat==0 || p->bHasStat==1 );

  assert( p->pSegments==0 );
  assert( 
      nArg==1                     /* DELETE operations */
   || nArg==(2 + p->nColumn + 3)  /* INSERT or UPDATE operations */
  );

  /* Check for a "special" INSERT operation. One of the form:
  **
  **   INSERT INTO xyz(xyz) VALUES('command');
  */
  if( nArg>1 
   && sqlite3_value_type(apVal[0])==SQLITE_NULL 
   && sqlite3_value_type(apVal[p->nColumn+2])!=SQLITE_NULL 
  ){
    rc = fts3SpecialInsert(p, apVal[p->nColumn+2]);
    goto update_out;
  }

  if( nArg>1 && sqlite3_value_int(apVal[2 + p->nColumn + 2])<0 ){
    rc = SQLITE_CONSTRAINT;
    goto update_out;
  }

  /* Allocate space to hold the change in document sizes */
  aSzDel = sqlite3_malloc( sizeof(aSzDel[0])*(p->nColumn+1)*2 );
  if( aSzDel==0 ){
    rc = SQLITE_NOMEM;
    goto update_out;
  }
  aSzIns = &aSzDel[p->nColumn+1];
  memset(aSzDel, 0, sizeof(aSzDel[0])*(p->nColumn+1)*2);

  rc = fts3Writelock(p);
  if( rc!=SQLITE_OK ) goto update_out;

  /* If this is an INSERT operation, or an UPDATE that modifies the rowid
  ** value, then this operation requires constraint handling.
  **
  ** If the on-conflict mode is REPLACE, this means that the existing row
  ** should be deleted from the database before inserting the new row. Or,
  ** if the on-conflict mode is other than REPLACE, then this method must
  ** detect the conflict and return SQLITE_CONSTRAINT before beginning to
  ** modify the database file.
  */
  if( nArg>1 && p->zContentTbl==0 ){
    /* Find the value object that holds the new rowid value. */
    sqlite3_value *pNewRowid = apVal[3+p->nColumn];
    if( sqlite3_value_type(pNewRowid)==SQLITE_NULL ){
      pNewRowid = apVal[1];
    }

    if( sqlite3_value_type(pNewRowid)!=SQLITE_NULL && ( 
        sqlite3_value_type(apVal[0])==SQLITE_NULL
     || sqlite3_value_int64(apVal[0])!=sqlite3_value_int64(pNewRowid)
    )){
      /* The new rowid is not NULL (in this case the rowid will be
      ** automatically assigned and there is no chance of a conflict), and 
      ** the statement is either an INSERT or an UPDATE that modifies the
      ** rowid column. So if the conflict mode is REPLACE, then delete any
      ** existing row with rowid=pNewRowid. 
      **
      ** Or, if the conflict mode is not REPLACE, insert the new record into 
      ** the %_content table. If we hit the duplicate rowid constraint (or any
      ** other error) while doing so, return immediately.
      **
      ** This branch may also run if pNewRowid contains a value that cannot
      ** be losslessly converted to an integer. In this case, the eventual 
      ** call to fts3InsertData() (either just below or further on in this
      ** function) will return SQLITE_MISMATCH. If fts3DeleteByRowid is 
      ** invoked, it will delete zero rows (since no row will have
      ** docid=$pNewRowid if $pNewRowid is not an integer value).
      */
      if( sqlite3_vtab_on_conflict(p->db)==SQLITE_REPLACE ){
        rc = fts3DeleteByRowid(p, pNewRowid, &nChng, aSzDel);
      }else{
        rc = fts3InsertData(p, apVal, pRowid);
        bInsertDone = 1;
      }
    }
  }
  if( rc!=SQLITE_OK ){
    goto update_out;
  }

  /* If this is a DELETE or UPDATE operation, remove the old record. */
  if( sqlite3_value_type(apVal[0])!=SQLITE_NULL ){
    assert( sqlite3_value_type(apVal[0])==SQLITE_INTEGER );
    rc = fts3DeleteByRowid(p, apVal[0], &nChng, aSzDel);
    isRemove = 1;
  }
  
  /* If this is an INSERT or UPDATE operation, insert the new record. */
  if( nArg>1 && rc==SQLITE_OK ){
    int iLangid = sqlite3_value_int(apVal[2 + p->nColumn + 2]);
    if( bInsertDone==0 ){
      rc = fts3InsertData(p, apVal, pRowid);
      if( rc==SQLITE_CONSTRAINT && p->zContentTbl==0 ){
        rc = FTS_CORRUPT_VTAB;
      }
    }
    if( rc==SQLITE_OK && (!isRemove || *pRowid!=p->iPrevDocid ) ){
      rc = fts3PendingTermsDocid(p, iLangid, *pRowid);
    }
    if( rc==SQLITE_OK ){
      assert( p->iPrevDocid==*pRowid );
      rc = fts3InsertTerms(p, iLangid, apVal, aSzIns);
    }
    if( p->bHasDocsize ){
      fts3InsertDocsize(&rc, p, aSzIns);
    }
    nChng++;
  }

  if( p->bFts4 ){
    fts3UpdateDocTotals(&rc, p, aSzIns, aSzDel, nChng);
  }

 update_out:
  sqlite3_free(aSzDel);
  sqlite3Fts3SegmentsClose(p);
  return rc;
}

/* 
** Flush any data in the pending-terms hash table to disk. If successful,
** merge all segments in the database (including the new segment, if 
** there was any data to flush) into a single segment. 
*/
SQLITE_PRIVATE int sqlite3Fts3Optimize(Fts3Table *p){
  int rc;
  rc = sqlite3_exec(p->db, "SAVEPOINT fts3", 0, 0, 0);
  if( rc==SQLITE_OK ){
    rc = fts3DoOptimize(p, 1);
    if( rc==SQLITE_OK || rc==SQLITE_DONE ){
      int rc2 = sqlite3_exec(p->db, "RELEASE fts3", 0, 0, 0);
      if( rc2!=SQLITE_OK ) rc = rc2;
    }else{
      sqlite3_exec(p->db, "ROLLBACK TO fts3", 0, 0, 0);
      sqlite3_exec(p->db, "RELEASE fts3", 0, 0, 0);
    }
  }
  sqlite3Fts3SegmentsClose(p);
  return rc;
}

#endif

/************** End of fts3_write.c ******************************************/
/************** Begin file fts3_snippet.c ************************************/
/*
** 2009 Oct 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
*/

/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <string.h> */
/* #include <assert.h> */

/*
** Characters that may appear in the second argument to matchinfo().
*/
#define FTS3_MATCHINFO_NPHRASE   'p'        /* 1 value */
#define FTS3_MATCHINFO_NCOL      'c'        /* 1 value */
#define FTS3_MATCHINFO_NDOC      'n'        /* 1 value */
#define FTS3_MATCHINFO_AVGLENGTH 'a'        /* nCol values */
#define FTS3_MATCHINFO_LENGTH    'l'        /* nCol values */
#define FTS3_MATCHINFO_LCS       's'        /* nCol values */
#define FTS3_MATCHINFO_HITS      'x'        /* 3*nCol*nPhrase values */
#define FTS3_MATCHINFO_LHITS     'y'        /* nCol*nPhrase values */
#define FTS3_MATCHINFO_LHITS_BM  'b'        /* nCol*nPhrase values */

/*
** The default value for the second argument to matchinfo(). 
*/
#define FTS3_MATCHINFO_DEFAULT   "pcx"


/*
** Used as an fts3ExprIterate() context when loading phrase doclists to
** Fts3Expr.aDoclist[]/nDoclist.
*/
typedef struct LoadDoclistCtx LoadDoclistCtx;
struct LoadDoclistCtx {
  Fts3Cursor *pCsr;               /* FTS3 Cursor */
  int nPhrase;                    /* Number of phrases seen so far */
  int nToken;                     /* Number of tokens seen so far */
};

/*
** The following types are used as part of the implementation of the 
** fts3BestSnippet() routine.
*/
typedef struct SnippetIter SnippetIter;
typedef struct SnippetPhrase SnippetPhrase;
typedef struct SnippetFragment SnippetFragment;

struct SnippetIter {
  Fts3Cursor *pCsr;               /* Cursor snippet is being generated from */
  int iCol;                       /* Extract snippet from this column */
  int nSnippet;                   /* Requested snippet length (in tokens) */
  int nPhrase;                    /* Number of phrases in query */
  SnippetPhrase *aPhrase;         /* Array of size nPhrase */
  int iCurrent;                   /* First token of current snippet */
};

struct SnippetPhrase {
  int nToken;                     /* Number of tokens in phrase */
  char *pList;                    /* Pointer to start of phrase position list */
  int iHead;                      /* Next value in position list */
  char *pHead;                    /* Position list data following iHead */
  int iTail;                      /* Next value in trailing position list */
  char *pTail;                    /* Position list data following iTail */
};

struct SnippetFragment {
  int iCol;                       /* Column snippet is extracted from */
  int iPos;                       /* Index of first token in snippet */
  u64 covered;                    /* Mask of query phrases covered */
  u64 hlmask;                     /* Mask of snippet terms to highlight */
};

/*
** This type is used as an fts3ExprIterate() context object while 
** accumulating the data returned by the matchinfo() function.
*/
typedef struct MatchInfo MatchInfo;
struct MatchInfo {
  Fts3Cursor *pCursor;            /* FTS3 Cursor */
  int nCol;                       /* Number of columns in table */
  int nPhrase;                    /* Number of matchable phrases in query */
  sqlite3_int64 nDoc;             /* Number of docs in database */
  char flag;
  u32 *aMatchinfo;                /* Pre-allocated buffer */
};

/*
** An instance of this structure is used to manage a pair of buffers, each
** (nElem * sizeof(u32)) bytes in size. See the MatchinfoBuffer code below
** for details.
*/
struct MatchinfoBuffer {
  u8 aRef[3];
  int nElem;
  int bGlobal;                    /* Set if global data is loaded */
  char *zMatchinfo;
  u32 aMatchinfo[1];
};


/*
** The snippet() and offsets() functions both return text values. An instance
** of the following structure is used to accumulate those values while the
** functions are running. See fts3StringAppend() for details.
*/
typedef struct StrBuffer StrBuffer;
struct StrBuffer {
  char *z;                        /* Pointer to buffer containing string */
  int n;                          /* Length of z in bytes (excl. nul-term) */
  int nAlloc;                     /* Allocated size of buffer z in bytes */
};


/*************************************************************************
** Start of MatchinfoBuffer code.
*/

/*
** Allocate a two-slot MatchinfoBuffer object.
*/
static MatchinfoBuffer *fts3MIBufferNew(int nElem, const char *zMatchinfo){
  MatchinfoBuffer *pRet;
  int nByte = sizeof(u32) * (2*nElem + 1) + sizeof(MatchinfoBuffer);
  int nStr = (int)strlen(zMatchinfo);

  pRet = sqlite3_malloc(nByte + nStr+1);
  if( pRet ){
    memset(pRet, 0, nByte);
    pRet->aMatchinfo[0] = (u8*)(&pRet->aMatchinfo[1]) - (u8*)pRet;
    pRet->aMatchinfo[1+nElem] = pRet->aMatchinfo[0] + sizeof(u32)*(nElem+1);
    pRet->nElem = nElem;
    pRet->zMatchinfo = ((char*)pRet) + nByte;
    memcpy(pRet->zMatchinfo, zMatchinfo, nStr+1);
    pRet->aRef[0] = 1;
  }

  return pRet;
}

static void fts3MIBufferFree(void *p){
  MatchinfoBuffer *pBuf = (MatchinfoBuffer*)((u8*)p - ((u32*)p)[-1]);

  assert( (u32*)p==&pBuf->aMatchinfo[1] 
       || (u32*)p==&pBuf->aMatchinfo[pBuf->nElem+2] 
  );
  if( (u32*)p==&pBuf->aMatchinfo[1] ){
    pBuf->aRef[1] = 0;
  }else{
    pBuf->aRef[2] = 0;
  }

  if( pBuf->aRef[0]==0 && pBuf->aRef[1]==0 && pBuf->aRef[2]==0 ){
    sqlite3_free(pBuf);
  }
}

static void (*fts3MIBufferAlloc(MatchinfoBuffer *p, u32 **paOut))(void*){
  void (*xRet)(void*) = 0;
  u32 *aOut = 0;

  if( p->aRef[1]==0 ){
    p->aRef[1] = 1;
    aOut = &p->aMatchinfo[1];
    xRet = fts3MIBufferFree;
  }
  else if( p->aRef[2]==0 ){
    p->aRef[2] = 1;
    aOut = &p->aMatchinfo[p->nElem+2];
    xRet = fts3MIBufferFree;
  }else{
    aOut = (u32*)sqlite3_malloc(p->nElem * sizeof(u32));
    if( aOut ){
      xRet = sqlite3_free;
      if( p->bGlobal ) memcpy(aOut, &p->aMatchinfo[1], p->nElem*sizeof(u32));
    }
  }

  *paOut = aOut;
  return xRet;
}

static void fts3MIBufferSetGlobal(MatchinfoBuffer *p){
  p->bGlobal = 1;
  memcpy(&p->aMatchinfo[2+p->nElem], &p->aMatchinfo[1], p->nElem*sizeof(u32));
}

/*
** Free a MatchinfoBuffer object allocated using fts3MIBufferNew()
*/
SQLITE_PRIVATE void sqlite3Fts3MIBufferFree(MatchinfoBuffer *p){
  if( p ){
    assert( p->aRef[0]==1 );
    p->aRef[0] = 0;
    if( p->aRef[0]==0 && p->aRef[1]==0 && p->aRef[2]==0 ){
      sqlite3_free(p);
    }
  }
}

/* 
** End of MatchinfoBuffer code.
*************************************************************************/


/*
** This function is used to help iterate through a position-list. A position
** list is a list of unique integers, sorted from smallest to largest. Each
** element of the list is represented by an FTS3 varint that takes the value
** of the difference between the current element and the previous one plus
** two. For example, to store the position-list:
**
**     4 9 113
**
** the three varints:
**
**     6 7 106
**
** are encoded.
**
** When this function is called, *pp points to the start of an element of
** the list. *piPos contains the value of the previous entry in the list.
** After it returns, *piPos contains the value of the next element of the
** list and *pp is advanced to the following varint.
*/
static void fts3GetDeltaPosition(char **pp, int *piPos){
  int iVal;
  *pp += fts3GetVarint32(*pp, &iVal);
  *piPos += (iVal-2);
}

/*
** Helper function for fts3ExprIterate() (see below).
*/
static int fts3ExprIterate2(
  Fts3Expr *pExpr,                /* Expression to iterate phrases of */
  int *piPhrase,                  /* Pointer to phrase counter */
  int (*x)(Fts3Expr*,int,void*),  /* Callback function to invoke for phrases */
  void *pCtx                      /* Second argument to pass to callback */
){
  int rc;                         /* Return code */
  int eType = pExpr->eType;     /* Type of expression node pExpr */

  if( eType!=FTSQUERY_PHRASE ){
    assert( pExpr->pLeft && pExpr->pRight );
    rc = fts3ExprIterate2(pExpr->pLeft, piPhrase, x, pCtx);
    if( rc==SQLITE_OK && eType!=FTSQUERY_NOT ){
      rc = fts3ExprIterate2(pExpr->pRight, piPhrase, x, pCtx);
    }
  }else{
    rc = x(pExpr, *piPhrase, pCtx);
    (*piPhrase)++;
  }
  return rc;
}

/*
** Iterate through all phrase nodes in an FTS3 query, except those that
** are part of a sub-tree that is the right-hand-side of a NOT operator.
** For each phrase node found, the supplied callback function is invoked.
**
** If the callback function returns anything other than SQLITE_OK, 
** the iteration is abandoned and the error code returned immediately.
** Otherwise, SQLITE_OK is returned after a callback has been made for
** all eligible phrase nodes.
*/
static int fts3ExprIterate(
  Fts3Expr *pExpr,                /* Expression to iterate phrases of */
  int (*x)(Fts3Expr*,int,void*),  /* Callback function to invoke for phrases */
  void *pCtx                      /* Second argument to pass to callback */
){
  int iPhrase = 0;                /* Variable used as the phrase counter */
  return fts3ExprIterate2(pExpr, &iPhrase, x, pCtx);
}


/*
** This is an fts3ExprIterate() callback used while loading the doclists
** for each phrase into Fts3Expr.aDoclist[]/nDoclist. See also
** fts3ExprLoadDoclists().
*/
static int fts3ExprLoadDoclistsCb(Fts3Expr *pExpr, int iPhrase, void *ctx){
  int rc = SQLITE_OK;
  Fts3Phrase *pPhrase = pExpr->pPhrase;
  LoadDoclistCtx *p = (LoadDoclistCtx *)ctx;

  UNUSED_PARAMETER(iPhrase);

  p->nPhrase++;
  p->nToken += pPhrase->nToken;

  return rc;
}

/*
** Load the doclists for each phrase in the query associated with FTS3 cursor
** pCsr. 
**
** If pnPhrase is not NULL, then *pnPhrase is set to the number of matchable 
** phrases in the expression (all phrases except those directly or 
** indirectly descended from the right-hand-side of a NOT operator). If 
** pnToken is not NULL, then it is set to the number of tokens in all
** matchable phrases of the expression.
*/
static int fts3ExprLoadDoclists(
  Fts3Cursor *pCsr,               /* Fts3 cursor for current query */
  int *pnPhrase,                  /* OUT: Number of phrases in query */
  int *pnToken                    /* OUT: Number of tokens in query */
){
  int rc;                         /* Return Code */
  LoadDoclistCtx sCtx = {0,0,0};  /* Context for fts3ExprIterate() */
  sCtx.pCsr = pCsr;
  rc = fts3ExprIterate(pCsr->pExpr, fts3ExprLoadDoclistsCb, (void *)&sCtx);
  if( pnPhrase ) *pnPhrase = sCtx.nPhrase;
  if( pnToken ) *pnToken = sCtx.nToken;
  return rc;
}

static int fts3ExprPhraseCountCb(Fts3Expr *pExpr, int iPhrase, void *ctx){
  (*(int *)ctx)++;
  pExpr->iPhrase = iPhrase;
  return SQLITE_OK;
}
static int fts3ExprPhraseCount(Fts3Expr *pExpr){
  int nPhrase = 0;
  (void)fts3ExprIterate(pExpr, fts3ExprPhraseCountCb, (void *)&nPhrase);
  return nPhrase;
}

/*
** Advance the position list iterator specified by the first two 
** arguments so that it points to the first element with a value greater
** than or equal to parameter iNext.
*/
static void fts3SnippetAdvance(char **ppIter, int *piIter, int iNext){
  char *pIter = *ppIter;
  if( pIter ){
    int iIter = *piIter;

    while( iIter<iNext ){
      if( 0==(*pIter & 0xFE) ){
        iIter = -1;
        pIter = 0;
        break;
      }
      fts3GetDeltaPosition(&pIter, &iIter);
    }

    *piIter = iIter;
    *ppIter = pIter;
  }
}

/*
** Advance the snippet iterator to the next candidate snippet.
*/
static int fts3SnippetNextCandidate(SnippetIter *pIter){
  int i;                          /* Loop counter */

  if( pIter->iCurrent<0 ){
    /* The SnippetIter object has just been initialized. The first snippet
    ** candidate always starts at offset 0 (even if this candidate has a
    ** score of 0.0).
    */
    pIter->iCurrent = 0;

    /* Advance the 'head' iterator of each phrase to the first offset that
    ** is greater than or equal to (iNext+nSnippet).
    */
    for(i=0; i<pIter->nPhrase; i++){
      SnippetPhrase *pPhrase = &pIter->aPhrase[i];
      fts3SnippetAdvance(&pPhrase->pHead, &pPhrase->iHead, pIter->nSnippet);
    }
  }else{
    int iStart;
    int iEnd = 0x7FFFFFFF;

    for(i=0; i<pIter->nPhrase; i++){
      SnippetPhrase *pPhrase = &pIter->aPhrase[i];
      if( pPhrase->pHead && pPhrase->iHead<iEnd ){
        iEnd = pPhrase->iHead;
      }
    }
    if( iEnd==0x7FFFFFFF ){
      return 1;
    }

    pIter->iCurrent = iStart = iEnd - pIter->nSnippet + 1;
    for(i=0; i<pIter->nPhrase; i++){
      SnippetPhrase *pPhrase = &pIter->aPhrase[i];
      fts3SnippetAdvance(&pPhrase->pHead, &pPhrase->iHead, iEnd+1);
      fts3SnippetAdvance(&pPhrase->pTail, &pPhrase->iTail, iStart);
    }
  }

  return 0;
}

/*
** Retrieve information about the current candidate snippet of snippet 
** iterator pIter.
*/
static void fts3SnippetDetails(
  SnippetIter *pIter,             /* Snippet iterator */
  u64 mCovered,                   /* Bitmask of phrases already covered */
  int *piToken,                   /* OUT: First token of proposed snippet */
  int *piScore,                   /* OUT: "Score" for this snippet */
  u64 *pmCover,                   /* OUT: Bitmask of phrases covered */
  u64 *pmHighlight                /* OUT: Bitmask of terms to highlight */
){
  int iStart = pIter->iCurrent;   /* First token of snippet */
  int iScore = 0;                 /* Score of this snippet */
  int i;                          /* Loop counter */
  u64 mCover = 0;                 /* Mask of phrases covered by this snippet */
  u64 mHighlight = 0;             /* Mask of tokens to highlight in snippet */

  for(i=0; i<pIter->nPhrase; i++){
    SnippetPhrase *pPhrase = &pIter->aPhrase[i];
    if( pPhrase->pTail ){
      char *pCsr = pPhrase->pTail;
      int iCsr = pPhrase->iTail;

      while( iCsr<(iStart+pIter->nSnippet) ){
        int j;
        u64 mPhrase = (u64)1 << i;
        u64 mPos = (u64)1 << (iCsr - iStart);
        assert( iCsr>=iStart );
        if( (mCover|mCovered)&mPhrase ){
          iScore++;
        }else{
          iScore += 1000;
        }
        mCover |= mPhrase;

        for(j=0; j<pPhrase->nToken; j++){
          mHighlight |= (mPos>>j);
        }

        if( 0==(*pCsr & 0x0FE) ) break;
        fts3GetDeltaPosition(&pCsr, &iCsr);
      }
    }
  }

  /* Set the output variables before returning. */
  *piToken = iStart;
  *piScore = iScore;
  *pmCover = mCover;
  *pmHighlight = mHighlight;
}

/*
** This function is an fts3ExprIterate() callback used by fts3BestSnippet().
** Each invocation populates an element of the SnippetIter.aPhrase[] array.
*/
static int fts3SnippetFindPositions(Fts3Expr *pExpr, int iPhrase, void *ctx){
  SnippetIter *p = (SnippetIter *)ctx;
  SnippetPhrase *pPhrase = &p->aPhrase[iPhrase];
  char *pCsr;
  int rc;

  pPhrase->nToken = pExpr->pPhrase->nToken;
  rc = sqlite3Fts3EvalPhrasePoslist(p->pCsr, pExpr, p->iCol, &pCsr);
  assert( rc==SQLITE_OK || pCsr==0 );
  if( pCsr ){
    int iFirst = 0;
    pPhrase->pList = pCsr;
    fts3GetDeltaPosition(&pCsr, &iFirst);
    assert( iFirst>=0 );
    pPhrase->pHead = pCsr;
    pPhrase->pTail = pCsr;
    pPhrase->iHead = iFirst;
    pPhrase->iTail = iFirst;
  }else{
    assert( rc!=SQLITE_OK || (
       pPhrase->pList==0 && pPhrase->pHead==0 && pPhrase->pTail==0 
    ));
  }

  return rc;
}

/*
** Select the fragment of text consisting of nFragment contiguous tokens 
** from column iCol that represent the "best" snippet. The best snippet
** is the snippet with the highest score, where scores are calculated
** by adding:
**
**   (a) +1 point for each occurrence of a matchable phrase in the snippet.
**
**   (b) +1000 points for the first occurrence of each matchable phrase in 
**       the snippet for which the corresponding mCovered bit is not set.
**
** The selected snippet parameters are stored in structure *pFragment before
** returning. The score of the selected snippet is stored in *piScore
** before returning.
*/
static int fts3BestSnippet(
  int nSnippet,                   /* Desired snippet length */
  Fts3Cursor *pCsr,               /* Cursor to create snippet for */
  int iCol,                       /* Index of column to create snippet from */
  u64 mCovered,                   /* Mask of phrases already covered */
  u64 *pmSeen,                    /* IN/OUT: Mask of phrases seen */
  SnippetFragment *pFragment,     /* OUT: Best snippet found */
  int *piScore                    /* OUT: Score of snippet pFragment */
){
  int rc;                         /* Return Code */
  int nList;                      /* Number of phrases in expression */
  SnippetIter sIter;              /* Iterates through snippet candidates */
  int nByte;                      /* Number of bytes of space to allocate */
  int iBestScore = -1;            /* Best snippet score found so far */
  int i;                          /* Loop counter */

  memset(&sIter, 0, sizeof(sIter));

  /* Iterate through the phrases in the expression to count them. The same
  ** callback makes sure the doclists are loaded for each phrase.
  */
  rc = fts3ExprLoadDoclists(pCsr, &nList, 0);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* Now that it is known how many phrases there are, allocate and zero
  ** the required space using malloc().
  */
  nByte = sizeof(SnippetPhrase) * nList;
  sIter.aPhrase = (SnippetPhrase *)sqlite3_malloc(nByte);
  if( !sIter.aPhrase ){
    return SQLITE_NOMEM;
  }
  memset(sIter.aPhrase, 0, nByte);

  /* Initialize the contents of the SnippetIter object. Then iterate through
  ** the set of phrases in the expression to populate the aPhrase[] array.
  */
  sIter.pCsr = pCsr;
  sIter.iCol = iCol;
  sIter.nSnippet = nSnippet;
  sIter.nPhrase = nList;
  sIter.iCurrent = -1;
  rc = fts3ExprIterate(pCsr->pExpr, fts3SnippetFindPositions, (void*)&sIter);
  if( rc==SQLITE_OK ){

    /* Set the *pmSeen output variable. */
    for(i=0; i<nList; i++){
      if( sIter.aPhrase[i].pHead ){
        *pmSeen |= (u64)1 << i;
      }
    }

    /* Loop through all candidate snippets. Store the best snippet in 
     ** *pFragment. Store its associated 'score' in iBestScore.
     */
    pFragment->iCol = iCol;
    while( !fts3SnippetNextCandidate(&sIter) ){
      int iPos;
      int iScore;
      u64 mCover;
      u64 mHighlite;
      fts3SnippetDetails(&sIter, mCovered, &iPos, &iScore, &mCover,&mHighlite);
      assert( iScore>=0 );
      if( iScore>iBestScore ){
        pFragment->iPos = iPos;
        pFragment->hlmask = mHighlite;
        pFragment->covered = mCover;
        iBestScore = iScore;
      }
    }

    *piScore = iBestScore;
  }
  sqlite3_free(sIter.aPhrase);
  return rc;
}


/*
** Append a string to the string-buffer passed as the first argument.
**
** If nAppend is negative, then the length of the string zAppend is
** determined using strlen().
*/
static int fts3StringAppend(
  StrBuffer *pStr,                /* Buffer to append to */
  const char *zAppend,            /* Pointer to data to append to buffer */
  int nAppend                     /* Size of zAppend in bytes (or -1) */
){
  if( nAppend<0 ){
    nAppend = (int)strlen(zAppend);
  }

  /* If there is insufficient space allocated at StrBuffer.z, use realloc()
  ** to grow the buffer until so that it is big enough to accomadate the
  ** appended data.
  */
  if( pStr->n+nAppend+1>=pStr->nAlloc ){
    int nAlloc = pStr->nAlloc+nAppend+100;
    char *zNew = sqlite3_realloc(pStr->z, nAlloc);
    if( !zNew ){
      return SQLITE_NOMEM;
    }
    pStr->z = zNew;
    pStr->nAlloc = nAlloc;
  }
  assert( pStr->z!=0 && (pStr->nAlloc >= pStr->n+nAppend+1) );

  /* Append the data to the string buffer. */
  memcpy(&pStr->z[pStr->n], zAppend, nAppend);
  pStr->n += nAppend;
  pStr->z[pStr->n] = '\0';

  return SQLITE_OK;
}

/*
** The fts3BestSnippet() function often selects snippets that end with a
** query term. That is, the final term of the snippet is always a term
** that requires highlighting. For example, if 'X' is a highlighted term
** and '.' is a non-highlighted term, BestSnippet() may select:
**
**     ........X.....X
**
** This function "shifts" the beginning of the snippet forward in the 
** document so that there are approximately the same number of 
** non-highlighted terms to the right of the final highlighted term as there
** are to the left of the first highlighted term. For example, to this:
**
**     ....X.....X....
**
** This is done as part of extracting the snippet text, not when selecting
** the snippet. Snippet selection is done based on doclists only, so there
** is no way for fts3BestSnippet() to know whether or not the document 
** actually contains terms that follow the final highlighted term. 
*/
static int fts3SnippetShift(
  Fts3Table *pTab,                /* FTS3 table snippet comes from */
  int iLangid,                    /* Language id to use in tokenizing */
  int nSnippet,                   /* Number of tokens desired for snippet */
  const char *zDoc,               /* Document text to extract snippet from */
  int nDoc,                       /* Size of buffer zDoc in bytes */
  int *piPos,                     /* IN/OUT: First token of snippet */
  u64 *pHlmask                    /* IN/OUT: Mask of tokens to highlight */
){
  u64 hlmask = *pHlmask;          /* Local copy of initial highlight-mask */

  if( hlmask ){
    int nLeft;                    /* Tokens to the left of first highlight */
    int nRight;                   /* Tokens to the right of last highlight */
    int nDesired;                 /* Ideal number of tokens to shift forward */

    for(nLeft=0; !(hlmask & ((u64)1 << nLeft)); nLeft++);
    for(nRight=0; !(hlmask & ((u64)1 << (nSnippet-1-nRight))); nRight++);
    nDesired = (nLeft-nRight)/2;

    /* Ideally, the start of the snippet should be pushed forward in the
    ** document nDesired tokens. This block checks if there are actually
    ** nDesired tokens to the right of the snippet. If so, *piPos and
    ** *pHlMask are updated to shift the snippet nDesired tokens to the
    ** right. Otherwise, the snippet is shifted by the number of tokens
    ** available.
    */
    if( nDesired>0 ){
      int nShift;                 /* Number of tokens to shift snippet by */
      int iCurrent = 0;           /* Token counter */
      int rc;                     /* Return Code */
      sqlite3_tokenizer_module *pMod;
      sqlite3_tokenizer_cursor *pC;
      pMod = (sqlite3_tokenizer_module *)pTab->pTokenizer->pModule;

      /* Open a cursor on zDoc/nDoc. Check if there are (nSnippet+nDesired)
      ** or more tokens in zDoc/nDoc.
      */
      rc = sqlite3Fts3OpenTokenizer(pTab->pTokenizer, iLangid, zDoc, nDoc, &pC);
      if( rc!=SQLITE_OK ){
        return rc;
      }
      while( rc==SQLITE_OK && iCurrent<(nSnippet+nDesired) ){
        const char *ZDUMMY; int DUMMY1 = 0, DUMMY2 = 0, DUMMY3 = 0;
        rc = pMod->xNext(pC, &ZDUMMY, &DUMMY1, &DUMMY2, &DUMMY3, &iCurrent);
      }
      pMod->xClose(pC);
      if( rc!=SQLITE_OK && rc!=SQLITE_DONE ){ return rc; }

      nShift = (rc==SQLITE_DONE)+iCurrent-nSnippet;
      assert( nShift<=nDesired );
      if( nShift>0 ){
        *piPos += nShift;
        *pHlmask = hlmask >> nShift;
      }
    }
  }
  return SQLITE_OK;
}

/*
** Extract the snippet text for fragment pFragment from cursor pCsr and
** append it to string buffer pOut.
*/
static int fts3SnippetText(
  Fts3Cursor *pCsr,               /* FTS3 Cursor */
  SnippetFragment *pFragment,     /* Snippet to extract */
  int iFragment,                  /* Fragment number */
  int isLast,                     /* True for final fragment in snippet */
  int nSnippet,                   /* Number of tokens in extracted snippet */
  const char *zOpen,              /* String inserted before highlighted term */
  const char *zClose,             /* String inserted after highlighted term */
  const char *zEllipsis,          /* String inserted between snippets */
  StrBuffer *pOut                 /* Write output here */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc;                         /* Return code */
  const char *zDoc;               /* Document text to extract snippet from */
  int nDoc;                       /* Size of zDoc in bytes */
  int iCurrent = 0;               /* Current token number of document */
  int iEnd = 0;                   /* Byte offset of end of current token */
  int isShiftDone = 0;            /* True after snippet is shifted */
  int iPos = pFragment->iPos;     /* First token of snippet */
  u64 hlmask = pFragment->hlmask; /* Highlight-mask for snippet */
  int iCol = pFragment->iCol+1;   /* Query column to extract text from */
  sqlite3_tokenizer_module *pMod; /* Tokenizer module methods object */
  sqlite3_tokenizer_cursor *pC;   /* Tokenizer cursor open on zDoc/nDoc */
  
  zDoc = (const char *)sqlite3_column_text(pCsr->pStmt, iCol);
  if( zDoc==0 ){
    if( sqlite3_column_type(pCsr->pStmt, iCol)!=SQLITE_NULL ){
      return SQLITE_NOMEM;
    }
    return SQLITE_OK;
  }
  nDoc = sqlite3_column_bytes(pCsr->pStmt, iCol);

  /* Open a token cursor on the document. */
  pMod = (sqlite3_tokenizer_module *)pTab->pTokenizer->pModule;
  rc = sqlite3Fts3OpenTokenizer(pTab->pTokenizer, pCsr->iLangid, zDoc,nDoc,&pC);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  while( rc==SQLITE_OK ){
    const char *ZDUMMY;           /* Dummy argument used with tokenizer */
    int DUMMY1 = -1;              /* Dummy argument used with tokenizer */
    int iBegin = 0;               /* Offset in zDoc of start of token */
    int iFin = 0;                 /* Offset in zDoc of end of token */
    int isHighlight = 0;          /* True for highlighted terms */

    /* Variable DUMMY1 is initialized to a negative value above. Elsewhere
    ** in the FTS code the variable that the third argument to xNext points to
    ** is initialized to zero before the first (*but not necessarily
    ** subsequent*) call to xNext(). This is done for a particular application
    ** that needs to know whether or not the tokenizer is being used for
    ** snippet generation or for some other purpose.
    **
    ** Extreme care is required when writing code to depend on this
    ** initialization. It is not a documented part of the tokenizer interface.
    ** If a tokenizer is used directly by any code outside of FTS, this
    ** convention might not be respected.  */
    rc = pMod->xNext(pC, &ZDUMMY, &DUMMY1, &iBegin, &iFin, &iCurrent);
    if( rc!=SQLITE_OK ){
      if( rc==SQLITE_DONE ){
        /* Special case - the last token of the snippet is also the last token
        ** of the column. Append any punctuation that occurred between the end
        ** of the previous token and the end of the document to the output. 
        ** Then break out of the loop. */
        rc = fts3StringAppend(pOut, &zDoc[iEnd], -1);
      }
      break;
    }
    if( iCurrent<iPos ){ continue; }

    if( !isShiftDone ){
      int n = nDoc - iBegin;
      rc = fts3SnippetShift(
          pTab, pCsr->iLangid, nSnippet, &zDoc[iBegin], n, &iPos, &hlmask
      );
      isShiftDone = 1;

      /* Now that the shift has been done, check if the initial "..." are
      ** required. They are required if (a) this is not the first fragment,
      ** or (b) this fragment does not begin at position 0 of its column. 
      */
      if( rc==SQLITE_OK ){
        if( iPos>0 || iFragment>0 ){
          rc = fts3StringAppend(pOut, zEllipsis, -1);
        }else if( iBegin ){
          rc = fts3StringAppend(pOut, zDoc, iBegin);
        }
      }
      if( rc!=SQLITE_OK || iCurrent<iPos ) continue;
    }

    if( iCurrent>=(iPos+nSnippet) ){
      if( isLast ){
        rc = fts3StringAppend(pOut, zEllipsis, -1);
      }
      break;
    }

    /* Set isHighlight to true if this term should be highlighted. */
    isHighlight = (hlmask & ((u64)1 << (iCurrent-iPos)))!=0;

    if( iCurrent>iPos ) rc = fts3StringAppend(pOut, &zDoc[iEnd], iBegin-iEnd);
    if( rc==SQLITE_OK && isHighlight ) rc = fts3StringAppend(pOut, zOpen, -1);
    if( rc==SQLITE_OK ) rc = fts3StringAppend(pOut, &zDoc[iBegin], iFin-iBegin);
    if( rc==SQLITE_OK && isHighlight ) rc = fts3StringAppend(pOut, zClose, -1);

    iEnd = iFin;
  }

  pMod->xClose(pC);
  return rc;
}


/*
** This function is used to count the entries in a column-list (a 
** delta-encoded list of term offsets within a single column of a single 
** row). When this function is called, *ppCollist should point to the
** beginning of the first varint in the column-list (the varint that
** contains the position of the first matching term in the column data).
** Before returning, *ppCollist is set to point to the first byte after
** the last varint in the column-list (either the 0x00 signifying the end
** of the position-list, or the 0x01 that precedes the column number of
** the next column in the position-list).
**
** The number of elements in the column-list is returned.
*/
static int fts3ColumnlistCount(char **ppCollist){
  char *pEnd = *ppCollist;
  char c = 0;
  int nEntry = 0;

  /* A column-list is terminated by either a 0x01 or 0x00. */
  while( 0xFE & (*pEnd | c) ){
    c = *pEnd++ & 0x80;
    if( !c ) nEntry++;
  }

  *ppCollist = pEnd;
  return nEntry;
}

/*
** This function gathers 'y' or 'b' data for a single phrase.
*/
static void fts3ExprLHits(
  Fts3Expr *pExpr,                /* Phrase expression node */
  MatchInfo *p                    /* Matchinfo context */
){
  Fts3Table *pTab = (Fts3Table *)p->pCursor->base.pVtab;
  int iStart;
  Fts3Phrase *pPhrase = pExpr->pPhrase;
  char *pIter = pPhrase->doclist.pList;
  int iCol = 0;

  assert( p->flag==FTS3_MATCHINFO_LHITS_BM || p->flag==FTS3_MATCHINFO_LHITS );
  if( p->flag==FTS3_MATCHINFO_LHITS ){
    iStart = pExpr->iPhrase * p->nCol;
  }else{
    iStart = pExpr->iPhrase * ((p->nCol + 31) / 32);
  }

  while( 1 ){
    int nHit = fts3ColumnlistCount(&pIter);
    if( (pPhrase->iColumn>=pTab->nColumn || pPhrase->iColumn==iCol) ){
      if( p->flag==FTS3_MATCHINFO_LHITS ){
        p->aMatchinfo[iStart + iCol] = (u32)nHit;
      }else if( nHit ){
        p->aMatchinfo[iStart + (iCol+1)/32] |= (1 << (iCol&0x1F));
      }
    }
    assert( *pIter==0x00 || *pIter==0x01 );
    if( *pIter!=0x01 ) break;
    pIter++;
    pIter += fts3GetVarint32(pIter, &iCol);
  }
}

/*
** Gather the results for matchinfo directives 'y' and 'b'.
*/
static void fts3ExprLHitGather(
  Fts3Expr *pExpr,
  MatchInfo *p
){
  assert( (pExpr->pLeft==0)==(pExpr->pRight==0) );
  if( pExpr->bEof==0 && pExpr->iDocid==p->pCursor->iPrevId ){
    if( pExpr->pLeft ){
      fts3ExprLHitGather(pExpr->pLeft, p);
      fts3ExprLHitGather(pExpr->pRight, p);
    }else{
      fts3ExprLHits(pExpr, p);
    }
  }
}

/*
** fts3ExprIterate() callback used to collect the "global" matchinfo stats
** for a single query. 
**
** fts3ExprIterate() callback to load the 'global' elements of a
** FTS3_MATCHINFO_HITS matchinfo array. The global stats are those elements 
** of the matchinfo array that are constant for all rows returned by the 
** current query.
**
** Argument pCtx is actually a pointer to a struct of type MatchInfo. This
** function populates Matchinfo.aMatchinfo[] as follows:
**
**   for(iCol=0; iCol<nCol; iCol++){
**     aMatchinfo[3*iPhrase*nCol + 3*iCol + 1] = X;
**     aMatchinfo[3*iPhrase*nCol + 3*iCol + 2] = Y;
**   }
**
** where X is the number of matches for phrase iPhrase is column iCol of all
** rows of the table. Y is the number of rows for which column iCol contains
** at least one instance of phrase iPhrase.
**
** If the phrase pExpr consists entirely of deferred tokens, then all X and
** Y values are set to nDoc, where nDoc is the number of documents in the 
** file system. This is done because the full-text index doclist is required
** to calculate these values properly, and the full-text index doclist is
** not available for deferred tokens.
*/
static int fts3ExprGlobalHitsCb(
  Fts3Expr *pExpr,                /* Phrase expression node */
  int iPhrase,                    /* Phrase number (numbered from zero) */
  void *pCtx                      /* Pointer to MatchInfo structure */
){
  MatchInfo *p = (MatchInfo *)pCtx;
  return sqlite3Fts3EvalPhraseStats(
      p->pCursor, pExpr, &p->aMatchinfo[3*iPhrase*p->nCol]
  );
}

/*
** fts3ExprIterate() callback used to collect the "local" part of the
** FTS3_MATCHINFO_HITS array. The local stats are those elements of the 
** array that are different for each row returned by the query.
*/
static int fts3ExprLocalHitsCb(
  Fts3Expr *pExpr,                /* Phrase expression node */
  int iPhrase,                    /* Phrase number */
  void *pCtx                      /* Pointer to MatchInfo structure */
){
  int rc = SQLITE_OK;
  MatchInfo *p = (MatchInfo *)pCtx;
  int iStart = iPhrase * p->nCol * 3;
  int i;

  for(i=0; i<p->nCol && rc==SQLITE_OK; i++){
    char *pCsr;
    rc = sqlite3Fts3EvalPhrasePoslist(p->pCursor, pExpr, i, &pCsr);
    if( pCsr ){
      p->aMatchinfo[iStart+i*3] = fts3ColumnlistCount(&pCsr);
    }else{
      p->aMatchinfo[iStart+i*3] = 0;
    }
  }

  return rc;
}

static int fts3MatchinfoCheck(
  Fts3Table *pTab, 
  char cArg,
  char **pzErr
){
  if( (cArg==FTS3_MATCHINFO_NPHRASE)
   || (cArg==FTS3_MATCHINFO_NCOL)
   || (cArg==FTS3_MATCHINFO_NDOC && pTab->bFts4)
   || (cArg==FTS3_MATCHINFO_AVGLENGTH && pTab->bFts4)
   || (cArg==FTS3_MATCHINFO_LENGTH && pTab->bHasDocsize)
   || (cArg==FTS3_MATCHINFO_LCS)
   || (cArg==FTS3_MATCHINFO_HITS)
   || (cArg==FTS3_MATCHINFO_LHITS)
   || (cArg==FTS3_MATCHINFO_LHITS_BM)
  ){
    return SQLITE_OK;
  }
  sqlite3Fts3ErrMsg(pzErr, "unrecognized matchinfo request: %c", cArg);
  return SQLITE_ERROR;
}

static int fts3MatchinfoSize(MatchInfo *pInfo, char cArg){
  int nVal;                       /* Number of integers output by cArg */

  switch( cArg ){
    case FTS3_MATCHINFO_NDOC:
    case FTS3_MATCHINFO_NPHRASE: 
    case FTS3_MATCHINFO_NCOL: 
      nVal = 1;
      break;

    case FTS3_MATCHINFO_AVGLENGTH:
    case FTS3_MATCHINFO_LENGTH:
    case FTS3_MATCHINFO_LCS:
      nVal = pInfo->nCol;
      break;

    case FTS3_MATCHINFO_LHITS:
      nVal = pInfo->nCol * pInfo->nPhrase;
      break;

    case FTS3_MATCHINFO_LHITS_BM:
      nVal = pInfo->nPhrase * ((pInfo->nCol + 31) / 32);
      break;

    default:
      assert( cArg==FTS3_MATCHINFO_HITS );
      nVal = pInfo->nCol * pInfo->nPhrase * 3;
      break;
  }

  return nVal;
}

static int fts3MatchinfoSelectDoctotal(
  Fts3Table *pTab,
  sqlite3_stmt **ppStmt,
  sqlite3_int64 *pnDoc,
  const char **paLen
){
  sqlite3_stmt *pStmt;
  const char *a;
  sqlite3_int64 nDoc;

  if( !*ppStmt ){
    int rc = sqlite3Fts3SelectDoctotal(pTab, ppStmt);
    if( rc!=SQLITE_OK ) return rc;
  }
  pStmt = *ppStmt;
  assert( sqlite3_data_count(pStmt)==1 );

  a = sqlite3_column_blob(pStmt, 0);
  a += sqlite3Fts3GetVarint(a, &nDoc);
  if( nDoc==0 ) return FTS_CORRUPT_VTAB;
  *pnDoc = (u32)nDoc;

  if( paLen ) *paLen = a;
  return SQLITE_OK;
}

/*
** An instance of the following structure is used to store state while 
** iterating through a multi-column position-list corresponding to the
** hits for a single phrase on a single row in order to calculate the
** values for a matchinfo() FTS3_MATCHINFO_LCS request.
*/
typedef struct LcsIterator LcsIterator;
struct LcsIterator {
  Fts3Expr *pExpr;                /* Pointer to phrase expression */
  int iPosOffset;                 /* Tokens count up to end of this phrase */
  char *pRead;                    /* Cursor used to iterate through aDoclist */
  int iPos;                       /* Current position */
};

/* 
** If LcsIterator.iCol is set to the following value, the iterator has
** finished iterating through all offsets for all columns.
*/
#define LCS_ITERATOR_FINISHED 0x7FFFFFFF;

static int fts3MatchinfoLcsCb(
  Fts3Expr *pExpr,                /* Phrase expression node */
  int iPhrase,                    /* Phrase number (numbered from zero) */
  void *pCtx                      /* Pointer to MatchInfo structure */
){
  LcsIterator *aIter = (LcsIterator *)pCtx;
  aIter[iPhrase].pExpr = pExpr;
  return SQLITE_OK;
}

/*
** Advance the iterator passed as an argument to the next position. Return
** 1 if the iterator is at EOF or if it now points to the start of the
** position list for the next column.
*/
static int fts3LcsIteratorAdvance(LcsIterator *pIter){
  char *pRead = pIter->pRead;
  sqlite3_int64 iRead;
  int rc = 0;

  pRead += sqlite3Fts3GetVarint(pRead, &iRead);
  if( iRead==0 || iRead==1 ){
    pRead = 0;
    rc = 1;
  }else{
    pIter->iPos += (int)(iRead-2);
  }

  pIter->pRead = pRead;
  return rc;
}
  
/*
** This function implements the FTS3_MATCHINFO_LCS matchinfo() flag. 
**
** If the call is successful, the longest-common-substring lengths for each
** column are written into the first nCol elements of the pInfo->aMatchinfo[] 
** array before returning. SQLITE_OK is returned in this case.
**
** Otherwise, if an error occurs, an SQLite error code is returned and the
** data written to the first nCol elements of pInfo->aMatchinfo[] is 
** undefined.
*/
static int fts3MatchinfoLcs(Fts3Cursor *pCsr, MatchInfo *pInfo){
  LcsIterator *aIter;
  int i;
  int iCol;
  int nToken = 0;

  /* Allocate and populate the array of LcsIterator objects. The array
  ** contains one element for each matchable phrase in the query.
  **/
  aIter = sqlite3_malloc(sizeof(LcsIterator) * pCsr->nPhrase);
  if( !aIter ) return SQLITE_NOMEM;
  memset(aIter, 0, sizeof(LcsIterator) * pCsr->nPhrase);
  (void)fts3ExprIterate(pCsr->pExpr, fts3MatchinfoLcsCb, (void*)aIter);

  for(i=0; i<pInfo->nPhrase; i++){
    LcsIterator *pIter = &aIter[i];
    nToken -= pIter->pExpr->pPhrase->nToken;
    pIter->iPosOffset = nToken;
  }

  for(iCol=0; iCol<pInfo->nCol; iCol++){
    int nLcs = 0;                 /* LCS value for this column */
    int nLive = 0;                /* Number of iterators in aIter not at EOF */

    for(i=0; i<pInfo->nPhrase; i++){
      int rc;
      LcsIterator *pIt = &aIter[i];
      rc = sqlite3Fts3EvalPhrasePoslist(pCsr, pIt->pExpr, iCol, &pIt->pRead);
      if( rc!=SQLITE_OK ) return rc;
      if( pIt->pRead ){
        pIt->iPos = pIt->iPosOffset;
        fts3LcsIteratorAdvance(&aIter[i]);
        nLive++;
      }
    }

    while( nLive>0 ){
      LcsIterator *pAdv = 0;      /* The iterator to advance by one position */
      int nThisLcs = 0;           /* LCS for the current iterator positions */

      for(i=0; i<pInfo->nPhrase; i++){
        LcsIterator *pIter = &aIter[i];
        if( pIter->pRead==0 ){
          /* This iterator is already at EOF for this column. */
          nThisLcs = 0;
        }else{
          if( pAdv==0 || pIter->iPos<pAdv->iPos ){
            pAdv = pIter;
          }
          if( nThisLcs==0 || pIter->iPos==pIter[-1].iPos ){
            nThisLcs++;
          }else{
            nThisLcs = 1;
          }
          if( nThisLcs>nLcs ) nLcs = nThisLcs;
        }
      }
      if( fts3LcsIteratorAdvance(pAdv) ) nLive--;
    }

    pInfo->aMatchinfo[iCol] = nLcs;
  }

  sqlite3_free(aIter);
  return SQLITE_OK;
}

/*
** Populate the buffer pInfo->aMatchinfo[] with an array of integers to
** be returned by the matchinfo() function. Argument zArg contains the 
** format string passed as the second argument to matchinfo (or the
** default value "pcx" if no second argument was specified). The format
** string has already been validated and the pInfo->aMatchinfo[] array
** is guaranteed to be large enough for the output.
**
** If bGlobal is true, then populate all fields of the matchinfo() output.
** If it is false, then assume that those fields that do not change between
** rows (i.e. FTS3_MATCHINFO_NPHRASE, NCOL, NDOC, AVGLENGTH and part of HITS)
** have already been populated.
**
** Return SQLITE_OK if successful, or an SQLite error code if an error 
** occurs. If a value other than SQLITE_OK is returned, the state the
** pInfo->aMatchinfo[] buffer is left in is undefined.
*/
static int fts3MatchinfoValues(
  Fts3Cursor *pCsr,               /* FTS3 cursor object */
  int bGlobal,                    /* True to grab the global stats */
  MatchInfo *pInfo,               /* Matchinfo context object */
  const char *zArg                /* Matchinfo format string */
){
  int rc = SQLITE_OK;
  int i;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  sqlite3_stmt *pSelect = 0;

  for(i=0; rc==SQLITE_OK && zArg[i]; i++){
    pInfo->flag = zArg[i];
    switch( zArg[i] ){
      case FTS3_MATCHINFO_NPHRASE:
        if( bGlobal ) pInfo->aMatchinfo[0] = pInfo->nPhrase;
        break;

      case FTS3_MATCHINFO_NCOL:
        if( bGlobal ) pInfo->aMatchinfo[0] = pInfo->nCol;
        break;
        
      case FTS3_MATCHINFO_NDOC:
        if( bGlobal ){
          sqlite3_int64 nDoc = 0;
          rc = fts3MatchinfoSelectDoctotal(pTab, &pSelect, &nDoc, 0);
          pInfo->aMatchinfo[0] = (u32)nDoc;
        }
        break;

      case FTS3_MATCHINFO_AVGLENGTH: 
        if( bGlobal ){
          sqlite3_int64 nDoc;     /* Number of rows in table */
          const char *a;          /* Aggregate column length array */

          rc = fts3MatchinfoSelectDoctotal(pTab, &pSelect, &nDoc, &a);
          if( rc==SQLITE_OK ){
            int iCol;
            for(iCol=0; iCol<pInfo->nCol; iCol++){
              u32 iVal;
              sqlite3_int64 nToken;
              a += sqlite3Fts3GetVarint(a, &nToken);
              iVal = (u32)(((u32)(nToken&0xffffffff)+nDoc/2)/nDoc);
              pInfo->aMatchinfo[iCol] = iVal;
            }
          }
        }
        break;

      case FTS3_MATCHINFO_LENGTH: {
        sqlite3_stmt *pSelectDocsize = 0;
        rc = sqlite3Fts3SelectDocsize(pTab, pCsr->iPrevId, &pSelectDocsize);
        if( rc==SQLITE_OK ){
          int iCol;
          const char *a = sqlite3_column_blob(pSelectDocsize, 0);
          for(iCol=0; iCol<pInfo->nCol; iCol++){
            sqlite3_int64 nToken;
            a += sqlite3Fts3GetVarint(a, &nToken);
            pInfo->aMatchinfo[iCol] = (u32)nToken;
          }
        }
        sqlite3_reset(pSelectDocsize);
        break;
      }

      case FTS3_MATCHINFO_LCS:
        rc = fts3ExprLoadDoclists(pCsr, 0, 0);
        if( rc==SQLITE_OK ){
          rc = fts3MatchinfoLcs(pCsr, pInfo);
        }
        break;

      case FTS3_MATCHINFO_LHITS_BM:
      case FTS3_MATCHINFO_LHITS: {
        int nZero = fts3MatchinfoSize(pInfo, zArg[i]) * sizeof(u32);
        memset(pInfo->aMatchinfo, 0, nZero);
        fts3ExprLHitGather(pCsr->pExpr, pInfo);
        break;
      }

      default: {
        Fts3Expr *pExpr;
        assert( zArg[i]==FTS3_MATCHINFO_HITS );
        pExpr = pCsr->pExpr;
        rc = fts3ExprLoadDoclists(pCsr, 0, 0);
        if( rc!=SQLITE_OK ) break;
        if( bGlobal ){
          if( pCsr->pDeferred ){
            rc = fts3MatchinfoSelectDoctotal(pTab, &pSelect, &pInfo->nDoc, 0);
            if( rc!=SQLITE_OK ) break;
          }
          rc = fts3ExprIterate(pExpr, fts3ExprGlobalHitsCb,(void*)pInfo);
          sqlite3Fts3EvalTestDeferred(pCsr, &rc);
          if( rc!=SQLITE_OK ) break;
        }
        (void)fts3ExprIterate(pExpr, fts3ExprLocalHitsCb,(void*)pInfo);
        break;
      }
    }

    pInfo->aMatchinfo += fts3MatchinfoSize(pInfo, zArg[i]);
  }

  sqlite3_reset(pSelect);
  return rc;
}


/*
** Populate pCsr->aMatchinfo[] with data for the current row. The 
** 'matchinfo' data is an array of 32-bit unsigned integers (C type u32).
*/
static void fts3GetMatchinfo(
  sqlite3_context *pCtx,        /* Return results here */
  Fts3Cursor *pCsr,               /* FTS3 Cursor object */
  const char *zArg                /* Second argument to matchinfo() function */
){
  MatchInfo sInfo;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;
  int bGlobal = 0;                /* Collect 'global' stats as well as local */

  u32 *aOut = 0;
  void (*xDestroyOut)(void*) = 0;

  memset(&sInfo, 0, sizeof(MatchInfo));
  sInfo.pCursor = pCsr;
  sInfo.nCol = pTab->nColumn;

  /* If there is cached matchinfo() data, but the format string for the 
  ** cache does not match the format string for this request, discard 
  ** the cached data. */
  if( pCsr->pMIBuffer && strcmp(pCsr->pMIBuffer->zMatchinfo, zArg) ){
    sqlite3Fts3MIBufferFree(pCsr->pMIBuffer);
    pCsr->pMIBuffer = 0;
  }

  /* If Fts3Cursor.pMIBuffer is NULL, then this is the first time the
  ** matchinfo function has been called for this query. In this case 
  ** allocate the array used to accumulate the matchinfo data and
  ** initialize those elements that are constant for every row.
  */
  if( pCsr->pMIBuffer==0 ){
    int nMatchinfo = 0;           /* Number of u32 elements in match-info */
    int i;                        /* Used to iterate through zArg */

    /* Determine the number of phrases in the query */
    pCsr->nPhrase = fts3ExprPhraseCount(pCsr->pExpr);
    sInfo.nPhrase = pCsr->nPhrase;

    /* Determine the number of integers in the buffer returned by this call. */
    for(i=0; zArg[i]; i++){
      char *zErr = 0;
      if( fts3MatchinfoCheck(pTab, zArg[i], &zErr) ){
        sqlite3_result_error(pCtx, zErr, -1);
        sqlite3_free(zErr);
        return;
      }
      nMatchinfo += fts3MatchinfoSize(&sInfo, zArg[i]);
    }

    /* Allocate space for Fts3Cursor.aMatchinfo[] and Fts3Cursor.zMatchinfo. */
    pCsr->pMIBuffer = fts3MIBufferNew(nMatchinfo, zArg);
    if( !pCsr->pMIBuffer ) rc = SQLITE_NOMEM;

    pCsr->isMatchinfoNeeded = 1;
    bGlobal = 1;
  }

  if( rc==SQLITE_OK ){
    xDestroyOut = fts3MIBufferAlloc(pCsr->pMIBuffer, &aOut);
    if( xDestroyOut==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  if( rc==SQLITE_OK ){
    sInfo.aMatchinfo = aOut;
    sInfo.nPhrase = pCsr->nPhrase;
    rc = fts3MatchinfoValues(pCsr, bGlobal, &sInfo, zArg);
    if( bGlobal ){
      fts3MIBufferSetGlobal(pCsr->pMIBuffer);
    }
  }

  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx, rc);
    if( xDestroyOut ) xDestroyOut(aOut);
  }else{
    int n = pCsr->pMIBuffer->nElem * sizeof(u32);
    sqlite3_result_blob(pCtx, aOut, n, xDestroyOut);
  }
}

/*
** Implementation of snippet() function.
*/
SQLITE_PRIVATE void sqlite3Fts3Snippet(
  sqlite3_context *pCtx,          /* SQLite function call context */
  Fts3Cursor *pCsr,               /* Cursor object */
  const char *zStart,             /* Snippet start text - "<b>" */
  const char *zEnd,               /* Snippet end text - "</b>" */
  const char *zEllipsis,          /* Snippet ellipsis text - "<b>...</b>" */
  int iCol,                       /* Extract snippet from this column */
  int nToken                      /* Approximate number of tokens in snippet */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;
  int i;
  StrBuffer res = {0, 0, 0};

  /* The returned text includes up to four fragments of text extracted from
  ** the data in the current row. The first iteration of the for(...) loop
  ** below attempts to locate a single fragment of text nToken tokens in 
  ** size that contains at least one instance of all phrases in the query
  ** expression that appear in the current row. If such a fragment of text
  ** cannot be found, the second iteration of the loop attempts to locate
  ** a pair of fragments, and so on.
  */
  int nSnippet = 0;               /* Number of fragments in this snippet */
  SnippetFragment aSnippet[4];    /* Maximum of 4 fragments per snippet */
  int nFToken = -1;               /* Number of tokens in each fragment */

  if( !pCsr->pExpr ){
    sqlite3_result_text(pCtx, "", 0, SQLITE_STATIC);
    return;
  }

  for(nSnippet=1; 1; nSnippet++){

    int iSnip;                    /* Loop counter 0..nSnippet-1 */
    u64 mCovered = 0;             /* Bitmask of phrases covered by snippet */
    u64 mSeen = 0;                /* Bitmask of phrases seen by BestSnippet() */

    if( nToken>=0 ){
      nFToken = (nToken+nSnippet-1) / nSnippet;
    }else{
      nFToken = -1 * nToken;
    }

    for(iSnip=0; iSnip<nSnippet; iSnip++){
      int iBestScore = -1;        /* Best score of columns checked so far */
      int iRead;                  /* Used to iterate through columns */
      SnippetFragment *pFragment = &aSnippet[iSnip];

      memset(pFragment, 0, sizeof(*pFragment));

      /* Loop through all columns of the table being considered for snippets.
      ** If the iCol argument to this function was negative, this means all
      ** columns of the FTS3 table. Otherwise, only column iCol is considered.
      */
      for(iRead=0; iRead<pTab->nColumn; iRead++){
        SnippetFragment sF = {0, 0, 0, 0};
        int iS = 0;
        if( iCol>=0 && iRead!=iCol ) continue;

        /* Find the best snippet of nFToken tokens in column iRead. */
        rc = fts3BestSnippet(nFToken, pCsr, iRead, mCovered, &mSeen, &sF, &iS);
        if( rc!=SQLITE_OK ){
          goto snippet_out;
        }
        if( iS>iBestScore ){
          *pFragment = sF;
          iBestScore = iS;
        }
      }

      mCovered |= pFragment->covered;
    }

    /* If all query phrases seen by fts3BestSnippet() are present in at least
    ** one of the nSnippet snippet fragments, break out of the loop.
    */
    assert( (mCovered&mSeen)==mCovered );
    if( mSeen==mCovered || nSnippet==SizeofArray(aSnippet) ) break;
  }

  assert( nFToken>0 );

  for(i=0; i<nSnippet && rc==SQLITE_OK; i++){
    rc = fts3SnippetText(pCsr, &aSnippet[i], 
        i, (i==nSnippet-1), nFToken, zStart, zEnd, zEllipsis, &res
    );
  }

 snippet_out:
  sqlite3Fts3SegmentsClose(pTab);
  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx, rc);
    sqlite3_free(res.z);
  }else{
    sqlite3_result_text(pCtx, res.z, -1, sqlite3_free);
  }
}


typedef struct TermOffset TermOffset;
typedef struct TermOffsetCtx TermOffsetCtx;

struct TermOffset {
  char *pList;                    /* Position-list */
  int iPos;                       /* Position just read from pList */
  int iOff;                       /* Offset of this term from read positions */
};

struct TermOffsetCtx {
  Fts3Cursor *pCsr;
  int iCol;                       /* Column of table to populate aTerm for */
  int iTerm;
  sqlite3_int64 iDocid;
  TermOffset *aTerm;
};

/*
** This function is an fts3ExprIterate() callback used by sqlite3Fts3Offsets().
*/
static int fts3ExprTermOffsetInit(Fts3Expr *pExpr, int iPhrase, void *ctx){
  TermOffsetCtx *p = (TermOffsetCtx *)ctx;
  int nTerm;                      /* Number of tokens in phrase */
  int iTerm;                      /* For looping through nTerm phrase terms */
  char *pList;                    /* Pointer to position list for phrase */
  int iPos = 0;                   /* First position in position-list */
  int rc;

  UNUSED_PARAMETER(iPhrase);
  rc = sqlite3Fts3EvalPhrasePoslist(p->pCsr, pExpr, p->iCol, &pList);
  nTerm = pExpr->pPhrase->nToken;
  if( pList ){
    fts3GetDeltaPosition(&pList, &iPos);
    assert( iPos>=0 );
  }

  for(iTerm=0; iTerm<nTerm; iTerm++){
    TermOffset *pT = &p->aTerm[p->iTerm++];
    pT->iOff = nTerm-iTerm-1;
    pT->pList = pList;
    pT->iPos = iPos;
  }

  return rc;
}

/*
** Implementation of offsets() function.
*/
SQLITE_PRIVATE void sqlite3Fts3Offsets(
  sqlite3_context *pCtx,          /* SQLite function call context */
  Fts3Cursor *pCsr                /* Cursor object */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  sqlite3_tokenizer_module const *pMod = pTab->pTokenizer->pModule;
  int rc;                         /* Return Code */
  int nToken;                     /* Number of tokens in query */
  int iCol;                       /* Column currently being processed */
  StrBuffer res = {0, 0, 0};      /* Result string */
  TermOffsetCtx sCtx;             /* Context for fts3ExprTermOffsetInit() */

  if( !pCsr->pExpr ){
    sqlite3_result_text(pCtx, "", 0, SQLITE_STATIC);
    return;
  }

  memset(&sCtx, 0, sizeof(sCtx));
  assert( pCsr->isRequireSeek==0 );

  /* Count the number of terms in the query */
  rc = fts3ExprLoadDoclists(pCsr, 0, &nToken);
  if( rc!=SQLITE_OK ) goto offsets_out;

  /* Allocate the array of TermOffset iterators. */
  sCtx.aTerm = (TermOffset *)sqlite3_malloc(sizeof(TermOffset)*nToken);
  if( 0==sCtx.aTerm ){
    rc = SQLITE_NOMEM;
    goto offsets_out;
  }
  sCtx.iDocid = pCsr->iPrevId;
  sCtx.pCsr = pCsr;

  /* Loop through the table columns, appending offset information to 
  ** string-buffer res for each column.
  */
  for(iCol=0; iCol<pTab->nColumn; iCol++){
    sqlite3_tokenizer_cursor *pC; /* Tokenizer cursor */
    const char *ZDUMMY;           /* Dummy argument used with xNext() */
    int NDUMMY = 0;               /* Dummy argument used with xNext() */
    int iStart = 0;
    int iEnd = 0;
    int iCurrent = 0;
    const char *zDoc;
    int nDoc;

    /* Initialize the contents of sCtx.aTerm[] for column iCol. There is 
    ** no way that this operation can fail, so the return code from
    ** fts3ExprIterate() can be discarded.
    */
    sCtx.iCol = iCol;
    sCtx.iTerm = 0;
    (void)fts3ExprIterate(pCsr->pExpr, fts3ExprTermOffsetInit, (void*)&sCtx);

    /* Retreive the text stored in column iCol. If an SQL NULL is stored 
    ** in column iCol, jump immediately to the next iteration of the loop.
    ** If an OOM occurs while retrieving the data (this can happen if SQLite
    ** needs to transform the data from utf-16 to utf-8), return SQLITE_NOMEM 
    ** to the caller. 
    */
    zDoc = (const char *)sqlite3_column_text(pCsr->pStmt, iCol+1);
    nDoc = sqlite3_column_bytes(pCsr->pStmt, iCol+1);
    if( zDoc==0 ){
      if( sqlite3_column_type(pCsr->pStmt, iCol+1)==SQLITE_NULL ){
        continue;
      }
      rc = SQLITE_NOMEM;
      goto offsets_out;
    }

    /* Initialize a tokenizer iterator to iterate through column iCol. */
    rc = sqlite3Fts3OpenTokenizer(pTab->pTokenizer, pCsr->iLangid,
        zDoc, nDoc, &pC
    );
    if( rc!=SQLITE_OK ) goto offsets_out;

    rc = pMod->xNext(pC, &ZDUMMY, &NDUMMY, &iStart, &iEnd, &iCurrent);
    while( rc==SQLITE_OK ){
      int i;                      /* Used to loop through terms */
      int iMinPos = 0x7FFFFFFF;   /* Position of next token */
      TermOffset *pTerm = 0;      /* TermOffset associated with next token */

      for(i=0; i<nToken; i++){
        TermOffset *pT = &sCtx.aTerm[i];
        if( pT->pList && (pT->iPos-pT->iOff)<iMinPos ){
          iMinPos = pT->iPos-pT->iOff;
          pTerm = pT;
        }
      }

      if( !pTerm ){
        /* All offsets for this column have been gathered. */
        rc = SQLITE_DONE;
      }else{
        assert( iCurrent<=iMinPos );
        if( 0==(0xFE&*pTerm->pList) ){
          pTerm->pList = 0;
        }else{
          fts3GetDeltaPosition(&pTerm->pList, &pTerm->iPos);
        }
        while( rc==SQLITE_OK && iCurrent<iMinPos ){
          rc = pMod->xNext(pC, &ZDUMMY, &NDUMMY, &iStart, &iEnd, &iCurrent);
        }
        if( rc==SQLITE_OK ){
          char aBuffer[64];
          sqlite3_snprintf(sizeof(aBuffer), aBuffer, 
              "%d %d %d %d ", iCol, pTerm-sCtx.aTerm, iStart, iEnd-iStart
          );
          rc = fts3StringAppend(&res, aBuffer, -1);
        }else if( rc==SQLITE_DONE && pTab->zContentTbl==0 ){
          rc = FTS_CORRUPT_VTAB;
        }
      }
    }
    if( rc==SQLITE_DONE ){
      rc = SQLITE_OK;
    }

    pMod->xClose(pC);
    if( rc!=SQLITE_OK ) goto offsets_out;
  }

 offsets_out:
  sqlite3_free(sCtx.aTerm);
  assert( rc!=SQLITE_DONE );
  sqlite3Fts3SegmentsClose(pTab);
  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx,  rc);
    sqlite3_free(res.z);
  }else{
    sqlite3_result_text(pCtx, res.z, res.n-1, sqlite3_free);
  }
  return;
}

/*
** Implementation of matchinfo() function.
*/
SQLITE_PRIVATE void sqlite3Fts3Matchinfo(
  sqlite3_context *pContext,      /* Function call context */
  Fts3Cursor *pCsr,               /* FTS3 table cursor */
  const char *zArg                /* Second arg to matchinfo() function */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  const char *zFormat;

  if( zArg ){
    zFormat = zArg;
  }else{
    zFormat = FTS3_MATCHINFO_DEFAULT;
  }

  if( !pCsr->pExpr ){
    sqlite3_result_blob(pContext, "", 0, SQLITE_STATIC);
    return;
  }else{
    /* Retrieve matchinfo() data. */
    fts3GetMatchinfo(pContext, pCsr, zFormat);
    sqlite3Fts3SegmentsClose(pTab);
  }
}

#endif

/************** End of fts3_snippet.c ****************************************/
/************** Begin file fts3_unicode.c ************************************/
/*
** 2012 May 24
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** Implementation of the "unicode" full-text-search tokenizer.
*/

#ifndef SQLITE_DISABLE_FTS3_UNICODE

/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <assert.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */

/* #include "fts3_tokenizer.h" */

/*
** The following two macros - READ_UTF8 and WRITE_UTF8 - have been copied
** from the sqlite3 source file utf.c. If this file is compiled as part
** of the amalgamation, they are not required.
*/
#ifndef SQLITE_AMALGAMATION

static const unsigned char sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    while( zIn!=zTerm && (*zIn & 0xc0)==0x80 ){            \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    if( c<0x80                                             \
        || (c&0xFFFFF800)==0xD800                          \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }        \
  }

#define WRITE_UTF8(zOut, c) {                          \
  if( c<0x00080 ){                                     \
    *zOut++ = (u8)(c&0xFF);                            \
  }                                                    \
  else if( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  else if( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }else{                                               \
    *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#endif /* ifndef SQLITE_AMALGAMATION */

typedef struct unicode_tokenizer unicode_tokenizer;
typedef struct unicode_cursor unicode_cursor;

struct unicode_tokenizer {
  sqlite3_tokenizer base;
  int bRemoveDiacritic;
  int nException;
  int *aiException;
};

struct unicode_cursor {
  sqlite3_tokenizer_cursor base;
  const unsigned char *aInput;    /* Input text being tokenized */
  int nInput;                     /* Size of aInput[] in bytes */
  int iOff;                       /* Current offset within aInput[] */
  int iToken;                     /* Index of next token to be returned */
  char *zToken;                   /* storage for current token */
  int nAlloc;                     /* space allocated at zToken */
};


/*
** Destroy a tokenizer allocated by unicodeCreate().
*/
static int unicodeDestroy(sqlite3_tokenizer *pTokenizer){
  if( pTokenizer ){
    unicode_tokenizer *p = (unicode_tokenizer *)pTokenizer;
    sqlite3_free(p->aiException);
    sqlite3_free(p);
  }
  return SQLITE_OK;
}

/*
** As part of a tokenchars= or separators= option, the CREATE VIRTUAL TABLE
** statement has specified that the tokenizer for this table shall consider
** all characters in string zIn/nIn to be separators (if bAlnum==0) or
** token characters (if bAlnum==1).
**
** For each codepoint in the zIn/nIn string, this function checks if the
** sqlite3FtsUnicodeIsalnum() function already returns the desired result.
** If so, no action is taken. Otherwise, the codepoint is added to the 
** unicode_tokenizer.aiException[] array. For the purposes of tokenization,
** the return value of sqlite3FtsUnicodeIsalnum() is inverted for all
** codepoints in the aiException[] array.
**
** If a standalone diacritic mark (one that sqlite3FtsUnicodeIsdiacritic()
** identifies as a diacritic) occurs in the zIn/nIn string it is ignored.
** It is not possible to change the behavior of the tokenizer with respect
** to these codepoints.
*/
static int unicodeAddExceptions(
  unicode_tokenizer *p,           /* Tokenizer to add exceptions to */
  int bAlnum,                     /* Replace Isalnum() return value with this */
  const char *zIn,                /* Array of characters to make exceptions */
  int nIn                         /* Length of z in bytes */
){
  const unsigned char *z = (const unsigned char *)zIn;
  const unsigned char *zTerm = &z[nIn];
  int iCode;
  int nEntry = 0;

  assert( bAlnum==0 || bAlnum==1 );

  while( z<zTerm ){
    READ_UTF8(z, zTerm, iCode);
    assert( (sqlite3FtsUnicodeIsalnum(iCode) & 0xFFFFFFFE)==0 );
    if( sqlite3FtsUnicodeIsalnum(iCode)!=bAlnum 
     && sqlite3FtsUnicodeIsdiacritic(iCode)==0 
    ){
      nEntry++;
    }
  }

  if( nEntry ){
    int *aNew;                    /* New aiException[] array */
    int nNew;                     /* Number of valid entries in array aNew[] */

    aNew = sqlite3_realloc(p->aiException, (p->nException+nEntry)*sizeof(int));
    if( aNew==0 ) return SQLITE_NOMEM;
    nNew = p->nException;

    z = (const unsigned char *)zIn;
    while( z<zTerm ){
      READ_UTF8(z, zTerm, iCode);
      if( sqlite3FtsUnicodeIsalnum(iCode)!=bAlnum 
       && sqlite3FtsUnicodeIsdiacritic(iCode)==0
      ){
        int i, j;
        for(i=0; i<nNew && aNew[i]<iCode; i++);
        for(j=nNew; j>i; j--) aNew[j] = aNew[j-1];
        aNew[i] = iCode;
        nNew++;
      }
    }
    p->aiException = aNew;
    p->nException = nNew;
  }

  return SQLITE_OK;
}

/*
** Return true if the p->aiException[] array contains the value iCode.
*/
static int unicodeIsException(unicode_tokenizer *p, int iCode){
  if( p->nException>0 ){
    int *a = p->aiException;
    int iLo = 0;
    int iHi = p->nException-1;

    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      if( iCode==a[iTest] ){
        return 1;
      }else if( iCode>a[iTest] ){
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
  }

  return 0;
}

/*
** Return true if, for the purposes of tokenization, codepoint iCode is
** considered a token character (not a separator).
*/
static int unicodeIsAlnum(unicode_tokenizer *p, int iCode){
  assert( (sqlite3FtsUnicodeIsalnum(iCode) & 0xFFFFFFFE)==0 );
  return sqlite3FtsUnicodeIsalnum(iCode) ^ unicodeIsException(p, iCode);
}

/*
** Create a new tokenizer instance.
*/
static int unicodeCreate(
  int nArg,                       /* Size of array argv[] */
  const char * const *azArg,      /* Tokenizer creation arguments */
  sqlite3_tokenizer **pp          /* OUT: New tokenizer handle */
){
  unicode_tokenizer *pNew;        /* New tokenizer object */
  int i;
  int rc = SQLITE_OK;

  pNew = (unicode_tokenizer *) sqlite3_malloc(sizeof(unicode_tokenizer));
  if( pNew==NULL ) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(unicode_tokenizer));
  pNew->bRemoveDiacritic = 1;

  for(i=0; rc==SQLITE_OK && i<nArg; i++){
    const char *z = azArg[i];
    int n = (int)strlen(z);

    if( n==19 && memcmp("remove_diacritics=1", z, 19)==0 ){
      pNew->bRemoveDiacritic = 1;
    }
    else if( n==19 && memcmp("remove_diacritics=0", z, 19)==0 ){
      pNew->bRemoveDiacritic = 0;
    }
    else if( n>=11 && memcmp("tokenchars=", z, 11)==0 ){
      rc = unicodeAddExceptions(pNew, 1, &z[11], n-11);
    }
    else if( n>=11 && memcmp("separators=", z, 11)==0 ){
      rc = unicodeAddExceptions(pNew, 0, &z[11], n-11);
    }
    else{
      /* Unrecognized argument */
      rc  = SQLITE_ERROR;
    }
  }

  if( rc!=SQLITE_OK ){
    unicodeDestroy((sqlite3_tokenizer *)pNew);
    pNew = 0;
  }
  *pp = (sqlite3_tokenizer *)pNew;
  return rc;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is pInput[0..nBytes-1].  A cursor
** used to incrementally tokenize this string is returned in 
** *ppCursor.
*/
static int unicodeOpen(
  sqlite3_tokenizer *p,           /* The tokenizer */
  const char *aInput,             /* Input string */
  int nInput,                     /* Size of string aInput in bytes */
  sqlite3_tokenizer_cursor **pp   /* OUT: New cursor object */
){
  unicode_cursor *pCsr;

  pCsr = (unicode_cursor *)sqlite3_malloc(sizeof(unicode_cursor));
  if( pCsr==0 ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(unicode_cursor));

  pCsr->aInput = (const unsigned char *)aInput;
  if( aInput==0 ){
    pCsr->nInput = 0;
  }else if( nInput<0 ){
    pCsr->nInput = (int)strlen(aInput);
  }else{
    pCsr->nInput = nInput;
  }

  *pp = &pCsr->base;
  UNUSED_PARAMETER(p);
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to
** simpleOpen() above.
*/
static int unicodeClose(sqlite3_tokenizer_cursor *pCursor){
  unicode_cursor *pCsr = (unicode_cursor *) pCursor;
  sqlite3_free(pCsr->zToken);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Extract the next token from a tokenization cursor.  The cursor must
** have been opened by a prior call to simpleOpen().
*/
static int unicodeNext(
  sqlite3_tokenizer_cursor *pC,   /* Cursor returned by simpleOpen */
  const char **paToken,           /* OUT: Token text */
  int *pnToken,                   /* OUT: Number of bytes at *paToken */
  int *piStart,                   /* OUT: Starting offset of token */
  int *piEnd,                     /* OUT: Ending offset of token */
  int *piPos                      /* OUT: Position integer of token */
){
  unicode_cursor *pCsr = (unicode_cursor *)pC;
  unicode_tokenizer *p = ((unicode_tokenizer *)pCsr->base.pTokenizer);
  int iCode = 0;
  char *zOut;
  const unsigned char *z = &pCsr->aInput[pCsr->iOff];
  const unsigned char *zStart = z;
  const unsigned char *zEnd;
  const unsigned char *zTerm = &pCsr->aInput[pCsr->nInput];

  /* Scan past any delimiter characters before the start of the next token.
  ** Return SQLITE_DONE early if this takes us all the way to the end of 
  ** the input.  */
  while( z<zTerm ){
    READ_UTF8(z, zTerm, iCode);
    if( unicodeIsAlnum(p, iCode) ) break;
    zStart = z;
  }
  if( zStart>=zTerm ) return SQLITE_DONE;

  zOut = pCsr->zToken;
  do {
    int iOut;

    /* Grow the output buffer if required. */
    if( (zOut-pCsr->zToken)>=(pCsr->nAlloc-4) ){
      char *zNew = sqlite3_realloc(pCsr->zToken, pCsr->nAlloc+64);
      if( !zNew ) return SQLITE_NOMEM;
      zOut = &zNew[zOut - pCsr->zToken];
      pCsr->zToken = zNew;
      pCsr->nAlloc += 64;
    }

    /* Write the folded case of the last character read to the output */
    zEnd = z;
    iOut = sqlite3FtsUnicodeFold(iCode, p->bRemoveDiacritic);
    if( iOut ){
      WRITE_UTF8(zOut, iOut);
    }

    /* If the cursor is not at EOF, read the next character */
    if( z>=zTerm ) break;
    READ_UTF8(z, zTerm, iCode);
  }while( unicodeIsAlnum(p, iCode) 
       || sqlite3FtsUnicodeIsdiacritic(iCode)
  );

  /* Set the output variables and return. */
  pCsr->iOff = (int)(z - pCsr->aInput);
  *paToken = pCsr->zToken;
  *pnToken = (int)(zOut - pCsr->zToken);
  *piStart = (int)(zStart - pCsr->aInput);
  *piEnd = (int)(zEnd - pCsr->aInput);
  *piPos = pCsr->iToken++;
  return SQLITE_OK;
}

/*
** Set *ppModule to a pointer to the sqlite3_tokenizer_module 
** structure for the unicode tokenizer.
*/
SQLITE_PRIVATE void sqlite3Fts3UnicodeTokenizer(sqlite3_tokenizer_module const **ppModule){
  static const sqlite3_tokenizer_module module = {
    0,
    unicodeCreate,
    unicodeDestroy,
    unicodeOpen,
    unicodeClose,
    unicodeNext,
    0,
  };
  *ppModule = &module;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */
#endif /* ifndef SQLITE_DISABLE_FTS3_UNICODE */

/************** End of fts3_unicode.c ****************************************/
/************** Begin file fts3_unicode2.c ***********************************/
/*
** 2012 May 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
*/

/*
** DO NOT EDIT THIS MACHINE GENERATED FILE.
*/

#ifndef SQLITE_DISABLE_FTS3_UNICODE
#if defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)

/* #include <assert.h> */

/*
** Return true if the argument corresponds to a unicode codepoint
** classified as either a letter or a number. Otherwise false.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeIsalnum(int c){
  /* Each unsigned integer in the following array corresponds to a contiguous
  ** range of unicode codepoints that are not either letters or numbers (i.e.
  ** codepoints for which this function should return 0).
  **
  ** The most significant 22 bits in each 32-bit value contain the first 
  ** codepoint in the range. The least significant 10 bits are used to store
  ** the size of the range (always at least 1). In other words, the value 
  ** ((C<<22) + N) represents a range of N codepoints starting with codepoint 
  ** C. It is not possible to represent a range larger than 1023 codepoints 
  ** using this format.
  */
  static const unsigned int aEntry[] = {
    0x00000030, 0x0000E807, 0x00016C06, 0x0001EC2F, 0x0002AC07,
    0x0002D001, 0x0002D803, 0x0002EC01, 0x0002FC01, 0x00035C01,
    0x0003DC01, 0x000B0804, 0x000B480E, 0x000B9407, 0x000BB401,
    0x000BBC81, 0x000DD401, 0x000DF801, 0x000E1002, 0x000E1C01,
    0x000FD801, 0x00120808, 0x00156806, 0x00162402, 0x00163C01,
    0x00164437, 0x0017CC02, 0x00180005, 0x00181816, 0x00187802,
    0x00192C15, 0x0019A804, 0x0019C001, 0x001B5001, 0x001B580F,
    0x001B9C07, 0x001BF402, 0x001C000E, 0x001C3C01, 0x001C4401,
    0x001CC01B, 0x001E980B, 0x001FAC09, 0x001FD804, 0x00205804,
    0x00206C09, 0x00209403, 0x0020A405, 0x0020C00F, 0x00216403,
    0x00217801, 0x0023901B, 0x00240004, 0x0024E803, 0x0024F812,
    0x00254407, 0x00258804, 0x0025C001, 0x00260403, 0x0026F001,
    0x0026F807, 0x00271C02, 0x00272C03, 0x00275C01, 0x00278802,
    0x0027C802, 0x0027E802, 0x00280403, 0x0028F001, 0x0028F805,
    0x00291C02, 0x00292C03, 0x00294401, 0x0029C002, 0x0029D401,
    0x002A0403, 0x002AF001, 0x002AF808, 0x002B1C03, 0x002B2C03,
    0x002B8802, 0x002BC002, 0x002C0403, 0x002CF001, 0x002CF807,
    0x002D1C02, 0x002D2C03, 0x002D5802, 0x002D8802, 0x002DC001,
    0x002E0801, 0x002EF805, 0x002F1803, 0x002F2804, 0x002F5C01,
    0x002FCC08, 0x00300403, 0x0030F807, 0x00311803, 0x00312804,
    0x00315402, 0x00318802, 0x0031FC01, 0x00320802, 0x0032F001,
    0x0032F807, 0x00331803, 0x00332804, 0x00335402, 0x00338802,
    0x00340802, 0x0034F807, 0x00351803, 0x00352804, 0x00355C01,
    0x00358802, 0x0035E401, 0x00360802, 0x00372801, 0x00373C06,
    0x00375801, 0x00376008, 0x0037C803, 0x0038C401, 0x0038D007,
    0x0038FC01, 0x00391C09, 0x00396802, 0x003AC401, 0x003AD006,
    0x003AEC02, 0x003B2006, 0x003C041F, 0x003CD00C, 0x003DC417,
    0x003E340B, 0x003E6424, 0x003EF80F, 0x003F380D, 0x0040AC14,
    0x00412806, 0x00415804, 0x00417803, 0x00418803, 0x00419C07,
    0x0041C404, 0x0042080C, 0x00423C01, 0x00426806, 0x0043EC01,
    0x004D740C, 0x004E400A, 0x00500001, 0x0059B402, 0x005A0001,
    0x005A6C02, 0x005BAC03, 0x005C4803, 0x005CC805, 0x005D4802,
    0x005DC802, 0x005ED023, 0x005F6004, 0x005F7401, 0x0060000F,
    0x0062A401, 0x0064800C, 0x0064C00C, 0x00650001, 0x00651002,
    0x0066C011, 0x00672002, 0x00677822, 0x00685C05, 0x00687802,
    0x0069540A, 0x0069801D, 0x0069FC01, 0x006A8007, 0x006AA006,
    0x006C0005, 0x006CD011, 0x006D6823, 0x006E0003, 0x006E840D,
    0x006F980E, 0x006FF004, 0x00709014, 0x0070EC05, 0x0071F802,
    0x00730008, 0x00734019, 0x0073B401, 0x0073C803, 0x00770027,
    0x0077F004, 0x007EF401, 0x007EFC03, 0x007F3403, 0x007F7403,
    0x007FB403, 0x007FF402, 0x00800065, 0x0081A806, 0x0081E805,
    0x00822805, 0x0082801A, 0x00834021, 0x00840002, 0x00840C04,
    0x00842002, 0x00845001, 0x00845803, 0x00847806, 0x00849401,
    0x00849C01, 0x0084A401, 0x0084B801, 0x0084E802, 0x00850005,
    0x00852804, 0x00853C01, 0x00864264, 0x00900027, 0x0091000B,
    0x0092704E, 0x00940200, 0x009C0475, 0x009E53B9, 0x00AD400A,
    0x00B39406, 0x00B3BC03, 0x00B3E404, 0x00B3F802, 0x00B5C001,
    0x00B5FC01, 0x00B7804F, 0x00B8C00C, 0x00BA001A, 0x00BA6C59,
    0x00BC00D6, 0x00BFC00C, 0x00C00005, 0x00C02019, 0x00C0A807,
    0x00C0D802, 0x00C0F403, 0x00C26404, 0x00C28001, 0x00C3EC01,
    0x00C64002, 0x00C6580A, 0x00C70024, 0x00C8001F, 0x00C8A81E,
    0x00C94001, 0x00C98020, 0x00CA2827, 0x00CB003F, 0x00CC0100,
    0x01370040, 0x02924037, 0x0293F802, 0x02983403, 0x0299BC10,
    0x029A7C01, 0x029BC008, 0x029C0017, 0x029C8002, 0x029E2402,
    0x02A00801, 0x02A01801, 0x02A02C01, 0x02A08C09, 0x02A0D804,
    0x02A1D004, 0x02A20002, 0x02A2D011, 0x02A33802, 0x02A38012,
    0x02A3E003, 0x02A4980A, 0x02A51C0D, 0x02A57C01, 0x02A60004,
    0x02A6CC1B, 0x02A77802, 0x02A8A40E, 0x02A90C01, 0x02A93002,
    0x02A97004, 0x02A9DC03, 0x02A9EC01, 0x02AAC001, 0x02AAC803,
    0x02AADC02, 0x02AAF802, 0x02AB0401, 0x02AB7802, 0x02ABAC07,
    0x02ABD402, 0x02AF8C0B, 0x03600001, 0x036DFC02, 0x036FFC02,
    0x037FFC01, 0x03EC7801, 0x03ECA401, 0x03EEC810, 0x03F4F802,
    0x03F7F002, 0x03F8001A, 0x03F88007, 0x03F8C023, 0x03F95013,
    0x03F9A004, 0x03FBFC01, 0x03FC040F, 0x03FC6807, 0x03FCEC06,
    0x03FD6C0B, 0x03FF8007, 0x03FFA007, 0x03FFE405, 0x04040003,
    0x0404DC09, 0x0405E411, 0x0406400C, 0x0407402E, 0x040E7C01,
    0x040F4001, 0x04215C01, 0x04247C01, 0x0424FC01, 0x04280403,
    0x04281402, 0x04283004, 0x0428E003, 0x0428FC01, 0x04294009,
    0x0429FC01, 0x042CE407, 0x04400003, 0x0440E016, 0x04420003,
    0x0442C012, 0x04440003, 0x04449C0E, 0x04450004, 0x04460003,
    0x0446CC0E, 0x04471404, 0x045AAC0D, 0x0491C004, 0x05BD442E,
    0x05BE3C04, 0x074000F6, 0x07440027, 0x0744A4B5, 0x07480046,
    0x074C0057, 0x075B0401, 0x075B6C01, 0x075BEC01, 0x075C5401,
    0x075CD401, 0x075D3C01, 0x075DBC01, 0x075E2401, 0x075EA401,
    0x075F0C01, 0x07BBC002, 0x07C0002C, 0x07C0C064, 0x07C2800F,
    0x07C2C40E, 0x07C3040F, 0x07C3440F, 0x07C4401F, 0x07C4C03C,
    0x07C5C02B, 0x07C7981D, 0x07C8402B, 0x07C90009, 0x07C94002,
    0x07CC0021, 0x07CCC006, 0x07CCDC46, 0x07CE0014, 0x07CE8025,
    0x07CF1805, 0x07CF8011, 0x07D0003F, 0x07D10001, 0x07D108B6,
    0x07D3E404, 0x07D4003E, 0x07D50004, 0x07D54018, 0x07D7EC46,
    0x07D9140B, 0x07DA0046, 0x07DC0074, 0x38000401, 0x38008060,
    0x380400F0,
  };
  static const unsigned int aAscii[4] = {
    0xFFFFFFFF, 0xFC00FFFF, 0xF8000001, 0xF8000001,
  };

  if( c<128 ){
    return ( (aAscii[c >> 5] & (1 << (c & 0x001F)))==0 );
  }else if( c<(1<<22) ){
    unsigned int key = (((unsigned int)c)<<10) | 0x000003FF;
    int iRes = 0;
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      if( key >= aEntry[iTest] ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
    assert( aEntry[0]<key );
    assert( key>=aEntry[iRes] );
    return (((unsigned int)c) >= ((aEntry[iRes]>>10) + (aEntry[iRes]&0x3FF)));
  }
  return 1;
}


/*
** If the argument is a codepoint corresponding to a lowercase letter
** in the ASCII range with a diacritic added, return the codepoint
** of the ASCII letter only. For example, if passed 235 - "LATIN
** SMALL LETTER E WITH DIAERESIS" - return 65 ("LATIN SMALL LETTER
** E"). The resuls of passing a codepoint that corresponds to an
** uppercase letter are undefined.
*/
static int remove_diacritic(int c){
  unsigned short aDia[] = {
        0,  1797,  1848,  1859,  1891,  1928,  1940,  1995, 
     2024,  2040,  2060,  2110,  2168,  2206,  2264,  2286, 
     2344,  2383,  2472,  2488,  2516,  2596,  2668,  2732, 
     2782,  2842,  2894,  2954,  2984,  3000,  3028,  3336, 
     3456,  3696,  3712,  3728,  3744,  3896,  3912,  3928, 
     3968,  4008,  4040,  4106,  4138,  4170,  4202,  4234, 
     4266,  4296,  4312,  4344,  4408,  4424,  4472,  4504, 
     6148,  6198,  6264,  6280,  6360,  6429,  6505,  6529, 
    61448, 61468, 61534, 61592, 61642, 61688, 61704, 61726, 
    61784, 61800, 61836, 61880, 61914, 61948, 61998, 62122, 
    62154, 62200, 62218, 62302, 62364, 62442, 62478, 62536, 
    62554, 62584, 62604, 62640, 62648, 62656, 62664, 62730, 
    62924, 63050, 63082, 63274, 63390, 
  };
  char aChar[] = {
    '\0', 'a',  'c',  'e',  'i',  'n',  'o',  'u',  'y',  'y',  'a',  'c',  
    'd',  'e',  'e',  'g',  'h',  'i',  'j',  'k',  'l',  'n',  'o',  'r',  
    's',  't',  'u',  'u',  'w',  'y',  'z',  'o',  'u',  'a',  'i',  'o',  
    'u',  'g',  'k',  'o',  'j',  'g',  'n',  'a',  'e',  'i',  'o',  'r',  
    'u',  's',  't',  'h',  'a',  'e',  'o',  'y',  '\0', '\0', '\0', '\0', 
    '\0', '\0', '\0', '\0', 'a',  'b',  'd',  'd',  'e',  'f',  'g',  'h',  
    'h',  'i',  'k',  'l',  'l',  'm',  'n',  'p',  'r',  'r',  's',  't',  
    'u',  'v',  'w',  'w',  'x',  'y',  'z',  'h',  't',  'w',  'y',  'a',  
    'e',  'i',  'o',  'u',  'y',  
  };

  unsigned int key = (((unsigned int)c)<<3) | 0x00000007;
  int iRes = 0;
  int iHi = sizeof(aDia)/sizeof(aDia[0]) - 1;
  int iLo = 0;
  while( iHi>=iLo ){
    int iTest = (iHi + iLo) / 2;
    if( key >= aDia[iTest] ){
      iRes = iTest;
      iLo = iTest+1;
    }else{
      iHi = iTest-1;
    }
  }
  assert( key>=aDia[iRes] );
  return ((c > (aDia[iRes]>>3) + (aDia[iRes]&0x07)) ? c : (int)aChar[iRes]);
}


/*
** Return true if the argument interpreted as a unicode codepoint
** is a diacritical modifier character.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeIsdiacritic(int c){
  unsigned int mask0 = 0x08029FDF;
  unsigned int mask1 = 0x000361F8;
  if( c<768 || c>817 ) return 0;
  return (c < 768+32) ?
      (mask0 & (1 << (c-768))) :
      (mask1 & (1 << (c-768-32)));
}


/*
** Interpret the argument as a unicode codepoint. If the codepoint
** is an upper case character that has a lower case equivalent,
** return the codepoint corresponding to the lower case version.
** Otherwise, return a copy of the argument.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeFold(int c, int bRemoveDiacritic){
  /* Each entry in the following array defines a rule for folding a range
  ** of codepoints to lower case. The rule applies to a range of nRange
  ** codepoints starting at codepoint iCode.
  **
  ** If the least significant bit in flags is clear, then the rule applies
  ** to all nRange codepoints (i.e. all nRange codepoints are upper case and
  ** need to be folded). Or, if it is set, then the rule only applies to
  ** every second codepoint in the range, starting with codepoint C.
  **
  ** The 7 most significant bits in flags are an index into the aiOff[]
  ** array. If a specific codepoint C does require folding, then its lower
  ** case equivalent is ((C + aiOff[flags>>1]) & 0xFFFF).
  **
  ** The contents of this array are generated by parsing the CaseFolding.txt
  ** file distributed as part of the "Unicode Character Database". See
  ** http://www.unicode.org for details.
  */
  static const struct TableEntry {
    unsigned short iCode;
    unsigned char flags;
    unsigned char nRange;
  } aEntry[] = {
    {65, 14, 26},          {181, 64, 1},          {192, 14, 23},
    {216, 14, 7},          {256, 1, 48},          {306, 1, 6},
    {313, 1, 16},          {330, 1, 46},          {376, 116, 1},
    {377, 1, 6},           {383, 104, 1},         {385, 50, 1},
    {386, 1, 4},           {390, 44, 1},          {391, 0, 1},
    {393, 42, 2},          {395, 0, 1},           {398, 32, 1},
    {399, 38, 1},          {400, 40, 1},          {401, 0, 1},
    {403, 42, 1},          {404, 46, 1},          {406, 52, 1},
    {407, 48, 1},          {408, 0, 1},           {412, 52, 1},
    {413, 54, 1},          {415, 56, 1},          {416, 1, 6},
    {422, 60, 1},          {423, 0, 1},           {425, 60, 1},
    {428, 0, 1},           {430, 60, 1},          {431, 0, 1},
    {433, 58, 2},          {435, 1, 4},           {439, 62, 1},
    {440, 0, 1},           {444, 0, 1},           {452, 2, 1},
    {453, 0, 1},           {455, 2, 1},           {456, 0, 1},
    {458, 2, 1},           {459, 1, 18},          {478, 1, 18},
    {497, 2, 1},           {498, 1, 4},           {502, 122, 1},
    {503, 134, 1},         {504, 1, 40},          {544, 110, 1},
    {546, 1, 18},          {570, 70, 1},          {571, 0, 1},
    {573, 108, 1},         {574, 68, 1},          {577, 0, 1},
    {579, 106, 1},         {580, 28, 1},          {581, 30, 1},
    {582, 1, 10},          {837, 36, 1},          {880, 1, 4},
    {886, 0, 1},           {902, 18, 1},          {904, 16, 3},
    {908, 26, 1},          {910, 24, 2},          {913, 14, 17},
    {931, 14, 9},          {962, 0, 1},           {975, 4, 1},
    {976, 140, 1},         {977, 142, 1},         {981, 146, 1},
    {982, 144, 1},         {984, 1, 24},          {1008, 136, 1},
    {1009, 138, 1},        {1012, 130, 1},        {1013, 128, 1},
    {1015, 0, 1},          {1017, 152, 1},        {1018, 0, 1},
    {1021, 110, 3},        {1024, 34, 16},        {1040, 14, 32},
    {1120, 1, 34},         {1162, 1, 54},         {1216, 6, 1},
    {1217, 1, 14},         {1232, 1, 88},         {1329, 22, 38},
    {4256, 66, 38},        {4295, 66, 1},         {4301, 66, 1},
    {7680, 1, 150},        {7835, 132, 1},        {7838, 96, 1},
    {7840, 1, 96},         {7944, 150, 8},        {7960, 150, 6},
    {7976, 150, 8},        {7992, 150, 8},        {8008, 150, 6},
    {8025, 151, 8},        {8040, 150, 8},        {8072, 150, 8},
    {8088, 150, 8},        {8104, 150, 8},        {8120, 150, 2},
    {8122, 126, 2},        {8124, 148, 1},        {8126, 100, 1},
    {8136, 124, 4},        {8140, 148, 1},        {8152, 150, 2},
    {8154, 120, 2},        {8168, 150, 2},        {8170, 118, 2},
    {8172, 152, 1},        {8184, 112, 2},        {8186, 114, 2},
    {8188, 148, 1},        {8486, 98, 1},         {8490, 92, 1},
    {8491, 94, 1},         {8498, 12, 1},         {8544, 8, 16},
    {8579, 0, 1},          {9398, 10, 26},        {11264, 22, 47},
    {11360, 0, 1},         {11362, 88, 1},        {11363, 102, 1},
    {11364, 90, 1},        {11367, 1, 6},         {11373, 84, 1},
    {11374, 86, 1},        {11375, 80, 1},        {11376, 82, 1},
    {11378, 0, 1},         {11381, 0, 1},         {11390, 78, 2},
    {11392, 1, 100},       {11499, 1, 4},         {11506, 0, 1},
    {42560, 1, 46},        {42624, 1, 24},        {42786, 1, 14},
    {42802, 1, 62},        {42873, 1, 4},         {42877, 76, 1},
    {42878, 1, 10},        {42891, 0, 1},         {42893, 74, 1},
    {42896, 1, 4},         {42912, 1, 10},        {42922, 72, 1},
    {65313, 14, 26},       
  };
  static const unsigned short aiOff[] = {
   1,     2,     8,     15,    16,    26,    28,    32,    
   37,    38,    40,    48,    63,    64,    69,    71,    
   79,    80,    116,   202,   203,   205,   206,   207,   
   209,   210,   211,   213,   214,   217,   218,   219,   
   775,   7264,  10792, 10795, 23228, 23256, 30204, 54721, 
   54753, 54754, 54756, 54787, 54793, 54809, 57153, 57274, 
   57921, 58019, 58363, 61722, 65268, 65341, 65373, 65406, 
   65408, 65410, 65415, 65424, 65436, 65439, 65450, 65462, 
   65472, 65476, 65478, 65480, 65482, 65488, 65506, 65511, 
   65514, 65521, 65527, 65528, 65529, 
  };

  int ret = c;

  assert( c>=0 );
  assert( sizeof(unsigned short)==2 && sizeof(unsigned char)==1 );

  if( c<128 ){
    if( c>='A' && c<='Z' ) ret = c + ('a' - 'A');
  }else if( c<65536 ){
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    int iRes = -1;

    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      int cmp = (c - aEntry[iTest].iCode);
      if( cmp>=0 ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
    assert( iRes<0 || c>=aEntry[iRes].iCode );

    if( iRes>=0 ){
      const struct TableEntry *p = &aEntry[iRes];
      if( c<(p->iCode + p->nRange) && 0==(0x01 & p->flags & (p->iCode ^ c)) ){
        ret = (c + (aiOff[p->flags>>1])) & 0x0000FFFF;
        assert( ret>0 );
      }
    }

    if( bRemoveDiacritic ) ret = remove_diacritic(ret);
  }
  
  else if( c>=66560 && c<66600 ){
    ret = c + 40;
  }

  return ret;
}
#endif /* defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4) */
#endif /* !defined(SQLITE_DISABLE_FTS3_UNICODE) */

/************** End of fts3_unicode2.c ***************************************/
/************** Begin file rtree.c *******************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code for implementations of the r-tree and r*-tree
** algorithms packaged as an SQLite virtual table module.
*/

/*
** Database Format of R-Tree Tables
** --------------------------------
**
** The data structure for a single virtual r-tree table is stored in three 
** native SQLite tables declared as follows. In each case, the '%' character
** in the table name is replaced with the user-supplied name of the r-tree
** table.
**
**   CREATE TABLE %_node(nodeno INTEGER PRIMARY KEY, data BLOB)
**   CREATE TABLE %_parent(nodeno INTEGER PRIMARY KEY, parentnode INTEGER)
**   CREATE TABLE %_rowid(rowid INTEGER PRIMARY KEY, nodeno INTEGER)
**
** The data for each node of the r-tree structure is stored in the %_node
** table. For each node that is not the root node of the r-tree, there is
** an entry in the %_parent table associating the node with its parent.
** And for each row of data in the table, there is an entry in the %_rowid
** table that maps from the entries rowid to the id of the node that it
** is stored on.
**
** The root node of an r-tree always exists, even if the r-tree table is
** empty. The nodeno of the root node is always 1. All other nodes in the
** table must be the same size as the root node. The content of each node
** is formatted as follows:
**
**   1. If the node is the root node (node 1), then the first 2 bytes
**      of the node contain the tree depth as a big-endian integer.
**      For non-root nodes, the first 2 bytes are left unused.
**
**   2. The next 2 bytes contain the number of entries currently 
**      stored in the node.
**
**   3. The remainder of the node contains the node entries. Each entry
**      consists of a single 8-byte integer followed by an even number
**      of 4-byte coordinates. For leaf nodes the integer is the rowid
**      of a record. For internal nodes it is the node number of a
**      child page.
*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_RTREE)

#ifndef SQLITE_CORE
/*   #include "sqlite3ext.h" */
  SQLITE_EXTENSION_INIT1
#else
/*   #include "sqlite3.h" */
#endif

/* #include <string.h> */
/* #include <assert.h> */
/* #include <stdio.h> */

#ifndef SQLITE_AMALGAMATION
#include "sqlite3rtree.h"
typedef sqlite3_int64 i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#endif

/*  The following macro is used to suppress compiler warnings.
*/
#ifndef UNUSED_PARAMETER
# define UNUSED_PARAMETER(x) (void)(x)
#endif

typedef struct Rtree Rtree;
typedef struct RtreeCursor RtreeCursor;
typedef struct RtreeNode RtreeNode;
typedef struct RtreeCell RtreeCell;
typedef struct RtreeConstraint RtreeConstraint;
typedef struct RtreeMatchArg RtreeMatchArg;
typedef struct RtreeGeomCallback RtreeGeomCallback;
typedef union RtreeCoord RtreeCoord;
typedef struct RtreeSearchPoint RtreeSearchPoint;

/* The rtree may have between 1 and RTREE_MAX_DIMENSIONS dimensions. */
#define RTREE_MAX_DIMENSIONS 5

/* Size of hash table Rtree.aHash. This hash table is not expected to
** ever contain very many entries, so a fixed number of buckets is 
** used.
*/
#define HASHSIZE 97

/* The xBestIndex method of this virtual table requires an estimate of
** the number of rows in the virtual table to calculate the costs of
** various strategies. If possible, this estimate is loaded from the
** sqlite_stat1 table (with RTREE_MIN_ROWEST as a hard-coded minimum).
** Otherwise, if no sqlite_stat1 entry is available, use 
** RTREE_DEFAULT_ROWEST.
*/
#define RTREE_DEFAULT_ROWEST 1048576
#define RTREE_MIN_ROWEST         100

/* 
** An rtree virtual-table object.
*/
struct Rtree {
  sqlite3_vtab base;          /* Base class.  Must be first */
  sqlite3 *db;                /* Host database connection */
  int iNodeSize;              /* Size in bytes of each node in the node table */
  u8 nDim;                    /* Number of dimensions */
  u8 eCoordType;              /* RTREE_COORD_REAL32 or RTREE_COORD_INT32 */
  u8 nBytesPerCell;           /* Bytes consumed per cell */
  int iDepth;                 /* Current depth of the r-tree structure */
  char *zDb;                  /* Name of database containing r-tree table */
  char *zName;                /* Name of r-tree table */ 
  int nBusy;                  /* Current number of users of this structure */
  i64 nRowEst;                /* Estimated number of rows in this table */

  /* List of nodes removed during a CondenseTree operation. List is
  ** linked together via the pointer normally used for hash chains -
  ** RtreeNode.pNext. RtreeNode.iNode stores the depth of the sub-tree 
  ** headed by the node (leaf nodes have RtreeNode.iNode==0).
  */
  RtreeNode *pDeleted;
  int iReinsertHeight;        /* Height of sub-trees Reinsert() has run on */

  /* Statements to read/write/delete a record from xxx_node */
  sqlite3_stmt *pReadNode;
  sqlite3_stmt *pWriteNode;
  sqlite3_stmt *pDeleteNode;

  /* Statements to read/write/delete a record from xxx_rowid */
  sqlite3_stmt *pReadRowid;
  sqlite3_stmt *pWriteRowid;
  sqlite3_stmt *pDeleteRowid;

  /* Statements to read/write/delete a record from xxx_parent */
  sqlite3_stmt *pReadParent;
  sqlite3_stmt *pWriteParent;
  sqlite3_stmt *pDeleteParent;

  RtreeNode *aHash[HASHSIZE]; /* Hash table of in-memory nodes. */ 
};

/* Possible values for Rtree.eCoordType: */
#define RTREE_COORD_REAL32 0
#define RTREE_COORD_INT32  1

/*
** If SQLITE_RTREE_INT_ONLY is defined, then this virtual table will
** only deal with integer coordinates.  No floating point operations
** will be done.
*/
#ifdef SQLITE_RTREE_INT_ONLY
  typedef sqlite3_int64 RtreeDValue;       /* High accuracy coordinate */
  typedef int RtreeValue;                  /* Low accuracy coordinate */
# define RTREE_ZERO 0
#else
  typedef double RtreeDValue;              /* High accuracy coordinate */
  typedef float RtreeValue;                /* Low accuracy coordinate */
# define RTREE_ZERO 0.0
#endif

/*
** When doing a search of an r-tree, instances of the following structure
** record intermediate results from the tree walk.
**
** The id is always a node-id.  For iLevel>=1 the id is the node-id of
** the node that the RtreeSearchPoint represents.  When iLevel==0, however,
** the id is of the parent node and the cell that RtreeSearchPoint
** represents is the iCell-th entry in the parent node.
*/
struct RtreeSearchPoint {
  RtreeDValue rScore;    /* The score for this node.  Smallest goes first. */
  sqlite3_int64 id;      /* Node ID */
  u8 iLevel;             /* 0=entries.  1=leaf node.  2+ for higher */
  u8 eWithin;            /* PARTLY_WITHIN or FULLY_WITHIN */
  u8 iCell;              /* Cell index within the node */
};

/*
** The minimum number of cells allowed for a node is a third of the 
** maximum. In Gutman's notation:
**
**     m = M/3
**
** If an R*-tree "Reinsert" operation is required, the same number of
** cells are removed from the overfull node and reinserted into the tree.
*/
#define RTREE_MINCELLS(p) ((((p)->iNodeSize-4)/(p)->nBytesPerCell)/3)
#define RTREE_REINSERT(p) RTREE_MINCELLS(p)
#define RTREE_MAXCELLS 51

/*
** The smallest possible node-size is (512-64)==448 bytes. And the largest
** supported cell size is 48 bytes (8 byte rowid + ten 4 byte coordinates).
** Therefore all non-root nodes must contain at least 3 entries. Since 
** 2^40 is greater than 2^64, an r-tree structure always has a depth of
** 40 or less.
*/
#define RTREE_MAX_DEPTH 40


/*
** Number of entries in the cursor RtreeNode cache.  The first entry is
** used to cache the RtreeNode for RtreeCursor.sPoint.  The remaining
** entries cache the RtreeNode for the first elements of the priority queue.
*/
#define RTREE_CACHE_SZ  5

/* 
** An rtree cursor object.
*/
struct RtreeCursor {
  sqlite3_vtab_cursor base;         /* Base class.  Must be first */
  u8 atEOF;                         /* True if at end of search */
  u8 bPoint;                        /* True if sPoint is valid */
  int iStrategy;                    /* Copy of idxNum search parameter */
  int nConstraint;                  /* Number of entries in aConstraint */
  RtreeConstraint *aConstraint;     /* Search constraints. */
  int nPointAlloc;                  /* Number of slots allocated for aPoint[] */
  int nPoint;                       /* Number of slots used in aPoint[] */
  int mxLevel;                      /* iLevel value for root of the tree */
  RtreeSearchPoint *aPoint;         /* Priority queue for search points */
  RtreeSearchPoint sPoint;          /* Cached next search point */
  RtreeNode *aNode[RTREE_CACHE_SZ]; /* Rtree node cache */
  u32 anQueue[RTREE_MAX_DEPTH+1];   /* Number of queued entries by iLevel */
};

/* Return the Rtree of a RtreeCursor */
#define RTREE_OF_CURSOR(X)   ((Rtree*)((X)->base.pVtab))

/*
** A coordinate can be either a floating point number or a integer.  All
** coordinates within a single R-Tree are always of the same time.
*/
union RtreeCoord {
  RtreeValue f;      /* Floating point value */
  int i;             /* Integer value */
  u32 u;             /* Unsigned for byte-order conversions */
};

/*
** The argument is an RtreeCoord. Return the value stored within the RtreeCoord
** formatted as a RtreeDValue (double or int64). This macro assumes that local
** variable pRtree points to the Rtree structure associated with the
** RtreeCoord.
*/
#ifdef SQLITE_RTREE_INT_ONLY
# define DCOORD(coord) ((RtreeDValue)coord.i)
#else
# define DCOORD(coord) (                           \
    (pRtree->eCoordType==RTREE_COORD_REAL32) ?      \
      ((double)coord.f) :                           \
      ((double)coord.i)                             \
  )
#endif

/*
** A search constraint.
*/
struct RtreeConstraint {
  int iCoord;                     /* Index of constrained coordinate */
  int op;                         /* Constraining operation */
  union {
    RtreeDValue rValue;             /* Constraint value. */
    int (*xGeom)(sqlite3_rtree_geometry*,int,RtreeDValue*,int*);
    int (*xQueryFunc)(sqlite3_rtree_query_info*);
  } u;
  sqlite3_rtree_query_info *pInfo;  /* xGeom and xQueryFunc argument */
};

/* Possible values for RtreeConstraint.op */
#define RTREE_EQ    0x41  /* A */
#define RTREE_LE    0x42  /* B */
#define RTREE_LT    0x43  /* C */
#define RTREE_GE    0x44  /* D */
#define RTREE_GT    0x45  /* E */
#define RTREE_MATCH 0x46  /* F: Old-style sqlite3_rtree_geometry_callback() */
#define RTREE_QUERY 0x47  /* G: New-style sqlite3_rtree_query_callback() */


/* 
** An rtree structure node.
*/
struct RtreeNode {
  RtreeNode *pParent;         /* Parent node */
  i64 iNode;                  /* The node number */
  int nRef;                   /* Number of references to this node */
  int isDirty;                /* True if the node needs to be written to disk */
  u8 *zData;                  /* Content of the node, as should be on disk */
  RtreeNode *pNext;           /* Next node in this hash collision chain */
};

/* Return the number of cells in a node  */
#define NCELL(pNode) readInt16(&(pNode)->zData[2])

/* 
** A single cell from a node, deserialized
*/
struct RtreeCell {
  i64 iRowid;                                 /* Node or entry ID */
  RtreeCoord aCoord[RTREE_MAX_DIMENSIONS*2];  /* Bounding box coordinates */
};


/*
** This object becomes the sqlite3_user_data() for the SQL functions
** that are created by sqlite3_rtree_geometry_callback() and
** sqlite3_rtree_query_callback() and which appear on the right of MATCH
** operators in order to constrain a search.
**
** xGeom and xQueryFunc are the callback functions.  Exactly one of 
** xGeom and xQueryFunc fields is non-NULL, depending on whether the
** SQL function was created using sqlite3_rtree_geometry_callback() or
** sqlite3_rtree_query_callback().
** 
** This object is deleted automatically by the destructor mechanism in
** sqlite3_create_function_v2().
*/
struct RtreeGeomCallback {
  int (*xGeom)(sqlite3_rtree_geometry*, int, RtreeDValue*, int*);
  int (*xQueryFunc)(sqlite3_rtree_query_info*);
  void (*xDestructor)(void*);
  void *pContext;
};


/*
** Value for the first field of every RtreeMatchArg object. The MATCH
** operator tests that the first field of a blob operand matches this
** value to avoid operating on invalid blobs (which could cause a segfault).
*/
#define RTREE_GEOMETRY_MAGIC 0x891245AB

/*
** An instance of this structure (in the form of a BLOB) is returned by
** the SQL functions that sqlite3_rtree_geometry_callback() and
** sqlite3_rtree_query_callback() create, and is read as the right-hand
** operand to the MATCH operator of an R-Tree.
*/
struct RtreeMatchArg {
  u32 magic;                  /* Always RTREE_GEOMETRY_MAGIC */
  RtreeGeomCallback cb;       /* Info about the callback functions */
  int nParam;                 /* Number of parameters to the SQL function */
  sqlite3_value **apSqlParam; /* Original SQL parameter values */
  RtreeDValue aParam[1];      /* Values for parameters to the SQL function */
};

#ifndef MAX
# define MAX(x,y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
# define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif

/*
** Functions to deserialize a 16 bit integer, 32 bit real number and
** 64 bit integer. The deserialized value is returned.
*/
static int readInt16(u8 *p){
  return (p[0]<<8) + p[1];
}
static void readCoord(u8 *p, RtreeCoord *pCoord){
  pCoord->u = (
    (((u32)p[0]) << 24) + 
    (((u32)p[1]) << 16) + 
    (((u32)p[2]) <<  8) + 
    (((u32)p[3]) <<  0)
  );
}
static i64 readInt64(u8 *p){
  return (
    (((i64)p[0]) << 56) + 
    (((i64)p[1]) << 48) + 
    (((i64)p[2]) << 40) + 
    (((i64)p[3]) << 32) + 
    (((i64)p[4]) << 24) + 
    (((i64)p[5]) << 16) + 
    (((i64)p[6]) <<  8) + 
    (((i64)p[7]) <<  0)
  );
}

/*
** Functions to serialize a 16 bit integer, 32 bit real number and
** 64 bit integer. The value returned is the number of bytes written
** to the argument buffer (always 2, 4 and 8 respectively).
*/
static int writeInt16(u8 *p, int i){
  p[0] = (i>> 8)&0xFF;
  p[1] = (i>> 0)&0xFF;
  return 2;
}
static int writeCoord(u8 *p, RtreeCoord *pCoord){
  u32 i;
  assert( sizeof(RtreeCoord)==4 );
  assert( sizeof(u32)==4 );
  i = pCoord->u;
  p[0] = (i>>24)&0xFF;
  p[1] = (i>>16)&0xFF;
  p[2] = (i>> 8)&0xFF;
  p[3] = (i>> 0)&0xFF;
  return 4;
}
static int writeInt64(u8 *p, i64 i){
  p[0] = (i>>56)&0xFF;
  p[1] = (i>>48)&0xFF;
  p[2] = (i>>40)&0xFF;
  p[3] = (i>>32)&0xFF;
  p[4] = (i>>24)&0xFF;
  p[5] = (i>>16)&0xFF;
  p[6] = (i>> 8)&0xFF;
  p[7] = (i>> 0)&0xFF;
  return 8;
}

/*
** Increment the reference count of node p.
*/
static void nodeReference(RtreeNode *p){
  if( p ){
    p->nRef++;
  }
}

/*
** Clear the content of node p (set all bytes to 0x00).
*/
static void nodeZero(Rtree *pRtree, RtreeNode *p){
  memset(&p->zData[2], 0, pRtree->iNodeSize-2);
  p->isDirty = 1;
}

/*
** Given a node number iNode, return the corresponding key to use
** in the Rtree.aHash table.
*/
static int nodeHash(i64 iNode){
  return iNode % HASHSIZE;
}

/*
** Search the node hash table for node iNode. If found, return a pointer
** to it. Otherwise, return 0.
*/
static RtreeNode *nodeHashLookup(Rtree *pRtree, i64 iNode){
  RtreeNode *p;
  for(p=pRtree->aHash[nodeHash(iNode)]; p && p->iNode!=iNode; p=p->pNext);
  return p;
}

/*
** Add node pNode to the node hash table.
*/
static void nodeHashInsert(Rtree *pRtree, RtreeNode *pNode){
  int iHash;
  assert( pNode->pNext==0 );
  iHash = nodeHash(pNode->iNode);
  pNode->pNext = pRtree->aHash[iHash];
  pRtree->aHash[iHash] = pNode;
}

/*
** Remove node pNode from the node hash table.
*/
static void nodeHashDelete(Rtree *pRtree, RtreeNode *pNode){
  RtreeNode **pp;
  if( pNode->iNode!=0 ){
    pp = &pRtree->aHash[nodeHash(pNode->iNode)];
    for( ; (*pp)!=pNode; pp = &(*pp)->pNext){ assert(*pp); }
    *pp = pNode->pNext;
    pNode->pNext = 0;
  }
}

/*
** Allocate and return new r-tree node. Initially, (RtreeNode.iNode==0),
** indicating that node has not yet been assigned a node number. It is
** assigned a node number when nodeWrite() is called to write the
** node contents out to the database.
*/
static RtreeNode *nodeNew(Rtree *pRtree, RtreeNode *pParent){
  RtreeNode *pNode;
  pNode = (RtreeNode *)sqlite3_malloc(sizeof(RtreeNode) + pRtree->iNodeSize);
  if( pNode ){
    memset(pNode, 0, sizeof(RtreeNode) + pRtree->iNodeSize);
    pNode->zData = (u8 *)&pNode[1];
    pNode->nRef = 1;
    pNode->pParent = pParent;
    pNode->isDirty = 1;
    nodeReference(pParent);
  }
  return pNode;
}

/*
** Obtain a reference to an r-tree node.
*/
static int nodeAcquire(
  Rtree *pRtree,             /* R-tree structure */
  i64 iNode,                 /* Node number to load */
  RtreeNode *pParent,        /* Either the parent node or NULL */
  RtreeNode **ppNode         /* OUT: Acquired node */
){
  int rc;
  int rc2 = SQLITE_OK;
  RtreeNode *pNode;

  /* Check if the requested node is already in the hash table. If so,
  ** increase its reference count and return it.
  */
  if( (pNode = nodeHashLookup(pRtree, iNode)) ){
    assert( !pParent || !pNode->pParent || pNode->pParent==pParent );
    if( pParent && !pNode->pParent ){
      nodeReference(pParent);
      pNode->pParent = pParent;
    }
    pNode->nRef++;
    *ppNode = pNode;
    return SQLITE_OK;
  }

  sqlite3_bind_int64(pRtree->pReadNode, 1, iNode);
  rc = sqlite3_step(pRtree->pReadNode);
  if( rc==SQLITE_ROW ){
    const u8 *zBlob = sqlite3_column_blob(pRtree->pReadNode, 0);
    if( pRtree->iNodeSize==sqlite3_column_bytes(pRtree->pReadNode, 0) ){
      pNode = (RtreeNode *)sqlite3_malloc(sizeof(RtreeNode)+pRtree->iNodeSize);
      if( !pNode ){
        rc2 = SQLITE_NOMEM;
      }else{
        pNode->pParent = pParent;
        pNode->zData = (u8 *)&pNode[1];
        pNode->nRef = 1;
        pNode->iNode = iNode;
        pNode->isDirty = 0;
        pNode->pNext = 0;
        memcpy(pNode->zData, zBlob, pRtree->iNodeSize);
        nodeReference(pParent);
      }
    }
  }
  rc = sqlite3_reset(pRtree->pReadNode);
  if( rc==SQLITE_OK ) rc = rc2;

  /* If the root node was just loaded, set pRtree->iDepth to the height
  ** of the r-tree structure. A height of zero means all data is stored on
  ** the root node. A height of one means the children of the root node
  ** are the leaves, and so on. If the depth as specified on the root node
  ** is greater than RTREE_MAX_DEPTH, the r-tree structure must be corrupt.
  */
  if( pNode && iNode==1 ){
    pRtree->iDepth = readInt16(pNode->zData);
    if( pRtree->iDepth>RTREE_MAX_DEPTH ){
      rc = SQLITE_CORRUPT_VTAB;
    }
  }

  /* If no error has occurred so far, check if the "number of entries"
  ** field on the node is too large. If so, set the return code to 
  ** SQLITE_CORRUPT_VTAB.
  */
  if( pNode && rc==SQLITE_OK ){
    if( NCELL(pNode)>((pRtree->iNodeSize-4)/pRtree->nBytesPerCell) ){
      rc = SQLITE_CORRUPT_VTAB;
    }
  }

  if( rc==SQLITE_OK ){
    if( pNode!=0 ){
      nodeHashInsert(pRtree, pNode);
    }else{
      rc = SQLITE_CORRUPT_VTAB;
    }
    *ppNode = pNode;
  }else{
    sqlite3_free(pNode);
    *ppNode = 0;
  }

  return rc;
}

/*
** Overwrite cell iCell of node pNode with the contents of pCell.
*/
static void nodeOverwriteCell(
  Rtree *pRtree,             /* The overall R-Tree */
  RtreeNode *pNode,          /* The node into which the cell is to be written */
  RtreeCell *pCell,          /* The cell to write */
  int iCell                  /* Index into pNode into which pCell is written */
){
  int ii;
  u8 *p = &pNode->zData[4 + pRtree->nBytesPerCell*iCell];
  p += writeInt64(p, pCell->iRowid);
  for(ii=0; ii<(pRtree->nDim*2); ii++){
    p += writeCoord(p, &pCell->aCoord[ii]);
  }
  pNode->isDirty = 1;
}

/*
** Remove the cell with index iCell from node pNode.
*/
static void nodeDeleteCell(Rtree *pRtree, RtreeNode *pNode, int iCell){
  u8 *pDst = &pNode->zData[4 + pRtree->nBytesPerCell*iCell];
  u8 *pSrc = &pDst[pRtree->nBytesPerCell];
  int nByte = (NCELL(pNode) - iCell - 1) * pRtree->nBytesPerCell;
  memmove(pDst, pSrc, nByte);
  writeInt16(&pNode->zData[2], NCELL(pNode)-1);
  pNode->isDirty = 1;
}

/*
** Insert the contents of cell pCell into node pNode. If the insert
** is successful, return SQLITE_OK.
**
** If there is not enough free space in pNode, return SQLITE_FULL.
*/
static int nodeInsertCell(
  Rtree *pRtree,                /* The overall R-Tree */
  RtreeNode *pNode,             /* Write new cell into this node */
  RtreeCell *pCell              /* The cell to be inserted */
){
  int nCell;                    /* Current number of cells in pNode */
  int nMaxCell;                 /* Maximum number of cells for pNode */

  nMaxCell = (pRtree->iNodeSize-4)/pRtree->nBytesPerCell;
  nCell = NCELL(pNode);

  assert( nCell<=nMaxCell );
  if( nCell<nMaxCell ){
    nodeOverwriteCell(pRtree, pNode, pCell, nCell);
    writeInt16(&pNode->zData[2], nCell+1);
    pNode->isDirty = 1;
  }

  return (nCell==nMaxCell);
}

/*
** If the node is dirty, write it out to the database.
*/
static int nodeWrite(Rtree *pRtree, RtreeNode *pNode){
  int rc = SQLITE_OK;
  if( pNode->isDirty ){
    sqlite3_stmt *p = pRtree->pWriteNode;
    if( pNode->iNode ){
      sqlite3_bind_int64(p, 1, pNode->iNode);
    }else{
      sqlite3_bind_null(p, 1);
    }
    sqlite3_bind_blob(p, 2, pNode->zData, pRtree->iNodeSize, SQLITE_STATIC);
    sqlite3_step(p);
    pNode->isDirty = 0;
    rc = sqlite3_reset(p);
    if( pNode->iNode==0 && rc==SQLITE_OK ){
      pNode->iNode = sqlite3_last_insert_rowid(pRtree->db);
      nodeHashInsert(pRtree, pNode);
    }
  }
  return rc;
}

/*
** Release a reference to a node. If the node is dirty and the reference
** count drops to zero, the node data is written to the database.
*/
static int nodeRelease(Rtree *pRtree, RtreeNode *pNode){
  int rc = SQLITE_OK;
  if( pNode ){
    assert( pNode->nRef>0 );
    pNode->nRef--;
    if( pNode->nRef==0 ){
      if( pNode->iNode==1 ){
        pRtree->iDepth = -1;
      }
      if( pNode->pParent ){
        rc = nodeRelease(pRtree, pNode->pParent);
      }
      if( rc==SQLITE_OK ){
        rc = nodeWrite(pRtree, pNode);
      }
      nodeHashDelete(pRtree, pNode);
      sqlite3_free(pNode);
    }
  }
  return rc;
}

/*
** Return the 64-bit integer value associated with cell iCell of
** node pNode. If pNode is a leaf node, this is a rowid. If it is
** an internal node, then the 64-bit integer is a child page number.
*/
static i64 nodeGetRowid(
  Rtree *pRtree,       /* The overall R-Tree */
  RtreeNode *pNode,    /* The node from which to extract the ID */
  int iCell            /* The cell index from which to extract the ID */
){
  assert( iCell<NCELL(pNode) );
  return readInt64(&pNode->zData[4 + pRtree->nBytesPerCell*iCell]);
}

/*
** Return coordinate iCoord from cell iCell in node pNode.
*/
static void nodeGetCoord(
  Rtree *pRtree,               /* The overall R-Tree */
  RtreeNode *pNode,            /* The node from which to extract a coordinate */
  int iCell,                   /* The index of the cell within the node */
  int iCoord,                  /* Which coordinate to extract */
  RtreeCoord *pCoord           /* OUT: Space to write result to */
){
  readCoord(&pNode->zData[12 + pRtree->nBytesPerCell*iCell + 4*iCoord], pCoord);
}

/*
** Deserialize cell iCell of node pNode. Populate the structure pointed
** to by pCell with the results.
*/
static void nodeGetCell(
  Rtree *pRtree,               /* The overall R-Tree */
  RtreeNode *pNode,            /* The node containing the cell to be read */
  int iCell,                   /* Index of the cell within the node */
  RtreeCell *pCell             /* OUT: Write the cell contents here */
){
  u8 *pData;
  RtreeCoord *pCoord;
  int ii;
  pCell->iRowid = nodeGetRowid(pRtree, pNode, iCell);
  pData = pNode->zData + (12 + pRtree->nBytesPerCell*iCell);
  pCoord = pCell->aCoord;
  for(ii=0; ii<pRtree->nDim*2; ii++){
    readCoord(&pData[ii*4], &pCoord[ii]);
  }
}


/* Forward declaration for the function that does the work of
** the virtual table module xCreate() and xConnect() methods.
*/
static int rtreeInit(
  sqlite3 *, void *, int, const char *const*, sqlite3_vtab **, char **, int
);

/* 
** Rtree virtual table module xCreate method.
*/
static int rtreeCreate(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return rtreeInit(db, pAux, argc, argv, ppVtab, pzErr, 1);
}

/* 
** Rtree virtual table module xConnect method.
*/
static int rtreeConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return rtreeInit(db, pAux, argc, argv, ppVtab, pzErr, 0);
}

/*
** Increment the r-tree reference count.
*/
static void rtreeReference(Rtree *pRtree){
  pRtree->nBusy++;
}

/*
** Decrement the r-tree reference count. When the reference count reaches
** zero the structure is deleted.
*/
static void rtreeRelease(Rtree *pRtree){
  pRtree->nBusy--;
  if( pRtree->nBusy==0 ){
    sqlite3_finalize(pRtree->pReadNode);
    sqlite3_finalize(pRtree->pWriteNode);
    sqlite3_finalize(pRtree->pDeleteNode);
    sqlite3_finalize(pRtree->pReadRowid);
    sqlite3_finalize(pRtree->pWriteRowid);
    sqlite3_finalize(pRtree->pDeleteRowid);
    sqlite3_finalize(pRtree->pReadParent);
    sqlite3_finalize(pRtree->pWriteParent);
    sqlite3_finalize(pRtree->pDeleteParent);
    sqlite3_free(pRtree);
  }
}

/* 
** Rtree virtual table module xDisconnect method.
*/
static int rtreeDisconnect(sqlite3_vtab *pVtab){
  rtreeRelease((Rtree *)pVtab);
  return SQLITE_OK;
}

/* 
** Rtree virtual table module xDestroy method.
*/
static int rtreeDestroy(sqlite3_vtab *pVtab){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc;
  char *zCreate = sqlite3_mprintf(
    "DROP TABLE '%q'.'%q_node';"
    "DROP TABLE '%q'.'%q_rowid';"
    "DROP TABLE '%q'.'%q_parent';",
    pRtree->zDb, pRtree->zName, 
    pRtree->zDb, pRtree->zName,
    pRtree->zDb, pRtree->zName
  );
  if( !zCreate ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_exec(pRtree->db, zCreate, 0, 0, 0);
    sqlite3_free(zCreate);
  }
  if( rc==SQLITE_OK ){
    rtreeRelease(pRtree);
  }

  return rc;
}

/* 
** Rtree virtual table module xOpen method.
*/
static int rtreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  int rc = SQLITE_NOMEM;
  RtreeCursor *pCsr;

  pCsr = (RtreeCursor *)sqlite3_malloc(sizeof(RtreeCursor));
  if( pCsr ){
    memset(pCsr, 0, sizeof(RtreeCursor));
    pCsr->base.pVtab = pVTab;
    rc = SQLITE_OK;
  }
  *ppCursor = (sqlite3_vtab_cursor *)pCsr;

  return rc;
}


/*
** Free the RtreeCursor.aConstraint[] array and its contents.
*/
static void freeCursorConstraints(RtreeCursor *pCsr){
  if( pCsr->aConstraint ){
    int i;                        /* Used to iterate through constraint array */
    for(i=0; i<pCsr->nConstraint; i++){
      sqlite3_rtree_query_info *pInfo = pCsr->aConstraint[i].pInfo;
      if( pInfo ){
        if( pInfo->xDelUser ) pInfo->xDelUser(pInfo->pUser);
        sqlite3_free(pInfo);
      }
    }
    sqlite3_free(pCsr->aConstraint);
    pCsr->aConstraint = 0;
  }
}

/* 
** Rtree virtual table module xClose method.
*/
static int rtreeClose(sqlite3_vtab_cursor *cur){
  Rtree *pRtree = (Rtree *)(cur->pVtab);
  int ii;
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  freeCursorConstraints(pCsr);
  sqlite3_free(pCsr->aPoint);
  for(ii=0; ii<RTREE_CACHE_SZ; ii++) nodeRelease(pRtree, pCsr->aNode[ii]);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Rtree virtual table module xEof method.
**
** Return non-zero if the cursor does not currently point to a valid 
** record (i.e if the scan has finished), or zero otherwise.
*/
static int rtreeEof(sqlite3_vtab_cursor *cur){
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  return pCsr->atEOF;
}

/*
** Convert raw bits from the on-disk RTree record into a coordinate value.
** The on-disk format is big-endian and needs to be converted for little-
** endian platforms.  The on-disk record stores integer coordinates if
** eInt is true and it stores 32-bit floating point records if eInt is
** false.  a[] is the four bytes of the on-disk record to be decoded.
** Store the results in "r".
**
** There are three versions of this macro, one each for little-endian and
** big-endian processors and a third generic implementation.  The endian-
** specific implementations are much faster and are preferred if the
** processor endianness is known at compile-time.  The SQLITE_BYTEORDER
** macro is part of sqliteInt.h and hence the endian-specific
** implementation will only be used if this module is compiled as part
** of the amalgamation.
*/
#if defined(SQLITE_BYTEORDER) && SQLITE_BYTEORDER==1234
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    memcpy(&c.u,a,4);                                           \
    c.u = ((c.u>>24)&0xff)|((c.u>>8)&0xff00)|                   \
          ((c.u&0xff)<<24)|((c.u&0xff00)<<8);                   \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#elif defined(SQLITE_BYTEORDER) && SQLITE_BYTEORDER==4321
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    memcpy(&c.u,a,4);                                           \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#else
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    c.u = ((u32)a[0]<<24) + ((u32)a[1]<<16)                     \
           +((u32)a[2]<<8) + a[3];                              \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#endif

/*
** Check the RTree node or entry given by pCellData and p against the MATCH
** constraint pConstraint.  
*/
static int rtreeCallbackConstraint(
  RtreeConstraint *pConstraint,  /* The constraint to test */
  int eInt,                      /* True if RTree holding integer coordinates */
  u8 *pCellData,                 /* Raw cell content */
  RtreeSearchPoint *pSearch,     /* Container of this cell */
  sqlite3_rtree_dbl *prScore,    /* OUT: score for the cell */
  int *peWithin                  /* OUT: visibility of the cell */
){
  int i;                                                /* Loop counter */
  sqlite3_rtree_query_info *pInfo = pConstraint->pInfo; /* Callback info */
  int nCoord = pInfo->nCoord;                           /* No. of coordinates */
  int rc;                                             /* Callback return code */
  sqlite3_rtree_dbl aCoord[RTREE_MAX_DIMENSIONS*2];   /* Decoded coordinates */

  assert( pConstraint->op==RTREE_MATCH || pConstraint->op==RTREE_QUERY );
  assert( nCoord==2 || nCoord==4 || nCoord==6 || nCoord==8 || nCoord==10 );

  if( pConstraint->op==RTREE_QUERY && pSearch->iLevel==1 ){
    pInfo->iRowid = readInt64(pCellData);
  }
  pCellData += 8;
  for(i=0; i<nCoord; i++, pCellData += 4){
    RTREE_DECODE_COORD(eInt, pCellData, aCoord[i]);
  }
  if( pConstraint->op==RTREE_MATCH ){
    rc = pConstraint->u.xGeom((sqlite3_rtree_geometry*)pInfo,
                              nCoord, aCoord, &i);
    if( i==0 ) *peWithin = NOT_WITHIN;
    *prScore = RTREE_ZERO;
  }else{
    pInfo->aCoord = aCoord;
    pInfo->iLevel = pSearch->iLevel - 1;
    pInfo->rScore = pInfo->rParentScore = pSearch->rScore;
    pInfo->eWithin = pInfo->eParentWithin = pSearch->eWithin;
    rc = pConstraint->u.xQueryFunc(pInfo);
    if( pInfo->eWithin<*peWithin ) *peWithin = pInfo->eWithin;
    if( pInfo->rScore<*prScore || *prScore<RTREE_ZERO ){
      *prScore = pInfo->rScore;
    }
  }
  return rc;
}

/* 
** Check the internal RTree node given by pCellData against constraint p.
** If this constraint cannot be satisfied by any child within the node,
** set *peWithin to NOT_WITHIN.
*/
static void rtreeNonleafConstraint(
  RtreeConstraint *p,        /* The constraint to test */
  int eInt,                  /* True if RTree holds integer coordinates */
  u8 *pCellData,             /* Raw cell content as appears on disk */
  int *peWithin              /* Adjust downward, as appropriate */
){
  sqlite3_rtree_dbl val;     /* Coordinate value convert to a double */

  /* p->iCoord might point to either a lower or upper bound coordinate
  ** in a coordinate pair.  But make pCellData point to the lower bound.
  */
  pCellData += 8 + 4*(p->iCoord&0xfe);

  assert(p->op==RTREE_LE || p->op==RTREE_LT || p->op==RTREE_GE 
      || p->op==RTREE_GT || p->op==RTREE_EQ );
  switch( p->op ){
    case RTREE_LE:
    case RTREE_LT:
    case RTREE_EQ:
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the lower bound of the coordinate pair */
      if( p->u.rValue>=val ) return;
      if( p->op!=RTREE_EQ ) break;  /* RTREE_LE and RTREE_LT end here */
      /* Fall through for the RTREE_EQ case */

    default: /* RTREE_GT or RTREE_GE,  or fallthrough of RTREE_EQ */
      pCellData += 4;
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the upper bound of the coordinate pair */
      if( p->u.rValue<=val ) return;
  }
  *peWithin = NOT_WITHIN;
}

/*
** Check the leaf RTree cell given by pCellData against constraint p.
** If this constraint is not satisfied, set *peWithin to NOT_WITHIN.
** If the constraint is satisfied, leave *peWithin unchanged.
**
** The constraint is of the form:  xN op $val
**
** The op is given by p->op.  The xN is p->iCoord-th coordinate in
** pCellData.  $val is given by p->u.rValue.
*/
static void rtreeLeafConstraint(
  RtreeConstraint *p,        /* The constraint to test */
  int eInt,                  /* True if RTree holds integer coordinates */
  u8 *pCellData,             /* Raw cell content as appears on disk */
  int *peWithin              /* Adjust downward, as appropriate */
){
  RtreeDValue xN;      /* Coordinate value converted to a double */

  assert(p->op==RTREE_LE || p->op==RTREE_LT || p->op==RTREE_GE 
      || p->op==RTREE_GT || p->op==RTREE_EQ );
  pCellData += 8 + p->iCoord*4;
  RTREE_DECODE_COORD(eInt, pCellData, xN);
  switch( p->op ){
    case RTREE_LE: if( xN <= p->u.rValue ) return;  break;
    case RTREE_LT: if( xN <  p->u.rValue ) return;  break;
    case RTREE_GE: if( xN >= p->u.rValue ) return;  break;
    case RTREE_GT: if( xN >  p->u.rValue ) return;  break;
    default:       if( xN == p->u.rValue ) return;  break;
  }
  *peWithin = NOT_WITHIN;
}

/*
** One of the cells in node pNode is guaranteed to have a 64-bit 
** integer value equal to iRowid. Return the index of this cell.
*/
static int nodeRowidIndex(
  Rtree *pRtree, 
  RtreeNode *pNode, 
  i64 iRowid,
  int *piIndex
){
  int ii;
  int nCell = NCELL(pNode);
  assert( nCell<200 );
  for(ii=0; ii<nCell; ii++){
    if( nodeGetRowid(pRtree, pNode, ii)==iRowid ){
      *piIndex = ii;
      return SQLITE_OK;
    }
  }
  return SQLITE_CORRUPT_VTAB;
}

/*
** Return the index of the cell containing a pointer to node pNode
** in its parent. If pNode is the root node, return -1.
*/
static int nodeParentIndex(Rtree *pRtree, RtreeNode *pNode, int *piIndex){
  RtreeNode *pParent = pNode->pParent;
  if( pParent ){
    return nodeRowidIndex(pRtree, pParent, pNode->iNode, piIndex);
  }
  *piIndex = -1;
  return SQLITE_OK;
}

/*
** Compare two search points.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second.
**
** The rScore is the primary key.  Smaller rScore values come first.
** If the rScore is a tie, then use iLevel as the tie breaker with smaller
** iLevel values coming first.  In this way, if rScore is the same for all
** SearchPoints, then iLevel becomes the deciding factor and the result
** is a depth-first search, which is the desired default behavior.
*/
static int rtreeSearchPointCompare(
  const RtreeSearchPoint *pA,
  const RtreeSearchPoint *pB
){
  if( pA->rScore<pB->rScore ) return -1;
  if( pA->rScore>pB->rScore ) return +1;
  if( pA->iLevel<pB->iLevel ) return -1;
  if( pA->iLevel>pB->iLevel ) return +1;
  return 0;
}

/*
** Interchange to search points in a cursor.
*/
static void rtreeSearchPointSwap(RtreeCursor *p, int i, int j){
  RtreeSearchPoint t = p->aPoint[i];
  assert( i<j );
  p->aPoint[i] = p->aPoint[j];
  p->aPoint[j] = t;
  i++; j++;
  if( i<RTREE_CACHE_SZ ){
    if( j>=RTREE_CACHE_SZ ){
      nodeRelease(RTREE_OF_CURSOR(p), p->aNode[i]);
      p->aNode[i] = 0;
    }else{
      RtreeNode *pTemp = p->aNode[i];
      p->aNode[i] = p->aNode[j];
      p->aNode[j] = pTemp;
    }
  }
}

/*
** Return the search point with the lowest current score.
*/
static RtreeSearchPoint *rtreeSearchPointFirst(RtreeCursor *pCur){
  return pCur->bPoint ? &pCur->sPoint : pCur->nPoint ? pCur->aPoint : 0;
}

/*
** Get the RtreeNode for the search point with the lowest score.
*/
static RtreeNode *rtreeNodeOfFirstSearchPoint(RtreeCursor *pCur, int *pRC){
  sqlite3_int64 id;
  int ii = 1 - pCur->bPoint;
  assert( ii==0 || ii==1 );
  assert( pCur->bPoint || pCur->nPoint );
  if( pCur->aNode[ii]==0 ){
    assert( pRC!=0 );
    id = ii ? pCur->aPoint[0].id : pCur->sPoint.id;
    *pRC = nodeAcquire(RTREE_OF_CURSOR(pCur), id, 0, &pCur->aNode[ii]);
  }
  return pCur->aNode[ii];
}

/*
** Push a new element onto the priority queue
*/
static RtreeSearchPoint *rtreeEnqueue(
  RtreeCursor *pCur,    /* The cursor */
  RtreeDValue rScore,   /* Score for the new search point */
  u8 iLevel             /* Level for the new search point */
){
  int i, j;
  RtreeSearchPoint *pNew;
  if( pCur->nPoint>=pCur->nPointAlloc ){
    int nNew = pCur->nPointAlloc*2 + 8;
    pNew = sqlite3_realloc(pCur->aPoint, nNew*sizeof(pCur->aPoint[0]));
    if( pNew==0 ) return 0;
    pCur->aPoint = pNew;
    pCur->nPointAlloc = nNew;
  }
  i = pCur->nPoint++;
  pNew = pCur->aPoint + i;
  pNew->rScore = rScore;
  pNew->iLevel = iLevel;
  assert( iLevel<=RTREE_MAX_DEPTH );
  while( i>0 ){
    RtreeSearchPoint *pParent;
    j = (i-1)/2;
    pParent = pCur->aPoint + j;
    if( rtreeSearchPointCompare(pNew, pParent)>=0 ) break;
    rtreeSearchPointSwap(pCur, j, i);
    i = j;
    pNew = pParent;
  }
  return pNew;
}

/*
** Allocate a new RtreeSearchPoint and return a pointer to it.  Return
** NULL if malloc fails.
*/
static RtreeSearchPoint *rtreeSearchPointNew(
  RtreeCursor *pCur,    /* The cursor */
  RtreeDValue rScore,   /* Score for the new search point */
  u8 iLevel             /* Level for the new search point */
){
  RtreeSearchPoint *pNew, *pFirst;
  pFirst = rtreeSearchPointFirst(pCur);
  pCur->anQueue[iLevel]++;
  if( pFirst==0
   || pFirst->rScore>rScore 
   || (pFirst->rScore==rScore && pFirst->iLevel>iLevel)
  ){
    if( pCur->bPoint ){
      int ii;
      pNew = rtreeEnqueue(pCur, rScore, iLevel);
      if( pNew==0 ) return 0;
      ii = (int)(pNew - pCur->aPoint) + 1;
      if( ii<RTREE_CACHE_SZ ){
        assert( pCur->aNode[ii]==0 );
        pCur->aNode[ii] = pCur->aNode[0];
       }else{
        nodeRelease(RTREE_OF_CURSOR(pCur), pCur->aNode[0]);
      }
      pCur->aNode[0] = 0;
      *pNew = pCur->sPoint;
    }
    pCur->sPoint.rScore = rScore;
    pCur->sPoint.iLevel = iLevel;
    pCur->bPoint = 1;
    return &pCur->sPoint;
  }else{
    return rtreeEnqueue(pCur, rScore, iLevel);
  }
}

#if 0
/* Tracing routines for the RtreeSearchPoint queue */
static void tracePoint(RtreeSearchPoint *p, int idx, RtreeCursor *pCur){
  if( idx<0 ){ printf(" s"); }else{ printf("%2d", idx); }
  printf(" %d.%05lld.%02d %g %d",
    p->iLevel, p->id, p->iCell, p->rScore, p->eWithin
  );
  idx++;
  if( idx<RTREE_CACHE_SZ ){
    printf(" %p\n", pCur->aNode[idx]);
  }else{
    printf("\n");
  }
}
static void traceQueue(RtreeCursor *pCur, const char *zPrefix){
  int ii;
  printf("=== %9s ", zPrefix);
  if( pCur->bPoint ){
    tracePoint(&pCur->sPoint, -1, pCur);
  }
  for(ii=0; ii<pCur->nPoint; ii++){
    if( ii>0 || pCur->bPoint ) printf("              ");
    tracePoint(&pCur->aPoint[ii], ii, pCur);
  }
}
# define RTREE_QUEUE_TRACE(A,B) traceQueue(A,B)
#else
# define RTREE_QUEUE_TRACE(A,B)   /* no-op */
#endif

/* Remove the search point with the lowest current score.
*/
static void rtreeSearchPointPop(RtreeCursor *p){
  int i, j, k, n;
  i = 1 - p->bPoint;
  assert( i==0 || i==1 );
  if( p->aNode[i] ){
    nodeRelease(RTREE_OF_CURSOR(p), p->aNode[i]);
    p->aNode[i] = 0;
  }
  if( p->bPoint ){
    p->anQueue[p->sPoint.iLevel]--;
    p->bPoint = 0;
  }else if( p->nPoint ){
    p->anQueue[p->aPoint[0].iLevel]--;
    n = --p->nPoint;
    p->aPoint[0] = p->aPoint[n];
    if( n<RTREE_CACHE_SZ-1 ){
      p->aNode[1] = p->aNode[n+1];
      p->aNode[n+1] = 0;
    }
    i = 0;
    while( (j = i*2+1)<n ){
      k = j+1;
      if( k<n && rtreeSearchPointCompare(&p->aPoint[k], &p->aPoint[j])<0 ){
        if( rtreeSearchPointCompare(&p->aPoint[k], &p->aPoint[i])<0 ){
          rtreeSearchPointSwap(p, i, k);
          i = k;
        }else{
          break;
        }
      }else{
        if( rtreeSearchPointCompare(&p->aPoint[j], &p->aPoint[i])<0 ){
          rtreeSearchPointSwap(p, i, j);
          i = j;
        }else{
          break;
        }
      }
    }
  }
}


/*
** Continue the search on cursor pCur until the front of the queue
** contains an entry suitable for returning as a result-set row,
** or until the RtreeSearchPoint queue is empty, indicating that the
** query has completed.
*/
static int rtreeStepToLeaf(RtreeCursor *pCur){
  RtreeSearchPoint *p;
  Rtree *pRtree = RTREE_OF_CURSOR(pCur);
  RtreeNode *pNode;
  int eWithin;
  int rc = SQLITE_OK;
  int nCell;
  int nConstraint = pCur->nConstraint;
  int ii;
  int eInt;
  RtreeSearchPoint x;

  eInt = pRtree->eCoordType==RTREE_COORD_INT32;
  while( (p = rtreeSearchPointFirst(pCur))!=0 && p->iLevel>0 ){
    pNode = rtreeNodeOfFirstSearchPoint(pCur, &rc);
    if( rc ) return rc;
    nCell = NCELL(pNode);
    assert( nCell<200 );
    while( p->iCell<nCell ){
      sqlite3_rtree_dbl rScore = (sqlite3_rtree_dbl)-1;
      u8 *pCellData = pNode->zData + (4+pRtree->nBytesPerCell*p->iCell);
      eWithin = FULLY_WITHIN;
      for(ii=0; ii<nConstraint; ii++){
        RtreeConstraint *pConstraint = pCur->aConstraint + ii;
        if( pConstraint->op>=RTREE_MATCH ){
          rc = rtreeCallbackConstraint(pConstraint, eInt, pCellData, p,
                                       &rScore, &eWithin);
          if( rc ) return rc;
        }else if( p->iLevel==1 ){
          rtreeLeafConstraint(pConstraint, eInt, pCellData, &eWithin);
        }else{
          rtreeNonleafConstraint(pConstraint, eInt, pCellData, &eWithin);
        }
        if( eWithin==NOT_WITHIN ) break;
      }
      p->iCell++;
      if( eWithin==NOT_WITHIN ) continue;
      x.iLevel = p->iLevel - 1;
      if( x.iLevel ){
        x.id = readInt64(pCellData);
        x.iCell = 0;
      }else{
        x.id = p->id;
        x.iCell = p->iCell - 1;
      }
      if( p->iCell>=nCell ){
        RTREE_QUEUE_TRACE(pCur, "POP-S:");
        rtreeSearchPointPop(pCur);
      }
      if( rScore<RTREE_ZERO ) rScore = RTREE_ZERO;
      p = rtreeSearchPointNew(pCur, rScore, x.iLevel);
      if( p==0 ) return SQLITE_NOMEM;
      p->eWithin = eWithin;
      p->id = x.id;
      p->iCell = x.iCell;
      RTREE_QUEUE_TRACE(pCur, "PUSH-S:");
      break;
    }
    if( p->iCell>=nCell ){
      RTREE_QUEUE_TRACE(pCur, "POP-Se:");
      rtreeSearchPointPop(pCur);
    }
  }
  pCur->atEOF = p==0;
  return SQLITE_OK;
}

/* 
** Rtree virtual table module xNext method.
*/
static int rtreeNext(sqlite3_vtab_cursor *pVtabCursor){
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  int rc = SQLITE_OK;

  /* Move to the next entry that matches the configured constraints. */
  RTREE_QUEUE_TRACE(pCsr, "POP-Nx:");
  rtreeSearchPointPop(pCsr);
  rc = rtreeStepToLeaf(pCsr);
  return rc;
}

/* 
** Rtree virtual table module xRowid method.
*/
static int rtreeRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);
  if( rc==SQLITE_OK && p ){
    *pRowid = nodeGetRowid(RTREE_OF_CURSOR(pCsr), pNode, p->iCell);
  }
  return rc;
}

/* 
** Rtree virtual table module xColumn method.
*/
static int rtreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i){
  Rtree *pRtree = (Rtree *)cur->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  RtreeCoord c;
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);

  if( rc ) return rc;
  if( p==0 ) return SQLITE_OK;
  if( i==0 ){
    sqlite3_result_int64(ctx, nodeGetRowid(pRtree, pNode, p->iCell));
  }else{
    if( rc ) return rc;
    nodeGetCoord(pRtree, pNode, p->iCell, i-1, &c);
#ifndef SQLITE_RTREE_INT_ONLY
    if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
      sqlite3_result_double(ctx, c.f);
    }else
#endif
    {
      assert( pRtree->eCoordType==RTREE_COORD_INT32 );
      sqlite3_result_int(ctx, c.i);
    }
  }
  return SQLITE_OK;
}

/* 
** Use nodeAcquire() to obtain the leaf node containing the record with 
** rowid iRowid. If successful, set *ppLeaf to point to the node and
** return SQLITE_OK. If there is no such record in the table, set
** *ppLeaf to 0 and return SQLITE_OK. If an error occurs, set *ppLeaf
** to zero and return an SQLite error code.
*/
static int findLeafNode(
  Rtree *pRtree,              /* RTree to search */
  i64 iRowid,                 /* The rowid searching for */
  RtreeNode **ppLeaf,         /* Write the node here */
  sqlite3_int64 *piNode       /* Write the node-id here */
){
  int rc;
  *ppLeaf = 0;
  sqlite3_bind_int64(pRtree->pReadRowid, 1, iRowid);
  if( sqlite3_step(pRtree->pReadRowid)==SQLITE_ROW ){
    i64 iNode = sqlite3_column_int64(pRtree->pReadRowid, 0);
    if( piNode ) *piNode = iNode;
    rc = nodeAcquire(pRtree, iNode, 0, ppLeaf);
    sqlite3_reset(pRtree->pReadRowid);
  }else{
    rc = sqlite3_reset(pRtree->pReadRowid);
  }
  return rc;
}

/*
** This function is called to configure the RtreeConstraint object passed
** as the second argument for a MATCH constraint. The value passed as the
** first argument to this function is the right-hand operand to the MATCH
** operator.
*/
static int deserializeGeometry(sqlite3_value *pValue, RtreeConstraint *pCons){
  RtreeMatchArg *pBlob;              /* BLOB returned by geometry function */
  sqlite3_rtree_query_info *pInfo;   /* Callback information */
  int nBlob;                         /* Size of the geometry function blob */
  int nExpected;                     /* Expected size of the BLOB */

  /* Check that value is actually a blob. */
  if( sqlite3_value_type(pValue)!=SQLITE_BLOB ) return SQLITE_ERROR;

  /* Check that the blob is roughly the right size. */
  nBlob = sqlite3_value_bytes(pValue);
  if( nBlob<(int)sizeof(RtreeMatchArg) ){
    return SQLITE_ERROR;
  }

  pInfo = (sqlite3_rtree_query_info*)sqlite3_malloc( sizeof(*pInfo)+nBlob );
  if( !pInfo ) return SQLITE_NOMEM;
  memset(pInfo, 0, sizeof(*pInfo));
  pBlob = (RtreeMatchArg*)&pInfo[1];

  memcpy(pBlob, sqlite3_value_blob(pValue), nBlob);
  nExpected = (int)(sizeof(RtreeMatchArg) +
                    pBlob->nParam*sizeof(sqlite3_value*) +
                    (pBlob->nParam-1)*sizeof(RtreeDValue));
  if( pBlob->magic!=RTREE_GEOMETRY_MAGIC || nBlob!=nExpected ){
    sqlite3_free(pInfo);
    return SQLITE_ERROR;
  }
  pInfo->pContext = pBlob->cb.pContext;
  pInfo->nParam = pBlob->nParam;
  pInfo->aParam = pBlob->aParam;
  pInfo->apSqlParam = pBlob->apSqlParam;

  if( pBlob->cb.xGeom ){
    pCons->u.xGeom = pBlob->cb.xGeom;
  }else{
    pCons->op = RTREE_QUERY;
    pCons->u.xQueryFunc = pBlob->cb.xQueryFunc;
  }
  pCons->pInfo = pInfo;
  return SQLITE_OK;
}

/* 
** Rtree virtual table module xFilter method.
*/
static int rtreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  Rtree *pRtree = (Rtree *)pVtabCursor->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeNode *pRoot = 0;
  int ii;
  int rc = SQLITE_OK;
  int iCell = 0;

  rtreeReference(pRtree);

  /* Reset the cursor to the same state as rtreeOpen() leaves it in. */
  freeCursorConstraints(pCsr);
  sqlite3_free(pCsr->aPoint);
  memset(pCsr, 0, sizeof(RtreeCursor));
  pCsr->base.pVtab = (sqlite3_vtab*)pRtree;

  pCsr->iStrategy = idxNum;
  if( idxNum==1 ){
    /* Special case - lookup by rowid. */
    RtreeNode *pLeaf;        /* Leaf on which the required cell resides */
    RtreeSearchPoint *p;     /* Search point for the the leaf */
    i64 iRowid = sqlite3_value_int64(argv[0]);
    i64 iNode = 0;
    rc = findLeafNode(pRtree, iRowid, &pLeaf, &iNode);
    if( rc==SQLITE_OK && pLeaf!=0 ){
      p = rtreeSearchPointNew(pCsr, RTREE_ZERO, 0);
      assert( p!=0 );  /* Always returns pCsr->sPoint */
      pCsr->aNode[0] = pLeaf;
      p->id = iNode;
      p->eWithin = PARTLY_WITHIN;
      rc = nodeRowidIndex(pRtree, pLeaf, iRowid, &iCell);
      p->iCell = iCell;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-F1:");
    }else{
      pCsr->atEOF = 1;
    }
  }else{
    /* Normal case - r-tree scan. Set up the RtreeCursor.aConstraint array 
    ** with the configured constraints. 
    */
    rc = nodeAcquire(pRtree, 1, 0, &pRoot);
    if( rc==SQLITE_OK && argc>0 ){
      pCsr->aConstraint = sqlite3_malloc(sizeof(RtreeConstraint)*argc);
      pCsr->nConstraint = argc;
      if( !pCsr->aConstraint ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pCsr->aConstraint, 0, sizeof(RtreeConstraint)*argc);
        memset(pCsr->anQueue, 0, sizeof(u32)*(pRtree->iDepth + 1));
        assert( (idxStr==0 && argc==0)
                || (idxStr && (int)strlen(idxStr)==argc*2) );
        for(ii=0; ii<argc; ii++){
          RtreeConstraint *p = &pCsr->aConstraint[ii];
          p->op = idxStr[ii*2];
          p->iCoord = idxStr[ii*2+1]-'0';
          if( p->op>=RTREE_MATCH ){
            /* A MATCH operator. The right-hand-side must be a blob that
            ** can be cast into an RtreeMatchArg object. One created using
            ** an sqlite3_rtree_geometry_callback() SQL user function.
            */
            rc = deserializeGeometry(argv[ii], p);
            if( rc!=SQLITE_OK ){
              break;
            }
            p->pInfo->nCoord = pRtree->nDim*2;
            p->pInfo->anQueue = pCsr->anQueue;
            p->pInfo->mxLevel = pRtree->iDepth + 1;
          }else{
#ifdef SQLITE_RTREE_INT_ONLY
            p->u.rValue = sqlite3_value_int64(argv[ii]);
#else
            p->u.rValue = sqlite3_value_double(argv[ii]);
#endif
          }
        }
      }
    }
    if( rc==SQLITE_OK ){
      RtreeSearchPoint *pNew;
      pNew = rtreeSearchPointNew(pCsr, RTREE_ZERO, pRtree->iDepth+1);
      if( pNew==0 ) return SQLITE_NOMEM;
      pNew->id = 1;
      pNew->iCell = 0;
      pNew->eWithin = PARTLY_WITHIN;
      assert( pCsr->bPoint==1 );
      pCsr->aNode[0] = pRoot;
      pRoot = 0;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-Fm:");
      rc = rtreeStepToLeaf(pCsr);
    }
  }

  nodeRelease(pRtree, pRoot);
  rtreeRelease(pRtree);
  return rc;
}

/*
** Set the pIdxInfo->estimatedRows variable to nRow. Unless this
** extension is currently being used by a version of SQLite too old to
** support estimatedRows. In that case this function is a no-op.
*/
static void setEstimatedRows(sqlite3_index_info *pIdxInfo, i64 nRow){
#if SQLITE_VERSION_NUMBER>=3008002
  if( sqlite3_libversion_number()>=3008002 ){
    pIdxInfo->estimatedRows = nRow;
  }
#endif
}

/*
** Rtree virtual table module xBestIndex method. There are three
** table scan strategies to choose from (in order from most to 
** least desirable):
**
**   idxNum     idxStr        Strategy
**   ------------------------------------------------
**     1        Unused        Direct lookup by rowid.
**     2        See below     R-tree query or full-table scan.
**   ------------------------------------------------
**
** If strategy 1 is used, then idxStr is not meaningful. If strategy
** 2 is used, idxStr is formatted to contain 2 bytes for each 
** constraint used. The first two bytes of idxStr correspond to 
** the constraint in sqlite3_index_info.aConstraintUsage[] with
** (argvIndex==1) etc.
**
** The first of each pair of bytes in idxStr identifies the constraint
** operator as follows:
**
**   Operator    Byte Value
**   ----------------------
**      =        0x41 ('A')
**     <=        0x42 ('B')
**      <        0x43 ('C')
**     >=        0x44 ('D')
**      >        0x45 ('E')
**   MATCH       0x46 ('F')
**   ----------------------
**
** The second of each pair of bytes identifies the coordinate column
** to which the constraint applies. The leftmost coordinate column
** is 'a', the second from the left 'b' etc.
*/
static int rtreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  Rtree *pRtree = (Rtree*)tab;
  int rc = SQLITE_OK;
  int ii;
  int bMatch = 0;                 /* True if there exists a MATCH constraint */
  i64 nRow;                       /* Estimated rows returned by this scan */

  int iIdx = 0;
  char zIdxStr[RTREE_MAX_DIMENSIONS*8+1];
  memset(zIdxStr, 0, sizeof(zIdxStr));

  /* Check if there exists a MATCH constraint - even an unusable one. If there
  ** is, do not consider the lookup-by-rowid plan as using such a plan would
  ** require the VDBE to evaluate the MATCH constraint, which is not currently
  ** possible. */
  for(ii=0; ii<pIdxInfo->nConstraint; ii++){
    if( pIdxInfo->aConstraint[ii].op==SQLITE_INDEX_CONSTRAINT_MATCH ){
      bMatch = 1;
    }
  }

  assert( pIdxInfo->idxStr==0 );
  for(ii=0; ii<pIdxInfo->nConstraint && iIdx<(int)(sizeof(zIdxStr)-1); ii++){
    struct sqlite3_index_constraint *p = &pIdxInfo->aConstraint[ii];

    if( bMatch==0 && p->usable 
     && p->iColumn==0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ 
    ){
      /* We have an equality constraint on the rowid. Use strategy 1. */
      int jj;
      for(jj=0; jj<ii; jj++){
        pIdxInfo->aConstraintUsage[jj].argvIndex = 0;
        pIdxInfo->aConstraintUsage[jj].omit = 0;
      }
      pIdxInfo->idxNum = 1;
      pIdxInfo->aConstraintUsage[ii].argvIndex = 1;
      pIdxInfo->aConstraintUsage[jj].omit = 1;

      /* This strategy involves a two rowid lookups on an B-Tree structures
      ** and then a linear search of an R-Tree node. This should be 
      ** considered almost as quick as a direct rowid lookup (for which 
      ** sqlite uses an internal cost of 0.0). It is expected to return
      ** a single row.
      */ 
      pIdxInfo->estimatedCost = 30.0;
      setEstimatedRows(pIdxInfo, 1);
      return SQLITE_OK;
    }

    if( p->usable && (p->iColumn>0 || p->op==SQLITE_INDEX_CONSTRAINT_MATCH) ){
      u8 op;
      switch( p->op ){
        case SQLITE_INDEX_CONSTRAINT_EQ: op = RTREE_EQ; break;
        case SQLITE_INDEX_CONSTRAINT_GT: op = RTREE_GT; break;
        case SQLITE_INDEX_CONSTRAINT_LE: op = RTREE_LE; break;
        case SQLITE_INDEX_CONSTRAINT_LT: op = RTREE_LT; break;
        case SQLITE_INDEX_CONSTRAINT_GE: op = RTREE_GE; break;
        default:
          assert( p->op==SQLITE_INDEX_CONSTRAINT_MATCH );
          op = RTREE_MATCH; 
          break;
      }
      zIdxStr[iIdx++] = op;
      zIdxStr[iIdx++] = p->iColumn - 1 + '0';
      pIdxInfo->aConstraintUsage[ii].argvIndex = (iIdx/2);
      pIdxInfo->aConstraintUsage[ii].omit = 1;
    }
  }

  pIdxInfo->idxNum = 2;
  pIdxInfo->needToFreeIdxStr = 1;
  if( iIdx>0 && 0==(pIdxInfo->idxStr = sqlite3_mprintf("%s", zIdxStr)) ){
    return SQLITE_NOMEM;
  }

  nRow = pRtree->nRowEst / (iIdx + 1);
  pIdxInfo->estimatedCost = (double)6.0 * (double)nRow;
  setEstimatedRows(pIdxInfo, nRow);

  return rc;
}

/*
** Return the N-dimensional volumn of the cell stored in *p.
*/
static RtreeDValue cellArea(Rtree *pRtree, RtreeCell *p){
  RtreeDValue area = (RtreeDValue)1;
  int ii;
  for(ii=0; ii<(pRtree->nDim*2); ii+=2){
    area = (area * (DCOORD(p->aCoord[ii+1]) - DCOORD(p->aCoord[ii])));
  }
  return area;
}

/*
** Return the margin length of cell p. The margin length is the sum
** of the objects size in each dimension.
*/
static RtreeDValue cellMargin(Rtree *pRtree, RtreeCell *p){
  RtreeDValue margin = (RtreeDValue)0;
  int ii;
  for(ii=0; ii<(pRtree->nDim*2); ii+=2){
    margin += (DCOORD(p->aCoord[ii+1]) - DCOORD(p->aCoord[ii]));
  }
  return margin;
}

/*
** Store the union of cells p1 and p2 in p1.
*/
static void cellUnion(Rtree *pRtree, RtreeCell *p1, RtreeCell *p2){
  int ii;
  if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
    for(ii=0; ii<(pRtree->nDim*2); ii+=2){
      p1->aCoord[ii].f = MIN(p1->aCoord[ii].f, p2->aCoord[ii].f);
      p1->aCoord[ii+1].f = MAX(p1->aCoord[ii+1].f, p2->aCoord[ii+1].f);
    }
  }else{
    for(ii=0; ii<(pRtree->nDim*2); ii+=2){
      p1->aCoord[ii].i = MIN(p1->aCoord[ii].i, p2->aCoord[ii].i);
      p1->aCoord[ii+1].i = MAX(p1->aCoord[ii+1].i, p2->aCoord[ii+1].i);
    }
  }
}

/*
** Return true if the area covered by p2 is a subset of the area covered
** by p1. False otherwise.
*/
static int cellContains(Rtree *pRtree, RtreeCell *p1, RtreeCell *p2){
  int ii;
  int isInt = (pRtree->eCoordType==RTREE_COORD_INT32);
  for(ii=0; ii<(pRtree->nDim*2); ii+=2){
    RtreeCoord *a1 = &p1->aCoord[ii];
    RtreeCoord *a2 = &p2->aCoord[ii];
    if( (!isInt && (a2[0].f<a1[0].f || a2[1].f>a1[1].f)) 
     || ( isInt && (a2[0].i<a1[0].i || a2[1].i>a1[1].i)) 
    ){
      return 0;
    }
  }
  return 1;
}

/*
** Return the amount cell p would grow by if it were unioned with pCell.
*/
static RtreeDValue cellGrowth(Rtree *pRtree, RtreeCell *p, RtreeCell *pCell){
  RtreeDValue area;
  RtreeCell cell;
  memcpy(&cell, p, sizeof(RtreeCell));
  area = cellArea(pRtree, &cell);
  cellUnion(pRtree, &cell, pCell);
  return (cellArea(pRtree, &cell)-area);
}

static RtreeDValue cellOverlap(
  Rtree *pRtree, 
  RtreeCell *p, 
  RtreeCell *aCell, 
  int nCell
){
  int ii;
  RtreeDValue overlap = RTREE_ZERO;
  for(ii=0; ii<nCell; ii++){
    int jj;
    RtreeDValue o = (RtreeDValue)1;
    for(jj=0; jj<(pRtree->nDim*2); jj+=2){
      RtreeDValue x1, x2;
      x1 = MAX(DCOORD(p->aCoord[jj]), DCOORD(aCell[ii].aCoord[jj]));
      x2 = MIN(DCOORD(p->aCoord[jj+1]), DCOORD(aCell[ii].aCoord[jj+1]));
      if( x2<x1 ){
        o = (RtreeDValue)0;
        break;
      }else{
        o = o * (x2-x1);
      }
    }
    overlap += o;
  }
  return overlap;
}


/*
** This function implements the ChooseLeaf algorithm from Gutman[84].
** ChooseSubTree in r*tree terminology.
*/
static int ChooseLeaf(
  Rtree *pRtree,               /* Rtree table */
  RtreeCell *pCell,            /* Cell to insert into rtree */
  int iHeight,                 /* Height of sub-tree rooted at pCell */
  RtreeNode **ppLeaf           /* OUT: Selected leaf page */
){
  int rc;
  int ii;
  RtreeNode *pNode;
  rc = nodeAcquire(pRtree, 1, 0, &pNode);

  for(ii=0; rc==SQLITE_OK && ii<(pRtree->iDepth-iHeight); ii++){
    int iCell;
    sqlite3_int64 iBest = 0;

    RtreeDValue fMinGrowth = RTREE_ZERO;
    RtreeDValue fMinArea = RTREE_ZERO;

    int nCell = NCELL(pNode);
    RtreeCell cell;
    RtreeNode *pChild;

    RtreeCell *aCell = 0;

    /* Select the child node which will be enlarged the least if pCell
    ** is inserted into it. Resolve ties by choosing the entry with
    ** the smallest area.
    */
    for(iCell=0; iCell<nCell; iCell++){
      int bBest = 0;
      RtreeDValue growth;
      RtreeDValue area;
      nodeGetCell(pRtree, pNode, iCell, &cell);
      growth = cellGrowth(pRtree, &cell, pCell);
      area = cellArea(pRtree, &cell);
      if( iCell==0||growth<fMinGrowth||(growth==fMinGrowth && area<fMinArea) ){
        bBest = 1;
      }
      if( bBest ){
        fMinGrowth = growth;
        fMinArea = area;
        iBest = cell.iRowid;
      }
    }

    sqlite3_free(aCell);
    rc = nodeAcquire(pRtree, iBest, pNode, &pChild);
    nodeRelease(pRtree, pNode);
    pNode = pChild;
  }

  *ppLeaf = pNode;
  return rc;
}

/*
** A cell with the same content as pCell has just been inserted into
** the node pNode. This function updates the bounding box cells in
** all ancestor elements.
*/
static int AdjustTree(
  Rtree *pRtree,                    /* Rtree table */
  RtreeNode *pNode,                 /* Adjust ancestry of this node. */
  RtreeCell *pCell                  /* This cell was just inserted */
){
  RtreeNode *p = pNode;
  while( p->pParent ){
    RtreeNode *pParent = p->pParent;
    RtreeCell cell;
    int iCell;

    if( nodeParentIndex(pRtree, p, &iCell) ){
      return SQLITE_CORRUPT_VTAB;
    }

    nodeGetCell(pRtree, pParent, iCell, &cell);
    if( !cellContains(pRtree, &cell, pCell) ){
      cellUnion(pRtree, &cell, pCell);
      nodeOverwriteCell(pRtree, pParent, &cell, iCell);
    }
 
    p = pParent;
  }
  return SQLITE_OK;
}

/*
** Write mapping (iRowid->iNode) to the <rtree>_rowid table.
*/
static int rowidWrite(Rtree *pRtree, sqlite3_int64 iRowid, sqlite3_int64 iNode){
  sqlite3_bind_int64(pRtree->pWriteRowid, 1, iRowid);
  sqlite3_bind_int64(pRtree->pWriteRowid, 2, iNode);
  sqlite3_step(pRtree->pWriteRowid);
  return sqlite3_reset(pRtree->pWriteRowid);
}

/*
** Write mapping (iNode->iPar) to the <rtree>_parent table.
*/
static int parentWrite(Rtree *pRtree, sqlite3_int64 iNode, sqlite3_int64 iPar){
  sqlite3_bind_int64(pRtree->pWriteParent, 1, iNode);
  sqlite3_bind_int64(pRtree->pWriteParent, 2, iPar);
  sqlite3_step(pRtree->pWriteParent);
  return sqlite3_reset(pRtree->pWriteParent);
}

static int rtreeInsertCell(Rtree *, RtreeNode *, RtreeCell *, int);


/*
** Arguments aIdx, aDistance and aSpare all point to arrays of size
** nIdx. The aIdx array contains the set of integers from 0 to 
** (nIdx-1) in no particular order. This function sorts the values
** in aIdx according to the indexed values in aDistance. For
** example, assuming the inputs:
**
**   aIdx      = { 0,   1,   2,   3 }
**   aDistance = { 5.0, 2.0, 7.0, 6.0 }
**
** this function sets the aIdx array to contain:
**
**   aIdx      = { 0,   1,   2,   3 }
**
** The aSpare array is used as temporary working space by the
** sorting algorithm.
*/
static void SortByDistance(
  int *aIdx, 
  int nIdx, 
  RtreeDValue *aDistance, 
  int *aSpare
){
  if( nIdx>1 ){
    int iLeft = 0;
    int iRight = 0;

    int nLeft = nIdx/2;
    int nRight = nIdx-nLeft;
    int *aLeft = aIdx;
    int *aRight = &aIdx[nLeft];

    SortByDistance(aLeft, nLeft, aDistance, aSpare);
    SortByDistance(aRight, nRight, aDistance, aSpare);

    memcpy(aSpare, aLeft, sizeof(int)*nLeft);
    aLeft = aSpare;

    while( iLeft<nLeft || iRight<nRight ){
      if( iLeft==nLeft ){
        aIdx[iLeft+iRight] = aRight[iRight];
        iRight++;
      }else if( iRight==nRight ){
        aIdx[iLeft+iRight] = aLeft[iLeft];
        iLeft++;
      }else{
        RtreeDValue fLeft = aDistance[aLeft[iLeft]];
        RtreeDValue fRight = aDistance[aRight[iRight]];
        if( fLeft<fRight ){
          aIdx[iLeft+iRight] = aLeft[iLeft];
          iLeft++;
        }else{
          aIdx[iLeft+iRight] = aRight[iRight];
          iRight++;
        }
      }
    }

#if 0
    /* Check that the sort worked */
    {
      int jj;
      for(jj=1; jj<nIdx; jj++){
        RtreeDValue left = aDistance[aIdx[jj-1]];
        RtreeDValue right = aDistance[aIdx[jj]];
        assert( left<=right );
      }
    }
#endif
  }
}

/*
** Arguments aIdx, aCell and aSpare all point to arrays of size
** nIdx. The aIdx array contains the set of integers from 0 to 
** (nIdx-1) in no particular order. This function sorts the values
** in aIdx according to dimension iDim of the cells in aCell. The
** minimum value of dimension iDim is considered first, the
** maximum used to break ties.
**
** The aSpare array is used as temporary working space by the
** sorting algorithm.
*/
static void SortByDimension(
  Rtree *pRtree,
  int *aIdx, 
  int nIdx, 
  int iDim, 
  RtreeCell *aCell, 
  int *aSpare
){
  if( nIdx>1 ){

    int iLeft = 0;
    int iRight = 0;

    int nLeft = nIdx/2;
    int nRight = nIdx-nLeft;
    int *aLeft = aIdx;
    int *aRight = &aIdx[nLeft];

    SortByDimension(pRtree, aLeft, nLeft, iDim, aCell, aSpare);
    SortByDimension(pRtree, aRight, nRight, iDim, aCell, aSpare);

    memcpy(aSpare, aLeft, sizeof(int)*nLeft);
    aLeft = aSpare;
    while( iLeft<nLeft || iRight<nRight ){
      RtreeDValue xleft1 = DCOORD(aCell[aLeft[iLeft]].aCoord[iDim*2]);
      RtreeDValue xleft2 = DCOORD(aCell[aLeft[iLeft]].aCoord[iDim*2+1]);
      RtreeDValue xright1 = DCOORD(aCell[aRight[iRight]].aCoord[iDim*2]);
      RtreeDValue xright2 = DCOORD(aCell[aRight[iRight]].aCoord[iDim*2+1]);
      if( (iLeft!=nLeft) && ((iRight==nRight)
       || (xleft1<xright1)
       || (xleft1==xright1 && xleft2<xright2)
      )){
        aIdx[iLeft+iRight] = aLeft[iLeft];
        iLeft++;
      }else{
        aIdx[iLeft+iRight] = aRight[iRight];
        iRight++;
      }
    }

#if 0
    /* Check that the sort worked */
    {
      int jj;
      for(jj=1; jj<nIdx; jj++){
        RtreeDValue xleft1 = aCell[aIdx[jj-1]].aCoord[iDim*2];
        RtreeDValue xleft2 = aCell[aIdx[jj-1]].aCoord[iDim*2+1];
        RtreeDValue xright1 = aCell[aIdx[jj]].aCoord[iDim*2];
        RtreeDValue xright2 = aCell[aIdx[jj]].aCoord[iDim*2+1];
        assert( xleft1<=xright1 && (xleft1<xright1 || xleft2<=xright2) );
      }
    }
#endif
  }
}

/*
** Implementation of the R*-tree variant of SplitNode from Beckman[1990].
*/
static int splitNodeStartree(
  Rtree *pRtree,
  RtreeCell *aCell,
  int nCell,
  RtreeNode *pLeft,
  RtreeNode *pRight,
  RtreeCell *pBboxLeft,
  RtreeCell *pBboxRight
){
  int **aaSorted;
  int *aSpare;
  int ii;

  int iBestDim = 0;
  int iBestSplit = 0;
  RtreeDValue fBestMargin = RTREE_ZERO;

  int nByte = (pRtree->nDim+1)*(sizeof(int*)+nCell*sizeof(int));

  aaSorted = (int **)sqlite3_malloc(nByte);
  if( !aaSorted ){
    return SQLITE_NOMEM;
  }

  aSpare = &((int *)&aaSorted[pRtree->nDim])[pRtree->nDim*nCell];
  memset(aaSorted, 0, nByte);
  for(ii=0; ii<pRtree->nDim; ii++){
    int jj;
    aaSorted[ii] = &((int *)&aaSorted[pRtree->nDim])[ii*nCell];
    for(jj=0; jj<nCell; jj++){
      aaSorted[ii][jj] = jj;
    }
    SortByDimension(pRtree, aaSorted[ii], nCell, ii, aCell, aSpare);
  }

  for(ii=0; ii<pRtree->nDim; ii++){
    RtreeDValue margin = RTREE_ZERO;
    RtreeDValue fBestOverlap = RTREE_ZERO;
    RtreeDValue fBestArea = RTREE_ZERO;
    int iBestLeft = 0;
    int nLeft;

    for(
      nLeft=RTREE_MINCELLS(pRtree); 
      nLeft<=(nCell-RTREE_MINCELLS(pRtree)); 
      nLeft++
    ){
      RtreeCell left;
      RtreeCell right;
      int kk;
      RtreeDValue overlap;
      RtreeDValue area;

      memcpy(&left, &aCell[aaSorted[ii][0]], sizeof(RtreeCell));
      memcpy(&right, &aCell[aaSorted[ii][nCell-1]], sizeof(RtreeCell));
      for(kk=1; kk<(nCell-1); kk++){
        if( kk<nLeft ){
          cellUnion(pRtree, &left, &aCell[aaSorted[ii][kk]]);
        }else{
          cellUnion(pRtree, &right, &aCell[aaSorted[ii][kk]]);
        }
      }
      margin += cellMargin(pRtree, &left);
      margin += cellMargin(pRtree, &right);
      overlap = cellOverlap(pRtree, &left, &right, 1);
      area = cellArea(pRtree, &left) + cellArea(pRtree, &right);
      if( (nLeft==RTREE_MINCELLS(pRtree))
       || (overlap<fBestOverlap)
       || (overlap==fBestOverlap && area<fBestArea)
      ){
        iBestLeft = nLeft;
        fBestOverlap = overlap;
        fBestArea = area;
      }
    }

    if( ii==0 || margin<fBestMargin ){
      iBestDim = ii;
      fBestMargin = margin;
      iBestSplit = iBestLeft;
    }
  }

  memcpy(pBboxLeft, &aCell[aaSorted[iBestDim][0]], sizeof(RtreeCell));
  memcpy(pBboxRight, &aCell[aaSorted[iBestDim][iBestSplit]], sizeof(RtreeCell));
  for(ii=0; ii<nCell; ii++){
    RtreeNode *pTarget = (ii<iBestSplit)?pLeft:pRight;
    RtreeCell *pBbox = (ii<iBestSplit)?pBboxLeft:pBboxRight;
    RtreeCell *pCell = &aCell[aaSorted[iBestDim][ii]];
    nodeInsertCell(pRtree, pTarget, pCell);
    cellUnion(pRtree, pBbox, pCell);
  }

  sqlite3_free(aaSorted);
  return SQLITE_OK;
}


static int updateMapping(
  Rtree *pRtree, 
  i64 iRowid, 
  RtreeNode *pNode, 
  int iHeight
){
  int (*xSetMapping)(Rtree *, sqlite3_int64, sqlite3_int64);
  xSetMapping = ((iHeight==0)?rowidWrite:parentWrite);
  if( iHeight>0 ){
    RtreeNode *pChild = nodeHashLookup(pRtree, iRowid);
    if( pChild ){
      nodeRelease(pRtree, pChild->pParent);
      nodeReference(pNode);
      pChild->pParent = pNode;
    }
  }
  return xSetMapping(pRtree, iRowid, pNode->iNode);
}

static int SplitNode(
  Rtree *pRtree,
  RtreeNode *pNode,
  RtreeCell *pCell,
  int iHeight
){
  int i;
  int newCellIsRight = 0;

  int rc = SQLITE_OK;
  int nCell = NCELL(pNode);
  RtreeCell *aCell;
  int *aiUsed;

  RtreeNode *pLeft = 0;
  RtreeNode *pRight = 0;

  RtreeCell leftbbox;
  RtreeCell rightbbox;

  /* Allocate an array and populate it with a copy of pCell and 
  ** all cells from node pLeft. Then zero the original node.
  */
  aCell = sqlite3_malloc((sizeof(RtreeCell)+sizeof(int))*(nCell+1));
  if( !aCell ){
    rc = SQLITE_NOMEM;
    goto splitnode_out;
  }
  aiUsed = (int *)&aCell[nCell+1];
  memset(aiUsed, 0, sizeof(int)*(nCell+1));
  for(i=0; i<nCell; i++){
    nodeGetCell(pRtree, pNode, i, &aCell[i]);
  }
  nodeZero(pRtree, pNode);
  memcpy(&aCell[nCell], pCell, sizeof(RtreeCell));
  nCell++;

  if( pNode->iNode==1 ){
    pRight = nodeNew(pRtree, pNode);
    pLeft = nodeNew(pRtree, pNode);
    pRtree->iDepth++;
    pNode->isDirty = 1;
    writeInt16(pNode->zData, pRtree->iDepth);
  }else{
    pLeft = pNode;
    pRight = nodeNew(pRtree, pLeft->pParent);
    nodeReference(pLeft);
  }

  if( !pLeft || !pRight ){
    rc = SQLITE_NOMEM;
    goto splitnode_out;
  }

  memset(pLeft->zData, 0, pRtree->iNodeSize);
  memset(pRight->zData, 0, pRtree->iNodeSize);

  rc = splitNodeStartree(pRtree, aCell, nCell, pLeft, pRight,
                         &leftbbox, &rightbbox);
  if( rc!=SQLITE_OK ){
    goto splitnode_out;
  }

  /* Ensure both child nodes have node numbers assigned to them by calling
  ** nodeWrite(). Node pRight always needs a node number, as it was created
  ** by nodeNew() above. But node pLeft sometimes already has a node number.
  ** In this case avoid the all to nodeWrite().
  */
  if( SQLITE_OK!=(rc = nodeWrite(pRtree, pRight))
   || (0==pLeft->iNode && SQLITE_OK!=(rc = nodeWrite(pRtree, pLeft)))
  ){
    goto splitnode_out;
  }

  rightbbox.iRowid = pRight->iNode;
  leftbbox.iRowid = pLeft->iNode;

  if( pNode->iNode==1 ){
    rc = rtreeInsertCell(pRtree, pLeft->pParent, &leftbbox, iHeight+1);
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }else{
    RtreeNode *pParent = pLeft->pParent;
    int iCell;
    rc = nodeParentIndex(pRtree, pLeft, &iCell);
    if( rc==SQLITE_OK ){
      nodeOverwriteCell(pRtree, pParent, &leftbbox, iCell);
      rc = AdjustTree(pRtree, pParent, &leftbbox);
    }
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }
  if( (rc = rtreeInsertCell(pRtree, pRight->pParent, &rightbbox, iHeight+1)) ){
    goto splitnode_out;
  }

  for(i=0; i<NCELL(pRight); i++){
    i64 iRowid = nodeGetRowid(pRtree, pRight, i);
    rc = updateMapping(pRtree, iRowid, pRight, iHeight);
    if( iRowid==pCell->iRowid ){
      newCellIsRight = 1;
    }
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }
  if( pNode->iNode==1 ){
    for(i=0; i<NCELL(pLeft); i++){
      i64 iRowid = nodeGetRowid(pRtree, pLeft, i);
      rc = updateMapping(pRtree, iRowid, pLeft, iHeight);
      if( rc!=SQLITE_OK ){
        goto splitnode_out;
      }
    }
  }else if( newCellIsRight==0 ){
    rc = updateMapping(pRtree, pCell->iRowid, pLeft, iHeight);
  }

  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pRight);
    pRight = 0;
  }
  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pLeft);
    pLeft = 0;
  }

splitnode_out:
  nodeRelease(pRtree, pRight);
  nodeRelease(pRtree, pLeft);
  sqlite3_free(aCell);
  return rc;
}

/*
** If node pLeaf is not the root of the r-tree and its pParent pointer is 
** still NULL, load all ancestor nodes of pLeaf into memory and populate
** the pLeaf->pParent chain all the way up to the root node.
**
** This operation is required when a row is deleted (or updated - an update
** is implemented as a delete followed by an insert). SQLite provides the
** rowid of the row to delete, which can be used to find the leaf on which
** the entry resides (argument pLeaf). Once the leaf is located, this 
** function is called to determine its ancestry.
*/
static int fixLeafParent(Rtree *pRtree, RtreeNode *pLeaf){
  int rc = SQLITE_OK;
  RtreeNode *pChild = pLeaf;
  while( rc==SQLITE_OK && pChild->iNode!=1 && pChild->pParent==0 ){
    int rc2 = SQLITE_OK;          /* sqlite3_reset() return code */
    sqlite3_bind_int64(pRtree->pReadParent, 1, pChild->iNode);
    rc = sqlite3_step(pRtree->pReadParent);
    if( rc==SQLITE_ROW ){
      RtreeNode *pTest;           /* Used to test for reference loops */
      i64 iNode;                  /* Node number of parent node */

      /* Before setting pChild->pParent, test that we are not creating a
      ** loop of references (as we would if, say, pChild==pParent). We don't
      ** want to do this as it leads to a memory leak when trying to delete
      ** the referenced counted node structures.
      */
      iNode = sqlite3_column_int64(pRtree->pReadParent, 0);
      for(pTest=pLeaf; pTest && pTest->iNode!=iNode; pTest=pTest->pParent);
      if( !pTest ){
        rc2 = nodeAcquire(pRtree, iNode, 0, &pChild->pParent);
      }
    }
    rc = sqlite3_reset(pRtree->pReadParent);
    if( rc==SQLITE_OK ) rc = rc2;
    if( rc==SQLITE_OK && !pChild->pParent ) rc = SQLITE_CORRUPT_VTAB;
    pChild = pChild->pParent;
  }
  return rc;
}

static int deleteCell(Rtree *, RtreeNode *, int, int);

static int removeNode(Rtree *pRtree, RtreeNode *pNode, int iHeight){
  int rc;
  int rc2;
  RtreeNode *pParent = 0;
  int iCell;

  assert( pNode->nRef==1 );

  /* Remove the entry in the parent cell. */
  rc = nodeParentIndex(pRtree, pNode, &iCell);
  if( rc==SQLITE_OK ){
    pParent = pNode->pParent;
    pNode->pParent = 0;
    rc = deleteCell(pRtree, pParent, iCell, iHeight+1);
  }
  rc2 = nodeRelease(pRtree, pParent);
  if( rc==SQLITE_OK ){
    rc = rc2;
  }
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* Remove the xxx_node entry. */
  sqlite3_bind_int64(pRtree->pDeleteNode, 1, pNode->iNode);
  sqlite3_step(pRtree->pDeleteNode);
  if( SQLITE_OK!=(rc = sqlite3_reset(pRtree->pDeleteNode)) ){
    return rc;
  }

  /* Remove the xxx_parent entry. */
  sqlite3_bind_int64(pRtree->pDeleteParent, 1, pNode->iNode);
  sqlite3_step(pRtree->pDeleteParent);
  if( SQLITE_OK!=(rc = sqlite3_reset(pRtree->pDeleteParent)) ){
    return rc;
  }
  
  /* Remove the node from the in-memory hash table and link it into
  ** the Rtree.pDeleted list. Its contents will be re-inserted later on.
  */
  nodeHashDelete(pRtree, pNode);
  pNode->iNode = iHeight;
  pNode->pNext = pRtree->pDeleted;
  pNode->nRef++;
  pRtree->pDeleted = pNode;

  return SQLITE_OK;
}

static int fixBoundingBox(Rtree *pRtree, RtreeNode *pNode){
  RtreeNode *pParent = pNode->pParent;
  int rc = SQLITE_OK; 
  if( pParent ){
    int ii; 
    int nCell = NCELL(pNode);
    RtreeCell box;                            /* Bounding box for pNode */
    nodeGetCell(pRtree, pNode, 0, &box);
    for(ii=1; ii<nCell; ii++){
      RtreeCell cell;
      nodeGetCell(pRtree, pNode, ii, &cell);
      cellUnion(pRtree, &box, &cell);
    }
    box.iRowid = pNode->iNode;
    rc = nodeParentIndex(pRtree, pNode, &ii);
    if( rc==SQLITE_OK ){
      nodeOverwriteCell(pRtree, pParent, &box, ii);
      rc = fixBoundingBox(pRtree, pParent);
    }
  }
  return rc;
}

/*
** Delete the cell at index iCell of node pNode. After removing the
** cell, adjust the r-tree data structure if required.
*/
static int deleteCell(Rtree *pRtree, RtreeNode *pNode, int iCell, int iHeight){
  RtreeNode *pParent;
  int rc;

  if( SQLITE_OK!=(rc = fixLeafParent(pRtree, pNode)) ){
    return rc;
  }

  /* Remove the cell from the node. This call just moves bytes around
  ** the in-memory node image, so it cannot fail.
  */
  nodeDeleteCell(pRtree, pNode, iCell);

  /* If the node is not the tree root and now has less than the minimum
  ** number of cells, remove it from the tree. Otherwise, update the
  ** cell in the parent node so that it tightly contains the updated
  ** node.
  */
  pParent = pNode->pParent;
  assert( pParent || pNode->iNode==1 );
  if( pParent ){
    if( NCELL(pNode)<RTREE_MINCELLS(pRtree) ){
      rc = removeNode(pRtree, pNode, iHeight);
    }else{
      rc = fixBoundingBox(pRtree, pNode);
    }
  }

  return rc;
}

static int Reinsert(
  Rtree *pRtree, 
  RtreeNode *pNode, 
  RtreeCell *pCell, 
  int iHeight
){
  int *aOrder;
  int *aSpare;
  RtreeCell *aCell;
  RtreeDValue *aDistance;
  int nCell;
  RtreeDValue aCenterCoord[RTREE_MAX_DIMENSIONS];
  int iDim;
  int ii;
  int rc = SQLITE_OK;
  int n;

  memset(aCenterCoord, 0, sizeof(RtreeDValue)*RTREE_MAX_DIMENSIONS);

  nCell = NCELL(pNode)+1;
  n = (nCell+1)&(~1);

  /* Allocate the buffers used by this operation. The allocation is
  ** relinquished before this function returns.
  */
  aCell = (RtreeCell *)sqlite3_malloc(n * (
    sizeof(RtreeCell)     +         /* aCell array */
    sizeof(int)           +         /* aOrder array */
    sizeof(int)           +         /* aSpare array */
    sizeof(RtreeDValue)             /* aDistance array */
  ));
  if( !aCell ){
    return SQLITE_NOMEM;
  }
  aOrder    = (int *)&aCell[n];
  aSpare    = (int *)&aOrder[n];
  aDistance = (RtreeDValue *)&aSpare[n];

  for(ii=0; ii<nCell; ii++){
    if( ii==(nCell-1) ){
      memcpy(&aCell[ii], pCell, sizeof(RtreeCell));
    }else{
      nodeGetCell(pRtree, pNode, ii, &aCell[ii]);
    }
    aOrder[ii] = ii;
    for(iDim=0; iDim<pRtree->nDim; iDim++){
      aCenterCoord[iDim] += DCOORD(aCell[ii].aCoord[iDim*2]);
      aCenterCoord[iDim] += DCOORD(aCell[ii].aCoord[iDim*2+1]);
    }
  }
  for(iDim=0; iDim<pRtree->nDim; iDim++){
    aCenterCoord[iDim] = (aCenterCoord[iDim]/(nCell*(RtreeDValue)2));
  }

  for(ii=0; ii<nCell; ii++){
    aDistance[ii] = RTREE_ZERO;
    for(iDim=0; iDim<pRtree->nDim; iDim++){
      RtreeDValue coord = (DCOORD(aCell[ii].aCoord[iDim*2+1]) - 
                               DCOORD(aCell[ii].aCoord[iDim*2]));
      aDistance[ii] += (coord-aCenterCoord[iDim])*(coord-aCenterCoord[iDim]);
    }
  }

  SortByDistance(aOrder, nCell, aDistance, aSpare);
  nodeZero(pRtree, pNode);

  for(ii=0; rc==SQLITE_OK && ii<(nCell-(RTREE_MINCELLS(pRtree)+1)); ii++){
    RtreeCell *p = &aCell[aOrder[ii]];
    nodeInsertCell(pRtree, pNode, p);
    if( p->iRowid==pCell->iRowid ){
      if( iHeight==0 ){
        rc = rowidWrite(pRtree, p->iRowid, pNode->iNode);
      }else{
        rc = parentWrite(pRtree, p->iRowid, pNode->iNode);
      }
    }
  }
  if( rc==SQLITE_OK ){
    rc = fixBoundingBox(pRtree, pNode);
  }
  for(; rc==SQLITE_OK && ii<nCell; ii++){
    /* Find a node to store this cell in. pNode->iNode currently contains
    ** the height of the sub-tree headed by the cell.
    */
    RtreeNode *pInsert;
    RtreeCell *p = &aCell[aOrder[ii]];
    rc = ChooseLeaf(pRtree, p, iHeight, &pInsert);
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pInsert, p, iHeight);
      rc2 = nodeRelease(pRtree, pInsert);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }

  sqlite3_free(aCell);
  return rc;
}

/*
** Insert cell pCell into node pNode. Node pNode is the head of a 
** subtree iHeight high (leaf nodes have iHeight==0).
*/
static int rtreeInsertCell(
  Rtree *pRtree,
  RtreeNode *pNode,
  RtreeCell *pCell,
  int iHeight
){
  int rc = SQLITE_OK;
  if( iHeight>0 ){
    RtreeNode *pChild = nodeHashLookup(pRtree, pCell->iRowid);
    if( pChild ){
      nodeRelease(pRtree, pChild->pParent);
      nodeReference(pNode);
      pChild->pParent = pNode;
    }
  }
  if( nodeInsertCell(pRtree, pNode, pCell) ){
    if( iHeight<=pRtree->iReinsertHeight || pNode->iNode==1){
      rc = SplitNode(pRtree, pNode, pCell, iHeight);
    }else{
      pRtree->iReinsertHeight = iHeight;
      rc = Reinsert(pRtree, pNode, pCell, iHeight);
    }
  }else{
    rc = AdjustTree(pRtree, pNode, pCell);
    if( rc==SQLITE_OK ){
      if( iHeight==0 ){
        rc = rowidWrite(pRtree, pCell->iRowid, pNode->iNode);
      }else{
        rc = parentWrite(pRtree, pCell->iRowid, pNode->iNode);
      }
    }
  }
  return rc;
}

static int reinsertNodeContent(Rtree *pRtree, RtreeNode *pNode){
  int ii;
  int rc = SQLITE_OK;
  int nCell = NCELL(pNode);

  for(ii=0; rc==SQLITE_OK && ii<nCell; ii++){
    RtreeNode *pInsert;
    RtreeCell cell;
    nodeGetCell(pRtree, pNode, ii, &cell);

    /* Find a node to store this cell in. pNode->iNode currently contains
    ** the height of the sub-tree headed by the cell.
    */
    rc = ChooseLeaf(pRtree, &cell, (int)pNode->iNode, &pInsert);
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pInsert, &cell, (int)pNode->iNode);
      rc2 = nodeRelease(pRtree, pInsert);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  return rc;
}

/*
** Select a currently unused rowid for a new r-tree record.
*/
static int newRowid(Rtree *pRtree, i64 *piRowid){
  int rc;
  sqlite3_bind_null(pRtree->pWriteRowid, 1);
  sqlite3_bind_null(pRtree->pWriteRowid, 2);
  sqlite3_step(pRtree->pWriteRowid);
  rc = sqlite3_reset(pRtree->pWriteRowid);
  *piRowid = sqlite3_last_insert_rowid(pRtree->db);
  return rc;
}

/*
** Remove the entry with rowid=iDelete from the r-tree structure.
*/
static int rtreeDeleteRowid(Rtree *pRtree, sqlite3_int64 iDelete){
  int rc;                         /* Return code */
  RtreeNode *pLeaf = 0;           /* Leaf node containing record iDelete */
  int iCell;                      /* Index of iDelete cell in pLeaf */
  RtreeNode *pRoot;               /* Root node of rtree structure */


  /* Obtain a reference to the root node to initialize Rtree.iDepth */
  rc = nodeAcquire(pRtree, 1, 0, &pRoot);

  /* Obtain a reference to the leaf node that contains the entry 
  ** about to be deleted. 
  */
  if( rc==SQLITE_OK ){
    rc = findLeafNode(pRtree, iDelete, &pLeaf, 0);
  }

  /* Delete the cell in question from the leaf node. */
  if( rc==SQLITE_OK ){
    int rc2;
    rc = nodeRowidIndex(pRtree, pLeaf, iDelete, &iCell);
    if( rc==SQLITE_OK ){
      rc = deleteCell(pRtree, pLeaf, iCell, 0);
    }
    rc2 = nodeRelease(pRtree, pLeaf);
    if( rc==SQLITE_OK ){
      rc = rc2;
    }
  }

  /* Delete the corresponding entry in the <rtree>_rowid table. */
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pRtree->pDeleteRowid, 1, iDelete);
    sqlite3_step(pRtree->pDeleteRowid);
    rc = sqlite3_reset(pRtree->pDeleteRowid);
  }

  /* Check if the root node now has exactly one child. If so, remove
  ** it, schedule the contents of the child for reinsertion and 
  ** reduce the tree height by one.
  **
  ** This is equivalent to copying the contents of the child into
  ** the root node (the operation that Gutman's paper says to perform 
  ** in this scenario).
  */
  if( rc==SQLITE_OK && pRtree->iDepth>0 && NCELL(pRoot)==1 ){
    int rc2;
    RtreeNode *pChild;
    i64 iChild = nodeGetRowid(pRtree, pRoot, 0);
    rc = nodeAcquire(pRtree, iChild, pRoot, &pChild);
    if( rc==SQLITE_OK ){
      rc = removeNode(pRtree, pChild, pRtree->iDepth-1);
    }
    rc2 = nodeRelease(pRtree, pChild);
    if( rc==SQLITE_OK ) rc = rc2;
    if( rc==SQLITE_OK ){
      pRtree->iDepth--;
      writeInt16(pRoot->zData, pRtree->iDepth);
      pRoot->isDirty = 1;
    }
  }

  /* Re-insert the contents of any underfull nodes removed from the tree. */
  for(pLeaf=pRtree->pDeleted; pLeaf; pLeaf=pRtree->pDeleted){
    if( rc==SQLITE_OK ){
      rc = reinsertNodeContent(pRtree, pLeaf);
    }
    pRtree->pDeleted = pLeaf->pNext;
    sqlite3_free(pLeaf);
  }

  /* Release the reference to the root node. */
  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pRoot);
  }else{
    nodeRelease(pRtree, pRoot);
  }

  return rc;
}

/*
** Rounding constants for float->double conversion.
*/
#define RNDTOWARDS  (1.0 - 1.0/8388608.0)  /* Round towards zero */
#define RNDAWAY     (1.0 + 1.0/8388608.0)  /* Round away from zero */

#if !defined(SQLITE_RTREE_INT_ONLY)
/*
** Convert an sqlite3_value into an RtreeValue (presumably a float)
** while taking care to round toward negative or positive, respectively.
*/
static RtreeValue rtreeValueDown(sqlite3_value *v){
  double d = sqlite3_value_double(v);
  float f = (float)d;
  if( f>d ){
    f = (float)(d*(d<0 ? RNDAWAY : RNDTOWARDS));
  }
  return f;
}
static RtreeValue rtreeValueUp(sqlite3_value *v){
  double d = sqlite3_value_double(v);
  float f = (float)d;
  if( f<d ){
    f = (float)(d*(d<0 ? RNDTOWARDS : RNDAWAY));
  }
  return f;
}
#endif /* !defined(SQLITE_RTREE_INT_ONLY) */


/*
** The xUpdate method for rtree module virtual tables.
*/
static int rtreeUpdate(
  sqlite3_vtab *pVtab, 
  int nData, 
  sqlite3_value **azData, 
  sqlite_int64 *pRowid
){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_OK;
  RtreeCell cell;                 /* New cell to insert if nData>1 */
  int bHaveRowid = 0;             /* Set to 1 after new rowid is determined */

  rtreeReference(pRtree);
  assert(nData>=1);

  cell.iRowid = 0;  /* Used only to suppress a compiler warning */

  /* Constraint handling. A write operation on an r-tree table may return
  ** SQLITE_CONSTRAINT for two reasons:
  **
  **   1. A duplicate rowid value, or
  **   2. The supplied data violates the "x2>=x1" constraint.
  **
  ** In the first case, if the conflict-handling mode is REPLACE, then
  ** the conflicting row can be removed before proceeding. In the second
  ** case, SQLITE_CONSTRAINT must be returned regardless of the
  ** conflict-handling mode specified by the user.
  */
  if( nData>1 ){
    int ii;

    /* Populate the cell.aCoord[] array. The first coordinate is azData[3].
    **
    ** NB: nData can only be less than nDim*2+3 if the rtree is mis-declared
    ** with "column" that are interpreted as table constraints.
    ** Example:  CREATE VIRTUAL TABLE bad USING rtree(x,y,CHECK(y>5));
    ** This problem was discovered after years of use, so we silently ignore
    ** these kinds of misdeclared tables to avoid breaking any legacy.
    */
    assert( nData<=(pRtree->nDim*2 + 3) );

#ifndef SQLITE_RTREE_INT_ONLY
    if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
      for(ii=0; ii<nData-4; ii+=2){
        cell.aCoord[ii].f = rtreeValueDown(azData[ii+3]);
        cell.aCoord[ii+1].f = rtreeValueUp(azData[ii+4]);
        if( cell.aCoord[ii].f>cell.aCoord[ii+1].f ){
          rc = SQLITE_CONSTRAINT;
          goto constraint;
        }
      }
    }else
#endif
    {
      for(ii=0; ii<nData-4; ii+=2){
        cell.aCoord[ii].i = sqlite3_value_int(azData[ii+3]);
        cell.aCoord[ii+1].i = sqlite3_value_int(azData[ii+4]);
        if( cell.aCoord[ii].i>cell.aCoord[ii+1].i ){
          rc = SQLITE_CONSTRAINT;
          goto constraint;
        }
      }
    }

    /* If a rowid value was supplied, check if it is already present in 
    ** the table. If so, the constraint has failed. */
    if( sqlite3_value_type(azData[2])!=SQLITE_NULL ){
      cell.iRowid = sqlite3_value_int64(azData[2]);
      if( sqlite3_value_type(azData[0])==SQLITE_NULL
       || sqlite3_value_int64(azData[0])!=cell.iRowid
      ){
        int steprc;
        sqlite3_bind_int64(pRtree->pReadRowid, 1, cell.iRowid);
        steprc = sqlite3_step(pRtree->pReadRowid);
        rc = sqlite3_reset(pRtree->pReadRowid);
        if( SQLITE_ROW==steprc ){
          if( sqlite3_vtab_on_conflict(pRtree->db)==SQLITE_REPLACE ){
            rc = rtreeDeleteRowid(pRtree, cell.iRowid);
          }else{
            rc = SQLITE_CONSTRAINT;
            goto constraint;
          }
        }
      }
      bHaveRowid = 1;
    }
  }

  /* If azData[0] is not an SQL NULL value, it is the rowid of a
  ** record to delete from the r-tree table. The following block does
  ** just that.
  */
  if( sqlite3_value_type(azData[0])!=SQLITE_NULL ){
    rc = rtreeDeleteRowid(pRtree, sqlite3_value_int64(azData[0]));
  }

  /* If the azData[] array contains more than one element, elements
  ** (azData[2]..azData[argc-1]) contain a new record to insert into
  ** the r-tree structure.
  */
  if( rc==SQLITE_OK && nData>1 ){
    /* Insert the new record into the r-tree */
    RtreeNode *pLeaf = 0;

    /* Figure out the rowid of the new row. */
    if( bHaveRowid==0 ){
      rc = newRowid(pRtree, &cell.iRowid);
    }
    *pRowid = cell.iRowid;

    if( rc==SQLITE_OK ){
      rc = ChooseLeaf(pRtree, &cell, 0, &pLeaf);
    }
    if( rc==SQLITE_OK ){
      int rc2;
      pRtree->iReinsertHeight = -1;
      rc = rtreeInsertCell(pRtree, pLeaf, &cell, 0);
      rc2 = nodeRelease(pRtree, pLeaf);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }

constraint:
  rtreeRelease(pRtree);
  return rc;
}

/*
** The xRename method for rtree module virtual tables.
*/
static int rtreeRename(sqlite3_vtab *pVtab, const char *zNewName){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_NOMEM;
  char *zSql = sqlite3_mprintf(
    "ALTER TABLE %Q.'%q_node'   RENAME TO \"%w_node\";"
    "ALTER TABLE %Q.'%q_parent' RENAME TO \"%w_parent\";"
    "ALTER TABLE %Q.'%q_rowid'  RENAME TO \"%w_rowid\";"
    , pRtree->zDb, pRtree->zName, zNewName 
    , pRtree->zDb, pRtree->zName, zNewName 
    , pRtree->zDb, pRtree->zName, zNewName
  );
  if( zSql ){
    rc = sqlite3_exec(pRtree->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
  return rc;
}

/*
** This function populates the pRtree->nRowEst variable with an estimate
** of the number of rows in the virtual table. If possible, this is based
** on sqlite_stat1 data. Otherwise, use RTREE_DEFAULT_ROWEST.
*/
static int rtreeQueryStat1(sqlite3 *db, Rtree *pRtree){
  const char *zFmt = "SELECT stat FROM %Q.sqlite_stat1 WHERE tbl = '%q_rowid'";
  char *zSql;
  sqlite3_stmt *p;
  int rc;
  i64 nRow = 0;

  zSql = sqlite3_mprintf(zFmt, pRtree->zDb, pRtree->zName);
  if( zSql==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_prepare_v2(db, zSql, -1, &p, 0);
    if( rc==SQLITE_OK ){
      if( sqlite3_step(p)==SQLITE_ROW ) nRow = sqlite3_column_int64(p, 0);
      rc = sqlite3_finalize(p);
    }else if( rc!=SQLITE_NOMEM ){
      rc = SQLITE_OK;
    }

    if( rc==SQLITE_OK ){
      if( nRow==0 ){
        pRtree->nRowEst = RTREE_DEFAULT_ROWEST;
      }else{
        pRtree->nRowEst = MAX(nRow, RTREE_MIN_ROWEST);
      }
    }
    sqlite3_free(zSql);
  }

  return rc;
}

static sqlite3_module rtreeModule = {
  0,                          /* iVersion */
  rtreeCreate,                /* xCreate - create a table */
  rtreeConnect,               /* xConnect - connect to an existing table */
  rtreeBestIndex,             /* xBestIndex - Determine search strategy */
  rtreeDisconnect,            /* xDisconnect - Disconnect from a table */
  rtreeDestroy,               /* xDestroy - Drop a table */
  rtreeOpen,                  /* xOpen - open a cursor */
  rtreeClose,                 /* xClose - close a cursor */
  rtreeFilter,                /* xFilter - configure scan constraints */
  rtreeNext,                  /* xNext - advance a cursor */
  rtreeEof,                   /* xEof */
  rtreeColumn,                /* xColumn - read data */
  rtreeRowid,                 /* xRowid - read data */
  rtreeUpdate,                /* xUpdate - write data */
  0,                          /* xBegin - begin transaction */
  0,                          /* xSync - sync transaction */
  0,                          /* xCommit - commit transaction */
  0,                          /* xRollback - rollback transaction */
  0,                          /* xFindFunction - function overloading */
  rtreeRename,                /* xRename - rename the table */
  0,                          /* xSavepoint */
  0,                          /* xRelease */
  0                           /* xRollbackTo */
};

static int rtreeSqlInit(
  Rtree *pRtree, 
  sqlite3 *db, 
  const char *zDb, 
  const char *zPrefix, 
  int isCreate
){
  int rc = SQLITE_OK;

  #define N_STATEMENT 9
  static const char *azSql[N_STATEMENT] = {
    /* Read and write the xxx_node table */
    "SELECT data FROM '%q'.'%q_node' WHERE nodeno = :1",
    "INSERT OR REPLACE INTO '%q'.'%q_node' VALUES(:1, :2)",
    "DELETE FROM '%q'.'%q_node' WHERE nodeno = :1",

    /* Read and write the xxx_rowid table */
    "SELECT nodeno FROM '%q'.'%q_rowid' WHERE rowid = :1",
    "INSERT OR REPLACE INTO '%q'.'%q_rowid' VALUES(:1, :2)",
    "DELETE FROM '%q'.'%q_rowid' WHERE rowid = :1",

    /* Read and write the xxx_parent table */
    "SELECT parentnode FROM '%q'.'%q_parent' WHERE nodeno = :1",
    "INSERT OR REPLACE INTO '%q'.'%q_parent' VALUES(:1, :2)",
    "DELETE FROM '%q'.'%q_parent' WHERE nodeno = :1"
  };
  sqlite3_stmt **appStmt[N_STATEMENT];
  int i;

  pRtree->db = db;

  if( isCreate ){
    char *zCreate = sqlite3_mprintf(
"CREATE TABLE \"%w\".\"%w_node\"(nodeno INTEGER PRIMARY KEY, data BLOB);"
"CREATE TABLE \"%w\".\"%w_rowid\"(rowid INTEGER PRIMARY KEY, nodeno INTEGER);"
"CREATE TABLE \"%w\".\"%w_parent\"(nodeno INTEGER PRIMARY KEY,"
                                  " parentnode INTEGER);"
"INSERT INTO '%q'.'%q_node' VALUES(1, zeroblob(%d))",
      zDb, zPrefix, zDb, zPrefix, zDb, zPrefix, zDb, zPrefix, pRtree->iNodeSize
    );
    if( !zCreate ){
      return SQLITE_NOMEM;
    }
    rc = sqlite3_exec(db, zCreate, 0, 0, 0);
    sqlite3_free(zCreate);
    if( rc!=SQLITE_OK ){
      return rc;
    }
  }

  appStmt[0] = &pRtree->pReadNode;
  appStmt[1] = &pRtree->pWriteNode;
  appStmt[2] = &pRtree->pDeleteNode;
  appStmt[3] = &pRtree->pReadRowid;
  appStmt[4] = &pRtree->pWriteRowid;
  appStmt[5] = &pRtree->pDeleteRowid;
  appStmt[6] = &pRtree->pReadParent;
  appStmt[7] = &pRtree->pWriteParent;
  appStmt[8] = &pRtree->pDeleteParent;

  rc = rtreeQueryStat1(db, pRtree);
  for(i=0; i<N_STATEMENT && rc==SQLITE_OK; i++){
    char *zSql = sqlite3_mprintf(azSql[i], zDb, zPrefix);
    if( zSql ){
      rc = sqlite3_prepare_v2(db, zSql, -1, appStmt[i], 0); 
    }else{
      rc = SQLITE_NOMEM;
    }
    sqlite3_free(zSql);
  }

  return rc;
}

/*
** The second argument to this function contains the text of an SQL statement
** that returns a single integer value. The statement is compiled and executed
** using database connection db. If successful, the integer value returned
** is written to *piVal and SQLITE_OK returned. Otherwise, an SQLite error
** code is returned and the value of *piVal after returning is not defined.
*/
static int getIntFromStmt(sqlite3 *db, const char *zSql, int *piVal){
  int rc = SQLITE_NOMEM;
  if( zSql ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc==SQLITE_OK ){
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        *piVal = sqlite3_column_int(pStmt, 0);
      }
      rc = sqlite3_finalize(pStmt);
    }
  }
  return rc;
}

/*
** This function is called from within the xConnect() or xCreate() method to
** determine the node-size used by the rtree table being created or connected
** to. If successful, pRtree->iNodeSize is populated and SQLITE_OK returned.
** Otherwise, an SQLite error code is returned.
**
** If this function is being called as part of an xConnect(), then the rtree
** table already exists. In this case the node-size is determined by inspecting
** the root node of the tree.
**
** Otherwise, for an xCreate(), use 64 bytes less than the database page-size. 
** This ensures that each node is stored on a single database page. If the 
** database page-size is so large that more than RTREE_MAXCELLS entries 
** would fit in a single node, use a smaller node-size.
*/
static int getNodeSize(
  sqlite3 *db,                    /* Database handle */
  Rtree *pRtree,                  /* Rtree handle */
  int isCreate,                   /* True for xCreate, false for xConnect */
  char **pzErr                    /* OUT: Error message, if any */
){
  int rc;
  char *zSql;
  if( isCreate ){
    int iPageSize = 0;
    zSql = sqlite3_mprintf("PRAGMA %Q.page_size", pRtree->zDb);
    rc = getIntFromStmt(db, zSql, &iPageSize);
    if( rc==SQLITE_OK ){
      pRtree->iNodeSize = iPageSize-64;
      if( (4+pRtree->nBytesPerCell*RTREE_MAXCELLS)<pRtree->iNodeSize ){
        pRtree->iNodeSize = 4+pRtree->nBytesPerCell*RTREE_MAXCELLS;
      }
    }else{
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }else{
    zSql = sqlite3_mprintf(
        "SELECT length(data) FROM '%q'.'%q_node' WHERE nodeno = 1",
        pRtree->zDb, pRtree->zName
    );
    rc = getIntFromStmt(db, zSql, &pRtree->iNodeSize);
    if( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  sqlite3_free(zSql);
  return rc;
}

/* 
** This function is the implementation of both the xConnect and xCreate
** methods of the r-tree virtual table.
**
**   argv[0]   -> module name
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> column names...
*/
static int rtreeInit(
  sqlite3 *db,                        /* Database connection */
  void *pAux,                         /* One of the RTREE_COORD_* constants */
  int argc, const char *const*argv,   /* Parameters to CREATE TABLE statement */
  sqlite3_vtab **ppVtab,              /* OUT: New virtual table */
  char **pzErr,                       /* OUT: Error message, if any */
  int isCreate                        /* True for xCreate, false for xConnect */
){
  int rc = SQLITE_OK;
  Rtree *pRtree;
  int nDb;              /* Length of string argv[1] */
  int nName;            /* Length of string argv[2] */
  int eCoordType = (pAux ? RTREE_COORD_INT32 : RTREE_COORD_REAL32);

  const char *aErrMsg[] = {
    0,                                                    /* 0 */
    "Wrong number of columns for an rtree table",         /* 1 */
    "Too few columns for an rtree table",                 /* 2 */
    "Too many columns for an rtree table"                 /* 3 */
  };

  int iErr = (argc<6) ? 2 : argc>(RTREE_MAX_DIMENSIONS*2+4) ? 3 : argc%2;
  if( aErrMsg[iErr] ){
    *pzErr = sqlite3_mprintf("%s", aErrMsg[iErr]);
    return SQLITE_ERROR;
  }

  sqlite3_vtab_config(db, SQLITE_VTAB_CONSTRAINT_SUPPORT, 1);

  /* Allocate the sqlite3_vtab structure */
  nDb = (int)strlen(argv[1]);
  nName = (int)strlen(argv[2]);
  pRtree = (Rtree *)sqlite3_malloc(sizeof(Rtree)+nDb+nName+2);
  if( !pRtree ){
    return SQLITE_NOMEM;
  }
  memset(pRtree, 0, sizeof(Rtree)+nDb+nName+2);
  pRtree->nBusy = 1;
  pRtree->base.pModule = &rtreeModule;
  pRtree->zDb = (char *)&pRtree[1];
  pRtree->zName = &pRtree->zDb[nDb+1];
  pRtree->nDim = (argc-4)/2;
  pRtree->nBytesPerCell = 8 + pRtree->nDim*4*2;
  pRtree->eCoordType = eCoordType;
  memcpy(pRtree->zDb, argv[1], nDb);
  memcpy(pRtree->zName, argv[2], nName);

  /* Figure out the node size to use. */
  rc = getNodeSize(db, pRtree, isCreate, pzErr);

  /* Create/Connect to the underlying relational database schema. If
  ** that is successful, call sqlite3_declare_vtab() to configure
  ** the r-tree table schema.
  */
  if( rc==SQLITE_OK ){
    if( (rc = rtreeSqlInit(pRtree, db, argv[1], argv[2], isCreate)) ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }else{
      char *zSql = sqlite3_mprintf("CREATE TABLE x(%s", argv[3]);
      char *zTmp;
      int ii;
      for(ii=4; zSql && ii<argc; ii++){
        zTmp = zSql;
        zSql = sqlite3_mprintf("%s, %s", zTmp, argv[ii]);
        sqlite3_free(zTmp);
      }
      if( zSql ){
        zTmp = zSql;
        zSql = sqlite3_mprintf("%s);", zTmp);
        sqlite3_free(zTmp);
      }
      if( !zSql ){
        rc = SQLITE_NOMEM;
      }else if( SQLITE_OK!=(rc = sqlite3_declare_vtab(db, zSql)) ){
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
      }
      sqlite3_free(zSql);
    }
  }

  if( rc==SQLITE_OK ){
    *ppVtab = (sqlite3_vtab *)pRtree;
  }else{
    assert( *ppVtab==0 );
    assert( pRtree->nBusy==1 );
    rtreeRelease(pRtree);
  }
  return rc;
}


/*
** Implementation of a scalar function that decodes r-tree nodes to
** human readable strings. This can be used for debugging and analysis.
**
** The scalar function takes two arguments: (1) the number of dimensions
** to the rtree (between 1 and 5, inclusive) and (2) a blob of data containing
** an r-tree node.  For a two-dimensional r-tree structure called "rt", to
** deserialize all nodes, a statement like:
**
**   SELECT rtreenode(2, data) FROM rt_node;
**
** The human readable string takes the form of a Tcl list with one
** entry for each cell in the r-tree node. Each entry is itself a
** list, containing the 8-byte rowid/pageno followed by the 
** <num-dimension>*2 coordinates.
*/
static void rtreenode(sqlite3_context *ctx, int nArg, sqlite3_value **apArg){
  char *zText = 0;
  RtreeNode node;
  Rtree tree;
  int ii;

  UNUSED_PARAMETER(nArg);
  memset(&node, 0, sizeof(RtreeNode));
  memset(&tree, 0, sizeof(Rtree));
  tree.nDim = sqlite3_value_int(apArg[0]);
  tree.nBytesPerCell = 8 + 8 * tree.nDim;
  node.zData = (u8 *)sqlite3_value_blob(apArg[1]);

  for(ii=0; ii<NCELL(&node); ii++){
    char zCell[512];
    int nCell = 0;
    RtreeCell cell;
    int jj;

    nodeGetCell(&tree, &node, ii, &cell);
    sqlite3_snprintf(512-nCell,&zCell[nCell],"%lld", cell.iRowid);
    nCell = (int)strlen(zCell);
    for(jj=0; jj<tree.nDim*2; jj++){
#ifndef SQLITE_RTREE_INT_ONLY
      sqlite3_snprintf(512-nCell,&zCell[nCell], " %g",
                       (double)cell.aCoord[jj].f);
#else
      sqlite3_snprintf(512-nCell,&zCell[nCell], " %d",
                       cell.aCoord[jj].i);
#endif
      nCell = (int)strlen(zCell);
    }

    if( zText ){
      char *zTextNew = sqlite3_mprintf("%s {%s}", zText, zCell);
      sqlite3_free(zText);
      zText = zTextNew;
    }else{
      zText = sqlite3_mprintf("{%s}", zCell);
    }
  }
  
  sqlite3_result_text(ctx, zText, -1, sqlite3_free);
}

/* This routine implements an SQL function that returns the "depth" parameter
** from the front of a blob that is an r-tree node.  For example:
**
**     SELECT rtreedepth(data) FROM rt_node WHERE nodeno=1;
**
** The depth value is 0 for all nodes other than the root node, and the root
** node always has nodeno=1, so the example above is the primary use for this
** routine.  This routine is intended for testing and analysis only.
*/
static void rtreedepth(sqlite3_context *ctx, int nArg, sqlite3_value **apArg){
  UNUSED_PARAMETER(nArg);
  if( sqlite3_value_type(apArg[0])!=SQLITE_BLOB 
   || sqlite3_value_bytes(apArg[0])<2
  ){
    sqlite3_result_error(ctx, "Invalid argument to rtreedepth()", -1); 
  }else{
    u8 *zBlob = (u8 *)sqlite3_value_blob(apArg[0]);
    sqlite3_result_int(ctx, readInt16(zBlob));
  }
}

/*
** Register the r-tree module with database handle db. This creates the
** virtual table module "rtree" and the debugging/analysis scalar 
** function "rtreenode".
*/
SQLITE_PRIVATE int sqlite3RtreeInit(sqlite3 *db){
  const int utf8 = SQLITE_UTF8;
  int rc;

  rc = sqlite3_create_function(db, "rtreenode", 2, utf8, 0, rtreenode, 0, 0);
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_function(db, "rtreedepth", 1, utf8, 0,rtreedepth, 0, 0);
  }
  if( rc==SQLITE_OK ){
#ifdef SQLITE_RTREE_INT_ONLY
    void *c = (void *)RTREE_COORD_INT32;
#else
    void *c = (void *)RTREE_COORD_REAL32;
#endif
    rc = sqlite3_create_module_v2(db, "rtree", &rtreeModule, c, 0);
  }
  if( rc==SQLITE_OK ){
    void *c = (void *)RTREE_COORD_INT32;
    rc = sqlite3_create_module_v2(db, "rtree_i32", &rtreeModule, c, 0);
  }

  return rc;
}

/*
** This routine deletes the RtreeGeomCallback object that was attached
** one of the SQL functions create by sqlite3_rtree_geometry_callback()
** or sqlite3_rtree_query_callback().  In other words, this routine is the
** destructor for an RtreeGeomCallback objecct.  This routine is called when
** the corresponding SQL function is deleted.
*/
static void rtreeFreeCallback(void *p){
  RtreeGeomCallback *pInfo = (RtreeGeomCallback*)p;
  if( pInfo->xDestructor ) pInfo->xDestructor(pInfo->pContext);
  sqlite3_free(p);
}

/*
** This routine frees the BLOB that is returned by geomCallback().
*/
static void rtreeMatchArgFree(void *pArg){
  int i;
  RtreeMatchArg *p = (RtreeMatchArg*)pArg;
  for(i=0; i<p->nParam; i++){
    sqlite3_value_free(p->apSqlParam[i]);
  }
  sqlite3_free(p);
}

/*
** Each call to sqlite3_rtree_geometry_callback() or
** sqlite3_rtree_query_callback() creates an ordinary SQLite
** scalar function that is implemented by this routine.
**
** All this function does is construct an RtreeMatchArg object that
** contains the geometry-checking callback routines and a list of
** parameters to this function, then return that RtreeMatchArg object
** as a BLOB.
**
** The R-Tree MATCH operator will read the returned BLOB, deserialize
** the RtreeMatchArg object, and use the RtreeMatchArg object to figure
** out which elements of the R-Tree should be returned by the query.
*/
static void geomCallback(sqlite3_context *ctx, int nArg, sqlite3_value **aArg){
  RtreeGeomCallback *pGeomCtx = (RtreeGeomCallback *)sqlite3_user_data(ctx);
  RtreeMatchArg *pBlob;
  int nBlob;
  int memErr = 0;

  nBlob = sizeof(RtreeMatchArg) + (nArg-1)*sizeof(RtreeDValue)
           + nArg*sizeof(sqlite3_value*);
  pBlob = (RtreeMatchArg *)sqlite3_malloc(nBlob);
  if( !pBlob ){
    sqlite3_result_error_nomem(ctx);
  }else{
    int i;
    pBlob->magic = RTREE_GEOMETRY_MAGIC;
    pBlob->cb = pGeomCtx[0];
    pBlob->apSqlParam = (sqlite3_value**)&pBlob->aParam[nArg];
    pBlob->nParam = nArg;
    for(i=0; i<nArg; i++){
      pBlob->apSqlParam[i] = sqlite3_value_dup(aArg[i]);
      if( pBlob->apSqlParam[i]==0 ) memErr = 1;
#ifdef SQLITE_RTREE_INT_ONLY
      pBlob->aParam[i] = sqlite3_value_int64(aArg[i]);
#else
      pBlob->aParam[i] = sqlite3_value_double(aArg[i]);
#endif
    }
    if( memErr ){
      sqlite3_result_error_nomem(ctx);
      rtreeMatchArgFree(pBlob);
    }else{
      sqlite3_result_blob(ctx, pBlob, nBlob, rtreeMatchArgFree);
    }
  }
}

/*
** Register a new geometry function for use with the r-tree MATCH operator.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_geometry_callback(
  sqlite3 *db,                  /* Register SQL function on this connection */
  const char *zGeom,            /* Name of the new SQL function */
  int (*xGeom)(sqlite3_rtree_geometry*,int,RtreeDValue*,int*), /* Callback */
  void *pContext                /* Extra data associated with the callback */
){
  RtreeGeomCallback *pGeomCtx;      /* Context object for new user-function */

  /* Allocate and populate the context object. */
  pGeomCtx = (RtreeGeomCallback *)sqlite3_malloc(sizeof(RtreeGeomCallback));
  if( !pGeomCtx ) return SQLITE_NOMEM;
  pGeomCtx->xGeom = xGeom;
  pGeomCtx->xQueryFunc = 0;
  pGeomCtx->xDestructor = 0;
  pGeomCtx->pContext = pContext;
  return sqlite3_create_function_v2(db, zGeom, -1, SQLITE_ANY, 
      (void *)pGeomCtx, geomCallback, 0, 0, rtreeFreeCallback
  );
}

/*
** Register a new 2nd-generation geometry function for use with the
** r-tree MATCH operator.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_query_callback(
  sqlite3 *db,                 /* Register SQL function on this connection */
  const char *zQueryFunc,      /* Name of new SQL function */
  int (*xQueryFunc)(sqlite3_rtree_query_info*), /* Callback */
  void *pContext,              /* Extra data passed into the callback */
  void (*xDestructor)(void*)   /* Destructor for the extra data */
){
  RtreeGeomCallback *pGeomCtx;      /* Context object for new user-function */

  /* Allocate and populate the context object. */
  pGeomCtx = (RtreeGeomCallback *)sqlite3_malloc(sizeof(RtreeGeomCallback));
  if( !pGeomCtx ) return SQLITE_NOMEM;
  pGeomCtx->xGeom = 0;
  pGeomCtx->xQueryFunc = xQueryFunc;
  pGeomCtx->xDestructor = xDestructor;
  pGeomCtx->pContext = pContext;
  return sqlite3_create_function_v2(db, zQueryFunc, -1, SQLITE_ANY, 
      (void *)pGeomCtx, geomCallback, 0, 0, rtreeFreeCallback
  );
}

#if !SQLITE_CORE
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3RtreeInit(db);
}
#endif

#endif

/************** End of rtree.c ***********************************************/
/************** Begin file icu.c *********************************************/
/*
** 2007 May 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** $Id: icu.c,v 1.7 2007/12/13 21:54:11 drh Exp $
**
** This file implements an integration between the ICU library 
** ("International Components for Unicode", an open-source library 
** for handling unicode data) and SQLite. The integration uses 
** ICU to provide the following to SQLite:
**
**   * An implementation of the SQL regexp() function (and hence REGEXP
**     operator) using the ICU uregex_XX() APIs.
**
**   * Implementations of the SQL scalar upper() and lower() functions
**     for case mapping.
**
**   * Integration of ICU and SQLite collation sequences.
**
**   * An implementation of the LIKE operator that uses ICU to 
**     provide case-independent matching.
*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_ICU)

/* Include ICU headers */
#include <unicode/utypes.h>
#include <unicode/uregex.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>

/* #include <assert.h> */

#ifndef SQLITE_CORE
/*   #include "sqlite3ext.h" */
  SQLITE_EXTENSION_INIT1
#else
/*   #include "sqlite3.h" */
#endif

/*
** Maximum length (in bytes) of the pattern in a LIKE or GLOB
** operator.
*/
#ifndef SQLITE_MAX_LIKE_PATTERN_LENGTH
# define SQLITE_MAX_LIKE_PATTERN_LENGTH 50000
#endif

/*
** Version of sqlite3_free() that is always a function, never a macro.
*/
static void xFree(void *p){
  sqlite3_free(p);
}

/*
** Compare two UTF-8 strings for equality where the first string is
** a "LIKE" expression. Return true (1) if they are the same and 
** false (0) if they are different.
*/
static int icuLikeCompare(
  const uint8_t *zPattern,   /* LIKE pattern */
  const uint8_t *zString,    /* The UTF-8 string to compare against */
  const UChar32 uEsc         /* The escape character */
){
  static const int MATCH_ONE = (UChar32)'_';
  static const int MATCH_ALL = (UChar32)'%';

  int iPattern = 0;       /* Current byte index in zPattern */
  int iString = 0;        /* Current byte index in zString */

  int prevEscape = 0;     /* True if the previous character was uEsc */

  while( zPattern[iPattern]!=0 ){

    /* Read (and consume) the next character from the input pattern. */
    UChar32 uPattern;
    U8_NEXT_UNSAFE(zPattern, iPattern, uPattern);

    /* There are now 4 possibilities:
    **
    **     1. uPattern is an unescaped match-all character "%",
    **     2. uPattern is an unescaped match-one character "_",
    **     3. uPattern is an unescaped escape character, or
    **     4. uPattern is to be handled as an ordinary character
    */
    if( !prevEscape && uPattern==MATCH_ALL ){
      /* Case 1. */
      uint8_t c;

      /* Skip any MATCH_ALL or MATCH_ONE characters that follow a
      ** MATCH_ALL. For each MATCH_ONE, skip one character in the 
      ** test string.
      */
      while( (c=zPattern[iPattern]) == MATCH_ALL || c == MATCH_ONE ){
        if( c==MATCH_ONE ){
          if( zString[iString]==0 ) return 0;
          U8_FWD_1_UNSAFE(zString, iString);
        }
        iPattern++;
      }

      if( zPattern[iPattern]==0 ) return 1;

      while( zString[iString] ){
        if( icuLikeCompare(&zPattern[iPattern], &zString[iString], uEsc) ){
          return 1;
        }
        U8_FWD_1_UNSAFE(zString, iString);
      }
      return 0;

    }else if( !prevEscape && uPattern==MATCH_ONE ){
      /* Case 2. */
      if( zString[iString]==0 ) return 0;
      U8_FWD_1_UNSAFE(zString, iString);

    }else if( !prevEscape && uPattern==uEsc){
      /* Case 3. */
      prevEscape = 1;

    }else{
      /* Case 4. */
      UChar32 uString;
      U8_NEXT_UNSAFE(zString, iString, uString);
      uString = u_foldCase(uString, U_FOLD_CASE_DEFAULT);
      uPattern = u_foldCase(uPattern, U_FOLD_CASE_DEFAULT);
      if( uString!=uPattern ){
        return 0;
      }
      prevEscape = 0;
    }
  }

  return zString[iString]==0;
}

/*
** Implementation of the like() SQL function.  This function implements
** the build-in LIKE operator.  The first argument to the function is the
** pattern and the second argument is the string.  So, the SQL statements:
**
**       A LIKE B
**
** is implemented as like(B, A). If there is an escape character E, 
**
**       A LIKE B ESCAPE E
**
** is mapped to like(B, A, E).
*/
static void icuLikeFunc(
  sqlite3_context *context, 
  int argc, 
  sqlite3_value **argv
){
  const unsigned char *zA = sqlite3_value_text(argv[0]);
  const unsigned char *zB = sqlite3_value_text(argv[1]);
  UChar32 uEsc = 0;

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  if( sqlite3_value_bytes(argv[0])>SQLITE_MAX_LIKE_PATTERN_LENGTH ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    return;
  }


  if( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    int nE= sqlite3_value_bytes(argv[2]);
    const unsigned char *zE = sqlite3_value_text(argv[2]);
    int i = 0;
    if( zE==0 ) return;
    U8_NEXT(zE, i, nE, uEsc);
    if( i!=nE){
      sqlite3_result_error(context, 
          "ESCAPE expression must be a single character", -1);
      return;
    }
  }

  if( zA && zB ){
    sqlite3_result_int(context, icuLikeCompare(zA, zB, uEsc));
  }
}

/*
** This function is called when an ICU function called from within
** the implementation of an SQL scalar function returns an error.
**
** The scalar function context passed as the first argument is 
** loaded with an error message based on the following two args.
*/
static void icuFunctionError(
  sqlite3_context *pCtx,       /* SQLite scalar function context */
  const char *zName,           /* Name of ICU function that failed */
  UErrorCode e                 /* Error code returned by ICU function */
){
  char zBuf[128];
  sqlite3_snprintf(128, zBuf, "ICU error: %s(): %s", zName, u_errorName(e));
  zBuf[127] = '\0';
  sqlite3_result_error(pCtx, zBuf, -1);
}

/*
** Function to delete compiled regexp objects. Registered as
** a destructor function with sqlite3_set_auxdata().
*/
static void icuRegexpDelete(void *p){
  URegularExpression *pExpr = (URegularExpression *)p;
  uregex_close(pExpr);
}

/*
** Implementation of SQLite REGEXP operator. This scalar function takes
** two arguments. The first is a regular expression pattern to compile
** the second is a string to match against that pattern. If either 
** argument is an SQL NULL, then NULL Is returned. Otherwise, the result
** is 1 if the string matches the pattern, or 0 otherwise.
**
** SQLite maps the regexp() function to the regexp() operator such
** that the following two are equivalent:
**
**     zString REGEXP zPattern
**     regexp(zPattern, zString)
**
** Uses the following ICU regexp APIs:
**
**     uregex_open()
**     uregex_matches()
**     uregex_close()
*/
static void icuRegexpFunc(sqlite3_context *p, int nArg, sqlite3_value **apArg){
  UErrorCode status = U_ZERO_ERROR;
  URegularExpression *pExpr;
  UBool res;
  const UChar *zString = sqlite3_value_text16(apArg[1]);

  (void)nArg;  /* Unused parameter */

  /* If the left hand side of the regexp operator is NULL, 
  ** then the result is also NULL. 
  */
  if( !zString ){
    return;
  }

  pExpr = sqlite3_get_auxdata(p, 0);
  if( !pExpr ){
    const UChar *zPattern = sqlite3_value_text16(apArg[0]);
    if( !zPattern ){
      return;
    }
    pExpr = uregex_open(zPattern, -1, 0, 0, &status);

    if( U_SUCCESS(status) ){
      sqlite3_set_auxdata(p, 0, pExpr, icuRegexpDelete);
    }else{
      assert(!pExpr);
      icuFunctionError(p, "uregex_open", status);
      return;
    }
  }

  /* Configure the text that the regular expression operates on. */
  uregex_setText(pExpr, zString, -1, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "uregex_setText", status);
    return;
  }

  /* Attempt the match */
  res = uregex_matches(pExpr, 0, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "uregex_matches", status);
    return;
  }

  /* Set the text that the regular expression operates on to a NULL
  ** pointer. This is not really necessary, but it is tidier than 
  ** leaving the regular expression object configured with an invalid
  ** pointer after this function returns.
  */
  uregex_setText(pExpr, 0, 0, &status);

  /* Return 1 or 0. */
  sqlite3_result_int(p, res ? 1 : 0);
}

/*
** Implementations of scalar functions for case mapping - upper() and 
** lower(). Function upper() converts its input to upper-case (ABC).
** Function lower() converts to lower-case (abc).
**
** ICU provides two types of case mapping, "general" case mapping and
** "language specific". Refer to ICU documentation for the differences
** between the two.
**
** To utilise "general" case mapping, the upper() or lower() scalar 
** functions are invoked with one argument:
**
**     upper('ABC') -> 'abc'
**     lower('abc') -> 'ABC'
**
** To access ICU "language specific" case mapping, upper() or lower()
** should be invoked with two arguments. The second argument is the name
** of the locale to use. Passing an empty string ("") or SQL NULL value
** as the second argument is the same as invoking the 1 argument version
** of upper() or lower().
**
**     lower('I', 'en_us') -> 'i'
**     lower('I', 'tr_tr') -> 'ı' (small dotless i)
**
** http://www.icu-project.org/userguide/posix.html#case_mappings
*/
static void icuCaseFunc16(sqlite3_context *p, int nArg, sqlite3_value **apArg){
  const UChar *zInput;
  UChar *zOutput;
  int nInput;
  int nOutput;

  UErrorCode status = U_ZERO_ERROR;
  const char *zLocale = 0;

  assert(nArg==1 || nArg==2);
  if( nArg==2 ){
    zLocale = (const char *)sqlite3_value_text(apArg[1]);
  }

  zInput = sqlite3_value_text16(apArg[0]);
  if( !zInput ){
    return;
  }
  nInput = sqlite3_value_bytes16(apArg[0]);

  nOutput = nInput * 2 + 2;
  zOutput = sqlite3_malloc(nOutput);
  if( !zOutput ){
    return;
  }

  if( sqlite3_user_data(p) ){
    u_strToUpper(zOutput, nOutput/2, zInput, nInput/2, zLocale, &status);
  }else{
    u_strToLower(zOutput, nOutput/2, zInput, nInput/2, zLocale, &status);
  }

  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "u_strToLower()/u_strToUpper", status);
    return;
  }

  sqlite3_result_text16(p, zOutput, -1, xFree);
}

/*
** Collation sequence destructor function. The pCtx argument points to
** a UCollator structure previously allocated using ucol_open().
*/
static void icuCollationDel(void *pCtx){
  UCollator *p = (UCollator *)pCtx;
  ucol_close(p);
}

/*
** Collation sequence comparison function. The pCtx argument points to
** a UCollator structure previously allocated using ucol_open().
*/
static int icuCollationColl(
  void *pCtx,
  int nLeft,
  const void *zLeft,
  int nRight,
  const void *zRight
){
  UCollationResult res;
  UCollator *p = (UCollator *)pCtx;
  res = ucol_strcoll(p, (UChar *)zLeft, nLeft/2, (UChar *)zRight, nRight/2);
  switch( res ){
    case UCOL_LESS:    return -1;
    case UCOL_GREATER: return +1;
    case UCOL_EQUAL:   return 0;
  }
  assert(!"Unexpected return value from ucol_strcoll()");
  return 0;
}

/*
** Implementation of the scalar function icu_load_collation().
**
** This scalar function is used to add ICU collation based collation 
** types to an SQLite database connection. It is intended to be called
** as follows:
**
**     SELECT icu_load_collation(<locale>, <collation-name>);
**
** Where <locale> is a string containing an ICU locale identifier (i.e.
** "en_AU", "tr_TR" etc.) and <collation-name> is the name of the
** collation sequence to create.
*/
static void icuLoadCollation(
  sqlite3_context *p, 
  int nArg, 
  sqlite3_value **apArg
){
  sqlite3 *db = (sqlite3 *)sqlite3_user_data(p);
  UErrorCode status = U_ZERO_ERROR;
  const char *zLocale;      /* Locale identifier - (eg. "jp_JP") */
  const char *zName;        /* SQL Collation sequence name (eg. "japanese") */
  UCollator *pUCollator;    /* ICU library collation object */
  int rc;                   /* Return code from sqlite3_create_collation_x() */

  assert(nArg==2);
  (void)nArg; /* Unused parameter */
  zLocale = (const char *)sqlite3_value_text(apArg[0]);
  zName = (const char *)sqlite3_value_text(apArg[1]);

  if( !zLocale || !zName ){
    return;
  }

  pUCollator = ucol_open(zLocale, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "ucol_open", status);
    return;
  }
  assert(p);

  rc = sqlite3_create_collation_v2(db, zName, SQLITE_UTF16, (void *)pUCollator, 
      icuCollationColl, icuCollationDel
  );
  if( rc!=SQLITE_OK ){
    ucol_close(pUCollator);
    sqlite3_result_error(p, "Error registering collation function", -1);
  }
}

/*
** Register the ICU extension functions with database db.
*/
SQLITE_PRIVATE int sqlite3IcuInit(sqlite3 *db){
  struct IcuScalar {
    const char *zName;                        /* Function name */
    int nArg;                                 /* Number of arguments */
    int enc;                                  /* Optimal text encoding */
    void *pContext;                           /* sqlite3_user_data() context */
    void (*xFunc)(sqlite3_context*,int,sqlite3_value**);
  } scalars[] = {
    {"regexp", 2, SQLITE_ANY,          0, icuRegexpFunc},

    {"lower",  1, SQLITE_UTF16,        0, icuCaseFunc16},
    {"lower",  2, SQLITE_UTF16,        0, icuCaseFunc16},
    {"upper",  1, SQLITE_UTF16, (void*)1, icuCaseFunc16},
    {"upper",  2, SQLITE_UTF16, (void*)1, icuCaseFunc16},

    {"lower",  1, SQLITE_UTF8,         0, icuCaseFunc16},
    {"lower",  2, SQLITE_UTF8,         0, icuCaseFunc16},
    {"upper",  1, SQLITE_UTF8,  (void*)1, icuCaseFunc16},
    {"upper",  2, SQLITE_UTF8,  (void*)1, icuCaseFunc16},

    {"like",   2, SQLITE_UTF8,         0, icuLikeFunc},
    {"like",   3, SQLITE_UTF8,         0, icuLikeFunc},

    {"icu_load_collation",  2, SQLITE_UTF8, (void*)db, icuLoadCollation},
  };

  int rc = SQLITE_OK;
  int i;

  for(i=0; rc==SQLITE_OK && i<(int)(sizeof(scalars)/sizeof(scalars[0])); i++){
    struct IcuScalar *p = &scalars[i];
    rc = sqlite3_create_function(
        db, p->zName, p->nArg, p->enc, p->pContext, p->xFunc, 0, 0
    );
  }

  return rc;
}

#if !SQLITE_CORE
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int SQLITE_STDCALL sqlite3_icu_init(
  sqlite3 *db, 
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3IcuInit(db);
}
#endif

#endif

/************** End of icu.c *************************************************/
/************** Begin file fts3_icu.c ****************************************/
/*
** 2007 June 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements a tokenizer for fts3 based on the ICU library.
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)
#ifdef SQLITE_ENABLE_ICU

/* #include <assert.h> */
/* #include <string.h> */
/* #include "fts3_tokenizer.h" */

#include <unicode/ubrk.h>
/* #include <unicode/ucol.h> */
/* #include <unicode/ustring.h> */
#include <unicode/utf16.h>

typedef struct IcuTokenizer IcuTokenizer;
typedef struct IcuCursor IcuCursor;

struct IcuTokenizer {
  sqlite3_tokenizer base;
  char *zLocale;
};

struct IcuCursor {
  sqlite3_tokenizer_cursor base;

  UBreakIterator *pIter;      /* ICU break-iterator object */
  int nChar;                  /* Number of UChar elements in pInput */
  UChar *aChar;               /* Copy of input using utf-16 encoding */
  int *aOffset;               /* Offsets of each character in utf-8 input */

  int nBuffer;
  char *zBuffer;

  int iToken;
};

/*
** Create a new tokenizer instance.
*/
static int icuCreate(
  int argc,                            /* Number of entries in argv[] */
  const char * const *argv,            /* Tokenizer creation arguments */
  sqlite3_tokenizer **ppTokenizer      /* OUT: Created tokenizer */
){
  IcuTokenizer *p;
  int n = 0;

  if( argc>0 ){
    n = strlen(argv[0])+1;
  }
  p = (IcuTokenizer *)sqlite3_malloc(sizeof(IcuTokenizer)+n);
  if( !p ){
    return SQLITE_NOMEM;
  }
  memset(p, 0, sizeof(IcuTokenizer));

  if( n ){
    p->zLocale = (char *)&p[1];
    memcpy(p->zLocale, argv[0], n);
  }

  *ppTokenizer = (sqlite3_tokenizer *)p;

  return SQLITE_OK;
}

/*
** Destroy a tokenizer
*/
static int icuDestroy(sqlite3_tokenizer *pTokenizer){
  IcuTokenizer *p = (IcuTokenizer *)pTokenizer;
  sqlite3_free(p);
  return SQLITE_OK;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is pInput[0..nBytes-1].  A cursor
** used to incrementally tokenize this string is returned in 
** *ppCursor.
*/
static int icuOpen(
  sqlite3_tokenizer *pTokenizer,         /* The tokenizer */
  const char *zInput,                    /* Input string */
  int nInput,                            /* Length of zInput in bytes */
  sqlite3_tokenizer_cursor **ppCursor    /* OUT: Tokenization cursor */
){
  IcuTokenizer *p = (IcuTokenizer *)pTokenizer;
  IcuCursor *pCsr;

  const int32_t opt = U_FOLD_CASE_DEFAULT;
  UErrorCode status = U_ZERO_ERROR;
  int nChar;

  UChar32 c;
  int iInput = 0;
  int iOut = 0;

  *ppCursor = 0;

  if( zInput==0 ){
    nInput = 0;
    zInput = "";
  }else if( nInput<0 ){
    nInput = strlen(zInput);
  }
  nChar = nInput+1;
  pCsr = (IcuCursor *)sqlite3_malloc(
      sizeof(IcuCursor) +                /* IcuCursor */
      ((nChar+3)&~3) * sizeof(UChar) +   /* IcuCursor.aChar[] */
      (nChar+1) * sizeof(int)            /* IcuCursor.aOffset[] */
  );
  if( !pCsr ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(IcuCursor));
  pCsr->aChar = (UChar *)&pCsr[1];
  pCsr->aOffset = (int *)&pCsr->aChar[(nChar+3)&~3];

  pCsr->aOffset[iOut] = iInput;
  U8_NEXT(zInput, iInput, nInput, c); 
  while( c>0 ){
    int isError = 0;
    c = u_foldCase(c, opt);
    U16_APPEND(pCsr->aChar, iOut, nChar, c, isError);
    if( isError ){
      sqlite3_free(pCsr);
      return SQLITE_ERROR;
    }
    pCsr->aOffset[iOut] = iInput;

    if( iInput<nInput ){
      U8_NEXT(zInput, iInput, nInput, c);
    }else{
      c = 0;
    }
  }

  pCsr->pIter = ubrk_open(UBRK_WORD, p->zLocale, pCsr->aChar, iOut, &status);
  if( !U_SUCCESS(status) ){
    sqlite3_free(pCsr);
    return SQLITE_ERROR;
  }
  pCsr->nChar = iOut;

  ubrk_first(pCsr->pIter);
  *ppCursor = (sqlite3_tokenizer_cursor *)pCsr;
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to icuOpen().
*/
static int icuClose(sqlite3_tokenizer_cursor *pCursor){
  IcuCursor *pCsr = (IcuCursor *)pCursor;
  ubrk_close(pCsr->pIter);
  sqlite3_free(pCsr->zBuffer);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Extract the next token from a tokenization cursor.
*/
static int icuNext(
  sqlite3_tokenizer_cursor *pCursor,  /* Cursor returned by simpleOpen */
  const char **ppToken,               /* OUT: *ppToken is the token text */
  int *pnBytes,                       /* OUT: Number of bytes in token */
  int *piStartOffset,                 /* OUT: Starting offset of token */
  int *piEndOffset,                   /* OUT: Ending offset of token */
  int *piPosition                     /* OUT: Position integer of token */
){
  IcuCursor *pCsr = (IcuCursor *)pCursor;

  int iStart = 0;
  int iEnd = 0;
  int nByte = 0;

  while( iStart==iEnd ){
    UChar32 c;

    iStart = ubrk_current(pCsr->pIter);
    iEnd = ubrk_next(pCsr->pIter);
    if( iEnd==UBRK_DONE ){
      return SQLITE_DONE;
    }

    while( iStart<iEnd ){
      int iWhite = iStart;
      U16_NEXT(pCsr->aChar, iWhite, pCsr->nChar, c);
      if( u_isspace(c) ){
        iStart = iWhite;
      }else{
        break;
      }
    }
    assert(iStart<=iEnd);
  }

  do {
    UErrorCode status = U_ZERO_ERROR;
    if( nByte ){
      char *zNew = sqlite3_realloc(pCsr->zBuffer, nByte);
      if( !zNew ){
        return SQLITE_NOMEM;
      }
      pCsr->zBuffer = zNew;
      pCsr->nBuffer = nByte;
    }

    u_strToUTF8(
        pCsr->zBuffer, pCsr->nBuffer, &nByte,    /* Output vars */
        &pCsr->aChar[iStart], iEnd-iStart,       /* Input vars */
        &status                                  /* Output success/failure */
    );
  } while( nByte>pCsr->nBuffer );

  *ppToken = pCsr->zBuffer;
  *pnBytes = nByte;
  *piStartOffset = pCsr->aOffset[iStart];
  *piEndOffset = pCsr->aOffset[iEnd];
  *piPosition = pCsr->iToken++;

  return SQLITE_OK;
}

/*
** The set of routines that implement the simple tokenizer
*/
static const sqlite3_tokenizer_module icuTokenizerModule = {
  0,                           /* iVersion    */
  icuCreate,                   /* xCreate     */
  icuDestroy,                  /* xCreate     */
  icuOpen,                     /* xOpen       */
  icuClose,                    /* xClose      */
  icuNext,                     /* xNext       */
  0,                           /* xLanguageid */
};

/*
** Set *ppModule to point at the implementation of the ICU tokenizer.
*/
SQLITE_PRIVATE void sqlite3Fts3IcuTokenizerModule(
  sqlite3_tokenizer_module const**ppModule
){
  *ppModule = &icuTokenizerModule;
}

#endif /* defined(SQLITE_ENABLE_ICU) */
#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_icu.c ********************************************/
/************** Begin file sqlite3rbu.c **************************************/
/*
** 2014 August 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
**
** OVERVIEW 
**
**  The RBU extension requires that the RBU update be packaged as an
**  SQLite database. The tables it expects to find are described in
**  sqlite3rbu.h.  Essentially, for each table xyz in the target database
**  that the user wishes to write to, a corresponding data_xyz table is
**  created in the RBU database and populated with one row for each row to
**  update, insert or delete from the target table.
** 
**  The update proceeds in three stages:
** 
**  1) The database is updated. The modified database pages are written
**     to a *-oal file. A *-oal file is just like a *-wal file, except
**     that it is named "<database>-oal" instead of "<database>-wal".
**     Because regular SQLite clients do not look for file named
**     "<database>-oal", they go on using the original database in
**     rollback mode while the *-oal file is being generated.
** 
**     During this stage RBU does not update the database by writing
**     directly to the target tables. Instead it creates "imposter"
**     tables using the SQLITE_TESTCTRL_IMPOSTER interface that it uses
**     to update each b-tree individually. All updates required by each
**     b-tree are completed before moving on to the next, and all
**     updates are done in sorted key order.
** 
**  2) The "<database>-oal" file is moved to the equivalent "<database>-wal"
**     location using a call to rename(2). Before doing this the RBU
**     module takes an EXCLUSIVE lock on the database file, ensuring
**     that there are no other active readers.
** 
**     Once the EXCLUSIVE lock is released, any other database readers
**     detect the new *-wal file and read the database in wal mode. At
**     this point they see the new version of the database - including
**     the updates made as part of the RBU update.
** 
**  3) The new *-wal file is checkpointed. This proceeds in the same way 
**     as a regular database checkpoint, except that a single frame is
**     checkpointed each time sqlite3rbu_step() is called. If the RBU
**     handle is closed before the entire *-wal file is checkpointed,
**     the checkpoint progress is saved in the RBU database and the
**     checkpoint can be resumed by another RBU client at some point in
**     the future.
**
** POTENTIAL PROBLEMS
** 
**  The rename() call might not be portable. And RBU is not currently
**  syncing the directory after renaming the file.
**
**  When state is saved, any commit to the *-oal file and the commit to
**  the RBU update database are not atomic. So if the power fails at the
**  wrong moment they might get out of sync. As the main database will be
**  committed before the RBU update database this will likely either just
**  pass unnoticed, or result in SQLITE_CONSTRAINT errors (due to UNIQUE
**  constraint violations).
**
**  If some client does modify the target database mid RBU update, or some
**  other error occurs, the RBU extension will keep throwing errors. It's
**  not really clear how to get out of this state. The system could just
**  by delete the RBU update database and *-oal file and have the device
**  download the update again and start over.
**
**  At present, for an UPDATE, both the new.* and old.* records are
**  collected in the rbu_xyz table. And for both UPDATEs and DELETEs all
**  fields are collected.  This means we're probably writing a lot more
**  data to disk when saving the state of an ongoing update to the RBU
**  update database than is strictly necessary.
** 
*/

/* #include <assert.h> */
/* #include <string.h> */
/* #include <stdio.h> */

/* #include "sqlite3.h" */

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_RBU)
/************** Include sqlite3rbu.h in the middle of sqlite3rbu.c ***********/
/************** Begin file sqlite3rbu.h **************************************/
/*
** 2014 August 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains the public interface for the RBU extension. 
*/

/*
** SUMMARY
**
** Writing a transaction containing a large number of operations on 
** b-tree indexes that are collectively larger than the available cache
** memory can be very inefficient. 
**
** The problem is that in order to update a b-tree, the leaf page (at least)
** containing the entry being inserted or deleted must be modified. If the
** working set of leaves is larger than the available cache memory, then a 
** single leaf that is modified more than once as part of the transaction 
** may be loaded from or written to the persistent media multiple times.
** Additionally, because the index updates are likely to be applied in
** random order, access to pages within the database is also likely to be in 
** random order, which is itself quite inefficient.
**
** One way to improve the situation is to sort the operations on each index
** by index key before applying them to the b-tree. This leads to an IO
** pattern that resembles a single linear scan through the index b-tree,
** and all but guarantees each modified leaf page is loaded and stored 
** exactly once. SQLite uses this trick to improve the performance of
** CREATE INDEX commands. This extension allows it to be used to improve
** the performance of large transactions on existing databases.
**
** Additionally, this extension allows the work involved in writing the 
** large transaction to be broken down into sub-transactions performed 
** sequentially by separate processes. This is useful if the system cannot 
** guarantee that a single update process will run for long enough to apply 
** the entire update, for example because the update is being applied on a 
** mobile device that is frequently rebooted. Even after the writer process 
** has committed one or more sub-transactions, other database clients continue
** to read from the original database snapshot. In other words, partially 
** applied transactions are not visible to other clients. 
**
** "RBU" stands for "Resumable Bulk Update". As in a large database update
** transmitted via a wireless network to a mobile device. A transaction
** applied using this extension is hence refered to as an "RBU update".
**
**
** LIMITATIONS
**
** An "RBU update" transaction is subject to the following limitations:
**
**   * The transaction must consist of INSERT, UPDATE and DELETE operations
**     only.
**
**   * INSERT statements may not use any default values.
**
**   * UPDATE and DELETE statements must identify their target rows by 
**     non-NULL PRIMARY KEY values. Rows with NULL values stored in PRIMARY
**     KEY fields may not be updated or deleted. If the table being written 
**     has no PRIMARY KEY, affected rows must be identified by rowid.
**
**   * UPDATE statements may not modify PRIMARY KEY columns.
**
**   * No triggers will be fired.
**
**   * No foreign key violations are detected or reported.
**
**   * CHECK constraints are not enforced.
**
**   * No constraint handling mode except for "OR ROLLBACK" is supported.
**
**
** PREPARATION
**
** An "RBU update" is stored as a separate SQLite database. A database
** containing an RBU update is an "RBU database". For each table in the 
** target database to be updated, the RBU database should contain a table
** named "data_<target name>" containing the same set of columns as the
** target table, and one more - "rbu_control". The data_% table should 
** have no PRIMARY KEY or UNIQUE constraints, but each column should have
** the same type as the corresponding column in the target database.
** The "rbu_control" column should have no type at all. For example, if
** the target database contains:
**
**   CREATE TABLE t1(a INTEGER PRIMARY KEY, b TEXT, c UNIQUE);
**
** Then the RBU database should contain:
**
**   CREATE TABLE data_t1(a INTEGER, b TEXT, c, rbu_control);
**
** The order of the columns in the data_% table does not matter.
**
** Instead of a regular table, the RBU database may also contain virtual
** tables or view named using the data_<target> naming scheme. 
**
** Instead of the plain data_<target> naming scheme, RBU database tables 
** may also be named data<integer>_<target>, where <integer> is any sequence
** of zero or more numeric characters (0-9). This can be significant because
** tables within the RBU database are always processed in order sorted by 
** name. By judicious selection of the the <integer> portion of the names
** of the RBU tables the user can therefore control the order in which they
** are processed. This can be useful, for example, to ensure that "external
** content" FTS4 tables are updated before their underlying content tables.
**
** If the target database table is a virtual table or a table that has no
** PRIMARY KEY declaration, the data_% table must also contain a column 
** named "rbu_rowid". This column is mapped to the tables implicit primary 
** key column - "rowid". Virtual tables for which the "rowid" column does 
** not function like a primary key value cannot be updated using RBU. For 
** example, if the target db contains either of the following:
**
**   CREATE VIRTUAL TABLE x1 USING fts3(a, b);
**   CREATE TABLE x1(a, b)
**
** then the RBU database should contain:
**
**   CREATE TABLE data_x1(a, b, rbu_rowid, rbu_control);
**
** All non-hidden columns (i.e. all columns matched by "SELECT *") of the
** target table must be present in the input table. For virtual tables,
** hidden columns are optional - they are updated by RBU if present in
** the input table, or not otherwise. For example, to write to an fts4
** table with a hidden languageid column such as:
**
**   CREATE VIRTUAL TABLE ft1 USING fts4(a, b, languageid='langid');
**
** Either of the following input table schemas may be used:
**
**   CREATE TABLE data_ft1(a, b, langid, rbu_rowid, rbu_control);
**   CREATE TABLE data_ft1(a, b, rbu_rowid, rbu_control);
**
** For each row to INSERT into the target database as part of the RBU 
** update, the corresponding data_% table should contain a single record
** with the "rbu_control" column set to contain integer value 0. The
** other columns should be set to the values that make up the new record 
** to insert. 
**
** If the target database table has an INTEGER PRIMARY KEY, it is not 
** possible to insert a NULL value into the IPK column. Attempting to 
** do so results in an SQLITE_MISMATCH error.
**
** For each row to DELETE from the target database as part of the RBU 
** update, the corresponding data_% table should contain a single record
** with the "rbu_control" column set to contain integer value 1. The
** real primary key values of the row to delete should be stored in the
** corresponding columns of the data_% table. The values stored in the
** other columns are not used.
**
** For each row to UPDATE from the target database as part of the RBU 
** update, the corresponding data_% table should contain a single record
** with the "rbu_control" column set to contain a value of type text.
** The real primary key values identifying the row to update should be 
** stored in the corresponding columns of the data_% table row, as should
** the new values of all columns being update. The text value in the 
** "rbu_control" column must contain the same number of characters as
** there are columns in the target database table, and must consist entirely
** of 'x' and '.' characters (or in some special cases 'd' - see below). For 
** each column that is being updated, the corresponding character is set to
** 'x'. For those that remain as they are, the corresponding character of the
** rbu_control value should be set to '.'. For example, given the tables 
** above, the update statement:
**
**   UPDATE t1 SET c = 'usa' WHERE a = 4;
**
** is represented by the data_t1 row created by:
**
**   INSERT INTO data_t1(a, b, c, rbu_control) VALUES(4, NULL, 'usa', '..x');
**
** Instead of an 'x' character, characters of the rbu_control value specified
** for UPDATEs may also be set to 'd'. In this case, instead of updating the
** target table with the value stored in the corresponding data_% column, the
** user-defined SQL function "rbu_delta()" is invoked and the result stored in
** the target table column. rbu_delta() is invoked with two arguments - the
** original value currently stored in the target table column and the 
** value specified in the data_xxx table.
**
** For example, this row:
**
**   INSERT INTO data_t1(a, b, c, rbu_control) VALUES(4, NULL, 'usa', '..d');
**
** is similar to an UPDATE statement such as: 
**
**   UPDATE t1 SET c = rbu_delta(c, 'usa') WHERE a = 4;
**
** Finally, if an 'f' character appears in place of a 'd' or 's' in an 
** ota_control string, the contents of the data_xxx table column is assumed
** to be a "fossil delta" - a patch to be applied to a blob value in the
** format used by the fossil source-code management system. In this case
** the existing value within the target database table must be of type BLOB. 
** It is replaced by the result of applying the specified fossil delta to
** itself.
**
** If the target database table is a virtual table or a table with no PRIMARY
** KEY, the rbu_control value should not include a character corresponding 
** to the rbu_rowid value. For example, this:
**
**   INSERT INTO data_ft1(a, b, rbu_rowid, rbu_control) 
**       VALUES(NULL, 'usa', 12, '.x');
**
** causes a result similar to:
**
**   UPDATE ft1 SET b = 'usa' WHERE rowid = 12;
**
** The data_xxx tables themselves should have no PRIMARY KEY declarations.
** However, RBU is more efficient if reading the rows in from each data_xxx
** table in "rowid" order is roughly the same as reading them sorted by
** the PRIMARY KEY of the corresponding target database table. In other 
** words, rows should be sorted using the destination table PRIMARY KEY 
** fields before they are inserted into the data_xxx tables.
**
** USAGE
**
** The API declared below allows an application to apply an RBU update 
** stored on disk to an existing target database. Essentially, the 
** application:
**
**     1) Opens an RBU handle using the sqlite3rbu_open() function.
**
**     2) Registers any required virtual table modules with the database
**        handle returned by sqlite3rbu_db(). Also, if required, register
**        the rbu_delta() implementation.
**
**     3) Calls the sqlite3rbu_step() function one or more times on
**        the new handle. Each call to sqlite3rbu_step() performs a single
**        b-tree operation, so thousands of calls may be required to apply 
**        a complete update.
**
**     4) Calls sqlite3rbu_close() to close the RBU update handle. If
**        sqlite3rbu_step() has been called enough times to completely
**        apply the update to the target database, then the RBU database
**        is marked as fully applied. Otherwise, the state of the RBU 
**        update application is saved in the RBU database for later 
**        resumption.
**
** See comments below for more detail on APIs.
**
** If an update is only partially applied to the target database by the
** time sqlite3rbu_close() is called, various state information is saved 
** within the RBU database. This allows subsequent processes to automatically
** resume the RBU update from where it left off.
**
** To remove all RBU extension state information, returning an RBU database 
** to its original contents, it is sufficient to drop all tables that begin
** with the prefix "rbu_"
**
** DATABASE LOCKING
**
** An RBU update may not be applied to a database in WAL mode. Attempting
** to do so is an error (SQLITE_ERROR).
**
** While an RBU handle is open, a SHARED lock may be held on the target
** database file. This means it is possible for other clients to read the
** database, but not to write it.
**
** If an RBU update is started and then suspended before it is completed,
** then an external client writes to the database, then attempting to resume
** the suspended RBU update is also an error (SQLITE_BUSY).
*/

#ifndef _SQLITE3RBU_H
#define _SQLITE3RBU_H

/* #include "sqlite3.h"              ** Required for error code definitions ** */

typedef struct sqlite3rbu sqlite3rbu;

/*
** Open an RBU handle.
**
** Argument zTarget is the path to the target database. Argument zRbu is
** the path to the RBU database. Each call to this function must be matched
** by a call to sqlite3rbu_close(). When opening the databases, RBU passes
** the SQLITE_CONFIG_URI flag to sqlite3_open_v2(). So if either zTarget
** or zRbu begin with "file:", it will be interpreted as an SQLite 
** database URI, not a regular file name.
**
** If the zState argument is passed a NULL value, the RBU extension stores 
** the current state of the update (how many rows have been updated, which 
** indexes are yet to be updated etc.) within the RBU database itself. This
** can be convenient, as it means that the RBU application does not need to
** organize removing a separate state file after the update is concluded. 
** Or, if zState is non-NULL, it must be a path to a database file in which 
** the RBU extension can store the state of the update.
**
** When resuming an RBU update, the zState argument must be passed the same
** value as when the RBU update was started.
**
** Once the RBU update is finished, the RBU extension does not 
** automatically remove any zState database file, even if it created it.
**
** By default, RBU uses the default VFS to access the files on disk. To
** use a VFS other than the default, an SQLite "file:" URI containing a
** "vfs=..." option may be passed as the zTarget option.
**
** IMPORTANT NOTE FOR ZIPVFS USERS: The RBU extension works with all of
** SQLite's built-in VFSs, including the multiplexor VFS. However it does
** not work out of the box with zipvfs. Refer to the comment describing
** the zipvfs_create_vfs() API below for details on using RBU with zipvfs.
*/
SQLITE_API sqlite3rbu *SQLITE_STDCALL sqlite3rbu_open(
  const char *zTarget, 
  const char *zRbu,
  const char *zState
);

/*
** Internally, each RBU connection uses a separate SQLite database 
** connection to access the target and rbu update databases. This
** API allows the application direct access to these database handles.
**
** The first argument passed to this function must be a valid, open, RBU
** handle. The second argument should be passed zero to access the target
** database handle, or non-zero to access the rbu update database handle.
** Accessing the underlying database handles may be useful in the
** following scenarios:
**
**   * If any target tables are virtual tables, it may be necessary to
**     call sqlite3_create_module() on the target database handle to 
**     register the required virtual table implementations.
**
**   * If the data_xxx tables in the RBU source database are virtual 
**     tables, the application may need to call sqlite3_create_module() on
**     the rbu update db handle to any required virtual table
**     implementations.
**
**   * If the application uses the "rbu_delta()" feature described above,
**     it must use sqlite3_create_function() or similar to register the
**     rbu_delta() implementation with the target database handle.
**
** If an error has occurred, either while opening or stepping the RBU object,
** this function may return NULL. The error code and message may be collected
** when sqlite3rbu_close() is called.
*/
SQLITE_API sqlite3 *SQLITE_STDCALL sqlite3rbu_db(sqlite3rbu*, int bRbu);

/*
** Do some work towards applying the RBU update to the target db. 
**
** Return SQLITE_DONE if the update has been completely applied, or 
** SQLITE_OK if no error occurs but there remains work to do to apply
** the RBU update. If an error does occur, some other error code is 
** returned. 
**
** Once a call to sqlite3rbu_step() has returned a value other than
** SQLITE_OK, all subsequent calls on the same RBU handle are no-ops
** that immediately return the same value.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_step(sqlite3rbu *pRbu);

/*
** Force RBU to save its state to disk.
**
** If a power failure or application crash occurs during an update, following
** system recovery RBU may resume the update from the point at which the state
** was last saved. In other words, from the most recent successful call to 
** sqlite3rbu_close() or this function.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_savestate(sqlite3rbu *pRbu);

/*
** Close an RBU handle. 
**
** If the RBU update has been completely applied, mark the RBU database
** as fully applied. Otherwise, assuming no error has occurred, save the
** current state of the RBU update appliation to the RBU database.
**
** If an error has already occurred as part of an sqlite3rbu_step()
** or sqlite3rbu_open() call, or if one occurs within this function, an
** SQLite error code is returned. Additionally, *pzErrmsg may be set to
** point to a buffer containing a utf-8 formatted English language error
** message. It is the responsibility of the caller to eventually free any 
** such buffer using sqlite3_free().
**
** Otherwise, if no error occurs, this function returns SQLITE_OK if the
** update has been partially applied, or SQLITE_DONE if it has been 
** completely applied.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_close(sqlite3rbu *pRbu, char **pzErrmsg);

/*
** Return the total number of key-value operations (inserts, deletes or 
** updates) that have been performed on the target database since the
** current RBU update was started.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3rbu_progress(sqlite3rbu *pRbu);

/*
** Create an RBU VFS named zName that accesses the underlying file-system
** via existing VFS zParent. Or, if the zParent parameter is passed NULL, 
** then the new RBU VFS uses the default system VFS to access the file-system.
** The new object is registered as a non-default VFS with SQLite before 
** returning.
**
** Part of the RBU implementation uses a custom VFS object. Usually, this
** object is created and deleted automatically by RBU. 
**
** The exception is for applications that also use zipvfs. In this case,
** the custom VFS must be explicitly created by the user before the RBU
** handle is opened. The RBU VFS should be installed so that the zipvfs
** VFS uses the RBU VFS, which in turn uses any other VFS layers in use 
** (for example multiplexor) to access the file-system. For example,
** to assemble an RBU enabled VFS stack that uses both zipvfs and 
** multiplexor (error checking omitted):
**
**     // Create a VFS named "multiplex" (not the default).
**     sqlite3_multiplex_initialize(0, 0);
**
**     // Create an rbu VFS named "rbu" that uses multiplexor. If the
**     // second argument were replaced with NULL, the "rbu" VFS would
**     // access the file-system via the system default VFS, bypassing the
**     // multiplexor.
**     sqlite3rbu_create_vfs("rbu", "multiplex");
**
**     // Create a zipvfs VFS named "zipvfs" that uses rbu.
**     zipvfs_create_vfs_v3("zipvfs", "rbu", 0, xCompressorAlgorithmDetector);
**
**     // Make zipvfs the default VFS.
**     sqlite3_vfs_register(sqlite3_vfs_find("zipvfs"), 1);
**
** Because the default VFS created above includes a RBU functionality, it
** may be used by RBU clients. Attempting to use RBU with a zipvfs VFS stack
** that does not include the RBU layer results in an error.
**
** The overhead of adding the "rbu" VFS to the system is negligible for 
** non-RBU users. There is no harm in an application accessing the 
** file-system via "rbu" all the time, even if it only uses RBU functionality 
** occasionally.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_create_vfs(const char *zName, const char *zParent);

/*
** Deregister and destroy an RBU vfs created by an earlier call to
** sqlite3rbu_create_vfs().
**
** VFS objects are not reference counted. If a VFS object is destroyed
** before all database handles that use it have been closed, the results
** are undefined.
*/
SQLITE_API void SQLITE_STDCALL sqlite3rbu_destroy_vfs(const char *zName);

#endif /* _SQLITE3RBU_H */

/************** End of sqlite3rbu.h ******************************************/
/************** Continuing where we left off in sqlite3rbu.c *****************/

/* Maximum number of prepared UPDATE statements held by this module */
#define SQLITE_RBU_UPDATE_CACHESIZE 16

/*
** Swap two objects of type TYPE.
*/
#if !defined(SQLITE_AMALGAMATION)
# define SWAP(TYPE,A,B) {TYPE t=A; A=B; B=t;}
#endif

/*
** The rbu_state table is used to save the state of a partially applied
** update so that it can be resumed later. The table consists of integer
** keys mapped to values as follows:
**
** RBU_STATE_STAGE:
**   May be set to integer values 1, 2, 4 or 5. As follows:
**       1: the *-rbu file is currently under construction.
**       2: the *-rbu file has been constructed, but not yet moved 
**          to the *-wal path.
**       4: the checkpoint is underway.
**       5: the rbu update has been checkpointed.
**
** RBU_STATE_TBL:
**   Only valid if STAGE==1. The target database name of the table 
**   currently being written.
**
** RBU_STATE_IDX:
**   Only valid if STAGE==1. The target database name of the index 
**   currently being written, or NULL if the main table is currently being
**   updated.
**
** RBU_STATE_ROW:
**   Only valid if STAGE==1. Number of rows already processed for the current
**   table/index.
**
** RBU_STATE_PROGRESS:
**   Trbul number of sqlite3rbu_step() calls made so far as part of this
**   rbu update.
**
** RBU_STATE_CKPT:
**   Valid if STAGE==4. The 64-bit checksum associated with the wal-index
**   header created by recovering the *-wal file. This is used to detect
**   cases when another client appends frames to the *-wal file in the
**   middle of an incremental checkpoint (an incremental checkpoint cannot
**   be continued if this happens).
**
** RBU_STATE_COOKIE:
**   Valid if STAGE==1. The current change-counter cookie value in the 
**   target db file.
**
** RBU_STATE_OALSZ:
**   Valid if STAGE==1. The size in bytes of the *-oal file.
*/
#define RBU_STATE_STAGE       1
#define RBU_STATE_TBL         2
#define RBU_STATE_IDX         3
#define RBU_STATE_ROW         4
#define RBU_STATE_PROGRESS    5
#define RBU_STATE_CKPT        6
#define RBU_STATE_COOKIE      7
#define RBU_STATE_OALSZ       8

#define RBU_STAGE_OAL         1
#define RBU_STAGE_MOVE        2
#define RBU_STAGE_CAPTURE     3
#define RBU_STAGE_CKPT        4
#define RBU_STAGE_DONE        5


#define RBU_CREATE_STATE \
  "CREATE TABLE IF NOT EXISTS %s.rbu_state(k INTEGER PRIMARY KEY, v)"

typedef struct RbuFrame RbuFrame;
typedef struct RbuObjIter RbuObjIter;
typedef struct RbuState RbuState;
typedef struct rbu_vfs rbu_vfs;
typedef struct rbu_file rbu_file;
typedef struct RbuUpdateStmt RbuUpdateStmt;

#if !defined(SQLITE_AMALGAMATION)
typedef unsigned int u32;
typedef unsigned char u8;
typedef sqlite3_int64 i64;
#endif

/*
** These values must match the values defined in wal.c for the equivalent
** locks. These are not magic numbers as they are part of the SQLite file
** format.
*/
#define WAL_LOCK_WRITE  0
#define WAL_LOCK_CKPT   1
#define WAL_LOCK_READ0  3

/*
** A structure to store values read from the rbu_state table in memory.
*/
struct RbuState {
  int eStage;
  char *zTbl;
  char *zIdx;
  i64 iWalCksum;
  int nRow;
  i64 nProgress;
  u32 iCookie;
  i64 iOalSz;
};

struct RbuUpdateStmt {
  char *zMask;                    /* Copy of update mask used with pUpdate */
  sqlite3_stmt *pUpdate;          /* Last update statement (or NULL) */
  RbuUpdateStmt *pNext;
};

/*
** An iterator of this type is used to iterate through all objects in
** the target database that require updating. For each such table, the
** iterator visits, in order:
**
**     * the table itself, 
**     * each index of the table (zero or more points to visit), and
**     * a special "cleanup table" state.
**
** abIndexed:
**   If the table has no indexes on it, abIndexed is set to NULL. Otherwise,
**   it points to an array of flags nTblCol elements in size. The flag is
**   set for each column that is either a part of the PK or a part of an
**   index. Or clear otherwise.
**   
*/
struct RbuObjIter {
  sqlite3_stmt *pTblIter;         /* Iterate through tables */
  sqlite3_stmt *pIdxIter;         /* Index iterator */
  int nTblCol;                    /* Size of azTblCol[] array */
  char **azTblCol;                /* Array of unquoted target column names */
  char **azTblType;               /* Array of target column types */
  int *aiSrcOrder;                /* src table col -> target table col */
  u8 *abTblPk;                    /* Array of flags, set on target PK columns */
  u8 *abNotNull;                  /* Array of flags, set on NOT NULL columns */
  u8 *abIndexed;                  /* Array of flags, set on indexed & PK cols */
  int eType;                      /* Table type - an RBU_PK_XXX value */

  /* Output variables. zTbl==0 implies EOF. */
  int bCleanup;                   /* True in "cleanup" state */
  const char *zTbl;               /* Name of target db table */
  const char *zDataTbl;           /* Name of rbu db table (or null) */
  const char *zIdx;               /* Name of target db index (or null) */
  int iTnum;                      /* Root page of current object */
  int iPkTnum;                    /* If eType==EXTERNAL, root of PK index */
  int bUnique;                    /* Current index is unique */

  /* Statements created by rbuObjIterPrepareAll() */
  int nCol;                       /* Number of columns in current object */
  sqlite3_stmt *pSelect;          /* Source data */
  sqlite3_stmt *pInsert;          /* Statement for INSERT operations */
  sqlite3_stmt *pDelete;          /* Statement for DELETE ops */
  sqlite3_stmt *pTmpInsert;       /* Insert into rbu_tmp_$zDataTbl */

  /* Last UPDATE used (for PK b-tree updates only), or NULL. */
  RbuUpdateStmt *pRbuUpdate;
};

/*
** Values for RbuObjIter.eType
**
**     0: Table does not exist (error)
**     1: Table has an implicit rowid.
**     2: Table has an explicit IPK column.
**     3: Table has an external PK index.
**     4: Table is WITHOUT ROWID.
**     5: Table is a virtual table.
*/
#define RBU_PK_NOTABLE        0
#define RBU_PK_NONE           1
#define RBU_PK_IPK            2
#define RBU_PK_EXTERNAL       3
#define RBU_PK_WITHOUT_ROWID  4
#define RBU_PK_VTAB           5


/*
** Within the RBU_STAGE_OAL stage, each call to sqlite3rbu_step() performs
** one of the following operations.
*/
#define RBU_INSERT     1          /* Insert on a main table b-tree */
#define RBU_DELETE     2          /* Delete a row from a main table b-tree */
#define RBU_IDX_DELETE 3          /* Delete a row from an aux. index b-tree */
#define RBU_IDX_INSERT 4          /* Insert on an aux. index b-tree */
#define RBU_UPDATE     5          /* Update a row in a main table b-tree */


/*
** A single step of an incremental checkpoint - frame iWalFrame of the wal
** file should be copied to page iDbPage of the database file.
*/
struct RbuFrame {
  u32 iDbPage;
  u32 iWalFrame;
};

/*
** RBU handle.
*/
struct sqlite3rbu {
  int eStage;                     /* Value of RBU_STATE_STAGE field */
  sqlite3 *dbMain;                /* target database handle */
  sqlite3 *dbRbu;                 /* rbu database handle */
  char *zTarget;                  /* Path to target db */
  char *zRbu;                     /* Path to rbu db */
  char *zState;                   /* Path to state db (or NULL if zRbu) */
  char zStateDb[5];               /* Db name for state ("stat" or "main") */
  int rc;                         /* Value returned by last rbu_step() call */
  char *zErrmsg;                  /* Error message if rc!=SQLITE_OK */
  int nStep;                      /* Rows processed for current object */
  int nProgress;                  /* Rows processed for all objects */
  RbuObjIter objiter;             /* Iterator for skipping through tbl/idx */
  const char *zVfsName;           /* Name of automatically created rbu vfs */
  rbu_file *pTargetFd;            /* File handle open on target db */
  i64 iOalSz;

  /* The following state variables are used as part of the incremental
  ** checkpoint stage (eStage==RBU_STAGE_CKPT). See comments surrounding
  ** function rbuSetupCheckpoint() for details.  */
  u32 iMaxFrame;                  /* Largest iWalFrame value in aFrame[] */
  u32 mLock;
  int nFrame;                     /* Entries in aFrame[] array */
  int nFrameAlloc;                /* Allocated size of aFrame[] array */
  RbuFrame *aFrame;
  int pgsz;
  u8 *aBuf;
  i64 iWalCksum;
};

/*
** An rbu VFS is implemented using an instance of this structure.
*/
struct rbu_vfs {
  sqlite3_vfs base;               /* rbu VFS shim methods */
  sqlite3_vfs *pRealVfs;          /* Underlying VFS */
  sqlite3_mutex *mutex;           /* Mutex to protect pMain */
  rbu_file *pMain;                /* Linked list of main db files */
};

/*
** Each file opened by an rbu VFS is represented by an instance of
** the following structure.
*/
struct rbu_file {
  sqlite3_file base;              /* sqlite3_file methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  rbu_vfs *pRbuVfs;               /* Pointer to the rbu_vfs object */
  sqlite3rbu *pRbu;               /* Pointer to rbu object (rbu target only) */

  int openFlags;                  /* Flags this file was opened with */
  u32 iCookie;                    /* Cookie value for main db files */
  u8 iWriteVer;                   /* "write-version" value for main db files */

  int nShm;                       /* Number of entries in apShm[] array */
  char **apShm;                   /* Array of mmap'd *-shm regions */
  char *zDel;                     /* Delete this when closing file */

  const char *zWal;               /* Wal filename for this main db file */
  rbu_file *pWalFd;               /* Wal file descriptor for this main db */
  rbu_file *pMainNext;            /* Next MAIN_DB file */
};


/*************************************************************************
** The following three functions, found below:
**
**   rbuDeltaGetInt()
**   rbuDeltaChecksum()
**   rbuDeltaApply()
**
** are lifted from the fossil source code (http://fossil-scm.org). They
** are used to implement the scalar SQL function rbu_fossil_delta().
*/

/*
** Read bytes from *pz and convert them into a positive integer.  When
** finished, leave *pz pointing to the first character past the end of
** the integer.  The *pLen parameter holds the length of the string
** in *pz and is decremented once for each character in the integer.
*/
static unsigned int rbuDeltaGetInt(const char **pz, int *pLen){
  static const signed char zValue[] = {
    -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,    8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16,   17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,   33, 34, 35, -1, -1, -1, -1, 36,
    -1, 37, 38, 39, 40, 41, 42, 43,   44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 59,   60, 61, 62, -1, -1, -1, 63, -1,
  };
  unsigned int v = 0;
  int c;
  unsigned char *z = (unsigned char*)*pz;
  unsigned char *zStart = z;
  while( (c = zValue[0x7f&*(z++)])>=0 ){
     v = (v<<6) + c;
  }
  z--;
  *pLen -= z - zStart;
  *pz = (char*)z;
  return v;
}

/*
** Compute a 32-bit checksum on the N-byte buffer.  Return the result.
*/
static unsigned int rbuDeltaChecksum(const char *zIn, size_t N){
  const unsigned char *z = (const unsigned char *)zIn;
  unsigned sum0 = 0;
  unsigned sum1 = 0;
  unsigned sum2 = 0;
  unsigned sum3 = 0;
  while(N >= 16){
    sum0 += ((unsigned)z[0] + z[4] + z[8] + z[12]);
    sum1 += ((unsigned)z[1] + z[5] + z[9] + z[13]);
    sum2 += ((unsigned)z[2] + z[6] + z[10]+ z[14]);
    sum3 += ((unsigned)z[3] + z[7] + z[11]+ z[15]);
    z += 16;
    N -= 16;
  }
  while(N >= 4){
    sum0 += z[0];
    sum1 += z[1];
    sum2 += z[2];
    sum3 += z[3];
    z += 4;
    N -= 4;
  }
  sum3 += (sum2 << 8) + (sum1 << 16) + (sum0 << 24);
  switch(N){
    case 3:   sum3 += (z[2] << 8);
    case 2:   sum3 += (z[1] << 16);
    case 1:   sum3 += (z[0] << 24);
    default:  ;
  }
  return sum3;
}

/*
** Apply a delta.
**
** The output buffer should be big enough to hold the whole output
** file and a NUL terminator at the end.  The delta_output_size()
** routine will determine this size for you.
**
** The delta string should be null-terminated.  But the delta string
** may contain embedded NUL characters (if the input and output are
** binary files) so we also have to pass in the length of the delta in
** the lenDelta parameter.
**
** This function returns the size of the output file in bytes (excluding
** the final NUL terminator character).  Except, if the delta string is
** malformed or intended for use with a source file other than zSrc,
** then this routine returns -1.
**
** Refer to the delta_create() documentation above for a description
** of the delta file format.
*/
static int rbuDeltaApply(
  const char *zSrc,      /* The source or pattern file */
  int lenSrc,            /* Length of the source file */
  const char *zDelta,    /* Delta to apply to the pattern */
  int lenDelta,          /* Length of the delta */
  char *zOut             /* Write the output into this preallocated buffer */
){
  unsigned int limit;
  unsigned int total = 0;
#ifndef FOSSIL_OMIT_DELTA_CKSUM_TEST
  char *zOrigOut = zOut;
#endif

  limit = rbuDeltaGetInt(&zDelta, &lenDelta);
  if( *zDelta!='\n' ){
    /* ERROR: size integer not terminated by "\n" */
    return -1;
  }
  zDelta++; lenDelta--;
  while( *zDelta && lenDelta>0 ){
    unsigned int cnt, ofst;
    cnt = rbuDeltaGetInt(&zDelta, &lenDelta);
    switch( zDelta[0] ){
      case '@': {
        zDelta++; lenDelta--;
        ofst = rbuDeltaGetInt(&zDelta, &lenDelta);
        if( lenDelta>0 && zDelta[0]!=',' ){
          /* ERROR: copy command not terminated by ',' */
          return -1;
        }
        zDelta++; lenDelta--;
        total += cnt;
        if( total>limit ){
          /* ERROR: copy exceeds output file size */
          return -1;
        }
        if( (int)(ofst+cnt) > lenSrc ){
          /* ERROR: copy extends past end of input */
          return -1;
        }
        memcpy(zOut, &zSrc[ofst], cnt);
        zOut += cnt;
        break;
      }
      case ':': {
        zDelta++; lenDelta--;
        total += cnt;
        if( total>limit ){
          /* ERROR:  insert command gives an output larger than predicted */
          return -1;
        }
        if( (int)cnt>lenDelta ){
          /* ERROR: insert count exceeds size of delta */
          return -1;
        }
        memcpy(zOut, zDelta, cnt);
        zOut += cnt;
        zDelta += cnt;
        lenDelta -= cnt;
        break;
      }
      case ';': {
        zDelta++; lenDelta--;
        zOut[0] = 0;
#ifndef FOSSIL_OMIT_DELTA_CKSUM_TEST
        if( cnt!=rbuDeltaChecksum(zOrigOut, total) ){
          /* ERROR:  bad checksum */
          return -1;
        }
#endif
        if( total!=limit ){
          /* ERROR: generated size does not match predicted size */
          return -1;
        }
        return total;
      }
      default: {
        /* ERROR: unknown delta operator */
        return -1;
      }
    }
  }
  /* ERROR: unterminated delta */
  return -1;
}

static int rbuDeltaOutputSize(const char *zDelta, int lenDelta){
  int size;
  size = rbuDeltaGetInt(&zDelta, &lenDelta);
  if( *zDelta!='\n' ){
    /* ERROR: size integer not terminated by "\n" */
    return -1;
  }
  return size;
}

/*
** End of code taken from fossil.
*************************************************************************/

/*
** Implementation of SQL scalar function rbu_fossil_delta().
**
** This function applies a fossil delta patch to a blob. Exactly two
** arguments must be passed to this function. The first is the blob to
** patch and the second the patch to apply. If no error occurs, this
** function returns the patched blob.
*/
static void rbuFossilDeltaFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const char *aDelta;
  int nDelta;
  const char *aOrig;
  int nOrig;

  int nOut;
  int nOut2;
  char *aOut;

  assert( argc==2 );

  nOrig = sqlite3_value_bytes(argv[0]);
  aOrig = (const char*)sqlite3_value_blob(argv[0]);
  nDelta = sqlite3_value_bytes(argv[1]);
  aDelta = (const char*)sqlite3_value_blob(argv[1]);

  /* Figure out the size of the output */
  nOut = rbuDeltaOutputSize(aDelta, nDelta);
  if( nOut<0 ){
    sqlite3_result_error(context, "corrupt fossil delta", -1);
    return;
  }

  aOut = sqlite3_malloc(nOut+1);
  if( aOut==0 ){
    sqlite3_result_error_nomem(context);
  }else{
    nOut2 = rbuDeltaApply(aOrig, nOrig, aDelta, nDelta, aOut);
    if( nOut2!=nOut ){
      sqlite3_result_error(context, "corrupt fossil delta", -1);
    }else{
      sqlite3_result_blob(context, aOut, nOut, sqlite3_free);
    }
  }
}


/*
** Prepare the SQL statement in buffer zSql against database handle db.
** If successful, set *ppStmt to point to the new statement and return
** SQLITE_OK. 
**
** Otherwise, if an error does occur, set *ppStmt to NULL and return
** an SQLite error code. Additionally, set output variable *pzErrmsg to
** point to a buffer containing an error message. It is the responsibility
** of the caller to (eventually) free this buffer using sqlite3_free().
*/
static int prepareAndCollectError(
  sqlite3 *db, 
  sqlite3_stmt **ppStmt,
  char **pzErrmsg,
  const char *zSql
){
  int rc = sqlite3_prepare_v2(db, zSql, -1, ppStmt, 0);
  if( rc!=SQLITE_OK ){
    *pzErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    *ppStmt = 0;
  }
  return rc;
}

/*
** Reset the SQL statement passed as the first argument. Return a copy
** of the value returned by sqlite3_reset().
**
** If an error has occurred, then set *pzErrmsg to point to a buffer
** containing an error message. It is the responsibility of the caller
** to eventually free this buffer using sqlite3_free().
*/
static int resetAndCollectError(sqlite3_stmt *pStmt, char **pzErrmsg){
  int rc = sqlite3_reset(pStmt);
  if( rc!=SQLITE_OK ){
    *pzErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(sqlite3_db_handle(pStmt)));
  }
  return rc;
}

/*
** Unless it is NULL, argument zSql points to a buffer allocated using
** sqlite3_malloc containing an SQL statement. This function prepares the SQL
** statement against database db and frees the buffer. If statement 
** compilation is successful, *ppStmt is set to point to the new statement 
** handle and SQLITE_OK is returned. 
**
** Otherwise, if an error occurs, *ppStmt is set to NULL and an error code
** returned. In this case, *pzErrmsg may also be set to point to an error
** message. It is the responsibility of the caller to free this error message
** buffer using sqlite3_free().
**
** If argument zSql is NULL, this function assumes that an OOM has occurred.
** In this case SQLITE_NOMEM is returned and *ppStmt set to NULL.
*/
static int prepareFreeAndCollectError(
  sqlite3 *db, 
  sqlite3_stmt **ppStmt,
  char **pzErrmsg,
  char *zSql
){
  int rc;
  assert( *pzErrmsg==0 );
  if( zSql==0 ){
    rc = SQLITE_NOMEM;
    *ppStmt = 0;
  }else{
    rc = prepareAndCollectError(db, ppStmt, pzErrmsg, zSql);
    sqlite3_free(zSql);
  }
  return rc;
}

/*
** Free the RbuObjIter.azTblCol[] and RbuObjIter.abTblPk[] arrays allocated
** by an earlier call to rbuObjIterCacheTableInfo().
*/
static void rbuObjIterFreeCols(RbuObjIter *pIter){
  int i;
  for(i=0; i<pIter->nTblCol; i++){
    sqlite3_free(pIter->azTblCol[i]);
    sqlite3_free(pIter->azTblType[i]);
  }
  sqlite3_free(pIter->azTblCol);
  pIter->azTblCol = 0;
  pIter->azTblType = 0;
  pIter->aiSrcOrder = 0;
  pIter->abTblPk = 0;
  pIter->abNotNull = 0;
  pIter->nTblCol = 0;
  pIter->eType = 0;               /* Invalid value */
}

/*
** Finalize all statements and free all allocations that are specific to
** the current object (table/index pair).
*/
static void rbuObjIterClearStatements(RbuObjIter *pIter){
  RbuUpdateStmt *pUp;

  sqlite3_finalize(pIter->pSelect);
  sqlite3_finalize(pIter->pInsert);
  sqlite3_finalize(pIter->pDelete);
  sqlite3_finalize(pIter->pTmpInsert);
  pUp = pIter->pRbuUpdate;
  while( pUp ){
    RbuUpdateStmt *pTmp = pUp->pNext;
    sqlite3_finalize(pUp->pUpdate);
    sqlite3_free(pUp);
    pUp = pTmp;
  }
  
  pIter->pSelect = 0;
  pIter->pInsert = 0;
  pIter->pDelete = 0;
  pIter->pRbuUpdate = 0;
  pIter->pTmpInsert = 0;
  pIter->nCol = 0;
}

/*
** Clean up any resources allocated as part of the iterator object passed
** as the only argument.
*/
static void rbuObjIterFinalize(RbuObjIter *pIter){
  rbuObjIterClearStatements(pIter);
  sqlite3_finalize(pIter->pTblIter);
  sqlite3_finalize(pIter->pIdxIter);
  rbuObjIterFreeCols(pIter);
  memset(pIter, 0, sizeof(RbuObjIter));
}

/*
** Advance the iterator to the next position.
**
** If no error occurs, SQLITE_OK is returned and the iterator is left 
** pointing to the next entry. Otherwise, an error code and message is 
** left in the RBU handle passed as the first argument. A copy of the 
** error code is returned.
*/
static int rbuObjIterNext(sqlite3rbu *p, RbuObjIter *pIter){
  int rc = p->rc;
  if( rc==SQLITE_OK ){

    /* Free any SQLite statements used while processing the previous object */ 
    rbuObjIterClearStatements(pIter);
    if( pIter->zIdx==0 ){
      rc = sqlite3_exec(p->dbMain,
          "DROP TRIGGER IF EXISTS temp.rbu_insert_tr;"
          "DROP TRIGGER IF EXISTS temp.rbu_update1_tr;"
          "DROP TRIGGER IF EXISTS temp.rbu_update2_tr;"
          "DROP TRIGGER IF EXISTS temp.rbu_delete_tr;"
          , 0, 0, &p->zErrmsg
      );
    }

    if( rc==SQLITE_OK ){
      if( pIter->bCleanup ){
        rbuObjIterFreeCols(pIter);
        pIter->bCleanup = 0;
        rc = sqlite3_step(pIter->pTblIter);
        if( rc!=SQLITE_ROW ){
          rc = resetAndCollectError(pIter->pTblIter, &p->zErrmsg);
          pIter->zTbl = 0;
        }else{
          pIter->zTbl = (const char*)sqlite3_column_text(pIter->pTblIter, 0);
          pIter->zDataTbl = (const char*)sqlite3_column_text(pIter->pTblIter,1);
          rc = (pIter->zDataTbl && pIter->zTbl) ? SQLITE_OK : SQLITE_NOMEM;
        }
      }else{
        if( pIter->zIdx==0 ){
          sqlite3_stmt *pIdx = pIter->pIdxIter;
          rc = sqlite3_bind_text(pIdx, 1, pIter->zTbl, -1, SQLITE_STATIC);
        }
        if( rc==SQLITE_OK ){
          rc = sqlite3_step(pIter->pIdxIter);
          if( rc!=SQLITE_ROW ){
            rc = resetAndCollectError(pIter->pIdxIter, &p->zErrmsg);
            pIter->bCleanup = 1;
            pIter->zIdx = 0;
          }else{
            pIter->zIdx = (const char*)sqlite3_column_text(pIter->pIdxIter, 0);
            pIter->iTnum = sqlite3_column_int(pIter->pIdxIter, 1);
            pIter->bUnique = sqlite3_column_int(pIter->pIdxIter, 2);
            rc = pIter->zIdx ? SQLITE_OK : SQLITE_NOMEM;
          }
        }
      }
    }
  }

  if( rc!=SQLITE_OK ){
    rbuObjIterFinalize(pIter);
    p->rc = rc;
  }
  return rc;
}


/*
** The implementation of the rbu_target_name() SQL function. This function
** accepts one argument - the name of a table in the RBU database. If the
** table name matches the pattern:
**
**     data[0-9]_<name>
**
** where <name> is any sequence of 1 or more characters, <name> is returned.
** Otherwise, if the only argument does not match the above pattern, an SQL
** NULL is returned.
**
**     "data_t1"     -> "t1"
**     "data0123_t2" -> "t2"
**     "dataAB_t3"   -> NULL
*/
static void rbuTargetNameFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const char *zIn;
  assert( argc==1 );

  zIn = (const char*)sqlite3_value_text(argv[0]);
  if( zIn && strlen(zIn)>4 && memcmp("data", zIn, 4)==0 ){
    int i;
    for(i=4; zIn[i]>='0' && zIn[i]<='9'; i++);
    if( zIn[i]=='_' && zIn[i+1] ){
      sqlite3_result_text(context, &zIn[i+1], -1, SQLITE_STATIC);
    }
  }
}

/*
** Initialize the iterator structure passed as the second argument.
**
** If no error occurs, SQLITE_OK is returned and the iterator is left 
** pointing to the first entry. Otherwise, an error code and message is 
** left in the RBU handle passed as the first argument. A copy of the 
** error code is returned.
*/
static int rbuObjIterFirst(sqlite3rbu *p, RbuObjIter *pIter){
  int rc;
  memset(pIter, 0, sizeof(RbuObjIter));

  rc = prepareAndCollectError(p->dbRbu, &pIter->pTblIter, &p->zErrmsg, 
      "SELECT rbu_target_name(name) AS target, name FROM sqlite_master "
      "WHERE type IN ('table', 'view') AND target IS NOT NULL "
      "ORDER BY name"
  );

  if( rc==SQLITE_OK ){
    rc = prepareAndCollectError(p->dbMain, &pIter->pIdxIter, &p->zErrmsg,
        "SELECT name, rootpage, sql IS NULL OR substr(8, 6)=='UNIQUE' "
        "  FROM main.sqlite_master "
        "  WHERE type='index' AND tbl_name = ?"
    );
  }

  pIter->bCleanup = 1;
  p->rc = rc;
  return rbuObjIterNext(p, pIter);
}

/*
** This is a wrapper around "sqlite3_mprintf(zFmt, ...)". If an OOM occurs,
** an error code is stored in the RBU handle passed as the first argument.
**
** If an error has already occurred (p->rc is already set to something other
** than SQLITE_OK), then this function returns NULL without modifying the
** stored error code. In this case it still calls sqlite3_free() on any 
** printf() parameters associated with %z conversions.
*/
static char *rbuMPrintf(sqlite3rbu *p, const char *zFmt, ...){
  char *zSql = 0;
  va_list ap;
  va_start(ap, zFmt);
  zSql = sqlite3_vmprintf(zFmt, ap);
  if( p->rc==SQLITE_OK ){
    if( zSql==0 ) p->rc = SQLITE_NOMEM;
  }else{
    sqlite3_free(zSql);
    zSql = 0;
  }
  va_end(ap);
  return zSql;
}

/*
** Argument zFmt is a sqlite3_mprintf() style format string. The trailing
** arguments are the usual subsitution values. This function performs
** the printf() style substitutions and executes the result as an SQL
** statement on the RBU handles database.
**
** If an error occurs, an error code and error message is stored in the
** RBU handle. If an error has already occurred when this function is
** called, it is a no-op.
*/
static int rbuMPrintfExec(sqlite3rbu *p, sqlite3 *db, const char *zFmt, ...){
  va_list ap;
  char *zSql;
  va_start(ap, zFmt);
  zSql = sqlite3_vmprintf(zFmt, ap);
  if( p->rc==SQLITE_OK ){
    if( zSql==0 ){
      p->rc = SQLITE_NOMEM;
    }else{
      p->rc = sqlite3_exec(db, zSql, 0, 0, &p->zErrmsg);
    }
  }
  sqlite3_free(zSql);
  va_end(ap);
  return p->rc;
}

/*
** Attempt to allocate and return a pointer to a zeroed block of nByte 
** bytes. 
**
** If an error (i.e. an OOM condition) occurs, return NULL and leave an 
** error code in the rbu handle passed as the first argument. Or, if an 
** error has already occurred when this function is called, return NULL 
** immediately without attempting the allocation or modifying the stored
** error code.
*/
static void *rbuMalloc(sqlite3rbu *p, int nByte){
  void *pRet = 0;
  if( p->rc==SQLITE_OK ){
    assert( nByte>0 );
    pRet = sqlite3_malloc(nByte);
    if( pRet==0 ){
      p->rc = SQLITE_NOMEM;
    }else{
      memset(pRet, 0, nByte);
    }
  }
  return pRet;
}


/*
** Allocate and zero the pIter->azTblCol[] and abTblPk[] arrays so that
** there is room for at least nCol elements. If an OOM occurs, store an
** error code in the RBU handle passed as the first argument.
*/
static void rbuAllocateIterArrays(sqlite3rbu *p, RbuObjIter *pIter, int nCol){
  int nByte = (2*sizeof(char*) + sizeof(int) + 3*sizeof(u8)) * nCol;
  char **azNew;

  azNew = (char**)rbuMalloc(p, nByte);
  if( azNew ){
    pIter->azTblCol = azNew;
    pIter->azTblType = &azNew[nCol];
    pIter->aiSrcOrder = (int*)&pIter->azTblType[nCol];
    pIter->abTblPk = (u8*)&pIter->aiSrcOrder[nCol];
    pIter->abNotNull = (u8*)&pIter->abTblPk[nCol];
    pIter->abIndexed = (u8*)&pIter->abNotNull[nCol];
  }
}

/*
** The first argument must be a nul-terminated string. This function
** returns a copy of the string in memory obtained from sqlite3_malloc().
** It is the responsibility of the caller to eventually free this memory
** using sqlite3_free().
**
** If an OOM condition is encountered when attempting to allocate memory,
** output variable (*pRc) is set to SQLITE_NOMEM before returning. Otherwise,
** if the allocation succeeds, (*pRc) is left unchanged.
*/
static char *rbuStrndup(const char *zStr, int *pRc){
  char *zRet = 0;

  assert( *pRc==SQLITE_OK );
  if( zStr ){
    int nCopy = strlen(zStr) + 1;
    zRet = (char*)sqlite3_malloc(nCopy);
    if( zRet ){
      memcpy(zRet, zStr, nCopy);
    }else{
      *pRc = SQLITE_NOMEM;
    }
  }

  return zRet;
}

/*
** Finalize the statement passed as the second argument.
**
** If the sqlite3_finalize() call indicates that an error occurs, and the
** rbu handle error code is not already set, set the error code and error
** message accordingly.
*/
static void rbuFinalize(sqlite3rbu *p, sqlite3_stmt *pStmt){
  sqlite3 *db = sqlite3_db_handle(pStmt);
  int rc = sqlite3_finalize(pStmt);
  if( p->rc==SQLITE_OK && rc!=SQLITE_OK ){
    p->rc = rc;
    p->zErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }
}

/* Determine the type of a table.
**
**   peType is of type (int*), a pointer to an output parameter of type
**   (int). This call sets the output parameter as follows, depending
**   on the type of the table specified by parameters dbName and zTbl.
**
**     RBU_PK_NOTABLE:       No such table.
**     RBU_PK_NONE:          Table has an implicit rowid.
**     RBU_PK_IPK:           Table has an explicit IPK column.
**     RBU_PK_EXTERNAL:      Table has an external PK index.
**     RBU_PK_WITHOUT_ROWID: Table is WITHOUT ROWID.
**     RBU_PK_VTAB:          Table is a virtual table.
**
**   Argument *piPk is also of type (int*), and also points to an output
**   parameter. Unless the table has an external primary key index 
**   (i.e. unless *peType is set to 3), then *piPk is set to zero. Or,
**   if the table does have an external primary key index, then *piPk
**   is set to the root page number of the primary key index before
**   returning.
**
** ALGORITHM:
**
**   if( no entry exists in sqlite_master ){
**     return RBU_PK_NOTABLE
**   }else if( sql for the entry starts with "CREATE VIRTUAL" ){
**     return RBU_PK_VTAB
**   }else if( "PRAGMA index_list()" for the table contains a "pk" index ){
**     if( the index that is the pk exists in sqlite_master ){
**       *piPK = rootpage of that index.
**       return RBU_PK_EXTERNAL
**     }else{
**       return RBU_PK_WITHOUT_ROWID
**     }
**   }else if( "PRAGMA table_info()" lists one or more "pk" columns ){
**     return RBU_PK_IPK
**   }else{
**     return RBU_PK_NONE
**   }
*/
static void rbuTableType(
  sqlite3rbu *p,
  const char *zTab,
  int *peType,
  int *piTnum,
  int *piPk
){
  /*
  ** 0) SELECT count(*) FROM sqlite_master where name=%Q AND IsVirtual(%Q)
  ** 1) PRAGMA index_list = ?
  ** 2) SELECT count(*) FROM sqlite_master where name=%Q 
  ** 3) PRAGMA table_info = ?
  */
  sqlite3_stmt *aStmt[4] = {0, 0, 0, 0};

  *peType = RBU_PK_NOTABLE;
  *piPk = 0;

  assert( p->rc==SQLITE_OK );
  p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[0], &p->zErrmsg, 
    sqlite3_mprintf(
          "SELECT (sql LIKE 'create virtual%%'), rootpage"
          "  FROM sqlite_master"
          " WHERE name=%Q", zTab
  ));
  if( p->rc!=SQLITE_OK || sqlite3_step(aStmt[0])!=SQLITE_ROW ){
    /* Either an error, or no such table. */
    goto rbuTableType_end;
  }
  if( sqlite3_column_int(aStmt[0], 0) ){
    *peType = RBU_PK_VTAB;                     /* virtual table */
    goto rbuTableType_end;
  }
  *piTnum = sqlite3_column_int(aStmt[0], 1);

  p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[1], &p->zErrmsg, 
    sqlite3_mprintf("PRAGMA index_list=%Q",zTab)
  );
  if( p->rc ) goto rbuTableType_end;
  while( sqlite3_step(aStmt[1])==SQLITE_ROW ){
    const u8 *zOrig = sqlite3_column_text(aStmt[1], 3);
    const u8 *zIdx = sqlite3_column_text(aStmt[1], 1);
    if( zOrig && zIdx && zOrig[0]=='p' ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[2], &p->zErrmsg, 
          sqlite3_mprintf(
            "SELECT rootpage FROM sqlite_master WHERE name = %Q", zIdx
      ));
      if( p->rc==SQLITE_OK ){
        if( sqlite3_step(aStmt[2])==SQLITE_ROW ){
          *piPk = sqlite3_column_int(aStmt[2], 0);
          *peType = RBU_PK_EXTERNAL;
        }else{
          *peType = RBU_PK_WITHOUT_ROWID;
        }
      }
      goto rbuTableType_end;
    }
  }

  p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[3], &p->zErrmsg, 
    sqlite3_mprintf("PRAGMA table_info=%Q",zTab)
  );
  if( p->rc==SQLITE_OK ){
    while( sqlite3_step(aStmt[3])==SQLITE_ROW ){
      if( sqlite3_column_int(aStmt[3],5)>0 ){
        *peType = RBU_PK_IPK;                /* explicit IPK column */
        goto rbuTableType_end;
      }
    }
    *peType = RBU_PK_NONE;
  }

rbuTableType_end: {
    unsigned int i;
    for(i=0; i<sizeof(aStmt)/sizeof(aStmt[0]); i++){
      rbuFinalize(p, aStmt[i]);
    }
  }
}

/*
** This is a helper function for rbuObjIterCacheTableInfo(). It populates
** the pIter->abIndexed[] array.
*/
static void rbuObjIterCacheIndexedCols(sqlite3rbu *p, RbuObjIter *pIter){
  sqlite3_stmt *pList = 0;
  int bIndex = 0;

  if( p->rc==SQLITE_OK ){
    memcpy(pIter->abIndexed, pIter->abTblPk, sizeof(u8)*pIter->nTblCol);
    p->rc = prepareFreeAndCollectError(p->dbMain, &pList, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_list = %Q", pIter->zTbl)
    );
  }

  while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pList) ){
    const char *zIdx = (const char*)sqlite3_column_text(pList, 1);
    sqlite3_stmt *pXInfo = 0;
    if( zIdx==0 ) break;
    p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", zIdx)
    );
    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
      int iCid = sqlite3_column_int(pXInfo, 1);
      if( iCid>=0 ) pIter->abIndexed[iCid] = 1;
    }
    rbuFinalize(p, pXInfo);
    bIndex = 1;
  }

  rbuFinalize(p, pList);
  if( bIndex==0 ) pIter->abIndexed = 0;
}


/*
** If they are not already populated, populate the pIter->azTblCol[],
** pIter->abTblPk[], pIter->nTblCol and pIter->bRowid variables according to
** the table (not index) that the iterator currently points to.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise. If
** an error does occur, an error code and error message are also left in 
** the RBU handle.
*/
static int rbuObjIterCacheTableInfo(sqlite3rbu *p, RbuObjIter *pIter){
  if( pIter->azTblCol==0 ){
    sqlite3_stmt *pStmt = 0;
    int nCol = 0;
    int i;                        /* for() loop iterator variable */
    int bRbuRowid = 0;            /* If input table has column "rbu_rowid" */
    int iOrder = 0;
    int iTnum = 0;

    /* Figure out the type of table this step will deal with. */
    assert( pIter->eType==0 );
    rbuTableType(p, pIter->zTbl, &pIter->eType, &iTnum, &pIter->iPkTnum);
    if( p->rc==SQLITE_OK && pIter->eType==RBU_PK_NOTABLE ){
      p->rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf("no such table: %s", pIter->zTbl);
    }
    if( p->rc ) return p->rc;
    if( pIter->zIdx==0 ) pIter->iTnum = iTnum;

    assert( pIter->eType==RBU_PK_NONE || pIter->eType==RBU_PK_IPK 
         || pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_WITHOUT_ROWID
         || pIter->eType==RBU_PK_VTAB
    );

    /* Populate the azTblCol[] and nTblCol variables based on the columns
    ** of the input table. Ignore any input table columns that begin with
    ** "rbu_".  */
    p->rc = prepareFreeAndCollectError(p->dbRbu, &pStmt, &p->zErrmsg, 
        sqlite3_mprintf("SELECT * FROM '%q'", pIter->zDataTbl)
    );
    if( p->rc==SQLITE_OK ){
      nCol = sqlite3_column_count(pStmt);
      rbuAllocateIterArrays(p, pIter, nCol);
    }
    for(i=0; p->rc==SQLITE_OK && i<nCol; i++){
      const char *zName = (const char*)sqlite3_column_name(pStmt, i);
      if( sqlite3_strnicmp("rbu_", zName, 4) ){
        char *zCopy = rbuStrndup(zName, &p->rc);
        pIter->aiSrcOrder[pIter->nTblCol] = pIter->nTblCol;
        pIter->azTblCol[pIter->nTblCol++] = zCopy;
      }
      else if( 0==sqlite3_stricmp("rbu_rowid", zName) ){
        bRbuRowid = 1;
      }
    }
    sqlite3_finalize(pStmt);
    pStmt = 0;

    if( p->rc==SQLITE_OK
     && bRbuRowid!=(pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE)
    ){
      p->rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf(
          "table %q %s rbu_rowid column", pIter->zDataTbl,
          (bRbuRowid ? "may not have" : "requires")
      );
    }

    /* Check that all non-HIDDEN columns in the destination table are also
    ** present in the input table. Populate the abTblPk[], azTblType[] and
    ** aiTblOrder[] arrays at the same time.  */
    if( p->rc==SQLITE_OK ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &pStmt, &p->zErrmsg, 
          sqlite3_mprintf("PRAGMA table_info(%Q)", pIter->zTbl)
      );
    }
    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
      const char *zName = (const char*)sqlite3_column_text(pStmt, 1);
      if( zName==0 ) break;  /* An OOM - finalize() below returns S_NOMEM */
      for(i=iOrder; i<pIter->nTblCol; i++){
        if( 0==strcmp(zName, pIter->azTblCol[i]) ) break;
      }
      if( i==pIter->nTblCol ){
        p->rc = SQLITE_ERROR;
        p->zErrmsg = sqlite3_mprintf("column missing from %q: %s",
            pIter->zDataTbl, zName
        );
      }else{
        int iPk = sqlite3_column_int(pStmt, 5);
        int bNotNull = sqlite3_column_int(pStmt, 3);
        const char *zType = (const char*)sqlite3_column_text(pStmt, 2);

        if( i!=iOrder ){
          SWAP(int, pIter->aiSrcOrder[i], pIter->aiSrcOrder[iOrder]);
          SWAP(char*, pIter->azTblCol[i], pIter->azTblCol[iOrder]);
        }

        pIter->azTblType[iOrder] = rbuStrndup(zType, &p->rc);
        pIter->abTblPk[iOrder] = (iPk!=0);
        pIter->abNotNull[iOrder] = (u8)bNotNull || (iPk!=0);
        iOrder++;
      }
    }

    rbuFinalize(p, pStmt);
    rbuObjIterCacheIndexedCols(p, pIter);
    assert( pIter->eType!=RBU_PK_VTAB || pIter->abIndexed==0 );
  }

  return p->rc;
}

/*
** This function constructs and returns a pointer to a nul-terminated 
** string containing some SQL clause or list based on one or more of the 
** column names currently stored in the pIter->azTblCol[] array.
*/
static char *rbuObjIterGetCollist(
  sqlite3rbu *p,                  /* RBU object */
  RbuObjIter *pIter               /* Object iterator for column names */
){
  char *zList = 0;
  const char *zSep = "";
  int i;
  for(i=0; i<pIter->nTblCol; i++){
    const char *z = pIter->azTblCol[i];
    zList = rbuMPrintf(p, "%z%s\"%w\"", zList, zSep, z);
    zSep = ", ";
  }
  return zList;
}

/*
** This function is used to create a SELECT list (the list of SQL 
** expressions that follows a SELECT keyword) for a SELECT statement 
** used to read from an data_xxx or rbu_tmp_xxx table while updating the 
** index object currently indicated by the iterator object passed as the 
** second argument. A "PRAGMA index_xinfo = <idxname>" statement is used 
** to obtain the required information.
**
** If the index is of the following form:
**
**   CREATE INDEX i1 ON t1(c, b COLLATE nocase);
**
** and "t1" is a table with an explicit INTEGER PRIMARY KEY column 
** "ipk", the returned string is:
**
**   "`c` COLLATE 'BINARY', `b` COLLATE 'NOCASE', `ipk` COLLATE 'BINARY'"
**
** As well as the returned string, three other malloc'd strings are 
** returned via output parameters. As follows:
**
**   pzImposterCols: ...
**   pzImposterPk: ...
**   pzWhere: ...
*/
static char *rbuObjIterGetIndexCols(
  sqlite3rbu *p,                  /* RBU object */
  RbuObjIter *pIter,              /* Object iterator for column names */
  char **pzImposterCols,          /* OUT: Columns for imposter table */
  char **pzImposterPk,            /* OUT: Imposter PK clause */
  char **pzWhere,                 /* OUT: WHERE clause */
  int *pnBind                     /* OUT: Trbul number of columns */
){
  int rc = p->rc;                 /* Error code */
  int rc2;                        /* sqlite3_finalize() return code */
  char *zRet = 0;                 /* String to return */
  char *zImpCols = 0;             /* String to return via *pzImposterCols */
  char *zImpPK = 0;               /* String to return via *pzImposterPK */
  char *zWhere = 0;               /* String to return via *pzWhere */
  int nBind = 0;                  /* Value to return via *pnBind */
  const char *zCom = "";          /* Set to ", " later on */
  const char *zAnd = "";          /* Set to " AND " later on */
  sqlite3_stmt *pXInfo = 0;       /* PRAGMA index_xinfo = ? */

  if( rc==SQLITE_OK ){
    assert( p->zErrmsg==0 );
    rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", pIter->zIdx)
    );
  }

  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
    int iCid = sqlite3_column_int(pXInfo, 1);
    int bDesc = sqlite3_column_int(pXInfo, 3);
    const char *zCollate = (const char*)sqlite3_column_text(pXInfo, 4);
    const char *zCol;
    const char *zType;

    if( iCid<0 ){
      /* An integer primary key. If the table has an explicit IPK, use
      ** its name. Otherwise, use "rbu_rowid".  */
      if( pIter->eType==RBU_PK_IPK ){
        int i;
        for(i=0; pIter->abTblPk[i]==0; i++);
        assert( i<pIter->nTblCol );
        zCol = pIter->azTblCol[i];
      }else{
        zCol = "rbu_rowid";
      }
      zType = "INTEGER";
    }else{
      zCol = pIter->azTblCol[iCid];
      zType = pIter->azTblType[iCid];
    }

    zRet = sqlite3_mprintf("%z%s\"%w\" COLLATE %Q", zRet, zCom, zCol, zCollate);
    if( pIter->bUnique==0 || sqlite3_column_int(pXInfo, 5) ){
      const char *zOrder = (bDesc ? " DESC" : "");
      zImpPK = sqlite3_mprintf("%z%s\"rbu_imp_%d%w\"%s", 
          zImpPK, zCom, nBind, zCol, zOrder
      );
    }
    zImpCols = sqlite3_mprintf("%z%s\"rbu_imp_%d%w\" %s COLLATE %Q", 
        zImpCols, zCom, nBind, zCol, zType, zCollate
    );
    zWhere = sqlite3_mprintf(
        "%z%s\"rbu_imp_%d%w\" IS ?", zWhere, zAnd, nBind, zCol
    );
    if( zRet==0 || zImpPK==0 || zImpCols==0 || zWhere==0 ) rc = SQLITE_NOMEM;
    zCom = ", ";
    zAnd = " AND ";
    nBind++;
  }

  rc2 = sqlite3_finalize(pXInfo);
  if( rc==SQLITE_OK ) rc = rc2;

  if( rc!=SQLITE_OK ){
    sqlite3_free(zRet);
    sqlite3_free(zImpCols);
    sqlite3_free(zImpPK);
    sqlite3_free(zWhere);
    zRet = 0;
    zImpCols = 0;
    zImpPK = 0;
    zWhere = 0;
    p->rc = rc;
  }

  *pzImposterCols = zImpCols;
  *pzImposterPk = zImpPK;
  *pzWhere = zWhere;
  *pnBind = nBind;
  return zRet;
}

/*
** Assuming the current table columns are "a", "b" and "c", and the zObj
** paramter is passed "old", return a string of the form:
**
**     "old.a, old.b, old.b"
**
** With the column names escaped.
**
** For tables with implicit rowids - RBU_PK_EXTERNAL and RBU_PK_NONE, append
** the text ", old._rowid_" to the returned value.
*/
static char *rbuObjIterGetOldlist(
  sqlite3rbu *p, 
  RbuObjIter *pIter,
  const char *zObj
){
  char *zList = 0;
  if( p->rc==SQLITE_OK && pIter->abIndexed ){
    const char *zS = "";
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( pIter->abIndexed[i] ){
        const char *zCol = pIter->azTblCol[i];
        zList = sqlite3_mprintf("%z%s%s.\"%w\"", zList, zS, zObj, zCol);
      }else{
        zList = sqlite3_mprintf("%z%sNULL", zList, zS);
      }
      zS = ", ";
      if( zList==0 ){
        p->rc = SQLITE_NOMEM;
        break;
      }
    }

    /* For a table with implicit rowids, append "old._rowid_" to the list. */
    if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
      zList = rbuMPrintf(p, "%z, %s._rowid_", zList, zObj);
    }
  }
  return zList;
}

/*
** Return an expression that can be used in a WHERE clause to match the
** primary key of the current table. For example, if the table is:
**
**   CREATE TABLE t1(a, b, c, PRIMARY KEY(b, c));
**
** Return the string:
**
**   "b = ?1 AND c = ?2"
*/
static char *rbuObjIterGetWhere(
  sqlite3rbu *p, 
  RbuObjIter *pIter
){
  char *zList = 0;
  if( pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE ){
    zList = rbuMPrintf(p, "_rowid_ = ?%d", pIter->nTblCol+1);
  }else if( pIter->eType==RBU_PK_EXTERNAL ){
    const char *zSep = "";
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( pIter->abTblPk[i] ){
        zList = rbuMPrintf(p, "%z%sc%d=?%d", zList, zSep, i, i+1);
        zSep = " AND ";
      }
    }
    zList = rbuMPrintf(p, 
        "_rowid_ = (SELECT id FROM rbu_imposter2 WHERE %z)", zList
    );

  }else{
    const char *zSep = "";
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( pIter->abTblPk[i] ){
        const char *zCol = pIter->azTblCol[i];
        zList = rbuMPrintf(p, "%z%s\"%w\"=?%d", zList, zSep, zCol, i+1);
        zSep = " AND ";
      }
    }
  }
  return zList;
}

/*
** The SELECT statement iterating through the keys for the current object
** (p->objiter.pSelect) currently points to a valid row. However, there
** is something wrong with the rbu_control value in the rbu_control value
** stored in the (p->nCol+1)'th column. Set the error code and error message
** of the RBU handle to something reflecting this.
*/
static void rbuBadControlError(sqlite3rbu *p){
  p->rc = SQLITE_ERROR;
  p->zErrmsg = sqlite3_mprintf("invalid rbu_control value");
}


/*
** Return a nul-terminated string containing the comma separated list of
** assignments that should be included following the "SET" keyword of
** an UPDATE statement used to update the table object that the iterator
** passed as the second argument currently points to if the rbu_control
** column of the data_xxx table entry is set to zMask.
**
** The memory for the returned string is obtained from sqlite3_malloc().
** It is the responsibility of the caller to eventually free it using
** sqlite3_free(). 
**
** If an OOM error is encountered when allocating space for the new
** string, an error code is left in the rbu handle passed as the first
** argument and NULL is returned. Or, if an error has already occurred
** when this function is called, NULL is returned immediately, without
** attempting the allocation or modifying the stored error code.
*/
static char *rbuObjIterGetSetlist(
  sqlite3rbu *p,
  RbuObjIter *pIter,
  const char *zMask
){
  char *zList = 0;
  if( p->rc==SQLITE_OK ){
    int i;

    if( (int)strlen(zMask)!=pIter->nTblCol ){
      rbuBadControlError(p);
    }else{
      const char *zSep = "";
      for(i=0; i<pIter->nTblCol; i++){
        char c = zMask[pIter->aiSrcOrder[i]];
        if( c=='x' ){
          zList = rbuMPrintf(p, "%z%s\"%w\"=?%d", 
              zList, zSep, pIter->azTblCol[i], i+1
          );
          zSep = ", ";
        }
        else if( c=='d' ){
          zList = rbuMPrintf(p, "%z%s\"%w\"=rbu_delta(\"%w\", ?%d)", 
              zList, zSep, pIter->azTblCol[i], pIter->azTblCol[i], i+1
          );
          zSep = ", ";
        }
        else if( c=='f' ){
          zList = rbuMPrintf(p, "%z%s\"%w\"=rbu_fossil_delta(\"%w\", ?%d)", 
              zList, zSep, pIter->azTblCol[i], pIter->azTblCol[i], i+1
          );
          zSep = ", ";
        }
      }
    }
  }
  return zList;
}

/*
** Return a nul-terminated string consisting of nByte comma separated
** "?" expressions. For example, if nByte is 3, return a pointer to
** a buffer containing the string "?,?,?".
**
** The memory for the returned string is obtained from sqlite3_malloc().
** It is the responsibility of the caller to eventually free it using
** sqlite3_free(). 
**
** If an OOM error is encountered when allocating space for the new
** string, an error code is left in the rbu handle passed as the first
** argument and NULL is returned. Or, if an error has already occurred
** when this function is called, NULL is returned immediately, without
** attempting the allocation or modifying the stored error code.
*/
static char *rbuObjIterGetBindlist(sqlite3rbu *p, int nBind){
  char *zRet = 0;
  int nByte = nBind*2 + 1;

  zRet = (char*)rbuMalloc(p, nByte);
  if( zRet ){
    int i;
    for(i=0; i<nBind; i++){
      zRet[i*2] = '?';
      zRet[i*2+1] = (i+1==nBind) ? '\0' : ',';
    }
  }
  return zRet;
}

/*
** The iterator currently points to a table (not index) of type 
** RBU_PK_WITHOUT_ROWID. This function creates the PRIMARY KEY 
** declaration for the corresponding imposter table. For example,
** if the iterator points to a table created as:
**
**   CREATE TABLE t1(a, b, c, PRIMARY KEY(b, a DESC)) WITHOUT ROWID
**
** this function returns:
**
**   PRIMARY KEY("b", "a" DESC)
*/
static char *rbuWithoutRowidPK(sqlite3rbu *p, RbuObjIter *pIter){
  char *z = 0;
  assert( pIter->zIdx==0 );
  if( p->rc==SQLITE_OK ){
    const char *zSep = "PRIMARY KEY(";
    sqlite3_stmt *pXList = 0;     /* PRAGMA index_list = (pIter->zTbl) */
    sqlite3_stmt *pXInfo = 0;     /* PRAGMA index_xinfo = <pk-index> */
   
    p->rc = prepareFreeAndCollectError(p->dbMain, &pXList, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_list = %Q", pIter->zTbl)
    );
    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXList) ){
      const char *zOrig = (const char*)sqlite3_column_text(pXList,3);
      if( zOrig && strcmp(zOrig, "pk")==0 ){
        const char *zIdx = (const char*)sqlite3_column_text(pXList,1);
        if( zIdx ){
          p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
              sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", zIdx)
          );
        }
        break;
      }
    }
    rbuFinalize(p, pXList);

    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
      if( sqlite3_column_int(pXInfo, 5) ){
        /* int iCid = sqlite3_column_int(pXInfo, 0); */
        const char *zCol = (const char*)sqlite3_column_text(pXInfo, 2);
        const char *zDesc = sqlite3_column_int(pXInfo, 3) ? " DESC" : "";
        z = rbuMPrintf(p, "%z%s\"%w\"%s", z, zSep, zCol, zDesc);
        zSep = ", ";
      }
    }
    z = rbuMPrintf(p, "%z)", z);
    rbuFinalize(p, pXInfo);
  }
  return z;
}

/*
** This function creates the second imposter table used when writing to
** a table b-tree where the table has an external primary key. If the
** iterator passed as the second argument does not currently point to
** a table (not index) with an external primary key, this function is a
** no-op. 
**
** Assuming the iterator does point to a table with an external PK, this
** function creates a WITHOUT ROWID imposter table named "rbu_imposter2"
** used to access that PK index. For example, if the target table is
** declared as follows:
**
**   CREATE TABLE t1(a, b TEXT, c REAL, PRIMARY KEY(b, c));
**
** then the imposter table schema is:
**
**   CREATE TABLE rbu_imposter2(c1 TEXT, c2 REAL, id INTEGER) WITHOUT ROWID;
**
*/
static void rbuCreateImposterTable2(sqlite3rbu *p, RbuObjIter *pIter){
  if( p->rc==SQLITE_OK && pIter->eType==RBU_PK_EXTERNAL ){
    int tnum = pIter->iPkTnum;    /* Root page of PK index */
    sqlite3_stmt *pQuery = 0;     /* SELECT name ... WHERE rootpage = $tnum */
    const char *zIdx = 0;         /* Name of PK index */
    sqlite3_stmt *pXInfo = 0;     /* PRAGMA main.index_xinfo = $zIdx */
    const char *zComma = "";
    char *zCols = 0;              /* Used to build up list of table cols */
    char *zPk = 0;                /* Used to build up table PK declaration */

    /* Figure out the name of the primary key index for the current table.
    ** This is needed for the argument to "PRAGMA index_xinfo". Set
    ** zIdx to point to a nul-terminated string containing this name. */
    p->rc = prepareAndCollectError(p->dbMain, &pQuery, &p->zErrmsg, 
        "SELECT name FROM sqlite_master WHERE rootpage = ?"
    );
    if( p->rc==SQLITE_OK ){
      sqlite3_bind_int(pQuery, 1, tnum);
      if( SQLITE_ROW==sqlite3_step(pQuery) ){
        zIdx = (const char*)sqlite3_column_text(pQuery, 0);
      }
    }
    if( zIdx ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
          sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", zIdx)
      );
    }
    rbuFinalize(p, pQuery);

    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
      int bKey = sqlite3_column_int(pXInfo, 5);
      if( bKey ){
        int iCid = sqlite3_column_int(pXInfo, 1);
        int bDesc = sqlite3_column_int(pXInfo, 3);
        const char *zCollate = (const char*)sqlite3_column_text(pXInfo, 4);
        zCols = rbuMPrintf(p, "%z%sc%d %s COLLATE %s", zCols, zComma, 
            iCid, pIter->azTblType[iCid], zCollate
        );
        zPk = rbuMPrintf(p, "%z%sc%d%s", zPk, zComma, iCid, bDesc?" DESC":"");
        zComma = ", ";
      }
    }
    zCols = rbuMPrintf(p, "%z, id INTEGER", zCols);
    rbuFinalize(p, pXInfo);

    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 1, tnum);
    rbuMPrintfExec(p, p->dbMain,
        "CREATE TABLE rbu_imposter2(%z, PRIMARY KEY(%z)) WITHOUT ROWID", 
        zCols, zPk
    );
    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 0);
  }
}

/*
** If an error has already occurred when this function is called, it 
** immediately returns zero (without doing any work). Or, if an error
** occurs during the execution of this function, it sets the error code
** in the sqlite3rbu object indicated by the first argument and returns
** zero.
**
** The iterator passed as the second argument is guaranteed to point to
** a table (not an index) when this function is called. This function
** attempts to create any imposter table required to write to the main
** table b-tree of the table before returning. Non-zero is returned if
** an imposter table are created, or zero otherwise.
**
** An imposter table is required in all cases except RBU_PK_VTAB. Only
** virtual tables are written to directly. The imposter table has the 
** same schema as the actual target table (less any UNIQUE constraints). 
** More precisely, the "same schema" means the same columns, types, 
** collation sequences. For tables that do not have an external PRIMARY
** KEY, it also means the same PRIMARY KEY declaration.
*/
static void rbuCreateImposterTable(sqlite3rbu *p, RbuObjIter *pIter){
  if( p->rc==SQLITE_OK && pIter->eType!=RBU_PK_VTAB ){
    int tnum = pIter->iTnum;
    const char *zComma = "";
    char *zSql = 0;
    int iCol;
    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 1);

    for(iCol=0; p->rc==SQLITE_OK && iCol<pIter->nTblCol; iCol++){
      const char *zPk = "";
      const char *zCol = pIter->azTblCol[iCol];
      const char *zColl = 0;

      p->rc = sqlite3_table_column_metadata(
          p->dbMain, "main", pIter->zTbl, zCol, 0, &zColl, 0, 0, 0
      );

      if( pIter->eType==RBU_PK_IPK && pIter->abTblPk[iCol] ){
        /* If the target table column is an "INTEGER PRIMARY KEY", add
        ** "PRIMARY KEY" to the imposter table column declaration. */
        zPk = "PRIMARY KEY ";
      }
      zSql = rbuMPrintf(p, "%z%s\"%w\" %s %sCOLLATE %s%s", 
          zSql, zComma, zCol, pIter->azTblType[iCol], zPk, zColl,
          (pIter->abNotNull[iCol] ? " NOT NULL" : "")
      );
      zComma = ", ";
    }

    if( pIter->eType==RBU_PK_WITHOUT_ROWID ){
      char *zPk = rbuWithoutRowidPK(p, pIter);
      if( zPk ){
        zSql = rbuMPrintf(p, "%z, %z", zSql, zPk);
      }
    }

    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 1, tnum);
    rbuMPrintfExec(p, p->dbMain, "CREATE TABLE \"rbu_imp_%w\"(%z)%s", 
        pIter->zTbl, zSql, 
        (pIter->eType==RBU_PK_WITHOUT_ROWID ? " WITHOUT ROWID" : "")
    );
    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 0);
  }
}

/*
** Prepare a statement used to insert rows into the "rbu_tmp_xxx" table.
** Specifically a statement of the form:
**
**     INSERT INTO rbu_tmp_xxx VALUES(?, ?, ? ...);
**
** The number of bound variables is equal to the number of columns in
** the target table, plus one (for the rbu_control column), plus one more 
** (for the rbu_rowid column) if the target table is an implicit IPK or 
** virtual table.
*/
static void rbuObjIterPrepareTmpInsert(
  sqlite3rbu *p, 
  RbuObjIter *pIter,
  const char *zCollist,
  const char *zRbuRowid
){
  int bRbuRowid = (pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE);
  char *zBind = rbuObjIterGetBindlist(p, pIter->nTblCol + 1 + bRbuRowid);
  if( zBind ){
    assert( pIter->pTmpInsert==0 );
    p->rc = prepareFreeAndCollectError(
        p->dbRbu, &pIter->pTmpInsert, &p->zErrmsg, sqlite3_mprintf(
          "INSERT INTO %s.'rbu_tmp_%q'(rbu_control,%s%s) VALUES(%z)", 
          p->zStateDb, pIter->zDataTbl, zCollist, zRbuRowid, zBind
    ));
  }
}

static void rbuTmpInsertFunc(
  sqlite3_context *pCtx, 
  int nVal,
  sqlite3_value **apVal
){
  sqlite3rbu *p = sqlite3_user_data(pCtx);
  int rc = SQLITE_OK;
  int i;

  for(i=0; rc==SQLITE_OK && i<nVal; i++){
    rc = sqlite3_bind_value(p->objiter.pTmpInsert, i+1, apVal[i]);
  }
  if( rc==SQLITE_OK ){
    sqlite3_step(p->objiter.pTmpInsert);
    rc = sqlite3_reset(p->objiter.pTmpInsert);
  }

  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx, rc);
  }
}

/*
** Ensure that the SQLite statement handles required to update the 
** target database object currently indicated by the iterator passed 
** as the second argument are available.
*/
static int rbuObjIterPrepareAll(
  sqlite3rbu *p, 
  RbuObjIter *pIter,
  int nOffset                     /* Add "LIMIT -1 OFFSET $nOffset" to SELECT */
){
  assert( pIter->bCleanup==0 );
  if( pIter->pSelect==0 && rbuObjIterCacheTableInfo(p, pIter)==SQLITE_OK ){
    const int tnum = pIter->iTnum;
    char *zCollist = 0;           /* List of indexed columns */
    char **pz = &p->zErrmsg;
    const char *zIdx = pIter->zIdx;
    char *zLimit = 0;

    if( nOffset ){
      zLimit = sqlite3_mprintf(" LIMIT -1 OFFSET %d", nOffset);
      if( !zLimit ) p->rc = SQLITE_NOMEM;
    }

    if( zIdx ){
      const char *zTbl = pIter->zTbl;
      char *zImposterCols = 0;    /* Columns for imposter table */
      char *zImposterPK = 0;      /* Primary key declaration for imposter */
      char *zWhere = 0;           /* WHERE clause on PK columns */
      char *zBind = 0;
      int nBind = 0;

      assert( pIter->eType!=RBU_PK_VTAB );
      zCollist = rbuObjIterGetIndexCols(
          p, pIter, &zImposterCols, &zImposterPK, &zWhere, &nBind
      );
      zBind = rbuObjIterGetBindlist(p, nBind);

      /* Create the imposter table used to write to this index. */
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 1);
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 1,tnum);
      rbuMPrintfExec(p, p->dbMain,
          "CREATE TABLE \"rbu_imp_%w\"( %s, PRIMARY KEY( %s ) ) WITHOUT ROWID",
          zTbl, zImposterCols, zImposterPK
      );
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 0);

      /* Create the statement to insert index entries */
      pIter->nCol = nBind;
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(
            p->dbMain, &pIter->pInsert, &p->zErrmsg,
          sqlite3_mprintf("INSERT INTO \"rbu_imp_%w\" VALUES(%s)", zTbl, zBind)
        );
      }

      /* And to delete index entries */
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(
            p->dbMain, &pIter->pDelete, &p->zErrmsg,
          sqlite3_mprintf("DELETE FROM \"rbu_imp_%w\" WHERE %s", zTbl, zWhere)
        );
      }

      /* Create the SELECT statement to read keys in sorted order */
      if( p->rc==SQLITE_OK ){
        char *zSql;
        if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
          zSql = sqlite3_mprintf(
              "SELECT %s, rbu_control FROM %s.'rbu_tmp_%q' ORDER BY %s%s",
              zCollist, p->zStateDb, pIter->zDataTbl,
              zCollist, zLimit
          );
        }else{
          zSql = sqlite3_mprintf(
              "SELECT %s, rbu_control FROM '%q' "
              "WHERE typeof(rbu_control)='integer' AND rbu_control!=1 "
              "UNION ALL "
              "SELECT %s, rbu_control FROM %s.'rbu_tmp_%q' "
              "ORDER BY %s%s",
              zCollist, pIter->zDataTbl, 
              zCollist, p->zStateDb, pIter->zDataTbl, 
              zCollist, zLimit
          );
        }
        p->rc = prepareFreeAndCollectError(p->dbRbu, &pIter->pSelect, pz, zSql);
      }

      sqlite3_free(zImposterCols);
      sqlite3_free(zImposterPK);
      sqlite3_free(zWhere);
      sqlite3_free(zBind);
    }else{
      int bRbuRowid = (pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE);
      const char *zTbl = pIter->zTbl;       /* Table this step applies to */
      const char *zWrite;                   /* Imposter table name */

      char *zBindings = rbuObjIterGetBindlist(p, pIter->nTblCol + bRbuRowid);
      char *zWhere = rbuObjIterGetWhere(p, pIter);
      char *zOldlist = rbuObjIterGetOldlist(p, pIter, "old");
      char *zNewlist = rbuObjIterGetOldlist(p, pIter, "new");

      zCollist = rbuObjIterGetCollist(p, pIter);
      pIter->nCol = pIter->nTblCol;

      /* Create the imposter table or tables (if required). */
      rbuCreateImposterTable(p, pIter);
      rbuCreateImposterTable2(p, pIter);
      zWrite = (pIter->eType==RBU_PK_VTAB ? "" : "rbu_imp_");

      /* Create the INSERT statement to write to the target PK b-tree */
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(p->dbMain, &pIter->pInsert, pz,
            sqlite3_mprintf(
              "INSERT INTO \"%s%w\"(%s%s) VALUES(%s)", 
              zWrite, zTbl, zCollist, (bRbuRowid ? ", _rowid_" : ""), zBindings
            )
        );
      }

      /* Create the DELETE statement to write to the target PK b-tree */
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(p->dbMain, &pIter->pDelete, pz,
            sqlite3_mprintf(
              "DELETE FROM \"%s%w\" WHERE %s", zWrite, zTbl, zWhere
            )
        );
      }

      if( pIter->abIndexed ){
        const char *zRbuRowid = "";
        if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
          zRbuRowid = ", rbu_rowid";
        }

        /* Create the rbu_tmp_xxx table and the triggers to populate it. */
        rbuMPrintfExec(p, p->dbRbu,
            "CREATE TABLE IF NOT EXISTS %s.'rbu_tmp_%q' AS "
            "SELECT *%s FROM '%q' WHERE 0;"
            , p->zStateDb, pIter->zDataTbl
            , (pIter->eType==RBU_PK_EXTERNAL ? ", 0 AS rbu_rowid" : "")
            , pIter->zDataTbl
        );

        rbuMPrintfExec(p, p->dbMain,
            "CREATE TEMP TRIGGER rbu_delete_tr BEFORE DELETE ON \"%s%w\" "
            "BEGIN "
            "  SELECT rbu_tmp_insert(2, %s);"
            "END;"

            "CREATE TEMP TRIGGER rbu_update1_tr BEFORE UPDATE ON \"%s%w\" "
            "BEGIN "
            "  SELECT rbu_tmp_insert(2, %s);"
            "END;"

            "CREATE TEMP TRIGGER rbu_update2_tr AFTER UPDATE ON \"%s%w\" "
            "BEGIN "
            "  SELECT rbu_tmp_insert(3, %s);"
            "END;",
            zWrite, zTbl, zOldlist,
            zWrite, zTbl, zOldlist,
            zWrite, zTbl, zNewlist
        );

        if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
          rbuMPrintfExec(p, p->dbMain,
              "CREATE TEMP TRIGGER rbu_insert_tr AFTER INSERT ON \"%s%w\" "
              "BEGIN "
              "  SELECT rbu_tmp_insert(0, %s);"
              "END;",
              zWrite, zTbl, zNewlist
          );
        }

        rbuObjIterPrepareTmpInsert(p, pIter, zCollist, zRbuRowid);
      }

      /* Create the SELECT statement to read keys from data_xxx */
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(p->dbRbu, &pIter->pSelect, pz,
            sqlite3_mprintf(
              "SELECT %s, rbu_control%s FROM '%q'%s", 
              zCollist, (bRbuRowid ? ", rbu_rowid" : ""), 
              pIter->zDataTbl, zLimit
            )
        );
      }

      sqlite3_free(zWhere);
      sqlite3_free(zOldlist);
      sqlite3_free(zNewlist);
      sqlite3_free(zBindings);
    }
    sqlite3_free(zCollist);
    sqlite3_free(zLimit);
  }
  
  return p->rc;
}

/*
** Set output variable *ppStmt to point to an UPDATE statement that may
** be used to update the imposter table for the main table b-tree of the
** table object that pIter currently points to, assuming that the 
** rbu_control column of the data_xyz table contains zMask.
** 
** If the zMask string does not specify any columns to update, then this
** is not an error. Output variable *ppStmt is set to NULL in this case.
*/
static int rbuGetUpdateStmt(
  sqlite3rbu *p,                  /* RBU handle */
  RbuObjIter *pIter,              /* Object iterator */
  const char *zMask,              /* rbu_control value ('x.x.') */
  sqlite3_stmt **ppStmt           /* OUT: UPDATE statement handle */
){
  RbuUpdateStmt **pp;
  RbuUpdateStmt *pUp = 0;
  int nUp = 0;

  /* In case an error occurs */
  *ppStmt = 0;

  /* Search for an existing statement. If one is found, shift it to the front
  ** of the LRU queue and return immediately. Otherwise, leave nUp pointing
  ** to the number of statements currently in the cache and pUp to the
  ** last object in the list.  */
  for(pp=&pIter->pRbuUpdate; *pp; pp=&((*pp)->pNext)){
    pUp = *pp;
    if( strcmp(pUp->zMask, zMask)==0 ){
      *pp = pUp->pNext;
      pUp->pNext = pIter->pRbuUpdate;
      pIter->pRbuUpdate = pUp;
      *ppStmt = pUp->pUpdate; 
      return SQLITE_OK;
    }
    nUp++;
  }
  assert( pUp==0 || pUp->pNext==0 );

  if( nUp>=SQLITE_RBU_UPDATE_CACHESIZE ){
    for(pp=&pIter->pRbuUpdate; *pp!=pUp; pp=&((*pp)->pNext));
    *pp = 0;
    sqlite3_finalize(pUp->pUpdate);
    pUp->pUpdate = 0;
  }else{
    pUp = (RbuUpdateStmt*)rbuMalloc(p, sizeof(RbuUpdateStmt)+pIter->nTblCol+1);
  }

  if( pUp ){
    char *zWhere = rbuObjIterGetWhere(p, pIter);
    char *zSet = rbuObjIterGetSetlist(p, pIter, zMask);
    char *zUpdate = 0;

    pUp->zMask = (char*)&pUp[1];
    memcpy(pUp->zMask, zMask, pIter->nTblCol);
    pUp->pNext = pIter->pRbuUpdate;
    pIter->pRbuUpdate = pUp;

    if( zSet ){
      const char *zPrefix = "";

      if( pIter->eType!=RBU_PK_VTAB ) zPrefix = "rbu_imp_";
      zUpdate = sqlite3_mprintf("UPDATE \"%s%w\" SET %s WHERE %s", 
          zPrefix, pIter->zTbl, zSet, zWhere
      );
      p->rc = prepareFreeAndCollectError(
          p->dbMain, &pUp->pUpdate, &p->zErrmsg, zUpdate
      );
      *ppStmt = pUp->pUpdate;
    }
    sqlite3_free(zWhere);
    sqlite3_free(zSet);
  }

  return p->rc;
}

static sqlite3 *rbuOpenDbhandle(sqlite3rbu *p, const char *zName){
  sqlite3 *db = 0;
  if( p->rc==SQLITE_OK ){
    const int flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_URI;
    p->rc = sqlite3_open_v2(zName, &db, flags, p->zVfsName);
    if( p->rc ){
      p->zErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
      sqlite3_close(db);
      db = 0;
    }
  }
  return db;
}

/*
** Open the database handle and attach the RBU database as "rbu". If an
** error occurs, leave an error code and message in the RBU handle.
*/
static void rbuOpenDatabase(sqlite3rbu *p){
  assert( p->rc==SQLITE_OK );
  assert( p->dbMain==0 && p->dbRbu==0 );

  p->eStage = 0;
  p->dbMain = rbuOpenDbhandle(p, p->zTarget);
  p->dbRbu = rbuOpenDbhandle(p, p->zRbu);

  /* If using separate RBU and state databases, attach the state database to
  ** the RBU db handle now.  */
  if( p->zState ){
    rbuMPrintfExec(p, p->dbRbu, "ATTACH %Q AS stat", p->zState);
    memcpy(p->zStateDb, "stat", 4);
  }else{
    memcpy(p->zStateDb, "main", 4);
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_create_function(p->dbMain, 
        "rbu_tmp_insert", -1, SQLITE_UTF8, (void*)p, rbuTmpInsertFunc, 0, 0
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_create_function(p->dbMain, 
        "rbu_fossil_delta", 2, SQLITE_UTF8, 0, rbuFossilDeltaFunc, 0, 0
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_create_function(p->dbRbu, 
        "rbu_target_name", 1, SQLITE_UTF8, (void*)p, rbuTargetNameFunc, 0, 0
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_file_control(p->dbMain, "main", SQLITE_FCNTL_RBU, (void*)p);
  }
  rbuMPrintfExec(p, p->dbMain, "SELECT * FROM sqlite_master");

  /* Mark the database file just opened as an RBU target database. If 
  ** this call returns SQLITE_NOTFOUND, then the RBU vfs is not in use.
  ** This is an error.  */
  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_file_control(p->dbMain, "main", SQLITE_FCNTL_RBU, (void*)p);
  }

  if( p->rc==SQLITE_NOTFOUND ){
    p->rc = SQLITE_ERROR;
    p->zErrmsg = sqlite3_mprintf("rbu vfs not found");
  }
}

/*
** This routine is a copy of the sqlite3FileSuffix3() routine from the core.
** It is a no-op unless SQLITE_ENABLE_8_3_NAMES is defined.
**
** If SQLITE_ENABLE_8_3_NAMES is set at compile-time and if the database
** filename in zBaseFilename is a URI with the "8_3_names=1" parameter and
** if filename in z[] has a suffix (a.k.a. "extension") that is longer than
** three characters, then shorten the suffix on z[] to be the last three
** characters of the original suffix.
**
** If SQLITE_ENABLE_8_3_NAMES is set to 2 at compile-time, then always
** do the suffix shortening regardless of URI parameter.
**
** Examples:
**
**     test.db-journal    =>   test.nal
**     test.db-wal        =>   test.wal
**     test.db-shm        =>   test.shm
**     test.db-mj7f3319fa =>   test.9fa
*/
static void rbuFileSuffix3(const char *zBase, char *z){
#ifdef SQLITE_ENABLE_8_3_NAMES
#if SQLITE_ENABLE_8_3_NAMES<2
  if( sqlite3_uri_boolean(zBase, "8_3_names", 0) )
#endif
  {
    int i, sz;
    sz = sqlite3Strlen30(z);
    for(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    if( z[i]=='.' && ALWAYS(sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
  }
#endif
}

/*
** Return the current wal-index header checksum for the target database 
** as a 64-bit integer.
**
** The checksum is store in the first page of xShmMap memory as an 8-byte 
** blob starting at byte offset 40.
*/
static i64 rbuShmChecksum(sqlite3rbu *p){
  i64 iRet = 0;
  if( p->rc==SQLITE_OK ){
    sqlite3_file *pDb = p->pTargetFd->pReal;
    u32 volatile *ptr;
    p->rc = pDb->pMethods->xShmMap(pDb, 0, 32*1024, 0, (void volatile**)&ptr);
    if( p->rc==SQLITE_OK ){
      iRet = ((i64)ptr[10] << 32) + ptr[11];
    }
  }
  return iRet;
}

/*
** This function is called as part of initializing or reinitializing an
** incremental checkpoint. 
**
** It populates the sqlite3rbu.aFrame[] array with the set of 
** (wal frame -> db page) copy operations required to checkpoint the 
** current wal file, and obtains the set of shm locks required to safely 
** perform the copy operations directly on the file-system.
**
** If argument pState is not NULL, then the incremental checkpoint is
** being resumed. In this case, if the checksum of the wal-index-header
** following recovery is not the same as the checksum saved in the RbuState
** object, then the rbu handle is set to DONE state. This occurs if some
** other client appends a transaction to the wal file in the middle of
** an incremental checkpoint.
*/
static void rbuSetupCheckpoint(sqlite3rbu *p, RbuState *pState){

  /* If pState is NULL, then the wal file may not have been opened and
  ** recovered. Running a read-statement here to ensure that doing so
  ** does not interfere with the "capture" process below.  */
  if( pState==0 ){
    p->eStage = 0;
    if( p->rc==SQLITE_OK ){
      p->rc = sqlite3_exec(p->dbMain, "SELECT * FROM sqlite_master", 0, 0, 0);
    }
  }

  /* Assuming no error has occurred, run a "restart" checkpoint with the
  ** sqlite3rbu.eStage variable set to CAPTURE. This turns on the following
  ** special behaviour in the rbu VFS:
  **
  **   * If the exclusive shm WRITER or READ0 lock cannot be obtained,
  **     the checkpoint fails with SQLITE_BUSY (normally SQLite would
  **     proceed with running a passive checkpoint instead of failing).
  **
  **   * Attempts to read from the *-wal file or write to the database file
  **     do not perform any IO. Instead, the frame/page combinations that
  **     would be read/written are recorded in the sqlite3rbu.aFrame[]
  **     array.
  **
  **   * Calls to xShmLock(UNLOCK) to release the exclusive shm WRITER, 
  **     READ0 and CHECKPOINT locks taken as part of the checkpoint are
  **     no-ops. These locks will not be released until the connection
  **     is closed.
  **
  **   * Attempting to xSync() the database file causes an SQLITE_INTERNAL 
  **     error.
  **
  ** As a result, unless an error (i.e. OOM or SQLITE_BUSY) occurs, the
  ** checkpoint below fails with SQLITE_INTERNAL, and leaves the aFrame[]
  ** array populated with a set of (frame -> page) mappings. Because the 
  ** WRITER, CHECKPOINT and READ0 locks are still held, it is safe to copy 
  ** data from the wal file into the database file according to the 
  ** contents of aFrame[].
  */
  if( p->rc==SQLITE_OK ){
    int rc2;
    p->eStage = RBU_STAGE_CAPTURE;
    rc2 = sqlite3_exec(p->dbMain, "PRAGMA main.wal_checkpoint=restart", 0, 0,0);
    if( rc2!=SQLITE_INTERNAL ) p->rc = rc2;
  }

  if( p->rc==SQLITE_OK ){
    p->eStage = RBU_STAGE_CKPT;
    p->nStep = (pState ? pState->nRow : 0);
    p->aBuf = rbuMalloc(p, p->pgsz);
    p->iWalCksum = rbuShmChecksum(p);
  }

  if( p->rc==SQLITE_OK && pState && pState->iWalCksum!=p->iWalCksum ){
    p->rc = SQLITE_DONE;
    p->eStage = RBU_STAGE_DONE;
  }
}

/*
** Called when iAmt bytes are read from offset iOff of the wal file while
** the rbu object is in capture mode. Record the frame number of the frame
** being read in the aFrame[] array.
*/
static int rbuCaptureWalRead(sqlite3rbu *pRbu, i64 iOff, int iAmt){
  const u32 mReq = (1<<WAL_LOCK_WRITE)|(1<<WAL_LOCK_CKPT)|(1<<WAL_LOCK_READ0);
  u32 iFrame;

  if( pRbu->mLock!=mReq ){
    pRbu->rc = SQLITE_BUSY;
    return SQLITE_INTERNAL;
  }

  pRbu->pgsz = iAmt;
  if( pRbu->nFrame==pRbu->nFrameAlloc ){
    int nNew = (pRbu->nFrameAlloc ? pRbu->nFrameAlloc : 64) * 2;
    RbuFrame *aNew;
    aNew = (RbuFrame*)sqlite3_realloc(pRbu->aFrame, nNew * sizeof(RbuFrame));
    if( aNew==0 ) return SQLITE_NOMEM;
    pRbu->aFrame = aNew;
    pRbu->nFrameAlloc = nNew;
  }

  iFrame = (u32)((iOff-32) / (i64)(iAmt+24)) + 1;
  if( pRbu->iMaxFrame<iFrame ) pRbu->iMaxFrame = iFrame;
  pRbu->aFrame[pRbu->nFrame].iWalFrame = iFrame;
  pRbu->aFrame[pRbu->nFrame].iDbPage = 0;
  pRbu->nFrame++;
  return SQLITE_OK;
}

/*
** Called when a page of data is written to offset iOff of the database
** file while the rbu handle is in capture mode. Record the page number 
** of the page being written in the aFrame[] array.
*/
static int rbuCaptureDbWrite(sqlite3rbu *pRbu, i64 iOff){
  pRbu->aFrame[pRbu->nFrame-1].iDbPage = (u32)(iOff / pRbu->pgsz) + 1;
  return SQLITE_OK;
}

/*
** This is called as part of an incremental checkpoint operation. Copy
** a single frame of data from the wal file into the database file, as
** indicated by the RbuFrame object.
*/
static void rbuCheckpointFrame(sqlite3rbu *p, RbuFrame *pFrame){
  sqlite3_file *pWal = p->pTargetFd->pWalFd->pReal;
  sqlite3_file *pDb = p->pTargetFd->pReal;
  i64 iOff;

  assert( p->rc==SQLITE_OK );
  iOff = (i64)(pFrame->iWalFrame-1) * (p->pgsz + 24) + 32 + 24;
  p->rc = pWal->pMethods->xRead(pWal, p->aBuf, p->pgsz, iOff);
  if( p->rc ) return;

  iOff = (i64)(pFrame->iDbPage-1) * p->pgsz;
  p->rc = pDb->pMethods->xWrite(pDb, p->aBuf, p->pgsz, iOff);
}


/*
** Take an EXCLUSIVE lock on the database file.
*/
static void rbuLockDatabase(sqlite3rbu *p){
  sqlite3_file *pReal = p->pTargetFd->pReal;
  assert( p->rc==SQLITE_OK );
  p->rc = pReal->pMethods->xLock(pReal, SQLITE_LOCK_SHARED);
  if( p->rc==SQLITE_OK ){
    p->rc = pReal->pMethods->xLock(pReal, SQLITE_LOCK_EXCLUSIVE);
  }
}

/*
** The RBU handle is currently in RBU_STAGE_OAL state, with a SHARED lock
** on the database file. This proc moves the *-oal file to the *-wal path,
** then reopens the database file (this time in vanilla, non-oal, WAL mode).
** If an error occurs, leave an error code and error message in the rbu 
** handle.
*/
static void rbuMoveOalFile(sqlite3rbu *p){
  const char *zBase = sqlite3_db_filename(p->dbMain, "main");

  char *zWal = sqlite3_mprintf("%s-wal", zBase);
  char *zOal = sqlite3_mprintf("%s-oal", zBase);

  assert( p->eStage==RBU_STAGE_MOVE );
  assert( p->rc==SQLITE_OK && p->zErrmsg==0 );
  if( zWal==0 || zOal==0 ){
    p->rc = SQLITE_NOMEM;
  }else{
    /* Move the *-oal file to *-wal. At this point connection p->db is
    ** holding a SHARED lock on the target database file (because it is
    ** in WAL mode). So no other connection may be writing the db. 
    **
    ** In order to ensure that there are no database readers, an EXCLUSIVE
    ** lock is obtained here before the *-oal is moved to *-wal.
    */
    rbuLockDatabase(p);
    if( p->rc==SQLITE_OK ){
      rbuFileSuffix3(zBase, zWal);
      rbuFileSuffix3(zBase, zOal);

      /* Re-open the databases. */
      rbuObjIterFinalize(&p->objiter);
      sqlite3_close(p->dbMain);
      sqlite3_close(p->dbRbu);
      p->rc = rename(zOal, zWal) ? SQLITE_IOERR : SQLITE_OK;
      if( p->rc==SQLITE_OK ){
        p->dbMain = 0;
        p->dbRbu = 0;
        rbuOpenDatabase(p);
        rbuSetupCheckpoint(p, 0);
      }
    }
  }

  sqlite3_free(zWal);
  sqlite3_free(zOal);
}

/*
** The SELECT statement iterating through the keys for the current object
** (p->objiter.pSelect) currently points to a valid row. This function
** determines the type of operation requested by this row and returns
** one of the following values to indicate the result:
**
**     * RBU_INSERT
**     * RBU_DELETE
**     * RBU_IDX_DELETE
**     * RBU_UPDATE
**
** If RBU_UPDATE is returned, then output variable *pzMask is set to
** point to the text value indicating the columns to update.
**
** If the rbu_control field contains an invalid value, an error code and
** message are left in the RBU handle and zero returned.
*/
static int rbuStepType(sqlite3rbu *p, const char **pzMask){
  int iCol = p->objiter.nCol;     /* Index of rbu_control column */
  int res = 0;                    /* Return value */

  switch( sqlite3_column_type(p->objiter.pSelect, iCol) ){
    case SQLITE_INTEGER: {
      int iVal = sqlite3_column_int(p->objiter.pSelect, iCol);
      if( iVal==0 ){
        res = RBU_INSERT;
      }else if( iVal==1 ){
        res = RBU_DELETE;
      }else if( iVal==2 ){
        res = RBU_IDX_DELETE;
      }else if( iVal==3 ){
        res = RBU_IDX_INSERT;
      }
      break;
    }

    case SQLITE_TEXT: {
      const unsigned char *z = sqlite3_column_text(p->objiter.pSelect, iCol);
      if( z==0 ){
        p->rc = SQLITE_NOMEM;
      }else{
        *pzMask = (const char*)z;
      }
      res = RBU_UPDATE;

      break;
    }

    default:
      break;
  }

  if( res==0 ){
    rbuBadControlError(p);
  }
  return res;
}

#ifdef SQLITE_DEBUG
/*
** Assert that column iCol of statement pStmt is named zName.
*/
static void assertColumnName(sqlite3_stmt *pStmt, int iCol, const char *zName){
  const char *zCol = sqlite3_column_name(pStmt, iCol);
  assert( 0==sqlite3_stricmp(zName, zCol) );
}
#else
# define assertColumnName(x,y,z)
#endif

/*
** This function does the work for an sqlite3rbu_step() call.
**
** The object-iterator (p->objiter) currently points to a valid object,
** and the input cursor (p->objiter.pSelect) currently points to a valid
** input row. Perform whatever processing is required and return.
**
** If no  error occurs, SQLITE_OK is returned. Otherwise, an error code
** and message is left in the RBU handle and a copy of the error code
** returned.
*/
static int rbuStep(sqlite3rbu *p){
  RbuObjIter *pIter = &p->objiter;
  const char *zMask = 0;
  int i;
  int eType = rbuStepType(p, &zMask);

  if( eType ){
    assert( eType!=RBU_UPDATE || pIter->zIdx==0 );

    if( pIter->zIdx==0 && eType==RBU_IDX_DELETE ){
      rbuBadControlError(p);
    }
    else if( 
        eType==RBU_INSERT 
     || eType==RBU_DELETE
     || eType==RBU_IDX_DELETE 
     || eType==RBU_IDX_INSERT
    ){
      sqlite3_value *pVal;
      sqlite3_stmt *pWriter;

      assert( eType!=RBU_UPDATE );
      assert( eType!=RBU_DELETE || pIter->zIdx==0 );

      if( eType==RBU_IDX_DELETE || eType==RBU_DELETE ){
        pWriter = pIter->pDelete;
      }else{
        pWriter = pIter->pInsert;
      }

      for(i=0; i<pIter->nCol; i++){
        /* If this is an INSERT into a table b-tree and the table has an
        ** explicit INTEGER PRIMARY KEY, check that this is not an attempt
        ** to write a NULL into the IPK column. That is not permitted.  */
        if( eType==RBU_INSERT 
         && pIter->zIdx==0 && pIter->eType==RBU_PK_IPK && pIter->abTblPk[i] 
         && sqlite3_column_type(pIter->pSelect, i)==SQLITE_NULL
        ){
          p->rc = SQLITE_MISMATCH;
          p->zErrmsg = sqlite3_mprintf("datatype mismatch");
          goto step_out;
        }

        if( eType==RBU_DELETE && pIter->abTblPk[i]==0 ){
          continue;
        }

        pVal = sqlite3_column_value(pIter->pSelect, i);
        p->rc = sqlite3_bind_value(pWriter, i+1, pVal);
        if( p->rc ) goto step_out;
      }
      if( pIter->zIdx==0
       && (pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE) 
      ){
        /* For a virtual table, or a table with no primary key, the 
        ** SELECT statement is:
        **
        **   SELECT <cols>, rbu_control, rbu_rowid FROM ....
        **
        ** Hence column_value(pIter->nCol+1).
        */
        assertColumnName(pIter->pSelect, pIter->nCol+1, "rbu_rowid");
        pVal = sqlite3_column_value(pIter->pSelect, pIter->nCol+1);
        p->rc = sqlite3_bind_value(pWriter, pIter->nCol+1, pVal);
      }
      if( p->rc==SQLITE_OK ){
        sqlite3_step(pWriter);
        p->rc = resetAndCollectError(pWriter, &p->zErrmsg);
      }
    }else{
      sqlite3_value *pVal;
      sqlite3_stmt *pUpdate = 0;
      assert( eType==RBU_UPDATE );
      rbuGetUpdateStmt(p, pIter, zMask, &pUpdate);
      if( pUpdate ){
        for(i=0; p->rc==SQLITE_OK && i<pIter->nCol; i++){
          char c = zMask[pIter->aiSrcOrder[i]];
          pVal = sqlite3_column_value(pIter->pSelect, i);
          if( pIter->abTblPk[i] || c!='.' ){
            p->rc = sqlite3_bind_value(pUpdate, i+1, pVal);
          }
        }
        if( p->rc==SQLITE_OK 
         && (pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE) 
        ){
          /* Bind the rbu_rowid value to column _rowid_ */
          assertColumnName(pIter->pSelect, pIter->nCol+1, "rbu_rowid");
          pVal = sqlite3_column_value(pIter->pSelect, pIter->nCol+1);
          p->rc = sqlite3_bind_value(pUpdate, pIter->nCol+1, pVal);
        }
        if( p->rc==SQLITE_OK ){
          sqlite3_step(pUpdate);
          p->rc = resetAndCollectError(pUpdate, &p->zErrmsg);
        }
      }
    }
  }

 step_out:
  return p->rc;
}

/*
** Increment the schema cookie of the main database opened by p->dbMain.
*/
static void rbuIncrSchemaCookie(sqlite3rbu *p){
  if( p->rc==SQLITE_OK ){
    int iCookie = 1000000;
    sqlite3_stmt *pStmt;

    p->rc = prepareAndCollectError(p->dbMain, &pStmt, &p->zErrmsg, 
        "PRAGMA schema_version"
    );
    if( p->rc==SQLITE_OK ){
      /* Coverage: it may be that this sqlite3_step() cannot fail. There
      ** is already a transaction open, so the prepared statement cannot
      ** throw an SQLITE_SCHEMA exception. The only database page the
      ** statement reads is page 1, which is guaranteed to be in the cache.
      ** And no memory allocations are required.  */
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        iCookie = sqlite3_column_int(pStmt, 0);
      }
      rbuFinalize(p, pStmt);
    }
    if( p->rc==SQLITE_OK ){
      rbuMPrintfExec(p, p->dbMain, "PRAGMA schema_version = %d", iCookie+1);
    }
  }
}

/*
** Update the contents of the rbu_state table within the rbu database. The
** value stored in the RBU_STATE_STAGE column is eStage. All other values
** are determined by inspecting the rbu handle passed as the first argument.
*/
static void rbuSaveState(sqlite3rbu *p, int eStage){
  if( p->rc==SQLITE_OK || p->rc==SQLITE_DONE ){
    sqlite3_stmt *pInsert = 0;
    int rc;

    assert( p->zErrmsg==0 );
    rc = prepareFreeAndCollectError(p->dbRbu, &pInsert, &p->zErrmsg, 
        sqlite3_mprintf(
          "INSERT OR REPLACE INTO %s.rbu_state(k, v) VALUES "
          "(%d, %d), "
          "(%d, %Q), "
          "(%d, %Q), "
          "(%d, %d), "
          "(%d, %d), "
          "(%d, %lld), "
          "(%d, %lld), "
          "(%d, %lld) ",
          p->zStateDb,
          RBU_STATE_STAGE, eStage,
          RBU_STATE_TBL, p->objiter.zTbl, 
          RBU_STATE_IDX, p->objiter.zIdx, 
          RBU_STATE_ROW, p->nStep, 
          RBU_STATE_PROGRESS, p->nProgress,
          RBU_STATE_CKPT, p->iWalCksum,
          RBU_STATE_COOKIE, (i64)p->pTargetFd->iCookie,
          RBU_STATE_OALSZ, p->iOalSz
      )
    );
    assert( pInsert==0 || rc==SQLITE_OK );

    if( rc==SQLITE_OK ){
      sqlite3_step(pInsert);
      rc = sqlite3_finalize(pInsert);
    }
    if( rc!=SQLITE_OK ) p->rc = rc;
  }
}


/*
** Step the RBU object.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_step(sqlite3rbu *p){
  if( p ){
    switch( p->eStage ){
      case RBU_STAGE_OAL: {
        RbuObjIter *pIter = &p->objiter;
        while( p->rc==SQLITE_OK && pIter->zTbl ){

          if( pIter->bCleanup ){
            /* Clean up the rbu_tmp_xxx table for the previous table. It 
            ** cannot be dropped as there are currently active SQL statements.
            ** But the contents can be deleted.  */
            if( pIter->abIndexed ){
              rbuMPrintfExec(p, p->dbRbu, 
                  "DELETE FROM %s.'rbu_tmp_%q'", p->zStateDb, pIter->zDataTbl
              );
            }
          }else{
            rbuObjIterPrepareAll(p, pIter, 0);

            /* Advance to the next row to process. */
            if( p->rc==SQLITE_OK ){
              int rc = sqlite3_step(pIter->pSelect);
              if( rc==SQLITE_ROW ){
                p->nProgress++;
                p->nStep++;
                return rbuStep(p);
              }
              p->rc = sqlite3_reset(pIter->pSelect);
              p->nStep = 0;
            }
          }

          rbuObjIterNext(p, pIter);
        }

        if( p->rc==SQLITE_OK ){
          assert( pIter->zTbl==0 );
          rbuSaveState(p, RBU_STAGE_MOVE);
          rbuIncrSchemaCookie(p);
          if( p->rc==SQLITE_OK ){
            p->rc = sqlite3_exec(p->dbMain, "COMMIT", 0, 0, &p->zErrmsg);
          }
          if( p->rc==SQLITE_OK ){
            p->rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, &p->zErrmsg);
          }
          p->eStage = RBU_STAGE_MOVE;
        }
        break;
      }

      case RBU_STAGE_MOVE: {
        if( p->rc==SQLITE_OK ){
          rbuMoveOalFile(p);
          p->nProgress++;
        }
        break;
      }

      case RBU_STAGE_CKPT: {
        if( p->rc==SQLITE_OK ){
          if( p->nStep>=p->nFrame ){
            sqlite3_file *pDb = p->pTargetFd->pReal;
  
            /* Sync the db file */
            p->rc = pDb->pMethods->xSync(pDb, SQLITE_SYNC_NORMAL);
  
            /* Update nBackfill */
            if( p->rc==SQLITE_OK ){
              void volatile *ptr;
              p->rc = pDb->pMethods->xShmMap(pDb, 0, 32*1024, 0, &ptr);
              if( p->rc==SQLITE_OK ){
                ((u32 volatile*)ptr)[24] = p->iMaxFrame;
              }
            }
  
            if( p->rc==SQLITE_OK ){
              p->eStage = RBU_STAGE_DONE;
              p->rc = SQLITE_DONE;
            }
          }else{
            RbuFrame *pFrame = &p->aFrame[p->nStep];
            rbuCheckpointFrame(p, pFrame);
            p->nStep++;
          }
          p->nProgress++;
        }
        break;
      }

      default:
        break;
    }
    return p->rc;
  }else{
    return SQLITE_NOMEM;
  }
}

/*
** Free an RbuState object allocated by rbuLoadState().
*/
static void rbuFreeState(RbuState *p){
  if( p ){
    sqlite3_free(p->zTbl);
    sqlite3_free(p->zIdx);
    sqlite3_free(p);
  }
}

/*
** Allocate an RbuState object and load the contents of the rbu_state 
** table into it. Return a pointer to the new object. It is the 
** responsibility of the caller to eventually free the object using
** sqlite3_free().
**
** If an error occurs, leave an error code and message in the rbu handle
** and return NULL.
*/
static RbuState *rbuLoadState(sqlite3rbu *p){
  RbuState *pRet = 0;
  sqlite3_stmt *pStmt = 0;
  int rc;
  int rc2;

  pRet = (RbuState*)rbuMalloc(p, sizeof(RbuState));
  if( pRet==0 ) return 0;

  rc = prepareFreeAndCollectError(p->dbRbu, &pStmt, &p->zErrmsg, 
      sqlite3_mprintf("SELECT k, v FROM %s.rbu_state", p->zStateDb)
  );
  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
    switch( sqlite3_column_int(pStmt, 0) ){
      case RBU_STATE_STAGE:
        pRet->eStage = sqlite3_column_int(pStmt, 1);
        if( pRet->eStage!=RBU_STAGE_OAL
         && pRet->eStage!=RBU_STAGE_MOVE
         && pRet->eStage!=RBU_STAGE_CKPT
        ){
          p->rc = SQLITE_CORRUPT;
        }
        break;

      case RBU_STATE_TBL:
        pRet->zTbl = rbuStrndup((char*)sqlite3_column_text(pStmt, 1), &rc);
        break;

      case RBU_STATE_IDX:
        pRet->zIdx = rbuStrndup((char*)sqlite3_column_text(pStmt, 1), &rc);
        break;

      case RBU_STATE_ROW:
        pRet->nRow = sqlite3_column_int(pStmt, 1);
        break;

      case RBU_STATE_PROGRESS:
        pRet->nProgress = sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_CKPT:
        pRet->iWalCksum = sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_COOKIE:
        pRet->iCookie = (u32)sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_OALSZ:
        pRet->iOalSz = (u32)sqlite3_column_int64(pStmt, 1);
        break;

      default:
        rc = SQLITE_CORRUPT;
        break;
    }
  }
  rc2 = sqlite3_finalize(pStmt);
  if( rc==SQLITE_OK ) rc = rc2;

  p->rc = rc;
  return pRet;
}

/*
** Compare strings z1 and z2, returning 0 if they are identical, or non-zero
** otherwise. Either or both argument may be NULL. Two NULL values are
** considered equal, and NULL is considered distinct from all other values.
*/
static int rbuStrCompare(const char *z1, const char *z2){
  if( z1==0 && z2==0 ) return 0;
  if( z1==0 || z2==0 ) return 1;
  return (sqlite3_stricmp(z1, z2)!=0);
}

/*
** This function is called as part of sqlite3rbu_open() when initializing
** an rbu handle in OAL stage. If the rbu update has not started (i.e.
** the rbu_state table was empty) it is a no-op. Otherwise, it arranges
** things so that the next call to sqlite3rbu_step() continues on from
** where the previous rbu handle left off.
**
** If an error occurs, an error code and error message are left in the
** rbu handle passed as the first argument.
*/
static void rbuSetupOal(sqlite3rbu *p, RbuState *pState){
  assert( p->rc==SQLITE_OK );
  if( pState->zTbl ){
    RbuObjIter *pIter = &p->objiter;
    int rc = SQLITE_OK;

    while( rc==SQLITE_OK && pIter->zTbl && (pIter->bCleanup 
       || rbuStrCompare(pIter->zIdx, pState->zIdx)
       || rbuStrCompare(pIter->zTbl, pState->zTbl) 
    )){
      rc = rbuObjIterNext(p, pIter);
    }

    if( rc==SQLITE_OK && !pIter->zTbl ){
      rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf("rbu_state mismatch error");
    }

    if( rc==SQLITE_OK ){
      p->nStep = pState->nRow;
      rc = rbuObjIterPrepareAll(p, &p->objiter, p->nStep);
    }

    p->rc = rc;
  }
}

/*
** If there is a "*-oal" file in the file-system corresponding to the
** target database in the file-system, delete it. If an error occurs,
** leave an error code and error message in the rbu handle.
*/
static void rbuDeleteOalFile(sqlite3rbu *p){
  char *zOal = rbuMPrintf(p, "%s-oal", p->zTarget);
  if( zOal ){
    sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
    assert( pVfs && p->rc==SQLITE_OK && p->zErrmsg==0 );
    pVfs->xDelete(pVfs, zOal, 0);
    sqlite3_free(zOal);
  }
}

/*
** Allocate a private rbu VFS for the rbu handle passed as the only
** argument. This VFS will be used unless the call to sqlite3rbu_open()
** specified a URI with a vfs=? option in place of a target database
** file name.
*/
static void rbuCreateVfs(sqlite3rbu *p){
  int rnd;
  char zRnd[64];

  assert( p->rc==SQLITE_OK );
  sqlite3_randomness(sizeof(int), (void*)&rnd);
  sqlite3_snprintf(sizeof(zRnd), zRnd, "rbu_vfs_%d", rnd);
  p->rc = sqlite3rbu_create_vfs(zRnd, 0);
  if( p->rc==SQLITE_OK ){
    sqlite3_vfs *pVfs = sqlite3_vfs_find(zRnd);
    assert( pVfs );
    p->zVfsName = pVfs->zName;
  }
}

/*
** Destroy the private VFS created for the rbu handle passed as the only
** argument by an earlier call to rbuCreateVfs().
*/
static void rbuDeleteVfs(sqlite3rbu *p){
  if( p->zVfsName ){
    sqlite3rbu_destroy_vfs(p->zVfsName);
    p->zVfsName = 0;
  }
}

/*
** Open and return a new RBU handle. 
*/
SQLITE_API sqlite3rbu *SQLITE_STDCALL sqlite3rbu_open(
  const char *zTarget, 
  const char *zRbu,
  const char *zState
){
  sqlite3rbu *p;
  int nTarget = strlen(zTarget);
  int nRbu = strlen(zRbu);
  int nState = zState ? strlen(zState) : 0;

  p = (sqlite3rbu*)sqlite3_malloc(sizeof(sqlite3rbu)+nTarget+1+nRbu+1+nState+1);
  if( p ){
    RbuState *pState = 0;

    /* Create the custom VFS. */
    memset(p, 0, sizeof(sqlite3rbu));
    rbuCreateVfs(p);

    /* Open the target database */
    if( p->rc==SQLITE_OK ){
      p->zTarget = (char*)&p[1];
      memcpy(p->zTarget, zTarget, nTarget+1);
      p->zRbu = &p->zTarget[nTarget+1];
      memcpy(p->zRbu, zRbu, nRbu+1);
      if( zState ){
        p->zState = &p->zRbu[nRbu+1];
        memcpy(p->zState, zState, nState+1);
      }
      rbuOpenDatabase(p);
    }

    /* If it has not already been created, create the rbu_state table */
    rbuMPrintfExec(p, p->dbRbu, RBU_CREATE_STATE, p->zStateDb);

    if( p->rc==SQLITE_OK ){
      pState = rbuLoadState(p);
      assert( pState || p->rc!=SQLITE_OK );
      if( p->rc==SQLITE_OK ){

        if( pState->eStage==0 ){ 
          rbuDeleteOalFile(p);
          p->eStage = RBU_STAGE_OAL;
        }else{
          p->eStage = pState->eStage;
        }
        p->nProgress = pState->nProgress;
        p->iOalSz = pState->iOalSz;
      }
    }
    assert( p->rc!=SQLITE_OK || p->eStage!=0 );

    if( p->rc==SQLITE_OK && p->pTargetFd->pWalFd ){
      if( p->eStage==RBU_STAGE_OAL ){
        p->rc = SQLITE_ERROR;
        p->zErrmsg = sqlite3_mprintf("cannot update wal mode database");
      }else if( p->eStage==RBU_STAGE_MOVE ){
        p->eStage = RBU_STAGE_CKPT;
        p->nStep = 0;
      }
    }

    if( p->rc==SQLITE_OK
     && (p->eStage==RBU_STAGE_OAL || p->eStage==RBU_STAGE_MOVE)
     && pState->eStage!=0 && p->pTargetFd->iCookie!=pState->iCookie
    ){   
      /* At this point (pTargetFd->iCookie) contains the value of the
      ** change-counter cookie (the thing that gets incremented when a 
      ** transaction is committed in rollback mode) currently stored on 
      ** page 1 of the database file. */
      p->rc = SQLITE_BUSY;
      p->zErrmsg = sqlite3_mprintf("database modified during rbu update");
    }

    if( p->rc==SQLITE_OK ){
      if( p->eStage==RBU_STAGE_OAL ){
        sqlite3 *db = p->dbMain;

        /* Open transactions both databases. The *-oal file is opened or
        ** created at this point. */
        p->rc = sqlite3_exec(db, "BEGIN IMMEDIATE", 0, 0, &p->zErrmsg);
        if( p->rc==SQLITE_OK ){
          p->rc = sqlite3_exec(p->dbRbu, "BEGIN IMMEDIATE", 0, 0, &p->zErrmsg);
        }

        /* Check if the main database is a zipvfs db. If it is, set the upper
        ** level pager to use "journal_mode=off". This prevents it from 
        ** generating a large journal using a temp file.  */
        if( p->rc==SQLITE_OK ){
          int frc = sqlite3_file_control(db, "main", SQLITE_FCNTL_ZIPVFS, 0);
          if( frc==SQLITE_OK ){
            p->rc = sqlite3_exec(db, "PRAGMA journal_mode=off",0,0,&p->zErrmsg);
          }
        }

        /* Point the object iterator at the first object */
        if( p->rc==SQLITE_OK ){
          p->rc = rbuObjIterFirst(p, &p->objiter);
        }

        /* If the RBU database contains no data_xxx tables, declare the RBU
        ** update finished.  */
        if( p->rc==SQLITE_OK && p->objiter.zTbl==0 ){
          p->rc = SQLITE_DONE;
        }

        if( p->rc==SQLITE_OK ){
          rbuSetupOal(p, pState);
        }

      }else if( p->eStage==RBU_STAGE_MOVE ){
        /* no-op */
      }else if( p->eStage==RBU_STAGE_CKPT ){
        rbuSetupCheckpoint(p, pState);
      }else if( p->eStage==RBU_STAGE_DONE ){
        p->rc = SQLITE_DONE;
      }else{
        p->rc = SQLITE_CORRUPT;
      }
    }

    rbuFreeState(pState);
  }

  return p;
}


/*
** Return the database handle used by pRbu.
*/
SQLITE_API sqlite3 *SQLITE_STDCALL sqlite3rbu_db(sqlite3rbu *pRbu, int bRbu){
  sqlite3 *db = 0;
  if( pRbu ){
    db = (bRbu ? pRbu->dbRbu : pRbu->dbMain);
  }
  return db;
}


/*
** If the error code currently stored in the RBU handle is SQLITE_CONSTRAINT,
** then edit any error message string so as to remove all occurrences of
** the pattern "rbu_imp_[0-9]*".
*/
static void rbuEditErrmsg(sqlite3rbu *p){
  if( p->rc==SQLITE_CONSTRAINT && p->zErrmsg ){
    int i;
    int nErrmsg = strlen(p->zErrmsg);
    for(i=0; i<(nErrmsg-8); i++){
      if( memcmp(&p->zErrmsg[i], "rbu_imp_", 8)==0 ){
        int nDel = 8;
        while( p->zErrmsg[i+nDel]>='0' && p->zErrmsg[i+nDel]<='9' ) nDel++;
        memmove(&p->zErrmsg[i], &p->zErrmsg[i+nDel], nErrmsg + 1 - i - nDel);
        nErrmsg -= nDel;
      }
    }
  }
}

/*
** Close the RBU handle.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_close(sqlite3rbu *p, char **pzErrmsg){
  int rc;
  if( p ){

    /* Commit the transaction to the *-oal file. */
    if( p->rc==SQLITE_OK && p->eStage==RBU_STAGE_OAL ){
      p->rc = sqlite3_exec(p->dbMain, "COMMIT", 0, 0, &p->zErrmsg);
    }

    rbuSaveState(p, p->eStage);

    if( p->rc==SQLITE_OK && p->eStage==RBU_STAGE_OAL ){
      p->rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, &p->zErrmsg);
    }

    /* Close any open statement handles. */
    rbuObjIterFinalize(&p->objiter);

    /* Close the open database handle and VFS object. */
    sqlite3_close(p->dbMain);
    sqlite3_close(p->dbRbu);
    rbuDeleteVfs(p);
    sqlite3_free(p->aBuf);
    sqlite3_free(p->aFrame);

    rbuEditErrmsg(p);
    rc = p->rc;
    *pzErrmsg = p->zErrmsg;
    sqlite3_free(p);
  }else{
    rc = SQLITE_NOMEM;
    *pzErrmsg = 0;
  }
  return rc;
}

/*
** Return the total number of key-value operations (inserts, deletes or 
** updates) that have been performed on the target database since the
** current RBU update was started.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3rbu_progress(sqlite3rbu *pRbu){
  return pRbu->nProgress;
}

SQLITE_API int SQLITE_STDCALL sqlite3rbu_savestate(sqlite3rbu *p){
  int rc = p->rc;
  
  if( rc==SQLITE_DONE ) return SQLITE_OK;

  assert( p->eStage>=RBU_STAGE_OAL && p->eStage<=RBU_STAGE_DONE );
  if( p->eStage==RBU_STAGE_OAL ){
    assert( rc!=SQLITE_DONE );
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbMain, "COMMIT", 0, 0, 0);
  }

  p->rc = rc;
  rbuSaveState(p, p->eStage);
  rc = p->rc;

  if( p->eStage==RBU_STAGE_OAL ){
    assert( rc!=SQLITE_DONE );
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, 0);
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbRbu, "BEGIN IMMEDIATE", 0, 0, 0);
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbMain, "BEGIN IMMEDIATE", 0, 0,0);
  }

  p->rc = rc;
  return rc;
}

/**************************************************************************
** Beginning of RBU VFS shim methods. The VFS shim modifies the behaviour
** of a standard VFS in the following ways:
**
** 1. Whenever the first page of a main database file is read or 
**    written, the value of the change-counter cookie is stored in
**    rbu_file.iCookie. Similarly, the value of the "write-version"
**    database header field is stored in rbu_file.iWriteVer. This ensures
**    that the values are always trustworthy within an open transaction.
**
** 2. Whenever an SQLITE_OPEN_WAL file is opened, the (rbu_file.pWalFd)
**    member variable of the associated database file descriptor is set
**    to point to the new file. A mutex protected linked list of all main 
**    db fds opened using a particular RBU VFS is maintained at 
**    rbu_vfs.pMain to facilitate this.
**
** 3. Using a new file-control "SQLITE_FCNTL_RBU", a main db rbu_file 
**    object can be marked as the target database of an RBU update. This
**    turns on the following extra special behaviour:
**
** 3a. If xAccess() is called to check if there exists a *-wal file 
**     associated with an RBU target database currently in RBU_STAGE_OAL
**     stage (preparing the *-oal file), the following special handling
**     applies:
**
**      * if the *-wal file does exist, return SQLITE_CANTOPEN. An RBU
**        target database may not be in wal mode already.
**
**      * if the *-wal file does not exist, set the output parameter to
**        non-zero (to tell SQLite that it does exist) anyway.
**
**     Then, when xOpen() is called to open the *-wal file associated with
**     the RBU target in RBU_STAGE_OAL stage, instead of opening the *-wal
**     file, the rbu vfs opens the corresponding *-oal file instead. 
**
** 3b. The *-shm pages returned by xShmMap() for a target db file in
**     RBU_STAGE_OAL mode are actually stored in heap memory. This is to
**     avoid creating a *-shm file on disk. Additionally, xShmLock() calls
**     are no-ops on target database files in RBU_STAGE_OAL mode. This is
**     because assert() statements in some VFS implementations fail if 
**     xShmLock() is called before xShmMap().
**
** 3c. If an EXCLUSIVE lock is attempted on a target database file in any
**     mode except RBU_STAGE_DONE (all work completed and checkpointed), it 
**     fails with an SQLITE_BUSY error. This is to stop RBU connections
**     from automatically checkpointing a *-wal (or *-oal) file from within
**     sqlite3_close().
**
** 3d. In RBU_STAGE_CAPTURE mode, all xRead() calls on the wal file, and
**     all xWrite() calls on the target database file perform no IO. 
**     Instead the frame and page numbers that would be read and written
**     are recorded. Additionally, successful attempts to obtain exclusive
**     xShmLock() WRITER, CHECKPOINTER and READ0 locks on the target 
**     database file are recorded. xShmLock() calls to unlock the same
**     locks are no-ops (so that once obtained, these locks are never
**     relinquished). Finally, calls to xSync() on the target database
**     file fail with SQLITE_INTERNAL errors.
*/

static void rbuUnlockShm(rbu_file *p){
  if( p->pRbu ){
    int (*xShmLock)(sqlite3_file*,int,int,int) = p->pReal->pMethods->xShmLock;
    int i;
    for(i=0; i<SQLITE_SHM_NLOCK;i++){
      if( (1<<i) & p->pRbu->mLock ){
        xShmLock(p->pReal, i, 1, SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE);
      }
    }
    p->pRbu->mLock = 0;
  }
}

/*
** Close an rbu file.
*/
static int rbuVfsClose(sqlite3_file *pFile){
  rbu_file *p = (rbu_file*)pFile;
  int rc;
  int i;

  /* Free the contents of the apShm[] array. And the array itself. */
  for(i=0; i<p->nShm; i++){
    sqlite3_free(p->apShm[i]);
  }
  sqlite3_free(p->apShm);
  p->apShm = 0;
  sqlite3_free(p->zDel);

  if( p->openFlags & SQLITE_OPEN_MAIN_DB ){
    rbu_file **pp;
    sqlite3_mutex_enter(p->pRbuVfs->mutex);
    for(pp=&p->pRbuVfs->pMain; *pp!=p; pp=&((*pp)->pMainNext));
    *pp = p->pMainNext;
    sqlite3_mutex_leave(p->pRbuVfs->mutex);
    rbuUnlockShm(p);
    p->pReal->pMethods->xShmUnmap(p->pReal, 0);
  }

  /* Close the underlying file handle */
  rc = p->pReal->pMethods->xClose(p->pReal);
  return rc;
}


/*
** Read and return an unsigned 32-bit big-endian integer from the buffer 
** passed as the only argument.
*/
static u32 rbuGetU32(u8 *aBuf){
  return ((u32)aBuf[0] << 24)
       + ((u32)aBuf[1] << 16)
       + ((u32)aBuf[2] <<  8)
       + ((u32)aBuf[3]);
}

/*
** Read data from an rbuVfs-file.
*/
static int rbuVfsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  int iAmt, 
  sqlite_int64 iOfst
){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc;

  if( pRbu && pRbu->eStage==RBU_STAGE_CAPTURE ){
    assert( p->openFlags & SQLITE_OPEN_WAL );
    rc = rbuCaptureWalRead(p->pRbu, iOfst, iAmt);
  }else{
    if( pRbu && pRbu->eStage==RBU_STAGE_OAL 
     && (p->openFlags & SQLITE_OPEN_WAL) 
     && iOfst>=pRbu->iOalSz 
    ){
      rc = SQLITE_OK;
      memset(zBuf, 0, iAmt);
    }else{
      rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
    }
    if( rc==SQLITE_OK && iOfst==0 && (p->openFlags & SQLITE_OPEN_MAIN_DB) ){
      /* These look like magic numbers. But they are stable, as they are part
       ** of the definition of the SQLite file format, which may not change. */
      u8 *pBuf = (u8*)zBuf;
      p->iCookie = rbuGetU32(&pBuf[24]);
      p->iWriteVer = pBuf[19];
    }
  }
  return rc;
}

/*
** Write data to an rbuVfs-file.
*/
static int rbuVfsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  int iAmt, 
  sqlite_int64 iOfst
){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc;

  if( pRbu && pRbu->eStage==RBU_STAGE_CAPTURE ){
    assert( p->openFlags & SQLITE_OPEN_MAIN_DB );
    rc = rbuCaptureDbWrite(p->pRbu, iOfst);
  }else{
    if( pRbu && pRbu->eStage==RBU_STAGE_OAL 
     && (p->openFlags & SQLITE_OPEN_WAL) 
     && iOfst>=pRbu->iOalSz
    ){
      pRbu->iOalSz = iAmt + iOfst;
    }
    rc = p->pReal->pMethods->xWrite(p->pReal, zBuf, iAmt, iOfst);
    if( rc==SQLITE_OK && iOfst==0 && (p->openFlags & SQLITE_OPEN_MAIN_DB) ){
      /* These look like magic numbers. But they are stable, as they are part
      ** of the definition of the SQLite file format, which may not change. */
      u8 *pBuf = (u8*)zBuf;
      p->iCookie = rbuGetU32(&pBuf[24]);
      p->iWriteVer = pBuf[19];
    }
  }
  return rc;
}

/*
** Truncate an rbuVfs-file.
*/
static int rbuVfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  rbu_file *p = (rbu_file*)pFile;
  return p->pReal->pMethods->xTruncate(p->pReal, size);
}

/*
** Sync an rbuVfs-file.
*/
static int rbuVfsSync(sqlite3_file *pFile, int flags){
  rbu_file *p = (rbu_file *)pFile;
  if( p->pRbu && p->pRbu->eStage==RBU_STAGE_CAPTURE ){
    if( p->openFlags & SQLITE_OPEN_MAIN_DB ){
      return SQLITE_INTERNAL;
    }
    return SQLITE_OK;
  }
  return p->pReal->pMethods->xSync(p->pReal, flags);
}

/*
** Return the current file-size of an rbuVfs-file.
*/
static int rbuVfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xFileSize(p->pReal, pSize);
}

/*
** Lock an rbuVfs-file.
*/
static int rbuVfsLock(sqlite3_file *pFile, int eLock){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc = SQLITE_OK;

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( pRbu && eLock==SQLITE_LOCK_EXCLUSIVE && pRbu->eStage!=RBU_STAGE_DONE ){
    /* Do not allow EXCLUSIVE locks. Preventing SQLite from taking this 
    ** prevents it from checkpointing the database from sqlite3_close(). */
    rc = SQLITE_BUSY;
  }else{
    rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  }

  return rc;
}

/*
** Unlock an rbuVfs-file.
*/
static int rbuVfsUnlock(sqlite3_file *pFile, int eLock){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an rbuVfs-file.
*/
static int rbuVfsCheckReservedLock(sqlite3_file *pFile, int *pResOut){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an rbuVfs-file.
*/
static int rbuVfsFileControl(sqlite3_file *pFile, int op, void *pArg){
  rbu_file *p = (rbu_file *)pFile;
  int (*xControl)(sqlite3_file*,int,void*) = p->pReal->pMethods->xFileControl;
  int rc;

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB)
       || p->openFlags & (SQLITE_OPEN_TRANSIENT_DB|SQLITE_OPEN_TEMP_JOURNAL)
  );
  if( op==SQLITE_FCNTL_RBU ){
    sqlite3rbu *pRbu = (sqlite3rbu*)pArg;

    /* First try to find another RBU vfs lower down in the vfs stack. If
    ** one is found, this vfs will operate in pass-through mode. The lower
    ** level vfs will do the special RBU handling.  */
    rc = xControl(p->pReal, op, pArg);

    if( rc==SQLITE_NOTFOUND ){
      /* Now search for a zipvfs instance lower down in the VFS stack. If
      ** one is found, this is an error.  */
      void *dummy = 0;
      rc = xControl(p->pReal, SQLITE_FCNTL_ZIPVFS, &dummy);
      if( rc==SQLITE_OK ){
        rc = SQLITE_ERROR;
        pRbu->zErrmsg = sqlite3_mprintf("rbu/zipvfs setup error");
      }else if( rc==SQLITE_NOTFOUND ){
        pRbu->pTargetFd = p;
        p->pRbu = pRbu;
        if( p->pWalFd ) p->pWalFd->pRbu = pRbu;
        rc = SQLITE_OK;
      }
    }
    return rc;
  }

  rc = xControl(p->pReal, op, pArg);
  if( rc==SQLITE_OK && op==SQLITE_FCNTL_VFSNAME ){
    rbu_vfs *pRbuVfs = p->pRbuVfs;
    char *zIn = *(char**)pArg;
    char *zOut = sqlite3_mprintf("rbu(%s)/%z", pRbuVfs->base.zName, zIn);
    *(char**)pArg = zOut;
    if( zOut==0 ) rc = SQLITE_NOMEM;
  }

  return rc;
}

/*
** Return the sector-size in bytes for an rbuVfs-file.
*/
static int rbuVfsSectorSize(sqlite3_file *pFile){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an rbuVfs-file.
*/
static int rbuVfsDeviceCharacteristics(sqlite3_file *pFile){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
}

/*
** Take or release a shared-memory lock.
*/
static int rbuVfsShmLock(sqlite3_file *pFile, int ofst, int n, int flags){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc = SQLITE_OK;

#ifdef SQLITE_AMALGAMATION
    assert( WAL_CKPT_LOCK==1 );
#endif

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( pRbu && (pRbu->eStage==RBU_STAGE_OAL || pRbu->eStage==RBU_STAGE_MOVE) ){
    /* Magic number 1 is the WAL_CKPT_LOCK lock. Preventing SQLite from
    ** taking this lock also prevents any checkpoints from occurring. 
    ** todo: really, it's not clear why this might occur, as 
    ** wal_autocheckpoint ought to be turned off.  */
    if( ofst==WAL_LOCK_CKPT && n==1 ) rc = SQLITE_BUSY;
  }else{
    int bCapture = 0;
    if( n==1 && (flags & SQLITE_SHM_EXCLUSIVE)
     && pRbu && pRbu->eStage==RBU_STAGE_CAPTURE
     && (ofst==WAL_LOCK_WRITE || ofst==WAL_LOCK_CKPT || ofst==WAL_LOCK_READ0)
    ){
      bCapture = 1;
    }

    if( bCapture==0 || 0==(flags & SQLITE_SHM_UNLOCK) ){
      rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
      if( bCapture && rc==SQLITE_OK ){
        pRbu->mLock |= (1 << ofst);
      }
    }
  }

  return rc;
}

/*
** Obtain a pointer to a mapping of a single 32KiB page of the *-shm file.
*/
static int rbuVfsShmMap(
  sqlite3_file *pFile, 
  int iRegion, 
  int szRegion, 
  int isWrite, 
  void volatile **pp
){
  rbu_file *p = (rbu_file*)pFile;
  int rc = SQLITE_OK;
  int eStage = (p->pRbu ? p->pRbu->eStage : 0);

  /* If not in RBU_STAGE_OAL, allow this call to pass through. Or, if this
  ** rbu is in the RBU_STAGE_OAL state, use heap memory for *-shm space 
  ** instead of a file on disk.  */
  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( eStage==RBU_STAGE_OAL || eStage==RBU_STAGE_MOVE ){
    if( iRegion<=p->nShm ){
      int nByte = (iRegion+1) * sizeof(char*);
      char **apNew = (char**)sqlite3_realloc(p->apShm, nByte);
      if( apNew==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memset(&apNew[p->nShm], 0, sizeof(char*) * (1 + iRegion - p->nShm));
        p->apShm = apNew;
        p->nShm = iRegion+1;
      }
    }

    if( rc==SQLITE_OK && p->apShm[iRegion]==0 ){
      char *pNew = (char*)sqlite3_malloc(szRegion);
      if( pNew==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pNew, 0, szRegion);
        p->apShm[iRegion] = pNew;
      }
    }

    if( rc==SQLITE_OK ){
      *pp = p->apShm[iRegion];
    }else{
      *pp = 0;
    }
  }else{
    assert( p->apShm==0 );
    rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  }

  return rc;
}

/*
** Memory barrier.
*/
static void rbuVfsShmBarrier(sqlite3_file *pFile){
  rbu_file *p = (rbu_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}

/*
** The xShmUnmap method.
*/
static int rbuVfsShmUnmap(sqlite3_file *pFile, int delFlag){
  rbu_file *p = (rbu_file*)pFile;
  int rc = SQLITE_OK;
  int eStage = (p->pRbu ? p->pRbu->eStage : 0);

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( eStage==RBU_STAGE_OAL || eStage==RBU_STAGE_MOVE ){
    /* no-op */
  }else{
    /* Release the checkpointer and writer locks */
    rbuUnlockShm(p);
    rc = p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
  }
  return rc;
}

/*
** Given that zWal points to a buffer containing a wal file name passed to 
** either the xOpen() or xAccess() VFS method, return a pointer to the
** file-handle opened by the same database connection on the corresponding
** database file.
*/
static rbu_file *rbuFindMaindb(rbu_vfs *pRbuVfs, const char *zWal){
  rbu_file *pDb;
  sqlite3_mutex_enter(pRbuVfs->mutex);
  for(pDb=pRbuVfs->pMain; pDb && pDb->zWal!=zWal; pDb=pDb->pMainNext);
  sqlite3_mutex_leave(pRbuVfs->mutex);
  return pDb;
}

/*
** Open an rbu file handle.
*/
static int rbuVfsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  int flags,
  int *pOutFlags
){
  static sqlite3_io_methods rbuvfs_io_methods = {
    2,                            /* iVersion */
    rbuVfsClose,                  /* xClose */
    rbuVfsRead,                   /* xRead */
    rbuVfsWrite,                  /* xWrite */
    rbuVfsTruncate,               /* xTruncate */
    rbuVfsSync,                   /* xSync */
    rbuVfsFileSize,               /* xFileSize */
    rbuVfsLock,                   /* xLock */
    rbuVfsUnlock,                 /* xUnlock */
    rbuVfsCheckReservedLock,      /* xCheckReservedLock */
    rbuVfsFileControl,            /* xFileControl */
    rbuVfsSectorSize,             /* xSectorSize */
    rbuVfsDeviceCharacteristics,  /* xDeviceCharacteristics */
    rbuVfsShmMap,                 /* xShmMap */
    rbuVfsShmLock,                /* xShmLock */
    rbuVfsShmBarrier,             /* xShmBarrier */
    rbuVfsShmUnmap,               /* xShmUnmap */
    0, 0                          /* xFetch, xUnfetch */
  };
  rbu_vfs *pRbuVfs = (rbu_vfs*)pVfs;
  sqlite3_vfs *pRealVfs = pRbuVfs->pRealVfs;
  rbu_file *pFd = (rbu_file *)pFile;
  int rc = SQLITE_OK;
  const char *zOpen = zName;

  memset(pFd, 0, sizeof(rbu_file));
  pFd->pReal = (sqlite3_file*)&pFd[1];
  pFd->pRbuVfs = pRbuVfs;
  pFd->openFlags = flags;
  if( zName ){
    if( flags & SQLITE_OPEN_MAIN_DB ){
      /* A main database has just been opened. The following block sets
      ** (pFd->zWal) to point to a buffer owned by SQLite that contains
      ** the name of the *-wal file this db connection will use. SQLite
      ** happens to pass a pointer to this buffer when using xAccess()
      ** or xOpen() to operate on the *-wal file.  */
      int n = strlen(zName);
      const char *z = &zName[n];
      if( flags & SQLITE_OPEN_URI ){
        int odd = 0;
        while( 1 ){
          if( z[0]==0 ){
            odd = 1 - odd;
            if( odd && z[1]==0 ) break;
          }
          z++;
        }
        z += 2;
      }else{
        while( *z==0 ) z++;
      }
      z += (n + 8 + 1);
      pFd->zWal = z;
    }
    else if( flags & SQLITE_OPEN_WAL ){
      rbu_file *pDb = rbuFindMaindb(pRbuVfs, zName);
      if( pDb ){
        if( pDb->pRbu && pDb->pRbu->eStage==RBU_STAGE_OAL ){
          /* This call is to open a *-wal file. Intead, open the *-oal. This
          ** code ensures that the string passed to xOpen() is terminated by a
          ** pair of '\0' bytes in case the VFS attempts to extract a URI 
          ** parameter from it.  */
          int nCopy = strlen(zName);
          char *zCopy = sqlite3_malloc(nCopy+2);
          if( zCopy ){
            memcpy(zCopy, zName, nCopy);
            zCopy[nCopy-3] = 'o';
            zCopy[nCopy] = '\0';
            zCopy[nCopy+1] = '\0';
            zOpen = (const char*)(pFd->zDel = zCopy);
          }else{
            rc = SQLITE_NOMEM;
          }
          pFd->pRbu = pDb->pRbu;
        }
        pDb->pWalFd = pFd;
      }
    }
  }

  if( rc==SQLITE_OK ){
    rc = pRealVfs->xOpen(pRealVfs, zOpen, pFd->pReal, flags, pOutFlags);
  }
  if( pFd->pReal->pMethods ){
    /* The xOpen() operation has succeeded. Set the sqlite3_file.pMethods
    ** pointer and, if the file is a main database file, link it into the
    ** mutex protected linked list of all such files.  */
    pFile->pMethods = &rbuvfs_io_methods;
    if( flags & SQLITE_OPEN_MAIN_DB ){
      sqlite3_mutex_enter(pRbuVfs->mutex);
      pFd->pMainNext = pRbuVfs->pMain;
      pRbuVfs->pMain = pFd;
      sqlite3_mutex_leave(pRbuVfs->mutex);
    }
  }else{
    sqlite3_free(pFd->zDel);
  }

  return rc;
}

/*
** Delete the file located at zPath.
*/
static int rbuVfsDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xDelete(pRealVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int rbuVfsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int flags, 
  int *pResOut
){
  rbu_vfs *pRbuVfs = (rbu_vfs*)pVfs;
  sqlite3_vfs *pRealVfs = pRbuVfs->pRealVfs;
  int rc;

  rc = pRealVfs->xAccess(pRealVfs, zPath, flags, pResOut);

  /* If this call is to check if a *-wal file associated with an RBU target
  ** database connection exists, and the RBU update is in RBU_STAGE_OAL,
  ** the following special handling is activated:
  **
  **   a) if the *-wal file does exist, return SQLITE_CANTOPEN. This
  **      ensures that the RBU extension never tries to update a database
  **      in wal mode, even if the first page of the database file has
  **      been damaged. 
  **
  **   b) if the *-wal file does not exist, claim that it does anyway,
  **      causing SQLite to call xOpen() to open it. This call will also
  **      be intercepted (see the rbuVfsOpen() function) and the *-oal
  **      file opened instead.
  */
  if( rc==SQLITE_OK && flags==SQLITE_ACCESS_EXISTS ){
    rbu_file *pDb = rbuFindMaindb(pRbuVfs, zPath);
    if( pDb && pDb->pRbu && pDb->pRbu->eStage==RBU_STAGE_OAL ){
      if( *pResOut ){
        rc = SQLITE_CANTOPEN;
      }else{
        *pResOut = 1;
      }
    }
  }

  return rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static int rbuVfsFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int nOut, 
  char *zOut
){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xFullPathname(pRealVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *rbuVfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xDlOpen(pRealVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void rbuVfsDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  pRealVfs->xDlError(pRealVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*rbuVfsDlSym(
  sqlite3_vfs *pVfs, 
  void *pArg, 
  const char *zSym
))(void){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xDlSym(pRealVfs, pArg, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void rbuVfsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  pRealVfs->xDlClose(pRealVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static int rbuVfsRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xRandomness(pRealVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static int rbuVfsSleep(sqlite3_vfs *pVfs, int nMicro){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xSleep(pRealVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int rbuVfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xCurrentTime(pRealVfs, pTimeOut);
}

/*
** No-op.
*/
static int rbuVfsGetLastError(sqlite3_vfs *pVfs, int a, char *b){
  return 0;
}

/*
** Deregister and destroy an RBU vfs created by an earlier call to
** sqlite3rbu_create_vfs().
*/
SQLITE_API void SQLITE_STDCALL sqlite3rbu_destroy_vfs(const char *zName){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(zName);
  if( pVfs && pVfs->xOpen==rbuVfsOpen ){
    sqlite3_mutex_free(((rbu_vfs*)pVfs)->mutex);
    sqlite3_vfs_unregister(pVfs);
    sqlite3_free(pVfs);
  }
}

/*
** Create an RBU VFS named zName that accesses the underlying file-system
** via existing VFS zParent. The new object is registered as a non-default
** VFS with SQLite before returning.
*/
SQLITE_API int SQLITE_STDCALL sqlite3rbu_create_vfs(const char *zName, const char *zParent){

  /* Template for VFS */
  static sqlite3_vfs vfs_template = {
    1,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    rbuVfsOpen,                   /* xOpen */
    rbuVfsDelete,                 /* xDelete */
    rbuVfsAccess,                 /* xAccess */
    rbuVfsFullPathname,           /* xFullPathname */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
    rbuVfsDlOpen,                 /* xDlOpen */
    rbuVfsDlError,                /* xDlError */
    rbuVfsDlSym,                  /* xDlSym */
    rbuVfsDlClose,                /* xDlClose */
#else
    0, 0, 0, 0,
#endif

    rbuVfsRandomness,             /* xRandomness */
    rbuVfsSleep,                  /* xSleep */
    rbuVfsCurrentTime,            /* xCurrentTime */
    rbuVfsGetLastError,           /* xGetLastError */
    0,                            /* xCurrentTimeInt64 (version 2) */
    0, 0, 0                       /* Unimplemented version 3 methods */
  };

  rbu_vfs *pNew = 0;              /* Newly allocated VFS */
  int nName;
  int rc = SQLITE_OK;

  int nByte;
  nName = strlen(zName);
  nByte = sizeof(rbu_vfs) + nName + 1;
  pNew = (rbu_vfs*)sqlite3_malloc(nByte);
  if( pNew==0 ){
    rc = SQLITE_NOMEM;
  }else{
    sqlite3_vfs *pParent;           /* Parent VFS */
    memset(pNew, 0, nByte);
    pParent = sqlite3_vfs_find(zParent);
    if( pParent==0 ){
      rc = SQLITE_NOTFOUND;
    }else{
      char *zSpace;
      memcpy(&pNew->base, &vfs_template, sizeof(sqlite3_vfs));
      pNew->base.mxPathname = pParent->mxPathname;
      pNew->base.szOsFile = sizeof(rbu_file) + pParent->szOsFile;
      pNew->pRealVfs = pParent;
      pNew->base.zName = (const char*)(zSpace = (char*)&pNew[1]);
      memcpy(zSpace, zName, nName);

      /* Allocate the mutex and register the new VFS (not as the default) */
      pNew->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
      if( pNew->mutex==0 ){
        rc = SQLITE_NOMEM;
      }else{
        rc = sqlite3_vfs_register(&pNew->base, 0);
      }
    }

    if( rc!=SQLITE_OK ){
      sqlite3_mutex_free(pNew->mutex);
      sqlite3_free(pNew);
    }
  }

  return rc;
}


/**************************************************************************/

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_RBU) */

/************** End of sqlite3rbu.c ******************************************/
/************** Begin file dbstat.c ******************************************/
/*
** 2010 July 12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains an implementation of the "dbstat" virtual table.
**
** The dbstat virtual table is used to extract low-level formatting
** information from an SQLite database in order to implement the
** "sqlite3_analyzer" utility.  See the ../tool/spaceanal.tcl script
** for an example implementation.
**
** Additional information is available on the "dbstat.html" page of the
** official SQLite documentation.
*/

/* #include "sqliteInt.h"   ** Requires access to internal data structures ** */
#if (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** Page paths:
** 
**   The value of the 'path' column describes the path taken from the 
**   root-node of the b-tree structure to each page. The value of the 
**   root-node path is '/'.
**
**   The value of the path for the left-most child page of the root of
**   a b-tree is '/000/'. (Btrees store content ordered from left to right
**   so the pages to the left have smaller keys than the pages to the right.)
**   The next to left-most child of the root page is
**   '/001', and so on, each sibling page identified by a 3-digit hex 
**   value. The children of the 451st left-most sibling have paths such
**   as '/1c2/000/, '/1c2/001/' etc.
**
**   Overflow pages are specified by appending a '+' character and a 
**   six-digit hexadecimal value to the path to the cell they are linked
**   from. For example, the three overflow pages in a chain linked from 
**   the left-most cell of the 450th child of the root page are identified
**   by the paths:
**
**      '/1c2/000+000000'         // First page in overflow chain
**      '/1c2/000+000001'         // Second page in overflow chain
**      '/1c2/000+000002'         // Third page in overflow chain
**
**   If the paths are sorted using the BINARY collation sequence, then
**   the overflow pages associated with a cell will appear earlier in the
**   sort-order than its child page:
**
**      '/1c2/000/'               // Left-most child of 451st child of root
*/
#define VTAB_SCHEMA                                                         \
  "CREATE TABLE xx( "                                                       \
  "  name       STRING,           /* Name of table or index */"             \
  "  path       INTEGER,          /* Path to page from root */"             \
  "  pageno     INTEGER,          /* Page number */"                        \
  "  pagetype   STRING,           /* 'internal', 'leaf' or 'overflow' */"   \
  "  ncell      INTEGER,          /* Cells on page (0 for overflow) */"     \
  "  payload    INTEGER,          /* Bytes of payload on this page */"      \
  "  unused     INTEGER,          /* Bytes of unused space on this page */" \
  "  mx_payload INTEGER,          /* Largest payload size of all cells */"  \
  "  pgoffset   INTEGER,          /* Offset of page in file */"             \
  "  pgsize     INTEGER,          /* Size of the page */"                   \
  "  schema     TEXT HIDDEN       /* Database schema being analyzed */"     \
  ");"


typedef struct StatTable StatTable;
typedef struct StatCursor StatCursor;
typedef struct StatPage StatPage;
typedef struct StatCell StatCell;

struct StatCell {
  int nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  int nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  int nLastOvfl;                  /* Bytes of payload on final overflow page */
  int iOvfl;                      /* Iterates through aOvfl[] */
};

struct StatPage {
  u32 iPgno;
  DbPage *pPg;
  int iCell;

  char *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  int nCell;                      /* Number of cells on page */
  int nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 iRightChildPg;              /* Right-child page number (or 0) */
  int nMxPayload;                 /* Largest payload of any cell on this page */
};

struct StatCursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  int isEof;                      /* After pStmt has returned SQLITE_DONE */
  int iDb;                        /* Schema used for this query */

  StatPage aPage[32];
  int iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  char *zName;                    /* Value of 'name' column */
  char *zPath;                    /* Value of 'path' column */
  u32 iPageno;                    /* Value of 'pageno' column */
  char *zPagetype;                /* Value of 'pagetype' column */
  int nCell;                      /* Value of 'ncell' column */
  int nPayload;                   /* Value of 'payload' column */
  int nUnused;                    /* Value of 'unused' column */
  int nMxPayload;                 /* Value of 'mx_payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  int szPage;                     /* Value of 'pgSize' column */
};

struct StatTable {
  sqlite3_vtab base;
  sqlite3 *db;
  int iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a statvfs virtual table.
*/
static int statConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  StatTable *pTab = 0;
  int rc = SQLITE_OK;
  int iDb;

  if( argc>=4 ){
    iDb = sqlite3FindDbName(db, argv[3]);
    if( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      return SQLITE_ERROR;
    }
  }else{
    iDb = 0;
  }
  rc = sqlite3_declare_vtab(db, VTAB_SCHEMA);
  if( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(sizeof(StatTable));
    if( pTab==0 ) rc = SQLITE_NOMEM;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  if( rc==SQLITE_OK ){
    memset(pTab, 0, sizeof(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  return rc;
}

/*
** Disconnect from or destroy a statvfs virtual table.
*/
static int statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan.  However, a schema=? constraint should cause this table to
** operate on a different database schema, so check for it.
**
** idxNum is normally 0, but will be 1 if a schema=? constraint exists.
*/
static int statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  int i;

  pIdxInfo->estimatedCost = 1.0e6;  /* Initial cost estimate */

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  for(i=0; i<pIdxInfo->nConstraint; i++){
    if( pIdxInfo->aConstraint[i].usable==0 ) continue;
    if( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    if( pIdxInfo->aConstraint[i].iColumn!=10 ) continue;
    pIdxInfo->idxNum = 1;
    pIdxInfo->estimatedCost = 1.0;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    break;
  }


  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  if( ( pIdxInfo->nOrderBy==1
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     ) ||
      ( pIdxInfo->nOrderBy==2
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     && pIdxInfo->aOrderBy[1].iColumn==1
     && pIdxInfo->aOrderBy[1].desc==0
     )
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  return SQLITE_OK;
}

/*
** Open a new statvfs cursor.
*/
static int statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(sizeof(StatCursor));
  if( pCsr==0 ){
    return SQLITE_NOMEM;
  }else{
    memset(pCsr, 0, sizeof(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  return SQLITE_OK;
}

static void statClearPage(StatPage *p){
  int i;
  if( p->aCell ){
    for(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  sqlite3PagerUnref(p->pPg);
  sqlite3_free(p->zPath);
  memset(p, 0, sizeof(StatPage));
}

static void statResetCsr(StatCursor *pCsr){
  int i;
  sqlite3_reset(pCsr->pStmt);
  for(i=0; i<ArraySize(pCsr->aPage); i++){
    statClearPage(&pCsr->aPage[i]);
  }
  pCsr->iPage = 0;
  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;
  pCsr->isEof = 0;
}

/*
** Close a statvfs cursor.
*/
static int statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

static void getLocalPayload(
  int nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  int nTotal,                     /* Total record (payload) size */
  int *pnLocal                    /* OUT: Bytes stored locally */
){
  int nLocal;
  int nMinLocal;
  int nMaxLocal;
 
  if( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }else{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  if( nLocal>nMaxLocal ) nLocal = nMinLocal;
  *pnLocal = nLocal;
}

static int statDecodePage(Btree *pBt, StatPage *p){
  int nUnused;
  int iOff;
  int nHdr;
  int isLeaf;
  int szPage;

  u8 *aData = sqlite3PagerGetData(p->pPg);
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;

  isLeaf = (p->flags==0x0A || p->flags==0x0D);
  nHdr = 12 - isLeaf*4 + (p->iPgno==1)*100;

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (int)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  while( iOff ){
    nUnused += get2byte(&aData[iOff+2]);
    iOff = get2byte(&aData[iOff]);
  }
  p->nUnused = nUnused;
  p->iRightChildPg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);
  szPage = sqlite3BtreeGetPageSize(pBt);

  if( p->nCell ){
    int i;                        /* Used to iterate through cells */
    int nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * sizeof(StatCell));
    if( p->aCell==0 ) return SQLITE_NOMEM;
    memset(p->aCell, 0, (p->nCell+1) * sizeof(StatCell));

    for(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      if( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      if( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }else{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        int nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        if( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        if( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        getLocalPayload(nUsable, p->flags, nPayload, &nLocal);
        pCell->nLocal = nLocal;
        assert( nLocal>=0 );
        assert( nPayload>=(u32)nLocal );
        assert( nLocal<=(nUsable-35) );
        if( nPayload>(u32)nLocal ){
          int j;
          int nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(sizeof(u32)*nOvfl);
          if( pCell->aOvfl==0 ) return SQLITE_NOMEM;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          for(j=1; j<nOvfl; j++){
            int rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg);
            if( rc!=SQLITE_OK ){
              assert( pPg==0 );
              return rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  return SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
static void statSizeAndOffset(StatCursor *pCsr){
  StatTable *pTab = (StatTable *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* The default page size and offset */
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->iOffset = (i64)pCsr->szPage * (pCsr->iPageno - 1);

  /* If connected to a ZIPVFS backend, override the page size and
  ** offset with actual values obtained from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  if( fd->pMethods!=0 && sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage = (int)x[1];
  }
}

/*
** Move a statvfs cursor to the next entry in the file.
*/
static int statNext(sqlite3_vtab_cursor *pCursor){
  int rc;
  int nPayload;
  char *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  if( pCsr->aPage[0].pPg==0 ){
    rc = sqlite3_step(pCsr->pStmt);
    if( rc==SQLITE_ROW ){
      int nPage;
      u32 iRoot = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      if( nPage==0 ){
        pCsr->isEof = 1;
        return sqlite3_reset(pCsr->pStmt);
      }
      rc = sqlite3PagerGet(pPager, iRoot, &pCsr->aPage[0].pPg);
      pCsr->aPage[0].iPgno = iRoot;
      pCsr->aPage[0].iCell = 0;
      pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
      pCsr->iPage = 0;
      if( z==0 ) rc = SQLITE_NOMEM;
    }else{
      pCsr->isEof = 1;
      return sqlite3_reset(pCsr->pStmt);
    }
  }else{

    /* Page p itself has already been visited. */
    StatPage *p = &pCsr->aPage[pCsr->iPage];

    while( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      if( pCell->iOvfl<pCell->nOvfl ){
        int nUsable;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->zName = (char *)sqlite3_column_text(pCsr->pStmt, 0);
        pCsr->iPageno = pCell->aOvfl[pCell->iOvfl];
        pCsr->zPagetype = "overflow";
        pCsr->nCell = 0;
        pCsr->nMxPayload = 0;
        pCsr->zPath = z = sqlite3_mprintf(
            "%s%.3x+%.6x", p->zPath, p->iCell, pCell->iOvfl
        );
        if( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nUnused = 0;
          pCsr->nPayload = nUsable - 4;
        }else{
          pCsr->nPayload = pCell->nLastOvfl;
          pCsr->nUnused = nUsable - 4 - pCsr->nPayload;
        }
        pCell->iOvfl++;
        statSizeAndOffset(pCsr);
        return z==0 ? SQLITE_NOMEM : SQLITE_OK;
      }
      if( p->iRightChildPg ) break;
      p->iCell++;
    }

    if( !p->iRightChildPg || p->iCell>p->nCell ){
      statClearPage(p);
      if( pCsr->iPage==0 ) return statNext(pCursor);
      pCsr->iPage--;
      goto statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    assert( p==&pCsr->aPage[pCsr->iPage-1] );

    if( p->iCell==p->nCell ){
      p[1].iPgno = p->iRightChildPg;
    }else{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = sqlite3PagerGet(pPager, p[1].iPgno, &p[1].pPg);
    p[1].iCell = 0;
    p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
    p->iCell++;
    if( z==0 ) rc = SQLITE_NOMEM;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  if( rc==SQLITE_OK ){
    int i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (char *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    if( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      switch( p->flags ){
        case 0x05:             /* table internal */
        case 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          break;
        case 0x0D:             /* table leaf */
        case 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          break;
        default:
          pCsr->zPagetype = "corrupted";
          break;
      }
      pCsr->nCell = p->nCell;
      pCsr->nUnused = p->nUnused;
      pCsr->nMxPayload = p->nMxPayload;
      pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
      if( z==0 ) rc = SQLITE_NOMEM;
      nPayload = 0;
      for(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload = nPayload;
    }
  }

  return rc;
}

static int statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  return pCsr->isEof;
}

static int statFilter(
  sqlite3_vtab_cursor *pCursor, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  char *zSql;
  int rc = SQLITE_OK;
  char *zMaster;

  if( idxNum==1 ){
    const char *zDbase = (const char*)sqlite3_value_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    if( pCsr->iDb<0 ){
      sqlite3_free(pCursor->pVtab->zErrMsg);
      pCursor->pVtab->zErrMsg = sqlite3_mprintf("no such schema: %s", zDbase);
      return pCursor->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
    }
  }else{
    pCsr->iDb = pTab->iDb;
  }
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  zMaster = pCsr->iDb==1 ? "sqlite_temp_master" : "sqlite_master";
  zSql = sqlite3_mprintf(
      "SELECT 'sqlite_master' AS name, 1 AS rootpage, 'table' AS type"
      "  UNION ALL  "
      "SELECT name, rootpage, type"
      "  FROM \"%w\".%s WHERE rootpage!=0"
      "  ORDER BY name", pTab->db->aDb[pCsr->iDb].zName, zMaster);
  if( zSql==0 ){
    return SQLITE_NOMEM;
  }else{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  if( rc==SQLITE_OK ){
    rc = statNext(pCursor);
  }
  return rc;
}

static int statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  int i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  switch( i ){
    case 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      break;
    case 1:            /* path */
      sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      break;
    case 2:            /* pageno */
      sqlite3_result_int64(ctx, pCsr->iPageno);
      break;
    case 3:            /* pagetype */
      sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      break;
    case 4:            /* ncell */
      sqlite3_result_int(ctx, pCsr->nCell);
      break;
    case 5:            /* payload */
      sqlite3_result_int(ctx, pCsr->nPayload);
      break;
    case 6:            /* unused */
      sqlite3_result_int(ctx, pCsr->nUnused);
      break;
    case 7:            /* mx_payload */
      sqlite3_result_int(ctx, pCsr->nMxPayload);
      break;
    case 8:            /* pgoffset */
      sqlite3_result_int64(ctx, pCsr->iOffset);
      break;
    case 9:            /* pgsize */
      sqlite3_result_int(ctx, pCsr->szPage);
      break;
    default: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      int iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zName, -1, SQLITE_STATIC);
      break;
    }
  }
  return SQLITE_OK;
}

static int statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  return SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
SQLITE_PRIVATE int sqlite3DbstatRegister(sqlite3 *db){
  static sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    statBestIndex,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    statNext,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    statColumn,                   /* xColumn - read data */
    statRowid,                    /* xRowid - read data */
    0,                            /* xUpdate */
    0,                            /* xBegin */
    0,                            /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
  };
  return sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#elif defined(SQLITE_ENABLE_DBSTAT_VTAB)
SQLITE_PRIVATE int sqlite3DbstatRegister(sqlite3 *db){ return SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */

/************** End of dbstat.c **********************************************/
/************** Begin file sqlite3session.c **********************************/

#if defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
/* #include "sqlite3session.h" */
/* #include <assert.h> */
/* #include <string.h> */

#ifndef SQLITE_AMALGAMATION
/* # include "sqliteInt.h" */
/* # include "vdbeInt.h" */
#endif

typedef struct SessionTable SessionTable;
typedef struct SessionChange SessionChange;
typedef struct SessionBuffer SessionBuffer;
typedef struct SessionInput SessionInput;

/*
** Minimum chunk size used by streaming versions of functions.
*/
#ifndef SESSIONS_STRM_CHUNK_SIZE
# ifdef SQLITE_TEST
#   define SESSIONS_STRM_CHUNK_SIZE 64
# else
#   define SESSIONS_STRM_CHUNK_SIZE 1024
# endif
#endif

typedef struct SessionHook SessionHook;
struct SessionHook {
  void *pCtx;
  int (*xOld)(void*,int,sqlite3_value**);
  int (*xNew)(void*,int,sqlite3_value**);
  int (*xCount)(void*);
  int (*xDepth)(void*);
};

/*
** Session handle structure.
*/
struct sqlite3_session {
  sqlite3 *db;                    /* Database handle session is attached to */
  char *zDb;                      /* Name of database session is attached to */
  int bEnable;                    /* True if currently recording */
  int bIndirect;                  /* True if all changes are indirect */
  int bAutoAttach;                /* True to auto-attach tables */
  int rc;                         /* Non-zero if an error has occurred */
  void *pFilterCtx;               /* First argument to pass to xTableFilter */
  int (*xTableFilter)(void *pCtx, const char *zTab);
  sqlite3_session *pNext;         /* Next session object on same db. */
  SessionTable *pTable;           /* List of attached tables */
  SessionHook hook;               /* APIs to grab new and old data with */
};

/*
** Instances of this structure are used to build strings or binary records.
*/
struct SessionBuffer {
  u8 *aBuf;                       /* Pointer to changeset buffer */
  int nBuf;                       /* Size of buffer aBuf */
  int nAlloc;                     /* Size of allocation containing aBuf */
};

/*
** An object of this type is used internally as an abstraction for 
** input data. Input data may be supplied either as a single large buffer
** (e.g. sqlite3changeset_start()) or using a stream function (e.g.
**  sqlite3changeset_start_strm()).
*/
struct SessionInput {
  int iNext;                      /* Offset in aData[] of next change */
  u8 *aData;                      /* Pointer to buffer containing changeset */
  int nData;                      /* Number of bytes in aData */

  SessionBuffer buf;              /* Current read buffer */
  int (*xInput)(void*, void*, int*);        /* Input stream call (or NULL) */
  void *pIn;                                /* First argument to xInput */
  int bEof;                       /* Set to true after xInput finished */
};

/*
** Structure for changeset iterators.
*/
struct sqlite3_changeset_iter {
  SessionInput in;                /* Input buffer or stream */
  SessionBuffer tblhdr;           /* Buffer to hold apValue/zTab/abPK/ */
  int bPatchset;                  /* True if this is a patchset */
  int rc;                         /* Iterator error code */
  sqlite3_stmt *pConflict;        /* Points to conflicting row, if any */
  char *zTab;                     /* Current table */
  int nCol;                       /* Number of columns in zTab */
  int op;                         /* Current operation */
  int bIndirect;                  /* True if current change was indirect */
  u8 *abPK;                       /* Primary key array */
  sqlite3_value **apValue;        /* old.* and new.* values */
};

/*
** Each session object maintains a set of the following structures, one
** for each table the session object is monitoring. The structures are
** stored in a linked list starting at sqlite3_session.pTable.
**
** The keys of the SessionTable.aChange[] hash table are all rows that have
** been modified in any way since the session object was attached to the
** table.
**
** The data associated with each hash-table entry is a structure containing
** a subset of the initial values that the modified row contained at the
** start of the session. Or no initial values if the row was inserted.
*/
struct SessionTable {
  SessionTable *pNext;
  char *zName;                    /* Local name of table */
  int nCol;                       /* Number of columns in table zName */
  const char **azCol;             /* Column names */
  u8 *abPK;                       /* Array of primary key flags */
  int nEntry;                     /* Total number of entries in hash table */
  int nChange;                    /* Size of apChange[] array */
  SessionChange **apChange;       /* Hash table buckets */
};

/* 
** RECORD FORMAT:
**
** The following record format is similar to (but not compatible with) that 
** used in SQLite database files. This format is used as part of the 
** change-set binary format, and so must be architecture independent.
**
** Unlike the SQLite database record format, each field is self-contained -
** there is no separation of header and data. Each field begins with a
** single byte describing its type, as follows:
**
**       0x00: Undefined value.
**       0x01: Integer value.
**       0x02: Real value.
**       0x03: Text value.
**       0x04: Blob value.
**       0x05: SQL NULL value.
**
** Note that the above match the definitions of SQLITE_INTEGER, SQLITE_TEXT
** and so on in sqlite3.h. For undefined and NULL values, the field consists
** only of the single type byte. For other types of values, the type byte
** is followed by:
**
**   Text values:
**     A varint containing the number of bytes in the value (encoded using
**     UTF-8). Followed by a buffer containing the UTF-8 representation
**     of the text value. There is no nul terminator.
**
**   Blob values:
**     A varint containing the number of bytes in the value, followed by
**     a buffer containing the value itself.
**
**   Integer values:
**     An 8-byte big-endian integer value.
**
**   Real values:
**     An 8-byte big-endian IEEE 754-2008 real value.
**
** Varint values are encoded in the same way as varints in the SQLite 
** record format.
**
** CHANGESET FORMAT:
**
** A changeset is a collection of DELETE, UPDATE and INSERT operations on
** one or more tables. Operations on a single table are grouped together,
** but may occur in any order (i.e. deletes, updates and inserts are all
** mixed together).
**
** Each group of changes begins with a table header:
**
**   1 byte: Constant 0x54 (capital 'T')
**   Varint: Number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more changes to the table.
**
**   1 byte: Either SQLITE_INSERT, UPDATE or DELETE.
**   1 byte: The "indirect-change" flag.
**   old.* record: (delete and update only)
**   new.* record: (insert and update only)
**
** The "old.*" and "new.*" records, if present, are N field records in the
** format described above under "RECORD FORMAT", where N is the number of
** columns in the table. The i'th field of each record is associated with
** the i'th column of the table, counting from left to right in the order
** in which columns were declared in the CREATE TABLE statement.
**
** The new.* record that is part of each INSERT change contains the values
** that make up the new row. Similarly, the old.* record that is part of each
** DELETE change contains the values that made up the row that was deleted 
** from the database. In the changeset format, the records that are part
** of INSERT or DELETE changes never contain any undefined (type byte 0x00)
** fields.
**
** Within the old.* record associated with an UPDATE change, all fields
** associated with table columns that are not PRIMARY KEY columns and are
** not modified by the UPDATE change are set to "undefined". Other fields
** are set to the values that made up the row before the UPDATE that the
** change records took place. Within the new.* record, fields associated 
** with table columns modified by the UPDATE change contain the new 
** values. Fields associated with table columns that are not modified
** are set to "undefined".
**
** PATCHSET FORMAT:
**
** A patchset is also a collection of changes. It is similar to a changeset,
** but leaves undefined those fields that are not useful if no conflict
** resolution is required when applying the changeset.
**
** Each group of changes begins with a table header:
**
**   1 byte: Constant 0x50 (capital 'P')
**   Varint: Number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more changes to the table.
**
**   1 byte: Either SQLITE_INSERT, UPDATE or DELETE.
**   1 byte: The "indirect-change" flag.
**   single record: (PK fields for DELETE, PK and modified fields for UPDATE,
**                   full record for INSERT).
**
** As in the changeset format, each field of the single record that is part
** of a patchset change is associated with the correspondingly positioned
** table column, counting from left to right within the CREATE TABLE 
** statement.
**
** For a DELETE change, all fields within the record except those associated
** with PRIMARY KEY columns are set to "undefined". The PRIMARY KEY fields
** contain the values identifying the row to delete.
**
** For an UPDATE change, all fields except those associated with PRIMARY KEY
** columns and columns that are modified by the UPDATE are set to "undefined".
** PRIMARY KEY fields contain the values identifying the table row to update,
** and fields associated with modified columns contain the new column values.
**
** The records associated with INSERT changes are in the same format as for
** changesets. It is not possible for a record associated with an INSERT
** change to contain a field set to "undefined".
*/

/*
** For each row modified during a session, there exists a single instance of
** this structure stored in a SessionTable.aChange[] hash table.
*/
struct SessionChange {
  int op;                         /* One of UPDATE, DELETE, INSERT */
  int bIndirect;                  /* True if this change is "indirect" */
  int nRecord;                    /* Number of bytes in buffer aRecord[] */
  u8 *aRecord;                    /* Buffer containing old.* record */
  SessionChange *pNext;           /* For hash-table collisions */
};

/*
** Write a varint with value iVal into the buffer at aBuf. Return the 
** number of bytes written.
*/
static int sessionVarintPut(u8 *aBuf, int iVal){
  return putVarint32(aBuf, iVal);
}

/*
** Return the number of bytes required to store value iVal as a varint.
*/
static int sessionVarintLen(int iVal){
  return sqlite3VarintLen(iVal);
}

/*
** Read a varint value from aBuf[] into *piVal. Return the number of 
** bytes read.
*/
static int sessionVarintGet(u8 *aBuf, int *piVal){
  return getVarint32(aBuf, *piVal);
}

/*
** Read a 64-bit big-endian integer value from buffer aRec[]. Return
** the value read.
*/
static sqlite3_int64 sessionGetI64(u8 *aRec){
  return (((sqlite3_int64)aRec[0]) << 56)
       + (((sqlite3_int64)aRec[1]) << 48)
       + (((sqlite3_int64)aRec[2]) << 40)
       + (((sqlite3_int64)aRec[3]) << 32)
       + (((sqlite3_int64)aRec[4]) << 24)
       + (((sqlite3_int64)aRec[5]) << 16)
       + (((sqlite3_int64)aRec[6]) <<  8)
       + (((sqlite3_int64)aRec[7]) <<  0);
}

/*
** Write a 64-bit big-endian integer value to the buffer aBuf[].
*/
static void sessionPutI64(u8 *aBuf, sqlite3_int64 i){
  aBuf[0] = (i>>56) & 0xFF;
  aBuf[1] = (i>>48) & 0xFF;
  aBuf[2] = (i>>40) & 0xFF;
  aBuf[3] = (i>>32) & 0xFF;
  aBuf[4] = (i>>24) & 0xFF;
  aBuf[5] = (i>>16) & 0xFF;
  aBuf[6] = (i>> 8) & 0xFF;
  aBuf[7] = (i>> 0) & 0xFF;
}

/*
** This function is used to serialize the contents of value pValue (see
** comment titled "RECORD FORMAT" above).
**
** If it is non-NULL, the serialized form of the value is written to 
** buffer aBuf. *pnWrite is set to the number of bytes written before
** returning. Or, if aBuf is NULL, the only thing this function does is
** set *pnWrite.
**
** If no error occurs, SQLITE_OK is returned. Or, if an OOM error occurs
** within a call to sqlite3_value_text() (may fail if the db is utf-16)) 
** SQLITE_NOMEM is returned.
*/
static int sessionSerializeValue(
  u8 *aBuf,                       /* If non-NULL, write serialized value here */
  sqlite3_value *pValue,          /* Value to serialize */
  int *pnWrite                    /* IN/OUT: Increment by bytes written */
){
  int nByte;                      /* Size of serialized value in bytes */

  if( pValue ){
    int eType;                    /* Value type (SQLITE_NULL, TEXT etc.) */
  
    eType = sqlite3_value_type(pValue);
    if( aBuf ) aBuf[0] = eType;
  
    switch( eType ){
      case SQLITE_NULL: 
        nByte = 1;
        break;
  
      case SQLITE_INTEGER: 
      case SQLITE_FLOAT:
        if( aBuf ){
          /* TODO: SQLite does something special to deal with mixed-endian
          ** floating point values (e.g. ARM7). This code probably should
          ** too.  */
          u64 i;
          if( eType==SQLITE_INTEGER ){
            i = (u64)sqlite3_value_int64(pValue);
          }else{
            double r;
            assert( sizeof(double)==8 && sizeof(u64)==8 );
            r = sqlite3_value_double(pValue);
            memcpy(&i, &r, 8);
          }
          sessionPutI64(&aBuf[1], i);
        }
        nByte = 9; 
        break;
  
      default: {
        u8 *z;
        int n;
        int nVarint;
  
        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        if( eType==SQLITE_TEXT ){
          z = (u8 *)sqlite3_value_text(pValue);
        }else{
          z = (u8 *)sqlite3_value_blob(pValue);
        }
        n = sqlite3_value_bytes(pValue);
        if( z==0 && (eType!=SQLITE_BLOB || n>0) ) return SQLITE_NOMEM;
        nVarint = sessionVarintLen(n);
  
        if( aBuf ){
          sessionVarintPut(&aBuf[1], n);
          memcpy(&aBuf[nVarint + 1], eType==SQLITE_TEXT ? 
              sqlite3_value_text(pValue) : sqlite3_value_blob(pValue), n
          );
        }
  
        nByte = 1 + nVarint + n;
        break;
      }
    }
  }else{
    nByte = 1;
    if( aBuf ) aBuf[0] = '\0';
  }

  if( pnWrite ) *pnWrite += nByte;
  return SQLITE_OK;
}


/*
** This macro is used to calculate hash key values for data structures. In
** order to use this macro, the entire data structure must be represented
** as a series of unsigned integers. In order to calculate a hash-key value
** for a data structure represented as three such integers, the macro may
** then be used as follows:
**
**    int hash_key_value;
**    hash_key_value = HASH_APPEND(0, <value 1>);
**    hash_key_value = HASH_APPEND(hash_key_value, <value 2>);
**    hash_key_value = HASH_APPEND(hash_key_value, <value 3>);
**
** In practice, the data structures this macro is used for are the primary
** key values of modified rows.
*/
#define HASH_APPEND(hash, add) ((hash) << 3) ^ (hash) ^ (unsigned int)(add)

/*
** Append the hash of the 64-bit integer passed as the second argument to the
** hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendI64(unsigned int h, i64 i){
  h = HASH_APPEND(h, i & 0xFFFFFFFF);
  return HASH_APPEND(h, (i>>32)&0xFFFFFFFF);
}

/*
** Append the hash of the blob passed via the second and third arguments to 
** the hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendBlob(unsigned int h, int n, const u8 *z){
  int i;
  for(i=0; i<n; i++) h = HASH_APPEND(h, z[i]);
  return h;
}

/*
** Append the hash of the data type passed as the second argument to the
** hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendType(unsigned int h, int eType){
  return HASH_APPEND(h, eType);
}

/*
** This function may only be called from within a pre-update callback.
** It calculates a hash based on the primary key values of the old.* or 
** new.* row currently available and, assuming no error occurs, writes it to
** *piHash before returning. If the primary key contains one or more NULL
** values, *pbNullPK is set to true before returning.
**
** If an error occurs, an SQLite error code is returned and the final values
** of *piHash asn *pbNullPK are undefined. Otherwise, SQLITE_OK is returned
** and the output variables are set as described above.
*/
static int sessionPreupdateHash(
  sqlite3_session *pSession,      /* Session object that owns pTab */
  SessionTable *pTab,             /* Session table handle */
  int bNew,                       /* True to hash the new.* PK */
  int *piHash,                    /* OUT: Hash value */
  int *pbNullPK                   /* OUT: True if there are NULL values in PK */
){
  unsigned int h = 0;             /* Hash value to return */
  int i;                          /* Used to iterate through columns */

  assert( *pbNullPK==0 );
  assert( pTab->nCol==pSession->hook.xCount(pSession->hook.pCtx) );
  for(i=0; i<pTab->nCol; i++){
    if( pTab->abPK[i] ){
      int rc;
      int eType;
      sqlite3_value *pVal;

      if( bNew ){
        rc = pSession->hook.xNew(pSession->hook.pCtx, i, &pVal);
      }else{
        rc = pSession->hook.xOld(pSession->hook.pCtx, i, &pVal);
      }
      if( rc!=SQLITE_OK ) return rc;

      eType = sqlite3_value_type(pVal);
      h = sessionHashAppendType(h, eType);
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        i64 iVal;
        if( eType==SQLITE_INTEGER ){
          iVal = sqlite3_value_int64(pVal);
        }else{
          double rVal = sqlite3_value_double(pVal);
          assert( sizeof(iVal)==8 && sizeof(rVal)==8 );
          memcpy(&iVal, &rVal, 8);
        }
        h = sessionHashAppendI64(h, iVal);
      }else if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        const u8 *z;
        int n;
        if( eType==SQLITE_TEXT ){
          z = (const u8 *)sqlite3_value_text(pVal);
        }else{
          z = (const u8 *)sqlite3_value_blob(pVal);
        }
        n = sqlite3_value_bytes(pVal);
        if( !z && (eType!=SQLITE_BLOB || n>0) ) return SQLITE_NOMEM;
        h = sessionHashAppendBlob(h, n, z);
      }else{
        assert( eType==SQLITE_NULL );
        *pbNullPK = 1;
      }
    }
  }

  *piHash = (h % pTab->nChange);
  return SQLITE_OK;
}

/*
** The buffer that the argument points to contains a serialized SQL value.
** Return the number of bytes of space occupied by the value (including
** the type byte).
*/
static int sessionSerialLen(u8 *a){
  int e = *a;
  int n;
  if( e==0 ) return 1;
  if( e==SQLITE_NULL ) return 1;
  if( e==SQLITE_INTEGER || e==SQLITE_FLOAT ) return 9;
  return sessionVarintGet(&a[1], &n) + 1 + n;
}

/*
** Based on the primary key values stored in change aRecord, calculate a
** hash key. Assume the has table has nBucket buckets. The hash keys
** calculated by this function are compatible with those calculated by
** sessionPreupdateHash().
**
** The bPkOnly argument is non-zero if the record at aRecord[] is from
** a patchset DELETE. In this case the non-PK fields are omitted entirely.
*/
static unsigned int sessionChangeHash(
  SessionTable *pTab,             /* Table handle */
  int bPkOnly,                    /* Record consists of PK fields only */
  u8 *aRecord,                    /* Change record */
  int nBucket                     /* Assume this many buckets in hash table */
){
  unsigned int h = 0;             /* Value to return */
  int i;                          /* Used to iterate through columns */
  u8 *a = aRecord;                /* Used to iterate through change record */

  for(i=0; i<pTab->nCol; i++){
    int eType = *a;
    int isPK = pTab->abPK[i];
    if( bPkOnly && isPK==0 ) continue;

    /* It is not possible for eType to be SQLITE_NULL here. The session 
    ** module does not record changes for rows with NULL values stored in
    ** primary key columns. */
    assert( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT 
         || eType==SQLITE_TEXT || eType==SQLITE_BLOB 
         || eType==SQLITE_NULL || eType==0 
    );
    assert( !isPK || (eType!=0 && eType!=SQLITE_NULL) );

    if( isPK ){
      a++;
      h = sessionHashAppendType(h, eType);
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        h = sessionHashAppendI64(h, sessionGetI64(a));
        a += 8;
      }else{
        int n; 
        a += sessionVarintGet(a, &n);
        h = sessionHashAppendBlob(h, n, a);
        a += n;
      }
    }else{
      a += sessionSerialLen(a);
    }
  }
  return (h % nBucket);
}

/*
** Arguments aLeft and aRight are pointers to change records for table pTab.
** This function returns true if the two records apply to the same row (i.e.
** have the same values stored in the primary key columns), or false 
** otherwise.
*/
static int sessionChangeEqual(
  SessionTable *pTab,             /* Table used for PK definition */
  int bLeftPkOnly,                /* True if aLeft[] contains PK fields only */
  u8 *aLeft,                      /* Change record */
  int bRightPkOnly,               /* True if aRight[] contains PK fields only */
  u8 *aRight                      /* Change record */
){
  u8 *a1 = aLeft;                 /* Cursor to iterate through aLeft */
  u8 *a2 = aRight;                /* Cursor to iterate through aRight */
  int iCol;                       /* Used to iterate through table columns */

  for(iCol=0; iCol<pTab->nCol; iCol++){
    int n1 = sessionSerialLen(a1);
    int n2 = sessionSerialLen(a2);

    if( pTab->abPK[iCol] && (n1!=n2 || memcmp(a1, a2, n1)) ){
      return 0;
    }
    if( pTab->abPK[iCol] || bLeftPkOnly==0 ) a1 += n1;
    if( pTab->abPK[iCol] || bRightPkOnly==0 ) a2 += n2;
  }

  return 1;
}

/*
** Arguments aLeft and aRight both point to buffers containing change
** records with nCol columns. This function "merges" the two records into
** a single records which is written to the buffer at *paOut. *paOut is
** then set to point to one byte after the last byte written before 
** returning.
**
** The merging of records is done as follows: For each column, if the 
** aRight record contains a value for the column, copy the value from
** their. Otherwise, if aLeft contains a value, copy it. If neither
** record contains a value for a given column, then neither does the
** output record.
*/
static void sessionMergeRecord(
  u8 **paOut, 
  int nCol,
  u8 *aLeft,
  u8 *aRight
){
  u8 *a1 = aLeft;                 /* Cursor used to iterate through aLeft */
  u8 *a2 = aRight;                /* Cursor used to iterate through aRight */
  u8 *aOut = *paOut;              /* Output cursor */
  int iCol;                       /* Used to iterate from 0 to nCol */

  for(iCol=0; iCol<nCol; iCol++){
    int n1 = sessionSerialLen(a1);
    int n2 = sessionSerialLen(a2);
    if( *a2 ){
      memcpy(aOut, a2, n2);
      aOut += n2;
    }else{
      memcpy(aOut, a1, n1);
      aOut += n1;
    }
    a1 += n1;
    a2 += n2;
  }

  *paOut = aOut;
}

/*
** This is a helper function used by sessionMergeUpdate().
**
** When this function is called, both *paOne and *paTwo point to a value 
** within a change record. Before it returns, both have been advanced so 
** as to point to the next value in the record.
**
** If, when this function is called, *paTwo points to a valid value (i.e.
** *paTwo[0] is not 0x00 - the "no value" placeholder), a copy of the *paTwo
** pointer is returned and *pnVal is set to the number of bytes in the 
** serialized value. Otherwise, a copy of *paOne is returned and *pnVal
** set to the number of bytes in the value at *paOne. If *paOne points
** to the "no value" placeholder, *pnVal is set to 1. In other words:
**
**   if( *paTwo is valid ) return *paTwo;
**   return *paOne;
**
*/
static u8 *sessionMergeValue(
  u8 **paOne,                     /* IN/OUT: Left-hand buffer pointer */
  u8 **paTwo,                     /* IN/OUT: Right-hand buffer pointer */
  int *pnVal                      /* OUT: Bytes in returned value */
){
  u8 *a1 = *paOne;
  u8 *a2 = *paTwo;
  u8 *pRet = 0;
  int n1;

  assert( a1 );
  if( a2 ){
    int n2 = sessionSerialLen(a2);
    if( *a2 ){
      *pnVal = n2;
      pRet = a2;
    }
    *paTwo = &a2[n2];
  }

  n1 = sessionSerialLen(a1);
  if( pRet==0 ){
    *pnVal = n1;
    pRet = a1;
  }
  *paOne = &a1[n1];

  return pRet;
}

/*
** This function is used by changeset_concat() to merge two UPDATE changes
** on the same row.
*/
static int sessionMergeUpdate(
  u8 **paOut,                     /* IN/OUT: Pointer to output buffer */
  SessionTable *pTab,             /* Table change pertains to */
  int bPatchset,                  /* True if records are patchset records */
  u8 *aOldRecord1,                /* old.* record for first change */
  u8 *aOldRecord2,                /* old.* record for second change */
  u8 *aNewRecord1,                /* new.* record for first change */
  u8 *aNewRecord2                 /* new.* record for second change */
){
  u8 *aOld1 = aOldRecord1;
  u8 *aOld2 = aOldRecord2;
  u8 *aNew1 = aNewRecord1;
  u8 *aNew2 = aNewRecord2;

  u8 *aOut = *paOut;
  int i;

  if( bPatchset==0 ){
    int bRequired = 0;

    assert( aOldRecord1 && aNewRecord1 );

    /* Write the old.* vector first. */
    for(i=0; i<pTab->nCol; i++){
      int nOld;
      u8 *aOld;
      int nNew;
      u8 *aNew;

      aOld = sessionMergeValue(&aOld1, &aOld2, &nOld);
      aNew = sessionMergeValue(&aNew1, &aNew2, &nNew);
      if( pTab->abPK[i] || nOld!=nNew || memcmp(aOld, aNew, nNew) ){
        if( pTab->abPK[i]==0 ) bRequired = 1;
        memcpy(aOut, aOld, nOld);
        aOut += nOld;
      }else{
        *(aOut++) = '\0';
      }
    }

    if( !bRequired ) return 0;
  }

  /* Write the new.* vector */
  aOld1 = aOldRecord1;
  aOld2 = aOldRecord2;
  aNew1 = aNewRecord1;
  aNew2 = aNewRecord2;
  for(i=0; i<pTab->nCol; i++){
    int nOld;
    u8 *aOld;
    int nNew;
    u8 *aNew;

    aOld = sessionMergeValue(&aOld1, &aOld2, &nOld);
    aNew = sessionMergeValue(&aNew1, &aNew2, &nNew);
    if( bPatchset==0 
     && (pTab->abPK[i] || (nOld==nNew && 0==memcmp(aOld, aNew, nNew))) 
    ){
      *(aOut++) = '\0';
    }else{
      memcpy(aOut, aNew, nNew);
      aOut += nNew;
    }
  }

  *paOut = aOut;
  return 1;
}

/*
** This function is only called from within a pre-update-hook callback.
** It determines if the current pre-update-hook change affects the same row
** as the change stored in argument pChange. If so, it returns true. Otherwise
** if the pre-update-hook does not affect the same row as pChange, it returns
** false.
*/
static int sessionPreupdateEqual(
  sqlite3_session *pSession,      /* Session object that owns SessionTable */
  SessionTable *pTab,             /* Table associated with change */
  SessionChange *pChange,         /* Change to compare to */
  int op                          /* Current pre-update operation */
){
  int iCol;                       /* Used to iterate through columns */
  u8 *a = pChange->aRecord;       /* Cursor used to scan change record */

  assert( op==SQLITE_INSERT || op==SQLITE_UPDATE || op==SQLITE_DELETE );
  for(iCol=0; iCol<pTab->nCol; iCol++){
    if( !pTab->abPK[iCol] ){
      a += sessionSerialLen(a);
    }else{
      sqlite3_value *pVal;        /* Value returned by preupdate_new/old */
      int rc;                     /* Error code from preupdate_new/old */
      int eType = *a++;           /* Type of value from change record */

      /* The following calls to preupdate_new() and preupdate_old() can not
      ** fail. This is because they cache their return values, and by the
      ** time control flows to here they have already been called once from
      ** within sessionPreupdateHash(). The first two asserts below verify
      ** this (that the method has already been called). */
      if( op==SQLITE_INSERT ){
        /* assert( db->pPreUpdate->pNewUnpacked || db->pPreUpdate->aNew ); */
        rc = pSession->hook.xNew(pSession->hook.pCtx, iCol, &pVal);
      }else{
        /* assert( db->pPreUpdate->pUnpacked ); */
        rc = pSession->hook.xOld(pSession->hook.pCtx, iCol, &pVal);
      }
      assert( rc==SQLITE_OK );
      if( sqlite3_value_type(pVal)!=eType ) return 0;

      /* A SessionChange object never has a NULL value in a PK column */
      assert( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT
           || eType==SQLITE_BLOB    || eType==SQLITE_TEXT
      );

      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        i64 iVal = sessionGetI64(a);
        a += 8;
        if( eType==SQLITE_INTEGER ){
          if( sqlite3_value_int64(pVal)!=iVal ) return 0;
        }else{
          double rVal;
          assert( sizeof(iVal)==8 && sizeof(rVal)==8 );
          memcpy(&rVal, &iVal, 8);
          if( sqlite3_value_double(pVal)!=rVal ) return 0;
        }
      }else{
        int n;
        const u8 *z;
        a += sessionVarintGet(a, &n);
        if( sqlite3_value_bytes(pVal)!=n ) return 0;
        if( eType==SQLITE_TEXT ){
          z = sqlite3_value_text(pVal);
        }else{
          z = sqlite3_value_blob(pVal);
        }
        if( memcmp(a, z, n) ) return 0;
        a += n;
        break;
      }
    }
  }

  return 1;
}

/*
** If required, grow the hash table used to store changes on table pTab 
** (part of the session pSession). If a fatal OOM error occurs, set the
** session object to failed and return SQLITE_ERROR. Otherwise, return
** SQLITE_OK.
**
** It is possible that a non-fatal OOM error occurs in this function. In
** that case the hash-table does not grow, but SQLITE_OK is returned anyway.
** Growing the hash table in this case is a performance optimization only,
** it is not required for correct operation.
*/
static int sessionGrowHash(int bPatchset, SessionTable *pTab){
  if( pTab->nChange==0 || pTab->nEntry>=(pTab->nChange/2) ){
    int i;
    SessionChange **apNew;
    int nNew = (pTab->nChange ? pTab->nChange : 128) * 2;

    apNew = (SessionChange **)sqlite3_malloc(sizeof(SessionChange *) * nNew);
    if( apNew==0 ){
      if( pTab->nChange==0 ){
        return SQLITE_ERROR;
      }
      return SQLITE_OK;
    }
    memset(apNew, 0, sizeof(SessionChange *) * nNew);

    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      SessionChange *pNext;
      for(p=pTab->apChange[i]; p; p=pNext){
        int bPkOnly = (p->op==SQLITE_DELETE && bPatchset);
        int iHash = sessionChangeHash(pTab, bPkOnly, p->aRecord, nNew);
        pNext = p->pNext;
        p->pNext = apNew[iHash];
        apNew[iHash] = p;
      }
    }

    sqlite3_free(pTab->apChange);
    pTab->nChange = nNew;
    pTab->apChange = apNew;
  }

  return SQLITE_OK;
}

/*
** This function queries the database for the names of the columns of table
** zThis, in schema zDb. It is expected that the table has nCol columns. If
** not, SQLITE_SCHEMA is returned and none of the output variables are
** populated.
**
** Otherwise, if they are not NULL, variable *pnCol is set to the number
** of columns in the database table and variable *pzTab is set to point to a
** nul-terminated copy of the table name. *pazCol (if not NULL) is set to
** point to an array of pointers to column names. And *pabPK (again, if not
** NULL) is set to point to an array of booleans - true if the corresponding
** column is part of the primary key.
**
** For example, if the table is declared as:
**
**     CREATE TABLE tbl1(w, x, y, z, PRIMARY KEY(w, z));
**
** Then the four output variables are populated as follows:
**
**     *pnCol  = 4
**     *pzTab  = "tbl1"
**     *pazCol = {"w", "x", "y", "z"}
**     *pabPK  = {1, 0, 0, 1}
**
** All returned buffers are part of the same single allocation, which must
** be freed using sqlite3_free() by the caller. If pazCol was not NULL, then
** pointer *pazCol should be freed to release all memory. Otherwise, pointer
** *pabPK. It is illegal for both pazCol and pabPK to be NULL.
*/
static int sessionTableInfo(
  sqlite3 *db,                    /* Database connection */
  const char *zDb,                /* Name of attached database (e.g. "main") */
  const char *zThis,              /* Table name */
  int *pnCol,                     /* OUT: number of columns */
  const char **pzTab,             /* OUT: Copy of zThis */
  const char ***pazCol,           /* OUT: Array of column names for table */
  u8 **pabPK                      /* OUT: Array of booleans - true for PK col */
){
  char *zPragma;
  sqlite3_stmt *pStmt;
  int rc;
  int nByte;
  int nDbCol = 0;
  int nThis;
  int i;
  u8 *pAlloc;
  char **azCol = 0;
  u8 *abPK;

  assert( pazCol && pabPK );

  nThis = sqlite3Strlen30(zThis);
  zPragma = sqlite3_mprintf("PRAGMA '%q'.table_info('%q')", zDb, zThis);
  if( !zPragma ) return SQLITE_NOMEM;

  rc = sqlite3_prepare_v2(db, zPragma, -1, &pStmt, 0);
  sqlite3_free(zPragma);
  if( rc!=SQLITE_OK ) return rc;

  nByte = nThis + 1;
  while( SQLITE_ROW==sqlite3_step(pStmt) ){
    nByte += sqlite3_column_bytes(pStmt, 1);
    nDbCol++;
  }
  rc = sqlite3_reset(pStmt);

  if( rc==SQLITE_OK ){
    nByte += nDbCol * (sizeof(const char *) + sizeof(u8) + 1);
    pAlloc = sqlite3_malloc(nByte);
    if( pAlloc==0 ){
      rc = SQLITE_NOMEM;
    }
  }
  if( rc==SQLITE_OK ){
    azCol = (char **)pAlloc;
    pAlloc = (u8 *)&azCol[nDbCol];
    abPK = (u8 *)pAlloc;
    pAlloc = &abPK[nDbCol];
    if( pzTab ){
      memcpy(pAlloc, zThis, nThis+1);
      *pzTab = (char *)pAlloc;
      pAlloc += nThis+1;
    }
  
    i = 0;
    while( SQLITE_ROW==sqlite3_step(pStmt) ){
      int nName = sqlite3_column_bytes(pStmt, 1);
      const unsigned char *zName = sqlite3_column_text(pStmt, 1);
      if( zName==0 ) break;
      memcpy(pAlloc, zName, nName+1);
      azCol[i] = (char *)pAlloc;
      pAlloc += nName+1;
      abPK[i] = sqlite3_column_int(pStmt, 5);
      i++;
    }
    rc = sqlite3_reset(pStmt);
  
  }

  /* If successful, populate the output variables. Otherwise, zero them and
  ** free any allocation made. An error code will be returned in this case.
  */
  if( rc==SQLITE_OK ){
    *pazCol = (const char **)azCol;
    *pabPK = abPK;
    *pnCol = nDbCol;
  }else{
    *pazCol = 0;
    *pabPK = 0;
    *pnCol = 0;
    if( pzTab ) *pzTab = 0;
    sqlite3_free(azCol);
  }
  sqlite3_finalize(pStmt);
  return rc;
}

/*
** This function is only called from within a pre-update handler for a
** write to table pTab, part of session pSession. If this is the first
** write to this table, initalize the SessionTable.nCol, azCol[] and
** abPK[] arrays accordingly.
**
** If an error occurs, an error code is stored in sqlite3_session.rc and
** non-zero returned. Or, if no error occurs but the table has no primary
** key, sqlite3_session.rc is left set to SQLITE_OK and non-zero returned to
** indicate that updates on this table should be ignored. SessionTable.abPK 
** is set to NULL in this case.
*/
static int sessionInitTable(sqlite3_session *pSession, SessionTable *pTab){
  if( pTab->nCol==0 ){
    u8 *abPK;
    assert( pTab->azCol==0 || pTab->abPK==0 );
    pSession->rc = sessionTableInfo(pSession->db, pSession->zDb, 
        pTab->zName, &pTab->nCol, 0, &pTab->azCol, &abPK
    );
    if( pSession->rc==SQLITE_OK ){
      int i;
      for(i=0; i<pTab->nCol; i++){
        if( abPK[i] ){
          pTab->abPK = abPK;
          break;
        }
      }
    }
  }
  return (pSession->rc || pTab->abPK==0);
}

/*
** This function is only called from with a pre-update-hook reporting a 
** change on table pTab (attached to session pSession). The type of change
** (UPDATE, INSERT, DELETE) is specified by the first argument.
**
** Unless one is already present or an error occurs, an entry is added
** to the changed-rows hash table associated with table pTab.
*/
static void sessionPreupdateOneChange(
  int op,                         /* One of SQLITE_UPDATE, INSERT, DELETE */
  sqlite3_session *pSession,      /* Session object pTab is attached to */
  SessionTable *pTab              /* Table that change applies to */
){
  int iHash; 
  int bNull = 0; 
  int rc = SQLITE_OK;

  if( pSession->rc ) return;

  /* Load table details if required */
  if( sessionInitTable(pSession, pTab) ) return;

  /* Check the number of columns in this xPreUpdate call matches the 
  ** number of columns in the table.  */
  if( pTab->nCol!=pSession->hook.xCount(pSession->hook.pCtx) ){
    pSession->rc = SQLITE_SCHEMA;
    return;
  }

  /* Grow the hash table if required */
  if( sessionGrowHash(0, pTab) ){
    pSession->rc = SQLITE_NOMEM;
    return;
  }

  /* Calculate the hash-key for this change. If the primary key of the row
  ** includes a NULL value, exit early. Such changes are ignored by the
  ** session module. */
  rc = sessionPreupdateHash(pSession, pTab, op==SQLITE_INSERT, &iHash, &bNull);
  if( rc!=SQLITE_OK ) goto error_out;

  if( bNull==0 ){
    /* Search the hash table for an existing record for this row. */
    SessionChange *pC;
    for(pC=pTab->apChange[iHash]; pC; pC=pC->pNext){
      if( sessionPreupdateEqual(pSession, pTab, pC, op) ) break;
    }

    if( pC==0 ){
      /* Create a new change object containing all the old values (if
      ** this is an SQLITE_UPDATE or SQLITE_DELETE), or just the PK
      ** values (if this is an INSERT). */
      SessionChange *pChange; /* New change object */
      int nByte;              /* Number of bytes to allocate */
      int i;                  /* Used to iterate through columns */
  
      assert( rc==SQLITE_OK );
      pTab->nEntry++;
  
      /* Figure out how large an allocation is required */
      nByte = sizeof(SessionChange);
      for(i=0; i<pTab->nCol; i++){
        sqlite3_value *p = 0;
        if( op!=SQLITE_INSERT ){
          TESTONLY(int trc = ) pSession->hook.xOld(pSession->hook.pCtx, i, &p);
          assert( trc==SQLITE_OK );
        }else if( pTab->abPK[i] ){
          TESTONLY(int trc = ) pSession->hook.xNew(pSession->hook.pCtx, i, &p);
          assert( trc==SQLITE_OK );
        }

        /* This may fail if SQLite value p contains a utf-16 string that must
        ** be converted to utf-8 and an OOM error occurs while doing so. */
        rc = sessionSerializeValue(0, p, &nByte);
        if( rc!=SQLITE_OK ) goto error_out;
      }
  
      /* Allocate the change object */
      pChange = (SessionChange *)sqlite3_malloc(nByte);
      if( !pChange ){
        rc = SQLITE_NOMEM;
        goto error_out;
      }else{
        memset(pChange, 0, sizeof(SessionChange));
        pChange->aRecord = (u8 *)&pChange[1];
      }
  
      /* Populate the change object. None of the preupdate_old(),
      ** preupdate_new() or SerializeValue() calls below may fail as all
      ** required values and encodings have already been cached in memory.
      ** It is not possible for an OOM to occur in this block. */
      nByte = 0;
      for(i=0; i<pTab->nCol; i++){
        sqlite3_value *p = 0;
        if( op!=SQLITE_INSERT ){
          pSession->hook.xOld(pSession->hook.pCtx, i, &p);
        }else if( pTab->abPK[i] ){
          pSession->hook.xNew(pSession->hook.pCtx, i, &p);
        }
        sessionSerializeValue(&pChange->aRecord[nByte], p, &nByte);
      }

      /* Add the change to the hash-table */
      if( pSession->bIndirect || pSession->hook.xDepth(pSession->hook.pCtx) ){
        pChange->bIndirect = 1;
      }
      pChange->nRecord = nByte;
      pChange->op = op;
      pChange->pNext = pTab->apChange[iHash];
      pTab->apChange[iHash] = pChange;

    }else if( pC->bIndirect ){
      /* If the existing change is considered "indirect", but this current
      ** change is "direct", mark the change object as direct. */
      if( pSession->hook.xDepth(pSession->hook.pCtx)==0 
       && pSession->bIndirect==0 
      ){
        pC->bIndirect = 0;
      }
    }
  }

  /* If an error has occurred, mark the session object as failed. */
 error_out:
  if( rc!=SQLITE_OK ){
    pSession->rc = rc;
  }
}

static int sessionFindTable(
  sqlite3_session *pSession, 
  const char *zName,
  SessionTable **ppTab
){
  int rc = SQLITE_OK;
  int nName = sqlite3Strlen30(zName);
  SessionTable *pRet;

  /* Search for an existing table */
  for(pRet=pSession->pTable; pRet; pRet=pRet->pNext){
    if( 0==sqlite3_strnicmp(pRet->zName, zName, nName+1) ) break;
  }

  if( pRet==0 && pSession->bAutoAttach ){
    /* If there is a table-filter configured, invoke it. If it returns 0,
    ** do not automatically add the new table. */
    if( pSession->xTableFilter==0
     || pSession->xTableFilter(pSession->pFilterCtx, zName) 
    ){
      rc = sqlite3session_attach(pSession, zName);
      if( rc==SQLITE_OK ){
        pRet = pSession->pTable;
        assert( 0==sqlite3_strnicmp(pRet->zName, zName, nName+1) );
      }
    }
  }

  assert( rc==SQLITE_OK || pRet==0 );
  *ppTab = pRet;
  return rc;
}

/*
** The 'pre-update' hook registered by this module with SQLite databases.
*/
static void xPreUpdate(
  void *pCtx,                     /* Copy of third arg to preupdate_hook() */
  sqlite3 *db,                    /* Database handle */
  int op,                         /* SQLITE_UPDATE, DELETE or INSERT */
  char const *zDb,                /* Database name */
  char const *zName,              /* Table name */
  sqlite3_int64 iKey1,            /* Rowid of row about to be deleted/updated */
  sqlite3_int64 iKey2             /* New rowid value (for a rowid UPDATE) */
){
  sqlite3_session *pSession;
  int nDb = sqlite3Strlen30(zDb);

  assert( sqlite3_mutex_held(db->mutex) );

  for(pSession=(sqlite3_session *)pCtx; pSession; pSession=pSession->pNext){
    SessionTable *pTab;

    /* If this session is attached to a different database ("main", "temp" 
    ** etc.), or if it is not currently enabled, there is nothing to do. Skip 
    ** to the next session object attached to this database. */
    if( pSession->bEnable==0 ) continue;
    if( pSession->rc ) continue;
    if( sqlite3_strnicmp(zDb, pSession->zDb, nDb+1) ) continue;

    pSession->rc = sessionFindTable(pSession, zName, &pTab);
    if( pTab ){
      assert( pSession->rc==SQLITE_OK );
      sessionPreupdateOneChange(op, pSession, pTab);
      if( op==SQLITE_UPDATE ){
        sessionPreupdateOneChange(SQLITE_INSERT, pSession, pTab);
      }
    }
  }
}

/*
** The pre-update hook implementations.
*/
static int sessionPreupdateOld(void *pCtx, int iVal, sqlite3_value **ppVal){
  return sqlite3_preupdate_old((sqlite3*)pCtx, iVal, ppVal);
}
static int sessionPreupdateNew(void *pCtx, int iVal, sqlite3_value **ppVal){
  return sqlite3_preupdate_new((sqlite3*)pCtx, iVal, ppVal);
}
static int sessionPreupdateCount(void *pCtx){
  return sqlite3_preupdate_count((sqlite3*)pCtx);
}
static int sessionPreupdateDepth(void *pCtx){
  return sqlite3_preupdate_depth((sqlite3*)pCtx);
}

/*
** Install the pre-update hooks on the session object passed as the only
** argument.
*/
static void sessionPreupdateHooks(
  sqlite3_session *pSession
){
  pSession->hook.pCtx = (void*)pSession->db;
  pSession->hook.xOld = sessionPreupdateOld;
  pSession->hook.xNew = sessionPreupdateNew;
  pSession->hook.xCount = sessionPreupdateCount;
  pSession->hook.xDepth = sessionPreupdateDepth;
}

typedef struct SessionDiffCtx SessionDiffCtx;
struct SessionDiffCtx {
  sqlite3_stmt *pStmt;
  int nOldOff;
};

/*
** The diff hook implementations.
*/
static int sessionDiffOld(void *pCtx, int iVal, sqlite3_value **ppVal){
  SessionDiffCtx *p = (SessionDiffCtx*)pCtx;
  *ppVal = sqlite3_column_value(p->pStmt, iVal+p->nOldOff);
  return SQLITE_OK;
}
static int sessionDiffNew(void *pCtx, int iVal, sqlite3_value **ppVal){
  SessionDiffCtx *p = (SessionDiffCtx*)pCtx;
  *ppVal = sqlite3_column_value(p->pStmt, iVal);
   return SQLITE_OK;
}
static int sessionDiffCount(void *pCtx){
  SessionDiffCtx *p = (SessionDiffCtx*)pCtx;
  return p->nOldOff ? p->nOldOff : sqlite3_column_count(p->pStmt);
}
static int sessionDiffDepth(void *pCtx){
  return 0;
}

/*
** Install the diff hooks on the session object passed as the only
** argument.
*/
static void sessionDiffHooks(
  sqlite3_session *pSession,
  SessionDiffCtx *pDiffCtx
){
  pSession->hook.pCtx = (void*)pDiffCtx;
  pSession->hook.xOld = sessionDiffOld;
  pSession->hook.xNew = sessionDiffNew;
  pSession->hook.xCount = sessionDiffCount;
  pSession->hook.xDepth = sessionDiffDepth;
}

static char *sessionExprComparePK(
  int nCol,
  const char *zDb1, const char *zDb2, 
  const char *zTab,
  const char **azCol, u8 *abPK
){
  int i;
  const char *zSep = "";
  char *zRet = 0;

  for(i=0; i<nCol; i++){
    if( abPK[i] ){
      zRet = sqlite3_mprintf("%z%s\"%w\".\"%w\".\"%w\"=\"%w\".\"%w\".\"%w\"",
          zRet, zSep, zDb1, zTab, azCol[i], zDb2, zTab, azCol[i]
      );
      zSep = " AND ";
      if( zRet==0 ) break;
    }
  }

  return zRet;
}

static char *sessionExprCompareOther(
  int nCol,
  const char *zDb1, const char *zDb2, 
  const char *zTab,
  const char **azCol, u8 *abPK
){
  int i;
  const char *zSep = "";
  char *zRet = 0;
  int bHave = 0;

  for(i=0; i<nCol; i++){
    if( abPK[i]==0 ){
      bHave = 1;
      zRet = sqlite3_mprintf(
          "%z%s\"%w\".\"%w\".\"%w\" IS NOT \"%w\".\"%w\".\"%w\"",
          zRet, zSep, zDb1, zTab, azCol[i], zDb2, zTab, azCol[i]
      );
      zSep = " OR ";
      if( zRet==0 ) break;
    }
  }

  if( bHave==0 ){
    assert( zRet==0 );
    zRet = sqlite3_mprintf("0");
  }

  return zRet;
}

static char *sessionSelectFindNew(
  int nCol,
  const char *zDb1,      /* Pick rows in this db only */
  const char *zDb2,      /* But not in this one */
  const char *zTbl,      /* Table name */
  const char *zExpr
){
  char *zRet = sqlite3_mprintf(
      "SELECT * FROM \"%w\".\"%w\" WHERE NOT EXISTS ("
      "  SELECT 1 FROM \"%w\".\"%w\" WHERE %s"
      ")",
      zDb1, zTbl, zDb2, zTbl, zExpr
  );
  return zRet;
}

static int sessionDiffFindNew(
  int op,
  sqlite3_session *pSession,
  SessionTable *pTab,
  const char *zDb1,
  const char *zDb2,
  char *zExpr
){
  int rc = SQLITE_OK;
  char *zStmt = sessionSelectFindNew(pTab->nCol, zDb1, zDb2, pTab->zName,zExpr);

  if( zStmt==0 ){
    rc = SQLITE_NOMEM;
  }else{
    sqlite3_stmt *pStmt;
    rc = sqlite3_prepare(pSession->db, zStmt, -1, &pStmt, 0);
    if( rc==SQLITE_OK ){
      SessionDiffCtx *pDiffCtx = (SessionDiffCtx*)pSession->hook.pCtx;
      pDiffCtx->pStmt = pStmt;
      pDiffCtx->nOldOff = 0;
      while( SQLITE_ROW==sqlite3_step(pStmt) ){
        sessionPreupdateOneChange(op, pSession, pTab);
      }
      rc = sqlite3_finalize(pStmt);
    }
    sqlite3_free(zStmt);
  }

  return rc;
}

static int sessionDiffFindModified(
  sqlite3_session *pSession, 
  SessionTable *pTab, 
  const char *zFrom, 
  const char *zExpr
){
  int rc = SQLITE_OK;

  char *zExpr2 = sessionExprCompareOther(pTab->nCol,
      pSession->zDb, zFrom, pTab->zName, pTab->azCol, pTab->abPK
  );
  if( zExpr2==0 ){
    rc = SQLITE_NOMEM;
  }else{
    char *zStmt = sqlite3_mprintf(
        "SELECT * FROM \"%w\".\"%w\", \"%w\".\"%w\" WHERE %s AND (%z)",
        pSession->zDb, pTab->zName, zFrom, pTab->zName, zExpr, zExpr2
    );
    if( zStmt==0 ){
      rc = SQLITE_NOMEM;
    }else{
      sqlite3_stmt *pStmt;
      rc = sqlite3_prepare(pSession->db, zStmt, -1, &pStmt, 0);

      if( rc==SQLITE_OK ){
        SessionDiffCtx *pDiffCtx = (SessionDiffCtx*)pSession->hook.pCtx;
        pDiffCtx->pStmt = pStmt;
        pDiffCtx->nOldOff = pTab->nCol;
        while( SQLITE_ROW==sqlite3_step(pStmt) ){
          sessionPreupdateOneChange(SQLITE_UPDATE, pSession, pTab);
        }
        rc = sqlite3_finalize(pStmt);
      }
      sqlite3_free(zStmt);
    }
  }

  return rc;
}

SQLITE_API int SQLITE_STDCALL sqlite3session_diff(
  sqlite3_session *pSession,
  const char *zFrom,
  const char *zTbl,
  char **pzErrMsg
){
  const char *zDb = pSession->zDb;
  int rc = pSession->rc;
  SessionDiffCtx d;

  memset(&d, 0, sizeof(d));
  sessionDiffHooks(pSession, &d);

  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( pzErrMsg ) *pzErrMsg = 0;
  if( rc==SQLITE_OK ){
    char *zExpr = 0;
    sqlite3 *db = pSession->db;
    SessionTable *pTo;            /* Table zTbl */

    /* Locate and if necessary initialize the target table object */
    rc = sessionFindTable(pSession, zTbl, &pTo);
    if( pTo==0 ) goto diff_out;
    if( sessionInitTable(pSession, pTo) ){
      rc = pSession->rc;
      goto diff_out;
    }

    /* Check the table schemas match */
    if( rc==SQLITE_OK ){
      int bHasPk = 0;
      int bMismatch = 0;
      int nCol;                   /* Columns in zFrom.zTbl */
      u8 *abPK;
      const char **azCol = 0;
      rc = sessionTableInfo(db, zFrom, zTbl, &nCol, 0, &azCol, &abPK);
      if( rc==SQLITE_OK ){
        if( pTo->nCol!=nCol ){
          bMismatch = 1;
        }else{
          int i;
          for(i=0; i<nCol; i++){
            if( pTo->abPK[i]!=abPK[i] ) bMismatch = 1;
            if( sqlite3_stricmp(azCol[i], pTo->azCol[i]) ) bMismatch = 1;
            if( abPK[i] ) bHasPk = 1;
          }
        }

      }
      sqlite3_free((char*)azCol);
      if( bMismatch ){
        *pzErrMsg = sqlite3_mprintf("table schemas do not match");
        rc = SQLITE_SCHEMA;
      }
      if( bHasPk==0 ){
        /* Ignore tables with no primary keys */
        goto diff_out;
      }
    }

    if( rc==SQLITE_OK ){
      zExpr = sessionExprComparePK(pTo->nCol, 
          zDb, zFrom, pTo->zName, pTo->azCol, pTo->abPK
      );
    }

    /* Find new rows */
    if( rc==SQLITE_OK ){
      rc = sessionDiffFindNew(SQLITE_INSERT, pSession, pTo, zDb, zFrom, zExpr);
    }

    /* Find old rows */
    if( rc==SQLITE_OK ){
      rc = sessionDiffFindNew(SQLITE_DELETE, pSession, pTo, zFrom, zDb, zExpr);
    }

    /* Find modified rows */
    if( rc==SQLITE_OK ){
      rc = sessionDiffFindModified(pSession, pTo, zFrom, zExpr);
    }

    sqlite3_free(zExpr);
  }

 diff_out:
  sessionPreupdateHooks(pSession);
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return rc;
}

/*
** Create a session object. This session object will record changes to
** database zDb attached to connection db.
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_create(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of db (e.g. "main") */
  sqlite3_session **ppSession     /* OUT: New session object */
){
  sqlite3_session *pNew;          /* Newly allocated session object */
  sqlite3_session *pOld;          /* Session object already attached to db */
  int nDb = sqlite3Strlen30(zDb); /* Length of zDb in bytes */

  /* Zero the output value in case an error occurs. */
  *ppSession = 0;

  /* Allocate and populate the new session object. */
  pNew = (sqlite3_session *)sqlite3_malloc(sizeof(sqlite3_session) + nDb + 1);
  if( !pNew ) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(sqlite3_session));
  pNew->db = db;
  pNew->zDb = (char *)&pNew[1];
  pNew->bEnable = 1;
  memcpy(pNew->zDb, zDb, nDb+1);
  sessionPreupdateHooks(pNew);

  /* Add the new session object to the linked list of session objects 
  ** attached to database handle $db. Do this under the cover of the db
  ** handle mutex.  */
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  pOld = (sqlite3_session*)sqlite3_preupdate_hook(db, xPreUpdate, (void*)pNew);
  pNew->pNext = pOld;
  sqlite3_mutex_leave(sqlite3_db_mutex(db));

  *ppSession = pNew;
  return SQLITE_OK;
}

/*
** Free the list of table objects passed as the first argument. The contents
** of the changed-rows hash tables are also deleted.
*/
static void sessionDeleteTable(SessionTable *pList){
  SessionTable *pNext;
  SessionTable *pTab;

  for(pTab=pList; pTab; pTab=pNext){
    int i;
    pNext = pTab->pNext;
    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      SessionChange *pNext;
      for(p=pTab->apChange[i]; p; p=pNext){
        pNext = p->pNext;
        sqlite3_free(p);
      }
    }
    sqlite3_free((char*)pTab->azCol);  /* cast works around VC++ bug */
    sqlite3_free(pTab->apChange);
    sqlite3_free(pTab);
  }
}

/*
** Delete a session object previously allocated using sqlite3session_create().
*/
SQLITE_API void SQLITE_STDCALL sqlite3session_delete(sqlite3_session *pSession){
  sqlite3 *db = pSession->db;
  sqlite3_session *pHead;
  sqlite3_session **pp;

  /* Unlink the session from the linked list of sessions attached to the
  ** database handle. Hold the db mutex while doing so.  */
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  pHead = (sqlite3_session*)sqlite3_preupdate_hook(db, 0, 0);
  for(pp=&pHead; (*pp)!=pSession; pp=&((*pp)->pNext));
  *pp = (*pp)->pNext;
  if( pHead ) sqlite3_preupdate_hook(db, xPreUpdate, (void *)pHead);
  sqlite3_mutex_leave(sqlite3_db_mutex(db));

  /* Delete all attached table objects. And the contents of their 
  ** associated hash-tables. */
  sessionDeleteTable(pSession->pTable);

  /* Free the session object itself. */
  sqlite3_free(pSession);
}

/*
** Set a table filter on a Session Object.
*/
SQLITE_API void SQLITE_STDCALL sqlite3session_table_filter(
  sqlite3_session *pSession, 
  int(*xFilter)(void*, const char*),
  void *pCtx                      /* First argument passed to xFilter */
){
  pSession->bAutoAttach = 1;
  pSession->pFilterCtx = pCtx;
  pSession->xTableFilter = xFilter;
}

/*
** Attach a table to a session. All subsequent changes made to the table
** while the session object is enabled will be recorded.
**
** Only tables that have a PRIMARY KEY defined may be attached. It does
** not matter if the PRIMARY KEY is an "INTEGER PRIMARY KEY" (rowid alias)
** or not.
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_attach(
  sqlite3_session *pSession,      /* Session object */
  const char *zName               /* Table name */
){
  int rc = SQLITE_OK;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));

  if( !zName ){
    pSession->bAutoAttach = 1;
  }else{
    SessionTable *pTab;           /* New table object (if required) */
    int nName;                    /* Number of bytes in string zName */

    /* First search for an existing entry. If one is found, this call is
    ** a no-op. Return early. */
    nName = sqlite3Strlen30(zName);
    for(pTab=pSession->pTable; pTab; pTab=pTab->pNext){
      if( 0==sqlite3_strnicmp(pTab->zName, zName, nName+1) ) break;
    }

    if( !pTab ){
      /* Allocate new SessionTable object. */
      pTab = (SessionTable *)sqlite3_malloc(sizeof(SessionTable) + nName + 1);
      if( !pTab ){
        rc = SQLITE_NOMEM;
      }else{
        /* Populate the new SessionTable object and link it into the list. */
        memset(pTab, 0, sizeof(SessionTable));
        pTab->zName = (char *)&pTab[1];
        memcpy(pTab->zName, zName, nName+1);
        pTab->pNext = pSession->pTable;
        pSession->pTable = pTab;
      }
    }
  }

  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return rc;
}

/*
** Ensure that there is room in the buffer to append nByte bytes of data.
** If not, use sqlite3_realloc() to grow the buffer so that there is.
**
** If successful, return zero. Otherwise, if an OOM condition is encountered,
** set *pRc to SQLITE_NOMEM and return non-zero.
*/
static int sessionBufferGrow(SessionBuffer *p, int nByte, int *pRc){
  if( *pRc==SQLITE_OK && p->nAlloc-p->nBuf<nByte ){
    u8 *aNew;
    int nNew = p->nAlloc ? p->nAlloc : 128;
    do {
      nNew = nNew*2;
    }while( nNew<(p->nBuf+nByte) );

    aNew = (u8 *)sqlite3_realloc(p->aBuf, nNew);
    if( 0==aNew ){
      *pRc = SQLITE_NOMEM;
    }else{
      p->aBuf = aNew;
      p->nAlloc = nNew;
    }
  }
  return (*pRc!=SQLITE_OK);
}

/*
** Append the value passed as the second argument to the buffer passed
** as the first.
**
** This function is a no-op if *pRc is non-zero when it is called.
** Otherwise, if an error occurs, *pRc is set to an SQLite error code
** before returning.
*/
static void sessionAppendValue(SessionBuffer *p, sqlite3_value *pVal, int *pRc){
  int rc = *pRc;
  if( rc==SQLITE_OK ){
    int nByte = 0;
    rc = sessionSerializeValue(0, pVal, &nByte);
    sessionBufferGrow(p, nByte, &rc);
    if( rc==SQLITE_OK ){
      rc = sessionSerializeValue(&p->aBuf[p->nBuf], pVal, 0);
      p->nBuf += nByte;
    }else{
      *pRc = rc;
    }
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a single byte to the buffer. 
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendByte(SessionBuffer *p, u8 v, int *pRc){
  if( 0==sessionBufferGrow(p, 1, pRc) ){
    p->aBuf[p->nBuf++] = v;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a single varint to the buffer. 
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendVarint(SessionBuffer *p, int v, int *pRc){
  if( 0==sessionBufferGrow(p, 9, pRc) ){
    p->nBuf += sessionVarintPut(&p->aBuf[p->nBuf], v);
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a blob of data to the buffer. 
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendBlob(
  SessionBuffer *p, 
  const u8 *aBlob, 
  int nBlob, 
  int *pRc
){
  if( 0==sessionBufferGrow(p, nBlob, pRc) ){
    memcpy(&p->aBuf[p->nBuf], aBlob, nBlob);
    p->nBuf += nBlob;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a string to the buffer. All bytes in the string
** up to (but not including) the nul-terminator are written to the buffer.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendStr(
  SessionBuffer *p, 
  const char *zStr, 
  int *pRc
){
  int nStr = sqlite3Strlen30(zStr);
  if( 0==sessionBufferGrow(p, nStr, pRc) ){
    memcpy(&p->aBuf[p->nBuf], zStr, nStr);
    p->nBuf += nStr;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append the string representation of integer iVal
** to the buffer. No nul-terminator is written.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendInteger(
  SessionBuffer *p,               /* Buffer to append to */
  int iVal,                       /* Value to write the string rep. of */
  int *pRc                        /* IN/OUT: Error code */
){
  char aBuf[24];
  sqlite3_snprintf(sizeof(aBuf)-1, aBuf, "%d", iVal);
  sessionAppendStr(p, aBuf, pRc);
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append the string zStr enclosed in quotes (") and
** with any embedded quote characters escaped to the buffer. No 
** nul-terminator byte is written.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendIdent(
  SessionBuffer *p,               /* Buffer to a append to */
  const char *zStr,               /* String to quote, escape and append */
  int *pRc                        /* IN/OUT: Error code */
){
  int nStr = sqlite3Strlen30(zStr)*2 + 2 + 1;
  if( 0==sessionBufferGrow(p, nStr, pRc) ){
    char *zOut = (char *)&p->aBuf[p->nBuf];
    const char *zIn = zStr;
    *zOut++ = '"';
    while( *zIn ){
      if( *zIn=='"' ) *zOut++ = '"';
      *zOut++ = *(zIn++);
    }
    *zOut++ = '"';
    p->nBuf = (int)((u8 *)zOut - p->aBuf);
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwse, it appends the serialized version of the value stored
** in column iCol of the row that SQL statement pStmt currently points
** to to the buffer.
*/
static void sessionAppendCol(
  SessionBuffer *p,               /* Buffer to append to */
  sqlite3_stmt *pStmt,            /* Handle pointing to row containing value */
  int iCol,                       /* Column to read value from */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int eType = sqlite3_column_type(pStmt, iCol);
    sessionAppendByte(p, (u8)eType, pRc);
    if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
      sqlite3_int64 i;
      u8 aBuf[8];
      if( eType==SQLITE_INTEGER ){
        i = sqlite3_column_int64(pStmt, iCol);
      }else{
        double r = sqlite3_column_double(pStmt, iCol);
        memcpy(&i, &r, 8);
      }
      sessionPutI64(aBuf, i);
      sessionAppendBlob(p, aBuf, 8, pRc);
    }
    if( eType==SQLITE_BLOB || eType==SQLITE_TEXT ){
      u8 *z;
      int nByte;
      if( eType==SQLITE_BLOB ){
        z = (u8 *)sqlite3_column_blob(pStmt, iCol);
      }else{
        z = (u8 *)sqlite3_column_text(pStmt, iCol);
      }
      nByte = sqlite3_column_bytes(pStmt, iCol);
      if( z || (eType==SQLITE_BLOB && nByte==0) ){
        sessionAppendVarint(p, nByte, pRc);
        sessionAppendBlob(p, z, nByte, pRc);
      }else{
        *pRc = SQLITE_NOMEM;
      }
    }
  }
}

/*
**
** This function appends an update change to the buffer (see the comments 
** under "CHANGESET FORMAT" at the top of the file). An update change 
** consists of:
**
**   1 byte:  SQLITE_UPDATE (0x17)
**   n bytes: old.* record (see RECORD FORMAT)
**   m bytes: new.* record (see RECORD FORMAT)
**
** The SessionChange object passed as the third argument contains the
** values that were stored in the row when the session began (the old.*
** values). The statement handle passed as the second argument points
** at the current version of the row (the new.* values).
**
** If all of the old.* values are equal to their corresponding new.* value
** (i.e. nothing has changed), then no data at all is appended to the buffer.
**
** Otherwise, the old.* record contains all primary key values and the 
** original values of any fields that have been modified. The new.* record 
** contains the new values of only those fields that have been modified.
*/ 
static int sessionAppendUpdate(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int bPatchset,                  /* True for "patchset", 0 for "changeset" */
  sqlite3_stmt *pStmt,            /* Statement handle pointing at new row */
  SessionChange *p,               /* Object containing old values */
  u8 *abPK                        /* Boolean array - true for PK columns */
){
  int rc = SQLITE_OK;
  SessionBuffer buf2 = {0,0,0}; /* Buffer to accumulate new.* record in */
  int bNoop = 1;                /* Set to zero if any values are modified */
  int nRewind = pBuf->nBuf;     /* Set to zero if any values are modified */
  int i;                        /* Used to iterate through columns */
  u8 *pCsr = p->aRecord;        /* Used to iterate through old.* values */

  sessionAppendByte(pBuf, SQLITE_UPDATE, &rc);
  sessionAppendByte(pBuf, p->bIndirect, &rc);
  for(i=0; i<sqlite3_column_count(pStmt); i++){
    int bChanged = 0;
    int nAdvance;
    int eType = *pCsr;
    switch( eType ){
      case SQLITE_NULL:
        nAdvance = 1;
        if( sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
          bChanged = 1;
        }
        break;

      case SQLITE_FLOAT:
      case SQLITE_INTEGER: {
        nAdvance = 9;
        if( eType==sqlite3_column_type(pStmt, i) ){
          sqlite3_int64 iVal = sessionGetI64(&pCsr[1]);
          if( eType==SQLITE_INTEGER ){
            if( iVal==sqlite3_column_int64(pStmt, i) ) break;
          }else{
            double dVal;
            memcpy(&dVal, &iVal, 8);
            if( dVal==sqlite3_column_double(pStmt, i) ) break;
          }
        }
        bChanged = 1;
        break;
      }

      default: {
        int nByte;
        int nHdr = 1 + sessionVarintGet(&pCsr[1], &nByte);
        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        nAdvance = nHdr + nByte;
        if( eType==sqlite3_column_type(pStmt, i) 
         && nByte==sqlite3_column_bytes(pStmt, i) 
         && 0==memcmp(&pCsr[nHdr], sqlite3_column_blob(pStmt, i), nByte)
        ){
          break;
        }
        bChanged = 1;
      }
    }

    /* If at least one field has been modified, this is not a no-op. */
    if( bChanged ) bNoop = 0;

    /* Add a field to the old.* record. This is omitted if this modules is
    ** currently generating a patchset. */
    if( bPatchset==0 ){
      if( bChanged || abPK[i] ){
        sessionAppendBlob(pBuf, pCsr, nAdvance, &rc);
      }else{
        sessionAppendByte(pBuf, 0, &rc);
      }
    }

    /* Add a field to the new.* record. Or the only record if currently
    ** generating a patchset.  */
    if( bChanged || (bPatchset && abPK[i]) ){
      sessionAppendCol(&buf2, pStmt, i, &rc);
    }else{
      sessionAppendByte(&buf2, 0, &rc);
    }

    pCsr += nAdvance;
  }

  if( bNoop ){
    pBuf->nBuf = nRewind;
  }else{
    sessionAppendBlob(pBuf, buf2.aBuf, buf2.nBuf, &rc);
  }
  sqlite3_free(buf2.aBuf);

  return rc;
}

/*
** Append a DELETE change to the buffer passed as the first argument. Use
** the changeset format if argument bPatchset is zero, or the patchset
** format otherwise.
*/
static int sessionAppendDelete(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int bPatchset,                  /* True for "patchset", 0 for "changeset" */
  SessionChange *p,               /* Object containing old values */
  int nCol,                       /* Number of columns in table */
  u8 *abPK                        /* Boolean array - true for PK columns */
){
  int rc = SQLITE_OK;

  sessionAppendByte(pBuf, SQLITE_DELETE, &rc);
  sessionAppendByte(pBuf, p->bIndirect, &rc);

  if( bPatchset==0 ){
    sessionAppendBlob(pBuf, p->aRecord, p->nRecord, &rc);
  }else{
    int i;
    u8 *a = p->aRecord;
    for(i=0; i<nCol; i++){
      u8 *pStart = a;
      int eType = *a++;

      switch( eType ){
        case 0:
        case SQLITE_NULL:
          assert( abPK[i]==0 );
          break;

        case SQLITE_FLOAT:
        case SQLITE_INTEGER:
          a += 8;
          break;

        default: {
          int n;
          a += sessionVarintGet(a, &n);
          a += n;
          break;
        }
      }
      if( abPK[i] ){
        sessionAppendBlob(pBuf, pStart, (int)(a-pStart), &rc);
      }
    }
    assert( (a - p->aRecord)==p->nRecord );
  }

  return rc;
}

/*
** Formulate and prepare a SELECT statement to retrieve a row from table
** zTab in database zDb based on its primary key. i.e.
**
**   SELECT * FROM zDb.zTab WHERE pk1 = ? AND pk2 = ? AND ...
*/
static int sessionSelectStmt(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Database name */
  const char *zTab,               /* Table name */
  int nCol,                       /* Number of columns in table */
  const char **azCol,             /* Names of table columns */
  u8 *abPK,                       /* PRIMARY KEY  array */
  sqlite3_stmt **ppStmt           /* OUT: Prepared SELECT statement */
){
  int rc = SQLITE_OK;
  int i;
  const char *zSep = "";
  SessionBuffer buf = {0, 0, 0};

  sessionAppendStr(&buf, "SELECT * FROM ", &rc);
  sessionAppendIdent(&buf, zDb, &rc);
  sessionAppendStr(&buf, ".", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " WHERE ", &rc);
  for(i=0; i<nCol; i++){
    if( abPK[i] ){
      sessionAppendStr(&buf, zSep, &rc);
      sessionAppendIdent(&buf, azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i+1, &rc);
      zSep = " AND ";
    }
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, ppStmt, 0);
  }
  sqlite3_free(buf.aBuf);
  return rc;
}

/*
** Bind the PRIMARY KEY values from the change passed in argument pChange
** to the SELECT statement passed as the first argument. The SELECT statement
** is as prepared by function sessionSelectStmt().
**
** Return SQLITE_OK if all PK values are successfully bound, or an SQLite
** error code (e.g. SQLITE_NOMEM) otherwise.
*/
static int sessionSelectBind(
  sqlite3_stmt *pSelect,          /* SELECT from sessionSelectStmt() */
  int nCol,                       /* Number of columns in table */
  u8 *abPK,                       /* PRIMARY KEY array */
  SessionChange *pChange          /* Change structure */
){
  int i;
  int rc = SQLITE_OK;
  u8 *a = pChange->aRecord;

  for(i=0; i<nCol && rc==SQLITE_OK; i++){
    int eType = *a++;

    switch( eType ){
      case 0:
      case SQLITE_NULL:
        assert( abPK[i]==0 );
        break;

      case SQLITE_INTEGER: {
        if( abPK[i] ){
          i64 iVal = sessionGetI64(a);
          rc = sqlite3_bind_int64(pSelect, i+1, iVal);
        }
        a += 8;
        break;
      }

      case SQLITE_FLOAT: {
        if( abPK[i] ){
          double rVal;
          i64 iVal = sessionGetI64(a);
          memcpy(&rVal, &iVal, 8);
          rc = sqlite3_bind_double(pSelect, i+1, rVal);
        }
        a += 8;
        break;
      }

      case SQLITE_TEXT: {
        int n;
        a += sessionVarintGet(a, &n);
        if( abPK[i] ){
          rc = sqlite3_bind_text(pSelect, i+1, (char *)a, n, SQLITE_TRANSIENT);
        }
        a += n;
        break;
      }

      default: {
        int n;
        assert( eType==SQLITE_BLOB );
        a += sessionVarintGet(a, &n);
        if( abPK[i] ){
          rc = sqlite3_bind_blob(pSelect, i+1, a, n, SQLITE_TRANSIENT);
        }
        a += n;
        break;
      }
    }
  }

  return rc;
}

/*
** This function is a no-op if *pRc is set to other than SQLITE_OK when it
** is called. Otherwise, append a serialized table header (part of the binary 
** changeset format) to buffer *pBuf. If an error occurs, set *pRc to an
** SQLite error code before returning.
*/
static void sessionAppendTableHdr(
  SessionBuffer *pBuf,            /* Append header to this buffer */
  int bPatchset,                  /* Use the patchset format if true */
  SessionTable *pTab,             /* Table object to append header for */
  int *pRc                        /* IN/OUT: Error code */
){
  /* Write a table header */
  sessionAppendByte(pBuf, (bPatchset ? 'P' : 'T'), pRc);
  sessionAppendVarint(pBuf, pTab->nCol, pRc);
  sessionAppendBlob(pBuf, pTab->abPK, pTab->nCol, pRc);
  sessionAppendBlob(pBuf, (u8 *)pTab->zName, (int)strlen(pTab->zName)+1, pRc);
}

/*
** Generate either a changeset (if argument bPatchset is zero) or a patchset
** (if it is non-zero) based on the current contents of the session object
** passed as the first argument.
**
** If no error occurs, SQLITE_OK is returned and the new changeset/patchset
** stored in output variables *pnChangeset and *ppChangeset. Or, if an error
** occurs, an SQLite error code is returned and both output variables set 
** to 0.
*/
static int sessionGenerateChangeset(
  sqlite3_session *pSession,      /* Session object */
  int bPatchset,                  /* True for patchset, false for changeset */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,                     /* First argument for xOutput */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
){
  sqlite3 *db = pSession->db;     /* Source database handle */
  SessionTable *pTab;             /* Used to iterate through attached tables */
  SessionBuffer buf = {0,0,0};    /* Buffer in which to accumlate changeset */
  int rc;                         /* Return code */

  assert( xOutput==0 || (pnChangeset==0 && ppChangeset==0 ) );

  /* Zero the output variables in case an error occurs. If this session
  ** object is already in the error state (sqlite3_session.rc != SQLITE_OK),
  ** this call will be a no-op.  */
  if( xOutput==0 ){
    *pnChangeset = 0;
    *ppChangeset = 0;
  }

  if( pSession->rc ) return pSession->rc;
  rc = sqlite3_exec(pSession->db, "SAVEPOINT changeset", 0, 0, 0);
  if( rc!=SQLITE_OK ) return rc;

  sqlite3_mutex_enter(sqlite3_db_mutex(db));

  for(pTab=pSession->pTable; rc==SQLITE_OK && pTab; pTab=pTab->pNext){
    if( pTab->nEntry ){
      const char *zName = pTab->zName;
      int nCol;                   /* Number of columns in table */
      u8 *abPK;                   /* Primary key array */
      const char **azCol = 0;     /* Table columns */
      int i;                      /* Used to iterate through hash buckets */
      sqlite3_stmt *pSel = 0;     /* SELECT statement to query table pTab */
      int nRewind = buf.nBuf;     /* Initial size of write buffer */
      int nNoop;                  /* Size of buffer after writing tbl header */

      /* Check the table schema is still Ok. */
      rc = sessionTableInfo(db, pSession->zDb, zName, &nCol, 0, &azCol, &abPK);
      if( !rc && (pTab->nCol!=nCol || memcmp(abPK, pTab->abPK, nCol)) ){
        rc = SQLITE_SCHEMA;
      }

      /* Write a table header */
      sessionAppendTableHdr(&buf, bPatchset, pTab, &rc);

      /* Build and compile a statement to execute: */
      if( rc==SQLITE_OK ){
        rc = sessionSelectStmt(
            db, pSession->zDb, zName, nCol, azCol, abPK, &pSel);
      }

      nNoop = buf.nBuf;
      for(i=0; i<pTab->nChange && rc==SQLITE_OK; i++){
        SessionChange *p;         /* Used to iterate through changes */

        for(p=pTab->apChange[i]; rc==SQLITE_OK && p; p=p->pNext){
          rc = sessionSelectBind(pSel, nCol, abPK, p);
          if( rc!=SQLITE_OK ) continue;
          if( sqlite3_step(pSel)==SQLITE_ROW ){
            if( p->op==SQLITE_INSERT ){
              int iCol;
              sessionAppendByte(&buf, SQLITE_INSERT, &rc);
              sessionAppendByte(&buf, p->bIndirect, &rc);
              for(iCol=0; iCol<nCol; iCol++){
                sessionAppendCol(&buf, pSel, iCol, &rc);
              }
            }else{
              rc = sessionAppendUpdate(&buf, bPatchset, pSel, p, abPK);
            }
          }else if( p->op!=SQLITE_INSERT ){
            rc = sessionAppendDelete(&buf, bPatchset, p, nCol, abPK);
          }
          if( rc==SQLITE_OK ){
            rc = sqlite3_reset(pSel);
          }

          /* If the buffer is now larger than SESSIONS_STRM_CHUNK_SIZE, pass
          ** its contents to the xOutput() callback. */
          if( xOutput 
           && rc==SQLITE_OK 
           && buf.nBuf>nNoop 
           && buf.nBuf>SESSIONS_STRM_CHUNK_SIZE 
          ){
            rc = xOutput(pOut, (void*)buf.aBuf, buf.nBuf);
            nNoop = -1;
            buf.nBuf = 0;
          }

        }
      }

      sqlite3_finalize(pSel);
      if( buf.nBuf==nNoop ){
        buf.nBuf = nRewind;
      }
      sqlite3_free((char*)azCol);  /* cast works around VC++ bug */
    }
  }

  if( rc==SQLITE_OK ){
    if( xOutput==0 ){
      *pnChangeset = buf.nBuf;
      *ppChangeset = buf.aBuf;
      buf.aBuf = 0;
    }else if( buf.nBuf>0 ){
      rc = xOutput(pOut, (void*)buf.aBuf, buf.nBuf);
    }
  }

  sqlite3_free(buf.aBuf);
  sqlite3_exec(db, "RELEASE changeset", 0, 0, 0);
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  return rc;
}

/*
** Obtain a changeset object containing all changes recorded by the 
** session object passed as the first argument.
**
** It is the responsibility of the caller to eventually free the buffer 
** using sqlite3_free().
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_changeset(
  sqlite3_session *pSession,      /* Session object */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
){
  return sessionGenerateChangeset(pSession, 0, 0, 0, pnChangeset, ppChangeset);
}

/*
** Streaming version of sqlite3session_changeset().
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_changeset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  return sessionGenerateChangeset(pSession, 0, xOutput, pOut, 0, 0);
}

/*
** Streaming version of sqlite3session_patchset().
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_patchset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  return sessionGenerateChangeset(pSession, 1, xOutput, pOut, 0, 0);
}

/*
** Obtain a patchset object containing all changes recorded by the 
** session object passed as the first argument.
**
** It is the responsibility of the caller to eventually free the buffer 
** using sqlite3_free().
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_patchset(
  sqlite3_session *pSession,      /* Session object */
  int *pnPatchset,                /* OUT: Size of buffer at *ppChangeset */
  void **ppPatchset               /* OUT: Buffer containing changeset */
){
  return sessionGenerateChangeset(pSession, 1, 0, 0, pnPatchset, ppPatchset);
}

/*
** Enable or disable the session object passed as the first argument.
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_enable(sqlite3_session *pSession, int bEnable){
  int ret;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( bEnable>=0 ){
    pSession->bEnable = bEnable;
  }
  ret = pSession->bEnable;
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return ret;
}

/*
** Enable or disable the session object passed as the first argument.
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_indirect(sqlite3_session *pSession, int bIndirect){
  int ret;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( bIndirect>=0 ){
    pSession->bIndirect = bIndirect;
  }
  ret = pSession->bIndirect;
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return ret;
}

/*
** Return true if there have been no changes to monitored tables recorded
** by the session object passed as the only argument.
*/
SQLITE_API int SQLITE_STDCALL sqlite3session_isempty(sqlite3_session *pSession){
  int ret = 0;
  SessionTable *pTab;

  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  for(pTab=pSession->pTable; pTab && ret==0; pTab=pTab->pNext){
    ret = (pTab->nEntry>0);
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));

  return (ret==0);
}

/*
** Do the work for either sqlite3changeset_start() or start_strm().
*/
static int sessionChangesetStart(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset                /* Pointer to buffer containing changeset */
){
  sqlite3_changeset_iter *pRet;   /* Iterator to return */
  int nByte;                      /* Number of bytes to allocate for iterator */

  assert( xInput==0 || (pChangeset==0 && nChangeset==0) );

  /* Zero the output variable in case an error occurs. */
  *pp = 0;

  /* Allocate and initialize the iterator structure. */
  nByte = sizeof(sqlite3_changeset_iter);
  pRet = (sqlite3_changeset_iter *)sqlite3_malloc(nByte);
  if( !pRet ) return SQLITE_NOMEM;
  memset(pRet, 0, sizeof(sqlite3_changeset_iter));
  pRet->in.aData = (u8 *)pChangeset;
  pRet->in.nData = nChangeset;
  pRet->in.xInput = xInput;
  pRet->in.pIn = pIn;
  pRet->in.iNext = 0;
  pRet->in.bEof = (xInput ? 0 : 1);

  /* Populate the output variable and return success. */
  *pp = pRet;
  return SQLITE_OK;
}

/*
** Create an iterator used to iterate through the contents of a changeset.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_start(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset                /* Pointer to buffer containing changeset */
){
  return sessionChangesetStart(pp, 0, 0, nChangeset, pChangeset);
}

/*
** Streaming version of sqlite3changeset_start().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_start_strm(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
){
  return sessionChangesetStart(pp, xInput, pIn, 0, 0);
}

/*
** Ensure that there are at least nByte bytes available in the buffer. Or,
** if there are not nByte bytes remaining in the input, that all available
** data is in the buffer.
**
** Return an SQLite error code if an error occurs, or SQLITE_OK otherwise.
*/
static int sessionInputBuffer(SessionInput *pIn, int nByte){
  int rc = SQLITE_OK;
  if( pIn->xInput ){
    while( !pIn->bEof && (pIn->iNext+nByte)>=pIn->nData && rc==SQLITE_OK ){
      int nNew = SESSIONS_STRM_CHUNK_SIZE;

      if( pIn->iNext>=SESSIONS_STRM_CHUNK_SIZE ){
        int nMove = pIn->buf.nBuf - pIn->iNext;
        memmove(pIn->buf.aBuf, &pIn->buf.aBuf[pIn->iNext], nMove);
        pIn->buf.nBuf -= pIn->iNext;
        pIn->iNext = 0;
      }

      if( SQLITE_OK==sessionBufferGrow(&pIn->buf, nNew, &rc) ){
        rc = pIn->xInput(pIn->pIn, &pIn->buf.aBuf[pIn->buf.nBuf], &nNew);
        if( nNew==0 ){
          pIn->bEof = 1;
        }else{
          pIn->buf.nBuf += nNew;
        }
      }

      pIn->aData = pIn->buf.aBuf;
      pIn->nData = pIn->buf.nBuf;
    }
  }
  return rc;
}

/*
** When this function is called, *ppRec points to the start of a record
** that contains nCol values. This function advances the pointer *ppRec
** until it points to the byte immediately following that record.
*/
static void sessionSkipRecord(
  u8 **ppRec,                     /* IN/OUT: Record pointer */
  int nCol                        /* Number of values in record */
){
  u8 *aRec = *ppRec;
  int i;
  for(i=0; i<nCol; i++){
    int eType = *aRec++;
    if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
      int nByte;
      aRec += sessionVarintGet((u8*)aRec, &nByte);
      aRec += nByte;
    }else if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
      aRec += 8;
    }
  }

  *ppRec = aRec;
}

/*
** This function sets the value of the sqlite3_value object passed as the
** first argument to a copy of the string or blob held in the aData[] 
** buffer. SQLITE_OK is returned if successful, or SQLITE_NOMEM if an OOM
** error occurs.
*/
static int sessionValueSetStr(
  sqlite3_value *pVal,            /* Set the value of this object */
  u8 *aData,                      /* Buffer containing string or blob data */
  int nData,                      /* Size of buffer aData[] in bytes */
  u8 enc                          /* String encoding (0 for blobs) */
){
  /* In theory this code could just pass SQLITE_TRANSIENT as the final
  ** argument to sqlite3ValueSetStr() and have the copy created 
  ** automatically. But doing so makes it difficult to detect any OOM
  ** error. Hence the code to create the copy externally. */
  u8 *aCopy = sqlite3_malloc(nData+1);
  if( aCopy==0 ) return SQLITE_NOMEM;
  memcpy(aCopy, aData, nData);
  sqlite3ValueSetStr(pVal, nData, (char*)aCopy, enc, sqlite3_free);
  return SQLITE_OK;
}

/*
** Deserialize a single record from a buffer in memory. See "RECORD FORMAT"
** for details.
**
** When this function is called, *paChange points to the start of the record
** to deserialize. Assuming no error occurs, *paChange is set to point to
** one byte after the end of the same record before this function returns.
** If the argument abPK is NULL, then the record contains nCol values. Or,
** if abPK is other than NULL, then the record contains only the PK fields
** (in other words, it is a patchset DELETE record).
**
** If successful, each element of the apOut[] array (allocated by the caller)
** is set to point to an sqlite3_value object containing the value read
** from the corresponding position in the record. If that value is not
** included in the record (i.e. because the record is part of an UPDATE change
** and the field was not modified), the corresponding element of apOut[] is
** set to NULL.
**
** It is the responsibility of the caller to free all sqlite_value structures
** using sqlite3_free().
**
** If an error occurs, an SQLite error code (e.g. SQLITE_NOMEM) is returned.
** The apOut[] array may have been partially populated in this case.
*/
static int sessionReadRecord(
  SessionInput *pIn,              /* Input data */
  int nCol,                       /* Number of values in record */
  u8 *abPK,                       /* Array of primary key flags, or NULL */
  sqlite3_value **apOut           /* Write values to this array */
){
  int i;                          /* Used to iterate through columns */
  int rc = SQLITE_OK;

  for(i=0; i<nCol && rc==SQLITE_OK; i++){
    int eType = 0;                /* Type of value (SQLITE_NULL, TEXT etc.) */
    if( abPK && abPK[i]==0 ) continue;
    rc = sessionInputBuffer(pIn, 9);
    if( rc==SQLITE_OK ){
      eType = pIn->aData[pIn->iNext++];
    }

    assert( apOut[i]==0 );
    if( eType ){
      apOut[i] = sqlite3ValueNew(0);
      if( !apOut[i] ) rc = SQLITE_NOMEM;
    }

    if( rc==SQLITE_OK ){
      u8 *aVal = &pIn->aData[pIn->iNext];
      if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int nByte;
        pIn->iNext += sessionVarintGet(aVal, &nByte);
        rc = sessionInputBuffer(pIn, nByte);
        if( rc==SQLITE_OK ){
          u8 enc = (eType==SQLITE_TEXT ? SQLITE_UTF8 : 0);
          rc = sessionValueSetStr(apOut[i],&pIn->aData[pIn->iNext],nByte,enc);
        }
        pIn->iNext += nByte;
      }
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        sqlite3_int64 v = sessionGetI64(aVal);
        if( eType==SQLITE_INTEGER ){
          sqlite3VdbeMemSetInt64(apOut[i], v);
        }else{
          double d;
          memcpy(&d, &v, 8);
          sqlite3VdbeMemSetDouble(apOut[i], d);
        }
        pIn->iNext += 8;
      }
    }
  }

  return rc;
}

/*
** The input pointer currently points to the second byte of a table-header.
** Specifically, to the following:
**
**   + number of columns in table (varint)
**   + array of PK flags (1 byte per column),
**   + table name (nul terminated).
**
** This function ensures that all of the above is present in the input 
** buffer (i.e. that it can be accessed without any calls to xInput()).
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code.
** The input pointer is not moved.
*/
static int sessionChangesetBufferTblhdr(SessionInput *pIn, int *pnByte){
  int rc = SQLITE_OK;
  int nCol = 0;
  int nRead = 0;

  rc = sessionInputBuffer(pIn, 9);
  if( rc==SQLITE_OK ){
    nRead += sessionVarintGet(&pIn->aData[pIn->iNext + nRead], &nCol);
    rc = sessionInputBuffer(pIn, nRead+nCol+100);
    nRead += nCol;
  }

  while( rc==SQLITE_OK ){
    while( (pIn->iNext + nRead)<pIn->nData && pIn->aData[pIn->iNext + nRead] ){
      nRead++;
    }
    if( (pIn->iNext + nRead)<pIn->nData ) break;
    rc = sessionInputBuffer(pIn, nRead + 100);
  }
  *pnByte = nRead+1;
  return rc;
}

/*
** The input pointer currently points to the first byte of the first field
** of a record consisting of nCol columns. This function ensures the entire
** record is buffered. It does not move the input pointer.
**
** If successful, SQLITE_OK is returned and *pnByte is set to the size of
** the record in bytes. Otherwise, an SQLite error code is returned. The
** final value of *pnByte is undefined in this case.
*/
static int sessionChangesetBufferRecord(
  SessionInput *pIn,              /* Input data */
  int nCol,                       /* Number of columns in record */
  int *pnByte                     /* OUT: Size of record in bytes */
){
  int rc = SQLITE_OK;
  int nByte = 0;
  int i;
  for(i=0; rc==SQLITE_OK && i<nCol; i++){
    int eType;
    rc = sessionInputBuffer(pIn, nByte + 10);
    if( rc==SQLITE_OK ){
      eType = pIn->aData[pIn->iNext + nByte++];
      if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int n;
        nByte += sessionVarintGet(&pIn->aData[pIn->iNext+nByte], &n);
        nByte += n;
        rc = sessionInputBuffer(pIn, nByte);
      }else if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        nByte += 8;
      }
    }
  }
  *pnByte = nByte;
  return rc;
}

/*
** The input pointer currently points to the second byte of a table-header.
** Specifically, to the following:
**
**   + number of columns in table (varint)
**   + array of PK flags (1 byte per column),
**   + table name (nul terminated).
**
** This function decodes the table-header and populates the p->nCol, 
** p->zTab and p->abPK[] variables accordingly. The p->apValue[] array is 
** also allocated or resized according to the new value of p->nCol. The
** input pointer is left pointing to the byte following the table header.
**
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code
** is returned and the final values of the various fields enumerated above
** are undefined.
*/
static int sessionChangesetReadTblhdr(sqlite3_changeset_iter *p){
  int rc;
  int nCopy;
  assert( p->rc==SQLITE_OK );

  rc = sessionChangesetBufferTblhdr(&p->in, &nCopy);
  if( rc==SQLITE_OK ){
    int nByte;
    int nVarint;
    nVarint = sessionVarintGet(&p->in.aData[p->in.iNext], &p->nCol);
    nCopy -= nVarint;
    p->in.iNext += nVarint;
    nByte = p->nCol * sizeof(sqlite3_value*) * 2 + nCopy;
    p->tblhdr.nBuf = 0;
    sessionBufferGrow(&p->tblhdr, nByte, &rc);
  }

  if( rc==SQLITE_OK ){
    int iPK = sizeof(sqlite3_value*)*p->nCol*2;
    memset(p->tblhdr.aBuf, 0, iPK);
    memcpy(&p->tblhdr.aBuf[iPK], &p->in.aData[p->in.iNext], nCopy);
    p->in.iNext += nCopy;
  }

  p->apValue = (sqlite3_value**)p->tblhdr.aBuf;
  p->abPK = (u8*)&p->apValue[p->nCol*2];
  p->zTab = (char*)&p->abPK[p->nCol];
  return (p->rc = rc);
}

/*
** Advance the changeset iterator to the next change.
**
** If both paRec and pnRec are NULL, then this function works like the public
** API sqlite3changeset_next(). If SQLITE_ROW is returned, then the
** sqlite3changeset_new() and old() APIs may be used to query for values.
**
** Otherwise, if paRec and pnRec are not NULL, then a pointer to the change
** record is written to *paRec before returning and the number of bytes in
** the record to *pnRec.
**
** Either way, this function returns SQLITE_ROW if the iterator is 
** successfully advanced to the next change in the changeset, an SQLite 
** error code if an error occurs, or SQLITE_DONE if there are no further 
** changes in the changeset.
*/
static int sessionChangesetNext(
  sqlite3_changeset_iter *p,      /* Changeset iterator */
  u8 **paRec,                     /* If non-NULL, store record pointer here */
  int *pnRec                      /* If non-NULL, store size of record here */
){
  int i;
  u8 op;

  assert( (paRec==0 && pnRec==0) || (paRec && pnRec) );

  /* If the iterator is in the error-state, return immediately. */
  if( p->rc!=SQLITE_OK ) return p->rc;

  /* Free the current contents of p->apValue[], if any. */
  if( p->apValue ){
    for(i=0; i<p->nCol*2; i++){
      sqlite3ValueFree(p->apValue[i]);
    }
    memset(p->apValue, 0, sizeof(sqlite3_value*)*p->nCol*2);
  }

  /* Make sure the buffer contains at least 10 bytes of input data, or all
  ** remaining data if there are less than 10 bytes available. This is
  ** sufficient either for the 'T' or 'P' byte and the varint that follows
  ** it, or for the two single byte values otherwise. */
  p->rc = sessionInputBuffer(&p->in, 2);
  if( p->rc!=SQLITE_OK ) return p->rc;

  /* If the iterator is already at the end of the changeset, return DONE. */
  if( p->in.iNext>=p->in.nData ){
    return SQLITE_DONE;
  }

  op = p->in.aData[p->in.iNext++];
  if( op=='T' || op=='P' ){
    p->bPatchset = (op=='P');
    if( sessionChangesetReadTblhdr(p) ) return p->rc;
    if( (p->rc = sessionInputBuffer(&p->in, 2)) ) return p->rc;
    op = p->in.aData[p->in.iNext++];
  }

  p->op = op;
  p->bIndirect = p->in.aData[p->in.iNext++];
  if( p->op!=SQLITE_UPDATE && p->op!=SQLITE_DELETE && p->op!=SQLITE_INSERT ){
    return (p->rc = SQLITE_CORRUPT_BKPT);
  }

  if( paRec ){ 
    int nVal;                     /* Number of values to buffer */
    if( p->bPatchset==0 && op==SQLITE_UPDATE ){
      nVal = p->nCol * 2;
    }else if( p->bPatchset && op==SQLITE_DELETE ){
      nVal = 0;
      for(i=0; i<p->nCol; i++) if( p->abPK[i] ) nVal++;
    }else{
      nVal = p->nCol;
    }
    p->rc = sessionChangesetBufferRecord(&p->in, nVal, pnRec);
    if( p->rc!=SQLITE_OK ) return p->rc;
    *paRec = &p->in.aData[p->in.iNext];
    p->in.iNext += *pnRec;
  }else{

    /* If this is an UPDATE or DELETE, read the old.* record. */
    if( p->op!=SQLITE_INSERT && (p->bPatchset==0 || p->op==SQLITE_DELETE) ){
      u8 *abPK = p->bPatchset ? p->abPK : 0;
      p->rc = sessionReadRecord(&p->in, p->nCol, abPK, p->apValue);
      if( p->rc!=SQLITE_OK ) return p->rc;
    }

    /* If this is an INSERT or UPDATE, read the new.* record. */
    if( p->op!=SQLITE_DELETE ){
      p->rc = sessionReadRecord(&p->in, p->nCol, 0, &p->apValue[p->nCol]);
      if( p->rc!=SQLITE_OK ) return p->rc;
    }

    if( p->bPatchset && p->op==SQLITE_UPDATE ){
      /* If this is an UPDATE that is part of a patchset, then all PK and
      ** modified fields are present in the new.* record. The old.* record
      ** is currently completely empty. This block shifts the PK fields from
      ** new.* to old.*, to accommodate the code that reads these arrays.  */
      int i;
      for(i=0; i<p->nCol; i++){
        assert( p->apValue[i]==0 );
        assert( p->abPK[i]==0 || p->apValue[i+p->nCol] );
        if( p->abPK[i] ){
          p->apValue[i] = p->apValue[i+p->nCol];
          p->apValue[i+p->nCol] = 0;
        }
      }
    }
  }

  return SQLITE_ROW;
}

/*
** Advance an iterator created by sqlite3changeset_start() to the next
** change in the changeset. This function may return SQLITE_ROW, SQLITE_DONE
** or SQLITE_CORRUPT.
**
** This function may not be called on iterators passed to a conflict handler
** callback by changeset_apply().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_next(sqlite3_changeset_iter *p){
  return sessionChangesetNext(p, 0, 0);
}

/*
** The following function extracts information on the current change
** from a changeset iterator. It may only be called after changeset_next()
** has returned SQLITE_ROW.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_op(
  sqlite3_changeset_iter *pIter,  /* Iterator handle */
  const char **pzTab,             /* OUT: Pointer to table name */
  int *pnCol,                     /* OUT: Number of columns in table */
  int *pOp,                       /* OUT: SQLITE_INSERT, DELETE or UPDATE */
  int *pbIndirect                 /* OUT: True if change is indirect */
){
  *pOp = pIter->op;
  *pnCol = pIter->nCol;
  *pzTab = pIter->zTab;
  if( pbIndirect ) *pbIndirect = pIter->bIndirect;
  return SQLITE_OK;
}

/*
** Return information regarding the PRIMARY KEY and number of columns in
** the database table affected by the change that pIter currently points
** to. This function may only be called after changeset_next() returns
** SQLITE_ROW.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_pk(
  sqlite3_changeset_iter *pIter,  /* Iterator object */
  unsigned char **pabPK,          /* OUT: Array of boolean - true for PK cols */
  int *pnCol                      /* OUT: Number of entries in output array */
){
  *pabPK = pIter->abPK;
  if( pnCol ) *pnCol = pIter->nCol;
  return SQLITE_OK;
}

/*
** This function may only be called while the iterator is pointing to an
** SQLITE_UPDATE or SQLITE_DELETE change (see sqlite3changeset_op()).
** Otherwise, SQLITE_MISUSE is returned.
**
** It sets *ppValue to point to an sqlite3_value structure containing the
** iVal'th value in the old.* record. Or, if that particular value is not
** included in the record (because the change is an UPDATE and the field
** was not modified and is not a PK column), set *ppValue to NULL.
**
** If value iVal is out-of-range, SQLITE_RANGE is returned and *ppValue is
** not modified. Otherwise, SQLITE_OK.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_old(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of old.* value to retrieve */
  sqlite3_value **ppValue         /* OUT: Old value (or NULL pointer) */
){
  if( pIter->op!=SQLITE_UPDATE && pIter->op!=SQLITE_DELETE ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = pIter->apValue[iVal];
  return SQLITE_OK;
}

/*
** This function may only be called while the iterator is pointing to an
** SQLITE_UPDATE or SQLITE_INSERT change (see sqlite3changeset_op()).
** Otherwise, SQLITE_MISUSE is returned.
**
** It sets *ppValue to point to an sqlite3_value structure containing the
** iVal'th value in the new.* record. Or, if that particular value is not
** included in the record (because the change is an UPDATE and the field
** was not modified), set *ppValue to NULL.
**
** If value iVal is out-of-range, SQLITE_RANGE is returned and *ppValue is
** not modified. Otherwise, SQLITE_OK.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_new(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of new.* value to retrieve */
  sqlite3_value **ppValue         /* OUT: New value (or NULL pointer) */
){
  if( pIter->op!=SQLITE_UPDATE && pIter->op!=SQLITE_INSERT ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = pIter->apValue[pIter->nCol+iVal];
  return SQLITE_OK;
}

/*
** The following two macros are used internally. They are similar to the
** sqlite3changeset_new() and sqlite3changeset_old() functions, except that
** they omit all error checking and return a pointer to the requested value.
*/
#define sessionChangesetNew(pIter, iVal) (pIter)->apValue[(pIter)->nCol+(iVal)]
#define sessionChangesetOld(pIter, iVal) (pIter)->apValue[(iVal)]

/*
** This function may only be called with a changeset iterator that has been
** passed to an SQLITE_CHANGESET_DATA or SQLITE_CHANGESET_CONFLICT 
** conflict-handler function. Otherwise, SQLITE_MISUSE is returned.
**
** If successful, *ppValue is set to point to an sqlite3_value structure
** containing the iVal'th value of the conflicting record.
**
** If value iVal is out-of-range or some other error occurs, an SQLite error
** code is returned. Otherwise, SQLITE_OK.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_conflict(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of conflict record value to fetch */
  sqlite3_value **ppValue         /* OUT: Value from conflicting row */
){
  if( !pIter->pConflict ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=sqlite3_column_count(pIter->pConflict) ){
    return SQLITE_RANGE;
  }
  *ppValue = sqlite3_column_value(pIter->pConflict, iVal);
  return SQLITE_OK;
}

/*
** This function may only be called with an iterator passed to an
** SQLITE_CHANGESET_FOREIGN_KEY conflict handler callback. In this case
** it sets the output variable to the total number of known foreign key
** violations in the destination database and returns SQLITE_OK.
**
** In all other cases this function returns SQLITE_MISUSE.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_fk_conflicts(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int *pnOut                      /* OUT: Number of FK violations */
){
  if( pIter->pConflict || pIter->apValue ){
    return SQLITE_MISUSE;
  }
  *pnOut = pIter->nCol;
  return SQLITE_OK;
}


/*
** Finalize an iterator allocated with sqlite3changeset_start().
**
** This function may not be called on iterators passed to a conflict handler
** callback by changeset_apply().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_finalize(sqlite3_changeset_iter *p){
  int rc = SQLITE_OK;
  if( p ){
    int i;                        /* Used to iterate through p->apValue[] */
    rc = p->rc;
    if( p->apValue ){
      for(i=0; i<p->nCol*2; i++) sqlite3ValueFree(p->apValue[i]);
    }
    sqlite3_free(p->tblhdr.aBuf);
    sqlite3_free(p->in.buf.aBuf);
    sqlite3_free(p);
  }
  return rc;
}

static int sessionChangesetInvert(
  SessionInput *pInput,           /* Input changeset */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,
  int *pnInverted,                /* OUT: Number of bytes in output changeset */
  void **ppInverted               /* OUT: Inverse of pChangeset */
){
  int rc = SQLITE_OK;             /* Return value */
  SessionBuffer sOut;             /* Output buffer */
  int nCol = 0;                   /* Number of cols in current table */
  u8 *abPK = 0;                   /* PK array for current table */
  sqlite3_value **apVal = 0;      /* Space for values for UPDATE inversion */
  SessionBuffer sPK = {0, 0, 0};  /* PK array for current table */

  /* Initialize the output buffer */
  memset(&sOut, 0, sizeof(SessionBuffer));

  /* Zero the output variables in case an error occurs. */
  if( ppInverted ){
    *ppInverted = 0;
    *pnInverted = 0;
  }

  while( 1 ){
    u8 eType;

    /* Test for EOF. */
    if( (rc = sessionInputBuffer(pInput, 2)) ) goto finished_invert;
    if( pInput->iNext>=pInput->nData ) break;
    eType = pInput->aData[pInput->iNext];

    switch( eType ){
      case 'T': {
        /* A 'table' record consists of:
        **
        **   * A constant 'T' character,
        **   * Number of columns in said table (a varint),
        **   * An array of nCol bytes (sPK),
        **   * A nul-terminated table name.
        */
        int nByte;
        int nVar;
        pInput->iNext++;
        if( (rc = sessionChangesetBufferTblhdr(pInput, &nByte)) ){
          goto finished_invert;
        }
        nVar = sessionVarintGet(&pInput->aData[pInput->iNext], &nCol);
        sPK.nBuf = 0;
        sessionAppendBlob(&sPK, &pInput->aData[pInput->iNext+nVar], nCol, &rc);
        sessionAppendByte(&sOut, eType, &rc);
        sessionAppendBlob(&sOut, &pInput->aData[pInput->iNext], nByte, &rc);
        if( rc ) goto finished_invert;

        pInput->iNext += nByte;
        sqlite3_free(apVal);
        apVal = 0;
        abPK = sPK.aBuf;
        break;
      }

      case SQLITE_INSERT:
      case SQLITE_DELETE: {
        int nByte;
        int bIndirect = pInput->aData[pInput->iNext+1];
        int eType2 = (eType==SQLITE_DELETE ? SQLITE_INSERT : SQLITE_DELETE);
        pInput->iNext += 2;
        assert( rc==SQLITE_OK );
        rc = sessionChangesetBufferRecord(pInput, nCol, &nByte);
        sessionAppendByte(&sOut, eType2, &rc);
        sessionAppendByte(&sOut, bIndirect, &rc);
        sessionAppendBlob(&sOut, &pInput->aData[pInput->iNext], nByte, &rc);
        pInput->iNext += nByte;
        if( rc ) goto finished_invert;
        break;
      }

      case SQLITE_UPDATE: {
        int iCol;

        if( 0==apVal ){
          apVal = (sqlite3_value **)sqlite3_malloc(sizeof(apVal[0])*nCol*2);
          if( 0==apVal ){
            rc = SQLITE_NOMEM;
            goto finished_invert;
          }
          memset(apVal, 0, sizeof(apVal[0])*nCol*2);
        }

        /* Write the header for the new UPDATE change. Same as the original. */
        sessionAppendByte(&sOut, eType, &rc);
        sessionAppendByte(&sOut, pInput->aData[pInput->iNext+1], &rc);

        /* Read the old.* and new.* records for the update change. */
        pInput->iNext += 2;
        rc = sessionReadRecord(pInput, nCol, 0, &apVal[0]);
        if( rc==SQLITE_OK ){
          rc = sessionReadRecord(pInput, nCol, 0, &apVal[nCol]);
        }

        /* Write the new old.* record. Consists of the PK columns from the
        ** original old.* record, and the other values from the original
        ** new.* record. */
        for(iCol=0; iCol<nCol; iCol++){
          sqlite3_value *pVal = apVal[iCol + (abPK[iCol] ? 0 : nCol)];
          sessionAppendValue(&sOut, pVal, &rc);
        }

        /* Write the new new.* record. Consists of a copy of all values
        ** from the original old.* record, except for the PK columns, which
        ** are set to "undefined". */
        for(iCol=0; iCol<nCol; iCol++){
          sqlite3_value *pVal = (abPK[iCol] ? 0 : apVal[iCol]);
          sessionAppendValue(&sOut, pVal, &rc);
        }

        for(iCol=0; iCol<nCol*2; iCol++){
          sqlite3ValueFree(apVal[iCol]);
        }
        memset(apVal, 0, sizeof(apVal[0])*nCol*2);
        if( rc!=SQLITE_OK ){
          goto finished_invert;
        }

        break;
      }

      default:
        rc = SQLITE_CORRUPT_BKPT;
        goto finished_invert;
    }

    assert( rc==SQLITE_OK );
    if( xOutput && sOut.nBuf>=SESSIONS_STRM_CHUNK_SIZE ){
      rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
      sOut.nBuf = 0;
      if( rc!=SQLITE_OK ) goto finished_invert;
    }
  }

  assert( rc==SQLITE_OK );
  if( pnInverted ){
    *pnInverted = sOut.nBuf;
    *ppInverted = sOut.aBuf;
    sOut.aBuf = 0;
  }else if( sOut.nBuf>0 ){
    rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
  }

 finished_invert:
  sqlite3_free(sOut.aBuf);
  sqlite3_free(apVal);
  sqlite3_free(sPK.aBuf);
  return rc;
}


/*
** Invert a changeset object.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_invert(
  int nChangeset,                 /* Number of bytes in input */
  const void *pChangeset,         /* Input changeset */
  int *pnInverted,                /* OUT: Number of bytes in output changeset */
  void **ppInverted               /* OUT: Inverse of pChangeset */
){
  SessionInput sInput;

  /* Set up the input stream */
  memset(&sInput, 0, sizeof(SessionInput));
  sInput.nData = nChangeset;
  sInput.aData = (u8*)pChangeset;

  return sessionChangesetInvert(&sInput, 0, 0, pnInverted, ppInverted);
}

/*
** Streaming version of sqlite3changeset_invert().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_invert_strm(
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  SessionInput sInput;
  int rc;

  /* Set up the input stream */
  memset(&sInput, 0, sizeof(SessionInput));
  sInput.xInput = xInput;
  sInput.pIn = pIn;

  rc = sessionChangesetInvert(&sInput, xOutput, pOut, 0, 0);
  sqlite3_free(sInput.buf.aBuf);
  return rc;
}

typedef struct SessionApplyCtx SessionApplyCtx;
struct SessionApplyCtx {
  sqlite3 *db;
  sqlite3_stmt *pDelete;          /* DELETE statement */
  sqlite3_stmt *pUpdate;          /* UPDATE statement */
  sqlite3_stmt *pInsert;          /* INSERT statement */
  sqlite3_stmt *pSelect;          /* SELECT statement */
  int nCol;                       /* Size of azCol[] and abPK[] arrays */
  const char **azCol;             /* Array of column names */
  u8 *abPK;                       /* Boolean array - true if column is in PK */
};

/*
** Formulate a statement to DELETE a row from database db. Assuming a table
** structure like this:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The DELETE statement looks like this:
**
**     DELETE FROM x WHERE a = :1 AND c = :3 AND (:5 OR b IS :2 AND d IS :4)
**
** Variable :5 (nCol+1) is a boolean. It should be set to 0 if we require
** matching b and d values, or 1 otherwise. The second case comes up if the
** conflict handler is invoked with NOTFOUND and returns CHANGESET_REPLACE.
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pDelete is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionDeleteRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int i;
  const char *zSep = "";
  int rc = SQLITE_OK;
  SessionBuffer buf = {0, 0, 0};
  int nPk = 0;

  sessionAppendStr(&buf, "DELETE FROM ", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " WHERE ", &rc);

  for(i=0; i<p->nCol; i++){
    if( p->abPK[i] ){
      nPk++;
      sessionAppendStr(&buf, zSep, &rc);
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i+1, &rc);
      zSep = " AND ";
    }
  }

  if( nPk<p->nCol ){
    sessionAppendStr(&buf, " AND (?", &rc);
    sessionAppendInteger(&buf, p->nCol+1, &rc);
    sessionAppendStr(&buf, " OR ", &rc);

    zSep = "";
    for(i=0; i<p->nCol; i++){
      if( !p->abPK[i] ){
        sessionAppendStr(&buf, zSep, &rc);
        sessionAppendIdent(&buf, p->azCol[i], &rc);
        sessionAppendStr(&buf, " IS ?", &rc);
        sessionAppendInteger(&buf, i+1, &rc);
        zSep = "AND ";
      }
    }
    sessionAppendStr(&buf, ")", &rc);
  }

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, &p->pDelete, 0);
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Formulate and prepare a statement to UPDATE a row from database db. 
** Assuming a table structure like this:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The UPDATE statement looks like this:
**
**     UPDATE x SET
**     a = CASE WHEN ?2  THEN ?3  ELSE a END,
**     b = CASE WHEN ?5  THEN ?6  ELSE b END,
**     c = CASE WHEN ?8  THEN ?9  ELSE c END,
**     d = CASE WHEN ?11 THEN ?12 ELSE d END
**     WHERE a = ?1 AND c = ?7 AND (?13 OR 
**       (?5==0 OR b IS ?4) AND (?11==0 OR d IS ?10) AND
**     )
**
** For each column in the table, there are three variables to bind:
**
**     ?(i*3+1)    The old.* value of the column, if any.
**     ?(i*3+2)    A boolean flag indicating that the value is being modified.
**     ?(i*3+3)    The new.* value of the column, if any.
**
** Also, a boolean flag that, if set to true, causes the statement to update
** a row even if the non-PK values do not match. This is required if the
** conflict-handler is invoked with CHANGESET_DATA and returns
** CHANGESET_REPLACE. This is variable "?(nCol*3+1)".
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pUpdate is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionUpdateRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int rc = SQLITE_OK;
  int i;
  const char *zSep = "";
  SessionBuffer buf = {0, 0, 0};

  /* Append "UPDATE tbl SET " */
  sessionAppendStr(&buf, "UPDATE ", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " SET ", &rc);

  /* Append the assignments */
  for(i=0; i<p->nCol; i++){
    sessionAppendStr(&buf, zSep, &rc);
    sessionAppendIdent(&buf, p->azCol[i], &rc);
    sessionAppendStr(&buf, " = CASE WHEN ?", &rc);
    sessionAppendInteger(&buf, i*3+2, &rc);
    sessionAppendStr(&buf, " THEN ?", &rc);
    sessionAppendInteger(&buf, i*3+3, &rc);
    sessionAppendStr(&buf, " ELSE ", &rc);
    sessionAppendIdent(&buf, p->azCol[i], &rc);
    sessionAppendStr(&buf, " END", &rc);
    zSep = ", ";
  }

  /* Append the PK part of the WHERE clause */
  sessionAppendStr(&buf, " WHERE ", &rc);
  for(i=0; i<p->nCol; i++){
    if( p->abPK[i] ){
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i*3+1, &rc);
      sessionAppendStr(&buf, " AND ", &rc);
    }
  }

  /* Append the non-PK part of the WHERE clause */
  sessionAppendStr(&buf, " (?", &rc);
  sessionAppendInteger(&buf, p->nCol*3+1, &rc);
  sessionAppendStr(&buf, " OR 1", &rc);
  for(i=0; i<p->nCol; i++){
    if( !p->abPK[i] ){
      sessionAppendStr(&buf, " AND (?", &rc);
      sessionAppendInteger(&buf, i*3+2, &rc);
      sessionAppendStr(&buf, "=0 OR ", &rc);
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " IS ?", &rc);
      sessionAppendInteger(&buf, i*3+1, &rc);
      sessionAppendStr(&buf, ")", &rc);
    }
  }
  sessionAppendStr(&buf, ")", &rc);

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, &p->pUpdate, 0);
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Formulate and prepare an SQL statement to query table zTab by primary
** key. Assuming the following table structure:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The SELECT statement looks like this:
**
**     SELECT * FROM x WHERE a = ?1 AND c = ?3
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pSelect is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionSelectRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  return sessionSelectStmt(
      db, "main", zTab, p->nCol, p->azCol, p->abPK, &p->pSelect);
}

/*
** Formulate and prepare an INSERT statement to add a record to table zTab.
** For example:
**
**     INSERT INTO main."zTab" VALUES(?1, ?2, ?3 ...);
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pInsert is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionInsertRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int rc = SQLITE_OK;
  int i;
  SessionBuffer buf = {0, 0, 0};

  sessionAppendStr(&buf, "INSERT INTO main.", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " VALUES(?", &rc);
  for(i=1; i<p->nCol; i++){
    sessionAppendStr(&buf, ", ?", &rc);
  }
  sessionAppendStr(&buf, ")", &rc);

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, &p->pInsert, 0);
  }
  sqlite3_free(buf.aBuf);
  return rc;
}

/*
** A wrapper around sqlite3_bind_value() that detects an extra problem. 
** See comments in the body of this function for details.
*/
static int sessionBindValue(
  sqlite3_stmt *pStmt,            /* Statement to bind value to */
  int i,                          /* Parameter number to bind to */
  sqlite3_value *pVal             /* Value to bind */
){
  int eType = sqlite3_value_type(pVal);
  /* COVERAGE: The (pVal->z==0) branch is never true using current versions
  ** of SQLite. If a malloc fails in an sqlite3_value_xxx() function, either
  ** the (pVal->z) variable remains as it was or the type of the value is
  ** set to SQLITE_NULL.  */
  if( (eType==SQLITE_TEXT || eType==SQLITE_BLOB) && pVal->z==0 ){
    /* This condition occurs when an earlier OOM in a call to
    ** sqlite3_value_text() or sqlite3_value_blob() (perhaps from within
    ** a conflict-handler) has zeroed the pVal->z pointer. Return NOMEM. */
    return SQLITE_NOMEM;
  }
  return sqlite3_bind_value(pStmt, i, pVal);
}

/*
** Iterator pIter must point to an SQLITE_INSERT entry. This function 
** transfers new.* values from the current iterator entry to statement
** pStmt. The table being inserted into has nCol columns.
**
** New.* value $i 0 from the iterator is bound to variable ($i+1) of 
** statement pStmt. If parameter abPK is NULL, all values from 0 to (nCol-1)
** are transfered to the statement. Otherwise, if abPK is not NULL, it points
** to an array nCol elements in size. In this case only those values for 
** which abPK[$i] is true are read from the iterator and bound to the 
** statement.
**
** An SQLite error code is returned if an error occurs. Otherwise, SQLITE_OK.
*/
static int sessionBindRow(
  sqlite3_changeset_iter *pIter,  /* Iterator to read values from */
  int(*xValue)(sqlite3_changeset_iter *, int, sqlite3_value **),
  int nCol,                       /* Number of columns */
  u8 *abPK,                       /* If not NULL, bind only if true */
  sqlite3_stmt *pStmt             /* Bind values to this statement */
){
  int i;
  int rc = SQLITE_OK;

  /* Neither sqlite3changeset_old or sqlite3changeset_new can fail if the
  ** argument iterator points to a suitable entry. Make sure that xValue 
  ** is one of these to guarantee that it is safe to ignore the return 
  ** in the code below. */
  assert( xValue==sqlite3changeset_old || xValue==sqlite3changeset_new );

  for(i=0; rc==SQLITE_OK && i<nCol; i++){
    if( !abPK || abPK[i] ){
      sqlite3_value *pVal;
      (void)xValue(pIter, i, &pVal);
      rc = sessionBindValue(pStmt, i+1, pVal);
    }
  }
  return rc;
}

/*
** SQL statement pSelect is as generated by the sessionSelectRow() function.
** This function binds the primary key values from the change that changeset
** iterator pIter points to to the SELECT and attempts to seek to the table
** entry. If a row is found, the SELECT statement left pointing at the row 
** and SQLITE_ROW is returned. Otherwise, if no row is found and no error
** has occured, the statement is reset and SQLITE_OK is returned. If an
** error occurs, the statement is reset and an SQLite error code is returned.
**
** If this function returns SQLITE_ROW, the caller must eventually reset() 
** statement pSelect. If any other value is returned, the statement does
** not require a reset().
**
** If the iterator currently points to an INSERT record, bind values from the
** new.* record to the SELECT statement. Or, if it points to a DELETE or
** UPDATE, bind values from the old.* record. 
*/
static int sessionSeekToRow(
  sqlite3 *db,                    /* Database handle */
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  u8 *abPK,                       /* Primary key flags array */
  sqlite3_stmt *pSelect           /* SELECT statement from sessionSelectRow() */
){
  int rc;                         /* Return code */
  int nCol;                       /* Number of columns in table */
  int op;                         /* Changset operation (SQLITE_UPDATE etc.) */
  const char *zDummy;             /* Unused */

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);
  rc = sessionBindRow(pIter, 
      op==SQLITE_INSERT ? sqlite3changeset_new : sqlite3changeset_old,
      nCol, abPK, pSelect
  );

  if( rc==SQLITE_OK ){
    rc = sqlite3_step(pSelect);
    if( rc!=SQLITE_ROW ) rc = sqlite3_reset(pSelect);
  }

  return rc;
}

/*
** Invoke the conflict handler for the change that the changeset iterator
** currently points to.
**
** Argument eType must be either CHANGESET_DATA or CHANGESET_CONFLICT.
** If argument pbReplace is NULL, then the type of conflict handler invoked
** depends solely on eType, as follows:
**
**    eType value                 Value passed to xConflict
**    -------------------------------------------------
**    CHANGESET_DATA              CHANGESET_NOTFOUND
**    CHANGESET_CONFLICT          CHANGESET_CONSTRAINT
**
** Or, if pbReplace is not NULL, then an attempt is made to find an existing
** record with the same primary key as the record about to be deleted, updated
** or inserted. If such a record can be found, it is available to the conflict
** handler as the "conflicting" record. In this case the type of conflict
** handler invoked is as follows:
**
**    eType value         PK Record found?   Value passed to xConflict
**    ----------------------------------------------------------------
**    CHANGESET_DATA      Yes                CHANGESET_DATA
**    CHANGESET_DATA      No                 CHANGESET_NOTFOUND
**    CHANGESET_CONFLICT  Yes                CHANGESET_CONFLICT
**    CHANGESET_CONFLICT  No                 CHANGESET_CONSTRAINT
**
** If pbReplace is not NULL, and a record with a matching PK is found, and
** the conflict handler function returns SQLITE_CHANGESET_REPLACE, *pbReplace
** is set to non-zero before returning SQLITE_OK.
**
** If the conflict handler returns SQLITE_CHANGESET_ABORT, SQLITE_ABORT is
** returned. Or, if the conflict handler returns an invalid value, 
** SQLITE_MISUSE. If the conflict handler returns SQLITE_CHANGESET_OMIT,
** this function returns SQLITE_OK.
*/
static int sessionConflictHandler(
  int eType,                      /* Either CHANGESET_DATA or CONFLICT */
  SessionApplyCtx *p,             /* changeset_apply() context */
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int(*xConflict)(void *, int, sqlite3_changeset_iter*),
  void *pCtx,                     /* First argument for conflict handler */
  int *pbReplace                  /* OUT: Set to true if PK row is found */
){
  int res;                        /* Value returned by conflict handler */
  int rc;
  int nCol;
  int op;
  const char *zDummy;

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);

  assert( eType==SQLITE_CHANGESET_CONFLICT || eType==SQLITE_CHANGESET_DATA );
  assert( SQLITE_CHANGESET_CONFLICT+1==SQLITE_CHANGESET_CONSTRAINT );
  assert( SQLITE_CHANGESET_DATA+1==SQLITE_CHANGESET_NOTFOUND );

  /* Bind the new.* PRIMARY KEY values to the SELECT statement. */
  if( pbReplace ){
    rc = sessionSeekToRow(p->db, pIter, p->abPK, p->pSelect);
  }else{
    rc = SQLITE_OK;
  }

  if( rc==SQLITE_ROW ){
    /* There exists another row with the new.* primary key. */
    pIter->pConflict = p->pSelect;
    res = xConflict(pCtx, eType, pIter);
    pIter->pConflict = 0;
    rc = sqlite3_reset(p->pSelect);
  }else if( rc==SQLITE_OK ){
    /* No other row with the new.* primary key. */
    res = xConflict(pCtx, eType+1, pIter);
    if( res==SQLITE_CHANGESET_REPLACE ) rc = SQLITE_MISUSE;
  }

  if( rc==SQLITE_OK ){
    switch( res ){
      case SQLITE_CHANGESET_REPLACE:
        assert( pbReplace );
        *pbReplace = 1;
        break;

      case SQLITE_CHANGESET_OMIT:
        break;

      case SQLITE_CHANGESET_ABORT:
        rc = SQLITE_ABORT;
        break;

      default:
        rc = SQLITE_MISUSE;
        break;
    }
  }

  return rc;
}

/*
** Attempt to apply the change that the iterator passed as the first argument
** currently points to to the database. If a conflict is encountered, invoke
** the conflict handler callback.
**
** If argument pbRetry is NULL, then ignore any CHANGESET_DATA conflict. If
** one is encountered, update or delete the row with the matching primary key
** instead. Or, if pbRetry is not NULL and a CHANGESET_DATA conflict occurs,
** invoke the conflict handler. If it returns CHANGESET_REPLACE, set *pbRetry
** to true before returning. In this case the caller will invoke this function
** again, this time with pbRetry set to NULL.
**
** If argument pbReplace is NULL and a CHANGESET_CONFLICT conflict is 
** encountered invoke the conflict handler with CHANGESET_CONSTRAINT instead.
** Or, if pbReplace is not NULL, invoke it with CHANGESET_CONFLICT. If such
** an invocation returns SQLITE_CHANGESET_REPLACE, set *pbReplace to true
** before retrying. In this case the caller attempts to remove the conflicting
** row before invoking this function again, this time with pbReplace set 
** to NULL.
**
** If any conflict handler returns SQLITE_CHANGESET_ABORT, this function
** returns SQLITE_ABORT. Otherwise, if no error occurs, SQLITE_OK is 
** returned.
*/
static int sessionApplyOneOp(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  SessionApplyCtx *p,             /* changeset_apply() context */
  int(*xConflict)(void *, int, sqlite3_changeset_iter *),
  void *pCtx,                     /* First argument for the conflict handler */
  int *pbReplace,                 /* OUT: True to remove PK row and retry */
  int *pbRetry                    /* OUT: True to retry. */
){
  const char *zDummy;
  int op;
  int nCol;
  int rc = SQLITE_OK;

  assert( p->pDelete && p->pUpdate && p->pInsert && p->pSelect );
  assert( p->azCol && p->abPK );
  assert( !pbReplace || *pbReplace==0 );

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);

  if( op==SQLITE_DELETE ){

    /* Bind values to the DELETE statement. If conflict handling is required,
    ** bind values for all columns and set bound variable (nCol+1) to true.
    ** Or, if conflict handling is not required, bind just the PK column
    ** values and, if it exists, set (nCol+1) to false. Conflict handling
    ** is not required if:
    **
    **   * this is a patchset, or
    **   * (pbRetry==0), or
    **   * all columns of the table are PK columns (in this case there is
    **     no (nCol+1) variable to bind to).
    */
    u8 *abPK = (pIter->bPatchset ? p->abPK : 0);
    rc = sessionBindRow(pIter, sqlite3changeset_old, nCol, abPK, p->pDelete);
    if( rc==SQLITE_OK && sqlite3_bind_parameter_count(p->pDelete)>nCol ){
      rc = sqlite3_bind_int(p->pDelete, nCol+1, (pbRetry==0 || abPK));
    }
    if( rc!=SQLITE_OK ) return rc;

    sqlite3_step(p->pDelete);
    rc = sqlite3_reset(p->pDelete);
    if( rc==SQLITE_OK && sqlite3_changes(p->db)==0 ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_DATA, p, pIter, xConflict, pCtx, pbRetry
      );
    }else if( (rc&0xff)==SQLITE_CONSTRAINT ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, 0
      );
    }

  }else if( op==SQLITE_UPDATE ){
    int i;

    /* Bind values to the UPDATE statement. */
    for(i=0; rc==SQLITE_OK && i<nCol; i++){
      sqlite3_value *pOld = sessionChangesetOld(pIter, i);
      sqlite3_value *pNew = sessionChangesetNew(pIter, i);

      sqlite3_bind_int(p->pUpdate, i*3+2, !!pNew);
      if( pOld ){
        rc = sessionBindValue(p->pUpdate, i*3+1, pOld);
      }
      if( rc==SQLITE_OK && pNew ){
        rc = sessionBindValue(p->pUpdate, i*3+3, pNew);
      }
    }
    if( rc==SQLITE_OK ){
      sqlite3_bind_int(p->pUpdate, nCol*3+1, pbRetry==0 || pIter->bPatchset);
    }
    if( rc!=SQLITE_OK ) return rc;

    /* Attempt the UPDATE. In the case of a NOTFOUND or DATA conflict,
    ** the result will be SQLITE_OK with 0 rows modified. */
    sqlite3_step(p->pUpdate);
    rc = sqlite3_reset(p->pUpdate);

    if( rc==SQLITE_OK && sqlite3_changes(p->db)==0 ){
      /* A NOTFOUND or DATA error. Search the table to see if it contains
      ** a row with a matching primary key. If so, this is a DATA conflict.
      ** Otherwise, if there is no primary key match, it is a NOTFOUND. */

      rc = sessionConflictHandler(
          SQLITE_CHANGESET_DATA, p, pIter, xConflict, pCtx, pbRetry
      );

    }else if( (rc&0xff)==SQLITE_CONSTRAINT ){
      /* This is always a CONSTRAINT conflict. */
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, 0
      );
    }

  }else{
    assert( op==SQLITE_INSERT );
    rc = sessionBindRow(pIter, sqlite3changeset_new, nCol, 0, p->pInsert);
    if( rc!=SQLITE_OK ) return rc;

    sqlite3_step(p->pInsert);
    rc = sqlite3_reset(p->pInsert);
    if( (rc&0xff)==SQLITE_CONSTRAINT ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, pbReplace
      );
    }
  }

  return rc;
}

/*
** Argument pIter is a changeset iterator that has been initialized, but
** not yet passed to sqlite3changeset_next(). This function applies the 
** changeset to the main database attached to handle "db". The supplied
** conflict handler callback is invoked to resolve any conflicts encountered
** while applying the change.
*/
static int sessionChangesetApply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  sqlite3_changeset_iter *pIter,  /* Changeset to apply */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of fifth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  int schemaMismatch = 0;
  int rc;                         /* Return code */
  const char *zTab = 0;           /* Name of current table */
  int nTab = 0;                   /* Result of sqlite3Strlen30(zTab) */
  SessionApplyCtx sApply;         /* changeset_apply() context object */

  assert( xConflict!=0 );

  memset(&sApply, 0, sizeof(sApply));
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  rc = sqlite3_exec(db, "SAVEPOINT changeset_apply", 0, 0, 0);
  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(db, "PRAGMA defer_foreign_keys = 1", 0, 0, 0);
  }
  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3changeset_next(pIter) ){
    int nCol;
    int op;
    int bReplace = 0;
    int bRetry = 0;
    const char *zNew;
    
    sqlite3changeset_op(pIter, &zNew, &nCol, &op, 0);

    if( zTab==0 || sqlite3_strnicmp(zNew, zTab, nTab+1) ){
      u8 *abPK;

      sqlite3_free((char*)sApply.azCol);  /* cast works around VC++ bug */
      sqlite3_finalize(sApply.pDelete);
      sqlite3_finalize(sApply.pUpdate); 
      sqlite3_finalize(sApply.pInsert);
      sqlite3_finalize(sApply.pSelect);
      memset(&sApply, 0, sizeof(sApply));
      sApply.db = db;

      /* If an xFilter() callback was specified, invoke it now. If the 
      ** xFilter callback returns zero, skip this table. If it returns
      ** non-zero, proceed. */
      schemaMismatch = (xFilter && (0==xFilter(pCtx, zNew)));
      if( schemaMismatch ){
        zTab = sqlite3_mprintf("%s", zNew);
        if( zTab==0 ){
          rc = SQLITE_NOMEM;
          break;
        }
        nTab = (int)strlen(zTab);
        sApply.azCol = (const char **)zTab;
      }else{
        sqlite3changeset_pk(pIter, &abPK, 0);
        rc = sessionTableInfo(
            db, "main", zNew, &sApply.nCol, &zTab, &sApply.azCol, &sApply.abPK
        );
        if( rc!=SQLITE_OK ) break;
  
        if( sApply.nCol==0 ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, 
              "sqlite3changeset_apply(): no such table: %s", zTab
          );
        }
        else if( sApply.nCol!=nCol ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, 
              "sqlite3changeset_apply(): table %s has %d columns, expected %d", 
              zTab, sApply.nCol, nCol
          );
        }
        else if( memcmp(sApply.abPK, abPK, nCol)!=0 ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, "sqlite3changeset_apply(): "
              "primary key mismatch for table %s", zTab
          );
        }
        else if( 
            (rc = sessionSelectRow(db, zTab, &sApply))
         || (rc = sessionUpdateRow(db, zTab, &sApply))
         || (rc = sessionDeleteRow(db, zTab, &sApply))
         || (rc = sessionInsertRow(db, zTab, &sApply))
        ){
          break;
        }
        nTab = sqlite3Strlen30(zTab);
      }
    }

    /* If there is a schema mismatch on the current table, proceed to the
    ** next change. A log message has already been issued. */
    if( schemaMismatch ) continue;

    rc = sessionApplyOneOp(pIter, &sApply, xConflict, pCtx, &bReplace, &bRetry);

    if( rc==SQLITE_OK && bRetry ){
      rc = sessionApplyOneOp(pIter, &sApply, xConflict, pCtx, &bReplace, 0);
    }

    if( bReplace ){
      assert( pIter->op==SQLITE_INSERT );
      rc = sqlite3_exec(db, "SAVEPOINT replace_op", 0, 0, 0);
      if( rc==SQLITE_OK ){
        rc = sessionBindRow(pIter, 
            sqlite3changeset_new, sApply.nCol, sApply.abPK, sApply.pDelete);
        sqlite3_bind_int(sApply.pDelete, sApply.nCol+1, 1);
      }
      if( rc==SQLITE_OK ){
        sqlite3_step(sApply.pDelete);
        rc = sqlite3_reset(sApply.pDelete);
      }
      if( rc==SQLITE_OK ){
        rc = sessionApplyOneOp(pIter, &sApply, xConflict, pCtx, 0, 0);
      }
      if( rc==SQLITE_OK ){
        rc = sqlite3_exec(db, "RELEASE replace_op", 0, 0, 0);
      }
    }
  }

  if( rc==SQLITE_OK ){
    rc = sqlite3changeset_finalize(pIter);
  }else{
    sqlite3changeset_finalize(pIter);
  }

  if( rc==SQLITE_OK ){
    int nFk, notUsed;
    sqlite3_db_status(db, SQLITE_DBSTATUS_DEFERRED_FKS, &nFk, &notUsed, 0);
    if( nFk!=0 ){
      int res = SQLITE_CHANGESET_ABORT;
      sqlite3_changeset_iter sIter;
      memset(&sIter, 0, sizeof(sIter));
      sIter.nCol = nFk;
      res = xConflict(pCtx, SQLITE_CHANGESET_FOREIGN_KEY, &sIter);
      if( res!=SQLITE_CHANGESET_OMIT ){
        rc = SQLITE_CONSTRAINT;
      }
    }
  }
  sqlite3_exec(db, "PRAGMA defer_foreign_keys = 0", 0, 0, 0);

  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(db, "RELEASE changeset_apply", 0, 0, 0);
  }else{
    sqlite3_exec(db, "ROLLBACK TO changeset_apply", 0, 0, 0);
    sqlite3_exec(db, "RELEASE changeset_apply", 0, 0, 0);
  }

  sqlite3_finalize(sApply.pInsert);
  sqlite3_finalize(sApply.pDelete);
  sqlite3_finalize(sApply.pUpdate);
  sqlite3_finalize(sApply.pSelect);
  sqlite3_free((char*)sApply.azCol);  /* cast works around VC++ bug */
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  return rc;
}

/*
** Apply the changeset passed via pChangeset/nChangeset to the main database
** attached to handle "db". Invoke the supplied conflict handler callback
** to resolve any conflicts encountered while applying the change.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_apply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of fifth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  sqlite3_changeset_iter *pIter;  /* Iterator to skip through changeset */  
  int rc = sqlite3changeset_start(&pIter, nChangeset, pChangeset);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetApply(db, pIter, xFilter, xConflict, pCtx);
  }
  return rc;
}

/*
** Apply the changeset passed via xInput/pIn to the main database
** attached to handle "db". Invoke the supplied conflict handler callback
** to resolve any conflicts encountered while applying the change.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_apply_strm(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  sqlite3_changeset_iter *pIter;  /* Iterator to skip through changeset */  
  int rc = sqlite3changeset_start_strm(&pIter, xInput, pIn);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetApply(db, pIter, xFilter, xConflict, pCtx);
  }
  return rc;
}

/*
** sqlite3_changegroup handle.
*/
struct sqlite3_changegroup {
  int rc;                         /* Error code */
  int bPatch;                     /* True to accumulate patchsets */
  SessionTable *pList;            /* List of tables in current patch */
};

/*
** This function is called to merge two changes to the same row together as
** part of an sqlite3changeset_concat() operation. A new change object is
** allocated and a pointer to it stored in *ppNew.
*/
static int sessionChangeMerge(
  SessionTable *pTab,             /* Table structure */
  int bPatchset,                  /* True for patchsets */
  SessionChange *pExist,          /* Existing change */
  int op2,                        /* Second change operation */
  int bIndirect,                  /* True if second change is indirect */
  u8 *aRec,                       /* Second change record */
  int nRec,                       /* Number of bytes in aRec */
  SessionChange **ppNew           /* OUT: Merged change */
){
  SessionChange *pNew = 0;

  if( !pExist ){
    pNew = (SessionChange *)sqlite3_malloc(sizeof(SessionChange) + nRec);
    if( !pNew ){
      return SQLITE_NOMEM;
    }
    memset(pNew, 0, sizeof(SessionChange));
    pNew->op = op2;
    pNew->bIndirect = bIndirect;
    pNew->nRecord = nRec;
    pNew->aRecord = (u8*)&pNew[1];
    memcpy(pNew->aRecord, aRec, nRec);
  }else{
    int op1 = pExist->op;

    /* 
    **   op1=INSERT, op2=INSERT      ->      Unsupported. Discard op2.
    **   op1=INSERT, op2=UPDATE      ->      INSERT.
    **   op1=INSERT, op2=DELETE      ->      (none)
    **
    **   op1=UPDATE, op2=INSERT      ->      Unsupported. Discard op2.
    **   op1=UPDATE, op2=UPDATE      ->      UPDATE.
    **   op1=UPDATE, op2=DELETE      ->      DELETE.
    **
    **   op1=DELETE, op2=INSERT      ->      UPDATE.
    **   op1=DELETE, op2=UPDATE      ->      Unsupported. Discard op2.
    **   op1=DELETE, op2=DELETE      ->      Unsupported. Discard op2.
    */   
    if( (op1==SQLITE_INSERT && op2==SQLITE_INSERT)
     || (op1==SQLITE_UPDATE && op2==SQLITE_INSERT)
     || (op1==SQLITE_DELETE && op2==SQLITE_UPDATE)
     || (op1==SQLITE_DELETE && op2==SQLITE_DELETE)
    ){
      pNew = pExist;
    }else if( op1==SQLITE_INSERT && op2==SQLITE_DELETE ){
      sqlite3_free(pExist);
      assert( pNew==0 );
    }else{
      u8 *aExist = pExist->aRecord;
      int nByte;
      u8 *aCsr;

      /* Allocate a new SessionChange object. Ensure that the aRecord[]
      ** buffer of the new object is large enough to hold any record that
      ** may be generated by combining the input records.  */
      nByte = sizeof(SessionChange) + pExist->nRecord + nRec;
      pNew = (SessionChange *)sqlite3_malloc(nByte);
      if( !pNew ){
        sqlite3_free(pExist);
        return SQLITE_NOMEM;
      }
      memset(pNew, 0, sizeof(SessionChange));
      pNew->bIndirect = (bIndirect && pExist->bIndirect);
      aCsr = pNew->aRecord = (u8 *)&pNew[1];

      if( op1==SQLITE_INSERT ){             /* INSERT + UPDATE */
        u8 *a1 = aRec;
        assert( op2==SQLITE_UPDATE );
        pNew->op = SQLITE_INSERT;
        if( bPatchset==0 ) sessionSkipRecord(&a1, pTab->nCol);
        sessionMergeRecord(&aCsr, pTab->nCol, aExist, a1);
      }else if( op1==SQLITE_DELETE ){       /* DELETE + INSERT */
        assert( op2==SQLITE_INSERT );
        pNew->op = SQLITE_UPDATE;
        if( bPatchset ){
          memcpy(aCsr, aRec, nRec);
          aCsr += nRec;
        }else{
          if( 0==sessionMergeUpdate(&aCsr, pTab, bPatchset, aExist, 0,aRec,0) ){
            sqlite3_free(pNew);
            pNew = 0;
          }
        }
      }else if( op2==SQLITE_UPDATE ){       /* UPDATE + UPDATE */
        u8 *a1 = aExist;
        u8 *a2 = aRec;
        assert( op1==SQLITE_UPDATE );
        if( bPatchset==0 ){
          sessionSkipRecord(&a1, pTab->nCol);
          sessionSkipRecord(&a2, pTab->nCol);
        }
        pNew->op = SQLITE_UPDATE;
        if( 0==sessionMergeUpdate(&aCsr, pTab, bPatchset, aRec, aExist,a1,a2) ){
          sqlite3_free(pNew);
          pNew = 0;
        }
      }else{                                /* UPDATE + DELETE */
        assert( op1==SQLITE_UPDATE && op2==SQLITE_DELETE );
        pNew->op = SQLITE_DELETE;
        if( bPatchset ){
          memcpy(aCsr, aRec, nRec);
          aCsr += nRec;
        }else{
          sessionMergeRecord(&aCsr, pTab->nCol, aRec, aExist);
        }
      }

      if( pNew ){
        pNew->nRecord = (int)(aCsr - pNew->aRecord);
      }
      sqlite3_free(pExist);
    }
  }

  *ppNew = pNew;
  return SQLITE_OK;
}

/*
** Add all changes in the changeset traversed by the iterator passed as
** the first argument to the changegroup hash tables.
*/
static int sessionChangesetToHash(
  sqlite3_changeset_iter *pIter,   /* Iterator to read from */
  sqlite3_changegroup *pGrp        /* Changegroup object to add changeset to */
){
  u8 *aRec;
  int nRec;
  int rc = SQLITE_OK;
  SessionTable *pTab = 0;


  while( SQLITE_ROW==sessionChangesetNext(pIter, &aRec, &nRec) ){
    const char *zNew;
    int nCol;
    int op;
    int iHash;
    int bIndirect;
    SessionChange *pChange;
    SessionChange *pExist = 0;
    SessionChange **pp;

    if( pGrp->pList==0 ){
      pGrp->bPatch = pIter->bPatchset;
    }else if( pIter->bPatchset!=pGrp->bPatch ){
      rc = SQLITE_ERROR;
      break;
    }

    sqlite3changeset_op(pIter, &zNew, &nCol, &op, &bIndirect);
    if( !pTab || sqlite3_stricmp(zNew, pTab->zName) ){
      /* Search the list for a matching table */
      int nNew = (int)strlen(zNew);
      u8 *abPK;

      sqlite3changeset_pk(pIter, &abPK, 0);
      for(pTab = pGrp->pList; pTab; pTab=pTab->pNext){
        if( 0==sqlite3_strnicmp(pTab->zName, zNew, nNew+1) ) break;
      }
      if( !pTab ){
        pTab = sqlite3_malloc(sizeof(SessionTable) + nCol + nNew+1);
        if( !pTab ){
          rc = SQLITE_NOMEM;
          break;
        }
        memset(pTab, 0, sizeof(SessionTable));
        pTab->pNext = pGrp->pList;
        pTab->nCol = nCol;
        pTab->abPK = (u8*)&pTab[1];
        memcpy(pTab->abPK, abPK, nCol);
        pTab->zName = (char*)&pTab->abPK[nCol];
        memcpy(pTab->zName, zNew, nNew+1);
        pGrp->pList = pTab;
      }else if( pTab->nCol!=nCol || memcmp(pTab->abPK, abPK, nCol) ){
        rc = SQLITE_SCHEMA;
        break;
      }
    }

    if( sessionGrowHash(pIter->bPatchset, pTab) ){
      rc = SQLITE_NOMEM;
      break;
    }
    iHash = sessionChangeHash(
        pTab, (pIter->bPatchset && op==SQLITE_DELETE), aRec, pTab->nChange
    );

    /* Search for existing entry. If found, remove it from the hash table. 
    ** Code below may link it back in.
    */
    for(pp=&pTab->apChange[iHash]; *pp; pp=&(*pp)->pNext){
      int bPkOnly1 = 0;
      int bPkOnly2 = 0;
      if( pIter->bPatchset ){
        bPkOnly1 = (*pp)->op==SQLITE_DELETE;
        bPkOnly2 = op==SQLITE_DELETE;
      }
      if( sessionChangeEqual(pTab, bPkOnly1, (*pp)->aRecord, bPkOnly2, aRec) ){
        pExist = *pp;
        *pp = (*pp)->pNext;
        pTab->nEntry--;
        break;
      }
    }

    rc = sessionChangeMerge(pTab, 
        pIter->bPatchset, pExist, op, bIndirect, aRec, nRec, &pChange
    );
    if( rc ) break;
    if( pChange ){
      pChange->pNext = pTab->apChange[iHash];
      pTab->apChange[iHash] = pChange;
      pTab->nEntry++;
    }
  }

  if( rc==SQLITE_OK ) rc = pIter->rc;
  return rc;
}

/*
** Serialize a changeset (or patchset) based on all changesets (or patchsets)
** added to the changegroup object passed as the first argument.
**
** If xOutput is not NULL, then the changeset/patchset is returned to the
** user via one or more calls to xOutput, as with the other streaming
** interfaces. 
**
** Or, if xOutput is NULL, then (*ppOut) is populated with a pointer to a
** buffer containing the output changeset before this function returns. In
** this case (*pnOut) is set to the size of the output buffer in bytes. It
** is the responsibility of the caller to free the output buffer using
** sqlite3_free() when it is no longer required.
**
** If successful, SQLITE_OK is returned. Or, if an error occurs, an SQLite
** error code. If an error occurs and xOutput is NULL, (*ppOut) and (*pnOut)
** are both set to 0 before returning.
*/
static int sessionChangegroupOutput(
  sqlite3_changegroup *pGrp,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,
  int *pnOut,
  void **ppOut
){
  int rc = SQLITE_OK;
  SessionBuffer buf = {0, 0, 0};
  SessionTable *pTab;
  assert( xOutput==0 || (ppOut==0 && pnOut==0) );

  /* Create the serialized output changeset based on the contents of the
  ** hash tables attached to the SessionTable objects in list p->pList. 
  */
  for(pTab=pGrp->pList; rc==SQLITE_OK && pTab; pTab=pTab->pNext){
    int i;
    if( pTab->nEntry==0 ) continue;

    sessionAppendTableHdr(&buf, pGrp->bPatch, pTab, &rc);
    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      for(p=pTab->apChange[i]; p; p=p->pNext){
        sessionAppendByte(&buf, p->op, &rc);
        sessionAppendByte(&buf, p->bIndirect, &rc);
        sessionAppendBlob(&buf, p->aRecord, p->nRecord, &rc);
      }
    }

    if( rc==SQLITE_OK && xOutput && buf.nBuf>=SESSIONS_STRM_CHUNK_SIZE ){
      rc = xOutput(pOut, buf.aBuf, buf.nBuf);
      buf.nBuf = 0;
    }
  }

  if( rc==SQLITE_OK ){
    if( xOutput ){
      if( buf.nBuf>0 ) rc = xOutput(pOut, buf.aBuf, buf.nBuf);
    }else{
      *ppOut = buf.aBuf;
      *pnOut = buf.nBuf;
      buf.aBuf = 0;
    }
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Allocate a new, empty, sqlite3_changegroup.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changegroup_new(sqlite3_changegroup **pp){
  int rc = SQLITE_OK;             /* Return code */
  sqlite3_changegroup *p;         /* New object */
  p = (sqlite3_changegroup*)sqlite3_malloc(sizeof(sqlite3_changegroup));
  if( p==0 ){
    rc = SQLITE_NOMEM;
  }else{
    memset(p, 0, sizeof(sqlite3_changegroup));
  }
  *pp = p;
  return rc;
}

/*
** Add the changeset currently stored in buffer pData, size nData bytes,
** to changeset-group p.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changegroup_add(sqlite3_changegroup *pGrp, int nData, void *pData){
  sqlite3_changeset_iter *pIter;  /* Iterator opened on pData/nData */
  int rc;                         /* Return code */

  rc = sqlite3changeset_start(&pIter, nData, pData);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetToHash(pIter, pGrp);
  }
  sqlite3changeset_finalize(pIter);
  return rc;
}

/*
** Obtain a buffer containing a changeset representing the concatenation
** of all changesets added to the group so far.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changegroup_output(
    sqlite3_changegroup *pGrp,
    int *pnData,
    void **ppData
){
  return sessionChangegroupOutput(pGrp, 0, 0, pnData, ppData);
}

/*
** Streaming versions of changegroup_add().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changegroup_add_strm(
  sqlite3_changegroup *pGrp,
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
){
  sqlite3_changeset_iter *pIter;  /* Iterator opened on pData/nData */
  int rc;                         /* Return code */

  rc = sqlite3changeset_start_strm(&pIter, xInput, pIn);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetToHash(pIter, pGrp);
  }
  sqlite3changeset_finalize(pIter);
  return rc;
}

/*
** Streaming versions of changegroup_output().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changegroup_output_strm(
  sqlite3_changegroup *pGrp,
  int (*xOutput)(void *pOut, const void *pData, int nData), 
  void *pOut
){
  return sessionChangegroupOutput(pGrp, xOutput, pOut, 0, 0);
}

/*
** Delete a changegroup object.
*/
SQLITE_API void SQLITE_STDCALL sqlite3changegroup_delete(sqlite3_changegroup *pGrp){
  if( pGrp ){
    sessionDeleteTable(pGrp->pList);
    sqlite3_free(pGrp);
  }
}

/* 
** Combine two changesets together.
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_concat(
  int nLeft,                      /* Number of bytes in lhs input */
  void *pLeft,                    /* Lhs input changeset */
  int nRight                      /* Number of bytes in rhs input */,
  void *pRight,                   /* Rhs input changeset */
  int *pnOut,                     /* OUT: Number of bytes in output changeset */
  void **ppOut                    /* OUT: changeset (left <concat> right) */
){
  sqlite3_changegroup *pGrp;
  int rc;

  rc = sqlite3changegroup_new(&pGrp);
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add(pGrp, nLeft, pLeft);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add(pGrp, nRight, pRight);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_output(pGrp, pnOut, ppOut);
  }
  sqlite3changegroup_delete(pGrp);

  return rc;
}

/*
** Streaming version of sqlite3changeset_concat().
*/
SQLITE_API int SQLITE_STDCALL sqlite3changeset_concat_strm(
  int (*xInputA)(void *pIn, void *pData, int *pnData),
  void *pInA,
  int (*xInputB)(void *pIn, void *pData, int *pnData),
  void *pInB,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  sqlite3_changegroup *pGrp;
  int rc;

  rc = sqlite3changegroup_new(&pGrp);
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add_strm(pGrp, xInputA, pInA);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add_strm(pGrp, xInputB, pInB);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_output_strm(pGrp, xOutput, pOut);
  }
  sqlite3changegroup_delete(pGrp);

  return rc;
}

#endif /* SQLITE_ENABLE_SESSION && SQLITE_ENABLE_PREUPDATE_HOOK */

/************** End of sqlite3session.c **************************************/
