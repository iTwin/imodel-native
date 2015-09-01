//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSSession.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Methods for class HPSSession
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include "HPSParserScope.h"
#include "HPSSession.h"

//---------------------------------------------------------------------------
HPSSession::HPSSession(HFCBinStream*                  pi_pStream,
                       HPMPool*                       pi_pPool,
                       const HFCPtr<HFCURL>&          pi_rpURL,
                       HPSParserScope*                pi_pTopScope,
                       const HFCPtr<HPSWorldCluster>  pi_rpHPSWorldCluster,
                       const HGF2DWorldIdentificator* pi_pCurrentWorldId)
    : HPASession(pi_pStream),
      m_pPool(pi_pPool),
      m_pURL(pi_rpURL),
      m_pTopScope(pi_pTopScope),
      m_pCurrentScope(pi_pTopScope),
      m_pCluster(pi_rpHPSWorldCluster)
    {
    if (pi_pCurrentWorldId != 0)
        {
        ChangeCurrentWorld(*pi_pCurrentWorldId);
        }
    else
        {
        ChangeCurrentWorld(0);
        }

    pi_pTopScope->SetSession(this);
    }

//---------------------------------------------------------------------------
HPSSession::~HPSSession()
    {
    // Clear() should be called to help destruction of this object.
    }

//---------------------------------------------------------------------------
void HPSSession::Clear()
    {
    m_PageNodes.clear();
    if (m_pTopScope != 0)
        {
        delete m_pTopScope;
        m_pTopScope = 0;
        }
    }

