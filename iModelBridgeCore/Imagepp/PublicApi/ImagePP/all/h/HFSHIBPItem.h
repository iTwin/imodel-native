//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSHIBPItem.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Class : HFSHIBPItem
//:>---------------------------------------------------------------------------

#pragma once

#include "HFSItem.h"
#include "HFCURL.h"
#include "HFCInternetConnection.h"
#include "HFSHIBPHandler.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a special file system entry for HIBP protocol.

    @see HFSItem
    -----------------------------------------------------------------------------
*/
class HFSHIBPItem : public HFSItem
    {
public:
    HDECLARE_CLASS_ID(5022, HFSItem);

    //:> Primary method
    HFSHIBPItem(HFCPtr<HFSHIBPHandler>&         pi_rpHIBPHandler,
                const WString&                  pi_rItemName,
                const HFCPtr<HFSHIBPItem>&      pi_rParent,
                const HFCPtr<HPMAttributeSet>&  pi_rpInterestingAttributes = 0);

    HFSHIBPItem(HFCPtr<HFSHIBPHandler>&         pi_rpHIBPHandler,
                const WString&                  pi_rItemPath,
                const HFCPtr<HPMAttributeSet>&  pi_rpInterestingAttributes = 0);

    HFSHIBPItem(HFCPtr<HFSHIBPHandler>&         pi_rpHIBPHandler,
                const WString&                  pi_rItemName,
                bool                           pi_Folder,
                const HFCPtr<HFSHIBPItem>&      pi_rParent,
                const HFCPtr<HPMAttributeSet>&  pi_rpInterestingAttributes = 0);

    virtual                     ~HFSHIBPItem();

    virtual bool               IsFolder() const;
    virtual void                Expand();

    virtual WString             GetPath() const;

    virtual const HFCPtr<HFSItem>&
    GetItem(const WString& pi_rItemName) const;

    virtual HFCNodeIterator*    CreateIterator() const;

    //:> overload from HFCNode
    virtual size_t              CountChild() const;
    virtual bool               HasParent() const;
    virtual const HFCPtr<HFCNode>&
    GetParent() const;

protected:

private:

    //:> members
    mutable HFCPtr<HFSHIBPHandler>
    m_pHIBPHandler;
    WString                     m_Host;
    mutable WString             m_ParentPath;
    bool                       m_Folder;
    bool                       m_Expanded;
    HFCPtr<HPMAttributeSet>     m_pInterestingAttributes;

    //:> Methods

    //:> Disabled methods
    HFSHIBPItem();
    HFSHIBPItem(const HFSHIBPItem&);
    HFSHIBPItem& operator=(const HFSHIBPItem&);
    };

//#include "HFSHIBPItem.hpp"

