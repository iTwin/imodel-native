/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "PropertyMap.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


struct NativeSqlBuilder;

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
//! Maps an ECClass to a DbTable
//! @remarks This is the base interface for querying information for a class mapping.
//! Populating a class map is not part of this, as that is limited to initialization code 
//! whereas the majority of code flows just needs to read out information from the class mapping.
// @bsiclass                                               Krischan.Eberle    02/2014
//+===============+===============+===============+===============+===============+======
struct IClassMap : NonCopyableClass
    {
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

    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      01/2014
    //+===============+===============+===============+===============+===============+======
    struct NativeSqlConverter : NonCopyableClass
        {
    private:
        virtual ECSqlStatus _GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const = 0;

    protected:
        explicit NativeSqlConverter () {}

    public:
        virtual ~NativeSqlConverter () {}

        ECSqlStatus GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const;
        };

private:
    virtual IClassMap const& _GetView (View classView) const = 0;
    virtual Type _GetClassMapType () const = 0;
    virtual ECN::ECClassCR _GetClass () const = 0;
    virtual ECN::ECClassId _GetParentMapClassId () const = 0;
    virtual PropertyMapCollection const& _GetPropertyMaps () const = 0;
    virtual ECDbSqlTable& _GetTable () const = 0;
    virtual MapStrategy _GetMapStrategy () const = 0;
    virtual ECDbMapCR _GetECDbMap () const = 0;
    virtual NativeSqlConverter const& _GetNativeSqlConverter () const = 0;
    virtual ClassDbView const& _GetDbView () const = 0;

public:
    virtual ~IClassMap () {}

    IClassMap const& GetView (View classView) const;

    PropertyMapCP GetPropertyMap (WCharCP propertyName) const;

    //! Returns a collection of this class map's property maps.
    //! @return Collection of property maps of this class map
    PropertyMapCollection const& GetPropertyMaps () const;

    //! Returns the class maps of the classes derived from this class map's class.
    //! @eturn Derived classes class maps
    std::vector<IClassMap const*> GetDerivedClassMaps () const;
    ECDbSqlTable& GetTable () const;

    //! Returns the tables to which this class map - and all derived classes' class maps if
    void GetTables (bset<ECDbSqlTable const*>& tables, bool includeDerivedClasses) const;

    //! Checks whether this class map contains a property map of type PropertyMapToTable.
    //! @return true, if the class map contains a PropertyMapToTable map. false otherwise.
    bool ContainsPropertyMapToTable () const;

    ECN::ECClassCR GetClass () const;
    ECN::ECClassId GetParentMapClassId () const;

    MapStrategy GetMapStrategy () const;
    ECDbMapCR GetECDbMap () const;
    NativeSqlConverter const& GetNativeSqlConverter () const;
    ClassDbView const& GetDbView () const;
    Type GetClassMapType () const;

    bool IsUnmapped () const;
    bool IsMappedToSecondaryTable () const;
    bool IsRelationshipClassMap () const;
    bool IsAbstractECClass () const;

    Utf8String ToString () const;

    static bool IsMapToSecondaryTableStrategy (ECN::ECClassCR ecClass);
    static bool IsDoNotMapStrategy (MapStrategy mapStrategy);
    static bool IsAbstractECClass (ECN::ECClassCR ecClass);
    static bool IsAnyClass (ECN::ECClassCR ecClass);
    };

