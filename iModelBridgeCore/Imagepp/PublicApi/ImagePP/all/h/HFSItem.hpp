//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSItem.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> inline methods for class HFSItem
//:>---------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
inline HFSItem::~HFSItem()
    {
    }

/**----------------------------------------------------------------------------
 Get the name of the item.

 @return const WString&  The name of the item.
-----------------------------------------------------------------------------*/
inline const WString& HFSItem::GetName() const
    {
    return m_ItemName;
    }


/**----------------------------------------------------------------------------
 Get the item attributes.

 @return const HFCPtr<HPMAttributSet>& The entry attributes.
-----------------------------------------------------------------------------*/
inline const HFCPtr<HPMAttributeSet>& HFSItem::GetAttributes() const
    {
    return m_pItemAttributes;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Set the item name.

 @param pi_rName    The entry name.
-----------------------------------------------------------------------------*/
inline void HFSItem::SetName(const WString& pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());

    m_ItemName = pi_rName;
    }

/**----------------------------------------------------------------------------
 Set the item attributes.

 @param pi_rpItemAttributes    The item attributes. Cannot be null.
-----------------------------------------------------------------------------*/
inline void HFSItem::SetAttributes(const HFCPtr<HPMAttributeSet>& pi_rpItemAttributes)
    {
    HPRECONDITION(pi_rpItemAttributes != 0);

    m_pItemAttributes = pi_rpItemAttributes;
    }


END_IMAGEPP_NAMESPACE

