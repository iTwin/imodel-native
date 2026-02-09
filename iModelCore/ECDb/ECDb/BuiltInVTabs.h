/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/VirtualTab.h>
#include <ECDb/ECDb.h>

#include <set>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Virtual Table to filter rows by property
// @bsiclass
//=======================================================================================
struct ClassPropsModule : BeSQLite::DbModule {
    constexpr static auto NAME = "class_props";
    struct ClassPropsVirtualTable : DbVirtualTable {
        struct ClassPropsCursor : DbCursor {
            enum class Columns {
                ClassId = 0,
                Text = 1,
            };

           private:
            Utf8String m_props;
            std::set<ECN::ECClassId> m_classIds;
            std::set<ECN::ECClassId>::const_iterator m_it;

           public:
            ClassPropsCursor(ClassPropsVirtualTable& vt) : DbCursor(vt), m_it(m_classIds.begin()) {}
            bool Eof() final { return m_it == m_classIds.end(); }
            DbResult Next() final;
            DbResult GetColumn(int i, Context& ctx) final;
            DbResult GetRowId(int64_t& rowId) final;
            DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final;
        };

       public:
        ClassPropsVirtualTable(ClassPropsModule& module) : DbVirtualTable(module) {}
        DbResult Open(DbCursor*& cur) override {
            cur = new ClassPropsCursor(*this);
            return BE_SQLITE_OK;
        }
        DbResult BestIndex(IndexInfo& indexInfo) final;
    };

   public:
    ClassPropsModule(DbR db) : DbModule(db, NAME, "CREATE TABLE x(class_id,prop_json_array hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final;
};

struct IdSetModule : ECDbModule {
    constexpr static auto NAME = "IdSet";
    struct IdSetTable : ECDbVirtualTable {
        struct IdSetCursor : ECDbCursor {
            enum class Columns {
                Id = 0,
                Json_array_ids = 1,
            };

           private:
            Utf8String m_text;
            bset<uint64_t> m_idSet;
            bset<uint64_t>::const_iterator m_index;

           public:
            IdSetCursor(IdSetTable& vt) : ECDbCursor(vt), m_index(m_idSet.begin()) {}
            bool Eof() final { return m_index == m_idSet.end(); }
            DbResult Next() final;
            DbResult GetColumn(int i, Context& ctx) final;
            DbResult GetRowId(int64_t& rowId) final;
            DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final;
            DbResult FilterJSONBasedOnType(BeJsConst& val);
            DbResult FilterJSONStringIntoArray(BeJsDocument& val);
            void Reset();
        };

       public:
        IdSetTable(IdSetModule& module) : ECDbVirtualTable(module) {}
        DbResult Open(DbCursor*& cur) override {
            cur = new IdSetCursor(*this);
            return BE_SQLITE_OK;
        }
        DbResult BestIndex(IndexInfo& indexInfo) final;
    };

   public:
    IdSetModule(ECDbR db) : ECDbModule(
                                db,
                                NAME,
                                "CREATE TABLE x(id, json_array_ids hidden)",
                                R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema
                    schemaName="ECVLib"
                    alias="ECVLib"
                    version="1.0.0"
                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
                <ECCustomAttributes>
                    <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
                </ECCustomAttributes>
                <ECEntityClass typeName="IdSet" modifier="Abstract">
                    <ECCustomAttributes>
                        <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="id"  typeName="long" extendedTypeName="Id"/>
                </ECEntityClass>
            </ECSchema>)xml") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final;
};

DbResult RegisterBuildInVTabs(ECDbR);

END_BENTLEY_SQLITE_EC_NAMESPACE