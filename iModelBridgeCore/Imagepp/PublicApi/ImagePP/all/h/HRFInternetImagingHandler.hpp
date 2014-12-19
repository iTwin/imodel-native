//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingHandler.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingHandler
//-----------------------------------------------------------------------------

#include "HRFInternetImagingFile.h"
#include <io.h>
#include <sys/stat.h>
#include <sys/utime.h>

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HRFInternetImagingHandler::HRFInternetImagingHandler(const string& pi_rLabel)
    : m_Label(pi_rLabel)
    {
    HPRECONDITION(!pi_rLabel.empty());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HRFInternetImagingHandler::~HRFInternetImagingHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline bool HRFInternetImagingHandler::CanHandle(const HFCBuffer& pi_rBuffer) const
    {
    HPRECONDITION(pi_rBuffer.GetDataSize() > 0);
    HPRECONDITION(pi_rBuffer.GetData() != 0);

    return ((pi_rBuffer.GetDataSize() >= m_Label.size()) ?
            (memicmp(m_Label.c_str(), pi_rBuffer.GetData(), m_Label.size()) == 0) :
            false);
    }