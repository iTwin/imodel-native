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
    typedef std::unique_ptr<DbColumnFactory> Ptr;
    typedef std::map<Utf8String, std::set<DbColumn const*>, CompareIUtf8Ascii> UsedColumnMap;
    private:
        ClassMap const& m_classMap;
        mutable UsedColumnMap m_usedColumnMap;
        mutable std::set<DbColumn const*> m_usedColumnSet;
        bool m_usesSharedColumnStrategy;
    private:
        static std::set<ECN::ECClassCP> GetRootClasses(ECN::ECClassCR ecClass);
        //!The funtion return deepest mapped classes from memory or disk and does not include class for which its called.
        static std::set<ClassMap const*> GetDeepestClassMapsInTph(ClassMap const& classMap);
        static Utf8String QualifiedAccessString(PropertyMap const& propertyMap);
        static UsedColumnMap BuildUsedColumnMap(ClassMap const& contextClassMap);
        static BentleyStatus ComputeReleventClassMaps(bmap<ECN::ECClassCP, ClassMap const*>& contextGraph, ClassMap const& classMap);
        void CacheUsedColumn(DbColumn const&, Utf8StringCR) const;
        ECN::ECClassId GetPersistenceClassId(ECN::ECPropertyCR, Utf8StringCR accessString) const;
        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId, int retryCount) const;

        DbColumn* CreateColumn(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR accessString) const;
        DbColumn* ApplyDefaultStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR accessString) const;
        DbColumn* ApplySharedColumnStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&) const;

        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const& column) const;
        bool IsCompitable(DbColumn const& avaliableColumn, DbColumn::Type type, DbColumn::CreateParams const& param) const;

        ECDbCR GetECDb() const;
        DbColumnFactory(ClassMap const& classMap) :m_classMap(classMap) {}
    public:
        ClassMap const& GetClassMap() const { return m_classMap; }
        bool UsesSharedColumnStrategy() const { return m_usesSharedColumnStrategy; }
        DbTable& GetTable() const;
        void Debug() const;
        //This function either create a column or grab a existing column
        DbColumn* AllocateDataColumn(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString) const;
        static Ptr Create(ClassMap const& classMap);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
