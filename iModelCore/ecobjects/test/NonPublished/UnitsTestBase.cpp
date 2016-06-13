/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/UnitsTestBase.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    InitializeUnits ("testschema",supplementalSchemas);

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
void UnitsTestBase::InitializeUnits (Utf8String testSchemaName, bvector< ECSchemaP > & testSupplementalSchemas)
    {
    if (0 == m_supplementalSchemas.size())
        {
        bvector<WString> searchPaths;
        searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
        SearchPathSchemaFileLocaterPtr schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
        ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
        schemaContext->AddSchemaLocater (*schemaLocater);
        SchemaKey key(testSchemaName.c_str(), 01, 00);
        
        m_testSchema = schemaContext->LocateSchema(key, SchemaMatchType::Latest);//ECSchema::LocateSchema(key, *schemaContext);

        EXPECT_TRUE(m_testSchema.IsValid());
        EXPECT_FALSE(m_testSchema->ShouldNotBeStored());
        ECClassP ecClass=m_testSchema->GetClassP("Bike");
        ASSERT_TRUE (NULL != ecClass);


        schemaContext = ECSchemaReadContext::CreateContext();
        schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
        schemaContext->AddSchemaLocater (*schemaLocater);
        m_supplementedSchema = schemaContext->LocateSchema (key, SchemaMatchType::Latest);
        EXPECT_TRUE (m_supplementedSchema.IsValid());
        EXPECT_FALSE (m_supplementedSchema.get() == m_testSchema.get());

      //   Load Supplemental Schema
        SchemaKey suppKey ("TestSupplementalSchema", 1, 0),
                  defKey  ("TestUnitDefaults", 1, 0),
                  widthKey ("WidthDefaults", 1, 0);

        m_supplementalSchemas.push_back ((schemaContext->LocateSchema(suppKey, SchemaMatchType::Latest)).get());
        m_supplementalSchemas.push_back ((schemaContext->LocateSchema(defKey, SchemaMatchType::Latest)).get());
        m_supplementalSchemas.push_back ((schemaContext->LocateSchema(widthKey, SchemaMatchType::Latest)).get());
        } 

    for (size_t i = 0; i < m_supplementalSchemas.size(); ++i)
        testSupplementalSchemas.push_back (m_supplementalSchemas[i].get());
    }

void UnitsTestBase::InitClassAndPropertyVariables ()
    {
    m_wheelClass                = m_supplementedSchema->GetClassP("Wheel");
    m_spokeLengthProp           = m_wheelClass->GetPropertyP ("SpokeLength");
    m_AreaProp                  = m_wheelClass->GetPropertyP ("Area");
    m_hubStructProp             = m_wheelClass->GetPropertyP("WheelHub");

    m_wheelsChildClass          = m_supplementedSchema->GetClassP("WheelsChild");
    m_wcDiameterProp            = m_wheelsChildClass->GetPropertyP("Diameter");
    m_wcWeightProp              = m_wheelsChildClass->GetPropertyP ("Weight");

    m_BikeClass                 = m_supplementedSchema->GetClassP ("Bike");
    
    m_frontWheelDiameterProp    = m_BikeClass->GetPropertyP ("FrontWheelDiameter");
    m_frontWheelPressureProp    = m_BikeClass->GetPropertyP ("FrontWheelPressure");
    m_rearWheelDiameterProp     = m_BikeClass->GetPropertyP ("RearWheelDiameter");
    m_rearWheelPressureProp     = m_BikeClass->GetPropertyP ("RearWheelPressure");
    m_trainingWheelDiameterProp = m_BikeClass->GetPropertyP ("TrainingWheelDiameter");
    m_frameHeightProp           = m_BikeClass->GetPropertyP ("FrameHeight");
    m_headSetAngleProp          = m_BikeClass->GetPropertyP ("HeadSetAngle");
    m_seatPostAngleProp         = m_BikeClass->GetPropertyP ("SeatPostAngle");

    m_standardUnitsClass        = m_supplementedSchema->GetClassP("StandardUnitsClass");
    m_sucAreaProp               = m_standardUnitsClass->GetPropertyP ("Area");
    m_sucVolumeProp             = m_standardUnitsClass->GetPropertyP ("Volume");
    m_sucTemperatureProp        = m_standardUnitsClass->GetPropertyP ("Temperature");
    m_sucWidthProp              = m_standardUnitsClass->GetPropertyP ("Width");
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Test.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
void UnitsTestBase::VerifyDefaultUnit (Utf8String expectedUnitName, Unit defaultUnit)
    {
    ASSERT_TRUE (expectedUnitName.Equals (defaultUnit.GetName())) << "Expected " << expectedUnitName.c_str() << " Actual " << defaultUnit.GetName();
    }

END_BENTLEY_ECN_TEST_NAMESPACE
