//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSInternetLister.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFSInternetLister
//-----------------------------------------------------------------------------

#pragma once


//############################
//INCLUDE FILES
//############################

// HFC
#include "HFCSocketConnection.h"
#include "HFCHTTPConnection.h"

// HFS
#include "HFSDirectoryLister.h"


class HFSInternetLister : public HFSDirectoryLister
    {
public:

    HDECLARE_CLASS_ID(5001, HFSDirectoryLister);

    // Construction - Destruction
    HFSInternetLister(HFCPtr<HFCConnection>& pi_pConnection,
                      bool pi_UseExtendedProtocol = true);
    virtual
    ~HFSInternetLister();

    virtual bool
    List(const WString& pi_rPath);

    virtual bool
    SupportFileDate() const;

    virtual bool
    SupportFileSize() const;


protected:

    bool               m_UseExtendedProtocol;
    WString             m_Command;

private:


    // Not implemented
    HFSInternetLister(const HFSInternetLister&);
    HFSInternetLister& operator=(const HFSInternetLister&);

    bool   BuildFileList(const string& pi_rData);

    bool   VerifyExtendedProtocol();

    bool   m_ProtocolChecked;

    };

#include "HFSInternetLister.hpp"

