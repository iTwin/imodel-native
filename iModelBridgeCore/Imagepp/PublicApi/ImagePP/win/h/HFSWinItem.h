//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFSWinItem.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Class : HFSWinItem
//:>---------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFSItem.h>
#include <ImagePP/all/h/HFCURL.h>

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a special file system entry for Windows.

    @see HFSItem
    -----------------------------------------------------------------------------
*/
class HFSWinItem : public HFSItem
    {
public:
    HDECLARE_CLASS_ID(5020, HFSItem);

    //:> Primary method
    HFSWinItem(const WString&                   pi_rItemName,
               const HFCPtr<HFSWinItem>&        pi_rpParent,
               const HFCPtr<HPMAttributeSet>&   pi_rpInterestingAttributes = 0);
    HFSWinItem(const WString&                   pi_rItemPath,
               const HFCPtr<HPMAttributeSet>&   pi_rpInterestingAttributes = 0);

    virtual                     ~HFSWinItem();

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


    virtual void                UniformPath(WString* pio_pPath) const;

protected:

private:

    //:> members
    mutable WString         m_ParentPath;
    bool                    m_Folder;
    bool                    m_Expanded;
    HAutoPtr<NETRESOURCEW>  m_pNetResource;
    HFCPtr<HPMAttributeSet> m_pInterestingAttributes;

    //:> Methods

    HFSWinItem(WIN32_FIND_DATA&             pi_rFindData,
               const HFCPtr<HFSWinItem>&    pi_rpParent,
               const HFCPtr<HPMAttributeSet>&
               pi_rpInterestingAttributes = 0);

    HFSWinItem(NETRESOURCEW&                 pi_rNetResource,
               const HFCPtr<HFSWinItem>&    pi_rpParent,
               const HFCPtr<HPMAttributeSet>&
               pi_rpInterestingAttributes = 0);

    void                    CreateItem(const WString&               pi_rItemName,
                                       const HFCPtr<HFSWinItem>&    pi_rpParent);

    void                    SetItemAttributes(NETRESOURCEW&          pi_rNetResource);
    void                    SetItemAttributes(WIN32_FIND_DATA&      pi_rFindData);

    NETRESOURCEW*            GetResource(const WString&              pi_rResourceName,
                                        NETRESOURCEW*                pi_pContainer);
    NETRESOURCEW*            CloneNetResource(const NETRESOURCEW&     pi_rNetResource);

    void                    HandleWinError(uint32_t pi_ErrorCode);

    //:> Disabled methods
    HFSWinItem();
    HFSWinItem(const HFSWinItem&);
    HFSWinItem& operator=(const HFSWinItem&);
    };

#include "HFSWinItem.hpp"

