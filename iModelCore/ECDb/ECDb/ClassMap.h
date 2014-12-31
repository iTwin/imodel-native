/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "PropertyMap.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsienum                                                
// @remarks See @ref ECDbSchemaPersistence to find how these enum values map to actual 
// persisted values in the Db. 
//+===============+===============+===============+===============+===============+======
enum class MapStrategy
    {
    // This first group of strategies no ramifications for subclasses
    NoHint = 0,         // Use default rules, which may include inheriting strategy of parent
    DoNotMap,           // Skip this one, but child ECClasses may still be mapped
    TableForThisClass,  // Put this class in a table, but do not pass the strategy along to child ECClasses 
    // Only DoNotMap and TableForThisClass are valid default strategies

    // These strategies are directly inherited, except for TablePerHierarchy, which causes its children to use InParentTable
    // They are listed in order of priority (when it comes to conflicts with/among base ECClasses)
    TablePerHierarchy,  // This class and all child ECClasses stored in one table
    InParentTable,      // Assigned by system for subclasses of ECClasses using TablePerHierarchy
    TablePerClass,      // Put each class in its own table (including child ECClasses
    DoNotMapHierarchy,  // Also don't map children (unless they are reached by a different inheritance pathway) 
    SharedTableForThisClass, // TableName must be provided. 
    // These strategies are applicable only to relationships
    RelationshipSourceTable,     // Store the relationship in the table in which the source class(es) are stored 
    RelationshipTargetTable,     // Store the relationship in the table in which the target class(es) are stored 
    };


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
    virtual DbTableR _GetTable () const = 0;
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
    DbTableR GetTable () const;

    //! Returns the tables to which this class map - and all derived classes' class maps if
    void GetTables (bset<DbTableCP>& tables, bool includeDerivedClasses) const;

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
    DbTableP                    m_table;
    MapStrategy                 m_mapStrategy;
    bool                        m_isDirty;
    bvector<ClassIndexInfoPtr> m_indexes;
protected:
    ECN::ECClassCR              m_ecClass;
    ECN::ECClassId              m_parentMapClassId;
    mutable std::unique_ptr<NativeSqlConverter> m_nativeSqlConverter;
    std::unique_ptr<ClassDbView> m_dbView;

private:
    PropertyMapCP GetPropertyMapForColumnName (Utf8CP columnName) const;
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

    MapStatus AddPropertyMaps (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap);
    void SetTable (DbTableP newTable) { m_table = newTable; }
    virtual PropertyMapCollection const& _GetPropertyMaps () const;
    virtual DbTableR _GetTable () const override { return *m_table; }
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
    DbColumnPtr FindOrCreateColumnForProperty(PropertyMapR propertyMap, Utf8CP requestedColumnName, ECN::PrimitiveType primitiveType, 
                                               bool nullable, bool unique, Collate collate);


    PropertyMapCP GetECInstanceIdPropertyMap () const;
    bool TryGetECInstanceIdPropertyMap (PropertyMapPtr& ecIstanceIdPropertyMap) const;
    
    bool IsDirty () const { return m_isDirty; }

    DbResult Save (bool includeFullGraph);
    void CreateIndices ();

    //! @deprecated: Remove once ECPersistence is being removed
    //! Adds parameter bindings to the supplied vector
    BentleyStatus GenerateParameterBindings (Bindings& parameterBindings, int firstParameterIndex) const;
    };


/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct MappedTable : public RefCountedBase
{
private:
    DbTableR                    m_table;
    bool                        m_generatedClassId;
    bvector<ClassMapCP>          m_classMaps;
    ECDbMapR                    m_ecDbMap;

    //~MappedTable(void);
    MappedTable(MappedTableCR); // hide copy constructor
    MappedTable (ECDbMapR ecDbMap, ClassMapCR classMap);

public:
    static MappedTablePtr Create (ECDbMapR ecDbMap, ClassMapCR classMap);

    DbTableR    GetTable() const {return m_table;}
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