/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECInstanceDeleteTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


//---------------------------------------------------------------------------------------
// @bsiClass                                       Maha Nasir                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECInstanceDeleteTestsFixture : public ECDbTestFixture
    {
    public:
        ECDbTestProject m_testproject;
        void CopyDgnDb ();
    };

END_ECDBUNITTESTS_NAMESPACE