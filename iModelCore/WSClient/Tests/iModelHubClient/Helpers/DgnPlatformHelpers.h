/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
Dgn::PhysicalPartitionPtr CreateModeledElement(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnElementCPtr CreateAndInsertModeledElement(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnCategoryId CreateCategory(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnCategoryId GetOrCreateCategory(Dgn::DgnDbR db, Utf8CP categoryName = nullptr);
Dgn::PhysicalModelPtr CreateModel(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnElementCPtr CreateElement(Dgn::DgnModelR model, Dgn::DgnCodeCR code = Dgn::DgnCode(), bool acquireLocks = true, Utf8CP categoryName = nullptr);
Dgn::DgnCode MakeStyleCode(Utf8CP name, Dgn::DgnDbR db);
Dgn::DgnDbStatus InsertStyle(Utf8CP name, Dgn::DgnDbR db, bool expectSuccess = true);
void InsertSpatialView(Dgn::SpatialModelR model, Utf8CP name, bool isPrivate = false);
void OpenDgnDb(Dgn::DgnDbPtr& db, BeFileNameCR path);
void OpenReadOnlyDgnDb(Dgn::DgnDbPtr& db, BeFileNameCR path);
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
