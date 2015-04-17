/*-------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/bitmask/PropMask.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Tools/PropMask.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
IPropMask::IPropMask ()
    {
    m_bitMaskArray = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
IPropMask::~IPropMask ()
    {
    Release ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::Initialize ()
    {
    m_bitMaskArray = (BitMaskH) malloc (_GetMaxCategories () * sizeof (BitMaskP));
    for (int i = 0; i < _GetMaxCategories (); i++)
        m_bitMaskArray[i] = BitMask::Create ( false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::Release ()
    {
    if (NULL == m_bitMaskArray)
        return SUCCESS;

    for (int i = 0; i < _GetMaxCategories (); i++)
        {
        if (NULL != m_bitMaskArray[i])
            (m_bitMaskArray[i])->Free ();
        }

    free (m_bitMaskArray);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
IPropMask&      IPropMask::Copy
(
IPropMask const*    from
)
    {
    int         iCategory;

    for (iCategory = 0; iCategory < _GetMaxCategories (); iCategory++)
        {
        if (NULL != m_bitMaskArray[iCategory])
            (m_bitMaskArray[iCategory])->Free ();

        m_bitMaskArray[iCategory] = BitMask::Clone (*(from->m_bitMaskArray[iCategory]));
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::_SetBitMaskForCategory
(
BitMaskP                pBitMaskIn,
uint16_t                categoryIdIn
)
    {
    if (0 > categoryIdIn || _GetMaxCategories () <= categoryIdIn)
        { BeAssert (false && "BitMask category out of range"); return ERROR; }

    BitMaskP  pBitMask = m_bitMaskArray[categoryIdIn];

    if (pBitMask)
        pBitMask->Free ();

    pBitMask = pBitMaskIn;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::_GetBitMaskByCategory
(
BitMaskH            ppBitMaskOut,
uint16_t            categoryIdIn
) const
    {
    if (0 > categoryIdIn || _GetMaxCategories () <= categoryIdIn)
        { BeAssert (false && "BitMask category out of range"); return ERROR; }

    BitMaskP pBitMask = m_bitMaskArray[categoryIdIn];

    if (ppBitMaskOut)
        *ppBitMaskOut = pBitMask;

    return NULL != pBitMask ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IPropMask::_SetAnyBit
(
void
)
    {
    if (NULL != m_bitMaskArray)
        {
        uint16_t    iCategory;
        BitMaskP    pBitMask;

        for (iCategory = 0; iCategory < _GetMaxCategories (); iCategory++)
            {
            if (SUCCESS == _GetBitMaskByCategory (&pBitMask, iCategory))
                if (pBitMask->AnyBitSet ())
                    return true;
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::_SetCategoryByBitArray
(
uint16_t        pArrayIn[],
int             nValidBits,
uint16_t        categoryId
)
    {
    BitMaskP    pBitMask = NULL;
    StatusInt   status;

    if (SUCCESS == (status = _GetBitMaskByCategory (&pBitMask, categoryId)))
        {
        status = pBitMask->SetFromBitArray( nValidBits,  pArrayIn);
        }
    else
        {
        if (NULL != (pBitMask = BitMask::Create (false)) &&
            SUCCESS == pBitMask->SetFromBitArray (nValidBits, pArrayIn))
            {
            if (pBitMask->AnyBitSet ())
                status = _SetBitMaskForCategory (pBitMask, categoryId);
            else
                pBitMask->Free ();
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::_SetBitByCategoryAndPosition
(
uint16_t            categoryID,
int                 bitPosition,
bool                bitValue
)
    {
    StatusInt   status;
    BitMaskP    pBitMask = NULL;

    if (SUCCESS == (status = _GetBitMaskByCategory (&pBitMask, categoryID)))
        {
        status = pBitMask->SetBit ( bitPosition,  bitValue);
        }
    else
        {
        if (bitValue)
            {
            if (  NULL != (pBitMask = BitMask::Create ( false))
               && SUCCESS == (status = _SetBitMaskForCategory (pBitMask, categoryID))
               )
                {
                status = pBitMask->SetBit ( bitPosition,  bitValue);
                }
            }
        else
            {
            status = SUCCESS;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sridhar.margam                  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            IPropMask::_ClearAllBits
(
void
)
    {
    if (NULL != m_bitMaskArray)
        {
        uint16_t    iCategory;
        BitMaskP    pBitMask;

        for (iCategory = 0; iCategory < _GetMaxCategories (); iCategory++)
            {
            if (SUCCESS == _GetBitMaskByCategory (&pBitMask, iCategory))
                pBitMask->SetAll ( false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::SetBitMaskForCategory (BitMaskP pBitMaskIn, uint16_t categoryIdIn)
    {
    return _SetBitMaskForCategory (pBitMaskIn, categoryIdIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::GetBitMaskByCategory (BitMaskH ppBitMaskOut, uint16_t categoryIdIn) const
    {
    return _GetBitMaskByCategory (ppBitMaskOut, categoryIdIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IPropMask::SetAnyBit ()
    {
    return _SetAnyBit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::SetCategoryByBitArray (uint16_t pArrayIn[], int nValidBits, uint16_t categoryId)
    {
    return _SetCategoryByBitArray (pArrayIn, nValidBits, categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPropMask::SetBitByCategoryAndPosition (uint16_t categoryId, int bitPosition, bool bitValue)
    {
    return _SetBitByCategoryAndPosition (categoryId, bitPosition, bitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            IPropMask::ClearAllBits ()
    {
    _ClearAllBits ();
    }

