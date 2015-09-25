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
#include "ClassMapInfo.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


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
            ECDbKnownColumns m_knownColumn;
        public:
            EndPoint (Utf8CP accessString, ECDbKnownColumns knownColumn, ECN::ECValueCR value)
                : m_accessString (accessString), m_column (nullptr), m_value (value), m_knownColumn (knownColumn)
                {
                }
            EndPoint (Utf8CP accessString, ECDbSqlColumn const& column, ECN::ECValueCR value)
                : m_accessString (accessString), m_column (&column), m_value (value), m_knownColumn (column.GetKnownColumnId ())
                {
                }
            ECDbSqlColumn const* GetColumn () const { return m_column; }
            Utf8StringCR GetAccessString () const { return m_accessString; }
            ECN::ECValueCR GetValue () const { return m_value; }
            ECDbKnownColumns GetKnownColumnId () const { return m_knownColumn; }
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
        const EndPoints FindEndPoints (ECDbKnownColumns filter) const;
        const EndPoint* GetEndPointByAccessString (Utf8CP accessString) const
            {
            auto itor = m_endPointByAccessString.find (accessString);
            if (itor == m_endPointByAccessString.end ())
                return nullptr;

            return itor->second;
            }
        static BentleyStatus AddSystemEndPoint (PropertyMapSet& propertySet, IClassMap const& classMap, ECDbKnownColumns knownColumnId, ECN::ECValueCR value, ECDbSqlColumn const* column = nullptr);
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
    bool m_isECInstanceIdAutogenerationDisabled;

    virtual IClassMap const& _GetView (View classView) const = 0;
    virtual Type _GetClassMapType () const = 0;
    virtual ECN::ECClassCR _GetClass () const = 0;
    virtual ECN::ECClassId _GetParentMapClassId () const = 0;
    virtual PropertyMapCollection const& _GetPropertyMaps () const = 0;
    virtual ECDbSqlTable& _GetTable () const = 0;
    virtual ECDbMapStrategy const& _GetMapStrategy () const = 0;
    virtual ECDbMapCR _GetECDbMap () const = 0;
    virtual ClassDbView const& _GetDbView () const = 0;

protected:
    IClassMap() :m_isECInstanceIdAutogenerationDisabled(false) {}

