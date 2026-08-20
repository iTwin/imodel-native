/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/InstanceGraph.h>
#include <ECDb/ECDbVirtualTab.h>
#include "InstanceGraphImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Virtual table module exposing relationship traversal as ECVLib.Relations()
// @bsiclass
//=======================================================================================
struct RelationsModule : ECDbModule
    {
    constexpr static auto NAME = "relations";

    struct RelationsTable : ECDbVirtualTable
        {
        struct RelationsCursor : ECDbCursor
            {
            enum class Columns
                {
                RelatedECInstanceId = 0,
                RelatedECClassId = 1,
                Direction = 2,
                RelationshipECClassId = 3,
                RelationshipECInstanceId = 4,
                // Hidden input columns
                ECInstanceId = 5,
                ECClassId = 6,
                TraversalDir = 7
                };

            private:
                GraphTraversalIterator m_iter;
                int64_t m_rowId = 0;
                bool m_eof = true;

                // The hidden argument columns must return the values they were called with:
                // IndexInfo::SetOmit is only a hint, so SQLite may still evaluate the constraint
                // and returning NULL would then silently produce zero rows.
                ECInstanceId m_seedInstanceId;
                ECN::ECClassId m_seedClassId;
                TraversalDirection m_dir = TraversalDirection::Both;

            public:
                RelationsCursor(RelationsTable& vt);
                bool Eof() final { return m_eof; }
                DbResult Next() final;
                DbResult GetColumn(int i, Context& ctx) final;
                DbResult GetRowId(int64_t& rowId) final;
                DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final;
            };

        public:
            RelationsTable(RelationsModule& module) : ECDbVirtualTable(module) {}
            DbResult Open(DbCursor*& cur) override { cur = new RelationsCursor(*this); return BE_SQLITE_OK; }
            DbResult BestIndex(IndexInfo& indexInfo) final;
        };

    public:
        RelationsModule(ECDbR db) : ECDbModule(
            db,
            NAME,
            "CREATE TABLE x(RelatedECInstanceId, RelatedECClassId, Direction, RelationshipECClassId, RelationshipECInstanceId,"
            " ECInstanceId HIDDEN, ECClassId HIDDEN, TraversalDirection HIDDEN)",
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
                <ECEntityClass typeName="Relations" modifier="Abstract">
                    <ECCustomAttributes>
                        <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="RelatedECInstanceId"  typeName="long" extendedTypeName="Id"/>
                    <ECProperty propertyName="RelatedECClassId"     typeName="long" extendedTypeName="Id"/>
                    <ECProperty propertyName="Direction"            typeName="string"/>
                    <ECProperty propertyName="RelationshipECClassId" typeName="long" extendedTypeName="Id"/>
                    <ECProperty propertyName="RelationshipECInstanceId" typeName="long" extendedTypeName="Id"/>
                </ECEntityClass>
            </ECSchema>)xml"
            ) {}
        DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
