/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/NonPublished/UnitsTestBase.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include "UnitsTestBase.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

void UnitsTestBase::SetUp()
    {
    m_testSchema = NULL;
    m_supplementedSchema = NULL;

    bvector<ECSchemaP> supplementalSchemas;
    InitializeUnits (L"testschema",supplementalSchemas);

    // Test that test schemas are not null
     ASSERT_TRUE(m_testSchema.IsValid())<<"Test setup failure: Domain Schema not loaded";
     ASSERT_TRUE (supplementalSchemas.size()!=0)<< "Test setup failure: Supplemental Schema not loaded";

    // Build Supplemented Schema
    SupplementedSchemaBuilder supplementedSchemaBuilder;

    supplementedSchemaBuilder.UpdateSchema (*m_supplementedSchema, supplementalSchemas);
    m_supplementedSchema->IsSupplemented();

    // SetUp Class and property variables
    InitClassAndPropertyVariables ();
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Initializes the units framework for test.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
void UnitsTestBase::InitializeUnits (WString testSchemaName, bvector< ECSchemaP > & testSupplementalSchemas)
    {
    if (0 == m_supplementalSchemas.size())
        {
        bvector<WString> searchPaths;
        searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
        SearchPathSchemaFileLocaterPtr schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
        ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
        schemaContext->AddSchemaLocater (*schemaLocater);
        SchemaKey key(testSchemaName.c_str(), 01, 00);
        
        m_testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);

        EXPECT_TRUE(m_testSchema.IsValid());
        EXPECT_FALSE(m_testSchema->ShouldNotBeStored());
        ECClassP ecClass=m_testSchema->GetClassP(L"Bike");
        ASSERT_TRUE (NULL != ecClass);


        schemaContext = ECSchemaReadContext::CreateContext();
        schemaContext->AddSchemaLocater (*schemaLocater);
        m_supplementedSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
        EXPECT_TRUE (m_supplementedSchema.IsValid());
        EXPECT_FALSE (m_supplementedSchema.get() == m_testSchema.get());

      //   Load Supplemental Schema
        SchemaKey suppKey (L"TestSupplementalSchema", 1, 0),
                  defKey  (L"TestUnitDefaults", 1, 0),
                  widthKey (L"WidthDefaults", 1, 0);

        m_supplementalSchemas.push_back ((schemaContext->LocateSchema(suppKey, SCHEMAMATCHTYPE_Latest)).get());
        m_supplementalSchemas.push_back ((schemaContext->LocateSchema(defKey, SCHEMAMATCHTYPE_Latest)).get());
        m_supplementalSchemas.push_back ((schemaContext->LocateSchema(widthKey, SCHEMAMATCHTYPE_Latest)).get());
        } 

    for (size_t i = 0; i < m_supplementalSchemas.size(); ++i)
        testSupplementalSchemas.push_back (m_supplementalSchemas[i].get());
    }

void UnitsTestBase::InitClassAndPropertyVariables ()
    {
    m_wheelClass                = m_supplementedSchema->GetClassP(L"Wheel");
    m_spokeLengthProp           = m_wheelClass->GetPropertyP (L"SpokeLength");
    m_AreaProp                  = m_wheelClass->GetPropertyP (L"Area");
    m_hubStructProp             = m_wheelClass->GetPropertyP(L"WheelHub");

    m_wheelsChildClass          = m_supplementedSchema->GetClassP(L"WheelsChild");
    m_wcDiameterProp            = m_wheelsChildClass->GetPropertyP(L"Diameter");
    m_wcWeightProp              = m_wheelsChildClass->GetPropertyP (L"Weight");

    m_BikeClass                 = m_supplementedSchema->GetClassP (L"Bike");
    
    m_frontWheelDiameterProp    = m_BikeClass->GetPropertyP (L"FrontWheelDiameter");
    m_frontWheelPressureProp    = m_BikeClass->GetPropertyP (L"FrontWheelPressure");
    m_rearWheelDiameterProp     = m_BikeClass->GetPropertyP (L"RearWheelDiameter");
    m_rearWheelPressureProp     = m_BikeClass->GetPropertyP (L"RearWheelPressure");
    m_trainingWheelDiameterProp = m_BikeClass->GetPropertyP (L"TrainingWheelDiameter");
    m_frameHeightProp           = m_BikeClass->GetPropertyP (L"FrameHeight");
    m_headSetAngleProp          = m_BikeClass->GetPropertyP (L"HeadSetAngle");
    m_seatPostAngleProp         = m_BikeClass->GetPropertyP (L"SeatPostAngle");

    m_standardUnitsClass        = m_supplementedSchema->GetClassP(L"StandardUnitsClass");
    m_sucAreaProp               = m_standardUnitsClass->GetPropertyP (L"Area");
    m_sucVolumeProp             = m_standardUnitsClass->GetPropertyP (L"Volume");
    m_sucTemperatureProp        = m_standardUnitsClass->GetPropertyP (L"Temperature");
    m_sucWidthProp              = m_standardUnitsClass->GetPropertyP (L"Width");
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Test.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
void UnitsTestBase::VerifyDefaultUnit (WString expectedUnitName, Unit defaultUnit)
    {
    ASSERT_TRUE (expectedUnitName.Equals (defaultUnit.GetName())) << "Expected " << expectedUnitName.c_str() << " Actual " << defaultUnit.GetName();
    }

END_BENTLEY_ECN_TEST_NAMESPACE