public:
    virtual ~IClassMap () {}

    IClassMap const& GetView (View classView) const;

    PropertyMapCP GetPropertyMap (Utf8CP propertyName) const;

    //! Returns a collection of this class map's property maps.
    //! @return Collection of property maps of this class map
    PropertyMapCollection const& GetPropertyMaps () const;

    //! Returns the class maps of the classes derived from this class map's class.
    //! @eturn Derived classes class maps
    std::vector<IClassMap const*> GetDerivedClassMaps () const;
    ECDbSqlTable& GetTable () const;

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
    bool IsMappedToSecondaryTable () const;
    bool IsRelationshipClassMap () const;
    bool IsAbstractECClass () const;

    Utf8String ToString () const;
    const std::set<ECDbSqlTable const*> GetSecondaryTables () const
        {
        std::set<ECDbSqlTable const*> secondaryTables;
        GetPropertyMaps ().Traverse ([&secondaryTables,this] (TraversalFeedback& feedback, PropertyMapCP propMap)
            {
            if (!propMap->IsVirtual ())
                {
                if (auto column = propMap->GetFirstColumn ())
                    {
                    if (&column->GetTable () != &GetTable ())
                        {
                        if (secondaryTables.find (&column->GetTable ()) == secondaryTables.end ())
                            secondaryTables.insert (&column->GetTable ());
                        }
                    }
                }
            feedback = TraversalFeedback::Next;
            }, true);

        return secondaryTables;
        }

    static bool IsMapToSecondaryTableStrategy (ECN::ECClassCR ecClass);
    static bool IsAbstractECClass (ECN::ECClassCR ecClass);
    static bool IsAnyClass (ECN::ECClassCR ecClass);
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ColumnFactory : NonCopyableClass
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
            ECDbSqlColumn::Constraint::Collation m_collation;
            GenerateColumnNameOptions m_generateColumnNameOptions;
            PersistenceType m_persistenceType;
            Utf8String m_accessString;
            Utf8String m_requestedColumnName;
            Strategy m_strategy;
            ECDbKnownColumns m_knownColumnId;
            bool m_isNotNull;
            bool m_isUnique;

        public:
            Specification (
                PropertyMapR propertyMap,
                Strategy stratgy = Strategy::CreateOrReuseSharedColumn,
                GenerateColumnNameOptions generateColumnNameOptions = GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence,
                Utf8CP columnName = nullptr,
                ECDbSqlColumn::Type columnType = ECDbSqlColumn::Type::Any,
                ECDbKnownColumns columnUserData = ECDbKnownColumns::DataColumn,
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
            ECDbKnownColumns GetKnownColumnId () const { return m_knownColumnId; }
            PersistenceType GetColumnPersistenceType () const { return m_persistenceType; }
            ECDbSqlColumn::Constraint::Collation GetCollation () const { return m_collation; }
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
        bool FindReusableSharedDataColumns (std::vector<ECDbSqlColumn const*>& columns, ECDbSqlTable const& table, ECDbSqlColumn::Constraint::Collation collation = ECDbSqlColumn::Constraint::Collation::Default, SortBy sortby = SortBy::None) const;
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
    private:
        ECDbMapCR                   m_ecDbMap;
        PropertyMapCollection       m_propertyMaps;
        ECDbSqlTable*               m_table;
        ECDbMapStrategy             m_mapStrategy;
        bool                        m_isDirty;
        ECDbClassMapId              m_id;

    protected:
        ECN::ECClassCR              m_ecClass;
        ECN::ECClassId              m_parentMapClassId;
        std::unique_ptr<ClassDbView> m_dbView;
        ColumnFactory               m_columnFactory;
    private:
        BentleyStatus ProcessStandardKeySpecifications(ClassMapInfo const&);

        //! Used to find an ECProperty from a propertyAccessString
        //! @param propertyAccessString (as used here) does not support access "inside" arrays, e.g. you can access a struct member inside an array of structs
        ECN::ECPropertyCP GetECProperty (ECN::ECClassCR ecClass, Utf8CP propertyAccessString);

        virtual MapStatus _OnInitialized ();
        virtual Type _GetClassMapType () const override;

    protected:
        ClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);

        virtual MapStatus _InitializePart1 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap);
        virtual MapStatus _InitializePart2 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap);
        virtual BentleyStatus _Save (std::set<ClassMap const*>& savedGraph); virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap);

        MapStatus AddPropertyMaps (IClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo, ClassMapInfo const* classMapInfo);
        void SetTable (ECDbSqlTable* newTable) { m_table = newTable; }
        virtual PropertyMapCollection const& _GetPropertyMaps () const;
        virtual ECDbSqlTable& _GetTable () const override { return *m_table; }
        virtual ECN::ECClassCR _GetClass () const override { return m_ecClass; }
        virtual ECDbMapStrategy const& _GetMapStrategy () const override { return m_mapStrategy; }
        virtual ECDbMapCR _GetECDbMap () const override { return m_ecDbMap; }
        virtual ECN::ECClassId _GetParentMapClassId () const override { return m_parentMapClassId; }
        virtual IClassMap const& _GetView (View classView) const override { return *this; };
        virtual ClassDbView const& _GetDbView () const override { return *m_dbView; }
        PropertyMapCollection& GetPropertyMapsR ();

        ECDbSchemaManagerCR Schemas () const;
    public:
        static ClassMapPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new ClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty); }
        //! Builds the list of PropertyMaps for this ClassMap
        //! @param  classMapInfo This will contain information cleaned from ClassMap custom attribute
        MapStatus Initialize (ClassMapInfo const& classMapInfo);
 
    
    //! Used when finding/creating DbColumns that are mapped to ECProperties of the ECClass
    //! If there is a name conflict, it may call the PropertyMap's SetColumnName method to resolve it
    ECDbSqlColumn* FindOrCreateColumnForProperty
        (
        ClassMapCR classMap,
        ClassMapInfo const* classMapInfo,
        PropertyMapR propertyMap, 
        Utf8CP requestedColumnName, 
        ECN::PrimitiveType primitiveType, 
        bool nullable, 
        bool unique, 
        ECDbSqlColumn::Constraint::Collation, 
        Utf8CP accessStringPrefix);


    PropertyMapCP GetECInstanceIdPropertyMap () const;
    bool TryGetECInstanceIdPropertyMap (PropertyMapPtr& ecIstanceIdPropertyMap) const;
    
    BentleyStatus CreateUserProvidedIndices(ClassMapInfo const&) const;

    bool IsDirty () const { return m_isDirty; }
    ECDbClassMapId GetId () const { return m_id; }
    void SetId (ECDbClassMapId id) { m_id = id; }
    BentleyStatus Save (std::set<ClassMap const*>& savedGraph) { return _Save (savedGraph); }
    BentleyStatus Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const*  parentClassMap) { return _Load (loadGraph, mapInfo, parentClassMap); }

    ColumnFactory const& GetColumnFactory () const { return m_columnFactory; }
    ColumnFactory& GetColumnFactoryR () { return m_columnFactory; }
    };


/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct MappedTable : public RefCountedBase
{
private:
    ECDbSqlTable& m_table;
    bool m_generatedClassIdColumn;
    bvector<ClassMapCP> m_classMaps;
    ECDbMapR m_ecDbMap;

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
    BentleyStatus FinishTableDefinition();
    BentleyStatus AddClassMap(ClassMapCR classMap);
    bool IsFinished() const { return m_generatedClassIdColumn;}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE