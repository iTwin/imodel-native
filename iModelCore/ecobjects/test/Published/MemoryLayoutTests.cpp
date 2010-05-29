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
void VerifyString (IECInstanceR instance, ECValueR v, wchar_t const * accessString, bool useIndex, UInt32 index, wchar_t const * value)
    {
    v.Clear();
    if (useIndex)
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, index));
    else
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_STREQ (value, v.GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyString (IECInstanceR instance, ECValueR v, wchar_t const * accessString, wchar_t const * value)
    {
    return VerifyString (instance, v, accessString, false, 0, value);
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
void VerifyInteger (IECInstanceR instance, ECValueR v, wchar_t const * accessString, bool useIndex, UInt32 index, UInt32 value)
    {
    v.Clear();
    if (useIndex)
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, index));
    else
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetInteger());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyInteger (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 value)
    {
    return VerifyInteger (instance, v, accessString, false, 0, value);
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
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyArrayInfo (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 count, bool isFixedCount)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (count, v.GetArrayInfo().GetCount());
    EXPECT_EQ (isFixedCount, v.GetArrayInfo().IsFixedCount());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyOutOfBoundsError (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 index)
    {
    v.Clear();    
    // WIP_FUSION .. ERROR_InvalidIndex
    // CGM - instance.GetValue() is still returning a StatusInt, but the underlying method is now returning an ECObjectsStatus
    EXPECT_TRUE (ECOBJECTS_STATUS_IndexOutOfRange == instance.GetValue (v, accessString, index));
    EXPECT_TRUE (ECOBJECTS_STATUS_IndexOutOfRange == instance.SetValue (accessString, v, index));
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyStringArray (IECInstanceR instance, ECValueR v, wchar_t const * accessString, wchar_t const * value, UInt32 start, UInt32 count)
    {
    std::wstring incrementingString = value;
   
    for (UInt32 i=start ; i < start + count ; i++)        
        {
        incrementingString.append (L"X");
        VerifyString (instance, v, accessString, true, i, incrementingString.c_str());
        }
    }  
              
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyStringArray (IECInstanceR instance, ECValueR v, wchar_t const * accessString, wchar_t const * value, UInt32 count)
    {
    std::wstring incrementingString = value;
    for (UInt32 i=0 ; i < count ; i++)        
        {
        incrementingString.append (L"X");
        v.SetString(incrementingString.c_str());
        EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v, i));
        }
    
    VerifyStringArray (instance, v, accessString, value, 0, count);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyIntegerArray (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 baseValue, UInt32 start, UInt32 count)
    {       
    for (UInt32 i=start ; i < start + count ; i++)        
        {
        VerifyInteger (instance, v, accessString, true, i, baseValue++);
        }
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyIntegerArray (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 baseValue, UInt32 count)
    {
    for (UInt32 i=0 ; i < count ; i++)        
        {
        v.SetInteger(baseValue + i);        
        EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v, i));
        }
        
    VerifyIntegerArray (instance, v, accessString, baseValue, 0, count);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, wchar_t const * accessString, UInt32 start, UInt32 count, bool isNull)
    {
    for (UInt32 i = start ; i < start + count ; i++)    
        {
        v.Clear();
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, i));
        EXPECT_TRUE (isNull == v.IsNull());        
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring    GetTestSchemaXMLString (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor, const wchar_t* className)
    {
    wchar_t fmt[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"%s\" nameSpacePrefix=\"test\" version=\"%02d.%02d\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"EmptyClass\" isDomainClass=\"True\">"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"CadData\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"Name\"         typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Count\"        typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"StartPoint\"   typeName=\"point3d\" />"
                    L"        <ECProperty propertyName=\"EndPoint\"     typeName=\"point3d\" />"
                    L"        <ECProperty propertyName=\"Size\"         typeName=\"point2d\" />"
                    L"        <ECProperty propertyName=\"Length\"       typeName=\"double\"  />"
                    L"        <ECProperty propertyName=\"Install_Date\" typeName=\"dateTime\"  />"
                    L"        <ECProperty propertyName=\"Service_Date\" typeName=\"dateTime\"  />"
                    L"        <ECProperty propertyName=\"Field_Tested\" typeName=\"boolean\"  />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"%s\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"BeginningArray\" typeName=\"string\" />"
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
                    L"        <ECArrayProperty propertyName=\"FixedArrayFixedElement\" typeName=\"int\" minOccurs=\"10\" maxOccurs=\"10\"/>"                    
                    L"        <ECProperty propertyName=\"Property10\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property11\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property12\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property13\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property14\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property15\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property16\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property17\" typeName=\"string\" />"                    
                    L"        <ECArrayProperty propertyName=\"VariableArrayFixedElement\" typeName=\"int\"/>"
                    L"        <ECArrayProperty propertyName=\"FixedArrayVariableElement\" typeName=\"string\" minOccurs=\"12\" maxOccurs=\"12\"/>"                    
                    L"        <ECProperty propertyName=\"Property18\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property19\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property20\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property21\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property22\" typeName=\"string\" />"
                    L"        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\"/>"
                    L"        <ECProperty propertyName=\"Property23\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property24\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property25\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property26\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Property27\" typeName=\"string\" />"
                    L"        <ECArrayProperty propertyName=\"VariableArrayVariableElement\" typeName=\"string\"/>"
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
                    L"        <ECArrayProperty propertyName=\"EndingArray\" typeName=\"string\" />"
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

    EXPECT_EQ (SUCCESS, ECSchema::ReadXmlFromString (schema, schemaXMLString.c_str(), NULL, NULL));   

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
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, ECSchema::ReadXmlFromString (schema, schemaXml.c_str(), NULL, NULL));

    return schema;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExerciseVariableCountIntArray (IECInstanceR instance, ECValue& v, wchar_t* arrayAccessor, int baseValue)
    {
    // test insertion in an empty array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 0, 5));
    VerifyArrayInfo             (instance, v, arrayAccessor, 5, false);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 5, true);    
    SetAndVerifyIntegerArray    (instance, v, arrayAccessor, baseValue, 5);   
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 5, false);
    VerifyOutOfBoundsError      (instance, v, arrayAccessor, 5);
    // test insertion in the middle of an array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 3, 3));    
    VerifyArrayInfo             (instance, v, arrayAccessor, 8, false);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 3, false);
    VerifyIntegerArray          (instance, v, arrayAccessor, baseValue, 0, 3);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 3, 3, true);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 6, 2, false);
    VerifyIntegerArray          (instance, v, arrayAccessor, baseValue + 3, 6, 2);
    SetAndVerifyIntegerArray    (instance, v, arrayAccessor, baseValue, 8);   
    // test insertion at the beginning of an array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 0, 4));    
    VerifyArrayInfo             (instance, v, arrayAccessor, 12, false);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 4, true);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 4, 8, false);
    VerifyIntegerArray          (instance, v, arrayAccessor, baseValue, 4, 8);    
    SetAndVerifyIntegerArray    (instance, v, arrayAccessor, baseValue, 12);     
    // test insertion at the end of an array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.AddArrayElements (arrayAccessor, 2));    
    VerifyArrayInfo             (instance, v, arrayAccessor, 14, false);    
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 12, 2, true);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 12, false);
    VerifyIntegerArray          (instance, v, arrayAccessor, baseValue, 0, 12);    
    SetAndVerifyIntegerArray    (instance, v, arrayAccessor, baseValue, 14);               
    }    
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExerciseVariableCountStringArray (IECInstanceR instance, ECValue& v, wchar_t* arrayAccessor, wchar_t* stringSeed)
    {
    // test insertion in an empty array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 0, 5));
    VerifyArrayInfo             (instance, v, arrayAccessor, 5, false);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 5, true);    
    SetAndVerifyStringArray     (instance, v, arrayAccessor, stringSeed, 5);   
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 5, false);
    VerifyOutOfBoundsError      (instance, v, arrayAccessor, 5);
    // test insertion in the middle of an array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 3, 3));    
    VerifyArrayInfo             (instance, v, arrayAccessor, 8, false);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 3, false);
    VerifyStringArray           (instance, v, arrayAccessor, stringSeed, 0, 3);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 3, 3, true);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 6, 2, false);
    std::wstring stringSeedXXX(stringSeed);
    stringSeedXXX.append (L"XXX");
    VerifyStringArray           (instance, v, arrayAccessor, stringSeedXXX.c_str(), 6, 2);
    SetAndVerifyStringArray     (instance, v, arrayAccessor, stringSeed, 8);   
    // test insertion at the beginning of an array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 0, 4));    
    VerifyArrayInfo             (instance, v, arrayAccessor, 12, false);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 4, true);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 4, 8, false);
    VerifyStringArray           (instance, v, arrayAccessor, stringSeed, 4, 8);    
    SetAndVerifyStringArray     (instance, v, arrayAccessor, stringSeed, 12);     
    // test insertion at the end of an array
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.AddArrayElements (arrayAccessor, 2));    
    VerifyArrayInfo             (instance, v, arrayAccessor, 14, false);    
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 12, 2, true);
    VerifyIsNullArrayElements   (instance, v, arrayAccessor, 0, 12, false);
    VerifyStringArray           (instance, v, arrayAccessor, stringSeed, 0, 12);    
    SetAndVerifyStringArray     (instance, v, arrayAccessor, stringSeed, 14);               
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyVariableCountManufacturerArray (IECInstanceR instance, ECValue& v, wchar_t* arrayAccessor)
    {    
    VerifyArrayInfo (instance, v, arrayAccessor, 4, false);
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, arrayAccessor));    
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 0, 4, false);
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, arrayAccessor, 0));    
    EXPECT_TRUE (v.IsStruct());    
    IECInstancePtr manufInst = v.GetStruct();
    VerifyString (*manufInst, v, L"Name", L"Nissan");
    VerifyInteger (*manufInst, v, L"AccountNo", 3475);
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, arrayAccessor, 1));    
    EXPECT_TRUE (v.IsStruct());    
    manufInst = v.GetStruct();
    VerifyString (*manufInst, v, L"Name", L"Ford");
    VerifyInteger (*manufInst, v, L"AccountNo", 381);    
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, arrayAccessor, 2));    
    EXPECT_TRUE (v.IsStruct());    
    manufInst = v.GetStruct();
    VerifyString (*manufInst, v, L"Name", L"Chrysler");
    VerifyInteger (*manufInst, v, L"AccountNo", 81645);    
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, arrayAccessor, 3));    
    EXPECT_TRUE (v.IsStruct());    
    manufInst = v.GetStruct();
    VerifyString (*manufInst, v, L"Name", L"Toyota");
    VerifyInteger (*manufInst, v, L"AccountNo", 6823);    
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExerciseVariableCountManufacturerArray (IECInstanceR instance, StandaloneECEnablerR manufacturerEnabler, ECValue& v, wchar_t* arrayAccessor)
    {    
    // WIP_FUSION, review this process of setting array eleeents.  Is it easy enough?
    VerifyArrayInfo (instance, v, arrayAccessor, 0, false);
    
    // create an array of two values
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.AddArrayElements (arrayAccessor, 2));
    VerifyArrayInfo (instance, v, arrayAccessor, 2, false);
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 0, 2, true);
    IECInstancePtr manufInst = manufacturerEnabler.CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Nissan");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 3475);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance.SetValue (arrayAccessor, v, 0));
    manufInst = manufacturerEnabler.CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Kia");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1791);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance.SetValue (arrayAccessor, v, 1));    
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 0, 2, false);    
   
    // insert two elements in the middle of the array   
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance.InsertArrayElements (arrayAccessor, 1, 2));
    VerifyArrayInfo (instance, v, arrayAccessor, 4, false);
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 0, 1, false);
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 1, 2, true);
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 3, 1, false);
    manufInst = manufacturerEnabler.CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Ford");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 381);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance.SetValue (arrayAccessor, v, 1)); 
    manufInst = manufacturerEnabler.CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Chrysler");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 81645);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS ==instance.SetValue (arrayAccessor, v, 2));        
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 0, 4, false);
    
    // ensure we can set a struct array value to NULL        
    v.SetToNull();
    ASSERT_TRUE (SUCCESS == instance.SetValue (arrayAccessor, v, 3));        
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 0, 3, false);
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 3, 1, true);
    manufInst = manufacturerEnabler.CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Acura");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 6);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance.SetValue (arrayAccessor, v, 3));        
    VerifyIsNullArrayElements (instance, v, arrayAccessor, 3, 1, false);
    manufInst = manufacturerEnabler.CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Toyota");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 6823);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance.SetValue (arrayAccessor, v, 3));        
    
    // ensure we can't set the array elements to other primitive types
    {
    // WIP_FUSION - these are busted need to fix
    DISABLE_ASSERTS    
    v.SetInteger (35);    
    //index = 2;
    //ASSERT_TRUE (ERROR == instance.SetValue (arrayAccessor, v, 1, &index));        
    v.SetString (L"foobar");    
    //index = 0;
    //ASSERT_TRUE (ERROR == instance.SetValue (arrayAccessor, v, 1, &index));            
    }
    
    // WIP_FUSION ensure we can not set the array to an instance of a struct that is not of the type (or derived from the type) of the property    
        
    VerifyVariableCountManufacturerArray (instance, v, arrayAccessor);
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
        
    VerifyArrayInfo (instance, v, L"BeginningArray[]", 0, false);
    VerifyArrayInfo (instance, v, L"FixedArrayFixedElement[]", 10, true);
    VerifyArrayInfo (instance, v, L"VariableArrayFixedElement[]", 0, false);
    VerifyArrayInfo (instance, v, L"FixedArrayVariableElement[]", 12, true);
    VerifyArrayInfo (instance, v, L"VariableArrayVariableElement[]", 0, false);
    VerifyArrayInfo (instance, v, L"EndingArray[]", 0, false);
    
    // WIP_FUSION, verify other properties of ArrayInfo are correct                        
    
    VerifyIsNullArrayElements (instance, v, L"FixedArrayFixedElement[]", 0, 10, true);
    SetAndVerifyIntegerArray (instance, v, L"FixedArrayFixedElement[]", 172, 10);
    VerifyIsNullArrayElements (instance, v, L"FixedArrayFixedElement[]", 0, 10, false);
    SetAndVerifyIntegerArray (instance, v, L"FixedArrayFixedElement[]", 283, 10);    
    
    VerifyIsNullArrayElements (instance, v, L"FixedArrayVariableElement[]", 0, 12, true);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement[]", L"BaseString", 12);       
    VerifyIsNullArrayElements (instance, v, L"FixedArrayVariableElement[]", 0, 12, false);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement[]", L"LaaaaaaargeString", 10);       
    VerifyStringArray (instance, v, L"FixedArrayVariableElement[]", L"BaseStringXXXXXXXXXX", 10, 2);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement[]", L"XString", 12);           
    
    ExerciseVariableCountStringArray (instance, v, L"BeginningArray[]", L"BAValue");
    ExerciseVariableCountIntArray    (instance, v, L"VariableArrayFixedElement[]", 57);
    ExerciseVariableCountStringArray (instance, v, L"VariableArrayVariableElement[]", L"Var+Var");
    ExerciseVariableCountStringArray (instance, v, L"EndingArray[]", L"EArray");        
    
    ECClassP manufacturerClass = instance.GetClass().GetSchema().GetClassP (L"Manufacturer");
    ASSERT_TRUE (manufacturerClass);   
    ClassLayoutP manufClassLayout = ClassLayout::BuildFromClass (*manufacturerClass, 43, 24);
    StandaloneECEnablerPtr manufEnabler = StandaloneECEnabler::CreateEnabler (*manufacturerClass, *manufClassLayout);        
    ExerciseVariableCountManufacturerArray (instance, *manufEnabler, v, L"ManufacturerArray[]");    
    
    // WIP_FUSION verify I can set array elements to NULL        
            
    // WIP_FUSION, add array members                       
    // WIP_FUSION, remove array members
    // WIP_FUSION, clear array
    
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
    VerifyArrayInfo     (instance, v, L"FixedArrayFixedElement[]", 10, true);
    VerifyIntegerArray  (instance, v, L"FixedArrayFixedElement[]", 283, 0, 10);             
    VerifyArrayInfo     (instance, v, L"BeginningArray[]", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"BeginningArray[]", 0, 14, false);
    VerifyStringArray   (instance, v, L"BeginningArray[]", L"BAValue", 0, 14);        
    VerifyArrayInfo     (instance, v, L"FixedArrayVariableElement[]", 12, true);
    VerifyIsNullArrayElements   (instance, v, L"FixedArrayVariableElement[]", 0, 12, false);
    VerifyStringArray   (instance, v, L"FixedArrayVariableElement[]", L"XString", 0, 12);           
    VerifyArrayInfo     (instance, v, L"VariableArrayFixedElement[]", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"VariableArrayFixedElement[]", 0, 14, false);
    VerifyIntegerArray  (instance, v, L"VariableArrayFixedElement[]", 57, 0, 14);                   
    VerifyArrayInfo     (instance, v, L"VariableArrayVariableElement[]", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"VariableArrayVariableElement[]", 0, 14, false);
    VerifyStringArray   (instance, v, L"VariableArrayVariableElement[]", L"Var+Var", 0, 14);               
    VerifyArrayInfo     (instance, v, L"EndingArray[]", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"EndingArray[]", 0, 14, false);
    VerifyStringArray   (instance, v, L"EndingArray[]", L"EArray", 0, 14);                
    VerifyVariableCountManufacturerArray (instance, v, L"ManufacturerArray[]");     
    
    instance.Dump();
    
    delete manufClassLayout;             
    }
                 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MemoryLayoutTests, InstantiateStandaloneInstance)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);

    SchemaLayout schemaLayout (24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout.GetSchemaIndex());
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);        

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    wstring instanceId = instance->GetInstanceId();
    instance->Dump();
    ExerciseInstance (*instance, L"Test");

    delete classLayout;

    // instance.Compact()... then check values again
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MemoryLayoutTests, InstantiateInstanceWithNoProperties)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"EmptyClass");
    ASSERT_TRUE (ecClass);

    SchemaLayout schemaLayout (25);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 52, schemaLayout.GetSchemaIndex());
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);        

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    wstring instanceId = instance->GetInstanceId();
    UInt32 size = instance->GetBytesUsed ();
    EXPECT_EQ (size, UInt32(sizeof(InstanceHeader)));

    instance->Dump();
    delete classLayout;

    // instance.Compact()... then check values again
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MemoryLayoutTests, DirectSetStandaloneInstance)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"CadData");
    ASSERT_TRUE (ecClass);
    
    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 0, 0);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);        
    
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    DPoint2d   inSize = {10.5, 22.3};
    DPoint3d   inPoint1 = {10.10, 11.11, 12.12};
    DPoint3d   inPoint2 ={100.100, 110.110, 120.120};
    SystemTime inTime = SystemTime::GetLocalTime();
    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;
    Int64      inTicks = 634027121070910000;

    instance->SetValue (L"Count",        ECValue (inCount));
    instance->SetValue (L"Name",         ECValue (L"Test"));
    instance->SetValue (L"Length",       ECValue (inLength));
    instance->SetValue (L"Field_Tested", ECValue (inTest));
    instance->SetValue (L"Size",         ECValue (inSize));
    instance->SetValue (L"StartPoint",   ECValue (inPoint1));
    instance->SetValue (L"EndPoint",     ECValue (inPoint2));
    instance->SetValue (L"Service_Date", ECValue (inTime));

    ECValue ecValue;
    ecValue.SetDateTimeTicks(inTicks);
    instance->SetValue (L"Install_Date", ecValue);

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Count"));
    EXPECT_TRUE (ecValue.GetInteger() == inCount);
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Name"));
    EXPECT_STREQ (ecValue.GetString(), L"Test");
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Length"));
    EXPECT_TRUE (ecValue.GetDouble() == inLength);
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Field_Tested"));
    EXPECT_TRUE (ecValue.GetBoolean() == inTest);
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Size"));
    EXPECT_TRUE (SUCCESS == memcmp (&inSize, &ecValue.GetPoint2D(), sizeof(inSize)));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"StartPoint"));
    EXPECT_TRUE (SUCCESS == memcmp (&inPoint1, &ecValue.GetPoint3D(), sizeof(inPoint1)));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"EndPoint"));
    EXPECT_TRUE (SUCCESS == memcmp (&inPoint2, &ecValue.GetPoint3D(), sizeof(inPoint2)));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Service_Date"));
    EXPECT_TRUE (SUCCESS == memcmp (&inTime, &ecValue.GetDateTime(), sizeof(inTime)));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Install_Date"));
    EXPECT_TRUE (ecValue.GetDateTimeTicks() == inTicks);

    delete classLayout;

    // instance.Compact()... then check values again
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MemoryLayoutTests, GetSetValuesByIndex)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);
    
    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 0, 0);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);        
    
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    const wchar_t * accessString = L"Property34";

    //UInt32          intValue = 12345;
    const wchar_t * stringValue = L"Xyz";

    //instance->SetValue  (accessString, ECValue (intValue));
    instance->SetValue  (accessString, ECValue (stringValue));

    ECValue value;    
    UInt32  propertyIndex;
    
    EXPECT_TRUE (SUCCESS  == enabler->GetPropertyIndex (propertyIndex, accessString));
    EXPECT_TRUE (SUCCESS  == instance->GetValue (value, propertyIndex));
    //EXPECT_TRUE (intValue == value.GetInteger());
    EXPECT_STREQ (stringValue, value.GetString());

#if defined (TIMING_ACCESS_BYINDEX)
    UInt32      numAccesses = 10000000;

    double      elapsedTime1 = 0.0;
    StopWatch   timer1 (L"Time getting values using index", true);

    for (UInt32 i = 0; i < numAccesses; i++)
        {
        timer1.Start();
        instance->GetValue (value, propertyIndex);
        timer1.Stop();

        elapsedTime1 += timer1.GetElapsedSeconds();
        }

    double      elapsedTime2 = 0.0;
    StopWatch   timer2 (L"Time getting values using accessString", true);

    for (UInt32 i = 0; i < numAccesses; i++)
        {
        timer2.Start();
        instance->GetValue (value, accessString);
        timer2.Stop();

        elapsedTime2 += timer2.GetElapsedSeconds();
        }

    wprintf (L"Time to set %d values by: accessString = %.4f, index = %.4f\n", numAccesses, elapsedTime1, elapsedTime2);
#endif

    delete classLayout;

    // instance.Compact()... then check values again
    CoUninitialize();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MemoryLayoutTests, ExpectErrorsWhenViolatingArrayConstraints)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);    
    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, 24);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);            
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    {
    DISABLE_ASSERTS
    // verify we can not change the size of fixed arrays        
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayFixedElement[]", 0, 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayFixedElement[]", 10, 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->AddArrayElements    (L"FixedArrayFixedElement[]", 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayVariableElement[]", 0, 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayVariableElement[]", 12, 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->AddArrayElements    (L"FixedArrayVariableElement[]", 1));
    
    // verify constraints of array insertion are enforced
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"NonExistArray", 0, 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"BeginningArray", 0, 1)); // missing brackets
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"BeginningArray[]", 2, 1)); // insert index is invalid    
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"BeginningArray[]", 0, 0)); // insert count is invalid    
    }
    
    ECValue v;
    VerifyOutOfBoundsError (*instance, v, L"BeginningArray[]", 0);
    VerifyOutOfBoundsError (*instance, v, L"FixedArrayFixedElement[]", 10);
    VerifyOutOfBoundsError (*instance, v, L"VariableArrayFixedElement[]", 0);
    VerifyOutOfBoundsError (*instance, v, L"FixedArrayVariableElement[]", 12);
    VerifyOutOfBoundsError (*instance, v, L"VariableArrayVariableElement[]", 0);
    VerifyOutOfBoundsError (*instance, v, L"EndingArray[]", 0);                     
    
    delete classLayout;
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
    
    //bool
    ECValue boolVal(true);
    EXPECT_TRUE (boolVal.IsBoolean());
    EXPECT_TRUE (boolVal.GetBoolean());

    //DPoint3d
    DPoint3d inPoint3 = {10.0, 100.0, 1000.0};
    ECValue pntVal3(inPoint3);
    DPoint3d outPoint3 = pntVal3.GetPoint3D ();
    EXPECT_TRUE (pntVal3.IsPoint3D());
    EXPECT_TRUE (0 == memcmp(&inPoint3, &outPoint3, sizeof(outPoint3)));
    std::wstring point3Str = pntVal3.ToString();
    EXPECT_TRUE (0 == point3Str.compare (L"{10,100,1000}"));

    //DPoint2d
    DPoint2d inPoint2 = {10.0, 100.0};
    ECValue pntVal2 (inPoint2);
    EXPECT_TRUE (pntVal2.IsPoint2D());
    DPoint2d outPoint2 = pntVal2.GetPoint2D ();
    EXPECT_TRUE (0 == memcmp(&inPoint2, &outPoint2, sizeof(outPoint2)));
    std::wstring point2Str = pntVal2.ToString();
    EXPECT_TRUE (0 == point2Str.compare (L"{10,100}"));

    // DateTime
    SystemTime now = SystemTime::GetLocalTime();
    ECValue dateValue (now);
    SystemTime nowUTC = SystemTime::GetSystemTime();
    Int64 ticks = dateValue.GetDateTimeTicks ();
    EXPECT_TRUE (dateValue.IsDateTime());
    SystemTime nowtoo = dateValue.GetDateTime ();
    EXPECT_TRUE (0 == memcmp(&nowtoo, &now, sizeof(nowtoo)));
    ECValue fixedDate;
    fixedDate.SetDateTimeTicks (634027121070910000);
    std::wstring dateStr = fixedDate.ToString();
    EXPECT_TRUE (0 == dateStr.compare (L"#2010/2/25-16:28:27:91#"));

    // WIP_FUSION - test array values
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MemoryLayoutTests, TestSetGetNull)
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);
        
    SchemaLayout schemaLayout (24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout.GetSchemaIndex());
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);
    
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
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

    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"D"));
    EXPECT_TRUE (v.IsNull());    
    
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, L"S"));
    EXPECT_FALSE (v.IsNull());     
    
    // WIP_FUSION test arrays

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

void SetValuesForProfiling (StandaloneECInstanceR instance)
    {
    for (NameVector::const_iterator it = s_propertyNames.begin(); it != s_propertyNames.end(); ++it)
        instance.SetValue (it->c_str(), ECValue (it->c_str()));
    }
    
TEST (MemoryLayoutTests, ProfileSettingValues)
    {
    int nStrings = 100;
    int nInstances = 1000;
    
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema = CreateProfilingSchema(nStrings);
    ECClassP ecClass = schema->GetClassP (L"Pidget");
    ASSERT_TRUE (ecClass);
        
    SchemaLayout schemaLayout(24);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 42, schemaLayout.GetSchemaIndex());
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    UInt32 slack = 0;
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