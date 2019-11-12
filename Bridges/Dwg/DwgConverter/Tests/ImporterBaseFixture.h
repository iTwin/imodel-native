/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ImporterTests.h"
#include <Dwg/DwgImporter.h>

USING_NAMESPACE_DWG

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                      04/15
//----------------------------------------------------------------------------------------
struct ImporterTestBaseFixture : public ImporterTests
{
private:
    bool        m_wantCleanUp = true;
    uint32_t    m_count = 0;
    double      m_scaleDwgToMeters = 1.0;

protected:
    void DoConvert(DwgImporter* importer, BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR inputFileName);
    void DoUpdate(DwgImporter* importer, BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR inputFileName);

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
    void InitializeImporterOptions (BentleyApi::BeFileNameCR dwgFilename, bool isUpdating);
    uint32_t GetCount() const;
    double GetScaleDwgToMeters() const;
};

