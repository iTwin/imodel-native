/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "NestedStructArrayTestSchemaHelper.h"
#include <BeRapidJson/BeRapidJson.h>
#include <Bentley/Base64Utilities.h>
#include <algorithm>
#include <cmath>
#include <regex>
#include <set>
USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlTemplate final {
   public:
    enum class Tags : uint64_t {
        NONE = 0,
        USE_DOT_ACCESSOR = 1,
    };

   private:
    Utf8String m_template;
    Tags m_tags = Tags::NONE;
    int m_lineNo = 0;

   public:
    ECSqlTemplate(Utf8CP templateStr) : m_template(templateStr) {}
    ECSqlTemplate(Utf8CP templateStr, Tags tag, int lineNo) : m_template(templateStr), m_tags(tag), m_lineNo(lineNo) {}
    ECSqlTemplate(const ECSqlTemplate& other) : m_template(other.m_template), m_tags(other.m_tags), m_lineNo(other.m_lineNo) {}
    ECSqlTemplate(ECSqlTemplate&& other) : m_template(std::move(other.m_template)), m_tags(std::move(other.m_tags)), m_lineNo(std::move(other.m_lineNo)) {}
    ECSqlTemplate& operator=(const ECSqlTemplate& other) {
        m_template = other.m_template;
        m_tags = other.m_tags;
        m_lineNo = other.m_lineNo;
        return *this;
    }
    ECSqlTemplate& operator=(ECSqlTemplate&& other) {
        m_template = std::move(other.m_template);
        m_tags = std::move(other.m_tags);
        m_lineNo = std::move(other.m_lineNo);
        return *this;
    }
    Utf8String GetTemplate() const { return m_template; }
    std::vector<Utf8String> GetParams() const {
        std::vector<Utf8String> out;
        std::regex pattern(R"(\$[\w]+)");
        std::sregex_iterator begin(m_template.begin(), m_template.end(), pattern);
        std::sregex_iterator end;
        for (std::sregex_iterator i = begin; i != end; ++i) {
            std::smatch match = *i;
            if (std::find(out.begin(), out.end(), match.str()) == out.end()) {
                out.push_back(match.str());
            }
        }
        return out;
    }
    int GetLineNo() const { return m_lineNo; }
    Utf8String Format(std::map<Utf8String, Utf8String> const& params) const {
        Utf8String query = m_template;
        std::vector<Utf8String> sortedParam;
        for (auto const& param : params) {
            sortedParam.push_back(param.first);
        }

        std::sort(sortedParam.begin(), sortedParam.end());
        std::reverse(sortedParam.begin(), sortedParam.end());
        for (auto const& param : sortedParam) {
            auto it = params.find(param);
            if (it == params.end()) {
                throw std::runtime_error("Parameter not found");
            }
            query.ReplaceAll(param.c_str(), it->second.c_str());
        }

        if (query.find('$') != Utf8String::npos) {
            throw std::runtime_error("Not all parameters were replaced");
        }
        return query;
    }
    Utf8String GetDebugString() const {
        return m_template + ": (" __FILE__ " declared at line: " + std::to_string(m_lineNo) + ")";
    }
    Tags GetTags() const { return m_tags; }
    bool HasTag(Tags tag) const { return (static_cast<uint64_t>(m_tags) & static_cast<uint64_t>(tag)) == static_cast<uint64_t>(tag); }
    void AddTag(Tags tag) { m_tags = static_cast<Tags>(static_cast<uint64_t>(m_tags) | static_cast<uint64_t>(tag)); }
    void RemoveTag(Tags tag) { m_tags = static_cast<Tags>(static_cast<uint64_t>(m_tags) & ~static_cast<uint64_t>(tag)); }
};
ENUM_IS_FLAGS(ECSqlTemplate::Tags);

struct ECSqlTemplates final {
   public:
    static std::vector<ECSqlTemplate> GetTemplates() {
        static std::vector<ECSqlTemplate> out;
        if (out.empty()) {
            auto add = [&](Utf8CP templateStr, int lineNo, ECSqlTemplate::Tags tags = ECSqlTemplate::Tags::NONE) {
                if (std::find_if(out.begin(), out.end(), [&](ECSqlTemplate const& t) { return t.GetTemplate() == templateStr; }) != out.end()) {
                    throw std::runtime_error("Duplicate template");
                }
                out.emplace_back(templateStr, tags, lineNo);
            };

            add(R"sql(SELECT (SELECT $p FROM $c LIMIT 1) )sql", __LINE__);
            add(R"sql(SELECT (SELECT t.$p FROM $c t LIMIT 1) m )sql", __LINE__);
            add(R"sql(SELECT (WITH x(c) AS (SELECT $p FROM $c LIMIT 1) SELECT c FROM x))sql", __LINE__);
            add(R"sql(SELECT (WITH x(c) AS (SELECT t.$p FROM $c t LIMIT 1) SELECT h.c FROM x h))sql", __LINE__);
            add(R"sql(SELECT * FROM (SELECT $p FROM $c))sql", __LINE__);
            add(R"sql(SELECT * FROM (SELECT $p m FROM $c))sql", __LINE__);
            add(R"sql(SELECT $p FROM (SELECT $p FROM $c))sql", __LINE__);
            add(R"sql(SELECT $p FROM $c)sql", __LINE__);
            add(R"sql(SELECT m FROM (SELECT $p m FROM $c))sql", __LINE__);
            add(R"sql(SELECT y.* FROM (SELECT $p FROM $c) y)sql", __LINE__);
            add(R"sql(SELECT y.* FROM (SELECT $p m FROM $c) y)sql", __LINE__);
            add(R"sql(SELECT y.* FROM (SELECT z.$p FROM $c z) y)sql", __LINE__);
            add(R"sql(SELECT y.* FROM (SELECT z.$p m FROM $c z) y)sql", __LINE__);
            add(R"sql(SELECT y.$p FROM (SELECT $p FROM $c) y)sql", __LINE__);
            add(R"sql(SELECT y.$p FROM (SELECT z.$p FROM $c z) y)sql", __LINE__);
            add(R"sql(SELECT y.m FROM (SELECT $p m FROM $c) y)sql", __LINE__);
            add(R"sql(SELECT y.m FROM (SELECT z.$p m FROM $c z) y)sql", __LINE__);
            add(R"sql(WITH x(c) AS (SELECT $p FROM $c LIMIT 1) SELECT * FROM x)sql", __LINE__);
            add(R"sql(WITH x(c) AS (SELECT $p FROM $c LIMIT 1) SELECT c FROM x)sql", __LINE__);
            add(R"sql(WITH x(c) AS (SELECT t.$p FROM $c t LIMIT 1) SELECT h.* FROM x h)sql", __LINE__);
            add(R"sql(WITH x(c) AS (SELECT t.$p FROM $c t LIMIT 1) SELECT h.c FROM x h)sql", __LINE__);
            add(R"sql(SELECT $p FROM $c UNION SELECT 100)sql", __LINE__);
        }
        return out;
    }
};

struct ECSqlRowAdaptorFixture : ECDbTestFixture {
   public:
    const char* expected_ECClassDef_Name = R"("ECDbMeta.ECClassDef")";
    const char* expected_ECClassDef_ClassId = R"("0x25")";
    const char* expected_SchemaOwnsClasses_Name = R"("ECDbMeta.SchemaOwnsClasses")";
    const char* expected_SchemaOwnsClasses_ClassId = R"("0x26")";
    const char* expected_ClassOwnsLocalProperties_Name = R"("ECDbMeta.ClassOwnsLocalProperties")";
    const char* expected_ClassOwnsLocalProperties_ClassId = R"("0x2b")";
    const char* expected_ECPropertyDef_Name = R"("ECDbMeta.ECPropertyDef")";
    const char* expected_ECPropertyDef_ClassId = R"("0x2c")";
    const char* expected_ECClassDef_ECInstanceId = R"("0x23")";
    const char* expected_ECClassDef_SchemaId = R"("0x1")";
    const char* expected_ClassOwnsLocalProperties_ECInstanceId = R"("0x1")";
    const char* expected_ClassOwnsLocalProperties_SourceECInstanceId = R"("0x1")";
    const char* expected_ClassOwnsLocalProperties_TargetECInstanceId = R"("0x1")";
    const char* expected_Schema_Value_Id = R"({"Id":"0x1","RelECClassId":"0x26"})";
    const char* expected_Schema_Value_Name = R"({"Id":"0x1","RelECClassId":"ECDbMeta.SchemaOwnsClasses"})";
    using Options = JsReadOptions;
    Utf8String GetValue(Utf8CP ecsql, Options options = Options()) {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << "Failed to prepare statement: " << ecsql;
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Failed to step statement: " << ecsql;

        ECSqlRowAdaptor adaptor(m_ecdb);
        adaptor.GetOptions() = options;
        BeJsDocument doc;
        EXPECT_EQ(SUCCESS, adaptor.RenderValue(doc, stmt.GetValue(0))) << "Failed to render value: " << ecsql;
        return doc.Stringify();
    }
};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlRowAdaptorFixture, default_class_id_rendering) {
    SetupECDb("test.ecdb");
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ECClassDef"}, {"$p", "ECClassId"}});
        EXPECT_STREQ(expected_ECClassDef_ClassId, GetValue(ecsql.c_str()).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "ECClassId"}});
        EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId, GetValue(ecsql.c_str()).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "SourceECClassId"}});
        EXPECT_STREQ(expected_ECClassDef_ClassId, GetValue(ecsql.c_str()).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "TargetECClassId"}});
        EXPECT_STREQ(expected_ECPropertyDef_ClassId, GetValue(ecsql.c_str()).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ECClassDef"}, {"$p", "Schema.RelECClassId"}});
        EXPECT_STREQ(expected_SchemaOwnsClasses_ClassId, GetValue(ecsql.c_str()).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlRowAdaptorFixture, class_id_rendering_with_class_ids_to_class_names) {
    SetupECDb("test.ecdb");
    Options opts;
    opts.SetConvertClassIdsToClassNames(true);
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ECClassDef"}, {"$p", "ECClassId"}});
        EXPECT_STREQ(expected_ECClassDef_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "ECClassId"}});
        EXPECT_STREQ(expected_ClassOwnsLocalProperties_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "SourceECClassId"}});
        EXPECT_STREQ(expected_ECClassDef_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "TargetECClassId"}});
        EXPECT_STREQ(expected_ECPropertyDef_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ECClassDef"}, {"$p", "Schema.RelECClassId"}});
        EXPECT_STREQ(expected_SchemaOwnsClasses_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlRowAdaptorFixture, class_id_rendering_with_js_name_option) {
    SetupECDb("test.ecdb");
    Options opts;
    opts.SetUseJsNames(true);
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ECClassDef"}, {"$p", "ECClassId"}});
        EXPECT_STREQ(expected_ECClassDef_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "ECClassId"}});
        EXPECT_STREQ(expected_ClassOwnsLocalProperties_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "SourceECClassId"}});
        EXPECT_STREQ(expected_ECClassDef_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ClassOwnsLocalProperties"}, {"$p", "TargetECClassId"}});
        EXPECT_STREQ(expected_ECPropertyDef_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
    for (auto const& ecsqlTemplate : ECSqlTemplates::GetTemplates()) {
        const auto ecsql = ecsqlTemplate.Format({{"$c", "meta.ECClassDef"}, {"$p", "Schema.RelECClassId"}});
        EXPECT_STREQ(expected_SchemaOwnsClasses_Name, GetValue(ecsql.c_str(), opts).c_str())
            << ecsql << "template: " << ecsqlTemplate.GetDebugString();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlRowAdaptorFixture, Test) {
    SetupECDb("test.ecdb");

    Options options;
    options.SetConvertClassIdsToClassNames(true);

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_Name,
                 GetValue("SELECT ECClassId FROM meta.ClassOwnsLocalProperties", options).c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ECInstanceId,
                 GetValue("SELECT ECInstanceId FROM meta.ClassOwnsLocalProperties", options).c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_SourceECInstanceId,
                 GetValue("SELECT SourceECInstanceId FROM meta.ClassOwnsLocalProperties", options).c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_TargetECInstanceId,
                 GetValue("SELECT TargetECInstanceId FROM meta.ClassOwnsLocalProperties", options).c_str());

    EXPECT_STREQ(expected_ECClassDef_Name,
                 GetValue("SELECT ECClassId FROM meta.ECClassDef", options).c_str());

    EXPECT_STREQ(expected_ECClassDef_Name,
                 GetValue("SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties", options).c_str());

    EXPECT_STREQ(expected_ECClassDef_ECInstanceId,
                 GetValue("SELECT ECInstanceId FROM meta.ECClassDef", options).c_str());

    EXPECT_STREQ(expected_ECClassDef_SchemaId,
                 GetValue("SELECT Schema.Id FROM meta.ECClassDef", options).c_str());

    EXPECT_STREQ(expected_ECPropertyDef_Name,
                 GetValue("SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties", options).c_str());

    EXPECT_STREQ(expected_Schema_Value_Name,
                 GetValue("SELECT Schema FROM meta.ECClassDef", options).c_str());

    EXPECT_STREQ(expected_SchemaOwnsClasses_Name,
                 GetValue("SELECT Schema.RelECClassId FROM meta.ECClassDef", options).c_str());
}
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlRowAdaptorFixture, Test1) {
    SetupECDb("test.ecdb");

    // ECClassId
    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId FROM meta.ECClassDef) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT ECClassId FROM meta.ECClassDef) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId y FROM meta.ECClassDef) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT ECClassId y FROM meta.ECClassDef) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId FROM meta.ECClassDef) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT ECClassId FROM meta.ECClassDef) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId y FROM meta.ECClassDef) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT ECClassId y FROM meta.ECClassDef) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId FROM meta.ECClassDef) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT k.ECClassId FROM (SELECT z.ECClassId FROM meta.ECClassDef z) k").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId y FROM meta.ECClassDef) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT k.y FROM (SELECT z.ECClassId y FROM meta.ECClassDef z) k").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId FROM meta.ECClassDef) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT ECClassId FROM (SELECT ECClassId FROM meta.ECClassDef)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId y FROM meta.ECClassDef) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT y FROM (SELECT ECClassId y FROM meta.ECClassDef)").c_str());

    // Relationship ClassId

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT k.ECClassId FROM (SELECT z.ECClassId FROM meta.ClassOwnsLocalProperties z) k").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT k.y FROM (SELECT z.ECClassId y FROM meta.ClassOwnsLocalProperties z) k").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT ECClassId FROM (SELECT ECClassId FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT y FROM (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties)").c_str());

    // SourceECClassId

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT k.SourceECClassId FROM (SELECT z.SourceECClassId FROM meta.ClassOwnsLocalProperties z) k").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT k.y FROM (SELECT z.SourceECClassId y FROM meta.ClassOwnsLocalProperties z) k").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT SourceECClassId FROM (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT y FROM (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties)").c_str());

    // TargetECClassId

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT c FROM (WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT (WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT m FROM (WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c m FROM x z)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT k.TargetECClassId FROM (SELECT z.TargetECClassId FROM meta.ClassOwnsLocalProperties z) k").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT z.c FROM x z").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT k.y FROM (SELECT z.TargetECClassId y FROM meta.ClassOwnsLocalProperties z) k").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT TargetECClassId FROM (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT y FROM (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT ECClassId FROM (SELECT ECClassId FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("WITH x(c) AS (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT y FROM (SELECT ECClassId y FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT SourceECClassId FROM (SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT y FROM (SELECT SourceECClassId y FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT TargetECClassId FROM (SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("WITH x(c) AS (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT y FROM (SELECT TargetECClassId y FROM meta.ClassOwnsLocalProperties)").c_str());

    EXPECT_STREQ(expected_SchemaOwnsClasses_ClassId,
                 GetValue("WITH x(c) AS (SELECT Schema.RelECClassId FROM meta.ECClassDef) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_SchemaOwnsClasses_ClassId,
                 GetValue("SELECT x FROM (SELECT Schema.RelECClassId x FROM meta.ECClassDef)").c_str());

    EXPECT_STREQ(expected_SchemaOwnsClasses_ClassId,
                 GetValue("WITH x(c) AS (SELECT Schema.RelECClassId y FROM meta.ECClassDef) SELECT c FROM x").c_str());

    EXPECT_STREQ(expected_SchemaOwnsClasses_ClassId,
                 GetValue("SELECT y FROM (SELECT Schema.RelECClassId y FROM meta.ECClassDef)").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ClassId,
                 GetValue("SELECT ECClassId FROM meta.ClassOwnsLocalProperties").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_ECInstanceId,
                 GetValue("SELECT ECInstanceId FROM meta.ClassOwnsLocalProperties").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_SourceECInstanceId,
                 GetValue("SELECT SourceECInstanceId FROM meta.ClassOwnsLocalProperties").c_str());

    EXPECT_STREQ(expected_ClassOwnsLocalProperties_TargetECInstanceId,
                 GetValue("SELECT TargetECInstanceId FROM meta.ClassOwnsLocalProperties").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT ECClassId FROM meta.ECClassDef").c_str());

    EXPECT_STREQ(expected_ECClassDef_ClassId,
                 GetValue("SELECT SourceECClassId FROM meta.ClassOwnsLocalProperties").c_str());

    EXPECT_STREQ(expected_ECClassDef_ECInstanceId,
                 GetValue("SELECT ECInstanceId FROM meta.ECClassDef").c_str());

    EXPECT_STREQ(expected_ECClassDef_SchemaId,
                 GetValue("SELECT Schema.Id FROM meta.ECClassDef").c_str());

    EXPECT_STREQ(expected_ECPropertyDef_ClassId,
                 GetValue("SELECT TargetECClassId FROM meta.ClassOwnsLocalProperties").c_str());

    EXPECT_STREQ(expected_Schema_Value_Id,
                 GetValue("SELECT Schema FROM meta.ECClassDef").c_str());

    EXPECT_STREQ(expected_SchemaOwnsClasses_ClassId,
                 GetValue("SELECT Schema.RelECClassId FROM meta.ECClassDef").c_str());
}

END_ECDBUNITTESTS_NAMESPACE