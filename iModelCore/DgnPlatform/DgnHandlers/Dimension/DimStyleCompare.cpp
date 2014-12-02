/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimStyleCompare.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnHandlers/DimensionElem.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#ifndef AUTODIMLOWTOL_TOL
#define AUTODIMLOWTOL_TOL 1.0e-6
#endif

#ifndef AUTODIMHITOL_TOL
#define AUTODIMHITOL_TOL  1.0e-6
#endif

#define DIMCOMPARE_GETBITPOSITION(p)                  ( (p % 100) - 1)
#define DIMCOMPARE_GETCATEGORY(g)                     ((DimStyle_Category) ( ( ((int)g) / ((int)100) ) - 1))

const UInt16 DimStylePropMask::s_templateCategoryId[24] = {
    DimStylePropMask::DIMSTYLE_CATEGORY_Template0Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template1Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template2Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template3Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template4Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template5Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template6Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template7Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template8Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template9Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template10Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template11Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template12Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template13Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template14Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template15Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template16Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template17Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template18Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template19Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template20Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template21Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template22Attributes,
    DimStylePropMask::DIMSTYLE_CATEGORY_Template23Attributes
    };
#define DIMSTYLE_CATEGORY_NUMBER                      34
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStylePropMask::DimStylePropMask ()
    {
    memset (m_pBitMasks, 0, sizeof(m_pBitMasks));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStylePropMask::~DimStylePropMask ()
    {
    for (size_t iCategory = 0; iCategory < DIMSTYLE_CATEGORY_COUNT; iCategory++)
        {
        if (NULL != m_pBitMasks[iCategory])
            (m_pBitMasks[iCategory])->Free ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStylePropMaskPtr DimStylePropMask::CreatePropMask ()
    {
    return new DimStylePropMask();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimStylePropMask::From (DimStylePropMaskCR inMask)
    {
    for (size_t iCategory = 0; iCategory < DIMSTYLE_CATEGORY_COUNT; iCategory++)
        {
        if (NULL != m_pBitMasks[iCategory])
            {
            (m_pBitMasks[iCategory])->Free ();
            (m_pBitMasks[iCategory]) = NULL;
            }

        if (inMask.m_pBitMasks[iCategory])
            m_pBitMasks[iCategory] = BitMask::Clone (*(inMask.m_pBitMasks[iCategory]));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStylePropMask::SetBitMaskP (BitMaskP bitMaskIn, DimStyle_Category categoryID)
    {
    if (0 > categoryID || DIMSTYLE_CATEGORY_NUMBER <= categoryID)
        { BeAssert (false && "BitMask category out of range"); return ERROR; }

    BitMaskH  ppBitMask = &m_pBitMasks[categoryID];

    if (*ppBitMask)
        (*ppBitMask)->Free ();

    *ppBitMask = bitMaskIn;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BitMaskP        DimStylePropMask::GetBitMaskP (DimStyle_Category categoryID)
    {
    if (0 > categoryID || DIMSTYLE_CATEGORY_COUNT <= categoryID)
        { BeAssert (false && "BitMask category out of range"); return NULL; }

    return m_pBitMasks[categoryID];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BitMaskCP       DimStylePropMask::GetBitMaskCP (DimStyle_Category categoryID) const
    {
    if (0 > categoryID || DIMSTYLE_CATEGORY_COUNT <= categoryID)
        { BeAssert (false && "BitMask category out of range"); return NULL; }

    return m_pBitMasks[categoryID];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimStylePropMask::AnyBitSet () const
    {
    for (UInt16 iCategory = 0; iCategory < DIMSTYLE_CATEGORY_NUMBER; iCategory++)
        {
        BitMaskCP    bitMask = GetBitMaskCP ((DimStyle_Category) iCategory);

        if (NULL != bitMask && bitMask->AnyBitSet ())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStylePropMask::SetBitMaskByBitArray (UInt16 pArrayIn[], int nValidBits, DimStyle_Category categoryID)
    {
    BitMaskP    pBitMask = GetBitMaskP (categoryID);
    
    if (NULL != pBitMask)
        return pBitMask->SetFromBitArray( nValidBits,  pArrayIn);

    if (NULL == (pBitMask = BitMask::Create (false)) ||
        SUCCESS != pBitMask->SetFromBitArray (nValidBits, pArrayIn))
        {
        return ERROR;
        }

    if (pBitMask->AnyBitSet ())
        {
        if (SUCCESS == SetBitMaskP (pBitMask, categoryID))
            return SUCCESS;
        }

    pBitMask->Free ();

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
*
* Note: This function is more efficient than DimStylePropMask::SetPropertyBit when
* all the properties are being compared. After the bit array is filled, it can be
* flushed into a bitmask by a single operation.
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::DimStyleComparer::SetPropertyBitInBitArray
(
UInt16                      pBitArray[],
DimStyleProp                dimStyleProp,
bool                        bitValue
)
    {
    int bitPositionInMask   = DIMCOMPARE_GETBITPOSITION (dimStyleProp);
    int arrayIndex  = bitPositionInMask / 16;
    int bitPositionAbsolute = bitPositionInMask % 16;

    if (bitValue)
        pBitArray[arrayIndex] |= (bitValue << bitPositionAbsolute);
    else
        pBitArray[arrayIndex] &= ~(bitValue << bitPositionAbsolute);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStylePropMask::SetBitByCategoryAndPosition (DimStyle_Category categoryID, int bitPosition, bool bitValue)
    {
    BitMaskP    pBitMask = GetBitMaskP (categoryID);

    // If we already have a bitmask just set the bit
    if (NULL != pBitMask)
        return pBitMask->SetBit ( bitPosition,  bitValue);

    // if there is no bitMask for the category, turning bit off is a no-op
    if ( ! bitValue)
        return SUCCESS;

    // We need to create the bitmask and set the bit
    pBitMask = BitMask::Create ( false);
    if (NULL == pBitMask)
        return ERROR;

    if (SUCCESS != SetBitMaskP (pBitMask, categoryID))
        {
        pBitMask->Free ();
        return ERROR;
        }

    return pBitMask->SetBit ( bitPosition,  bitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void        DimStylePropMask::ClearAllBits ()
    {
    UInt16      iCategory;

    for (iCategory = 0; iCategory < DIMSTYLE_CATEGORY_COUNT; iCategory++)
        {
        BitMaskP    pBitMask = GetBitMaskP ((DimStyle_Category) iCategory);

        if (NULL == pBitMask)
            continue;

        pBitMask->SetAll ( false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStylePropMask::ClearTemplateBits (int templateIndex)
    {
    if (0 > templateIndex)
        {
        BeAssert (false && "Dimension template with negative index found");
        return ERROR;
        }

    DimStyle_Category   categoryID  = (DimStyle_Category) s_templateCategoryId[templateIndex];

    return SetBitMaskP (NULL, categoryID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStylePropMask::SetPropertyBit (DimStyleProp dimStyleProp, bool bitValue)
    {
    StatusInt       status    = SUCCESS;
    StatusInt       tmpStatus = SUCCESS;

    /*------------------------------------------------------------------------------
        The consolidation properties must be special cased.  The majority of the
        properties are handled in the default case.
    ------------------------------------------------------------------------------*/
    switch (dimStyleProp)
        {
        case DIMSTYLE_PROP_Text_FrameType_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Text_Box_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Text_Capsule_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_Format_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_AltFormat_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_SecFormat_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_AltSecFormat_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_Value_UnitMaster_ONEUNIT:
        case DIMSTYLE_PROP_Value_UnitSub_ONEUNIT:
            {
            status = SetPropertyBit (DIMSTYLE_PROP_Value_Unit_UNITS, bitValue);

            break;
            }
        case DIMSTYLE_PROP_Value_SecUnitMaster_ONEUNIT:
        case DIMSTYLE_PROP_Value_SecUnitSub_ONEUNIT:
            {
            status = SetPropertyBit (DIMSTYLE_PROP_Value_UnitSec_UNITS, bitValue);

            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT, bitValue)))
                status = tmpStatus;
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT, bitValue)))
                status = tmpStatus;

            break;
            }
        case DIMSTYLE_PROP_BallAndChain_Mode_INTEGER:
            {
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_BallAndChain_IsActive_BOOLINT, bitValue)))
                status = tmpStatus;
            break;
            }
        case DIMSTYLE_PROP_General_FitOption_INTEGER:
            {
            // have to support shields backward compatibility
            if (SUCCESS != (tmpStatus = SetPropertyBit (DIMSTYLE_PROP_Terminator_Mode_INTEGER, bitValue)))
                status = tmpStatus;

            break;
            }
        default:
            {
            /*----------------------------------------------------------------------
               This is the COMMON CASE
            ----------------------------------------------------------------------*/
            int                 bitPosition     = DIMCOMPARE_GETBITPOSITION (dimStyleProp);
            DimStyle_Category   categoryID      = DIMCOMPARE_GETCATEGORY  (dimStyleProp);

            status = SetBitByCategoryAndPosition (categoryID, bitPosition, bitValue);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimStylePropMask::MapTemplateBitPosition (int& bitPosition, DimStyleProp dimStyleProp)
    {
    switch (dimStyleProp)
        {
        case DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG:
            bitPosition = 0;
            break;
        case DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG:
            bitPosition = 1;
            break;
        case DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG:
            bitPosition = 2;
            break;
        case DIMSTYLE_PROP_General_Stacked_TEMPLATEFLAG:
            bitPosition = 3;
            break;
        case DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG:
            bitPosition = 4;
            break;
        case DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG:
            bitPosition = 5;
            break;
        case DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG:
            bitPosition = 6;
            break;
        case DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG:
            bitPosition = 7;
            break;
        case DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG:
            bitPosition = 8;
            break;
        case DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG:
            bitPosition = 9;
            break;
        case DIMSTYLE_PROP_Text_ArcLengthSymbol_TEMPLATEFLAG:
            bitPosition = 10;
            break;
        case DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG:
            bitPosition = 11;
            break;
        case DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG:
            bitPosition = 12;
            break;
        case DIMSTYLE_PROP_ExtensionLine_AngleChordAlign_TEMPLATEFLAG:
            bitPosition = 13;
            break;
        default:
            // should never happen
            BeAssert (false);
            bitPosition = 0;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
*
* Note: This function is more efficient than DimStylePropMask::SetTemplateBit when
* all the template properties are being compared. After the bit array is filled, it
* can be flushed into a bitmask by a single operation.
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::DimStyleComparer::SetTemplateBitInBitArray
(
UInt16                      pBitArray[],
DimStyleProp                dimStyleProp,
bool                        bitValue
)
    {
    if (DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG == dimStyleProp)
        {
        // VerticalOpts is the consolidation of these other two properties
        SetTemplateBitInBitArray (pBitArray, DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG, bitValue);
        SetTemplateBitInBitArray (pBitArray, DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG, bitValue);
        }
    else
        {
        int arrayIndex          = 0;
        int bitPositionInMask   = 0;
        int bitPositionAbsolute = 0;

        DimStylePropMask::MapTemplateBitPosition (bitPositionInMask, dimStyleProp);

        arrayIndex  = (bitPositionInMask) / 16;
        bitPositionAbsolute = bitPositionInMask - (arrayIndex * 16);

        if (bitValue)
            pBitArray[arrayIndex] |= (bitValue << bitPositionAbsolute);
        else
            pBitArray[arrayIndex] &= ~(bitValue << bitPositionAbsolute);
        }

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStylePropMask::SetTemplateBit (DimStyleProp dimStyleProp, bool bitValue, DimensionType dimensionType)
    {
    if ( (DimensionType::None == dimensionType) || (dimensionType > DimensionType::MaxThatHasTemplate) )
        {
        BeAssert (false && "Dimension type out of range requested");
        return false;
        }

    if (DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG == dimStyleProp)
        {
        StatusInt   status;
        StatusInt   retStatus = SUCCESS;

        // VerticalOpts is the consolidation of these other two properties
        if (SUCCESS != (status = SetTemplateBit (DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG, bitValue, dimensionType)))
            retStatus = status;
        if (SUCCESS != (status = SetTemplateBit (DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG, bitValue, dimensionType)))
            retStatus = status;

        return retStatus;
        }
    else
        {
        int                 bitPosition = 0;
        int                 templateIndex   = static_cast<int>(dimensionType) - 1;
        DimStyle_Category   categoryID  = (DimStyle_Category) s_templateCategoryId[templateIndex];

        DimStylePropMask::MapTemplateBitPosition (bitPosition, dimStyleProp);

        return SetBitByCategoryAndPosition (categoryID, bitPosition, bitValue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool    DimStylePropMask::GetPropertyBit (DimStyleProp dimStyleProp) const
    {
    bool bitValue = false;

    /*------------------------------------------------------------------------------
        The consolidation properties must be special cased.  The majority of the
        properties are handled in the default case.
    ------------------------------------------------------------------------------*/
    switch (dimStyleProp)
        {
        case DIMSTYLE_PROP_Text_FrameType_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Text_Box_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Text_Capsule_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_Format_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_AltFormat_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_SecFormat_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_AltSecFormat_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT);

            break;
            }
        case DIMSTYLE_PROP_Value_UnitMaster_ONEUNIT:
        case DIMSTYLE_PROP_Value_UnitSub_ONEUNIT:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_Unit_UNITS);

            break;
            }
        case DIMSTYLE_PROP_Value_SecUnitMaster_ONEUNIT:
        case DIMSTYLE_PROP_Value_SecUnitSub_ONEUNIT:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_UnitSec_UNITS);

            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT)  ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT) ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT)   ||
                       GetPropertyBit (DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT);
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_Mode_INTEGER:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_BallAndChain_IsActive_BOOLINT);
            break;
            }
        case DIMSTYLE_PROP_General_FitOption_INTEGER:
            {
            // have to support shields backward compatibility
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Terminator_Mode_INTEGER);

            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruArrow_BOOLINT:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT);
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruDot_BOOLINT:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT);
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruOrigin_BOOLINT:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT);
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruStroke_BOOLINT:
            {
            bitValue = GetPropertyBit (DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT);
            break;
            }

        default:
            {
            /*----------------------------------------------------------------------
               This is the COMMON CASE
            ----------------------------------------------------------------------*/
            DimStyle_Category   categoryID      = DIMCOMPARE_GETCATEGORY (dimStyleProp);
            int                 bitPosition     = DIMCOMPARE_GETBITPOSITION (dimStyleProp);
            BitMaskCP           pBitMask        = GetBitMaskCP (categoryID);

            if (NULL != pBitMask)
                {
                bool    boolIntVal = false;

                boolIntVal = pBitMask->Test( bitPosition);
                bitValue = TO_BOOL (boolIntVal);
                }
            }
        }

    return bitValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool    DimStylePropMask::GetTemplateBit (DimStyleProp dimStyleProp, DimensionType dimensionType) const
    {
    bool bitValue = false;

    if ( (DimensionType::None == dimensionType) || (dimensionType > DimensionType::MaxThatHasTemplate) )
        {
        BeAssert (false && "Dimension type out of range requested");
        return false;
        }

    if (DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG == dimStyleProp)
        {
        // VerticalOpts is the consolidation of these other two properties
        bitValue = GetTemplateBit (DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG, dimensionType) ||
                   GetTemplateBit (DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG, dimensionType);
        }
    else
        {
        int                 bitPosition     = 0;
        int                 templateIndex   = static_cast<int>(dimensionType) - 1;
        DimStyle_Category   categoryID  = (DimStyle_Category) s_templateCategoryId[templateIndex];
        BitMaskCP           pBitMask    = GetBitMaskCP (categoryID);

        DimStylePropMask::MapTemplateBitPosition (bitPosition, dimStyleProp);

        if (NULL != pBitMask)
            {
            bool    boolIntVal = false;

            boolIntVal = pBitMask->Test( bitPosition);
            bitValue = TO_BOOL (boolIntVal);
            }

        }

    return bitValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    dimStyleCompare_areDoublesEqual
(
double          value1,
double          value2,
double          tol
)
    {
    return  ! (fabs (value1 - value2) > tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    dimStyleCompare_areDistancesEqual
(
double          value1,
double          value2,
double          uorScale,
double          tol
)
    {
    return dimStyleCompare_areDoublesEqual (uorScale * value1, value2, tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    dimStyleCompare_areDimUnitsEqual
(
DimUnits const& dimUnits1,
DimUnits const& dimUnits2
)
    {
    UnitDefinition unit1 = adim_getUnitDefFromDimUnit (dimUnits1);
    UnitDefinition unit2 = adim_getUnitDefFromDimUnit (dimUnits2);

    return (0 == unit1.CompareByScale (unit2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool areSameFont (UInt32 fontNo1, DgnModelP mr1, UInt32 fontNo2, DgnModelP mr2)
    {
    DgnProjectP file1 = &mr1->GetDgnProject();
    DgnProjectP file2 = &mr2->GetDgnProject();

    if (file1 == file2)
        return  fontNo1 == fontNo2;

    DgnFontCP font1 = DgnFontManager::ResolveFont (fontNo1, *file1, DGNFONTVARIANT_DontCare);
    DgnFontCP font2 = DgnFontManager::ResolveFont (fontNo2, *file2, DGNFONTVARIANT_DontCare);

    return  (font1 == font2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareBallAndChainAttributes ()
    {
    UInt16      bitArray[2];

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER, m_dimStyle1.m_data.ad6.bnc.flags.alignment != m_dimStyle2.m_data.ad6.bnc.flags.alignment);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER, m_dimStyle1.m_data.ad6.bnc.flags.terminator != m_dimStyle2.m_data.ad6.bnc.flags.terminator);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER, m_dimStyle1.m_data.ad6.bnc.flags.chainType != m_dimStyle2.m_data.ad6.bnc.flags.chainType);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT, m_dimStyle1.m_data.ad6.bnc.flags.elbow != m_dimStyle2.m_data.ad6.bnc.flags.elbow);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT, m_dimStyle1.m_data.ad6.bnc.flags.noDockOnDimLine != m_dimStyle2.m_data.ad6.bnc.flags.noDockOnDimLine);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_BallAndChainAttributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareExtensionLineAttributes ()
    {
    UInt16      bitArray[2];

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Color_COLOR, ! ColorUtil::ElementColorsAreEqual (m_dimStyle1.m_data.ad5.altSymb.color, m_cache1, m_dimStyle2.m_data.ad5.altSymb.color, m_cache2, true));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad4.witness_extend, m_dimStyle2.m_data.ad4.witness_extend, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Join_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.joiner != m_dimStyle2.m_data.ad4.ext_dimflg.joiner);
    
    bool samelineStyle = 0 == LineStyleManager::GetStringFromNumber (m_dimStyle1.m_data.ad5.altSymb.style, m_dimStyle1.GetDgnProject()).CompareTo(LineStyleManager::GetStringFromNumber (m_dimStyle2.m_data.ad5.altSymb.style, m_dimStyle2.GetDgnProject()));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_LineStyle_LINESTYLE, !samelineStyle);

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad4.witness_offset, m_dimStyle2.m_data.ad4.witness_offset, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT, m_dimStyle1.m_data.ad5.flags.altColor != m_dimStyle2.m_data.ad5.flags.altColor);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT, m_dimStyle1.m_data.ad5.flags.altStyle != m_dimStyle2.m_data.ad5.flags.altStyle);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT, m_dimStyle1.m_data.ad5.flags.altWeight != m_dimStyle2.m_data.ad5.flags.altWeight);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT,  m_dimStyle1.m_data.ad1.mode.witness !=  m_dimStyle2.m_data.ad1.mode.witness);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Weight_WEIGHT, m_dimStyle1.m_data.ad5.altSymb.weight != m_dimStyle2.m_data.ad5.altSymb.weight);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_ExtensionLineAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareGeneralAttributes ()
    {
    UInt16      bitArray[2];
    double      uorScale = 1.0;

#if defined (REMOVE_DEAD_CODE)
    modelInfo_getUorScaleBetweenModels (&uorScale, m_cache1, m_cache2);
#endif

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_Alignment_INTEGER, m_dimStyle1.m_data.ad1.mode.parallel != m_dimStyle2.m_data.ad1.mode.parallel);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad4.dimcen, m_dimStyle2.m_data.ad4.dimcen, uorScale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_Color_COLOR, ! ColorUtil::ElementColorsAreEqual (m_dimStyle1.m_data.ad4.dim_color, m_cache1, m_dimStyle2.m_data.ad4.dim_color, m_cache2, true));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_DimensionScale_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad4.dimension_scale, m_dimStyle2.m_data.ad4.dimension_scale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_Font_FONT, !areSameFont (m_dimStyle1.m_data.ad4.dimfont, m_cache1, m_dimStyle2.m_data.ad4.dimfont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_IgnoreLevelSymbology_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.noLevelSymb != m_dimStyle2.m_data.ad4.ext_dimflg.noLevelSymb);
    
    bool samelineStyle = 0 == LineStyleManager::GetStringFromNumber (m_dimStyle1.m_data.ad5.altSymb.style, m_dimStyle1.GetDgnProject()).CompareTo(LineStyleManager::GetStringFromNumber (m_dimStyle2.m_data.ad5.altSymb.style, m_dimStyle2.GetDgnProject()));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_LineStyle_LINESTYLE, !samelineStyle);
    
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_OverrideColor_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.colorOverride != m_dimStyle2.m_data.ad4.ext_dimflg.colorOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_OverrideLineStyle_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.styleOverride != m_dimStyle2.m_data.ad4.ext_dimflg.styleOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_OverrideWeight_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.weightOverride != m_dimStyle2.m_data.ad4.ext_dimflg.weightOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_RadialMode_INTEGER, m_dimStyle1.m_data.ad5.flags.radialMode != m_dimStyle2.m_data.ad5.flags.radialMode);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_RelativeDimLine_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.relDimLine != m_dimStyle2.m_data.ad4.ext_dimflg.relDimLine);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_StackOffset_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad4.stack_offset, m_dimStyle2.m_data.ad4.stack_offset, uorScale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_General_Weight_WEIGHT, m_dimStyle1.m_data.ad4.dim_weight != m_dimStyle2.m_data.ad4.dim_weight);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_GeneralAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareMultiLineNotesAttributes ()
    {
    UInt16      bitArray[2];

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_MLNote_Justification_INTEGER, m_dimStyle1.m_data.ad5.flags.multiJust != m_dimStyle2.m_data.ad5.flags.multiJust);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT, m_dimStyle1.m_data.ad5.flags.multiLeader != m_dimStyle2.m_data.ad5.flags.multiLeader);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_MultiLineAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::ComparePlacementAttributes ()
    {
    UInt16      bitArray[2];

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Placement_CompatibleV3_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.compatible!= m_dimStyle2.m_data.ad4.ext_dimflg.compatible);
#if defined (BEIJING_DGNPLATFORM_WIP_JS) //TODO WIP_JS
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Placement_Level_LEVEL, !mdlLevel_haveSameName (m_dimStyle1.m_data.ad1.level, modelRef1, m_dimStyle2.m_data.ad1.level, m_cache2));
#endif
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Placement_OverrideLevel_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.levelOverride != m_dimStyle2.m_data.ad4.ext_dimflg.levelOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Placement_TextPosition_INTEGER,
                             (m_dimStyle1.m_data.ad1.mode.automan != m_dimStyle2.m_data.ad1.mode.automan) ||
                             (m_dimStyle1.m_data.ad4.ext_dimflg.semiauto != m_dimStyle2.m_data.ad4.ext_dimflg.semiauto));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Placement_UseReferenceScale_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.useRefUnits != m_dimStyle2.m_data.ad4.ext_dimflg.useRefUnits);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_PlacementAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareSymbolAttributes ()
    {
    UInt16      bitArray[2];

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_DiameterChar_CHAR, m_dimStyle1.m_data.ad2.diam != m_dimStyle2.m_data.ad2.diam);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_DiameterFont_FONT, !areSameFont (m_dimStyle1.m_data.ad2.diamfont, m_cache1, m_dimStyle2.m_data.ad2.diamfont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_DiameterType_INTEGER, m_dimStyle1.m_data.ad6.flags.diameter != m_dimStyle2.m_data.ad6.flags.diameter);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.lower_prefix != m_dimStyle2.m_data.ad4.dimtxt.lower_prefix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.lower_suffix != m_dimStyle2.m_data.ad4.dimtxt.lower_suffix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.main_prefix!= m_dimStyle2.m_data.ad4.dimtxt.main_prefix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.main_suffix != m_dimStyle2.m_data.ad4.dimtxt.main_suffix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR, m_dimStyle1.m_data.ad6.pmChar != m_dimStyle2.m_data.ad6.pmChar);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER, m_dimStyle1.m_data.ad6.flags.plusMinus != m_dimStyle2.m_data.ad6.flags.plusMinus);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_PrefixChar_CHAR, m_dimStyle1.m_data.ad6.preChar!= m_dimStyle2.m_data.ad6.preChar);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_PrefixFont_FONT, !areSameFont (m_dimStyle1.m_data.ad6.preFont, m_cache1, m_dimStyle2.m_data.ad6.preFont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_PrefixType_INTEGER, m_dimStyle1.m_data.ad6.flags.prefix != m_dimStyle2.m_data.ad6.flags.prefix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_SuffixChar_CHAR, m_dimStyle1.m_data.ad6.sufChar != m_dimStyle2.m_data.ad6.sufChar);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_SuffixFont_FONT, !areSameFont (m_dimStyle1.m_data.ad6.sufFont, m_cache1, m_dimStyle2.m_data.ad6.sufFont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_SuffixType_INTEGER, m_dimStyle1.m_data.ad6.flags.suffix != m_dimStyle2.m_data.ad6.flags.suffix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.tol_prefix != m_dimStyle2.m_data.ad4.dimtxt.tol_prefix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.tol_suffix != m_dimStyle2.m_data.ad4.dimtxt.tol_suffix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.upper_prefix!= m_dimStyle2.m_data.ad4.dimtxt.upper_prefix);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR, m_dimStyle1.m_data.ad4.dimtxt.upper_suffix != m_dimStyle2.m_data.ad4.dimtxt.upper_suffix);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_SymbolAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareTerminatorAttributes ()
    {
    UInt16      bitArray[4];

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_ArrowChar_CHAR, m_dimStyle1.m_data.ad2.arrhead != m_dimStyle2.m_data.ad2.arrhead);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_ArrowFont_FONT, !areSameFont (m_dimStyle1.m_data.ad2.arrfont, m_cache1, m_dimStyle2.m_data.ad2.arrfont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_ArrowType_INTEGER, m_dimStyle1.m_data.ad6.flags.arrow!= m_dimStyle2.m_data.ad6.flags.arrow);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Color_COLOR, ! ColorUtil::ElementColorsAreEqual (m_dimStyle1.m_data.ad5.termSymb.color, m_cache1, m_dimStyle2.m_data.ad5.termSymb.color, m_cache2, true));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_DotChar_CHAR, m_dimStyle1.m_data.ad4.bowtie_symbol != m_dimStyle2.m_data.ad4.bowtie_symbol);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_DotFont_FONT, !areSameFont (m_dimStyle1.m_data.ad4.bowtie_font, m_cache1, m_dimStyle2.m_data.ad4.bowtie_font, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_DotType_INTEGER, m_dimStyle1.m_data.ad6.flags.dot != m_dimStyle2.m_data.ad6.flags.dot);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Height_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.termHeight, m_dimStyle2.m_data.ad5.termHeight, m_doubleTol));
    
    bool samelineStyle = 0 == LineStyleManager::GetStringFromNumber (m_dimStyle1.m_data.ad5.altSymb.style, m_dimStyle1.GetDgnProject()).CompareTo(LineStyleManager::GetStringFromNumber (m_dimStyle2.m_data.ad5.altSymb.style, m_dimStyle2.GetDgnProject()));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE, !samelineStyle);

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad4.text_margin, m_dimStyle2.m_data.ad4.text_margin, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT, m_dimStyle1.m_data.ad6.flags.noLineThruArrowTerm != m_dimStyle2.m_data.ad6.flags.noLineThruArrowTerm);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT, m_dimStyle1.m_data.ad6.flags.noLineThruDotTerm != m_dimStyle2.m_data.ad6.flags.noLineThruDotTerm);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT, m_dimStyle1.m_data.ad6.flags.noLineThruOriginTerm != m_dimStyle2.m_data.ad6.flags.noLineThruOriginTerm);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT, m_dimStyle1.m_data.ad6.flags.noLineThruStrokeTerm!= m_dimStyle2.m_data.ad6.flags.noLineThruStrokeTerm);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_OriginChar_CHAR, m_dimStyle1.m_data.ad2.comorg!= m_dimStyle2.m_data.ad2.comorg);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_OriginFont_FONT, !areSameFont (m_dimStyle1.m_data.ad2.cofont, m_cache1, m_dimStyle2.m_data.ad2.cofont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_OriginType_INTEGER, m_dimStyle1.m_data.ad6.flags.origin != m_dimStyle2.m_data.ad6.flags.origin);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT, m_dimStyle1.m_data.ad5.flags.termColor != m_dimStyle2.m_data.ad5.flags.termColor);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT, m_dimStyle1.m_data.ad5.flags.termStyle!= m_dimStyle2.m_data.ad5.flags.termStyle);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT, m_dimStyle1.m_data.ad5.flags.termWeight != m_dimStyle2.m_data.ad5.flags.termWeight);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER, m_dimStyle1.m_data.ad4.ext_dimflg.arrowhead != m_dimStyle2.m_data.ad4.ext_dimflg.arrowhead);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_StrokeChar_CHAR, m_dimStyle1.m_data.ad2.oblique != m_dimStyle2.m_data.ad2.oblique);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_StrokeFont_FONT, !areSameFont (m_dimStyle1.m_data.ad2.oblqfont, m_cache1, m_dimStyle2.m_data.ad2.oblqfont, m_cache2));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_StrokeType_INTEGER, m_dimStyle1.m_data.ad6.flags.stroke!= m_dimStyle2.m_data.ad6.flags.stroke);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Weight_WEIGHT, m_dimStyle1.m_data.ad5.termSymb.weight!= m_dimStyle2.m_data.ad5.termSymb.weight);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Width_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.termWidth, m_dimStyle2.m_data.ad5.termWidth, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT, m_dimStyle1.m_data.ad5.flags.uniformCellScale != m_dimStyle2.m_data.ad5.flags.uniformCellScale);

    if (bitArray[0] || bitArray[1] || bitArray[2] || bitArray[3])
        m_differences->SetBitMaskByBitArray (bitArray, 64, DimStylePropMask::DIMSTYLE_CATEGORY_TerminatorAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareTextAttributes ()
    {
    UInt16      bitArray[3];
    double      uorScale = 1.0;

#if defined (REMOVE_DEAD_CODE)
    modelInfo_getUorScaleBetweenModels (&uorScale, m_cache1, m_cache2);
#endif

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_AutoLift_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.noAutoTextLift!= m_dimStyle2.m_data.ad4.ext_dimflg.noAutoTextLift);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Box_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.boxtext!= m_dimStyle2.m_data.ad4.ext_dimflg.boxtext);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Capsule_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.captext!= m_dimStyle2.m_data.ad4.ext_dimflg.captext);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Color_COLOR, ! ColorUtil::ElementColorsAreEqual (m_dimStyle1.m_data.ad4.dimtxt_color, m_cache1, m_dimStyle2.m_data.ad4.dimtxt_color, m_cache2, true));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_DecimalComma_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.decimal_comma!= m_dimStyle2.m_data.ad4.ext_dimflg.decimal_comma);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Embed_BOOLINT, m_dimStyle1.m_data.ad1.params.embed != m_dimStyle2.m_data.ad1.params.embed);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Font_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.fontOverride!= m_dimStyle2.m_data.ad4.ext_dimflg.fontOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Height_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad2.txheight, m_dimStyle2.m_data.ad2.txheight, uorScale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Horizontal_BOOLINT, m_dimStyle1.m_data.ad1.params.horizontal!= m_dimStyle2.m_data.ad1.params.horizontal);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.textMarginH, m_dimStyle2.m_data.ad5.textMarginH, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_LeadingZero_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.leading_zero != m_dimStyle2.m_data.ad4.ext_dimflg.leading_zero);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OmitLeadingDelimiter_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.omitLeadingDelimeter != m_dimStyle2.m_data.ad4.ext_dimflg.omitLeadingDelimeter);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OverrideColor_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.textColorOverride != m_dimStyle2.m_data.ad4.ext_dimflg.textColorOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.textSizeOverride!= m_dimStyle2.m_data.ad4.ext_dimflg.textSizeOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.textWeightOverride != m_dimStyle2.m_data.ad4.ext_dimflg.textWeightOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT, m_dimStyle1.m_data.ad5.flags.useWidth!= m_dimStyle2.m_data.ad5.flags.useWidth);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_SecLeadingZero_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.leading_zero2 != m_dimStyle2.m_data.ad4.ext_dimflg.leading_zero2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT, m_dimStyle1.m_data.ad1.params.dual != m_dimStyle2.m_data.ad1.params.dual && m_dimStyle1.m_data.ad5.flags.useSecUnits != m_dimStyle2.m_data.ad5.flags.useSecUnits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Underline_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.underlineText!= m_dimStyle2.m_data.ad4.ext_dimflg.underlineText);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.textMarginV, m_dimStyle2.m_data.ad5.textMarginV, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Weight_WEIGHT, m_dimStyle1.m_data.ad4.dimtxt_weight != m_dimStyle2.m_data.ad4.dimtxt_weight);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Width_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad5.textWidth, m_dimStyle2.m_data.ad5.textWidth, uorScale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER, m_dimStyle1.m_data.ad5.flags.stackedFractionAlign != m_dimStyle2.m_data.ad5.flags.stackedFractionAlign);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_StackedFractions_BOOLINT, m_dimStyle1.m_data.ad5.flags.useStackedFractions != m_dimStyle2.m_data.ad5.flags.useStackedFractions);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_StackedFractionType_INTEGER, m_dimStyle1.m_data.ad5.flags.stackedFractionType != m_dimStyle2.m_data.ad5.flags.stackedFractionType);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OverrideStackedFractions_BOOLINT, m_dimStyle1.m_data.ad6.flags.fractionOverride != m_dimStyle2.m_data.ad6.flags.fractionOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT, m_dimStyle1.m_data.ad6.flags.underlineOverride != m_dimStyle2.m_data.ad6.flags.underlineOverride);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER, m_dimStyle1.m_data.ad4.ext_styleflg.superscriptMode != m_dimStyle2.m_data.ad4.ext_styleflg.superscriptMode);

    if (bitArray[0] || bitArray[1] || bitArray[2])
        m_differences->SetBitMaskByBitArray (bitArray, 48, DimStylePropMask::DIMSTYLE_CATEGORY_TextAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareToleranceAttributes ()
    {
    UInt16      bitArray[2];
    double      uorScale = 1.0;

#if defined (REMOVE_DEAD_CODE)
    modelInfo_getUorScaleBetweenModels (&uorScale, m_cache1, m_cache2);
#endif

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad2.lowtol, m_dimStyle2.m_data.ad2.lowtol, uorScale, AUTODIMLOWTOL_TOL));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_Mode_BOOLINT, m_dimStyle1.m_data.ad1.params.tolmode!= m_dimStyle2.m_data.ad1.params.tolmode);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_Show_BOOLINT, m_dimStyle1.m_data.ad1.params.tolerance!= m_dimStyle2.m_data.ad1.params.tolerance);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT, m_dimStyle1.m_data.ad1.params.tolStackEqual!= m_dimStyle2.m_data.ad1.params.tolStackEqual);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.tolMarginH, m_dimStyle2.m_data.ad5.tolMarginH, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad4.toltxt_scale, m_dimStyle2.m_data.ad4.toltxt_scale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_TextVerticalMargin_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.tolMarginV, m_dimStyle2.m_data.ad5.tolMarginV, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE, !dimStyleCompare_areDoublesEqual (m_dimStyle1.m_data.ad5.tolSepV, m_dimStyle2.m_data.ad5.tolSepV, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad2.uptol, m_dimStyle2.m_data.ad2.uptol, uorScale, AUTODIMHITOL_TOL));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Tolerance_ShowSignForZero_BOOLINT, m_dimStyle1.m_data.ad4.ext_styleflg.tolSignForZero != m_dimStyle2.m_data.ad4.ext_styleflg.tolSignForZero);

    if (bitArray[0] || bitArray[1])
        m_differences->SetBitMaskByBitArray (bitArray, 32, DimStylePropMask::DIMSTYLE_CATEGORY_ToleranceAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareValueAttributes ()
    {
    int         iArray, numArrays = 16;
    int         numUsedBits = 0;
    UInt16      bitArray[16 /*numArrays*/];
    double      uorScale = 1.0;

#if defined (REMOVE_DEAD_CODE)
    modelInfo_getUorScaleBetweenModels (&uorScale, m_cache1, m_cache2);
#endif

    memset (bitArray, 0, sizeof (bitArray));

    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_Accuracy_ACCURACY, m_dimStyle1.m_data.ad1.primaryAccuracy != m_dimStyle2.m_data.ad1.primaryAccuracy);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY, m_dimStyle1.m_data.ad6.altFormat.accuracy!= m_dimStyle2.m_data.ad6.altFormat.accuracy);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltIsActive_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.useAltFmt!= m_dimStyle2.m_data.ad6.altFormat.flags.useAltFmt);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY, m_dimStyle1.m_data.ad6.secAltFormat.accuracy != m_dimStyle2.m_data.ad6.secAltFormat.accuracy);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.useAltFmt!= m_dimStyle2.m_data.ad6.secAltFormat.flags.useAltFmt);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.adp_delimiter!= m_dimStyle2.m_data.ad6.secAltFormat.flags.adp_delimiter);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.adp_nomastunits != m_dimStyle2.m_data.ad6.secAltFormat.flags.adp_nomastunits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.adp_subunits != m_dimStyle2.m_data.ad6.secAltFormat.flags.adp_subunits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.adp_label!= m_dimStyle2.m_data.ad6.secAltFormat.flags.adp_label);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.equalToThreshold != m_dimStyle2.m_data.ad6.secAltFormat.flags.equalToThreshold);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.lessThanThreshold != m_dimStyle2.m_data.ad6.secAltFormat.flags.lessThanThreshold);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.adp_allowZeroMast != m_dimStyle2.m_data.ad6.secAltFormat.flags.adp_allowZeroMast);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT, m_dimStyle1.m_data.ad6.secAltFormat.flags.adp_hideZeroSub != m_dimStyle2.m_data.ad6.secAltFormat.flags.adp_hideZeroSub);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad6.secAltFormat.threshold, m_dimStyle2.m_data.ad6.secAltFormat.threshold, uorScale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.adp_delimiter != m_dimStyle2.m_data.ad6.altFormat.flags.adp_delimiter);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.adp_nomastunits!= m_dimStyle2.m_data.ad6.altFormat.flags.adp_nomastunits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.adp_subunits!= m_dimStyle2.m_data.ad6.altFormat.flags.adp_subunits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.adp_label!= m_dimStyle2.m_data.ad6.altFormat.flags.adp_label);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.equalToThreshold != m_dimStyle2.m_data.ad6.altFormat.flags.equalToThreshold);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT, m_dimStyle1.m_data.ad6.altFormat.flags.lessThanThreshold!= m_dimStyle2.m_data.ad6.altFormat.flags.lessThanThreshold);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT,  m_dimStyle1.m_data.ad6.altFormat.flags.adp_allowZeroMast != m_dimStyle2.m_data.ad6.altFormat.flags.adp_allowZeroMast);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT,  m_dimStyle1.m_data.ad6.altFormat.flags.adp_hideZeroSub != m_dimStyle2.m_data.ad6.altFormat.flags.adp_hideZeroSub);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AltThreshold_DOUBLE, !dimStyleCompare_areDistancesEqual (m_dimStyle1.m_data.ad6.altFormat.threshold, m_dimStyle2.m_data.ad6.altFormat.threshold, uorScale, m_doubleTol));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AngleFormat_INTEGER, m_dimStyle1.m_data.ad4.angle!= m_dimStyle2.m_data.ad4.angle);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT, m_dimStyle1.m_data.ad4.ext_styleflg.angleLeadingZero!= m_dimStyle2.m_data.ad4.ext_styleflg.angleLeadingZero);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AngleMeasure_BOOLINT, m_dimStyle1.m_data.ad1.mode.labang!= m_dimStyle2.m_data.ad1.mode.labang);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AnglePrecision_INTEGER, m_dimStyle1.m_data.ad4.refdispl != m_dimStyle2.m_data.ad4.refdispl);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT, m_dimStyle1.m_data.ad4.ext_styleflg.angleTrailingZero!= m_dimStyle2.m_data.ad4.ext_styleflg.angleTrailingZero);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_RoundLSD_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.roundLSD != m_dimStyle2.m_data.ad4.ext_dimflg.roundLSD);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY, m_dimStyle1.m_data.ad1.secondaryAccuracy!= m_dimStyle2.m_data.ad1.secondaryAccuracy);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_delimiter2!= m_dimStyle2.m_data.ad1.format.adp_delimiter2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_nomastunits2!= m_dimStyle2.m_data.ad1.format.adp_nomastunits2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_subunits2!= m_dimStyle2.m_data.ad1.format.adp_subunits2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowTrailingZeros_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.trailing_zeros2!= m_dimStyle2.m_data.ad4.ext_dimflg.trailing_zeros2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_label2!= m_dimStyle2.m_data.ad1.format.adp_label2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowZeroMasterUnit_BOOLINT, m_dimStyle1.m_data.ad1.params.adp_allowZeroMast2 != m_dimStyle2.m_data.ad1.params.adp_allowZeroMast2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SecShowZeroSubUnit_BOOLINT, m_dimStyle1.m_data.ad1.params.adp_hideZeroSub2 != m_dimStyle2.m_data.ad1.params.adp_hideZeroSub2);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_delimiter!= m_dimStyle2.m_data.ad1.format.adp_delimiter);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_nomastunits!= m_dimStyle2.m_data.ad1.format.adp_nomastunits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_subunits!= m_dimStyle2.m_data.ad1.format.adp_subunits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowTrailingZeros_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.trailing_zeros!= m_dimStyle2.m_data.ad4.ext_dimflg.trailing_zeros);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT, m_dimStyle1.m_data.ad1.format.adp_label != m_dimStyle2.m_data.ad1.format.adp_label);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowZeroMasterUnit_BOOLINT, m_dimStyle1.m_data.ad1.params.adp_allowZeroMast != m_dimStyle2.m_data.ad1.params.adp_allowZeroMast);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ShowZeroSubUnit_BOOLINT, m_dimStyle1.m_data.ad1.params.adp_hideZeroSub != m_dimStyle2.m_data.ad1.params.adp_hideZeroSub);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT, m_dimStyle1.m_data.ad4.ext_dimflg.superscriptLSD!= m_dimStyle2.m_data.ad4.ext_dimflg.superscriptLSD);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT, m_dimStyle1.m_data.ad5.flags.thousandSep != m_dimStyle2.m_data.ad5.flags.thousandSep);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT, m_dimStyle1.m_data.ad5.flags.metricSpc != m_dimStyle2.m_data.ad5.flags.metricSpc);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_Unit_UNITS,
                            (!dimStyleCompare_areDimUnitsEqual (m_dimStyle1.m_data.ad7.primaryMaster, m_dimStyle2.m_data.ad7.primaryMaster)) ||
                            (!dimStyleCompare_areDimUnitsEqual (m_dimStyle1.m_data.ad7.primarySub, m_dimStyle2.m_data.ad7.primarySub)));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_UnitSec_UNITS,
                            (!dimStyleCompare_areDimUnitsEqual (m_dimStyle1.m_data.ad7.secondaryMaster, m_dimStyle2.m_data.ad7.secondaryMaster)) ||
                            (!dimStyleCompare_areDimUnitsEqual (m_dimStyle1.m_data.ad7.secondarySub, m_dimStyle2.m_data.ad7.secondarySub)));
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT, m_dimStyle1.m_data.ad1.params.overrideWorkingUnits != m_dimStyle2.m_data.ad1.params.overrideWorkingUnits);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER, m_dimStyle1.m_data.ad6.flags.dmsAccuracyMode != m_dimStyle2.m_data.ad6.flags.dmsAccuracyMode);
    SetPropertyBitInBitArray (bitArray, DIMSTYLE_PROP_Value_SpaceAfterNonStackedFraction_BOOLINT, m_dimStyle1.m_data.ad6.flags.noNonStackedSpace != m_dimStyle2.m_data.ad6.flags.noNonStackedSpace);

    // Find the last array that is non-zero
    for (iArray = 0; iArray < numArrays; iArray++)
        {
        if (0 != bitArray[iArray])
            numUsedBits = 16 * (1 + iArray);
        }

    if (0 < numUsedBits)
        m_differences->SetBitMaskByBitArray (bitArray, numUsedBits, DimStylePropMask::DIMSTYLE_CATEGORY_ValueAttributes);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareTemplateFlags ()
    {
    int     i = 0;
    UInt16  bitArray[2];

    for (i = 0; i < 24; i++)
        {
        memset (bitArray, 0, sizeof (bitArray));

        if(m_dimStyle1.m_data.ad4.dim_template[i].left_witness != m_dimStyle2.m_data.ad4.dim_template[i].left_witness)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].right_witness != m_dimStyle2.m_data.ad4.dim_template[i].right_witness)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].centermark != m_dimStyle2.m_data.ad4.dim_template[i].centermark)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].stacked != m_dimStyle2.m_data.ad4.dim_template[i].stacked)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_General_Stacked_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].pre_symbol != m_dimStyle2.m_data.ad4.dim_template[i].pre_symbol)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].post_symbol != m_dimStyle2.m_data.ad4.dim_template[i].post_symbol)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].first_term != m_dimStyle2.m_data.ad4.dim_template[i].first_term)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].bowtie_symbol != m_dimStyle2.m_data.ad4.dim_template[i].bowtie_symbol)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].left_term != m_dimStyle2.m_data.ad4.dim_template[i].left_term)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].right_term != m_dimStyle2.m_data.ad4.dim_template[i].right_term)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].above_symbol != m_dimStyle2.m_data.ad4.dim_template[i].above_symbol)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Text_ArcLengthSymbol_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].nofit_vertical != m_dimStyle2.m_data.ad4.dim_template[i].nofit_vertical)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].vertical_text != m_dimStyle2.m_data.ad4.dim_template[i].vertical_text)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG, true);

        if(m_dimStyle1.m_data.ad4.dim_template[i].altExt != m_dimStyle2.m_data.ad4.dim_template[i].altExt)
            SetTemplateBitInBitArray (bitArray, DIMSTYLE_PROP_ExtensionLine_AngleChordAlign_TEMPLATEFLAG, true);

        if (bitArray[0] || bitArray[1])
            m_differences->SetBitMaskByBitArray (bitArray, 32, (DimStylePropMask::DimStyle_Category) DimStylePropMask::s_templateCategoryId[i]);
        }

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareStrings ()
    {
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_DimStyleDescription_MSWCHAR, TO_BOOL (wcscmp (m_dimStyle1.m_description.c_str(),    m_dimStyle2.m_description.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_DimStyleName_MSWCHAR,        TO_BOOL (wcscmp (m_dimStyle1.m_name.c_str(),           m_dimStyle2.m_name.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR,       TO_BOOL (wcscmp (m_dimStyle1.m_prefixCellName.c_str(), m_dimStyle2.m_prefixCellName.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR,       TO_BOOL (wcscmp (m_dimStyle1.m_suffixCellName.c_str(), m_dimStyle2.m_suffixCellName.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR,    TO_BOOL (wcscmp (m_dimStyle1.m_arrowCellName.c_str(),  m_dimStyle2.m_arrowCellName.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR,      TO_BOOL (wcscmp (m_dimStyle1.m_dotCellName.c_str(),    m_dimStyle2.m_dotCellName.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR,   TO_BOOL (wcscmp (m_dimStyle1.m_originCellName.c_str(), m_dimStyle2.m_originCellName.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR,   TO_BOOL (wcscmp (m_dimStyle1.m_strokeCellName.c_str(), m_dimStyle2.m_strokeCellName.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR,       TO_BOOL (wcscmp (m_dimStyle1.m_primary.m_masterUnitLabel.c_str(), m_dimStyle2.m_primary.m_masterUnitLabel.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR,    TO_BOOL (wcscmp (m_dimStyle1.m_secondary.m_masterUnitLabel.c_str(), m_dimStyle2.m_secondary.m_masterUnitLabel.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR,       TO_BOOL (wcscmp (m_dimStyle1.m_secondary.m_subUnitLabel.c_str(), m_dimStyle2.m_secondary.m_subUnitLabel.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR,          TO_BOOL (wcscmp (m_dimStyle1.m_primary.m_subUnitLabel.c_str(), m_dimStyle2.m_primary.m_subUnitLabel.c_str())));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR,     TO_BOOL (wcscmp (m_dimStyle1.m_noteCellName.c_str(), m_dimStyle2.m_noteCellName.c_str())));

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionStyle::DimStyleComparer::CompareExtensions ()
    {
    int         intVal1, intVal2;
    double      uorScale = 1.0;

    const DimStyleExtensions   *pExt1 = &m_dimStyle1.m_extensions;
    const DimStyleExtensions   *pExt2 = &m_dimStyle2.m_extensions;

#if defined (REMOVE_DEAD_CODE)
    modelInfo_getUorScaleBetweenModels (&uorScale, m_cache1, m_cache2);
#endif

    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_OrdDecrementReverse_BOOLINT, pExt1->flags.uOrdDecrementReverse != pExt2->flags.uOrdDecrementReverse);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT, pExt1->flags.uOrdUseDatumValue != pExt2->flags.uOrdUseDatumValue);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT, pExt1->flags.labelLineSupressAngle != pExt2->flags.labelLineSupressAngle);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT, pExt1->flags.labelLineSupressLength != pExt2->flags.labelLineSupressLength);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT, pExt1->flags.labelLineInvertLabels != pExt2->flags.labelLineInvertLabels);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE, !dimStyleCompare_areDistancesEqual (pExt1->dOrdinateDatumValue, pExt2->dOrdinateDatumValue, uorScale, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dNoteElbowLength, pExt2->dNoteElbowLength, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_FrameScale_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dNoteFrameScale, pExt2->dNoteFrameScale, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_BallAndChain_ElbowLength_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dBncElbowLength, pExt2->dBncElbowLength, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_BallAndChain_UseElbowLength_BOOLINT, pExt1->flags3.uUseBncElbowLength != pExt2->flags3.uUseBncElbowLength);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Text_StackedFractionScale_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->stackedFractionScale, pExt2->stackedFractionScale, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Text_InlineTextLift_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dInlineTextLift, pExt2->dInlineTextLift, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY, pExt1->primaryTolAcc != pExt2->primaryTolAcc);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY, pExt1->secondaryTolAcc != pExt2->secondaryTolAcc);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_RoundOff_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dRoundOff, pExt2->dRoundOff, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_SecRoundOff_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dSecondaryRoundOff, pExt2->dSecondaryRoundOff, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER, pExt1->flags.uMultiJustVertical != pExt2->flags.uMultiJustVertical);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT, pExt1->flags.uNoReduceFraction != pExt2->flags.uNoReduceFraction);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_NoReduceAltFraction_BOOLINT, pExt1->flags.uNoReduceAltFraction != pExt2->flags.uNoReduceAltFraction);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_NoReduceTolFraction_BOOLINT, pExt1->flags.uNoReduceTolFraction != pExt2->flags.uNoReduceTolFraction);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_NoReduceSecFraction_BOOLINT, pExt1->flags.uNoReduceSecFraction != pExt2->flags.uNoReduceSecFraction);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_NoReduceAltSecFraction_BOOLINT, pExt1->flags.uNoReduceAltSecFraction != pExt2->flags.uNoReduceAltSecFraction);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_NoReduceTolSecFraction_BOOLINT, pExt1->flags.uNoReduceTolSecFraction != pExt2->flags.uNoReduceTolSecFraction);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_LeaderType_BOOLINT, pExt1->flags2.uNoteLeaderType != pExt2->flags2.uNoteLeaderType);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_Note_INTEGER, pExt1->flags2.uNoteTerminator != pExt2->flags2.uNoteTerminator);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_TextRotation_INTEGER, pExt1->flags2.uNoteTextRotation != pExt2->flags2.uNoteTextRotation);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER, pExt1->flags2.uNoteHorAttachment != pExt2->flags2.uNoteHorAttachment);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER, pExt1->flags2.uNoteVerLeftAttachment != pExt2->flags2.uNoteVerLeftAttachment);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER, pExt1->flags2.uNoteVerRightAttachment != pExt2->flags2.uNoteVerRightAttachment);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT, pExt1->flags.uLabelLineAdjacentLabels != pExt2->flags.uLabelLineAdjacentLabels);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Value_OrdFreeLocation_BOOLINT, pExt1->flags2.uOrdFreeLocation != pExt2->flags2.uOrdFreeLocation);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT, pExt1->flags2.uNotUseModelAnnotationScale != pExt2->flags2.uNotUseModelAnnotationScale);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_ScaleFrame_BOOLINT, pExt1->flags2.uNoteScaleFrame != pExt2->flags2.uNoteScaleFrame);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dAnnotationScale, pExt2->dAnnotationScale, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dNoteLeftMargin, pExt2->dNoteLeftMargin, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE, !dimStyleCompare_areDoublesEqual (pExt1->dNoteLowerMargin, pExt2->dNoteLowerMargin, m_doubleTol));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_NoteType_INTEGER, pExt1->flags2.uNoteTerminatorType != pExt2->flags2.uNoteTerminatorType);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_NoteChar_CHAR, pExt1->iNoteTerminatorChar != pExt2->iNoteTerminatorChar);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Terminator_NoteFont_FONT, !areSameFont (pExt1->iNoteTerminatorFont, m_cache1, pExt2->iNoteTerminatorFont, m_cache2));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_SuppressUnfitTerminators_BOOLINT, pExt1->flags3.uNoTermsOutside != pExt2->flags3.uNoTermsOutside);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_TightFitTextAbove_BOOLINT, pExt1->flags3.uTightFitTextAbove != pExt2->flags3.uTightFitTextAbove);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_FitInclinedTextBox_BOOLINT, pExt1->flags3.uFitInclinedTextBox != pExt2->flags3.uFitInclinedTextBox);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_UseMinLeader_BOOLINT, pExt1->flags3.uIgnoreMinLeader != pExt2->flags3.uIgnoreMinLeader);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT, pExt1->flags3.uExtendDimLineUnderText != pExt2->flags3.uExtendDimLineUnderText);

    m_dimStyle1.GetIntegerProp (intVal1, DIMSTYLE_PROP_Text_Location_INTEGER);
    m_dimStyle2.GetIntegerProp (intVal2, DIMSTYLE_PROP_Text_Location_INTEGER);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Text_Location_INTEGER, intVal1 != intVal2);

    m_dimStyle1.GetIntegerProp (intVal1, DIMSTYLE_PROP_MLNote_FrameType_INTEGER);
    m_dimStyle2.GetIntegerProp (intVal2, DIMSTYLE_PROP_MLNote_FrameType_INTEGER);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_MLNote_FrameType_INTEGER, intVal1 != intVal2);

    m_dimStyle1.GetIntegerProp (intVal1, DIMSTYLE_PROP_BallAndChain_Mode_INTEGER);
    m_dimStyle2.GetIntegerProp (intVal2, DIMSTYLE_PROP_BallAndChain_Mode_INTEGER);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_BallAndChain_Mode_INTEGER, intVal1 != intVal2);

    m_dimStyle1.GetIntegerProp (intVal1, DIMSTYLE_PROP_Text_Justification_INTEGER);
    m_dimStyle2.GetIntegerProp (intVal2, DIMSTYLE_PROP_Text_Justification_INTEGER);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Text_Justification_INTEGER, intVal1 != intVal2);

    m_dimStyle1.GetIntegerProp (intVal1, DIMSTYLE_PROP_General_FitOption_INTEGER);
    m_dimStyle2.GetIntegerProp (intVal2, DIMSTYLE_PROP_General_FitOption_INTEGER);
    m_differences->SetPropertyBit (DIMSTYLE_PROP_General_FitOption_INTEGER, intVal1 != intVal2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionStyle::DimStyleComparer::StripUnusedDiffs ()
    {
    int                 iProp, numProps;
    PropToOverrideMap  *propToOverridesMap = dgnDimStyle_getPropToOverridesMap (&numProps);

    /* If a property is inherited by both styles, that should not be considered
       as a difference between the styles. */

    for (iProp = 0; iProp < numProps; iProp++)
        {
        bool         inverted     = TO_BOOL (propToOverridesMap[iProp].inverted);
        DimStyleProp overrideProp = propToOverridesMap[iProp].m_override;
        DimStyleProp property     = propToOverridesMap[iProp].m_property;

        // If the property is marked as a diff, we might turn it off
        if ( ! m_differences->GetPropertyBit (property))
            continue;

        bool override1, override2;

        if (SUCCESS == m_dimStyle1.GetBooleanProp (override1, overrideProp) &&
            SUCCESS == m_dimStyle2.GetBooleanProp (override2, overrideProp))
            {
            bool bothInherited;

            if ( ! inverted)
                bothInherited = (false == override1 && false == override2);
            else
                bothInherited = (false != override1 && false != override2);

            if (bothInherited)
                m_differences->SetPropertyBit (property, false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStylePropMaskPtr DimensionStyle::DimStyleComparer::CompareStyles ()
    {
    CompareBallAndChainAttributes   ();
    CompareExtensionLineAttributes  ();
    CompareGeneralAttributes        ();
    CompareMultiLineNotesAttributes ();
    ComparePlacementAttributes      ();
    CompareSymbolAttributes         ();
    CompareTerminatorAttributes     ();
    CompareTextAttributes           ();
    CompareToleranceAttributes      ();
    CompareValueAttributes          ();
    CompareTemplateFlags            ();
    CompareExtensions               ();
    CompareStrings                  ();

    DgnTextStylePtr tstyle1 = m_dimStyle1.GetDgnProject()->Styles().TextStyles().QueryById(DgnStyleId(m_dimStyle1.GetTextStyleId()));
    DgnTextStylePtr tstyle2 = m_dimStyle2.GetDgnProject()->Styles().TextStyles().QueryById(DgnStyleId(m_dimStyle2.GetTextStyleId()));
    bool different = false;
    different |= tstyle1.IsValid() && !tstyle2.IsValid();
    different |= !tstyle1.IsValid() && tstyle2.IsValid();
    if (tstyle1.IsValid() && tstyle2.IsValid())
        different = (0 != tstyle1->GetName().CompareToI(tstyle2->GetName ()));
    m_differences->SetPropertyBit (DIMSTYLE_PROP_Text_TextStyleID_INTEGER, different);

    if (DIMSTYLE_COMPAREOPTS_IgnoreUnusedDiffs & m_compareOpts)
        StripUnusedDiffs ();

    return m_differences;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStyle::DimStyleComparer::DimStyleComparer (DimensionStyleCR style1, DimensionStyleCR style2, UInt32 compareOpts)
    :
    m_dimStyle1 (style1),
    m_dimStyle2 (style2),
    m_compareOpts (compareOpts)
    {
    m_differences = DimStylePropMask::CreatePropMask ();

    // doubleTol was originally mgds_fc_epsilon = 1e-5.  This was not sufficiently tight since the dialog
    // box displays 6 places of accuracy.  At 1e-5 tolerance, changes visible in the dialog box could be
    // ignored by the comparison routine.
    m_doubleTol = 1e-8;

    m_cache1 = m_dimStyle1.m_project->Models().GetDictionaryModel();
    m_cache2 = m_dimStyle2.m_project->Models().GetDictionaryModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStylePropMaskPtr DimensionStyle::Compare (DimensionStyleCR otherStyle, UInt32 compareOpts) const
    {
    DimStyleComparer comparer (*this, otherStyle, compareOpts);

    return comparer.CompareStyles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
UShort          DimStylePropMask::GetLinkageKeyFromCategoryID
(
DimStyle_Category   categoryID      // =>
)
    {
    return static_cast <UShort> (categoryID + BITMASK_LINKAGE_KEY_DimShieldsBase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimStylePropMask::DeleteLinkages (DgnElementR elem)
    {
    UShort      categoryID;

    // NEEDSWORK:
    // This is pretty inefficient.  We could do alot better to implement a generic
    // linkage iterator, with a client for each linkage type to interpret the
    // linkage contents.

    for (categoryID = 0; categoryID < DIMSTYLE_CATEGORY_NUMBER; categoryID++)
        {
        UShort iKey = GetLinkageKeyFromCategoryID ((DimStyle_Category) categoryID);

        BitMaskLinkage::DeleteBitMask (&elem, iKey, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public DimStylePropMaskPtr  DimStylePropMask::ExtractFromLinkages (ElementHandleCR elem)
    {
    UShort              categoryID;
    DimStylePropMaskPtr propMask = CreatePropMask();

    // NEEDSWORK:
    // This is pretty inefficient.  We could do alot better to implement a generic
    // linkage iterator, with a client for each linkage type to interpret the
    // linkage contents.

    for (categoryID = 0; categoryID < DIMSTYLE_CATEGORY_NUMBER; categoryID++)
        {
        UShort      iKey = GetLinkageKeyFromCategoryID ((DimStyle_Category) categoryID);
        BitMaskP    pBitMask = NULL;

        if (SUCCESS == BitMaskLinkage::ExtractBitMask (&pBitMask, elem.GetElementCP(), iKey, 0))
            propMask->SetBitMaskP (pBitMask, (DimStyle_Category) categoryID);
        }

    return propMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimStylePropMask::AppendAsLinkages (DgnElementR element) const
    {
    for (int iCategory = 0; iCategory < DIMSTYLE_CATEGORY_NUMBER; iCategory++)
        {
        UShort      iKey = GetLinkageKeyFromCategoryID ((DimStyle_Category) iCategory);
        BitMaskCP   pBitMask = GetBitMaskCP ((DimStyle_Category) iCategory);

        if (NULL != pBitMask && pBitMask->AnyBitSet ())
            BitMaskLinkage::SetBitMask (&element, iKey, pBitMask);
        else
            BitMaskLinkage::DeleteBitMask (&element, iKey, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimStylePropMask::LogicalOperation (DimStylePropMaskR destPropMask, DimStylePropMaskCR sourcePropMask, BitMaskOperation operation)
    {
    UInt16                      iCategory;
    BitMaskCP                   pBitMaskSource  = NULL;
    BitMaskCP                   pBitMaskDest    = NULL;
    BitMaskP                    pBitMaskFinal   = NULL;
    BitMaskP                    pEmptyBitMask   = NULL;

    for (iCategory = 0; iCategory < DIMSTYLE_CATEGORY_NUMBER; iCategory++)
        {
        if (NULL == (pBitMaskSource = sourcePropMask.GetBitMaskCP ((DimStyle_Category) iCategory)))
            {
            if (!pEmptyBitMask)
                pEmptyBitMask = BitMask::Create ( false);
            pBitMaskSource = pEmptyBitMask;
            }

        if (NULL != (pBitMaskDest = destPropMask.GetBitMaskCP ((DimStyle_Category) iCategory)))
            pBitMaskFinal = BitMask::Clone (*pBitMaskDest);
        else
            pBitMaskFinal = BitMask::Create ( false);

        pBitMaskFinal->LogicalOperation(*pBitMaskSource,  operation);

        if (pBitMaskFinal->AnyBitSet ())
            {
            destPropMask.SetBitMaskP (pBitMaskFinal, (DimStyle_Category) iCategory);
            }
        else
            {
            destPropMask.SetBitMaskP (NULL, (DimStyle_Category) iCategory);
            pBitMaskFinal->Free ();
            }
        }

    if (pEmptyBitMask)
        pEmptyBitMask->Free ();

    // NEEDSWORK : Currently, logical NOT will not do anything because it flips the state
    // of only the bits in the available bitarray and does not affect all the bitarrays.
    // In order to accomplish this, follow one of the following routes.
    // a. Hold the full array size of each bitmask in dimstylepropmask and NOT would
    //    flip all of them.
    // b. Have NOT flip the DefaultValue of the bitmask. If we choose to do this, do it in
    //    BitMask::LogicalOperation. This may have other repercussions because someone
    //    may copy a NOT'ted bitmask and assume its default value to still be 0.
    }
