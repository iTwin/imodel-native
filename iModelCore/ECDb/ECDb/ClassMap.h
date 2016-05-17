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
//! Represents the SQLite view to which an ECClass is mapped by a ClassMap
//! ECDb does not create a persistent database view, but just generates SQLite SELECT
//! statements on demand.
// @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPrepareContext;
struct ClassDbView : NonCopyableClass
    {
private:
    ClassMap const* m_classMap;

public:
    explicit ClassDbView (ClassMap const& classMap) : m_classMap (&classMap) 
        {}
    virtual ~ClassDbView () {}

    BentleyStatus Generate (NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& preparedContext) const;
    };

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
            EndPoint (Utf8CP accessString, ColumnKind columnKin, ECN::ECValueCR value)
                : m_accessString (accessString), m_column (nullptr), m_value (value), m_columnKind (columnKin)
                {
                }
            EndPoint (Utf8CP accessString, ECDbSqlColumn const& column, ECN::ECValueCR value)
                : m_accessString (accessString), m_column (&column), m_value (value), m_columnKind (column.GetKind ())
                {
                }
            ECDbSqlColumn const* GetColumn () const { return m_column; }
            Utf8StringCR GetAccessString () const { return m_accessString; }
            ECN::ECValueCR GetValue () const { return m_value; }
            ColumnKind GetColumnKind () const { return m_columnKind; }
        };


    typedef std::vector<EndPoint const*> EndPoints;
    private:
        std::vector<std::unique_ptr<EndPoint>> m_orderedEndPoints;
        std::map<Utf8CP,EndPoint const*, CompareUtf8> m_endPointByAccessString;
        IClassMap const& m_classMap;

        PropertyMapSet (IClassMap const& classMap)
            :m_classMap (classMap)
            {
            }
    public:
        IClassMap const& GetClassMap () const;
        const EndPoints GetEndPoints () const;
        const EndPoints FindEndPoints (ColumnKind filter) const;
        const EndPoint* GetEndPointByAccessString (Utf8CP accessString) const
            {
            auto itor = m_endPointByAccessString.find (accessString);
            if (itor == m_endPointByAccessString.end ())
                return nullptr;

            return itor->second;
            }
        static BentleyStatus AddSystemEndPoint (PropertyMapSet& propertySet, IClassMap const& classMap, ColumnKind, ECN::ECValueCR value, ECDbSqlColumn const* column = nullptr);
        static PropertyMapSet::Ptr Create (IClassMap const& classMap);
    };

