/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMappingInfo.h"
#include "DbSchema.h"
#include "ECDbSystemSchemaHelper.h"
#include "ECSql/NativeSqlBuilder.h"
#include "DbSchemaPersistenceManager.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

enum class TraversalFeedback
    {
    Cancel, //! cancel traversal altogether and return
    NextSibling, //! do not traverse children of current node instead go with next sibling
    Next //! if there is children of current node process them first and then go to next sibling
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
    typedef std::map<Utf8CP, PropertyMapPtr, CompareIUtf8Ascii> PropertyMapsByAccessString;

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

    BentleyStatus AddPropertyMap(PropertyMapPtr const& propertyMap, size_t position = UINT_MAX);
    BentleyStatus AddPropertyMap(Utf8CP propertyAccessString, PropertyMapPtr const& propertyMap, size_t position = UINT_MAX);

    size_t Size() const { return m_orderedCollection.size();}
    bool IsEmpty() const { return Size() == 0; }

    bool TryGetPropertyMap(PropertyMapCP& propertyMap, Utf8CP propertyAccessString, bool recursive = false) const;
    bool TryGetPropertyMap(PropertyMapPtr& propertyMap, Utf8CP propertyAccessString, bool recursive = false) const;

    void Traverse(std::function<void(TraversalFeedback& feedback, PropertyMapCP propMap)> const& nodeOperation, bool recursive) const;

    const_iterator begin() const { return m_orderedCollection.begin(); }
    const_iterator end() const { return m_orderedCollection.end(); }
    };

struct StructArrayJsonPropertyMap;
struct NavigationPropertyMap;
struct RelConstraintECClassIdPropertyMap;
struct ColumnMappedToProperty
    {
    enum class LoadFlags :uint32_t
        {
        None,
        Column = 1,
        AccessString = 2,
        StrongType = 4,
        PropertyMap = 8
        };

    private:
        LoadFlags m_loadFlag;
        DbColumn const* m_column;
        Utf8String  m_accessString;
        DbColumn::Type m_strongType;
        PropertyMap const* m_propertyMap;

    public:
        ColumnMappedToProperty()
            :m_loadFlag(LoadFlags::None), m_column(nullptr), m_strongType(DbColumn::Type::Any), m_propertyMap(nullptr)
            {}
        ColumnMappedToProperty(ColumnMappedToProperty const& rhs)
            :m_loadFlag(rhs.m_loadFlag), m_column(rhs.m_column), m_accessString(rhs.m_accessString), m_strongType(rhs.m_strongType), m_propertyMap(rhs.m_propertyMap)
            {}

        ColumnMappedToProperty(ColumnMappedToProperty const&& rhs)
            :m_loadFlag(std::move(rhs.m_loadFlag)), m_column(std::move(rhs.m_column)), m_accessString(std::move(rhs.m_accessString)), m_strongType(std::move(rhs.m_strongType)), m_propertyMap(std::move(rhs.m_propertyMap))
            {}
        ColumnMappedToProperty& operator = (ColumnMappedToProperty const& rhs) 
            {
            if (this != &rhs)
                {
                m_loadFlag = rhs.m_loadFlag;
                m_column = rhs.m_column;
                m_strongType = rhs.m_strongType;
                m_propertyMap = rhs.m_propertyMap;
                m_accessString = rhs.m_accessString;
                }

            return *this;
            }
        ColumnMappedToProperty& operator = (ColumnMappedToProperty const&& rhs)
            {
            if (this != &rhs)
                {
                m_loadFlag = std::move(rhs.m_loadFlag);
                m_column = std::move(rhs.m_column);
                m_strongType = std::move(rhs.m_strongType);
                m_propertyMap = std::move(rhs.m_propertyMap);
                m_accessString = std::move(rhs.m_accessString);
                }

            return *this;
            }

        ~ColumnMappedToProperty()
            {
            Reset();
            }
        void SetColumn(DbColumn const& column) { m_loadFlag = Enum::Or(m_loadFlag, LoadFlags::Column); m_column = &column; }
        void SetAccessString(Utf8StringCR  accessString)
            {
            m_loadFlag = Enum::Or(m_loadFlag, LoadFlags::AccessString);
            m_accessString = accessString;
            }
        void SetStrongType(DbColumn::Type type) { m_loadFlag = Enum::Or(m_loadFlag, LoadFlags::StrongType); m_strongType = type; }
        void SetPropertyMap(PropertyMap const& propertyMap) { m_loadFlag = Enum::Or(m_loadFlag, LoadFlags::PropertyMap); m_propertyMap = &propertyMap; }
        void Reset()
            {
            m_loadFlag = LoadFlags::None;
            m_column = nullptr;
            m_accessString.clear();
            m_strongType = DbColumn::Type::Any;
            m_propertyMap = nullptr;
            }
        LoadFlags GetLoadFlags() const { return m_loadFlag; }
        DbColumn const* GetColumn() const { BeAssert(Enum::Contains(m_loadFlag, LoadFlags::Column)); return m_column; }
        Utf8CP GetAccessString() const { BeAssert(Enum::Contains(m_loadFlag, LoadFlags::AccessString)); return m_accessString.c_str(); }
        DbColumn::Type GetStrongType() const { BeAssert(Enum::Contains(m_loadFlag, LoadFlags::StrongType)); return m_strongType; }
        PropertyMapCP GetPropertyMap() const { BeAssert(Enum::Contains(m_loadFlag, LoadFlags::PropertyMap)); return m_propertyMap; }
    };
typedef std::vector<ColumnMappedToProperty> ColumnMappedToPropertyList;

//=======================================================================================
// @bsimethod                                                   affan.khan        03/2012
//+===============+===============+===============+===============+===============+======
struct PropertyMap : RefCountedBase, NonCopyableClass
    {
public:
    enum class Type
        {
        Primitive,
        Point,
        PrimitiveArray,
        Struct,
        StructArray,
        Navigation,
        ECInstanceId,
        ECClassId,
        RelConstraintECInstanceId,
        RelConstraintECClassId
        };

 protected:
    ECN::ECPropertyCR m_ecProperty;
    Utf8String m_propertyAccessString; // We need to own this for nested property in embedded struct as they are dynamically generated
    std::vector<DbTable const*> m_mappedTables;
    PropertyMapCP m_parent;
    PropertyMapCollection m_children;
private:
    const Type m_type;
    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, DbTable const* tableFilter) const;

    virtual bool _IsVirtual() const { return false; }

    virtual void _GetColumns(std::vector<DbColumn const*>&) const;

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) { return SUCCESS; }
    virtual BentleyStatus _Save(DbClassMapSaveContext&) const;
    virtual BentleyStatus _Load(DbClassMapLoadContext const&) { return SUCCESS; }
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const = 0;
    virtual BentleyStatus _GetPropertyPathList(std::vector<Utf8String>& propertyPathList) const;

protected:
    //!@param[in] parent Parent property map in terms of a property map node hierarchy. Nothing to do with inheritance.
    //!E.g. a struct property map has a child property map for each of its members. The struct property map then is the parent
    //!of each member property map
    PropertyMap(Type type, ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parent) : m_type(type), m_ecProperty(ecProperty), m_propertyAccessString(propertyAccessString), m_parent(parent) {}
    PropertyMap(PropertyMapCR rhs, PropertyMapCP parentPropertyMap) : m_ecProperty(rhs.m_ecProperty), m_parent(parentPropertyMap), m_propertyAccessString(rhs.m_propertyAccessString),
        m_mappedTables(rhs.m_mappedTables),m_type(rhs.m_type)
        {}

    //!Gets the root property map of the parent-child node hierarchy
    PropertyMapCR GetRoot() const;
    BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation& collation, ECDbCR ecdb) const { return DetermineColumnInfo(columnName, isNullable, isUnique, collation, ecdb, GetProperty(), GetPropertyAccessString()); }

public:
    virtual ~PropertyMap() {}
    Type GetType() const { return m_type; }
    bool IsSystemPropertyMap() const { return m_type == Type::ECClassId || m_type == Type::ECInstanceId || m_type == Type::RelConstraintECClassId || m_type == Type::RelConstraintECInstanceId; }
    ECN::ECPropertyCR GetProperty() const { return m_ecProperty; }
    Utf8CP GetPropertyAccessString() const { return m_propertyAccessString.c_str(); }
    PropertyMapCollection const& GetChildren() const { return m_children; }
    virtual BentleyStatus GetPropertyPathList(std::vector<Utf8String>& propertyPathList) const { return _GetPropertyPathList(propertyPathList); }
    //! Gets a value indicating whether this property map is a virtual mapping, i.e. maps
    //! to a virtual DbColumn. A virtual DbColumn does not exist in a real table, but might
    //! be used as column aliases in views.
    //! @return true if property map is virtual, false otherwise
    bool IsVirtual() const { return _IsVirtual(); }

    std::vector<DbTable const*> const& GetTables() const { return m_mappedTables; }
    DbTable const* GetTable() const;
    bool MapsToTable(DbTable const&) const;
    ColumnMappedToPropertyList QueryColumnInfo(ColumnMappedToProperty::LoadFlags loadFlags, bool recusive = true) const
        {
        ColumnMappedToPropertyList queryResult;
        _QueryColumnMappedToProperty(queryResult, loadFlags, recusive);
        return queryResult;
        }
    //! Gets the columns (if any) mapped to this property
    void GetColumns(std::vector<DbColumn const*>& columns) const { return _GetColumns(columns); }
    void GetColumns(std::vector<DbColumn const*>&, DbTable const&) const;
    DbColumn const* GetSingleColumn() const;
    DbColumn const* GetSingleColumn(DbTable const&, bool alwaysFilterByTable) const;
    DbTable const* GetSingleTable() const;

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
    NativeSqlBuilder::List ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter = nullptr) const;

    BentleyStatus Save(DbClassMapSaveContext& ctx) const { return _Save(ctx); }
    BentleyStatus Load(DbClassMapLoadContext const& mapping) { return _Load(mapping); }
    
    //! Make sure our table has the necessary columns, if any
    BentleyStatus FindOrCreateColumnsInTable(ClassMap const& classMap) { return _FindOrCreateColumnsInTable(classMap); }

    static BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation&, ECDbCR, ECN::ECPropertyCR, Utf8CP propAccessString);

    static Utf8CP TypeToString(Type);
    };

//=======================================================================================
//! Simple primitives map to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct SingleColumnPropertyMap : PropertyMap
{
private:
    DbColumn const* m_column;

    virtual bool _IsVirtual () const override;

    virtual BentleyStatus _Load(DbClassMapLoadContext const& classMapInfo) override;
    virtual void _GetColumns(std::vector<DbColumn const*>& columns) const override;
    void SetColumn(DbColumn const&);
    virtual BentleyStatus _GetPropertyPathList(std::vector<Utf8String>& propertyPathList)const override  { propertyPathList.push_back(GetPropertyAccessString());  return SUCCESS; }

protected:
    SingleColumnPropertyMap (Type type, ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
        : PropertyMap(type, ecProperty, propertyAccessString, parentPropertyMap), m_column(nullptr) {}

    SingleColumnPropertyMap(SingleColumnPropertyMap const& proto, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto, parentPropertyMap), m_column(proto.m_column) {}

    virtual ~SingleColumnPropertyMap() {}

    BentleyStatus DoFindOrCreateColumnsInTable(ClassMap const&, DbColumn::Type);

    DbColumn const& GetColumn() const { return *m_column; }
};

//=======================================================================================
// @bsiclass                                                     Krischan.Eberle  03/2016
//=======================================================================================
struct PrimitivePropertyMap : SingleColumnPropertyMap
    {
private:
    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) override;
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const override;
    PrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) : SingleColumnPropertyMap(Type::Primitive, ecProperty, propertyAccessString, parentPropertyMap) {}
    PrimitivePropertyMap(PrimitivePropertyMap const& proto, PropertyMap const* parentPropertyMap) : SingleColumnPropertyMap(proto, parentPropertyMap) {}

    ECN::PrimitiveECPropertyCR GetPrimitiveProperty() const { BeAssert(GetProperty().GetIsPrimitive()); return *GetProperty().GetAsPrimitiveProperty(); }

public:
    ~PrimitivePropertyMap() {}
    static PropertyMapPtr Create(ECN::PrimitiveECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) { return new PrimitivePropertyMap(ecProperty, propertyAccessString, parentPropertyMap); }
    static PropertyMapPtr Clone(PrimitivePropertyMap const& proto, PropertyMap const* parentPropertyMap) { return new PrimitivePropertyMap(proto, parentPropertyMap); }
    };

