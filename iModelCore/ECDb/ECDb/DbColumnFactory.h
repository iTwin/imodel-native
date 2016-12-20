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
#include "DbSchema.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct DbColumnFactory  final: NonCopyableClass
    {
    private:
        ClassMap const& m_classMap;
        mutable std::map<Utf8String, std::set<DbColumn const*>, CompareIUtf8Ascii> m_usedColumnMap;
        mutable std::set<DbColumn const*> m_usedColumnSet;
        bool m_usesSharedColumnStrategy;

        void Initialize();

        ECN::ECClassId GetPersistenceClassId(ECN::ECPropertyCR, Utf8StringCR accessString) const;
        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId, int retryCount) const;

        DbColumn* CreateColumn(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR accessString) const;
        DbColumn* ApplyDefaultStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR accessString) const;
        DbColumn* ApplySharedColumnStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&) const;

        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const& column) const { return m_usedColumnSet.find(&column) != m_usedColumnSet.end(); }
        bool IsCompatible(DbColumn const& avaliableColumn, DbColumn::Type type, DbColumn::CreateParams const& param) const;

        void AddColumnToCache(DbColumn const&, Utf8StringCR) const;

        ClassMap const& GetClassMap() const { return m_classMap; }
        DbTable& GetTable() const;
        ECDbCR GetECDb() const;

        static std::set<ECN::ECClassCP> GetRootClasses(ECN::ECClassCR);
        static std::set<ClassMap const*> GetDeepestClassMapsInTph(ClassMap const&);

    public:
        explicit DbColumnFactory(ClassMap const& classMap);

        //This function either create a column or grab a existing column
        DbColumn* AllocateDataColumn(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString) const;

        void Debug() const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
