/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/BackDoor/PublicAPI/BackDoor/ECDb/Backdoor.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <UnitTests/BackDoor/ECDb/ECDbTests.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//========================================================================================
//! Helper functions that published (and non-published) tests can use to check on results.
//! They are in a namespace called "backdoor" to emphasize the fact that they allow published
//! tests to call functions that are not part of the published api.
// @bsiclass                                                 
//+===============+===============+===============+===============+===============+======
namespace Backdoor
    {
    namespace ECDb
        {
        void* GetSqliteDb (BeSQLite::EC::ECDbCR ecdb);

        namespace ECSqlStatement
            {
            BeSQLite::EC::ECSqlStatus BindDateTime (BeSQLite::EC::ECSqlStatement& stmt, int parameterIndex, uint64_t julianDayTicksHns, DateTime::Info const* metadata);
            }
        }

    namespace ECObjects
        {
        namespace ECValue
            {
            void SetAllowsPointersIntoInstanceMemory (ECN::ECValueR value, bool allow);
            bool AllowsPointersIntoInstanceMemory (ECN::ECValueCR value);
            }
        }

    };

END_ECDBUNITTESTS_NAMESPACE