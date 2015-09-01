//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSItem.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCNode.h"
#include "HPMAttributeSet.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a special node for an HFS - File System.

    @inheritance This class cannot be instanciate.

    @see HFCNode
    @see HFSItemIterator
    -----------------------------------------------------------------------------
*/
class HFSItemIterator;

class HFSItem : public HFCNode
    {
public:
    HDECLARE_CLASS_ID(HFSItemId_Base, HFCNode);

    // Primary method
    IMAGEPP_EXPORT virtual                     ~HFSItem();

    virtual const WString&      GetName() const;
    IMAGEPP_EXPORT virtual WString             GetPath() const;

    virtual const HFCPtr<HPMAttributeSet>&
    GetAttributes() const;

    virtual bool               IsFolder() const = 0;
    virtual void                Expand() = 0;

    IMAGEPP_EXPORT const HFCPtr<HFSItem>&      GetItem(const WString& pi_rItemName) const;

    IMAGEPP_EXPORT virtual HFCNodeIterator*    CreateIterator() const;

    const HFCPtr<HFSItem>&      GetRoot() const;

protected:

    IMAGEPP_EXPORT                         HFSItem(const WString&                  pi_rName,
                                           const HFCPtr<HFSItem>&          pi_rpParent,
                                           const HFCPtr<HPMAttributeSet>&  pi_rpAttributes);
    IMAGEPP_EXPORT                         HFSItem();

    void                    SetName(const WString& pi_rName);
    void                    SetAttributes(const HFCPtr<HPMAttributeSet>& pi_rpEntryAttributes);



private:

    // members
    WString                 m_ItemName;
    HFCPtr<HPMAttributeSet> m_pItemAttributes;

    // optimization
    mutable HFCPtr<HFSItem> m_pResult;

    // Disabled methods
    HFSItem(const HFSItem&);
    HFSItem& operator=(const HFSItem&);
    };

END_IMAGEPP_NAMESPACE
#include "HFSItem.hpp"
