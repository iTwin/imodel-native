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
//! with generated instances. Every test starts from a copy of that file.
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
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ExpectedDataChanges final
    {
    private:
        std::set<Utf8String> m_classesWithDeletedInstances;

    public:
        //! @param[in] classFullName Class in the format {SchemaName}:{ClassName}, as it was before the update
        ExpectedDataChanges& DeleteInstancesOf(Utf8CP classFullName) { m_classesWithDeletedInstances.insert(classFullName); return *this; }
        bool AreInstancesDeleted(Utf8StringCR classFullName) const { return m_classesWithDeletedInstances.find(classFullName) != m_classesWithDeletedInstances.end(); }
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
//! Data is read with plain ECSQL in two ways, which have to agree:
//! - each concrete entity class with ONLY and an explicit property list
//! - each entity class, including abstract classes and mixins, polymorphically
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaUpdateTestFixture : ECDbTestFixture
    {
    protected:
        struct InstanceData final
            {
            ECN::ECClassId m_classId;
            Utf8String m_className;
            //! Property name to the property value as JSON. NULL values are left out.
            std::map<Utf8String, Utf8String> m_values;
            };

        using DataSnapshot = std::map<ECInstanceId, InstanceData>;

    private:
        SchemaUpdateBaseline const* m_baseline = nullptr;
        Utf8String m_schemaName;
        DataSnapshot m_dataBeforeUpdate;
        Utf8String m_schemaXmlBeforeUpdate;
        bool m_lastUpdateFailed = false;

        void ReadData(DataSnapshot&) const;
        Utf8String GetPersistedSchemaXml() const;
        void VerifyIntegrity() const;
        void ExpectNoRows(Utf8CP ecsql) const;
        void VerifyWriteProbe();
        void VerifyWriteProbeInstances();
        DbResult ReopenWithExperimentalFeatures();

    protected:
        //! Opens a copy of the baseline as m_ecdb and reads its data as the state before the update.
        BentleyStatus SetupFromBaseline(SchemaUpdateBaseline const&);

        //! Imports the schema XML. The harness sets the schema version relative to the persisted one.
        //! @return SUCCESS if ECDb accepted the update, ERROR if it rejected it
        BentleyStatus UpdateSchema(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor,
            SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);

        //! Reopens the file and verifies all data against the state before the update, plus the expected changes.
        //! After a rejected update, also verifies that the persisted schema did not change.
        //! Runs the integrity checks and finally writes and reads back one new instance per concrete class.
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
