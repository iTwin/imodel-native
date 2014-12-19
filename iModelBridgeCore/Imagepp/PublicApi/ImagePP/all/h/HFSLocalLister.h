//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSLocalLister.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFSLocalLister
//-----------------------------------------------------------------------------

#pragma once


//############################
//INCLUDE FILES
//############################

// HFC
#include "HFCURLFile.h"
#include "HFCLocalConnection.h"

// HFS
#include "HFSDirectoryLister.h"


class HFSLocalLister : public HFSDirectoryLister
    {
public:

    HDECLARE_CLASS_ID(5002, HFSDirectoryLister);

    // Construction - Destruction
    HFSLocalLister(HFCPtr<HFCConnection>& pi_pConnection,
                   WString                pi_DefaultPattern = WString(L"*.*"));
    virtual
    ~HFSLocalLister();

    virtual bool
    List(const WString& pi_rPath);

    virtual bool
    SupportFileDate() const;

    virtual bool
    SupportFileSize() const;


protected:

private:

    // Not implemented
    HFSLocalLister(const HFSLocalLister&);
    HFSLocalLister& operator=(const HFSLocalLister&);
    };

#include "HFSLocalLister.hpp"

