/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/PropMask.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include    <DgnPlatform/DgnPlatform.h>
#include    <DgnPlatform/Tools/BitMask.h>

BEGIN_BENTLEY_NAMESPACE

/*__BENTLEY_INTERNAL_ONLY__*/

struct IPropMask
{
private:

    BitMaskH                                            m_bitMaskArray;

protected:

    DGNPLATFORM_EXPORT IPropMask&                      Copy (IPropMask const* from);
    DGNPLATFORM_EXPORT virtual StatusInt               _SetBitMaskForCategory (BitMaskP pBitMaskIn, uint16_t categoryIdIn);
    DGNPLATFORM_EXPORT virtual StatusInt               _GetBitMaskByCategory (BitMaskH ppBitMaskOut, uint16_t categoryIdIn) const;
    DGNPLATFORM_EXPORT virtual bool                    _SetAnyBit ();
    DGNPLATFORM_EXPORT virtual StatusInt               _SetCategoryByBitArray (uint16_t pArrayIn[], int nValidBits, uint16_t categoryId);
    DGNPLATFORM_EXPORT virtual StatusInt               _SetBitByCategoryAndPosition (uint16_t categoryId, int bitPosition, bool bitValue);
    DGNPLATFORM_EXPORT virtual void                    _ClearAllBits ();
    DGNPLATFORM_EXPORT virtual int                     _GetMaxCategories () const {return 0;}
    
    DGNPLATFORM_EXPORT                                 IPropMask ();

public:

    DGNPLATFORM_EXPORT virtual                          ~IPropMask ();

    DGNPLATFORM_EXPORT StatusInt                        SetBitMaskForCategory (BitMaskP pBitMaskIn, uint16_t categoryIdIn);
    DGNPLATFORM_EXPORT StatusInt                        GetBitMaskByCategory (BitMaskH ppBitMaskOut, uint16_t categoryIdIn) const;
    DGNPLATFORM_EXPORT bool                             SetAnyBit ();
    DGNPLATFORM_EXPORT StatusInt                        SetCategoryByBitArray (uint16_t pArrayIn[], int nValidBits, uint16_t categoryId);
    DGNPLATFORM_EXPORT StatusInt                        SetBitByCategoryAndPosition (uint16_t categoryId, int bitPosition, bool bitValue);
    DGNPLATFORM_EXPORT void                             ClearAllBits ();
    DGNPLATFORM_EXPORT int                              GetMaxCategories () const {return _GetMaxCategories ();}
    DGNPLATFORM_EXPORT StatusInt                        Initialize ();
    DGNPLATFORM_EXPORT StatusInt                        Release ();

};  // IPropMask

END_BENTLEY_NAMESPACE
