/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <comdef.h>
#include "StopWatch.h"

BEGIN_BENTLEY_EC_NAMESPACE

using namespace std;

// WIP_FUSION: these verify methods are duplicated in DgnPlatformTest... how do we share that code?    
// WIP_FUSION: where is the right place to share these methods even among ECObjects tests? 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyString (IECInstanceR instance, ECValueR v, wchar_t const * accessString, wchar_t const * value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_STREQ (value, v.GetString());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyString (IECInstanceR instance, ECValueR v, wchar_t const * accessString, wchar_t const * value)
    {
    v.SetString(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyString (instance, v, accessString, value);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyInteger (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetInteger());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyInteger (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 value)
    {
    v.SetInteger(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyInteger (instance, v, accessString, value);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyDouble (IECInstanceR instance, ECValueR v, wchar_t const * accessString, double value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetDouble());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyDouble (IECInstanceR instance, ECValueR v, wchar_t const * accessString, double value)
    {
    v.SetDouble(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyDouble (instance, v, accessString, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyLong (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt64 value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetLong());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyLong (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt64 value)
    {
    v.SetLong(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyLong (instance, v, accessString, value);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring    GetTestSchemaXMLString (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor, const wchar_t* className)
    {
    wchar_t fmt[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"%s\" nameSpacePrefix=\"test\" version=\"%02d.%02d\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"%s\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"A\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"AA\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"B\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"C\" typeName=\"long\" />"
                    L"        <ECProperty propertyName=\"D\" typeName=\"double\" />"
                    L"        <ECProperty propertyName=\"S\" typeName=\"string\" />"
                    L"        <ECStructProperty propertyName=\"Manufacturer\" typeName=\"Manufacturer\" />"
                    L"        <ECProperty propertyName=\"Property0\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property3\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property4\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property5\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property6\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property7\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property8\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property9\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property10\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property11\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property12\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property13\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property14\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property15\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property16\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property17\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property18\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property19\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property20\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property21\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property22\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property23\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property24\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property25\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property26\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property27\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property28\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property29\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property30\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property31\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property32\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property33\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property34\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property35\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property36\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property37\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property38\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property39\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property40\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property41\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property42\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property43\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property44\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property45\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property46\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property47\" typeName=\"string\" />"
                    L"    </ECClass>"
                    L"</ECSchema>";

    wchar_t* buff = (wchar_t*) _alloca (2 * (50 + wcslen (fmt) + wcslen (schemaName) + wcslen (className)));

    swprintf (buff, fmt, schemaName, versionMajor, versionMinor, className);

    return buff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr       CreateTestSchema ()
    {
    std::wstring schemaXMLString = GetTestSchemaXMLString (L"TestSchema", 0, 0, L"TestClass");

    ECSchemaPtr schema = NULL;

    EXPECT_EQ (SUCCESS, ECSchema::ReadXmlFromString (schema, schemaXMLString.c_str()));

    return schema;
    }
    
typedef std::vector<std::wstring> NameVector;
static std::vector<std::wstring> s_propertyNames;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr       CreateProfilingSchema (int nStrings)
    {
    s_propertyNames.clear();
    
    std::wstring schemaXml = 
                    L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"ProfilingSchema\" nameSpacePrefix=\"p\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"Pidget\" isDomainClass=\"True\">";

    for (int i = 0; i < nStrings; i++)
        {
        wchar_t propertyName[32];
        swprintf(propertyName, L"StringProperty%02d", i);
        s_propertyNames.push_back (propertyName);
        wchar_t const * propertyFormat = 
                    L"        <ECProperty propertyName=\"%s\" typeName=\"string\" />";
        wchar_t propertyXml[128];
        swprintf (propertyXml, propertyFormat, propertyName);
        schemaXml += propertyXml;
        }                    

    schemaXml +=    L"    </ECClass>"
                    L"</ECSchema>";

    ECSchemaPtr schema = NULL;
    EXPECT_EQ (SUCCESS, ECSchema::ReadXmlFromString (schema, schemaXml.c_str()));

    return schema;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
void ExerciseInstance (IECInstanceR instance, wchar_t* valueForFinalStrings)
    {   
    ECValue v;
    
    StatusInt status = instance.GetValue (v, L"D");
    
    double doubleValue = 1.0/3.0;
    SetAndVerifyDouble (instance, v, L"D", doubleValue);

    SetAndVerifyInteger (instance, v, L"A", 97);
    SetAndVerifyInteger (instance, v, L"AA", 12);
    
    SetAndVerifyString (instance, v, L"B", L"Happy");
    SetAndVerifyString (instance, v, L"B", L"Very Happy");
    SetAndVerifyString (instance, v, L"B", L"sad");
    SetAndVerifyString (instance, v, L"S", L"Lucky");
    SetAndVerifyString (instance, v, L"B", L"sad");
    SetAndVerifyString (instance, v, L"B", L"Very Very Happy");
    VerifyString (instance, v, L"S", L"Lucky");
    SetAndVerifyString (instance, v, L"Manufacturer.Name", L"Charmed");
    SetAndVerifyString (instance, v, L"S", L"Lucky Strike");
        
    wchar_t largeString[3300];
    largeString[0] = L'\0';
    for (int i = 0; i < 20; i++)
        wcscat (largeString, L"S2345678901234567890123456789012");
    
    size_t len = wcslen(largeString);
    SetAndVerifyString (instance, v, L"S", largeString);
    
    for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
        {
        wchar_t propertyName[66];
        swprintf (propertyName, L"Property%i", i);
        SetAndVerifyString (instance, v, propertyName, valueForFinalStrings);
        }
           
    // Make sure that everything still has the final value that we expected
    VerifyString (instance, v, L"S", largeString);
    VerifyInteger (instance, v, L"A", 97);
    VerifyDouble  (instance, v, L"D", doubleValue);
    VerifyInteger (instance, v, L"AA", 12);
    VerifyString  (instance, v, L"B", L"Very Very Happy");
    VerifyString (instance, v, L"Manufacturer.Name", L"Charmed");
    for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
        {
        wchar_t propertyName[66];
        swprintf (propertyName, L"Property%i", i);
        VerifyString (instance, v, propertyName, valueForFinalStrings);
        }    
               
#if later        
    // array of ints
    UInt32 indices[1];
    for (int i = 0; i < 3; i++)
        {
        v.SetInteger(i + 1);
        indices[0] = i;
        EXPECT_TRUE (SUCCESS == instance.SetValue (L"ArrayOfInts[]", v, 1, indices));
        }

    for (int i = 0; i < 3; i++)
        {
        v.Clear();
        indices[0] = i;
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, L"ArrayOfInts[]", 1, indices));
        EXPECT_TRUE (i + 1 == v.GetInteger());
        }
#endif        
    }
                 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MemoryLayoutTests, InstantiateStandaloneInstance)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);
    
    SchemaLayout schemaLayout;
    schemaLayout.SetSchemaIndex(24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*classLayout);
    EC::StandaloneECInstanceFactoryP factory = new StandaloneECInstanceFactory (*classLayout);
    
    EC::StandaloneECInstanceP instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));    
    
    wstring instanceId = instance->GetInstanceId();
    
    ExerciseInstance (*instance, L"Test");
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction (instance));    
    
    instance->Dump();
    
    delete classLayout;

    // instance.Compact()... then check values again
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MemoryLayoutTests, Values) // move it!
    {
    ECValue i(3);
    EXPECT_TRUE (i.IsInteger());
    EXPECT_TRUE (!i.IsNull());
    EXPECT_EQ (3, i.GetInteger());
    i.SetInteger(4);
    EXPECT_EQ (4, i.GetInteger());
    
    i.SetString(L"Type changed to string");
    EXPECT_TRUE (i.IsString());
    EXPECT_TRUE (!i.IsNull());
    EXPECT_STREQ (L"Type changed to string", i.GetString());    
    
    i.Clear();
    EXPECT_TRUE (i.IsUninitialized());
    EXPECT_TRUE (i.IsNull());
    
    ECValue v;
    EXPECT_TRUE (v.IsUninitialized());
    EXPECT_TRUE (v.IsNull());
    
    double doubleValue = 1./3.;
    v.SetDouble(doubleValue);
    EXPECT_TRUE (v.IsDouble());
    EXPECT_EQ (doubleValue, v.GetDouble());
    
    ECValue nullInt = ECValue(EC::PRIMITIVETYPE_Integer);
    EXPECT_TRUE (nullInt.IsNull());
    EXPECT_TRUE (nullInt.IsInteger());

    ECValue long64 ((::Int64)3);
    EXPECT_TRUE (!long64.IsNull());
    EXPECT_TRUE (long64.IsLong());
    EXPECT_EQ (3, long64.GetLong());

    ECValue s(L"Hello");
    EXPECT_TRUE (s.IsString());
    EXPECT_TRUE (!s.IsNull());
    EXPECT_STREQ (L"Hello", s.GetString());
    const wstring ws = s.GetString();
    
    s.SetString(L"Nice one");
    EXPECT_STREQ (L"Nice one", s.GetString());
    
    s.SetString(NULL);
    EXPECT_TRUE (s.IsNull());
    EXPECT_TRUE (NULL == s.GetString());
    
    ECValue snull((wchar_t*)NULL);
    EXPECT_TRUE (snull.IsString());
    EXPECT_TRUE (snull.IsNull());
    wchar_t const * wcnull = snull.GetString();
    EXPECT_EQ (NULL, s.GetString());
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MemoryLayoutTests, TestSetGetNull)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);
        
    SchemaLayout schemaLayout;
    schemaLayout.SetSchemaIndex(24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*classLayout);
    EC::StandaloneECInstanceFactoryP factory = new StandaloneECInstanceFactory (*classLayout);
    
    EC::StandaloneECInstanceP instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    
    ECValue v;
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());
    
    double doubleValue = 1.0/3.0;
    SetAndVerifyDouble (*instance, v, L"D", doubleValue);
    EXPECT_TRUE (!v.IsNull());    
    
    v.SetToNull();
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"D", v));
    v.SetString(L"Just making sure that it is not NULL before calling GetValue in the next line.");
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());
        
    SetAndVerifyString (*instance, v, L"S", L"Yo!");

    factory->FinishConstruction(instance);
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());    
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"S"));
    EXPECT_FALSE (v.IsNull());     

    delete classLayout;

    CoUninitialize();
    }
    
