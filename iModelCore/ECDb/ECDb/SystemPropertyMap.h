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
struct SystemPropertyMap : PropertyMap
    {
    private:
        ECSqlSystemProperty m_kind;
        std::vector<std::weak_ptr<DbColumn>> m_columns;

        virtual bool _IsVirtual() const override { return !m_columns.front().expired() && m_columns.front().lock()->GetPersistenceType() == PersistenceType::Virtual; }
        virtual void _GetColumns(std::vector<DbColumn const*>& columns) const override;
        virtual void _QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const override;
    protected:
        SystemPropertyMap(Type, ECN::ECPropertyCR, std::vector<DbColumn const*>, ECSqlSystemProperty);
        SystemPropertyMap(SystemPropertyMap const& proto)
            :PropertyMap(proto, nullptr), m_kind(proto.m_kind), m_columns(proto.m_columns)
            {}

        DbColumn const& GetColumn() const;

        ECSqlSystemProperty GetKind() const { return m_kind; }
        std::vector<std::weak_ptr<DbColumn>>& GetColumnWeakPtrs() { return m_columns; }

    public:
        virtual ~SystemPropertyMap() {}
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdPropertyMap : SystemPropertyMap
    {
private:
    ECInstanceIdPropertyMap(ECN::ECPropertyCR ecInstanceIdProperty, ClassMap const&, std::vector<DbColumn const*>);
    ECInstanceIdPropertyMap(ECInstanceIdPropertyMap const& proto) :SystemPropertyMap(proto) {}

public:
    ~ECInstanceIdPropertyMap() {}
    static PropertyMapPtr Create(ECDbSchemaManagerCR, ClassMap const&);
    static PropertyMapPtr Create(ECDbSchemaManagerCR, ClassMap const&, std::vector<DbColumn const*>);
    static PropertyMapPtr Clone(ECInstanceIdPropertyMap const& proto) { return new ECInstanceIdPropertyMap(proto); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct ECClassIdPropertyMap : SystemPropertyMap
    {
private:
    ECN::ECClassId m_defaultConstraintClassId;
    ECClassIdPropertyMap(ECN::ECPropertyCR ecInstanceIdProperty, ClassMap const&, std::vector<DbColumn const*>);
    ECClassIdPropertyMap(ECClassIdPropertyMap const& proto) :SystemPropertyMap(proto), m_defaultConstraintClassId(proto.m_defaultConstraintClassId) {}
    virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, DbTable const* tableFilter) const override;
public:
    ~ECClassIdPropertyMap() {}
    static PropertyMapPtr Create(ECDbSchemaManagerCR, ClassMap const&, std::vector<DbColumn const*>);
    static PropertyMapPtr Clone(ECClassIdPropertyMap const& proto) { return new ECClassIdPropertyMap(proto); }
    ECN::ECClassId GetDefaultConstraintClassId() const { return m_defaultConstraintClassId; }
    bool IsPersisted() const { return GetSingleColumn()->GetPersistenceType() == PersistenceType::Persisted; }

    };

//=======================================================================================
// @bsiclass                                                 Affan.Khan   06/2013
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintPropertyMap : SystemPropertyMap
    {
    private:
        Utf8String m_viewColumnAlias;

    protected:
        RelationshipConstraintPropertyMap(Type, ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*>, ECSqlSystemProperty, Utf8CP endTableColumnAlias);
        RelationshipConstraintPropertyMap(RelationshipConstraintPropertyMap const& proto) :SystemPropertyMap(proto), m_viewColumnAlias(proto.m_viewColumnAlias) {}

        bool HasViewColumnAlias() const { return !m_viewColumnAlias.empty(); }
        //! In the view generated for select statements, the constraint columns cannot be used directly for end-table mappings
        //! as the this end's columns are actually the key columns of the end table.
        //! Therefore an alias is used which is returned by this method.
        //! @return view column alias
        Utf8CP GetViewColumnAlias() const { return m_viewColumnAlias.c_str(); }

    public:
        virtual ~RelationshipConstraintPropertyMap() {}

        //! This is only to be called during SELECT view generation. It appends the column name
        //! and, if set, the view column alias to to the select clause of the view.
        //! @param[in,out] viewSql View SQL builder to append to
        void AppendSelectClauseSqlSnippetForView(NativeSqlBuilder& viewSql, DbTable const&) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintECInstanceIdPropertyMap : RelationshipConstraintPropertyMap
    {
    private:
        RelationshipConstraintECInstanceIdPropertyMap(ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*>, ECSqlSystemProperty, Utf8CP endTableColumnAlias);
        RelationshipConstraintECInstanceIdPropertyMap(RelationshipConstraintECInstanceIdPropertyMap const& proto) : RelationshipConstraintPropertyMap(proto) {}

        virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, DbTable const* tableFilter) const override;

    public:
        ~RelationshipConstraintECInstanceIdPropertyMap() {}

        static PropertyMapPtr Create(ECN::ECRelationshipEnd, ECDbSchemaManagerCR, std::vector<DbColumn const*>, Utf8CP endTableColumnAlias = nullptr);
        static PropertyMapPtr Clone(RelationshipConstraintECInstanceIdPropertyMap const& proto) { return new RelationshipConstraintECInstanceIdPropertyMap(proto); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       01/2014
//+===============+===============+===============+===============+===============+======
struct RelConstraintECClassIdPropertyMap : RelationshipConstraintPropertyMap
    {
    private:
        ECN::ECClassId m_defaultConstraintClassId;
        bool m_isMappedToClassMapTables;

        RelConstraintECClassIdPropertyMap(ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*>, ECSqlSystemProperty, ECN::ECClassId defaultClassId, ClassMap const&, Utf8CP endTableColumnAlias, bool colIsDelayGenerated);
        RelConstraintECClassIdPropertyMap(RelConstraintECClassIdPropertyMap const& proto) : RelationshipConstraintPropertyMap(proto) {}

        virtual NativeSqlBuilder::List _ToNativeSql(Utf8CP classIdentifier, ECSqlType, bool wrapInParentheses, DbTable const* tableFilter) const override;

    public:
        ~RelConstraintECClassIdPropertyMap() {}

        ECN::ECClassId GetDefaultConstraintECClassId() const { return m_defaultConstraintClassId; }
        static RefCountedPtr<RelConstraintECClassIdPropertyMap> Create(ECN::ECRelationshipEnd, ECDbSchemaManagerCR, std::vector<DbColumn const*>, ECN::ECClassId defaultSourceECClassId, ClassMap const&, Utf8CP endTableColumnAlias = nullptr, bool colIsDelayGenerated = false);
        static PropertyMapPtr Clone(RelConstraintECClassIdPropertyMap const& proto) { return new RelConstraintECClassIdPropertyMap(proto); }

        //!ConstraintClassId columns are not always created in the table to which the relationship is mapped to. 
        //!If this method returns false, the relationship table doesn't have a constraint class id column, but the class id
        //!column resides elsewhere.
        bool IsMappedToClassMapTables() const { return m_isMappedToClassMapTables; }
        bool IsMappedToECClassId() const
            {
            return Enum::Contains(GetSingleColumn()->GetKind(), DbColumn::Kind::ECClassId);
            }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

