/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECQuantityFormattingTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
//#include <Formatting/FormattingApi.h>
//#include <ECObjects/ECSchema.h>
#include <ECObjects/ECQuantityFormatting.h>

namespace BEU = BentleyApi::Units;
namespace BEF = BentleyApi::Formatting;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             David.Fox-Rabinovitz                    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECQuantityFormattingTests, Preliminary)
    {
    LOG.infov("================  Quantity Formatting Log ===========================");



    LOG.infov("================  End of Quantity Formatting Log  ===========================");
    }

END_BENTLEY_ECN_TEST_NAMESPACE