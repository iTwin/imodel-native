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
    std::weak_ptr<ECDbSqlColumn> m_column;

    virtual bool _IsVirtual () const override { return !m_column.expired() && m_column.lock()->GetPersistenceType() == PersistenceType::Virtual; }
    virtual bool _IsSystemPropertyMap() const override { return true; }
    virtual void _GetColumns (std::vector<ECDbSqlColumn const*>& columns) const override;

protected:
    PropertyMapSystem (ECN::ECPropertyCR property, std::weak_ptr<ECDbSqlColumn> column, ECSqlSystemProperty kind);

    ECDbSqlColumn const& GetColumn () const;

    void ReplaceColumn (std::weak_ptr<ECDbSqlColumn> column) { m_column = column; }
    ECSqlSystemProperty GetKind () const { return m_kind; }

public:
    virtual ~PropertyMapSystem () {}
    std::weak_ptr<ECDbSqlColumn> GetColumnWeakPtr () const { return m_column; }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapECInstanceId : PropertyMapSystem
    {
public:
    static Utf8CP const PROPERTYACCESSSTRING;

private:
    PropertyMapECInstanceId (ECN::ECPropertyCR ecInstanceIdProperty, ECDbSqlColumn*);

    virtual bool _IsECInstanceIdPropertyMap() const override { return true; }
    virtual Utf8String _ToString () const override;

public:
    ~PropertyMapECInstanceId () {}
    static PropertyMapPtr Create (ECDbSchemaManagerCR, ClassMap const&);
    };


//=======================================================================================
//! PropertyMapStructArrayTableKey is a property map for the system properties used to
//! identify a row in a secondary table.
/// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapStructArrayTableKey : PropertyMapSystem
    {
private:
    explicit PropertyMapStructArrayTableKey (ECN::ECPropertyCR systemProperty, ECDbSqlColumn*, ECSqlSystemProperty);

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
    PropertyMapRelationshipConstraint (ECN::ECPropertyCR constraintProperty, ECDbSqlColumn*, ECSqlSystemProperty, Utf8CP columnAliasInView);

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
    void AppendSelectClauseSqlSnippetForView (NativeSqlBuilder& viewSql) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraintECInstanceId : PropertyMapRelationshipConstraint
    {
private:
    PropertyMapRelationshipConstraintECInstanceId(ECN::ECPropertyCR constraintProperty, ECDbSqlColumn*, ECSqlSystemProperty, Utf8CP columnAliasInView);

    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses) const override;
    virtual Utf8String _ToString() const override;

public:
    ~PropertyMapRelationshipConstraintECInstanceId() {}

    static PropertyMapPtr Create(ECN::ECRelationshipEnd, ECDbSchemaManagerCR, ECDbSqlColumn*, Utf8CP viewColumnAlias = nullptr);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraintClassId : PropertyMapRelationshipConstraint
    {
private:
    ECN::ECClassId m_defaultConstraintClassId;
    bool m_isMappedToClassMapTables;

    PropertyMapRelationshipConstraintClassId (ECN::ECPropertyCR constraintProperty, ECDbSqlColumn*, ECSqlSystemProperty, ECN::ECClassId defaultClassId, ClassMap const&, Utf8CP columnAliasInView, bool colIsDelayGenerated);

    virtual PropertyMapRelationshipConstraintClassId const* _GetAsPropertyMapRelationshipConstraintClassId() const override { return this; }
    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses) const override;
    virtual Utf8String _ToString () const override;

public:
    ~PropertyMapRelationshipConstraintClassId () {}

    ECN::ECClassId GetDefaultConstraintECClassId () const { return m_defaultConstraintClassId; }
    static RefCountedPtr<PropertyMapRelationshipConstraintClassId> Create (ECN::ECRelationshipEnd, ECDbSchemaManagerCR, ECDbSqlColumn*, ECN::ECClassId defaultSourceECClassId, ClassMap const&, Utf8CP viewColumnAlias = nullptr, bool colIsDelayGenerated = false);

    //!ConstraintClassId columns are not always created in the table to which the relationship is mapped to. 
    //!If this method returns false, the relationship table doesn't have a constraint class id column, but the class id
    //!column resides elsewhere.
    bool IsMappedToClassMapTables() const { return m_isMappedToClassMapTables; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

