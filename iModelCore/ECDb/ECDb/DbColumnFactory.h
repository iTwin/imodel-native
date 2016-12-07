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
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct DbColumnFactory : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        ClassMapCR m_classMap;
        bool m_usesSharedColumnStrategy;
        mutable bset<DbColumnId> m_idsOfColumnsInUseByClassMap; //should include relationships

        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8CP requestedColumnName, ECN::ECClassId, int retryCount) const;

        DbColumn* ApplyDefaultStrategy(Utf8CP requestedColumnName, ECN::ECPropertyCR, Utf8CP accessString, DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation) const;
        DbColumn* ApplySharedColumnStrategy(DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation) const;

        ECN::ECClassId GetPersistenceClassId(ECN::ECPropertyCR, Utf8CP accessString) const;
        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const&) const;
        void CacheUsedColumn(DbColumn const&) const;

        bool CanEnforceColumnConstraints() const;
        BentleyStatus GetDerivedColumnList(std::vector<DbColumn const*>&) const;

        DbTable& GetTable() const;

    public:
        DbColumnFactory(ECDbCR ecdb, ClassMapCR classMap);
        ~DbColumnFactory() {}

        DbColumn* CreateColumn(ECN::ECPropertyCR, Utf8CP accessString, Utf8CP requestedColumnName, DbColumn::Type, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation) const;
        void Update(bool includeDerivedClasses) const;

        bool UsesSharedColumnStrategy() const { return m_usesSharedColumnStrategy; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
