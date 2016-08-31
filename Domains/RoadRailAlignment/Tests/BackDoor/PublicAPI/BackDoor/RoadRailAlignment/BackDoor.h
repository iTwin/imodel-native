#pragma once

#include "RoadRailAlignmentTests.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE

//========================================================================================
//! Helper functions that published (and non-published) tests can use to check on results.
//! They are in a namespace called "backdoor" to emphasize the fact that they allow published
//! tests to call functions that are not part of the published api.
// @bsiclass                                                 
//+===============+===============+===============+===============+===============+======
namespace Backdoor
    {
    namespace RoadRailAlignment
        {
        }
    }

END_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE
