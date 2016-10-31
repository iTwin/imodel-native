/*--------------------------------------------------------------------------------------+
|
|  $Source: DataCaptureSchema/Tests/BackDoor/PublicAPI/BackDoor/DataCapture/BackDoor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DataCaptureTests.h"
#include "DataCaptureTestDomain.h"
#include "DataCaptureTestFixture.h"

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

//========================================================================================
//! Helper functions that published (and non-published) tests can use to check on results.
//! They are in a namespace called "backdoor" to emphasize the fact that they allow published
//! tests to call functions that are not part of the published api.
// @bsiclass                                                 
//+===============+===============+===============+===============+===============+======
namespace Backdoor
    {
    namespace DataCapture
        {
        }
    }

END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE
