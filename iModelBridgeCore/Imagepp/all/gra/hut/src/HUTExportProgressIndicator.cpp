//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTExportProgressIndicator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Class : HUTExportProgressIndicator
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HUTExportProgressIndicator.h>

#include <Imagepp/all/h/HRAUpdateSubResProgressIndicator.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>

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
