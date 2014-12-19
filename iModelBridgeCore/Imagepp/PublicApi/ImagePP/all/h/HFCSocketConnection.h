//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSocketConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCSocketConnection
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCURLCommonInternet.h>
#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HFCEvent.h>

class SocketThread;

class HFCSocketConnection : public HFCInternetConnection
    {
    friend class SocketThread;

public:
    HDECLARE_CLASS_ID(1802, HFCInternetConnection)


    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    _HDLLu                 HFCSocketConnection(const WString& pi_rServer,
                                               const WString& pi_rUserName = WString(L""),
                                               const WString& pi_rPassword = WString(L""));
    _HDLLu virtual         ~HFCSocketConnection();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // I/O methods
    _HDLLu virtual void    Send   (const Byte* pi_pData, size_t pi_DataSize) override;
    _HDLLu virtual void    Receive(Byte* po_pData, size_t po_DataSize);
    _HDLLu virtual void    Receive(Byte* po_pData, size_t* pio_pDataSize);

    // Connection/disconnection
    _HDLLu virtual bool   Connect        (const WString&  pi_rUserName,
                                           const WString&  pi_rPassword,
                                           time_t          pi_TimeOut = 30000);
    _HDLLu virtual bool   ValidateConnect(uint32_t pi_TimeOut = 30000);
    _HDLLu virtual void    Disconnect     ();

    // Data Query
    _HDLLu virtual size_t  WaitDataAvailable();
    _HDLLu virtual size_t  WaitDataAvailable(uint32_t pi_TimeOut);

    // Utility
    _HDLLu static uint32_t ntohl2 (uint32_t);

protected:

    HFCPtr<HFCURLCommonInternet>    m_pURL;

private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    ULONG_PTR                       m_hSocket;

    // Data Buffer
    mutable HFCExclusiveKey         m_BufferKey;
    mutable HFCBuffer               m_Buffer;

    // Implementation Specific members
    mutable HAutoPtr<SocketThread>  m_pThread;
    mutable HFCEvent                m_SocketEvent;
    };

