//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPCustomConvFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// User defined convolution filter.
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPCustomConvFilter.h>
#include <Imagepp/all/h/HRPPixelNeighbourhood.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPCustomConvFilter::HRPCustomConvFilter()
    :HRPConvFilterV24R8G8B8()
    {
    m_MatrixWidth = 0;
    m_MatrixHeight = 0;
    m_WeightMatrix = 0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPCustomConvFilter::HRPCustomConvFilter (uint32_t pi_MatrixWidth,
                                          uint32_t pi_MatrixHeigth,
                                          uint32_t pi_XOrigin,
                                          uint32_t pi_YOrigin,
                                          int32_t* pi_pWeightMatrix)
    :HRPConvFilterV24R8G8B8( HRPPixelNeighbourhood(pi_MatrixWidth,
                                                   pi_MatrixHeigth,
                                                   pi_XOrigin,
                                                   pi_YOrigin))

    {
    m_MatrixWidth = pi_MatrixWidth;
    m_MatrixHeight = pi_MatrixHeigth;

    uint32_t MatrixSize = pi_MatrixWidth * pi_MatrixHeigth;

    // Get a copy of the convolution matrix
    m_WeightMatrix = new int32_t[MatrixSize];
    memcpy(m_WeightMatrix, pi_pWeightMatrix, MatrixSize * sizeof(uint32_t));
    SetWeightMatrix( m_WeightMatrix, ComputeDivisionFactor(pi_pWeightMatrix, MatrixSize));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPCustomConvFilter::HRPCustomConvFilter(const HRPCustomConvFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    m_MatrixWidth = pi_rFilter.m_MatrixWidth;
    m_MatrixHeight = pi_rFilter.m_MatrixHeight;

    uint32_t MatrixSize = m_MatrixWidth * m_MatrixHeight;

    m_WeightMatrix = new int32_t[MatrixSize];
    memcpy(m_WeightMatrix, pi_rFilter.m_WeightMatrix, MatrixSize * sizeof(uint32_t));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPCustomConvFilter::~HRPCustomConvFilter()
    {
    delete m_WeightMatrix;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPFilter* HRPCustomConvFilter::Clone() const
    {
    return new HRPCustomConvFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPCustomConvFilter::ComputeDivisionFactor(int32_t* pi_pWeightMatrix, uint32_t pi_MatrixSize)
    {
    int32_t DivisionFactor = 0;

    for (uint32_t MatrixIndex=0; MatrixIndex < pi_MatrixSize; MatrixIndex++)
        DivisionFactor += pi_pWeightMatrix[MatrixIndex];

    // DivisionFactor cannot be NULL
    if (!DivisionFactor)
        DivisionFactor++;

    return DivisionFactor;
    }
