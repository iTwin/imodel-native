/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterBaseFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ImporterTests.h"
#include <DgnDbSync/Dwg/DwgImporter.h>

USING_NAMESPACE_DGNDBSYNC_DWG

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                      04/15
//----------------------------------------------------------------------------------------
struct ImporterTestBaseFixture : public ImporterTests
{
private:
    bool        m_wantCleanUp = true;
    uint32_t    m_count = 0;
    double      m_scaleDwgToMeters = 1.0;

public:
    DwgImporter::Options   m_options;
    BentleyApi::BeFileName m_dwgFileName;
    BentleyApi::BeFileName m_dgnDbFileName;
    BentleyApi::BeFileName m_seedDgnDbFileName;

    virtual void SetUp();
    virtual void TearDown();

    void LineUpFiles(BentleyApi::WCharCP outputDgnDbFileName, BentleyApi::WCharCP inputDwgFileName, bool doConvert);
    void DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input);
    void DoUpdate(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input, bool expectFailure = false);
    void SetUp_CreateNewDgnDb(); // look for name of bim to create in m_seedDgnDbFileName
    void LineUpFilesForNewDwg(WCharCP outputDgnDbFileName, WCharCP inputDwgFileName);
    uint32_t GetCount() const;
    double GetScaleDwgToMeters() const;
};

