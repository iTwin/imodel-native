/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMapValidator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "DbMap.h"
#include "RelationshipClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Affan.Khan     07/2017
//+===============+===============+===============+===============+===============+======
struct DbMapValidator final : NonCopyableClass
    {
    enum class Mode
        {
        InMemory,
        All
        };

    private:
        DbMap const& m_dbMap;
        SchemaImportContext& m_schemaImportContext;
        Mode m_mode = Mode::All;

        BentleyStatus ValidateDbSchema() const;
        BentleyStatus ValidateDbTable(DbTable const&) const;
        BentleyStatus ValidateDbColumn(DbColumn const&, bset<Utf8String, CompareIUtf8Ascii> const& physicalColumns) const;
        BentleyStatus ValidateDbConstraint(DbConstraint const&) const;
        BentleyStatus ValidateForeignKeyDbConstraint(ForeignKeyDbConstraint const&) const;
        BentleyStatus ValidatePrimaryKeyDbConstraint(PrimaryKeyDbConstraint const&) const;
        BentleyStatus ValidateDbIndex(DbIndex const&) const;
        BentleyStatus ValidateDbTrigger(DbTrigger const&) const { return SUCCESS; }

        BentleyStatus ValidateDbMap() const;
        BentleyStatus ValidateClassMap(ClassMap const&) const;
        BentleyStatus ValidateRelationshipClassMap(RelationshipClassMap const&) const;
        BentleyStatus ValidateRelationshipClassEndTableMap(RelationshipClassEndTableMap const&) const;
        BentleyStatus ValidateRelationshipClassLinkTableMap(RelationshipClassLinkTableMap const&) const;
        BentleyStatus ValidatePropertyMap(PropertyMap const&, bmap<DbColumn const*, SingleColumnDataPropertyMap const*>& duplicateColumnMappings) const;

        ECDbCR GetECDb() const { return m_dbMap.GetECDb(); }
        DbSchema const& GetDbSchema() const { return m_dbMap.GetDbSchema(); }
        IssueReporter const& Issues() const { return m_dbMap.Issues(); }

    public:
        DbMapValidator(DbMap const& dbMap, SchemaImportContext& ctx, Mode validationMode = Mode::All) : m_dbMap(dbMap), m_schemaImportContext(ctx), m_mode(validationMode) {}
        ~DbMapValidator() {}

        BentleyStatus Validate() const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
