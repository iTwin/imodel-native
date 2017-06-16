/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaManager.h>
#include "ClassMap.h"
#include "DbSchemaPersistenceManager.h"
#include "SchemaComparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct SchemaPolicy : NonCopyableClass
    {
    friend struct SchemaPolicies;

    enum class Type
        {
        NoAdditionalRootEntityClasses = 1,
        NoAdditionalLinkTables = 2,
        NoAdditionalForeignKeyConstraints = 4
        };

    typedef bset<Utf8String, CompareIUtf8Ascii> Exceptions;

    private:
        Type m_type;
        ECN::ECSchemaId m_optingInSchemaId;
        Exceptions m_exceptions;
        static std::unique_ptr<SchemaPolicy> Create(Type, ECN::ECSchemaId optingInSchemaId);

    protected:
        SchemaPolicy(Type type, ECN::ECSchemaId optingInSchemaId) : m_type(type), m_optingInSchemaId(optingInSchemaId) {}

        Exceptions const& GetExceptions() const { return m_exceptions; }

    public:
        virtual ~SchemaPolicy() {}

        template<typename TPolicy>
        TPolicy const& GetAs() const { BeAssert(dynamic_cast<TPolicy const*> (this) != nullptr);  return static_cast<TPolicy const&> (*this); }

        Type GetType() const { return m_type; }
        ECN::ECSchemaId GetOptingInSchemaId() const { return m_optingInSchemaId; }

        static Utf8CP TypeToString(Type);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct NoAdditionalRootEntityClassesPolicy final : SchemaPolicy
    {
    private:
        bool IsException(ECN::ECClassCR ecClass) const { return GetExceptions().find(ecClass.GetFullName()) != GetExceptions().end(); }

    public:
        explicit NoAdditionalRootEntityClassesPolicy(ECN::ECSchemaId optingInSchemaId) : SchemaPolicy(Type::NoAdditionalRootEntityClasses, optingInSchemaId) {}
        ~NoAdditionalRootEntityClassesPolicy() {}

        BentleyStatus Evaluate(ECDbCR, ECN::ECClassCR) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct NoAdditionalLinkTablesPolicy final : SchemaPolicy
    {
    private:
        bool IsException(ECN::ECRelationshipClassCR relClass) const { return GetExceptions().find(relClass.GetFullName()) != GetExceptions().end(); }

    public:
        explicit NoAdditionalLinkTablesPolicy(ECN::ECSchemaId optingInSchemaId) : SchemaPolicy(Type::NoAdditionalLinkTables, optingInSchemaId) {}
        ~NoAdditionalLinkTablesPolicy() {}

        BentleyStatus Evaluate(ECDbCR, ECN::ECRelationshipClassCR) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct NoAdditionalForeignKeyConstraintsPolicy final : SchemaPolicy
    {
    private:
        bool IsException(ECN::NavigationECPropertyCR navProp) const
            {
            Utf8String searchString(navProp.GetClass().GetFullName());
            searchString.append(".").append(navProp.GetName());

            return GetExceptions().find(searchString) != GetExceptions().end(); 
            }

    public:
        explicit NoAdditionalForeignKeyConstraintsPolicy(ECN::ECSchemaId optingInSchemaId) : SchemaPolicy(Type::NoAdditionalForeignKeyConstraints, optingInSchemaId) {}
        ~NoAdditionalForeignKeyConstraintsPolicy() {}

        BentleyStatus Evaluate(ECDbCR, ECN::NavigationECPropertyCR) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct SchemaPolicies final
    {
    private:
        std::map<SchemaPolicy::Type, std::unique_ptr<SchemaPolicy>> m_optedInPolicies;

        BentleyStatus ReadPolicy(ECDbCR, ECN::ECSchemaCR, SchemaPolicy::Type);

    public:
        SchemaPolicies() {}

        BentleyStatus ReadPolicies(ECDbCR);
        bool IsOptedIn(SchemaPolicy const*&, SchemaPolicy::Type) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext final
    {
    enum class Phase
        {
        ImportingSchemas,
        MappingSchemas,
        MappingMixins,
        MappingEntities,
        MappingRelationships,
        CreatingUserDefinedIndexes
        };
private:
    SchemaManager::SchemaImportOptions m_options;
    Phase m_phase = Phase::ImportingSchemas;

    ClassMapLoadContext m_loadContext;
    bset<ECN::ECClassId> m_classMapsToSave;
    mutable std::map<ECN::ECClassCP, std::unique_ptr<ClassMappingCACache>> m_classMappingCACache;
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> m_classMappingInfoCache;
    bmap<ECN::ECClassId, ForeignKeyConstraintCustomAttribute> m_fkConstraintCACache;
    ForeignKeyConstraintCustomAttribute m_emptyFkConstraintCA;

    SchemaPolicies m_schemaPolicies;

public:
    explicit SchemaImportContext(SchemaManager::SchemaImportOptions options) : m_options(options) {}

    void SetPhase(Phase phase) { BeAssert(Enum::ToInt(m_phase) < Enum::ToInt(phase)); m_phase = phase; }
    Phase GetPhase() const { return m_phase; }

    SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }

    ClassMappingCACache const* GetClassMappingCACache(ECN::ECClassCR) const;
    ClassMappingCACache* GetClassMappingCACacheP(ECN::ECClassCR) const;

    void CacheClassMapInfo(ClassMap const&, std::unique_ptr<ClassMappingInfo>&);
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> const& GetClassMappingInfoCache() const { return m_classMappingInfoCache; }
    //!@return true if @p navProp has the ForeignConstraintCA, false otherwise
    bool CacheFkConstraintCA(ECN::NavigationECPropertyCR navProp);

    ForeignKeyConstraintCustomAttribute const& GetFkConstraintCAFromCache(ECN::ECClassId relClassId) const
        {
        auto it = m_fkConstraintCACache.find(relClassId);
        if (it == m_fkConstraintCACache.end())
            return m_emptyFkConstraintCA;

        return it->second;
        }

    ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
    void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
    bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }

    SchemaPolicies const& GetSchemaPolicies() const { return m_schemaPolicies; }
    SchemaPolicies& GetSchemaPoliciesR() { return m_schemaPolicies; }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaCompareContext final
    {
    private:
        bvector<ECN::ECSchemaCP> m_existingSchemas;
        bvector<ECN::ECSchemaCP> m_schemasToImport;
        SchemaChanges m_changes;

    public:
        SchemaCompareContext() {}
        ~SchemaCompareContext() {}

        BentleyStatus Prepare(SchemaManager const&, bvector<ECN::ECSchemaCP> const& dependencyOrderedPrimarySchemas);
        bvector<ECN::ECSchemaCP>  const& GetSchemasToImport() const { return m_schemasToImport; }
        SchemaChanges& GetChanges() { return m_changes; }

        ECN::ECSchemaCP FindExistingSchema(Utf8CP schemaName) const;

        BentleyStatus ReloadContextECSchemas(SchemaManager const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE