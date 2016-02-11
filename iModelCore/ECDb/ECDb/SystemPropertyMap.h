/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"
#include "PropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! PropertyMapSystem is a property map for the ECSQL system properties 
// @bsiclass                                                 Krischan.Eberle    02/2014
//+===============+===============+===============+===============+===============+======
struct PropertyMapSystem : PropertyMap
    {
private:
    ECSqlSystemProperty m_kind;
    std::vector<std::weak_ptr<ECDbSqlColumn>> m_columns;

    virtual bool _IsVirtual () const override { return !m_columns.front().expired() && m_columns.front().lock()->GetPersistenceType() == PersistenceType::Virtual; }
    virtual bool _IsSystemPropertyMap() const override { return true; }
    virtual void _GetColumns (std::vector<ECDbSqlColumn const*>& columns) const override;

protected:
    PropertyMapSystem (ECN::ECPropertyCR property, std::vector<ECDbSqlColumn const*> columns, ECSqlSystemProperty kind);

    ECDbSqlColumn const& GetColumn () const;

    ECSqlSystemProperty GetKind () const { return m_kind; }
    ColumnKind ToColumnKind() const;
    static std::vector<std::weak_ptr<ECDbSqlColumn>> ToWeakPtr(std::vector<ECDbSqlColumn const*> const& columns);
    std::vector<std::weak_ptr<ECDbSqlColumn>>& GetColumnWeakPtrs ()  { return m_columns; }   

public:
    virtual ~PropertyMapSystem () {}

    ECDbSqlColumn const* GetColumn(ECDbSqlTable const& table) const;

    static std::vector<ECDbSqlColumn const*> ToVector(ECDbSqlColumn const* column);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapECInstanceId : PropertyMapSystem
    {
public:
    static Utf8CP const PROPERTYACCESSSTRING;

private:
    PropertyMapECInstanceId(ECN::ECPropertyCR ecInstanceIdProperty, ClassMap const&, std::vector<ECDbSqlColumn const*>);

    virtual bool _IsECInstanceIdPropertyMap() const override { return true; }
    virtual Utf8String _ToString () const override;

public:
    ~PropertyMapECInstanceId() {}
    static PropertyMapPtr Create(ECDbSchemaManagerCR, ClassMap const&);
    static PropertyMapPtr Create(ECDbSchemaManagerCR, ClassMap const&, std::vector<ECDbSqlColumn const*>);
    };


//=======================================================================================
//! PropertyMapStructArrayTableKey is a property map for the system properties used to
//! identify a row in a secondary table.
/// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapStructArrayTableKey : PropertyMapSystem
    {
private:
    explicit PropertyMapStructArrayTableKey (ECN::ECPropertyCR systemProperty, std::vector<ECDbSqlColumn const*>, ECSqlSystemProperty);

    virtual Utf8String _ToString () const override;

public:
    ~PropertyMapStructArrayTableKey () {}
    static PropertyMapPtr Create (ECDbSchemaManagerCR, ECSqlSystemProperty, IClassMap const&);
    };

//=======================================================================================
// @bsiclass                                                 Affan.Khan   06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraint : PropertyMapSystem
    {
private:
    Utf8String m_viewColumnAlias;

protected:
    PropertyMapRelationshipConstraint (ECN::ECPropertyCR constraintProperty, std::vector<ECDbSqlColumn const*>, ECSqlSystemProperty, Utf8CP endTableColumnAlias);
    bool HasViewColumnAlias () const { return !m_viewColumnAlias.empty (); }
    //! In the view generated for select statements, the constraint columns cannot be used directly for end-table mappings
    //! as the this end's columns are actually the key columns of the end table.
    //! Therefore an alias is used which is returned by this method.
    //! @return view column alias
    Utf8CP GetViewColumnAlias () const { return m_viewColumnAlias.c_str (); }

public:
    virtual ~PropertyMapRelationshipConstraint () {}

    //! This is only to be called during SELECT view generation. It appends the column name
    //! and, if set, the view column alias to to the select clause of the view.
    //! @param[in,out] viewSql View SQL builder to append to
    void AppendSelectClauseSqlSnippetForView (NativeSqlBuilder& viewSql, ECDbSqlTable const& table) const;
    void AppendSelectClauseSqlSnippetForView (NativeSqlBuilder& viewSql) const;

    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraintECInstanceId : PropertyMapRelationshipConstraint
    {
private:
    PropertyMapRelationshipConstraintECInstanceId(ECN::ECPropertyCR constraintProperty, std::vector<ECDbSqlColumn const*>, ECSqlSystemProperty, Utf8CP endTableColumnAlias);

    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const override;
    virtual Utf8String _ToString() const override;

public:
    ~PropertyMapRelationshipConstraintECInstanceId() {}

    static PropertyMapPtr Create(ECN::ECRelationshipEnd, ECDbSchemaManagerCR, std::vector<ECDbSqlColumn const*>, Utf8CP endTableColumnAlias = nullptr);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraintClassId : PropertyMapRelationshipConstraint
    {
private:
    ECN::ECClassId m_defaultConstraintClassId;
    bool m_isMappedToClassMapTables;

    PropertyMapRelationshipConstraintClassId (ECN::ECPropertyCR constraintProperty, std::vector<ECDbSqlColumn const*>, ECSqlSystemProperty, ECN::ECClassId defaultClassId, ClassMap const&, Utf8CP endTableColumnAlias, bool colIsDelayGenerated);

    virtual PropertyMapRelationshipConstraintClassId const* _GetAsPropertyMapRelationshipConstraintClassId() const override { return this; }
    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const override;
    virtual Utf8String _ToString () const override;

public:
    ~PropertyMapRelationshipConstraintClassId () {}

    ECN::ECClassId GetDefaultConstraintECClassId () const { return m_defaultConstraintClassId; }
    static RefCountedPtr<PropertyMapRelationshipConstraintClassId> Create (ECN::ECRelationshipEnd, ECDbSchemaManagerCR, std::vector<ECDbSqlColumn const*>, ECN::ECClassId defaultSourceECClassId, ClassMap const&, Utf8CP endTableColumnAlias = nullptr, bool colIsDelayGenerated = false);

    //!ConstraintClassId columns are not always created in the table to which the relationship is mapped to. 
    //!If this method returns false, the relationship table doesn't have a constraint class id column, but the class id
    //!column resides elsewhere.
    bool IsMappedToClassMapTables() const { return m_isMappedToClassMapTables; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

