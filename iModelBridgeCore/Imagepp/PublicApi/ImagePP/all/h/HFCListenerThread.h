//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCListenerThread.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCListenerThread
//-----------------------------------------------------------------------------
// HFCListenerThread.h : header file
//-----------------------------------------------------------------------------

#pragma once

#include "HAutoPtr.h"
#include "HFCThread.h"
#include "HFCEvent.h"

BEGIN_IMAGEPP_NAMESPACE

template <class T>
class HFCListenerThread : public HFCThread
    {
public:
    //--------------------------------------
    // Construction - Destruction
    //--------------------------------------

    HFCListenerThread(unsigned short pi_Port,
                      HFCThread*    pi_pStarterThread);
    virtual         ~HFCListenerThread();


    //--------------------------------------
    // Overloaded from HFCThread
    //--------------------------------------

    virtual void    Go();


    //--------------------------------------
    // Methods
    //--------------------------------------

    unsigned short GetPort() const;


protected:
private:

    //--------------------------------------
    // Not implemented
    //--------------------------------------

    HFCListenerThread(const HFCListenerThread&);
    HFCListenerThread& operator=(const HFCListenerThread&);

    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Port to listen to
    HFCEvent        m_PortEvent;
    unsigned short m_Port;
    HFCThread*      m_pStarterThread;
    };

END_IMAGEPP_NAMESPACE

#include "HFCListenerThread.hpp"

