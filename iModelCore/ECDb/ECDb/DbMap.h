/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SchemaImportContext.h"
#include "DbSchema.h"
#include "IssueReporter.h"
#include "LightweightCache.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct DbMap final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        DbSchema m_dbSchema;
        mutable bmap<ECN::ECClassId, ClassMapPtr> m_classMapDictionary;
        mutable LightweightCache m_lightweightCache;

        BentleyStatus TryGetClassMap(ClassMap const*&, ClassMapLoadContext&, ECN::ECClassCR) const;
        BentleyStatus TryGetClassMap(ClassMapPtr&, ClassMapLoadContext&, ECN::ECClassCR) const;
        ClassMapPtr DoGetClassMap(ECN::ECClassCR) const;
        BentleyStatus TryLoadClassMap(ClassMapPtr&, ClassMapLoadContext& ctx, ECN::ECClassCR) const;
        BentleyStatus DoMapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;
        ClassMappingStatus MapClass(SchemaImportContext&, ClassMappingInfo const&) const;
        ClassMappingStatus MapDerivedClasses(SchemaImportContext&, ECN::ECClassCR baseClass) const;
        BentleyStatus SaveDbSchema(SchemaImportContext&) const;
        BentleyStatus CreateOrUpdateRequiredTables() const;
        BentleyStatus CreateOrUpdateIndexesInDb(SchemaImportContext&) const;
        BentleyStatus PurgeOrphanTables() const;
        ClassMappingStatus AddClassMap(ClassMapPtr&) const;
        BentleyStatus GetRelationshipConstraintClassMaps(SchemaImportContext&, std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;
        static void GatherRootClasses(ECN::ECClassCR ecclass, std::set<ECN::ECClassCP>& doneList, std::set<ECN::ECClassCP>& rootClassSet, std::vector<ECN::ECClassCP>& rootClassList, std::vector<ECN::ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixins);

    public:
        explicit DbMap(ECDbCR ecdb) : m_ecdb(ecdb), m_dbSchema(ecdb), m_lightweightCache(ecdb) {}
        ~DbMap() {}

        void ClearCache() const;
        ClassMap const* GetClassMap(ECN::ECClassCR) const;

        BentleyStatus MapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;
        ClassMappingStatus MapClass(SchemaImportContext&, ECN::ECClassCR) const;

        bmap<ECN::ECClassId, ClassMapPtr> const& GetClassMapCache() const { return m_classMapDictionary; }

        //!Loads the class maps if they were not loaded yet
        size_t GetRelationshipConstraintTableCount(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;
        std::set<DbTable const*> GetRelationshipConstraintPrimaryTables(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;
        std::set<ClassMap const*> GetRelationshipConstraintClassMaps(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;

        DbSchema const& GetDbSchema() const { return m_dbSchema; }
        DbSchema& GetDbSchemaR() const { return const_cast<DbSchema&> (m_dbSchema); }
        LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const { return m_ecdb.GetImpl().Issues(); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle  04/2017
//+===============+===============+===============+===============+===============+======
struct ClassMapFactory final : NonCopyableClass
    {
private:
    ClassMapFactory();
    ~ClassMapFactory();

public:
    template<typename TClassMap>
    static ClassMapPtr Create(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy) { return new TClassMap(ecdb, ecClass, mapStrategy); }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
