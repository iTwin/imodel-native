/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECDbSystemSchemaHelper.h"
#include "ECSql/NativeSqlBuilder.h"
#include "InstanceInserter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


enum class TraversalFeedback
    {
    Cancel, //! cancel traversal altogether and return
    NextSibling, //! donot traverse children of current node instead go with next sibling
    Next //! if there is children of current node process them first and then go to nextr sibling
    };

//=======================================================================================
// Represents an iterable collection of property maps
// @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapCollection : NonCopyableClass
    {
public:
    typedef std::vector<PropertyMapCP>::const_iterator const_iterator;

private:
    typedef bmap<WCharCP, PropertyMapPtr, CompareWChar> PropertyMapsByAccessString;

    PropertyMapsByAccessString m_dictionary;
    std::vector<PropertyMapCP> m_orderedCollection;

    bool TryGetPropertyMap (PropertyMapPtr& propertyMap, bvector<WString>::const_iterator& propertyAccessStringTokenIterator, bvector<WString>::const_iterator& propertyAccessStringTokenEndIterator) const;
    bool TryGetPropertyMapNonRecursively (PropertyMapPtr& propertyMap, WCharCP propertyAccessString) const;

    static void Traverse (std::set<PropertyMapCollection const*>& doneList, PropertyMapCollection const& childPropMaps, std::function<void (TraversalFeedback&, PropertyMapCP)> const& nodeOperation, bool recursive);

public:
    PropertyMapCollection ();
    ~PropertyMapCollection () {}
    PropertyMapCollection (PropertyMapCollection&& rhs);

    PropertyMapCollection& operator= (PropertyMapCollection&& rhs);

    void AddPropertyMap (PropertyMapPtr const& propertyMap);
    void AddPropertyMap (WCharCP propertyAccessString, PropertyMapPtr const& propertyMap);

    size_t Size () const;
    bool IsEmpty () const;

    bool TryGetPropertyMap (PropertyMapCP& propertyMap, WCharCP propertyAccessString, bool recursive = false) const;
    bool TryGetPropertyMap (PropertyMapPtr& propertyMap, WCharCP propertyAccessString, bool recursive = false) const;

    void Traverse (std::function<void (TraversalFeedback& feedback, PropertyMapCP propMap)> const& nodeOperation, bool recursive) const;

    const_iterator begin () const;
    const_iterator end () const;
    };


