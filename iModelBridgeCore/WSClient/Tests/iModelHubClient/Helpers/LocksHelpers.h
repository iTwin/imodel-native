/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/LocksHelpers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
RepositoryStatus AcquireLock(DgnModelCR model, LockLevel level);
RepositoryStatus AcquireLock(DgnDbR db, DgnModelCR model, LockLevel level);
RepositoryStatus AcquireLock(DgnDbR db, LockLevel level = LockLevel::Exclusive);
RepositoryStatus AcquireLock(DgnDbR db, DgnElementCR element, LockLevel level = LockLevel::Exclusive);
RepositoryStatus DemoteLock(DgnModelCR model, LockLevel level = LockLevel::None);
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
