//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSBufferedConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSBufferedConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HFCInternetConnection.h"
#include "HFCBuffer.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a buffered internet connection. The class behaves exactly
    like any other internet connection except that data received is buffered in a
    buffer of indicated size.

    The class is used to envelop an existing connection into another with the
    same interface but with the additional buffering capability.
    -----------------------------------------------------------------------------
*/
class HCSBufferedConnection : public HFCInternetConnection
    {
public:
    HDECLARE_CLASS_ID(1825, HFCInternetConnection)
    friend class HCSBufferedConnectionPool;

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSBufferedConnection(HFCInternetConnection* pi_pConnection,
                          size_t                 pi_BufferGrowSize,
                          bool                  pi_OwnsConnection = false,
                          size_t                 pi_ReadSize = 4096);
    virtual         ~HCSBufferedConnection();


    //--------------------------------------
    // I/O Methods
    //--------------------------------------

    virtual bool   Connect        (const WString& pi_rUserName,
                                    const WString& pi_rPassword,
                                    time_t pi_TimeOut = 30000);
    virtual bool   ValidateConnect(uint32_t pi_TimeOut = 30000);
    virtual void    Disconnect     ();


    // NOTE: The send and receive methods must be thread safe
    // in the sense that one thread can receive will another
    // sends on the same connection object.
    // The Send methods must return immediately.  If thread must be added
    // to the connection to send and wait, then so be it. (eg HTTP).

    virtual void    Send(const Byte* pi_pData, size_t pi_DataSize) override;
    virtual void    Receive(Byte* po_pData, size_t pi_DataSize);

    // NOTE: This send method only sends the data that the network
    // can send immediately.
    virtual void    SendImmediate(const Byte* pi_pData, size_t* pio_pDataSize) override;

    // NOTE: This receive method only retrieve the data available.
    // It waits if no data is available.
    virtual void    Receive(Byte* po_pData, size_t* pio_pDataSize);


    //--------------------------------------
    // Query
    //--------------------------------------

    // NOTE:  The ___DataAvailable() methods return the number of bytes
    // available on the connection.  If none is available, blocks until
    // new data arrives or until time-out expires.
    virtual size_t  WaitDataAvailable();
    virtual size_t  WaitDataAvailable(uint32_t pi_TimeOut);

    virtual bool   WaitDataWriteable();
    virtual bool   WaitDataWriteable(uint32_t pi_TimeOut);

    HFCInternetConnection*
    GetInternalConnection() const;

    bool            HasConnectionOwnership() const;
    void            SetConnectionOwnership(bool pi_Owner);

protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The quantity of data to attempt to read
    size_t          m_ReadSize;

    // The buffer and the connection
    HFCBuffer       m_Buffer;
    HFCInternetConnection*
    m_pConnection;
    bool           m_HasConnectionOwnership;
    };


