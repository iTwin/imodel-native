#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldGeometryConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Geometry.dgn", "ORDGeometryTest.bim", false));
    //VerifyConvertedElements("ORDGeometryTest.bim", 1, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldCorridorConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Corridor.dgn", "ORDCorridorTest.bim", false));
    //VerifyConvertedElements("ORDCorridorTest.bim", 1, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDFullyFederatedConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Fully Federated\\container 2.dgn", "ORDFullyFederatedTest.bim", false));
    //VerifyConvertedElements("ORDFullyFederatedTest.bim", 2, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIHLConversionTest)
    {
    ASSERT_TRUE(RunTestApp("IHL.dgn", "ORDIHLTest.bim", false));
    //VerifyConvertedElements("ORDIHLTest.bim", 11, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIntersectionWithEditsTest)
    {
    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Original.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp("NewIntersection.dgn", "ORDIntersectionTest.bim", false));
    //VerifyConvertedElements("ORDIntersectionTest.bim", 1, 1);

    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Edits.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp("NewIntersection.dgn", "ORDIntersectionTest.bim", true));
    //VerifyConvertedElements("ORDIntersectionTest.bim", 9, 4);
    }
