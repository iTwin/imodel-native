/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/BuildInstanceAndSerializeToXML.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"


#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

struct BasicTest : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* Specify the ECSchema that contains all the class/struct definitions
* @bsimethod                                                    BillSteinbock   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaPtr       CreateTestSchema ()
    {
    wchar_t schemaXML[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"SampleDataClass\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"Name\"           typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"ID\"             typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"StartPoint\"     typeName=\"point3d\" />"
                    L"        <ECProperty propertyName=\"EndPoint\"       typeName=\"point3d\" />"
                    L"        <ECProperty propertyName=\"XYSize\"         typeName=\"point2d\" />"
                    L"        <ECProperty propertyName=\"Length\"         typeName=\"double\"  />"
                    L"        <ECArrayProperty propertyName=\"Readings\"  typeName=\"long\" />"
                    L"        <ECProperty propertyName=\"Field_Tested\"   typeName=\"boolean\"  />"
                    L"    </ECClass>"
                    L"</ECSchema>";

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema; 
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
    EXPECT_EQ (SUCCESS, status);  
    
    return schema;
    }

//=========================================================
// Native structure
//=========================================================
struct  SampleData
    {
    // struct data
    WString                  m_name;
    int                      m_id;
    DPoint3d                 m_startPoint;
    DPoint3d                 m_endPoint;
    DPoint2d                 m_xySize;
    double                   m_length;
    UInt32                   m_numReadings;
    Int64*                   m_readings;
    bool                     m_tested;

    SampleData (int id, UInt32 numReadings)
        {
        m_name.Sprintf (L"Sample_%d", id);
        m_id = id;
        m_numReadings = numReadings;

        if (numReadings > 0)
            {
            m_readings = (Int64*) calloc (m_numReadings, sizeof(Int64));
            for (UInt32 i=0; i<m_numReadings; i++)
                m_readings[i] = (Int64)i*100;     
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
        ECClassP ecClass = m_schema->GetClassP (L"SampleDataClass");
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
        instance->SetValue(L"Name",         ECValue (data.m_name.c_str()));
        instance->SetValue(L"ID",           ECValue (data.m_id));
        instance->SetValue(L"StartPoint",   ECValue (data.m_startPoint));
        instance->SetValue(L"EndPoint",     ECValue (data.m_endPoint));
        instance->SetValue(L"XYSize",       ECValue (data.m_xySize));
        instance->SetValue(L"Length",       ECValue (data.m_length));
        instance->SetValue(L"Field_Tested", ECValue (data.m_tested));

        if (data.m_numReadings > 0)
            {
            // add room for the array members and then set the values for each
            instance->AddArrayElements(L"Readings", data.m_numReadings);
            for (UInt32 i=0; i<data.m_numReadings; i++)
                instance->SetValue(L"Readings", ECValue (data.m_readings[i]), i);
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
    WString ecInstanceXml;

    for (int i=0; i<5; i++)
        {
        SampleData sampleData (i, (UInt32)(i%2)*3);
        ECN::StandaloneECInstancePtr testInstance = instanceManager.CreateInstance (sampleData);
        ASSERT_TRUE (testInstance.IsValid());

        InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, false);
        EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);
        }

    //CoUninitialize();
    }


END_BENTLEY_ECOBJECT_NAMESPACE
