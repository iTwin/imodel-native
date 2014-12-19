//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFSWinItem.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> inline method for class HFSWinEntry
//:>---------------------------------------------------------------------------

//:>---------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------



/**----------------------------------------------------------------------------
 Destructor for this class.

-----------------------------------------------------------------------------*/
inline HFSWinItem::~HFSWinItem()
    {
    }


/**----------------------------------------------------------------------------
 Check if the items was a folder.

 @return bool true if the item is a folder, false otherwise.
-----------------------------------------------------------------------------*/
inline bool HFSWinItem::IsFolder() const
    {
    return m_Folder;
    }

/**----------------------------------------------------------------------------
 Get a specific item into this folder.

 @note if you iterate into the folder, this method will be change can be change
       the current item.
 @return const HFCPtr<HFSItem>& See HFSItem::GetItem for returned value.

 @see HFSItem::GetItem
 @see GetFirstItem
 @see GetNextItem
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSWinItem::GetItem(const WString& pi_rItemName) const
    {
    HPRECONDITION(!pi_rItemName.empty());
    HPRECONDITION(IsFolder());

    const_cast<HFSWinItem*>(this)->Expand();

    return HFSItem::GetItem(pi_rItemName);
    }

/**----------------------------------------------------------------------------
 Create iterator.

 @return HFSNodeIterator*
-----------------------------------------------------------------------------*/
inline HFCNodeIterator* HFSWinItem::CreateIterator() const
    {
    const_cast<HFSWinItem*>(this)->Expand();

    return HFSItem::CreateIterator();
    }

/**----------------------------------------------------------------------------
 Count the number of item.

 @note This method was overloaded from HFCNode,

 @return UInt32 The number of item.
-----------------------------------------------------------------------------*/
inline size_t HFSWinItem::CountChild() const
    {
    HPRECONDITION(IsFolder());

    const_cast<HFSWinItem*>(this)->Expand();

    return HFCNode::CountChild();
    }

//:>---------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------


//:>---------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------

