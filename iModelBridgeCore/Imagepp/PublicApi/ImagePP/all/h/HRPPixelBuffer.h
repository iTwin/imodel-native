//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelBuffer.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelBuffer
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelNeighbourhood.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;

class HRPPixelBuffer
    {
public:

    // Primary methods

    HRPPixelBuffer(const HRPPixelNeighbourhood&
                   pi_rNeighbourhood,
                   const HRPPixelType&  pi_rPixelType,
                   void*                pio_pRawDataPtr,
                   uint32_t             pi_Width,
                   uint32_t             pi_Height,
                   uint32_t             pi_PaddingBytes=0,
                   bool                pi_InternalBuffer = false,
                   const double*       pi_pPositionsX = 0,
                   const double*       pi_pPositionsY = 0);

    IMAGEPP_EXPORT                     HRPPixelBuffer(const HRPPixelType&  pi_rPixelType,
                                              void*                pio_pRawDataPtr,
                                              uint32_t             pi_Width,
                                              uint32_t             pi_Height,
                                              uint32_t             pi_PaddingBytes=0,
                                              const double*       pi_pPositionsX = 0,
                                              const double*       pi_pPositionsY = 0);

    HRPPixelBuffer(const HRPPixelNeighbourhood&
                   pi_rNeighbourhood,
                   const HRPPixelType&  pi_rPixelType);

    HRPPixelBuffer(const HRPPixelType&  pi_rpPixelType);


    IMAGEPP_EXPORT virtual                ~HRPPixelBuffer();


    // Buffer settings

    const void*         GetBufferPtr() const;

    uint32_t            GetBytesPerLine() const;

    uint32_t              GetHeight() const;

    uint32_t              GetWidth() const;

    void                SetRawData(void*            pio_pRawDataPtr,
                                   uint32_t         pi_Width,
                                   uint32_t         pi_Height,
                                   uint32_t         pi_PaddingBytes = 0,
                                   bool            pi_InternalBorders = false,
                                   const double*   pi_pPositionsX = 0,
                                   const double*   pi_pPositionsY = 0);

    // Position buffer
    const double*      GetPositionsX() const;
    const double*      GetPositionsY() const;

    void                SetBorders(const HRPPixelNeighbourhood&
                                   pi_rNeighbourhood,
                                   const void*         pi_pTopBorder,
                                   const void*         pi_pLeftBorder,
                                   const void*         pi_pRightBorder,
                                   const void*         pi_pBottomBorder,
                                   uint32_t            pi_TopBorderPaddingBytes,
                                   uint32_t            pi_LeftBorderPaddingBytes,
                                   uint32_t            pi_RightBorderPaddingBytes,
                                   uint32_t            pi_BottomBorderPaddingBytes);


    // Raw data

    Byte*              Extract() const;

    void                GetLine(int32_t pi_Y, void* po_pRawData) const;

    uint32_t            GetPaddingBytes() const;


protected:

    HRPPixelBuffer();

    HRPPixelBuffer& operator=(const HRPPixelBuffer& pi_rObj);


private:

    void                Initialize(const HRPPixelNeighbourhood& pi_rNeighbourhood,
                                   const HRPPixelType&          pi_rPixelType);

    // bits per pixel in the buffer
    uint32_t                m_BitsPerPixel;

    // dimensions of the buffer
    uint32_t                  m_Width;
    uint32_t                  m_Height;

    // Neighbourhood of the buffer
    HRPPixelNeighbourhood   m_Neighbourhood;

    // pointers to buffer and borders
    Byte*                    m_pBuffer;
    Byte*                 m_pTopBorder;
    Byte*                 m_pLeftBorder;
    Byte*                 m_pRightBorder;
    Byte*                 m_pBottomBorder;

    // pointers the position array
    const double*          m_pPositionsX;
    const double*          m_pPositionsY;

    // padding bytes for buffer and borders
    uint32_t                m_PaddingBytes;
    uint32_t                m_TopBorderPaddingBytes;
    uint32_t                m_LeftBorderPaddingBytes;
    uint32_t                m_RightBorderPaddingBytes;
    uint32_t                m_BottomBorderPaddingBytes;
    };
END_IMAGEPP_NAMESPACE

#include "HRPPixelBuffer.hpp"

