//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBEngineThread.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBEngineThread
//-----------------------------------------------------------------------------
#pragma once

#include "HFCThread.h"

class PUBRequestDispatcher;

class PUBEngineThread : public HFCThread
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    PUBEngineThread (uint32_t              pi_ID,
                     PUBRequestDispatcher& pi_rDispatcher);
    virtual                 ~PUBEngineThread();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Engine implementation
    virtual void            Go();

    // Gets the assigned request dispatcher
    PUBRequestDispatcher&   GetRequestDispatcher() const;


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    uint32_t                m_ID;
    PUBRequestDispatcher&   m_rDispatcher;

    };