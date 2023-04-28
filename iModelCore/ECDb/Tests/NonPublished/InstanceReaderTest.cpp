/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <sstream>
#include <rapidjson/ostreamwrapper.h>
#include <random>
#include <filesystem>
USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct InstanceReaderFixture : ECDbTestFixture {

    DbResult OpenECDbTestDataFile(Utf8CP name) {
        auto getDataPath = []() {
            BeFileName docRoot;
            BeTest::GetHost().GetDocumentsRoot(docRoot);
            docRoot.AppendToPath(L"ECDb");
            return docRoot;
        };

        const auto bimPath = getDataPath().AppendToPath(WString(name, true).c_str());
        if (m_ecdb.IsDbOpen()) {
            m_ecdb.CloseDb();
        }
        return m_ecdb.OpenBeSQLiteDb(bimPath, Db::OpenParams(Db::OpenMode::Readonly));
    }

};

struct InstancePropPerfTest {
    enum class PropFilter {
        None,
        AnyPropExists,
        AllPropExists,
        AnyPropIsNotNull,
        AllPropAreNotNull
    };
    constexpr static auto JInTestName = "testName";
    constexpr static auto JInRootClassName = "rootClassName";
    constexpr static auto JInMaxRows = "maxRows";
    constexpr static auto JInMaxProps = "maxProps";
    constexpr static auto JInIncludeDeriveProps = "includeDeriveProps";
    constexpr static auto JInShuffleProps = "shuffleProps";
    constexpr static auto JInFilter = "addPropExistsFilter";
    constexpr static auto JOutRowCount = "rowCount";
    constexpr static auto JOutElapsedTime = "elapsedSeconds";
    constexpr static auto JOutQuery = "query";
    constexpr static auto JOutRootSubject = "rootSubject";
    constexpr static auto JOutFileSize = "fileSizeBytes";
    constexpr static auto JOutIModelId = "iModelId";

    constexpr static auto JInFilterAnyPropertyExists = "filter-any-prop-exists";
    constexpr static auto JInFilterAllPropertiesExists = "filter-all-prop-exists";
    constexpr static auto JInFilterAnyPropertyIsNotNull = "filter-any-prop-is-not-null";
    constexpr static auto JInFilterAllPropertiesAreNotNull = "filter-all-prop-are-not-null";
    constexpr static auto JInFilterNone = "filter-none";

    constexpr static auto JTestQueryInstanceWithNoFilter = "test-query-instance-with-no-filter";
    constexpr static auto JTestQueryInstanceProperties = "test-query-instance-properties";
    using TestFunc = std::function<void(BeJsValue, std::function<void(BeJsValue)>, std::function<void(BeJsValue)>)>;
    struct CompareIUtf8Ascii {
        bool operator()(Utf8StringCR s1, Utf8StringCR s2) const { return BeStringUtilities::StricmpAscii(s1.c_str(), s2.c_str()) < 0; } };
    private:
        ECDb m_conn;
        std::default_random_engine m_randEngine;
        std::map<Utf8String, TestFunc, CompareIUtf8Ascii> m_testFuncs;
        BentleyStatus GetClassId(Utf8StringCR className, std::vector<ECN::ECClassId>& ids) const {
            ECSqlStatement stmt;
            if (stmt.Prepare(m_conn,
                SqlPrintfString("SELECT ECClassId, COUNT(*) FROM %s GROUP BY ECClassId", className.c_str())) != ECSqlStatus::Success){
                return ERROR;
            }
            while(stmt.Step() == BE_SQLITE_ROW) {
                ids.push_back(stmt.GetValueId<ECClassId>(0));
            }
            return SUCCESS;
        }
        static PropFilter FromPropFilter(Utf8StringCR str) {
            if (str.EqualsIAscii(JInFilterAnyPropertyExists))
                return PropFilter::AnyPropExists;
            if (str.EqualsIAscii(JInFilterAllPropertiesExists))
                return PropFilter::AllPropExists;
            if (str.EqualsIAscii(JInFilterAnyPropertyIsNotNull))
                return PropFilter::AnyPropIsNotNull;
            if (str.EqualsIAscii(JInFilterAllPropertiesAreNotNull))
                return PropFilter::AllPropAreNotNull;
            return PropFilter::None;
        }
        BentleyStatus GetProps(ECClassId classId, std::vector<std::string>& props ) const {
            Statement stmt;
            if (BE_SQLITE_OK != stmt.Prepare(m_conn, R"x(
                SELECT DISTINCT
                    IIF (
                        INSTR ([pp].[AccessString], '.') == 0,
                        [pp].[AccessString],
                        SUBSTR ([pp].[AccessString], 0,
                                INSTR ([pp].[AccessString], '.'))) [AccessString]
                    FROM   [ec_propertyMap] [pm]
                        JOIN [ec_PropertyPath] [pp] ON [pp].[Id] = [pm].[PropertyPathId]
                        JOIN [ec_ClassMap] [cm] ON [cm].[ClassId] = [pm].[ClassId]
                    WHERE  [cm].[MapStrategy] != 0 AND cm.ClassId = ?
                    ORDER  BY
                            [pm].[PropertyPathId],
                            [pm].[classid];
                )x")) {
                return ERROR;
                }
            stmt.BindId(1, classId);
            while(stmt.Step() == BE_SQLITE_ROW) {
                props.push_back(stmt.GetValueText(0));
            }
            return SUCCESS;
        }
        BentleyStatus GetDerivedProps(ECClassId classId, std::vector<std::string>& props ) const {
            Statement stmt;
            if (BE_SQLITE_OK != stmt.Prepare(m_conn, R"x(
                SELECT DISTINCT
                IIF (
                    INSTR ([pp].[AccessString], '.') == 0,
                    [pp].[AccessString],
                    SUBSTR ([pp].[AccessString], 0,
                            INSTR ([pp].[AccessString], '.'))) [AccessString]
                FROM   [ec_propertyMap] [pm]
                    JOIN [ec_PropertyPath] [pp] ON [pp].[Id] = [pm].[PropertyPathId]
                    JOIN [ec_ClassMap] [cm] ON [cm].[ClassId] = [pm].[ClassId]
                    JOIN ec_cache_ClassHierarchy ch on ch.ClassId = pm.ClassId
                WHERE  [cm].[MapStrategy] != 0 AND ch.BaseClassId=?
                )x")) {
                return ERROR;
            }
            stmt.BindId(1, classId);
            while(stmt.Step() == BE_SQLITE_ROW) {
                props.push_back(stmt.GetValueText(0));
            }
            return SUCCESS;
        }
        BentleyStatus ReadInstances (BeJsValue param, std::function<void(BeJsValue)> onStart, std::function<void(BeJsValue)> onFinish) {
            if (!param[JInTestName].asString().EqualsIAscii(JTestQueryInstanceWithNoFilter)) {
                return ERROR;
            }
            Utf8String className = param[JInRootClassName].asString();
            int64_t maxRows = param[JInMaxRows].asInt64();
            Utf8String ecsql = SqlPrintfString("SELECT $ FROM %s", className.c_str()).GetUtf8CP();
            ECSqlStatement stmt;
            if (stmt.Prepare(m_conn, ecsql.c_str()) != ECSqlStatus::Success) {
                return ERROR;
            }
            if (maxRows <= 0) {
                maxRows = std::numeric_limits<int64_t>::max();
            }

            if (onStart != nullptr) onStart(param);
            int64_t rowCount = 0;
            param[JOutElapsedTime] = StopWatch::Measure([&]() {
            //-> test start ----------------------------------------------------
                while(stmt.Step() == BE_SQLITE_ROW) {
                    stmt.GetValueText(0);
                    ++rowCount;
                    if (rowCount >= maxRows) {
                        break;
                    }
                }
            //-> test stop  ----------------------------------------------------
            }).ToSeconds();
            param[JOutRowCount] = rowCount;
            param[JOutQuery] = ecsql;
            if (onFinish != nullptr) onFinish(param);
            return SUCCESS;
        }
        enum class FilterType {
            PropExists_NonOptional,
            PropExists_Optional,
            ExtractProp_NonOptional,
            ExtractProp_Optional
        };
        BentleyStatus ReadProps (BeJsValue param, std::function<void(BeJsValue)> onStart, std::function<void(BeJsValue)> onFinish) {
            if (!param[JInTestName].asString().EqualsIAscii(JTestQueryInstanceProperties)) {
                return ERROR;
            }
            auto className = param[JInRootClassName].asString();
            auto maxRows = param[JInMaxRows].asInt64();
            auto maxProps  = param[JInMaxProps].asInt();
            auto includeDeriveProps = param[JInIncludeDeriveProps].asBool();
            auto shuffleProps= param[JInShuffleProps].asBool();
            auto propFilter = FromPropFilter(param[JInFilter].asString());

            ECSqlStatement stmt;
            Utf8String str;
            std::vector<std::string> props;
            auto classCP = m_conn.Schemas().FindClass(className);
            if (classCP == nullptr) {
                return ERROR;
            }
            if (includeDeriveProps) {
                if (GetDerivedProps(classCP->GetId(), props) != SUCCESS) {
                    return ERROR;
                }
            } else {
                if (GetProps(classCP->GetId(), props) != SUCCESS) {
                    return ERROR;
                }
            }
            if(shuffleProps) {
                std::shuffle(std::begin(props), std::end(props), m_randEngine);
            }
            if (maxProps <= 0) {
                maxProps = std::numeric_limits<int>::max();
            }
            while(props.size() > maxProps) {
                props.pop_back();
            }
            Utf8String ecsql = "SELECT ";
            Utf8String filter = " WHERE ";
            bool first = true;
            for(auto& prop : props){
                if (first) {
                    first = false;
                } else {
                    ecsql.append(", ");
                    if (propFilter == PropFilter::AllPropAreNotNull || propFilter == PropFilter::AllPropExists)
                        filter.append(" AND ");
                    else
                        filter.append(" OR ");
                }
                ecsql.append("$->").append(prop);

                if (propFilter == PropFilter::AllPropAreNotNull || propFilter == PropFilter::AnyPropIsNotNull) {
                    filter.append(SqlPrintfString("PROP_EXISTS(ECClassId,'%s')", prop.c_str()));
                } else {
                    filter.append(SqlPrintfString("$->%s IS NOT NULL", prop.c_str()));
                }
            }
            ecsql.append(" FROM ").append(className);
            if (propFilter != PropFilter::None) {
                ecsql.append(filter);
            }
            if (maxRows <= 0) {
                maxRows = std::numeric_limits<int64_t>::max();
            }
            if (stmt.Prepare(m_conn, ecsql.c_str()) != ECSqlStatus::Success) {
                return ERROR;
            }

            if (onStart != nullptr) onStart(param);
            int64_t rowCount = 0;
            param[JOutElapsedTime] = StopWatch::Measure([&]() {
            //-> test start ----------------------------------------------------
                while(stmt.Step() == BE_SQLITE_ROW) {
                    ++rowCount;
                    if (rowCount >= maxRows) {
                        break;
                    }
                }
            //-> test stop  ----------------------------------------------------
            }).ToSeconds();
            param[JOutRowCount] = rowCount;
            param[JOutQuery] = ecsql;
            if (onFinish != nullptr) onFinish(param);
            return SUCCESS;
        }
        Utf8String GetRootSubject() {
            ECSqlStatement stmt;
            if (stmt.Prepare(m_conn, "SELECT CodeValue FROM bis.subject WHERE ECInstanceId = 1") != ECSqlStatus::Success) {
                return "";
            }
            if (stmt.Step() != BE_SQLITE_ROW){
                return "";
            }
            return stmt.GetValueText(0);
        }

        Utf8String GetNativeSQL(Utf8CP ecsql) const{
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_conn, ecsql));
            return stmt.GetNativeSql();
        }
        void SetCommonResultProps(BeJsValue val) {
            val[JOutRootSubject] = GetRootSubject();
            val[JOutIModelId] = m_conn.QueryProjectGuid().ToString();
            val[JOutFileSize] = (int64_t)std::filesystem::file_size(std::filesystem::path{m_conn.GetDbFileName()});
        }
    public:

        InstancePropPerfTest(){
            m_testFuncs.insert_or_assign(JTestQueryInstanceWithNoFilter, [&](BeJsValue param, std::function<void(BeJsValue)> onStart, std::function<void(BeJsValue)> onFinish) { ReadInstances(param, onStart, onFinish); });
            m_testFuncs.insert_or_assign(JTestQueryInstanceProperties, [&](BeJsValue param, std::function<void(BeJsValue)> onStart, std::function<void(BeJsValue)> onFinish) { ReadProps(param, onStart, onFinish); });
        }
        ~InstancePropPerfTest(){}

        BentleyStatus Execute(std::filesystem::path fileName, BeJsValue param, std::function<void(BeJsValue)> onStart = nullptr, std::function<void(BeJsValue)> onFinish =nullptr) {
            m_randEngine.seed();
            if (m_conn.IsDbOpen()) {
                m_conn.CloseDb();
            }

            auto entry = std::filesystem::directory_entry(fileName);
            if (!entry.is_regular_file() && !entry.exists()) {
                return ERROR;
            }

            if (BE_SQLITE_OK != m_conn.OpenBeSQLiteDb(BeFileName(fileName.c_str()), Db::OpenParams(Db::OpenMode::Readonly))){
                return ERROR;
            }

            ECSqlStatement stmt;
            if (stmt.Prepare(m_conn, "PRAGMA experimental_features_enabled=true") != ECSqlStatus::Success) {
                return ERROR;
            }
            stmt.Step();
            const auto testName = param[JInTestName].asString();
            SetCommonResultProps(param);
            auto it = m_testFuncs.find(testName);
            if (it == m_testFuncs.end()) {
                return ERROR;
            }
            it->second(param, onStart, onFinish);
            return SUCCESS;
        }
};
#if 0
TEST_F(InstanceReaderFixture, native_sql) {
    InstancePropPerfTest gen;
    auto basePath = std::filesystem::path{"D:\\temp\\test-files"};
    auto reportOutDir = std::filesystem::path(basePath.c_str()).append("perf_data.db");
    Db resultDb;

    if (std::filesystem::exists(reportOutDir)) {
        ASSERT_EQ(BE_SQLITE_OK, resultDb.OpenBeSQLiteDb(BeFileName(reportOutDir.c_str()), Db::OpenParams(Db::OpenMode::ReadWrite)));
    } else {
        ASSERT_EQ(BE_SQLITE_OK, resultDb.CreateNewDb(BeFileName(reportOutDir.c_str())));
    }
    if (!resultDb.TableExists("perf_results")) {
        ASSERT_EQ(BE_SQLITE_OK, resultDb.ExecuteSql(R"x(
            CREATE TABLE [perf_results](
                [id] INTEGER PRIMARY KEY,
                [created_on] DATEITME DEFAULT (datetime ()),
                [release_build] INTEGER,
                [session] TEXT NOT NULL,
                [test_name] TEXT NOT NULL,
                [subject] TEXT NOT NULL,
                [root_class] TEXT,
                [row_count] INTEGER,
                [elapsed_sec] REAL,
                [result] TEXT);
            )x"));
    }
    ASSERT_EQ(BE_SQLITE_OK, resultDb.SaveChanges());
    auto saveResults = [](DbR db, BeJsValue val) {
        static BeGuid session(true);
        Statement stmt;
        stmt.Prepare(db, "INSERT INTO perf_results(release_build, session, test_name, subject, root_class, row_count, elapsed_sec, result) VALUES (?,?,?,?,?,?,?,?)");
        #ifdef NDEBUG
            stmt.BindInt(1, 1);
        #else
            stmt.BindInt(1, 0);
        #endif
        stmt.BindText(2, session.ToString().c_str(), Statement::MakeCopy::Yes);
        stmt.BindText(3, val[InstancePropPerfTest::JInTestName].asCString(), Statement::MakeCopy::Yes);
        stmt.BindText(4, val[InstancePropPerfTest::JOutRootSubject].asCString(), Statement::MakeCopy::Yes);
        stmt.BindText(5, val[InstancePropPerfTest::JInRootClassName].asCString(), Statement::MakeCopy::Yes);
        stmt.BindInt64(6, val[InstancePropPerfTest::JOutRowCount].asInt64());
        stmt.BindDouble(7, val[InstancePropPerfTest::JOutElapsedTime].asDouble());
        stmt.BindText(8, val.Stringify().c_str(), Statement::MakeCopy::Yes);
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
        db.SaveChanges();
        printf("%s\n", val.Stringify(StringifyFormat::Indented).c_str());
    };

    auto testReadInstances = [&](std::filesystem::path pathName, Utf8String className, int64_t maxRows = 0) {
        BeJsDocument param;
        param[InstancePropPerfTest::JInTestName] = InstancePropPerfTest::JTestQueryInstanceWithNoFilter;
        param[InstancePropPerfTest::JInRootClassName] = className;
        param[InstancePropPerfTest::JInMaxRows] = maxRows;
        EXPECT_EQ(SUCCESS, gen.Execute(pathName, param));
        saveResults(resultDb, param);
    };

    auto testReadProps = [&](
        std::filesystem::path pathName,
        Utf8String className,
        int64_t maxRows = 0,
        int maxProps = 10,
        Utf8String filterType = InstancePropPerfTest::JInFilterNone,
        bool includeDeriveProps = true,
        bool shuffleProps = true ) {

        BeJsDocument param;
        param[InstancePropPerfTest::JInTestName] = InstancePropPerfTest::JTestQueryInstanceProperties;
        param[InstancePropPerfTest::JInRootClassName] = className;
        param[InstancePropPerfTest::JInMaxRows] = maxRows;
        param[InstancePropPerfTest::JInFilter] = filterType;
        param[InstancePropPerfTest::JInShuffleProps] = shuffleProps;
        param[InstancePropPerfTest::JInIncludeDeriveProps] = includeDeriveProps;
        param[InstancePropPerfTest::JInMaxProps] = maxProps;
        EXPECT_EQ(SUCCESS, gen.Execute(pathName, param));
        saveResults(resultDb, param);
    };

    int maxRows = 100000;

    for(auto pathname: std::filesystem::directory_iterator(basePath)) {
        if (!pathname.is_regular_file())
            continue;

        if (!Utf8String(pathname.path().c_str()).EndsWithI(".bim"))
            continue;

        // testReadInstances(pathname.path(), "BisCore.Element", maxRows);
        testReadInstances(pathname.path(), "BisCore.ElementAspect", maxRows);

        for (auto propCount : std::vector{1, 5, 10, 20}) {
            // testReadProps(pathname.path(), "BisCore.Element", maxRows, propCount, InstancePropPerfTest::JInFilterAnyPropertyExists, true, true);
            // testReadProps(pathname.path(), "BisCore.Element", maxRows, propCount, InstancePropPerfTest::JInFilterAnyPropertyIsNotNull, true, true);
            // testReadProps(pathname.path(), "BisCore.Element", maxRows, propCount, InstancePropPerfTest::JInFilterAllPropertiesAreNotNull, true, true);
            // testReadProps(pathname.path(), "BisCore.Element", maxRows, propCount, InstancePropPerfTest::JInFilterAllPropertiesExists, true, true);

            testReadProps(pathname.path(), "BisCore.ElementAspect", maxRows, propCount, InstancePropPerfTest::JInFilterAnyPropertyExists, true, true);
            testReadProps(pathname.path(), "BisCore.ElementAspect", maxRows, propCount, InstancePropPerfTest::JInFilterAnyPropertyIsNotNull, true, true);
            testReadProps(pathname.path(), "BisCore.ElementAspect", maxRows, propCount, InstancePropPerfTest::JInFilterAllPropertiesAreNotNull, true, true);
            testReadProps(pathname.path(), "BisCore.ElementAspect", maxRows, propCount, InstancePropPerfTest::JInFilterAllPropertiesExists, true, true);
        }
    }

}
#endif
TEST_F(InstanceReaderFixture, experimental_check) {
    ASSERT_EQ(BE_SQLITE_OK, OpenECDbTestDataFile("test.bim"));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));

    ECSqlStatement stmt0;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt0.Prepare(m_ecdb, "SELECT $ FROM bis.CategorySelectorRefersToCategories"));

    ECSqlStatement stmt1;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt1.Prepare(m_ecdb, "SELECT $->ECInstanceId FROM bis.CategorySelectorRefersToCategories"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, check_link_table_serialization) {
    ASSERT_EQ(BE_SQLITE_OK, OpenECDbTestDataFile("test.bim"));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $ FROM bis.CategorySelectorRefersToCategories"));
    while(stmt.Step() == BE_SQLITE_ROW) {
        BeJsDocument doc;
        ASSERT_FALSE(stmt.IsValueNull(0)) << "$ cannot be NULL";
        doc.Parse(stmt.GetValueText(0));
        ASSERT_TRUE(doc.hasMember("ECInstanceId"))       << "Must have ECInstanceId Property";
        ASSERT_TRUE(doc.hasMember("ECClassId"))          << "Must have ECClassId Property";
        ASSERT_TRUE(doc.hasMember("SourceECInstanceId")) << "Must have SourceECInstanceId Property";
        ASSERT_TRUE(doc.hasMember("SourceECClassId"))    << "Must have SourceECClassId Property";
        ASSERT_TRUE(doc.hasMember("TargetECInstanceId")) << "Must have TargetECInstanceId Property";
        ASSERT_TRUE(doc.hasMember("TargetECClassId"))    << "Must have TargetECClassId Property";
    }
    stmt.Finalize();

}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, check_instance_serialization) {
    ASSERT_EQ(BE_SQLITE_OK, OpenECDbTestDataFile("test.bim"));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $ FROM bis.Element"));
    while(stmt.Step() == BE_SQLITE_ROW) {
        BeJsDocument doc;
        doc.Parse(stmt.GetValueText(0));
        ASSERT_TRUE(doc.hasMember("ECInstanceId")) << "Must have ECInstanceId Property";
        ASSERT_TRUE(doc.hasMember("ECClassId")) << "Must have ECClassId Property";
    }
    stmt.Finalize();

    auto expectedJson = R"json(
        [
            {
                "ECInstanceId":"0x38",
                "ECClassId":"0xe7",
                "Model":{
                    "Id":"0x1f",
                    "RelECClassId":"0x40"
                },
                "LastMod":"2017-07-25T20:44:59.926Z",
                "CodeSpec":{
                    "Id":"0x1",
                    "RelECClassId":"0x47"
                },
                "CodeScope":{
                    "Id":"0x1",
                    "RelECClassId":"0x49"
                },
                "Category":{
                    "Id":"0x17",
                    "RelECClassId":"0x8c"
                },
                "InSpatialIndex":true,
                "Origin":{
                    "X":6.494445575423782,
                    "Y":19.89784647571006,
                    "Z":8.020100502512559
                },
                "Yaw":25.949359512071446,
                "Pitch":4.770832022195274e-15,
                "Roll":114.7782627769506,
                "BBoxLow":{
                    "X":-9.735928156263862,
                    "Y":-9.735928156263864,
                    "Z":-9.735928156263858
                },
                "BBoxHigh":{
                    "X":9.735928156263858,
                    "Y":9.73592815626386,
                    "Z":9.735928156263855
                },
                "GeometryStream":"encoding=base64;yw=="
            },
            {
                "ECInstanceId":"0x39",
                "ECClassId":"0xe7",
                "Model":{
                    "Id":"0x24",
                    "RelECClassId":"0x40"
                },
                "LastMod":"2017-07-25T20:44:59.926Z",
                "CodeSpec":{
                    "Id":"0x1",
                    "RelECClassId":"0x47"
                },
                "CodeScope":{
                    "Id":"0x1",
                    "RelECClassId":"0x49"
                },
                "Category":{
                    "Id":"0x17",
                    "RelECClassId":"0x8c"
                },
                "InSpatialIndex":true,
                "Origin":{
                    "X":17.534003916481428,
                    "Y":13.798192542788694,
                    "Z":0.0
                },
                "Yaw":65.4666878656572,
                "Pitch":-3.1805546814635176e-15,
                "Roll":63.6877065778212,
                "BBoxLow":{
                    "X":-7.538875423282141,
                    "Y":-7.538875423282143,
                    "Z":-7.538875423282142
                },
                "BBoxHigh":{
                    "X":7.538875423282141,
                    "Y":7.538875423282143,
                    "Z":7.538875423282142
                },
                "GeometryStream":"encoding=base64;yQ=="
            },
            {
                "ECInstanceId":"0x3a",
                "ECClassId":"0xe7",
                "Model":{
                    "Id":"0x22",
                    "RelECClassId":"0x40"
                },
                "LastMod":"2017-07-25T20:44:59.926Z",
                "CodeSpec":{
                    "Id":"0x1",
                    "RelECClassId":"0x47"
                },
                "CodeScope":{
                    "Id":"0x1",
                    "RelECClassId":"0x49"
                },
                "Category":{
                    "Id":"0x17",
                    "RelECClassId":"0x8c"
                },
                "InSpatialIndex":true,
                "Origin":{
                    "X":3.5267011211011448,
                    "Y":-0.14981669899059627,
                    "Z":4.0100502512562795
                },
                "Yaw":25.94935951207144,
                "Pitch":-3.1805546814635168e-15,
                "Roll":114.7782627769506,
                "BBoxLow":{
                    "X":-9.735928156263856,
                    "Y":-9.735928156263853,
                    "Z":-9.735928156263855
                },
                "BBoxHigh":{
                    "X":9.735928156263856,
                    "Y":9.735928156263853,
                    "Z":9.735928156263855
                },
                "GeometryStream":"encoding=base64;zA=="
            },
            {
                "ECInstanceId":"0x3b",
                "ECClassId":"0xe7",
                "Model":{
                    "Id":"0x23",
                    "RelECClassId":"0x40"
                },
                "LastMod":"2017-07-25T20:44:59.942Z",
                "CodeSpec":{
                    "Id":"0x1",
                    "RelECClassId":"0x47"
                },
                "CodeScope":{
                    "Id":"0x1",
                    "RelECClassId":"0x49"
                },
                "Category":{
                    "Id":"0x17",
                    "RelECClassId":"0x8c"
                },
                "InSpatialIndex":true,
                "Origin":{
                    "X":2.080496897152193,
                    "Y":-4.824215490117017,
                    "Z":0.0
                },
                "Yaw":65.4666878656572,
                "Pitch":-3.1805546814635176e-15,
                "Roll":63.6877065778212,
                "BBoxLow":{
                    "X":-7.538875423282141,
                    "Y":-7.538875423282143,
                    "Z":-7.538875423282142
                },
                "BBoxHigh":{
                    "X":7.538875423282141,
                    "Y":7.538875423282143,
                    "Z":7.538875423282142
                },
                "GeometryStream":"encoding=base64;yQ=="
            }
        ]

    )json";

    BeJsDocument expectedDoc;
    expectedDoc.Parse(expectedJson);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select JSON_GROUP_ARRAY(JSON($)) from bis.GeometricElement3d"));
    while(stmt.Step() == BE_SQLITE_ROW) {
        BeJsDocument actualDoc;
        actualDoc.Parse(stmt.GetValueText(0));
        ASSERT_STRCASEEQ(expectedDoc.Stringify(StringifyFormat::Indented).c_str(), actualDoc.Stringify(StringifyFormat::Indented).c_str());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, ecsql_read_instance) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("instanceReader.ecdb"));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"sql(
        SELECT $ FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'
    )sql"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_STREQ(stmt.GetNativeSql(), "SELECT extract_inst([ECClassDef].[ECClassId],[ECClassDef].[ECInstanceId]) FROM (SELECT [Id] ECInstanceId,32 ECClassId,[Description] FROM [main].[ec_Class]) [ECClassDef] WHERE [ECClassDef].[Description]='Relates the property to its PropertyCategory.'");
    ASSERT_STREQ(stmt.GetValueText(0), R"json({"ECInstanceId":"0x2e","ECClassId":"0x20","Schema":{"Id":"0x4","RelECClassId":"0x21"},"Name":"PropertyHasCategory","Description":"Relates the property to its PropertyCategory.","Type":1,"Modifier":2,"RelationshipStrength":0,"RelationshipStrengthDirection":1})json");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, ecsql_read_property) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("instanceReader.ecdb"));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"sql(
        SELECT $ -> name FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'
    )sql"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ(stmt.GetNativeSql(), "SELECT extract_prop([ECClassDef].[ECClassId],[ECClassDef].[ECInstanceId],'name',:ecdb_this_ptr,0) FROM (SELECT [Id] ECInstanceId,32 ECClassId,[Description] FROM [main].[ec_Class]) [ECClassDef] WHERE [ECClassDef].[Description]='Relates the property to its PropertyCategory.'");
    ASSERT_STREQ(stmt.GetValueText(0), "PropertyHasCategory");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, rapid_json_patch_to_render_inf_and_nan_as_null_instead_of_failing_stringify) {
        // Test for bentley specific change \src\imodel-native\iModelCore\libsrc\rapidjson\vendor\include\rapidjson\writer.h#551
        // Patch to RapidJson write null instead of failing
       BeJsDocument docNan;
       docNan.toObject();
       docNan["a"] = 0.1;
       docNan["b"] = std::numeric_limits<double>::quiet_NaN();
       docNan["c"] = 4.4;
       ASSERT_STRCASEEQ("{\"a\":0.1,\"b\":null,\"c\":4.4}", docNan.Stringify().c_str());


       BeJsDocument docInf;
       docInf.toObject();
       docInf["a"] = 0.1;
       docInf["b"] = std::numeric_limits<double>::infinity();
       docInf["c"] = 4.4;
       ASSERT_STRCASEEQ("{\"a\":0.1,\"b\":null,\"c\":4.4}", docInf.Stringify().c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, rapid_json_patch_to_render_inf_and_nan_as_null_instead_of_failing_writer) {
        // Test for bentley specific change \src\imodel-native\iModelCore\libsrc\rapidjson\vendor\include\rapidjson\writer.h#353
        // Patch to RapidJson write null instead of failing
       std::ostringstream stream0;
       rapidjson::OStreamWrapper osw0(stream0);

       rapidjson::Writer<rapidjson::OStreamWrapper> writer0(osw0);
       writer0.StartObject();
       writer0.Key("a");
       writer0.Double(0.1);
       writer0.Key("b");
       writer0.Double(std::numeric_limits<double>::infinity());
       writer0.Key("c");
       writer0.Double(4.4);
       writer0.EndObject();
       writer0.Flush();
       osw0.Flush();
       stream0.flush();
       ASSERT_STRCASEEQ("{\"a\":0.1,\"b\":null,\"c\":4.4}", stream0.str().c_str());


       std::ostringstream stream1;
       rapidjson::OStreamWrapper osw1(stream1);
       rapidjson::Writer<rapidjson::OStreamWrapper> writer1(osw1);
       writer1.StartObject();
       writer1.Key("a");
       writer1.Double(0.1);
       writer1.Key("b");
       writer1.Double(std::numeric_limits<double>::quiet_NaN());
       writer1.Key("c");
       writer1.Double(4.4);
       writer1.EndObject();
       writer1.Flush();
       osw1.Flush();
       stream1.flush();
       ASSERT_STRCASEEQ("{\"a\":0.1,\"b\":null,\"c\":4.4}", stream1.str().c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, instance_reader) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("instanceReader.ecdb"));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"sql(
        SELECT ECClassId, ECInstanceId, EXTRACT_INST('meta.ecClassDef',ECInstanceId) FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'
    )sql"));

    BeJsDocument doc;
    doc.Parse(R"json({
        "ECInstanceId": "0x2e",
        "ECClassId": "0x20",
        "Schema": {
            "Id": "0x4",
            "RelECClassId": "0x21"
        },
        "Name": "PropertyHasCategory",
        "Description": "Relates the property to its PropertyCategory.",
        "Type": 1,
        "Modifier": 2,
        "RelationshipStrength": 0,
        "RelationshipStrengthDirection": 1
    })json");
    auto& reader = m_ecdb.GetInstanceReader();
    if(stmt.Step() == BE_SQLITE_ROW) {
        ECInstanceKey instanceKey (stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
        auto pos = InstanceReader::Position(stmt.GetValueId<ECInstanceId>(1), stmt.GetValueId<ECClassId>(0));
        ASSERT_EQ(true, reader.Seek(pos,[&](InstanceReader::IRowContext const& row){
            EXPECT_STRCASEEQ(doc.Stringify(StringifyFormat::Indented).c_str(), row.GetJson().Stringify(StringifyFormat::Indented).c_str());
        }));
    }
    if ("use syntax to get full instance") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, ECInstanceId, $ FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'"));
        const auto expectedSQL = "SELECT [ECClassDef].[ECClassId],[ECClassDef].[ECInstanceId],extract_inst([ECClassDef].[ECClassId],[ECClassDef].[ECInstanceId]) FROM (SELECT [Id] ECInstanceId,32 ECClassId,[Description] FROM [main].[ec_Class]) [ECClassDef] WHERE [ECClassDef].[Description]='Relates the property to its PropertyCategory.'";
        EXPECT_STRCASEEQ(expectedSQL, stmt.GetNativeSql());
        if(stmt.Step() == BE_SQLITE_ROW) {
            BeJsDocument inst;
            inst.Parse(stmt.GetValueText(2));
            EXPECT_STRCASEEQ(doc.Stringify(StringifyFormat::Indented).c_str(), inst.Stringify(StringifyFormat::Indented).c_str());

        }
    }
    if ("use syntax to get full instance using alias") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT m.ECClassId, m.ECInstanceId, m.$ FROM meta.ECClassDef m WHERE m.Description='Relates the property to its PropertyCategory.'"));
        const auto expectedSQL = "SELECT [m].[ECClassId],[m].[ECInstanceId],extract_inst([m].[ECClassId],[m].[ECInstanceId]) FROM (SELECT [Id] ECInstanceId,32 ECClassId,[Description] FROM [main].[ec_Class]) [m] WHERE [m].[Description]='Relates the property to its PropertyCategory.'";
        EXPECT_STRCASEEQ(expectedSQL, stmt.GetNativeSql());
        if(stmt.Step() == BE_SQLITE_ROW) {
            BeJsDocument inst;
            inst.Parse(stmt.GetValueText(2));
            EXPECT_STRCASEEQ(doc.Stringify(StringifyFormat::Indented).c_str(), inst.Stringify(StringifyFormat::Indented).c_str());

        }
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, dynamic_meta_data) {
    ASSERT_EQ(SUCCESS, SetupECDb("meta_data.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
            <ECEntityClass typeName="T000"
                description="Test meta data">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>500</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="T100"><BaseClass>T000</BaseClass><ECProperty propertyName="a" typeName="int" description="info-a"/></ECEntityClass>
            <ECEntityClass typeName="T200"><BaseClass>T000</BaseClass><ECProperty propertyName="b" typeName="int" description="info-b"/></ECEntityClass>
            <ECEntityClass typeName="T110"><BaseClass>T100</BaseClass><ECProperty propertyName="c" typeName="int" description="info-c"/></ECEntityClass>
            <ECEntityClass typeName="T120"><BaseClass>T100</BaseClass><ECProperty propertyName="d" typeName="int" description="info-d"/></ECEntityClass>
            <ECEntityClass typeName="T211"><BaseClass>T200</BaseClass><ECProperty propertyName="e" typeName="int" description="info-e"/></ECEntityClass>
            <ECEntityClass typeName="T212"><BaseClass>T200</BaseClass><ECProperty propertyName="f" typeName="int" description="info-f"/></ECEntityClass>
            <ECEntityClass typeName="T111"><BaseClass>T110</BaseClass><ECProperty propertyName="g" typeName="int" description="info-g"/></ECEntityClass>
            <ECEntityClass typeName="T112"><BaseClass>T110</BaseClass><ECProperty propertyName="h" typeName="int" description="info-h"/></ECEntityClass>
            <ECEntityClass typeName="T121"><BaseClass>T120</BaseClass><ECProperty propertyName="i" typeName="int" description="info-i"/></ECEntityClass>
            <ECEntityClass typeName="T122"><BaseClass>T120</BaseClass><ECProperty propertyName="j" typeName="int" description="info-j"/></ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    auto exec = [&](Utf8CP ecsql) {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
        m_ecdb.SaveChanges();
    };
    auto assertDefault = [](ECSqlStatement& stmt, int cl, Utf8CP displayLabel, Utf8CP propertyName) {
        ECSqlColumnInfo const* ci;
        PrimitiveECPropertyCP pr;
        Utf8CP className = "DynamicECSqlSelectClause";
        ci = &stmt.GetColumnInfo(cl);
        EXPECT_TRUE(ci->IsDynamic());
        EXPECT_TRUE(ci->GetDataType().IsPrimitive());
        pr = ci->GetProperty()->GetAsPrimitiveProperty();
        EXPECT_FALSE(pr->HasId());
        EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, pr->GetType());
        EXPECT_STRCASEEQ(className                   , pr->GetClass().GetName().c_str());
        EXPECT_STRCASEEQ(propertyName                , pr->GetName().c_str());
        EXPECT_STRCASEEQ(""                          , pr->GetDescription().c_str());
        EXPECT_STRCASEEQ(displayLabel                , pr->GetDisplayLabel().c_str());
        EXPECT_STRCASEEQ("json"                      , pr->GetExtendedTypeName().c_str());
    };
    auto assertDynamic = [](ECSqlStatement& stmt, int cl, Utf8CP displayLabel, Utf8CP propertyName, Utf8CP description, Utf8CP className, PrimitiveType t) {
        ECSqlColumnInfo const* ci;
        PrimitiveECPropertyCP pr;
        ci = &stmt.GetColumnInfo(cl);
        EXPECT_TRUE(ci->IsDynamic());
        EXPECT_TRUE(ci->GetDataType().IsPrimitive());
        pr = ci->GetProperty()->GetAsPrimitiveProperty();
        EXPECT_TRUE(pr->HasId());
        EXPECT_EQ(t                  , pr->GetType());
        EXPECT_STRCASEEQ(className   , pr->GetClass().GetName().c_str());
        EXPECT_STRCASEEQ(propertyName, pr->GetName().c_str());
        EXPECT_STRCASEEQ(description , pr->GetDescription().c_str());
        EXPECT_STRCASEEQ(displayLabel, pr->GetDisplayLabel().c_str());
        EXPECT_STRCASEEQ(""          , pr->GetExtendedTypeName().c_str());
    };
    exec("insert into ts.t100 ( ecInstanceId, a       ) values ( 10, 100           )");
    exec("insert into ts.t200 ( ecInstanceId, b       ) values ( 11, 101           )");
    exec("insert into ts.t110 ( ecInstanceId, a, c    ) values ( 12, 102, 200      )");
    exec("insert into ts.t120 ( ecInstanceId, a, d    ) values ( 13, 103, 201      )");
    exec("insert into ts.t211 ( ecInstanceId, b, e    ) values ( 14, 104, 202      )");
    exec("insert into ts.t212 ( ecInstanceId, b, f    ) values ( 15, 105, 203      )");
    exec("insert into ts.t111 ( ecInstanceId, a, c, g ) values ( 16, 106, 204, 301 )");
    exec("insert into ts.t112 ( ecInstanceId, a, c, h ) values ( 17, 107, 205, 302 )");
    exec("insert into ts.t121 ( ecInstanceId, a, d, i ) values ( 18, 108, 206, 303 )");
    exec("insert into ts.t122 ( ecInstanceId, a, d, j ) values ( 19, 109, 207, 304 )");

    if ("top level instance props meta data") {
        const auto sql = R"x(
            select
                ecInstanceId,
                $->a,
                $->b,
                $->c,
                $->d,
                $->e,
                $->f,
                $->g,
                $->h,
                $->i,
                $->j
            from ts.t000
        )x";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
        while(stmt.Step() == BE_SQLITE_ROW) {
            auto ecInstanceId = stmt.GetValueInt(0);
            if (ecInstanceId == 10) { // t100, a
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 11) { // t200, b
                assertDefault(stmt, 1 , "$ -> a", "__x0024____x0020____x002D____x003E____x0020__a");
                assertDynamic(stmt, 2 , "b", "b", "info-b", "T200", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 12) { // t110, a, c
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDynamic(stmt, 3 , "c", "c", "info-c", "T110", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 13) { // t120, a, d
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDynamic(stmt, 4 , "d", "d", "info-d", "T120", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 14) { // t211, b, e
                assertDefault(stmt, 1 , "$ -> a", "__x0024____x0020____x002D____x003E____x0020__a");
                assertDynamic(stmt, 2 , "b", "b", "info-b", "T200", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDynamic(stmt, 5 , "e", "e", "info-e", "T211", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 15) { // t212, b, f
                assertDefault(stmt, 1 , "$ -> a", "__x0024____x0020____x002D____x003E____x0020__a");
                assertDynamic(stmt, 2 , "b", "b", "info-b", "T200", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDynamic(stmt, 6 , "f", "f", "info-f", "T212", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 16) { // t111, a, c, g
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDynamic(stmt, 3 , "c", "c", "info-c", "T110", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDynamic(stmt, 7 , "g", "g", "info-g", "T111", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 17) { // t112, a, c, h
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDynamic(stmt, 3 , "c", "c", "info-c", "T110", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDynamic(stmt, 8 , "h", "h", "info-h", "T112", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 18) { // t121, a, d, i
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDynamic(stmt, 4 , "d", "d", "info-d", "T120", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDynamic(stmt, 9 , "i", "i", "info-i", "T121", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 19) { // t122, a, d, j
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDynamic(stmt, 4 , "d", "d", "info-d", "T120", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDynamic(stmt, 10, "j", "j", "info-j", "T122", PrimitiveType::PRIMITIVETYPE_Integer);
            }
        }
    }
    if ("subquery instance prop meta data ") {
        const auto sql = R"x(
            select * from (
                select
                    ecInstanceId,
                    $->a,
                    $->b,
                    $->c,
                    $->d,
                    $->e,
                    $->f,
                    $->g,
                    $->h,
                    $->i,
                    $->j
                from ts.t000
            )
        )x";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
        while(stmt.Step() == BE_SQLITE_ROW) {
            auto ecInstanceId = stmt.GetValueInt(0);
            if (ecInstanceId == 10) { // t100, a
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 11) { // t200, b
                assertDefault(stmt, 1 , "$ -> a", "__x0024____x0020____x002D____x003E____x0020__a");
                assertDynamic(stmt, 2 , "b", "b", "info-b", "T200", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 12) { // t110, a, c
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDynamic(stmt, 3 , "c", "c", "info-c", "T110", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 13) { // t120, a, d
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDynamic(stmt, 4 , "d", "d", "info-d", "T120", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 14) { // t211, b, e
                assertDefault(stmt, 1 , "$ -> a", "__x0024____x0020____x002D____x003E____x0020__a");
                assertDynamic(stmt, 2 , "b", "b", "info-b", "T200", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDynamic(stmt, 5 , "e", "e", "info-e", "T211", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 15) { // t212, b, f
                assertDefault(stmt, 1 , "$ -> a", "__x0024____x0020____x002D____x003E____x0020__a");
                assertDynamic(stmt, 2 , "b", "b", "info-b", "T200", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDynamic(stmt, 6 , "f", "f", "info-f", "T212", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 16) { // t111, a, c, g
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDynamic(stmt, 3 , "c", "c", "info-c", "T110", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDynamic(stmt, 7 , "g", "g", "info-g", "T111", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 17) { // t112, a, c, h
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDynamic(stmt, 3 , "c", "c", "info-c", "T110", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 4 , "$ -> d", "__x0024____x0020____x002D____x003E____x0020__d");
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDynamic(stmt, 8 , "h", "h", "info-h", "T112", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 18) { // t121, a, d, i
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDynamic(stmt, 4 , "d", "d", "info-d", "T120", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDynamic(stmt, 9 , "i", "i", "info-i", "T121", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 10, "$ -> j", "__x0024____x0020____x002D____x003E____x0020__j");
            } else if (ecInstanceId == 19) { // t122, a, d, j
                assertDynamic(stmt, 1 , "a", "a", "info-a", "T100", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 2 , "$ -> b", "__x0024____x0020____x002D____x003E____x0020__b");
                assertDefault(stmt, 3 , "$ -> c", "__x0024____x0020____x002D____x003E____x0020__c");
                assertDynamic(stmt, 4 , "d", "d", "info-d", "T120", PrimitiveType::PRIMITIVETYPE_Integer);
                assertDefault(stmt, 5 , "$ -> e", "__x0024____x0020____x002D____x003E____x0020__e");
                assertDefault(stmt, 6 , "$ -> f", "__x0024____x0020____x002D____x003E____x0020__f");
                assertDefault(stmt, 7 , "$ -> g", "__x0024____x0020____x002D____x003E____x0020__g");
                assertDefault(stmt, 8 , "$ -> h", "__x0024____x0020____x002D____x003E____x0020__h");
                assertDefault(stmt, 9 , "$ -> i", "__x0024____x0020____x002D____x003E____x0020__i");
                assertDynamic(stmt, 10, "j", "j", "info-j", "T122", PrimitiveType::PRIMITIVETYPE_Integer);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, extract_prop) {
    ASSERT_EQ(SUCCESS, SetupECDb("PROP_EXISTS.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                    <ECEntityClass typeName="P">
                        <ECProperty propertyName="s"    typeName="string"  />
                        <ECProperty propertyName="i"    typeName="int"     />
                        <ECProperty propertyName="d"    typeName="double"  />
                        <ECProperty propertyName="p2d"  typeName="point2d" />
                        <ECProperty propertyName="p3d"  typeName="point3d" />
                        <ECProperty propertyName="bi"   typeName="binary"  />
                        <ECProperty propertyName="l"    typeName="long"    />
                        <ECProperty propertyName="dt"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECProperty>
                        <ECProperty propertyName="b"    typeName="boolean" />
                    </ECEntityClass>
               </ECSchema>)xml")));
    m_ecdb.SaveChanges();
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    // sample primitive type
    const bool kB = true;
    const DateTime kDt = DateTime::GetCurrentTimeUtc();
    const double kD = 3.13;
    const DPoint2d kP2d = DPoint2d::From(2, 4);
    const DPoint3d kP3d = DPoint3d::From(4, 5, 6);
    const int kBiLen = 10;
    const int kI = 0x3432;
    const int64_t kL = 0xfefefefefefefefe;
    const uint8_t kBi[kBiLen] = {0x71, 0xdd, 0x83, 0x7d, 0x0b, 0xf2, 0x50, 0x01, 0x0a, 0xe1};
    const char* kText = "Hello, World";

    if ("insert a row with primitive value") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success,
                stmt.Prepare(m_ecdb, "INSERT INTO ts.P(s,i,d,p2d,p3d,bi,l,dt,b) VALUES(?,?,?,?,?,?,?,?,?)"));

        // bind primitive types
        stmt.BindText(1, kText, IECSqlBinder::MakeCopy::No);
        stmt.BindInt(2, kI);
        stmt.BindDouble(3, kD);
        stmt.BindPoint2d(4, kP2d);
        stmt.BindPoint3d(5, kP3d);
        stmt.BindBlob(6, &kBi[0], kBiLen, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(7, kL);
        stmt.BindDateTime(8, kDt);
        stmt.BindBoolean(9, kB);
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    const auto sql =
        "SELECT                                        "
        "   EXTRACT_INST(ecClassId, ecInstanceId),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 's'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'i'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'd'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'p2d'),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'p3d'),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'bi' ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'l'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'dt' ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'b'  ) "
        "FROM ts.P                                     ";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    int i = 1;
    ASSERT_STRCASEEQ(stmt.GetValueText(i++), kText);
    ASSERT_EQ(stmt.GetValueInt(i++), kI);
    ASSERT_EQ(stmt.GetValueDouble(i++), kD);
    ASSERT_STRCASEEQ(stmt.GetValueText(i++), "{\"X\":2.0,\"Y\":4.0}"); // return as json
    ASSERT_STRCASEEQ(stmt.GetValueText(i++), "{\"X\":4.0,\"Y\":5.0,\"Z\":6.0}"); // return as json
    ASSERT_EQ(memcmp(stmt.GetValueBlob(i++), &kBi[0], kBiLen), 0);
    ASSERT_EQ(stmt.GetValueInt64(i++), kL);
    ASSERT_TRUE(stmt.GetValueDateTime (i++).Equals(kDt, true));
    ASSERT_EQ(stmt.GetValueBoolean(i++), kB);

    if ("use syntax to extract property") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $->s, $->i, $->d, $->p2d, $->p3d, $->bi, $->l, $->dt, $->b FROM ts.P"));
        const auto expectedSQL = "SELECT extract_prop([P].[ECClassId],[P].[ECInstanceId],'s',:ecdb_this_ptr,0),extract_prop([P].[ECClassId],[P].[ECInstanceId],'i',:ecdb_this_ptr,1),extract_prop([P].[ECClassId],[P].[ECInstanceId],'d',:ecdb_this_ptr,2),extract_prop([P].[ECClassId],[P].[ECInstanceId],'p2d',:ecdb_this_ptr,3),extract_prop([P].[ECClassId],[P].[ECInstanceId],'p3d',:ecdb_this_ptr,4),extract_prop([P].[ECClassId],[P].[ECInstanceId],'bi',:ecdb_this_ptr,5),extract_prop([P].[ECClassId],[P].[ECInstanceId],'l',:ecdb_this_ptr,6),extract_prop([P].[ECClassId],[P].[ECInstanceId],'dt',:ecdb_this_ptr,7),extract_prop([P].[ECClassId],[P].[ECInstanceId],'b',:ecdb_this_ptr,8) FROM (SELECT [Id] ECInstanceId,73 ECClassId FROM [main].[ts_P]) [P]";
        EXPECT_STRCASEEQ(expectedSQL, stmt.GetNativeSql());
        if(stmt.Step() == BE_SQLITE_ROW) {
            int i = 0;
            ASSERT_STRCASEEQ(stmt.GetValueText(i++), kText);
            ASSERT_EQ(stmt.GetValueInt(i++), kI);
            ASSERT_EQ(stmt.GetValueDouble(i++), kD);
            ASSERT_STRCASEEQ(stmt.GetValueText(i++), "{\"X\":2.0,\"Y\":4.0}"); // return as json
            ASSERT_STRCASEEQ(stmt.GetValueText(i++), "{\"X\":4.0,\"Y\":5.0,\"Z\":6.0}"); // return as json
            ASSERT_EQ(memcmp(stmt.GetValueBlob(i++), &kBi[0], kBiLen), 0);
            ASSERT_EQ(stmt.GetValueInt64(i++), kL);
            ASSERT_TRUE(stmt.GetValueDateTime (i++).Equals(kDt, true));
            ASSERT_EQ(stmt.GetValueBoolean(i++), kB);
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, prop_exists) {
    ASSERT_EQ(SUCCESS, SetupECDb("PROP_EXISTS.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="SubProp1" typeName="string" />
                        <ECProperty propertyName="SubProp2" typeName="int" />
                    </ECEntityClass>
               </ECSchema>)xml")));
    m_ecdb.SaveChanges();

    if ("case sensitive check") {
        const auto sql =
            "SELECT "
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECClassId'   ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECInstanceId'),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'Prop1'       ),"
            "   PROP_EXISTS(ec_classId('ts.base'), 'Prop2'       ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SubProp1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SubProp2'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECClassId'   ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECInstanceId'),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'Prop1'       ),"
            "   PROP_EXISTS(ec_classId('ts.Sub'),  'Prop2'       ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SubProp1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SubProp2'    )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));

        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        int i = 0;
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp1 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp2 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
    }

    if ("case insensitive check") {
        const auto sql =
            "SELECT "
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECCLASSID'   ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECINSTANCEID'),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'PROP1'       ),"
            "   PROP_EXISTS(ec_classId('ts.base'), 'PROP2'       ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SUBPROP1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SUBPROP2'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECCLASSID'   ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECINSTANCEID'),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'PROP1'       ),"
            "   PROP_EXISTS(ec_classId('ts.Sub'),  'PROP2'       ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SUBPROP1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SUBPROP2'    )";

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        int i = 0;
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp1 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp2 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, nested_struct) {
    ASSERT_EQ(SUCCESS, SetupECDb("nested_struct.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                <ECStructClass typeName="struct_p" description="Struct with primitive props (default mappings)">
                    <ECProperty propertyName="b" typeName="boolean" />
                    <ECProperty propertyName="bi" typeName="binary" />
                    <ECProperty propertyName="d" typeName="double" />
                    <ECProperty propertyName="dt" typeName="dateTime" />
                    <ECProperty propertyName="dtUtc" typeName="dateTime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="s" typeName="string" />
                    <ECProperty propertyName="p2d" typeName="point2d" />
                    <ECProperty propertyName="p3d" typeName="point3d" />
                    <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
                </ECStructClass>
                <ECStructClass typeName="struct_pa" description="Primitive array">
                    <ECArrayProperty propertyName="b_array" typeName="boolean" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="bi_array" typeName="binary" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="d_array" typeName="double" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dt_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dtUtc_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECArrayProperty>
                    <ECArrayProperty propertyName="i_array" typeName="int" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="l_array" typeName="long" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="s_array" typeName="string" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p2d_array" typeName="point2d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p3d_array" typeName="point3d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded" />
                </ECStructClass>
                <ECEntityClass typeName="e_mix"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <MaxSharedColumnsBeforeOverflow>500</MaxSharedColumnsBeforeOverflow>
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECNavigationProperty propertyName="parent" relationshipName="e_mix_has_base_mix" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                                <OnDeleteAction>Cascade</OnDeleteAction>
                                <OnUpdateAction>Cascade</OnUpdateAction>
                            </ForeignKeyConstraint>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                    <ECProperty propertyName="b" typeName="boolean" />
                    <ECProperty propertyName="bi" typeName="binary" />
                    <ECProperty propertyName="d" typeName="double" />
                    <ECProperty propertyName="dt" typeName="dateTime" />
                    <ECProperty propertyName="dtUtc" typeName="dateTime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="s" typeName="string" />
                    <ECProperty propertyName="p2d" typeName="point2d" />
                    <ECProperty propertyName="p3d" typeName="point3d" />
                    <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECArrayProperty propertyName="b_array" typeName="boolean" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="bi_array" typeName="binary" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="d_array" typeName="double" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dt_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dtUtc_array" typeName="dateTime" minOccurs="0"
                        maxOccurs="unbounded">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECArrayProperty>
                    <ECArrayProperty propertyName="i_array" typeName="int" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="l_array" typeName="long" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="s_array" typeName="string" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p2d_array" typeName="point2d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p3d_array" typeName="point3d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded" />
                    <ECStructProperty propertyName="p" typeName="struct_p" />
                    <ECStructProperty propertyName="pa" typeName="struct_pa" />
                    <ECStructArrayProperty propertyName="array_of_p" typeName="struct_p" minOccurs="0" maxOccurs="unbounded" />
                    <ECStructArrayProperty propertyName="array_of_pa" typeName="struct_pa" minOccurs="0" maxOccurs="unbounded" />
                </ECEntityClass>
                <ECRelationshipClass typeName="e_mix_has_base_mix" strength="Embedding" modifier="Sealed">
                    <Source multiplicity="(0..1)" polymorphic="false" roleLabel="e_mix">
                        <Class class="e_mix" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="false" roleLabel="e_mix">
                        <Class class="e_mix" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECInstanceKey instKey;
    if ("insert data") {
        ECSqlStatement stmt;
        auto rc = stmt.Prepare(m_ecdb, R"sql(
            insert into ts.e_mix(
                parent,
                b, bi, d, dt, dtUtc, i, l, s, p2d, p3d, geom,
                b_array, bi_array, d_array, dt_array, dtUtc_array,
                i_array, l_array, s_array, p2d_array, p3d_array, geom_array,
                p, pa, array_of_p, array_of_pa)
            values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )sql");

        const auto b = true;
        const uint8_t bi[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};
        const double d = PI;
        const auto dt = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
        const auto dtUtc = DateTime(DateTime::Kind::Utc, 2018, 2, 17, 0, 0);
        const int i = 0xfffafaff;
        const int64_t l = 0xfffafaffafffffff;
        const auto s = std::string{"Hello, World!"};
        const auto p2d = DPoint2d::From(22.33, -21.34);
        const auto p3d = DPoint3d::From(12.13, -42.34, -93.12);
        auto geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
        const bool b_array[] = {true, false, true};
        const std::vector<std::vector<uint8_t>> bi_array = {
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6}
        };
        const std::vector<double> d_array = {123.3434, 345.223, -532.123};
        const std::vector<DateTime> dt_array = {
            DateTime(DateTime::Kind::Unspecified, 2017, 1, 14, 0, 0),
            DateTime(DateTime::Kind::Unspecified, 2018, 1, 13, 0, 0),
            DateTime(DateTime::Kind::Unspecified, 2019, 1, 11, 0, 0),
        };
        const std::vector<DateTime> dtUtc_array = {
            DateTime(DateTime::Kind::Utc, 2017, 1, 17, 0, 0),
            DateTime(DateTime::Kind::Utc, 2018, 1, 11, 0, 0),
            DateTime(DateTime::Kind::Utc, 2019, 1, 10, 0, 0),
        };
        const std::vector<int> i_array = {3842, -4923, 8291};
        const std::vector<int64_t> l_array = {384242, -234923, 528291};
        const std::vector<std::string> s_array = {"Bentley", "System"};
        const std::vector<DPoint2d> p2d_array = {
            DPoint2d::From(22.33 , -81.17),
            DPoint2d::From(-42.74,  16.29),
            DPoint2d::From(77.45 , -32.98),
        };
        const std::vector<DPoint3d> p3d_array = {
            DPoint3d::From( 84.13,  99.23, -121.75),
            DPoint3d::From(-90.34,  45.75, -452.34),
            DPoint3d::From(-12.54, -84.23, -343.45),
        };
        const std::vector<IGeometryPtr> geom_array = {
            IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 4.0, 2.1, 1.2))),
            IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.1, 2.5, 4.2))),
            IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 9.1, 3.6, 3.8))),
        };
        int idx = 0;
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(++idx));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(++idx, true));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(++idx, (void const*)bi, (int)sizeof(bi), IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(++idx, d));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(++idx, dt));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(++idx, dtUtc));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(++idx, i));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(++idx, l));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(++idx, s.c_str(), IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(++idx, p2d));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(++idx, p3d));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(++idx, *geom));
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : b_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindBoolean(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : bi_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : d_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindDouble(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : dt_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindDateTime(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : dtUtc_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindDateTime(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : i_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindInt(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : l_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindInt64(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : s_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindText(m.c_str(), IECSqlBinder::MakeCopy::No));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : p2d_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindPoint2d(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : p3d_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindPoint3d(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : geom_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindGeometry(*m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            ASSERT_EQ(ECSqlStatus::Success, (*v)["b"].BindBoolean(b));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["bi"].BindBlob((void const*)bi, (int)sizeof(bi), IECSqlBinder::MakeCopy::No));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["d"].BindDouble(d));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["dt"].BindDateTime(dt));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["dtUtc"].BindDateTime(dtUtc));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["i"].BindInt(i));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["l"].BindInt64(l));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["s"].BindText(s.c_str(), IECSqlBinder::MakeCopy::No));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["p2d"].BindPoint2d(p2d));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["p3d"].BindPoint3d(p3d));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["geom"].BindGeometry(*geom));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : b_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["b_array"].AddArrayElement().BindBoolean(m));
            for(auto& m : bi_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["bi_array"].AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));
            for(auto& m : d_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["d_array"].AddArrayElement().BindDouble(m));
            for(auto& m : dt_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["dt_array"].AddArrayElement().BindDateTime(m));
            for(auto& m : dtUtc_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["dtUtc_array"].AddArrayElement().BindDateTime(m));
            for(auto& m : i_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["i_array"].AddArrayElement().BindInt(m));
            for(auto& m : l_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["l_array"].AddArrayElement().BindInt64(m));
            for(auto& m : s_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["s_array"].AddArrayElement().BindText(m.c_str(), IECSqlBinder::MakeCopy::No));
            for(auto& m : p2d_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["p2d_array"].AddArrayElement().BindPoint2d(m));
            for(auto& m : p3d_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["p3d_array"].AddArrayElement().BindPoint3d(m));
            for(auto& m : geom_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["geom_array"].AddArrayElement().BindGeometry(*m));
        }
        if (auto o = &stmt.GetBinder(++idx)) {
            for (int n = 0; n < 2;++n) {
                auto& v = o->AddArrayElement();
                ASSERT_EQ(ECSqlStatus::Success, v["b"].BindBoolean(b_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["bi"].BindBlob((void const*)&bi_array[n][0], (int)bi_array[n].size(), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ(ECSqlStatus::Success, v["d"].BindDouble(d_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["dt"].BindDateTime(dt_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["dtUtc"].BindDateTime(dtUtc_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["i"].BindInt(i_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["l"].BindInt64(l_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["s"].BindText(s_array[n].c_str(), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ(ECSqlStatus::Success, v["p2d"].BindPoint2d(p2d_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["p3d"].BindPoint3d(p3d_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["geom"].BindGeometry(*geom_array[n]));
            }
        }
        if (auto o = &stmt.GetBinder(++idx)) {
            for (int n = 0; n < 2;++n) {
                auto& v = o->AddArrayElement();
                for(auto& m : b_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["b_array"].AddArrayElement().BindBoolean(m));
                for(auto& m : bi_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["bi_array"].AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));
                for(auto& m : d_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["d_array"].AddArrayElement().BindDouble(m));
                for(auto& m : dt_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["dt_array"].AddArrayElement().BindDateTime(m));
                for(auto& m : dtUtc_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["dtUtc_array"].AddArrayElement().BindDateTime(m));
                for(auto& m : i_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["i_array"].AddArrayElement().BindInt(m));
                for(auto& m : l_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["l_array"].AddArrayElement().BindInt64(m));
                for(auto& m : s_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["s_array"].AddArrayElement().BindText(m.c_str(), IECSqlBinder::MakeCopy::No));
                for(auto& m : p2d_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["p2d_array"].AddArrayElement().BindPoint2d(m));
                for(auto& m : p3d_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["p3d_array"].AddArrayElement().BindPoint3d(m));
                for(auto& m : geom_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["geom_array"].AddArrayElement().BindGeometry(*m));
            }
        }
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(instKey));
        m_ecdb.SaveChanges();
    }

    BeJsDocument expected;
    expected.Parse(R"json({
        "ECInstanceId": "0x1",
        "ECClassId": "0x49",
        "b": true,
        "bi": "encoding=base64;SA==",
        "d": 3.141592653589793,
        "dt": "2017-01-17T00:00:00.000",
        "dtUtc": "2018-02-17T00:00:00.000Z",
        "i": -328961,
        "l": -1412873783869441.0,
        "s": "Hello, World!",
        "p2d": {
            "X": 22.33,
            "Y": -21.34
        },
        "p3d": {
            "X": 12.13,
            "Y": -42.34,
            "Z": -93.12
        },
        "geom": {
            "lineSegment": [
                [
                    0,
                    0,
                    0
                ],
                [
                    1,
                    1,
                    1
                ]
            ]
        },
        "b_array": [
            true,
            false,
            true
        ],
        "bi_array": [
            "encoding=base64;SA==",
            "encoding=base64;SA==",
            "encoding=base64;SA=="
        ],
        "d_array": [
            123.3434,
            345.223,
            -532.123
        ],
        "dt_array": [
            "2017-01-14T00:00:00.000",
            "2018-01-13T00:00:00.000",
            "2019-01-11T00:00:00.000"
        ],
        "dtUtc_array": [
            "2017-01-17T00:00:00.000",
            "2018-01-11T00:00:00.000",
            "2019-01-10T00:00:00.000"
        ],
        "i_array": [
            3842,
            -4923,
            8291
        ],
        "l_array": [
            384242.0,
            -234923.0,
            528291.0
        ],
        "s_array": [
            "Bentley",
            "System"
        ],
        "p2d_array": [
            {
                "X": 22.33,
                "Y": -81.17
            },
            {
                "X": -42.74,
                "Y": 16.29
            },
            {
                "X": 77.45,
                "Y": -32.98
            }
        ],
        "p3d_array": [
            {
                "X": 84.13,
                "Y": 99.23,
                "Z": -121.75
            },
            {
                "X": -90.34,
                "Y": 45.75,
                "Z": -452.34
            },
            {
                "X": -12.54,
                "Y": -84.23,
                "Z": -343.45
            }
        ],
        "geom_array": [
            {
                "lineSegment": [
                    [
                    0,
                    0,
                    0
                    ],
                    [
                    4,
                    2.1,
                    1.2
                    ]
                ]
            },
            {
                "lineSegment": [
                    [
                    0,
                    0,
                    0
                    ],
                    [
                    1.1,
                    2.5,
                    4.2
                    ]
                ]
            },
            {
                "lineSegment": [
                    [
                    0,
                    0,
                    0
                    ],
                    [
                    9.1,
                    3.6,
                    3.8
                    ]
                ]
            }
        ],
        "p": {
            "b": true,
            "bi": "encoding=base64;SA==",
            "d": 3.141592653589793,
            "dt": "2017-01-17T00:00:00.000",
            "dtUtc": "2018-02-17T00:00:00.000Z",
            "geom": {
                "lineSegment": [
                    [
                    0,
                    0,
                    0
                    ],
                    [
                    1,
                    1,
                    1
                    ]
                ]
            },
            "i": -328961,
            "l": -1412873783869441.0,
            "p2d": {
                "X": 22.33,
                "Y": -21.34
            },
            "p3d": {
                "X": 12.13,
                "Y": -42.34,
                "Z": -93.12
            },
            "s": "Hello, World!"
        },
        "pa": {
            "b_array": [
                true,
                false,
                true
            ],
            "bi_array": [
                "encoding=base64;SA==",
                "encoding=base64;SA==",
                "encoding=base64;SA=="
            ],
            "d_array": [
                123.3434,
                345.223,
                -532.123
            ],
            "dt_array": [
                "2017-01-14T00:00:00.000",
                "2018-01-13T00:00:00.000",
                "2019-01-11T00:00:00.000"
            ],
            "dtUtc_array": [
                "2017-01-17T00:00:00.000",
                "2018-01-11T00:00:00.000",
                "2019-01-10T00:00:00.000"
            ],
            "geom_array": [
                {
                    "lineSegment": [
                    [
                        0,
                        0,
                        0
                    ],
                    [
                        4,
                        2.1,
                        1.2
                    ]
                    ]
                },
                {
                    "lineSegment": [
                    [
                        0,
                        0,
                        0
                    ],
                    [
                        1.1,
                        2.5,
                        4.2
                    ]
                    ]
                },
                {
                    "lineSegment": [
                    [
                        0,
                        0,
                        0
                    ],
                    [
                        9.1,
                        3.6,
                        3.8
                    ]
                    ]
                }
            ],
            "i_array": [
                3842,
                -4923,
                8291
            ],
            "l_array": [
                384242.0,
                -234923.0,
                528291.0
            ],
            "p2d_array": [
                {
                    "X": 22.33,
                    "Y": -81.17
                },
                {
                    "X": -42.74,
                    "Y": 16.29
                },
                {
                    "X": 77.45,
                    "Y": -32.98
                }
            ],
            "p3d_array": [
                {
                    "X": 84.13,
                    "Y": 99.23,
                    "Z": -121.75
                },
                {
                    "X": -90.34,
                    "Y": 45.75,
                    "Z": -452.34
                },
                {
                    "X": -12.54,
                    "Y": -84.23,
                    "Z": -343.45
                }
            ],
            "s_array": [
                "Bentley",
                "System"
            ]
        },
        "array_of_p": [
            {
                "b": true,
                "bi": "encoding=base64;SA==",
                "d": 123.3434,
                "dt": "2017-01-14T00:00:00.000",
                "dtUtc": "2017-01-17T00:00:00.000Z",
                "i": 3842,
                "l": 384242.0,
                "s": "Bentley",
                "p2d": {
                    "X": 22.33,
                    "Y": -81.17
                },
                "p3d": {
                    "X": 84.13,
                    "Y": 99.23,
                    "Z": -121.75
                },
                "geom": {
                    "lineSegment": [
                    [
                        0,
                        0,
                        0
                    ],
                    [
                        4,
                        2.1,
                        1.2
                    ]
                    ]
                }
            },
            {
                "b": false,
                "bi": "encoding=base64;SA==",
                "d": 345.223,
                "dt": "2018-01-13T00:00:00.000",
                "dtUtc": "2018-01-11T00:00:00.000Z",
                "i": -4923,
                "l": -234923.0,
                "s": "System",
                "p2d": {
                    "X": -42.74,
                    "Y": 16.29
                },
                "p3d": {
                    "X": -90.34,
                    "Y": 45.75,
                    "Z": -452.34
                },
                "geom": {
                    "lineSegment": [
                    [
                        0,
                        0,
                        0
                    ],
                    [
                        1.1,
                        2.5,
                        4.2
                    ]
                    ]
                }
            }
        ],
        "array_of_pa": [
            {
                "b_array": [
                    true,
                    false,
                    true
                ],
                "bi_array": [
                    "encoding=base64;SA==",
                    "encoding=base64;SA==",
                    "encoding=base64;SA=="
                ],
                "d_array": [
                    123.3434,
                    345.223,
                    -532.123
                ],
                "dt_array": [
                    "2017-01-14T00:00:00.000",
                    "2018-01-13T00:00:00.000",
                    "2019-01-11T00:00:00.000"
                ],
                "dtUtc_array": [
                    "2017-01-17T00:00:00.000Z",
                    "2018-01-11T00:00:00.000Z",
                    "2019-01-10T00:00:00.000Z"
                ],
                "i_array": [
                    3842,
                    -4923,
                    8291
                ],
                "l_array": [
                    384242.0,
                    -234923.0,
                    528291.0
                ],
                "s_array": [
                    "Bentley",
                    "System"
                ],
                "p2d_array": [
                    {
                    "X": 22.33,
                    "Y": -81.17
                    },
                    {
                    "X": -42.74,
                    "Y": 16.29
                    },
                    {
                    "X": 77.45,
                    "Y": -32.98
                    }
                ],
                "p3d_array": [
                    {
                    "X": 84.13,
                    "Y": 99.23,
                    "Z": -121.75
                    },
                    {
                    "X": -90.34,
                    "Y": 45.75,
                    "Z": -452.34
                    },
                    {
                    "X": -12.54,
                    "Y": -84.23,
                    "Z": -343.45
                    }
                ],
                "geom_array": [
                    {
                    "lineSegment": [
                        [
                            0,
                            0,
                            0
                        ],
                        [
                            4,
                            2.1,
                            1.2
                        ]
                    ]
                    },
                    {
                    "lineSegment": [
                        [
                            0,
                            0,
                            0
                        ],
                        [
                            1.1,
                            2.5,
                            4.2
                        ]
                    ]
                    },
                    {
                    "lineSegment": [
                        [
                            0,
                            0,
                            0
                        ],
                        [
                            9.1,
                            3.6,
                            3.8
                        ]
                    ]
                    }
                ]
            },
            {
                "b_array": [
                    true,
                    false,
                    true
                ],
                "bi_array": [
                    "encoding=base64;SA==",
                    "encoding=base64;SA==",
                    "encoding=base64;SA=="
                ],
                "d_array": [
                    123.3434,
                    345.223,
                    -532.123
                ],
                "dt_array": [
                    "2017-01-14T00:00:00.000",
                    "2018-01-13T00:00:00.000",
                    "2019-01-11T00:00:00.000"
                ],
                "dtUtc_array": [
                    "2017-01-17T00:00:00.000Z",
                    "2018-01-11T00:00:00.000Z",
                    "2019-01-10T00:00:00.000Z"
                ],
                "i_array": [
                    3842,
                    -4923,
                    8291
                ],
                "l_array": [
                    384242.0,
                    -234923.0,
                    528291.0
                ],
                "s_array": [
                    "Bentley",
                    "System"
                ],
                "p2d_array": [
                    {
                    "X": 22.33,
                    "Y": -81.17
                    },
                    {
                    "X": -42.74,
                    "Y": 16.29
                    },
                    {
                    "X": 77.45,
                    "Y": -32.98
                    }
                ],
                "p3d_array": [
                    {
                    "X": 84.13,
                    "Y": 99.23,
                    "Z": -121.75
                    },
                    {
                    "X": -90.34,
                    "Y": 45.75,
                    "Z": -452.34
                    },
                    {
                    "X": -12.54,
                    "Y": -84.23,
                    "Z": -343.45
                    }
                ],
                "geom_array": [
                    {
                    "lineSegment": [
                        [
                            0,
                            0,
                            0
                        ],
                        [
                            4,
                            2.1,
                            1.2
                        ]
                    ]
                    },
                    {
                    "lineSegment": [
                        [
                            0,
                            0,
                            0
                        ],
                        [
                            1.1,
                            2.5,
                            4.2
                        ]
                    ]
                    },
                    {
                    "lineSegment": [
                        [
                            0,
                            0,
                            0
                        ],
                        [
                            9.1,
                            3.6,
                            3.8
                        ]
                    ]
                    }
                ]
            }
        ]
        })json");

    if ("check out instance reader with complex data") {
        auto& reader = m_ecdb.GetInstanceReader();
        auto pos = InstanceReader::Position(instKey.GetInstanceId(), instKey.GetClassId());
        ASSERT_EQ(true, reader.Seek(pos, [&](InstanceReader::IRowContext const& row) {
            EXPECT_STRCASEEQ(expected.Stringify(StringifyFormat::Indented).c_str(), row.GetJson().Stringify(StringifyFormat::Indented).c_str());
        }));
    }
    if ("test if we get similar result from instance rendered by $") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $ FROM ts.e_mix"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        BeJsDocument actual;
        actual.Parse(stmt.GetValueText(0));
        EXPECT_STRCASEEQ(expected.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }

    if ("test if we get similar result from instance rendered by $") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $ FROM ts.e_mix"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        BeJsDocument actual;
        actual.Parse(stmt.GetValueText(0));
        EXPECT_STRCASEEQ(expected.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }
    if ("test if we get similar result from instance rendered by $") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(
            m_ecdb, R"sql(
                SELECT
                    $->b,
                    $->bi,
                    $->d,
                    $->dt,
                    $->dtUtc,
                    $->i,
                    $->l,
                    $->s,
                    $->p2d,
                    $->p3d,
                    $->b_array,
                    $->bi_array,
                    $->d_array,
                    $->dt_array,
                    $->dtUtc_array,
                    $->i_array,
                    $->l_array,
                    $->s_array,
                    $->p2d_array,
                    $->p3d_array,
                    $->geom_array,
                    $->p,
                    $->pa,
                    $->array_of_p,
                    $->array_of_pa
                FROM ts.e_mix
            )sql"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        ASSERT_TRUE(stmt.GetColumnInfo(0).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        ASSERT_EQ(expected["b"].asBool(), stmt.GetValueBoolean(0));

        ASSERT_TRUE(stmt.GetColumnInfo(1).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        ASSERT_STRCASEEQ("Hello, World!", stmt.GetValueText(1));

        ASSERT_TRUE(stmt.GetColumnInfo(2).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        ASSERT_EQ(expected["d"].asDouble(), stmt.GetValueDouble(2));

        ASSERT_TRUE(stmt.GetColumnInfo(3).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        ASSERT_STRCASEEQ(expected["dt"].asCString(), stmt.GetValueDateTime(3).ToString().c_str());

        ASSERT_TRUE(stmt.GetColumnInfo(4).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        ASSERT_STRCASEEQ(expected["dtUtc"].asCString(), stmt.GetValueDateTime(4).ToString().c_str());

        ASSERT_TRUE(stmt.GetColumnInfo(5).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(5).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        ASSERT_EQ(expected["i"].asInt(), stmt.GetValueInt64(5));

        ASSERT_TRUE(stmt.GetColumnInfo(6).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(6).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        ASSERT_EQ(expected["l"].asInt64(), stmt.GetValueInt64(6));

        ASSERT_TRUE(stmt.GetColumnInfo(7).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(7).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        ASSERT_STRCASEEQ(expected["s"].asCString(), stmt.GetValueText(7));

        ASSERT_TRUE(stmt.GetColumnInfo(8).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(8).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        ASSERT_STRCASEEQ(stmt.GetColumnInfo(8).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "json");
        ASSERT_STRCASEEQ(expected["p2d"].Stringify().c_str(), stmt.GetValueText(8));

        ASSERT_TRUE(stmt.GetColumnInfo(9).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(9).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        ASSERT_STRCASEEQ(stmt.GetColumnInfo(9).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "json");
        ASSERT_STRCASEEQ(expected["p3d"].Stringify().c_str(), stmt.GetValueText(9));

        ASSERT_TRUE(stmt.GetColumnInfo(10).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(10).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        ASSERT_STRCASEEQ(stmt.GetColumnInfo(10).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "json");
        ASSERT_STRCASEEQ(expected["b_array"].Stringify().c_str(), stmt.GetValueText(10));

        ASSERT_TRUE(stmt.GetColumnInfo(11).GetDataType().IsPrimitive());
        ASSERT_EQ(stmt.GetColumnInfo(11).GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        ASSERT_STRCASEEQ(stmt.GetColumnInfo(11).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "json");
        ASSERT_STRCASEEQ(expected["bi_array"].Stringify().c_str(), stmt.GetValueText(11));


        // BeJsDocument actual;
        // actual.Parse(stmt.GetValueText(0));
        // EXPECT_STRCASEEQ(expected.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }
}

END_ECDBUNITTESTS_NAMESPACE
