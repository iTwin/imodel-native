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

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Format"))

TEST(ECQuantityFormattingTests, Preliminary)
    {
    LOG.infov("================  Quantity Formatting Log ===========================");



    LOG.infov("================  End of Quantity Formatting Log  ===========================");
    }

END_BENTLEY_ECOBJECT_NAMESPACE