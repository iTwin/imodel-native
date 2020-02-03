//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Class : HUTExportProgressIndicator
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HUTExportProgressIndicator.h>

#include <ImagePP/all/h/HRAUpdateSubResProgressIndicator.h>
#include <ImagePP/all/h/HRADrawProgressIndicator.h>

//----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HUTExportProgressIndicator)

//----------------------------------------------------------------------------

HUTExportProgressIndicator::HUTExportProgressIndicator()
    : HFCProgressIndicator(),
      m_pExportedFile(0),
      m_FirstResBlockSentToPool(0),
      m_FirstResBlockAlreadyCount(0)
    {
    }

void HUTExportProgressIndicator::StopIteration()
    {
    if (m_pExportedFile != 0)
        {
        //TR 204461
        m_pExportedFile->CancelCreate();
        }

    //Ensure that the sub-resolution update is cancelled
    HRAUpdateSubResProgressIndicator::GetInstance()->StopIteration();
    HRADrawProgressIndicator::GetInstance()->StopIteration();

    HFCProgressIndicator::StopIteration();
    }

//----------------------------------------------------------------------------
