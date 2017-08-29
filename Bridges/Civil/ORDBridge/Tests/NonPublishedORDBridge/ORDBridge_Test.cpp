#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldGeometryConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Geometry.dgn", "default", "ORDGeometryTest.bim"));
    VerifyConvertedElements("ORDGeometryTest.bim", 1, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldCorridorConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Corridor.dgn", "default", "ORDCorridorTest.bim"));
    VerifyConvertedElements("ORDCorridorTest.bim", 16, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDFullyFederatedConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Fully Federated\\container 2.dgn", "default", "ORDFullyFederatedTest.bim"));
    VerifyConvertedElements("ORDFullyFederatedTest.bim", 4, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIHLConversionTest)
    {
    ASSERT_TRUE(RunTestApp("IHL.dgn", "default", "ORDIHLTest.bim"));
    VerifyConvertedElements("ORDIHLTest.bim", 44, 0);
    }