//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PointPropertyMap : PropertyMap
    {
private:
    static const ECN::PrimitiveType s_defaultCoordinateECType = ECN::PRIMITIVETYPE_Double;

    bool m_is3d;
    DbColumn const* m_xColumn;
    DbColumn const* m_yColumn;
    DbColumn const* m_zColumn;

    PointPropertyMap(ECN::PrimitiveECPropertyCR pointProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PointPropertyMap(PointPropertyMap const& proto, PropertyMap const* parentPropertyMap)
        :PropertyMap(proto, parentPropertyMap), m_xColumn(proto.m_xColumn), m_yColumn(proto.m_yColumn), m_zColumn(proto.m_zColumn), m_is3d(proto.m_is3d)
        {}

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) override;
    void _GetColumns(std::vector<DbColumn const*>& columns) const;
    virtual BentleyStatus _Save(DbClassMapSaveContext&) const override;
    virtual BentleyStatus _Load(DbClassMapLoadContext const&) override;
    virtual BentleyStatus _GetPropertyPathList(std::vector<Utf8String>& propertyPathList)const override
        {
        propertyPathList.push_back(GetPropertyAccessString() + Utf8String(".X"));
        propertyPathList.push_back(GetPropertyAccessString() + Utf8String(".Y"));
        if (m_is3d)
            propertyPathList.push_back(GetPropertyAccessString() + Utf8String(".X"));

        return SUCCESS;
        }
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const override;
    BentleyStatus SetColumns(DbColumn const&, DbColumn const&, DbColumn const*);

public:
    ~PointPropertyMap() {}
    static PropertyMapPtr Create(ECN::PrimitiveECPropertyCR pointProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) { return new PointPropertyMap(pointProperty, propertyAccessString, parentPropertyMap); }
    static PropertyMapPtr Clone(PointPropertyMap const& proto, PropertyMap const* parentPropertyMap) { return new PointPropertyMap(proto, parentPropertyMap); }

    bool Is3d() const { return m_is3d; }

    static DbColumn::Type GetDefaultColumnType() { return DbColumn::PrimitiveTypeToColumnType(s_defaultCoordinateECType); }
};

struct ClassMapLoadContext;

