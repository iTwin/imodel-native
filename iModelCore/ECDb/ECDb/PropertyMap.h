/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMapInfo.h"
#include "ColumnInfo.h"

#include "ECDbSystemSchemaHelper.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

enum class TraversalFeedback
    {
    Cancel, //! cancel traversal altogether and return
    NextSibling, //! donot traverse children of current node instead go with next sibling
    Next //! if there is children of current node process them first and then go to nextr sibling
    };

struct ClassMap;
//=======================================================================================
// Represents an iterable collection of property maps
// @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapCollection : NonCopyableClass
    {
public:
    typedef std::vector<PropertyMapCP>::const_iterator const_iterator;

private:
    typedef std::map<Utf8CP, PropertyMapPtr, CompareUtf8> PropertyMapsByAccessString;

    PropertyMapsByAccessString m_dictionary;
    std::vector<PropertyMapCP> m_orderedCollection;

    bool TryGetPropertyMap (PropertyMapPtr& propertyMap, bvector<Utf8String>::const_iterator& propertyAccessStringTokenIterator, bvector<Utf8String>::const_iterator& propertyAccessStringTokenEndIterator) const;
    bool TryGetPropertyMapNonRecursively (PropertyMapPtr& propertyMap, Utf8CP propertyAccessString) const;

    static void Traverse (std::set<PropertyMapCollection const*>& doneList, PropertyMapCollection const& childPropMaps, std::function<void (TraversalFeedback&, PropertyMapCP)> const& nodeOperation, bool recursive);

public:
    PropertyMapCollection ();
    ~PropertyMapCollection () {}
    PropertyMapCollection (PropertyMapCollection&& rhs);

    PropertyMapCollection& operator= (PropertyMapCollection&& rhs);

    void AddPropertyMap (PropertyMapPtr const& propertyMap);
    void AddPropertyMap (Utf8CP propertyAccessString, PropertyMapPtr const& propertyMap);

    size_t Size () const;
    bool IsEmpty () const;

    bool TryGetPropertyMap (PropertyMapCP& propertyMap, Utf8CP propertyAccessString, bool recursive = false) const;
    bool TryGetPropertyMap (PropertyMapPtr& propertyMap, Utf8CP propertyAccessString, bool recursive = false) const;

    void Traverse (std::function<void (TraversalFeedback& feedback, PropertyMapCP propMap)> const& nodeOperation, bool recursive) const;

    const_iterator begin () const;
    const_iterator end () const;
    };

struct PropertyMapSingleColumn;
struct PropertyMapStructArray;
struct PropertyMapPrimitiveArray;