void SetStringToSpecifiedNumberOfCharacters (IECInstanceR instance, int nChars)
    {
    wchar_t * string = (wchar_t *)alloca ((nChars + 1) * sizeof(wchar_t));
    string[0] = '\0';
    for (int i = 0; i < nChars; i++)
        {
        int digit = i % 10;
        wchar_t digitAsString[2];
        swprintf (digitAsString, L"%d", digit);
        wcscat (string, digitAsString);
        }
        
    ECValue v(string);
    EXPECT_TRUE (SUCCESS == instance.SetValue (L"S", v));
    }

TEST (MemoryLayoutTests, DemonstrateInstanceFactory)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);
        
    SchemaLayout schemaLayout;
    schemaLayout.SetSchemaIndex(24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*classLayout);

    EC::StandaloneECInstanceP instance = NULL;
    
    UInt32 slack = 0;
    EC::StandaloneECInstanceFactoryP factory = new StandaloneECInstanceFactory (*classLayout, slack);

    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));    
    SetStringToSpecifiedNumberOfCharacters (*instance, 1); // There is no headroom, so this tiny addition triggers a realloc
    EXPECT_EQ (0, factory->GetReallocationCount()); // Realloc not noticed until the instance is finished
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction(instance));
    EXPECT_EQ (1, factory->GetReallocationCount());
    
    instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    EXPECT_EQ (1, factory->GetReallocationCount());
    SetStringToSpecifiedNumberOfCharacters (*instance, 10);
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction(instance));
    EXPECT_EQ (1, factory->GetReallocationCount()); // No new realloc, because it doubled its size when it realloced
    
    instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    SetStringToSpecifiedNumberOfCharacters (*instance, 1000);  // This is big enough to trigger another realloc.
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction(instance));
    EXPECT_EQ (2, factory->GetReallocationCount()); // No new realloc, because it double its size when it realloced
    
    factory = new StandaloneECInstanceFactory (*classLayout, slack, 4000);
    instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    EXPECT_EQ (0, factory->GetReallocationCount()); 
    SetStringToSpecifiedNumberOfCharacters (*instance, 1000);
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction(instance));
    EXPECT_EQ (0, factory->GetReallocationCount()); // We made it big enough the first time

    // Test CancelConstruction
    instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    SetStringToSpecifiedNumberOfCharacters (*instance, 1000);
    EXPECT_TRUE (SUCCESS == factory->CancelConstruction(instance));
    EXPECT_EQ (NULL, instance);
    
    // Ensure we can continue to use the factory after a cancel
    instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    SetStringToSpecifiedNumberOfCharacters (*instance, 1000);
    EXPECT_TRUE (SUCCESS == factory->CancelConstruction(instance));
    
    // The factory only keeps one "under construction" at a time... and it tracks which one it is.
    EC::StandaloneECInstanceP oldInstance = instance;    
    factory = new StandaloneECInstanceFactory (*classLayout, slack, 2000);
    instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));
    DISABLE_ASSERTS // Otherwise, the lines below would trigger assert.
    EXPECT_TRUE (SUCCESS != factory->BeginConstruction (oldInstance));
    EXPECT_TRUE (SUCCESS != factory->FinishConstruction(oldInstance));    
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction(instance));    
    EXPECT_TRUE (SUCCESS != factory->FinishConstruction(instance)); // It can only finish once

    delete classLayout;

    CoUninitialize();
    }

void SetValuesForProfiling (StandaloneECInstanceR instance)
    {
    for (NameVector::const_iterator it = s_propertyNames.begin(); it != s_propertyNames.end(); ++it)
        instance.SetStringValue (it->c_str(), it->c_str());
    }
    
TEST (MemoryLayoutTests, ProfileSettingValues)
    {
    int nStrings = 100;
    int nInstances = 1000;
    
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateProfilingSchema(nStrings);
    ECClassP ecClass = schema->GetClassP (L"Pidget");
    ASSERT_TRUE (ecClass);
        
    SchemaLayout schemaLayout;
    schemaLayout.SetSchemaIndex(24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*classLayout);

    EC::StandaloneECInstanceP instance = NULL;
    
    UInt32 slack = 0;
    EC::StandaloneECInstanceFactoryP factory = new StandaloneECInstanceFactory (*classLayout, slack);

    EXPECT_TRUE (SUCCESS == factory->BeginConstruction (instance));    
    EXPECT_TRUE (SUCCESS == factory->FinishConstruction(instance));

    double elapsedSeconds = 0.0;
    StopWatch timer (L"Time setting of values in a new StandaloneECInstance", true);
    for (int i = 0; i < nInstances; i++)
        {
        timer.Start();
        SetValuesForProfiling (*instance);
        timer.Stop();
        
        elapsedSeconds += timer.GetElapsedSeconds();
        instance->ClearValues();
        }
    
    wprintf (L"  %d StandaloneECInstances with %d string properties initialized in %.4f seconds.\n", nInstances, nStrings, elapsedSeconds);
    }
    

END_BENTLEY_EC_NAMESPACE