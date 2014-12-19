//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetDisconnectionThread.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetDisconnectionThread
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCThread.h"

class HFCInternetConnection;

//--------------------------------------
// Thread Class Declaration
//--------------------------------------
class HRFInternetDisconnectionThread : public HFCThread,
    public HFCShareableObject<HRFInternetDisconnectionThread>
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetDisconnectionThread();
    virtual         ~HRFInternetDisconnectionThread();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // adds an internet connection to be closed
    void            AddConnection(HFCInternetConnection* pi_pConnection);

    // Execution method
    virtual void    Go();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // list of connection to close
    typedef list<HFCInternetConnection*>
    ConnectionList;

    // List of the known handlers
    HFCExclusiveKey         m_ListMonitor;
    ConnectionList          m_List;
    HFCEvent                m_ListEvent;
    };

