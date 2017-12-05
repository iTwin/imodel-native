/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMappingManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "RelationshipClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct SchemaImportContext;


//=======================================================================================
// @bsiclass                                                Affan.Khan      07/2017
//+===============+===============+===============+===============+===============+======
struct FkRelationshipMappingInfo final
    {
    public:
        struct Collection final
            {
            private:
                std::map<ECN::ECClassId, std::unique_ptr<FkRelationshipMappingInfo>> m_fkRelMappingInfos;
                std::vector<FkRelationshipMappingInfo const*> m_orderedList;

            public:
                Collection() {}
                std::vector<FkRelationshipMappingInfo const*> const& Get() const { return m_orderedList; }
                bool TryGet(FkRelationshipMappingInfo*& fkRelMappingInfo, ECN::ECClassId relClassId) const 
                    {
                    auto it = m_fkRelMappingInfos.find(relClassId);
                    if (it != m_fkRelMappingInfos.end())
                        {
                        fkRelMappingInfo = it->second.get();
                        return true;
                        }

                    fkRelMappingInfo = nullptr;
                    return false;
                    }

                FkRelationshipMappingInfo& Add(ECDbCR ecdb, ECN::ECRelationshipClassCR relClass, MapStrategy mapStrategy)
                    {
                    BeAssert(m_fkRelMappingInfos.find(relClass.GetId()) == m_fkRelMappingInfos.end());
                    std::unique_ptr<FkRelationshipMappingInfo> newFkRelMappingInfo = std::make_unique<FkRelationshipMappingInfo>(ecdb, relClass, mapStrategy);
                    FkRelationshipMappingInfo& fkRelMappingInfoR = *newFkRelMappingInfo;
                    m_fkRelMappingInfos[relClass.GetId()] = std::move(newFkRelMappingInfo);
                    m_orderedList.push_back(&fkRelMappingInfoR);
                    return fkRelMappingInfoR;
                    }
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

        ECN::ECRelationshipClassCR m_relClass;
        std::unique_ptr<ForeignKeyPartitionView> m_fkPartitionView;
        bool m_fkConstraintCAIsRead = false;
        ForeignKeyConstraintCustomAttribute m_fkConstraintCA;

        //not copyable
        FkRelationshipMappingInfo(FkRelationshipMappingInfo const&) = delete;
        FkRelationshipMappingInfo& operator=(FkRelationshipMappingInfo const&) = delete;

    public:
        FkRelationshipMappingInfo(ECDbCR, ECN::ECRelationshipClassCR, MapStrategy);

        ECN::ECRelationshipClassCR GetRelationshipClass() const { return m_relClass; }
        void ReadFkConstraintCA(SchemaImportContext&, ECN::NavigationECPropertyCR);
        bool IsForeignKeyConstraintCustomAttributeRead() const { return m_fkConstraintCAIsRead; }
        bool IsPhysicalForeignKey() const { return m_fkConstraintCA.IsValid(); }
        ForeignKeyConstraintCustomAttribute const& GetFkConstraintCA() const { return m_fkConstraintCA; }
        ECN::ECRelationshipEnd GetForeignKeyEnd() const;
        ECN::ECRelationshipEnd GetReferencedEnd() const;
        ForeignKeyPartitionView const& GetPartitionView() const { return *m_fkPartitionView; }
        ForeignKeyPartitionView& GetPartitionViewR() const { return *m_fkPartitionView; }
    };

//======================================================================================
// @bsiclass                                              Krischan.Eberle        07/2017
//======================================================================================
struct DbMappingManager final
    {
    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2016
    //+===============+===============+===============+===============+===============+======
    struct Classes final
        {
        public:
            enum class PropertyMapInheritanceMode
                {
                NotInherited, //!< indicates that base property map is not inherited, but created from scratch
                Clone //! inherited property maps areGet cloned from the base class property map
                };

        private:
            struct Context final
                {
                enum class Mode
                    {
                    ImportingSchema,
                    Loading
                    };

                SchemaImportContext* m_importCtx = nullptr;
                DbClassMapLoadContext const* m_loadCtx = nullptr;
                ClassMap& m_classMap;

                Context(SchemaImportContext& importCtx, ClassMap& classMap) : m_importCtx(&importCtx), m_classMap(classMap) {}
                Context(DbClassMapLoadContext const& loadCtx, ClassMap& classMap) : m_loadCtx(&loadCtx), m_classMap(classMap) {}
                //not copyable
                Context(Context const&) = delete;
                Context& operator=(Context const&) = delete;

                Mode GetMode() const { return m_importCtx != nullptr ? Mode::ImportingSchema : Mode::Loading; }
                };

            Classes() = delete;
            ~Classes() = delete;

            static PropertyMap* ProcessProperty(Context&, ECN::ECPropertyCR);
            static RefCountedPtr<DataPropertyMap> MapPrimitiveProperty(Context&, ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* compoundPropMap);
            static RefCountedPtr<Point2dPropertyMap> MapPoint2dProperty(Context&, ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
            static RefCountedPtr<Point3dPropertyMap> MapPoint3dProperty(Context&, ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
            static RefCountedPtr<PrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(Context&, ECN::PrimitiveArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
            static RefCountedPtr<StructPropertyMap> MapStructProperty(Context&, ECN::StructECPropertyCR, StructPropertyMap const* parentPropMap);
            static RefCountedPtr<StructArrayPropertyMap> MapStructArrayProperty(Context&, ECN::StructArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
            static RefCountedPtr<NavigationPropertyMap> MapNavigationProperty(Context&, ECN::NavigationECPropertyCR);
            static Utf8String ComputeAccessString(ECN::ECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
            static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR, RelationshipClassMapCR, NavigationPropertyMap::NavigationEnd);
            static BentleyStatus DetermineColumnInfoForPrimitiveProperty(DbColumn::CreateParams&, ClassMap const&, ECN::PrimitiveECPropertyCR, Utf8StringCR accessString);

            static BentleyStatus MapUserDefinedIndex(SchemaImportContext&, ClassMap const&, DbIndexListCustomAttribute::DbIndex const&);

        public:
            static PropertyMap* MapProperty(SchemaImportContext& importCtx, ClassMap& classMap, ECN::ECPropertyCR prop) { Context ctx(importCtx, classMap); return ProcessProperty(ctx, prop); }
            //WIP_CLEANUP This should be moved out of here. It seems like it doesn't have to do with mapping. And if it has, it needs to be renamed/refactored
            static PropertyMap* LoadPropertyMap(DbClassMapLoadContext const& loadCtx, ClassMap& classMap, ECN::ECPropertyCR prop) { Context ctx(loadCtx, classMap);  return ProcessProperty(ctx, prop); }
            //WIP_CLEANUP Moving the below MapNavigationProperty from elsewhere here makes evident that we now have 2 equally named methods
            //for mapping nav props, plus a third method related to nav props. This needs to be cleaned up.
            static ClassMappingStatus MapNavigationProperty(SchemaImportContext&, NavigationPropertyMap&);

            static BentleyStatus MapUserDefinedIndexes(SchemaImportContext&, ClassMap const&);

            static BentleyStatus TryDetermineRelationshipMappingType(RelationshipMappingType&, SchemaImportContext const&, ECN::ECRelationshipClassCR);

            //! Rules:
            //! If MapStrategy != TPH: NotInherited
            //! Else: Clone
            static PropertyMapInheritanceMode GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const&);
        };

    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2017
    //+===============+===============+===============+===============+===============+======
    struct FkRelationships final
        {
        private:
            FkRelationships() = delete;
            ~FkRelationships() = delete;

            static BentleyStatus AddIndexToRelationshipEnd(SchemaImportContext&, FkRelationshipMappingInfo const&, ForeignKeyPartitionView::Partition const&);
            static DbColumn* CreateForeignKeyColumn(FkRelationshipMappingInfo::ForeignKeyColumnInfo&, SchemaImportContext&, FkRelationshipMappingInfo const&, DbTable& fkTable, NavigationPropertyMap const&);
            static BentleyStatus CreateForeignKeyConstraint(SchemaImportContext&, FkRelationshipMappingInfo const&, DbTable const& referencedTable);
            static DbColumn* CreateRelECClassIdColumn(SchemaImportContext&, FkRelationshipMappingInfo const&, FkRelationshipMappingInfo::ForeignKeyColumnInfo const&, DbTable& fkTable, DbColumn const& fkCol, NavigationPropertyMap const&);
            static BentleyStatus FinishMapping(SchemaImportContext&, FkRelationshipMappingInfo const&);
            static BentleyStatus ValidateForeignKeyColumn(SchemaImportContext&, FkRelationshipMappingInfo const&, DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol);

        public:
            static BentleyStatus UpdatePersistedEnd(SchemaImportContext&, FkRelationshipMappingInfo&, NavigationPropertyMap&);

            static BentleyStatus FinishMapping(SchemaImportContext&);

            static BentleyStatus TryDetermineFkEnd(ECN::ECRelationshipEnd&, ECN::ECRelationshipClassCR, IssueReporter const&);
        };

    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      01/2016
    //+===============+===============+===============+===============+===============+======
    struct Tables final
        {
        private:
            Tables() = delete;
            ~Tables() = delete;

            static DbTable* FindOrCreateTable(SchemaImportContext&, ClassMap const&, ClassMappingInfo const&, DbTable::Type, DbTable const* primaryTable);
            static DbTable* CreateTableForExistingTableStrategy(SchemaImportContext&, ClassMap const&, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId);
            static DbTable* CreateTableForOtherStrategies(SchemaImportContext&, ClassMap const&, Utf8StringCR tableName, DbTable::Type, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable);
            static BentleyStatus CreateClassIdColumn(SchemaImportContext&, DbTable&, PersistenceType);
            static bool IsExclusiveRootClassOfTable(ClassMappingInfo const&);
            static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);

        public:
            static BentleyStatus MapToTable(SchemaImportContext&, ClassMap&, ClassMappingInfo const&);
            static BentleyStatus CreateVirtualTableForFkRelationship(SchemaImportContext&, RelationshipClassEndTableMap&, ClassMappingInfo const&);
            static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
            static DbTable* CreateOverflowTable(SchemaImportContext&, DbTable const&);
            static BentleyStatus CreateIndex(SchemaImportContext&, DbTable&, Utf8StringCR indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);
        };

private:
    DbMappingManager() = delete;
    ~DbMappingManager() = delete;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE