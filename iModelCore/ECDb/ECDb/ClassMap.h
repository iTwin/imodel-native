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
#include <Bentley/NonCopyableClass.h>
#include "DebugWriter.h"
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

    BentleyStatus Postprocess(ECDbMapCR);
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

        DbColumn* ApplyDefaultStrategy(Utf8CP requestedColumnName, PropertyMapCR, DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraint::Collation) const;
        DbColumn* ApplySharedColumnStrategy() const;

        ECN::ECClassId GetPersistenceClassId(PropertyMapCR) const;
        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const&) const;
        void CacheUsedColumn(DbColumn const&) const;

        DbTable& GetTable() const;

    public:
        explicit ColumnFactory(ClassMapCR classMap);
        ~ColumnFactory() {}

        DbColumn* CreateColumn(PropertyMapCR, Utf8CP requestedColumnName, DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraint::Collation) const;
        void Update();

        bool UsesSharedColumnStrategy() const { return m_usesSharedColumnStrategy; }
    };

struct ECSqlPrepareContext;

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
            Unmapped
            };

    private:
        ECDbMapCR m_ecDbMap;
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
        ECN::ECClassId m_parentMapClassId;

    private:
        BentleyStatus InitializeDisableECInstanceIdAutogeneration();
        BentleyStatus CreateCurrentTimeStampTrigger(ECN::ECPropertyCR);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
        virtual MappingStatus _OnInitialized() { return MappingStatus::Success; }

    protected:
        ClassMap(Type, ECN::ECClassCR, ECDbMapCR, ECDbMapStrategy, bool setIsDirty);

        virtual MappingStatus _MapPart1(SchemaImportContext&, ClassMappingInfo const&, ClassMap const* parentClassMap);
        virtual MappingStatus _MapPart2(SchemaImportContext&, ClassMappingInfo const&, ClassMap const* parentClassMap);
        virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ClassDbMapping const&, ClassMap const* parentClassMap);
        virtual BentleyStatus _Save(std::set<ClassMap const*>& savedGraph);
        virtual void _WriteDebugInfo(DebugWriter& writer) const;
        MappingStatus AddPropertyMaps(ClassMapLoadContext&, ClassMap const* parentClassMap, ClassDbMapping const* loadInfo, ClassMappingInfo const* classMapInfo);
        void SetTable(DbTable& newTable, bool append = false);
        PropertyMapCollection& GetPropertyMapsR() { return m_propertyMaps; }
        ECDbSchemaManagerCR Schemas() const;

        static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);

    public:
        static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new ClassMap(Type::Class, ecClass, ecdbMap, mapStrategy, setIsDirty); }

        //! Called when loading an existing class map from the ECDb file 
        BentleyStatus Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ClassDbMapping const& mapInfo, ClassMap const* parentClassMap) { return _Load(loadGraph, ctx, mapInfo, parentClassMap); }

        //! Called during schema import when creating the class map from the imported ECClass 
        MappingStatus Map(SchemaImportContext&, ClassMappingInfo const& classMapInfo);

        PropertyMapCollection const& GetPropertyMaps() const { return m_propertyMaps; }
        PropertyMapCP GetPropertyMap(Utf8CP propertyName) const;
        PropertyMapCP GetECInstanceIdPropertyMap() const;
        bool TryGetECInstanceIdPropertyMap(PropertyMapPtr& ecIstanceIdPropertyMap) const;

        BentleyStatus CreateUserProvidedIndexes(SchemaImportContext&, std::vector<IndexMappingInfoPtr> const&) const;

        Type GetType() const { return m_type; }
        bool IsDirty() const { return m_isDirty; }
        ClassMapId GetId() const { return m_id; }
        void SetId(ClassMapId id) { m_id = id; }
        BentleyStatus Save(std::set<ClassMap const*>& savedGraph) { return _Save(savedGraph); }

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
        ECN::ECClassId GetParentMapClassId() const { return m_parentMapClassId; }

        ECDbMapStrategy const& GetMapStrategy() const { return m_mapStrategy; }
        ECDbMapCR GetECDbMap() const { return m_ecDbMap; }
        bool IsECInstanceIdAutogenerationDisabled() const { return m_isECInstanceIdAutogenerationDisabled; }

        StorageDescription const& GetStorageDescription() const;
        bool IsRelationshipClassMap() const { return m_type == Type::RelationshipEndTable || m_type == Type::RelationshipLinkTable; }
        bool HasJoinedTable() const;
        bool IsParentOfJoinedTable() const;

        Utf8String GetUpdatableViewName() const;
        BentleyStatus GenerateSelectViewSql(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& prepareContext) const;

        static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
        static bool IsAnyClass(ECN::ECClassCR ecclass) { return ecclass.GetSchema().IsStandardSchema() && ecclass.GetName().Equals("AnyClass"); }
        void WriteDebugInfo(DebugWriter& writer) const { _WriteDebugInfo(writer); }
    };


//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct UnmappedClassMap : public ClassMap
    {
private:
    UnmappedClassMap(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) : ClassMap(Type::Unmapped, ecClass, ecdbMap, mapStrategy, setIsDirty) {}

    virtual MappingStatus _MapPart1(SchemaImportContext&, ClassMappingInfo const& classMapInfo, ClassMap const* parentClassMap) override;
    virtual MappingStatus _MapPart2(SchemaImportContext&, ClassMappingInfo const& classMapInfo, ClassMap const* parentClassMap) override { return MappingStatus::Success; }
    virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ClassDbMapping const& mapInfo, ClassMap const* parentClassMap) override;

public:
    ~UnmappedClassMap() {}

    static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new UnmappedClassMap(ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE