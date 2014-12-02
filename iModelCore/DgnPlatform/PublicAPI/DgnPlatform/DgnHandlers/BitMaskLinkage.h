/*----------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/BitMaskLinkage.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/Tools/BitMask.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct BitMaskLinkage
{
public:
//!  Extracts the bit mask linkage from the given element
//! @param        pBitMaskOut     OUT the bit mask extracted from the linkage, must be freed by caller
//! @param        pElementIn      IN  the element that has the bit mask linkage attached
//! @param        linkageKeyIn    IN  the linkage key for the bit mask
//! @param        linkageIndexIn  IN  the index of the bit mask linkage on the element
//! @return       SUCCESS if the linkage is found and extracted
DGNPLATFORM_EXPORT static StatusInt ExtractBitMask (BitMaskH pBitMaskOut, DgnElementCP pElementIn, UShort linkageKeyIn, int linkageIndexIn);

//!  Sets the contents of a bit mask linkage on the given element descriptor.
//! @param        ppElemDscrIn    IN OUT  the element descriptor the linkage is on
//! @param        linkageKeyIn    IN      the linkage key for the bit mask linkage
//! @param        pBitMaskIn      IN      the bit mask contents to set as the linkage contents
//! @return       SUCCESS If the bit mask was found and set
DGNPLATFORM_EXPORT static StatusInt SetBitMaskUsingDescr (MSElementDescrH ppElemDscrIn, UShort linkageKeyIn, BitMaskCP pBitMaskIn);

//!  Sets the contents of a bit mask already appended as a linkage on an element
//! @param        pElementIn  IN  the element on which the bit mask is attached
//! @param        linkageKey  IN  the linkage key of the bit mask linkage
//! @param        pBitMaskIn  IN  the bit mask contents to set
//! @return       SUCCESS if the bit mask linkage was found and could be set
DGNPLATFORM_EXPORT static StatusInt SetBitMask (DgnElementP pElementIn, UShort linkageKey, BitMaskCP pBitMaskIn);

//!  Appends the specified bitmask as a linkage on the given element.
//! @param        pElementIn      IN  the element to append bitmask linkage to
//! @param        linkageKeyIn    IN  the linkage to use when appending the bitmask
//! @param        pBitMaskIn      IN  the bit mask to add as a linkage
//! @return       SUCCESS if the linkage could be attached
DGNPLATFORM_EXPORT static StatusInt AppendBitMask (DgnElementP pElementIn, UShort linkageKeyIn, BitMaskCP pBitMaskIn);

//!  Checks the size of the bit mask linkage on the specified element descriptor.
//! @param        ppElemDscrIn    IN OUT  the element descriptor on which to set the linkage
//! @param        linkageKeyIn    IN      the linkage key to use for the bit mask
//! @param        numBitsIn       IN      the size of the bitmask to ensure in the linkage
//! @return       SUCCESS if the bit mask size is acceptable
DGNPLATFORM_EXPORT static StatusInt EnsureCapacityUsingDescr (MSElementDescrH ppElemDscrIn, UShort linkageKeyIn, int numBitsIn);

//!  Deletes the bit mask linkage from the specified element.
//! @param        pElementIn      IN  the element to delete the bitmask linkage from
//! @param        linkageKeyIn    IN  the linkage key for the bit mask
//! @param        linkageIndexIn  IN  the index of the bit mask linkage on the element
//! @return       SUCCESS if the bitmask linkage was found and deleted successfully
DGNPLATFORM_EXPORT static StatusInt DeleteBitMask (DgnElementP pElementIn, UShort linkageKeyIn, int linkageIndexIn);

};

END_BENTLEY_DGNPLATFORM_NAMESPACE


