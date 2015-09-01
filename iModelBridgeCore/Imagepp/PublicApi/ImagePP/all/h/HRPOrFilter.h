//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPOrFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPOrFilter
//-----------------------------------------------------------------------------
// This class describes the Or filter
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPOrFilter : public HRPFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Or, HRPFilter)

public:


    // Primary methods

    HRPOrFilter();

    HRPOrFilter(bool pi_OnBit);

    HRPOrFilter(const HRPOrFilter& pi_rFilter);

    virtual                    ~HRPOrFilter();


    // Conversion

    void                    Convert(HRPPixelBuffer* pi_pInputBuffer,
                                    HRPPixelBuffer* pio_pOutputBuffer);
    // Others

    virtual HRPFilter* Clone() const override;

protected:


private:

    bool                   m_OnBit;
    static Byte           m_sTable[2][256];
    static bool            m_sTableUpdated;
    static Byte           m_sMask[8];

    void                    InitObject();
    void                    UpdateTable();
    };
END_IMAGEPP_NAMESPACE

