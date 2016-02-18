/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlPrepareContext;

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       07/2013
+===============+===============+===============+===============+===============+======*/
struct ViewGenerator
    { 
private:
    struct ViewMember
        {
    private:
        std::vector<IClassMap const*> m_classMaps;
        DbMetaDataHelper::ObjectType m_storageType;
    public:
        ViewMember() :m_storageType(DbMetaDataHelper::ObjectType::Table) {}
        ViewMember (DbMetaDataHelper::ObjectType storageType, IClassMap const& classMap) :m_storageType(storageType) { m_classMaps.push_back(&classMap); }

        void AddClassMap(IClassMap const& classMap) { m_classMaps.push_back(&classMap); }
        std::vector<IClassMap const*> const& GetClassMaps() const { return m_classMaps; }
        DbMetaDataHelper::ObjectType GetStorageType() const { return m_storageType; }
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

    static BentleyStatus BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, ECDbSqlTable const& contextTable);
    //! Append view prop map list separated by comma.
    static BentleyStatus AppendViewPropMapsToQuery (NativeSqlBuilder& viewQuery, ECDbR ecdb, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView = false);

    static BentleyStatus AppendSystemPropMaps (NativeSqlBuilder& viewQuery, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, ECDbSqlTable const& contextTable);
    static BentleyStatus AppendSystemPropMapsToNullView (NativeSqlBuilder& viewQuery, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, bool endWithComma);
    static BentleyStatus AppendConstraintClassIdPropMap (NativeSqlBuilder& viewQuery, ECSqlPrepareContext const& prepareContext, PropertyMapRelationshipConstraint const& propMap, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, ECN::ECRelationshipConstraintCR constraint, ECDbSqlTable const& contextTable);

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

#if 0
//! WIP effort to unify all view generation into one
/*

*/
struct ECDbViewFactory
    {
    struct Param
        {
        enum class Type
            {
            Debug,
            System
            };

        private:
            Type m_type;
            bool m_supportDelete;
            bool m_supportInsert;
            bool m_supportUpdate;
            bool m_polymorphic;
            bool m_neverReturnNullView;
            std::set<Utf8String> m_propertyPathFilter;
        public:
            //! Create system view
            Param(bool polymorphic, bool neverReturnNullView, bool supportDelete, bool supportInsert, bool supportUpdate)
                :m_type(Type::System), m_neverReturnNullView(neverReturnNullView), m_supportDelete(supportDelete), m_supportInsert(supportInsert), m_supportUpdate(supportUpdate)
                {}
            //! Create debug view
            Param()
                :m_type(Type::Debug), m_neverReturnNullView(false), m_supportDelete(false), m_supportInsert(false), m_supportUpdate(false)
                {}
            //! Support delete trigger on view
            bool SystemViewSupportDelete() const {
                return m_supportDelete;
                }
            //! Support delete insert on view
            bool SystemViewSupportInsert() const {
                return m_supportInsert;
                }
            //! Support update trigger on view
            bool SystemViewSupportUpdate() const {
                return m_supportUpdate;
                }
            //! Generate Polymorphic view include derived class
            bool Polymorphic() const {
                return m_polymorphic;
                }
            //! If class resolved into no persistent table then do not return null view.
            bool NeverReturnNullView() const {
                return m_neverReturnNullView;
                }
            //! Only include set of property path. System properties are always included. If no filter is specified all properties is returned
            void IncludePropertyPath(Utf8CP propertyPath) {
                m_propertyPathFilter.insert(propertyPath);
                }
            //! Return list of filter property path that would be included. Empty mean include all.
            const std::set<Utf8String>& GetPropertyPaths() const {
                return m_propertyPathFilter;
                }
            //! Type determins if view will be for debug with PropertyPath or column name in case of system view.
            Type Type() const {
                return m_type;
                }
        };
    private:
        Param m_param;
        ECDbCR m_ecdb;

    private:
        BentleyStatus RenderClass(SqlViewBuilder& viewBuilder, IClassMap const& classMap);
        BentleyStatus RenderEntityClass(SqlViewBuilder& viewBuilder, IClassMap const& classMap)
            {
            StorageDescription const& storageDescr = classMap.GetStorageDescription();
            for (auto const& horizontalDescr : storageDescr.GetHorizontalPartitions())
                {
                if (horizontalDescr.GetTable().GetPersistenceType() == PersistenceType::Virtual)
                    {
                    // CREATE TRIGGER _reject_abstract_class INSTEAD OF INSERT ON b WHEN ECClassId IN () BEGIN SELECT RAISE (FAIL, 'Cannot INSERT instance of abstract class');

                    continue;
                    }

                    // CREATE TRIGGER _insert_table1 INSTEAD OF INSERT ON _<class_name> WHEN ECClassId IN (...) BEGIN INSERT INTO table1 (...) VALUES(...);... END
                    // CREATE TRIGGER _update_table1 INSTEAD OF UPDATE ON _<class_name> WHEN ECClassId IN (...) BEGIN UPDATE table1 SET ... WHERE ;... END
                    // CREATE TRIGGER _delete_table1 INSTEAD OF DELETE ON _<class_name> WHEN ECClassId IN (...) BEGIN DELETE FROM  table1 WHERE ;... END
                    
                //dgn_Element.Id 
                }
            }
        BentleyStatus RenderRelationshipClass(SqlViewBuilder& viewBuilder);
        BentleyStatus RenderRelationshipClassEndTable(SqlViewBuilder& viewBuilder);
        BentleyStatus RenderRelationshipClassLinkTable(SqlViewBuilder& viewBuilder);

    public:
        ECDbViewFactory(ECDbCR ecdb, Param param = Param())
            :m_ecdb(ecdb), m_param(param)
            {
            }


        BentleyStatus CreateView(SqlViewBuilder& viewBuilder);
    };
#endif
END_BENTLEY_SQLITE_EC_NAMESPACE