//=======================================================================================
//! Maps an ECClass to a DbTable
//! @remarks This is the base interface for querying information for a class mapping.
//! Populating a class map is not part of this, as that is limited to initialization code 
//! whereas the majority of code flows just needs to read out information from the class mapping.
// @bsiclass                                               Krischan.Eberle    02/2014
//+===============+===============+===============+===============+===============+======
struct IClassMap : NonCopyableClass
    {
    typedef std::vector<ECDbSqlTable*>& TableListR;
public:
    //! Class map type. Used to avoid dynamic_cast when casting to sub classes of ClassMap.
    enum class Type
        {
        Class,
        SecondaryTable,
        EmbeddedType,
        RelationshipEndTable,
        RelationshipLinkTable,
        Unmapped
        };

    //! Class map view. Depending on the view, a class map exposes different property maps.
    //! @remarks The view only differs for ECClasses which are domain classes and structs at the same time
    //! Furthermore, the view EmbeddedType is only used internally for executing secondary ECSqlStatements.
    enum class View
        {
        DomainClass, //!<Class of the mapping is viewed as domain class
        EmbeddedType //!<Class of the mapping is viewed as struct type embedded into another class
        };
  
private:
    virtual IClassMap const& _GetView (View classView) const = 0;
    virtual Type _GetClassMapType () const = 0;
    virtual ECN::ECClassCR _GetClass () const = 0;
    virtual ECN::ECClassId _GetParentMapClassId () const = 0;
    virtual PropertyMapCollection const& _GetPropertyMaps () const = 0;
    virtual TableListR _GetTables () const = 0;
    virtual ECDbMapStrategy const& _GetMapStrategy () const = 0;
    virtual ECDbMapCR _GetECDbMap () const = 0;
    virtual ClassDbView const& _GetDbView () const = 0;

protected:
    bool m_isECInstanceIdAutogenerationDisabled;

    IClassMap() : m_isECInstanceIdAutogenerationDisabled(false) {}

public:
    virtual ~IClassMap () {}

    IClassMap const& GetView (View classView) const;

    TableListR GetTables() const { return _GetTables(); }
    ECDbSqlTable& GetPrimaryTable() const { BeAssert(!GetTables().empty()); return *GetTables().front(); }
    ECDbSqlTable& GetJoinedTable() const { BeAssert(!GetTables().empty()); return *GetTables().back(); }
    bool IsMappedTo(ECDbSqlTable const& table) const { TableListR tables = GetTables(); return std::find(tables.begin(), tables.end(), &table) != tables.end(); }
    bool IsMappedToSingleTable() const { return GetTables().size() == 1; }

    IClassMap const* FindSharedTableRootClassMap() const;
    IClassMap const* FindClassMapOfParentOfJoinedTable() const;
    BentleyStatus GetPathToParentOfJoinedTable(std::vector<IClassMap const*>& path) const;
    IClassMap const* GetParentOfJoinedTable() const;
    PropertyMapCP GetPropertyMap (Utf8CP propertyName) const;

    //! Returns a collection of this class map's property maps.
    //! @return Collection of property maps of this class map
    PropertyMapCollection const& GetPropertyMaps () const;

    //! Returns the class maps of the classes derived from this class map's class.
    //! @return Derived classes class maps
    std::vector<IClassMap const*> GetDerivedClassMaps () const;

    //! Checks whether this class map contains a property map of type PropertyMapToTable.
    //! @return true, if the class map contains a PropertyMapToTable map. false otherwise.
    bool ContainsPropertyMapToTable () const;

    ECN::ECClassCR GetClass () const;
    ECN::ECClassId GetParentMapClassId () const;

    ECDbMapStrategy const& GetMapStrategy () const;
    ECDbMapCR GetECDbMap () const;
    ClassDbView const& GetDbView () const;
    Type GetClassMapType () const;
    bool IsECInstanceIdAutogenerationDisabled() const { return m_isECInstanceIdAutogenerationDisabled; }

    StorageDescription const& GetStorageDescription() const;
    bool IsRelationshipClassMap() const;
    bool HasJoinedTable() const;
    bool IsParentOfJoinedTable() const;
    bool MapsToStructArrayTable () const;
    static bool MapsToStructArrayTable(ECN::ECClassCR);
    Utf8String ToString () const;
    const Utf8String GetPersistedViewName() const;
    bool HasPersistedView() const;

    static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
    static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);
    static bool IsAnyClass (ECN::ECClassCR);
    virtual bool SupportECSql(ECSqlType ecsqlType, Utf8StringP reason = nullptr) const { BeAssert(false); return false;}


    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ColumnFactory : NonCopyableClass
    {
    struct Specification
        {
        enum class Strategy
            {
            Create,
            CreateOrReuse,
            CreateOrReuseSharedColumn, //! If no column avaliable will use NameBasedOnLetterFollowedByIntegerSequence to generate new name
            };
        enum class GenerateColumnNameOptions
            {
            NameBasedOnClassAndPropertyName,
            NameBasedOnPropertyNameAndPropertyId,
            NameBasedOnClassIdAndCaseSaveAccessString,
            NameBasedOnLetterFollowedByIntegerSequence, //! always default when CreateOrReuseSharedColumn is used
            NeverGenerate
            };

        private:
            PropertyMapR m_propertyMap;
            ECDbSqlColumn::Type m_columnType;
            ECDbSqlColumn::Constraint::Collation m_collation;
            GenerateColumnNameOptions m_generateColumnNameOptions;
            PersistenceType m_persistenceType;
            Utf8String m_accessString;
            Utf8String m_requestedColumnName;
            Strategy m_strategy;
            ColumnKind m_columnKind;
            bool m_isNotNull;
            bool m_isUnique;

        public:
            Specification (
                PropertyMapR propertyMap,
                Strategy stratgy = Strategy::CreateOrReuseSharedColumn,
                GenerateColumnNameOptions generateColumnNameOptions = GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence,
                Utf8CP columnName = nullptr,
                ECDbSqlColumn::Type columnType = ECDbSqlColumn::Type::Any,
                ColumnKind columnUserData = ColumnKind::DataColumn,
                PersistenceType persistenceType = PersistenceType::Persisted,
                Utf8CP accessStringPrefix = nullptr,
                bool isNotNull = false,
                bool isUnique = false,
                ECDbSqlColumn::Constraint::Collation collation = ECDbSqlColumn::Constraint::Collation::Default);

            PropertyMapCR GetPropertyMap () const { return m_propertyMap; }
            Utf8StringCR GetColumnName () const { return m_requestedColumnName; }
            Utf8StringCR GetAccessString () const { return m_accessString; }
            ECDbSqlColumn::Type GetColumnType () const { return m_columnType; }
            bool IsNotNull () const { return m_isNotNull; }
            bool IsUnique () const { return m_isUnique; }
            GenerateColumnNameOptions GetGenerateColumnNameOptions () const { return m_generateColumnNameOptions; }
            Strategy GetStrategy () const { return m_strategy; }
            ColumnKind GetColumnKind () const { return m_columnKind; }
            PersistenceType GetColumnPersistenceType () const { return m_persistenceType; }
            ECDbSqlColumn::Constraint::Collation GetCollation () const { return m_collation; }
        };

    private:
        ClassMapCR m_classMap;
        std::set<Utf8String, CompareIUtf8> columnsInUseSet;

        BentleyStatus ResolveColumnName (Utf8StringR resolvedColumName, Specification const&, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId, int retryCount) const;
        ECDbSqlColumn* ApplyCreateStrategy (Specification const&, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId);
        ECDbSqlColumn* ApplyCreateOrReuseStrategy (Specification const&, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId);
        ECDbSqlColumn* ApplyCreateOrReuseSharedColumnStrategy (Specification const&, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId);
        ECN::ECClassId GetPersistenceClassId (Specification const&) const;
        bool TryFindReusableSharedDataColumn (ECDbSqlColumn const*& reusableColumn, ECDbSqlTable const& table, ECDbSqlColumn::Constraint::Collation collation = ECDbSqlColumn::Constraint::Collation::Default) const;
        bool IsColumnInUse (Utf8CP columnFullName) const;
        bool IsColumnInUse (Utf8CP tableName, Utf8CP columnName) const;
        bool IsColumnInUse (ECDbSqlColumn const&) const;
        const Utf8String Encode (Utf8StringCR acessString) const;

    public:
        ColumnFactory (ClassMapCR);
        ~ColumnFactory (){}
        ECDbSqlTable & GetTable();
        void RegisterColumnInUse (ECDbSqlColumn const&);
        void Reset ();
        void Update ();
        ECDbSqlColumn* Configure (Specification const&, ECDbSqlTable&);
        ECDbSqlColumn* Configure (Specification const& specs);
    };
