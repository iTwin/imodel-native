/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/BuildInstanceAndSerializeToXML.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/SchemaLocalizedStrings.h>
#include <ECObjects/ECValue.h>
#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

using namespace std;
using namespace BentleyApi::ECN;

struct BasicTest : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* Specify the ECSchema that contains all the class/struct definitions
* @bsimethod                                                    BillSteinbock   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaPtr       CreateTestSchema ()
    {
    Utf8Char schemaXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    "    <ECClass typeName=\"SampleDataClass\" isStruct=\"True\" isDomainClass=\"True\">"
                    "        <ECProperty propertyName=\"Name\"           typeName=\"string\" />"
                    "        <ECProperty propertyName=\"ID\"             typeName=\"int\" />"
                    "        <ECProperty propertyName=\"StartPoint\"     typeName=\"point3d\" />"
                    "        <ECProperty propertyName=\"EndPoint\"       typeName=\"point3d\" />"
                    "        <ECProperty propertyName=\"XYSize\"         typeName=\"point2d\" />"
                    "        <ECProperty propertyName=\"Length\"         typeName=\"double\"  />"
                    "        <ECArrayProperty propertyName=\"Readings\"  typeName=\"long\" />"
                    "        <ECProperty propertyName=\"Field_Tested\"   typeName=\"boolean\"  />"
                    "    </ECClass>"
                    "</ECSchema>";

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema; 
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);  
    
    return schema;
    }

//=========================================================
// Native structure
//=========================================================
struct  SampleData
    {
    // struct data
    Utf8String                  m_name;
    int                      m_id;
    DPoint3d                 m_startPoint;
    DPoint3d                 m_endPoint;
    DPoint2d                 m_xySize;
    double                   m_length;
    uint32_t                 m_numReadings;
    int64_t*                   m_readings;
    bool                     m_tested;

    SampleData (int id, uint32_t numReadings)
        {
        m_name.Sprintf ("Sample_%d", id);
        m_id = id;
        m_numReadings = numReadings;

        if (numReadings > 0)
            {
            m_readings = (int64_t*) calloc (m_numReadings, sizeof(int64_t));
            for (uint32_t i=0; i<m_numReadings; i++)
                m_readings[i] = (int64_t)i*100;     
            }
        else
            {
            m_readings = NULL;
            }

        m_length = id * 3.75;

        m_startPoint.x = id * 1.0;
        m_startPoint.y = id * 1.0;
        m_startPoint.z = id * 1.0;

        m_endPoint.x = id * 1.0 + 100.0;
        m_endPoint.y = id * 1.0 + 100.0;
        m_endPoint.z = id * 1.0 + 100.0;

        m_xySize.x = id * 0.5;
        m_xySize.y = id * 1.0 + 0.5;

        m_tested = (0 == id%2);   
        }

    ~SampleData ()
        {
        if (m_readings)
            free (m_readings);
        }
    };

//=========================================================
// SampleDataInstanceManager - holds schema and enabler that will be
// needed to create standalone instances. 
//=========================================================
struct  SampleDataInstanceManager
    {
    // Instance support
    StandaloneECEnablerPtr   m_enabler;
    ECSchemaPtr              m_schema;

    SampleDataInstanceManager ()
        {
        // create the test schema (the created schema is added to the schema cache m_schemaOwner
        m_schema = CreateTestSchema();

        // get the class that represents the native SampleData stuct and get the enabler that will be used to create instances
        ECClassP ecClass = m_schema->GetClassP ("SampleDataClass");
        if (ecClass != NULL)
            m_enabler = ecClass->GetDefaultStandaloneEnabler();
        else
            m_enabler = NULL;
        }

    // Method to populate a standalone instance from a struct
    StandaloneECInstancePtr CreateInstance(SampleData& data)
        {
        if (!m_enabler.IsValid())
            return NULL;

        // create an empty instance
        ECN::StandaloneECInstancePtr instance = m_enabler->CreateInstance();

        // populate the instance
        instance->SetValue("Name",         ECValue (data.m_name.c_str()));
        instance->SetValue("ID",           ECValue (data.m_id));
        instance->SetValue("StartPoint",   ECValue (data.m_startPoint));
        instance->SetValue("EndPoint",     ECValue (data.m_endPoint));
        instance->SetValue("XYSize",       ECValue (data.m_xySize));
        instance->SetValue("Length",       ECValue (data.m_length));
        instance->SetValue("Field_Tested", ECValue (data.m_tested));

        if (data.m_numReadings > 0)
            {
            // add room for the array members and then set the values for each
            instance->AddArrayElements("Readings", data.m_numReadings);
            for (uint32_t i=0; i<data.m_numReadings; i++)
                instance->SetValue("Readings", ECValue (data.m_readings[i]), i);
            }

        return instance;
        }
    };

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BillSteinbock   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTest, BuildInstanceAndSerializeToXML)
    {
//    EXPECT_EQ (S_OK, CoInitialize(NULL));  

    SampleDataInstanceManager instanceManager;
    Utf8String ecInstanceXml;

    for (int i=0; i<5; i++)
        {
        SampleData sampleData (i, (uint32_t)(i%2)*3);
        ECN::StandaloneECInstancePtr testInstance = instanceManager.CreateInstance (sampleData);
        ASSERT_TRUE (testInstance.IsValid());

        InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, false);
        EXPECT_EQ(InstanceWriteStatus::Success, status2);
        }

    //CoUninitialize();
    }


END_BENTLEY_ECN_TEST_NAMESPACE
