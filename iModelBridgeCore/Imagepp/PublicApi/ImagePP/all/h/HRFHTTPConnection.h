//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFHTTPConnection
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCHTTPConnection.h>
#include "HRFURLInternetImagingHTTP.h"

class HRFHTTPConnection : public HFCHTTPConnection
    {
public:
    HDECLARE_CLASS_ID(1452, HFCHTTPConnection)

    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HRFHTTPConnection(const HRFURLInternetImagingHTTP&  pi_rURL,
                      const WString&                    pi_rUserName = WString(L""),
                      const WString&                    pi_rPassword = WString(L""));
    virtual         ~HRFHTTPConnection();

    virtual void    Send   (const Byte* pi_pData, size_t pi_DataSize) override;

    bool           RequestEnded() const;

protected:
    //--------------------------------------
    // methods
    //--------------------------------------

    // called when a request is finished
    virtual void    RequestHasEnded(bool pi_Success);

private:
    bool   m_RequestEnded;
    };