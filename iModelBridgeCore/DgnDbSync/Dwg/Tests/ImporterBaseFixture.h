/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterBaseFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ImporterTests.h"
#include <DgnDbSync/Dwg/DwgImporter.h>

USING_NAMESPACE_DGNDBSYNC_DWG

static bool s_initialized;

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                      04/15
//----------------------------------------------------------------------------------------
struct ImporterTestBaseFixture : public ImporterTests
{

    bool m_wantCleanUp = true;
public:
    ImporterTestsHost m_host;
    DwgImporter::Options   m_options;
    NopProgressMeter m_meter;
    BentleyApi::BeFileName m_dwgFileName;
    BentleyApi::BeFileName m_dgnDbFileName;

    virtual void SetUp();
    virtual void TearDown();

    void LineUpFiles(BentleyApi::WCharCP outputDgnDbFileName, BentleyApi::WCharCP inputV8FileName, bool doConvert);
    void MakeCopyOfFile(BentleyApi::BeFileNameR refV8File, BentleyApi::WCharCP suffix);

    void DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input);
    void DoUpdate(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input, bool expectFailure = false);
};

