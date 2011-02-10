/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <comdef.h>
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
    bwstring incrementingString = value;
   
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
    bwstring incrementingString = value;
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
* @bsimethod                                                    Dylan.Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool VerifyPair (IECInstancePtr source, ECValueAccessorPairCR pair)
    {
    ECValue original;
    if(ECOBJECTS_STATUS_Success != source->GetValueUsingAccessor (original, pair.GetAccessor()))
        return false;
    return original.Equals (pair.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring    GetTestSchemaXMLString (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor, const wchar_t* className)
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
                    L"    <ECClass typeName=\"Address\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"HouseNumber\"  typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Street\"       typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Town\"         typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"State\"        typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"Zip\"          typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"PhoneNumber\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"AreaCode\"     typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"Number\"       typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ContactInfo\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECStructProperty propertyName=\"PhoneNumber\" typeName=\"PhoneNumber\" />"
                    L"        <ECStructProperty propertyName=\"Address\"     typeName=\"Address\" />"
                    L"        <ECProperty       propertyName=\"Email\"       typeName=\"string\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"Employee\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty       propertyName=\"Name\"       typeName=\"string\" />"
                    L"        <ECStructProperty propertyName=\"Home\"       typeName=\"ContactInfo\" />"
                    L"        <ECStructProperty propertyName=\"Work\"       typeName=\"ContactInfo\" />"
                    L"        <ECStructProperty propertyName=\"Alternate\"  typeName=\"ContactInfo\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"EmployeeDirectory\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"Employees\" typeName=\"Employee\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
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
    bwstring schemaXMLString = GetTestSchemaXMLString (L"TestSchema", 0, 0, L"TestClass");

    EXPECT_EQ (S_OK, CoInitialize(NULL));  

    ECSchemaDeserializationContextPtr  schemaContext = ECSchemaDeserializationContext::CreateContext(schemaOwner, schemaOwner);

    ECSchemaP schema;        
    EXPECT_EQ (SUCCESS, ECSchema::ReadXmlFromString (schema, schemaXMLString.c_str(), *schemaContext));  

    CoUninitialize();
    return schema;
    }
    
typedef std::vector<bwstring> NameVector;
static std::vector<bwstring> s_propertyNames;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       CreateProfilingSchema (int nStrings, ECSchemaCacheR schemaOwner)
    {
    EXPECT_EQ (S_OK, CoInitialize(NULL)); 

    s_propertyNames.clear();
    
    bwstring schemaXml = 
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

    ECSchemaDeserializationContextPtr  schemaContext = ECSchemaDeserializationContext::CreateContext(schemaOwner, schemaOwner);

    ECSchemaP schema;        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, ECSchema::ReadXmlFromString (schema, schemaXml.c_str(), *schemaContext));

    CoUninitialize ();
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
    bwstring stringSeedXXX(stringSeed);
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

#ifdef OLD_WAY    
    ClassLayoutP manufClassLayout = ClassLayout::BuildFromClass (*manufacturerClass, 43, 24);
    StandaloneECEnablerPtr manufEnabler = StandaloneECEnabler::CreateEnabler (*manufacturerClass, *manufClassLayout, true);
#endif
    StandaloneECEnablerPtr manufEnabler =  instance.GetEnablerR().ObtainStandaloneInstanceEnabler (manufacturerClass->Schema.Name.c_str(), manufacturerClass->Name.c_str()); 
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

#ifdef  MUST_PUBLISH_ECInstanceInteropHelper
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetValuesUsingInteropHelper)
    {
    EXPECT_EQ (S_OK, CoInitialize(NULL)); 

    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 0, 0);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout, true);

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
    const wchar_t* stringValueP = NULL;

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetStringValue (*instance, L"ManufacturerArray[1].Name", testString.c_str()));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetString (*instance, stringValueP, L"ManufacturerArray[1].Name"));
    EXPECT_STREQ (testString.c_str(), stringValueP);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetStringValue (*instance, L"ManufacturerArray[0].Name", testString2.c_str()));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetString (*instance, stringValueP, L"ManufacturerArray[0].Name"));
    EXPECT_STREQ (testString2.c_str(), stringValueP);

    CoUninitialize();
    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ECValueEqualsMethod)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler =  schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str()); 

    ECValue v1, v2;
    EXPECT_TRUE   (v1.Equals(v2));
    v1.SetInteger (3425);
    v2.SetInteger (6548);
    EXPECT_FALSE  (v1.Equals (v2));
    v2.SetInteger (v1.GetInteger());
    EXPECT_TRUE   (v1.Equals (v2));

    v1.SetString  (L"Something");
    v2.SetString  (L"Something else");
    EXPECT_FALSE  (v1.Equals (v2));
    v2.SetString  (v1.GetString());
    EXPECT_TRUE   (v1.Equals (v2));

    //Conflicting types
    v2.SetInteger (3425);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetDouble  (1.0);
    v2.SetDouble  (1.0);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetDouble  (2.0);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetLong    ((Int64)345);
    v2.SetLong    ((Int64)345);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetLong    ((Int64)345345);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetBoolean (false);
    v2.SetBoolean (false);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetBoolean (true);
    EXPECT_FALSE  (v1.Equals (v2));

    SystemTime timeInput  = SystemTime::GetLocalTime();
    v1.SetDateTime(timeInput);
    v2.SetDateTime(timeInput);
    EXPECT_TRUE   (v1.Equals (v2));
    timeInput.wYear++;
    v2.SetDateTime(timeInput);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetDateTimeTicks((Int64)633487865666864601);
    v2.SetDateTimeTicks((Int64)633487865666864601);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetDateTimeTicks((Int64)633487865666866601);
    EXPECT_FALSE  (v1.Equals (v2));

    const static bool HOLD_AS_DUPLICATE = true;
    const byte binaryValue0[4] = {0x00, 0x01, 0x02, 0x03};
    const byte binaryValue1[4] = {0x10, 0x11, 0x12, 0x13};
    EXPECT_EQ (sizeof(binaryValue0), 4);
    EXPECT_EQ (sizeof(binaryValue0), 4);
    v1.SetBinary(binaryValue0, sizeof(binaryValue0), HOLD_AS_DUPLICATE);
    v2.SetBinary(binaryValue0, sizeof(binaryValue0), HOLD_AS_DUPLICATE);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetBinary(binaryValue1, sizeof(binaryValue1), HOLD_AS_DUPLICATE);
    EXPECT_FALSE  (v1.Equals (v2));

    DPoint2d   point2dInput0 = {1.0, 2.0};
    DPoint2d   point2dInput1 = {3.0, 4.0};
    v1.SetPoint2D (point2dInput0);
    v2.SetPoint2D (point2dInput0);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetPoint2D (point2dInput1);
    EXPECT_FALSE  (v1.Equals (v2));

    DPoint3d   point3dInput0 = {1.0, 2.0, -10.0};
    DPoint3d   point3dInput1 = {3.0, 4.0, -123.0};
    v1.SetPoint3D (point3dInput0);
    v2.SetPoint3D (point3dInput0);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetPoint3D (point3dInput1);
    EXPECT_FALSE  (v1.Equals (v2));

    EC::StandaloneECInstancePtr testInstance0 = enabler->CreateInstance();
    EC::StandaloneECInstancePtr testInstance1 = enabler->CreateInstance();
    v1.SetStruct  (testInstance0.get());
    v2.SetStruct  (testInstance0.get());
    EXPECT_TRUE   (v1.Equals(v2));
    v2.SetStruct  (testInstance1.get());
    EXPECT_FALSE  (v1.Equals (v2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetEnablerPropertyInformation)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str()); 

    const int expectedPropertyCount = 19;

    UInt32 propertyCount = enabler->GetPropertyCount();

    EXPECT_EQ (expectedPropertyCount, propertyCount);

    wchar_t* expectedProperties [expectedPropertyCount] = 
        {
        L"",
        L"AString",
        L"AnInt",
        L"APoint3d",
        L"APoint2d",
        L"ADouble",
        L"ADateTime",
        L"ABoolean",
        L"ALong",
        L"ABinary",
        L"SomeStrings[]",
        L"SomeInts[]",
        L"SomePoint3ds[]",
        L"SomePoint2ds[]",
        L"SomeDoubles[]",
        L"SomeDateTimes[]",
        L"SomeBooleans[]",
        L"SomeLongs[]",
        L"SomeBinaries[]"
        };

    for (UInt32 i=0; i < expectedPropertyCount; i++)
        {
        const wchar_t* expectedPropertyName = expectedProperties [i];
        const wchar_t* propertyName         = NULL;
        UInt32 propertyIndex          = 0;

        EXPECT_TRUE (ECOBJECTS_STATUS_Success == enabler->GetPropertyIndex (propertyIndex, expectedPropertyName));
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == enabler->GetAccessString  (propertyName,  propertyIndex));

        EXPECT_STREQ (expectedPropertyName, propertyName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpPropertyValues (ECValuesCollectionR collection)
    {
    for each (ECPropertyValuePtr propertyValue in collection)
        {
        ECValueAccessorCR   accessor = propertyValue->GetValueAccessor();
        const wchar_t *     accessString = accessor.GetAccessString (accessor.GetDepth() - 1);
        
        ECValue v;
        propertyValue->GetValue(v);

        printf ("%S = %S\n", accessString, v.ToString().c_str());

        if (propertyValue->HasChildValues ())
            {
            ECValuesCollection children = propertyValue->GetChildValues();
            dumpPropertyValues (children);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_PrimitiveArray)
    {
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaCache);
    ASSERT_TRUE (schema != NULL);

    StandaloneECEnablerPtr enabler = schemaCache->ObtainStandaloneInstanceEnabler (schema->GetName().c_str(), L"AllPrimitives");
    ASSERT_TRUE (enabler.IsValid());

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECValue v;
    v.SetString(L"Happy String");
    instance->SetValue(L"AString", v);

    v.SetInteger(6);
    instance->SetValue(L"AnInt", v);

    instance->AddArrayElements(L"SomeStrings[]", 3);

    v.SetString(L"ArrayMember 1");
    instance->SetValue(L"SomeStrings[]", v, 0);

    v.SetString(L"ArrayMember 2");
    instance->SetValue(L"SomeStrings[]", v, 1);

    v.SetString(L"ArrayMember 3");
    instance->SetValue(L"SomeStrings[]", v, 2);

    dumpPropertyValues (ECValuesCollection (*instance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void setContactInfo
(
wchar_t const * houseNumber,
wchar_t const * street,
wchar_t const * town,
wchar_t const * state,
int             zip,
int             areaCode,
int             phoneNumber,
wchar_t const * email,
IECInstanceR    instance
)
    {
    instance.SetValue(L"Address.HouseNumber",   ECValue (houseNumber));
    instance.SetValue(L"Address.Street",        ECValue (street));
    instance.SetValue(L"Address.Town",          ECValue (town));
    instance.SetValue(L"Address.State",         ECValue (state));
    instance.SetValue(L"Address.Zip",           ECValue (zip));
    instance.SetValue(L"PhoneNumber.AreaCode",  ECValue (areaCode));
    instance.SetValue(L"PhoneNumber.Number",    ECValue (phoneNumber));
    instance.SetValue(L"Email",                 ECValue (email));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_EmbeddedStructs)
    {
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaCache);
    ASSERT_TRUE (schema != NULL);

    StandaloneECEnablerPtr enabler = schemaCache->ObtainStandaloneInstanceEnabler (schema->GetName().c_str(), L"ContactInfo");
    ASSERT_TRUE (enabler.IsValid());

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    setContactInfo (L"123-4", L"Main Street", L"Exton", L"PA", 12345, 610, 1234567, L"nobody@nowhere.com", *instance);

    dumpPropertyValues (ECValuesCollection (*instance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_StructArray)
    {
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaCache);
    ASSERT_TRUE (schema != NULL);

    StandaloneECEnablerPtr enabler = schemaCache->ObtainStandaloneInstanceEnabler (schema->GetName().c_str(), L"EmployeeDirectory");
    ASSERT_TRUE (enabler.IsValid());

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    instance->AddArrayElements(L"Employees[]", 2);

    StandaloneECEnablerPtr arrayMemberEnabler = schemaCache->ObtainStandaloneInstanceEnabler (schema->GetName().c_str(), L"Employee");
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    EC::StandaloneECInstancePtr arrayMemberInstance1 = arrayMemberEnabler->CreateInstance();
    setContactInfo (L"123-4", L"Main Street", L"Exton", L"PA", 12345, 610, 1234567, L"nobody@nowhere.com", *arrayMemberInstance1);
    v.SetStruct(arrayMemberInstance1.get());
    instance->SetValue (L"Employees[]", v, 0);

    EC::StandaloneECInstancePtr arrayMemberInstance2 = arrayMemberEnabler->CreateInstance();
    setContactInfo (L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, 555, 1234567, L"president@whitehouse.gov", *arrayMemberInstance2);
    v.SetStruct(arrayMemberInstance2.get());
    instance->SetValue (L"Employees[]", v, 1);

    dumpPropertyValues (ECValuesCollection (*instance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, TestECValueEnumeration)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str()); 

    EC::StandaloneECInstancePtr sourceInstance = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance = enabler->CreateInstance();

    ECValue v;
    v.SetDouble(1.0/3.0);
    sourceInstance->SetValue(L"ADouble", v);
    v.SetInteger(234);
    sourceInstance->SetValue(L"AnInt", v);
    v.SetInteger(50);
    sourceInstance->SetValue(L"SomeInts[0]", v);
    v.SetInteger(60);
    sourceInstance->SetValue(L"SomeInts[1]", v);
    v.SetInteger(70);
    sourceInstance->SetValue(L"SomeInts[2]", v);
    v.SetInteger(80);
    sourceInstance->SetValue(L"SomeInts[3]", v);
    v.SetString(L"This is a string");
    sourceInstance->SetValue(L"AString", v);
    sourceInstance->SetValue(L"SomeStrings[0]", v);
    v.SetLong((Int64)2309480);
    sourceInstance->SetValue(L"ALong", v);
    sourceInstance->SetValue(L"SomeLongs[0]", v);
    v.SetBoolean(true);
    sourceInstance->SetValue(L"ABoolean", v);
    sourceInstance->SetValue(L"SomeBooleans[0]", v);
    DPoint2d   point2dInput = {1.0, 2.0};
    v.SetPoint2D(point2dInput);
    sourceInstance->SetValue(L"APoint2d", v);
    sourceInstance->SetValue(L"SomePoint2ds[0]", v);
    DPoint3d   point3dInput = {1.0, 2.0, 3.0};
    v.SetPoint3D(point3dInput);
    sourceInstance->SetValue(L"APoint3d", v);
    sourceInstance->SetValue(L"SomePoint3ds[0]", v);
    SystemTime timeInput  = SystemTime::GetLocalTime();
    v.SetDateTime(timeInput);
    sourceInstance->SetValue(L"ADateTime", v);
    sourceInstance->SetValue(L"SomeDateTimes[0]", v);

    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance, false);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        ECValue         value      = pair.GetValue();
        ECValueAccessor accessor   = pair.GetAccessor();

        //wprintf(L"%ls: %ls\n", accessor.GetManagedAccessString(), value.ToString());
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance->SetValueUsingAccessor(accessor, value));
        }

    ECValueAccessorPairCollectionOptionsPtr targetOptions = ECValueAccessorPairCollectionOptions::Create (*targetInstance, false);
    ECValueAccessorPairCollection targetCollection(*targetOptions);
    for each (ECValueAccessorPair pair in targetCollection)
        {
        ECValueAccessor accessor   = pair.GetAccessor();
        ECValue         value      = pair.GetValue();
        //wprintf(L"%ls: %ls\n", accessor.GetManagedAccessString(), value.ToString());
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }

    // instance.Compact()... then check values again
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, TestECValueEnumerationStructArray)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"ClassWithStructArray");
    ECClassP primitiveClass = schema->GetClassP (L"AllPrimitives");

    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler          = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str()); 
    StandaloneECEnablerPtr primitiveEnabler = schemaOwner->ObtainStandaloneInstanceEnabler (primitiveClass->Schema.Name.c_str(), primitiveClass->Name.c_str()); 

    EC::StandaloneECInstancePtr sourceInstance  = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance0 = enabler->CreateInstance();

    EC::StandaloneECInstancePtr primitiveInstance0 = primitiveEnabler->CreateInstance();
    EC::StandaloneECInstancePtr primitiveInstance1 = primitiveEnabler->CreateInstance();
    EC::StandaloneECInstancePtr primitiveInstance2 = primitiveEnabler->CreateInstance();

    sourceInstance->AddArrayElements(L"StructArray[]", 2);

    ECValue v;
    v.SetDouble(1.0/3.0);
    primitiveInstance0->SetValue(L"ADouble", v);
    v.SetInteger(234);
    primitiveInstance0->SetValue(L"AnInt", v);
    v.SetInteger(50);
    primitiveInstance0->AddArrayElements(L"SomeInts[]", 4);
    primitiveInstance0->SetValue(L"SomeInts[]", v, 0);
    v.SetInteger(60);
    primitiveInstance0->SetValue(L"SomeInts[]", v, 1);
    v.SetInteger(70);
    primitiveInstance0->SetValue(L"SomeInts[]", v, 2);
    v.SetInteger(80);
    primitiveInstance0->SetValue(L"SomeInts[]", v, 3);

    ECValue structArrayMemberPlaceholder;
    structArrayMemberPlaceholder.SetStruct(primitiveInstance0.get());
    sourceInstance->SetValue(L"StructArray[]", structArrayMemberPlaceholder, 0);

    v.SetString(L"This is a string");
    primitiveInstance1->SetValue(L"AString", v);
    primitiveInstance1->AddArrayElements(L"SomeStrings[]", 1);
    primitiveInstance1->SetValue(L"SomeStrings[]", v, 0);
    v.SetLong((Int64)2309480);
    primitiveInstance1->SetValue(L"ALong", v);
    primitiveInstance1->AddArrayElements(L"SomeLongs[]", 1);
    primitiveInstance1->SetValue(L"SomeLongs[]", v, 0);
    v.SetBoolean(true);
    primitiveInstance1->SetValue(L"ABoolean", v);
    primitiveInstance1->AddArrayElements(L"SomeBooleans[]", 1);
    primitiveInstance1->SetValue(L"SomeBooleans[]", v, 0);

    structArrayMemberPlaceholder.SetStruct(primitiveInstance1.get());
    sourceInstance->SetValue(L"StructArray[]", structArrayMemberPlaceholder, 1);

    DPoint2d   point2dInput = {1.0, 2.0};
    v.SetPoint2D(point2dInput);
    sourceInstance->SetValue(L"StructMember.APoint2d", v);
    sourceInstance->AddArrayElements(L"StructMember.SomePoint2ds[]", 1);
    sourceInstance->SetValue(L"StructMember.SomePoint2ds[]", v, 0);
    DPoint3d   point3dInput = {1.0, 2.0, 3.0};
    v.SetPoint3D(point3dInput);
    sourceInstance->SetValue(L"StructMember.APoint3d", v);
    sourceInstance->AddArrayElements(L"StructMember.SomePoint3ds[]", 1);
    sourceInstance->SetValue(L"StructMember.SomePoint3ds[]", v, 0);
    SystemTime timeInput  = SystemTime::GetLocalTime();
    v.SetDateTime(timeInput);
    sourceInstance->SetValue(L"StructMember.ADateTime", v);
    sourceInstance->AddArrayElements(L"StructMember.SomeDateTimes[]", 1);
    sourceInstance->SetValue(L"StructMember.SomeDateTimes[]", v, 0);


    int valuesFound;
    //Below, true indicates that null values will be included.
    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance, true);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);

    //Enumerate all values (including nulls.)  Does not output.
    valuesFound = 0;
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        valuesFound ++;
        }
    EXPECT_TRUE (18 < valuesFound);

    valuesFound = 0;
    sourceOptions->SetIncludesNullValues(false);
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        valuesFound ++;
        ECValueAccessor accessor   = pair.GetAccessor();
        ECValue         value      = pair.GetValue();
        //wprintf(L"%ls: %ls\n", accessor.GetManagedAccessString(), value.ToString());
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance0->SetValueUsingAccessor (accessor, value));
        }
    EXPECT_EQ (18, valuesFound);

    valuesFound = 0;   
    ECValueAccessorPairCollectionOptionsPtr targetOptions0 = ECValueAccessorPairCollectionOptions::Create (*targetInstance0, false);
    ECValueAccessorPairCollection targetCollection0(*targetOptions0);
    for each (ECValueAccessorPair pair in targetCollection0)
        {
        valuesFound ++;
        ECValueAccessor accessor = pair.GetAccessor();
        ECValue value = pair.GetValue();
        ECValue temp;
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance0->GetValueUsingAccessor (temp, accessor));
        EXPECT_TRUE (value.Equals (temp));
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    EXPECT_EQ (18, valuesFound);

    valuesFound = 0;   
    ECValueAccessorPairCollectionOptionsPtr targetOptions1 = ECValueAccessorPairCollectionOptions::Create (*targetInstance0, false);
    ECValueAccessorPairCollection targetCollection1(*targetOptions0);
    for each (ECValueAccessorPair pair in targetCollection1)
        {
        valuesFound ++;
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    EXPECT_EQ (18, valuesFound);

    StandaloneECInstancePtr duplicatedTarget = StandaloneECInstance::Duplicate (*targetInstance0);
    ECValueAccessorPairCollectionOptionsPtr duplicatedOptions = ECValueAccessorPairCollectionOptions::Create (*duplicatedTarget, false);
    ECValueAccessorPairCollection duplicatedTargetCollection(*duplicatedOptions);
    valuesFound = 0;
    for each (ECValueAccessorPair pair in duplicatedTargetCollection)
        {
        valuesFound++;
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    EXPECT_EQ (18, valuesFound); 
    
    // instance.Compact()... then check values again
    
    };

//#ifdef NEEDSWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ECValueEnumerationOverFixedSizeArrays)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass0 = schema->GetClassP (L"FixedSizeArrayTester");
    ASSERT_TRUE (ecClass0);
    ECClassP ecClass1 = schema->GetClassP (L"BaseClass0");
    ASSERT_TRUE (ecClass1);

    StandaloneECEnablerPtr enabler          = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass0->Schema.Name.c_str(), ecClass0->Name.c_str()); 
    StandaloneECEnablerPtr primitiveEnabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass1->Schema.Name.c_str(), ecClass1->Name.c_str()); 

    StandaloneECInstancePtr sourceInstance  = enabler->CreateInstance();
    StandaloneECInstancePtr targetInstance  = enabler->CreateInstance();

    ECValue v;
    v.SetString (L"a fixed string");
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == sourceInstance->SetValue (L"FixedString1[]", v, 0));

    for (int i=0; i<10; i++)
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == sourceInstance->SetValue (L"FixedString10[]", v, i));

    v.SetInteger (44);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == sourceInstance->SetValue (L"FixedInt1[]", v, 0));

    for (int i=0; i<10; i++)
        {
        v.SetInteger (i*i*i);
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == sourceInstance->SetValue (L"FixedInt10[]", v, i));
        }

    IECInstancePtr instance0 = primitiveEnabler->CreateInstance();
    ECValue vForSettingStruct;
    vForSettingStruct.SetInteger (5);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == instance0->SetValue (L"BaseIntProperty", vForSettingStruct));
    v.SetStruct (instance0.get());
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == sourceInstance->SetValue (L"Struct1[]", v, 0));

    for (int i=0; i<10; i++)
        {
        instance0 = primitiveEnabler->CreateInstance();
        vForSettingStruct.SetInteger(i*i*i);
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == instance0->SetValue(L"BaseIntProperty", vForSettingStruct));
        v.SetStruct (instance0.get());
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == sourceInstance->SetValue (L"Struct10[]", v, i));
        }

    EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, L"FixedString1[]", 0));
    EXPECT_FALSE (v.IsNull());
    EXPECT_STREQ (L"a fixed string", v.GetString());

    for (int i=0; i<10; i++)
        {
        EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, L"FixedString10[]", i));
        EXPECT_FALSE (v.IsNull());
        EXPECT_STREQ (L"a fixed string", v.GetString());
        }

    EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, L"FixedInt1[]", 0));
    EXPECT_FALSE (v.IsNull());
    EXPECT_EQ    (44, v.GetInteger());

    for (int i=0; i<10; i++)
        {
        EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, L"FixedInt10[]", i));
        EXPECT_FALSE (v.IsNull());
        EXPECT_EQ    (i*i*i, v.GetInteger());
        }

    EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, L"Struct1[]", 0));
    EXPECT_FALSE (v.IsNull());
    instance0   = v.GetStruct();
    EXPECT_TRUE  (ECOBJECTS_STATUS_Success == instance0->GetValue (v, L"BaseIntProperty"));
    EXPECT_EQ    (5, v.GetInteger());

    for (int i=0; i<10; i++)
        {
        EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, L"Struct10[]", i));
        EXPECT_FALSE (v.IsNull());
        instance0   = v.GetStruct();
        EXPECT_TRUE  (ECOBJECTS_STATUS_Success == instance0->GetValue (v, L"BaseIntProperty"));
        EXPECT_EQ    (i*i*i, v.GetInteger());
        }

    UInt32 propertyIndexOfStructArray;
    enabler->GetPropertyIndex (propertyIndexOfStructArray, L"Struct1[]");

    EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, propertyIndexOfStructArray, 0));
    EXPECT_FALSE (v.IsNull());
    instance0   = v.GetStruct();
    EXPECT_TRUE  (ECOBJECTS_STATUS_Success == instance0->GetValue (v, L"BaseIntProperty"));
    EXPECT_EQ    (5, v.GetInteger());

    enabler->GetPropertyIndex (propertyIndexOfStructArray, L"Struct10[]");
    for (int i=0; i<10; i++)
        {
        EXPECT_TRUE  (ECOBJECTS_STATUS_Success == sourceInstance->GetValue (v, propertyIndexOfStructArray, i));
        EXPECT_FALSE (v.IsNull());
        instance0   = v.GetStruct();
        EXPECT_TRUE  (ECOBJECTS_STATUS_Success == instance0->GetValue (v, L"BaseIntProperty"));
        EXPECT_EQ    (i*i*i, v.GetInteger());
        }

    int valuesFound;
    valuesFound = 0;
    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance, false);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        valuesFound ++;
        ECValueAccessor accessor = pair.GetAccessor();
        ECValue         value = pair.GetValue();
        //wprintf(L"%ls: %ls\n", accessor.GetManagedAccessString(), value.ToString());
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance->SetValueUsingAccessor (accessor, value));
        }
    EXPECT_EQ (33, valuesFound);

    //Copy to target instance
    //wprintf(L"\nEnumerating values in target instance...\n");
    valuesFound = 0;   
    ECValueAccessorPairCollectionOptionsPtr targetOptions = ECValueAccessorPairCollectionOptions::Create (*targetInstance, false);
    ECValueAccessorPairCollection targetCollection(*targetOptions);
    for each (ECValueAccessorPair pair in targetCollection)
        {
        valuesFound ++;
        //ECValueAccessor accessor = pair.GetAccessor();
        //ECValue value = pair.GetValue();
        //wprintf(L"%ls: %ls", accessor.GetManagedAccessString(), value.ToString());
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    EXPECT_EQ (33, valuesFound);

    };
//#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ECValueEnumerationPerformance)
    {
    //The purpose of this test is to see the performance loss from using two different class
    //layouts.
#ifdef IGNORE____
    // CGM - this CoInitialize is only needed to de-serialize the schema.  Ideally, CreateTestSchema should handle this since the test shouldn't need to know
    // that implementation detail
    // DHR - I stuck this in CreateTestSchema(), but I don't see an easy way around having to CoUninitialize()
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    //These two classes are thought to have different ClassLayouts.
    ECClassP ecClass0 = schema->GetClassP (L"ClassLayoutPerformanceTest0");
    ASSERT_TRUE (ecClass0);
    ECClassP ecClass1 = schema->GetClassP (L"ClassLayoutPerformanceTest1");
    ASSERT_TRUE (ecClass1);

    StandaloneECEnablerPtr enabler          = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass0->Schema.Name.c_str(), ecClass0->Name.c_str()); 
    StandaloneECEnablerPtr alternateEnabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass1->Schema.Name.c_str(), ecClass1->Name.c_str()); 

    EC::StandaloneECInstancePtr sourceInstance  = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance0 = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance1 = alternateEnabler->CreateInstance();

    ECValue v;
    SetAndVerifyDouble (*sourceInstance, v, L"ADouble", 5.8);
    SetAndVerifyInteger (*sourceInstance, v, L"AnInt", 8851);
    SetAndVerifyString (*sourceInstance, v, L"AString", L"Do or do not, there is no try.");

    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance, false);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);

    UInt32      numAccesses = 300000;

    double      elapsedTime1 = 0.0;
    StopWatch   timer1 (L"Time setting values using index", true);


    for (UInt32 i = 0; i < numAccesses; i++)
        {
        timer1.Start();
        for each (ECValueAccessorPair pair in sourceCollection)
            {
            targetInstance0->SetValueUsingAccessor(pair.GetAccessor(), pair.GetValue());
            }
        timer1.Stop();

        elapsedTime1 += timer1.GetElapsedSeconds();
        }

    double      elapsedTime2 = 0.0;
    StopWatch   timer2 (L"Time setting values using accessString", true);
    for (UInt32 i = 0; i < numAccesses; i++)
        {
        timer2.Start();
        for each (ECValueAccessorPair pair in sourceCollection)
            {
            targetInstance1->SetValueUsingAccessor(pair.GetAccessor(), pair.GetValue());
            }
        timer2.Stop();

        elapsedTime2 += timer2.GetElapsedSeconds();
        }

    double      elapsedTime3 = 0.0;
    StopWatch   timer3 (L"Time setting values using interop helper", true);
    for (UInt32 i = 0; i < numAccesses; i++)
        {
        timer3.Start();
        ECValue v;
        ECInstanceInteropHelper::GetValue  (*sourceInstance, v, L"ADouble");
        ECInstanceInteropHelper::SetValue  (*targetInstance0, L"ADouble", v);

        ECInstanceInteropHelper::GetValue (*sourceInstance, v, L"AnInt");
        ECInstanceInteropHelper::SetValue (*targetInstance0, L"AnInt", v);

        ECInstanceInteropHelper::GetValue  (*sourceInstance, v, L"AString");
        ECInstanceInteropHelper::SetValue  (*targetInstance0, L"AString", v);
        timer3.Stop();
        elapsedTime3 += timer3.GetElapsedSeconds();
        }

    double      elapsedTime4 = 0.0;
    StopWatch   timer4 (L"Time setting values using interop helper and enum", true);
    for (UInt32 i = 0; i < numAccesses; i++)
        {
        timer4.Start();
        for each (ECValueAccessorPair pair in sourceCollection)
            {
            ECInstanceInteropHelper::SetValue (*targetInstance0, pair.GetAccessor().GetManagedAccessString().c_str(), pair.GetValue());
            }
        timer4.Stop();
        elapsedTime4 += timer4.GetElapsedSeconds();
        }
    
    wprintf (L"Time to set %d values by: index = %.4f, \n\
              access string = %.4f, \n\
              interop helper - harcoded access strings = %.4f\n\
              interop helper - enumerated access strings = %.4f\n\
              \n", numAccesses, elapsedTime1, elapsedTime2, elapsedTime3, elapsedTime4);

#endif

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, PolymorphicStructArrayEnumeration)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);

    ECClassP ecClass = schema->GetClassP (L"ClassWithPolymorphicStructArray");
    ASSERT_TRUE (ecClass);
    ECClassP derivedClass0 = schema->GetClassP (L"DerivedClass0");
    ASSERT_TRUE (derivedClass0);
    ECClassP derivedClass1 = schema->GetClassP (L"DerivedClass1");
    ASSERT_TRUE (derivedClass1);

    StandaloneECEnablerPtr enabler   = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str()); 
    StandaloneECEnablerPtr dEnabler0 = schemaOwner->ObtainStandaloneInstanceEnabler (derivedClass0->Schema.Name.c_str(), derivedClass0->Name.c_str()); 
    StandaloneECEnablerPtr dEnabler1 = schemaOwner->ObtainStandaloneInstanceEnabler (derivedClass1->Schema.Name.c_str(), derivedClass1->Name.c_str()); 

    EC::StandaloneECInstancePtr dInstance0 = dEnabler0->CreateInstance();

    ECValue v;
    v.SetInteger(1);
    dInstance0->SetValue(L"BaseIntProperty", v);
    v.SetString(L"Hello, polymorphism!");
    dInstance0->SetValue(L"DerivedStringProperty", v);

    EC::StandaloneECInstancePtr dInstance1 = dEnabler1->CreateInstance();

    v.SetInteger(1);
    dInstance1->SetValue(L"BaseIntProperty", v);
    v.SetDouble(3.1415);
    dInstance1->SetValue(L"DerivedDoubleProperty", v);

    EC::StandaloneECInstancePtr sourceInstance = enabler->CreateInstance();

    sourceInstance->AddArrayElements(L"PolymorphicStructArray[]", 2);
    v.SetStruct(dInstance0.get());
    sourceInstance->SetValue(L"PolymorphicStructArray[]", v, 0);
    v.SetStruct(dInstance1.get());
    sourceInstance->SetValue(L"PolymorphicStructArray[]", v, 1);

    EC::StandaloneECInstancePtr targetInstance = enabler->CreateInstance();

    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance, false);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);

    int valuesFound;

    valuesFound = 0;
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        valuesFound++;
        ECValue value = pair.GetValue();
        ECValueAccessor accessor = pair.GetAccessor();
        }
    EXPECT_EQ (4, valuesFound); 

    //Set those values to a target instance
    valuesFound = 0;
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        valuesFound++;
        ECValue value = pair.GetValue();
        ECValueAccessor accessor = pair.GetAccessor();
        targetInstance->SetValueUsingAccessor (accessor, value);
        }
    ASSERT_TRUE (4 == valuesFound); 

    ECValueAccessorPairCollectionOptionsPtr targetOptions = ECValueAccessorPairCollectionOptions::Create (*targetInstance, false);
    ECValueAccessorPairCollection targetCollection(*targetOptions);
    valuesFound = 0;
    for each (ECValueAccessorPair pair in targetCollection)
        {
        valuesFound++;
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    ASSERT_TRUE (4 == valuesFound); 

    StandaloneECInstancePtr duplicatedTarget = StandaloneECInstance::Duplicate (*targetInstance);
    ECValueAccessorPairCollectionOptionsPtr duplicatedTargetOptions = ECValueAccessorPairCollectionOptions::Create (*duplicatedTarget, false);
    ECValueAccessorPairCollection duplicatedTargetCollection(*duplicatedTargetOptions);
    valuesFound = 0;
    for each (ECValueAccessorPair pair in duplicatedTargetCollection)
        {
        valuesFound++;
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    ASSERT_TRUE (4 == valuesFound); 
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ManualUseOfAccessors)
    {
    ECSchemaCachePtr schemaSession = ECSchemaCache::Create ();
    ECSchemaP        schema = CreateTestSchema(*schemaSession);
    ASSERT_TRUE (schema != NULL);

    StandaloneECEnablerPtr  enabler                 =  schemaSession->ObtainStandaloneInstanceEnabler (schema->Name.c_str(), L"ClassWithStructArray");
    StandaloneECEnablerPtr  structEnabler           =  schemaSession->ObtainStandaloneInstanceEnabler (schema->Name.c_str(), L"AllPrimitives");
    StandaloneECEnablerPtr  nestedStuctEnabler      =  schemaSession->ObtainStandaloneInstanceEnabler (schema->Name.c_str(), L"NestedStructArray");
    StandaloneECEnablerPtr  manufactureStuctEnabler =  schemaSession->ObtainStandaloneInstanceEnabler (schema->Name.c_str(), L"Manufacturer");

    EC::StandaloneECInstancePtr sourceInstance  = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance0 = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance1 = enabler->CreateInstance();

    ECValueAccessor accessor;
    ECValue         v;
    ECValue         readValue;

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == ECValueAccessor::PopulateValueAccessor (accessor, *sourceInstance, L"StructMember.ADouble"));
    v.SetDouble(3.1415);
    sourceInstance->SetValueUsingAccessor (accessor, v);
    sourceInstance->GetValueUsingAccessor (readValue, accessor);

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == ECValueAccessor::PopulateValueAccessor (accessor, *sourceInstance, L"StructMember.SomeInts"));
    sourceInstance->GetValueUsingAccessor (v, accessor);
    ArrayInfo arrayInfo = v.GetArrayInfo ();
    UInt32 count = arrayInfo.GetCount();
    ASSERT_TRUE (0 == count); 
    UInt32 newCount=5;

    for (UInt32 newIndex=0; newIndex<newCount; newIndex++)
        {
        // set the accessor to add a new entry into the array
        accessor.DeepestLocation().arrayIndex = newIndex;
        v.SetInteger(100*newIndex);
        sourceInstance->SetValueUsingAccessor(accessor, v);
        sourceInstance->GetValueUsingAccessor (readValue, accessor);
        }

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == ECValueAccessor::PopulateValueAccessor (accessor, *sourceInstance, L"StructMember.SomeInts"));
    sourceInstance->GetValueUsingAccessor (v, accessor);
    arrayInfo = v.GetArrayInfo ();
    count = arrayInfo.GetCount();
    ASSERT_TRUE (count == newCount); 

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == ECValueAccessor::PopulateValueAccessor (accessor, *sourceInstance, L"ComplicatedStructArray"));
    sourceInstance->GetValueUsingAccessor (v, accessor);
    arrayInfo = v.GetArrayInfo ();
    count = arrayInfo.GetCount();
    ASSERT_TRUE (0 == count); 

    // set the accessor to add a new entry into the ComplicatedStructArray array
    accessor.DeepestLocation().arrayIndex = 0;

    accessor.PushLocation(*nestedStuctEnabler, L"NestPropString", -1);
    v.SetString(L"It is possible to use accessors manually.");
    sourceInstance->SetValueUsingAccessor(accessor, v);
    sourceInstance->GetValueUsingAccessor (readValue, accessor);
    accessor.PopLocation();    // remove Location of NestPropString

    // push to the first array entry
    accessor.PushLocation(*nestedStuctEnabler, L"ManufacturerArray[]", 0);
    accessor.PushLocation(*manufactureStuctEnabler, L"Name");

    // this will force a new standalone instance to get created
    v.SetString(L"Nissan");
    sourceInstance->SetValueUsingAccessor(accessor, v);
    sourceInstance->GetValueUsingAccessor (readValue, accessor);

    accessor.PopLocation();
    accessor.PushLocation(*manufactureStuctEnabler, L"AccountNo");
    v.SetInteger(3475);
    sourceInstance->SetValueUsingAccessor(accessor, v);
    sourceInstance->GetValueUsingAccessor (readValue, accessor);

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == ECValueAccessor::PopulateValueAccessor (accessor, *sourceInstance, L"ComplicatedStructArray[0].ManufacturerArray[0].Name")); 
    sourceInstance->GetValueUsingAccessor (readValue, accessor);

    accessor.Clear ();
    accessor.PushLocation(*sourceInstance, L"StructMember.ADouble", -1);
    v.SetDouble(3.1415);
    sourceInstance->SetValueUsingAccessor(accessor, v);
    accessor.PopLocation();

    accessor.PushLocation(*sourceInstance, L"StructMember.AnInt", -1);
    v.SetInteger(234);
    sourceInstance->SetValueUsingAccessor(accessor, v);
    accessor.PopLocation();

    accessor.PushLocation(*sourceInstance, L"StructMember.SomeInts[]", 0);
    v.SetInteger(432);
    sourceInstance->SetValueUsingAccessor(accessor, v);
    accessor.PopLocation();
    accessor.PushLocation(*sourceInstance, L"StructMember.SomeInts[]", 1);
    v.SetInteger(555);
    sourceInstance->SetValueUsingAccessor(accessor, v);
    accessor.PopLocation();

    //Step into struct array
    accessor.PushLocation(*sourceInstance, L"StructArray[]", 0);

    accessor.PushLocation(*structEnabler, L"AString", -1);
    v.SetString(L"It is possible to use accessors manually.");
    sourceInstance->SetValueUsingAccessor(accessor, v);
    accessor.PopLocation();
    accessor.PushLocation(*structEnabler, L"ALong", -1);
    v.SetLong(5846543);
    sourceInstance->SetValueUsingAccessor(accessor, v);
    accessor.PopLocation();

    int valuesFound;

    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance, false);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);

    valuesFound = 0;
    sourceOptions->SetIncludesNullValues(true);
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        ECValueAccessor accessor   = pair.GetAccessor();
        wprintf (L"%ls\n", accessor.GetManagedAccessString());
        valuesFound ++;
        }
    EXPECT_TRUE (12 < valuesFound);

    valuesFound = 0;
    sourceOptions->SetIncludesNullValues(false);
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        valuesFound ++;
        ECValueAccessor accessor   = pair.GetAccessor();
        ECValue         value      = pair.GetValue();
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance0->SetValueUsingAccessor (accessor, value));
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance1->SetValueUsingAccessor (accessor, value));
        }
    EXPECT_EQ (12, valuesFound); 

    //Copy to target instance
    valuesFound = 0;   
    ECValueAccessorPairCollectionOptionsPtr targetOptions0 = ECValueAccessorPairCollectionOptions::Create (*targetInstance0, false);
    ECValueAccessorPairCollection targetCollection0(*targetOptions0);
    for each (ECValueAccessorPair pair in targetCollection0)
        {
        valuesFound ++;
        ECValueAccessorCR accessor = pair.GetAccessor();
        ECValueCR value = pair.GetValue();
        ECValue temp;
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == targetInstance1->GetValueUsingAccessor (temp, accessor));
        EXPECT_TRUE (value.Equals (temp));
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    EXPECT_EQ (12, valuesFound); 

    valuesFound = 0;   
    ECValueAccessorPairCollectionOptionsPtr targetOptions1 = ECValueAccessorPairCollectionOptions::Create (*targetInstance1, false);
    ECValueAccessorPairCollection targetCollection1(*targetOptions1);
    for each (ECValueAccessorPair pair in targetCollection1)
        {
        valuesFound ++;
        EXPECT_TRUE (VerifyPair (sourceInstance, pair));
        }
    EXPECT_EQ (12, valuesFound); 
    
    // instance.Compact()... then check values again
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, SimpleMergeTwoInstances)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP primitiveClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (primitiveClass);

    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (primitiveClass->Schema.Name.c_str(), primitiveClass->Name.c_str());
    
    EC::StandaloneECInstancePtr sourceInstance0 = enabler->CreateInstance();
    EC::StandaloneECInstancePtr sourceInstance1 = enabler->CreateInstance();
    EC::StandaloneECInstancePtr targetInstance  = enabler->CreateInstance();

    ECValue v;
    v.SetDouble(1.0/3.0);
    sourceInstance0->SetValue(L"ADouble", v);
    v.SetString(L"Weaker source instance");
    sourceInstance0->SetValue(L"AString", v);
    v.SetInteger(234);
    sourceInstance0->SetValue(L"AnInt", v);
    v.SetInteger(50);
    sourceInstance0->AddArrayElements(L"SomeInts[]", 4);
    sourceInstance0->SetValue(L"SomeInts[]", v, 0);
    v.SetInteger(60);
    sourceInstance0->SetValue(L"SomeInts[]", v, 1);
    v.SetInteger(70);
    sourceInstance0->SetValue(L"SomeInts[]", v, 2);
    v.SetInteger(80);
    sourceInstance0->SetValue(L"SomeInts[]", v, 3);

    v.SetDouble(10.0/3.0);
    sourceInstance1->SetValue(L"ADouble", v);
    v.SetLong((Int64)2345978);
    sourceInstance1->SetValue(L"ALong", v);
    v.SetString(L"Dominant source instance");
    sourceInstance1->SetValue(L"AString", v);
    v.SetInteger(99999999);
    sourceInstance1->AddArrayElements(L"SomeInts[]", 4);
    sourceInstance1->SetValue(L"SomeInts[]", v, 1);

    /*
    Merging two instances into a third instance:
    In this example, values from sourceInstance 1 will take precedence over 
    values in sourceInstance0 in the even that neither are null.
    Note that in Options::Create (), the second flag is set to true: in this
    case, it is wise to include accessors that have null values.
    */
    ECValueAccessorPairCollectionOptionsPtr sourceOptions = ECValueAccessorPairCollectionOptions::Create (*sourceInstance1, true);
    ECValueAccessorPairCollection sourceCollection(*sourceOptions);
    for each (ECValueAccessorPair pair in sourceCollection)
        {
        //value came from sourceInstance1
        ECValue value = pair.GetValue();
        //if the value is null, get it from sourceInstance0
        if(value.IsNull())
            sourceInstance0->GetValueUsingAccessor (value, pair.GetAccessor());
        //set the value to target instance
        if(!value.IsNull())
            targetInstance->SetValueUsingAccessor (pair.GetAccessor(), value);
        }

    int valuesCounted = 0;
    ECValueAccessorPairCollectionOptionsPtr targetOptions = ECValueAccessorPairCollectionOptions::Create (*targetInstance, false);
    ECValueAccessorPairCollection targetCollection(*targetOptions);
    for each (ECValueAccessorPair pair in targetCollection)
        {
        valuesCounted++;
        //wprintf(L"%ls: %ls\n", pair.GetAccessor().GetManagedAccessString(), pair.GetValue().ToString());
        }
    //Verify that the merge succeeded
    EXPECT_EQ (8, valuesCounted);
    targetInstance->GetValue (v, L"AnInt");    //Came from sourceInstance0
    EXPECT_EQ (234, v.GetInteger());
    targetInstance->GetValue (v, L"ADouble");  //Came from sourceInstance1
    EXPECT_EQ (10.0/3.0, v.GetDouble());
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
    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    bwstring instanceId = instance->GetInstanceId();
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
    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());

    EC::StandaloneECInstancePtr instance = enabler->CreateInstance();
    bwstring instanceId = instance->GetInstanceId();
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
    ASSERT_TRUE (ecClass);
    
    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());
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
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetSetValuesByIndex)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();;
    ECSchemaP        schema = CreateTestSchema(*schemaOwner);
    ASSERT_TRUE (schema != NULL);
    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);
    
    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());
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
    ASSERT_TRUE (ecClass);    
    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());
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
    bwstring point3Str = pntVal3.ToString();
    EXPECT_TRUE (0 == point3Str.compare (L"{10,100,1000}"));

    //DPoint2d
    DPoint2d inPoint2 = {10.0, 100.0};
    ECValue pntVal2 (inPoint2);
    EXPECT_TRUE (pntVal2.IsPoint2D());
    DPoint2d outPoint2 = pntVal2.GetPoint2D ();
    EXPECT_TRUE (0 == memcmp(&inPoint2, &outPoint2, sizeof(outPoint2)));
    bwstring point2Str = pntVal2.ToString();
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
    bwstring dateStr = fixedDate.ToString();
    EXPECT_TRUE (0 == dateStr.compare (L"#2010/2/25-16:28:27:91#"));

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
    ASSERT_TRUE (ecClass);
        
    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());
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
    
TEST_F (MemoryLayoutTests, ProfileSettingValues)
    {
    int nStrings = 100;
    int nInstances = 1000;

    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create();
    ECSchemaP          schema      = CreateProfilingSchema(nStrings, *schemaOwner);
    ECClassP           ecClass     = schema->GetClassP (L"Pidget");
    ASSERT_TRUE (ecClass);
        
    StandaloneECEnablerPtr enabler = schemaOwner->ObtainStandaloneInstanceEnabler (ecClass->Schema.Name.c_str(), ecClass->Name.c_str());
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
    };
    

END_BENTLEY_EC_NAMESPACE