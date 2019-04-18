/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "checkers.h"
#include <Bentley/BeTest.h>


//This is a sample TestFixture Class which is creating a .dgnjs file upon calling SetUp()
//and deleting that file upon calling TearDown() for each test in a test case
//This class can be inherited and overriden.

/*================================================================================**//**
* @bsiclass                                                   Farhad.Kabir      01/17
+===============+===============+===============+===============+===============+======*/
class GeomFixture :public testing::Test
    {
    public:
        CharCP nameFT;  // pass this as an argument to ClearGeometry, for creating .dgnjs file

        void SetUp()
            {
            bvector<Utf8CP> fixtureTestName = { TEST_FIXTURE_NAME, TEST_NAME };
            Utf8CP delimiter = "_";
            Utf8String fixtureTName = BeStringUtilities::Join(fixtureTestName, delimiter);
            nameFT = fixtureTName.c_str();
            Check::SetUp();
            }
        void TearDown()
            {
            Check::TearDown();
            }       
    };