/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"


#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <Bentley/BeTimeUtilities.h>

#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

#define FIXED_COUNT_ARRAYS_ARE_SUPPORTED 0

using namespace std;
USING_NAMESPACE_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct MemoryLayoutTests : ECTestFixture {};

namespace {

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

        ECObjectsStatus status = instance.SetValue (accessString, v, i);
        EXPECT_TRUE (SUCCESS == status);
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

        ECObjectsStatus status = instance.SetValue (accessString, v, i);
        EXPECT_TRUE (SUCCESS == status);
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
                    L"    <ECClass typeName=\"ArrayTest\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"SomeStrings\" typeName=\"string\" />"
                    L"        <ECArrayProperty propertyName=\"SomeInts\"    typeName=\"int\" />"
                    L"        <ECArrayProperty propertyName=\"SomePoint3ds\"    typeName=\"point3d\" />"
                    L"        <ECArrayProperty propertyName=\"SomePoint2ds\"    typeName=\"point2d\" />"
                    L"        <ECArrayProperty propertyName=\"SomeDoubles\"     typeName=\"double\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeDateTimes\"   typeName=\"dateTime\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeBooleans\"    typeName=\"boolean\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeLongs\"       typeName=\"long\"  />"
                    L"        <ECArrayProperty propertyName=\"SomeBinaries\"    typeName=\"binary\"  />"
                    L"        <ECArrayProperty propertyName=\"FixedArrayFixedElement\" typeName=\"int\" minOccurs=\"10\" maxOccurs=\"10\"/>"  
                    L"        <ECArrayProperty propertyName=\"FixedArrayVariableElement\" typeName=\"string\" minOccurs=\"12\" maxOccurs=\"12\"/>"  
                    L"        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\" />"
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
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"EmployeeDirectory\" isDomainClass=\"True\">"
                    L"        <ECArrayProperty propertyName=\"Employees\" typeName=\"Employee\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"Car\" isStruct=\"True\" isDomainClass=\"True\">"
                    L"        <ECProperty       propertyName=\"Name\"       typeName=\"string\"/>"
                    L"        <ECProperty       propertyName=\"Wheels\"     typeName=\"int\"  readOnly=\"True\"/>"
                    L"    </ECClass>"
                    L"  <ECClass typeName=\"StructClass\" isStruct=\"True\" isDomainClass=\"False\">"
                    L"    <ECProperty propertyName=\"StringProperty\" typeName=\"string\" /> "
                    L"    <ECProperty propertyName=\"IntProperty\" typeName=\"int\" /> "
                    L"    <ECArrayProperty propertyName=\"ArrayProperty\" typeName=\"string\" minOccurs=\"1\" maxOccurs=\"1\" /> "
                    L"    </ECClass>"
                    L"  <ECClass typeName=\"ComplexClass\" isDomainClass=\"True\">"
                    L"    <ECProperty propertyName=\"IntProperty\" typeName=\"int\" />" 
                    L"    <ECProperty propertyName=\"StringProperty\" typeName=\"string\" /> "
                    L"    <ECProperty propertyName=\"DoubleProperty\" typeName=\"double\" /> "
                    L"    <ECProperty propertyName=\"DateTimeProperty\" typeName=\"dateTime\" />" 
                    L"    <ECProperty propertyName=\"BooleanProperty\" typeName=\"boolean\" />" 
                    L"    <ECArrayProperty propertyName=\"SimpleArrayProperty\" typeName=\"string\" minOccurs=\"1\" maxOccurs=\"1\" />"
                    L"    <ECArrayProperty propertyName=\"StructArrayProperty\" typeName=\"StructClass\" minOccurs=\"1\" maxOccurs=\"1\" isStruct=\"True\" />" 
                    L"    <ECStructProperty propertyName=\"StructProperty\" typeName=\"StructClass\" />" 
                    L"  </ECClass>"
                    L"</ECSchema>";

    WString buff;

    buff.Sprintf (fmt, schemaName, versionMajor, versionMinor, className);

    return buff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr       CreateTestSchema ()
    {
    WString schemaXMLString = GetTestSchemaXMLString (L"TestSchema", 0, 0, L"TestClass");

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (schema, schemaXMLString.c_str(), *schemaContext));
    return schema;
    }
    
typedef std::vector<WString> NameVector;
static std::vector<WString> s_propertyNames;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     CreateProfilingSchema (int nStrings)
    {
    s_propertyNames.clear();
    
    WString schemaXml = 
                    L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"ProfilingSchema\" nameSpacePrefix=\"p\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"Pidget\" isDomainClass=\"True\">";

    for (int i = 0; i < nStrings; i++)
        {
        WString propertyName;
        propertyName.Sprintf (L"StringProperty%02d", i);
        s_propertyNames.push_back (propertyName);
        WCharCP propertyFormat = 
                    L"        <ECProperty propertyName=\"%s\" typeName=\"string\" />";
        WString propertyXml;
        propertyXml.Sprintf (propertyFormat, propertyName.c_str());
        schemaXml += propertyXml;
        }                    

    schemaXml +=    L"    </ECClass>"
                    L"</ECSchema>";

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
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
        WString propertyName;
        propertyName.Sprintf (L"Property%i", i);
        SetAndVerifyString (instance, v, propertyName.c_str(), valueForFinalStrings);
        }          
        
    VerifyArrayInfo (instance, v, L"BeginningArray", 0, false);
    VerifyArrayInfo (instance, v, L"VariableArrayFixedElement", 0, false);
    VerifyArrayInfo (instance, v, L"VariableArrayVariableElement", 0, false);
    VerifyArrayInfo (instance, v, L"EndingArray", 0, false);
    
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    // We cannot honor min/maxOccurs attributes of ArrayECProperty, so arrays are always unbounded
    VerifyArrayInfo (instance, v, L"FixedArrayFixedElement", 10, true);
    VerifyArrayInfo (instance, v, L"FixedArrayVariableElement", 12, true);
    VerifyIsNullArrayElements (instance, v, L"FixedArrayFixedElement", 0, 10, true);
    SetAndVerifyIntegerArray (instance, v, L"FixedArrayFixedElement", 172, 10);
    VerifyIsNullArrayElements (instance, v, L"FixedArrayFixedElement", 0, 10, false);
    SetAndVerifyIntegerArray (instance, v, L"FixedArrayFixedElement", 283, 10);    
    
    VerifyIsNullArrayElements (instance, v, L"FixedArrayVariableElement", 0, 12, true);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement", L"BaseString", 12);       
    VerifyIsNullArrayElements (instance, v, L"FixedArrayVariableElement", 0, 12, false);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement", L"LaaaaaaargeString", 10);       
    VerifyStringArray (instance, v, L"FixedArrayVariableElement", L"BaseStringXXXXXXXXXX", 10, 2);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement", L"XString", 12);           
