//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCInternetConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCInternetConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HFCException.h"
#include "HFCConnection.h"
#include "HFCURLCommonInternet.h"


//-----------------------------------------------------------------------------
// HFCInternetConnection class
//-----------------------------------------------------------------------------
class HFCInternetConnection : public HFCConnection
    {
public:
    HDECLARE_CLASS_ID(1801, HFCConnection)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                 HFCInternetConnection(const WString& pi_rServer,
                                                 const WString& pi_rUserName,
                                                 const WString& pi_rPassword);
    _HDLLu virtual         ~HFCInternetConnection() = 0;


    //--------------------------------------
    // I/O Methods
    //--------------------------------------

    virtual bool   ValidateConnect(uint32_t pi_TimeOut = 30000) = 0;

    // NOTE: The send and receive methods must be thread safe
    // in the sense that one thread can receive will another
    // sends on the same connection object.
    // The Send methods must return immediately.  If thread must be added
    // to the connection to send and wait, then so be it. (eg HTTP).

    _HDLLu virtual void    Send(const Byte* pi_pData, size_t pi_DataSize) = 0;
    _HDLLu virtual void    Receive(Byte* po_pData, size_t pi_DataSize)    = 0;

    // NOTE: This send method only sends the data that the network
    // can send immediately.
    // Default implementation (here) does a full send (potentially blocking)
    _HDLLu virtual void    SendImmediate(const Byte* pi_pData, size_t* pio_pDataSize);

    // NOTE: This receive method only retrieve the data available.
    // It waits if no data is available.
    _HDLLu virtual void    Receive(Byte* po_pData, size_t* pio_pDataSize) = 0;


    //--------------------------------------
    // Time out
    //--------------------------------------

    void            SetTimeOut(uint32_t pi_TimeOut);
    uint32_t        GetTimeOut() const;


    //--------------------------------------
    // Query
    //--------------------------------------

    // NOTE:  The ___DataAvailable() methods return the number of bytes
    // available on the connection.  If none is available, blocks until
    // new data arrives or until time-out expires.
    _HDLLu virtual size_t WaitDataAvailable()                            = 0;
    _HDLLu virtual size_t WaitDataAvailable(uint32_t pi_TimeOut)           = 0;

    // NOTE:  The ___DataWriteable() methods indicate if new data can
    // be written on the connection. If none can be written, blocks until
    // condition becomes true or until time-out expires.
    _HDLLu virtual bool   WaitDataWriteable();
    _HDLLu virtual bool   WaitDataWriteable(uint32_t pi_TimeOut);


    void            SetUserName(const WString& pi_rUserName);
    void            SetPassword(const WString& pi_rPassword);
    const WString&  GetUserName() const;
    const WString&  GetPassword() const;


    // Proxy information
    void            SetProxyUserName(const WString& pi_rUserName);
    void            SetProxyPassword(const WString& pi_rPassword);
    const WString&  GetProxyUserName() const;
    const WString&  GetProxyPassword() const;


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Time out
    HFCInterlockedValue<uint32_t>
    m_TimeOut;

    WString         m_UserName;
    WString         m_Password;

    WString         m_ProxyUserName;
    WString         m_ProxyPassword;
    };

#include "HFCInternetConnection.hpp"

