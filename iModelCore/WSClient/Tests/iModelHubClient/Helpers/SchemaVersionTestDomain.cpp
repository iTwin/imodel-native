/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "SchemaVersionTestDomain.h"

DEFINE_POINTER_SUFFIX_TYPEDEFS(TestElement)
DEFINE_REF_COUNTED_PTR(TestElement)
USING_NAMESPACE_BENTLEY_DGN


//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TestElement::TestElement(CreateParams const& params) : PhysicalElement(params)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
Dgn::DgnClassId TestElement::QueryClassId(DgnDbCR dgndb) { return DgnClassId(dgndb.Schemas().GetClassId(SCHEMA_VERSION_TEST_SCHEMA_NAME, TEST_ELEMENT_CLASS_NAME)); }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
RefCountedPtr<TestElement> TestElement::Create(Dgn::PhysicalModelR model, Dgn::DgnCategoryId categoryId, Dgn::DgnCode code)
    {
    return new TestElement(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId, Placement3d(), code));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr SchemaVersionTestDomain::InsertPhysicalModel(DgnDbR db, Utf8CP partitionName) const
    {
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
DgnCategoryId SchemaVersionTestDomain::InsertSpatialCategory(DgnDbR db, Utf8CP categoryName) const
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::Black());

    SpatialCategory category(db.GetDictionaryModel(), categoryName, DgnCategory::Rank::Application);
    SpatialCategoryCPtr persistentCategory = category.Insert(appearance);
    EXPECT_TRUE(persistentCategory.IsValid());

    return persistentCategory.IsValid() ? persistentCategory->GetCategoryId() : DgnCategoryId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
void SchemaVersionTestDomain::_OnSchemaImported(DgnDbR db) const
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, "SchemaVersionTest");
    if (codeSpec.IsValid())
        codeSpec->Insert();

    PhysicalModelPtr model = InsertPhysicalModel(db, "OnSchemaImportedPartition");
    BeAssert(model.IsValid());

    DgnCategoryId categoryId = InsertSpatialCategory(db, "SchemaVersionTestCategory");
    BeAssert(categoryId.IsValid());

    DgnCode code = CreateCode(db, "OnSchemaImportedElement");

    TestElementPtr el = TestElement::Create(*model, categoryId, code);
    TestElementCPtr cEl = db.Elements().Insert<TestElement>(*el);
    BeAssert(cEl.IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
SchemaVersionTestDomain::SchemaVersionTestDomain() : DgnDomain(SCHEMA_VERSION_TEST_SCHEMA_NAME, "Version Test Domain", 1)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
void SchemaVersionTestDomain::ClearHandlers()
    {
    m_handlers.clear();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
void SchemaVersionTestDomain::SetVersion(Utf8CP version)
    {
    m_relativePath.SetNameUtf8(Utf8PrintfString("ECSchemas\\" SCHEMA_VERSION_TEST_SCHEMA_NAME ".%s.ecschema.xml", version));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
void SchemaVersionTestDomain::Register(Utf8CP version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly)
    {
    SchemaVersionTestDomain::GetDomain().SetVersion(version);
    DgnDomains::RegisterDomain(SchemaVersionTestDomain::GetDomain(), isRequired, isReadonly);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
DgnCode SchemaVersionTestDomain::CreateCode(DgnDbR dgndb, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, "SchemaVersionTest", value);
    }

DOMAIN_DEFINE_MEMBERS(SchemaVersionTestDomain)
HANDLER_DEFINE_MEMBERS(TestElementHandler)
