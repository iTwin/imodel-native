/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/UnitsTestBase.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "StopWatch.h"
#include "TestFixture.h"
#include "UnitsTestBase.h"

using namespace Bentley::ECN;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//UnitsTestBase::UnitsTestBase()
//{}
void UnitsTestBase::SetUp
(
)
    {
    m_testSchema = NULL;
    m_supplementedSchema = NULL;

    bvector<ECSchemaP> supplementalSchemas;
    InitializeUnits (L"testschema",supplementalSchemas);

    // Test that test schemas are not null
     ASSERT_TRUE(m_testSchema!=NULL)<<"Test setup failure: Domain Schema not loaded";
     ASSERT_TRUE (supplementalSchemas.size()!=0)<< "Test setup failure: Supplemental Schema not loaded";
    //m_testSchema2 = CreateSchemaWithKOQHierarchy ();
    //Assert.IsNotNull (m_testSchema2, "Test setup failure: Secondary Domain Schema not loaded");
    //m_referencedSchema = ECObjects.LocateSchema ("testReferencedSchema.01.00", SchemaMatchType.Exact, NULL, NULL);
    //Assert.IsNotNull (m_referencedSchema, "Test setup failure: Schema Referenced by domain schema not loaded.");

    //DuplicateAndModifyTestSupplementalSchema (m_supplementalSchemas);

    // Build Supplemented Schema
    SupplementedSchemaBuilder supplementedSchemaBuilder;// = new SupplementedSchemaBuilder ();
#ifdef COPY_SCHEMA_IS_BROKEN
	m_testSchema->CopySchema (m_supplementedSchema);
	m_supplementedSchema->IsSupplemented();
#endif


    supplementedSchemaBuilder.UpdateSchema (*m_supplementedSchema, supplementalSchemas);
	m_supplementedSchema->IsSupplemented();
   // m_supplementedSchema = supplementedSchemaBuilder.SupplementedSchema;
   // Assert.IsNotNull (m_supplementedSchema, "Test setup Failure: The Supplemented Schema is null");
    //// Build UnitsManager
    //m_manager = new ECSchemaUnitsManager (m_supplementedSchema);

    //// SetUp Class and property variables
    InitClassAndPropertyVariables ();

    InitExpectedResultLists ();

//#define DEBUGGING_SUPPLEMENTED_SCHEMA
#ifdef DEBUGGING_SUPPLEMENTED_SCHEMA
    static bool s_once = false;
    if (!s_once)
        {
        s_once = true;

        WString schemaXml;
        m_supplementedSchema->WriteToXmlString (schemaXml);
        wprintf (schemaXml.c_str());
        }
#endif
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Initializes the units framework for test.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
void UnitsTestBase::InitializeUnits
(
WString testSchemaName,
bvector< ECSchemaP > & testSupplementalSchemas
)
    {

    if (0 == m_supplementalSchemas.size())
        {
	bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    SearchPathSchemaFileLocaterPtr schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
	ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
	schemaContext->AddSchemaLocater (*schemaLocater);
	SchemaKey key(testSchemaName.c_str(), 01, 00);
    
    m_testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);//ECSchema::LocateSchema(key, *schemaContext);

    EXPECT_TRUE(m_testSchema.IsValid());
    EXPECT_FALSE(m_testSchema->ShouldNotBeStored());
	ECClassP ecClass=m_testSchema->GetClassP(L"Bike");
	ASSERT_TRUE (NULL != ecClass);
	//ECSchema::ReadFromXmlFile (testSchema, ECTestFixture::GetTestDataPath(testSchemaName.c_str()).c_str(), *schemaContext);
    //testSchema = ECObjects.LocateSchema (testSchemaName, SchemaMatchType.Exact, NULL, NULL);


    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    m_supplementedSchema = schemaContext->LocateSchema (key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE (m_supplementedSchema.IsValid());
    EXPECT_FALSE (m_supplementedSchema.get() == m_testSchema.get());

  //   Load Supplemental Schema
    //testSupplementalSchemas = new bvector< ECSchemaP > ();
    SchemaKey suppKey (L"TestSupplementalSchema", 1, 0),
              defKey  (L"TestUnitDefaults", 1, 0),
              widthKey (L"WidthDefaults", 1, 0);

    m_supplementalSchemas.push_back ((schemaContext->LocateSchema(suppKey, SCHEMAMATCHTYPE_Latest)).get());
    m_supplementalSchemas.push_back ((schemaContext->LocateSchema(defKey, SCHEMAMATCHTYPE_Latest)).get());
    m_supplementalSchemas.push_back ((schemaContext->LocateSchema(widthKey, SCHEMAMATCHTYPE_Latest)).get());
    }

    for (size_t i = 0; i < m_supplementalSchemas.size(); ++i)
        testSupplementalSchemas.push_back (m_supplementalSchemas[i].get());
    }

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

void UnitsTestBase::InitClassAndPropertyVariables
(
)
    {
     m_wheelClass                = m_supplementedSchema->GetClassP(L"Wheel");
    //m_diameterProp              = m_wheelClass["Diameter"];
    //m_radiusProp                = m_wheelClass["Radius"];
     m_spokeLengthProp           = m_wheelClass->GetPropertyP (L"SpokeLength");
    //m_distanceTraveledProp      = m_wheelClass["DistanceTraveled"];
     m_AreaProp                  = m_wheelClass->GetPropertyP (L"Area");
    //m_widthProp                 = m_wheelClass["Width"];
    //m_quantityProp              = m_wheelClass["Quantity"];
    //m_weightProp                = m_wheelClass["Weight"];
    //m_hubStructProp             = m_wheelClass->GetPropertyP(L"WheelHub");

    m_wheelsChildClass          = m_supplementedSchema->GetClassP(L"WheelsChild");
    m_wcDiameterProp            = m_wheelsChildClass->GetPropertyP(L"Diameter");
    m_wcWeightProp              = m_wheelsChildClass->GetPropertyP (L"Weight");

    m_BikeClass                 = m_supplementedSchema->GetClassP (L"Bike");
	
    m_frontWheelDiameterProp    = m_BikeClass->GetPropertyP (L"FrontWheelDiameter");
    m_frontWheelPressureProp    = m_BikeClass->GetPropertyP (L"FrontWheelPressure");
    m_rearWheelDiameterProp     = m_BikeClass->GetPropertyP (L"RearWheelDiameter");
    m_rearWheelPressureProp     = m_BikeClass->GetPropertyP (L"RearWheelPressure");
    m_trainingWheelDiameterProp = m_BikeClass->GetPropertyP (L"TrainingWheelDiameter");
    m_frameHeightProp           = m_BikeClass->GetPropertyP (L"FrameHeight");
    m_headSetAngleProp          = m_BikeClass->GetPropertyP (L"HeadSetAngle");
    m_seatPostAngleProp         = m_BikeClass->GetPropertyP (L"SeatPostAngle");

   // m_ExceptionClass = m_supplementedSchema["ExceptionCauser"];

    m_standardUnitsClass        = m_supplementedSchema->GetClassP(L"StandardUnitsClass");
    m_sucAreaProp               = m_standardUnitsClass->GetPropertyP (L"Area");
    m_sucVolumeProp             = m_standardUnitsClass->GetPropertyP (L"Volume");
    m_sucTemperatureProp        = m_standardUnitsClass->GetPropertyP (L"Temperature");
    m_sucWidthProp              = m_standardUnitsClass->GetPropertyP (L"Width");
/*
    m_pipeClass                 = m_testSchema2["PipeClass"];
    m_roughnessProp             = m_pipeClass["Roughness"];
    m_absRoughnessProp          = m_pipeClass["AbsoluteRoughness"];
    m_manningsRoughnessProp     = m_pipeClass["ManningsRoughness"];
    m_kuttersRoughnessProp      = m_pipeClass["KuttersRoughness"];
    m_pipeDepthProp             = m_pipeClass["PipeDepth"];
    m_scaleFactorProp           = m_pipeClass["ScaleFactor"];
    m_flowChangeProp            = m_pipeClass["FlowChange"];
    m_mrNoDimensionProp         = m_pipeClass["MrNoDimension"];

    m_referencedClass           = m_referencedSchema["ReferencedClass"];
    m_classLengthReferenced     = m_referencedClass["ClassLength"];
    m_derivedClass              = m_supplementedSchema["DerivedClass"];
    m_classLengthDerived        = m_derivedClass["ClassLength"];
    m_derivedClassOverrideProp  = m_supplementedSchema["DerivedClassOverride"];
    m_classLengthOverrideProp   = m_derivedClassOverrideProp["ClassLength"];
    m_derivedClassOverrideUnit  = m_supplementedSchema["DerivedClassOverrideUnits"];
    m_classLengthOverrideUnit   = m_derivedClassOverrideUnit["ClassLength"];*/
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Initializes the expected results.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
void UnitsTestBase::InitExpectedResultLists
(
)
    {
    /*m_expectedResults_ForWheelClass = new List<string> (new string[] {"CENTIMETRE", "DECIMETRE", "FOOT", "INCH",
    "KILOMETRE", "METRE", "MILLIFOOT", "MILE", "MILLIMETRE", "YARD", "MICROINCH", "MICROMETRE", "MILLIINCH", "UOR",
    "US_SURVEY_FOOT", "US_SURVEY_INCH", "US_SURVEY_MILE"});

    m_expectedResults_ForRadiusProperty = new List<string> (new string[] {"CENTIMETRE", "DECIMETRE", "METRE", "MILLIMETRE"});

    m_expectedResults_ForSpokeLengthProperty = new List<string> (new string[] { "CENTIMETRE", "MILLIMETRE" });

    m_expectedResults_ForArea = new List<string> (new string[] {"CENTIMETRE_SQUARED", "MILLIMETRE_SQUARED"});

    m_expectedResults_ForDiameter = new List<string> (new string[] {"CENTIMETRE", "DECIMETRE", "FOOT", "INCH"});

    m_expectedResults_ForDiameterLarge = new List<string> (new string[] { "CENTIMETRE", "MILLIMETRE" });

    m_expectedResults_ForWidth = new List<string> (new string[] {"CENTIMETRE", "MILLIMETRE", "INCH"});

    m_expectedResults_ForFrontWheelProperty = new List<string> (new string[] {"CENTIMETRE", "FOOT", "INCH"});

    m_expectedResults_ForRearWheelProperty = new List<string> (new string[] {"FOOT", "INCH"});

    m_expectedResults_ForTrainingWheelProperty = new List<string> (new string[] {"CENTIMETRE", "INCH"});

    m_expectedResults_ForFrameHeightProperty = m_expectedResults_ForWheelClass;

    m_expectedResults_ForDepthKOQ_Pipe = new List<string> (new string[] {"FOOT", "MILLIMETRE"});

    m_expectedResults_ForManningsRoughnessKOQ_Pipe = m_expectedResults_ForTrainingWheelProperty;

    m_expectedResults_ForClassLengthProperty = new List<string> (new string[] {"DECIMETRE", "METRE", "MILLIINCH"});*/
    }


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
/*------------------------------------------------------------------------------------**/
/// <summary>Test.</summary>
/// <author>Colin.Kerr</author>                            <date>3/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
void UnitsTestBase::VerifyDefaultUnit
(
WString expectedUnitName,
Unit defaultUnit
)
    {
   ASSERT_TRUE (expectedUnitName.Equals (defaultUnit.GetName())) << "Expected " << expectedUnitName.c_str() << " Actual " << defaultUnit.GetName();
    }
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
END_BENTLEY_ECOBJECT_NAMESPACE
