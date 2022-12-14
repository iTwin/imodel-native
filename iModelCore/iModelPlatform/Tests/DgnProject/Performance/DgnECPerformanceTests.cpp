/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "DgnECPerformanceTests.h"
#include "TestSchemaHelper.h"
#include <ECDb/ECDbApi.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

double PerformanceDgnECTests::s_increment = 5.0;
double PerformanceDgnECTests::s_xCoord = 0.0;
double PerformanceDgnECTests::s_yCoord = 0.0;
double PerformanceDgnECTests::s_zCoord = 0.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PerformanceDgnECTests::CreateArbitraryElement(DgnElementPtr& out, DgnModelR model, DgnCategoryId categoryId, IECInstance* instance, DgnClassId classId)
    {
    if (!model.Is3d())
        return ERROR; // What kind of model is this?!?

    ElementHandlerP elementHandler = dgn_ElementHandler::Element::FindHandler(model.GetDgnDb(), classId);

    if (nullptr == elementHandler)
        return ERROR;

    DgnElementPtr element = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(model.GetDgnDb(), model.GetModelId(), DgnClassId(model.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), categoryId, Placement3d()));

    if (!element.IsValid())
        return ERROR;

    GeometrySourceP geomElement = element->ToGeometrySourceP();

    if (nullptr == geomElement)
        return ERROR;

    geomElement->SetCategoryId(categoryId);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*geomElement);

    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(s_xCoord, s_yCoord, s_zCoord), DPoint3d::From(s_xCoord + s_increment, s_yCoord + s_increment, s_zCoord + s_increment))));

    if (SUCCESS != builder->Finish(*geomElement))
        return ERROR;

    element->SetPropertyValues(*instance);
    out = element;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceDgnECTests::RunInsertTests(ECN::ECSchemaR schema, DgnDbR project, Utf8String testcaseName, Utf8String testName)
    {
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*project.Elements().GetRootSubject(), "DefaultModel");
    DgnModels& modelTable = project.Models();
    DgnModelId id = modelTable.QuerySubModelId(partitionCode);
    DgnModelPtr model = modelTable.GetModel(id);

    ECClassP testClass = schema.GetClassP(TEST_CLASS_NAME);
    // We don't want to time the creation of the elements.  So we create them in one loop, and then insert instances
    // in a second loop
    bvector<IECInstancePtr> instances;
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ECValue value;
        FOR_EACH(ECPropertyCP ecProperty, testClass->GetProperties(true))
            {
            if (ecProperty->GetIsPrimitive())
                {
                AssignRandomECValue(value, *ecProperty);
                instance->SetValue(ecProperty->GetName().c_str(), value);
                }
            }
        instances.push_back(instance);
        }

    Utf8String attaching;
    attaching.Sprintf("Creating and Inserting %d elements with primary instance attached", TESTCLASS_INSTANCE_COUNT);

    StopWatch attachingTimer(attaching.c_str(), false);
    //WIP All data modification must go through API. Must not use ECSQL or ECSQL adapters

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(model->GetDgnDb(), "TestCategory");
    EXPECT_TRUE(categoryId.IsValid());

    DgnClassId classId = model->GetDgnDb().Schemas().GetClassId(testClass->GetSchema().GetName(), testClass->GetName());
    DgnElementPtrVec elements;
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        StatusInt status;
        DgnElementPtr element;
        status = CreateArbitraryElement(element, *model, categoryId, instances[i].get(), classId);
        ASSERT_EQ(SUCCESS, status);
        elements.push_back(element);
        }

    attachingTimer.Start();
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        DgnElementPtr element = elements[i];

#if defined (NEEDS_WORK_DGNITEM)
        // NEEDSWORK: primary ECInstance affects element class/element handler...should be supplied to CreateArbitraryElement???
        ECInstanceId ecInstanceId;
        ECInstanceId::FromString(ecInstanceId, instances[i]->GetInstanceId().c_str());
        StatusInt stat2 = DgnECPersistence::SetPrimaryInstanceOnElement(*elemHandle, ECInstanceKey(testClass->GetId(), ecInstanceId), *tdm.GetDgnProjectP());
        ASSERT_EQ(SUCCESS, stat2);
