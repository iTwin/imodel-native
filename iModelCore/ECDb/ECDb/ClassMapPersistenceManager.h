/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapPersistenceManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbClassMapLoadContext final : public NonCopyableClass
    {
    private:
        bool m_classMapExists = false;
        MapStrategyExtendedInfo m_mapStrategyExtInfo;
        std::map<Utf8String, std::vector<DbColumn const*>> m_columnByAccessString;

        static BentleyStatus ReadPropertyMaps(DbClassMapLoadContext&, ECDbCR, ECN::ECClassId);


        static Nullable<MapStrategy> ToMapStrategy(int val)
            {
            if (val == Enum::ToInt(MapStrategy::NotMapped) || val == Enum::ToInt(MapStrategy::OwnTable) || val == Enum::ToInt(MapStrategy::TablePerHierarchy) ||
                val == Enum::ToInt(MapStrategy::ExistingTable) || val == Enum::ToInt(MapStrategy::ForeignKeyRelationshipInTargetTable) || val == Enum::ToInt(MapStrategy::ForeignKeyRelationshipInSourceTable))
                return Enum::FromInt<MapStrategy>(val);

            return Nullable<MapStrategy>();
            };

        static Nullable<TablePerHierarchyInfo::ShareColumnsMode> ToShareColumnsMode(int val)
            {
            if (val == Enum::ToInt(TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly) || val == Enum::ToInt(TablePerHierarchyInfo::ShareColumnsMode::No) || val == Enum::ToInt(TablePerHierarchyInfo::ShareColumnsMode::Yes))
                return Enum::FromInt<TablePerHierarchyInfo::ShareColumnsMode>(val);

            return Nullable<TablePerHierarchyInfo::ShareColumnsMode>();
            };

        static Nullable<JoinedTableInfo> ToJoinedTableInfo(int val)
            {
            if (val == Enum::ToInt(JoinedTableInfo::None) || val == Enum::ToInt(JoinedTableInfo::JoinedTable) || val == Enum::ToInt(JoinedTableInfo::ParentOfJoinedTable))
                return Enum::FromInt<JoinedTableInfo>(val);

            return Nullable<JoinedTableInfo>();
            };

    public:
        DbClassMapLoadContext() {}
        ~DbClassMapLoadContext() {}

        static BentleyStatus Load(DbClassMapLoadContext&, ClassMapLoadContext&, ECDbCR, ECN::ECClassCR);

        bool ClassMapExists() const { return m_classMapExists; }
        MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
        bool HasMappedProperties() const { return !m_columnByAccessString.empty(); }
        std::map<Utf8String, std::vector<DbColumn const*>> const& GetPropertyMaps() const { return m_columnByAccessString; }
        std::vector<DbColumn const*> const* FindColumnByAccessString(Utf8StringCR accessString) const;
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbMapSaveContext final: public NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        std::map<ECN::ECClassId, ClassMapCP> m_savedClassMaps;
        std::stack<ClassMapCP> m_editStack;

    public:
        explicit DbMapSaveContext(ECDbCR ecdb) :m_ecdb(ecdb) {}
        ~DbMapSaveContext() {}

        bool IsAlreadySaved(ClassMapCR) const;
        void BeginSaving(ClassMapCR);
        void EndSaving(ClassMapCR);
        ClassMapCP GetCurrent() const { return m_editStack.top(); }
        BentleyStatus InsertClassMap(ECN::ECClassId, MapStrategyExtendedInfo const&);
        BentleyStatus TryGetPropertyPathId(PropertyPathId&, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, bool addIfDoesNotExist);
        ECDbCR GetECDb() const { return m_ecdb; }
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbClassMapSaveContext final : public NonCopyableClass
    {
    private:
        DbMapSaveContext& m_classMapContext;
        ClassMapCR  m_classMap;

    public:
        explicit DbClassMapSaveContext(DbMapSaveContext& ctx);
        ~DbClassMapSaveContext() {}
        DbMapSaveContext const& GetMapSaveContext() const { return m_classMapContext; }
        ClassMapCR GetClassMap() const { return m_classMap; }
        BentleyStatus InsertPropertyMap(ECN::ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
