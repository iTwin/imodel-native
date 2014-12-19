//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSSocketConnectionGroup.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketConnectionGroup
//-----------------------------------------------------------------------------

#pragma once

// Headers
#include "HCSConnectionGroup.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCSemaphore.h"
#ifndef _WIN32
#error Not yet implemented for this OS
#endif

// Forward declaration
class HCSSocketConnection;

class HCSSocketConnectionGroup : public HCSConnectionGroup
    {
public:
    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HCSSocketConnectionGroup ();
    virtual         ~HCSSocketConnectionGroup();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Add a connection to the group
    virtual void    AddConnection(HFCInternetConnection* pi_pConnection);

    // Removes a  connection to the group
    virtual void    RemoveConnection(HFCInternetConnection* pi_pConnection);

    // Checks for activity on a group of sockets.
    virtual HFCInternetConnection*
    CheckActivity(uint32_t pi_TimeOut);


private:
    //--------------------------------------
    // Types
    //--------------------------------------

    // A map to associate SOCKET handles to socket objects
    typedef map<SOCKET, HCSSocketConnection*>
    ConnectionMap;

    // a list of socket connection
    typedef list<HCSSocketConnection*>
    ConnectionList;


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The all-mighty exclusive key
    mutable HFCExclusiveKey m_Key;

#ifdef _WIN32
    // a map of Socket handle to HFCSocketConnection objects
    ConnectionMap           m_ConnectionMap;

    // set of sockets to check for activity.  A fd_set has a limit
    // of FD_SETSIZE entries.  So when the set becomes full, add the
    // connection in the waiting list.
    fd_set                  m_SocketSet;
    mutable HFCSemaphore    m_SocketSetSemaphore;
    ConnectionList          m_WaitingSockets;

    // protects the select call
    mutable HFCExclusiveKey m_ActiveConnectionKey;

    // a back log of active sockets
    ConnectionList          m_ActiveConnections;

#endif
    };


