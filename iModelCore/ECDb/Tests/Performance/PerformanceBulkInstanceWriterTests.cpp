/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <ECDb/BulkInstanceWriter.h>
#include <ECDb/InstanceWriter.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Compares BulkInstanceWriter against InstanceWriter for full and partial updates of an
//! instance of a three level (Base -> Mid -> Leaf) TablePerHierarchy class.
//!
//! full    : every property of the instance is supplied and written
//! partial : only the properties declared by the leaf class are supplied. For
//!           BulkInstanceWriter that is its native partial (hierarchy level granular) mode,
//!           for InstanceWriter it is UseIncrementalUpdate, which reads the instance back
//!           through InstanceReader to refill the properties the caller left out.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PerformanceBulkInstanceWriter : ECDbTestFixture {
    static const int s_propsPerLevel = 10;

    std::vector<ECInstanceKey> m_keys;
    ECClassId m_leafClassId;

    static Utf8String PropName(Utf8CP levelPrefix, int i) {
        Utf8String name;
        name.Sprintf("%s%d", levelPrefix, i);
        return name;
    }

    //! 0-3 int, 4-6 double, 7-9 string
    static Utf8CP PropType(int i) { return i < 4 ? "int" : (i < 7 ? "double" : "string"); }

    static Utf8String LevelProperties(Utf8CP levelPrefix) {
        Utf8String xml;
        for (int i = 0; i < s_propsPerLevel; ++i) {
            Utf8String prop;
            prop.Sprintf("<ECProperty propertyName=\"%s\" typeName=\"%s\"/>", PropName(levelPrefix, i).c_str(), PropType(i));
            xml.append(prop);
        }
        return xml;
    }

    static Utf8String SchemaXml() {
        Utf8String xml;
        xml.Sprintf(R"xml(<ECSchema schemaName="PerfSchema" alias="ps" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    %s
                </ECEntityClass>
                <ECEntityClass typeName="Mid">
                    <BaseClass>Base</BaseClass>
                    %s
                </ECEntityClass>
                <ECEntityClass typeName="Leaf">
                    <BaseClass>Mid</BaseClass>
                    %s
                </ECEntityClass>
            </ECSchema>)xml",
            LevelProperties("b").c_str(), LevelProperties("m").c_str(), LevelProperties("l").c_str());
        return xml;
    }

    //! Binds the properties of the given levels. Values depend on the seed so that every pass
    //! writes different values and SQLite cannot short circuit anything.
    static void BindLevels(BulkInstanceWriter::IBindContext const& ctx, int seed, bool baseAndMid) {
        const auto bindLevel = [&](Utf8CP prefix) {
            for (int i = 0; i < s_propsPerLevel; ++i) {
                auto binder = ctx.FindBinder(PropName(prefix, i).c_str());
                BeAssert(binder != nullptr);
                if (i < 4)
                    binder->BindInt(seed + i);
                else if (i < 7)
                    binder->BindDouble(seed + i + 0.5);
                else {
                    Utf8String s;
                    s.Sprintf("%s%d-%d", prefix, i, seed);
                    binder->BindText(s.c_str(), IECSqlBinder::MakeCopy::Yes);
                }
            }
        };

        if (baseAndMid) {
            bindLevel("b");
            bindLevel("m");
        }
        bindLevel("l");
    }

    //! Fills a JSON instance (ECSql names) with the properties of the given levels.
    static void FillJson(BeJsValue json, int seed, bool baseAndMid) {
        const auto fillLevel = [&](Utf8CP prefix) {
            for (int i = 0; i < s_propsPerLevel; ++i) {
                const auto name = PropName(prefix, i);
                if (i < 4)
                    json[name.c_str()] = seed + i;
                else if (i < 7)
                    json[name.c_str()] = seed + i + 0.5;
                else {
                    Utf8String s;
                    s.Sprintf("%s%d-%d", prefix, i, seed);
                    json[name.c_str()] = s;
                }
            }
        };

        if (baseAndMid) {
            fillLevel("b");
            fillLevel("m");
        }
        fillLevel("l");
    }

    void SetupInstances(int instanceCount) {
        ASSERT_EQ(SUCCESS, SetupECDb("perf_bulkinstancewriter.ecdb", SchemaItem(SchemaXml())));
        auto leafClass = m_ecdb.Schemas().GetClass("PerfSchema", "Leaf");
        ASSERT_TRUE(leafClass != nullptr);
        m_leafClassId = leafClass->GetId();

        BulkInstanceWriter writer(m_ecdb);
        m_keys.reserve(instanceCount);
        for (int i = 0; i < instanceCount; ++i) {
            ECInstanceKey key;
            ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(m_leafClassId, [&](BulkInstanceWriter::IBindContext const& ctx) {
                BindLevels(ctx, i, true);
            }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();
            m_keys.push_back(key);
        }
        m_ecdb.SaveChanges();
    }

    double RunBulk(bool fullUpdate, int passSeed, bool timeIt) {
        BulkInstanceWriter writer(m_ecdb);
        BulkInstanceWriter::UpdateOptions options;
        if (fullUpdate)
            options.UseFullUpdate();
        else
            options.UsePartialUpdate();

        StopWatch timer(true);
        int i = 0;
        for (auto const& key : m_keys) {
            const auto seed = passSeed + i++;
            const auto r = writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
                BindLevels(ctx, seed, fullUpdate);
            }, options);
            BeAssert(r == BE_SQLITE_DONE);
            UNUSED_VARIABLE(r);
        }
        timer.Stop();
        m_ecdb.SaveChanges();
        return timeIt ? timer.GetElapsedSeconds() : 0.0;
    }

    double RunInstanceWriter(bool fullUpdate, int passSeed, bool timeIt) {
        InstanceWriter writer(m_ecdb);
        InstanceWriter::UpdateOptions options;
        if (!fullUpdate)
            options.UseIncrementalUpdate(true);

        StopWatch timer(true);
        int i = 0;
        for (auto const& key : m_keys) {
            const auto seed = passSeed + i++;
            BeJsDocument json;
            json["ECInstanceId"] = key.GetInstanceId().ToHexStr();
            json["ECClassId"] = key.GetClassId().ToHexStr();
            FillJson(json, seed, fullUpdate);
            const auto r = writer.Update(json, options);
            BeAssert(r == BE_SQLITE_DONE);
            UNUSED_VARIABLE(r);
        }
        timer.Stop();
        m_ecdb.SaveChanges();
        return timeIt ? timer.GetElapsedSeconds() : 0.0;
    }

    void Report(Utf8CP what, double seconds, int opCount) {
        printf("%-42s %10.2f ms total %10.4f ms/op %12.0f ops/s\n", what, seconds * 1000.0,
            seconds * 1000.0 / opCount, opCount / seconds);
        LOG.infov("%s: %d ops took %.4f msecs", what, opCount, seconds * 1000.0);
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBulkInstanceWriter, FullAndPartialUpdate) {
    const int instanceCount = 20000;
    SetupInstances(instanceCount);

    // warm the page cache and the writers' statement caches for both APIs
    RunBulk(true, 1000000, false);
    RunBulk(false, 1100000, false);
    RunInstanceWriter(true, 1200000, false);
    RunInstanceWriter(false, 1300000, false);

    const auto bulkFull = RunBulk(true, 1, true);
    const auto bulkPartial = RunBulk(false, 2, true);
    const auto instFull = RunInstanceWriter(true, 3, true);
    const auto instPartial = RunInstanceWriter(false, 4, true);

    printf("\nUpdate of %d instances of a 3 level TablePerHierarchy class (%d properties per level)\n\n",
        instanceCount, s_propsPerLevel);
    Report("BulkInstanceWriter  full   (30 props)", bulkFull, instanceCount);
    Report("InstanceWriter      full   (30 props)", instFull, instanceCount);
    Report("BulkInstanceWriter  partial(10 props)", bulkPartial, instanceCount);
    Report("InstanceWriter      partial(10 props)", instPartial, instanceCount);
    printf("\nspeedup full    : %.2fx\n", instFull / bulkFull);
    printf("speedup partial : %.2fx\n\n", instPartial / bulkPartial);

    LOGTODB(TEST_DETAILS, bulkFull, instanceCount, "BulkInstanceWriter full update");
    LOGTODB(TEST_DETAILS, instFull, instanceCount, "InstanceWriter full update");
    LOGTODB(TEST_DETAILS, bulkPartial, instanceCount, "BulkInstanceWriter partial update");
    LOGTODB(TEST_DETAILS, instPartial, instanceCount, "InstanceWriter partial update");
}

END_ECDBUNITTESTS_NAMESPACE
