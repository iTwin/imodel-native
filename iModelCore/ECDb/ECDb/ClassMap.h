/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "PropertyMap.h"
#include "ClassMappingInfo.h"
#include "IssueReporter.h"
#include "ClassMapColumnFactory.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassMapLoadContext
    {
private:
    std::set<ECN::ECClassCP> m_constraintClasses;

    //not copyable
    ClassMapLoadContext(ClassMapLoadContext const&) = delete;
    ClassMapLoadContext& operator=(ClassMapLoadContext const) = delete;

public:
    ClassMapLoadContext() {}

    void AddConstraintClass(ECN::ECClassCR ecClass)
        {
        //LOG.debugv("ClassMapLoadContext> Added ECRelationshipConstraint ECClass '%s' to context %p.", ecClass.GetFullName(), this);
        m_constraintClasses.insert(&ecClass);
        }

    };

struct NativeSqlBuilder;
struct StorageDescription;
struct ECSqlPrepareContext;
struct ECInstanceIdPropertyMap;
struct ECClassIdPropertyMap;
struct DbClassMapLoadContext;
struct DbMapSaveContext;
struct TableSpaceSchemaManager;

enum ObjectState
    {
    Persisted, //loaded from disk
    Modified, // loaded from disk but modified
    New //new object not yet persisted to disk
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassMap
    {
    friend struct ClassMapFactory;

    public:
        enum class Type
            {
            Class,
            RelationshipEndTable,
            RelationshipLinkTable,
            NotMapped
            };


        struct TablePerHierarchyHelper final
            {
        private:
            ClassMap const& m_classMap;
            mutable ECN::ECClassId m_parentOfJoinedTableECClassId;
            TablePerHierarchyInfo const& GetTphInfo() const { return m_classMap.GetMapStrategy().GetTphInfo(); }
        public:
            explicit TablePerHierarchyHelper(ClassMap const& classMap) : m_classMap(classMap) {}

            bool HasJoinedTable() const { return GetTphInfo().IsValid() && GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable; }
            bool IsParentOfJoinedTable() const { return GetTphInfo().IsValid() && GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::ParentOfJoinedTable; }

            ECN::ECClassId DetermineTphRootClassId() const { return m_classMap.GetPrimaryTable().GetExclusiveRootECClassId(); }
            };

    protected:
        struct ClassMappingContext final
            {
            private:
                SchemaImportContext& m_importCtx;
                ClassMappingInfo const& m_classMappingInfo;

                //not copyable
                ClassMappingContext(ClassMappingContext const&) = delete;
                ClassMappingContext& operator=(ClassMappingContext const&) = delete;

            public:
                ClassMappingContext(SchemaImportContext& importCtx, ClassMappingInfo const& classMappingInfo)
                    : m_importCtx(importCtx), m_classMappingInfo(classMappingInfo)
                {}

                SchemaImportContext& GetImportCtx() const { return m_importCtx; }
                ClassMappingInfo const& GetClassMappingInfo() const { return m_classMappingInfo; }
            };

    private:
        Type m_type;
        ECDb const& m_ecdb;
        TableSpaceSchemaManager const& m_schemaManager;

        ECN::ECClassCR m_ecClass;
        const MapStrategyExtendedInfo m_mapStrategyExtInfo;
        PropertyMapContainer m_propertyMaps;
        mutable std::vector<DbTable*> m_tables;
        mutable std::unique_ptr<ClassMapColumnFactory> m_columnFactory;
        std::unique_ptr<TablePerHierarchyHelper> m_tphHelper;
        bvector<ECN::ECPropertyCP> m_failedToLoadProperties;
        ObjectState m_state;
        BentleyStatus CreateCurrentTimeStampTrigger(ECN::PrimitiveECPropertyCR);
        BentleyStatus AddOrUpdateTableList(DataPropertyMap const& propertyThatIsNotYetAdded);
        BentleyStatus CopyModifiedBasePropertyMaps(SchemaImportContext& ctx );
    protected:
        ClassMap(ECDb const&, TableSpaceSchemaManager const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&);

        virtual ClassMappingStatus _Map(ClassMappingContext&);
        ClassMappingStatus DoMapPart1(ClassMappingContext&);
        ClassMappingStatus DoMapPart2(ClassMappingContext&);
        ClassMappingStatus MapProperties(ClassMappingContext&);
        virtual BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&);
        BentleyStatus LoadPropertyMaps(ClassMapLoadContext&, DbClassMapLoadContext const&);
        IssueDataSource const& Issues() const;
        BentleyStatus MapSystemColumns();

    public:
        ClassMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrat) : ClassMap(ecdb, manager, Type::Class, ecClass, mapStrat) {}
        virtual ~ClassMap() {}
        void Modified() { if (m_state == ObjectState::Persisted) m_state = ObjectState::Modified; }

        //! Called when loading an existing class map from the ECDb file
        BentleyStatus Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbLoadCtx) { return _Load(ctx, dbLoadCtx); }

        ObjectState GetState() const { return m_state; }
        template<typename TClassMap>
        TClassMap const& GetAs() const { BeAssert(dynamic_cast<TClassMap const*> (this) != nullptr); return *static_cast<TClassMap const*>(this); }
        Type GetType() const { return m_type;}
        PropertyMapContainer const& GetPropertyMaps() const { return m_propertyMaps; }
        ECInstanceIdPropertyMap const* GetECInstanceIdPropertyMap() const;
        ECClassIdPropertyMap const* GetECClassIdPropertyMap() const;
        std::vector<DbTable*>& GetTables() const { return m_tables; }
        DbTable& GetPrimaryTable() const;
        DbTable& GetJoinedOrPrimaryTable() const;
        DbTable* GetOverflowTable() const;
        bool IsMixin() const { ECN::ECEntityClassCP entityClass = m_ecClass.GetEntityClassCP(); return entityClass != nullptr && entityClass->IsMixin(); }
        bool IsMappedTo(DbTable const& table) const { return std::find(m_tables.begin(), m_tables.end(), &table) != m_tables.end(); }
        bool IsMappedToSingleTable() const { return m_tables.size() == 1; }

        //! Returns the class maps of the classes derived from this class map's class.
        //! @return Derived classes class maps
        Nullable<std::vector<ClassMap const*>> GetDerivedClassMaps() const;
        ECN::ECClassCR GetClass() const { return m_ecClass; }
        MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
        //!Only call this if the map strategy is TablePerHierarchy
        TablePerHierarchyHelper const* GetTphHelper() const { BeAssert(m_tphHelper != nullptr); return m_tphHelper.get(); }

        StorageDescription const& GetStorageDescription() const;
        bool IsRelationshipClassMap() const { return m_type == Type::RelationshipEndTable || m_type == Type::RelationshipLinkTable; }
        ECDb const& GetECDb() const { return m_ecdb; }
        TableSpaceSchemaManager const& GetSchemaManager() const { return m_schemaManager; }

        ClassMappingStatus Map(SchemaImportContext& importCtx, ClassMappingInfo const& info) { ClassMappingContext ctx(importCtx, info);  return _Map(ctx); }
        BentleyStatus Save(SchemaImportContext&, DbMapSaveContext&);
        BentleyStatus Update(SchemaImportContext& ctx);
        void SetTable(DbTable& newTable) { m_tables.clear(); AddTable(newTable); }
        void AddTable(DbTable& newTable) { BeAssert(std::find(begin(m_tables), end(m_tables), &newTable) == end(m_tables)); m_tables.push_back(&newTable); }
        BentleyStatus SetOverflowTable(DbTable& overflowTable);
        ClassMapColumnFactory const& GetColumnFactory() const;
        PropertyMapContainer& GetPropertyMapsR() { return m_propertyMaps; }

        template<typename TClassMap>
        static std::unique_ptr<TClassMap> Create(ECDb const& ecdb, TableSpaceSchemaManager const& tableSpaceManager, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy)
            {
            return std::make_unique<TClassMap>(ecdb, tableSpaceManager, ecClass, mapStrategy);
            }

    };

    typedef ClassMap const& ClassMapCR;

//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct NotMappedClassMap final : public ClassMap
    {
private:
    ClassMappingStatus _Map(ClassMappingContext&) override;
    BentleyStatus _Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo) override;

public:
    NotMappedClassMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy) : ClassMap(ecdb, manager, Type::NotMapped, ecClass, mapStrategy) {}
    ~NotMappedClassMap() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE