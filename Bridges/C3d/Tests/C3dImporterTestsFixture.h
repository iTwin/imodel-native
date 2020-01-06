/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "C3dImporterTests.h"

USING_NAMESPACE_C3D

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                      04/15
//----------------------------------------------------------------------------------------
struct C3dImporterTestsFixture : public C3dImporterTests
{
private:
    bool        m_wantCleanUp = true;
    uint32_t    m_count = 0;
    double      m_scaleDwgToMeters = 1.0;

protected:
    void DoConvert(C3dImporter* importer, BeFileNameCR output, BeFileNameCR inputFileName);
    void DoUpdate(C3dImporter* importer, BeFileNameCR output, BeFileNameCR inputFileName);

public:
    DwgImporter::Options   m_options;
    BeFileName m_dwgFileName;
    BeFileName m_dgnDbFileName;
    BeFileName m_seedDgnDbFileName;

    virtual void SetUp();
    virtual void TearDown();

    void LineUpFiles(WCharCP outputDgnDbFileName, WCharCP inputDwgFileName, bool doConvert);
    void DoConvert(BeFileNameCR output, BeFileNameCR input);
    void DoUpdate(BeFileNameCR output, BeFileNameCR input, bool expectFailure = false);
    void SetUp_CreateNewDgnDb(); // look for name of bim to create in m_seedDgnDbFileName
    void LineUpFilesForNewDwg(WCharCP outputDgnDbFileName, WCharCP inputDwgFileName);
    void InitializeImporterOptions (BeFileNameCR dwgFilename, bool isUpdating);
    uint32_t GetCount() const;
    double GetScaleDwgToMeters() const;
};
