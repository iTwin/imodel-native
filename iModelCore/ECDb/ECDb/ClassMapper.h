/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "PropertyMap.h"
#include "ClassMappingInfo.h"
#include "IssueReporter.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
 
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ClassMapper final
    {
    public:
        //======================================================================================
        // @bsiclass                                              Krischan.Eberle        12/2016
        //======================================================================================
        struct TableMapper final : NonCopyableClass
            {
            private:
                TableMapper();
                ~TableMapper();

                static DbTable* FindOrCreateTable(ClassMap const&, ClassMappingInfo const&, DbTable::Type, DbTable const* primaryTable);
                static DbTable* CreateTableForExistingTableStrategy(ClassMap const&, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId);
                static DbTable* CreateTableForOtherStrategies(ClassMap const&, Utf8StringCR tableName, DbTable::Type, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable);

                static BentleyStatus CreateClassIdColumn(DbSchema&, DbTable&, PersistenceType);
                
                static bool IsExclusiveRootClassOfTable(ClassMappingInfo const&);
                static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);

            public:
                static BentleyStatus MapToTable(ClassMap&, ClassMappingInfo const&);
                static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
            };

        enum class PropertyMapInheritanceMode
            {
            NotInherited, //!< indicates that base property map is not inherited, but created from scratch
            Clone //! inherited property maps areGet cloned from the base class property map
            };


    private:
        ClassMap& m_classMap;
        DbClassMapLoadContext const* m_loadContext;

        explicit ClassMapper(ClassMap& classMap) : m_classMap(classMap), m_loadContext(nullptr) {}
        ClassMapper(ClassMap& classMap, DbClassMapLoadContext const& loadContext) : m_classMap(classMap), m_loadContext(&loadContext) {}

        PropertyMap* ProcessProperty(ECN::ECPropertyCR);

        RefCountedPtr<DataPropertyMap> MapPrimitiveProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* compoundPropMap);
        RefCountedPtr<Point2dPropertyMap> MapPoint2dProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
        RefCountedPtr<Point3dPropertyMap> MapPoint3dProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
        RefCountedPtr<PrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(ECN::PrimitiveArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<StructPropertyMap> MapStructProperty(ECN::StructECPropertyCR, StructPropertyMap const* parentPropMap);
        RefCountedPtr<StructArrayPropertyMap> MapStructArrayProperty(ECN::StructArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<NavigationPropertyMap> MapNavigationProperty(ECN::NavigationECPropertyCR);
        Utf8String ComputeAccessString(ECN::ECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR, RelationshipClassMapCR, NavigationPropertyMap::NavigationEnd);

        static BentleyStatus DetermineColumnInfoForPrimitiveProperty(DbColumn::CreateParams&, ClassMap const&, ECN::PrimitiveECPropertyCR, Utf8StringCR accessString);

    public:
        static PropertyMap* MapProperty(ClassMap&, ECN::ECPropertyCR);
        static PropertyMap* LoadPropertyMap(ClassMap&, ECN::ECPropertyCR, DbClassMapLoadContext const&);
        static BentleyStatus SetupNavigationPropertyMap(NavigationPropertyMap&);

        //! Rules:
        //! If MapStrategy != TPH: NotInherited
        //! Else: Clone
        static PropertyMapInheritanceMode GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const&);
    };


