/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceElementsCRUDTests.h"
#define INFO_SQL R"(with 
	pr_application_id as (select 1 id, application_id from pragma_application_id()),
	pr_auto_vacuum as (select 1 id, case auto_vacuum when 0 then 'false' else 'true' end  auto_vacuum from pragma_auto_vacuum()),
	pr_cache_size as (select 1 id, cache_size from pragma_cache_size()),
	pr_collation_list as (select 1 id, json('[' || group_concat( '"' ||name ||'"',',') || ']') collation_list from pragma_collation_list()),
	pr_encoding as (select 1 id, encoding from pragma_encoding()), 
	pr_foreign_keys as (select 1 id, foreign_keys from pragma_foreign_keys()),
	pr_freelist_count as (select 1 id, freelist_count from pragma_freelist_count()),
	pr_journal_mode as (select 1 id, journal_mode from pragma_journal_mode()),
	pr_journal_size_limit as (select 1 id, journal_size_limit from pragma_journal_size_limit()),
	pr_legacy_file_format as (select 1 id, case legacy_file_format when 0 then 'true' else 'false' end legacy_file_format from pragma_legacy_file_format()),
	pr_max_page_count as (select 1 id, max_page_count from pragma_max_page_count()),
	pr_page_count as (select 1 id, page_count from pragma_page_count()),
	pr_page_size as (select 1 id, page_size from pragma_page_size()),
	pr_schema_version as (select 1 id, schema_version from pragma_schema_version()),
	pr_user_version as (select 1 id, user_version from pragma_user_version()),
	pr_writable_schema as (select 1 id, case writable_schema when 0 then 'false' else 'true' end writable_schema from pragma_writable_schema()), 
	ss_cell_size_check as (select 1 id, case cell_size_check when 0 then 'false' else 'true' end cell_size_check from pragma_cell_size_check()),
	ss_checkpoint_fullfsync as (select 1 id, case checkpoint_fullfsync when 0 then 'false' else 'true' end checkpoint_fullfsync from pragma_checkpoint_fullfsync()),
	ss_defer_foreign_keys as (select 1 id, case defer_foreign_keys when 0 then 'false' else 'true' end defer_foreign_keys from pragma_defer_foreign_keys()),
	ss_fullfsync as (select 1 id, case fullfsync  when 0 then 'false' else 'true' end fullfsync from pragma_fullfsync()),
	ss_ignore_check_constraints as (select 1 id, case ignore_check_constraints when 0 then 'false' else 'true' end ignore_check_constraints from pragma_ignore_check_constraints()),
	ss_query_only as (select 1 id, case query_only when 0 then 'false' else 'true' end query_only from pragma_query_only()),
	ss_read_uncommitted as (select 1 id, case read_uncommitted when 0 then 'false' else 'true' end read_uncommitted from pragma_read_uncommitted()),
	ss_recursive_triggers as (select 1 id, case recursive_triggers when 0 then 'false' else 'true' end recursive_triggers from pragma_recursive_triggers()),
	ss_reverse_unordered_selects as (select 1 id, case reverse_unordered_selects when 0 then 'false' else 'true' end reverse_unordered_selects from pragma_reverse_unordered_selects()),
	ss_secure_delete as (select 1 id, case secure_delete when 0 then 'false' else 'true' end secure_delete from pragma_secure_delete()),
	ss_automatic_index as (select 1 id, case automatic_index when 0 then 'false' else 'true' end automatic_index from pragma_automatic_index()),
	ss_busy_timeout as (select 1 id, timeout from pragma_busy_timeout()),
	ss_cache_spill as (select 1 id, cache_spill from pragma_cache_spill()),
	ss_data_version as (select 1 id, data_version from pragma_data_version()),
	ss_soft_heap_limit as (select 1 id, soft_heap_limit from pragma_soft_heap_limit()),
	ss_threads as (select 1 id, threads from pragma_threads()),
	ss_locking_mode as (select 1 id, locking_mode from pragma_locking_mode()),
	ss_synchronous as (select 1 id, case synchronous when 0 then 'off' when 1 then 'normal' when 2 then 'full' when 3 then 'extra' end synchronous from pragma_synchronous()),
	ss_temp_store as (select 1 id, case temp_store when 0 then 'default' when 1 then 'file' when 2 then 'memory' end temp_store from pragma_temp_store()) ,
	ss_compile_options as (select 1 id, json('[' || group_concat( '"' ||compile_options ||'"',',') || ']') compile_options from pragma_compile_options())
select 
	json_object(
	'application_id', application_id,
	'auto_vacuum', auto_vacuum,
	'cache_size', cache_size,
	'collation_list', json(collation_list),
	'encoding', encoding,
	'foreign_keys', foreign_keys,
	'freelist_count', freelist_count,
	'journal_mode', journal_mode,
	'journal_size_limit', journal_size_limit,
	'legacy_file_format', legacy_file_format,
	'max_page_count', max_page_count,
	'page_count', page_count,
	'page_size', page_size,
	'schema_version', schema_version,
	'user_version', user_version,
	'writable_schema', writable_schema,
	'cell_size_check', cell_size_check,
	'checkpoint_fullfsync', checkpoint_fullfsync,
	'defer_foreign_keys', defer_foreign_keys,
	'fullfsync', fullfsync,
	'ignore_check_constraints', ignore_check_constraints,
	'query_only', query_only,
	'read_uncommitted', read_uncommitted,
	'recursive_triggers', recursive_triggers,
	'reverse_unordered_selects', reverse_unordered_selects,
	'secure_delete', secure_delete,
	'automatic_index', automatic_index,
	'busy_timeout', timeout,
	'cache_spill', cache_spill,
	'data_version', data_version,
	'soft_heap_limit', soft_heap_limit,
	'threads', threads,
	'locking_mode', locking_mode,
	'synchronous', synchronous,
	'temp_store', temp_store,
	'compile_options', json(compile_options)) settings
from pr_application_id
	join pr_auto_vacuum using (id)
	join pr_cache_size using (id)
	join pr_encoding using (id)
	join pr_foreign_keys using (id)
	join pr_collation_list using (id)
	join pr_freelist_count using (id)
	join pr_journal_mode using (id)
	join pr_journal_size_limit using (id)
	join pr_legacy_file_format using (id)
	join pr_max_page_count using (id)
	join pr_page_count using (id)
	join pr_page_size using (id)
	join pr_schema_version using (id)
	join pr_user_version using (id)
	join pr_writable_schema using (id)
	join ss_cell_size_check using (id)
	join ss_checkpoint_fullfsync using (id)
	join ss_defer_foreign_keys using (id)
	join ss_fullfsync using (id)
	join ss_ignore_check_constraints using (id)
	join ss_query_only using (id)
	join ss_read_uncommitted using (id)
	join ss_recursive_triggers using (id)
	join ss_reverse_unordered_selects using (id)
	join ss_secure_delete using (id)
	join ss_automatic_index using (id)
	join ss_busy_timeout using (id)
	join ss_cache_spill using (id)
	join ss_data_version using (id)
	join ss_soft_heap_limit using(id)
	join ss_threads using(id)
	join ss_locking_mode using(id)
	join ss_synchronous using(id)
	join ss_temp_store using(id)
	join ss_compile_options using(id))"

Utf8String PerformanceElementsCRUDTestFixture::GetDbSettings() const
    {
    if (m_db.IsValid())
        {
        Statement stmt;
        stmt.Prepare(*m_db, INFO_SQL);
        stmt.Step();
        return Utf8String(stmt.GetValueText(0));
        }
    return "<null>";
    }
void PerformanceElementsCRUDTestFixture::ApplyPragmas(Db& db)
    {
    for (Utf8StringCR pragmaCmd : m_pragms)
        {
        //printf("%s\n", pragmaCmd.c_str());
        auto rc = db.ExecuteSql(pragmaCmd.c_str());
        if (BE_SQLITE_OK != rc)
            {
            printf("Error: %s\n", db.GetLastError().c_str());
            }
        ASSERT_EQ(BE_SQLITE_OK, rc) << "Failed to execute PRAGMA > " << pragmaCmd.c_str();
        }
    }
// Uncomment this if you want elapsed time of each test case logged to console in addition to the log file.
// #define PERF_ELEM_CRUD_LOG_TO_CONSOLE 1
//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SetUpTestDgnDb(WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount)
    {
    WString seedFileName;
    BeFileName seedFilePath;
    seedFileName.Sprintf(L"dgndb_ecsqlvssqlite_%d_%ls_seed%d.ibim", initialInstanceCount, WString(testClassName, BentleyCharEncoding::Utf8).c_str(), DateTime::GetCurrentTimeUtc().GetDayOfYear());
    BeTest::GetHost().GetOutputRoot(seedFilePath);
    seedFilePath.AppendToPath(BeFileName(BeTest::GetNameOfCurrentTestCase()));
    seedFilePath.AppendToPath(seedFileName.c_str());
    if (!seedFilePath.DoesPathExist())
        {
        SetupSeedProject(seedFileName.c_str());
        ASSERT_EQ(SchemaStatus::Success, PerfTestDomain::GetDomain().ImportSchema(*m_db));
        ASSERT_TRUE(m_db->IsDbOpen());
        ApplyPragmas(*m_db);
        CreateElementsAndInsert(initialInstanceCount, testClassName, "InitialInstances");        
        m_db->ExecuteSql("analyze");
        m_db->CloseDb();
        }

    BeFileName dgndbFilePath;
    BeTest::GetHost().GetOutputRoot(dgndbFilePath);
    dgndbFilePath.AppendToPath(destFileName);
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedFilePath, dgndbFilePath, false));
    DbResult status;
    m_db = DgnDb::OpenDgnDb(&status, dgndbFilePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ApplyPragmas(*m_db);
    EXPECT_EQ(DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(m_db.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool appendEllipse3d(GeometryBuilder& builder, double cx, double cy, double cz)
    {
    DEllipse3d ellipseData = DEllipse3d::From(cx, cy, cz,
                                              0, 0, 2,
                                              0, 3, 0,
                                              0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    return builder.Append(*ellipse);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
static bool appendSolidPrimitive(GeometryBuilder& builder, double dz, double radius)
    {
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    GeometricPrimitivePtr elmGeom3 = GeometricPrimitive::Create(*solidPrimitive);
    EXPECT_TRUE(elmGeom3.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::SolidPrimitive == elmGeom3->GetGeometryType());
    ISolidPrimitivePtr getAsSolid = elmGeom3->GetAsISolidPrimitive();
    EXPECT_TRUE(getAsSolid.IsValid());

    return builder.Append(*getAsSolid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
static bool useEllipse = true;
void PerformanceElementsCRUDTestFixture::AddGeometry(DgnElementPtr element) const
    {
    GeometrySourceP geomElem = element->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*element->GetModel(), geomElem->GetCategoryId(), DPoint3d::From(0.0, 0.0, 0.0));
    if (useEllipse)
        ASSERT_TRUE(appendEllipse3d(*builder, 1, 2, 3));
    else
        ASSERT_TRUE(appendSolidPrimitive(*builder, 3.0, 1.5));
    ASSERT_EQ(SUCCESS, builder->Finish(*geomElem));

    ASSERT_TRUE(geomElem->HasGeometry());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ExtendGeometry(DgnElementPtr element) const
    {
    GeometrySourceP geomElem = element->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*element->GetModel(), geomElem->GetCategoryId(), DPoint3d::From(0.0, 0.0, 0.0));
    if (useEllipse)
        ASSERT_TRUE(appendEllipse3d(*builder, 3, 2, 1));
    else
        ASSERT_TRUE(appendSolidPrimitive(*builder, 6.0, 3.0));

    ASSERT_EQ(SUCCESS, builder->Finish(*geomElem));
    ASSERT_TRUE(geomElem->HasGeometry());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementPropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElement - ";
    uint64_t longVal = 10000000LL;
    double doubleVal = -3.1416;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == element->SetPropertyValue("BaseStr", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("BaseLong", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("BaseDouble", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementSub1PropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElementSub1 - ";
    uint64_t longVal = 20000000LL;
    double doubleVal = 2.71828;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == SetPerfElementPropertyValues(element, update)) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub1Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub1Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub1Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementSub2PropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElementSub2 - ";
    uint64_t longVal = 30000000LL;
    double doubleVal = 1.414121;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == SetPerfElementSub1PropertyValues(element, update)) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub2Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub2Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub2Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementSub3PropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElementSub3 - ";
    uint64_t longVal = 40000000LL;
    double doubleVal = 1.61803398874;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == SetPerfElementSub2PropertyValues(element, update)) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub3Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub3Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub3Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPropertyValues(Utf8CP className, DgnElementPtr element, bool update) const
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME))
        return SetPerfElementPropertyValues(element, update);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME))
        return SetPerfElementSub1PropertyValues(element, update);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME))
        return SetPerfElementSub2PropertyValues(element, update);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME))
        return SetPerfElementSub3PropertyValues(element, update);

    return DgnDbStatus::BadElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       09/17
