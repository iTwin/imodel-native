//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPConvFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPConvFilter
//-----------------------------------------------------------------------------
// This class describes a convolution filter.
//-----------------------------------------------------------------------------

#pragma once

#include "HRPTypedFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPConvFilter : public HRPTypedFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Conv, HRPTypedFilter)

public:

    // Primary methods

    virtual            ~HRPConvFilter();


    // Conversion

    virtual void    Convert(HRPPixelBuffer* pi_pInputBuffer,
                            HRPPixelBuffer* pio_pOutputBuffer);

    virtual void    Convert(const void*     pi_pInputData[],
                            void*           po_pOutputData,
                            const double*  pi_pPosX,
                            const double*  pi_pPosY);

    // Decapsulation

    virtual bool   IsAConvolutionFilter() const;


    // Composing

    HRPFilter*      ComposeWith(const HRPFilter* pi_pFilter);

    // Cloning
    virtual HRPFilter* Clone() const override = 0;

    int32_t        GetDivisionFactor() const;
    
    const int32_t*   GetWeightMatrix() const;

protected:

    // Primary methods

    HRPConvFilter(  const HFCPtr<HRPPixelType>& pi_pFilterPixelType);

    HRPConvFilter(  const HFCPtr<HRPPixelType>& pi_pFilterPixelType,
                    const HRPPixelNeighbourhood& pi_rNeighbourhood,
                    const int32_t* pi_pWeightMatrix = 0,
                    int32_t pi_DivisionFactor = 0);

    HRPConvFilter( const HRPConvFilter& pi_rFilter);
    
    void            SetNeighbourhood (const HRPPixelNeighbourhood& pi_rNeighbourhood);

protected:
    void            SetWeightMatrix(const int32_t* pi_pWeightMatrix,
                                    int32_t pi_DivisionFactor);

    // DON'T USE THESE METHODS YET! NOT TESTED!

    bool           IsHorizontallySymetric() const;

    bool           IsVerticallySymetric() const;

    bool           IsPerfectlySymetric() const;

private:

    // Convolution method

    virtual void    Convoluate(const void*      pi_pSrcRawData[],
                               void*            pi_pDestRawData,
                               uint32_t         pi_Width,
                               const double*   pi_pPositionsX,
                               const double*   pi_pPositionsY) const = 0;

    // Attributes

    HArrayAutoPtr<int32_t>   m_pWeightMatrix;
    size_t                  m_WeightMatrixSize;

    uint32_t                m_DivisionFactor;

    // Optimization members
    uint32_t                m_FilterBytesPerPixel;
    };
END_IMAGEPP_NAMESPACE

#include "HRPConvFilter.hpp"

