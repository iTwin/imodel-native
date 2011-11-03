/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "StopWatch.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>

BEGIN_BENTLEY_EC_NAMESPACE

using namespace std;

struct MemoryLayoutTests : ECTestFixture {};

// WIP_FUSION: these verify methods are duplicated in DgnPlatformTest... how do we share that code?    
// WIP_FUSION: where is the right place to share these methods even among ECObjects tests? 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, WCharCP value)
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
void VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value)
    {
    return VerifyString (instance, v, accessString, false, 0, value);
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value)
    {
    v.SetString(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyString (instance, v, accessString, value);
    }
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, UInt32 value)
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
void VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value)
    {
    return VerifyInteger (instance, v, accessString, false, 0, value);
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value)
    {
    v.SetInteger(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyInteger (instance, v, accessString, value);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyDouble (IECInstanceR instance, ECValueR v, WCharCP accessString, double value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetDouble());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyDouble (IECInstanceR instance, ECValueR v, WCharCP accessString, double value)
    {
    v.SetDouble(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyDouble (instance, v, accessString, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyLong (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt64 value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetLong());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyLong (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt64 value)
    {
    v.SetLong(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyLong (instance, v, accessString, value);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyArrayInfo (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 count, bool isFixedCount)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (count, v.GetArrayInfo().GetCount());
    EXPECT_EQ (isFixedCount, v.GetArrayInfo().IsFixedCount());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyOutOfBoundsError (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 index)
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
void VerifyStringArray (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value, UInt32 start, UInt32 count)
    {
    WString incrementingString = value;
   
    for (UInt32 i=start ; i < start + count ; i++)        
        {
        incrementingString.append (L"X");
        VerifyString (instance, v, accessString, true, i, incrementingString.c_str());
        }
    }  
              
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyStringArray (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value, UInt32 count)
    {
    WString incrementingString = value;
    for (UInt32 i=0 ; i < count ; i++)        
        {
        incrementingString.append (L"X");
        v.SetString(incrementingString.c_str());

        // since the test sets some of the array values more than once to the same value we must check SUCCESS || ECOBJECTS_STATUS_PropertyValueMatchesNoChange 
        ECObjectsStatus status = instance.SetValue (accessString, v, i);
        EXPECT_TRUE (SUCCESS == status || ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status);
        }
    
    VerifyStringArray (instance, v, accessString, value, 0, count);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void VerifyIntegerArray (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 baseValue, UInt32 start, UInt32 count)
    {       
    for (UInt32 i=start ; i < start + count ; i++)        
        {
        VerifyInteger (instance, v, accessString, true, i, baseValue++);
        }
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void SetAndVerifyIntegerArray (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 baseValue, UInt32 count)
    {
    for (UInt32 i=0 ; i < count ; i++)        
        {
        v.SetInteger(baseValue + i); 

        // since the test sets some of the array values more than once to the same value we must check SUCCESS || ECOBJECTS_STATUS_PropertyValueMatchesNoChange 
        ECObjectsStatus status = instance.SetValue (accessString, v, i);
        EXPECT_TRUE (SUCCESS == status || ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status);
        }
        
    VerifyIntegerArray (instance, v, accessString, baseValue, 0, count);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 start, UInt32 count, bool isNull)
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
WString    GetTestSchemaXMLString (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor, WCharCP className)
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
                    L"    <ECClass typeName=\"AllPrimitives\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"AString\"          typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"AnInt\"            typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"APoint3d\"         typeName=\"point3d\" />"
                    L"        <ECProperty propertyName=\"APoint2d\"         typeName=\"point2d\" />"
                    L"        <ECProperty propertyName=\"ADouble\"          typeName=\"double\"  />"
                    L"        <ECProperty propertyName=\"ADateTime\"        typeName=\"dateTime\"  />"
                    L"        <ECProperty propertyName=\"ABoolean\"         typeName=\"boolean\"  />"
                    L"        <ECProperty propertyName=\"ALong\"            typeName=\"long\"  />"
                    L"        <ECProperty propertyName=\"ABinary\"          typeName=\"binary\"  />"
                    L"        <ECProperty propertyName=\"ReadOnlyInt\"      typeName=\"int\" readOnly=\"True\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeStrings\" typeName=\"string\" />"
                    L"        <ECArrayProperty propertyName=\"SomeInts\"    typeName=\"int\" />"
                    L"        <ECArrayProperty propertyName=\"SomePoint3ds\"    typeName=\"point3d\" />"
                    L"        <ECArrayProperty propertyName=\"SomePoint2ds\"    typeName=\"point2d\" />"
                    L"        <ECArrayProperty propertyName=\"SomeDoubles\"     typeName=\"double\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeDateTimes\"   typeName=\"dateTime\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeBooleans\"    typeName=\"boolean\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeLongs\"       typeName=\"long\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeBinaries\"    typeName=\"binary\"  />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"FixedSizeArrayTester\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"FixedString1\"  typeName=\"string\"     minOccurs=\"1\"  maxOccurs=\"1\" />"
                    L"        <ECArrayProperty propertyName=\"FixedInt1\"     typeName=\"int\"        minOccurs=\"1\"  maxOccurs=\"1\" />"
                    L"        <ECArrayProperty propertyName=\"FixedString10\" typeName=\"string\"     minOccurs=\"10\" maxOccurs=\"10\" />"
                    L"        <ECArrayProperty propertyName=\"FixedInt10\"    typeName=\"int\"        minOccurs=\"10\" maxOccurs=\"10\" />"
                    L"        <ECArrayProperty propertyName=\"Struct1\"       typeName=\"BaseClass0\" minOccurs=\"1\"  maxOccurs=\"1\" />"
                    L"        <ECArrayProperty propertyName=\"Struct10\"      typeName=\"BaseClass0\" minOccurs=\"10\" maxOccurs=\"10\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassLayoutPerformanceTest0\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"AString\"  typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"AnInt\"    typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"ADouble\"  typeName=\"double\"  />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassLayoutPerformanceTest1\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"AMonkeywrench\"    typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"ADouble\"          typeName=\"double\"  />"
                    L"        <ECProperty propertyName=\"AString\"          typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"AnInt\"            typeName=\"int\" />"
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
                    L"    <ECClass typeName=\"NestedStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"NestPropString\" typeName=\"string\" />"
                    L"        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassWithStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"StructArray\" typeName=\"AllPrimitives\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
                    L"        <ECStructProperty propertyName=\"StructMember\" typeName=\"AllPrimitives\" />"
                    L"        <ECArrayProperty propertyName=\"ComplicatedStructArray\" typeName=\"NestedStructArray\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassWithPolymorphicStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"PolymorphicStructArray\" typeName=\"BaseClass0\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"BaseClass0\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"BaseIntProperty\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"DerivedClass0\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <BaseClass>BaseClass0</BaseClass>"
                    L"        <ECProperty propertyName=\"DerivedStringProperty\" typeName=\"string\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"DerivedClass1\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <BaseClass>BaseClass0</BaseClass>"
                    L"        <ECProperty propertyName=\"DerivedDoubleProperty\" typeName=\"double\" />"
                    L"    </ECClass>"
                    L"</ECSchema>";

    wchar_t* buff = (wchar_t*) _alloca (2 * (50 + wcslen (fmt) + wcslen (schemaName) + wcslen (className)));

    swprintf (buff, fmt, schemaName, versionMajor, versionMinor, className);

    return buff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       CreateTestSchema (ECSchemaCacheR schemaOwner)
    {
    WString schemaXMLString = GetTestSchemaXMLString (L"TestSchema", 0, 0, L"TestClass");

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext(schemaOwner);

    ECSchemaP schema;        
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (schema, schemaXMLString.c_str(), *schemaContext));  

    return schema;
    }
    
typedef std::vector<WString> NameVector;
static std::vector<WString> s_propertyNames;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       CreateProfilingSchema (int nStrings, ECSchemaCacheR schemaOwner)
    {
    s_propertyNames.clear();
    
    WString schemaXml = 
                    L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"ProfilingSchema\" nameSpacePrefix=\"p\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"Pidget\" isDomainClass=\"True\">";

    for (int i = 0; i < nStrings; i++)
        {
        wchar_t propertyName[32];
        swprintf(propertyName, L"StringProperty%02d", i);
        s_propertyNames.push_back (propertyName);
        WCharCP propertyFormat = 
                    L"        <ECProperty propertyName=\"%s\" typeName=\"string\" />";
        wchar_t propertyXml[128];
        swprintf (propertyXml, propertyFormat, propertyName);
        schemaXml += propertyXml;
        }                    

    schemaXml +=    L"    </ECClass>"
                    L"</ECSchema>";

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext(schemaOwner);

    ECSchemaP schema;        
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString (schema, schemaXml.c_str(), *schemaContext));

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
    WString stringSeedXXX(stringSeed);
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
    instance.GetValue (v, L"D");
    
    double doubleValue = 1.0/3.0;
    SetAndVerifyDouble (instance, v, L"D", doubleValue);

    SetAndVerifyInteger (instance, v, L"A", 97);
    SetAndVerifyInteger (instance, v, L"AA", 12);   
    
    SetAndVerifyString (instance, v, L"B", L"Happy");
    SetAndVerifyString (instance, v, L"B", L"Very Happy");
    SetAndVerifyString (instance, v, L"B", L"sad");
    SetAndVerifyString (instance, v, L"S", L"Lucky");
    SetAndVerifyString (instance, v, L"B", L"Very Very Happy");
    VerifyString (instance, v, L"S", L"Lucky");
    SetAndVerifyString (instance, v, L"Manufacturer.Name", L"Charmed");
    SetAndVerifyString (instance, v, L"S", L"Lucky Strike");
        
    wchar_t largeString[3300];
    largeString[0] = L'\0';
    for (int i = 0; i < 20; i++)
        wcscat (largeString, L"S2345678901234567890123456789012");
    
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
    ASSERT_TRUE (NULL != manufacturerClass);

#ifdef OLD_WAY    
    ClassLayoutP manufClassLayout = ClassLayout::BuildFromClass (*manufacturerClass, 43, 24);
    StandaloneECEnablerPtr manufEnabler = StandaloneECEnabler::CreateEnabler (*manufacturerClass, *manufClassLayout, true);
#endif
    StandaloneECEnablerPtr manufEnabler =  manufacturerClass->GetDefaultStandaloneEnabler();
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
    
    // WIP_FUSION: should pass the string to the logger via a backdoor
    instance.ToString(L"").c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetPrimitiveValuesUsingInteropHelper)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    double    doubleVal;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDoubleValue (*instance, L"ADouble", (double)(1.0/3.0)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDouble      (*instance, doubleVal, L"ADouble"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDoubleValue (*instance, L"SomeDoubles[0]", (double)(7.0/3.0)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDouble      (*instance, doubleVal, L"SomeDoubles[0]"));
    EXPECT_TRUE ((double)(7.0/3.0) == doubleVal);

    int       intVal;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetIntegerValue (*instance, L"AnInt", (int)(50)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetInteger      (*instance, intVal, L"AnInt"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetIntegerValue (*instance, L"SomeInts[0]", (int)(50)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetInteger      (*instance, intVal, L"SomeInts[0]"));
    EXPECT_TRUE ((int)(50) == intVal);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetIntegerValue (*instance, L"ReadOnlyInt", (int)(50)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetInteger  (*instance, intVal, L"ReadOnlyInt"));
    EXPECT_TRUE ((int)(50) == intVal);

    WCharCP   stringVal;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetStringValue (*instance, L"AString", L"TEST123"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetString      (*instance, stringVal, L"AString"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetStringValue (*instance, L"SomeStrings[0]", L"TEST432"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetString      (*instance, stringVal, L"SomeStrings[0]"));
    EXPECT_TRUE (0 == wcscmp(L"TEST432", stringVal));

    Int64       longVal;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetLongValue (*instance, L"ALong", (Int64)(50)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetLong      (*instance, longVal, L"ALong"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetLongValue (*instance, L"SomeLongs[0]", (Int64)(50)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetLong      (*instance, longVal, L"SomeLongs[0]"));
    EXPECT_TRUE ((Int64)(50) == longVal);

    bool       boolVal;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetBooleanValue (*instance, L"ABoolean", false));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetBoolean      (*instance, boolVal, L"ABoolean"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetBooleanValue (*instance, L"SomeBooleans[0]", false));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetBoolean      (*instance, boolVal, L"SomeBooleans[0]"));
    EXPECT_TRUE (false == boolVal);

    DPoint2d   point2dInput = {1.0, 2.0};
    DPoint2d   point2dOutput;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetPoint2DValue (*instance, L"APoint2d", point2dInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetPoint2D      (*instance, point2dOutput, L"APoint2d"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetPoint2DValue (*instance, L"SomePoint2ds[0]", point2dInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetPoint2D      (*instance, point2dOutput, L"SomePoint2ds[0]"));
    EXPECT_TRUE (point2dInput.x == point2dOutput.x && point2dInput.y == point2dOutput.y);

    DPoint3d   point3dInput = {1.0, 2.0, 3.0};
    DPoint3d   point3dOutput;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetPoint3DValue (*instance, L"APoint3d", point3dInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetPoint3D      (*instance, point3dOutput, L"APoint3d"));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetPoint3DValue (*instance, L"SomePoint3ds[0]", point3dInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetPoint3D      (*instance, point3dOutput, L"SomePoint3ds[0]"));
    EXPECT_TRUE (point3dInput.x == point3dOutput.x && point3dInput.y == point3dOutput.y && point3dInput.z == point3dOutput.z);

    SystemTime timeInput  = SystemTime::GetLocalTime();
    Int64      ticksInput = 634027121070910000;
    SystemTime timeOutput;
    Int64      ticksOutput;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDateTimeValue (*instance, L"ADateTime", timeInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDateTime      (*instance, timeOutput, L"ADateTime"));
    EXPECT_TRUE (timeInput == timeOutput);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDateTimeTicks (*instance, L"ADateTime", ticksInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDateTimeTicks (*instance, ticksOutput, L"ADateTime"));
    EXPECT_TRUE (ticksInput == ticksOutput);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDateTimeValue (*instance, L"SomeDateTimes[0]", timeInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDateTime      (*instance, timeOutput, L"SomeDateTimes[0]"));
    EXPECT_TRUE (timeInput == timeOutput);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDateTimeTicks (*instance, L"SomeDateTimes[1]", ticksInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDateTimeTicks (*instance, ticksOutput, L"SomeDateTimes[1]"));
    EXPECT_TRUE (ticksInput == ticksOutput);

    const byte  binaryInput[] = {0x01, 0x23, 0x26, 0x78};
    const size_t sizeInput   = 4;
    ECValue      valueInput;
    valueInput.SetBinary(binaryInput, sizeInput, false);
    const byte*  binaryOutput;
    size_t       sizeOutput;
    ECValue      valueOutput;

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetValue (*instance, L"ABinary", valueInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetValue (*instance, valueOutput, L"ABinary"));
    binaryOutput = valueOutput.GetBinary (sizeOutput);
    EXPECT_TRUE (sizeInput == sizeOutput);
    for(int i=0; i<(int)sizeInput; i++)
        EXPECT_TRUE (binaryInput[i] == binaryOutput[i]);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetValue (*instance, L"SomeBinaries[0]", valueInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetValue (*instance, valueOutput, L"SomeBinaries[0]"));
    binaryOutput = valueOutput.GetBinary (sizeOutput);
    EXPECT_TRUE (sizeInput == sizeOutput);
    for(int i=0; i<(int)sizeInput; i++)
        EXPECT_TRUE (binaryInput[i] == binaryOutput[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetStructArraysUsingInteropHelper)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"ClassWithStructArray");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    //Testing array of structs
    ECValue structArrayValueInput(42);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetValue (*instance, L"StructArray[1].AnInt", structArrayValueInput));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetValue (*instance, L"StructArray[0].AnInt", structArrayValueInput));


    ECValue structArrayValueOutput;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetValue (*instance, structArrayValueOutput, L"StructArray[0].AnInt"));

    EXPECT_TRUE (structArrayValueInput.GetInteger() == structArrayValueOutput.GetInteger());

    //Just seeing if it's possible to set a struct array element directly using the interop helper.

    ECClassP structClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (NULL != structClass);

    StandaloneECEnablerPtr structEnabler          = structClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr newStructInstance = structEnabler->CreateInstance();

    ECValue manualIntEntry(64);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetValue (*newStructInstance, L"AnInt", manualIntEntry));

    ECValue newStructValue;
    newStructValue.SetStruct(newStructInstance.get());

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetValue (*instance, L"StructArray[2]", newStructValue));

    ECValue manualIntEntryOutput;
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetValue (*instance, manualIntEntryOutput, L"StructArray[2].AnInt"));

    EXPECT_TRUE (manualIntEntryOutput.GetInteger() == manualIntEntry.GetInteger());
    };

 #ifdef  WIP_INSTANCE_BUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetStructArraysUsingInteropHelper)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"ClassWithStructArray");
    ASSERT_TRUE (ecClass);
    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 0, 0);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout, true);

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    // WIP_FUSION
    // Cannot modify the value of a binary primitive after it has been set.
    // Revision: It appears that you cannot change the size of a primitive binary value after it has been set.
    // This is because of the way it's stored in native ECObjects: as a stream of bits, rather that each
    // element as a separate entity.
    const byte bugTestBinary1[8] = { 0x21, 0xa9, 0x84, 0x9d, 0xca, 0x1d, 0x8a, 0x78 };
    const byte bugTestBinary2[4] = { 0x12, 0x79, 0xca, 0xde };

    ECValue bugTestValue0(bugTestBinary2, 4);           


    EC::ECInstanceInteropHelper::SetValue (wipInstance, L"SmallBinaryArray[0]", bugTestValue0);
       
    ECValue bugTestValue1(bugTestBinary1, 8);     

    EC::ECInstanceInteropHelper::SetValue (wipInstance, L"SmallBinaryArray[0]", bugTestValue1);
  
    EC::ECValue bugTestValue2;

    EC::ECInstanceInteropHelper::GetValue (wipInstance, bugTestValue2, L"SmallBinaryArray[0]");
    size_t size1=-1;
    size_t size2=-2;
    const byte* data1 = bugTestValue1.GetBinary(size1);
    const byte* data2 = bugTestValue2.GetBinary(size2);
    Console::WriteLine("Binary 1 : ");
    for(int i=0; i<(int)size1; i++)
        Console::Write("{0}", data1[i]);
    Console::WriteLine("");
    Console::WriteLine("Binary 2 : ");
    for(int i=0; i<(int)size2; i++)
        Console::Write("{0}", data2[i]);
    Console::WriteLine("");
    Console::WriteLine("Size 1: {0}", size1);
    Console::WriteLine("Size 2: {0}", size2);
    DEBUG_EXPECT (size1 == size2);
    for(int i=0; i<(int)size1; i++)
        DEBUG_EXPECT(data1[i] == data2[i]);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetValuesUsingInteropHelper)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    double    doubleVal;
    int       intVal;

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDoubleValue (*instance, L"D", (double)(1.0/3.0)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDouble      (*instance, doubleVal, L"D"));
    EXPECT_TRUE ((double)(1.0/3.0) == doubleVal);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetIntegerValue (*instance, L"FixedArrayFixedElement[0]", (int)(97)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetInteger      (*instance, intVal, L"FixedArrayFixedElement[0]"));
    EXPECT_TRUE (97 == intVal);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetIntegerValue (*instance, L"VariableArrayFixedElement[1]", (int)(101)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetInteger      (*instance, intVal, L"VariableArrayFixedElement[1]"));
    EXPECT_TRUE (101 == intVal);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetIntegerValue (*instance, L"VariableArrayFixedElement[0]", (int)(100)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetInteger      (*instance, intVal, L"VariableArrayFixedElement[0]"));
    EXPECT_TRUE (100 == intVal);

    WString testString = L"Charmed";
    WString testString2 = L"Charmed2";
    WCharCP stringValueP = NULL;

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetStringValue (*instance, L"ManufacturerArray[1].Name", testString.c_str()));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetString (*instance, stringValueP, L"ManufacturerArray[1].Name"));
    EXPECT_STREQ (testString.c_str(), stringValueP);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetStringValue (*instance, L"ManufacturerArray[0].Name", testString2.c_str()));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetString (*instance, stringValueP, L"ManufacturerArray[0].Name"));
    EXPECT_STREQ (testString2.c_str(), stringValueP);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, InstantiateStandaloneInstance)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    WString instanceId = instance->GetInstanceId();
    // WIP_FUSION: should pass the string to the logger via a backdoor
    instance->ToString(L"").c_str();
    ExerciseInstance (*instance, L"Test");

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, InstantiateInstanceWithNoProperties)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"EmptyClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    WString instanceId = instance->GetInstanceId();
    UInt32 size = instance->GetBytesUsed ();
    EXPECT_EQ (size, UInt32(sizeof(InstanceHeader)));

    // WIP_FUSION: should pass the string to the logger via a backdoor
    instance->ToString(L"").c_str();

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, DirectSetStandaloneInstance)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"CadData");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
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

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void  checkValue (WCharCP accessString, ECValueCR value, EC::StandaloneECInstancePtr& instance)
    {
    UInt32  propertyIndex;
    bool isSet;

    ECValue ecValue;

    EXPECT_TRUE (SUCCESS  == instance->GetEnabler().GetPropertyIndex (propertyIndex, accessString));
    EXPECT_TRUE (SUCCESS  == instance->GetValue (ecValue, propertyIndex));
    EXPECT_TRUE (ecValue.Equals(value));

    EXPECT_TRUE (SUCCESS  == instance->IsPerPropertyBitSet (isSet, 0, propertyIndex));
    EXPECT_TRUE (true == isSet);
    EXPECT_TRUE (SUCCESS  == instance->IsPerPropertyBitSet (isSet, 1, propertyIndex));
    EXPECT_TRUE ((0==propertyIndex%2) == isSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void  setValue (WCharCP accessString, ECValueCR value, EC::StandaloneECInstancePtr& instance)
    {
    UInt32  propertyIndex;
    bool isSet;

    EXPECT_TRUE (SUCCESS  == instance->GetEnabler().GetPropertyIndex (propertyIndex, accessString));
    EXPECT_TRUE (SUCCESS  == instance->SetValue (propertyIndex, value));

    EXPECT_TRUE (SUCCESS  == instance->IsPerPropertyBitSet (isSet, 0, propertyIndex));
    EXPECT_TRUE (false  == isSet);
    EXPECT_TRUE (SUCCESS  == instance->SetPerPropertyBit (0, propertyIndex, true));
    EXPECT_TRUE (SUCCESS  == instance->SetPerPropertyBit (1, propertyIndex, 0==propertyIndex%2));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, CheckPerPropertyFlags)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"CadData");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    UInt8  numBitPerProperty =  instance->GetNumBitsInPerPropertyFlags ();
    EXPECT_TRUE (numBitPerProperty == 2);

    DPoint2d   inSize = {10.5, 22.3};
    DPoint3d   inPoint1 = {10.10, 11.11, 12.12};
    DPoint3d   inPoint2 ={100.100, 110.110, 120.120};
    SystemTime inTime = SystemTime::GetLocalTime();
    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;
    Int64      inTicks = 634027121070910000;

    ECValue ecValue;
    ecValue.SetDateTimeTicks(inTicks);

    setValue (L"Count",        ECValue (inCount), instance);
    setValue (L"Name",         ECValue (L"Test"), instance);
    setValue (L"Length",       ECValue (inLength), instance);
    setValue (L"Field_Tested", ECValue (inTest), instance);
    setValue (L"Size",         ECValue (inSize), instance);
    setValue (L"StartPoint",   ECValue (inPoint1), instance);
    setValue (L"EndPoint",     ECValue (inPoint2), instance);
    setValue (L"Service_Date", ECValue (inTime), instance);
    setValue (L"Install_Date", ecValue, instance);

    checkValue (L"Count",        ECValue (inCount), instance);
    checkValue (L"Name",         ECValue (L"Test"), instance);
    checkValue (L"Length",       ECValue (inLength), instance);
    checkValue (L"Field_Tested", ECValue (inTest), instance);
    checkValue (L"Size",         ECValue (inSize), instance);
    checkValue (L"StartPoint",   ECValue (inPoint1), instance);
    checkValue (L"EndPoint",     ECValue (inPoint2), instance);
    checkValue (L"Service_Date", ECValue (inTime), instance);
    checkValue (L"Install_Date", ecValue, instance);

    bool isSet, lastPropertyEncountered=false;
    UInt32  propertyIndex=0;
    instance->ClearAllPerPropertyFlags ();
    ECObjectsStatus status;

    while (!lastPropertyEncountered)
        {
        for (UInt8 i=0; i<numBitPerProperty; i++)
            {
            status = instance->IsPerPropertyBitSet (isSet, i, propertyIndex);
            // break when property index exceeds actual property count
            if (ECOBJECTS_STATUS_Success == status)
                {
                EXPECT_TRUE (false == isSet);
                }
            else
                {
                lastPropertyEncountered = true;
                break;
                }
            }

        propertyIndex++;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetSetValuesByIndex)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    WCharCP accessString = L"Property34";

    //UInt32          intValue = 12345;
    WCharCP stringValue = L"Xyz";

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

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ExpectErrorsWhenViolatingArrayConstraints)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
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
    };    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, Values) // move it!
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
    
    ECValue nullInt (EC::PRIMITIVETYPE_Integer);
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
    WString point3Str = pntVal3.ToString();
    EXPECT_TRUE (0 == point3Str.compare (L"{10,100,1000}"));

    //DPoint2d
    DPoint2d inPoint2 = {10.0, 100.0};
    ECValue pntVal2 (inPoint2);
    EXPECT_TRUE (pntVal2.IsPoint2D());
    DPoint2d outPoint2 = pntVal2.GetPoint2D ();
    EXPECT_TRUE (0 == memcmp(&inPoint2, &outPoint2, sizeof(outPoint2)));
    WString point2Str = pntVal2.ToString();
    EXPECT_TRUE (0 == point2Str.compare (L"{10,100}"));

    // DateTime
    SystemTime now = SystemTime::GetLocalTime();
    ECValue dateValue (now);
    SystemTime nowUTC = SystemTime::GetSystemTime();
    dateValue.GetDateTimeTicks ();
    EXPECT_TRUE (dateValue.IsDateTime());
    SystemTime nowtoo = dateValue.GetDateTime ();
    EXPECT_TRUE (0 == memcmp(&nowtoo, &now, sizeof(nowtoo)));
    ECValue fixedDate;
    fixedDate.SetDateTimeTicks (634027121070910000);
    WString dateStr = fixedDate.ToString();
    EXPECT_TRUE (0 == dateStr.compare (L"#2010/2/25-16:28:27:91#"));

    // test operator ==
    SystemTime specificTime (now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
    EXPECT_TRUE (specificTime == now);

    SystemTime defaultTime1;
    SystemTime defaultTime2;
    EXPECT_TRUE (defaultTime1 == defaultTime2);

    // WIP_FUSION - test array values
    };
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestSetGetNull)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
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

    
    };
    
void SetStringToSpecifiedNumberOfCharacters (IECInstanceR instance, int nChars)
    {
    WCharP string = (WCharP)alloca ((nChars + 1) * sizeof(wchar_t));
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
    
TEST_F (MemoryLayoutTests, ProfileSettingValues)
    {
    int nStrings = 100;
    int nInstances = 1000;

    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP          schema      = CreateProfilingSchema(nStrings, *schemaOwner);
    ECClassP           ecClass     = schema->GetClassP (L"Pidget");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
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