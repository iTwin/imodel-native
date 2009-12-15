/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

BEGIN_BENTLEY_EC_NAMESPACE

using namespace std;

// WIP_FUSION: these verify methods are duplicated in DgnPlatformTest... how do we share that code?    
// WIP_FUSION: where is the right place to share these methods even among ECObjects tests? 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyString (InstanceR instance, ValueR v, wchar_t const * accessString, wchar_t const * value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_STREQ (value, v.GetString());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyString (InstanceR instance, ValueR v, wchar_t const * accessString, wchar_t const * value)
    {
    v.SetString(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyString (instance, v, accessString, value);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyInteger (InstanceR instance, ValueR v, wchar_t const * accessString, UInt32 value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetInteger());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyInteger (InstanceR instance, ValueR v, wchar_t const * accessString, UInt32 value)
    {
    v.SetInteger(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyInteger (instance, v, accessString, value);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyDouble (InstanceR instance, ValueR v, wchar_t const * accessString, double value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetDouble());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyDouble (InstanceR instance, ValueR v, wchar_t const * accessString, double value)
    {
    v.SetDouble(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyDouble (instance, v, accessString, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyLong (InstanceR instance, ValueR v, wchar_t const * accessString, UInt64 value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetLong());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyLong (InstanceR instance, ValueR v, wchar_t const * accessString, UInt64 value)
    {
    v.SetLong(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyLong (instance, v, accessString, value);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
void ExerciseInstance (InstanceR instance, wchar_t* valueForFinalStrings)
    {   
    Value v;
    
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
    SchemaPtr schema;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, Schema::CreateSchema (schema, L"TestSchema"));
    ClassP ecClass;
    schema->CreateClass(ecClass, L"TestClass");    
    ASSERT_TRUE (ecClass);
    
    StandaloneInstance instance(*ecClass);
    wstring instanceID = instance.GetInstanceID();
    
    ExerciseInstance (instance, L"Test");
    
    //instance.DumpInstanceData ();
    
    // instance.Compact()... then check values again
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MemoryLayoutTests, Values) // move it!
    {
    Value i(3);
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
    
    Value v;
    EXPECT_TRUE (v.IsUninitialized());
    EXPECT_TRUE (v.IsNull());
    
    double doubleValue = 1./3.;
    v.SetDouble(doubleValue);
    EXPECT_TRUE (v.IsDouble());
    EXPECT_EQ (doubleValue, v.GetDouble());
    
    Value nullInt = Value(EC::DATATYPE_Integer32);
    EXPECT_TRUE (nullInt.IsNull());
    EXPECT_TRUE (nullInt.IsInteger());

    Value long64 ((::Int64)3);
    EXPECT_TRUE (!long64.IsNull());
    EXPECT_TRUE (long64.IsLong());
    EXPECT_EQ (3, long64.GetLong());

    Value s(L"Hello");
    EXPECT_TRUE (s.IsString());
    EXPECT_TRUE (!s.IsNull());
    EXPECT_STREQ (L"Hello", s.GetString());
    const wstring ws = s.GetString();
    
    s.SetString(L"Nice one");
    EXPECT_STREQ (L"Nice one", s.GetString());
    
    s.SetString(NULL);
    EXPECT_TRUE (s.IsNull());
    EXPECT_TRUE (NULL == s.GetString());
    
    Value snull((wchar_t*)NULL);
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
    SchemaPtr schema;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, Schema::CreateSchema (schema, L"TestSchema"));
    ClassP ecClass;
    schema->CreateClass(ecClass, L"TestClass");    
    ASSERT_TRUE (ecClass);
        
    MemoryEnablerPtr enabler = StandaloneInstance::CreateEnabler (*ecClass);
    EC::StandaloneInstanceFactoryP factory = new StandaloneInstanceFactory (*enabler.get());
    
    EC::StandaloneInstanceP instance = NULL;
    EXPECT_TRUE (SUCCESS == factory->BeginInstanceConstruction (instance));
    
    Value v;
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());
    
    double doubleValue = 1.0/3.0;
    SetAndVerifyDouble (*instance, v, L"D", doubleValue);
    EXPECT_FALSE (v.IsNull());    
    
    v.SetToNull();
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"D", v));
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());
        
    SetAndVerifyString (*instance, v, L"S", L"Yo!");

    factory->FinishInstance(instance);
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());    
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"S"));
    EXPECT_FALSE (v.IsNull());     
    }
    
void SetStringToSpecifiedNumberOfCharacters (InstanceR instance, int nChars)
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
        
    Value v(string);
    EXPECT_TRUE (SUCCESS == instance.SetValue (L"S", v));
    }

TEST (MemoryLayoutTests, DemonstrateInstanceFactory)
    {
    SchemaPtr schema;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, Schema::CreateSchema (schema, L"TestSchema"));
    ClassP ecClass;
    schema->CreateClass(ecClass, L"TestClass");    
    ASSERT_TRUE (ecClass);
        
    MemoryEnablerPtr enabler = StandaloneInstance::CreateEnabler (*ecClass);

    EC::StandaloneInstanceP instance = NULL;
    
    UInt32 slack = 0;
    EC::StandaloneInstanceFactoryP factory = new StandaloneInstanceFactory (*enabler, slack);

    EXPECT_TRUE (SUCCESS == factory->BeginInstanceConstruction (instance));    
    SetStringToSpecifiedNumberOfCharacters (*instance, 1); // There is no headroom, so this tiny addition triggers a realloc
    EXPECT_EQ (0, factory->GetReallocationCount()); // Realloc not noticed until the instance is finished
    EXPECT_TRUE (SUCCESS == factory->FinishInstance(instance));
    EXPECT_EQ (1, factory->GetReallocationCount());
    
    EXPECT_TRUE (SUCCESS == factory->BeginInstanceConstruction (instance));
    EXPECT_EQ (1, factory->GetReallocationCount());
    SetStringToSpecifiedNumberOfCharacters (*instance, 10);
    EXPECT_TRUE (SUCCESS == factory->FinishInstance(instance));
    EXPECT_EQ (1, factory->GetReallocationCount()); // No new realloc, because it doubled its size when it realloced
    
    EXPECT_TRUE (SUCCESS == factory->BeginInstanceConstruction (instance));
    SetStringToSpecifiedNumberOfCharacters (*instance, 1000);  // This is big enough to trigger another realloc.
    EXPECT_TRUE (SUCCESS == factory->FinishInstance(instance));
    EXPECT_EQ (2, factory->GetReallocationCount()); // No new realloc, because it double its size when it realloced
    
    factory = new StandaloneInstanceFactory (*enabler.get(), slack, 4000);
    EXPECT_TRUE (SUCCESS == factory->BeginInstanceConstruction (instance));
    EXPECT_EQ (0, factory->GetReallocationCount()); 
    SetStringToSpecifiedNumberOfCharacters (*instance, 1000);
    EXPECT_TRUE (SUCCESS == factory->FinishInstance(instance));
    EXPECT_EQ (0, factory->GetReallocationCount()); // We made it big enough the first time


    // The factory only keeps one "under construction" at a time... and it tracks which one it is.
    EC::StandaloneInstanceP oldInstance = instance;    
    factory = new StandaloneInstanceFactory (*enabler.get(), slack, 2000);
    EXPECT_TRUE (SUCCESS == factory->BeginInstanceConstruction (instance));
    DISABLE_ASSERTS // Otherwise, the lines below would trigger assert.
    EXPECT_TRUE (SUCCESS != factory->BeginInstanceConstruction (oldInstance));
    EXPECT_TRUE (SUCCESS != factory->FinishInstance(oldInstance));    
    EXPECT_TRUE (SUCCESS == factory->FinishInstance(instance));    
    EXPECT_TRUE (SUCCESS != factory->FinishInstance(instance)); // It can only finish once
    }
    
END_BENTLEY_EC_NAMESPACE