/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    namespace BentleyGeometryFlatBuffer
        {
        IGeometryPtr BytesToGeometrySafe(Byte const*, size_t bufferSize);
        void GeometryToBytes(bvector<Byte>&, IGeometryCR);
        }
    };

END_ECDBUNITTESTS_NAMESPACE