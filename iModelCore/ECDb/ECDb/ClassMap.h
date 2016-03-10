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
#include "ClassMapInfo.h"
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

    BentleyStatus Postprocess(ECDbMapCR) const;
    };


struct NativeSqlBuilder;
struct StorageDescription;

//=======================================================================================
//! A helper class to help generate view/trigger in standard way
// @bsiclass                                               Affan.Khan          08/2015
//+===============+===============+===============+===============+===============+======
struct PropertyMapSet : NonCopyableClass
    {
    typedef std::unique_ptr<PropertyMapSet> Ptr;
    struct EndPoint : NonCopyableClass
        {
        private:
            ECDbSqlColumn const* m_column;
            Utf8String m_accessString;
            ECN::ECValue m_value;
            ColumnKind m_columnKind;
        public:
            EndPoint(Utf8CP accessString, ColumnKind columnKin, ECN::ECValueCR value)
                : m_accessString(accessString), m_column(nullptr), m_value(value), m_columnKind(columnKin)
                {}
            EndPoint(Utf8CP accessString, ECDbSqlColumn const& column, ECN::ECValueCR value)
                : m_accessString(accessString), m_column(&column), m_value(value), m_columnKind(column.GetKind())
                {}
            ECDbSqlColumn const* GetColumn() const { return m_column; }
            Utf8StringCR GetAccessString() const { return m_accessString; }
            ECN::ECValueCR GetValue() const { return m_value; }
            ColumnKind GetColumnKind() const { return m_columnKind; }
        };


    typedef std::vector<EndPoint const*> EndPoints;
    private:
        std::vector<std::unique_ptr<EndPoint>> m_orderedEndPoints;
        std::map<Utf8CP, EndPoint const*, CompareUtf8> m_endPointByAccessString;
        ClassMap const& m_classMap;

        PropertyMapSet(ClassMap const& classMap) :m_classMap(classMap) {}

    public:
        const EndPoints GetEndPoints() const;
        const EndPoint* GetEndPointByAccessString(Utf8CP accessString) const
            {
            auto itor = m_endPointByAccessString.find(accessString);
            if (itor == m_endPointByAccessString.end())
                return nullptr;

            return itor->second;
            }

        static BentleyStatus AddSystemEndPoint(PropertyMapSet& propertySet, ClassMap const&, ColumnKind, ECN::ECValueCR, ECDbSqlColumn const* column = nullptr);
        static PropertyMapSet::Ptr Create(ClassMap const&);
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ColumnFactory : NonCopyableClass
    {
    private:
        ClassMapCR m_classMap;
        bool m_usesSharedColumnStrategy;
        mutable std::set<Utf8String, CompareIUtf8> m_columnsInUse;

        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8CP requestedColumnName, ECN::ECClassId, int retryCount) const;

        ECDbSqlColumn* ApplyDefaultStrategy(Utf8CP requestedColumnName, PropertyMapCR, ECDbSqlColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, ECDbSqlColumn::Constraint::Collation) const;
        ECDbSqlColumn* ApplySharedColumnStrategy() const;

        ECN::ECClassId GetPersistenceClassId(PropertyMapCR) const;
        bool TryFindReusableSharedDataColumn(ECDbSqlColumn const*& reusableColumn) const;
        bool IsColumnInUse(ECDbSqlColumn const&) const;
        void CacheUsedColumn(ECDbSqlColumn const&) const;

        ECDbSqlTable& GetTable() const;

    public:
        explicit ColumnFactory(ClassMapCR classMap);
        ~ColumnFactory() {}

        ECDbSqlColumn* CreateColumn(PropertyMapCR, Utf8CP requestedColumnName, ECDbSqlColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, ECDbSqlColumn::Constraint::Collation) const;
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
        typedef std::vector<ECDbSqlTable*>& TableListR;

        enum class Type
            {
            Class,
            RelationshipEndTable,
            RelationshipLinkTable,
            Unmapped
            };

    private:
        ECDbMapCR m_ecDbMap;
        ECDbClassMapId m_id;
        Type m_type;
        ECDbMapStrategy m_mapStrategy;
        PropertyMapCollection m_propertyMaps;
        mutable std::vector<ECDbSqlTable*> m_tables;
        bool m_isDirty;
        bool m_isECInstanceIdAutogenerationDisabled;
        ECN::ECClassCR m_ecClass;
        ColumnFactory m_columnFactory;

    protected:
        ECN::ECClassId m_parentMapClassId;

    private:
        BentleyStatus InitializeDisableECInstanceIdAutogeneration();
        BentleyStatus CreateCurrentTimeStampTrigger(ECN::ECPropertyCR);

        virtual MapStatus _OnInitialized() { return MapStatus::Success; }

    protected:
        ClassMap(Type, ECN::ECClassCR, ECDbMapCR, ECDbMapStrategy, bool setIsDirty);

        virtual MapStatus _MapPart1(SchemaImportContext&, ClassMapInfo const&, ClassMap const* parentClassMap);
        virtual MapStatus _MapPart2(SchemaImportContext&, ClassMapInfo const&, ClassMap const* parentClassMap);
        virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ECDbClassMapInfo const&, ClassMap const* parentClassMap);
        virtual BentleyStatus _Save(std::set<ClassMap const*>& savedGraph);


        MapStatus AddPropertyMaps(ClassMapLoadContext&, ClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo, ClassMapInfo const* classMapInfo);
        void SetTable(ECDbSqlTable& newTable, bool append = false);
        PropertyMapCollection& GetPropertyMapsR() { return m_propertyMaps; }
        ECDbSchemaManagerCR Schemas() const;

    public:
        static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new ClassMap(Type::Class, ecClass, ecdbMap, mapStrategy, setIsDirty); }

        //! Called when loading an existing class map from the ECDb file 
        BentleyStatus Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ECDbClassMapInfo const& mapInfo, ClassMap const* parentClassMap) { return _Load(loadGraph, ctx, mapInfo, parentClassMap); }

        //! Called during schema import when creating the class map from the imported ECClass 
        MapStatus Map(SchemaImportContext&, ClassMapInfo const& classMapInfo);

        PropertyMapCollection const& GetPropertyMaps() const { return m_propertyMaps; }
        PropertyMapCP GetPropertyMap(Utf8CP propertyName) const;
        PropertyMapCP GetECInstanceIdPropertyMap() const;
        bool TryGetECInstanceIdPropertyMap(PropertyMapPtr& ecIstanceIdPropertyMap) const;

        BentleyStatus CreateUserProvidedIndexes(SchemaImportContext&, bvector<ClassIndexInfoPtr> const&) const;

        Type GetType() const { return m_type; }
        bool IsDirty() const { return m_isDirty; }
        ECDbClassMapId GetId() const { return m_id; }
        void SetId(ECDbClassMapId id) { m_id = id; }
        BentleyStatus Save(std::set<ClassMap const*>& savedGraph) { return _Save(savedGraph); }

        ColumnFactory const& GetColumnFactory() const { return m_columnFactory; }
        ColumnFactory& GetColumnFactoryR() { return m_columnFactory; }

        TableListR GetTables() const { return m_tables; }
        ECDbSqlTable& GetPrimaryTable() const { BeAssert(!GetTables().empty()); return *GetTables().front(); }
        ECDbSqlTable& GetJoinedTable() const { BeAssert(!GetTables().empty()); return *GetTables().back(); }
        bool IsMappedTo(ECDbSqlTable const& table) const { TableListR tables = GetTables(); return std::find(tables.begin(), tables.end(), &table) != tables.end(); }
        bool IsMappedToSingleTable() const { return GetTables().size() == 1; }

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
        bool IsRelationshipClassMap() const;
        bool HasJoinedTable() const;
        bool IsParentOfJoinedTable() const;

        Utf8String GetPersistedViewName() const;
        bool HasPersistedView() const;

        BentleyStatus GenerateSelectView(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& prepareContext) const;

        Utf8String ToString() const;

        static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
        static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);
        static bool IsAnyClass(ECN::ECClassCR);
    };


//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct UnmappedClassMap : public ClassMap
    {
private:
    UnmappedClassMap(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) : ClassMap(Type::Unmapped, ecClass, ecdbMap, mapStrategy, setIsDirty) {}

    virtual MapStatus _MapPart1(SchemaImportContext&, ClassMapInfo const& classMapInfo, ClassMap const* parentClassMap) override;
    virtual MapStatus _MapPart2(SchemaImportContext&, ClassMapInfo const& classMapInfo, ClassMap const* parentClassMap) override { return MapStatus::Success; }
    virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ECDbClassMapInfo const& mapInfo, ClassMap const* parentClassMap) override;

public:
    ~UnmappedClassMap() {}

    static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new UnmappedClassMap(ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE