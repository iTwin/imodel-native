/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaEditor.h>
#include "../SchemaImportContext.h"
#include <functional>
#include <map>
#include <set>
#include <variant>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// Identifiers in persistence statements come from command implementations, never from input.
using SchemaMetaValue = std::variant<std::nullptr_t, int64_t, double, Utf8String>;
using SchemaMetaRow = bvector<std::pair<Utf8String, SchemaMetaValue>>;

struct SchemaMetaTableWriter final {
    ECDbR m_db;
    //! Successful writes that can change ec_cache_ClassHierarchy: base-class rows, class inserts/deletes and schema deletes.
    uint64_t m_hierarchyWrites = 0;
    explicit SchemaMetaTableWriter(ECDbR db) : m_db(db) {}
    SchemaCommandResult Insert(Utf8CP table, SchemaMetaRow const& values);
    SchemaCommandResult Update(Utf8CP table, BeInt64Id id, SchemaMetaRow const& values);
    SchemaCommandResult Delete(Utf8CP table, BeInt64Id id);
};

//! State that outlives a single command but belongs to one editor session.
struct SchemaEditSessionState final {
    //! Upgrade rules use the persisted ECXML version from before any command in the session.
    std::map<ECN::ECSchemaId, std::pair<uint32_t, uint32_t>> m_originalECXmlVersions;
    //! Set only by SchemaImportAdapter: permits creating/upgrading reference-graph standard schemas.
    bool m_allowStandardSchemas = false;
    //! Explicitly declared references (owner, referenced) that reference pruning must preserve.
    std::set<std::pair<ECN::ECSchemaId, ECN::ECSchemaId>> m_pinnedReferences;
    //! Classes created by the currently executing dependency group; create-only rules still apply to them.
    std::set<ECN::ECClassId> m_groupCreatedClasses;
    bool m_inGroup = false;
    //! Owned graph published by the last command of a successful unit. It equals the persisted
    //! schema because mapping writes no EC metadata; dropped when references are pruned or the session ends.
    ECN::ECSchemaPtr m_reusableGraph;
};

struct SchemaEditContext final {
    ECDbR m_db;
    SchemaImportOptions m_options;
    SchemaMetaTableWriter m_writer;
    SchemaImportContext m_mapping;
    std::map<ECN::ECSchemaId, uint32_t> const& m_originalReadVersions;
    SchemaEditSessionState& m_session;
    std::set<ECN::ECClassId> m_affectedClasses;
    std::set<ECN::ECSchemaId> m_affectedSchemas;
    std::set<ECN::ECClassId> m_createdClasses;
    //! Owned post-apply graph for consecutive class and local property commands in the current group.
    ECN::ECSchemaPtr m_propertyCreationSchema;
    std::unique_ptr<SchemaPolicies> m_workingSchemaPolicies;
    //! Set while the schema reader lacks properties created on this class by the current
    //! property-creation run. Only AddProperty on the same class may read it before RefreshReader.
    ECN::ECClassId m_staleReaderClass;

    SchemaEditContext(ECDbR db, SchemaImportOptions options,
                      std::map<ECN::ECSchemaId, uint32_t> const& originalReadVersions, SchemaEditSessionState& session)
        : m_db(db), m_options(options), m_writer(db), m_mapping(db, options),
          m_originalReadVersions(originalReadVersions), m_session(session) {}

    //! True when the class was inserted by this command or earlier in the current dependency group.
    bool IsCreatedInCurrentUnit(ECN::ECClassId id) const { return m_createdClasses.count(id) != 0 || m_session.m_groupCreatedClasses.count(id) != 0; }

    //! Clears reader and ClassMap caches left stale by a property-creation run.
    void RefreshReader() {
        if (m_staleReaderClass.IsValid()) {
            m_db.ClearECDbCache();
            m_staleReaderClass.Invalidate();
        }
    }

    SchemaCommandResult RequireMajorUpgrade(ECN::ECSchemaCR schema) const;
    SchemaCommandResult AddReference(ECN::ECSchemaId owner, ECN::ECSchemaId referenced);
    SchemaCommandResult PruneReferences(ECN::ECSchemaId owner, bool* changed = nullptr);
    SchemaCommandResult CheckInstances(ECN::ECClassCR ecClass, Utf8CP property = nullptr) const;
    SchemaCommandResult ValidateWorkingSchema(ECN::ECSchemaR schema);
    //! Validates a property added to a working graph that already passed ValidateWorkingSchema.
    SchemaCommandResult ValidateWorkingPropertyCreation(ECN::ECSchemaR schema, ECN::ECClassR ecClass,
                                                        ECN::ECClassId classId, ECN::ECPropertyCR prop);

    //! Validates a class newly created in a working graph that already passed ValidateWorkingSchema.
    //! Validates one new (or, with @p persisted, one changed existing) class in an already validated working graph.
    SchemaCommandResult ValidateWorkingClassCreation(ECN::ECSchemaR schema, ECN::ECClassR ecClass, ECN::ECClassId persisted = ECN::ECClassId());

private:
    SchemaCommandResult WorkingSchemaPolicies(SchemaPolicies const*& policies);
    ECN::ECSchemaId AssignWorkingSchemaId(ECN::ECSchemaR schema);
    SchemaCommandResult CheckClassPolicies(SchemaPolicies const& policies, ECN::ECClassR ecClass, ECN::ECClassId classId);
    SchemaCommandResult CheckForeignKeyPolicy(SchemaPolicies const& policies, ECN::ECClassR ecClass,
                                              ECN::ECClassId classId, ECN::ECPropertyCR prop);
};

struct PreparedSchemaCommand final {
    std::function<SchemaCommandResult(SchemaEditContext&)> m_apply;
    bool m_requiresCacheTableRefresh = true;
    //! The command only inserted a class without derived classes and wrote no mapping rows, so
    //! within a group its ec_cache_ClassHierarchy rows can be appended instead of rebuilding the caches.
    bool m_createsLeafClass = false;
    //! False when the command only writes items no class, class map or hierarchy cache row depends on
    //! (units, formats, kinds of quantity, property categories); such commands skip cache rebuilds and mapping.
    bool m_mapsClasses = true;
    ECN::ECSchemaPtr m_propertyCreationSchema;
    //! Class whose local property was added or edited; valid only with m_propertyCreationSchema.
    ECN::ECClassId m_propertyCreationClass;
};

struct ISchemaCommand {
    virtual ~ISchemaCommand() = default;
    virtual SchemaCommandResult Prepare(SchemaEditContext&, BeJsConst, PreparedSchemaCommand&) const = 0;
};

using SchemaCommandFactory = std::function<std::unique_ptr<ISchemaCommand>()>;
using SchemaCommandRegistry = std::map<Utf8String, SchemaCommandFactory, CompareIUtf8Ascii>;

void RegisterSchemaCommands(SchemaCommandRegistry&);

struct IncrementalSchemaMapper final {
    static SchemaCommandResult Map(SchemaEditContext&);
    static SchemaCommandResult PlanOverflowRows(SchemaEditContext&, std::set<ECN::ECClassId> const&);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
