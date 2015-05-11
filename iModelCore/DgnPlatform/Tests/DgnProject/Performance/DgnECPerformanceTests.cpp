/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/DgnECPerformanceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnECPerformanceTests.h"
#include "TestSchemaHelper.h"
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

double PerformanceDgnECTests::s_increment = 5.0;
double PerformanceDgnECTests::s_xCoord = 0.0;
double PerformanceDgnECTests::s_yCoord = 0.0;
double PerformanceDgnECTests::s_zCoord = 0.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PerformanceDgnECTests::CreateArbitraryElement (DgnElementPtr& out, DgnModelR model)
    {
    if (!model.Is3d())
        return ERROR; // What kind of model is this?!?

    DgnClassId elementClassId = DgnClassId(model.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement)); // Should be passed in from primary ECInstance...
    ElementHandlerP elementHandler = ElementHandler::FindHandler(model.GetDgnDb(), elementClassId);

    if (nullptr == elementHandler)
        return ERROR;

    DgnCategoryId categoryId = model.GetDgnDb().Categories().MakeIterator().begin().GetCategoryId(); // Do any categories exist? Test probably needs to add one...

    if (!categoryId.IsValid())
        return ERROR;

    DgnElementPtr element = elementHandler->Create(DgnElement::CreateParams(model, elementClassId, categoryId));

    if (!element.IsValid())
        return ERROR;

    GeometricElementP geomElement = element->ToGeometricElementP();

    if (nullptr == geomElement)
        return ERROR;

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geomElement);

    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(s_xCoord, s_yCoord, s_zCoord), DPoint3d::From(s_xCoord + s_increment, s_yCoord + s_increment, s_zCoord + s_increment))));

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElement))
        return ERROR;

    out = element;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceDgnECTests::RunInsertTests
(
ECSchemaR schema,
DgnDbTestDgnManager tdm
)
    {
    DgnModelP model = tdm.GetDgnModelP();
    ECClassP testClass = schema.GetClassP(TEST_CLASS_NAME);
    // We don't want to time the creation of the elements.  So we create them in one loop, and then insert instances
    // in a second loop
    bvector<IECInstancePtr> instances;
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ECValue value;
        FOR_EACH (ECPropertyCP ecProperty, testClass->GetProperties(true))
            {
            if (ecProperty->GetIsPrimitive ())
                {
                AssignRandomECValue (value, *ecProperty);
                instance->SetValue (ecProperty->GetName().c_str(), value);
                }
            }
        instances.push_back(instance);
        }

    wchar_t countName[256];
    BeStringUtilities::Snwprintf (countName, L"Inserting %d instances (total) (%ls schema)", TESTCLASS_INSTANCE_COUNT, schema.GetFullSchemaName().c_str());

    StopWatch totalInsertingStopwatch(countName, false);
    StopWatch instructionTimer("Single step", false);

    Utf8String inserting;
    inserting.Sprintf("Inserting %d instances (inserting) (%s schema)", TESTCLASS_INSTANCE_COUNT, Utf8String(schema.GetFullSchemaName().c_str()).c_str());
    Utf8String attaching;
    attaching.Sprintf("Creating and Inserting %d elements with primary instance attached", TESTCLASS_INSTANCE_COUNT);

    StopWatch insertingTimer(inserting.c_str(), false);
    StopWatch attachingTimer(attaching.c_str(), false);
    ECInstanceInserter inserter (*tdm.GetDgnProjectP(), *testClass);
    ASSERT_TRUE (inserter.IsValid ());

    totalInsertingStopwatch.Start();
    insertingTimer.Start();
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        ECInstanceKey ecInstanceKey;
        auto insertStatus = inserter.Insert (ecInstanceKey, *instances[i]);
        ASSERT_EQ (SUCCESS, insertStatus);
        }
    insertingTimer.Stop();

    DgnElementPtrVec elements;
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        StatusInt status;
        DgnElementPtr element;
        status = CreateArbitraryElement (element, *model);
        ASSERT_EQ(SUCCESS, status);
        elements.push_back (element);
        }

    attachingTimer.Start();
    for (int i = 0; i < TESTCLASS_INSTANCE_COUNT; i++)
        {
        DgnElementPtr element = elements[i];

#if defined (NEEDS_WORK_DGNITEM)
        // NEEDSWORK: primary ECInstance affects element class/element handler...should be supplied to CreateArbitraryElement???
        ECInstanceId ecInstanceId;
        ECInstanceIdHelper::FromString(ecInstanceId, instances[i]->GetInstanceId().c_str());
        StatusInt stat2 = DgnECPersistence::SetPrimaryInstanceOnElement (*elemHandle, ECInstanceKey (testClass->GetId(), ecInstanceId), *tdm.GetDgnProjectP());
        ASSERT_EQ (SUCCESS, stat2);
#endif
        DgnModelStatus modelStatus;
        model->GetDgnDb().Elements().Insert(element, &modelStatus);
        ASSERT_EQ (SUCCESS, modelStatus);
        }
    attachingTimer.Stop();
    totalInsertingStopwatch.Stop();
    PERFORMANCELOG.infov (L"Inserting %d instances (total): %.4lf", TESTCLASS_INSTANCE_COUNT, totalInsertingStopwatch.GetElapsedSeconds ());
    PERFORMANCELOG.infov (L"Inserting %d instances (inserting): %.4lf", TESTCLASS_INSTANCE_COUNT, insertingTimer.GetElapsedSeconds());
    PERFORMANCELOG.infov (L"Creating and Inserting %d elements with primary instance attached: %.4lf", TESTCLASS_INSTANCE_COUNT, attachingTimer.GetElapsedSeconds());

    bmap<Utf8String, double> results;
    Utf8String total(totalInsertingStopwatch.GetDescription().c_str());
    results[total] = totalInsertingStopwatch.GetElapsedSeconds();

    results[inserting] = insertingTimer.GetElapsedSeconds();

    results[attaching] = attachingTimer.GetElapsedSeconds();

    LogResultsToFile(results);
    tdm.GetDgnProjectP()->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceDgnECTests::RunQueryTests