#else
    // Replace #ifdef'ed out section above
    VerifyArrayInfo (instance, v, L"FixedArrayFixedElement", 0, false);
    VerifyArrayInfo (instance, v, L"FixedArrayVariableElement", 0, false);
    instance.AddArrayElements (L"FixedArrayFixedElement", 10);                       // if we supported fixed count arrays, the elements would already have been allocated and this would be unnecessary
    VerifyIsNullArrayElements (instance, v, L"FixedArrayFixedElement", 0, 10, true);
    SetAndVerifyIntegerArray (instance, v, L"FixedArrayFixedElement", 172, 10);
    VerifyIsNullArrayElements (instance, v, L"FixedArrayFixedElement", 0, 10, false);
    SetAndVerifyIntegerArray (instance, v, L"FixedArrayFixedElement", 283, 10);

    instance.AddArrayElements (L"FixedArrayVariableElement", 12);                    // if we supported fixed count arrays, the elements would already have been allocated and this would be unnecessary
    VerifyIsNullArrayElements (instance, v, L"FixedArrayVariableElement", 0, 12, true);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement", L"BaseString", 12);
    VerifyIsNullArrayElements (instance, v, L"FixedArrayVariableElement", 0, 12, false);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement", L"LaaaaaaargeString", 10);
    VerifyStringArray (instance, v, L"FixedArrayVariableElement", L"BaseStringXXXXXXXXXX", 10, 2);
    SetAndVerifyStringArray (instance, v, L"FixedArrayVariableElement", L"XString", 12);
#endif

    ExerciseVariableCountStringArray (instance, v, L"BeginningArray", L"BAValue");
    ExerciseVariableCountIntArray    (instance, v, L"VariableArrayFixedElement", 57);
    ExerciseVariableCountStringArray (instance, v, L"VariableArrayVariableElement", L"Var+Var");
    ExerciseVariableCountStringArray (instance, v, L"EndingArray", L"EArray");        
    
    ECClassCP manufacturerClass = instance.GetClass().GetSchema().GetClassCP (L"Manufacturer");
    ASSERT_TRUE (NULL != manufacturerClass);

#ifdef OLD_WAY    
    ClassLayoutP manufClassLayout = ClassLayout::BuildFromClass (*manufacturerClass, 43, 24);
    StandaloneECEnablerPtr manufEnabler = StandaloneECEnabler::CreateEnabler (*manufacturerClass, *manufClassLayout, true);
#endif
    StandaloneECEnablerPtr manufEnabler =  instance.GetEnablerR().GetEnablerForStructArrayMember (manufacturerClass->GetSchema().GetSchemaKey(), manufacturerClass->GetName().c_str()); 
    ExerciseVariableCountManufacturerArray (instance, *manufEnabler, v, L"ManufacturerArray");
    
    // Make sure that everything still has the final value that we expected
    VerifyString (instance, v, L"S", largeString);
    VerifyInteger (instance, v, L"A", 97);
    VerifyDouble  (instance, v, L"D", doubleValue);
    VerifyInteger (instance, v, L"AA", 12);
    VerifyString  (instance, v, L"B", L"Very Very Happy");
    VerifyString (instance, v, L"Manufacturer.Name", L"Charmed");
    for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
        {
        WString propertyName;
        propertyName.Sprintf (L"Property%i", i);
        VerifyString (instance, v, propertyName.c_str(), valueForFinalStrings);
        }    
    VerifyArrayInfo     (instance, v, L"BeginningArray", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"BeginningArray", 0, 14, false);
    VerifyStringArray   (instance, v, L"BeginningArray", L"BAValue", 0, 14);        
    VerifyArrayInfo     (instance, v, L"VariableArrayVariableElement", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"VariableArrayVariableElement", 0, 14, false);
    VerifyStringArray   (instance, v, L"VariableArrayVariableElement", L"Var+Var", 0, 14);               
    VerifyArrayInfo     (instance, v, L"EndingArray", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"EndingArray", 0, 14, false);
    VerifyStringArray   (instance, v, L"EndingArray", L"EArray", 0, 14);                
    VerifyVariableCountManufacturerArray (instance, v, L"ManufacturerArray");     
    
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    VerifyArrayInfo     (instance, v, L"FixedArrayFixedElement", 10, true);
    VerifyIntegerArray  (instance, v, L"FixedArrayFixedElement", 283, 0, 10);             
    VerifyArrayInfo     (instance, v, L"FixedArrayVariableElement", 12, true);
    VerifyIsNullArrayElements   (instance, v, L"FixedArrayVariableElement", 0, 12, false);
    VerifyStringArray   (instance, v, L"FixedArrayVariableElement", L"XString", 0, 12);           
    VerifyArrayInfo     (instance, v, L"VariableArrayFixedElement", 14, false);
    VerifyIsNullArrayElements   (instance, v, L"VariableArrayFixedElement", 0, 14, false);
    VerifyIntegerArray  (instance, v, L"VariableArrayFixedElement", 57, 0, 14);                   
#endif

    instance.ToString(L"").c_str();
    }

};

#ifdef  MUST_PUBLISH_ECInstanceInteropHelper
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetValuesUsingInteropHelper)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (ecClass);

    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 0, 0);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout, true);

    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    double    doubleVal;
    int       intVal;

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::SetDoubleValue (*instance, L"D", (double)(1.0/3.0)));
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::GetDouble      (*instance, doubleVal, L"D"));
    EXPECT_TRUE ((double)(1.0/3.0) == doubleVal);

    EXPECT_TRUE (ECOBJECTS_STATUS_Success == ECInstanceInteropHelper::AddArrayElements (L"FixedArrayFixedElement", 1));
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ECValueEqualsMethod)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler =  ecClass->GetDefaultStandaloneEnabler();

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

    DateTime timeInput = DateTime::GetCurrentTimeUtc ();
    v1.SetDateTime(timeInput);
    v2.SetDateTime(timeInput);
    EXPECT_TRUE   (v1.Equals (v2));
    DateTime timeInput2 (timeInput.GetInfo ().GetKind (), timeInput.GetYear () + 1, timeInput.GetMonth (), timeInput.GetDay (), timeInput.GetHour (), timeInput.GetMinute (), timeInput.GetSecond (), timeInput.GetHectoNanosecond ());
    v2.SetDateTime(timeInput2);
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

    ECN::StandaloneECInstancePtr testInstance0 = enabler->CreateInstance();
    ECN::StandaloneECInstancePtr testInstance1 = enabler->CreateInstance();
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
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();

    const int expectedPropertyCount = 19;

/** -- Can't test this method via published API -- tested indirectly below
    UInt32 propertyCount = enabler->GetClassLayout().GetPropertyCount();
    EXPECT_EQ (expectedPropertyCount, propertyCount);
**/

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
        L"SomeStrings",
        L"SomeInts",
        L"SomePoint3ds",
        L"SomePoint2ds",
        L"SomeDoubles",
        L"SomeDateTimes",
        L"SomeBooleans",
        L"SomeLongs",
        L"SomeBinaries"
        };

    for (UInt32 i=0; i < expectedPropertyCount; i++)
        {
        WCharCP expectedPropertyName = expectedProperties [i];
        WCharCP propertyName         = NULL;
        UInt32 propertyIndex          = 0;

        EXPECT_TRUE (ECOBJECTS_STATUS_Success == enabler->GetPropertyIndex (propertyIndex, expectedPropertyName));
        EXPECT_TRUE (ECOBJECTS_STATUS_Success == enabler->GetAccessString  (propertyName,  propertyIndex));

        EXPECT_STREQ (expectedPropertyName, propertyName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     printfIndent (UInt32 indentDepth)
    {
    for (UInt32 i = 0; i < indentDepth; i++)
        printf ("  ");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     dumpPropertyValues (ECValuesCollectionR collection, bool isArray, UInt32 indentDepth)
    {
    UInt32  arrayIndex = 0;

    for (ECPropertyValueCR propertyValue : collection)
        {
        ECValueCR v = propertyValue.GetValue();

        printfIndent (indentDepth);
        ECValueAccessorCR   accessor = propertyValue.GetValueAccessor();
        UInt32  accessorDepth = accessor.GetDepth();
        WCharCP accessString = accessor.GetAccessString (accessorDepth- 1);

        if (isArray)
            {
            printf ("Array Member [%d] %S (depth=%d) = %S\n", arrayIndex++, accessString, accessorDepth, v.ToString().c_str());
            }
        else
            {
            printf ("%S (depth=%d)", accessString, accessorDepth);
            if ( ! v.IsStruct())
                printf (" = %S", v.ToString().c_str());

            printf ("\n");
            }

        if (propertyValue.HasChildValues ())
            {
            ECValuesCollectionPtr children = propertyValue.GetChildValues();
            dumpPropertyValues (*children, v.IsArray(), indentDepth+1);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     dumpLoadedPropertyValues (ECValuesCollectionR collection, bool isArray, UInt32 indentDepth, bool printValues, int& count)
    {
    UInt32  arrayIndex = 0;

    for (ECPropertyValueCR propertyValue : collection)
        {
        ECValueCR v = propertyValue.GetValue();
        if (!v.IsLoaded())
            continue;

        count++;
        if (printValues)
            {
            printfIndent (indentDepth);
            ECValueAccessorCR   accessor = propertyValue.GetValueAccessor();
            UInt32  accessorDepth = accessor.GetDepth();
            WCharCP accessString = accessor.GetAccessString (accessorDepth- 1);

            if (isArray)
                {
                printf ("Array Member [%d] %S (depth=%d) = %S\n", arrayIndex++, accessString, accessorDepth, v.ToString().c_str());
                }
            else
                {
                printf ("%S (depth=%d)", accessString, accessorDepth);
                if ( ! v.IsStruct())
                    printf (" = %S", v.ToString().c_str());

                printf ("\n");
                }
            }

        if (propertyValue.HasChildValues ())
            {
            ECValuesCollectionPtr children = propertyValue.GetChildValues();
            dumpLoadedPropertyValues (*children, v.IsArray(), indentDepth+1, printValues, count);
            }
        }
    }

typedef bpair<WString, ECValue>  AccessStringValuePair;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     verifyECValueEnumeration (ECValuesCollectionR collection, bvector <AccessStringValuePair>& expectedValues, UInt32& iValue, bool isDup)
    {
    for (ECPropertyValueCR propertyValue : collection)
        {
        WString   foundAccessString    = propertyValue.GetValueAccessor().GetManagedAccessString(); 
        WString   expectedAccessString = expectedValues[iValue].first;

        EXPECT_STREQ (expectedAccessString.c_str(), foundAccessString.c_str());

        ECValueCR foundValue    = propertyValue.GetValue();
        ECValueCR expectedValue = expectedValues[iValue].second;

        if ( ! isDup || ! foundValue.IsStruct())
            {
            EXPECT_TRUE (foundValue.Equals (expectedValue));
            }
        else
            {
            // If we are enumerating a duplicate, it will have its own struct instances
            // and we expect the struct pointers to be different so we can't call Equals
            EXPECT_TRUE (foundValue.IsNull()   == expectedValue.IsNull());
            EXPECT_TRUE (foundValue.IsStruct() == expectedValue.IsStruct());
            }

        iValue++;;

        if (propertyValue.HasChildValues ())
            {
            ECValuesCollectionPtr children = propertyValue.GetChildValues();
            verifyECValueEnumeration (*children, expectedValues, iValue, isDup);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      5/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_EmptyInstance)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());


    StandaloneECEnablerPtr enabler = schema->GetClassP(L"EmptyClass")->GetDefaultStandaloneEnabler ();

    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Create an empty instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECN::ECValuesCollectionPtr collection = ECN::ECValuesCollection::Create (*instance);

    /*--------------------------------------------------------------------------
        Iterate through its values - shouldn't find any
    --------------------------------------------------------------------------*/
    UInt32 foundValues = 0;
    for (ECPropertyValueCR propertyValue : *collection)
        {
        propertyValue.HasChildValues(); // Use it to avoid warning about unused propertyValue object
        foundValues++;
        }
    EXPECT_TRUE (0 == foundValues);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    foundValues = 0;
    for (ECPropertyValueCR propertyValue : *collection)
        {
        propertyValue.HasChildValues(); // Use it to avoid warning about unused propertyValue object
        foundValues++;
        }
    EXPECT_TRUE (0 == foundValues);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_PrimitiveProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"CadData")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->SetValue(L"Name",         ECValue (L"My Name"));
    instance->SetValue(L"Count",        ECValue (14));
    instance->SetValue(L"Length",       ECValue (142.5));
    instance->SetValue(L"Field_Tested", ECValue (true));

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair (L"Count",          ECValue(14)));
    expectedValues.push_back (AccessStringValuePair (L"StartPoint",     ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"EndPoint",       ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Size",           ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Length",         ECValue (142.5)));
    expectedValues.push_back (AccessStringValuePair (L"Install_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Service_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Field_Tested",   ECValue (true)));
    expectedValues.push_back (AccessStringValuePair (L"Name",           ECValue (L"My Name")));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    UInt32                  iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, CopyInstanceProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.get() != NULL);

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"CadData")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->SetValue(L"Name",         ECValue (L"My Name"));
    instance->SetValue(L"Count",        ECValue (14));
    instance->SetValue(L"Length",       ECValue (142.5));
    instance->SetValue(L"Field_Tested", ECValue (true));

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair (L"Count",          ECValue(14)));
    expectedValues.push_back (AccessStringValuePair (L"StartPoint",     ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"EndPoint",       ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Size",           ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Length",         ECValue (142.5)));
    expectedValues.push_back (AccessStringValuePair (L"Install_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Service_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Field_Tested",   ECValue (true)));
    expectedValues.push_back (AccessStringValuePair (L"Name",           ECValue (L"My Name")));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    UInt32                  iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr duplicateInstance = enabler->CreateInstance();

    ECObjectsStatus copyStatus = duplicateInstance->CopyValues (*instance);
    EXPECT_TRUE (ECOBJECTS_STATUS_Success == copyStatus);

    collection = ECValuesCollection::Create (*duplicateInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, MergeInstanceProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.get() != NULL);

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"CadData")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the base instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr mergeToInstance = enabler->CreateInstance();

    mergeToInstance->SetValue(L"Name",         ECValue (L"base"));
    mergeToInstance->SetValue(L"Length",       ECValue (142.5));
    mergeToInstance->SetValue(L"Field_Tested", ECValue (true));

    /*--------------------------------------------------------------------------
        Build the instance with data to merge
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr mergeFromInstance = enabler->CreateInstance();

    DPoint2d   tstSize = {10.5, 22.3};

    ECValue nullBool (ECN::PRIMITIVETYPE_Boolean);

    mergeFromInstance->SetValue(L"Name",         ECValue (L"merge"));
    mergeFromInstance->SetValue(L"Count",        ECValue (14));
    mergeFromInstance->SetValue(L"Field_Tested", nullBool);
    mergeFromInstance->SetValue (L"Size",        ECValue (tstSize));

    MemoryECInstanceBase* mbInstance = mergeToInstance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*mergeFromInstance);

    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair (L"Count",          ECValue(14)));
    expectedValues.push_back (AccessStringValuePair (L"StartPoint",     ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"EndPoint",       ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Size",           ECValue (tstSize)));
    expectedValues.push_back (AccessStringValuePair (L"Length",         ECValue (142.5)));
    expectedValues.push_back (AccessStringValuePair (L"Install_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Service_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"Field_Tested",   nullBool));
    expectedValues.push_back (AccessStringValuePair (L"Name",           ECValue (L"merge")));

    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*mergeToInstance);
//    dumpPropertyValues (*collection, false, 0);

    UInt32                  iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_PrimitiveArray)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"AllPrimitives")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->SetValue(L"AString",  ECValue (L"Happy String"));
    instance->SetValue(L"AnInt",    ECValue (6));

    instance->AddArrayElements(L"SomeStrings", 5);

    instance->SetValue(L"SomeStrings", ECValue (L"ArrayMember 1"), 0);
    instance->SetValue(L"SomeStrings", ECValue (L"ArrayMember 2"), 2);
    instance->SetValue(L"SomeStrings", ECValue (L"ArrayMember 3"), 4);

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    ECValue arrayValue;
    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair (L"AnInt", ECValue (6)));
    expectedValues.push_back (AccessStringValuePair (L"APoint3d", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"APoint2d", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"ADouble", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"ADateTime", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"ABoolean", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"ALong", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"AString", ECValue (L"Happy String")));
    expectedValues.push_back (AccessStringValuePair (L"ABinary", ECValue ()));

    arrayValue.Clear();
    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_String, 5, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeStrings", arrayValue));

    expectedValues.push_back (AccessStringValuePair (L"SomeStrings[0]", ECValue (L"ArrayMember 1")));
    expectedValues.push_back (AccessStringValuePair (L"SomeStrings[1]", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"SomeStrings[2]", ECValue (L"ArrayMember 2")));
    expectedValues.push_back (AccessStringValuePair (L"SomeStrings[3]", ECValue ()));
    expectedValues.push_back (AccessStringValuePair (L"SomeStrings[4]", ECValue (L"ArrayMember 3")));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Integer, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeInts",     arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Point3D, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomePoint3ds", arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Point2D, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomePoint2ds", arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Double, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeDoubles",  arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_DateTime, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeDateTimes",arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Boolean, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeBooleans", arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Long, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeLongs",    arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Binary, 0, false);
    expectedValues.push_back (AccessStringValuePair (L"SomeBinaries", arrayValue));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    UInt32                  iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static WString  buildAccessString
(
WCharCP  accessPrefix,
WCharCP  propertyString
)
    {
    WString accessString;

    if (accessPrefix && 0 < wcslen (accessPrefix))
        {
        accessString.append (accessPrefix);
        accessString.append (L".");
        }

    accessString.append (propertyString);
    
    return accessString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setValue
(
WCharCP  accessPrefix,
WCharCP  propertyString,
ECValueCR       ecValue,
IECInstanceR    instance
)
    {
    WString accessString = buildAccessString (accessPrefix, propertyString);

    instance.SetValue (accessString.c_str(), ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setPartialContactInfo
(
bool            skipPhoneNumberData,
WCharCP         prefix,
int             areaCode,
int             phoneNumber,
WCharCP houseNumber,
WCharCP street,
WCharCP town,
WCharCP state,
int             zip,
WCharCP email,
IECInstanceR    instance
)
    {
    if (!skipPhoneNumberData)
        {
        setValue (prefix, L"PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
        setValue (prefix, L"PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
        setValue (prefix, L"PhoneNumber.Number",    ECValue (phoneNumber),  instance);
        setValue (prefix, L"Address.HouseNumber",   ECValue (houseNumber),  instance);
        }

    setValue (prefix, L"Address.Street",        ECValue (street),       instance);
    setValue (prefix, L"Address.Town",          ECValue (town),         instance);
    setValue (prefix, L"Address.State",         ECValue (state),        instance);
    setValue (prefix, L"Address.Zip",           ECValue (zip),          instance);
    setValue (prefix, L"Email",                 ECValue (email),        instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setContactInfo
(
WCharCP prefix,
int             areaCode,
int             phoneNumber,
WCharCP houseNumber,
WCharCP street,
WCharCP town,
WCharCP state,
int             zip,
WCharCP email,
IECInstanceR    instance
)
    {
    setValue (prefix, L"PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
    setValue (prefix, L"PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
    setValue (prefix, L"PhoneNumber.Number",    ECValue (phoneNumber),  instance);
    setValue (prefix, L"Address.HouseNumber",   ECValue (houseNumber),  instance);
    setValue (prefix, L"Address.Street",        ECValue (street),       instance);
    setValue (prefix, L"Address.Town",          ECValue (town),         instance);
    setValue (prefix, L"Address.State",         ECValue (state),        instance);
    setValue (prefix, L"Address.Zip",           ECValue (zip),          instance);
    setValue (prefix, L"Email",                 ECValue (email),        instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addValue
(
WCharCP  accessPrefix,
WCharCP  propertyString,
ECValueCR       ecValue,
bvector <AccessStringValuePair>& expectedValues
)
    {
    WString accessString = buildAccessString (accessPrefix, propertyString);

    expectedValues.push_back (AccessStringValuePair (accessString.c_str(), ecValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addExpectedContactInfo
(
WCharCP prefix,
int             areaCode,
int             phoneNumber,
WCharCP houseNumber,
WCharCP street,
WCharCP town,
WCharCP state,
int             zip,
WCharCP email,
bvector <AccessStringValuePair>& expectedValues
)
    {
    if (NULL != prefix  && 0 < wcslen (prefix))
        expectedValues.push_back (AccessStringValuePair (prefix, ECValue (VALUEKIND_Struct)));

    addValue (prefix, L"PhoneNumber",           ECValue (VALUEKIND_Struct), expectedValues);
    addValue (prefix, L"PhoneNumber.AreaCode",  ECValue (areaCode),         expectedValues);
    addValue (prefix, L"PhoneNumber.Number",    ECValue (phoneNumber),      expectedValues);
    addValue (prefix, L"Address",               ECValue (VALUEKIND_Struct), expectedValues);
    addValue (prefix, L"Address.Zip",           ECValue (zip),              expectedValues);

    addValue (prefix, L"Address.HouseNumber",   ECValue (houseNumber),     expectedValues);
    addValue (prefix, L"Address.Street",        ECValue (street),          expectedValues);
    addValue (prefix, L"Address.Town",          ECValue (town),            expectedValues);
    addValue (prefix, L"Address.State",         ECValue (state),           expectedValues);
    addValue (prefix, L"Email",                 ECValue (email),           expectedValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_EmbeddedStructs)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"ContactInfo")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    setContactInfo (L"", 610, 1234567, L"123-4", L"Main Street", L"Exton", L"PA", 12345, L"nobody@nowhere.com", *instance);

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    addExpectedContactInfo (L"", 610, 1234567, L"123-4", L"Main Street", L"Exton", L"PA", 12345, L"nobody@nowhere.com", expectedValues);

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    UInt32                  iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_StructArray)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"EmployeeDirectory")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    instance->AddArrayElements(L"Employees", 2);

    StandaloneECEnablerPtr arrayMemberEnabler = schema->GetClassP(L"Employee")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    ECN::StandaloneECInstancePtr arrayMemberInstance1 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance1->SetValue(L"Name", ECValue (L"John Smith"));

    setContactInfo (L"Home",   610, 7654321, L"175",   L"Oak Lane",    L"Wayne", L"PA", 12348, L"jsmith@home.com", *arrayMemberInstance1);
    setContactInfo (L"Work",   610, 1234567, L"123-4", L"Main Street", L"Exton", L"PA", 12345, L"jsmith@work.com", *arrayMemberInstance1);
    v.SetStruct(arrayMemberInstance1.get());
    instance->SetValue (L"Employees", v, 0);

    ECN::StandaloneECInstancePtr arrayMemberInstance2 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance2->SetValue(L"Name", ECValue (L"Jane Doe"));
    setContactInfo (L"Home",   555, 1122334, L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, L"prez@gmail.com", *arrayMemberInstance2);
    setContactInfo (L"Work",   555, 1000000, L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, L"president@whitehouse.gov", *arrayMemberInstance2);
    v.SetStruct(arrayMemberInstance2.get());
    instance->SetValue (L"Employees", v, 1);

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    ECValue arrayValue;
    arrayValue.SetStructArrayInfo (2, false);
    expectedValues.push_back (AccessStringValuePair (L"Employees", arrayValue));

    ECValue structValue;
    structValue.SetStruct (arrayMemberInstance1.get());
    expectedValues.push_back (AccessStringValuePair (L"Employees[0]", structValue));

    addExpectedContactInfo (L"Employees[0].Home", 610, 7654321, L"175",   L"Oak Lane",    L"Wayne", L"PA", 12348, L"jsmith@home.com", expectedValues);
    addExpectedContactInfo (L"Employees[0].Work", 610, 1234567, L"123-4", L"Main Street", L"Exton", L"PA", 12345, L"jsmith@work.com", expectedValues);

    expectedValues.push_back (AccessStringValuePair (L"Employees[0].Name", ECValue (L"John Smith")));

    structValue.SetStruct (arrayMemberInstance2.get());
    expectedValues.push_back (AccessStringValuePair (L"Employees[1]", structValue));

    addExpectedContactInfo (L"Employees[1].Home", 555, 1122334, L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, L"prez@gmail.com", expectedValues);
    addExpectedContactInfo (L"Employees[1].Work", 555, 1000000, L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, L"president@whitehouse.gov", expectedValues);

    expectedValues.push_back (AccessStringValuePair (L"Employees[1].Name", ECValue (L"Jane Doe")));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    UInt32                  iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, MergeStructArray)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"EmployeeDirectory")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    instance->AddArrayElements(L"Employees", 2);

    StandaloneECEnablerPtr arrayMemberEnabler = schema->GetClassP(L"Employee")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    ECN::StandaloneECInstancePtr arrayMemberInstance1 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance1->SetValue(L"Name", ECValue (L"John Smith"));

    setContactInfo (L"Home",   610, 7654321, L"175",   L"Oak Lane",    L"Wayne", L"PA", 12348, L"jsmith@home.com", *arrayMemberInstance1);
    setPartialContactInfo (true, L"Work",   610, 1234567, L"123-4", L"Main Street", L"Exton", L"PA", 12345, L"jsmith@work.com", *arrayMemberInstance1);
    v.SetStruct(arrayMemberInstance1.get());
    instance->SetValue (L"Employees", v, 0);

    ECN::StandaloneECInstancePtr arrayMemberInstance2 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance2->SetValue(L"Name", ECValue (L"Jane Doe"));
    setPartialContactInfo (false, L"Home",   555, 1122334, L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, L"prez@gmail.com", *arrayMemberInstance2);
    setPartialContactInfo (true, L"Work",   555, 1000000, L"1600", L"Pennsylvania Ave", L"Washington", L"DC", 10001, L"president@whitehouse.gov", *arrayMemberInstance2);
    v.SetStruct(arrayMemberInstance2.get());
    instance->SetValue (L"Employees", v, 1);

    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    int originalCount = 0;
    int count = 0;

    dumpLoadedPropertyValues  (*collection, false, 0, false, originalCount);

    ECN::StandaloneECInstancePtr toInstance = enabler->CreateInstance();

    MemoryECInstanceBase* mbInstance = toInstance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*instance);

    collection = ECValuesCollection::Create (*toInstance);

    dumpLoadedPropertyValues  (*collection, false, 0, false, count);
    ASSERT_TRUE (count==originalCount);
    ASSERT_TRUE (count==41);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, MergeStruct)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP(L"Employee")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    ECN::StandaloneECInstancePtr employeeInstance = enabler->CreateInstance();
    employeeInstance->SetValue(L"Name", ECValue (L"John Smith"));

    setPartialContactInfo (false, L"Home",   610, 7654321, L"175",   L"Oak Lane",    L"Wayne", L"PA", 12348, L"jsmith@home.com", *employeeInstance);
    setPartialContactInfo (true, L"Work",   610, 1234567, L"123-4", L"Main Street", L"Exton", L"PA", 12345, L"jsmith@work.com", *employeeInstance);

    int originalCount = 0;
    int count = 0;

    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*employeeInstance);
    dumpLoadedPropertyValues  (*collection, false, 0, false, originalCount);

    ECN::StandaloneECInstancePtr toInstance = enabler->CreateInstance();

    MemoryECInstanceBase* mbInstance = toInstance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*employeeInstance);

    collection = ECValuesCollection::Create (*toInstance);
    dumpLoadedPropertyValues  (*collection, false, 0, false, count);
    ASSERT_TRUE (count==originalCount);
    ASSERT_TRUE (count==19);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, SimpleMergeTwoInstances)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP primitiveClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (NULL != primitiveClass);

    StandaloneECEnablerPtr enabler = primitiveClass->GetDefaultStandaloneEnabler();
    
    ECN::StandaloneECInstancePtr sourceInstance0 = enabler->CreateInstance();
    ECN::StandaloneECInstancePtr sourceInstance1 = enabler->CreateInstance();
    ECN::StandaloneECInstancePtr targetInstance  = enabler->CreateInstance();

    ECValue v;
    v.SetDouble(1.0/3.0);
    sourceInstance0->SetValue(L"ADouble", v);
    v.SetString(L"Weaker source instance");
    sourceInstance0->SetValue(L"AString", v);
    v.SetInteger(234);
    sourceInstance0->SetValue(L"AnInt", v);
    v.SetInteger(50);
    sourceInstance0->AddArrayElements(L"SomeInts", 4);
    sourceInstance0->SetValue(L"SomeInts", v, 0);
    v.SetInteger(60);
    sourceInstance0->SetValue(L"SomeInts", v, 1);
    v.SetInteger(70);
    sourceInstance0->SetValue(L"SomeInts", v, 2);
    v.SetInteger(80);
    sourceInstance0->SetValue(L"SomeInts", v, 3);

    v.SetDouble(10.0/3.0);
    sourceInstance1->SetValue(L"ADouble", v);
    v.SetLong((Int64)2345978);
    sourceInstance1->SetValue(L"ALong", v);
    v.SetString(L"Dominant source instance");
    sourceInstance1->SetValue(L"AString", v);
    v.SetInteger(99999999);
    sourceInstance1->AddArrayElements(L"SomeInts", 4);
    sourceInstance1->SetValue(L"SomeInts", v, 1);

    /*
    Merging two instances into a third instance:
    In this example, values from sourceInstance 1 will take precedence over 
    values in sourceInstance0 in the even that neither are null.
    Note that in Options::Create (), the second flag is set to true: in this
    case, it is wise to include accessors that have null values.
    */
    ECValuesCollectionPtr collection = ECValuesCollection::Create (*sourceInstance1);

    for (ECPropertyValueCR propertyValue : *collection)
        {
        ECValue             localValue;
        ECValueCP           ecValue  = &propertyValue.GetValue();

        if ( ! ecValue->IsPrimitive())
            continue;

        ECValueAccessorCR   accessor = propertyValue.GetValueAccessor();

        // If the value from instance1 is NULL, try to get it from instance0
        if (ecValue->IsNull())
            {
            sourceInstance0->GetValueUsingAccessor (localValue, accessor);
            ecValue = &localValue;
            }
            
        //set the value to target instance
        if(!ecValue->IsNull())
            targetInstance->SetValueUsingAccessor (accessor, *ecValue);
        }

    int valuesCounted = 0;
    ECValuesCollectionPtr targetCollection = ECValuesCollection::Create (*targetInstance);

    for (ECPropertyValueCR propertyValue : *targetCollection)
        {
        if ( ! propertyValue.GetValue().IsPrimitive())
            continue;

        valuesCounted++;
        //wprintf(L"%ls: %ls\n", propertyValue.GetValueAccessor().GetManagedAccessString(), propertyValue.GetValue().ToString());
        }

    //Verify that the merge succeeded
    EXPECT_EQ (9, valuesCounted);
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
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    WString instanceId = instance->GetInstanceId();
    instance->ToString(L"").c_str();
    ExerciseInstance (*instance, L"Test");

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, InstantiateInstanceWithNoProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"EmptyClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    WString instanceId = instance->GetInstanceId();

    instance->ToString(L"").c_str();

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, DirectSetStandaloneInstance)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"CadData");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    DPoint2d   inSize = {10.5, 22.3};
    DPoint3d   inPoint1 = {10.10, 11.11, 12.12};
    DPoint3d   inPoint2 ={200.100, 210.110, 220.120};
    DateTime   inTimeUtc = DateTime::GetCurrentTimeUtc ();
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
    instance->SetValue (L"Service_Date", ECValue (inTimeUtc));

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
    DPoint2d    point2d = ecValue.GetPoint2D ();
    EXPECT_TRUE (SUCCESS == memcmp (&inSize, &point2d, sizeof(DPoint2d)));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"StartPoint"));
    DPoint3d    point3d = ecValue.GetPoint3D ();
    EXPECT_TRUE (SUCCESS == memcmp (&inPoint1, &point3d, sizeof(DPoint3d)));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"EndPoint"));
    point3d = ecValue.GetPoint3D ();
    EXPECT_TRUE (SUCCESS == memcmp (&inPoint2, &point3d, sizeof(DPoint3d)));
    //in absence of the DateTimeInfo custom attribute on Service_Date the retrieved
    //date time will always be of kind Unspecified, i.e. the original kind (here Utc)
    //gets lost
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Service_Date"));
    DateTime  sysTime = ecValue.GetDateTime ();
    EXPECT_TRUE (inTimeUtc.Compare (sysTime, true));
    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"Install_Date"));
    EXPECT_TRUE (ecValue.GetDateTimeTicks() == inTicks);

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetSetValuesByIndex)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    WCharCP accessString = L"Property34";

    //UInt32          intValue = 12345;
    WCharCP stringValue = L"Xyz";

    //instance->SetValue  (accessString, ECValue (intValue));
    instance->SetValue  (accessString, ECValue (stringValue));

    ECValue value;
    UInt32  propertyIndex = 0;

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
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    {
    DISABLE_ASSERTS

#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    // verify we can not change the size of fixed arrays        
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayFixedElement", 0, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayFixedElement", 10, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECOBJECTS_STATUS_Success != instance->AddArrayElements    (L"FixedArrayFixedElement", 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayVariableElement", 0, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"FixedArrayVariableElement", 12, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECOBJECTS_STATUS_Success != instance->AddArrayElements    (L"FixedArrayVariableElement", 1));
#endif

    // verify constraints of array insertion are enforced
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"NonExistArray", 0, 1));
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"BeginningArray", 2, 1)); // insert index is invalid    
    ASSERT_TRUE (ECOBJECTS_STATUS_Success != instance->InsertArrayElements (L"BeginningArray", 0, 0)); // insert count is invalid    
    }
    
    ECValue v;
    VerifyOutOfBoundsError (*instance, v, L"BeginningArray", 0);
    VerifyOutOfBoundsError (*instance, v, L"FixedArrayFixedElement", 10);
    VerifyOutOfBoundsError (*instance, v, L"VariableArrayFixedElement", 0);
    VerifyOutOfBoundsError (*instance, v, L"FixedArrayVariableElement", 12);
    VerifyOutOfBoundsError (*instance, v, L"VariableArrayVariableElement", 0);
    VerifyOutOfBoundsError (*instance, v, L"EndingArray", 0);                     
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
    
    ECValue nullInt (ECN::PRIMITIVETYPE_Integer);
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
    //WCharCP wcnull = snull.GetString();
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
    EXPECT_TRUE (0 == point3Str.compare (L"10,100,1000"));

    //DPoint2d
    DPoint2d inPoint2 = {10.0, 100.0};
    ECValue pntVal2 (inPoint2);
    EXPECT_TRUE (pntVal2.IsPoint2D());
    DPoint2d outPoint2 = pntVal2.GetPoint2D ();
    EXPECT_TRUE (0 == memcmp(&inPoint2, &outPoint2, sizeof(outPoint2)));
    WString point2Str = pntVal2.ToString();
    EXPECT_TRUE (0 == point2Str.compare (L"10,100"));

    // DateTime
    DateTime nowUtc = DateTime::GetCurrentTimeUtc ();
    ECValue dateValue (nowUtc);
    EXPECT_TRUE (dateValue.IsDateTime());
    DateTime nowToo = dateValue.GetDateTime ();
    EXPECT_TRUE (nowToo.Compare (nowUtc, true));

    ECValue fixedDate;
    fixedDate.SetDateTimeTicks (634027121070910000);
    WString dateStr = fixedDate.ToString();
    EXPECT_TRUE (0 == dateStr.compare (L"2010-02-25T16:28:27.091")) << L"Expected date: " << fixedDate.GetDateTime ().ToString ().c_str ();
    };
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestSetGetNull)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
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
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestPropertyReadOnly)
    {
    //L"    <ECClass typeName=\"Car\" isStruct=\"True\" isDomainClass=\"True\">"
    //L"        <ECProperty       propertyName=\"Name\"       typeName=\"string\"/>"
    //L"        <ECProperty       propertyName=\"Wheels\"     typeName=\"int\"  readOnly=\"True\"/>"
    //L"    </ECClass>"

    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"Car");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    WCharCP nameAccessString = L"Name";
    WCharCP wheelsAccessString = L"Wheels";
    UInt32  namePropertyIndex = 9999;
    UInt32  wheelsPropertyIndex = 9998;
    EXPECT_TRUE (SUCCESS == enabler->GetPropertyIndex (namePropertyIndex, nameAccessString));
    EXPECT_TRUE (SUCCESS == enabler->GetPropertyIndex (wheelsPropertyIndex, wheelsAccessString));

    EXPECT_FALSE (instance->IsPropertyReadOnly (nameAccessString));
    EXPECT_FALSE (instance->IsPropertyReadOnly (namePropertyIndex));

    EXPECT_TRUE  (instance->IsPropertyReadOnly (wheelsAccessString));
    EXPECT_TRUE  (instance->IsPropertyReadOnly (wheelsPropertyIndex));  

    ECValue v;
    v.SetInteger(610);
    EXPECT_TRUE (SUCCESS == instance->SetValue (wheelsAccessString, v));  // should work since original value is NULL
    v.SetInteger(512);
    EXPECT_TRUE (ECOBJECTS_STATUS_UnableToSetReadOnlyProperty == instance->SetValue (wheelsAccessString, v));  // should fail since read only and value is not NULL

    // make sure we can copy an instance contains read only properties
    StandaloneECInstancePtr  copyInstance =  StandaloneECInstance::Duplicate (*instance);
    EXPECT_TRUE (SUCCESS == instance->GetValue (v, wheelsAccessString));
    EXPECT_TRUE (610 == v.GetInteger());

    // make sure we can deserialize and instance from XML that contains read only properties
    WString ecInstanceXml;
    instance->WriteToXmlString (ecInstanceXml, true, false);
    ECN::IECInstancePtr deserializedInstance = NULL;
    ECN::ECInstanceReadContextPtr instanceContext = ECN::ECInstanceReadContext::CreateContext (*schema);
    EXPECT_TRUE (INSTANCE_READ_STATUS_Success == IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext));
    EXPECT_TRUE (SUCCESS == deserializedInstance->GetValue (v, wheelsAccessString));
    EXPECT_TRUE (610 == v.GetInteger());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestBinarySetGet)
    {
    const static bool HOLD_AS_DUPLICATE = true;
    const byte binaryValue0[4] = {0x00, 0x01, 0x02, 0x03};
    const byte binaryValue1[2] = {0x99, 0x88};

    EXPECT_EQ (sizeof(binaryValue0), 4);
    EXPECT_EQ (sizeof(binaryValue1), 2);

    ECValue v0In;
    ECValue v0Out;
    ECValue v1In;
    ECValue v1Out;

    v0In.SetBinary(binaryValue0, sizeof(binaryValue0), HOLD_AS_DUPLICATE);
    v1In.SetBinary(binaryValue1, sizeof(binaryValue1), HOLD_AS_DUPLICATE);

    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"ABinary", v0In));
    EXPECT_TRUE (SUCCESS == instance->GetValue (v0Out, L"ABinary"));
    EXPECT_TRUE (v0In.Equals (v0Out));

    // now set it to a smaller size
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"ABinary", v1In));
    EXPECT_TRUE (SUCCESS == instance->GetValue (v1Out, L"ABinary"));
    EXPECT_TRUE (v1In.Equals (v1Out));
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void validateArrayCount  (ECN::StandaloneECInstanceCR instance, WCharCP propertyName, UInt32 expectedCount)
    {
    ECValue varray;
    EXPECT_TRUE (SUCCESS == instance.GetValue (varray, propertyName));
    UInt32 count = varray.GetArrayInfo().GetCount();
    EXPECT_TRUE (count == expectedCount);

    ECValue ventry;

    for (UInt32 i=0; i<count; i++)
        {
        EXPECT_TRUE (SUCCESS == instance.GetValue (ventry, propertyName, i));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestRemovingArrayEntries)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"ArrayTest");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayFixedElement",  ECValue ((int)1), 1));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayFixedElement",  ECValue ((int)3), 3));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayFixedElement",  ECValue ((int)5), 5));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayFixedElement",  ECValue ((int)7), 7));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayFixedElement",  ECValue ((int)9), 9));

    {
    DISABLE_ASSERTS    
    EXPECT_TRUE (ECOBJECTS_STATUS_Success != instance->RemoveArrayElement(L"FixedArrayFixedElement", 2));
    }

    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayVariableElement", ECValue (L"ArrayMember 1"), 1));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayVariableElement", ECValue (L"ArrayMember 3"), 3));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayVariableElement", ECValue (L"ArrayMember 5"), 5));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayVariableElement", ECValue (L"ArrayMember 7"), 7));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayVariableElement", ECValue (L"ArrayMember 9"), 9));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"FixedArrayVariableElement", ECValue (L"ArrayMember 11"), 11));

    {
    DISABLE_ASSERTS    
    EXPECT_TRUE (ECOBJECTS_STATUS_Success != instance->RemoveArrayElement(L"FixedArrayVariableElement", 2));
    }
#endif

    instance->AddArrayElements(L"SomeStrings", 5);

    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeStrings",  ECValue (L"ArrayMember 0"), 0));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeStrings",  ECValue (L"ArrayMember 1"), 1));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeStrings",  ECValue (L"ArrayMember 2"), 2));
    // leave index 3 null
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeStrings",  ECValue (L"ArrayMember 4"), 4));

    validateArrayCount (*instance, L"SomeStrings", 5); 

    instance->AddArrayElements(L"SomeInts", 6);

    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeInts",  ECValue ((int)0), 0));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeInts",  ECValue ((int)1), 1));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeInts",  ECValue ((int)2), 2));
    // leave index 3 null
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeInts",  ECValue ((int)4), 4));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SomeInts",  ECValue ((int)5), 5));

    validateArrayCount (*instance, L"SomeInts", 6); 

    // define struct array
    StandaloneECEnablerPtr manufacturerEnabler = instance->GetEnablerR().GetEnablerForStructArrayMember (schema->GetSchemaKey(), L"Manufacturer"); 
    EXPECT_TRUE (manufacturerEnabler.IsValid());

    ECValue v;
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance->AddArrayElements (L"ManufacturerArray", 4));
    VerifyArrayInfo (*instance, v, L"ManufacturerArray", 4, false);
    VerifyIsNullArrayElements (*instance, v, L"ManufacturerArray", 0, 4, true);

    IECInstancePtr manufInst = manufacturerEnabler->CreateInstance().get();    

    SetAndVerifyString (*manufInst, v, L"Name", L"Nissan");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 3475);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"ManufacturerArray", v, 0));

    manufInst = manufacturerEnabler->CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Kia");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1791);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"ManufacturerArray", v, 1));    

    manufInst = manufacturerEnabler->CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Honda");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1592);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"ManufacturerArray", v, 2));    

    manufInst = manufacturerEnabler->CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, L"Name", L"Chevy");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 19341);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"ManufacturerArray", v, 3));    

    VerifyIsNullArrayElements (*instance, v, L"ManufacturerArray", 0, 4, false);    

    // remove from start of array
    instance->RemoveArrayElement(L"SomeStrings", 0);
    validateArrayCount (*instance, L"SomeStrings", 4); 

    // remove from middle of array
    instance->RemoveArrayElement(L"SomeStrings", 2);
    validateArrayCount (*instance, L"SomeStrings", 3); 

    // remove from end of array
    instance->RemoveArrayElement(L"SomeInts", 2);
    validateArrayCount (*instance, L"SomeInts", 5);

    // remove struct array element
    instance->RemoveArrayElement(L"ManufacturerArray", 2);
    validateArrayCount (*instance, L"ManufacturerArray", 3);
    }

TEST_F (MemoryLayoutTests, IterateCompleClass)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP (L"ComplexClass");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECValue b(true);
    ECValue s1(L"719372644");
    ECValue s2(L"asasdasd");
    ECValue s3(L"1338164264");
    ECValue s4(L"string val");
    ECValue s5(L"asdasdas");
    ECValue s6(L"392010267");
    ECValue i1((int)1683483880);
    ECValue i2((int)1367822242);
    ECValue i3((int)32323);
    ECValue d1(0.71266461290077521);

    EXPECT_TRUE (SUCCESS == instance->SetValue (L"StringProperty", s4));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"IntProperty", i2));

    StandaloneECEnablerPtr structArrayEnabler = schema->GetClassP(L"StructClass")->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr structInstance = structArrayEnabler->CreateInstance();

    EXPECT_TRUE (SUCCESS == instance->SetValue (L"BooleanProperty", b));
    EXPECT_TRUE (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == instance->ChangeValue (L"BooleanProperty", b));
#if !FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (SUCCESS == instance->AddArrayElements (L"SimpleArrayProperty", 1));
#endif
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"SimpleArrayProperty", s1, 0));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"StructProperty.StringProperty", s2));
    EXPECT_TRUE (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == instance->ChangeValue (L"StructProperty.StringProperty", s2));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"StructProperty.IntProperty", i1));
    EXPECT_TRUE (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == instance->ChangeValue (L"StructProperty.IntProperty", i1));
#if !FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (SUCCESS == instance->AddArrayElements (L"StructProperty.ArrayProperty", 1));
#endif
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"StructProperty.ArrayProperty", s3, 0));
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"DoubleProperty", d1));
    EXPECT_TRUE (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == instance->ChangeValue (L"DoubleProperty", d1));

    EXPECT_TRUE (SUCCESS == structInstance->SetValue (L"StringProperty", s5));
    EXPECT_TRUE (SUCCESS == structInstance->SetValue (L"IntProperty", i3));
#if !FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (SUCCESS == structInstance->AddArrayElements (L"ArrayProperty", 1));
#endif
    EXPECT_TRUE (SUCCESS == structInstance->SetValue (L"ArrayProperty", s6, 0));

#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    // This is a fixed-size struct array so we don't have to insert members
#else
    EXPECT_TRUE (SUCCESS == instance->AddArrayElements (L"StructArrayProperty", 1));
#endif
    ECValue structVal;
    structVal.SetStruct (structInstance.get());
    EXPECT_TRUE (SUCCESS == instance->SetValue (L"StructArrayProperty", structVal, 0));

    // ensure we can walk the properties
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    //dumpPropertyValues (*collection, false, 0);
    }


void SetStringToSpecifiedNumberOfCharacters (IECInstanceR instance, int nChars)
    {
    WString string;
    for (int i = 0; i < nChars; i++)
        {
        int digit = i % 10;
        WString digitAsString;
        digitAsString.Sprintf (L"%d", digit);
        string.append (digitAsString);
        }
        
    ECValue v(string.c_str());
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

    ECSchemaPtr         schema      = CreateProfilingSchema(nStrings);
    ECClassP           ecClass     = schema->GetClassP (L"Pidget");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    //UInt32 slack = 0;
    double elapsedSeconds = 0.0;
    StopWatch timer (L"Time setting of values in a new StandaloneECInstance", true);
    for (int i = 0; i < nInstances; i++)
        {
        timer.Start();
        SetValuesForProfiling (*instance);
        timer.Stop();
        
        elapsedSeconds += timer.GetElapsedSeconds();
        instance->GetAsMemoryECInstanceP()->ClearValues();
        }
    
    //wprintf (L"  %d StandaloneECInstances with %d string properties initialized in %.4f seconds.\n", nInstances, nStrings, elapsedSeconds);
    }
    

TEST_F (MemoryLayoutTests, TestValueAccessor)
    {
    ECValueAccessor m_accessor;
    }

END_BENTLEY_ECN_TEST_NAMESPACE