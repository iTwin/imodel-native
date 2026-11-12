/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"
#include <map>
#include <set>

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Schema and data a group of schema update tests starts from.
//! The fixture creates the ECDb file once per test run and fills every concrete entity class
//! and link table relationship class with generated instances. Every test starts from a copy of that file.
//! Baselines are cached by name, so every baseline needs a unique name.
//! The baseline is a single schema that only references standard schemas like ECDbMap.
//! Supported: all primitive types, enumerations, structs, primitive and struct arrays, navigation
//! properties and link table relationships. Arrays of enumerations get plain values that ignore the enumerators.
//! Instance k of a class relates to candidate (k * (instancesPerClass + 1)) % candidateCount, so the
//! instances of one class point to instances of different classes. Navigation properties cycle
//! through the concrete relationship classes of their hierarchy that allow the instance, so derived
//! relationship classes get instances too. Navigation properties with a NOT NULL column are not supported.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaUpdateBaseline final
    {
    private:
        Utf8String m_name;
        Utf8String m_schemaXml;
        int m_instancesPerClass;

    public:
        SchemaUpdateBaseline(Utf8CP name, Utf8CP schemaXml, int instancesPerClass = 3) : m_name(name), m_schemaXml(schemaXml), m_instancesPerClass(instancesPerClass) {}

        Utf8StringCR GetName() const { return m_name; }
        Utf8StringCR GetSchemaXml() const { return m_schemaXml; }
        int GetInstancesPerClass() const { return m_instancesPerClass; }
    };

//=======================================================================================
//! Data changes a schema update is expected to cause. Everything not listed here has to read the
//! same after the update, with properties the update deleted left out and properties it added read as NULL.
//! If a test needs to declare another kind of change, add it here instead of working around
//! VerifyData in the test.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ExpectedDataChanges final
    {
    private:
        std::set<Utf8String> m_classesWithDeletedInstances;
        std::set<Utf8String> m_classesWithDeletedReferencingInstances;

    public:
        //! @param[in] classFullName Class in the format {SchemaName}:{ClassName}, as it was before the update
        ExpectedDataChanges& DeleteInstancesOf(Utf8CP classFullName) { m_classesWithDeletedInstances.insert(classFullName); return *this; }
        //! Instances of the class that point to an instance deleted through DeleteInstancesOf, through a navigation
        //! property or as source or target of a link table relationship, are deleted too, e.g. by a cascading foreign key.
        //! @param[in] classFullName Class in the format {SchemaName}:{ClassName}, as it was before the update
        ExpectedDataChanges& DeleteReferencingInstancesOf(Utf8CP classFullName) { m_classesWithDeletedReferencingInstances.insert(classFullName); return *this; }
        bool AreInstancesDeleted(Utf8StringCR classFullName) const { return m_classesWithDeletedInstances.find(classFullName) != m_classesWithDeletedInstances.end(); }
        bool AreReferencingInstancesDeleted(Utf8StringCR classFullName) const { return m_classesWithDeletedReferencingInstances.find(classFullName) != m_classesWithDeletedReferencingInstances.end(); }
        std::set<Utf8String> const& GetClassesWithDeletedReferencingInstances() const { return m_classesWithDeletedReferencingInstances; }
    };

enum class SchemaVersionChange
    {
    None,
    Minor,
    Major
    };

//=======================================================================================
//! Runs a schema update on a copy of a baseline and verifies all data in the file afterwards,
//! not only the data of the classes the update touches.
//!
//! Usage, see SchemaUpdateTests.cpp for examples:
//! - Derive a fixture per area with a static baseline. Keep baselines small, the update runs
//!   against the whole schema. Add a new fixture rather than growing an unrelated baseline.
//! - SetupFromBaseline, edit GetBaselineSchemaXml with ReplaceInClass, RemoveClass and
//!   AppendToSchema, then UpdateSchema. Leave the version in the XML alone, UpdateSchema sets it.
//! - After a rejected update, state the reason with ExpectRejectionReason. VerifyData fails otherwise.
//! - Always end with VerifyData, also after a rejected update.
//! - Assert only on observable results: accepted or rejected, data, persisted schema.
//!   Never on tables or columns, so the tests stay valid when the mapping changes.
//! - SchemaUpdateHarnessTests checks the harness itself. Run it after changing the harness.
//!
//! Data is read with plain ECSQL in two ways, which have to agree:
//! - each concrete class with ONLY and an explicit property list
//! - each class, including abstract classes and mixins, polymorphically
//! Classes are entity classes and link table relationship classes. Relationships mapped to
//! navigation properties are covered by reading those properties, and their relationship classes
//! are queried with ONLY and polymorphically, which has to return a row per navigation property value.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaUpdateTestFixture : ECDbTestFixture
    {
    public:
        struct NavigationValue final
            {
            ECInstanceId m_id;
            ECN::ECClassId m_relClassId;
            };

    protected:
        struct InstanceData final
            {
            ECN::ECClassId m_classId;
            Utf8String m_className;
            //! Property name to the property value as JSON. NULL values are left out.
            std::map<Utf8String, Utf8String> m_values;
            //! Navigation property name to its value. NULL values are left out.
            std::map<Utf8String, NavigationValue> m_navigationValues;
            };

        using DataSnapshot = std::map<ECInstanceId, InstanceData>;

    private:
        SchemaUpdateBaseline const* m_baseline = nullptr;
        Utf8String m_schemaName;
        DataSnapshot m_dataBeforeUpdate;
        Utf8String m_schemaXmlBeforeUpdate;
        bool m_lastUpdateFailed = false;
        std::vector<ReportedIssue> m_lastUpdateIssues;
        bool m_isRejectionReasonVerified = false;

        void ReadData(DataSnapshot&) const;
        void VerifyForeignKeyRelationships(DataSnapshot const&) const;
        Utf8String GetPersistedSchemaXml() const;
        void VerifyIntegrity() const;
        void ExpectNoRows(Utf8CP ecsql) const;
        void VerifyWriteProbe();
        void VerifyWriteProbeInstances();
        DbResult ReopenWithExperimentalFeatures();

    protected:
        //! Opens a copy of the baseline as m_ecdb and reads its data as the state before the update.
        BentleyStatus SetupFromBaseline(SchemaUpdateBaseline const&);

        //! Runs an ECSQL statement that changes the baseline data for this test, then reads the data again as the state before the update.
        void ChangeDataBeforeUpdate(Utf8CP ecsql);

        //! Imports the schema XML. The harness sets the schema version relative to the persisted one.
        //! VerifyPersistedSchemas is always added to the options, so an update that is accepted but not stored as imported fails.
        //! @return SUCCESS if ECDb accepted the update, ERROR if it rejected it
        BentleyStatus UpdateSchema(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor,
            SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);

        //! Checks that the last update was rejected and that ECDb reported the given issue, e.g. "ECDb_0386".
        //! Required after every rejected update, so a test cannot pass because the update was rejected for another reason.
        void ExpectRejectionReason(Utf8CP issueId);

        //! Reopens the file and verifies all data against the state before the update, plus the expected changes.
        //! After a rejected update, also verifies that the persisted schema did not change.
        //! Runs the integrity checks and finally writes and reads back one new instance per concrete class.
        //! The integrity checks include check_nav_ids, which reports navigation properties pointing to deleted instances.
        void VerifyData(ExpectedDataChanges const& = ExpectedDataChanges());

        Utf8String GetBaselineSchemaXml() const { return m_baseline != nullptr ? m_baseline->GetSchemaXml() : Utf8String(); }

        //! Replaces the only occurrence of @p oldText within the XML element of the given class.
        static void ReplaceInClass(Utf8StringR schemaXml, Utf8CP className, Utf8CP oldText, Utf8CP newText);
        //! Removes the XML element of the given class.
        static void RemoveClass(Utf8StringR schemaXml, Utf8CP className);
        //! Adds schema items, e.g. new classes, at the end of the schema.
        static void AppendToSchema(Utf8StringR schemaXml, Utf8CP schemaItemsXml);
    };

END_ECDBUNITTESTS_NAMESPACE
