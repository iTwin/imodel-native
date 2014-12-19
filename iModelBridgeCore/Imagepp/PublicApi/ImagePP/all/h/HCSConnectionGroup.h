//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSConnectionGroup.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSConnectionGroup
//-----------------------------------------------------------------------------

#pragma once

class HFCInternetConnection;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a connection group interface. A connection group is a simple
    collection of internet connections of any kind.

    This class is pure virtual and must be inherited from to use.

    The internal list of connections can be accessed.
    -----------------------------------------------------------------------------
*/
class HCSConnectionGroup
    {
public:
    //--------------------------------------
    // Types
    //--------------------------------------

    // a list of  connection.  Used when checking
    // for activity on a group of connections.
    typedef list<HFCInternetConnection*>
    ConnectionList;


    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HCSConnectionGroup ();
    virtual         ~HCSConnectionGroup();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Add a connection to the group
    virtual void    AddConnection(HFCInternetConnection* pi_pConnection) = 0;

    // Removes a connection to the group
    virtual void    RemoveConnection(HFCInternetConnection* pi_pConnection) = 0;

    // Checks for activity on a group of connections.
    virtual HFCInternetConnection*
    CheckActivity(uint32_t pi_TimeOut) = 0;
    };

#include "HCSConnectionGroup.hpp"

