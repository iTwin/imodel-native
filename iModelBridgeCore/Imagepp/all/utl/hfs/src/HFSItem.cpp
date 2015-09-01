//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSItem.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Methods for class HFSItem
//:>---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFSItem.h>
#include <Imagepp/all/h/HFSItemIterator.h>

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Get the path ot the item.

 @note This method is recusive, cannot be inline.

 @return string  The item's path.
-----------------------------------------------------------------------------*/
WString HFSItem::GetPath() const
    {
    if (HasParent())
        {
        HPRECONDITION(GetParent()->IsCompatibleWith(HFSItem::CLASS_ID));
        return ((const HFCPtr<HFSItem>&)GetParent())->GetPath() + L"\\" + GetName();
        }
    else
        return GetName();
    }


/**----------------------------------------------------------------------------
 Get a specific item into this folder.

 @return const HFCPtr<HFSItem>& The specific item, 0 if the item was not found.
-----------------------------------------------------------------------------*/
const HFCPtr<HFSItem>& HFSItem::GetItem(const WString& pi_rItemName) const
    {
    HPRECONDITION(!pi_rItemName.empty());
    HPRECONDITION(IsFolder());

    HAutoPtr<HFSItemIterator> pItr((HFSItemIterator*)CreateIterator());

    bool ItemFound = false;
    if ((m_pResult = pItr->GetFirstItem()) != 0)
        {
        do
            {
            ItemFound = wcscmp(m_pResult->GetName().c_str(), pi_rItemName.c_str()) == 0;
            }
        while (!ItemFound && (m_pResult = pItr->GetNextItem()) != 0);
        }

    return m_pResult;
    }


/**----------------------------------------------------------------------------
 Create iterator.

 @return const HFCNodeIterator*

 @see HFSItemIterator
 @see HFCNodeIterator
-----------------------------------------------------------------------------*/
HFCNodeIterator* HFSItem::CreateIterator() const
    {
    HPRECONDITIONSHAREABLE();

    return new HFSItemIterator(const_cast<HFSItem*>(this));
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor for this class.

 @param pi_rEntryName   The name of the entry.
 @param pi_rParent      The parent of the entry.
-----------------------------------------------------------------------------*/
HFSItem::HFSItem(const WString&                 pi_rItemName,
                 const HFCPtr<HFSItem>&         pi_rParent,
                 const HFCPtr<HPMAttributeSet>& pi_rpAttributes)
    : HFCNode((const HFCPtr<HFCNode>&)pi_rParent),
      m_ItemName(pi_rItemName),
      m_pItemAttributes(pi_rpAttributes)

    {
    HPRECONDITION(!pi_rItemName.empty());
    HPRECONDITION(pi_rParent == 0 || pi_rParent->IsFolder());
    }

/**----------------------------------------------------------------------------
 Constructor for this class.
-----------------------------------------------------------------------------*/
HFSItem::HFSItem()
    : HFCNode(0)
    {
    }

//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------








