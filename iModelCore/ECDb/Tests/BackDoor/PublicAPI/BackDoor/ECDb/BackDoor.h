/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"
#include <Geom/GeomApi.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//========================================================================================
//! Helper functions that published (and non-published) tests can use to check on results.
//! They are in a namespace called "backdoor" to emphasize the fact that they allow published
//! tests to call functions that are not part of the published api.
// @bsiclass                                                 
//+===============+===============+===============+===============+===============+======
namespace BackDoor
    {
    namespace ECObjects
        {
        namespace ECValue
            {
            void SetAllowsPointersIntoInstanceMemory (ECN::ECValueR, bool allow);
            bool AllowsPointersIntoInstanceMemory (ECN::ECValueCR);
            }

        namespace ECSchemaReadContext
            {
            ECN::ECObjectsStatus AddSchema(ECN::ECSchemaReadContext&, ECN::ECSchemaR);
            }
        }

    namespace IGeometryFlatBuffer
        {
        IGeometryPtr BytesToGeometry(bvector <Byte> &buffer);
        }
    };

END_ECDBUNITTESTS_NAMESPACE