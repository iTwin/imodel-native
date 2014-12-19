//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCBuffer.h"
#include "HRPPixelType.h"
#include "HGF2DTransfoModel.h"
#include "HGF2DWorld.h"
#include "HGF2DLiteExtent.h"
#include "HFCVersion.h"
#include "HFCMonitor.h"

class HRFInternetImagingFile;
class HFCInternetConnection;

class HRFInternetImagingHandler : public HFCShareableObject<HRFInternetImagingHandler>
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetImagingHandler(const string& pi_rLabel);
    virtual         ~HRFInternetImagingHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // verify if the current data can be handle by this class
    bool           CanHandle(const HFCBuffer& pi_rBuffer) const;

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection) = 0;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // label of the handler.  All responses that begin with this label
    // will be handling by this object
    string                  m_Label;
    };

#include "HRFInternetImagingHandler.hpp"

