/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "BimUpgraderTestFixture.h"

BEGIN_BIM_UPGRADER_TEST_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BeFileName BimUpgraderTestFixture::GetOutputDir()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetOutputRoot(filepath);
    filepath.AppendToPath(L"Output");
    return filepath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BeFileName BimUpgraderTestFixture::GetOutputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BeFileName BimUpgraderTestFixture::GetOutputFileName(BentleyApi::Utf8CP filename)
    {
    WString wFileName(filename, BentleyCharEncoding::Utf8);
    return GetOutputFileName(wFileName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbPtr BimUpgraderTestFixture::OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode)
    {
    BentleyApi::BeSQLite::DbResult fileStatus;

    DgnDb::OpenParams openParams(mode);

    DgnDbPtr ret = DgnDb::OpenDgnDb(&fileStatus, projectName, openParams);
    return ret;
    }
END_BIM_UPGRADER_TEST_NAMESPACE