(
ECSchemaR schema,
DgnDbTestDgnManager tdm
)
    {
    //DgnModelP model = tdm.GetDgnModelP();

    ECClassP testClass = schema.GetClassP(TEST_CLASS_NAME);

    ECSqlStatement statement;
    Utf8String query;
    query.Sprintf("SELECT * FROM %ls.%ls", schema.GetName().c_str(), TEST_CLASS_NAME);
    statement.Prepare (*tdm.GetDgnProjectP(), query.c_str());

    int count = 0;
    wchar_t findName[256];
    BeStringUtilities::Snwprintf (findName, L"Querying for %d instances (%ls schema)", TESTCLASS_INSTANCE_COUNT, schema.GetFullSchemaName().c_str());

    StopWatch stopwatch(findName, false);
    // WIP_ECSQL - this should all be replaced by the ECSqlInstanceAdapter when it is ready

    stopwatch.Start();
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        IECInstancePtr current = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

        for(int i=0; i < statement.GetColumnCount(); i++)
            {
            ECPropertyCP prop = statement.GetColumnInfo (i).GetProperty();
            BeAssert (prop != nullptr && "ColumnInfo::GetProperty can return nullptr.");

            ECN::ECValue val;
            if (prop->GetIsPrimitive ())
                {
                auto primitiveProp = prop->GetAsPrimitiveProperty ();
                switch (primitiveProp->GetType ())
                    {
                    case ECN::PRIMITIVETYPE_Integer:
                        {
                        auto intValue = statement.GetValueInt(i);
                        val.SetInteger(intValue);
                        break;
                        }
                    case ECN::PRIMITIVETYPE_String :
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
                    case ECN::PRIMITIVETYPE_Boolean : 
                        {
                        auto boolValue = statement.GetValueBoolean(i);
                        val.SetBoolean(boolValue);
                        break;
                        }
                    case ECN::PRIMITIVETYPE_Point2D : 
                        {
                        auto d = statement.GetValuePoint2D(i);
                        val.SetPoint2D(d);
                        break;
                        }
                    case ECN::PRIMITIVETYPE_Point3D : 
                        {
                        auto d = statement.GetValuePoint3D(i);
                        val.SetPoint3D(d);
                        break;
                        }
                    }
                }
            current->SetValue(prop->GetName().c_str(), val);
            }
        count++;
        }
    stopwatch.Stop();
    bmap<Utf8String, double> results;
    PERFORMANCELOG.infov("Found %d instances of class %ls:%ls in %.4lf seconds", count, schema.GetFullSchemaName().c_str(), TEST_CLASS_NAME, stopwatch.GetElapsedSeconds());
    results[Utf8String(stopwatch.GetDescription().c_str())] = stopwatch.GetElapsedSeconds();

    wchar_t countName[256];
    BeStringUtilities::Snwprintf (countName, L"Instance counts for %ls.%ls", schema.GetFullSchemaName().c_str(), TEST_CLASS_NAME);

    StopWatch countWatch(countName, false);
    query.Sprintf("SELECT Count(*) FROM %ls.%ls", schema.GetName().c_str(), TEST_CLASS_NAME);
    ECSqlStatement statement2;
    statement2.Prepare (*tdm.GetDgnProjectP(), query.c_str());

    countWatch.Start();
    while (statement2.Step() == ECSqlStepStatus::HasRow)
        {
        count = statement2.GetValueInt(0);
        }
    countWatch.Stop();
    PERFORMANCELOG.infov("Found %d instances of class %ls:%ls in %.4lf seconds", count, schema.GetFullSchemaName().c_str(), TEST_CLASS_NAME, countWatch.GetElapsedSeconds());
    results[Utf8String(countWatch.GetDescription().c_str())] = countWatch.GetElapsedSeconds();
    LogResultsToFile(results);
    }

TEST_F(PerformanceDgnECTests, InsertingAndQueryingInstances)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite);

    ECSchemaPtr schema;
    PerformanceTestFixture::ImportTestSchema (schema, tdm, 25, 25);
    RunInsertTests(*schema, tdm);
    RunQueryTests(*schema, tdm);
    }

TEST_F(PerformanceDgnECTests, InsertingAndQueryingInstancesWithComplexSchema)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite);

    ECSchemaPtr schema;
    PerformanceTestFixture::ImportComplexTestSchema (schema, tdm);
    RunInsertTests(*schema, tdm);
    //RunQueryTests(*schema, tdm);

    }
END_DGNDB_UNIT_TESTS_NAMESPACE
