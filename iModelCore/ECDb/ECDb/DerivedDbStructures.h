/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct MainSchemaManager;

//======================================================================================
//! The physical structures the ec_ tables do not describe: foreign key constraints and
//! triggers. Everything else the DDL generator emits comes straight from ec_Column or
//! ec_Index; these two only ever existed as text inside a CREATE TABLE / CREATE TRIGGER
//! statement, so a file that learns a schema by copying ec_ rows cannot recover them.
//!
//! Works them out again from persisted state and puts them on the in-memory DbTable.
//! Must run before tables are created - SQLite cannot add a constraint to an existing
//! table, so the CREATE TABLE has to come out right the first time. On tables that
//! already exist it does nothing, which is correct: they got their structures when they
//! were created.
//!
//! Idempotent, and deterministic from the file's own contents alone. That is a hard
//! requirement rather than a nicety: a briefcase can adopt ec_ rows that carry no delta
//! and nothing recorded anywhere to replay, and still has to end up with the same tables
//! as the briefcase that mapped them.
// @bsiclass
//======================================================================================
struct DerivedDbStructures final
    {
    private:
        DerivedDbStructures() = delete;
        ~DerivedDbStructures() = delete;

        static BentleyStatus AddChildTableForeignKeys(MainSchemaManager const&);
        static BentleyStatus AddRelationshipForeignKeys(MainSchemaManager const&);
        static BentleyStatus AddLinkTableForeignKeys(MainSchemaManager const&, RelationshipClassMap const&);
        static BentleyStatus AddNavigationPropertyForeignKeys(MainSchemaManager const&, ECN::ECRelationshipClassCR, MapStrategy);
        static BentleyStatus AddCurrentTimeStampTriggers(MainSchemaManager const&);

        static BentleyStatus EnsureForeignKey(DbTable&, DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDelete, ForeignKeyDbConstraint::ActionType onUpdate);
        static std::set<DbTable const*> GetConstraintPrimaryTables(MainSchemaManager const&, ECN::ECRelationshipConstraintCR);
        static ECN::NavigationECPropertyCP FindNavigationProperty(MainSchemaManager const&, ECN::ECRelationshipConstraintCR foreignEndConstraint, ECN::ECRelationshipClassCR);

    public:
        //! @param[in] manager schema manager whose in-memory DbTables get the structures
        //! @pre The EC cache tables reflect the persisted metadata. Relationship derivation reads
        //!      ec_cache_ClassHasTables, so callers must refresh the caches after their last metadata change.
        static BentleyStatus Derive(MainSchemaManager const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