//+---------------+---------------+---------------+---------------+---------------+------
std::function<DgnDbStatus(Dgn::PhysicalElementPtr& element, bool)> PerformanceElementsCRUDTestFixture::SetPropertyValuesMethod(Utf8CP className) const
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME))
        return [&] (Dgn::PhysicalElementPtr& element, bool update) { return SetPerfElementPropertyValues(element, update); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME))
        return [&] (Dgn::PhysicalElementPtr& element, bool update) { return SetPerfElementSub1PropertyValues(element, update); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME))
        return [&] (Dgn::PhysicalElementPtr& element, bool update) { return SetPerfElementSub2PropertyValues(element, update); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME))
        return [&] (Dgn::PhysicalElementPtr& element, bool update) { return SetPerfElementSub3PropertyValues(element, update); };

    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                      Sam.Wilson                       01/17
//+---------------+---------------+---------------+---------------+---------------+------
Dgn::PhysicalElementPtr PerformanceElementsCRUDTestFixture::CreatePerfElement(Utf8CP className, DgnModelR targetModel, DgnCategoryId catId, DgnElementId parent, DgnClassId dgnClassId) const
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        return PerfElement::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        return PerfElementSub1::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        return PerfElementSub2::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        return PerfElementSub3::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME))
        return PerfElementCHBase::Create(*m_db, targetModel.GetModelId(), catId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME))
        return PerfElementCHSub1::Create(*m_db, targetModel.GetModelId(), catId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME))
        return PerfElementCHSub2::Create(*m_db, targetModel.GetModelId(), catId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME))
        return PerfElementCHSub3::Create(*m_db, targetModel.GetModelId(), catId);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Sam.Wilson                       01/17
//+---------------+---------------+---------------+---------------+---------------+------
std::function<PhysicalElementPtr(void)> PerformanceElementsCRUDTestFixture::CreatePerfElementMethod(Utf8CP className, DgnModelR targetModel, DgnCategoryId catId, DgnElementId parent, DgnClassId dgnClassId) const
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        return [&] () { return PerfElement::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        return [&] () { return PerfElementSub1::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        return [&] () { return  PerfElementSub2::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        return [&] () { return PerfElementSub3::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME))
        return [&] () { return PerfElementCHBase::Create(*m_db, targetModel.GetModelId(), catId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME))
        return [&] () { return PerfElementCHSub1::Create(*m_db, targetModel.GetModelId(), catId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME))
        return [&] () { return PerfElementCHSub2::Create(*m_db, targetModel.GetModelId(), catId); };
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME))
        return [&] () { return PerfElementCHSub3::Create(*m_db, targetModel.GetModelId(), catId); };
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan Khan                       09/167
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElementsAndInsert(int numInstances, Utf8CP className, Utf8CP modelName) const
    {
    ASSERT_TRUE(m_db != nullptr);
    PhysicalModelPtr targetModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, modelName);
    Utf8String categoryName;
    categoryName.Sprintf("%s_Category", modelName);
    DgnCategoryId catId = DgnDbTestUtils::InsertSpatialCategory(*m_db, categoryName.c_str());
    DgnDbStatus stat = DgnDbStatus::Success;
    std::function<PhysicalElementPtr(void)> createPerfElementMethod = CreatePerfElementMethod(className, *targetModel, catId);
    std::function<DgnDbStatus(Dgn::PhysicalElementPtr& element,bool)> setPropertyValuesMethod =SetPropertyValuesMethod(className);

    for (int i = 0; i < numInstances; i++)
        {
        Dgn::PhysicalElementPtr element = createPerfElementMethod();
        ASSERT_TRUE(element != nullptr);
        ASSERT_EQ(DgnDbStatus::Success, setPropertyValuesMethod(element, false));
        AddGeometry(element);
        element->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(element.IsValid());
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElements(int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8CP modelName) const
    {
    PhysicalModelPtr targetModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, modelName);
    Utf8String categoryName;
    categoryName.Sprintf("%s_Category", modelName);
    DgnCategoryId catId = DgnDbTestUtils::InsertSpatialCategory(*m_db, categoryName.c_str());

    bool addMultiAspect = false;

    for (int i = 0; i < numInstances; i++)
        {
        Dgn::PhysicalElementPtr element = CreatePerfElement(className, *targetModel, catId);
        ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element));
        ASSERT_TRUE(element != nullptr);

        AddGeometry(element);
        if (addMultiAspect)
            DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));

        elements.push_back(element);
        }
    ASSERT_EQ(numInstances, (int) elements.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSelectParams(DgnElementCR element)
    {
    if (0 != strcmp("PerfElement - InitValue", element.GetPropertyValueString("BaseStr").c_str()))
        return DgnDbStatus::ReadError;

    if (10000000 != element.GetPropertyValueUInt64("BaseLong"))
        return DgnDbStatus::ReadError;

    if (-3.1416 != element.GetPropertyValueDouble("BaseDouble"))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSub1SelectParams(DgnElementCR element)
    {
    if (DgnDbStatus::Success != VerifyPerfElementSelectParams(element))
        return DgnDbStatus::ReadError;;

    if (0 != strcmp("PerfElementSub1 - InitValue", element.GetPropertyValueString("Sub1Str").c_str()))
        return DgnDbStatus::ReadError;

    if (20000000 != element.GetPropertyValueUInt64("Sub1Long"))
        return DgnDbStatus::ReadError;

    if (2.71828 != element.GetPropertyValueDouble("Sub1Double"))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSub2SelectParams(DgnElementCR element)
    {
    if (DgnDbStatus::Success != VerifyPerfElementSub1SelectParams(element))
        return DgnDbStatus::ReadError;;
    
    if (0 != strcmp("PerfElementSub2 - InitValue", element.GetPropertyValueString("Sub2Str").c_str()))
        return DgnDbStatus::ReadError;
    
    if (30000000 != element.GetPropertyValueUInt64("Sub2Long"))
        return DgnDbStatus::ReadError;
    
    if (1.414121 != element.GetPropertyValueDouble("Sub2Double"))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSub3SelectParams(DgnElementCR element)
    {
    if ((DgnDbStatus::Success != VerifyPerfElementSub2SelectParams(element)) ||
        0 != strcmp("PerfElementSub3 - InitValue", element.GetPropertyValueString("Sub3Str").c_str()) ||
        (40000000 != element.GetPropertyValueUInt64("Sub3Long")) ||
        (1.61803398874 != element.GetPropertyValueDouble("Sub3Double")))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetPropertyValues(DgnElementCR element, Utf8CP className)
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        return VerifyPerfElementSelectParams(element);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        return VerifyPerfElementSub1SelectParams(element);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        return VerifyPerfElementSub2SelectParams(element);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        return VerifyPerfElementSub3SelectParams(element);

    return DgnDbStatus::BadElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
DgnElementId PerformanceElementsCRUDTestFixture::generateTimeBasedId(int counter)
    {
    uint64_t part1 = BeTimeUtilities::QueryMillisecondsCounter() << 12;
    uint64_t part2 = counter & 0xFFF;
    return DgnElementId(part1 + part2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
DgnElementId PerformanceElementsCRUDTestFixture::generateAlternatingBriefcaseId(int counter)
    {
    BeBriefcaseId briefcaseId((counter / 100) % 10 + 2);
    return DgnElementId(BeBriefcaseBasedId(briefcaseId, counter).GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiInsertTime(Utf8CP className, int initialInstanceCount, int opCount, bool setFederationGuid, int idStrategy)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"ElementApiInsert%ls_%d.ibim", wClassName.c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    testElements.reserve(opCount);
    CreateElements(opCount, className, testElements, "ElementApiInstances");
    ASSERT_EQ(opCount, (int) testElements.size());
    int i = 0;
    WaitForUserInputIfAny();
    StopWatch timer(true);
    for (DgnElementPtr& element : testElements)
        {
        // optionally allow FederationGuid to be set as part of the performance test
        if (setFederationGuid)
            element->SetFederationGuid(BeGuid(true));

        // optionally allow a different ID allocation strategy for performance comparison purposes
        if (1 == idStrategy)
            element->ForceElementIdForInsert(generateTimeBasedId(++i));
        else if (2 == idStrategy)
            element->ForceElementIdForInsert(generateAlternatingBriefcaseId(++i));

        DgnDbStatus stat = DgnDbStatus::Success;
        element->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Insert", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiSelectTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"ElementApiSelect%ls_%d.ibim", wClassName.c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    int minElemId = GetfirstElementId(className);
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);
    WaitForUserInputIfAny();
    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        ASSERT_TRUE(element != nullptr);
        ASSERT_EQ(DgnDbStatus::Success, GetPropertyValues(*element, className));
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Read", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiUpdateTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiUpdate%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    int minElemId = GetfirstElementId(className);
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    //First build dgnelements with modified Geomtry
    bvector<DgnElementPtr> elements;
    elements.reserve(opCount);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(id);
        ASSERT_TRUE(element != nullptr);

        ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element, true));

        ExtendGeometry(element);
        elements.push_back(element);
        }

    WaitForUserInputIfAny();
    //Now update and record time
    StopWatch timer(true);
    for (DgnElementPtr& element : elements)
        {
        DgnDbStatus stat = DgnDbStatus::Success;
        element->DgnElement::Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Update", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//
void PerformanceElementsCRUDTestFixture::ApiDeleteTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiDelete%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    int minElemId = GetfirstElementId(className);
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);
    //<<<<<=======================================
    //              Warm up cache
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        ASSERT_TRUE(element != nullptr);
        }
    //=========================================>>>>
    WaitForUserInputIfAny();
    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
        const DgnDbStatus stat = m_db->Elements().Delete(id);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }

    timer.Stop();
    WaitForUserInputIfAny();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Delete", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::LogTiming(StopWatch& timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const
    {
    Utf8CP noClassIdFilterStr = omitClassIdFilter ? "w/o ECClassId filter " : " ";

    Utf8String totalDescription;
    totalDescription.Sprintf("%s %s '%s' [Initial count: %d]", description, noClassIdFilterStr, testClassName, initialInstanceCount);
    Utf8String desc;
    desc.Sprintf("%s", description);
    int pos = desc.find("API");
    Utf8String opType = desc.substr(pos + 4);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, totalDescription.c_str(), totalDescription.c_str(), opType.ToUpper(), initialInstanceCount);
#ifdef PERF_ELEM_CRUD_LOG_TO_CONSOLE
    printf("%.8f %s\n", timer.GetElapsedSeconds(), totalDescription.c_str());
#endif
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
int  PerformanceElementsCRUDTestFixture::GetfirstElementId(Utf8CP className)
{
    uint64_t firstElemId = s_firstElementId;
    const DgnElementId id(firstElemId);
    DgnElementCPtr element = m_db->Elements().GetElement(id);
    if (!element.IsValid())
    {// Get the minimum Id from bis_Element table.
        Statement stat1;
        DgnClassId classId = m_db->Schemas().GetClassId(PTEST_SCHEMA_NAME, className);
        
        DbResult result = stat1.Prepare(*m_db, "SELECT min(Id) from bis_Element where ECClassId=?");
        stat1.BindId(1, classId);
        EXPECT_EQ(result, BE_SQLITE_OK);
        EXPECT_TRUE(stat1.IsPrepared());

        EXPECT_EQ(BE_SQLITE_ROW, stat1.Step());
        firstElemId = stat1.GetValueInt(0);
        const DgnElementId id2(firstElemId);
        DgnElementCPtr element2 = m_db->Elements().GetElement(id2);
        EXPECT_TRUE(element2.IsValid());
    }
    return firstElemId;
}


/*******************************************************Class Hierarchy For Performance Tests***********************************************************************************

---------------------------------------------------------------PerfElement(Str, Long, Double)
--------------------------------------------------------------------|
-------------------------------------------------------------PerfElementSub1(Sub1Str, Sub1Long, Sub1Double)
--------------------------------------------------------------------|
-------------------------------------------------------------PerfElementSub2(Sub2Str, Sub2Long, Sub2Double)
--------------------------------------------------------------------|
-------------------------------------------------------------PerfElementSub3(Sub3Str, Sub3Long, Sub3Double)

********************************************************************************************************************************************************************************/

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, InsertApi)
    {
    ApiInsertTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Sam.Wilson                      01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, InsertApiCH)
    {
    ApiInsertTime(PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, SelectApi)
    {
    ApiSelectTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiSelectTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiSelectTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiSelectTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, UpdateApi)
    {
    ApiUpdateTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiUpdateTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiUpdateTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiUpdateTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, DeleteApi)
    {
    ApiDeleteTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiDeleteTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiDeleteTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiDeleteTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

// Uncomment this to profile ElementLocksPerformanceTest
// #define PROFILE_ELEMENT_LOCKS_TEST 1
// Uncomment this to output timings of ElementLocksPerformanceTest runs
// #define PRINT_ELEMENT_LOCKS_TEST 1

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/16
//=======================================================================================
struct ElementLocksPerformanceTest : PerformanceElementsCRUDTestFixture
    {
    void TestInsert(bool asBriefcase, Utf8CP className, int numElems)
        {
        auto dbName = asBriefcase ? L"LocksBriefcase.ibim" : L"LocksRepository.ibim";
        SetUpTestDgnDb(dbName, className, 0);
        if (asBriefcase)
            TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

        bvector<DgnElementPtr> elems;
        elems.reserve(numElems);
        CreateElements(numElems, className, elems, "MyModel");

#ifdef PROFILE_ELEMENT_LOCKS_TEST
        printf("Attach profiler...\n");
        getchar();
#endif
        StopWatch timer(true);
        for (auto& elem : elems)
            {
            DgnDbStatus stat;
            elem->Insert(&stat);
            EXPECT_EQ(DgnDbStatus::Success, stat);
            }

        timer.Stop();
#ifdef PRINT_ELEMENT_LOCKS_TEST
        printf("%ls (%d): %f\n", dbName, m_db->GetBriefcaseId().GetValue(), timer.GetElapsedSeconds());
#endif

        m_db->SaveChanges();
        }

    void TestInsert(bool asBriefcase, int nElems) { TestInsert(asBriefcase, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, nElems); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert100) { TestInsert(false, 100); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert1000) { TestInsert(false, 1000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert10000) { TestInsert(false, 10000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert100) { TestInsert(true, 100); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert1000) { TestInsert(true, 1000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert10000) { TestInsert(true, 10000); }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct QueryCategoryIdPerformanceTest : PerformanceElementsCRUDTestFixture
{
    DgnSubCategoryId    m_subCategoryId;

    void Initialize();

    template<typename T> double TimeStatement(T& stmt, uint32_t nIterations, Utf8CP sql)
        {
        StopWatch timer(true);
        for (uint32_t i = 0; i < nIterations; i++)
            {
            stmt.BindId(1, m_subCategoryId);
            stmt.Step();
            stmt.Reset();
            }

        double elapsed = timer.GetCurrentSeconds();
#if defined(QUERY_CATEGORY_ID_OUTPUT_RESULTS)
        printf("%f seconds to execute %u iterations of statement: %s\n", elapsed, nIterations, sql);
#endif
        return elapsed;
        }

    double TimeSQLStatement(Utf8CP sql, uint32_t nIterations) { return TimeStatement(*m_db->GetCachedStatement(sql), nIterations, sql); }
    double TimeECSqlStatement(Utf8CP ecsql, uint32_t nIterations)
        {
        auto stmt = m_db->GetPreparedECSqlStatement(ecsql);
        double elapsed = TimeStatement(*stmt, nIterations, ecsql);

#if defined(QUERY_CATEGORY_ID_EXPLAIN_QUERY)
        printf("Native SQL: %s\n", stmt->GetNativeSql());
        auto exp1 = m_db->ExplainQuery(stmt->GetNativeSql(), false),
             exp2 = m_db->ExplainQuery(stmt->GetNativeSql(), true);

        printf("ExplainQuery(false) =>\n%s\n", exp1.c_str());
        printf("ExplainQuery(true) => \n%s\n", exp2.c_str());
#endif

        return elapsed;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryCategoryIdPerformanceTest::Initialize()
    {
    SetUpTestDgnDb(L"QueryCategoryIdsPerf.ibim", PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0);
    m_subCategoryId = DgnCategory::GetDefaultSubCategoryId(m_defaultCategoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryCategoryIdPerformanceTest, TimeStatements)
    {
    Initialize();

    constexpr uint32_t nIterations = 500000;
    TimeSQLStatement("SELECT ParentId from bis_Element WHERE Id=?", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM " BIS_SCHEMA(BIS_CLASS_SubCategory) " WHERE ECInstanceId=?", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM " BIS_SCHEMA(BIS_CLASS_SubCategory) " WHERE ECInstanceId=? ECSqlOptions NoECClassIdFilter", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM bis.Element WHERE ECInstanceId=?", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM bis.Element WHERE ECInstanceId=? ECSqlOptions NoECClassIdFilter", nIterations);
    }