/*---------------------------------------------------------------------------------------
* Abstract class for property mapping
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMap : RefCountedBase, NonCopyableClass
{
private:
    virtual NativeSqlBuilder::List _ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const;

protected:
    ECN::ECPropertyCR       m_ecProperty;
    PropertyMapCP           m_parentPropertyMap;
    Utf8String              m_propertyAccessString; // We need to own this for nested property in embedded struct as they are dynamically generated
    PropertyMapCollection   m_children;
    ECDbSqlTable const*     m_primaryTable;

    mutable ECDbPropertyPathId      m_propertyPathId;
    PropertyMap (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);

    virtual bool _IsVirtual () const;
    virtual bool _IsUnmapped () const;

    //! @see PropertyMap::GetColumns
    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>& columns) const;

    //! Make sure our table has the necessary columns, if any
    virtual BentleyStatus _FindOrCreateColumnsInTable(SchemaImportContext*, ClassMap& classMap, ClassMapInfo const* classMapInfo) { return SUCCESS; }

    virtual PropertyMapSingleColumn const* _GetAsPropertyMapSingleColumn () const { return nullptr; }
    virtual PropertyMapStructArray const* _GetAsPropertyMapStructArray () const { return nullptr; }

    virtual bool _IsECInstanceIdPropertyMap () const;
    virtual bool _IsSystemPropertyMap () const;
    virtual BentleyStatus _Save(ECDbClassMapInfo & classMapInfo) const;
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) { return SUCCESS; }

    //! For debugging and logging
    virtual Utf8String _ToString() const;

public:
    virtual ~PropertyMap () {}
    ECDbPropertyPathId GetPropertyPathId () const  { BeAssert (m_propertyPathId != 0); return m_propertyPathId; }
    PropertyMapSingleColumn const* GetAsPropertyMapSingleColumn() const { return _GetAsPropertyMapSingleColumn(); }
    PropertyMapStructArrayCP GetAsPropertyMapStructArray () const {return _GetAsPropertyMapStructArray();}
    ECN::ECPropertyCR GetProperty () const;
    PropertyMapCP GetParent () const { return m_parentPropertyMap; }
    PropertyMapCR GetRoot () const 
        { 
        auto current = this;
        while (current->m_parentPropertyMap != nullptr)
            current = current->m_parentPropertyMap;

        return *current;
        }
    ECDbSqlTable const* GetPrimaryTable () const { if (m_primaryTable) return m_primaryTable; return GetRoot ().GetPrimaryTable (); }
    ECDbSqlTable const* GetTable () const 
        { 
        auto column = GetFirstColumn(); 
        if (column)
            return &(column->GetTable ());

        return nullptr;
        }

    bool IsMappedToPrimaryTable () const
        {
        auto primaryTable = GetPrimaryTable ();
        auto thisTable = GetTable ();
        BeAssert (primaryTable != nullptr);

        return (primaryTable != nullptr && thisTable != nullptr && primaryTable == thisTable);
        }

    static uint32_t GetPropertyIndex (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty); //needs to take an enabler, not a class    
    PropertyMapCollection const& GetChildren () const { return m_children; }

    //! Gets a value indicating whether this property map is a virtual mapping, i.e. maps
    //! to a virtual DbColumn. A virtual DbColumn does not exist in a real table, but might
    //! be used as column aliases in views.
    //! @return true if property map is virtual, false otherwise
    bool IsVirtual () const;

    //! Gets a value indicating whether the property of this property map is not mapped to a database column.
    //! @return true if the property is not mapped, false otherwise
    bool IsUnmapped () const;

    //! Gets the columns (if any) mapped to this property
    void GetColumns(std::vector<ECDbSqlColumn const*>& columns) const;

    //! Gets the first column if any
    ECDbSqlColumn const* GetFirstColumn() const;

    //! Generates the native SQL snippets from the columns related to this property map.
    //! SQL generation depends on various properties of the property map (e.g whether the property map is virtual)
    //! and the ECSQL type. So the result of this method is not always just the column name(s).
    //! @param[in] classIdentifier Class name or class alias which will be prepended to the sql snippet, e.g.
    //!                            <classidentifier>.<columnname>
    //! @param[in] ecsqlType ECSQL type for which the native SQL is generated. SQL generation depends on the ECSQL type
    //!            which is why this parameter is needed
    //! @param[in] wrapInParentheses true if the native SQL snippets should be wrapped in parentheses. false otherwise
    //! @return List of native SQL snippets, one snippet per column this PropertyMap maps to.
    NativeSqlBuilder::List ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const;


    //! Saves the base column name, if it differs from the property name
    BentleyStatus Save(ECDbClassMapInfo & classMapInfo) const;
    BentleyStatus Load(ECDbClassMapInfo const& classMapInfo) { return _Load(classMapInfo); }

    Utf8CP GetPropertyAccessString () const;

    //! Make sure our table has the necessary columns, if any
    BentleyStatus FindOrCreateColumnsInTable(SchemaImportContext*, ClassMap& classMap, ClassMapInfo const* classMapInfo);

    //! Returns whether this property map refers to the ECInstanceId system property or not.
    //! @return true if the property map refers to the ECInstanceId system property. false otherwise.
    bool IsECInstanceIdPropertyMap () const;

    //! Returns whether this property map refers to an ECSQL system property or not.
    //! @return true if the property map refers to an ECSQL system property. false otherwise.
    bool IsSystemPropertyMap () const;

    //! For debugging and logging
    Utf8String ToString() const;

    PropertyMapPtr Clone(ECDbSqlTable const* newContext = nullptr) const { return Clone(*this, newContext, nullptr); }
    static PropertyMapPtr Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);

    //! An abstract factory method that constructs a subtype of PropertyMap, based on the ecProperty, hints, and mapping rules
    static PropertyMapPtr CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
    };

//=======================================================================================
//! Simple primitives map to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapSingleColumn : PropertyMap
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
friend PropertyMapPtr PropertyMap::Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);
private:
    ECN::PrimitiveECPropertyCP m_primitiveProperty;

    virtual bool _IsVirtual () const override;

    /*---------------------------------------------------------------------------------------
    * @bsimethod                                                    affan.khan      01/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) override
        {
        BeAssert (m_column == nullptr);
        auto info = classMapInfo.FindPropertyMap (GetRoot ().GetProperty ().GetId (), Utf8String (GetPropertyAccessString ()).c_str ());
        if (info == nullptr)
            {
            BeAssert (false && "Failed to read back property map");
            return ERROR;
            }

        m_column = const_cast<ECDbSqlColumn*>(&info->GetColumn ());
        return SUCCESS;
        }
protected:
    //! Metadata from which the column can be created
    ColumnInfo      m_columnInfo;

    //! The in-memory representation of a column definition in the database
    ECDbSqlColumn*     m_column;

    //! basic constructor
    PropertyMapSingleColumn (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap);
    PropertyMapSingleColumn(PropertyMapSingleColumn const& proto, ECDbSqlTable const* primaryTable, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto.GetProperty(), proto.GetPropertyAccessString(), primaryTable, parentPropertyMap), m_columnInfo(proto.m_columnInfo), m_column(proto.m_column)
        {}

    virtual PropertyMapSingleColumn const* _GetAsPropertyMapSingleColumn() const override { return this; }

    //! Make sure our table has the necessary columns, if any
    virtual BentleyStatus _FindOrCreateColumnsInTable(SchemaImportContext*, ClassMap& classMap, ClassMapInfo const* classMapInfo) override;

    //! @see PropertyMap::GetColumns
    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>& columns) const;
   
    //! For debugging and logging
    virtual Utf8String _ToString() const override;
public:

};

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct as inline
* @bsimethod                                                    affan.khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMapStruct : PropertyMap
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
friend PropertyMapPtr PropertyMap::Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);

private:
    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>& columns) const override;
    virtual BentleyStatus _FindOrCreateColumnsInTable(SchemaImportContext*, ClassMap& classMap, ClassMapInfo const* classMapInfo) override;
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) override;
    virtual BentleyStatus _Save(ECDbClassMapInfo & classMapInfo) const override;

    //! For debugging and logging
    virtual Utf8String _ToString() const override;

    PropertyMapStruct(ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
    PropertyMapStruct(PropertyMapStruct const& proto, ECDbSqlTable const* primaryTable, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto.GetProperty(), proto.GetPropertyAccessString(), primaryTable, parentPropertyMap)
        {
        for (auto const& protoChild : proto.m_children)
            {
            m_children.AddPropertyMap(protoChild->GetProperty().GetName().c_str(), PropertyMap::Clone(*protoChild, primaryTable, this));
            }
        }

    BentleyStatus Initialize(ECDbMapCR map);

public:
    static PropertyMapStructPtr Create (ECN::ECPropertyCR prop, ECDbMapCR ecDbMap, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);

    PropertyMapCP GetPropertyMap (Utf8CP propertyName) const;
};

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct or array of ECStructs to a table
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMapStructArray : PropertyMap
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
friend PropertyMapPtr PropertyMap::Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);
private:
    // WIP_ECDB: These seem redundant, m_elementType will always be the ECClass from m_classMapForProperty, right?
    ECN::ECClassCR m_structElementType;

    //! For debugging and logging
    virtual Utf8String _ToString() const override;

protected:
    PropertyMapStructArray (ECN::ECPropertyCR ecProperty, ECN::ECClassCR elementType, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
    PropertyMapStructArray(PropertyMapStructArray const& proto, ECDbSqlTable const* primaryTable, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto.GetProperty(), proto.GetPropertyAccessString(), primaryTable, parentPropertyMap), m_structElementType(proto.m_structElementType)
        {}
    virtual PropertyMapStructArrayCP _GetAsPropertyMapStructArray () const override { return this; }
    virtual BentleyStatus _Save(ECDbClassMapInfo & classMapInfo) const override;
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) override;
public:
    static PropertyMapStructArrayPtr Create (ECN::ECPropertyCR prop, ECDbMapCR ecDbMap, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
    ECN::ECPropertyId GetPropertyId ();
    ECN::ECClassCR GetElementType() const {return m_structElementType;}
};

//=======================================================================================
//! Mapping an array of primitives to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapPrimitiveArray : PropertyMapSingleColumn
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
friend PropertyMapPtr PropertyMap::Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);
private:
    ECN::StandaloneECEnablerP       m_primitiveArrayEnabler;

    //! basic constructor
    PropertyMapPrimitiveArray (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, ColumnInfoCR columnInfo, ECDbMapCR ecDbMap, PropertyMapCP parentPropertyMap);
    PropertyMapPrimitiveArray(PropertyMapPrimitiveArray const& proto, ECDbSqlTable const* primaryTable, PropertyMap const* parentPropertyMap)
        :PropertyMapSingleColumn(static_cast<PropertyMapSingleColumn const&>(proto), primaryTable, parentPropertyMap), m_primitiveArrayEnabler(proto.m_primitiveArrayEnabler)
        {}

    //! For debugging and logging
    Utf8String _ToString() const override;
};

//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapPoint : PropertyMap
{
friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
friend PropertyMapPtr PropertyMap::Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);
private:
    //! true if 3d, false if 2d
    bool            m_is3d;

    //! Metadata from which the columns can be created
    ColumnInfo      m_columnInfo;

    //! The in-memory representation of the database column definitions
    ECDbSqlColumn*     m_xColumn;
    ECDbSqlColumn*     m_yColumn;
    ECDbSqlColumn*     m_zColumn;

    //! basic constructor
    PropertyMapPoint (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap);
    PropertyMapPoint(PropertyMapPoint const& proto, ECDbSqlTable const* primaryTable, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto.GetProperty(), proto.GetPropertyAccessString(), primaryTable, parentPropertyMap), 
        m_xColumn(proto.m_xColumn), m_yColumn(proto.m_yColumn), m_zColumn(proto.m_zColumn), m_is3d(proto.m_is3d), m_columnInfo(proto.m_columnInfo)
        {
        }
    //! Make sure our table has the necessary columns, if any
    BentleyStatus _FindOrCreateColumnsInTable(SchemaImportContext*, ClassMap& classMap, ClassMapInfo const* classMapInfo) override;

    //! @see PropertyMap::GetColumns
    void _GetColumns (std::vector<ECDbSqlColumn const*>& columns) const;

    virtual BentleyStatus _Save(ECDbClassMapInfo & classMapInfo) const override;
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) override;
    //! For debugging and logging
    Utf8String _ToString() const override;
public:
    bool Is3d () const { return m_is3d; }
};

//=======================================================================================
// @bsiclass                                                    Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyMap : PropertyMap
    {
private:
    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ECN::ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(PropertyMapCR proto, ECDbSqlTable const* newContext, PropertyMap const* parentPropertyMap);

    ECN::NavigationECPropertyCP m_navigationProperty;
    RelationshipClassMap const* m_relClassMap;

    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const override;
    virtual Utf8String _ToString() const override { return Utf8PrintfString("NavigationPropertyMap: ecProperty=%s.%s", m_ecProperty.GetClass().GetFullName(), m_ecProperty.GetName().c_str()); }

    //BentleyStatus _FindOrCreateColumnsInTable(SchemaImportContext*, ClassMap&, ClassMapInfo const*) override;
    //void _GetColumns(std::vector<ECDbSqlColumn const*>& columns) const;

    NavigationPropertyMap(ECN::ECPropertyCR, ECDbMapCR, Utf8CP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap);
    NavigationPropertyMap(NavigationPropertyMap const& proto, ECDbSqlTable const* primaryTable, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto.GetProperty(), proto.GetPropertyAccessString(), primaryTable, parentPropertyMap), m_navigationProperty(proto.m_navigationProperty)
        {}

    ECN::ECRelationshipEnd GetConstraintEnd() const;

public:
    ~NavigationPropertyMap() {}

    bool CanOnlyHaveOneRelatedInstance() const { return CanOnlyHaveOneRelatedInstance(*m_navigationProperty); }
    static bool CanOnlyHaveOneRelatedInstance(ECN::NavigationECPropertyCR);
    static ECN::ECRelationshipConstraintCR GetConstraint(ECN::NavigationECPropertyCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
