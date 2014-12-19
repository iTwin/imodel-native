//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSNamedPipeConnectionGroup.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSNamedPipeConnectionGroup
//-----------------------------------------------------------------------------

#pragma once

// Headers
#include "HCSConnectionGroup.h"
#include "HFCEvent.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>

// Forward declaration
class HCSNamedPipeConnection;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a named pipe connection group.

    ????
    -----------------------------------------------------------------------------
*/
class HCSNamedPipeConnectionGroup : public HCSConnectionGroup
    {
public:
    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HCSNamedPipeConnectionGroup ();
    virtual         ~HCSNamedPipeConnectionGroup();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Add a connection to the group
    virtual void    AddConnection(HFCInternetConnection* pi_pConnection);

    // Removes a  connection to the group
    virtual void    RemoveConnection(HFCInternetConnection* pi_pConnection);

    // Checks for activity on a group of sockets.
    virtual HFCInternetConnection*
    CheckActivity(time_t pi_TimeOut);


private:
    //--------------------------------------
    // Types
    //--------------------------------------

    // A map to associate events to its pipe object
    typedef map<const HFCSynchro*, HCSNamedPipeConnection*>
    ConnectionMap;

    // a list of socket connection
    typedef list<HCSNamedPipeConnection*>
    ConnectionList;


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The all-mighty exclusive key
    mutable HFCExclusiveKey m_Key;

#ifdef _WIN32
    // a map of event handle to HFCNamedPipeConnection objects
    ConnectionMap           m_ConnectionMap;

    // set of sockets to check for activity.  A fd_set has a limit
    // of FD_SETSIZE entries.  So when the set becomes full, add the
    // connection in the waiting list.
    HFCSynchroContainer     m_PipeSet;
    ConnectionList          m_WaitingPipes;

    // protects the select call
    mutable HFCExclusiveKey m_ActiveConnectionKey;

    // a back log of active sockets
    ConnectionList          m_ActiveConnections;

#endif
    };

