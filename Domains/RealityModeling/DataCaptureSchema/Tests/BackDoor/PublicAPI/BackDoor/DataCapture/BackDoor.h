/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
