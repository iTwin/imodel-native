//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTExportProgressIndicator
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <ImagePP/all/h/HFCProgressIndicator.h>
#include <ImagePP/all/h/HFCMacros.h>

#include "HRFRasterFile.h"
//----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HUTExportProgressIndicator : public HFCProgressIndicator
    {
public:

    IMAGEPP_EXPORT virtual void StopIteration();

    void         SetExportedFile(HRFRasterFile*  pi_pRasterFile);
    bool        ContinueIteration(HRFRasterFile* pi_pWrittenRasterFile,
                                   uint32_t      pi_ResIndex,
                                   bool          pi_BlockSentToPool = false);
    uint32_t    GetNbExportedBlocks(uint32_t pi_ResIndex);

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HUTExportProgressIndicator)

    // Disabled methodes
    HUTExportProgressIndicator();

    HRFRasterFile*    m_pExportedFile;
    uint32_t         m_FirstResBlockSentToPool;
    uint32_t         m_FirstResBlockAlreadyCount;
    vector<uint32_t>   m_NbBlocksPerRes; //Number of block already written for each resolution
    };
END_IMAGEPP_NAMESPACE

//----------------------------------------------------------------------------
#include "HUTExportProgressIndicator.hpp"


