/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BeSQLite/VirtualTab.h>

#include "SQLite/sqlite3.h"


#define LOG (NativeLogging::CategoryLogger("BeSQLite"))

using namespace std;
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
// @bsiclass DbModule::VirtualTable::Cursor::CallbackData
//=======================================================================================
struct DbModule::VirtualTable::Cursor::CallbackData {
    sqlite3_vtab_cursor base;
    DbModule::VirtualTable::Cursor* m_this;
};

//=======================================================================================
// @bsiclass DbModule::VirtualTable::CallbackData
//=======================================================================================
struct DbModule::VirtualTable::CallbackData {
    sqlite3_vtab base;
    DbModule::VirtualTable* m_this;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbModule::VirtualTable::Cursor::Cursor(DbModule::VirtualTable& vt):m_table(vt){
    m_ctx = new CallbackData();
    m_ctx->m_this = this;
    memset(&m_ctx->base, 0, sizeof(m_ctx->base));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbModule::VirtualTable::Cursor::~Cursor() { delete m_ctx; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbModule::VirtualTable::VirtualTable(DbModule& module):m_module(module) {
    m_ctx = new CallbackData();
    m_ctx->m_this = this;
    memset(&m_ctx->base, 0, sizeof(m_ctx->base));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbModule::VirtualTable::~VirtualTable() { delete m_ctx; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbModule::VirtualTable::SetError(Utf8CP error) {
	if (error == nullptr)
        m_ctx->base.zErrMsg = 0;
	else
    	m_ctx->base.zErrMsg = sqlite3_mprintf("%s", error);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbModule::VirtualTable::Cursor::Context::SetResultBlob(void const* value, int length, CopyData doCopy) {sqlite3_result_blob((sqlite3_context*) this, value, length, (sqlite3_destructor_type) doCopy);}
void DbModule::VirtualTable::Cursor::Context::SetResultDouble(double val){sqlite3_result_double((sqlite3_context*) this, val);}
void DbModule::VirtualTable::Cursor::Context::SetResultError(Utf8CP val, int len){sqlite3_result_error((sqlite3_context*) this, val, len);}
void DbModule::VirtualTable::Cursor::Context::SetResultError_toobig(){sqlite3_result_error_toobig((sqlite3_context*) this);}
void DbModule::VirtualTable::Cursor::Context::SetResultError_nomem(){sqlite3_result_error_nomem((sqlite3_context*) this);}
void DbModule::VirtualTable::Cursor::Context::SetResultError_code(int val){sqlite3_result_error_code((sqlite3_context*) this, val);}
void DbModule::VirtualTable::Cursor::Context::SetResultInt(int val){sqlite3_result_int((sqlite3_context*) this, val);}
void DbModule::VirtualTable::Cursor::Context::SetResultInt64(int64_t val){sqlite3_result_int64((sqlite3_context*) this, val);}
void DbModule::VirtualTable::Cursor::Context::SetResultNull(){sqlite3_result_null((sqlite3_context*) this);}
void DbModule::VirtualTable::Cursor::Context::SetResultText(Utf8CP val, int length, CopyData doCopy){sqlite3_result_text((sqlite3_context*) this, val, length,(sqlite3_destructor_type) doCopy);}
void DbModule::VirtualTable::Cursor::Context::SetResultZeroblob(int length){sqlite3_result_zeroblob((sqlite3_context*)this, length);}
void DbModule::VirtualTable::Cursor::Context::SetResultValue(DbValue val){sqlite3_result_value((sqlite3_context*)this, val.GetSqlValueP());}
void DbModule::VirtualTable::Cursor::Context::SetResultPointer(void* ptr,const char* name ,void(*destroy)(void*) ){sqlite3_result_pointer((sqlite3_context*)this, ptr, name, destroy);}
void DbModule::VirtualTable::Cursor::Context::SetResultZeroBlob(int n){sqlite3_result_zeroblob((sqlite3_context*)this, n);}
void DbModule::VirtualTable::Cursor::Context::SetSubType(int type){sqlite3_result_subtype((sqlite3_context*)this, type);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void vtabDestroy(void* pAux){
    delete (DbModule*)pAux;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int vtabConnect(sqlite3* db,
                    void* pAux,
                    int argc, const char* const* argv,
                    sqlite3_vtab** ppVtab,
                    char** pzErr){
    DbModule* module= (DbModule*)(pAux);
    DbModule::VirtualTable* vt = nullptr;
	DbModule::Config conf;
	auto rc = (int)module->Connect(vt, conf, argc, argv);
    if (rc != SQLITE_OK) {
        if (vt) delete vt;
        return (int)rc;
    }
    *ppVtab = &vt->GetCallbackData()->base;
    rc = sqlite3_declare_vtab(db, module->GetDeclaration().c_str());
	if (rc != BE_SQLITE_OK)
        return rc;
    return sqlite3_vtab_config(db, (int)conf.GetTag());
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int vtabDisconnect(sqlite3_vtab *pVtab){
  delete ((DbModule::VirtualTable::CallbackData*)pVtab)->m_this;
  return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int vtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
    auto vt = (DbModule::VirtualTable::CallbackData*)(p);
    DbModule::VirtualTable::Cursor* cur;
    auto rc = vt->m_this->Open(cur);
    if (rc != BE_SQLITE_OK) {
        if (cur) delete cur;
        return (int)rc;
    }
    *ppCursor = &cur->GetCallbackData()->base;
    return SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int vtabClose(sqlite3_vtab_cursor *pCur){
    auto cur = (DbModule::VirtualTable::Cursor::CallbackData*)pCur;
    auto rc = cur->m_this->Close();
    delete cur->m_this;
    return (int)rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int vtabRowId(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
    int64_t rowId;
    auto rc = ((DbModule::VirtualTable::Cursor::CallbackData*)(cur))->m_this->GetRowId(rowId);
    if (pRowid) {
        *pRowid = (int64_t)rowId;
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int vtabNext(sqlite3_vtab_cursor *cur){ return (int)((DbModule::VirtualTable::Cursor::CallbackData*)(cur))->m_this->Next(); }
static int vtabColumn( sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i ){ return (int)(((DbModule::VirtualTable::Cursor::CallbackData*)(cur))->m_this->GetColumn(i, (DbModule::VirtualTable::Cursor::Context&)*ctx)); }
static int vtabEof(sqlite3_vtab_cursor *cur){ return (int)(((DbModule::VirtualTable::Cursor::CallbackData*)(cur))->m_this->Eof()); }
static int vtabFilter(sqlite3_vtab_cursor *pVtabCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv){ return (int)(((DbModule::VirtualTable::Cursor::CallbackData*)(pVtabCursor))->m_this->Filter(idxNum, idxStr, argc, (DbValue*)argv)); }
static int vtabBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *pIdxInfo ){ return ((DbModule::VirtualTable::CallbackData*)(pVTab))->m_this->BestIndex((DbModule::VirtualTable::IndexInfo&)(*pIdxInfo)); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult DbModule::Register() {
    static sqlite3_module s_module = {
        0,                 /* iVersion */
        0,
        vtabConnect,
        vtabBestIndex,
        vtabDisconnect,
        0,
        vtabOpen,           /* xOpen - open a cursor */
        vtabClose,          /* xClose - close a cursor */
        vtabFilter,         /* xFilter - configure scan constraints */
        vtabNext,           /* xNext - advance a cursor */
        vtabEof,            /* xEof - check for end of scan */
        vtabColumn,         /* xColumn - read data */
        vtabRowId,          /* xRowid - read data */
        0,                  /* xUpdate */
        0,                  /* xBegin */
        0,                  /* xSync */
        0,                  /* xCommit */
        0,                  /* xRollback */
        0,                  /* xFindMethod */
        0,                  /* xRename */
    };
    return (DbResult)sqlite3_create_module_v2(m_db.GetSqlDb(), m_name.c_str(), &s_module, this, vtabDestroy);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbModule::VirtualTable::IndexInfo::SetIdxStr(const char* idxStr, bool makeCopy) {
    auto info = (sqlite3_index_info*)this;
    if (makeCopy) {
        auto len = (int)(strlen(idxStr) + 1);
        auto copyStr = (char*)sqlite3_malloc(len );
        BeStringUtilities::Strncpy(copyStr, len, idxStr, len);
        info->idxStr = copyStr;
        info->needToFreeIdxStr = 1;
    } else {
        info->idxStr = const_cast<char*>(idxStr);
        info->needToFreeIdxStr = 0;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int DbModule::VirtualTable::IndexInfo::ConstraintUsage::GetArgvIndex() const { return ((sqlite3_index_info::sqlite3_index_constraint_usage const*)this)->argvIndex;}
void DbModule::VirtualTable::IndexInfo::ConstraintUsage::SetArgvIndex(int argvIndex) { ((sqlite3_index_info::sqlite3_index_constraint_usage*)this)->argvIndex = argvIndex; }
bool DbModule::VirtualTable::IndexInfo::ConstraintUsage::GetOmit() const {return (bool)(((sqlite3_index_info::sqlite3_index_constraint_usage const*)this)->omit);}
void DbModule::VirtualTable::IndexInfo::ConstraintUsage::SetOmit(bool omit){ ((sqlite3_index_info::sqlite3_index_constraint_usage*)this)->omit = omit ? 1: 0; }
int DbModule::VirtualTable::IndexInfo::IndexOrderBy::GetColumn() const { return ((sqlite3_index_info::sqlite3_index_orderby const*)this)->iColumn; }
bool DbModule::VirtualTable::IndexInfo::IndexOrderBy::GetDesc() const{ return (bool)(((sqlite3_index_info::sqlite3_index_orderby const*)this)->desc); }
int DbModule::VirtualTable::IndexInfo::IndexConstraint::GetColumn() const { return ((sqlite3_index_info::sqlite3_index_constraint const *)this)->iColumn; }
DbModule::VirtualTable::IndexInfo::Operator DbModule::VirtualTable::IndexInfo::IndexConstraint::GetOp() const { return (Operator)(((sqlite3_index_info::sqlite3_index_constraint const*)this)->op); }
bool DbModule::VirtualTable::IndexInfo::IndexConstraint::IsUsable() const { return (bool)(((sqlite3_index_info::sqlite3_index_constraint const *)this)->usable); }
int DbModule::VirtualTable::IndexInfo::GetConstraintCount() const { return ((sqlite3_index_info const*)this)->nConstraint; }
const DbModule::VirtualTable::IndexInfo::IndexConstraint* DbModule::VirtualTable::IndexInfo::GetConstraint(int i)  const { return ( const IndexConstraint*)&(((sqlite3_index_info const*)this)->aConstraint[i]); }
int DbModule::VirtualTable::IndexInfo::GetIndexOrderByCount() const{ return ((sqlite3_index_info const*)this)->nOrderBy; }
const DbModule::VirtualTable::IndexInfo::IndexOrderBy* DbModule::VirtualTable::IndexInfo::GetOrderBy(int i)  const { return (IndexOrderBy const*)&(((sqlite3_index_info const*)this)->aOrderBy[i]); }
int DbModule::VirtualTable::IndexInfo::GetConstraintUsageCount() const { return GetConstraintCount(); }
DbModule::VirtualTable::IndexInfo::ConstraintUsage* DbModule::VirtualTable::IndexInfo::GetConstraintUsage(int i)  { return (ConstraintUsage*)&(((sqlite3_index_info*)this)->aConstraintUsage[i]); }
void DbModule::VirtualTable::IndexInfo::SetIdxNum(int idxNum) { ((sqlite3_index_info*)this)->idxNum = idxNum; }
int DbModule::VirtualTable::IndexInfo::GetIdxNum() const{ return ((sqlite3_index_info const *)this)->idxNum; }
const char* DbModule::VirtualTable::IndexInfo::GetIdStr() const { return ((sqlite3_index_info const *)this)->idxStr; }
bool DbModule::VirtualTable::IndexInfo::GetOrderByConsumed() const { return ((sqlite3_index_info const *)this)->orderByConsumed; }
void DbModule::VirtualTable::IndexInfo::SetOrderByConsumed(bool orderByConsumed) { ((sqlite3_index_info*)this)->orderByConsumed = orderByConsumed ? 1 : 0; }
double DbModule::VirtualTable::IndexInfo::GetEstimatedCost() const{ return ((sqlite3_index_info const *)this)->estimatedCost; }
void DbModule::VirtualTable::IndexInfo::SetEstimatedCost(double estimatedCost) { ((sqlite3_index_info*)this)->estimatedCost = estimatedCost; }
int64_t DbModule::VirtualTable::IndexInfo::GetEstimatedRows() const{ return ((sqlite3_index_info const *)this)->estimatedRows; }
void DbModule::VirtualTable::IndexInfo::SetEstimatedRows(int64_t estimatedRows) { ((sqlite3_index_info*)this)->estimatedRows = estimatedRows; }
DbModule::VirtualTable::IndexInfo::ScanFlags DbModule::VirtualTable::IndexInfo::GetIdxFlags() const { return (ScanFlags)(((sqlite3_index_info const*)this)->idxFlags); }
void DbModule::VirtualTable::IndexInfo::SetIdxFlags(ScanFlags idxFlags){ ((sqlite3_index_info*)this)->idxFlags =  (int)idxFlags; }
int64_t DbModule::VirtualTable::IndexInfo::GetColUsed() const { return ((sqlite3_index_info const *)this)->colUsed; }
void DbModule::VirtualTable::IndexInfo::SetColUsed(int64_t colUsed) { ((sqlite3_index_info*)this)->colUsed =  colUsed; }
bool DbModule::VirtualTable::IndexInfo::IsDistinct() const { return (bool)sqlite3_vtab_distinct((sqlite3_index_info*)const_cast<IndexInfo*>(this)); }

END_BENTLEY_SQLITE_NAMESPACE
