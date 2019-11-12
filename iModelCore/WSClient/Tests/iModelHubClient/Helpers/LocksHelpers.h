/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
