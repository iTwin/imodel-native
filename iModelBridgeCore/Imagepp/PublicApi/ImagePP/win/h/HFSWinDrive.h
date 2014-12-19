//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFSWinDrive.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//Class : HFSWinDrive
//---------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFSFileSystem.h>
#include "HFSWinItem.h"
#include <ImagePP/all/h/HFCURL.h>
#include <ImagePP/all/h/HPMAttributeSet.h>

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a Windows file system.

    @see HFSFileSystem
    -----------------------------------------------------------------------------
*/
class HFSWinDrive : public HFSFileSystem
    {
public:
    HDECLARE_CLASS_ID(5021, HFSFileSystem);

    //:> Primary method
    _HDLLw                         HFSWinDrive(const WString&                  pi_rDriveName,
                                               const HFCPtr<HPMAttributeSet>&  pi_rpInterestingAttributes = 0);
    _HDLLw                         HFSWinDrive(const HFCPtr<HFCURL>&           pi_rpPath,
                                               const HFCPtr<HPMAttributeSet>&  pi_rpInterestingAttributes = 0);
    _HDLLw virtual                 ~HFSWinDrive();

    virtual HFCPtr<HFCURL>  GetCurrentURLPath() const;
    virtual WString         GetCurrentPath() const;
    virtual const HFCPtr<HFSItem>&
    ChangeFolder(const WString& pi_rPath);
    virtual const HFCPtr<HFSItem>&
    GetCurrentFolder() const;
    virtual const HFCPtr<HFSItem>&
    GetItem(const WString& pi_rPath) const;


    const string&           GetDriveName() const;

protected:

private:

    //:> members
    WString                     m_Drive;
    HFCPtr<HFSWinItem>          m_pCurrentFolder;
    HFCPtr<HPMAttributeSet>     m_pInterestingAttributes;

    //:> optimization
    mutable HFCPtr<HFSWinItem>  m_pResult;

    //:> Disabled methods
    HFSWinDrive(const HFSWinDrive&);
    HFSWinDrive& operator=(const HFSWinDrive&);
    };

#include "HFSWinDrive.hpp"

