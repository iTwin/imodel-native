/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <BeSQLite/VirtualTab.h>
#include <set>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Virtual Table to filter rows by property
// @bsiclass
//=======================================================================================
struct ClassPropsModule : BeSQLite::DbModule {
    struct ClassPropsVirtualTable : VirtualTable {
        struct ClassPropsCursor : Cursor {
            enum class Columns{
                ClassId = 0,
                Text = 1,
            };
            private:
                Utf8String m_props;
                std::set<ECN::ECClassId> m_classIds;
                std::set<ECN::ECClassId>::const_iterator m_it;

            public:
                ClassPropsCursor(ClassPropsVirtualTable& vt): Cursor(vt), m_it(m_classIds.begin()){}
                bool Eof() final { return m_it == m_classIds.end(); }
                DbResult Next() final;
                DbResult GetColumn(int i, Context& ctx) final;
                DbResult GetRowId(int64_t& rowId) final;
                DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final;
        };
        public:
            ClassPropsVirtualTable(ClassPropsModule& module): VirtualTable(module) {}
            DbResult Open(Cursor*& cur) override { cur = new ClassPropsCursor(*this); return BE_SQLITE_OK; }
            DbResult BestIndex(IndexInfo& indexInfo) final;
    };
    public:
        ClassPropsModule(DbR db): DbModule(db, "contain_props", "CREATE TABLE x(class_id,prop_json_array hidden)") {}
        DbResult Connect(VirtualTable*& out, Config& conf, int argc, const char* const* argv) final;
};

DbResult RegisterBuildInVTabs(ECDbR);

END_BENTLEY_SQLITE_EC_NAMESPACE