/*---------------------------------------------------------------------------------------
* Maps an embedded ECStruct as inline
* @bsimethod                                                    affan.khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct StructPropertyMap : PropertyMap
    {
private:
    StructPropertyMap(ECN::StructECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    StructPropertyMap(ECDbMap const& ecdbMap, StructPropertyMap const& proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap);

    virtual void _GetColumns(std::vector<DbColumn const*>& columns) const override;
    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) override;
    virtual BentleyStatus _Load(DbClassMapLoadContext const&) override;
    virtual BentleyStatus _Save(DbClassMapSaveContext&) const override;
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const override;
    BentleyStatus Initialize(ClassMapLoadContext&, ECDbCR);

public:
    ~StructPropertyMap() {}
    static PropertyMapPtr Create(ClassMapLoadContext&, ECDbCR, ECN::StructECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    static PropertyMapPtr Clone(ECDbMap const& ecdbMap, StructPropertyMap const& proto, ECN::ECClassCR clonedBy, PropertyMap const* parentPropertyMap) { return new StructPropertyMap(ecdbMap, proto, clonedBy, parentPropertyMap); }
    };

//=======================================================================================
//! Mapping an array of primitives to a *single* column
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct PrimitiveArrayPropertyMap : SingleColumnPropertyMap
    {
private:
    ECN::StandaloneECEnablerP m_primitiveArrayEnabler;

    PrimitiveArrayPropertyMap(ECDbCR, ECN::ArrayECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    PrimitiveArrayPropertyMap(PrimitiveArrayPropertyMap const& proto, PropertyMap const* parentPropertyMap)
        :SingleColumnPropertyMap(proto, parentPropertyMap), m_primitiveArrayEnabler(proto.m_primitiveArrayEnabler)
        {}

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) override;
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const override;
public:
    ~PrimitiveArrayPropertyMap() {}
    static PropertyMapPtr Create(ECDbCR ecdb, ECN::ArrayECPropertyCR arrayProp, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) { return new PrimitiveArrayPropertyMap(ecdb, arrayProp, propertyAccessString, parentPropertyMap); }
    static PropertyMapPtr Clone(PrimitiveArrayPropertyMap const& proto, PropertyMap const* parentPropertyMap) { return new PrimitiveArrayPropertyMap(proto, parentPropertyMap); }
    };

//=======================================================================================
// @bsiclass                                                    Krischan.Eberle   03/2016
//+===============+===============+===============+===============+===============+======
struct StructArrayJsonPropertyMap : SingleColumnPropertyMap
    {
private:
    StructArrayJsonPropertyMap(ECN::StructArrayECPropertyCR structArrayProp, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) : SingleColumnPropertyMap(Type::StructArray, structArrayProp, propertyAccessString, parentPropertyMap) {}
    StructArrayJsonPropertyMap(StructArrayJsonPropertyMap const& proto, PropertyMap const* parentPropertyMap) :SingleColumnPropertyMap(proto, parentPropertyMap) {}

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) override;
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const override;
    ECN::StructArrayECPropertyCR GetStructArrayProperty() const { BeAssert(GetProperty().GetIsStructArray()); return *GetProperty().GetAsStructArrayProperty(); }

public:
    ~StructArrayJsonPropertyMap() {}

    static PropertyMapPtr Create(ECDbCR ecdb, ECN::StructArrayECPropertyCR structArrayProp, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) { return new StructArrayJsonPropertyMap(structArrayProp, propertyAccessString, parentPropertyMap); }
    static PropertyMapPtr Clone(StructArrayJsonPropertyMap const& proto, PropertyMap const* parentPropertyMap) { return new StructArrayJsonPropertyMap(proto, parentPropertyMap); }

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
    RelationshipClassMap const* m_relClassMap;
    std::vector<DbColumn const*> m_columns;
    ECN::ECClassCR m_ecClass; //cannot use GetProperty().GetClass() because a nav prop map can be cloned when inheriting.

    //NavigationProperties can only be created on EntityECClasses, therefore the ctors don't take a parent property map
    //which would be the struct property containing the prop
    NavigationPropertyMap(ClassMapLoadContext&, ECN::ECClassCR, ECN::NavigationECPropertyCR, Utf8CP propertyAccessString);
    NavigationPropertyMap(ClassMapLoadContext&, NavigationPropertyMap const& proto, ECN::ECClassCR targetClass);

    virtual void _GetColumns(std::vector<DbColumn const*>&) const override;

    //nothing to do when loading/saving the prop map
    virtual BentleyStatus _Load(DbClassMapLoadContext const&) override { return SUCCESS; }
    virtual BentleyStatus _Save(DbClassMapSaveContext&) const override { return SUCCESS; }
    virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive)  const override;

    ECN::ECRelationshipEnd GetConstraintEnd(NavigationEnd end) const { return GetConstraintEnd(GetNavigationProperty(), end); }

    ECN::NavigationECPropertyCR GetNavigationProperty() const { return *GetProperty().GetAsNavigationProperty(); }
    static ECN::ECRelationshipEnd GetConstraintEnd(ECN::NavigationECPropertyCR, NavigationEnd);

public:
    ~NavigationPropertyMap() {}
    static PropertyMapPtr Create(ClassMapLoadContext&, ECDbCR, ECN::ECClassCR, ECN::NavigationECPropertyCR, Utf8CP propertyAccessString);
    static PropertyMapPtr Clone(ClassMapLoadContext& ctx, NavigationPropertyMap const& proto, ECN::ECClassCR targetClass) { return new NavigationPropertyMap(ctx, proto, targetClass); }

    BentleyStatus Postprocess(ECDbMap const&);

    bool IsSupportedInECSql(bool logIfNotSupported = false, ECDbCP ecdb = nullptr) const;

    RelationshipClassMap const& GetRelationshipClassMap() const { BeAssert(m_relClassMap != nullptr); return *m_relClassMap; }

    RelationshipConstraintMap const& GetConstraintMap(NavigationEnd end) const;
    static ECN::ECRelationshipConstraintCR GetConstraint(ECN::NavigationECPropertyCR, NavigationEnd);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct PropertyMapFactory
    {
private:
    PropertyMapFactory();
    ~PropertyMapFactory();

public:
    //ECClass must be passed explicitly, as ECPropertyCR can be inherited from base class, in which case ECProperty::GetClass would
    //return the base class and not the class for which the prop map is to be created
    static PropertyMapPtr CreatePropertyMap(ClassMapLoadContext&, ECDbCR, ECN::ECClassCR, ECN::ECPropertyCR, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap);
    
    //only called during schema import
    static PropertyMapPtr ClonePropertyMap(ECDbMap const&, PropertyMapCR, ECN::ECClassCR targetClass, PropertyMap const* parentPropertyMap);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
