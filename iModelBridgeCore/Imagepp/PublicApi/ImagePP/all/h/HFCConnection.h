//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCConnection.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCInterlockedValue.h"

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// HFCConnection class
//-----------------------------------------------------------------------------
class HFCConnection : public HFCShareableObject<HFCConnection>
    {
public:
    HDECLARE_BASECLASS_ID(HFCConnectionId_Base)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    IMAGEPP_EXPORT                 HFCConnection(const WString& pi_rServer);
    IMAGEPP_EXPORT virtual         ~HFCConnection() = 0;


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Establishes the connection to the host
    virtual bool   Connect(const WString& pi_rUserName,
                            const WString& pi_rPassword,
                            time_t         pi_TimeOut) = 0;

    virtual void    Disconnect()                       = 0;

    // Restablishes the connection if needed
    virtual bool   ValidateConnect(uint32_t pi_TimeOut) = 0;

    // Connection State methods
    bool           IsConnected() const {return m_ConnectionState;}

    // Get the host
    const WString&  GetServer() const {return m_Server;}


protected:
    //--------------------------------------
    // methods
    //--------------------------------------

    // Derived classes may use this method to change the state
    // of the connection.
    void            SetConnected(bool pi_IsConnected) {m_ConnectionState = pi_IsConnected;}


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // URL to the server
    WString         m_Server;


    // Connection State.
    HFCInterlockedValue<bool>
    m_ConnectionState;
    };

END_IMAGEPP_NAMESPACE


