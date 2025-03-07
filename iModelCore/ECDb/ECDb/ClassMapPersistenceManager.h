/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassMap.h"

struct TableSpaceSchemaManager;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass
//======================================================================================
struct DbClassMapLoadContext final
    {
    private:
        bool m_classMapExists = false;
        MapStrategyExtendedInfo m_mapStrategyExtInfo;
        std::map<Utf8String, std::vector<DbColumn const*>> m_columnByAccessString;
        std::set<Utf8String> m_propertiesToIgnore;

        //not copyable
        DbClassMapLoadContext(DbClassMapLoadContext const&) = delete;
        DbClassMapLoadContext& operator=(DbClassMapLoadContext const&) = delete;

        static BentleyStatus ReadPropertyMaps(DbClassMapLoadContext&, ECDbCR, TableSpaceSchemaManager const&, ECN::ECClassId);

    public:
        DbClassMapLoadContext() {}
        ~DbClassMapLoadContext() {}

        static BentleyStatus Load(DbClassMapLoadContext&, ClassMapLoadContext&, ECDbCR, TableSpaceSchemaManager const&, ECN::ECClassCR);

        bool ClassMapExists() const { return m_classMapExists; }
        MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
        bool HasMappedProperties() const { return !m_columnByAccessString.empty(); }
        std::map<Utf8String, std::vector<DbColumn const*>> const& GetPropertyMaps() const { return m_columnByAccessString; }
        std::vector<DbColumn const*> const* FindColumnByAccessString(Utf8StringCR accessString) const;
        void AddPropertyToIgnoreList(Utf8StringCR propertyName) { m_propertiesToIgnore.insert(propertyName); }
        bool IsPropertyIgnored(Utf8StringCR propertyName) const { return m_propertiesToIgnore.find(propertyName) != m_propertiesToIgnore.end(); }
    };


//======================================================================================
// @bsiclass
//======================================================================================
struct DbMapSaveContext final
    {
    private:
        ECDbCR m_ecdb;
        std::map<ECN::ECClassId, ClassMap const*> m_savedClassMaps;
        std::stack<ClassMap const*> m_editStack;

        //not copyable
        DbMapSaveContext(DbMapSaveContext const&) = delete;
        DbMapSaveContext& operator=(DbMapSaveContext const&) = delete;

    public:
        explicit DbMapSaveContext(ECDbCR ecdb) :m_ecdb(ecdb) {}
        ~DbMapSaveContext() {}

        bool IsAlreadySaved(ClassMapCR) const;
        void BeginSaving(ClassMapCR);
        void EndSaving(ClassMapCR);
        ClassMap const* GetCurrent() const { return m_editStack.top(); }
        BentleyStatus InsertClassMap(ECN::ECClassId, MapStrategyExtendedInfo const&);
        BentleyStatus TryGetPropertyPathId(PropertyPathId&, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, bool addIfDoesNotExist);
        ECDbCR GetECDb() const { return m_ecdb; }
    };


//======================================================================================
// @bsiclass
//======================================================================================
struct DbClassMapSaveContext final
    {
    private:
        DbMapSaveContext& m_classMapContext;
        ClassMapCR  m_classMap;

        //not copyable
        DbClassMapSaveContext(DbClassMapSaveContext const&) = delete;
        DbClassMapSaveContext& operator=(DbClassMapSaveContext const&) = delete;

    public:
        explicit DbClassMapSaveContext(DbMapSaveContext& ctx);
        ~DbClassMapSaveContext() {}
        DbMapSaveContext const& GetMapSaveContext() const { return m_classMapContext; }
        ClassMapCR GetClassMap() const { return m_classMap; }
        BentleyStatus InsertPropertyMap(ECN::ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId);
        BentleyStatus TryPropertyMapExists(uint64_t& propertyMapId, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId);
        BentleyStatus UpdatePropertyMapColumn(uint64_t propertyMapId, DbColumnId columnId);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
