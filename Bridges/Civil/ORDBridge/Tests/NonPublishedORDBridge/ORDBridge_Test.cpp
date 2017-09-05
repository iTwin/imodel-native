#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldGeometryConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Geometry.dgn", "ORDGeometryTest.bim"));
    VerifyConvertedElements("ORDGeometryTest.bim", 1, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldCorridorConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Corridor.dgn", "ORDCorridorTest.bim"));
    VerifyConvertedElements("ORDCorridorTest.bim", 16, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDFullyFederatedConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Fully Federated\\container 2.dgn", "ORDFullyFederatedTest.bim"));
    VerifyConvertedElements("ORDFullyFederatedTest.bim", 2, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIHLConversionTest)
    {
    ASSERT_TRUE(RunTestApp("IHL.dgn", "ORDIHLTest.bim"));
    VerifyConvertedElements("ORDIHLTest.bim", 11, 0);
    }
