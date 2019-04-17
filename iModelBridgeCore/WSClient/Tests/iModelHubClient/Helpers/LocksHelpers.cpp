/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "LocksHelpers.h"
#include <DgnPlatform/DgnCoreAPI.h>
#include <Bentley/BeTest.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock(DgnModelCR model, LockLevel level)
    {
    LockRequest req;
    req.Insert(model, level);
    return model.GetDgnDb().BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 08/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock(DgnDbR db, DgnModelCR model, LockLevel level)
    {
    LockRequest req;
    req.Insert(model, level);
    return db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock(DgnDbR db, LockLevel level)
    {
    LockRequest req;
    req.Insert(db, level);
    return db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock(DgnDbR db, DgnElementCR element, LockLevel level)
    {
    LockRequest req;
    req.Insert(element, level);
    return db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus DemoteLock(DgnModelCR model, LockLevel level)
    {
    DgnLockSet toRelease;
    toRelease.insert(DgnLock(LockableId(model.GetModelId()), level));
    toRelease.insert(DgnLock(LockableId(model.GetModeledElementId()), level));
    return model.GetDgnDb().BriefcaseManager().DemoteLocks(toRelease);
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
