/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>
#include "PropertyMap.h"
#include "ClassMappingInfo.h"
#include "IssueReporter.h"
#include <Bentley/NonCopyableClass.h>
#include "ClassMapColumnFactory.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct DbMap;

/* ----------------Required Refactor--------------------------------------
struct ClassMap
    {};
struct IRelationshipClassMap
    {};
struct ForeignKeyRelationshipClassMap :ClassMap, IRelationshipClassMap
    {};
struct SingleTableClassMap :ClassMap
    {};
struct EntityClassMap :SingleTableClassMap
    {};
struct LinkTableRelationshipClassMap :SingleTableClassMap, IRelationshipClassMap
    {};
*/


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      01/2016
//+===============+===============+===============+===============+===============+======
struct ClassMapLoadContext : NonCopyableClass
    {
private:
    std::set<ECN::ECClassCP> m_constraintClasses;
    std::vector<NavigationPropertyMap*> m_navPropMaps;
    
public:
    ClassMapLoadContext() {}

    void AddConstraintClass(ECN::ECClassCR ecClass) 
        { 
        //LOG.debugv("ClassMapLoadContext> Added ECRelationshipConstraint ECClass '%s' to context %p.", ecClass.GetFullName(), this);
        m_constraintClasses.insert(&ecClass);
        }
    void AddNavigationPropertyMap(NavigationPropertyMap& propMap) 
        { 
        //LOG.debugv("ClassMapLoadContext> Added NavPropMap '%s.%s' to context %p.",propMap.GetProperty().GetClass().GetFullName(), propMap.GetProperty().GetName().c_str(), this);
        m_navPropMaps.push_back(&propMap); 
        }

    BentleyStatus Postprocess(DbMap const&);
    };

struct NativeSqlBuilder;
struct StorageDescription;
struct ECSqlPrepareContext;
struct ECInstanceIdPropertyMap;
struct ECClassIdPropertyMap;
struct DbClassMapLoadContext;
struct DbMapSaveContext;

struct ClassMap;
typedef RefCountedPtr<ClassMap> ClassMapPtr;
typedef ClassMap const& ClassMapCR;
typedef ClassMap const* ClassMapCP;

enum ObjectState
    {
    Persisted, //loaded from disk
    Modified, // loaded from disk but modified
    New //new object not yet persisted to disk
    };
//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMap : RefCountedBase
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
            ECN::ECClassId DetermineParentOfJoinedTableECClassId() const;

            };

        struct UpdatableViewInfo final
            {
        private:
            Utf8String m_viewName;

        public:
            UpdatableViewInfo() {}
            explicit UpdatableViewInfo(Utf8StringCR viewName) : m_viewName(viewName) {}

            Utf8StringCR GetViewName() const { return m_viewName; }
            bool HasView() const { return !m_viewName.empty(); }
            };

    protected:
        struct ClassMappingContext final : NonCopyableClass
            {
            private:
                SchemaImportContext& m_importCtx;
                ClassMappingInfo const& m_classMappingInfo;

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
        ECN::ECClassCR m_ecClass;
        MapStrategyExtendedInfo m_mapStrategyExtInfo;
        PropertyMapContainer m_propertyMaps;
        mutable std::vector<DbTable*> m_tables;
        UpdatableViewInfo m_updatableViewInfo;
        mutable std::unique_ptr<ClassMapColumnFactory> m_columnFactory;
        std::unique_ptr<TablePerHierarchyHelper> m_tphHelper;
        bvector<ECN::ECPropertyCP> m_failedToLoadProperties;
        ObjectState m_state;
        BentleyStatus CreateCurrentTimeStampTrigger(ECN::PrimitiveECPropertyCR);

        ClassMap(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrat) : ClassMap(ecdb, Type::Class, ecClass, mapStrat) {}
        ClassMap(ECDb const&ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrat, UpdatableViewInfo const& updatableViewInfo) : ClassMap(ecdb, Type::Class, ecClass, mapStrat, updatableViewInfo) {}

    protected:
        ClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        ClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&, UpdatableViewInfo const&);

        virtual ClassMappingStatus _Map(ClassMappingContext&);
        ClassMappingStatus DoMapPart1(ClassMappingContext&);
        ClassMappingStatus DoMapPart2(ClassMappingContext&);
        ClassMappingStatus MapProperties(ClassMappingContext&);
        virtual BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&);
        BentleyStatus LoadPropertyMaps(ClassMapLoadContext&, DbClassMapLoadContext const&);

        ECDb const& GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const;
        BentleyStatus MapSystemColumns();

    public:
        virtual ~ClassMap() {}

        //! Called when loading an existing class map from the ECDb file 
        BentleyStatus Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbLoadCtx) { return _Load(ctx, dbLoadCtx); }

        ObjectState GetState() const { return m_state; }
        template<typename TClassMap>
        TClassMap const& GetAs() const { BeAssert(dynamic_cast<TClassMap const*> (this) != nullptr); return *static_cast<TClassMap const*>(this); }
        Type GetType() const { return m_type; }
        PropertyMapContainer const& GetPropertyMaps() const { return m_propertyMaps; }
        ECInstanceIdPropertyMap const* GetECInstanceIdPropertyMap() const;
        ECClassIdPropertyMap const* GetECClassIdPropertyMap() const;

        std::vector<DbTable*>& GetTables() const { return m_tables; }
        DbTable& GetPrimaryTable() const 
            { 
            if (GetType() == Type::RelationshipEndTable)
                return *m_tables.front();

            for (DbTable* table : GetTables())
                {
                if (table->GetType() == DbTable::Type::Primary || table->GetType() == DbTable::Type::Existing || table->GetType() == DbTable::Type::Virtual)
                    return *table;
                }

            BeAssert(false);
            DbTable* nulltable = nullptr;
            return *nulltable;
            }
        DbTable& GetJoinedOrPrimaryTable() const 
            {
            DbTable* joinedTable = nullptr;
            DbTable* primaryTable = nullptr;
            for (DbTable* table : m_tables)
                {
                if (table->GetType() == DbTable::Type::Joined)
                    joinedTable = table;
                else if (table->GetType() == DbTable::Type::Primary || table->GetType() == DbTable::Type::Existing || table->GetType() == DbTable::Type::Virtual)
                    primaryTable = table;
                
                if (joinedTable != nullptr)
                    return *joinedTable;
                }

            BeAssert(primaryTable != nullptr);
            return *primaryTable;
            }
        DbTable* GetOverflowTable() const
            {
            for (DbTable* table : GetTables())
                {
                if (table->GetType() == DbTable::Type::Overflow)
                    return table;
                }

            return nullptr;
            }

        bool IsMappedTo(DbTable const& table) const { return std::find(m_tables.begin(), m_tables.end(), &table) != m_tables.end(); }
        bool IsMappedToSingleTable() const { return m_tables.size() == 1; }
        UpdatableViewInfo const& GetUpdatableViewInfo() const { return m_updatableViewInfo; }

        //! Returns the class maps of the classes derived from this class map's class.
        //! @return Derived classes class maps
        std::vector<ClassMap const*> GetDerivedClassMaps() const;
        ECN::ECClassCR GetClass() const { return m_ecClass; }
        MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
        //!Only call this if the map strategy is TablePerHierarchy
        TablePerHierarchyHelper const* GetTphHelper() const { BeAssert(m_tphHelper != nullptr); return m_tphHelper.get(); }

        StorageDescription const& GetStorageDescription() const;
        bool IsRelationshipClassMap() const { return m_type == Type::RelationshipEndTable || m_type == Type::RelationshipLinkTable; }
        DbMap const& GetDbMap() const { return m_ecdb.Schemas().GetDbMap(); }
        DbTable const* ExpectingSingleTable() const;

        ClassMappingStatus Map(SchemaImportContext& importCtx, ClassMappingInfo const& info) { ClassMappingContext ctx(importCtx, info);  return _Map(ctx); }
        BentleyStatus Save(SchemaImportContext&, DbMapSaveContext&);
        BentleyStatus Update();
        BentleyStatus CreateUserProvidedIndexes(SchemaImportContext&, std::vector<IndexMappingInfoPtr> const&) const;
        void SetTable(DbTable& newTable) { m_tables.clear(); AddTable(newTable); }
        void AddTable(DbTable& newTable) { BeAssert(std::find(begin(m_tables), end(m_tables), &newTable) == end(m_tables)); m_tables.push_back(&newTable); }
        BentleyStatus SetOverflowTable(DbTable& overflowTable);
        ClassMapColumnFactory const& GetColumnFactory() const;
        PropertyMapContainer& GetPropertyMapsR() { return m_propertyMaps; }
   
        static bool IsAnyClass(ECN::ECClassCR ecclass) { return ecclass.GetSchema().IsStandardSchema() && ecclass.GetName().Equals("AnyClass"); }
        static Utf8CP TypeToString(Type);
    };


//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct NotMappedClassMap final : public ClassMap
    {
    friend struct ClassMapFactory;

private:
    NotMappedClassMap(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy) : ClassMap(ecdb, Type::NotMapped, ecClass, mapStrategy) {}
    NotMappedClassMap(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy, UpdatableViewInfo const&) : NotMappedClassMap(ecdb, ecClass, mapStrategy) {}

    ClassMappingStatus _Map(ClassMappingContext&) override;
    BentleyStatus _Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo) override;

public:
    ~NotMappedClassMap() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE