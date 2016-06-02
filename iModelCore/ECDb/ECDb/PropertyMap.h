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
#include "DebugWriter.h"
#include "ECDbSystemSchemaHelper.h"
#include "ECSql/NativeSqlBuilder.h"

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

struct StructArrayJsonPropertyMap;
struct NavigationPropertyMap;
struct ECClassIdRelationshipConstraintPropertyMap;

/*---------------------------------------------------------------------------------------
* Abstract class for property mapping
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyMap : RefCountedBase, NonCopyableClass
    {
protected:
    ECN::ECPropertyCR m_ecProperty;
    PropertyMapCP m_parentPropertyMap;
    Utf8String m_propertyAccessString; // We need to own this for nested property in embedded struct as they are dynamically generated
    std::vector<DbTable const*> m_mappedTables;
    PropertyMapCollection m_children;
    mutable PropertyPathId m_propertyPathId;

private:
    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, DbTable const* tableFilter) const;

    virtual bool _IsVirtual() const { return false; }
    virtual bool _IsECInstanceIdPropertyMap() const { return false; }
    virtual bool _IsSystemPropertyMap() const { return false; }

    virtual void _GetColumns(std::vector<DbColumn const*>&) const;

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) { return SUCCESS; }
    virtual BentleyStatus _Save(ClassDbMapping&) const;
    virtual BentleyStatus _Load(ClassDbMapping const&) { return SUCCESS; }
    virtual BentleyStatus _GetPropertyPathList(std::vector<Utf8String>& propertyPathList) const
        { 
        if (GetChildren().IsEmpty())
            {
            if (GetSingleColumn() == nullptr)
                return ERROR;

            propertyPathList.push_back(GetPropertyAccessString());
            }
        else
            {
            for (PropertyMapCP child : GetChildren())
                {
                if (child->GetPropertyPathList(propertyPathList) != SUCCESS)
                    return ERROR;
                }
            }
        return SUCCESS; 
        }
    virtual StructArrayJsonPropertyMap const* _GetAsStructArrayPropertyMap() const { return nullptr; }
    virtual NavigationPropertyMap const* _GetAsNavigationPropertyMap() const { return nullptr; }
    virtual ECClassIdRelationshipConstraintPropertyMap const* _GetAsECClassIdRelationshipConstraintPropertyMapRelationship() const { return nullptr; }

    virtual Utf8String _ToString() const = 0;
    virtual void _WriteDebugInfo(DebugWriter& writer) const;
protected:
    PropertyMap(ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) : m_ecProperty(ecProperty), m_propertyAccessString(propertyAccessString), m_parentPropertyMap(parentPropertyMap) {}
    PropertyMap(PropertyMapCR rhs, PropertyMapCP parentPropertyMap) : m_ecProperty(rhs.m_ecProperty), m_parentPropertyMap(parentPropertyMap), m_propertyAccessString(rhs.m_propertyAccessString),
        m_mappedTables(rhs.m_mappedTables), m_propertyPathId(rhs.m_propertyPathId)
        {}

    PropertyMapCR GetRoot() const;

    BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation& collation, ECDbCR ecdb) const { return DetermineColumnInfo(columnName, isNullable, isUnique, collation, ecdb, GetProperty(), GetPropertyAccessString()); }

public:
    virtual ~PropertyMap() {}
    PropertyPathId GetPropertyPathId() const { BeAssert(m_propertyPathId.IsValid()); return m_propertyPathId; }
    NavigationPropertyMap const* GetAsNavigationPropertyMap() const { return _GetAsNavigationPropertyMap(); }
    StructArrayJsonPropertyMap const* GetAsStructArrayPropertyMap() const { return _GetAsStructArrayPropertyMap(); }
    ECClassIdRelationshipConstraintPropertyMap const* GetAsECClassIdRelationshipConstraintPropertyMap() const { return _GetAsECClassIdRelationshipConstraintPropertyMapRelationship(); }
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

    BentleyStatus Save(ClassDbMapping& mapping) const { return _Save(mapping); }
    BentleyStatus Load(ClassDbMapping const& mapping) { return _Load(mapping); }
    
    //! Make sure our table has the necessary columns, if any
    BentleyStatus FindOrCreateColumnsInTable(ClassMap const&);

    //! Returns whether this property map refers to the ECInstanceId system property or not.
    //! @return true if the property map refers to the ECInstanceId system property. false otherwise.
    bool IsECInstanceIdPropertyMap() const { return _IsECInstanceIdPropertyMap(); }

    //! Returns whether this property map refers to an ECSQL system property or not.
    //! @return true if the property map refers to an ECSQL system property. false otherwise.
    bool IsSystemPropertyMap() const { return _IsSystemPropertyMap(); }

    Utf8String ToString() const { return _ToString(); }

    static BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation&, ECDbCR, ECN::ECPropertyCR, Utf8CP propAccessString);
    void WriteDebugInfo(DebugWriter& writer) const { _WriteDebugInfo(writer); }
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

    virtual BentleyStatus _Load(ClassDbMapping const& classMapInfo) override;
    virtual void _GetColumns(std::vector<DbColumn const*>& columns) const override;
    void SetColumn(DbColumn const&);
    virtual BentleyStatus _GetPropertyPathList(std::vector<Utf8String>& propertyPathList)const override  { propertyPathList.push_back(GetPropertyAccessString());  return SUCCESS; }

protected:
    SingleColumnPropertyMap (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
        : PropertyMap(ecProperty, propertyAccessString, parentPropertyMap), m_column(nullptr) {}

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
    virtual Utf8String _ToString() const override;

    PrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) : SingleColumnPropertyMap(ecProperty, propertyAccessString, parentPropertyMap) {}
    PrimitivePropertyMap(PrimitivePropertyMap const& proto, PropertyMap const* parentPropertyMap) :SingleColumnPropertyMap(proto, parentPropertyMap) {}

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
    virtual BentleyStatus _Save(ClassDbMapping&) const override;
    virtual BentleyStatus _Load(ClassDbMapping const&) override;
    virtual Utf8String _ToString() const override;
    virtual BentleyStatus _GetPropertyPathList(std::vector<Utf8String>& propertyPathList)const override
        {
        propertyPathList.push_back(GetPropertyAccessString() + Utf8String(".X"));
        propertyPathList.push_back(GetPropertyAccessString() + Utf8String(".Y"));
        if (m_is3d)
            propertyPathList.push_back(GetPropertyAccessString() + Utf8String(".X"));

        return SUCCESS;
        }

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
    virtual BentleyStatus _Load(ClassDbMapping const&) override;
    virtual BentleyStatus _Save(ClassDbMapping&) const override;

    virtual Utf8String _ToString() const override;

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
    virtual Utf8String _ToString() const override;

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
    StructArrayJsonPropertyMap(ECN::StructArrayECPropertyCR structArrayProp, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) : SingleColumnPropertyMap(structArrayProp, propertyAccessString, parentPropertyMap) {}
    StructArrayJsonPropertyMap(StructArrayJsonPropertyMap const& proto, PropertyMap const* parentPropertyMap) :SingleColumnPropertyMap(proto, parentPropertyMap) {}

    virtual BentleyStatus _FindOrCreateColumnsInTable(ClassMap const&) override;
    virtual StructArrayJsonPropertyMap const* _GetAsStructArrayPropertyMap() const override { return this; }
    virtual Utf8String _ToString() const override;

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
    std::vector<DbColumn const* > m_columns;
    ECN::ECClassCR m_ecClass; //cannot use GetProperty().GetClass() because a nav prop map can be cloned when inheriting.

    //NavigationProperties can only be created on EntityECClasses, therefore the ctors don't take a parent property map
    //which would be the struct property containing the prop
    NavigationPropertyMap(ClassMapLoadContext&, ECN::ECClassCR, ECN::NavigationECPropertyCR, Utf8CP propertyAccessString);
    NavigationPropertyMap(ClassMapLoadContext&, NavigationPropertyMap const& proto, ECN::ECClassCR targetClass);

    virtual void _GetColumns(std::vector<DbColumn const*>&) const override;
    virtual Utf8String _ToString() const override { return Utf8PrintfString("NavigationPropertyMap: ecProperty=%s.%s", m_ecProperty.GetClass().GetFullName(), m_ecProperty.GetName().c_str()); }

    //nothing to do when loading/saving the prop map
    virtual BentleyStatus _Load(ClassDbMapping const&) override { return SUCCESS; }
    virtual BentleyStatus _Save(ClassDbMapping&) const override { return SUCCESS; }
    virtual NavigationPropertyMap const* _GetAsNavigationPropertyMap() const override { return this; }

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
