#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldGeometryConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"Geometry.dgn"), WCharCP(L"ORDGeometryTest.bim"), false));
    VerifyConvertedElements("ORDGeometryTest.bim", 1, 0);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldCorridorConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"Corridor.dgn"), WCharCP(L"ORDCorridorTest.bim"), false));
    VerifyConvertedElements("ORDCorridorTest.bim", 1, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDFullyFederatedConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"Fully Federated\\container 2.dgn"), WCharCP(L"ORDFullyFederatedTest.bim"), false));
    VerifyConvertedElements("ORDFullyFederatedTest.bim", 2, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIHLConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"IHL.dgn"), WCharCP(L"ORDIHLTest.bim"), false));
    VerifyConvertedElements("ORDIHLTest.bim", 11, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIntersectionWithEditsTest)
    {
    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Original.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp(WCharCP(L"NewIntersection.dgn"), WCharCP(L"ORDIntersectionTest.bim"), false));
    VerifyConvertedElements("ORDIntersectionTest.bim", 1, 1);

    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Edits.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp(WCharCP(L"NewIntersection.dgn"), WCharCP(L"ORDIntersectionTest.bim"), true));
    VerifyConvertedElements("ORDIntersectionTest.bim", 9, 4);
    }
