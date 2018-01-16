/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/DgnPlatformHelpers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
Dgn::PhysicalPartitionPtr CreateModeledElement(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnElementCPtr CreateAndInsertModeledElement(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnCategoryId CreateCategory(Utf8CP name, Dgn::DgnDbR db);
Dgn::PhysicalModelPtr CreateModel(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnElementCPtr CreateElement(Dgn::DgnModelR model, bool acquireLocks);
Dgn::DgnCode MakeStyleCode(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnDbStatus InsertStyle(Utf8CP name, Dgn::DgnDbR db, bool expectSuccess = true);
void OpenDgnDb(Dgn::DgnDbPtr& db, BeFileNameCR path);
void OpenReadOnlyDgnDb(Dgn::DgnDbPtr& db, BeFileNameCR path);
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
