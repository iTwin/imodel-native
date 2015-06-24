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


struct ECSqlPrepareContext;

//=======================================================================================
// @bsiclass                                               Affan.Khan           06/2015
//+===============+===============+===============+===============+===============+======
struct SqlGenerator
    {
    enum class RelationshipFilter
        {
        Source = 1,
        Target = 2,
        Both = Source | Target
        };
    private:
        static BentleyStatus BuildHoldingConstraint (NativeSqlBuilder& stmt, ECDbMapCR map, RelationshipClassMapCR const& classMap);
        static BentleyStatus BuildEmbeddingConstraint (NativeSqlBuilder& stmt, ECDbMapCR map, RelationshipClassMapCR const& classMap);

        static BentleyStatus BuildHoldingView (NativeSqlBuilder& sql, ECDbMapCR map);
        static BentleyStatus BuildDeleteTriggersForRelationships (NativeSqlBuilder::List& triggers, ECDbMapCR map, ClassMapCR const& classMap);
        static BentleyStatus FindRelationshipReferences (std::map<RelationshipClassMapCP, RelationshipFilter>& relationships, ECDbMapCR map, ClassMapCR classMap);
        static void CollectDerivedEndTableRelationships (std::set<RelationshipClassEndTableMapCP>& childMaps, RelationshipClassMapCR const& classMap);
        static Utf8String BuildSchemaQualifiedClassName (ECN::ECClassCR ecClass);
        static Utf8String BuildViewClassName (ECN::ECClassCR ecClass);
        static BentleyStatus BuildDerivedFilterClause (Utf8StringR filter, ECDb& db, ECN::ECClassId baseClassId);
        static Utf8CP GetECClassIdPrimaryTableAlias (ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }
        static BentleyStatus BuildECInstanceIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildECClassIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, bool topLevel);
        static BentleyStatus BuildEndTableRelationshipView (NativeSqlBuilder::List& viewSql, ECDbMapCR map, RelationshipClassMapCR const& classMap);

        static BentleyStatus BuildDeleteTriggersForDerivedClasses (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap);
        static BentleyStatus BuildDeleteTriggerForMe (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap);
        static BentleyStatus BuildDeleteTriggerForEndTableMe (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap);

        static BentleyStatus BuildPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildColumnExpression (NativeSqlBuilder::List& viewSql, Utf8CP tablePrefix, Utf8CP columnName, Utf8CP accessString, bool addECPropertyPathAlias, bool nullValue, bool escapeColumName = true);
        static BentleyStatus BuildPointPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapPoint const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildPrimitivePropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToColumnCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildStructPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToInLineStructCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildSystemSelectionClause (NativeSqlBuilder::List& fragments, ClassMapCR const& classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildSelectionClause (NativeSqlBuilder& viewSql, ECDbMapCR map, ClassMapCR const& baseClassMap, ClassMapCR const& classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
        static BentleyStatus BuildClassView (NativeSqlBuilder& viewSql, ECDbMapCR map, ClassMapCR const& classMap);
        static BentleyStatus BuildView (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap);
        static BentleyStatus BuildDeleteTriggerForStructArrays (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap);
        static BentleyStatus CreateView (ECDbMapR map, IClassMap const& classMap, bool dropViewIfExist);
        static BentleyStatus BuildDeleteTriggers (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap);
        static BentleyStatus DropViewIfExists (ECDbR map, Utf8CP viewName);
    public:
        static BentleyStatus BuildViewInfrastructure (std::set<ClassMap const*>& classMaps, ECDbMapR map);
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
    static BentleyStatus CreateSystemView (NativeSqlBuilder& viewSql, SystemViewType systemView, ECDbMapCR map, std::vector<ECN::ECClassId> const& classesToInclude, bool polymorphic, ECSqlPrepareContext const& prepareContext);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
