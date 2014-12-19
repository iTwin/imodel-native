//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSHIB.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HFSHIB
//:>---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFSFileSystem.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HPMAttributeSet.h>
#include <Imagepp/all/h/HFCConnection.h>
#include <Imagepp/all/h/HFSHIBPItem.h>

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a server file system.

    @see HFSFileSystem
    -----------------------------------------------------------------------------
*/
class HFSHIB : public HFSFileSystem
    {
public:
    HDECLARE_CLASS_ID(5024, HFSFileSystem);

    //:> Primary method
    _HDLLu                         HFSHIB(HFCPtr<HFCConnection>&    pi_rpConnection,
                                          uint32_t                         pi_ServerTimeOut,
                                          const WString&                   pi_rFolderPath,
                                          const HFCPtr<HPMAttributeSet>&   pi_rpInterestingAttributes = 0);
    _HDLLu                        HFSHIB(const HFCPtr<HFCURL>&     pi_rpPath,
                                          uint32_t                         pi_ServerTimeOut,
                                          const HFCPtr<HPMAttributeSet>&   pi_rpInterestingAttributes = 0);


    _HDLLu virtual                 ~HFSHIB();

    virtual HFCPtr<HFCURL>  GetCurrentURLPath() const;
    virtual WString         GetCurrentPath() const;
    virtual const HFCPtr<HFSItem>&
    ChangeFolder(const WString& pi_rPath);
    virtual const HFCPtr<HFSItem>&
    GetCurrentFolder() const;
    virtual const HFCPtr<HFSItem>&
    GetItem(const WString& pi_rPath) const;

    const HFCPtr<HFCConnection>&
    GetConnection() const;

    const HFCPtr<HPMAttributeSet>&
    GetInterestingAttributes() const;

protected:

    void RemoveAllDescendantNodes(const HFCPtr<HFCNode>& pi_prNode) const;

private:

    //:> members
    HFCPtr<HFCConnection>       m_pConnection;
    HFCPtr<HFSHIBPItem>         m_pCurrentFolder;
    HFCPtr<HPMAttributeSet>     m_pInterestingAttributes;

    //:> optimization
    mutable HFCPtr<HFSHIBPItem> m_pResult;
    WString                     m_Host;

    //:> Disabled methods
    HFSHIB(const HFSHIB&);
    HFSHIB& operator=(const HFSHIB&);
    };

#include "HFSHIB.hpp"
