/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ECTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include "ConverterAppTestsBaseFixture.h"
#include "ImportConfigEditor.h"
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>
#include <VersionedDgnV8Api/DgnPlatform/ECXAProvider.h>
#include <VersionedDgnV8Api/DgnPlatform/ECXAInstance.h>

BEGIN_UNNAMED_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct ECConversionTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    void SetUp();
    void TearDown();

    protected:
        void VerifyElement(DgnV8Api::ElementId&, Utf8CP className, bool isPrimaryInstance);
        void VerifyElement(DgnDbR db, DgnV8Api::ElementId&, Utf8CP className, bool isPrimaryInstance);
        void ECConversionTests::LoadECXAttributes(ECObjectsV8::StandaloneECInstanceR instance , ECObjectsV8::ECClassCP elementClass);
        void ECConversionTests::VerifyECXAttributes(BentleyApi::Dgn::DgnElementCPtr bimel);
    };

END_UNNAMED_NAMESPACE

static Utf8CP s_testSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"String\" typeName=\"string\" />"
"        <ECArrayProperty propertyName=\"StringArray\" typeName=\"string\"  minOccurs=\"0\" maxOccurs=\"10\" />"
"        <ECArrayProperty propertyName=\"myStringArray\" typeName=\"string\" />"
"        <ECProperty propertyName = \"myInt\" typeName=\"int\" />"
"        <ECArrayProperty propertyName=\"myIntArray\" typeName=\"int\" />"
"        <ECProperty propertyName=\"my3dPoint\"     typeName=\"point3d\" />"
"        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
"        <ECProperty propertyName=\"myDate\" typeName=\"dateTime\"  />"
"        <ECArrayProperty propertyName=\"myDateArray\" typeName=\"dateTime\" />"
"        <ECProperty propertyName=\"myLong\" typeName=\"long\" />"
"        <ECArrayProperty propertyName=\"myLongArray\" typeName=\"long\" />"
"        <ECProperty propertyName=\"myDouble\" typeName=\"double\" />"
"        <ECArrayProperty propertyName=\"myDoubleArray\" typeName=\"double\" />"
"        <ECProperty propertyName=\"myBool\" typeName=\"boolean\"  />"
"        <ECArrayProperty propertyName=\"myBoolArray\" typeName=\"boolean\" />"
"        <ECProperty propertyName=\"my2dPoint\" typeName=\"point2d\" />"
"        <ECArrayProperty propertyName=\"my2dPointArray\" typeName=\"point2d\" />"
"        <ECStructProperty propertyName=\"myStruct\" typeName=\"Struct1\" />"
"        <ECArrayProperty propertyName=\"myStructArray\" typeName=\"Struct1\"/>"
"        <ECStructProperty propertyName=\"myComplicated\" typeName=\"Complicated\" />"
"    </ECClass>"
"    <ECClass typeName=\"Struct1\" isStruct=\"True\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
"        <ECProperty propertyName=\"No\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"Complicated\" isStruct=\"True\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"ExtName\" typeName=\"string\" />"
"        <ECStructProperty propertyName=\"ExtStruct\" typeName=\"Struct1\" />"
"        <ECArrayProperty propertyName=\"ExtStructs\" typeName=\"Struct1\" />"
"    </ECClass>"
"    <ECClass typeName=\"TestClass2\" >"
"        <ECProperty propertyName=\"String2\" typeName=\"string\" />"
"        <ECProperty propertyName=\"ReferenceLine\" typeName=\"Bentley.Geometry.Common.IGeometry\" displayLabel=\"Reference Line\" />"
"    </ECClass>"
"    <ECClass typeName=\"DerivedClass\" >"
"        <BaseClass>TestClass</BaseClass>"
"        <ECProperty propertyName=\"String2\" typeName=\"string\" />"
"    </ECClass>"
"</ECSchema>";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECConversionTests::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECConversionTests::TearDown()
    {
    m_wantCleanUp = false;
    T_Super::TearDown();
    }

void ECConversionTests::VerifyElement(DgnV8Api::ElementId& eid, Utf8CP className, bool isPrimaryInstance)
    {
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eid, className, isPrimaryInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
void ECConversionTests::VerifyElement(DgnDbR db, DgnV8Api::ElementId& eid, Utf8CP className, bool isPrimaryInstance)
    {
    DgnElementCPtr elem1 = FindV8ElementInDgnDb(db, eid);
    ASSERT_TRUE(elem1.IsValid());

    if (isPrimaryInstance)
        {
        EXPECT_TRUE(elem1->GetElementClass()->GetName().Equals(className));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Ridha.Malik            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ECConversionTests::LoadECXAttributes(ECObjectsV8::StandaloneECInstanceR instance , ECObjectsV8::ECClassCP elementClass)
    {
    if (0 == elementClass->GetName().compare(L"TestClass"))
        {
        DPoint3d test3dValue = { 123.456, 123.456, 123.456 };
        DPoint2d test2dValue = { 987.654, 987.654 };
        DateTime dateTime = DateTime(2016, 9, 24);
        uint32_t count = 3;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"String", ECN::ECValue(L"TestString")));
        uint32_t index = 0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"myStringArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStringArray", ECN::ECValue(L"TestString1"), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStringArray", ECN::ECValue(L"TestString2"), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStringArray", ECN::ECValue(L"TestString3"), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myInt", ECN::ECValue(100)));
        index = 0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"myIntArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myIntArray", ECN::ECValue(1), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myIntArray", ECN::ECValue(2), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myIntArray", ECN::ECValue(3), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDouble", ECN::ECValue(100.001)));
        index = 0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"myDoubleArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDoubleArray", ECN::ECValue(1.1), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDoubleArray", ECN::ECValue(2.2), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDoubleArray", ECN::ECValue(3.3), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myBool", ECN::ECValue(true)));
        index = 0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"myBoolArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myBoolArray", ECN::ECValue(false), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myBoolArray", ECN::ECValue(true), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myBoolArray", ECN::ECValue(false), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my3dPoint", ECN::ECValue(test3dValue)));
        index = 0;
        test3dValue.x = 1.0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"my3dPointArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my3dPointArray", ECN::ECValue(test3dValue), index));
        index++;
        test3dValue.x = 2.0;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my3dPointArray", ECN::ECValue(test3dValue), index));
        index++;
        test3dValue.x = 3.0;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my3dPointArray", ECN::ECValue(test3dValue), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my2dPoint", ECN::ECValue(test2dValue)));
        index = 0;
        test2dValue.x = 1.0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"my2dPointArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my2dPointArray", ECN::ECValue(test2dValue), index));
        index++;
        test2dValue.x = 2.0;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my2dPointArray", ECN::ECValue(test2dValue), index));
        index++;
        test2dValue.x = 3.0;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"my2dPointArray", ECN::ECValue(test2dValue), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDate", ECN::ECValue(dateTime)));
        index = 0;
        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.AddArrayElements(L"myDateArray", count));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDateArray", ECN::ECValue(dateTime), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDateArray", ECN::ECValue(dateTime), index));
        index++;
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myDateArray", ECN::ECValue(dateTime), index));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStruct.Name", ECN::ECValue(L"Ferris")));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStruct.No", ECN::ECValue(999)));

        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myComplicated.ExtName", ECN::ECValue(L"Complicated")));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myComplicated.ExtStruct.Name", ECN::ECValue(L"Caterpillar")));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myComplicated.ExtStruct.No", ECN::ECValue(888)));

        ASSERT_TRUE(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success == instance.InsertArrayElements(L"myStructArray", 0, 2));
        ECN::ECClassCP StructClass = instance.GetClass().GetSchema().GetClassCP(L"Struct1");
        ASSERT_TRUE(nullptr != StructClass);
        ECN::StandaloneECEnablerPtr StructEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember(StructClass->GetSchema().GetSchemaKey(), StructClass->GetName().c_str());
        ECN::StandaloneECInstancePtr StructarrayInstance = StructEnabler->CreateInstance().get();
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, StructarrayInstance->SetValue(L"Name", ECN::ECValue(L"Ferris")));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, StructarrayInstance->SetValue(L"No", ECN::ECValue(999)));
        ECN::ECValue arrayValue;
        arrayValue.SetStruct(StructarrayInstance.get());
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStructArray", arrayValue, 0));

        StructarrayInstance = StructEnabler->CreateInstance().get();
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, StructarrayInstance->SetValue(L"Name", ECN::ECValue(L"Ferris1")));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, StructarrayInstance->SetValue(L"No", ECN::ECValue(999)));
        arrayValue.SetStruct(StructarrayInstance.get());
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myStructArray", arrayValue, 1));

        // add array element and populate it
        ASSERT_TRUE(SUCCESS == instance.AddArrayElements(L"myComplicated.ExtStructs", 1));
        StructarrayInstance = StructEnabler->CreateInstance().get();
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, StructarrayInstance->SetValue(L"Name", ECN::ECValue(L"Ferris1")));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, StructarrayInstance->SetValue(L"No", ECN::ECValue(999)));
        arrayValue.SetStruct(StructarrayInstance.get());
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"myComplicated.ExtStructs", arrayValue, 0));
        }
    if (0 == elementClass->GetName().compare(L"TestClass2"))
       {
        IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
        ECObjectsV8::ECValue geomValue; 
        ASSERT_EQ(BentleyStatus::SUCCESS,geomValue.SetIGeometry(*geom));
        ASSERT_EQ(ECN::ECObjectsStatus::ECOBJECTS_STATUS_Success, instance.SetValue(L"ReferenceLine", geomValue));
       }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Ridha.Malik            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ECConversionTests::VerifyECXAttributes(BentleyApi::Dgn::DgnElementCPtr bimel)
    {
    BentleyApi::ECN::ECClassCP classname=bimel->GetElementClass();
    if (classname->GetName() == "TestClass")
       {
        BentleyApi::ECN::ECValue checkValue;
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "String"));
        ASSERT_EQ("TestString", checkValue.ToString());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myStringArray", PropertyArrayIndex(0)));
        ASSERT_EQ("TestString1", checkValue.ToString());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myStringArray", PropertyArrayIndex(1)));
        ASSERT_EQ("TestString2", checkValue.ToString());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myStringArray", PropertyArrayIndex(2)));
        ASSERT_EQ("TestString3", checkValue.ToString());
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myInt"));
        ASSERT_EQ(100, checkValue.GetInteger());
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myIntArray", PropertyArrayIndex(0)));
        ASSERT_EQ(1, checkValue.GetInteger());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myIntArray", PropertyArrayIndex(1)));
        ASSERT_EQ(2, checkValue.GetInteger());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myIntArray", PropertyArrayIndex(2)));
        ASSERT_EQ(3, checkValue.GetInteger());
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDouble"));
        ASSERT_EQ(100.001, checkValue.GetDouble());
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDoubleArray", PropertyArrayIndex(0)));
        ASSERT_EQ(1.1, checkValue.GetDouble());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDoubleArray", PropertyArrayIndex(1)));
        ASSERT_EQ(2.2, checkValue.GetDouble());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDoubleArray", PropertyArrayIndex(2)));
        ASSERT_EQ(3.3, checkValue.GetDouble());
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myBool"));
        ASSERT_EQ(true, checkValue.GetBoolean());
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myBoolArray", PropertyArrayIndex(0)));
        ASSERT_EQ(false, checkValue.GetBoolean());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myBoolArray", PropertyArrayIndex(1)));
        ASSERT_EQ(true, checkValue.GetBoolean());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myBoolArray", PropertyArrayIndex(2)));
        ASSERT_EQ(false, checkValue.GetBoolean());
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my3dPoint"));
        ASSERT_TRUE(checkValue.GetPoint3d().BentleyApi::DPoint3d::IsEqual(BentleyApi::DPoint3d::From(123.456, 123.456, 123.456)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my3dPointArray", PropertyArrayIndex(0)));
        ASSERT_TRUE(checkValue.GetPoint3d().BentleyApi::DPoint3d::IsEqual(BentleyApi::DPoint3d::From(1.0, 123.456, 123.456)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my3dPointArray", PropertyArrayIndex(1)));
        ASSERT_TRUE(checkValue.GetPoint3d().BentleyApi::DPoint3d::IsEqual(BentleyApi::DPoint3d::From(2.0, 123.456, 123.456)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my3dPointArray", PropertyArrayIndex(2)));
        ASSERT_TRUE(checkValue.GetPoint3d().BentleyApi::DPoint3d::IsEqual(BentleyApi::DPoint3d::From(3.0, 123.456, 123.456)));
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my2dPoint"));
        ASSERT_TRUE(checkValue.GetPoint2d().BentleyApi::DPoint2d::IsEqual(BentleyApi::DPoint2d::From(987.654, 987.654)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my2dPointArray", PropertyArrayIndex(0)));
        ASSERT_TRUE(checkValue.GetPoint2d().BentleyApi::DPoint2d::IsEqual(BentleyApi::DPoint2d::From(1.0, 987.654)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my2dPointArray", PropertyArrayIndex(1)));
        ASSERT_TRUE(checkValue.GetPoint2d().BentleyApi::DPoint2d::IsEqual(BentleyApi::DPoint2d::From(2.0, 987.654)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "my2dPointArray", PropertyArrayIndex(2)));
        ASSERT_TRUE(checkValue.GetPoint2d().BentleyApi::DPoint2d::IsEqual(BentleyApi::DPoint2d::From(3.0, 987.654)));
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDate"));
        ASSERT_TRUE(checkValue.GetDateTime().BentleyApi::DateTime::Equals(BentleyApi::DateTime(2016, 9, 24), true));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDateArray", PropertyArrayIndex(0)));
        ASSERT_TRUE(checkValue.GetDateTime().BentleyApi::DateTime::Equals(BentleyApi::DateTime(2016, 9, 24), true));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDateArray", PropertyArrayIndex(1)));
        ASSERT_TRUE(checkValue.GetDateTime().BentleyApi::DateTime::Equals(BentleyApi::DateTime(2016, 9, 24), true));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myDateArray", PropertyArrayIndex(2)));
        ASSERT_TRUE(checkValue.GetDateTime().BentleyApi::DateTime::Equals(BentleyApi::DateTime(2016, 9, 24), true));
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myStruct.Name"));
        ASSERT_EQ("Ferris", checkValue.ToString());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myStruct.No"));
        ASSERT_EQ(999, checkValue.GetInteger());
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myComplicated.ExtName"));
        ASSERT_EQ("Complicated", checkValue.ToString());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myComplicated.ExtStruct.Name"));
        ASSERT_EQ("Caterpillar", checkValue.ToString());
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "myComplicated.ExtStruct.No"));
        ASSERT_EQ(888, checkValue.GetInteger());
        checkValue.Clear();

        BentleyApi::ECN::ECValue arrayValue;
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(arrayValue, "myStructArray", PropertyArrayIndex(0)));
        ASSERT_TRUE(arrayValue.IsStruct());
        BentleyApi::ECN::IECInstancePtr arrayEntryInstance = arrayValue.GetStruct();
        ASSERT_TRUE(arrayEntryInstance.IsValid());
        ASSERT_EQ(BentleyApi::ECN::ECObjectsStatus::Success, arrayEntryInstance->GetValue(arrayValue, "Name"));
        ASSERT_EQ("Ferris", arrayValue.ToString());
        arrayValue.Clear();
        ASSERT_EQ(BentleyApi::ECN::ECObjectsStatus::Success, arrayEntryInstance->GetValue(arrayValue, "No"));
        ASSERT_EQ(999, arrayValue.GetInteger());
        arrayValue.Clear();
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(arrayValue, "myStructArray", PropertyArrayIndex(1)));
        ASSERT_TRUE(arrayValue.IsStruct());
        arrayEntryInstance = arrayValue.GetStruct();
        ASSERT_TRUE(arrayEntryInstance.IsValid());
        ASSERT_EQ(BentleyApi::ECN::ECObjectsStatus::Success, arrayEntryInstance->GetValue(arrayValue, "Name"));
        ASSERT_EQ("Ferris1", arrayValue.ToString());
        arrayValue.Clear();
        ASSERT_EQ(BentleyApi::ECN::ECObjectsStatus::Success, arrayEntryInstance->GetValue(arrayValue, "No"));
        ASSERT_EQ(999, arrayValue.GetInteger());
        arrayValue.Clear();

        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(arrayValue, "myComplicated.ExtStructs", PropertyArrayIndex(0)));
        ASSERT_TRUE(arrayValue.IsStruct());
        arrayEntryInstance = arrayValue.GetStruct();
        ASSERT_TRUE(arrayEntryInstance.IsValid());
        ASSERT_EQ(BentleyApi::ECN::ECObjectsStatus::Success, arrayEntryInstance->GetValue(arrayValue, "Name"));
        ASSERT_EQ("Ferris1", arrayValue.ToString());
        arrayValue.Clear();
        ASSERT_EQ(BentleyApi::ECN::ECObjectsStatus::Success, arrayEntryInstance->GetValue(arrayValue, "No"));
        ASSERT_EQ(999, arrayValue.GetInteger());
        arrayValue.Clear();
        }
    else if (classname->GetName() == "TestClass2") 
        {
        BentleyApi::ECN::ECValue checkValue;
        ASSERT_EQ(DgnDbStatus::Success, bimel->BentleyApi::Dgn::DgnElement::GetPropertyValue(checkValue, "geomValue"));
        ASSERT_FALSE(checkValue.IsNull());
        ASSERT_TRUE(checkValue.IsIGeometry());
        ASSERT_TRUE(checkValue.GetIGeometry().IsValid());
        }
      }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECConversionTests, ConversionRuleTest)
    {
    LineUpFiles(L"SupplementalSchemaWithCARule.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;

    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    filepath.AppendToPath(L"Schemas");
    filepath.AppendToPath(L"AutoPlantPDW_CustomAttributes.08.11.ecschema.xml");

    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlFile(schema, filepath.GetName(), *schemaContext));
    V8FileEditor v8Editor;
    v8Editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8Editor.m_file)));
    v8Editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    //Verify Schema, classes and Properties have been converted. 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("AutoPlantPDW_CustomAttributes");
    ASSERT_TRUE(NULL != ecSchema);

    BentleyApi::ECN::ECClassCP ecClass = ecSchema->GetClassCP("AP_3D_AttributeProperties");
    ASSERT_TRUE(NULL != ecClass);
    ASSERT_EQ(1, ecClass->GetPropertyCount());
    ASSERT_TRUE(ecClass->IsCustomAttributeClass());
    ASSERT_FALSE(ecClass->IsStructClass());

    ecClass = ecSchema->GetClassCP("AP_3D_ClassProperties");
    ASSERT_TRUE(NULL != ecClass);
    ASSERT_EQ(2, ecClass->GetPropertyCount());
    ASSERT_TRUE(ecClass->IsCustomAttributeClass());
    ASSERT_FALSE(ecClass->IsStructClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECConversionTests, VerifyLimitsForArrayPropertyAfterConversion)
    {
    LineUpFiles(L"VerifyLimitsForArrayPropertyAfterConversion.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP testSchema = db->Schemas().GetSchema("testSchema");
    ASSERT_TRUE(NULL != testSchema);

    BentleyApi::ECN::ECClassCP ecClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(ecClass != NULL);

    ASSERT_EQ(39, ecClass->GetPropertyCount()); // *** NEEDS WORK: schema changes??? 2 from testSchema:TestClass + 17 from the generic:PhysicalObject base class + ???

    BentleyApi::ECN::ECPropertyP prop = ecClass->GetPropertyP("StringArray");
    ASSERT_TRUE(prop != NULL);

    BentleyApi::ECN::ArrayECPropertyCP Array = prop->GetAsPrimitiveArrayProperty();

    ASSERT_TRUE(prop->GetIsArray());
    ASSERT_TRUE(prop->GetIsPrimitiveArray());
    ASSERT_EQ(uint32_t(0), Array->GetMinOccurs());
    ASSERT_EQ(UINT32_MAX, Array->GetMaxOccurs());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Ridha.Malik            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, ECXAdata)
    {
    LineUpFiles(L"ECXAdata.ibim", L"Test3d.dgn", false);
    DgnV8Api::DgnModelStatus modelStatus;
    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    ECObjectsV8::ECClassCP elementClass = schema->GetClassCP(L"TestClass");
    DgnV8Api::DgnECInstanceEnablerPtr enabler = DgnV8Api::DgnECManager::GetManager().ObtainInstanceEnabler(*elementClass, *(v8editor.m_file));
    ASSERT_FALSE(enabler == NULL);
    ECObjectsV8::StandaloneECInstanceR wip = enabler->GetSharedWipInstance();

    LoadECXAttributes(wip, elementClass);

    // Create a drawing model ...
    Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid, drawingModel);
    DgnV8Api::ElementHandle eh1(eid, drawingModel);

    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;

    // create createdDgnECInstance in drawing model
    ASSERT_EQ(Bentley::DgnPlatform::DgnECInstanceStatus::DGNECINSTANCESTATUS_Success, enabler->CreateInstanceOnElement(&createdDgnECInstance, wip, eh1));
    
    elementClass = schema->GetClassCP(L"TestClass2");
    DgnV8Api::DgnECInstanceEnablerPtr enabler2 = DgnV8Api::DgnECManager::GetManager().ObtainInstanceEnabler(*elementClass, *(v8editor.m_file));
    ASSERT_FALSE(enabler2 == NULL);
    ECObjectsV8::StandaloneECInstanceR wip2 = enabler2->GetSharedWipInstance();
    LoadECXAttributes(wip2, elementClass);
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh2(eid, v8editor.m_defaultModel);
    // create createdDgnECInstance in default 3dmodel
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    ASSERT_EQ(Bentley::DgnPlatform::DgnECInstanceStatus::DGNECINSTANCESTATUS_Success, enabler2->CreateInstanceOnElement(&createdDgnECInstance2, wip2, eh2));

    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    EC::ECSqlStatement stmt;
    EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT * FROM test.TestClass"));
    BentleyApi::Dgn::DgnElementId bimeid;
    BentleyApi::Dgn::DgnElementCPtr bimel;
    BentleyApi::Dgn::DgnModelId bimodelid;
    BentleyApi::Dgn::DgnModelPtr bimodel;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        bimeid = stmt.GetValueId<DgnElementId>(0);
        bimel = db->Elements().GetElement(bimeid);
        ASSERT_TRUE(bimel.IsValid());
        VerifyECXAttributes(bimel);
        bimodelid = bimel->GetModelId();
        ASSERT_TRUE(bimodelid.IsValid());
        bimodel = db->Models().GetModel(bimodelid);
        ASSERT_TRUE(bimodel.IsValid());
        if (bimodel->IsDrawingModel())
            ASSERT_TRUE(nullptr != dynamic_cast<DrawingGraphicCP>(bimel.get()));
        else
            ASSERT_TRUE(nullptr != dynamic_cast<PhysicalElementCP>(bimel.get()));
        }
    stmt.Finalize();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      1/17
//--------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, ECXA2d)
    {
    LineUpFiles(L"ECXA2d.ibim", L"Test2d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    EC::ECSqlStatement stmt;
    EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT ECInstanceId FROM test.TestClass"));
    EXPECT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    auto bimeid = stmt.GetValueId<DgnElementId>(0);
    auto bimel = db->Elements().GetElement(bimeid);
    ASSERT_TRUE(bimel.IsValid());
    ASSERT_TRUE(nullptr != dynamic_cast<DrawingGraphicCP>(bimel.get()));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateWithChangedSchema)
    {
    LineUpFiles(L"UpdateWithChangedSchema.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    {
    ECObjectsV8::ECClassP newClass;
    schema->CreateClass(newClass, L"NewClass");
    EXPECT_EQ(DgnV8Api::SCHEMAUPDATE_Success, DgnV8Api::DgnECManager::GetManager().UpdateSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();
    }

    DoUpdate(m_dgnDbFileName, m_v8FileName, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateWithChangedSchema_VersionUpdate)
    {
    LineUpFiles(L"UpdateWithChangedSchema.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    {
    schema->SetVersionMinor(3);
    EXPECT_EQ(DgnV8Api::SCHEMAUPDATE_Success, DgnV8Api::DgnECManager::GetManager().UpdateSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();
    }

    DoUpdate(m_dgnDbFileName, m_v8FileName, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateWithNewSchema)
    {
    LineUpFiles(L"UpdateWithNewSchema.ibim", L"Test3d.dgn", false);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName, true);

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateWithNewElementWithInstance)
    {
    LineUpFiles(L"UpdateWithNewElement.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    {
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance2, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    v8editor.Save();
    }

    DoUpdate(m_dgnDbFileName, m_v8FileName, false);
    EXPECT_EQ(1, m_count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateWithNewElementAndNoPreviousECData)
    {
    LineUpFiles(L"UpdateWithNewElement.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    DgnV8Api::ElementId eid;
    {
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    v8editor.Save();
    }

    DoUpdate(m_dgnDbFileName, m_v8FileName, false);
    EXPECT_EQ(1, m_count);
    VerifyElement(eid, "TestClass", true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson       03/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateExistingWithNewInstance)
    {
    LineUpFiles(L"UpdateExistingWithNewInstance.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

    //  Insert an element that has no ECInstance
    DgnV8Api::ElementId eidNoInst;
    {
    v8editor.AddLine(&eidNoInst);
    DgnV8Api::ElementHandle eh(eidNoInst, v8editor.m_defaultModel);
    v8editor.Save();
    }

    m_count = 0;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    EXPECT_EQ(1, m_count);

    // Add a primary ECInstance to an existing element.
    if (true)
        {
        DgnV8Api::ElementHandle eh(eidNoInst, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
        v8editor.Save();
        }

    m_count = 0;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false);
    ASSERT_EQ(1, m_count);
    VerifyElement(eidNoInst, nullptr, false);
    // NEEDS_WORK: The instance wasn't inserted because the TestClassElementAspect class doesn't exist.  Need to enhance the test to confirm this behavior
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdateWithSecondaryInstances)
    {
    LineUpFiles(L"UpdateUsingSecondary.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    DgnV8Api::ElementId eid;
    {
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance2, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass2"));
    }

    //  Insert an element that has no ECInstance
    DgnV8Api::ElementId eidNoInst;
    {
    v8editor.AddLine(&eidNoInst);
    DgnV8Api::ElementHandle eh2(eidNoInst, v8editor.m_defaultModel);
    }
    v8editor.Save();

    m_count = 0;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    EXPECT_EQ(2, m_count);

    // Add a primary ECInstance to an existing element.
    if (true)
        {
        DgnV8Api::ElementHandle eh(eidNoInst, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance3;
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance3, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass2"));
        v8editor.Save();
        }

    m_count = 0;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false);
    ASSERT_EQ(1, m_count);
    // Because we have initially converted a secondary instance and created the TestClass2ElementAspect class, adding an instance to an existing element will work.
    VerifyElement(eidNoInst, "TestClass2", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, CreateSecondaryInstanceWithProperties)
    {
    LineUpFiles(L"UpdateUsingSecondary.idgndb", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    DgnV8Api::ElementId eid;
    {
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance2, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"DerivedClass"));

    ECObjectsV8::ECValue v;
    v.SetString(L"String");
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, createdDgnECInstance2->SetValue(L"String", v));

    v.SetString(L"String2");
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, createdDgnECInstance2->SetValue(L"String2", v));

    const DgnV8Api::DgnECInstanceEnabler& nativeEnabler = createdDgnECInstance2->GetDgnECInstanceEnabler();
    nativeEnabler.ReplaceInstanceOnElement(NULL, *createdDgnECInstance2, createdDgnECInstance2->GetModelRef(),
                                           createdDgnECInstance2->GetElementRef(), createdDgnECInstance2->GetLocalId());
    }

    v8editor.Save();
    DoConvert(m_dgnDbFileName, m_v8FileName);

    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        Utf8String selEcSql;
        selEcSql.append("SELECT [String], [String2] FROM test.DerivedClassElementAspect");
        EC::ECSqlStatement stmt;
        stmt.Prepare(*db, selEcSql.c_str());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_TRUE(0 == strcmp("String", stmt.GetValueText(0)));
        ASSERT_TRUE(0 == strcmp("String2", stmt.GetValueText(1)));
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson       03/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, UpdatePropertyValueOnExistingInstance)
    {
    LineUpFiles(L"UpdateExistingWithNewInstance.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    //  Insert an element with a primary ECInstance
    DgnV8Api::ElementId eidWithInst;
    {
    v8editor.AddLine(&eidWithInst);
    DgnV8Api::ElementHandle eh(eidWithInst, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));
    v8editor.Save();
    }

    m_count = 0;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    EXPECT_EQ(1, m_count);

    // Modify a property of an existing (primary) ECInstance
    if (true)
        {
        DgnV8Api::EditElementHandle eeh(eidWithInst, v8editor.m_defaultModel);
        Bentley::DgnECInstancePtr inst = v8editor.QueryECInstance(eeh, L"TestSchema", L"TestClass");
        ASSERT_TRUE(inst.IsValid());
        Bentley::ECN::ECValue oldValue;
        ASSERT_EQ(Bentley::ECN::ECOBJECTS_STATUS_Success, inst->GetValue(oldValue, L"String"));
        ASSERT_TRUE(oldValue.IsNull() || 0 != wcscmp(L"Changed", oldValue.GetString()));
        inst->SetValue(L"String", Bentley::ECN::ECValue(L"Changed"));
        inst->ScheduleWriteChanges(eeh);
        eeh.ReplaceInModel(eeh.GetElementRef());
        v8editor.Save();
        }

    if (true)
        {
        // double-check that my change to V8 was saved in V8
        DgnV8Api::ElementHandle eh(eidWithInst, v8editor.m_defaultModel);
        Bentley::DgnECInstancePtr inst = v8editor.QueryECInstance(eh, L"TestSchema", L"TestClass");
        ASSERT_TRUE(inst.IsValid());
        Bentley::ECN::ECValue currentValue;
        ASSERT_EQ(Bentley::ECN::ECOBJECTS_STATUS_Success, inst->GetValue(currentValue, L"String"));
        ASSERT_TRUE(!currentValue.IsNull() && 0 == wcscmp(L"Changed", currentValue.GetString()));
        }

    m_count = 0;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false);
    ASSERT_EQ(1, m_count);

    if (true)
        {
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);
        SyncInfo::V8FileSyncInfoId editV8FileSyncInfoId;
        syncInfo.MustFindFileByName(editV8FileSyncInfoId, m_v8FileName);
        SyncInfo::V8ModelSyncInfoId editV8ModelSyncInfoId;
        syncInfo.MustFindModelByV8ModelId(editV8ModelSyncInfoId, editV8FileSyncInfoId, v8editor.m_defaultModel->GetModelId());
        DgnElementId dgnDbElementId;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementId, editV8ModelSyncInfoId, eidWithInst);

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        auto dgnDbElement = db->Elements().GetElement(dgnDbElementId);
        ASSERT_TRUE(dgnDbElement.IsValid());

        Utf8String selEcSql;
        selEcSql.append("SELECT [String] FROM ").append(dgnDbElement->GetElementClass()->GetECSqlName().c_str()).append("WHERE ECInstanceId=?");
        EC::ECSqlStatement stmt;
        stmt.Prepare(*db, selEcSql.c_str());
        stmt.BindId(1, dgnDbElementId);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_TRUE(0 == strcmp("Changed", stmt.GetValueText(0)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECConversionTests, CellWithInstance)
    {
    LineUpFiles(L"Cell.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    BentleyStatus status = ERROR;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    // -----------------------------------------------------------------------------------------------------------
    // Create Named Cell 
    DgnV8Api::EditElementHandle arcEEH1, arcEEH2;
    v8editor.CreateArc(arcEEH1, false);
    v8editor.CreateArc(arcEEH2, false);

    DgnV8Api::EditElementHandle cellEEH;
    v8editor.CreateCell(cellEEH, L"UserCell", false);

    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arcEEH1);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arcEEH2);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildComplete(cellEEH);
    EXPECT_TRUE(SUCCESS == status);

    EXPECT_TRUE(SUCCESS == cellEEH.AddToModel());
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&cellEEH), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));

    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            06/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, ImportInstanceWithCalculatedSpec)
    {
    LineUpFiles(L"Calculated.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.13\" prefix=\"bsca\" />"
        "    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"MyString\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"MyNum\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"MyCalc\" typeName=\"string\" >"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.13\">"
        "                    <ECExpression>this.MyString &amp; \"-\" &amp; this.MyNum</ECExpression>"
        "                    <IsDefaultValueOnly>False</IsDefaultValueOnly>"
        "                    <UseLastValidValueOnFailure>True</UseLastValidValueOnFailure>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECClass>"
        "</ECSchema>";

    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    ASSERT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

    Bentley::DgnFileR targetFile = *(v8editor.m_defaultModel->GetDgnFileP());
    Bentley::DgnECInstanceEnablerP   elementECInstanceEnabler = NULL;

    elementECInstanceEnabler = DgnV8Api::ECXAProvider::GetProvider().ObtainInstanceEnablerByName(L"TestSchema", L"TestClass", DgnV8Api::ECXAProvider::GetProvider().GetPerFileCache(targetFile));
    ASSERT_TRUE(NULL != elementECInstanceEnabler);

    ECObjectsV8::StandaloneECInstanceR wipInstance = elementECInstanceEnabler->GetSharedWipInstance();
    wipInstance.SetValue(L"MyString", Bentley::ECN::ECValue(L"ABC"));
    wipInstance.SetValue(L"MyNum", Bentley::ECN::ECValue(123));

    DgnV8Api::DgnECInstanceStatus ecInstanceStatus;
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    DgnV8Api::ElementId eidWithInst;
    {
    v8editor.AddLine(&eidWithInst);
    DgnV8Api::ElementHandle eh(eidWithInst, v8editor.m_defaultModel);
    ecInstanceStatus = elementECInstanceEnabler->CreateInstanceOnElement(&createdDgnECInstance, wipInstance, eh);
    ASSERT_EQ(DgnV8Api::DGNECINSTANCESTATUS_Success, ecInstanceStatus);
    v8editor.Save();
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    // Verify the converted values
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EC::ECSqlStatement stmt;
        EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT MyCalc FROM test.TestClass WHERE MyCalc='ABC-123'"));

        EC::ECInstanceECSqlSelectAdapter adapter(stmt);
        bool found = false;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            BentleyApi::ECN::IECInstancePtr actual = adapter.GetInstance();
            BentleyApi::ECN::ECValue v;
            ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "MyCalc"));
            ASSERT_TRUE(!v.IsNull());
            ASSERT_TRUE(strcmp("ABC-123", v.GetUtf8CP()) == 0);
            found = true;
            }
        ASSERT_TRUE(found) << "Should have found an instance using a WHERE criteria";
    }

    // Update the values
        {
        ECObjectsV8::ECValue v;
        v.SetString(L"XYZ");
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, createdDgnECInstance->SetValue(L"MyString", v));

        v.SetInteger(987);
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, createdDgnECInstance->SetValue(L"MyNum", v));

        const DgnV8Api::DgnECInstanceEnabler& nativeEnabler = createdDgnECInstance->GetDgnECInstanceEnabler();
        nativeEnabler.ReplaceInstanceOnElement(NULL, *createdDgnECInstance, createdDgnECInstance->GetModelRef(),
                                               createdDgnECInstance->GetElementRef(), createdDgnECInstance->GetLocalId());
        v8editor.Save();
        }
    DoUpdate(m_dgnDbFileName, m_v8FileName);

    // Verify the converted values
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EC::ECSqlStatement stmt;
        EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT MyCalc FROM test.TestClass WHERE MyCalc='XYZ-987'"));

        EC::ECInstanceECSqlSelectAdapter adapter(stmt);
        bool found = false;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            BentleyApi::ECN::IECInstancePtr actual = adapter.GetInstance();
            BentleyApi::ECN::ECValue v;
            ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "MyCalc"));
            ASSERT_TRUE(!v.IsNull());
            ASSERT_TRUE(strcmp("XYZ-987", v.GetUtf8CP()) == 0);
            found = true;
            }
        ASSERT_TRUE(found) << "Should have found an instance using a WHERE criteria after update";
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, ImportInstanceWithCalculatedSpecUsingRelated)
    {
    LineUpFiles(L"CalculatedRelated.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.13\" prefix=\"bsca\" />"
        "    <ECClass typeName=\"Owner\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"FirstName\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"LastName\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"FullName\" typeName=\"string\" >"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.13\">"
        "                    <ECExpression>this.FirstName &amp; \"-\" &amp; this.LastName</ECExpression>"
        "                    <IsDefaultValueOnly>False</IsDefaultValueOnly>"
        "                    <UseLastValidValueOnFailure>True</UseLastValidValueOnFailure>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECClass>"
        "    <ECClass typeName=\"Chair\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Color\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"OwnerName\" typeName=\"string\" >"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.13\">"
        "                    <ECExpression>this.GetRelatedInstance(\"ChairHasOwner:0:Owner\").FirstName</ECExpression>"
        "                    <IsDefaultValueOnly>False</IsDefaultValueOnly>"
        "                    <UseLastValidValueOnFailure>True</UseLastValidValueOnFailure>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"ChairHasOwner\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "       <Source cardinality=\"(0,1)\" polymorphic=\"True\">"
        "           <Class class=\"Chair\" />"
        "       </Source>"
        "       <Target cardinality=\"(0,1)\" polymorphic=\"True\">"
        "           <Class class=\"Owner\"/>"
        "       </Target>"
        "   </ECRelationshipClass>"

        "</ECSchema>";

    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    ASSERT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

    Bentley::DgnFileR targetFile = *(v8editor.m_defaultModel->GetDgnFileP());
    Bentley::DgnECInstanceEnablerP   ownerElementEnabler = DgnV8Api::ECXAProvider::GetProvider().ObtainInstanceEnablerByName(L"TestSchema", L"Owner", DgnV8Api::ECXAProvider::GetProvider().GetPerFileCache(targetFile));
    ASSERT_TRUE(NULL != ownerElementEnabler);

    ECObjectsV8::StandaloneECInstanceR wipInstance = ownerElementEnabler->GetSharedWipInstance();
    wipInstance.SetValue(L"FirstName", Bentley::ECN::ECValue(L"John"));
    wipInstance.SetValue(L"LastName", Bentley::ECN::ECValue(L"Smith"));

    DgnV8Api::DgnECInstanceStatus ecInstanceStatus;
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    DgnV8Api::ElementId eidWithInst;
    v8editor.AddLine(&eidWithInst);
    DgnV8Api::ElementHandle eh(eidWithInst, v8editor.m_defaultModel);
    ecInstanceStatus = ownerElementEnabler->CreateInstanceOnElement(&createdDgnECInstance, wipInstance, eh);
    ASSERT_EQ(DgnV8Api::DGNECINSTANCESTATUS_Success, ecInstanceStatus);

    Bentley::DgnECInstanceEnablerP   chairElementEnabler = DgnV8Api::ECXAProvider::GetProvider().ObtainInstanceEnablerByName(L"TestSchema", L"Chair", DgnV8Api::ECXAProvider::GetProvider().GetPerFileCache(targetFile));
    ASSERT_TRUE(NULL != chairElementEnabler);

    ECObjectsV8::StandaloneECInstanceR wipInstance2 = chairElementEnabler->GetSharedWipInstance();
    wipInstance2.SetValue(L"Color", Bentley::ECN::ECValue(L"Purple"));

    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    DgnV8Api::ElementId eidWithInst2;
    v8editor.AddLine(&eidWithInst2);
    DgnV8Api::ElementHandle eh2(eidWithInst2, v8editor.m_defaultModel);
    ecInstanceStatus = chairElementEnabler->CreateInstanceOnElement(&createdDgnECInstance2, wipInstance2, eh2);
    ASSERT_EQ(DgnV8Api::DGNECINSTANCESTATUS_Success, ecInstanceStatus);

    Bentley::DgnECRelationshipEnablerP relationshipEnabler = DgnV8Api::ECXAProvider::GetProvider().ObtainRelationshipEnabler(L"TestSchema", L"ChairHasOwner", targetFile);
    ECObjectsV8::StandaloneECRelationshipInstanceR wipRelInstance = relationshipEnabler->GetSharedStandaloneWipInstance();
    DgnV8Api::IDgnECRelationshipInstancePtr ecxRelationship = NULL;
    EXPECT_EQ(SUCCESS, relationshipEnabler->CreateRelationship(&ecxRelationship, wipRelInstance, *createdDgnECInstance, *createdDgnECInstance2, eh.GetModelRef(), eh.GetElementRef()));

    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    //Verify the converted values
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EC::ECSqlStatement stmt;
        EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT Color, OwnerName FROM test.Chair WHERE OwnerName='John'"));

        EC::ECInstanceECSqlSelectAdapter adapter(stmt);
        bool found = false;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            BentleyApi::ECN::IECInstancePtr actual = adapter.GetInstance();
            BentleyApi::ECN::ECValue v;
            ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "Color"));
            ASSERT_TRUE(!v.IsNull());
            ASSERT_TRUE(strcmp("Purple", v.GetUtf8CP()) == 0);

            ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "OwnerName"));
            ASSERT_TRUE(!v.IsNull());
            ASSERT_TRUE(strcmp("John", v.GetUtf8CP()) == 0);
            found=true;
            }
        ASSERT_TRUE(found) << "Should have found an instance using a WHERE criteria";
        }
    // Update the values
        {
        ECObjectsV8::ECValue v;
        v.SetString(L"Jack");
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, createdDgnECInstance->SetValue(L"FirstName", v));

        const DgnV8Api::DgnECInstanceEnabler& nativeEnabler = createdDgnECInstance->GetDgnECInstanceEnabler();
        nativeEnabler.ReplaceInstanceOnElement(NULL, *createdDgnECInstance, createdDgnECInstance->GetModelRef(),
                                               createdDgnECInstance->GetElementRef(), createdDgnECInstance->GetLocalId());
        v8editor.Save();
        }
    DoUpdate(m_dgnDbFileName, m_v8FileName);

    // Verify the converted values
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EC::ECSqlStatement stmt;
        EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT Color, OwnerName FROM test.Chair WHERE OwnerName='Jack'"));

        EC::ECInstanceECSqlSelectAdapter adapter(stmt);
        bool found = false;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            found = true;
            BentleyApi::ECN::IECInstancePtr actual = adapter.GetInstance();
            BentleyApi::ECN::ECValue v;
            ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "Color"));
            ASSERT_TRUE(!v.IsNull());
            ASSERT_TRUE(strcmp("Purple", v.GetUtf8CP()) == 0);

            ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "OwnerName"));
            ASSERT_TRUE(!v.IsNull());
            ASSERT_TRUE(strcmp("Jack", v.GetUtf8CP()) == 0);
            }
        ASSERT_TRUE(found) << "Should have found an instance using a WHERE criteria after update";
        }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECConversionTests, RelationshipInstance)
    {
    LineUpFiles(L"UpdateWithChangedSchema.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    ECObjectsV8::ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, L"ALikesB");

    ECObjectsV8::StandaloneECRelationshipEnablerPtr  enabler = ECObjectsV8::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relationshipClass);
    ECObjectsV8::StandaloneECRelationshipInstancePtr relInstance = enabler->CreateRelationshipInstance();

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);

    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass"));

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    {
    ECObjectsV8::ECClassP newClass;
    schema->CreateClass(newClass, L"NewClass");
    EXPECT_EQ(DgnV8Api::SCHEMAUPDATE_Success, DgnV8Api::DgnECManager::GetManager().UpdateSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();
    }

    DoUpdate(m_dgnDbFileName, m_v8FileName, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECConversionTests, IGeometryValue)
    {
    LineUpFiles(L"IGeometryValue.idgndb", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);

    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"TestClass2"));

    DEllipse3d ellipseData = DEllipse3d::From(0.0, 0.0, 0.0,
                                              1.0, 0.0, 0.0,
                                              0.0, 1.0, 0.0,
                                              0.0, Angle::TwoPi());

    ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc(ellipseData);
    IGeometryPtr iGeom = IGeometry::Create(originalArc);
    ECObjectsV8::ECValue geomValue;
    geomValue.SetIGeometry(*iGeom);
    createdDgnECInstance->SetValue(L"ReferenceLine", geomValue);
    createdDgnECInstance->WriteChanges();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    EC::ECSqlStatement stmt;
    EXPECT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT * FROM test.TestClass2"));

    EC::ECInstanceECSqlSelectAdapter adapter(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        BentleyApi::ECN::IECInstancePtr actual = adapter.GetInstance();
        BentleyApi::ECN::ECValue v;
        ASSERT_TRUE(BentleyApi::ECN::ECObjectsStatus::Success == actual->GetValue(v, "ReferenceLine"));
        ASSERT_TRUE(!v.IsNull());
        ASSERT_TRUE(v.IsIGeometry());
        BentleyApi::IGeometryPtr dbGeom = v.GetIGeometry();
        ASSERT_TRUE(dbGeom.IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataValidation_Tests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    protected:

        void VerifyInstanceCounts(BentleyApi::BeFileName& dbFilePath, BentleyApi::bmap<BentleyApi::Utf8String, int>& benchMark, BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String>& schemasToCheck);
        void VerifySchemasExist(DgnDbPtr db, BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String>& schemasToCheck);

        void SetUp()
            {
            T_Super::SetUp();
            }

        void TearDown()
            {
            m_wantCleanUp = false;
            T_Super::TearDown();
            }
    };

void DataValidation_Tests::VerifySchemasExist(DgnDbPtr db, BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String>& schemasToCheck)
    {
    for each (auto schemaName in schemasToCheck)
        {
        BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema(schemaName.c_str());
        ASSERT_TRUE(NULL != ecSchema) << "ECSchema " << schemaName.c_str() << " in file " << db->GetFileName().GetFileNameAndExtension().c_str() << " hasn't been converted";
        }
    }

#define ASSERT_ECSCHEMAS(DBPTR, SCHEMASLIST)         VerifySchemasExist(DBPTR, SCHEMASLIST);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DataValidation_Tests::VerifyInstanceCounts(BentleyApi::BeFileName& dbFilePath, BentleyApi::bmap<BentleyApi::Utf8String, int>& benchMark, BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String>& schemasToCheck)
    {
    BentleyApi::BeSQLite::Db::OpenMode mode = BentleyApi::BeSQLite::Db::OpenMode::Readonly;

    DgnDbPtr db = OpenExistingDgnDb(dbFilePath, mode);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_ECSCHEMAS(db, schemasToCheck);

    BentleyApi::Bstdcxx::bvector<BentleyApi::ECN::ECSchemaCP> schemaList = db->Schemas().GetSchemas();
    ASSERT_FALSE(schemaList.empty());

    BentleyApi::BeSQLite::EC::ECSqlStatement stmt;

    BentleyApi::bmap<BentleyApi::Utf8String, int> classList;

    for (auto i = schemaList.begin(); i != schemaList.end(); ++i)
        {
        BentleyApi::ECN::ECSchemaCP schema = *i;
        BentleyApi::Utf8StringCR schemaName = schema->GetName();
        BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String>::const_iterator iter = std::find(schemasToCheck.begin(), schemasToCheck.end(), schemaName);

        if (schemasToCheck.end() != iter)
            {
            BentleyApi::Utf8String query;
            for (BentleyApi::ECN::ECClassCP const& ecClass : schema->GetClasses())
                {
                if (ecClass->IsEntityClass() == false || BentleyApi::ECN::ECClassModifier::Abstract == ecClass->GetClassModifier())
                    continue;
                if (!Utf8String::IsNullOrEmpty(query.c_str()))
                    query.append(" UNION ALL ");

                query.append("SELECT COUNT(*), '").append(ecClass->GetName()).append("' FROM ONLY ").append(ecClass->GetECSqlName());

                if (query.empty())
                    continue;

                ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, query.c_str())) << "Statement prepare failed for " << query.c_str();
                while (DbResult::BE_SQLITE_ROW == stmt.Step())
                    {
                    int count = stmt.GetValueInt(0);
                    if (0 != count)
                        {
                        classList[stmt.GetValueText(1)] = stmt.GetValueInt(0);
                        printf("class: %s, count: %d \n", stmt.GetValueText(1), stmt.GetValueInt(0));
                        }
                    }
                stmt.Finalize();
                query.clear();
                }
            }
        }

    if (classList.size() != benchMark.size())
        {
        for (BentleyApi::bmap<BentleyApi::Utf8String, int>::const_iterator iter = classList.begin(); iter != classList.end(); iter++)
            LOG.errorv("%s:%d", iter->first.c_str(), iter->second);
        }
    ASSERT_TRUE(classList.size() == benchMark.size()) << "Class count doesn't match, Db Name: " << db->GetFileName().GetFileNameAndExtension().c_str() << " Expected: " << (int) benchMark.size() << " Actual: " << (int) classList.size();

    BentleyApi::bmap<BentleyApi::Utf8String, int>::iterator i, j;
    i = classList.begin();
    j = benchMark.begin();

    while (i != classList.end() && j != benchMark.end())
        {
        BentleyApi::Utf8String actualClassName = i->first;
        BentleyApi::Utf8String expectedClassName = j->first;
        int actualInstanceCount = i->second;
        int expectedInstanceCount = j->second;

        EXPECT_STREQ(expectedClassName.c_str(), actualClassName.c_str()) << "ECClass '" << expectedClassName.c_str() << "' Doesn't exist in converted Db '" << db->GetFileName().GetFileNameAndExtension().c_str() << "'";
        EXPECT_EQ(expectedInstanceCount, actualInstanceCount) << "Instance count mismatch for class '" << i->first << "'";
        ++i;
        ++j;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TestDataNotAvailable
TEST_F(DataValidation_Tests, VerifyECData)
    {
    BentleyApi::BeFileName outFile;
    GetWorkingDgnDbFile(outFile);
    DgnDbPtr db = OpenExistingDgnDb(outFile, Db::OpenMode::Readonly);
    ASSERT_TRUE(db->IsDbOpen());

    BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String> schemasToCheck {"AutoPlantPDWPersistenceStrategySchema", "AutoPlantPDW_CustomAttributes", "ProStructures"};
    ASSERT_ECSCHEMAS(db, schemasToCheck);

    BentleyApi::BeSQLite::EC::ECSqlStatement stmt;
    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT TAG FROM appdw.Equipment WHERE EQUIP_NO='50P-104B'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("50P-104B", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COUNT(EQUIP_NO) FROM appdw.Equipment WHERE INSUL_THK='2' AND DES_TEMP='0'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT DISTINCT INSULATION FROM appdw.PipingComponent"));
    Utf8String ExpectedInsulationValue = "AA-N-";
    Utf8String ActualInsulationValue = "";
    while (stmt.Step() != DbResult::BE_SQLITE_DONE)
        {
        ActualInsulationValue.append(stmt.GetValueText(0));
        ActualInsulationValue.append("-");
        }
    ASSERT_EQ(ExpectedInsulationValue, ActualInsulationValue);
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT AREA FROM appdw.PipingComponent WHERE MAIN_SIZE='10'"));
    int ExpectedSumOfArea = 500;
    int ActualSumOfArea = 0;
    while (stmt.Step() != DbResult::BE_SQLITE_DONE)
        {
        ActualSumOfArea += stmt.GetValueInt(0);
        }
    ASSERT_EQ(ExpectedSumOfArea, ActualSumOfArea);
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT EQUIP_NO FROM appdw.Nozzle WHERE COMP_LEN='4.5' OR COMPONENT_ID='AT_EGXUTRMA_1A'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("50TW-102", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT NAME FROM appdw.Pipeline WHERE NAME LIKE 'TRIM'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("TRIM", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COMPONENT_ID FROM appdw.Equipment_Part WHERE EQUIP_NO IN(SELECT EQUIP_NO FROM appdw.Equipment_Part WHERE POSX=15798.99902)"));
    Utf8String ExpectedCompIdValue = "AT_EGXUTRMA_3-AT_EGXUTRMA_5-AT_EGXUTRMA_8-AT_EGXUTRMA_9-";
    Utf8String ActualCompIdValue = "";
    while (stmt.Step() != DbResult::BE_SQLITE_DONE)
        {
        ActualCompIdValue.append(stmt.GetValueText(0));
        ActualCompIdValue.append("-");
        }
    ASSERT_EQ(ExpectedCompIdValue, ActualCompIdValue);
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COMPONENT_ID FROM appdw.Nozzle WHERE COMPONENT_ID LIKE '%_3G'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("AT_EGXUTRMA_3G", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COUNT(EQUIP_NO) FROM appdw.Equipment WHERE POSX BETWEEN 16300 AND 16400"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(2, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COUNT(COMPONENT_ID) FROM appdw.Equipment_Part WHERE TAG IS NULL"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT DISTINCT MODULE_NAME FROM appdw.PipingComponent WHERE COMPONENT_ID IN(SELECT COMPONENT_ID FROM appdw.PipingComponent WHERE MAIN_SIZE IS NOT NULL AND MAIN_SIZE='10')"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("Base", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT SUM(POSX) FROM appdw.Equipment WHERE INSUL_THK='2'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(95429, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT MAX(POSY) FROM appdw.Equipment"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(5403.87402, stmt.GetValueDouble(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT AVG(POSZ) FROM appdw.Equipment"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(239, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COUNT(INSUL_THK) FROM appdw.Equipment WHERE INSUL_THK='2'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT EQUIP_NO FROM ONLY appdw.Equipment WHERE EQUIP_NO='50E-101A'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("50E-101A", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "Select COMPONENT_ID From appdw.Nozzle where COMPONENT_ID ='AT_EGXUTRMA_1P' "));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ("AT_EGXUTRMA_1P", stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "Select COUNT(SPEC_TABLE) From appdw.PipingComponent where SPEC_TABLE Like 'ELBOW'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(147, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "Select COMPONENT_ID From appdw.Nozzle where COMPONENT_ID Like '%A_3%'"));

    Utf8String ExpectedStringValue = "AT_EGXUTRMA_35-AT_EGXUTRMA_36-AT_EGXUTRMA_37-AT_EGXUTRMA_38-AT_EGXUTRMA_3D-AT_EGXUTRMA_3E-AT_EGXUTRMA_3F-AT_EGXUTRMA_3G-";
    Utf8String ActualStringValue = "";

    while (stmt.Step() != DbResult::BE_SQLITE_DONE)
        {
        ActualStringValue.append(stmt.GetValueText(0));
        ActualStringValue.append("-");
        }
    ASSERT_EQ(ExpectedStringValue, ActualStringValue);
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::InvalidECSql, stmt.Prepare(*db, "Select * FROM appdw.blabla")) << "No such table exists.";
    stmt.Finalize();

    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*db, "Select POSX,POSY,POSZ From appdw.Equipment WHERE EQUIP_NO='50TW-102'"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(16026.99902, stmt.GetValueDouble(0));
    ASSERT_EQ(5235.62402, stmt.GetValueDouble(1));
    ASSERT_EQ(353.99803, stmt.GetValueDouble(2));
    stmt.Finalize();
    }
#endif // TestDataNotAvailable

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataValidation_Tests, VerifyViewsForConvertediBimFile)
    {
    BentleyApi::BeFileName sourceFilesPath = GetInputFileName(L"ibimFiles");

    sourceFilesPath.AppendToPath(L"*.ibim");
    BentleyApi::BeFileListIterator filesIterator(sourceFilesPath, false);
    BentleyApi::BeFileName dbName;

    while (filesIterator.GetNextFileName(dbName) != ERROR)
        {
        //printf("ibim Name: %s\n", dbName.GetNameUtf8().c_str());

        DgnDbPtr dgnProj = OpenExistingDgnDb(dbName, Db::OpenMode::Readonly);
        EXPECT_TRUE(dgnProj->IsDbOpen());

        BentleyApi::BeSQLite::Statement statement;
        EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, statement.Prepare(*dgnProj, "select '[' || name || ']'  from sqlite_master where type = 'view' and instr (name,'.') and instr(sql, '--### ECCLASS VIEW')"));
        while (statement.Step() == BE_SQLITE_ROW)
            {
            //printf ("\n ViewName : %s \n", statement.GetValueText (0));
            BentleyApi::BeSQLite::Statement stmt;
            BentleyApi::Utf8String sql;
            sql.Sprintf("SELECT * FROM %s", statement.GetValueText(0));
            //printf("Select sql:  %s \n", sql.c_str());
            EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, stmt.Prepare(*dgnProj, sql.c_str())) << "ECClassView " << stmt.GetValueText(0) << " has invalid DDL: " << dgnProj->GetLastError().c_str() << " in DgnDb : " << dbName.c_str();
            }
        statement.Finalize();
        dgnProj->CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataValidation_Tests, VerifyECInstanceCount)
    {
    BentleyApi::BeFileName sqliteDb = GetInputFileName(L"InstanceVerification.db");

    BentleyApi::BeSQLite::Db db;
    BentleyApi::BeSQLite::DbResult status = db.OpenBeSQLiteDb(sqliteDb, BentleyApi::BeSQLite::Db::OpenParams(BentleyApi::BeSQLite::Db::OpenMode::Readonly));
    ASSERT_EQ(status, BE_SQLITE_OK);
    ASSERT_TRUE(db.IsDbOpen());

    BentleyApi::BeFileName sourceFilesPath = GetInputFileName(L"ibimFiles");

    sourceFilesPath.AppendToPath(L"*.ibim");
    BentleyApi::BeFileListIterator filesIterator(sourceFilesPath, false);
    BentleyApi::BeFileName dbName;

    while (filesIterator.GetNextFileName(dbName) != ERROR)
        {
        printf("\n Db Name: %s\n", dbName.GetNameUtf8().c_str());

        BentleyApi::BeSQLite::Statement statement;
        Utf8String sql;
        sql.Sprintf("SELECT DISTINCT SchemaName FROM ec_Data WHERE FileName='%ls' AND SchemaName != ''", dbName.GetFileNameWithoutExtension().c_str());
        //printf("sql:%s \n", sql.c_str());
        ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(db, sql.c_str()));
        BentleyApi::Bstdcxx::bvector<BentleyApi::Utf8String> schemasToCheck;
        while (statement.Step() != BE_SQLITE_DONE)
            {
            schemasToCheck.push_back(statement.GetValueText(0));
            //printf("Schema:%s\n", statement.GetValueText(0));
            }

        statement.Finalize();
        sql.Sprintf("SELECT ClassName, Count FROM ec_Data WHERE FileName='%ls' AND ClassName != ''", dbName.GetFileNameWithoutExtension().c_str());
        //printf("sql:%s \n", sql.c_str());
        ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(db, sql.c_str()));
        BentleyApi::bmap<BentleyApi::Utf8String, int> benchMark;
        while (statement.Step() != BE_SQLITE_DONE)
            {
            benchMark[statement.GetValueText(0)] = statement.GetValueInt(1);
            //printf("ClassName:%s, Count:%d\n", statement.GetValueText(0), statement.GetValueInt(1));
            }

        VerifyInstanceCounts(dbName, benchMark, schemasToCheck);
        }
    }
