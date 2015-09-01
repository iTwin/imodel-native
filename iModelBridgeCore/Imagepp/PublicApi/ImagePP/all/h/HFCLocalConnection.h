//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCLocalConnection.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCLocalConnection
//-----------------------------------------------------------------------------

#pragma once


//############################
//INCLUDE FILES
//############################

// HFC
#include "HFCConnection.h"
#include "HFCURLFile.h"

BEGIN_IMAGEPP_NAMESPACE

class HFCLocalConnection : public HFCConnection
    {
public:
    HDECLARE_CLASS_ID(HFCConnectionId_Local, HFCConnection)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCLocalConnection(const WString& pi_rServer,
                       const WString& pi_rUserName,
                       const WString& pi_rPassword);
    virtual
    ~HFCLocalConnection();

    // Connection/disconnection
    virtual bool
    Connect(const WString& pi_rUserName,
            const WString& pi_rPassword,
            time_t pi_TimeOut = 30000);

    virtual bool
    ValidateConnect(uint32_t pi_TimeOut = 30000);

    virtual void
    Disconnect();

    const WString&
    GetUserName();

    const WString&
    GetPassword();

protected:

    WString m_UserName;
    WString m_Password;

    HFCPtr<HFCURLFile> m_pURL;

private:

    HFCLocalConnection();
    HFCLocalConnection(const HFCLocalConnection&);
    HFCLocalConnection& operator=(const HFCLocalConnection&);
    };

END_IMAGEPP_NAMESPACE