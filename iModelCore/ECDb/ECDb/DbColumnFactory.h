/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbColumnFactory.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>
#include "ClassMap.h"
#include "DbSchema.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct IUsedSharedColumnQuery : NonCopyableClass
    {
    typedef std::unique_ptr<IUsedSharedColumnQuery> Ptr;
    private:
        virtual BentleyStatus _Query(bset<DbColumn const*>& columns, DbTable const& table) const = 0;
    public:
        BentleyStatus Query(bset<DbColumn const*>& columns, DbTable const& table) const { return _Query(columns, table); }
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct DerivedClassUsedSharedColumnQuery : IUsedSharedColumnQuery
    {
    private:
        ECDbCR m_ecdb;
        ECN::ECClassId m_classId;
        BentleyStatus _Query(bset<DbColumn const*>& columns, DbTable const& table) const override;
        DerivedClassUsedSharedColumnQuery(ECDbCR ecdb, ECN::ECClassId ecClassId)
            :m_ecdb(m_ecdb), m_classId(ecClassId)
            {}
    public:
        static IUsedSharedColumnQuery::Ptr Create(ECDbCR ecdb, ECN::ECClassId ecClassId)
            {
            return IUsedSharedColumnQuery::Ptr(new DerivedClassUsedSharedColumnQuery(ecdb, ecClassId));
            }
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ClassMapUsedSharedColumnQuery : IUsedSharedColumnQuery
    {
    private:
        ClassMapCR m_classMap;
        BentleyStatus _Query(bset<DbColumn const*>& columns, DbTable const& table) const override;
        ClassMapUsedSharedColumnQuery(ClassMapCR classMap)
            : m_classMap(classMap)
            {}
    public:
        static IUsedSharedColumnQuery::Ptr Create(ClassMapCR classMap)
            {
            return IUsedSharedColumnQuery::Ptr(new ClassMapUsedSharedColumnQuery(classMap));
            }
    };
//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ForeignEndRelationshipUsedSharedColumnQuery : IUsedSharedColumnQuery
    {
    private:
        ClassMapCR m_classMap;
        BentleyStatus _Query(bset<DbColumn const*>& columns, DbTable const& table) const override
            {
            //Need all the relationship that is store in same table and oent hat mapped

            return SUCCESS;
            }
        ForeignEndRelationshipUsedSharedColumnQuery(ClassMapCR classMap)
            : m_classMap(classMap)
            {}
    public:
        static IUsedSharedColumnQuery::Ptr Create(ClassMapCR classMap)
            {
            return IUsedSharedColumnQuery::Ptr(new ForeignEndRelationshipUsedSharedColumnQuery(classMap));
            }
    };
	
//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct DbColumnFactory : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        ClassMapCR m_classMap;
        bool m_usesSharedColumnStrategy;
        mutable bset<DbColumnId> m_idsOfColumnsInUseByClassMap;

        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId, int retryCount) const;

        DbColumn* ApplyDefaultStrategy(ECN::ECPropertyCR, Utf8StringCR accessString, DbColumn::Type, DbColumn::CreateParams const&) const;
        DbColumn* ApplySharedColumnStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&) const;

        ECN::ECClassId GetPersistenceClassId(ECN::ECPropertyCR, Utf8StringCR accessString) const;
        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const&) const;
        void CacheUsedColumn(DbColumn const&) const;

        BentleyStatus GetDerivedColumnList(std::vector<DbColumn const*>&) const;

        DbTable& GetTable() const;

    public:
        DbColumnFactory(ECDbCR ecdb, ClassMapCR classMap);
        ~DbColumnFactory() {}

        DbColumn* CreateColumn(ECN::ECPropertyCR, Utf8StringCR accessString, DbColumn::Type, DbColumn::CreateParams const&) const;
        void Update(bool includeDerivedClasses) const;

        bool UsesSharedColumnStrategy() const { return m_usesSharedColumnStrategy; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
