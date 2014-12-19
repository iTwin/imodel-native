//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingThread.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingThread
//-----------------------------------------------------------------------------

#pragma once

#include "HFCThread.h"
#include "HRFInternetImagingHandler.h"
#include "HFCMonitor.h"
#include "HFCBuffer.h"
#include "HFCInterlockedValue.h"

class HRFInternetImagingFile;

//--------------------------------------
// Thread Class Declaration
//--------------------------------------
class HRFInternetImagingThread : public HFCThread
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetImagingThread(HRFInternetImagingFile& pi_rFile);
    virtual         ~HRFInternetImagingThread();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Add a handler to the thread's ability.  All handlers must be
    // added before the thread is started.
    void            AddHandler (const HFCPtr<HRFInternetImagingHandler>& pi_rpHandler);
    void            AddHandlers(const HandlerList&                 pi_rHandlers);
    void            ClearHandlers();

    // Clears the internal read buffer
    void            ClearBuffer();

    // Execution method
    virtual void    Go();

private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // List of the known handlers
    HFCExclusiveKey         m_HandlerMonitor;
    HandlerList             m_Handlers;

    // Read buffer
    HFCInterlockedValue<bool>
    m_ClearReadBuffer;
    HFCBuffer               m_ReadBuffer;

    // Reference to the internet imaging that owns the thread
    HRFInternetImagingFile& m_rFile;
    };

