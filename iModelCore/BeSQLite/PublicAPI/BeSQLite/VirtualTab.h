
/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeSQLite.h"
#include <Bentley/BeId.h>

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DbModule : NonCopyableClass {
   public:
    // Eponymous-only virtual tables (they are useful as table-valued functions)
    struct DbVirtualTable : NonCopyableClass {
        struct IndexInfo final {
            enum class Operator {
                EQ = 2,
                GT = 4,
                LE = 8,
                LT = 16,
                GE = 32,
                MATCH = 64,
                LIKE = 65,
                GLOB = 66,
                REGEXP = 67,
                NE = 68,
                ISNOT = 69,
                ISNOTNULL = 70,
                ISNULL = 71,
                IS = 72,
                LIMIT = 73,
                OFFSET = 74,
                FUNCTION = 150,
            };
            enum class ScanFlags {
                NONE = 0,
                UNIQUE = 1,
            };
            struct IndexConstraint final {
                BE_SQLITE_EXPORT int GetColumn() const;  /* Column constrained.  -1 for ROWID */
                BE_SQLITE_EXPORT Operator GetOp() const; /* Constraint operator */
                BE_SQLITE_EXPORT bool IsUsable() const;  /* True if this constraint is usable */
            };
            struct IndexOrderBy final {
                BE_SQLITE_EXPORT int GetColumn() const; /* Column number */
                BE_SQLITE_EXPORT bool GetDesc() const;  /* True for DESC.  False for ASC. */
            };
            struct ConstraintUsage final {
                BE_SQLITE_EXPORT int GetArgvIndex() const;
                BE_SQLITE_EXPORT void SetArgvIndex(int); /* if >0, constraint is part of argv to xFilter */
                BE_SQLITE_EXPORT bool GetOmit() const;
                BE_SQLITE_EXPORT void SetOmit(bool); /* Do not code a test for this constraint */
            };
            BE_SQLITE_EXPORT int GetConstraintCount() const; /* Number of entries in aConstraint */
            BE_SQLITE_EXPORT const IndexConstraint* GetConstraint(int) const;
            BE_SQLITE_EXPORT int GetIndexOrderByCount() const; /* Number of terms in the ORDER BY clause */
            BE_SQLITE_EXPORT const IndexOrderBy* GetOrderBy(int) const;
            /* Outputs */
            BE_SQLITE_EXPORT int GetConstraintUsageCount() const;
            BE_SQLITE_EXPORT ConstraintUsage* GetConstraintUsage(int);
            BE_SQLITE_EXPORT void SetIdxNum(int); /* Number used to identify the index */
            BE_SQLITE_EXPORT int GetIdxNum() const;
            BE_SQLITE_EXPORT void SetIdxStr(const char* idxStr, bool makeCopy = true); /* String, possibly obtained from sqlite3_malloc */
            BE_SQLITE_EXPORT const char* GetIdStr() const;
            BE_SQLITE_EXPORT bool GetOrderByConsumed() const;
            BE_SQLITE_EXPORT void SetOrderByConsumed(bool); /* True if output is already ordered */
            BE_SQLITE_EXPORT double GetEstimatedCost() const;
            BE_SQLITE_EXPORT void SetEstimatedCost(double); /* Estimated cost of using this index */
            BE_SQLITE_EXPORT int64_t GetEstimatedRows() const;
            BE_SQLITE_EXPORT void SetEstimatedRows(int64_t); /* Estimated number of rows returned */
            BE_SQLITE_EXPORT ScanFlags GetIdxFlags() const;
            BE_SQLITE_EXPORT void SetIdxFlags(ScanFlags); /* Mask of SQLITE_INDEX_SCAN_* flags */
            BE_SQLITE_EXPORT int64_t GetColUsed() const;
            BE_SQLITE_EXPORT void SetColUsed(int64_t); /* Input: Mask of columns used by statement */
            BE_SQLITE_EXPORT bool IsDistinct() const;  /* Determine if a virtual table query is DISTINCT sqlite3_vtab_distinct() */
        };

       public:
        struct CallbackData;
        struct DbCursor : NonCopyableClass {
           public:
            struct CallbackData;
            struct Context {
                enum class CopyData : int { No = 0,
                                            Yes = -1 };                                                             //!< see sqlite3_destructor_type
                BE_SQLITE_EXPORT void SetResultBlob(void const* value, int length, CopyData copy = CopyData::Yes);  //!< see sqlite3_result_blob
                BE_SQLITE_EXPORT void SetResultDouble(double);                                                      //!< see sqlite3_result_double
                BE_SQLITE_EXPORT void SetResultError(Utf8CP, int len = -1);                                         //!< see sqlite3_result_error
                BE_SQLITE_EXPORT void SetResultError_toobig();                                                      //!< see sqlite3_result_error_toobig
                BE_SQLITE_EXPORT void SetResultError_nomem();                                                       //!< see sqlite3_result_error_nomem
                BE_SQLITE_EXPORT void SetResultError_code(int);                                                     //!< see sqlite3_result_error_code
                BE_SQLITE_EXPORT void SetResultInt(int);                                                            //!< see sqlite3_result_int
                BE_SQLITE_EXPORT void SetResultInt64(int64_t);                                                      //!< see sqlite3_result_int64
                BE_SQLITE_EXPORT void SetResultNull();                                                              //!< see sqlite3_result_null
                BE_SQLITE_EXPORT void SetResultText(Utf8CP value, int length, CopyData);                            //!< see sqlite3_result_text
                BE_SQLITE_EXPORT void SetResultZeroblob(int length);                                                //!< see sqlite3_result_zeroblob
                BE_SQLITE_EXPORT void SetResultValue(DbValue);                                                      //!< see sqlite3_result_value
                BE_SQLITE_EXPORT void SetResultPointer(void*, const char*, void (*destroy)(void*));                 //!< see sqlite3_result_pointer
                BE_SQLITE_EXPORT void SetResultZeroBlob(int n);                                                     //!< see sqlite3_result_zeroblob
                BE_SQLITE_EXPORT void SetSubType(int type);                                                         //!< see sqlite3_result_subtype
            };

           private:
            CallbackData* m_ctx = nullptr;
            DbVirtualTable& m_table;

           public:
            BE_SQLITE_EXPORT DbCursor(DbVirtualTable& vt);
            BE_SQLITE_EXPORT virtual ~DbCursor();
            //! Internally used
            CallbackData* GetCallbackData() { return m_ctx; }
            //! return VTab
            DbVirtualTable& GetTable() { return m_table; }
            //! Check if EOF
            virtual bool Eof() = 0;
            //! Move to next row
            virtual DbResult Next() = 0;
            //! Get value for a column for current row
            virtual DbResult GetColumn(int i, Context& ctx) = 0;
            //! Get RowId for current row
            virtual DbResult GetRowId(int64_t&) = 0;
            //! Called before closing the cursor
            virtual DbResult Close() { return BE_SQLITE_OK; }
            //~ Filter rows and move cursor to start of first filtered row.
            virtual DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* args) = 0;
        };

       private:
        CallbackData* m_ctx = nullptr;
        DbModule& m_module;

       public:
        BE_SQLITE_EXPORT explicit DbVirtualTable(DbModule& module);
        BE_SQLITE_EXPORT virtual ~DbVirtualTable();
        //! Set error message
        BE_SQLITE_EXPORT void SetError(Utf8CP);
        //! Internally used
        CallbackData* GetCallbackData() { return m_ctx; }
        DbModule& GetModule() { return m_module; }
        //! create and return cursor implementation
        virtual DbResult Open(DbCursor*& cur) = 0;
        //! set index info use by query planner
        virtual DbResult BestIndex(IndexInfo& indexInfo) = 0;
    };
    struct Config {
        enum class Tags {
            ConstraintSupport = 1,  //! use with xUpdate()
            Innocuous = 2,          //! Identify that virtual table as being safe to use from within triggers and views
            DirectOnly = 3,         //! Prohibits that virtual table from being used from within triggers and views
        };

       private:
        Tags m_tag;

       public:
        Config() : m_tag(Tags::Innocuous) {}
        void SetTag(Tags tag) { m_tag = tag; }
        Tags GetTag() const { return m_tag; }
    };

   private:
    DbR m_db;
    Utf8String m_name;
    Utf8String m_declaration;

   protected:
    virtual DbResult _OnRegister() { return BE_SQLITE_OK; }

   public:
    DbModule(DbR db, Utf8CP name, Utf8CP declaration) : m_name(name), m_db(db), m_declaration(declaration) {}
    virtual ~DbModule() {}
    Utf8StringCR GetName() const { return m_name; }
    Utf8StringCR GetDeclaration() const { return m_declaration; }
    DbR GetDb() { return m_db; }
    virtual DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) = 0;

    //! Register and transfer ownership to BeSQLite. It will automatically delete the instance of DbModule
    //! on which register is called.
    BE_SQLITE_EXPORT DbResult Register();
};
END_BENTLEY_SQLITE_NAMESPACE