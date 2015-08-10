/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
enum class SqlOption
    {
    Create,
    CreateIfNotExist,
    Drop,
    DropIfExists
    };

struct ECSqlPrepareContext;

struct SqlTriggerBuilder
    {
    public:
        enum class Condition
            {
            After,
            Before,
            InsteadOf
            };
        enum class Type
            {
            Insert,
            Update,
            UpdateOf,
            Delete
            };

    private:
        NativeSqlBuilder m_name;
        NativeSqlBuilder m_when;
        NativeSqlBuilder m_body;
        NativeSqlBuilder m_on;
        bool m_temprory;
        Type m_type;
        Condition m_condition;
        std::unique_ptr<std::vector<Utf8String>> m_ofColumns;

    public:
        SqlTriggerBuilder (Type type, Condition condition, bool temprary)
            :m_type (type), m_condition (condition), m_temprory (temprary)
            {
            if (m_type == Type::UpdateOf)
                m_ofColumns = std::unique_ptr<std::vector<Utf8String>> (new std::vector<Utf8String> ());
            }
        SqlTriggerBuilder (SqlTriggerBuilder&& rhs)
            :m_type (std::move (rhs.m_type)), m_condition (std::move (rhs.m_condition)), m_temprory (std::move (rhs.m_temprory)), m_name (std::move (rhs.m_name)),
            m_when (std::move (rhs.m_when)), m_body (std::move (rhs.m_body)), m_on (rhs.m_on)
            {
            }
        NativeSqlBuilder& GetNameBuilder () { return m_name; }
        NativeSqlBuilder& GetWhenBuilder () { return m_when; }
        NativeSqlBuilder& GetBodyBuilder () { return m_body; }
        NativeSqlBuilder& GetOnBuilder () { return m_on; }
        Type GetType () const { return m_type; }
        Condition GetCondition () const { return m_condition; }
        std::vector<Utf8String> const* GetColumns () const
            {
            BeAssert (m_type == Type::UpdateOf);
            return m_ofColumns.get ();
            }
        std::vector<Utf8String>* GetUpdateOfColumnsP ()
            {
            BeAssert (m_type == Type::UpdateOf);
            return m_ofColumns.get ();
            }
        Utf8CP GetName () const { return m_name.ToString (); }
        Utf8CP GetWhen () const { return m_when.ToString (); }
        Utf8CP GetBody () const { return m_body.ToString (); }
        Utf8CP GetOn () const { return m_on.ToString (); }
        bool IsTemprory () const { return m_temprory; }
        bool IsValid () const
            {
            if (m_name.IsEmpty ())
                {
                BeAssert (false && "Must specify a trigger name");
                return false;
                }

            if (m_on.IsEmpty ())
                {
                BeAssert (false && "Must specify a trigger ON Table/View");
                return false;
                }

            if (m_body.IsEmpty ())
                {
                BeAssert (false && "Must specify a trigger body");
                return false;
                }

            if (m_type == Type::UpdateOf && m_ofColumns->empty ())
                {
                BeAssert (false && "For UPDATE OF trigger must specify atleast one column");
                return false;
                }

            return true;
            }
        const Utf8String ToString (SqlOption option, bool escape) const
            {
            if (!IsValid ())
                {
                BeAssert (false && "Trigger specification is not valid");
                }

            NativeSqlBuilder sql;
            if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
                {
                sql.Append ("DROP TRIGGER ").AppendIf (option == SqlOption::DropIfExists, "IF EXISTS ").AppendEscapedIf (escape, GetName ()).Append (";");
                }
            else
                {
                sql.AppendLine ("--### WARNING: SYSTEM GENERATED TRIGGER. DO NOT CHANGE THIS TRIGGER IN ANYWAY. ####");
                sql.Append ("CREATE TRIGGER ").AppendIf (IsTemprory (), "TEMP ").AppendIf (option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").AppendEscapedIf (escape, GetName ()).AppendEOL ();
                switch (m_condition)
                    {
                    case Condition::After:
                        sql.Append ("AFTER "); break;
                    case Condition::Before:
                        sql.Append ("BEFORE "); break;
                    case Condition::InsteadOf:
                        sql.Append ("INSTEAD OF "); break;
                    }

                switch (m_type)
                    {
                    case Type::Delete:
                        sql.Append ("DELETE "); break;
                    case Type::Insert:
                        sql.Append ("INSERT "); break;
                    case Type::Update:
                        sql.Append ("UPDATE "); break;
                    case Type::UpdateOf:
                        sql.Append ("UPDATE OF ");
                        for (auto& column : *m_ofColumns)
                            {
                            if (&column != &m_ofColumns->front ())
                                sql.Append (", ");

                            sql.AppendEscapedIf (escape, column.c_str ());
                            }
                        break;
                    }

                sql.AppendEOL ();
                sql.Append ("ON ").AppendEscapedIf (escape, GetOn ()).AppendEOL ();
                if (!m_when.IsEmpty ())
                    {
                    sql.Append ("\tWHEN ").Append (GetWhen ()).AppendEOL ();
                    }

                sql.Append ("BEGIN").AppendEOL ();
                sql.Append (GetBody ());
                sql.Append ("END;").AppendEOL ();;
                }

            return sql.ToString ();
            }
    };
    struct SqlViewBuilder 
        {
        private:
            NativeSqlBuilder m_name;
            NativeSqlBuilder::List m_selectList;
            bool m_isTmp;
            bool m_isNullView;
        public:
            SqlViewBuilder ()
                :m_isTmp (false), m_isNullView (false)
                {
                }
            void MarkAsNullView ()
                {
                m_isNullView = true;
                }
           
            bool IsNullView () const { return m_isNullView; }
            NativeSqlBuilder& GetNameBuilder ()  { return m_name; }
            void SetTemprory (bool tmp) { m_isTmp = tmp; }
            NativeSqlBuilder& AddSelect ()
                {
                m_selectList.push_back (NativeSqlBuilder ());
                return m_selectList.back ();
                }
            bool IsEmpty () const
                {
                return m_selectList.empty () && m_name.IsEmpty ();
                }
            bool IsValid () const
                {
                if (m_name.IsEmpty ())
                    {
                    BeAssert (false && "Must specify a view name");
                    return false;
                    }

                if (m_selectList.empty ())
                    {
                    BeAssert (false && "View must have atleast one select statement");
                    return false;
                    }

                return true;
                }
            Utf8CP GetName () const{ return m_name.ToString (); }
            bool IsTemprory () const { return m_isTmp; }
            bool IsCompound () { return m_selectList.size () > 1; }
            const Utf8String ToString (SqlOption option, bool escape = false, bool useUnionAll = true) const
                {
                if (!IsValid ())
                    {
                    BeAssert (false && "view specification is not valid");
                    }

                NativeSqlBuilder sql;
                if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
                    {
                    sql.Append ("DROP VIEW ").AppendIf (option == SqlOption::DropIfExists, "IF EXISTS ").AppendEscapedIf (escape, GetName ()).Append (";");
                    }
                else
                    {
                    sql.AppendLine ("--### WARNING: SYSTEM GENERATED VIEW. DO NOT CHANGE THIS VIEW IN ANYWAY. ####");
                    sql.Append ("CREATE ").AppendIf (IsTemprory (), "TEMP ").Append ("VIEW ").AppendIf (option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").AppendEscapedIf (escape, GetName ()).AppendEOL ();
                    sql.Append ("AS").AppendEOL ();
                    for (auto& select : m_selectList)
                        {
                        if (&select != &m_selectList.front ())
                            sql.AppendTAB ().Append ("UNION ").AppendIf (useUnionAll, "ALL").AppendEOL ();

                        sql.AppendTAB (2).AppendLine (select.ToString ());
                        }    
                    sql.Append (";");
                    }

                return sql.ToString ();
                }
        };



    struct SqlClassPersistenceMethod : NonCopyableClass
    {
    private:
        SqlViewBuilder m_viewBuilder;
        NativeSqlBuilder m_tableName;
        NativeSqlBuilder m_rowFilter;
        std::vector<SqlTriggerBuilder> m_triggerBuilderList;
        ClassMapCR m_classMap;
    public:
        SqlClassPersistenceMethod (ClassMapCR classMap)
            :m_classMap (classMap)
            {}

        ~SqlClassPersistenceMethod ()
            {}
        ClassMapCR GetClassMap () const { return m_classMap; }
        NativeSqlBuilder& GetTableNameBuilder () { return m_tableName; }
        NativeSqlBuilder& GetRowFilterBuilder () { return m_rowFilter; }
        SqlViewBuilder& GetViewBuilder () { return m_viewBuilder; }
       
        SqlTriggerBuilder& AddTrigger (SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary)
            {
            m_triggerBuilderList.push_back (SqlTriggerBuilder (type, condition, temprary));
            return m_triggerBuilderList.back ();
            }
        Utf8CP GetAffectedTargetId (DMLPolicy::Operation op) const
            {
            auto target = GetClassMap().GetDMLPolicy ().Get (op);
            if (target == DMLPolicy::Target::None)
                return "";

            if (target == DMLPolicy::Target::Table)
                return GetClassMap ().GetTable ().GetName ().c_str();

            return const_cast<SqlClassPersistenceMethod*>(this)->GetViewBuilder ().GetNameBuilder ().ToString ();
            }

        std::vector<SqlTriggerBuilder> const& GetTriggerBuilderList () { return m_triggerBuilderList; }

        Utf8String ToString (SqlOption option, bool escape = false)
            {
            NativeSqlBuilder sql;
            sql.AppendLine ("--#-----------------------View--------------------------");
            sql.AppendLine (m_viewBuilder.ToString (option, escape).c_str());

            if (!m_triggerBuilderList.empty ())
                sql.AppendLine ("--#-----------------------Trigger-----------------------");
            for (auto& trigger : m_triggerBuilderList)
                {
                sql.AppendLine (trigger.ToString (option, escape).c_str());
                }

            return sql.ToString();
            }
    };






        
//=======================================================================================
// @bsiclass                                               Affan.Khan           06/2015
//+===============+===============+===============+===============+===============+======
struct SqlGenerator
    {

    private:
        ECDbMapR m_map;
        std::map<ECN::ECClassId, std::unique_ptr<SqlClassPersistenceMethod>> m_scpms;
    private:
        const std::vector<ClassMapCP> GetEndClassMaps (ECN::ECRelationshipClassCR relationship, ECN::ECRelationshipEnd end);
         BentleyStatus BuildHoldingConstraint (NativeSqlBuilder& stmt, RelationshipClassMapCR classMap);
         BentleyStatus BuildEmbeddingConstraint (NativeSqlBuilder& stmt, RelationshipClassMapCR classMap);

         SqlClassPersistenceMethod* GetClassPersistenceMethod (ClassMapCR classMap);
         BentleyStatus BuildHoldingView (NativeSqlBuilder& sql);

         BentleyStatus FindRelationshipReferences (bmap<RelationshipClassMapCP, ECDbMap::LightWeightMapCache::RelationshipEnd>& relationships, ClassMapCR classMap);
         void CollectDerivedEndTableRelationships (std::set<RelationshipClassEndTableMapCP>& childMaps, RelationshipClassMapCR classMap);
        
         BentleyStatus BuildDerivedFilterClause (Utf8StringR filter, ECDb& db, ECN::ECClassId baseClassId);
         Utf8CP GetECClassIdPrimaryTableAlias (ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }
         BentleyStatus BuildECInstanceIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildECClassIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, bool topLevel);
         BentleyStatus BuildEndTableRelationshipView (NativeSqlBuilder::List& viewSql, RelationshipClassMapCR classMap);

         BentleyStatus BuildDeleteTriggersForDerivedClasses (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggerForMe (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggerForEndTableMe (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggersForRelationships (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggers (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggerForStructArrays (SqlClassPersistenceMethod& scpm);

         BentleyStatus BuildPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildColumnExpression (NativeSqlBuilder::List& viewSql, Utf8CP tablePrefix, Utf8CP columnName, Utf8CP accessString, bool addECPropertyPathAlias, bool nullValue, bool escapeColumName = true);
         BentleyStatus BuildPointPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapPoint const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildPrimitivePropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToColumn const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildStructPropertyExpression(NativeSqlBuilder& viewSql, PropertyMapToInLineStruct const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildSystemSelectionClause (NativeSqlBuilder::List& fragments, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildSelectionClause (NativeSqlBuilder& viewSql, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildClassView (SqlClassPersistenceMethod& scpm);

         BentleyStatus DropViewIfExists (ECDbR map, Utf8CP viewName);
    public:
        SqlGenerator (ECDbMapR map):m_map (map) {}
        static Utf8String BuildViewClassName (ECN::ECClassCR ecClass);
         BentleyStatus BuildViewInfrastructure (std::set<ClassMap const*>& classMaps);
         static Utf8String BuildSchemaQualifiedClassName (ECN::ECClassCR ecClass);

    };

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       07/2013
+===============+===============+===============+===============+===============+======*/
struct ViewGenerator
    { 
public:
    //! Name of the ECClassId column in the generated view
    static Utf8CP const ECCLASSID_COLUMNNAME;

private:
    struct ViewMember
        {
    private:
        std::vector<IClassMap const*> m_classMaps;
        DbMetaDataHelper::ObjectType m_storageType;
    public:
        std::vector<IClassMap const*>& GetClassMaps () { return m_classMaps; }
        DbMetaDataHelper::ObjectType GetStorageType() const { return m_storageType;}

        ViewMember()
            :m_storageType(DbMetaDataHelper::ObjectType::Table)
            {
            }
        ViewMember (DbMetaDataHelper::ObjectType storageType, IClassMap const& classMap)
            :m_storageType(storageType)
            {
            m_classMaps.push_back(&classMap);
            }
        };

    typedef bmap<ECDbSqlTable const*, ViewMember> ViewMemberByTable; 
    static BentleyStatus ComputeViewMembers (ViewMemberByTable& viewMembers, ECDbMapCR map, ECN::ECClassCR ecClass, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables, bool ensureDerivedClassesAreLoaded);
    static BentleyStatus GetRootClasses (std::vector<IClassMap const*>& rootClasses, ECDbR db);
    static BentleyStatus GetViewQueryForChild (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, const std::vector<IClassMap const*>& childClassMap, IClassMap const& baseClassMap, bool isPolymorphic);
    //! Relationship polymorphic query
    static BentleyStatus CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables);
    static BentleyStatus CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, RelationshipClassEndTableMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, IClassMap const& classMap);
    static BentleyStatus GetAllChildRelationships (std::vector<RelationshipClassMapCP>& relationshipMaps, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& baseRelationMap);
    static Utf8CP GetECClassIdPrimaryTableAlias (ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }

    static BentleyStatus BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint);
    //! Append view prop map list separated by comma.
    static BentleyStatus AppendViewPropMapsToQuery (NativeSqlBuilder& viewQuery, ECDbR ecdb, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView = false);

    static BentleyStatus AppendSystemPropMaps (NativeSqlBuilder& viewQuery, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap);
    static BentleyStatus AppendSystemPropMapsToNullView (NativeSqlBuilder& viewQuery, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, bool endWithComma);
    static BentleyStatus AppendConstraintClassIdPropMap (NativeSqlBuilder& viewQuery, ECSqlPrepareContext const& prepareContext, PropertyMapRelationshipConstraint const& propMap, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, ECN::ECRelationshipConstraintCR constraint);

    //! Return prop maps of child base on parent map. So only prop maps that make up baseClass properties are selected.
    static BentleyStatus GetPropertyMapsOfDerivedClassCastAsBaseClass (std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, ECSqlPrepareContext const& prepareContext, IClassMap const& baseClassMap, IClassMap const& childClassMap, bool skipSystemProperties, bool embededStatement);

    static void LoadDerivedClassMaps (std::map<ECN::ECClassId, IClassMap const *>& viewClasses, ECDbMapCR map, IClassMap const* classMap);
    static void CreateSystemClassView (NativeSqlBuilder &viewSql, std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> &tableMap, std::set<ECDbSqlTable const*> &tableToIncludeEntirly, bool forStructArray, ECSqlPrepareContext const& prepareContext);

public:

    //! Create a SQLite polymorphic SELECT query for a given classMap
    //! @param viewSql [out] Output SQL for view
    //! @param map [in] ECDbMap instance
    //! @param classMap [in] Source classMap for which to generate view
    //! @param isPolymorphicQuery [in] if true return a polymorphic view of ECClass else return a non-polymorphic view. Intend to be use by ECSQL "ONLY <ecClass>"
    //! @param structArrayProperty [in] specify structArrayProperty for which struct classmap view is needed
    //! @param optimizeByIncludingOnlyRealTables [in] Enabling would produce small length views but it does take a little long to generate
    //! @return The number of relevant relationships found
    //! @remarks Only work work normal ECClasses but not relationship. It also support query over ecdb.Instances
    static BentleyStatus CreateView (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext, bool optimizeByIncludingOnlyRealTables);
    enum class SystemViewType
        {
        Class,
        RelationshipClass,
        StructArray
        };

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
