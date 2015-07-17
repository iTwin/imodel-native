/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/UnitsTestBase.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include "../ECObjectsTestPCH.h"

#include <ECUnits/Units.h>

USING_NAMESPACE_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
/*====================================================================================**/
/// <summary>Tests for Bentley.Units and Bentley.ECObjects.Units.</summary>
/// <author>Colin.Kerr</author>                             <date>3/2008</date>
/*==============+===============+===============+===============+===============+======*/
struct UnitsTestBase : ECTestFixture
{
public: UnitsTestBase(){}
private: const  Utf8String UNIT_SPECIFICATIONS_PROPERTY;// = L"UnitSpecifications";
/// <summary>The schema to test</summary>
protected: ECSchemaPtr m_testSchema;
/// <summary>The supplemented version of the schema to test</summary>
ECSchemaPtr m_supplementedSchema;
/// <summary>The schema supplemental modification to test</summary>
ECSchemaPtr m_supplementalSchemaMod;
/// <summary>The list of supplemental schemas to test</summary>
bvector< ECSchemaPtr > m_supplementalSchemas;
/// <summary>The second schema to test</summary>
ECSchemaPtr  m_testSchema2;
/// <summary>The schema referenced by the schema to test</summary>
ECSchemaPtr m_referencedSchema;

///// <summary>Wheel Class</summary>
ECClassP m_wheelClass;//                 = null;
///// <summary>Wheel Property</summary>
//IECProperty m_diameterProp            = null;
///// <summary>Wheel Property</summary>
//IECProperty m_radiusProp              = null;
///// <summary>Wheel Property</summary>
ECPropertyP m_spokeLengthProp;//         = null;
///// <summary>Wheel Property</summary>
//IECProperty m_distanceTraveledProp    = null;
///// <summary>Wheel Property</summary>
ECPropertyP m_AreaProp;//                = null;
///// <summary>Wheel Property</summary>
//IECProperty m_widthProp               = null;
///// <summary>Wheel Property</summary>
//IECProperty m_quantityProp            = null;
///// <summary>Wheel Property</summary>
//IECProperty m_weightProp              = null;
///// <summary>Wheel Property</summary>
ECPropertyP m_hubStructProp;

/// <summary>Bike Class</summary>
ECClassP m_BikeClass;
/// <summary>Bike Property</summary>
ECPropertyP m_frontWheelDiameterProp;
/// <summary>Bike Property</summary>
ECPropertyP m_frontWheelPressureProp;
/// <summary>Bike Property</summary>
ECPropertyP m_rearWheelDiameterProp;
/// <summary>Bike Property</summary>
ECPropertyP m_rearWheelPressureProp;
/// <summary>Bike Property</summary>
ECPropertyP m_trainingWheelDiameterProp;
/// <summary>Bike Property</summary>
ECPropertyP m_frameHeightProp;
/// <summary>Bike Property</summary>
ECPropertyP m_headSetAngleProp;
/// <summary>Bike Property</summary>
ECPropertyP m_seatPostAngleProp;

///// <summary>Wheels Child Class (class that has Wheel as a base class)</summary>
ECClassP m_wheelsChildClass;//   = null;
///// <summary>Wheel child Property</summary>
ECPropertyP m_wcDiameterProp;//  = null;
///// <summary>Wheel child Property</summary>
ECPropertyP m_wcWeightProp;//    = null;

/// <summary>Exception Class</summary>
ECClassP m_ExceptionClass;

/// <summary>StandardUnits Class</summary>
ECClassP m_standardUnitsClass;
/// <summary>Standard Units Property</summary>
ECPropertyP m_sucAreaProp;
/// <summary>Standard Units Property</summary>
ECPropertyP m_sucVolumeProp;
/// <summary>Standard Units Property</summary>
ECPropertyP m_sucTemperatureProp;
/// <summary>Standard Units Property</summary>
ECPropertyP m_sucWidthProp;

/*/// <summary>Pipe Class</summary>
ECClassP m_pipeClass                  = null;
/// <summary>Pipe Property</summary>
IECProperty m_roughnessProp           = null;
/// <summary>Pipe Property</summary>
IECProperty m_absRoughnessProp        = null;
/// <summary>Pipe Property</summary>
IECProperty m_manningsRoughnessProp   = null;
/// <summary>Pipe Property</summary>
IECProperty m_kuttersRoughnessProp    = null;
/// <summary>Pipe Property</summary>
IECProperty m_pipeDepthProp           = null;
/// <summary>Pipe Property</summary>
IECProperty m_scaleFactorProp         = null;
/// <summary>Pipe Property</summary>
IECProperty m_flowChangeProp          = null;
/// <summary>Pipe Property</summary>
IECProperty m_mrNoDimensionProp       = null*/;

// Referenced and Derrived Classes
/// <summary>A referenced class</summary>
ECClassP m_referencedClass;
/// <summary> A referenced property</summary>
ECPropertyP m_classLengthReferenced;
/// <summary>A derived class</summary>
ECClassP m_derivedClass;
/// <summary>A derived Property</summary>
ECPropertyP m_classLengthDerived;
/// <summary>A derived class that overrides a property Property</summary>
ECClassP m_derivedClassOverrideProp;
/// <summary>An overrridden Property</summary>
ECPropertyP m_classLengthOverrideProp;
/// <summary>A derived class which has a property that overrides a unit</summary>
ECClassP m_derivedClassOverrideUnit;
/// <summary>A Property that overrides the unit from it's base property</summary>
ECPropertyP m_classLengthOverrideUnit;

// Allowable Unit Lists
// Wheel Class
///// <summary>Expected allowable units for wheel class</summary>
//List<string> m_expectedResults_ForWheelClass          = null;
///// <summary>Expected allowable units for radius property</summary>
//List<string> m_expectedResults_ForRadiusProperty      = null;
///// <summary>Expected allowable units for spoke length property</summary>
//List<string> m_expectedResults_ForSpokeLengthProperty = null;
///// <summary>Expected allowable units for area koq</summary>
//List<string> m_expectedResults_ForArea                = null;
///// <summary>Expected allowable units for diameter koq</summary>
//List<string> m_expectedResults_ForDiameter            = null;
///// <summary>Expected allowable units for width koq</summary>
//List<string> m_expectedResults_ForWidth               = null;
///// <summary>Expected allowable units for diameter large koq</summary>
//List<string> m_expectedResults_ForDiameterLarge       = null;
//
//// Bike Class
///// <summary>Expected allowable units for front wheel property</summary>
//List<string> m_expectedResults_ForFrontWheelProperty      = null;
///// <summary>Expected allowable units for rear wheel property</summary>
//List<string> m_expectedResults_ForRearWheelProperty       = null;
///// <summary>Expected allowable units for training wheel property</summary>
//List<string> m_expectedResults_ForTrainingWheelProperty   = null;
///// <summary>Expected allowable units for frame height property</summary>
//List<string> m_expectedResults_ForFrameHeightProperty     = null;
//
//// PipeClass Class
///// <summary>Expected allowable units for depth koq</summary>
//List<string> m_expectedResults_ForDepthKOQ_Pipe               = null;
///// <summary>Expected allowable units for mannings roughness koq</summary>
//List<string> m_expectedResults_ForManningsRoughnessKOQ_Pipe   = null;
//
//// Referenced and Derived classes
///// <summary>Expected allowable units for class length property</summary>
//List<string> m_expectedResults_ForClassLengthProperty = null;
//
///// <summary>The ECSchemaUnitsManager for testing</summary>
//ECSchemaUnitsManager m_manager = null;
//
//#region Initialization
/*------------------------------------------------------------------------------------**/
/// <summary>Test Setup.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public: void SetUp
(
);

/*------------------------------------------------------------------------------------**/
/// <summary>Initializes the units framework for test.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public: void InitializeUnits
(
Utf8String testSchemaName,
bvector< ECSchemaP > & testSupplementalSchemas
);

/*------------------------------------------------------------------------------------**/
/// <summary>Cleaning display preferences before each test to avoid unknown start state.</summary>
/// <author>Colin.Kerr</author>                            <date>7/2010</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
//[SetUp]
//public: void TestSetup
//(
//)
//    {
//    UnitDisplayPreferences.Current.ClearDisplayUnitSpecifications ();
//    }

/*------------------------------------------------------------------------------------**/
/// <summary>Creates a copy of the 'TestSupplementalSchema' then modifies the
/// Wheel.Weight property so that it's unit is 'GRAM'.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
//private: void DuplicateAndModifyTestSupplementalSchema
//(
//List<IECSchema> supplementalSchemas
//)
//    {
//    foreach (IECSchema schema in supplementalSchemas)
//        {
//        if (schema.FullName == "TestSupplementalSchema.01.00")
//            {
//            List<UnitSpecification> supplementalUnitSpecs = UnitsSchemaReader.BuildUnitSpecificationsFromSchema (schema);
//            foreach (UnitSpecification unitSpec in supplementalUnitSpecs)
//                {
//                if (unitSpec.ClassName == "Wheel" && unitSpec.PropertyName == "Weight")
//                    unitSpec.Unit = StandardUnits.GRAM;
//                }
//            m_supplementalSchemaMod = UnitsSchemaCreator.BuildSchemaFromUnitSpecifications ("TestSupplementalSchema", 1, 1, schema.NamespacePrefix,
//                                                        UnitsConstants.SupplementalSchema, 500, "testschema", 1, 0, supplementalUnitSpecs);
//            }
//        }
//    }

//private: ECSchemaP DuplicateSchema
//(
//ECSchemaP originalSchema
//)
//    {
//    ECSchemaP schemaCopy = NULL;
//    if (null != originalSchema)
//        {
//        using (Stream schemaStream = new MemoryStream ())
//            {
//            XmlTextWriter schemaWriter = NULL;
//            XmlTextReader schemaReader = NULL;
//            try
//                {
//                schemaWriter = new XmlTextWriter (schemaStream, Encoding.UTF8);
//                DynamicXmlSerializer.Serialize (schemaWriter, originalSchema);
//                schemaWriter.Flush ();
//                schemaStream.Seek (0, SeekOrigin.Begin);
//                schemaReader = new XmlTextReader (schemaStream);
//                schemaCopy = DynamicXmlSerializer.Deserialize (schemaReader) as IECSchema;
//                }
//            finally
//                {
//                if (null != schemaWriter)
//                    schemaWriter.Close ();
//                if (null != schemaReader)
//                    schemaReader.Close ();
//                }
//            }
//        }
//    return schemaCopy;
//    }

private: void InitClassAndPropertyVariables
(
);

/*------------------------------------------------------------------------------------**/
/// <summary>Initializes the expected results.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public: void InitExpectedResultLists
(
);


/*------------------------------------------------------------------------------------**/
/// <summary>Creates a schema that uses koq hierarchy to get units.</summary>
/// <author>Colin.Kerr</author>                            <date>7/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
//protected IECSchema CreateSchemaWithKOQHierarchy
//(
//)
//    {
//    IECSchema koqHierarchySchema = new ECSchema ("koqHier", 4, 7, "koqh");
//    IECClass pipeClass = new ECClass ("PipeClass");
//    koqHierarchySchema.AddClass (pipeClass);
//    IECProperty roughness = new ECProperty ("Roughness", ECObjects.DoubleType);
//    pipeClass.Add (roughness);
//    IECProperty absoluteRoughness = new ECProperty ("AbsoluteRoughness", ECObjects.DoubleType);
//    pipeClass.Add (absoluteRoughness);
//    IECProperty manningsRoughness = new ECProperty ("ManningsRoughness", ECObjects.DoubleType);
//    pipeClass.Add (manningsRoughness);
//    IECProperty kuttersRoughness = new ECProperty ("KuttersRoughness", ECObjects.DoubleType);
//    pipeClass.Add (kuttersRoughness);
//    IECProperty pipeDepth = new ECProperty ("PipeDepth", ECObjects.DoubleType);
//    pipeClass.Add (pipeDepth);
//    IECProperty scaleFactor = new ECProperty ("ScaleFactor", ECObjects.DoubleType);
//    pipeClass.Add (scaleFactor);
//    IECProperty flowChange = new ECProperty ("FlowChange", ECObjects.DoubleType);
//    pipeClass.Add (flowChange);
//    IECProperty dimensionlessQuantity = new ECProperty ("MrNoDimension", ECObjects.DoubleType);
//    pipeClass.Add (dimensionlessQuantity);
//
//    List<UnitSpecification> unitSpecificationList = new List<UnitSpecification> ();
//    // Property Unit Specifications
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "Roughness", "ROUGHNESS"));
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "AbsoluteRoughness", "ROUGHNESS_ABSOLUTE"));
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "ManningsRoughness", "ROUGHNESS_MANNINGS"));
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "KuttersRoughness", "ROUGHNESS_KUTTERS"));
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "PipeDepth", "DEPTH"));
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "ScaleFactor", "SCALE"));
//    unitSpecificationList.Add (UnitSpecification.CreateProperty ("PipeClass", "FlowChange", "RELATIVE_FLOW_CHANGE"));
//    UnitSpecification mrNoUnitSpec = new UnitSpecification ();
//    mrNoUnitSpec.ClassName     = "PipeClass";
//    mrNoUnitSpec.PropertyName  = "MrNoDimension";
//    mrNoUnitSpec.DimensionName = "ONE";
//    unitSpecificationList.Add (mrNoUnitSpec);
//    // KOQ/Dimension UnitSpecifications
//    unitSpecificationList.Add (UnitSpecification.CreateKindOfQuantity ("ROUGHNESS", "MILLIMETRE"));
//    unitSpecificationList.Add (UnitSpecification.CreateKindOfQuantity ("ROUGHNESS_MANNINGS", "CENTIMETRE", new string[] {"CENTIMETRE", "INCH"}));
//    unitSpecificationList.Add (UnitSpecification.CreateKindOfQuantity ("ROUGHNESS_KUTTERS", "MILE"));
//    unitSpecificationList.Add (UnitSpecification.CreateKindOfQuantity ("DEPTH", "FOOT", new string[] {"FOOT", "MILLIMETRE"}));
//    unitSpecificationList.Add (UnitSpecification.CreateKindOfQuantity ("RATIO", "MILLIMETRE_PER_METRE"));
//    unitSpecificationList.Add (UnitSpecification.CreateKindOfQuantity ("RELATIVE_FLOW_CHANGE", "NONE"));
//    unitSpecificationList.Add (UnitSpecification.CreateDimension ("ONE", "DOLLAR", new string[] {"NONE", "DOLLAR", "PERCENT_PERCENT"}));
//
//    // Add UnitSpecifications to the schema
//    UnitsSchemaCreator.AddUnitSpecificationsToSchema (koqHierarchySchema, unitSpecificationList, true);
//
//    return koqHierarchySchema;
//    }
//#endregion
//
//#region Test Result Verification Methods
///*------------------------------------------------------------------------------------**/
///// <summary>Test.</summary>
///// <author>Colin.Kerr</author>                            <date>3/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
//public void VerifyUnitSpecification
//(
//UnitSpecification expectedUnitSpec,
//UnitSpecification actualUnitSpec
//)
//    {
//    Assert.IsNotNull (actualUnitSpec, "The UnitSpecification returned for '" + expectedUnitSpec.BuildUnitSpecificationKey () + "' is null");
//    Assert.IsTrue (expectedUnitSpec.Equals (actualUnitSpec), "The actual UnitSpecification for 'Bike.FrontWheelDiamter' did not match the expected.\nExpected:\n"
//                    + expectedUnitSpec.ToString ()
//                    + "\nActual:\n" + actualUnitSpec.ToString ());
//    }
//
///*------------------------------------------------------------------------------------**/
///// <summary>Test.</summary>
///// <author>Colin.Kerr</author>                            <date>3/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
//public void VerifyAllowableUnits
//(
//List<string> expectedAllowableUnitNames,
//List<Unit> allowableUnits,
//string       propertyName
//)
//    {
//    Assert.IsNotNull (allowableUnits, "The List of returned units is null.");
//
//    string expectedNames = "";
//    foreach (string expectedAllowableUnitName in expectedAllowableUnitNames)
//        expectedNames += expectedAllowableUnitName + "\t";
//    string actualNames = "";
//    foreach (Unit ecUnit in allowableUnits)
//        actualNames += ecUnit.Name + "\t";
//
//    Assert.AreEqual (expectedAllowableUnitNames.Count, allowableUnits.Count,
//        "\nThe wrong number of AllowableUnits were returned when getting them from the ECProperty: " + propertyName +
//        "\nExpected: " + expectedNames +
//        "\nActual:   " + actualNames);
//    foreach (Unit ecUnit in allowableUnits)
//        {
//        Assert.IsTrue (expectedAllowableUnitNames.Contains (ecUnit.Name),
//            "The Unit with Name: '" + ecUnit.Name + "' was found but was not expected." +
//            "\nExpected: " + expectedNames +
//            "\nActual:   " + actualNames);
//        }
//    } 
//
///*------------------------------------------------------------------------------------**/
///// <summary>Test.</summary>
///// <author>Colin.Kerr</author>                            <date>3/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
public: void VerifyDefaultUnit
(
Utf8String expectedUnitName,
Unit defaultUnit
);
//    {
//    Assert.IsNotNull (defaultUnit, "The returned Unit is null.");
//    Assert.AreEqual (expectedUnitName, defaultUnit.Name, "The expected Unit name does not match the actual name.");
//    }
//
///*------------------------------------------------------------------------------------**/
///// <summary>Test.</summary>
///// <author>Colin.Kerr</author>                            <date>3/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
//public void VerifyKindOfQuantity
//(
//string           expectedKindOfQuantityName,
//KindOfQuantity actualKindOfQuantity
//)
//    {
//    Assert.IsNotNull (actualKindOfQuantity, "The returned KindOfQuantity is null.");
//    Assert.AreEqual (expectedKindOfQuantityName, actualKindOfQuantity.Name, "The expected KindOfQuantity name does not match the actual name.");
//    }
//
///*------------------------------------------------------------------------------------**/
///// <summary>Compares schema before and after supplementation.</summary>
///// <author>Colin.Kerr</author>                            <date>7/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
//protected void AssertSchemasAreEqual
//(
//IECSchema schemaPreSupplement,
//IECSchema schemaPostSupplement
//)
//    {
//    Assert.IsTrue (schemaPreSupplement.IsSamePrimarySchema (schemaPostSupplement),
//        "The two schemas {0} and {1}, returned false to 'IsSamePrimarySchema' even though they should be equal.",
//        schemaPreSupplement.FullName, schemaPostSupplement.FullName);
//
//    // Test the custom attributes at the top level of the schema.
//    IList<IECInstance> preSupplementCustomAttributes  = schemaPreSupplement.GetCustomAttributeList ();
//    IList<IECInstance> postSupplementCustomAttributes = schemaPostSupplement.GetCustomAttributeList ();
//    CompareInstanceLists (preSupplementCustomAttributes, postSupplementCustomAttributes);
//
//    // Test that the classes are the same.
//    IECClass[] postSupplementClasses = schemaPostSupplement.GetClasses ();
//    foreach (IECClass preSupplementClass in schemaPreSupplement.GetClasses ())
//        {
//        foreach (IECClass postSupplementClass in postSupplementClasses)
//            {
//            if (preSupplementClass.Name == postSupplementClass.Name)
//                {
//                if (null != preSupplementClass.GetPrimaryCustomAttributeList ())
//                    foreach (IECInstance originalCustomAttribute in preSupplementClass.GetPrimaryCustomAttributeList ())
//                        {
//                        Assert.IsTrue (TestHelpers.HasSameValues (originalCustomAttribute, postSupplementClass.GetPrimaryCustomAttributes (originalCustomAttribute.ClassDefinition.Name)),
//                            "The Custom Attribute class: " + originalCustomAttribute.ClassDefinition.Name + " is not the same on the supplemented class as the original even though we are using 'GetPrimaryCustomAttributes'");
//                        }
//                Assert.IsTrue (ECClassHelper.AreEqual (preSupplementClass, postSupplementClass), "The classes with the name: " + preSupplementClass.Name + " should be equal but are not");
//                Assert.IsTrue (TestHelpers.AreIdentical (preSupplementClass, postSupplementClass), "The classes with the name: " + preSupplementClass.Name + " should be identical but are not");
//                break;
//                }
//            }
//        }
//    }
//
///*------------------------------------------------------------------------------------**/
///// <summary>Compares two lists of instances.</summary>
///// <author>Colin.Kerr</author>                            <date>7/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
//protected void CompareInstanceLists
//(
//IList<IECInstance> preSupplementInstances,
//IList<IECInstance> postSupplementInstances
//)
//    {
//    if (null == preSupplementInstances && null == postSupplementInstances)
//        return;
//    if (null == preSupplementInstances || null == postSupplementInstances)
//        Assert.Fail ("One of the Pre and Post supplement instance lists are null.");
//    if (preSupplementInstances.Count != postSupplementInstances.Count)
//        Assert.Fail ("The pre supplement instance list has {0} entries but the post supplement instance list has {1}.  They should be equal.",
//            preSupplementInstances.Count, postSupplementInstances.Count);
//
//    foreach (IECInstance preSupplementInstance in preSupplementInstances)
//        {
//        foreach (IECInstance postSupplementInstance in postSupplementInstances)
//            {
//            if (preSupplementInstance.ClassDefinition.Name == postSupplementInstance.ClassDefinition.Name)
//                {
//                Assert.IsTrue (TestHelpers.HasSameValues (preSupplementInstance, postSupplementInstance),
//                    "The Pre and Post supplement instances of class: {0}are different even though they should not have been modified.",
//                    preSupplementInstance.ClassDefinition.Name);
//                break;
//                }
//            }
//        }
//    }
//
///*------------------------------------------------------------------------------------**/
///// <summary>Compares classes from a primary and supplemented schema.</summary>
///// <author>Colin.Kerr</author>                            <date>7/2008</date>
///*--------------+---------------+---------------+---------------+---------------+------*/
//protected void CompareSupplementedAndOriginalClasses
//(
//)
//    {
//    IECClass[] supplementedClasses = m_supplementedSchema.GetClasses ();
//    foreach (IECClass originalClass in m_testSchema.GetClasses ())
//        {
//        foreach (IECClass supplementedClass in supplementedClasses)
//            {
//            if (originalClass.Name == supplementedClass.Name)
//                {
//                // Test that all the classes in the schemas are equal but not identical if they were supplemented.  If they were not supplemented then they should be identical
//                Assert.IsTrue (ECClassHelper.AreEqual (originalClass, supplementedClass), "The classes with the name: " + originalClass.Name + " should be equal but are not");
//                if (originalClass.Name != "ExceptionCauser" &&
//                    originalClass.Name != "StandardUnitsClass" &&
//                    originalClass.Name != "Hub" &&
//                    originalClass.Name != "DerivedClass" &&
//                    originalClass.Name != "DerivedClassOverride" &&
//                    originalClass.Name != "DerivedClassOverrideUnits")
//                    Assert.IsFalse (TestHelpers.AreIdentical (originalClass, supplementedClass), "The classes with the name: " + originalClass.Name + " should not be identical but are");
//                else if (originalClass.Name != "DerivedClass" &&
//                         originalClass.Name != "DerivedClassOverride" &&
//                         originalClass.Name != "DerivedClassOverrideUnits") // These classes are skiped because they are not supplemented and their base class comes from a referenced schema.
//                    Assert.IsTrue (TestHelpers.AreIdentical (originalClass, supplementedClass), "The classes with the name: " + originalClass.Name + " should be identical but are not");
//                // Test that the custom attributes from the class and property are the same when we get the primary values
//                if (null != originalClass.GetPrimaryCustomAttributeList ())
//                    foreach (IECInstance originalCustomAttribute in originalClass.GetPrimaryCustomAttributeList ())
//                        {
//                        Assert.IsTrue (TestHelpers.HasSameValues (originalCustomAttribute, supplementedClass.GetPrimaryCustomAttributes (originalCustomAttribute.ClassDefinition.Name)),
//                            "The Custom Attribute class: " + originalCustomAttribute.ClassDefinition.Name + " is not the same on the supplemented class as the original even though we are using 'GetPrimaryCustomAttributes'");
//                        }
//                break;
//                }
//            }
//        }
//    }
//
//#endregion
//}
};END_BENTLEY_ECN_TEST_NAMESPACE
