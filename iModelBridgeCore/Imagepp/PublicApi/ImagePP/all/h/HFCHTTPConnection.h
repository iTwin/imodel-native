//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHTTPConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHTTPConnection
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include <Imagepp/all/h/HFCEvent.h>
#include <Imagepp/all/h/HFCBuffer.h>

class HTTPThread;

class HFCHTTPConnection : public HFCInternetConnection
    {
    friend class HTTPThread;

public:
    HDECLARE_CLASS_ID(1803, HFCInternetConnection)

    //--------------------------------------
    // Static methods
    //--------------------------------------

    _HDLLu static bool SetProxy(const WString& pi_rProxyList,
                                const WString& pi_rBypass,
                                uint32_t       pi_Flags=0);

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu          HFCHTTPConnection(const WString& pi_rServer,
                                      const WString& pi_rUserName = WString(L""),
                                      const WString& pi_rPassword = WString(L""));
    _HDLLu virtual  ~HFCHTTPConnection();


    //--------------------------------------
    // Overridden from HFCInternetConnection
    //--------------------------------------

    // I/O Methods
    _HDLLu virtual void     Send   (const Byte* pi_pData, size_t pi_DataSize);
    _HDLLu virtual void     Receive(Byte* po_pData, size_t po_DataSize);
    _HDLLu virtual void     Receive(Byte* po_pData, size_t* pio_pDataSize);

    // Connection/Disconnection Methods
    _HDLLu virtual bool    Connect        (const WString& pi_rUserName,
                                            const WString& pi_rPassword,
                                            time_t pi_TimeOut = 30000);
    _HDLLu virtual bool    ValidateConnect(uint32_t pi_TimeOut = 30000);
    _HDLLu virtual void     Disconnect     ();

    // Data Query Methods
    _HDLLu virtual size_t   WaitDataAvailable();
    _HDLLu virtual size_t   WaitDataAvailable(uint32_t pi_TimeOut);


    //--------------------------------------
    // Prefix Methods
    //--------------------------------------

    // Set the server extention to use
    const WString&  GetExtention() const;
    void            SetExtention(const WString& pi_rBase);

    // The URL search base is appended after the URL & extention,
    // but before the actual request. The search base is expected
    // to be encoded correctly and escaped.
    const string&  GetSearchBase() const;
    void           SetSearchBase(const string& pi_rBase);

    const          HFCPtr<HFCURL>& GetURL();

protected:

    enum AuthorizationStatus
        {
        UNINITIALIZE = 1,
        FAILED,
        SUCCEED
        };

    //--------------------------------------
    // methods
    //--------------------------------------

    // called when a request is finished
    virtual void    RequestHasEnded (bool          pi_Success);
    bool           Authorize       (ULONG_PTR      pi_rRequest);
    HSTATUS         SendRequest     (const string&  pi_rRequest);


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Data Buffer
    mutable HFCExclusiveKey         m_BufferKey;
    mutable HFCBuffer               m_Buffer;

    HFCPtr<HFCURLCommonInternet>    m_pURL;
    bool                           m_SecureConnection;

private:

    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Internet connection
    ULONG_PTR                       m_hConnection;

    // URL bases
    WString                         m_Extention;
    string                          m_SearchBase;

    // Implementation specific members
    mutable HAutoPtr<HTTPThread>    m_pThread;
    mutable HFCEvent                m_HTTPEvent;
    };

#include "HFCHTTPConnection.hpp"

