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

//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMap : RefCountedBase
    {
    public:
        enum class Type
            {
            Class,
            RelationshipEndTable,
            RelationshipLinkTable,
            NotMapped
            };

        enum class PropertyMapInheritanceMode
            {
            NotInherited, //!< indicates that base property map is not inherited, but created from scratch
            Clone //! inherited property maps are cloned from the base class property map
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
        mutable std::unique_ptr<ClassMapColumnFactory> m_columnFactory;
        std::unique_ptr<TablePerHierarchyHelper> m_tphHelper;
        bool m_isDirty;
        bvector<ECN::ECPropertyCP> m_failedToLoadProperties;
        BentleyStatus CreateCurrentTimeStampTrigger(ECN::PrimitiveECPropertyCR);

        bool DetermineIsExclusiveRootClassOfTable(ClassMappingInfo const&) const;

        PropertyMapInheritanceMode GetPropertyMapInheritanceMode() const { return GetPropertyMapInheritanceMode(m_mapStrategyExtInfo); }

    protected:
        ClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&, bool setIsDirty);
 
        virtual ClassMappingStatus _Map(ClassMappingContext&);
        ClassMappingStatus DoMapPart1(ClassMappingContext&);
        ClassMappingStatus DoMapPart2(ClassMappingContext&);
        ClassMappingStatus MapProperties(ClassMappingContext&);
        virtual BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&);
        BentleyStatus LoadPropertyMaps(ClassMapLoadContext&, DbClassMapLoadContext const&);

        void SetTable(DbTable& newTable) { m_tables.clear(); AddTable(newTable); }
        void AddTable(DbTable& newTable) { m_tables.push_back(&newTable); }

        ECDb const& GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const;
        static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);
        BentleyStatus MapSystemColumns();

    public:
        virtual ~ClassMap() {}

        static ClassMapPtr Create(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty) { return new ClassMap(ecdb, Type::Class, ecClass, mapStrategy, setIsDirty); }

        template<typename TClassMap>
        TClassMap const& GetAs() const { BeAssert(dynamic_cast<TClassMap const*> (this) != nullptr); return *static_cast<TClassMap const*>(this); }

        PropertyMapContainer const& GetPropertyMaps() const { return m_propertyMaps; }
        ECInstanceIdPropertyMap const* GetECInstanceIdPropertyMap() const;
        ECClassIdPropertyMap const* GetECClassIdPropertyMap() const;
        BentleyStatus CreateUserProvidedIndexes(SchemaImportContext&, std::vector<IndexMappingInfoPtr> const&) const;
        Type GetType() const { return m_type; }
        bool IsDirty() const { return m_isDirty; }
        ClassMapColumnFactory const& GetColumnFactory(bool refresh = false) const;
        std::vector<DbTable*>& GetTables() const { return m_tables; }
        DbTable& GetPrimaryTable() const { BeAssert(!GetTables().empty()); return *GetTables().front(); }
        DbTable& GetJoinedTable() const { BeAssert(!GetTables().empty()); return *GetTables().back(); }
        bool IsMappedTo(DbTable const& table) const { return std::find(m_tables.begin(), m_tables.end(), &table) != m_tables.end(); }
        bool IsMappedToSingleTable() const { return m_tables.size() == 1; }
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
        Utf8String GetUpdatableViewName() const;
        DbTable const* ExpectingSingleTable() const;
        BentleyStatus SetOverflowTable(DbTable& overflowTable);
        //! Called when loading an existing class map from the ECDb file 
        BentleyStatus Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbLoadCtx) { return _Load(ctx, dbLoadCtx); }
        //! Called during schema import when creating the class map from the imported ECClass 
        ClassMappingStatus Map(SchemaImportContext& importCtx, ClassMappingInfo const& info) { ClassMappingContext ctx(importCtx, info);  return _Map(ctx); }
        BentleyStatus Save(DbMapSaveContext&);
        BentleyStatus Update();
        PropertyMapContainer& GetPropertyMapsR() { return m_propertyMaps; }

        //! Rules:
        //! If MapStrategy != TPH: NotInherited
        //! Else: Clone
        static PropertyMapInheritanceMode GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const&);
        static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
        static bool IsAnyClass(ECN::ECClassCR ecclass) { return ecclass.GetSchema().IsStandardSchema() && ecclass.GetName().Equals("AnyClass"); }

        static Utf8CP TypeToString(Type);
    };


//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct NotMappedClassMap final : public ClassMap
    {
private:
    NotMappedClassMap(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty) : ClassMap(ecdb, Type::NotMapped, ecClass, mapStrategy, setIsDirty) {}

    ClassMappingStatus _Map(ClassMappingContext&) override;
    BentleyStatus _Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo) override;

public:
    ~NotMappedClassMap() {}

    static ClassMapPtr Create(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty) { return new NotMappedClassMap(ecdb, ecClass, mapStrategy, setIsDirty); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE