/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <map>
#include <set>
#include <ECDb/JsonAdapter.h>
#include "MockHubApi.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Sequences of dynamic-schema updates, shaped like connector syncs, that left iModels with
// properties missing from ec_Property, duplicate or diverged ec_PropertyMap rows, orphan
// custom attributes, or instance values read from the wrong column.
//
// Every scenario is produced by schema imports, instance writes and mock-hub push/pull
// alone; no test writes to an ec_ table. After every step, the importer and each briefcase
// that pulled the changeset must have healthy property maps, identical ec_ tables, and
// instances that read exactly the values they read before the step.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyMapIntegrityTestFixture : public ECDbTestFixture
    {
    using Row = std::map<Utf8String, Utf8String>;   //!< member name -> value as JSON; NULL members are absent
    using Rows = std::map<uint64_t, Row>;           //!< ECInstanceId -> row
    using Snapshot = std::map<Utf8String, Rows>;    //!< class full name -> rows read with FROM ONLY
    using Names = std::set<Utf8String>;
    using Writer = std::function<void(ECDbR)>;

    //! Connectors import dynamic schemas, which may make major changes such as deleting
    //! properties and schema items, and remapping may have to move instance data.
    static SchemaManager::SchemaImportOptions UpgradeOptions()
        {
        return SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade |
            SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas;
        }

    //! A dynamic schema with the TablePerHierarchy root and the joined table with shared
    //! columns that physical elements use. @p body adds the classes under test.
    static Utf8String MakeSchema(int minor, Utf8StringCR body, int maxSharedColumns = 32, Utf8CP extraReferences = "")
        {
        return Utf8PrintfString(R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.%02d" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
  <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
  <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
  %s
  <ECCustomAttributes>
    <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
  </ECCustomAttributes>
  <ECEntityClass typeName="Root" modifier="Abstract">
    <ECCustomAttributes>
      <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
      <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
    </ECCustomAttributes>
  </ECEntityClass>
  <ECEntityClass typeName="JoinedBase" modifier="Abstract">
    <BaseClass>Root</BaseClass>
    <ECCustomAttributes>
      <ShareColumns xmlns="ECDbMap.02.00.00">
        <MaxSharedColumnsBeforeOverflow>%d</MaxSharedColumnsBeforeOverflow>
        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
      </ShareColumns>
    </ECCustomAttributes>
  </ECEntityClass>
  %s
</ECSchema>)xml", minor, extraReferences, maxSharedColumns, body.c_str());
        }

    static Utf8String Prop(Utf8CP name, Utf8CP type, Utf8CP attributes = "", Utf8CP customAttributes = nullptr)
        {
        if (customAttributes == nullptr)
            return Utf8PrintfString(R"xml(<ECProperty propertyName="%s" typeName="%s" %s/>)xml", name, type, attributes);
        return Utf8PrintfString(R"xml(<ECProperty propertyName="%s" typeName="%s" %s><ECCustomAttributes>%s</ECCustomAttributes></ECProperty>)xml",
            name, type, attributes, customAttributes);
        }

    //! @p bases is a comma-separated list; the first is the base class, any others are mixins.
    static Utf8String Entity(Utf8CP name, Utf8CP bases, Utf8StringCR members, bool isAbstract = false, Utf8CP customAttributes = nullptr)
        {
        Utf8String xml = Utf8PrintfString(R"xml(<ECEntityClass typeName="%s"%s>)xml", name, isAbstract ? R"xml( modifier="Abstract")xml" : "");
        bvector<Utf8String> baseNames;
        BeStringUtilities::Split(bases, ",", baseNames);
        for (auto const& baseName : baseNames)
            {
            if (!baseName.empty())
                xml.append("<BaseClass>").append(baseName).append("</BaseClass>");
            }
        if (customAttributes != nullptr)
            xml.append("<ECCustomAttributes>").append(customAttributes).append("</ECCustomAttributes>");
        return xml.append(members).append("</ECEntityClass>");
        }

    //! An abstract base over concrete leaves, every leaf property in a shared column of the
    //! joined table, as in plant-design connector schemas. Tests edit the members of a class
    //! to make a schema change.
    struct Plant
        {
        Utf8String m_plantBase = Prop("Guid", "string");
        Utf8String m_item = Prop("Note", "string");
        Utf8String m_pump = Prop("Flow", "double") + Prop("Tag", "string");
        Utf8String m_valve = Prop("Size", "double") + Prop("Tag", "string");
        Utf8String m_gateValve = Prop("Stem", "string");
        Utf8String m_itemBases = "PlantBase";
        Utf8String m_pumpBases = "Item";
        Utf8String m_valveBases = "Item";
        Utf8String m_extra; //!< other schema items, such as structs or mixins

        Utf8String ToXml() const
            {
            return m_extra +
                Entity("PlantBase", "JoinedBase", m_plantBase, true) +
                Entity("Item", m_itemBases.c_str(), m_item) +
                Entity("Pump", m_pumpBases.c_str(), m_pump) +
                Entity("Valve", m_valveBases.c_str(), m_valve) +
                Entity("GateValve", "Valve", m_gateValve);
            }
        };

    static Utf8CP SeverityName(ECN::IssueSeverity severity)
        {
        switch (severity)
            {
            case ECN::IssueSeverity::Fatal: return "Fatal";
            case ECN::IssueSeverity::Error: return "Error";
            case ECN::IssueSeverity::CriticalWarning: return "CriticalWarning";
            case ECN::IssueSeverity::Warning: return "Warning";
            default: return "Info";
            }
        }

    static void PrintIssues(TestIssueListener const& listener, Utf8CP context)
        {
        for (auto const& issue : listener.m_issues)
            {
            if (issue.severity != ECN::IssueSeverity::Info)
                printf("[%s] %s %s: %s\n", context, SeverityName(issue.severity), issue.id.m_issueId, issue.message.c_str());
            }
        }

    //! Imports @p xml with the options a connector uses. Reports the issues if it fails.
    static SchemaImportResult Import(ECDbR db, Utf8StringCR xml, Utf8CP context)
        {
        TestIssueListener listener;
        db.AddIssueListener(listener);
        const auto result = SchemaSyncTestFixture::ImportSchema(db, SchemaItem(xml), UpgradeOptions());
        db.RemoveIssueListener();
        if (result != SchemaImportResult::OK)
            {
            PrintIssues(listener, context);
            db.AbandonChanges();
            }
        return result;
        }

    static void Execute(ECDbR db, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(db, ecsql))
            {
            ADD_FAILURE() << "Failed to prepare " << ecsql;
            return;
            }
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecsql;
        }

    //! Reads every row of @p ecsql, a SELECT * whose first column is ECInstanceId, keyed by
    //! that id. Fails the test if the statement cannot be prepared or stepped to the end.
    static bool ReadRows(Rows& rows, ECDbR db, Utf8StringCR ecsql, Utf8CP context, ECClassId bindClassId = ECClassId())
        {
        TestIssueListener listener;
        const bool listening = SUCCESS == db.AddIssueListener(listener);
        ECSqlStatement stmt;
        const auto prepareStatus = stmt.Prepare(db, ecsql.c_str());
        if (listening)
            db.RemoveIssueListener();
        if (ECSqlStatus::Success != prepareStatus)
            {
            ADD_FAILURE() << context << ": cannot prepare " << ecsql.c_str() << ": " << listener.GetLastMessage().c_str();
            return false;
            }
        if (bindClassId.IsValid())
            stmt.BindId(1, bindClassId);

        JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
        DbResult rc;
        while (BE_SQLITE_ROW == (rc = stmt.Step()))
            {
            BeJsDocument json;
            if (SUCCESS != adapter.GetRow(json))
                {
                ADD_FAILURE() << context << ": cannot convert a row of " << ecsql.c_str() << " to JSON";
                return false;
                }
            Row row;
            json.ForEachProperty([&](Utf8CP name, BeJsConst value)
                {
                if (0 != strcmp(name, ECJsonSystemNames::Id()))
                    row[name] = value.Stringify();
                return false;
                });
            rows[stmt.GetValueId<ECInstanceId>(0).GetValue()] = row;
            }
        if (BE_SQLITE_DONE != rc)
            {
            ADD_FAILURE() << context << ": " << ecsql.c_str() << " failed to step: " << db.GetLastError().c_str();
            return false;
            }
        return true;
        }

    static std::vector<ECClassCP> GetEntityClasses(ECDbR db)
        {
        std::vector<ECClassCP> classes;
        ECSchemaCP schema = db.Schemas().GetSchema("TestSchema", true);
        if (schema == nullptr)
            return classes;
        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (ecClass->IsEntityClass())
                classes.push_back(ecClass);
            }
        return classes;
        }

    static bool IsConcrete(ECClassCR ecClass) { return ecClass.GetClassModifier() != ECClassModifier::Abstract && !ecClass.IsMixin(); }

    //! Every instance of the test schema, read class by class with FROM ONLY.
    static Snapshot TakeSnapshot(ECDbR db, Utf8CP context)
        {
        Snapshot snapshot;
        for (ECClassCP ecClass : GetEntityClasses(db))
            {
            if (!IsConcrete(*ecClass))
                continue;
            Rows rows;
            ReadRows(rows, db, Utf8PrintfString("SELECT * FROM ONLY %s", ecClass->GetECSqlName().c_str()), context);
            if (!rows.empty())
                snapshot[ecClass->GetFullName()] = rows;
            }
        return snapshot;
        }

    //! Each member must read the same value in @p actual as in @p expected. A member in
    //! @p mayBecomeNull may read NULL in @p actual, because its property was deleted, but no
    //! other value. No member may gain a value: a property added since must read NULL.
    static void ExpectSameRow(Row const& expected, Row const& actual, Names const& mayBecomeNull, Utf8CP context, Utf8StringCR className, uint64_t id)
        {
        Names members;
        for (auto const& member : expected)
            members.insert(member.first);
        for (auto const& member : actual)
            members.insert(member.first);
        for (auto const& member : members)
            {
            const auto e = expected.find(member);
            const auto a = actual.find(member);
            if (e != expected.end() && a != actual.end())
                {
                if (!e->second.Equals(a->second))
                    ADD_FAILURE() << context << ": " << className.c_str() << " " << id << " reads " << member.c_str() << "=" << a->second.c_str() << ", expected " << e->second.c_str();
                }
            else if (a == actual.end())
                {
                if (mayBecomeNull.find(member) == mayBecomeNull.end())
                    ADD_FAILURE() << context << ": " << className.c_str() << " " << id << " reads " << member.c_str() << "=NULL, expected " << e->second.c_str();
                }
            else
                ADD_FAILURE() << context << ": " << className.c_str() << " " << id << " reads " << member.c_str() << "=" << a->second.c_str() << ", expected NULL";
            }
        }

    static void ExpectSameValues(Snapshot const& expected, Snapshot const& actual, Utf8CP context, Names const& mayBecomeNull = {})
        {
        static const Rows s_none;
        Names classNames;
        for (auto const& entry : expected)
            classNames.insert(entry.first);
        for (auto const& entry : actual)
            classNames.insert(entry.first);
        for (auto const& className : classNames)
            {
            const auto e = expected.find(className);
            const auto a = actual.find(className);
            Rows const& expectedRows = e == expected.end() ? s_none : e->second;
            Rows const& actualRows = a == actual.end() ? s_none : a->second;
            for (auto const& row : expectedRows)
                {
                const auto found = actualRows.find(row.first);
                if (found == actualRows.end())
                    ADD_FAILURE() << context << ": " << className.c_str() << " " << row.first << " is gone";
                else
                    ExpectSameRow(row.second, found->second, mayBecomeNull, context, className, row.first);
                }
            for (auto const& row : actualRows)
                {
                if (expectedRows.find(row.first) == expectedRows.end())
                    ADD_FAILURE() << context << ": " << className.c_str() << " " << row.first << " appeared";
                }
            }
        }

    //! Fails once for each row @p sql, a read of ec_ tables, returns.
    static void ExpectNoRows(ECDbR db, Utf8CP sql, Utf8CP context, Utf8CP problem, std::function<void(Statement&)> bind = nullptr)
        {
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(db, sql))
            {
            ADD_FAILURE() << context << ": cannot prepare " << sql << ": " << db.GetLastError().c_str();
            return;
            }
        if (bind)
            bind(stmt);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            Utf8String row;
            for (int i = 0; i < stmt.GetColumnCount(); ++i)
                row.append(i == 0 ? "" : " | ").append(stmt.IsColumnNull(i) ? "NULL" : stmt.GetValueText(i));
            ADD_FAILURE() << context << ": " << problem << ": " << row.c_str();
            }
        }

    //! Each mapped class of the test schema must have a map for every property it has, and no
    //! map for a property it does not have.
    static void ExpectMapsMatchProperties(ECDbR db, Utf8CP context)
        {
        for (ECClassCP ecClass : GetEntityClasses(db))
            {
            const auto strategy = db.Schemas().GetClassMapStrategy("TestSchema", ecClass->GetName());
            if (ecClass->IsMixin() || strategy.IsEmpty() || strategy.GetStrategy() == ClassMapStrategy::MapStrategy::NotMapped)
                continue;

            Names mapped;
            Statement stmt;
            ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT DISTINCT CASE WHEN instr(pp.AccessString,'.')>0 THEN substr(pp.AccessString,1,instr(pp.AccessString,'.')-1) ELSE pp.AccessString END "
                "FROM main.ec_PropertyMap pm JOIN main.ec_PropertyPath pp ON pp.Id=pm.PropertyPathId WHERE pm.ClassId=?"));
            stmt.BindId(1, ecClass->GetId());
            while (BE_SQLITE_ROW == stmt.Step())
                mapped.insert(stmt.GetValueText(0));

            Names properties = {"ECInstanceId", "ECClassId"};
            for (ECPropertyCP prop : ecClass->GetProperties(true))
                properties.insert(prop->GetName());
            for (auto const& name : properties)
                {
                if (mapped.find(name) == mapped.end())
                    ADD_FAILURE() << context << ": " << ecClass->GetFullName() << "." << name.c_str() << " has no property map";
                }
            for (auto const& name : mapped)
                {
                if (properties.find(name) == properties.end())
                    ADD_FAILURE() << context << ": " << ecClass->GetFullName() << " has a property map for " << name.c_str() << ", which is not one of its properties";
                }
            }
        }

    static void CollectAncestors(ECClassCR ecClass, std::set<ECClassCP>& ancestors)
        {
        for (ECClassCP baseClass : ecClass.GetBaseClasses())
            {
            if (ancestors.insert(baseClass).second)
                CollectAncestors(*baseClass, ancestors);
            }
        }

    //! Every class of the test schema must be readable, and each instance must read the same
    //! values through each of its base classes and mixins as through its own class.
    static void ExpectConsistentReads(ECDbR db, Utf8CP context)
        {
        const auto classes = GetEntityClasses(db);
        for (ECClassCP ecClass : classes)
            {
            Rows rows;
            ReadRows(rows, db, Utf8PrintfString("SELECT * FROM %s", ecClass->GetECSqlName().c_str()), context);
            }

        for (ECClassCP concrete : classes)
            {
            if (!IsConcrete(*concrete))
                continue;
            Rows exact;
            if (!ReadRows(exact, db, Utf8PrintfString("SELECT * FROM ONLY %s", concrete->GetECSqlName().c_str()), context))
                continue;

            std::set<ECClassCP> ancestors;
            CollectAncestors(*concrete, ancestors);
            for (ECClassCP ancestor : ancestors)
                {
                Rows polymorphic;
                if (!ReadRows(polymorphic, db, Utf8PrintfString("SELECT * FROM %s WHERE ECClassId=?", ancestor->GetECSqlName().c_str()), context, concrete->GetId()))
                    continue;

                Names members = {"className"};
                for (ECPropertyCP prop : ancestor->GetProperties(true))
                    members.insert(prop->GetName());
                const Utf8PrintfString through("%s, %s read through %s", context, concrete->GetName().c_str(), ancestor->GetName().c_str());
                for (auto const& row : exact)
                    {
                    const auto found = polymorphic.find(row.first);
                    if (found == polymorphic.end())
                        {
                        ADD_FAILURE() << through.c_str() << ": instance " << row.first << " is missing";
                        continue;
                        }
                    Row expected;
                    for (auto const& member : row.second)
                        {
                        if (members.find(member.first) != members.end())
                            expected.insert(member);
                        }
                    ExpectSameRow(expected, found->second, {}, through.c_str(), concrete->GetName(), row.first);
                    }
                for (auto const& row : polymorphic)
                    {
                    if (exact.find(row.first) == exact.end())
                        ADD_FAILURE() << through.c_str() << ": instance " << row.first << " is not an instance of the class";
                    }
                }
            }
        }

    //! The checks used to diagnose the damaged iModels, run against the test schema.
    static void ExpectHealthyPropertyMaps(ECDbR db, Utf8CP context)
        {
        ExpectNoRows(db, "SELECT c.Name, pp.AccessString, COUNT(*) FROM main.ec_PropertyMap pm "
            "JOIN main.ec_PropertyPath pp ON pp.Id=pm.PropertyPathId JOIN main.ec_Class c ON c.Id=pm.ClassId JOIN main.ec_Schema s ON s.Id=c.SchemaId "
            "WHERE s.Name='TestSchema' AND pp.AccessString NOT IN ('ECInstanceId','ECClassId') "
            "GROUP BY pm.ClassId, pp.AccessString HAVING COUNT(*)>1",
            context, "duplicate property maps (class | access string | maps)");

        ExpectNoRows(db, "SELECT c.Name, t.Name, col.Name, GROUP_CONCAT(pp.AccessString) FROM main.ec_PropertyMap pm "
            "JOIN main.ec_PropertyPath pp ON pp.Id=pm.PropertyPathId JOIN main.ec_Column col ON col.Id=pm.ColumnId JOIN main.ec_Table t ON t.Id=col.TableId "
            "JOIN main.ec_Class c ON c.Id=pm.ClassId JOIN main.ec_Schema s ON s.Id=c.SchemaId "
            "WHERE s.Name='TestSchema' AND col.IsVirtual=0 "
            "GROUP BY pm.ClassId, pm.ColumnId HAVING COUNT(DISTINCT pp.AccessString)>1",
            context, "one column holds several properties of a class (class | table | column | access strings)");

        ExpectNoRows(db, "SELECT ca.Id, ca.ContainerId, cc.Name FROM main.ec_CustomAttribute ca LEFT JOIN main.ec_Class cc ON cc.Id=ca.ClassId "
            "WHERE ca.ContainerType=? AND NOT EXISTS (SELECT 1 FROM main.ec_Property p WHERE p.Id=ca.ContainerId)",
            context, "orphan property custom attribute (id | container id | custom attribute class)",
            [](Statement& stmt) { stmt.BindInt(1, static_cast<int>(CustomAttributeContainerType::AnyProperty)); });

        ExpectNoRows(db, "SELECT ca.Id, ca.ContainerId, cc.Name FROM main.ec_CustomAttribute ca LEFT JOIN main.ec_Class cc ON cc.Id=ca.ClassId "
            "WHERE ca.ContainerType=? AND NOT EXISTS (SELECT 1 FROM main.ec_Class c WHERE c.Id=ca.ContainerId)",
            context, "orphan class custom attribute (id | container id | custom attribute class)",
            [](Statement& stmt) { stmt.BindInt(1, static_cast<int>(CustomAttributeContainerType::AnyClass)); });

        ExpectMapsMatchProperties(db, context);

        // The pragma reloads the tables of the DbSchema but not the class maps that ECSQL
        // statements already loaded, which still point at the old tables.
        db.ClearECDbCache();
        ECSqlStatement validation;
        if (ECSqlStatus::Success != validation.Prepare(db, "PRAGMA validate_persisted_mappings OPTIONS enable_experimental_features"))
            ADD_FAILURE() << context << ": cannot prepare validate_persisted_mappings";
        else
            {
            while (BE_SQLITE_ROW == validation.Step())
                {
                if (0 == strcmp("Error", validation.GetValueText(0)))
                    ADD_FAILURE() << context << ": validate_persisted_mappings " << validation.GetValueText(3) << ": " << validation.GetValueText(4);
                }
            }

        ECSqlStatement diverged;
        if (ECSqlStatus::Success != diverged.Prepare(db, "PRAGMA integrity_check(check_diverged_prop_maps) OPTIONS enable_experimental_features"))
            ADD_FAILURE() << context << ": cannot prepare check_diverged_prop_maps";
        else
            {
            while (BE_SQLITE_ROW == diverged.Step())
                ADD_FAILURE() << context << ": " << diverged.GetValueText(2) << "." << diverged.GetValueText(5) << " maps to " << diverged.GetValueText(7)
                    << " but its base class " << diverged.GetValueText(4) << " maps it to " << diverged.GetValueText(6);
            }

        ExpectConsistentReads(db, context);
        SchemaSyncTestFixture::VerifyFileIsSound(db, context);
        }

    //! Pulls and merges what other briefcases pushed, then pushes local changes, as a
    //! connector does around a sync. The merge must not report errors.
    static void Sync(TrackedECDb& db, Utf8CP context)
        {
        TestIssueListener listener;
        db.AddIssueListener(listener);
        const auto rc = db.PullMergePush(context);
        db.RemoveIssueListener();
        PrintIssues(listener, context);
        EXPECT_EQ(BE_SQLITE_OK, rc) << context << ": pull/merge/push failed";
        for (auto const& issue : listener.m_issues)
            EXPECT_NE(ECN::IssueSeverity::Error, issue.severity) << context << ": " << issue.message.c_str();
        }

    //! One connector sync: @p importer pulls, imports @p xml, writes with @p writer, and
    //! pushes; then each of @p pullers pulls. The import must not change what any instance
    //! reads, except that members in @p mayBecomeNull may read NULL. The importer and every
    //! puller must pass ExpectHealthyPropertyMaps, and each puller must end up with the
    //! importer's ec_ tables and instance values. Returns false if anything failed so far.
    static bool ImportPushPull(TrackedECDb& importer, std::vector<TrackedECDb*> const& pullers, Utf8StringCR xml, Utf8CP step, Names const& mayBecomeNull = {}, Writer writer = nullptr)
        {
        const Utf8PrintfString context("%s, importer", step);
        Sync(importer, context.c_str());
        const auto before = TakeSnapshot(importer, context.c_str());
        if (SchemaImportResult::OK != Import(importer, xml, context.c_str()))
            {
            ADD_FAILURE() << context.c_str() << ": schema import failed";
            return false;
            }
        EXPECT_EQ(BE_SQLITE_OK, importer.SaveChanges()) << context.c_str();
        ExpectSameValues(before, TakeSnapshot(importer, context.c_str()), context.c_str(), mayBecomeNull);
        ExpectHealthyPropertyMaps(importer, context.c_str());
        if (writer != nullptr)
            {
            writer(importer);
            EXPECT_EQ(BE_SQLITE_OK, importer.SaveChanges()) << context.c_str();
            }
        Sync(importer, context.c_str());
        const auto expected = TakeSnapshot(importer, context.c_str());

        int index = 0;
        for (TrackedECDb* puller : pullers)
            {
            const Utf8PrintfString pullerContext("%s, puller %d", step, ++index);
            Sync(*puller, pullerContext.c_str());
            ExpectHealthyPropertyMaps(*puller, pullerContext.c_str());
            SchemaSyncTestFixture::ExpectECTablesIdentical(*puller, importer, pullerContext.c_str());
            ExpectSameValues(expected, TakeSnapshot(*puller, pullerContext.c_str()), pullerContext.c_str());
            }
        return !SchemaSyncTestFixture::CurrentTestHasFailed();
        }

    //! One instance of each class of the default Plant, with every property set.
    static void InsertPlant(ECDbR db, Utf8CP label)
        {
        Execute(db, Utf8PrintfString("INSERT INTO ts.Item(Guid,Note) VALUES('%s-item','%s item note')", label, label).c_str());
        Execute(db, Utf8PrintfString("INSERT INTO ts.Pump(Guid,Note,Flow,Tag) VALUES('%s-pump','%s pump note',12.5,'%s-P')", label, label, label).c_str());
        Execute(db, Utf8PrintfString("INSERT INTO ts.Valve(Guid,Note,Size,Tag) VALUES('%s-valve','%s valve note',4.25,'%s-V')", label, label, label).c_str());
        Execute(db, Utf8PrintfString("INSERT INTO ts.GateValve(Guid,Note,Size,Tag,Stem) VALUES('%s-gate','%s gate note',6.5,'%s-G','%s stem')", label, label, label, label).c_str());
        }

    static Utf8CP HiddenProperty() { return R"xml(<HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>)xml"; }
    static Utf8CP UnitsReference() { return R"xml(<ECSchemaReference name="Units" version="01.00.00" alias="u"/>)xml"; }

    //! A schema whose @p item, a PropertyCategory, KindOfQuantity or ECEnumeration, is used
    //! by Item.@p moved, which carries a custom attribute, by Leaf.LeafProp, declared like
    //! Item.@p moved, and by Item.@p plain. @p itemMembers adds properties to Item.
    static Utf8String ReferencingSchema(int minor, Utf8CP item, Utf8CP moved, Utf8CP movedType, Utf8CP movedAttributes,
        Utf8CP plain, Utf8CP plainType, Utf8CP plainAttributes, Utf8CP extraReferences = "", Utf8CP itemMembers = "")
        {
        return MakeSchema(minor, Utf8String(item) +
            Entity("Item", "JoinedBase", Prop(moved, movedType, movedAttributes, HiddenProperty()) + Prop(plain, plainType, plainAttributes) + Prop("Other", "string") + itemMembers) +
            Entity("Leaf", "Item", Prop("LeafProp", movedType, movedAttributes)), 32, extraReferences);
        }

    //! A sync without any checks, for steps whose damage another test already reports.
    static void ImportAndPush(TrackedECDb& importer, std::vector<TrackedECDb*> const& pullers, Utf8StringCR xml, Utf8CP step, Writer writer = nullptr)
        {
        ASSERT_EQ(BE_SQLITE_OK, importer.PullMergePush(step));
        ASSERT_EQ(SchemaImportResult::OK, Import(importer, xml, step));
        if (writer != nullptr)
            writer(importer);
        ASSERT_EQ(BE_SQLITE_OK, importer.SaveChanges());
        ASSERT_EQ(BE_SQLITE_OK, importer.PullMergePush(step));
        for (TrackedECDb* puller : pullers)
            ASSERT_EQ(BE_SQLITE_OK, puller->PullMergePush(step));
        }

    //! Imports @p v1 with one instance of Item and one of Leaf, then @p v2.
    static void RunReferencingSchemas(Utf8StringCR v1, Utf8StringCR v2, Utf8CP insertItem, Utf8CP insertLeaf)
        {
        ECDbHub hub;
        auto b1 = hub.CreateBriefcase();
        auto b2 = hub.CreateBriefcase();
        ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, v1, "v1", {}, [&](ECDbR db) { Execute(db, insertItem); Execute(db, insertLeaf); }));
        ImportPushPull(*b1, {b2.get()}, v2, "v2");
        }

    //! Fails unless @p className declares @p propertyName in @p category, or in no category if @p category is null.
    static void ExpectCategory(ECDbR db, Utf8CP className, Utf8CP propertyName, Utf8CP category, Utf8CP context)
        {
        ECClassCP ecClass = db.Schemas().GetClass("TestSchema", className);
        ECPropertyCP prop = ecClass == nullptr ? nullptr : ecClass->GetPropertyP(propertyName, false);
        if (prop == nullptr)
            {
            ADD_FAILURE() << context << ": " << className << "." << propertyName << " is missing";
            return;
            }
        PropertyCategoryCP actual = prop->GetCategory();
        EXPECT_STREQ(category, actual == nullptr ? nullptr : actual->GetName().c_str()) << context << ": category of " << className << "." << propertyName;
        }
    };

//---------------------------------------------------------------------------------------
// A sync moves two properties to a new PropertyCategory, takes the category off a third and
// deletes the old category. The importer clears every reference to the old category before
// deleting it. A briefcase that applies the changeset must end up with the same schema: the
// ON DELETE CASCADE of ec_Property.CategoryId must not delete properties, with their maps,
// because the category row was deleted before the property rows were updated. A deleted
// property would also leave its custom attribute behind as an orphan.
// See https://github.com/iTwin/itwinjs-backlog/issues/2606
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyCategoryReassignedAndOldCategoryDeleted)
    {
    RunReferencingSchemas(
        ReferencingSchema(0, R"xml(<PropertyCategory typeName="OldCategory" priority="1"/>)xml",
            "Kept", "string", R"xml(category="OldCategory")xml", "Uncategorized", "string", R"xml(category="OldCategory")xml"),
        ReferencingSchema(1, R"xml(<PropertyCategory typeName="NewCategory" priority="2"/>)xml",
            "Kept", "string", R"xml(category="NewCategory")xml", "Uncategorized", "string", ""),
        "INSERT INTO ts.Item(Kept,Uncategorized,Other) VALUES('item kept','item uncategorized','item other')",
        "INSERT INTO ts.Leaf(Kept,Uncategorized,Other,LeafProp) VALUES('leaf kept','leaf uncategorized','leaf other','leaf prop')");
    }

//---------------------------------------------------------------------------------------
// The same as PropertyCategoryReassignedAndOldCategoryDeleted for a KindOfQuantity, which
// ec_Property.KindOfQuantityId references with ON DELETE CASCADE.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, KindOfQuantityReassignedAndOldKindOfQuantityDeleted)
    {
    RunReferencingSchemas(
        ReferencingSchema(0, R"xml(<KindOfQuantity typeName="OldKoq" persistenceUnit="u:M" relativeError="0.001"/>)xml",
            "Length", "double", R"xml(kindOfQuantity="OldKoq")xml", "Width", "double", R"xml(kindOfQuantity="OldKoq")xml", UnitsReference()),
        ReferencingSchema(1, R"xml(<KindOfQuantity typeName="NewKoq" persistenceUnit="u:M" relativeError="0.001"/>)xml",
            "Length", "double", R"xml(kindOfQuantity="NewKoq")xml", "Width", "double", R"xml(kindOfQuantity="NewKoq")xml", UnitsReference()),
        "INSERT INTO ts.Item(Length,Width,Other) VALUES(1.5,2.5,'item other')",
        "INSERT INTO ts.Leaf(Length,Width,Other,LeafProp) VALUES(3.5,4.5,'leaf other',5.5)");
    }

//---------------------------------------------------------------------------------------
// The same as PropertyCategoryReassignedAndOldCategoryDeleted for an ECEnumeration, which
// ec_Property.EnumerationId references with ON DELETE CASCADE. Two properties move to the
// new enumeration, one to the backing type.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, EnumerationReassignedAndOldEnumerationDeleted)
    {
    RunReferencingSchemas(
        ReferencingSchema(0, R"xml(<ECEnumeration typeName="OldEnum" backingTypeName="int" isStrict="false"><ECEnumerator name="One" value="1"/><ECEnumerator name="Two" value="2"/></ECEnumeration>)xml",
            "Kept", "OldEnum", "", "Plain", "OldEnum", ""),
        ReferencingSchema(1, R"xml(<ECEnumeration typeName="NewEnum" backingTypeName="int" isStrict="false"><ECEnumerator name="One" value="1"/><ECEnumerator name="Two" value="2"/></ECEnumeration>)xml",
            "Kept", "NewEnum", "", "Plain", "int", ""),
        "INSERT INTO ts.Item(Kept,Plain,Other) VALUES(1,2,'item other')",
        "INSERT INTO ts.Leaf(Kept,Plain,Other,LeafProp) VALUES(2,1,'leaf other',1)");
    }

//---------------------------------------------------------------------------------------
// A sync moves Tag, which two sibling classes declare, up to their base class. One sibling
// keeps an override. The sibling instances must keep their Tag values, which the import has
// to move to the column of the base class property, without duplicate maps. The next sync,
// imported by the briefcase that pulled the move, adds properties, which must not take a
// column that still holds values.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyMovedUpToBaseClass)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    plant.m_item = Prop("Note", "string") + Prop("Tag", "string");
    plant.m_pump = Prop("Flow", "double");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db)
        {
        InsertPlant(db, "v2");
        Execute(db, "INSERT INTO ts.Item(Guid,Note,Tag) VALUES('v2-tagged','v2 tagged note','v2-I')");
        }));

    plant.m_item += Prop("Area", "double");
    plant.m_valve += Prop("Rating", "double");
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db) { InsertPlant(db, "v3"); });
    }

//---------------------------------------------------------------------------------------
// A sync adds overrides of inherited properties to a leaf class, the next removes them and
// a third adds one back. Each override must replace the map of the property it overrides,
// in the same column, and removing it must restore the inherited map.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyOverrideAddedRemovedAndAddedAgain)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    const Utf8String stem = plant.m_gateValve;
    plant.m_gateValve = stem + Prop("Size", "double", R"xml(displayLabel="Gate size")xml") + Prop("Note", "string", R"xml(displayLabel="Gate note")xml");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db) { InsertPlant(db, "v2"); }));

    plant.m_gateValve = stem;
    ASSERT_TRUE(ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db) { InsertPlant(db, "v3"); }));

    plant.m_gateValve = stem + Prop("Size", "double", R"xml(displayLabel="Gate size again")xml");
    ImportPushPull(*b1, {b2.get()}, MakeSchema(3, plant.ToXml()), "v4", {}, [](ECDbR db) { InsertPlant(db, "v4"); });
    }

//---------------------------------------------------------------------------------------
// A sync deletes a property and adds another to the same class, and a later sync adds a
// property with the deleted one's name to the base class. Values of the deleted property may
// be lost, but must never be read through a property added since.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyDeletedThenAddedToBaseClass)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    plant.m_valve = Prop("Tag", "string") + Prop("Rating", "double");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {"Size"}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Valve(Guid,Note,Tag,Rating) VALUES('v2-valve','v2 valve note','v2-V',8.5)");
        Execute(db, "INSERT INTO ts.GateValve(Guid,Note,Tag,Rating,Stem) VALUES('v2-gate','v2 gate note','v2-G',9.5,'v2 stem')");
        }));

    plant.m_item = Prop("Note", "string") + Prop("Size", "double");
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db) { InsertPlant(db, "v3"); });
    }

//---------------------------------------------------------------------------------------
// Inserts a Pump of PropertyMapIntegrityTestFixture.StructPropertiesInOverflowTable, with a
// struct and a struct array.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static void InsertPumpWithDims(ECDbR db, Utf8CP label)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Pump(Guid,Note,Flow,Tag,D,Hist) VALUES(?,?,?,?,?,?)"));
    const Utf8PrintfString guid("%s-dims", label);
    const Utf8PrintfString note("%s dims note", label);
    stmt.BindText(1, guid.c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, note.c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindDouble(3, 7.25);
    stmt.BindText(4, guid.c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.GetBinder(5)["W"].BindDouble(1.5);
    stmt.GetBinder(5)["H"].BindDouble(2.5);
    for (double width : {3.5, 5.5})
        {
        IECSqlBinder& element = stmt.GetBinder(6).AddArrayElement();
        element["W"].BindDouble(width);
        element["H"].BindDouble(width + 1);
        }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << db.GetLastError().c_str();
    }

//---------------------------------------------------------------------------------------
// With only two shared columns before overflow, most properties, including a struct and a
// struct array, live in the overflow table. Syncs add properties to base classes, which
// existing instances need overflow rows for, and move the struct array up.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, StructPropertiesInOverflowTable)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    plant.m_extra = R"xml(<ECStructClass typeName="Dims"><ECProperty propertyName="W" typeName="double"/><ECProperty propertyName="H" typeName="double"/></ECStructClass>)xml";
    const Utf8String dims = R"xml(<ECStructProperty propertyName="D" typeName="Dims"/>)xml";
    const Utf8String hist = R"xml(<ECStructArrayProperty propertyName="Hist" typeName="Dims" minOccurs="0" maxOccurs="unbounded"/>)xml";
    plant.m_pump = Prop("Flow", "double") + Prop("Tag", "string") + dims + hist;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml(), 2), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); InsertPumpWithDims(db, "v1"); }));

    plant.m_plantBase += Prop("Code", "string");
    plant.m_item += Prop("Area", "double");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml(), 2), "v2", {}, [](ECDbR db) { InsertPlant(db, "v2"); InsertPumpWithDims(db, "v2"); }));

    plant.m_item += hist;
    plant.m_pump = Prop("Flow", "double") + Prop("Tag", "string") + dims;
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml(), 2), "v3", {}, [](ECDbR db) { InsertPlant(db, "v3"); InsertPumpWithDims(db, "v3"); });
    }

//---------------------------------------------------------------------------------------
// A sync applies a new mixin to a class that has instances and the next adds a property to
// the mixin. Instances must read the same values through the mixin as through their class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, MixinAddedToPopulatedClass)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    Utf8CP const isMixin = R"xml(<IsMixin xmlns="CoreCustomAttributes.01.00.03"><AppliesToEntityClass>Item</AppliesToEntityClass></IsMixin>)xml";
    plant.m_extra = Entity("ITagged", "", Prop("Code", "string"), true, isMixin);
    plant.m_valveBases = "Item,ITagged";
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db)
        {
        InsertPlant(db, "v2");
        Execute(db, "INSERT INTO ts.GateValve(Guid,Note,Size,Tag,Stem,Code) VALUES('v2-coded','v2 coded note',9.5,'v2-C','v2 coded stem','C-1')");
        }));

    plant.m_extra = Entity("ITagged", "", Prop("Code", "string") + Prop("Label", "string"), true, isMixin);
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db)
        {
        InsertPlant(db, "v3");
        Execute(db, "INSERT INTO ts.Valve(Guid,Note,Size,Tag,Code,Label) VALUES('v3-labelled','v3 labelled note',1.75,'v3-L','C-2','label')");
        });
    }

//---------------------------------------------------------------------------------------
// A sync moves Note from a class down to two of its derived classes. Deleting the base class
// property may lose its values, but the new properties must not read another property's
// values, and a property the next sync adds to the base class must not take their column.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyMovedDownToDerivedClasses)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    plant.m_item = "";
    plant.m_pump = Prop("Flow", "double") + Prop("Tag", "string") + Prop("Note", "string");
    plant.m_valve = Prop("Size", "double") + Prop("Tag", "string") + Prop("Note", "string");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {"Note"}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Pump(Guid,Flow,Tag,Note) VALUES('v2-pump',1.25,'v2-P','v2 pump note')");
        Execute(db, "INSERT INTO ts.GateValve(Guid,Size,Tag,Note,Stem) VALUES('v2-gate',2.25,'v2-G','v2 gate note','v2 stem')");
        }));

    plant.m_item = Prop("Remark", "string");
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Guid,Remark) VALUES('v3-item','v3 remark')");
        });
    }

//---------------------------------------------------------------------------------------
// PropertyMovedUpToBaseClass in a joined table without shared columns, where each property
// has a column of its own: a sync moves Rpm, which two sibling classes declare, up to their
// base class. One sibling overrides it, and the other inherited Name.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyMovedUpInDedicatedColumns)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    const auto schema = [](int minor, Utf8StringCR equipment, Utf8StringCR motor, Utf8StringCR fan)
        {
        return MakeSchema(minor, Entity("Equipment", "Root", equipment) + Entity("Motor", "Equipment", motor) + Entity("Fan", "Equipment", fan));
        };
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, schema(0, Prop("Name", "string"), Prop("Rpm", "int"), Prop("Rpm", "int")), "v1", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Equipment(Name) VALUES('equipment')");
        Execute(db, "INSERT INTO ts.Motor(Name,Rpm) VALUES('motor',1450)");
        Execute(db, "INSERT INTO ts.Fan(Name,Rpm) VALUES('fan',960)");
        }));

    ImportPushPull(*b1, {b2.get()}, schema(1, Prop("Name", "string") + Prop("Rpm", "int"),
        Prop("Rpm", "int", R"xml(displayLabel="Motor speed")xml") + Prop("Name", "string", R"xml(displayLabel="Motor name")xml"), ""), "v2", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Equipment(Name,Rpm) VALUES('v2 equipment',50)");
        Execute(db, "INSERT INTO ts.Fan(Name,Rpm) VALUES('v2 fan',720)");
        });
    }

//---------------------------------------------------------------------------------------
// Two sibling classes declare A and B in opposite column order, and a sync moves both up to
// their base class. However the base class maps them, the instances of one sibling may need
// their A and B values exchanged between columns. Each must still read its own values.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertiesMovedUpFromSwappedColumns)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    plant.m_pump = Prop("Flow", "double") + Prop("Tag", "string") + Prop("A", "string") + Prop("B", "string");
    plant.m_valve = Prop("Size", "double") + Prop("Tag", "string") + Prop("B", "string") + Prop("A", "string");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Pump(Guid,Note,Flow,Tag,A,B) VALUES('v1-pump','v1 pump note',12.5,'v1-P','pump a','pump b')");
        Execute(db, "INSERT INTO ts.Valve(Guid,Note,Size,Tag,A,B) VALUES('v1-valve','v1 valve note',4.25,'v1-V','valve a','valve b')");
        Execute(db, "INSERT INTO ts.GateValve(Guid,Note,Size,Tag,Stem,A,B) VALUES('v1-gate','v1 gate note',6.5,'v1-G','v1 stem','gate a','gate b')");
        }));

    plant.m_item = Prop("Note", "string") + Prop("A", "string") + Prop("B", "string");
    plant.m_pump = Prop("Flow", "double") + Prop("Tag", "string");
    plant.m_valve = Prop("Size", "double") + Prop("Tag", "string");
    ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Guid,Note,A,B) VALUES('v2-item','v2 item note','item a','item b')");
        });
    }

//---------------------------------------------------------------------------------------
// A sync inserts a new class between Item and its derived classes Pump and Valve, which
// declare Tag in different columns, and moves Tag up to it. The property of the new base
// class must be remapped throughout the existing derived classes, together with their values.
// The next sync, imported by the briefcase that pulled it, adds properties to both bases.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, IntermediateBaseClassInserted)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    plant.m_pump = Prop("Tag", "string") + Prop("Flow", "double");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    plant.m_extra = Entity("TaggedItem", "Item", Prop("Tag", "string"));
    plant.m_pumpBases = plant.m_valveBases = "TaggedItem";
    plant.m_pump = Prop("Flow", "double");
    plant.m_valve = Prop("Size", "double");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db)
        {
        InsertPlant(db, "v2");
        Execute(db, "INSERT INTO ts.TaggedItem(Guid,Note,Tag) VALUES('v2-tagged','v2 tagged note','v2-T')");
        }));

    plant.m_extra = Entity("TaggedItem", "Item", Prop("Tag", "string") + Prop("Area", "double"));
    plant.m_item += Prop("Remark", "string");
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db) { InsertPlant(db, "v3"); });
    }

//---------------------------------------------------------------------------------------
// The briefcase that pulled the category change of PropertyCategoryReassignedAndOldCategoryDeleted
// imports the next schema version, which adds a property, and pushes it. Both briefcases
// must end up with healthy, identical schemas that read every value.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, BriefcaseThatPulledCategoryDeletionImportsNextSchema)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Utf8CP const newCategory = R"xml(<PropertyCategory typeName="NewCategory" priority="2"/>)xml";
    Utf8CP const kept = R"xml(category="NewCategory")xml";
    ImportAndPush(*b1, {b2.get()}, ReferencingSchema(0, R"xml(<PropertyCategory typeName="OldCategory" priority="1"/>)xml",
        "Kept", "string", R"xml(category="OldCategory")xml", "Uncategorized", "string", R"xml(category="OldCategory")xml"), "v1", [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Kept,Uncategorized,Other) VALUES('item kept','item uncategorized','item other')");
        Execute(db, "INSERT INTO ts.Leaf(Kept,Uncategorized,Other,LeafProp) VALUES('leaf kept','leaf uncategorized','leaf other','leaf prop')");
        });
    ASSERT_FALSE(HasFatalFailure());
    ImportAndPush(*b1, {b2.get()}, ReferencingSchema(1, newCategory, "Kept", "string", kept, "Uncategorized", "string", ""), "v2");
    ASSERT_FALSE(HasFatalFailure());

    ImportPushPull(*b2, {b1.get()}, ReferencingSchema(2, newCategory, "Kept", "string", kept, "Uncategorized", "string", "", "", Prop("Added", "string").c_str()), "v3");
    }

//---------------------------------------------------------------------------------------
// BriefcaseThatPulledCategoryDeletionImportsNextSchema without the custom attribute whose
// orphan made the schema import fail: the briefcase that pulled the category change, where
// the cascade used to delete Kept and LeafProp, imports the next schema version, which
// declares them again, and pushes it. Both briefcases must end up with healthy, identical
// schemas that read every value.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, BriefcaseThatLostPropertiesToCascadeImportsNextSchema)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    const auto schema = [](int minor, Utf8CP category, Utf8StringCR added)
        {
        const Utf8PrintfString attributes(R"xml(category="%s")xml", category);
        return MakeSchema(minor, Utf8PrintfString(R"xml(<PropertyCategory typeName="%s" priority="1"/>)xml", category) +
            Entity("Item", "JoinedBase", Prop("Kept", "string", attributes.c_str()) + Prop("Other", "string") + added) +
            Entity("Leaf", "Item", Prop("LeafProp", "string", attributes.c_str())));
        };
    ImportAndPush(*b1, {b2.get()}, schema(0, "OldCategory", ""), "v1", [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Kept,Other) VALUES('item kept','item other')");
        Execute(db, "INSERT INTO ts.Leaf(Kept,Other,LeafProp) VALUES('leaf kept','leaf other','leaf prop')");
        });
    ASSERT_FALSE(HasFatalFailure());
    ImportAndPush(*b1, {b2.get()}, schema(1, "NewCategory", ""), "v2");
    ASSERT_FALSE(HasFatalFailure());

    ImportPushPull(*b2, {b1.get()}, schema(2, "NewCategory", Prop("Added", "string")), "v3");
    }

//---------------------------------------------------------------------------------------
// PropertyCategoryReassignedAndOldCategoryDeleted where the category change is a local txn
// that has to be rebased: b2 imports the change, and before b2 pushes it, b1 pushes an
// unrelated insert. b2 reverses its txn, merges the insert and reinstates its txn, which
// deletes the old category row again. The reinstated txn must keep Kept, Uncategorized and
// LeafProp, with their maps and the custom attribute of Kept, and b1 must get them from the
// pushed txn. Both briefcases must end up with healthy, identical schemas that read every value.
// See https://github.com/iTwin/itwinjs-backlog/issues/2606
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyCategoryReassignedInLocalTxnThatIsRebased)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    ImportAndPush(*b1, {b2.get()}, ReferencingSchema(0, R"xml(<PropertyCategory typeName="OldCategory" priority="1"/>)xml",
        "Kept", "string", R"xml(category="OldCategory")xml", "Uncategorized", "string", R"xml(category="OldCategory")xml"), "v1", [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Kept,Uncategorized,Other) VALUES('item kept','item uncategorized','item other')");
        Execute(db, "INSERT INTO ts.Leaf(Kept,Uncategorized,Other,LeafProp) VALUES('leaf kept','leaf uncategorized','leaf other','leaf prop')");
        });
    ASSERT_FALSE(HasFatalFailure());

    ASSERT_EQ(SchemaImportResult::OK, Import(*b2, ReferencingSchema(1, R"xml(<PropertyCategory typeName="NewCategory" priority="2"/>)xml",
        "Kept", "string", R"xml(category="NewCategory")xml", "Uncategorized", "string", ""), "v2, b2"));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    Execute(*b1, "INSERT INTO ts.Item(Kept,Uncategorized,Other) VALUES('b1 kept','b1 uncategorized','b1 other')");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    Sync(*b1, "insert, b1");
    const auto expected = TakeSnapshot(*b1, "insert, b1");

    Sync(*b2, "rebase, b2");
    Sync(*b1, "pull, b1");
    const auto verify = [&expected](ECDbR db, Utf8CP context)
        {
        ExpectHealthyPropertyMaps(db, context);
        ExpectSameValues(expected, TakeSnapshot(db, context), context);
        ExpectCategory(db, "Item", "Kept", "NewCategory", context);
        ExpectCategory(db, "Item", "Uncategorized", nullptr, context);
        ExpectCategory(db, "Leaf", "LeafProp", "NewCategory", context);
        ECSchemaCP schema = db.Schemas().GetSchema("TestSchema", true);
        ASSERT_NE(nullptr, schema) << context;
        EXPECT_EQ(nullptr, schema->GetPropertyCategoryCP("OldCategory")) << context << ": OldCategory is still there";
        ECClassCP item = schema->GetClassCP("Item");
        ECPropertyCP kept = item == nullptr ? nullptr : item->GetPropertyP("Kept", false);
        EXPECT_TRUE(kept != nullptr && kept->GetCustomAttribute("CoreCustomAttributes", "HiddenProperty").IsValid()) << context << ": Item.Kept lost its HiddenProperty custom attribute";
        };
    verify(*b2, "rebase, b2");
    verify(*b1, "pull, b1");
    SchemaSyncTestFixture::ExpectECTablesIdentical(*b1, *b2, "pull, b1");
    }

//---------------------------------------------------------------------------------------
// A sync inserts Mid, which declares Code, between Item and Pump. Pump inherits Code and
// must get a map of its own for it, as each derived class does when a property is added to
// its base class: ECDb finds the instances whose values to move from those maps when it
// remaps a property. The next sync, imported by the briefcase that pulled the first, moves
// Code up to Item, and every Pump must keep its Code value.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, BaseClassWithPropertyInsertedAbovePopulatedClass)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    plant.m_extra = Entity("Mid", "Item", Prop("Code", "string"));
    plant.m_pumpBases = "Mid";
    ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db)
        {
        InsertPlant(db, "v2");
        Execute(db, "INSERT INTO ts.Pump(Guid,Note,Flow,Tag,Code) VALUES('v2-coded','v2 coded note',2.5,'v2-C','pump code')");
        Execute(db, "INSERT INTO ts.Mid(Guid,Note,Code) VALUES('v2-mid','v2 mid note','mid code')");
        });

    plant.m_extra = Entity("Mid", "Item", "");
    plant.m_item = Prop("Note", "string") + Prop("Code", "string");
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml()), "v3", {}, [](ECDbR db) { InsertPlant(db, "v3"); });
    }

//---------------------------------------------------------------------------------------
// A sync adds Code to PlantBase and inserts Mid between PlantBase and Item. Item and each
// class derived from it inherit Code and must get a map of their own for it, as they do when
// either change is imported alone.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, BaseClassInsertedWhileAncestorGainsProperty)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml()), "v1", {}, [](ECDbR db) { InsertPlant(db, "v1"); }));

    plant.m_plantBase += Prop("Code", "string");
    plant.m_extra = Entity("Mid", "PlantBase", "", true);
    plant.m_itemBases = "Mid";
    ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml()), "v2", {}, [](ECDbR db) { InsertPlant(db, "v2"); });
    }

//---------------------------------------------------------------------------------------
// With two shared columns before overflow, Item and its sibling Pipe each keep a property in
// the second shared column of the joined table, and Tank, derived from Item, keeps its own
// properties in the overflow table. A sync moves Note up from Item to PlantBase, which can
// only map it in the overflow table. The import must move every Note value there, including
// those of the instances of Item, which have no overflow row yet.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, PropertyMovedUpIntoOverflowTable)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    const auto schema = [](int minor, Utf8StringCR plantBase, Utf8StringCR item)
        {
        return MakeSchema(minor, Entity("PlantBase", "JoinedBase", plantBase, true) + Entity("Item", "PlantBase", item) +
            Entity("Tank", "Item", Prop("Volume", "double") + Prop("Level", "double")) + Entity("Pipe", "PlantBase", Prop("Length", "double")), 2);
        };
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, schema(0, Prop("Guid", "string"), Prop("Note", "string")), "v1", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Guid,Note) VALUES('v1-item','v1 item note')");
        Execute(db, "INSERT INTO ts.Tank(Guid,Note,Volume,Level) VALUES('v1-tank','v1 tank note',30.5,2.25)");
        Execute(db, "INSERT INTO ts.Pipe(Guid,Length) VALUES('v1-pipe',12.5)");
        }));

    ImportPushPull(*b1, {b2.get()}, schema(1, Prop("Guid", "string") + Prop("Note", "string"), ""), "v2", {}, [](ECDbR db)
        {
        Execute(db, "INSERT INTO ts.Item(Guid,Note) VALUES('v2-item','v2 item note')");
        });
    }

//---------------------------------------------------------------------------------------
// With four shared columns before overflow, GateValve keeps Stem in the overflow table. A
// sync moves Stem up to Item, which frees that overflow column, and adds an override of Note
// to Valve. Like an override added alone, it must share the column of Item.Note: in a column
// of its own, Valve and GateValve instances read NULL for Note through Item, and Valve
// instances, which have no overflow row, lose it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, OverrideAddedWhilePropertyLeavesOverflowTable)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    plant.m_extra = Entity("Pipe", "PlantBase", Prop("Length", "double"));
    const auto insert = [](ECDbR db, Utf8CP label)
        {
        InsertPlant(db, label);
        Execute(db, Utf8PrintfString("INSERT INTO ts.Pipe(Guid,Length) VALUES('%s-pipe',4.5)", label).c_str());
        };
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml(), 4), "v1", {}, [&](ECDbR db) { insert(db, "v1"); }));

    plant.m_item = Prop("Note", "string") + Prop("Stem", "string");
    plant.m_valve += Prop("Note", "string", R"xml(displayLabel="Valve note")xml");
    plant.m_gateValve = "";
    ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml(), 4), "v2", {}, [&](ECDbR db) { insert(db, "v2"); });
    }

//---------------------------------------------------------------------------------------
// Inserts an instance of @p className, with Guid and the members of the struct D set.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static void InsertWithDims(ECDbR db, Utf8CP className, Utf8CP label, double width)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO ts.%s(Guid,D) VALUES(?,?)", className).c_str()));
    const Utf8PrintfString guid("%s-%s-dims", label, className);
    stmt.BindText(1, guid.c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.GetBinder(2)["W"].BindDouble(width);
    stmt.GetBinder(2)["H"].BindDouble(width + 1);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << db.GetLastError().c_str();
    }

//---------------------------------------------------------------------------------------
// OverrideAddedWhilePropertyLeavesOverflowTable for a struct. PlantBase keeps the members of
// D in shared columns of the joined table, and with four shared columns before overflow,
// Item.Note takes the last one. A sync moves Stem up from GateValve to Item, which frees its
// overflow column, and adds an override of D to Valve, which has no shared column left for
// a struct and puts it in the overflow table. Each member must still share the column of
// the same member of PlantBase.D: in the overflow table, Valve and GateValve instances read
// NULL for D through PlantBase.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, StructOverrideAddedWhilePropertyLeavesOverflowTable)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    plant.m_extra = R"xml(<ECStructClass typeName="Dims"><ECProperty propertyName="W" typeName="double"/><ECProperty propertyName="H" typeName="double"/></ECStructClass>)xml";
    plant.m_plantBase += R"xml(<ECStructProperty propertyName="D" typeName="Dims"/>)xml";
    const auto insert = [](ECDbR db, Utf8CP label)
        {
        InsertPlant(db, label);
        InsertWithDims(db, "Item", label, 1.5);
        InsertWithDims(db, "Valve", label, 3.5);
        InsertWithDims(db, "GateValve", label, 5.5);
        };
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml(), 4), "v1", {}, [&](ECDbR db) { insert(db, "v1"); }));

    plant.m_item = Prop("Note", "string") + Prop("Stem", "string");
    plant.m_valve += R"xml(<ECStructProperty propertyName="D" typeName="Dims" displayLabel="Valve dims"/>)xml";
    plant.m_gateValve = "";
    ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml(), 4), "v2", {}, [&](ECDbR db) { insert(db, "v2"); });
    }

//---------------------------------------------------------------------------------------
// With four shared columns before overflow, GateValve keeps Stem in the overflow table. A
// sync adds an override of Guid to Item, and the next deletes it again and moves Stem up to
// Valve, which frees that overflow column. Item and each class derived from it then inherit
// Guid again and must map it to the column of PlantBase.Guid, as they did before. Mapped to
// a column of their own, the whole subtree diverges from PlantBase: its instances read NULL
// for Guid, and the values remain only in a column no read uses.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, OverrideDeletedWhilePropertyLeavesOverflowTable)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    Plant plant;
    plant.m_extra = Entity("Pipe", "PlantBase", Prop("Length", "double"));
    const auto insert = [](ECDbR db, Utf8CP label)
        {
        InsertPlant(db, label);
        Execute(db, Utf8PrintfString("INSERT INTO ts.Pipe(Guid,Length) VALUES('%s-pipe',4.5)", label).c_str());
        };
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(0, plant.ToXml(), 4), "v1", {}, [&](ECDbR db) { insert(db, "v1"); }));

    plant.m_item = Prop("Guid", "string", R"xml(displayLabel="Item guid")xml") + Prop("Note", "string");
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, MakeSchema(1, plant.ToXml(), 4), "v2", {}, [&](ECDbR db) { insert(db, "v2"); }));

    plant.m_item = Prop("Note", "string");
    plant.m_valve += Prop("Stem", "string");
    plant.m_gateValve = "";
    ImportPushPull(*b2, {b1.get()}, MakeSchema(2, plant.ToXml(), 4), "v3", {}, [&](ECDbR db) { insert(db, "v3"); });
    }

//---------------------------------------------------------------------------------------
// With two shared columns before overflow, Pump inherits Guid from PlantBase, in the first
// shared column, and Remark from Item, in the overflow table. A sync adds an override of
// Guid to Pump, and the next deletes it again and adds an override of Remark. Pump must map
// Guid to the column of PlantBase.Guid and Remark to the column of Item.Remark, which hold
// its values. Mapped to columns of their own, they diverge from their base classes, and Pump
// instances read NULL for Guid.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, OverrideDeletedWhileOverflowPropertyOverridden)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    const auto schema = [](int minor, Utf8StringCR pump)
        {
        return MakeSchema(minor, Entity("PlantBase", "JoinedBase", Prop("Guid", "string"), true) +
            Entity("Item", "PlantBase", Prop("Note", "string") + Prop("Remark", "string")) + Entity("Pump", "Item", pump), 2);
        };
    const auto insert = [](ECDbR db, Utf8CP label)
        {
        Execute(db, Utf8PrintfString("INSERT INTO ts.Item(Guid,Note,Remark) VALUES('%s-item','%s item note','%s item remark')", label, label, label).c_str());
        Execute(db, Utf8PrintfString("INSERT INTO ts.Pump(Guid,Note,Remark,Flow) VALUES('%s-pump','%s pump note','%s pump remark',7.5)", label, label, label).c_str());
        };
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, schema(0, Prop("Flow", "double")), "v1", {}, [&](ECDbR db) { insert(db, "v1"); }));
    ASSERT_TRUE(ImportPushPull(*b1, {b2.get()}, schema(1, Prop("Guid", "string", R"xml(displayLabel="Pump guid")xml") + Prop("Flow", "double")), "v2", {},
        [&](ECDbR db) { insert(db, "v2"); }));

    ImportPushPull(*b2, {b1.get()}, schema(2, Prop("Remark", "string", R"xml(displayLabel="Pump remark")xml") + Prop("Flow", "double")), "v3", {},
        [&](ECDbR db) { insert(db, "v3"); });
    }

//=======================================================================================
// A plant-design dynamic schema that a seeded generator evolves with the changes connectors
// make between syncs, and the values every instance must read. The values are worked out
// without ECDb: an import keeps the value of every property an instance's class still has,
// except that deleting a property that overrides nothing may clear it for the whole subtree,
// even in derived classes that declare it, and a property new to a class reads NULL.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaEvolution final
    {
    using Fixture = PropertyMapIntegrityTestFixture;

    struct ClassDef
        {
        Utf8String m_name;
        Utf8String m_base; //!< empty for PlantBase, which derives from JoinedBase
        bool m_isAbstract = false;
        std::map<Utf8String, Utf8String> m_properties; //!< declared by the class: name -> type
        };

    struct Instance
        {
        Utf8String m_class;
        Fixture::Row m_row; //!< the values it must read; NULL members are absent
        Fixture::Names m_mayBecomeNull; //!< members the last import may have cleared
        };

    std::vector<ClassDef> m_classes;
    std::vector<std::pair<Utf8String, Utf8String>> m_deleted; //!< (name, type) of deleted properties, which a later sync may add again
    std::map<uint64_t, Instance> m_instances;
    uint32_t m_random;
    int m_maxSharedColumns;
    int m_nextName = 0;
    int m_nextValue = 0;

    explicit SchemaEvolution(uint32_t seed) : m_random(seed * 2654435761u + 1u)
        {
        if (m_random == 0)
            m_random = 1;
        m_maxSharedColumns = Pick(3) == 0 ? 4 : 32;
        if (Utf8CP maxShared = getenv("ECDB_PROPMAP_MAXSHARED"))
            m_maxSharedColumns = atoi(maxShared);
        Declare("PlantBase", "", true, {{"Guid", "string"}});
        Declare("Item", "PlantBase", false, {{"Note", "string"}});
        Declare("Pump", "Item", false, {{"Flow", "double"}, {"Tag", "string"}, {"Hist", "Dims[]"}});
        Declare("Valve", "Item", false, {{"Size", "double"}, {"Tag", "string"}});
        Declare("GateValve", "Valve", false, {{"Stem", "string"}});
        Declare("Pipe", "PlantBase", false, {{"Length", "double"}, {"Section", "Dims"}});
        }

    void Declare(Utf8CP name, Utf8CP base, bool isAbstract, std::map<Utf8String, Utf8String> const& properties)
        {
        ClassDef c;
        c.m_name = name;
        c.m_base = base;
        c.m_isAbstract = isAbstract;
        c.m_properties = properties;
        m_classes.push_back(c);
        }

    uint32_t Next()
        {
        m_random ^= m_random << 13;
        m_random ^= m_random >> 17;
        m_random ^= m_random << 5;
        return m_random;
        }
    size_t Pick(size_t count) { return Next() % count; }
    bool Chance(uint32_t percent) { return Next() % 100 < percent; }

    ClassDef* Find(Utf8StringCR name)
        {
        for (auto& c : m_classes)
            {
            if (c.m_name.Equals(name))
                return &c;
            }
        return nullptr;
        }
    ClassDef const* Find(Utf8StringCR name) const { return const_cast<SchemaEvolution*>(this)->Find(name); }

    //! True if @p name is @p ancestor or derives from it.
    bool IsA(Utf8StringCR name, Utf8StringCR ancestor) const
        {
        for (ClassDef const* c = Find(name); c != nullptr; c = c->m_base.empty() ? nullptr : Find(c->m_base))
            {
            if (c->m_name.Equals(ancestor))
                return true;
            }
        return false;
        }

    //! The properties @p name declares or inherits: name -> type.
    std::map<Utf8String, Utf8String> Properties(Utf8StringCR name) const
        {
        std::map<Utf8String, Utf8String> properties;
        for (ClassDef const* c = Find(name); c != nullptr; c = c->m_base.empty() ? nullptr : Find(c->m_base))
            properties.insert(c->m_properties.begin(), c->m_properties.end());
        return properties;
        }

    bool Inherits(Utf8StringCR name, Utf8StringCR property) const
        {
        ClassDef const* c = Find(name);
        return c != nullptr && !c->m_base.empty() && Properties(c->m_base).count(property) != 0;
        }

    //! True if no base or derived class of @p name declares @p property with a type other than @p type.
    bool CanDeclare(Utf8StringCR name, Utf8StringCR property, Utf8StringCR type) const
        {
        for (auto const& c : m_classes)
            {
            if (!IsA(c.m_name, name) && !IsA(name, c.m_name))
                continue;
            auto const found = c.m_properties.find(property);
            if (found != c.m_properties.end() && !found->second.Equals(type))
                return false;
            }
        return true;
        }

    std::vector<ClassDef*> Children(Utf8StringCR name)
        {
        std::vector<ClassDef*> children;
        for (auto& c : m_classes)
            {
            if (c.m_base.Equals(name))
                children.push_back(&c);
            }
        return children;
        }

    //! Every instance needs a property to insert it with.
    static bool IsPinned(ClassDef const& c, Utf8StringCR property) { return c.m_name.Equals("PlantBase") && property.Equals("Guid"); }

    Utf8String NewName(Utf8CP prefix) { return Utf8PrintfString("%s%d", prefix, ++m_nextName); }

    Utf8String NewType()
        {
        static Utf8CP const s_types[] = {"string", "string", "double", "int", "Dims", "Dims[]"};
        return s_types[Pick(6)];
        }

    //! A new property, or one that an earlier sync deleted, added to a class.
    Utf8String AddProperty()
        {
        ClassDef& c = m_classes[Pick(m_classes.size())];
        Utf8String name = NewName("P");
        Utf8String type = NewType();
        if (!m_deleted.empty() && Chance(50))
            {
            auto const deleted = m_deleted[Pick(m_deleted.size())];
            if (Properties(c.m_name).count(deleted.first) != 0 || !CanDeclare(c.m_name, deleted.first, deleted.second))
                return "";
            name = deleted.first;
            type = deleted.second;
            }
        c.m_properties[name] = type;
        return Utf8PrintfString("add %s.%s %s", c.m_name.c_str(), name.c_str(), type.c_str());
        }

    //! A property deleted from the class that declares it, which may override another.
    Utf8String DeleteProperty()
        {
        std::vector<std::pair<ClassDef*, Utf8String>> candidates;
        for (auto& c : m_classes)
            {
            for (auto const& p : c.m_properties)
                {
                if (!IsPinned(c, p.first))
                    candidates.push_back({&c, p.first});
                }
            }
        if (candidates.empty())
            return "";
        auto const chosen = candidates[Pick(candidates.size())];
        const bool isOverride = Inherits(chosen.first->m_name, chosen.second);
        if (!isOverride)
            m_deleted.push_back({chosen.second, chosen.first->m_properties[chosen.second]});
        chosen.first->m_properties.erase(chosen.second);
        return Utf8PrintfString("%s %s.%s", isOverride ? "delete override" : "delete", chosen.first->m_name.c_str(), chosen.second.c_str());
        }

    //! A property that derived classes declare, with one type, declared by their base class
    //! instead. Some derived classes may keep theirs as overrides.
    Utf8String MoveUp()
        {
        struct Candidate { Utf8String m_base, m_property, m_type; };
        std::vector<Candidate> candidates;
        for (auto const& base : m_classes)
            {
            std::map<Utf8String, Utf8String> below;
            Fixture::Names conflicting;
            for (auto const& c : m_classes)
                {
                if (c.m_name.Equals(base.m_name) || !IsA(c.m_name, base.m_name))
                    continue;
                for (auto const& p : c.m_properties)
                    {
                    auto const inserted = below.insert(p);
                    if (!inserted.second && !inserted.first->second.Equals(p.second))
                        conflicting.insert(p.first);
                    }
                }
            auto const visible = Properties(base.m_name);
            for (auto const& p : below)
                {
                if (conflicting.count(p.first) == 0 && visible.count(p.first) == 0)
                    candidates.push_back({base.m_name, p.first, p.second});
                }
            }
        if (candidates.empty())
            return "";
        Candidate const chosen = candidates[Pick(candidates.size())];
        Find(chosen.m_base)->m_properties[chosen.m_property] = chosen.m_type;
        Utf8String from, kept;
        for (auto& c : m_classes)
            {
            if (c.m_name.Equals(chosen.m_base) || !IsA(c.m_name, chosen.m_base) || c.m_properties.count(chosen.m_property) == 0)
                continue;
            if (Chance(80))
                {
                c.m_properties.erase(chosen.m_property);
                from.append(" ").append(c.m_name);
                }
            else
                kept.append(" ").append(c.m_name);
            }
        return Utf8PrintfString("move %s up to %s from%s%s%s", chosen.m_property.c_str(), chosen.m_base.c_str(), from.c_str(),
            kept.empty() ? "" : ", still overridden in", kept.c_str());
        }

    //! A property deleted from the class that declares it and declared by some of its derived
    //! classes instead.
    Utf8String MoveDown()
        {
        std::vector<std::pair<ClassDef*, Utf8String>> candidates;
        for (auto& c : m_classes)
            {
            if (Children(c.m_name).empty())
                continue;
            for (auto const& p : c.m_properties)
                {
                if (!IsPinned(c, p.first) && !Inherits(c.m_name, p.first))
                    candidates.push_back({&c, p.first});
                }
            }
        if (candidates.empty())
            return "";
        auto const chosen = candidates[Pick(candidates.size())];
        Utf8String const type = chosen.first->m_properties[chosen.second];
        chosen.first->m_properties.erase(chosen.second);
        auto children = Children(chosen.first->m_name);
        const size_t first = Pick(children.size());
        Utf8String to;
        for (size_t i = 0; i < children.size(); ++i)
            {
            if ((i == first || Chance(50)) && children[i]->m_properties.count(chosen.second) == 0)
                {
                children[i]->m_properties[chosen.second] = type;
                to.append(" ").append(children[i]->m_name);
                }
            }
        return Utf8PrintfString("move %s.%s down to%s", chosen.first->m_name.c_str(), chosen.second.c_str(), to.c_str());
        }

    //! An inherited property declared again by a derived class.
    Utf8String AddOverride()
        {
        std::vector<std::pair<ClassDef*, Utf8String>> candidates;
        for (auto& c : m_classes)
            {
            for (auto const& p : Properties(c.m_name))
                {
                if (c.m_properties.count(p.first) == 0)
                    candidates.push_back({&c, p.first});
                }
            }
        if (candidates.empty())
            return "";
        auto const chosen = candidates[Pick(candidates.size())];
        Utf8String const type = Properties(chosen.first->m_name)[chosen.second];
        chosen.first->m_properties[chosen.second] = type;
        return Utf8PrintfString("override %s.%s", chosen.first->m_name.c_str(), chosen.second.c_str());
        }

    Utf8String AddClass()
        {
        ClassDef c;
        c.m_name = NewName("C");
        c.m_base = m_classes[Pick(m_classes.size())].m_name;
        for (size_t count = Pick(3); count > 0; --count)
            c.m_properties[NewName("P")] = NewType();
        m_classes.push_back(c);
        return Utf8PrintfString("add class %s : %s", c.m_name.c_str(), c.m_base.c_str());
        }

    //! A new class between a class and some of its derived classes.
    Utf8String InsertBaseClass()
        {
        std::vector<Utf8String> parents;
        for (auto const& c : m_classes)
            {
            if (!Children(c.m_name).empty())
                parents.push_back(c.m_name);
            }
        if (parents.empty())
            return "";
        ClassDef inserted;
        inserted.m_name = NewName("M");
        inserted.m_base = parents[Pick(parents.size())];
        inserted.m_isAbstract = Chance(50);
        auto children = Children(inserted.m_base);
        const size_t first = Pick(children.size());
        Utf8String moved;
        for (size_t i = 0; i < children.size(); ++i)
            {
            if (i == first || Chance(50))
                {
                children[i]->m_base = inserted.m_name;
                moved.append(" ").append(children[i]->m_name);
                }
            }
        m_classes.push_back(inserted);
        return Utf8PrintfString("insert %s%s between %s and%s", inserted.m_isAbstract ? "abstract " : "", inserted.m_name.c_str(), inserted.m_base.c_str(), moved.c_str());
        }

    Utf8String Mutate()
        {
        for (int attempt = 0; attempt < 20; ++attempt)
            {
            Utf8String change;
            switch (Pick(10))
                {
                case 0: case 1: change = AddProperty(); break;
                case 2: case 3: change = DeleteProperty(); break;
                case 4: case 5: change = MoveUp(); break;
                case 6: change = MoveDown(); break;
                case 7: change = AddOverride(); break;
                case 8: change = AddClass(); break;
                default: change = getenv("ECDB_PROPMAP_NO_INSERT") != nullptr ? AddClass() : InsertBaseClass(); break;
                }
            if (!change.empty())
                return change;
            }
        return "";
        }

    static Utf8String PropertyXml(Utf8StringCR name, Utf8StringCR type)
        {
        if (type.Equals("Dims"))
            return Utf8PrintfString(R"xml(<ECStructProperty propertyName="%s" typeName="Dims"/>)xml", name.c_str());
        if (type.Equals("Dims[]"))
            return Utf8PrintfString(R"xml(<ECStructArrayProperty propertyName="%s" typeName="Dims" minOccurs="0" maxOccurs="unbounded"/>)xml", name.c_str());
        return Fixture::Prop(name.c_str(), type.c_str());
        }

    void AppendClasses(Utf8StringR xml, Utf8StringCR base) const
        {
        for (auto const& c : m_classes)
            {
            if (!c.m_base.Equals(base))
                continue;
            Utf8String members;
            for (auto const& p : c.m_properties)
                members.append(PropertyXml(p.first, p.second));
            xml.append(Fixture::Entity(c.m_name.c_str(), c.m_base.empty() ? "JoinedBase" : c.m_base.c_str(), members, c.m_isAbstract));
            AppendClasses(xml, c.m_name);
            }
        }

    Utf8String ToXml(int version) const
        {
        Utf8String body(R"xml(<ECStructClass typeName="Dims"><ECProperty propertyName="A" typeName="double"/><ECProperty propertyName="B" typeName="double"/></ECStructClass>)xml");
        AppendClasses(body, "");
        return Fixture::MakeSchema(version, body, m_maxSharedColumns);
        }

    //! Works out what each instance must read after an import changed @p before into the
    //! current classes. A class that no longer has a property it declared lost it: the import
    //! cleared it throughout the class's subtree, where derived classes may still declare it.
    void Imported(std::vector<ClassDef> const& before)
        {
        std::vector<std::pair<Utf8String, Utf8String>> cleared;
        for (auto const& old : before)
            {
            for (auto const& p : old.m_properties)
                {
                if (Properties(old.m_name).count(p.first) == 0)
                    cleared.push_back({old.m_name, p.first});
                }
            }
        for (auto& entry : m_instances)
            {
            Instance& instance = entry.second;
            auto const properties = Properties(instance.m_class);
            for (auto it = instance.m_row.begin(); it != instance.m_row.end();)
                it = properties.count(it->first) == 0 ? instance.m_row.erase(it) : std::next(it);
            for (auto const& c : cleared)
                {
                if (IsA(instance.m_class, c.first) && properties.count(c.second) != 0)
                    instance.m_mayBecomeNull.insert(c.second);
                }
            }
        }

    //! Each instance must read what the model says through the class it was inserted as, and
    //! no class may have any other instance.
    void ExpectInstances(ECDbR db, Utf8CP context)
        {
        std::map<Utf8String, Fixture::Rows> rowsByClass;
        for (auto const& c : m_classes)
            {
            if (!c.m_isAbstract)
                Fixture::ReadRows(rowsByClass[c.m_name], db, Utf8PrintfString("SELECT * FROM ONLY ts.%s", c.m_name.c_str()), context);
            }
        for (auto& entry : m_instances)
            {
            Instance& instance = entry.second;
            auto& rows = rowsByClass[instance.m_class];
            auto const found = rows.find(entry.first);
            if (found == rows.end())
                {
                ADD_FAILURE() << context << ": " << instance.m_class.c_str() << " " << entry.first << " is gone";
                continue;
                }
            Fixture::Row actual = found->second;
            actual.erase("className");
            Fixture::ExpectSameRow(instance.m_row, actual, instance.m_mayBecomeNull, context, instance.m_class, entry.first);
            instance.m_row = actual;
            instance.m_mayBecomeNull.clear();
            rows.erase(found);
            }
        for (auto const& entry : rowsByClass)
            {
            for (auto const& row : entry.second)
                ADD_FAILURE() << context << ": " << entry.first.c_str() << " " << row.first << " appeared";
            }
        }

    //! Binds a new value of @p type and returns the JSON that reading it must give.
    Utf8String BindValue(IECSqlBinder& binder, Utf8StringCR type)
        {
        const int v = ++m_nextValue;
        Utf8String json;
        if (type.Equals("string"))
            {
            Utf8PrintfString text("s%d", v);
            binder.BindText(text.c_str(), IECSqlBinder::MakeCopy::Yes);
            json.Sprintf("\"%s\"", text.c_str());
            }
        else if (type.Equals("double"))
            {
            binder.BindDouble(v + 0.5);
            json.Sprintf("%d.5", v);
            }
        else if (type.Equals("int"))
            {
            binder.BindInt(v);
            json.Sprintf("%d", v);
            }
        else if (type.Equals("Dims"))
            {
            binder["A"].BindDouble(v + 0.25);
            binder["B"].BindDouble(v + 0.75);
            json.Sprintf(R"({"A":%d.25,"B":%d.75})", v, v);
            }
        else
            {
            for (int i = 0; i < 2; ++i)
                {
                IECSqlBinder& element = binder.AddArrayElement();
                element["A"].BindDouble(v + i + 0.25);
                element["B"].BindDouble(v + i + 0.75);
                }
            json.Sprintf(R"([{"A":%d.25,"B":%d.75},{"A":%d.25,"B":%d.75}])", v, v, v + 1, v + 1);
            }
        return BeJsDocument(json.c_str()).Stringify();
        }

    void ExpectWritten(ECDbR db, uint64_t id, Utf8CP context)
        {
        Instance const& instance = m_instances[id];
        Fixture::Rows rows;
        Fixture::ReadRows(rows, db, Utf8PrintfString("SELECT * FROM ONLY ts.%s WHERE ECInstanceId=%" PRIu64, instance.m_class.c_str(), id), context);
        auto const found = rows.find(id);
        if (found == rows.end())
            {
            ADD_FAILURE() << context << ": cannot read the " << instance.m_class.c_str() << " just written, " << id;
            return;
            }
        Fixture::Row actual = found->second;
        actual.erase("className");
        Fixture::ExpectSameRow(instance.m_row, actual, {}, context, instance.m_class, id);
        }

    void Insert(ECDbR db, Utf8StringCR className, Utf8CP context)
        {
        auto const properties = Properties(className);
        Utf8String names, values;
        for (auto const& p : properties)
            {
            names.append(names.empty() ? "[" : ",[").append(p.first).append("]");
            values.append(values.empty() ? "?" : ",?");
            }
        Utf8PrintfString ecsql("INSERT INTO ts.%s(%s) VALUES(%s)", className.c_str(), names.c_str(), values.c_str());
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(db, ecsql.c_str()))
            {
            ADD_FAILURE() << context << ": cannot prepare " << ecsql.c_str();
            return;
            }
        Instance instance;
        instance.m_class = className;
        int index = 0;
        for (auto const& p : properties)
            instance.m_row[p.first] = BindValue(stmt.GetBinder(++index), p.second);
        ECInstanceKey key;
        if (BE_SQLITE_DONE != stmt.Step(key))
            {
            ADD_FAILURE() << context << ": " << ecsql.c_str() << " failed: " << db.GetLastError().c_str();
            return;
            }
        m_instances[key.GetInstanceId().GetValue()] = instance;
        ExpectWritten(db, key.GetInstanceId().GetValue(), context);
        }

    void Update(ECDbR db, uint64_t id, Utf8CP context)
        {
        Instance& instance = m_instances[id];
        auto const properties = Properties(instance.m_class);
        auto property = properties.begin();
        std::advance(property, Pick(properties.size()));
        Utf8PrintfString ecsql("UPDATE ONLY ts.%s SET [%s]=? WHERE ECInstanceId=%" PRIu64, instance.m_class.c_str(), property->first.c_str(), id);
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(db, ecsql.c_str()))
            {
            ADD_FAILURE() << context << ": cannot prepare " << ecsql.c_str();
            return;
            }
        Utf8String const value = BindValue(stmt.GetBinder(1), property->second);
        if (BE_SQLITE_DONE != stmt.Step())
            {
            ADD_FAILURE() << context << ": " << ecsql.c_str() << " failed: " << db.GetLastError().c_str();
            return;
            }
        instance.m_row[property->first] = value;
        ExpectWritten(db, id, context);
        }

    //! Inserts instances, at least one of each concrete class, and updates a few properties.
    void Write(ECDbR db, Utf8CP context)
        {
        Fixture::Names populated;
        for (auto const& entry : m_instances)
            populated.insert(entry.second.m_class);
        Fixture::Names toInsert;
        for (auto const& c : m_classes)
            {
            if (!c.m_isAbstract && (populated.count(c.m_name) == 0 || Chance(40)))
                toInsert.insert(c.m_name);
            }
        for (auto const& className : toInsert)
            Insert(db, className, context);
        for (int i = 0; i < 3 && !m_instances.empty(); ++i)
            {
            auto instance = m_instances.begin();
            std::advance(instance, Pick(m_instances.size()));
            Update(db, instance->first, context);
            }
        }
    };

static int FailureCount() { return ::testing::UnitTest::GetInstance()->current_test_info()->result()->total_part_count(); }


//---------------------------------------------------------------------------------------
// Random sequences of connector syncs. Two briefcases take turns to hold the schema lock:
// each pulls, imports the next version of the dynamic schema, writes instances and pushes,
// and the other pulls and may write instances too. Each version differs from the one before
// by one to three changes connectors make between syncs: properties added, deleted, moved up
// to a base class or down to derived classes, overrides added or deleted, classes added or
// inserted between a class and its derived classes. The importer and the puller must pass
// ExpectHealthyPropertyMaps and read exactly the values SchemaEvolution works out.
// Environment variables narrow the search: ECDB_PROPMAP_SEEDS=first[-last] and
// ECDB_PROPMAP_STEPS choose the sequences, ECDB_PROPMAP_MAXSHARED sets the shared columns
// before overflow, ECDB_PROPMAP_NO_INSERT stops classes being inserted between others, and
// ECDB_PROPMAP_VERBOSE also prints the changes of the sequences that pass.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyMapIntegrityTestFixture, RandomSchemaEvolution)
    {
    unsigned firstSeed = 1, lastSeed = 30;
    int steps = 10;
    if (Utf8CP seeds = getenv("ECDB_PROPMAP_SEEDS"))
        {
        const int count = sscanf(seeds, "%u-%u", &firstSeed, &lastSeed);
        if (count == 1)
            lastSeed = firstSeed;
        }
    if (Utf8CP stepCount = getenv("ECDB_PROPMAP_STEPS"))
        steps = atoi(stepCount);

    int imported = 0, rejected = 0, failedSeeds = 0;
    for (unsigned seed = firstSeed; seed <= lastSeed; ++seed)
        {
        ECDbHub hub;
        auto b1 = hub.CreateBriefcase();
        auto b2 = hub.CreateBriefcase();
        TrackedECDb* const briefcases[] = {b1.get(), b2.get()};
        SchemaEvolution model(seed);
        Utf8PrintfString log("max shared columns %d", model.m_maxSharedColumns);
        const int failuresBefore = FailureCount();
        for (int step = 0; step < steps && FailureCount() == failuresBefore; ++step)
            {
            const int importerIndex = step == 0 ? 0 : static_cast<int>(model.Pick(2));
            TrackedECDb& importer = *briefcases[importerIndex];
            TrackedECDb& other = *briefcases[1 - importerIndex];
            const Utf8PrintfString context("seed %u step %d, b%d imports", seed, step, importerIndex + 1);
            Sync(importer, context.c_str());
            if (step > 0)
                model.ExpectInstances(importer, context.c_str());

            auto const classesBefore = model.m_classes;
            auto const deletedBefore = model.m_deleted;
            Utf8String changes;
            for (size_t count = step == 0 ? 0 : 1 + model.Pick(3); count > 0; --count)
                {
                Utf8String const change = model.Mutate();
                if (!change.empty())
                    changes.append(changes.empty() ? "" : "; ").append(change);
                }
            log.append(Utf8PrintfString("\n  step %d, b%d imports: %s", step, importerIndex + 1, step == 0 ? "the first version" : changes.c_str()));
            if (SchemaImportResult::OK != Import(importer, model.ToXml(step), context.c_str()))
                {
                ASSERT_NE(0, step) << "the first version must import";
                log.append(" => rejected");
                ++rejected;
                model.m_classes = classesBefore;
                model.m_deleted = deletedBefore;
                continue;
                }
            ++imported;
            EXPECT_EQ(BE_SQLITE_OK, importer.SaveChanges()) << context.c_str();
            model.Imported(classesBefore);
            model.ExpectInstances(importer, context.c_str());
            ExpectHealthyPropertyMaps(importer, context.c_str());
            model.Write(importer, context.c_str());
            EXPECT_EQ(BE_SQLITE_OK, importer.SaveChanges()) << context.c_str();
            Sync(importer, context.c_str());
            auto const expected = TakeSnapshot(importer, context.c_str());

            const Utf8PrintfString pullContext("seed %u step %d, b%d pulls", seed, step, 2 - importerIndex);
            Sync(other, pullContext.c_str());
            ExpectHealthyPropertyMaps(other, pullContext.c_str());
            SchemaSyncTestFixture::ExpectECTablesIdentical(other, importer, pullContext.c_str());
            ExpectSameValues(expected, TakeSnapshot(other, pullContext.c_str()), pullContext.c_str());
            model.ExpectInstances(other, pullContext.c_str());
            if (model.Chance(50))
                {
                model.Write(other, pullContext.c_str());
                EXPECT_EQ(BE_SQLITE_OK, other.SaveChanges()) << pullContext.c_str();
                Sync(other, pullContext.c_str());
                }
            }
        if (FailureCount() != failuresBefore)
            {
            ++failedSeeds;
            printf("[RandomSchemaEvolution] seed %u failed, %s\n", seed, log.c_str());
            }
        else if (getenv("ECDB_PROPMAP_VERBOSE") != nullptr)
            printf("[RandomSchemaEvolution] seed %u passed, %s\n", seed, log.c_str());
        }
    printf("[RandomSchemaEvolution] seeds %u-%u: %d imports, %d rejected, %d seeds failed\n", firstSeed, lastSeed, imported, rejected, failedSeeds);
    }

END_ECDBUNITTESTS_NAMESPACE
