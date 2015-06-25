/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"
#include "PropertyMap.h"
#include "ClassMap.h"

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

    virtual bool _IsVirtual () const override;
    virtual bool _IsECInstanceIdPropertyMap () const override;
    virtual bool _IsSystemPropertyMap () const override;
    virtual MapStatus _FindOrCreateColumnsInTable (ClassMap& classMap, ClassMapInfo const* classMapInfo) override;
    virtual void _GetColumns (std::vector<ECDbSqlColumn const*>& columns) const override;
    virtual Utf8CP _GetColumnBaseName () const override;

protected:
    PropertyMapSystem (ECN::ECPropertyCR property, std::weak_ptr<ECDbSqlColumn> column, ECSqlSystemProperty kind, ECDbSqlTable const* primaryTable);

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
    static WCharCP const PROPERTYACCESSSTRING;

private:
    PropertyMapECInstanceId (ECN::ECPropertyCR ecInstanceIdProperty, ECDbSqlColumn* column);

    virtual bool _IsECInstanceIdPropertyMap () const override;
    virtual WString _ToString () const override;

public:
    ~PropertyMapECInstanceId () {}
    static PropertyMapPtr Create (ECDbSchemaManagerCR schemaManager, IClassMap const& classMap);
    };


//=======================================================================================
//! PropertyMapSecondaryTableKey is a property map for the system properties used to
//! identify a row in a secondary table.
/// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapSecondaryTableKey : PropertyMapSystem
    {
private:
    explicit PropertyMapSecondaryTableKey (ECN::ECPropertyCR systemProperty, ECDbSqlColumn* column, ECSqlSystemProperty kind);

    virtual WString _ToString () const override;

public:
    ~PropertyMapSecondaryTableKey () {}
    static PropertyMapPtr Create (ECDbSchemaManagerCR schemaManager, ECSqlSystemProperty kind, IClassMap const& classMap);
    };

//=======================================================================================
// @bsiclass                                                 Affan.Khan   06/2013
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraint : PropertyMapSystem
    {
private:
    Utf8String m_viewColumnAlias;

protected:
    PropertyMapRelationshipConstraint (ECN::ECPropertyCR constraintProperty, ECDbSqlColumn* column, ECSqlSystemProperty kind, ECDbSqlTable const* primaryTable, Utf8CP columnAliasInView);

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
    PropertyMapRelationshipConstraintECInstanceId (ECN::ECPropertyCR constraintProperty, ECDbSqlColumn* column, ECSqlSystemProperty kind, Utf8CP columnAliasInView);

    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const override;

    virtual WString _ToString () const override;

public:
    ~PropertyMapRelationshipConstraintECInstanceId () {}

    static PropertyMapPtr Create (ECN::ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, ECDbSqlColumn* column, IClassMap const& classMap, Utf8CP viewColumnAlias = nullptr);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct PropertyMapRelationshipConstraintClassId : PropertyMapRelationshipConstraint
    {
private:
    ECN::ECClassId m_defaultConstraintClassId;

    PropertyMapRelationshipConstraintClassId (ECN::ECPropertyCR constraintProperty, ECDbSqlColumn* column, ECSqlSystemProperty kind, ECN::ECClassId defaultClassId, ECDbSqlTable const* primaryTable, Utf8CP columnAliasInView, ECDbSqlTable* table);

    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const override;

    virtual WString _ToString () const override;

public:
    ~PropertyMapRelationshipConstraintClassId () {}

    ECN::ECClassId GetDefaultConstraintECClassId () const { return m_defaultConstraintClassId; }
    static PropertyMapPtr Create (ECN::ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, ECDbSqlColumn* column, ECN::ECClassId defaultSourceECClassId, IClassMap const& classMap, Utf8CP viewColumnAlias = nullptr, ECDbSqlTable* table = nullptr);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