/*---------------------------------------------------------------------------------------
* Abstract class for property mapping
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMap : RefCountedBase, NonCopyableClass
{
private:
    virtual NativeSqlBuilder::List _ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType) const;

protected:
    ECN::ECPropertyCR       m_ecProperty;
    PropertyMapCP           m_parentPropertyMap;
    WString                 m_propertyAccessString; // We need to own this for nested property in embedded struct as they are dynamically generated
    PropertyMapCollection   m_children;

    PropertyMap (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

    virtual bool _IsVirtual () const;
    virtual bool _IsUnmapped () const;

    //! Checks whether this PropertyMap maps to a column of the given name
    virtual bool _MapsToColumn (Utf8CP columnName) const;

    //! @see PropertyMap::GetColumns
    virtual void _GetColumns(DbColumnList& columns) const;

    //! @see PropertyMap::GetColumnBaseName
    virtual Utf8CP _GetColumnBaseName() const;

    //! @see PropertyMap::SetColumnBaseName
    virtual void _SetColumnBaseName(Utf8CP columnName) { };

    //! Make sure our table has the necessary columns, if any
    virtual MapStatus _FindOrCreateColumnsInTable (ClassMapR classMap);

    virtual NativeSqlBuilder::List _ToNativeSql (ECDbR ecdb, DbTableCR table) const;

    virtual ECN::ECPropertyId _GetECPropertyIdForPersistence (ECN::ECClassId relativeToECClassId, ECDbR db) const;

    virtual PropertyMapToColumnCP _GetAsPropertyMapToColumn () const { return nullptr; }
    virtual PropertyMapToTableCP _GetAsPropertyMapToTable () const { return nullptr; }
    virtual PropertyMapArrayOfPrimitivesCP _GetAsPropertyMapArrayOfPrimitives () const { return nullptr; }

    virtual bool _IsECInstanceIdPropertyMap () const;
    virtual bool _IsSystemPropertyMap () const;

    //! For debugging and logging
    virtual WString _ToString() const;

    //********* DEPRECATED. Will be removed once ECPersistence is removed ******************
    //! The default implementation adds a "dummy" placeholder binding with m_sqlIndex = BINDING_NotBound (-1). It is needed to handle cases where the requested property
    //! is not handled (at the time of writing IGeometry properties were not handled) or arrays of structs (which are handled differently, at the time of writing)
    virtual void _AddBindings (BindingsR bindings, UInt32 propertyIndex, int& sqlIndex, ECN::ECEnablerCR enabler) const;

    //! Bind an ECProperty value from the ecInstance to the statement
    //! @param iBinding will be incremented by 1 or more. It is an index into parameterBindings
    //! @param parameterBindings holds bindings for all parameters relevant for the statement
    //! @param ecInstance holds values that are to be bound to the statement
    virtual DbResult _Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance) const;

public:
    virtual ~PropertyMap () {}
    ECN::ECPropertyId GetECPropertyIdForPersistence (ECN::ECClassId relativeToECClassId, ECDbR db) const { return _GetECPropertyIdForPersistence (relativeToECClassId, db); }
    PropertyMapToColumnCP GetAsPropertyMapToColumn () const {return _GetAsPropertyMapToColumn ();}
    PropertyMapToTableCP GetAsPropertyMapToTable () const {return _GetAsPropertyMapToTable();}
    PropertyMapArrayOfPrimitivesCP GetAsPropertyMapArrayOfPrimitives () const {return _GetAsPropertyMapArrayOfPrimitives();}
    ECN::ECPropertyCR GetProperty () const;
    PropertyMapCP GetParent () const { return m_parentPropertyMap; }
    static UInt32 GetPropertyIndex (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty); //needs to take an enabler, not a class
    
    PropertyMapCollection const& GetChildren () const { return m_children; }

    //! Gets a value indicating whether this property map is a virtual mapping, i.e. maps
    //! to a virtual DbColumn. A virtual DbColumn does not exist in a real table, but might
    //! be used as column aliases in views.
    //! @return true if property map is virtual, false otherwise
    bool IsVirtual () const;

    //! Gets a value indicating whether the property of this property map is not mapped to a database column.
    //! @return true if the property is not mapped, false otherwise
    bool IsUnmapped () const;

    //! Checks whether this PropertyMap maps to a column of the given name
    bool MapsToColumn (Utf8CP columnName) const;

    //! Gets the columns (if any) mapped to this property
    void GetColumns(DbColumnList& columns) const;

    //! Gets the first column if any
    DbColumnCP GetFirstColumn() const;

    //! For properties that map to columns, the name of the column... or 'base' name in case of a multi-column DPoint2d, 
    //! e.g. "origin", when the actual columns are origin.X and origin.Y
    //! @return nullptr if there is no relevant column, or column names does not differ from the default
    Utf8CP GetColumnBaseName() const;

    //! For cases where a column name must be remapped by the system due to a conflict
    void SetColumnBaseName(Utf8CP columnName);

    //! Generates the native SQL snippets from the columns related to this property map.
    //! SQL generation depends on various properties of the property map (e.g whether the property map is virtual)
    //! and the ECSQL type. So the result of this method is not always just the column name(s).
    //! @param[in] classIdentifier Class name or class alias which will be prepended to the sql snippet, e.g.
    //!                            <classidentifier>.<columnname>
    //! @param[in] ecsqlType ECSQL type for which the native SQL is generated. SQL generation depends on the ECSQL type
    //!            which is why this parameter is needed
    //! @return List of native SQL snippets, one snippet per column this PropertyMap maps to.
    NativeSqlBuilder::List ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType) const;

    NativeSqlBuilder::List ToNativeSql (ECDbR ecdb, DbTableCR table) const;

    //! Saves the base column name, if it differs from the property name
    BeSQLite::DbResult Save(ECDbR ecdb) const;

    WCharCP GetPropertyAccessString () const;

    //! Make sure our table has the necessary columns, if any
    MapStatus FindOrCreateColumnsInTable (ClassMapR classMap);

    //! Returns whether this property map refers to the ECInstanceId system property or not.
    //! @return true if the property map refers to the ECInstanceId system property. false otherwise.
    bool IsECInstanceIdPropertyMap () const;

    //! Returns whether this property map refers to an ECSQL system property or not.
    //! @return true if the property map refers to an ECSQL system property. false otherwise.
    bool IsSystemPropertyMap () const;

    //! For debugging and logging
    WString ToString() const;

    //*************** DEPRECATED **********************************************
    //Will be removed once ECDbStatement (and/or ECPersistence) was removed
    //**************************************************************************

    //! Called when preparing an ECDbStatement's parameter or selected column bindings. Adds 0 or more bindings
    void AddBindings (BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECN::ECEnablerCR enabler) const;

    //! An abstract factory method that constructs a subtype of PropertyMap, based on the ecProperty, hints, and mapping rules
    static PropertyMapPtr CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

    //! Bind an ECProperty value from the ecInstance to the statement
    //! @param iBinding             will be incremented by 1 or more. It is an index into parameterBindings
    //! @param parameterBindings    holds bindings for all parameters relevant for the statement
    //! @param statement            The statement with parameters to be bound
    //! @param ecInstance           holds values that are to be bound to the statement
    DbResult Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance) const;
    };


//=======================================================================================
//! Represents an "is not mapped" mapping for an ECProperty.
//! Some ECProperties cannot be mapped to a column in the database because they are not supported
//! in ECDb.
// @bsiclass                                                   Krischan.Eberle    03/2014
//=======================================================================================
struct UnmappedPropertyMap : PropertyMap
    {
private:
    UnmappedPropertyMap (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

    virtual bool _IsUnmapped () const override { return true; }
    virtual WString _ToString () const override;

public:
    static PropertyMapPtr Create (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

    ~UnmappedPropertyMap () {}
    };


//=======================================================================================
//! Simple primitives map to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapToColumn : PropertyMap
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);
private:
    ECN::PrimitiveECPropertyCP m_primitiveProperty;

    virtual bool _IsVirtual () const override;
    virtual NativeSqlBuilder::List _ToNativeSql (ECDbR ecdb, DbTableCR table) const override;

    DbResult BindECValueToParameter (Statement& statement, ECN::ECValueCR v, BindingCR binding) const;

protected:
    //! Metadata from which the column can be created
    ColumnInfo      m_columnInfo;

    //! The in-memory representation of a column definition in the database
    DbColumnPtr     m_column;

    //! basic constructor
    PropertyMapToColumn (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap);
    
    virtual PropertyMapToColumnCP _GetAsPropertyMapToColumn () const override { return this; }

    //! Make sure our table has the necessary columns, if any
    virtual MapStatus _FindOrCreateColumnsInTable (ClassMapR classMap) override;

    //! Checks whether this PropertyMap maps to a column of the given name
    virtual bool _MapsToColumn (Utf8CP columnName) const override;

    //! @see PropertyMap::GetColumns
    virtual void _GetColumns(DbColumnList& columns) const;

    //! @see PropertyMap::GetColumnBaseName
    virtual Utf8CP _GetColumnBaseName() const override;

    //! @see PropertyMap::SetColumnBaseName
    virtual void _SetColumnBaseName(Utf8CP columnName) override;

    //! Called when preparing an ECDbStatement's parameter or selected column bindings. Adds 0 or more bindings
    virtual void _AddBindings (BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECN::ECEnablerCR enabler) const override;

   //! Bind an ECProperty value from the ecInstance to the statement
    //! @param iBinding will be incremented by 1 or more. It is an index into parameterBindings
    //! @param parameterBindings holds bindings for all parameters relevant for the statement
    //! @param ecInstance holds values that are to be bound to the statement
    virtual DbResult _Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance) const;
    
    //! For debugging and logging
    virtual WString _ToString() const override;

public:
    DbResult BindDateTime (double jd, bool hasMetadata, DateTime::Info const& metadata, BindingCR binding, BeSQLiteStatementR statement) const;
    DbResult GetValueDateTime (double& jd, DateTime::Info& metadata, BindingCR binding, BeSQLiteStatementR statement) const;
};

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct as inline
* @bsimethod                                                    affan.khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMapToInLineStruct : PropertyMap
{
friend struct PropertyMap;
private:

    virtual NativeSqlBuilder::List _ToNativeSql (ECDbR ecdb, DbTableCR table) const override;

    //! For debugging and logging
    virtual WString _ToString() const override;

protected:
    PropertyMapToInLineStruct (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

    BentleyStatus Initialize(ECDbMapCR map);

    virtual void _GetColumns(DbColumnList& columns) const override;
    virtual MapStatus _FindOrCreateColumnsInTable (ClassMapR classMap) override;

public:
    static PropertyMapToInLineStructPtr Create (ECN::ECPropertyCR prop, ECDbMapCR ecDbMap, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

    PropertyMapCP GetPropertyMap (WCharCP propertyName) const;
};

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct or array of ECStructs to a table
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMapToTable : PropertyMap
{
friend struct PropertyMap;
private:
    // WIP_ECDB: These seem redundant, m_elementType will always be the ECClass from m_classMapForProperty, right?
    ECN::ECClassCR m_structElementType;
    mutable bmap<ECN::ECClassId, ECN::ECPropertyId> m_persistenceECPropertyIdMap;
    //! @see PropertyMap::IsValueNull
    bool _IsValueNull (int iFirstBinding, BindingsCR columnBindings, BeSQLiteStatementR statement) const;

    //! For debugging and logging
    virtual WString _ToString() const override;

protected:
    PropertyMapToTable (ECN::ECPropertyCR ecProperty, ECN::ECClassCR elementType, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);
    virtual MapStatus _FindOrCreateColumnsInTable (ClassMapR classMap) override;
    virtual PropertyMapToTableCP _GetAsPropertyMapToTable () const override { return this; }
    virtual void _GetColumns(DbColumnList& columns) const override;
    virtual ECN::ECPropertyId _GetECPropertyIdForPersistence (ECN::ECClassId relativeToECClassId, ECDbR db) const override;

public:
    static PropertyMapToTablePtr Create (ECN::ECPropertyCR prop, ECDbMapCR ecDbMap, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);
    ECN::ECPropertyId GetPropertyId ();
    ECN::ECClassCR GetElementType() const {return m_structElementType;}
};

//=======================================================================================
//! Mapping an array of primitives to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapArrayOfPrimitives : PropertyMapToColumn
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

private:
    ECN::StandaloneECEnablerP       m_primitiveArrayEnabler;

    //! basic constructor
    PropertyMapArrayOfPrimitives (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ColumnInfoCR columnInfo, ECDbMapCR ecDbMap, PropertyMapCP parentPropertyMap);
    
    virtual NativeSqlBuilder::List _ToNativeSql (ECDbR ecdb, DbTableCR table) const override;

    //! Bind an ECProperty value from the ecInstance to the statement
    //! @param iBinding will be incremented by 1 or more. It is an index into parameterBindings
    //! @param parameterBindings holds bindings for all parameters relevant for the statement
    //! @param ecInstance holds values that are to be bound to the statement
    DbResult _Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance) const;

    //! @see PropertyMap::IsValueNull
    bool _IsValueNull (int iFirstBinding, BindingsCR columnBindings, BeSQLiteStatementR statement) const;
    
    //! For debugging and logging
    WString _ToString() const override;

    PropertyMapArrayOfPrimitivesCP _GetAsPropertyMapArrayOfPrimitives () const { return this; }
};

//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapPoint : PropertyMap
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap);

private:
    //! true if 3d, false if 2d
    bool            m_is3d;

    //! Metadata from which the columns can be created
    ColumnInfo      m_columnInfo;

    //! The in-memory representation of the database column definitions
    DbColumnPtr     m_xColumn;
    DbColumnPtr     m_yColumn;
    DbColumnPtr     m_zColumn;

    //! basic constructor
    PropertyMapPoint (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap);
    
    //! Make sure our table has the necessary columns, if any
    MapStatus _FindOrCreateColumnsInTable (ClassMapR classMap) override;

    //! Checks whether this PropertyMap maps to a column of the given name
    bool _MapsToColumn (Utf8CP columnName) const override;

    //! @see PropertyMap::GetColumns
    void _GetColumns(DbColumnList& columns) const;

    //! @see PropertyMap::GetColumnBaseName
    Utf8CP _GetColumnBaseName() const override;

    //! @see PropertyMap::SetColumnBaseName
    void _SetColumnBaseName(Utf8CP columnName) override;

    //! Called when preparing an ECDbStatement's parameter or selected column bindings. Adds 0 or more bindings
    void _AddBindings (BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECN::ECEnablerCR enabler) const override;

    //! Bind an ECProperty value from the ecInstance to the statement
    //! @param iBinding will be incremented by 1 or more. It is an index into parameterBindings
    //! @param parameterBindings holds bindings for all parameters relevant for the statement
    //! @param ecInstance holds values that are to be bound to the statement
    DbResult _Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance) const;
    
    //! @see PropertyMap::IsValueNull
    bool _IsValueNull (int iFirstBinding, BindingsCR columnBindings, BeSQLiteStatementR statement) const;

    //! For debugging and logging
    WString _ToString() const override;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
