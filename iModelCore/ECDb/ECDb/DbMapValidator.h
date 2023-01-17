/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "SchemaManagerDispatcher.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DbMapValidator final
    {
    private:
        SchemaImportContext& m_schemaImportContext;

        mutable bmap<DbColumnId, bset<DbIndex const*>> m_indexesByColumnCache;

        //not copyable
        DbMapValidator(DbMapValidator const&) = delete;
        DbMapValidator& operator=(DbMapValidator const&) = delete;

        BentleyStatus Initialize() const;
        BentleyStatus ValidateViews() const;
        BentleyStatus ValidateDbSchema() const;
        BentleyStatus ValidateDbTable(DbTable const&) const;
        BentleyStatus ValidateDbColumn(DbColumn const&, bset<Utf8String, CompareIUtf8Ascii> const& physicalColumns) const;
        BentleyStatus ValidateDbConstraint(DbConstraint const&) const;
        BentleyStatus ValidateForeignKeyDbConstraint(ForeignKeyDbConstraint const&) const;
        BentleyStatus ValidatePrimaryKeyDbConstraint(PrimaryKeyDbConstraint const&) const;
        BentleyStatus ValidateDbIndex(DbIndex const&) const;
        BentleyStatus ValidateDbTrigger(DbTrigger const&) const { return SUCCESS; }
        BentleyStatus ValidateCustomAttributeTable() const;
        BentleyStatus ValidateDbMap() const;
        BentleyStatus ValidateClassMap(ClassMap const&) const;
        BentleyStatus ValidateMapStrategy(ClassMap const&) const;
        BentleyStatus ValidateRelationshipClassEndTableMap(RelationshipClassEndTableMap const&) const;
        BentleyStatus ValidateRelationshipClassLinkTableMap(RelationshipClassLinkTableMap const&) const;
        BentleyStatus ValidatePropertyMap(PropertyMap const&, bmap<DbColumn const*, SingleColumnDataPropertyMap const*>& duplicateColumnMappings) const;
        BentleyStatus ValidateNavigationPropertyMap(NavigationPropertyMap const&) const;
        BentleyStatus ValidateNavigationPropertyMapNotNull(NavigationPropertyMap const&, DbColumn const& idCol, DbColumn const& relClassIdCol, bool isPhysicalFk) const;
        BentleyStatus ValidateNavigationPropertyMapUniqueness(NavigationPropertyMap const&, DbColumn const& idCol, DbColumn const& relClassIdCol, bool isPhysicalFk) const;
        BentleyStatus ValidateOverflowPropertyMaps(ClassMap const& classMap) const;
        BentleyStatus CheckDuplicateDataPropertyMap() const;
        ECDbCR GetECDb() const { return m_schemaImportContext.GetECDb(); }
        MainSchemaManager const& GetSchemaManager() const { return m_schemaImportContext.GetSchemaManager(); }
        DbSchema const& GetDbSchema() const { return GetSchemaManager().GetDbSchema(); }
        IssueDataSource const& Issues() const { return GetSchemaManager().Issues(); }

    public:
        explicit DbMapValidator(SchemaImportContext& ctx) : m_schemaImportContext(ctx) {}
        ~DbMapValidator() {}

        BentleyStatus Validate() const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
