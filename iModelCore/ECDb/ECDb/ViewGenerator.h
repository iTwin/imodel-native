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
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlPrepareContext;
struct ConstraintECClassIdJoinInfo;

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       07/2013
+===============+===============+===============+===============+===============+======*/
struct ViewGenerator
    {
    private:

        ECDb const& m_ecdb;
        bool m_optimizeByIncludingOnlyRealTables;
        ECSqlPrepareContext const* m_prepareContext;
        bool m_isPolymorphic;
        std::unique_ptr<std::vector<Utf8String>> m_viewAccessStringList;
        bool m_asSubQuery;
        bool m_captureViewAccessStringList;

        explicit ViewGenerator(ECDb const& ecdb, bool returnViewAccessStringList = false, bool asSubQuery = true) :
            m_ecdb(ecdb), m_optimizeByIncludingOnlyRealTables(true), m_prepareContext(nullptr), m_asSubQuery(asSubQuery), m_captureViewAccessStringList(true)
            {
            if (returnViewAccessStringList)
                m_viewAccessStringList = decltype(m_viewAccessStringList)(new std::vector<Utf8String>());
            }

        static BentleyStatus CreateECClassView(ECDbCR, ClassMapCR);
        static BentleyStatus CreateUpdatableViewIfRequired(ECDbCR, ClassMap const&);
        BentleyStatus GenerateViewSql(NativeSqlBuilder& viewSql, ClassMap const&, bool isPolymorphicQuery, ECSqlPrepareContext const*);

        

        //! Return prop maps of child base on parent map. So only prop maps that make up baseClass properties are selected.
        
        static BentleyStatus GenerateUpdateTriggerSetClause(NativeSqlBuilder& sql, ClassMap const& baseClassMap, ClassMap const& derivedClassMap);

        /*=====================NEW API=================*/
        bool IsECClassIdFilterEnabled() const;
        void RecordPropertyMapIfRequried(PropertyMap const& accessString);
        BentleyStatus RenderPropertyMaps(NativeSqlBuilder& sqlView, DbTable const*& requireJoinTo, ClassMapCR classMap, DbTable const& contextTable, ClassMapCP baseClass = nullptr, PropertyMap::Kind filter = PropertyMap::Kind::Entity);
        BentleyStatus RenderRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassEndTableMap const& relationMap);
        BentleyStatus RenderRelationshipClassMap(NativeSqlBuilder& viewSql, RelationshipClassMap const& relationMap, DbTable const& contextTable, ConstraintECClassIdJoinInfo const* sourceJoinInfo, ConstraintECClassIdJoinInfo const* targetJoinInfo, RelationshipClassLinkTableMap const* castInto = nullptr) ;
        BentleyStatus RenderNullView(NativeSqlBuilder& viewSql, ClassMap const& classMap);
        BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, ClassMap const& classMap);
        BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, ClassMap const& classMap, DbTable const& contextTable, ClassMapCP castAs = nullptr);
        BentleyStatus RenderRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassLinkTableMap const& relationMap);
        /*=====================NEW API=================*/
    public:
        //! Generates a SQLite polymorphic SELECT query for a given classMap
        //! @param viewSql [out] Output SQL for view
        //! @param classMap [in] Source classMap for which to generate view
        //! @param isPolymorphicQuery [in] if true return a polymorphic view of ECClass else return a non-polymorphic view. Intend to be use by ECSQL "ONLY <ecClass>"
        //! @param prepareContext [in] prepareContext from ECSQL
        //! @remarks Only work work normal ECClasses but not relationship. It also support query over ecdb.Instances
        static BentleyStatus GenerateSelectViewSql(NativeSqlBuilder& viewSql, ECDb const&, ClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext);

        static BentleyStatus CreateUpdatableViews(ECDbCR);
        static BentleyStatus DropUpdatableViews(ECDbCR);

        static BentleyStatus CreateECClassViews(ECDbCR);
        static BentleyStatus DropECClassViews(ECDbCR);
    };

struct ConstraintECClassIdJoinInfo : NonCopyableClass
    {
    typedef std::unique_ptr<ConstraintECClassIdJoinInfo> Ptr;
    private:
        DbColumn const& m_primaryECInstanceId;
        DbColumn const& m_primaryECClassId;
        DbColumn const& m_forignECInstanceId;
        ConstraintECClassIdPropertyMap const& m_propertyMap;
        Utf8CP GetSqlTableAlias()const;
        Utf8CP GetSqlECClassIdColumnAlias()const;
        ConstraintECClassIdJoinInfo(ConstraintECClassIdPropertyMap const& propertyMap, DbColumn const& primaryECInstanceId, DbColumn const& primaryECClassId, DbColumn const& forignECClassId)
            : m_primaryECInstanceId(primaryECInstanceId), m_primaryECClassId(primaryECClassId), m_forignECInstanceId(forignECClassId), m_propertyMap(propertyMap)
            {}
  

    public:
        ConstraintECClassIdPropertyMap const& GetConstraintECClassId() const { return m_propertyMap; }
        DbColumn const& GetPrimaryECInstanceIdColumn() const { return m_primaryECClassId; }
        DbColumn const& GetPrimaryECClassIdColumn() const { return m_primaryECClassId; }
        DbColumn const& GetForignECInstanceIdColumn() const { return m_forignECInstanceId; }
        NativeSqlBuilder GetNativeConstraintECClassIdSQL(bool appendAlias) const;
        ~ConstraintECClassIdJoinInfo() {}
        NativeSqlBuilder GetNativeJoinSQL() const;
        static DbTable const* RequiresJoinTo(ConstraintECClassIdPropertyMap const& propertyMap);
        static Ptr Create(ConstraintECClassIdPropertyMap const& propertyMap, DbTable const& contextTable);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
