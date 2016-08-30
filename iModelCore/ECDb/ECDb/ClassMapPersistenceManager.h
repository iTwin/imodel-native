/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapPersistenceManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//======================================================================================
// @bsiclass                                                 Krischan.Eberle     08/2016
//======================================================================================
struct ClassMapPersistenceManager
    {
public:
    struct SaveContext : public NonCopyableClass
        {
        private:
            ECDbCR m_ecdb;
            std::map<ECN::ECClassId, ClassMapCP> m_savedClassMaps;
            std::stack<ClassMapCP> m_editStack;
        
        public:
            explicit SaveContext(ECDbCR ecdb) :m_ecdb(ecdb) {}
            ~SaveContext() {}

            bool IsAlreadySaved(ClassMapCR) const;
            void BeginSaving(ClassMapCR);
            void EndSaving(ClassMapCR);
            ClassMapCP GetCurrent() const { return m_editStack.top(); }

            ECDbCR GetECDb() const { return m_ecdb; }
            ECDbMap const& GetECDbMap() const { return m_ecdb.GetECDbImplR().GetECDbMap(); }
        };

    BentleyStatus InsertClassMap(ClassMapId& classMapId, ECN::ECClassId classId, MapStrategyExtendedInfo const&, ClassMapId baseClassMapId);
    BentleyStatus TryGetPropertyPathId(PropertyPathId& id, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, bool addIfDoesNotExist);

private:
    ClassMapPersistenceManager();
    ~ClassMapPersistenceManager();

public:
    static BentleyStatus Save(SaveContext&, ClassMapCR);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbClassMapLoadContext : public NonCopyableClass
    {
    private:
        bool m_isValid;
        MapStrategyExtendedInfo m_mapStrategyExtInfo;
        ClassMapId m_classMapId;
        ECN::ECClassId m_baseClassId;
        std::map<Utf8String, std::vector<DbColumn const*>> m_columnByAccessString;
        ClassMapCP m_baseClassMap;

        static BentleyStatus ReadPropertyMaps(DbClassMapLoadContext& ctx, ECDbCR ecdb);

    public:
        DbClassMapLoadContext() :m_isValid(false), m_baseClassMap(nullptr) {}
        ~DbClassMapLoadContext() {}

        ClassMapId const& GetClassMapId() const { return m_classMapId; }

        ClassMapCP GetBaseClassMap() const { return m_baseClassMap; }
        BentleyStatus SetBaseClassMap(ClassMapCR classMap);
        ECN::ECClassId const& GetBaseClassId() const { return m_baseClassId; }
        MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }

        bool HasMappedProperties() const { return !m_columnByAccessString.empty(); }
        std::map<Utf8String, std::vector<DbColumn const*>> const& GetPropertyMaps() const { return m_columnByAccessString; }
        std::vector<DbColumn const*> const* FindColumnByAccessString(Utf8CP accessString) const;

        static BentleyStatus Load(DbClassMapLoadContext&, ECDbCR, ECN::ECClassId);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbMapSaveContext : public NonCopyableClass
    {
    private:
        std::map<ECN::ECClassId, ClassMapCP> m_savedClassMaps;
        std::stack<ClassMapCP> m_editStack;
        ECDbCR m_ecdb;
    public:
        DbMapSaveContext(ECDbCR ecdb) :m_ecdb(ecdb)
            {}
        ~DbMapSaveContext() {}

        ECDbCR GetECDb() const { return m_ecdb; }
        bool IsAlreadySaved(ClassMapCR classMap) const;
        void BeginSaving(ClassMapCR classMap);
        void EndSaving(ClassMapCR classMap);
        ClassMapCP GetCurrent() const { return m_editStack.top(); }
        BentleyStatus InsertClassMap(ClassMapId&, ECN::ECClassId, MapStrategyExtendedInfo const&, ClassMapId baseClassMapId);
        BentleyStatus TryGetPropertyPathId(PropertyPathId&, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, bool addIfDoesNotExist);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbClassMapSaveContext : public NonCopyableClass
    {
    private:
        DbMapSaveContext& m_classMapContext;
        ClassMapCR  m_classMap;

    public:
        DbClassMapSaveContext(DbMapSaveContext& ctx);
        ~DbClassMapSaveContext() {}
        DbMapSaveContext const& GetMapSaveContext() const { return m_classMapContext; }
        ClassMapCR GetClassMap() const { return m_classMap; }
        BentleyStatus InsertPropertyMap(ECN::ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