#endif
        DgnDbStatus modelStatus;
        model->GetDgnDb().Elements().Insert(*element, &modelStatus);
        ASSERT_TRUE(DgnDbStatus::Success == modelStatus);
        }
    attachingTimer.Stop();

    PERFORMANCELOG.infov(L"Creating and Inserting %d elements with primary instance attached: %.4lf", TESTCLASS_INSTANCE_COUNT, attachingTimer.GetElapsedSeconds());
    LOGTODB(testcaseName.c_str(), testName.c_str(), attachingTimer.GetElapsedSeconds(), TESTCLASS_INSTANCE_COUNT, "Creating and Inserting elements with primary instance attached");


    bmap<Utf8String, double> results;
    results[attaching] = attachingTimer.GetElapsedSeconds();

    LogResultsToFile(results);
    project.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceDgnECTests::RunQueryTests(ECSchemaR schema, DgnDbR project, Utf8String testcaseName, Utf8String testName)
    {
    ECClassP testClass = schema.GetClassP(TEST_CLASS_NAME);

    ECSqlStatement statement;
    Utf8String query;
    query.Sprintf("SELECT * FROM %s.%s", schema.GetAlias().c_str(), TEST_CLASS_NAME);
    statement.Prepare(project, query.c_str());

    int count = 0;

    StopWatch timer(true);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        IECInstancePtr current = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

        for (int i = 0; i < statement.GetColumnCount(); i++)
            {
            ECPropertyCP prop = statement.GetColumnInfo(i).GetProperty();
            BeAssert(prop != nullptr && "ColumnInfo::GetProperty can return nullptr.");

            ECN::ECValue val;
            if (prop->GetIsPrimitive())
                {
                auto primitiveProp = prop->GetAsPrimitiveProperty();
                switch (primitiveProp->GetType())
                    {
                        case ECN::PRIMITIVETYPE_Integer:
                        {
                        auto intValue = statement.GetValueInt(i);
                        val.SetInteger(intValue);
                        break;
                        }
                        case ECN::PRIMITIVETYPE_String:
                        {
                        auto str = statement.GetValueText(i);
                        val.SetUtf8CP(str);
                        break;
                        }
                        case ECN::PRIMITIVETYPE_Long:
                        {
                        auto intValue = statement.GetValueInt64(i);
                        val.SetLong(intValue);
                        break;
                        }
                        case ECN::PRIMITIVETYPE_Double:
                        {
                        auto doubleValue = statement.GetValueDouble(i);
                        val.SetDouble(doubleValue);
                        break;
                        }
                        case ECN::PRIMITIVETYPE_Boolean:
                        {
                        auto boolValue = statement.GetValueBoolean(i);
                        val.SetBoolean(boolValue);
                        break;
                        }
                        case ECN::PRIMITIVETYPE_Point2d:
                        {
                        auto d = statement.GetValuePoint2d(i);
                        val.SetPoint2d(d);
                        break;
                        }
                        case ECN::PRIMITIVETYPE_Point3d:
                        {
                        auto d = statement.GetValuePoint3d(i);
                        val.SetPoint3d(d);
                        break;
                        }
                    }
                }
            current->SetValue(prop->GetName().c_str(), val);
            }
        count++;
        }
    timer.Stop();
    statement.Finalize();

    query.Sprintf("SELECT Count(*) FROM %s.%s", schema.GetName().c_str(), TEST_CLASS_NAME);
    statement.Prepare(project, query.c_str());
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(count, statement.GetValueInt(0));

    PERFORMANCELOG.infov("Found %d instances of class %s:%s in %.4lf seconds", count, schema.GetFullSchemaName().c_str(), TEST_CLASS_NAME, timer.GetElapsedSeconds());
    LOGTODB(testcaseName.c_str(), testName.c_str(), timer.GetElapsedSeconds(), count, Utf8PrintfString("Found Instance of class: %s ", TEST_CLASS_NAME).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceDgnECTests, InsertingAndQueryingInstances)
    {
    SetupSeedProject();

    ECSchemaPtr schema;
    PerformanceTestFixture::ImportTestSchema(schema, *m_db, 25, 25);
    RunInsertTests(*schema, *m_db, TEST_DETAILS);
    RunQueryTests(*schema, *m_db, TEST_DETAILS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceDgnECTests, InsertingAndQueryingInstancesWithComplexSchema)
    {
    SetupSeedProject();

    ECSchemaPtr schema;
    PerformanceTestFixture::ImportComplexTestSchema(schema, *m_db);
    RunInsertTests(*schema, *m_db, TEST_DETAILS);
    //RunQueryTests(*schema, *m_db, TEST_DETAILS);
    }

END_DGNDB_UNIT_TESTS_NAMESPACE
