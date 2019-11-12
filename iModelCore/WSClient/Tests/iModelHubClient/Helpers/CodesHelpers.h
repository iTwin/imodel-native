/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <DgnPlatform/DgnDb.h>

USING_NAMESPACE_BENTLEY_DGN
#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
void ExpectCodesState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager);
void ExpectCodeState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager);
void ExpectNoCodesWithState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager);
void ExpectNoCodeWithState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager);
DgnCodeInfo CreateCodeAvailable(DgnCodeCR code);
DgnCodeInfo CreateCodeDiscarded(DgnCodeCR code, Utf8StringCR changeSetId);
DgnCodeInfo CreateCodeReserved(DgnCodeCR code, DgnDbR db);
DgnCodeInfo CreateCodeUsed(DgnCodeCR code, Utf8StringCR changeSetId);
DgnCode CreateElementCode(DgnDbR db, Utf8StringCR name, Utf8CP nameSpace = nullptr);
DgnCode MakeModelCode(Utf8CP name, DgnDbR db);
int GetCodesCount(DgnDbR db);
Utf8String GetScopeString(CodeSpecPtr codeSpec, DgnElementCR scopeElement);
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE