//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSocketConnection.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFSocketConnection
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCSocketConnection.h>
#include "HFCURLInternetImagingSocket.h"

class HRFSocketConnection : public HFCSocketConnection
    {
public:
    HDECLARE_CLASS_ID(1451, HFCSocketConnection)


    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HRFSocketConnection(const HFCURLInternetImagingSocket& pi_rURL,
                        const WString&                     pi_rUserName = WString(L""),
                        const WString&                     pi_rPassword = WString(L""));
    virtual         ~HRFSocketConnection();


    //--------------------------------------
    // I/O Methods
    //--------------------------------------

    virtual void    Send(const Byte* pi_pData, size_t pi_DataSize) override;

    bool           RequestEnded() const;

protected:
    void            RequestHasEnded(bool pi_Success);

private:

    bool           m_RequestEnded;
    };

