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
#include "ECDbSql.h"

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
struct ClassMapLoadContext;

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

    bool TryGetPropertyMap(PropertyMapPtr& propertyMap, bvector<Utf8String>::const_iterator& propertyAccessStringTokenIterator, bvector<Utf8String>::const_iterator& propertyAccessStringTokenEndIterator) const;
    bool TryGetPropertyMapNonRecursively(PropertyMapPtr& propertyMap, Utf8CP propertyAccessString) const;

    static void Traverse(std::set<PropertyMapCollection const*>& doneList, PropertyMapCollection const& childPropMaps, std::function<void(TraversalFeedback&, PropertyMapCP)> const& nodeOperation, bool recursive);

public:
    PropertyMapCollection() {}
    ~PropertyMapCollection() {}
    PropertyMapCollection(PropertyMapCollection&& rhs);

    PropertyMapCollection& operator= (PropertyMapCollection&& rhs);

    void AddPropertyMap(PropertyMapPtr const& propertyMap);
    void AddPropertyMap(Utf8CP propertyAccessString, PropertyMapPtr const& propertyMap);

    size_t Size() const { return m_orderedCollection.size();}
    bool IsEmpty() const { return Size() == 0; }

    bool TryGetPropertyMap(PropertyMapCP& propertyMap, Utf8CP propertyAccessString, bool recursive = false) const;
    bool TryGetPropertyMap(PropertyMapPtr& propertyMap, Utf8CP propertyAccessString, bool recursive = false) const;

    void Traverse(std::function<void(TraversalFeedback& feedback, PropertyMapCP propMap)> const& nodeOperation, bool recursive) const;

    const_iterator begin() const { return m_orderedCollection.begin(); }
    const_iterator end() const { return m_orderedCollection.end(); }
    };

struct PropertyMapStructArray;
struct NavigationPropertyMap;
struct PropertyMapRelationshipConstraintClassId;

/*---------------------------------------------------------------------------------------
* Abstract class for property mapping
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMap : RefCountedBase, NonCopyableClass
{
private:
    virtual NativeSqlBuilder::List _ToNativeSql (Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const;

    virtual bool _IsVirtual() const { return false; }
    virtual bool _IsECInstanceIdPropertyMap() const { return false; }
    virtual bool _IsSystemPropertyMap() const { return false; }

    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>&) const;
    
    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap&, ClassMapInfo const*) { return SUCCESS; }
    virtual BentleyStatus _Save(ECDbClassMapInfo&) const;
    virtual BentleyStatus _Load(ECDbClassMapInfo const&) { return SUCCESS; }

    virtual PropertyMapStructArray const* _GetAsPropertyMapStructArray() const { return nullptr; }
    virtual NavigationPropertyMap const* _GetAsNavigationPropertyMap() const { return nullptr; }
    virtual PropertyMapRelationshipConstraintClassId const* _GetAsPropertyMapRelationshipConstraintClassId() const { return nullptr; }

    virtual Utf8String _ToString() const;

protected:
    ECN::ECPropertyCR m_ecProperty;
    PropertyMapCP m_parentPropertyMap;
    Utf8String m_propertyAccessString; // We need to own this for nested property in embedded struct as they are dynamically generated
    std::vector<ECDbSqlTable const*> m_mappedTables;
    PropertyMapCollection m_children;
    mutable ECDbPropertyPathId m_propertyPathId;

    PropertyMap(ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PropertyMap(PropertyMapCR rhs, PropertyMapCP parentPropertyMap) : m_ecProperty(rhs.m_ecProperty), m_parentPropertyMap(parentPropertyMap), m_propertyAccessString(rhs.m_propertyAccessString),
        m_mappedTables(rhs.m_mappedTables), m_propertyPathId(rhs.m_propertyPathId)
        {}

    PropertyMapCR GetRoot() const
        {
        PropertyMapCP current = this;
        while (current->m_parentPropertyMap != nullptr)
            current = current->m_parentPropertyMap;

        BeAssert(current != nullptr);
        return *current;
        }

    BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, ECDbSqlColumn::Constraint::Collation& collation, ECDbCR ecdb) const { return DetermineColumnInfo(columnName, isNullable, isUnique, collation, ecdb, GetProperty(), GetPropertyAccessString()); }

public:
    virtual ~PropertyMap () {}
    ECDbPropertyPathId GetPropertyPathId () const  { BeAssert (m_propertyPathId != 0); return m_propertyPathId; }
    NavigationPropertyMap const* GetAsNavigationPropertyMap() const { return _GetAsNavigationPropertyMap(); }
    PropertyMapStructArrayCP GetAsPropertyMapStructArray () const {return _GetAsPropertyMapStructArray();}
    PropertyMapRelationshipConstraintClassId const* GetAsPropertyMapRelationshipConstraintClassId() const { return _GetAsPropertyMapRelationshipConstraintClassId(); }
    ECN::ECPropertyCR GetProperty () const;
    PropertyMapCP GetParent () const { return m_parentPropertyMap; }
    static uint32_t GetPropertyIndex (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty); //needs to take an enabler, not a class    
    PropertyMapCollection const& GetChildren () const { return m_children; }

    //! Gets a value indicating whether this property map is a virtual mapping, i.e. maps
    //! to a virtual DbColumn. A virtual DbColumn does not exist in a real table, but might
    //! be used as column aliases in views.
    //! @return true if property map is virtual, false otherwise
    bool IsVirtual () const;

    std::vector<ECDbSqlTable const*> const& GetTables() const { return m_mappedTables; }
    ECDbSqlTable const* GetTable() const;
    bool MapsToTable(ECDbSqlTable const&) const;

    //! Gets the columns (if any) mapped to this property
    void GetColumns(std::vector<ECDbSqlColumn const*>&) const;
    void GetColumns(std::vector<ECDbSqlColumn const*>&, ECDbSqlTable const&) const;
    ECDbSqlColumn const* GetSingleColumn() const;
    ECDbSqlColumn const* GetSingleColumn(ECDbSqlTable const&, bool alwaysFilterByTable) const;
    ECDbSqlTable const* GetSingleTable() const;

    //! Generates the native SQL snippets from the columns related to this property map.
    //! SQL generation depends on various properties of the property map (e.g whether the property map is virtual)
    //! and the ECSQL type. So the result of this method is not always just the column name(s).
    //! @param[in] classIdentifier Class name or class alias which will be prepended to the sql snippet, e.g.
    //!                            <classidentifier>.<columnname>
    //! @param[in] ecsqlType ECSQL type for which the native SQL is generated. SQL generation depends on the ECSQL type
    //!            which is why this parameter is needed
    //! @param[in] wrapInParentheses true if the native SQL snippets should be wrapped in parentheses. false otherwise
    //! @param[in] tableFilter if not nullptr, only columns from the specified tables are used to generate the native SQL.
    //! @return List of native SQL snippets, one snippet per column this PropertyMap maps to.
    NativeSqlBuilder::List ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter = nullptr) const;

  
    //! Saves the base column name, if it differs from the property name
    BentleyStatus Save(ECDbClassMapInfo & classMapInfo) const;
    BentleyStatus Load(ECDbClassMapInfo const& classMapInfo) { return _Load(classMapInfo); }

    Utf8CP GetPropertyAccessString () const;

    //! Make sure our table has the necessary columns, if any
    BentleyStatus FindOrCreateColumnsInTable(ClassMap& classMap, ClassMapInfo const* classMapInfo);

    //! Returns whether this property map refers to the ECInstanceId system property or not.
    //! @return true if the property map refers to the ECInstanceId system property. false otherwise.
    bool IsECInstanceIdPropertyMap () const;

    //! Returns whether this property map refers to an ECSQL system property or not.
    //! @return true if the property map refers to an ECSQL system property. false otherwise.
    bool IsSystemPropertyMap () const;

    //! For debugging and logging
    Utf8String ToString() const;

    static BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, ECDbSqlColumn::Constraint::Collation&, ECDbCR, ECN::ECPropertyCR, Utf8CP propAccessString);

    static PropertyMapPtr CreateAndEvaluateMapping (ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    //only called during schema import
    static PropertyMapPtr Clone(ECDbMapCR, PropertyMapCR, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);
    };

//=======================================================================================
//! Simple primitives map to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapSingleColumn : PropertyMap
{
private:
    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(ECDbMapCR, PropertyMapCR, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    ECN::PrimitiveECPropertyCP m_primitiveProperty;

    virtual bool _IsVirtual () const override;

    /*---------------------------------------------------------------------------------------
    * @bsimethod                                                    affan.khan      01/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) override;
    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap&, ClassMapInfo const*) override;
    virtual Utf8String _ToString() const override;

protected:
    ECDbSqlColumn*     m_column;

    PropertyMapSingleColumn (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PropertyMapSingleColumn(PropertyMapSingleColumn const& proto, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto, parentPropertyMap), m_column(proto.m_column)
        {}

    void SetColumn(ECDbSqlColumn&);

    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>& columns) const;
};

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct as inline
* @bsimethod                                                    affan.khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMapStruct : PropertyMap
{
private:
    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(ECDbMapCR, PropertyMapCR proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>& columns) const override;
    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap&, ClassMapInfo const*) override;
    virtual BentleyStatus _Load(ECDbClassMapInfo const&) override;
    virtual BentleyStatus _Save(ECDbClassMapInfo&) const override;

    virtual Utf8String _ToString() const override;

    PropertyMapStruct(ECN::ECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PropertyMapStruct(ECDbMapCR ecdbMap, PropertyMapStruct const& proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto, parentPropertyMap)
        {
        for (PropertyMap const* protoChild : proto.m_children)
            {
            m_children.AddPropertyMap(protoChild->GetProperty().GetName().c_str(), PropertyMap::Clone(ecdbMap, *protoChild, clonedBy, this));
            }
        }

    BentleyStatus Initialize(ClassMapLoadContext&, ECDbCR);

public:
    static PropertyMapStructPtr Create (ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);

    PropertyMapCP GetPropertyMap (Utf8CP propertyName) const;
};

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct or array of ECStructs to a table
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMapStructArray : PropertyMap
{
private:
    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(ECDbMapCR, PropertyMapCR proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    // WIP_ECDB: These seem redundant, m_elementType will always be the ECClass from m_classMapForProperty, right?
    ECN::ECClassCR m_structElementType;

    virtual Utf8String _ToString() const override;

protected:
    PropertyMapStructArray (ECN::ECPropertyCR ecProperty, ECN::ECClassCR elementType, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PropertyMapStructArray(PropertyMapStructArray const& proto, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto, parentPropertyMap), m_structElementType(proto.m_structElementType)
        {}
    virtual PropertyMapStructArrayCP _GetAsPropertyMapStructArray () const override { return this; }
    virtual BentleyStatus _Save(ECDbClassMapInfo & classMapInfo) const override;
    virtual BentleyStatus _Load(ECDbClassMapInfo const& classMapInfo) override;
public:
    static PropertyMapStructArrayPtr Create (ECN::ECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    ECN::ECPropertyId GetPropertyId ();
    ECN::ECClassCR GetElementType() const {return m_structElementType;}
};

//=======================================================================================
//! Mapping an array of primitives to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapPrimitiveArray : PropertyMapSingleColumn
{
private:
    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(ECDbMapCR, PropertyMapCR proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    ECN::StandaloneECEnablerP m_primitiveArrayEnabler;

    PropertyMapPrimitiveArray(ECDbCR, ECN::ECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PropertyMapPrimitiveArray(PropertyMapPrimitiveArray const& proto, PropertyMap const* parentPropertyMap)
        :PropertyMapSingleColumn(static_cast<PropertyMapSingleColumn const&>(proto), parentPropertyMap), m_primitiveArrayEnabler(proto.m_primitiveArrayEnabler)
        {}

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap&, ClassMapInfo const*) override;
    Utf8String _ToString() const override;
};

//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PropertyMapPoint : PropertyMap
{
private:
    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(ECDbMapCR, PropertyMapCR proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    static const ECN::PrimitiveType s_defaultCoordinateECType = ECN::PRIMITIVETYPE_Double;

    bool m_is3d;
    ECDbSqlColumn const* m_xColumn;
    ECDbSqlColumn const* m_yColumn;
    ECDbSqlColumn const* m_zColumn;

    PropertyMapPoint (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PropertyMapPoint(PropertyMapPoint const& proto, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto, parentPropertyMap), m_xColumn(proto.m_xColumn), m_yColumn(proto.m_yColumn), m_zColumn(proto.m_zColumn), m_is3d(proto.m_is3d)
        {}

    BentleyStatus _FindOrCreateColumnsInTable(ClassMap& classMap, ClassMapInfo const* classMapInfo) override;
    void _GetColumns (std::vector<ECDbSqlColumn const*>& columns) const;
    virtual BentleyStatus _Save(ECDbClassMapInfo&) const override;
    virtual BentleyStatus _Load(ECDbClassMapInfo const&) override;
    Utf8String _ToString() const override;

    BentleyStatus SetColumns(ECDbSqlColumn const&, ECDbSqlColumn const&, ECDbSqlColumn const*);

public:
    bool Is3d () const { return m_is3d; }

    static ECDbSqlColumn::Type GetDefaultColumnType() { return ECDbSqlColumn::PrimitiveTypeToColumnType(s_defaultCoordinateECType); }
};

struct RelationshipConstraintMap;

//=======================================================================================
// @bsiclass                                                    Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyMap : PropertyMap
    {
public:
    enum class NavigationEnd
        {
        From,
        To
        };

private:

    friend PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, ECN::ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    friend PropertyMapPtr PropertyMap::Clone(ECDbMapCR, PropertyMapCR, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    ECN::NavigationECPropertyCP m_navigationProperty;
    RelationshipClassMap const* m_relClassMap;
    std::vector<ECDbSqlColumn const* > m_columns;
    ECN::ECClassCP m_createdByClass;

    NavigationPropertyMap(ClassMapLoadContext&, ECN::ECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap, ECN::ECClassCR createdByClass);
    NavigationPropertyMap(ClassMapLoadContext&, NavigationPropertyMap const& proto, PropertyMap const* parentPropertyMap, ECN::ECClassCR createdByClass);
    static PropertyMapPtr Create(ClassMapLoadContext&, ECDbCR, ECN::ECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap, ECN::ECClassCR createdByClass);

    virtual void _GetColumns(std::vector<ECDbSqlColumn const*>&) const override;
    virtual Utf8String _ToString() const override { return Utf8PrintfString("NavigationPropertyMap: ecProperty=%s.%s", m_ecProperty.GetClass().GetFullName(), m_ecProperty.GetName().c_str()); }

    //nothing to do when loading/saving the prop map
    virtual BentleyStatus _Load(ECDbClassMapInfo const&) override { return SUCCESS; }
    virtual BentleyStatus _Save(ECDbClassMapInfo&) const override { return SUCCESS; }
    virtual NavigationPropertyMap const* _GetAsNavigationPropertyMap() const override { return this; }

    ECN::ECRelationshipEnd GetConstraintEnd(NavigationEnd end) const { return GetConstraintEnd(*m_navigationProperty, end); }
    static ECN::ECRelationshipEnd GetConstraintEnd(ECN::NavigationECPropertyCR, NavigationEnd);

public:
    ~NavigationPropertyMap() {}

    BentleyStatus Postprocess(ECDbMapCR);

    bool IsSupportedInECSql(bool logIfNotSupported = false, ECDbCP ecdb = nullptr) const;

    RelationshipClassMap const& GetRelationshipClassMap() const { BeAssert(m_relClassMap != nullptr); return *m_relClassMap; }

    RelationshipConstraintMap const& GetConstraintMap(NavigationEnd end) const;
    static ECN::ECRelationshipConstraintCR GetConstraint(ECN::NavigationECPropertyCR, NavigationEnd);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