//=======================================================================================
//!Maps an ECClass to a DbTable
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMap : public IClassMap, RefCountedBase
    {
    private:
        ECDbMapCR                   m_ecDbMap;
        PropertyMapCollection       m_propertyMaps;
        mutable std::vector<ECDbSqlTable*> m_tables;
        ECDbMapStrategy             m_mapStrategy;
        bool                        m_isDirty;
        ECDbClassMapId              m_id;

    protected:
        ECN::ECClassCR              m_ecClass;
        ECN::ECClassId              m_parentMapClassId;
        std::unique_ptr<ClassDbView> m_dbView;
        ColumnFactory               m_columnFactory;

    private:
        BentleyStatus InitializeDisableECInstanceIdAutogeneration();

        //! Used to find an ECProperty from a propertyAccessString
        //! @param propertyAccessString (as used here) does not support access "inside" arrays, e.g. you can access a struct member inside an array of structs
        ECN::ECPropertyCP GetECProperty(ECN::ECClassCR ecClass, Utf8CP propertyAccessString);

        virtual MappingStatus _OnInitialized();
        virtual Type _GetClassMapType() const override;

    protected:
        ClassMap(ECN::ECClassCR, ECDbMapCR, ECDbMapStrategy, bool setIsDirty);

        virtual MappingStatus _MapPart1(SchemaImportContext&, ClassMapInfo const&, IClassMap const* parentClassMap);
        virtual MappingStatus _MapPart2(SchemaImportContext&, ClassMapInfo const&, IClassMap const* parentClassMap);
        virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ECDbClassMapInfo const&, IClassMap const* parentClassMap);
        virtual BentleyStatus _Save(std::set<ClassMap const*>& savedGraph); 
        

        MappingStatus AddPropertyMaps(ClassMapLoadContext&, IClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo, ClassMapInfo const* classMapInfo);
        void SetTable(ECDbSqlTable& newTable, bool append = false);
        virtual PropertyMapCollection const& _GetPropertyMaps() const;
        virtual TableListR _GetTables() const override { return m_tables; }
        virtual ECN::ECClassCR _GetClass() const override { return m_ecClass; }
        virtual ECDbMapStrategy const& _GetMapStrategy() const override { return m_mapStrategy; }
        virtual ECDbMapCR _GetECDbMap() const override { return m_ecDbMap; }
        virtual ECN::ECClassId _GetParentMapClassId() const override { return m_parentMapClassId; }
        virtual IClassMap const& _GetView(View classView) const override { return *this; };
        virtual ClassDbView const& _GetDbView() const override { return *m_dbView; }
        PropertyMapCollection& GetPropertyMapsR();

        ECDbSchemaManagerCR Schemas() const;
    public:
        static ClassMapPtr Create(ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new ClassMap(ecClass, ecdbMap, mapStrategy, setIsDirty); }
        
        //! Called when loading an existing class map from the ECDb file 
        BentleyStatus Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap) { return _Load(loadGraph, ctx, mapInfo, parentClassMap); }

        //! Called during schema import when creating the class map from the imported ECClass 
        MappingStatus Map(SchemaImportContext&, ClassMapInfo const& classMapInfo);

        ECDbSqlColumn* FindOrCreateColumnForProperty(ClassMapCR, ClassMapInfo const*, PropertyMapR,
                                                     Utf8CP requestedColumnName, ECN::PrimitiveType, bool nullable, bool unique, ECDbSqlColumn::Constraint::Collation, Utf8CP accessStringPrefix);

        PropertyMapCP GetECInstanceIdPropertyMap() const;
        bool TryGetECInstanceIdPropertyMap(PropertyMapPtr& ecIstanceIdPropertyMap) const;

        BentleyStatus CreateUserProvidedIndexes(SchemaImportContext&, bvector<ClassIndexInfoPtr> const&) const;

        bool IsDirty() const { return m_isDirty; }
        ECDbClassMapId GetId() const { return m_id; }
        void SetId(ECDbClassMapId id) { m_id = id; }
        BentleyStatus Save(std::set<ClassMap const*>& savedGraph) { return _Save(savedGraph); }

        ColumnFactory const& GetColumnFactory() const { return m_columnFactory; }
        ColumnFactory& GetColumnFactoryR() { return m_columnFactory; }

    };

END_BENTLEY_SQLITE_EC_NAMESPACE