struct ClassMap;
//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct  ColumnFactory : NonCopyableClass
    {
    enum class SortBy
        {
        None,
        LeastUsedColumn,
        MostUsedColumn,
        LeftToRightColumnOrderInTable
        };
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
            ECDbSqlColumn::Constraint::Collate m_collate;
            GenerateColumnNameOptions m_generateColumnNameOptions;
            PersistenceType m_persistenceType;
            Utf8String m_accessString;
            Utf8String m_requestedColumnName;
            Strategy m_strategy;
            uint32_t m_columnUserData;
            bool m_isNotNull;
            bool m_isUnique;

        public:
            Specification (
                PropertyMapR propertyMap,
                Strategy stratgy = Strategy::CreateOrReuseSharedColumn,
                GenerateColumnNameOptions generateColumnNameOptions = GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence,
                Utf8CP columnName = nullptr,
                ECDbSqlColumn::Type columnType = ECDbSqlColumn::Type::Any,
                uint32_t columnUserData = ECDbDataColumn,
                PersistenceType persistenceType = PersistenceType::Persisted,
                Utf8CP accessStringPrefix = nullptr,
                bool isNotNull = false,
                bool isUnique = false,
                ECDbSqlColumn::Constraint::Collate collate = ECDbSqlColumn::Constraint::Collate::Default);

            PropertyMapCR GetPropertyMap () const { return m_propertyMap; }
            Utf8StringCR GetColumnName () const { return m_requestedColumnName; }
            Utf8StringCR GetAccessString () const { return m_accessString; }
            ECDbSqlColumn::Type GetColumnType () const { return m_columnType; }
            bool IsNotNull () const { return m_isNotNull; }
            bool IsUnique () const { return m_isUnique; }
            GenerateColumnNameOptions GetGenerateColumnNameOptions () const { return m_generateColumnNameOptions; }
            Strategy GetStrategy () const { return m_strategy; }
            uint32_t GetColumnUserDate () const { return m_columnUserData; }
            PersistenceType GetColumnPersistenceType () const { return m_persistenceType; }
            ECDbSqlColumn::Constraint::Collate GetCollate () const { return m_collate; }
        };

    private:
        ClassMapCR m_classMap;
        std::set<Utf8String, CompareIUtf8> columnsInUseSet;

        static void  SortByLeastUsedColumnFirst (std::vector<ECDbSqlColumn const*>& columns);
        static void  SortByMostUsedColumnFirst (std::vector<ECDbSqlColumn const*>& columns);
        static void  SortByColumnOrderInTable (std::vector<ECDbSqlColumn const*>& columns);

        BentleyStatus ResolveColumnName (Utf8StringR resolvedColumName, Specification const& specifications, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId, int retryCount) const;
        ECDbSqlColumn* ApplyCreateStrategy (Specification const& specifications, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId);
        ECDbSqlColumn* ApplyCreateOrReuseStrategy (Specification const& specifications, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId);
        ECDbSqlColumn* ApplyCreateOrReuseSharedColumnStrategy (Specification const& specifications, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId);
        ECN::ECClassId GetPersistenceClassId (Specification const& specifications) const;
        bool FindReusableSharedDataColumns (std::vector<ECDbSqlColumn const*>& columns, ECDbSqlTable const& table, ECDbSqlColumn::Constraint::Collate collate = ECDbSqlColumn::Constraint::Collate::Default, SortBy sortby = SortBy::None) const;
        bool IsColumnInUse (Utf8CP columnFullName) const;
        bool IsColumnInUse (Utf8CP tableName, Utf8CP columnName) const;
        bool IsColumnInUse (ECDbSqlColumn const& column) const;
        const Utf8String Encode (Utf8StringCR acessString) const;
    public:
        ColumnFactory (ClassMapCR classMap);
        ~ColumnFactory (){}
        ECDbSqlTable & GetTable ();
        void RegisterColumnInUse (ECDbSqlColumn const& column);
        void Reset ();
        void Update ();
        ECDbSqlColumn* Configure (Specification const& specifications, ECDbSqlTable& targetTable);
        ECDbSqlColumn* Configure (Specification const& specifications);
    };
