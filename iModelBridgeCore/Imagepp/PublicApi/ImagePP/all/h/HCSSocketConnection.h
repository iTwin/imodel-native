//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSSocketConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HFCInternetConnection.h"
#include "HFCURLCommonInternet.h"
#include "HFCBuffer.h"
#include "HFCEvent.h"

#ifndef _WIN32
#error Not yet implemented for this OS
#endif

class HCSSocketConnection : public HFCInternetConnection
    {
    friend class HCSSocketServerConnection;
    friend class HCSSocketConnectionGroup;

public:
    HDECLARE_CLASS_ID(1822, HFCInternetConnection)


    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HCSSocketConnection(const WString& pi_rServer,
                        const WString& pi_rUserName = WString(L""),
                        const WString& pi_rPassword = WString(L""));
    virtual         ~HCSSocketConnection();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // I/O methods
    virtual void    Send   (const Byte* pi_pData, size_t pi_DataSize) override;
    virtual void    SendImmediate   (const Byte* pi_pData, size_t* pio_pDataSize) override;
    virtual void    Receive(Byte* po_pData, size_t po_DataSize);
    virtual void    Receive(Byte* po_pData, size_t* pio_pDataSize);

    // Connection/disconnection
    virtual bool   Connect        (const WString& pi_rUserName,
                                    const WString& pi_rPassword,
                                    time_t         pi_TimeOut = 30000);
    virtual bool   ValidateConnect(uint32_t pi_TimeOut = 30000);
    virtual void    Disconnect     ();

    bool           SetBlockingMode(bool pi_Blocking);

    // Data Query
    virtual size_t  WaitDataAvailable();
    virtual size_t  WaitDataAvailable(uint32_t pi_TimeOut);

    virtual bool   WaitDataWriteable();
    virtual bool   WaitDataWriteable(uint32_t pi_TimeOut);

    HFCPtr<HFCURLCommonInternet>
    GetURL();
protected:

    HFCPtr<HFCURLCommonInternet>    m_pURL;

private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

#ifdef _WIN32
    SOCKET          m_hSocket;
#endif

    // Note if the socket is in blocking mode or not.
    bool           m_Blocking;

    //--------------------------------------
    // Methods
    //--------------------------------------

#ifdef _WIN32
    // Used only by the ServerSocket
    HCSSocketConnection(SOCKET pi_Socket);
#endif
    };

