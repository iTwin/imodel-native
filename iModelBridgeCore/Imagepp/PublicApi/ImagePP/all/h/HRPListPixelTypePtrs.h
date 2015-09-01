//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPListPixelTypePtrs.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPListPixelTypePtrs
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

#include "HRPPixelTypeI1R8G8B8.h"
#include "HRPPixelTypeI1R8G8B8RLE.h"
#include "HRPPixelTypeI1R8G8B8A8.h"
#include "HRPPixelTypeI1R8G8B8A8RLE.h"
#include "HRPPixelTypeI4R8G8B8.h"
#include "HRPPixelTypeI4R8G8B8A8.h"
#include "HRPPixelTypeI8R8G8B8.h"
#include "HRPPixelTypeI8R8G8B8A8.h"
#include "HRPPixelTypeI8VA8R8G8B8.h"
#include "HRPPixelTypeV1Gray1.h"
#include "HRPPixelTypeV8Gray8.h"
#include "HRPPixelTypeV16PRGray8A8.h"
#include "HRPPixelTypeV24B8G8R8.h"
#include "HRPPixelTypeV24PhotoYCC.h"
#include "HRPPixelTypeV24R8G8B8.h"
#include "HRPPixelTypeV32A8R8G8B8.h"
#include "HRPPixelTypeV32PRPhotoYCCA8.h"
#include "HRPPixelTypeV32PR8PG8PB8A8.h"
#include "HRPPixelTypeV32R8G8B8A8.h"
#include "HRPPixelTypeV32R8G8B8X8.h"
#include "HRPPixelTypeV32B8G8R8X8.h"
#include "HRPPixelTypeV32CMYK.h"
#include "HRPPixelTypeV48R16G16B16.h"
#include "HRPPixelTypeV64R16G16B16A16.h"
#include "HRPPixelTypeV64R16G16B16x16.h"
#include "HRPPixelTypeV1GrayWhite1.h"
#include "HRPPixelTypeV8GrayWhite8.h"
#include "HRPPixelTypeV16Gray16.h"
#include "HRPPixelTypeV16Int16.h"
#include "HRPPixelTypeV32Float32.h"
#include "HRPPixelTypeI8R8G8B8Mask.h"
#include "HRPPixelTypeI8Gray8.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPListPixelTypePtrs
    {
public:

    typedef list<HRPPixelType*, allocator<HRPPixelType*> >
    ListPixelTypePtrs;

    ListPixelTypePtrs m_List;

    HRPListPixelTypePtrs() : m_List()
        {
        m_List.push_front(&m_V1Gray1);
        m_List.push_front(&m_V8Gray8);
        m_List.push_front(&m_V24R8G8B8);
        m_List.push_front(&m_V24B8G8R8);
        m_List.push_front(&m_V32R8G8B8A8);
        m_List.push_front(&m_V32R8G8B8X8);
        m_List.push_front(&m_V32B8G8R8X8);
        m_List.push_front(&m_I8R8G8B8);
        m_List.push_front(&m_I8R8G8B8A8);
        m_List.push_front(&m_I8VA8R8G8B8);
        m_List.push_front(&m_V32PR8PG8PB8A8);
        m_List.push_front(&m_V24PhotoYCC);
        m_List.push_front(&m_V32PRPhotoYCCA8);
        m_List.push_front(&m_V32CMYK);
        m_List.push_front(&m_I1R8G8B8);
        m_List.push_front(&m_I1R8G8B8RLE);
        m_List.push_front(&m_V16PRGray8A8);
        m_List.push_front(&m_V32A8R8G8B8);
        //m_List.push_front(&m_I2R8G8B8);
        m_List.push_front(&m_I4R8G8B8);
        m_List.push_front(&m_I4R8G8B8A8);
        m_List.push_front(&m_I1R8G8B8A8);
        m_List.push_front(&m_I1R8G8B8A8RLE);
        m_List.push_front(&m_V1GrayWhite1);
        m_List.push_front(&m_V8GrayWhite8);
        m_List.push_front(&m_V48R16G16B16);
        m_List.push_front(&m_V16Gray16);
        m_List.push_front(&m_V16Int16);
        m_List.push_front(&m_V32Float32);
        m_List.push_front(&m_I8R8G8B8Mask);
        m_List.push_front(&m_I8Gray8);
        };

private:

    HRPPixelTypeV1Gray1         m_V1Gray1;
    HRPPixelTypeV8Gray8         m_V8Gray8;
    HRPPixelTypeV24R8G8B8       m_V24R8G8B8;
    HRPPixelTypeV32R8G8B8A8     m_V32R8G8B8A8;
    HRPPixelTypeV32R8G8B8X8     m_V32R8G8B8X8;
    HRPPixelTypeV32B8G8R8X8     m_V32B8G8R8X8;
    HRPPixelTypeV24B8G8R8       m_V24B8G8R8;
    HRPPixelTypeI8R8G8B8        m_I8R8G8B8;
    HRPPixelTypeI8R8G8B8A8      m_I8R8G8B8A8;
    HRPPixelTypeI8VA8R8G8B8     m_I8VA8R8G8B8;
    HRPPixelTypeV32PR8PG8PB8A8  m_V32PR8PG8PB8A8;
    HRPPixelTypeV24PhotoYCC     m_V24PhotoYCC;
    HRPPixelTypeV32PRPhotoYCCA8 m_V32PRPhotoYCCA8;
    HRPPixelTypeV32CMYK         m_V32CMYK;
    HRPPixelTypeI1R8G8B8        m_I1R8G8B8;
    HRPPixelTypeI1R8G8B8RLE     m_I1R8G8B8RLE;
    HRPPixelTypeV16PRGray8A8    m_V16PRGray8A8;
    HRPPixelTypeV32A8R8G8B8     m_V32A8R8G8B8;
    //Disabled. HRPPixelTypeI2R8G8B8        m_I2R8G8B8;
    HRPPixelTypeI4R8G8B8        m_I4R8G8B8;
    HRPPixelTypeI4R8G8B8A8      m_I4R8G8B8A8;
    HRPPixelTypeI1R8G8B8A8      m_I1R8G8B8A8;
    HRPPixelTypeI1R8G8B8A8RLE   m_I1R8G8B8A8RLE;
    HRPPixelTypeV1GrayWhite1    m_V1GrayWhite1;
    HRPPixelTypeV8GrayWhite8    m_V8GrayWhite8;
    HRPPixelTypeV48R16G16B16    m_V48R16G16B16;
    HRPPixelTypeV64R16G16B16A16 m_V64R16G16B16A16;
    HRPPixelTypeV64R16G16B16X16 m_V64R16G16B16X16;
    HRPPixelTypeV16Gray16       m_V16Gray16;
    HRPPixelTypeV16Int16        m_V16Int16;
    HRPPixelTypeV32Float32      m_V32Float32;
    HRPPixelTypeI8R8G8B8Mask    m_I8R8G8B8Mask;
    HRPPixelTypeI8Gray8         m_I8Gray8;
    };
END_IMAGEPP_NAMESPACE