//=======================================================================================
//!Maps an ECClass to a DbTable
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMap : public IClassMap, RefCountedBase
{
protected:
    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      01/2014
    //+===============+===============+===============+===============+===============+======
    struct NativeSqlConverterImpl : NativeSqlConverter
        {
    private:
        ClassMapCR m_classMap;
       
    protected:
        virtual ECSqlStatus _GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const override;

        ClassMapCR GetClassMap () const { return m_classMap; }
    public:
        explicit NativeSqlConverterImpl (ClassMapCR classMap);
        virtual ~NativeSqlConverterImpl () {}
        };


private:
    ECDbMapCR                   m_ecDbMap;
    PropertyMapCollection       m_propertyMaps;
    ECDbSqlTable*               m_table;
    MapStrategy                 m_mapStrategy;
    bool                        m_isDirty;
    bvector<ClassIndexInfoPtr>  m_indexes;
    bool                        m_useSharedColumnStrategy;
    ECDbClassMapId              m_id;
protected:
    ECN::ECClassCR              m_ecClass;
    ECN::ECClassId              m_parentMapClassId;
    mutable std::unique_ptr<NativeSqlConverter> m_nativeSqlConverter;
    std::unique_ptr<ClassDbView> m_dbView;
    ColumnFactory               m_columnFactory;
private:
    MapStatus ProcessIndices (ClassMapInfoCR classMapInfo);
    void ProcessStandardKeySpecifications (ClassMapInfoCR mapInfo);
    void SetUserProvidedIndex (bvector<ClassIndexInfoPtr> const& indexes)
        {
        m_indexes = indexes;
        }
    //! Used to find an ECProperty from a propertyAccessString
    //! @param propertyAccessString (as used here) does not support access "inside" arrays, e.g. you can access a struct member inside an array of structs
    ECN::ECPropertyCP GetECProperty(ECN::ECClassCR ecClass, WCharCP propertyAccessString);

    virtual MapStatus _OnInitialized ();
    virtual Type _GetClassMapType () const override;

protected:
    ClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty);

    virtual MapStatus _InitializePart1 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap);
    virtual MapStatus _InitializePart2 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap);
    virtual BentleyStatus _Save (std::set<ClassMap const*>& savedGraph);
    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap);

    MapStatus AddPropertyMaps (IClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo);
    void SetTable (ECDbSqlTable* newTable) { m_table = newTable; }
    virtual PropertyMapCollection const& _GetPropertyMaps () const;
    virtual ECDbSqlTable& _GetTable () const override { return *m_table; }
    virtual NativeSqlConverter const& _GetNativeSqlConverter () const override;
    virtual ECN::ECClassCR _GetClass () const override { return m_ecClass; }
    virtual MapStrategy _GetMapStrategy () const override { return m_mapStrategy; }
    virtual ECDbMapCR _GetECDbMap () const override { return m_ecDbMap; }
    virtual ECN::ECClassId _GetParentMapClassId () const override { return m_parentMapClassId; }
    virtual IClassMap const& _GetView (View classView) const override { return *this; };
    virtual ClassDbView const& _GetDbView () const override { return *m_dbView; }


    PropertyMapCollection& GetPropertyMapsR ();
    
    ECDbSchemaManagerCR GetSchemaManager () const;
public:
    static ClassMapPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, MapStrategy mapStrategy, bool setIsDirty) { return new ClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty); }
    //! Builds the list of PropertyMaps for this ClassMap
    //! @param  classMapInfo This will contain information cleaned from ECDbClassHint
    MapStatus Initialize (ClassMapInfoCR classMapInfo);

    //! Used when finding/creating DbColumns that are mapped to ECProperties of the ECClass
    //! If there is a name conflict, it may call the PropertyMap's SetColumnName method to resolve it
    ECDbSqlColumn* FindOrCreateColumnForProperty
        (
        ClassMapCR classMap,
        PropertyMapR propertyMap, 
        Utf8CP requestedColumnName, 
        ECN::PrimitiveType primitiveType, 
        bool nullable, 
        bool unique, 
        ECDbSqlColumn::Constraint::Collate collate, 
        Utf8CP accessStringPrefix);


    PropertyMapCP GetECInstanceIdPropertyMap () const;
    bool TryGetECInstanceIdPropertyMap (PropertyMapPtr& ecIstanceIdPropertyMap) const;
    
    bool IsDirty () const { return m_isDirty; }
    ECDbClassMapId GetId () const { return m_id; }
    BentleyStatus Save (std::set<ClassMap const*>& savedGraph) { return _Save (savedGraph); }
    BentleyStatus Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const*  parentClassMap) { return _Load (loadGraph, mapInfo, parentClassMap); }

    void CreateIndices ();
    ColumnFactory const& GetColumnFactory () const { return m_columnFactory; }
    ColumnFactory& GetColumnFactoryR () { return m_columnFactory; }
    };


/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct MappedTable : public RefCountedBase
{
private:
    ECDbSqlTable&                    m_table;
    bool                        m_generatedClassId;
    bvector<ClassMapCP>          m_classMaps;
    ECDbMapR                    m_ecDbMap;

    //~MappedTable(void);
    MappedTable(MappedTableCR); // hide copy constructor
    MappedTable (ECDbMapR ecDbMap, ClassMapCR classMap);

public:
    static MappedTablePtr Create (ECDbMapR ecDbMap, ClassMapCR classMap);

    ECDbSqlTable&    GetTable() const {return m_table;}
    bool        HasSingleClass() const {return m_classMaps.size() == 1;}
    const bvector<ClassMapCP>&      GetClassMaps() const { return m_classMaps;}
    //! FinishTableDefinition should be called once all ECClasses have been added to this cluster.
    //! FinishTableDefinition will ensure there is a primary key for the table, and will add a
    //! ClassId column, if necessary
    //! It must be called prior to calling ECDbMap::CreateTableInDb()
    StatusInt   FinishTableDefinition();
    StatusInt   AddClassMap (ClassMapCR classMap);
    bool        IsFinished() const { return m_generatedClassId;}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE