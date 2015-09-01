//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSocketConnection.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCSocketConnection
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCURLCommonInternet.h>
#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HFCEvent.h>

BEGIN_IMAGEPP_NAMESPACE
class SocketThread;

class HFCSocketConnection : public HFCInternetConnection
    {
    friend class SocketThread;

public:
    HDECLARE_CLASS_ID(HFCConnectionId_Socket, HFCInternetConnection)


    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    IMAGEPP_EXPORT                 HFCSocketConnection(const WString& pi_rServer,
                                               const WString& pi_rUserName = WString(L""),
                                               const WString& pi_rPassword = WString(L""));
    IMAGEPP_EXPORT virtual         ~HFCSocketConnection();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // I/O methods
    IMAGEPP_EXPORT virtual void    Send   (const Byte* pi_pData, size_t pi_DataSize) override;
    IMAGEPP_EXPORT virtual void    Receive(Byte* po_pData, size_t po_DataSize);
    IMAGEPP_EXPORT virtual void    Receive(Byte* po_pData, size_t* pio_pDataSize);

    // Connection/disconnection
    IMAGEPP_EXPORT virtual bool   Connect        (const WString&  pi_rUserName,
                                           const WString&  pi_rPassword,
                                           time_t          pi_TimeOut = 30000);
    IMAGEPP_EXPORT virtual bool   ValidateConnect(uint32_t pi_TimeOut = 30000);
    IMAGEPP_EXPORT virtual void    Disconnect     ();

    // Data Query
    IMAGEPP_EXPORT virtual size_t  WaitDataAvailable();
    IMAGEPP_EXPORT virtual size_t  WaitDataAvailable(uint32_t pi_TimeOut);

    // Utility
    IMAGEPP_EXPORT static uint32_t ntohl2 (uint32_t);

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

END_IMAGEPP_NAMESPACE