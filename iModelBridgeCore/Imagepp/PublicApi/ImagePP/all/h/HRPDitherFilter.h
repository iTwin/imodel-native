//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPDitherFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPDitherFilter
//-----------------------------------------------------------------------------
// This class describes the dither filter
//-----------------------------------------------------------------------------
#pragma once

#include "HRPFilter.h"
#include "HMGMessageReceiver.h"
#include "HMGMacros.h"

#include "HRPPaletteOctreeR8G8B8.h"


BEGIN_IMAGEPP_NAMESPACE
class HRPDitherFilter : public HRPFilter, public HMGMessageReceiver
    {
    HDECLARE_CLASS_ID(HRPFilterId_Dither, HRPFilter)

public:

    // Primary methods
    HRPDitherFilter();

    HRPDitherFilter(const HFCPtr<HRPPixelType>& pi_pPixelType);

    HRPDitherFilter(const HRPDitherFilter& pi_rFilter);

    virtual                    ~HRPDitherFilter();

    // Conversion
    void                    Convert(HRPPixelBuffer* pi_pInputBuffer,
                                    HRPPixelBuffer* pio_pOutputBuffer);

    virtual HRPFilter* Clone() const override;


    // Messaging
    bool                    NotifyPaletteChanged (const HMGMessage& pi_rMessage);


    // Atributes
    HFCPtr<HRPPixelType>                m_pPixelType;

protected:

private:
    
    // Private methods
    void                    InitObject();

    void                    FillQuantizedPalette();

    void                    Process(Byte* pi_pSrcRawData[],
                                    Byte* po_pDestRawData,
                                    uint32_t pi_Width);

    HRPPaletteOctreeR8G8B8                m_QuantizedPalette;
    //HRPQuantizedPaletteR8G8B8                m_QuantizedPalette;

    short m_RightError[512];
    short m_BottomLeftError[512];
    short m_BottomError[512];
    short m_BottomRightError[512];

    Byte                              m_MinMaxTable[480];

    // Messaging
    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT);
    };
END_IMAGEPP_NAMESPACE

