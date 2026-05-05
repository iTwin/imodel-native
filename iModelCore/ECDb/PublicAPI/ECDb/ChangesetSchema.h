/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <BeSQLite/ChangeSet.h>
#include <BeRapidJson/BeJsValue.h>
#include <map>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetSchemaPropertyMap final
    {
    Utf8String m_table;
    Utf8String m_column;

    ChangesetSchemaPropertyMap() {}
    ChangesetSchemaPropertyMap(Utf8StringCR table, Utf8StringCR column)
        : m_table(table), m_column(column) {}

    bool operator==(ChangesetSchemaPropertyMap const& rhs) const { return m_table == rhs.m_table && m_column == rhs.m_column; }
    bool operator!=(ChangesetSchemaPropertyMap const& rhs) const { return !(*this == rhs); }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetSchemaTableInfo final
    {
    Utf8String m_type; // "Primary", "Joined", "Overflow"

    ChangesetSchemaTableInfo() {}
    explicit ChangesetSchemaTableInfo(Utf8StringCR type) : m_type(type) {}
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetSchemaClassEntry final
    {
    Utf8String m_classKey;  // "SchemaName:ClassName"
    ECN::ECClassId m_classId;
    Utf8String m_baseClass; // empty if root

    std::map<Utf8String, ChangesetSchemaTableInfo> m_tables;
    std::map<Utf8String, ChangesetSchemaPropertyMap> m_propertyMaps;
    };

//=======================================================================================
// @bsiclass
// Captures the schema-to-SQLite mapping state relevant to a changeset.
// Serializable to/from JSON for storage alongside the changeset.
//=======================================================================================
struct ChangesetSchema final
    {
private:
    std::map<Utf8String, ChangesetSchemaClassEntry> m_classes;

public:
    ChangesetSchema() {}

    std::map<Utf8String, ChangesetSchemaClassEntry> const& GetClasses() const { return m_classes; }
    std::map<Utf8String, ChangesetSchemaClassEntry>& GetClassesR() { return m_classes; }

    //! Capture the schema mapping state for all mapped classes in the ECDb.
    static BentleyStatus CaptureFromDb(ChangesetSchema& out, ECDbCR ecdb);

    //! Capture the schema mapping state for only classes referenced by the given changeset.
    //! Iterates the changeset to discover referenced tables and ECClassId values,
    //! then captures only those classes and their inheritance chain.
    static BentleyStatus Capture(ChangesetSchema& out, ECDbCR ecdb, BeSQLite::ChangeStream const& changeset);

    //! Serialize to a BeJsValue (object).
    void ToJson(BeJsValue out) const;

    //! Deserialize from a BeJsConst.
    static BentleyStatus FromJson(ChangesetSchema& out, BeJsConst json);

    //! Resolve the full property map for a class by walking the inheritance chain.
    std::map<Utf8String, ChangesetSchemaPropertyMap> GetFullPropertyMap(Utf8StringCR classKey) const;
    };

//=======================================================================================
// @bsiclass
// Result of diffing two ChangesetSchema snapshots.
//=======================================================================================
struct ChangesetSchemaDiff final
    {
    struct ClassIdRemap
        {
        Utf8String m_classKey;
        ECN::ECClassId m_oldClassId;
        ECN::ECClassId m_newClassId;
        };

    struct ColumnSwap
        {
        Utf8String m_classKey;
        Utf8String m_accessString;
        Utf8String m_oldTable;
        Utf8String m_oldColumn;
        Utf8String m_newTable;
        Utf8String m_newColumn;
        };

    struct OverflowTableAdded
        {
        Utf8String m_classKey;
        Utf8String m_overflowTable;
        Utf8String m_parentTable;
        bool m_hasECClassIdColumn;
        };

    struct MissingMapping
        {
        enum class Kind { Class, Property, Table };
        Kind m_kind;
        Utf8String m_classKey;
        Utf8String m_accessString;
        Utf8String m_tableName;
        Utf8String m_message;
        };

    bvector<ClassIdRemap> m_classIdRemaps;
    bvector<ColumnSwap> m_columnSwaps;
    bvector<OverflowTableAdded> m_overflowTablesAdded;
    bvector<MissingMapping> m_errors;

    bool NeedsTransform() const
        {
        return !HasErrors()
            && (!m_classIdRemaps.empty()
                || !m_columnSwaps.empty()
                || !m_overflowTablesAdded.empty());
        }

    bool HasErrors() const { return !m_errors.empty(); }

    void ToJson(BeJsValue out) const;

    static BentleyStatus Diff(ChangesetSchemaDiff& out,
                              ChangesetSchema const& before,
                              ChangesetSchema const& after);
    };

//=======================================================================================
// @bsiclass
// Transforms a changeset binary by applying the diff (class ID remaps, column swaps,
// overflow INSERTs) produced by ChangesetSchemaDiff::Diff.
//=======================================================================================
struct ChangesetTransformer final
    {
    //! Apply all transforms from the diff to the source changeset.
    //! Produces a new transformed changeset in 'output'.
    //! Returns ERROR if diff.HasErrors() — the changeset must not be transformed
    //! when the diff contains missing-class or missing-property errors.
    static BentleyStatus Transform(BeSQLite::ChangeSet& output,
                                   BeSQLite::ChangeStream const& source,
                                   ChangesetSchemaDiff const& diff,
                                   ECDbCR ecdb);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
