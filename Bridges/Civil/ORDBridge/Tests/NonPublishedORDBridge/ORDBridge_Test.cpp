#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CivilSharedORDBridgeTests, ORDHelloWorldGeometryConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Hello World\\Geometry.dgn", "default", "ORDHelloWorldGeometryTest.bim"));
    VerifyConvertedElements("ORDHelloWorldGeometryTest.bim", 1, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CivilSharedORDBridgeTests, ORDHelloWorldCorridorConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Hello World\\Corridor.dgn", "default", "ORDHelloWorldCorridorTest.bim"));
    VerifyConvertedElements("ORDHelloWorldCorridorTest.bim", 16, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CivilSharedORDBridgeTests, ORDFullyFederatedConversionTest)
    {
    ASSERT_TRUE(RunTestApp("Fully Federated\\container.dgn", "default", "ORDFullyFederatedTest.bim"));
    VerifyConvertedElements("ORDFullyFederatedTest.bim", 2, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CivilSharedORDBridgeTests, ORDIHLConversionTest)
    {
    ASSERT_TRUE(RunTestApp("IHL\\IHL.dgn", "default", "ORDIHLTest.bim"));
    VerifyConvertedElements("ORDFullyFederatedTest.bim", 2, 0);
    }
