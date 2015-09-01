//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAListRelativeAttribute.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HIDXAListRelativeAttribute
//-----------------------------------------------------------------------------
// relative indexing attribute
//-----------------------------------------------------------------------------

#pragma once


#include "HIDXAList.h"

BEGIN_IMAGEPP_NAMESPACE

template<class T, int C> class HIDXAListRelativeAttribute : public HIDXAttribute
    {
public:

    HIDXAListRelativeAttribute(HIDXAListBlock<T, C>* pi_pBlock,
                               uint32_t              pi_AbsolutePosition);

    virtual         ~HIDXAListRelativeAttribute();

    HIDXAListBlock<T, C>*
    GetBlock() const;
    uint32_t        GetPosition() const;
    uint32_t        GetRelativePosition() const;
    void            SetBlock(HIDXAListBlock<T, C>* pi_pBlock);
    void            SetPosition(uint32_t pi_AbsolutePosition);
    void            SetRelativePosition(uint32_t pi_RelativePosition);
    void            SetInformation(HIDXAListBlock<T, C>* pi_pBlock, uint32_t pi_AbsolutePosition);
    void            SetRelativeInformation(HIDXAListBlock<T, C>* pi_pBlock, uint32_t pi_RelativePosition);

private:

    // Disabled.
    HIDXAListRelativeAttribute(const HIDXAListRelativeAttribute<T, C>& pi_rObj);
    HIDXAListRelativeAttribute<T, C>&
    operator=(const HIDXAListRelativeAttribute<T, C>& pi_rObj);

    // Pointer to the block that contains this element
    HIDXAListBlock<T, C>*
    m_pBlock;

    // Absolute position in the list
    uint32_t        m_AbsolutePosition;
    };

END_IMAGEPP_NAMESPACE
#include "HIDXAListRelativeAttribute.hpp"