//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct RelationshipClassEndTableMappingContext : NonCopyableClass
    {
    private:
        //=======================================================================================
        // @bsiclass                                                   Affan.Khan          07/16
        //+===============+===============+===============+===============+===============+======
        struct PartitionInfo
            {
            enum class ColumnId
                {
                ECInstanceId = 0,
                ECClassId = 1,
                SourceECInstanceId = 2,
                SourceEClassId = 3,
                TargetECInstanceId = 4,
                TargetECClassId = 5
                };

            private:
                const DbColumn* m_cols[6];
                bool m_isPersisted;

            public:
                PartitionInfo() :m_isPersisted(false)
                    {
                    Clear();
                    }
                PartitionInfo(RelationshipClassEndTableMap::Partition const& partition);
                DbColumn const* Get(ColumnId id) const { return m_cols[Enum::ToInt(id)]; }
                void Set(ColumnId id, DbColumn const* column);
                void Clear();
                bool IsValid() const;
                bool IsPersisted() const { return m_isPersisted; }
                static ColumnId ConstraintECInstanceId(ECN::ECRelationshipEnd end)  { return end == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnId::SourceECInstanceId : ColumnId::TargetECInstanceId; }
                static ColumnId ConstraintECClassId(ECN::ECRelationshipEnd end) { return end == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnId::SourceEClassId : ColumnId::TargetECClassId; }
            };

        struct ForeignKeyColumnInfo final
            {
            private:
                Utf8String m_fkColName;
                Utf8String m_relClassIdColName;

                explicit ForeignKeyColumnInfo(Utf8StringCR fkColName) : m_fkColName(fkColName) {}

                static Utf8String DetermineRelClassIdColumnName(Utf8StringCR fkColName);

            public:
                ForeignKeyColumnInfo() {}
                static ForeignKeyColumnInfo FromNavigationProperty(ECN::NavigationECPropertyCR);

                Utf8StringCR GetFkColumnName() const { return m_fkColName; }
                Utf8StringCR GetRelClassIdColumnName() const { return m_relClassIdColName; }
            };

    private:
        static Utf8CP RELECCLASSID_COLNAME_TOKEN;
        RelationshipMappingInfo const& m_relInfo;
        RelationshipClassEndTableMap const& m_relationshipMap;
        std::map<DbTableId, std::vector<PartitionInfo>> m_partitions;
        SchemaImportContext& m_schemaContext;

        const std::vector<DbColumn const*> GetPartitionColumns(PartitionInfo::ColumnId) const;
        bool PersistedEndHasNonVirtualForeignKeyColumn() const;
        bool TryGetPartition(ClassMapCR, std::vector<PartitionInfo*>&);
        PartitionInfo* CreatePartition(DbTableId);
        BentleyStatus AddIndexToRelationshipEnd(PartitionInfo const&);
        BentleyStatus ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol);
        DbColumn* CreateForeignKeyColumn(DbTable& fkTable, NavigationPropertyMap const&, ForeignKeyColumnInfo&);
        ClassMappingStatus CreateForeignKeyConstraint(DbTable const& referencedTable);
        DbColumn* CreateRelECClassIdColumn(DbMap const&, ECN::ECClassCR, DbTable& fkTable, ForeignKeyColumnInfo const&, DbColumn const& fkCol, NavigationPropertyMap const&);
        ECN::ECRelationshipEnd GetForeignEnd() const;
        ECN::ECRelationshipEnd GetReferencedEnd() const;
        IssueReporter const& Issues() const;
        ECDbCR GetECDb() const;

        RelationshipClassEndTableMappingContext(SchemaImportContext&, RelationshipClassEndTableMap const&, RelationshipMappingInfo const&);

    public:
        static std::unique_ptr<RelationshipClassEndTableMappingContext> Create(SchemaImportContext&, RelationshipClassEndTableMap const&, RelationshipMappingInfo const&);

        ClassMappingStatus UpdatePersistedEnd(NavigationPropertyMap&);
        ClassMappingStatus FinishMapping();
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct EndTableMappingContextCollection
    {
    private:
        std::vector <std::unique_ptr<RelationshipClassEndTableMappingContext>> m_contextList;
        std::map<ECN::ECClassId, RelationshipClassEndTableMappingContext*> m_contentMap;
        SchemaImportContext& m_schemaImportContext;
        static ECN::ECClassCP GetRoot(ECN::ECClassCR ecClass);

    public:
        EndTableMappingContextCollection(SchemaImportContext& ctx);
        void RegisterContext(RelationshipClassEndTableMap const& relationshipMap, RelationshipMappingInfo const& relinfo);
        ClassMappingStatus FinishMapping();
        ClassMappingStatus Map(NavigationPropertyMap& navPropMap);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE