/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "PropertyMap.h"
#include "IssueReporter.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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

    BentleyStatus Postprocess(ECDbMap const&);
    };

struct NativeSqlBuilder;
struct StorageDescription;
//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ColumnFactory : NonCopyableClass
    {
    private:
        ClassMapCR m_classMap;
        bool m_usesSharedColumnStrategy;
        mutable bset<DbColumnId> m_idsOfColumnsInUseByClassMap;

        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8CP requestedColumnName, ECN::ECClassId, int retryCount) const;

        DbColumn* ApplyDefaultStrategy(Utf8CP requestedColumnName, PropertyMapCR, DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation) const;
        DbColumn* ApplySharedColumnStrategy() const;

        ECN::ECClassId GetPersistenceClassId(PropertyMapCR) const;
        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const&) const;
        void CacheUsedColumn(DbColumn const&) const;

        DbTable& GetTable() const;

    public:
        explicit ColumnFactory(ClassMapCR classMap);
        ~ColumnFactory() {}

        DbColumn* CreateColumn(PropertyMapCR, Utf8CP requestedColumnName, DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation) const;
        void Update();

        bool UsesSharedColumnStrategy() const { return m_usesSharedColumnStrategy; }
    };

struct ECDbMap;
struct ECSqlPrepareContext;
struct ECInstanceIdPropertyMap;
struct ECClassIdPropertyMap;
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

    private:
        ECDbMap const& m_ecDbMap;
        ClassMapId m_id;
        Type m_type;
        ECDbMapStrategy m_mapStrategy;
        PropertyMapCollection m_propertyMaps;
        mutable std::vector<DbTable*> m_tables;
        bool m_isDirty;
        bool m_isECInstanceIdAutogenerationDisabled;
        ECN::ECClassCR m_ecClass;
        ColumnFactory m_columnFactory;

    protected:
        ECN::ECClassId m_baseClassId;

    private:
        BentleyStatus InitializeDisableECInstanceIdAutogeneration();
        BentleyStatus CreateCurrentTimeStampTrigger(ECN::ECPropertyCR);

    protected:
        ClassMap(Type, ECN::ECClassCR, ECDbMap const&, ECDbMapStrategy const&, bool setIsDirty);

        virtual MappingStatus _Map(SchemaImportContext&, ClassMappingInfo const&);
        MappingStatus DoMapPart1(SchemaImportContext&, ClassMappingInfo const&);
        MappingStatus DoMapPart2(SchemaImportContext&, ClassMappingInfo const&);
        virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ClassDbMapping const&, ClassMap const* baseClassMap);
        MappingStatus AddPropertyMaps(ClassMapLoadContext&, ClassMap const* baseClassMap, ClassDbMapping const*, ClassMappingInfo const*);
        void SetTable(DbTable& newTable, bool append = false);
        PropertyMapCollection& GetPropertyMapsR() { return m_propertyMaps; }
        ECDbSchemaManagerCR Schemas() const;
        IssueReporter const& Issues() const;
        static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);

    public:
        static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMap const& ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new ClassMap(Type::Class, ecClass, ecdbMap, mapStrategy, setIsDirty); }

        //! Called when loading an existing class map from the ECDb file 
        BentleyStatus Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ClassDbMapping const& mapInfo, ClassMap const* parentClassMap) { return _Load(loadGraph, ctx, mapInfo, parentClassMap); }

        //! Called during schema import when creating the class map from the imported ECClass 
        MappingStatus Map(SchemaImportContext&, ClassMappingInfo const& classMapInfo);
        BentleyStatus Save(SchemaImportContext&);

        PropertyMapCollection const& GetPropertyMaps() const { return m_propertyMaps; }
        PropertyMapCP GetPropertyMap(Utf8CP propertyName) const;
        ECInstanceIdPropertyMap const* GetECInstanceIdPropertyMap() const;
        ECClassIdPropertyMap const* GetECClassIdPropertyMap() const;
        BentleyStatus ConfigureECClassId(DbColumn const& classIdColumn, bool loadingFromDisk = false);
        BentleyStatus ConfigureECClassId(std::vector<DbColumn const*> const& columns, bool loadingFromDisk = false);
        BentleyStatus CreateUserProvidedIndexes(SchemaImportContext&, std::vector<IndexMappingInfoPtr> const&) const;

        Type GetType() const { return m_type; }
        bool IsDirty() const { return m_isDirty; }
        ClassMapId GetId() const { return m_id; }
        void SetId(ClassMapId id) { m_id = id; }

        ColumnFactory const& GetColumnFactory() const { return m_columnFactory; }
        ColumnFactory& GetColumnFactoryR() { return m_columnFactory; }

        std::vector<DbTable*>& GetTables() const { return m_tables; }
        DbTable& GetPrimaryTable() const { BeAssert(!GetTables().empty()); return *GetTables().front(); }
        DbTable& GetJoinedTable() const { BeAssert(!GetTables().empty()); return *GetTables().back(); }
        bool IsMappedTo(DbTable const& table) const { return std::find(m_tables.begin(), m_tables.end(), &table) != m_tables.end(); }
        bool IsMappedToSingleTable() const { return m_tables.size() == 1; }

        ClassMap const* FindSharedTableRootClassMap() const;
        ClassMap const* FindClassMapOfParentOfJoinedTable() const;
        BentleyStatus GetPathToParentOfJoinedTable(std::vector<ClassMap const*>& path) const;
        ClassMap const* GetParentOfJoinedTable() const;

        //! Returns the class maps of the classes derived from this class map's class.
        //! @return Derived classes class maps
        std::vector<ClassMap const*> GetDerivedClassMaps() const;

        ECN::ECClassCR GetClass() const { return m_ecClass; }
        ECN::ECClassId GetBaseClassId() const { return m_baseClassId; }

        ECDbMapStrategy const& GetMapStrategy() const { return m_mapStrategy; }
        ECDbMap const& GetECDbMap() const { return m_ecDbMap; }
        bool IsECInstanceIdAutogenerationDisabled() const { return m_isECInstanceIdAutogenerationDisabled; }

        StorageDescription const& GetStorageDescription() const;
        bool IsRelationshipClassMap() const { return m_type == Type::RelationshipEndTable || m_type == Type::RelationshipLinkTable; }
        bool HasJoinedTable() const;
        bool IsParentOfJoinedTable() const;

        Utf8String GetUpdatableViewName() const;
        BentleyStatus GenerateSelectViewSql(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& prepareContext) const;
        DbTable const* ExpectingSingleTable() const 
            {
            BeAssert(GetTables().size() == 1);
            if (GetTables().size() != 1)
                return nullptr;

            return &GetJoinedTable();
            }
        static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
        static bool IsAnyClass(ECN::ECClassCR ecclass) { return ecclass.GetSchema().IsStandardSchema() && ecclass.GetName().Equals("AnyClass"); }

        static Utf8CP TypeToString(Type);
    };


//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct NotMappedClassMap : public ClassMap
    {
private:
    NotMappedClassMap(ECN::ECClassCR ecClass, ECDbMap const& ecdbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty) : ClassMap(Type::NotMapped, ecClass, ecdbMap, mapStrategy, setIsDirty) {}

    virtual MappingStatus _Map(SchemaImportContext&, ClassMappingInfo const&) override;
    virtual BentleyStatus _Load(std::set<ClassMap const*>&, ClassMapLoadContext&, ClassDbMapping const&, ClassMap const* baseClassMap) override;

public:
    ~NotMappedClassMap() {}

    static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMap const& ecdbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty) { return new NotMappedClassMap(ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE