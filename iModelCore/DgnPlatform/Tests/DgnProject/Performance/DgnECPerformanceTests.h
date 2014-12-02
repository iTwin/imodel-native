/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/DgnECPerformanceTests.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PerformanceTestFixture.h"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

struct PerformanceDgnECTests : public PerformanceTestFixture
    {
    private:
        static double s_xCoord;
        static double s_yCoord;
        static double s_zCoord;

        static double s_increment;

        StatusInt CreateArbitraryElement (EditElementHandleR editElementHandle, DgnModelR model);

    protected:
        PerformanceDgnECTests() : PerformanceTestFixture() {} 
        void RunInsertTests(ECN::ECSchemaR schema, DgnDbTestDgnManager tdm);
        void RunQueryTests(ECN::ECSchemaR schema, DgnDbTestDgnManager tdm);
    };

END_DGNDB_UNIT_TESTS_NAMESPACE
