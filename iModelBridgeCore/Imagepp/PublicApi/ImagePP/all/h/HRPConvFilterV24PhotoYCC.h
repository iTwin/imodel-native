//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPConvFilterV24PhotoYCC.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPConvFilterV24PhotoYCC
//-----------------------------------------------------------------------------
// This class describes a V24PhotoYCC convolution filter.
//-----------------------------------------------------------------------------

#pragma once


#include "HRPConvFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPConvFilterV24PhotoYCC : public HRPConvFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_ConvV24PhotoYCC, HRPConvFilter)

public:

    // Primary methods

    HRPConvFilterV24PhotoYCC();

    HRPConvFilterV24PhotoYCC(
        const HRPPixelNeighbourhood& pi_rPixelNeighbourhood,
        const int32_t* pi_pWeightMatrix = 0,
        int32_t pi_DivisionFactor = 0);

    virtual            ~HRPConvFilterV24PhotoYCC();


    // Conversion method

    void            Convert(    HRPPixelBuffer* pi_pInputBuffer,
                                HRPPixelBuffer* pio_pOutputBuffer);

    virtual void    Convert(const void*     pi_pInputData[],
                            void*           po_pOutputData,
                            const double*  pi_pPosX,
                            const double*  pi_pPosY)
        {
        T_Super::Convert(pi_pInputData,po_pOutputData,pi_pPosX,pi_pPosY);
        }

    // Cloning
    HRPFilter*      Clone() const override = 0;

protected:
    HRPConvFilterV24PhotoYCC(const HRPConvFilterV24PhotoYCC& pi_rFilter);

private:

    // Convolution method

    void            Convoluate(const void*      pi_pSrcRawData[],
                               void*            pi_pDestRawData,
                               uint32_t         pi_Width,
                               const double*   pi_pPositionsX,
                               const double*   pi_pPositionsY) const;

    // Attributes

    double**      m_pPreMulTables;
    double*       m_pResultLine;
    bool*         m_pAllocation;
    };
END_IMAGEPP_NAMESPACE

