//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelConverter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelConverter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPPixelType.h>

//-----------------------------------------------------------------------------
// Constructor with source and dest pixel type pointers
//-----------------------------------------------------------------------------
HRPPixelConverter::HRPPixelConverter(const HRPPixelType* pi_pSourcePixelType,
                                     const HRPPixelType* pi_pDestPixelType)
    {
    HPRECONDITION(pi_pSourcePixelType != 0);
    HPRECONDITION(pi_pDestPixelType != 0);

    m_pSourcePixelType = pi_pSourcePixelType;
    m_pDestPixelType   = pi_pDestPixelType;

    // update possible data used for conversion
    Update();
    }

//-----------------------------------------------------------------------------
// Default constructor.
//-----------------------------------------------------------------------------
HRPPixelConverter::HRPPixelConverter()
    {
    m_pSourcePixelType = 0;
    m_pDestPixelType   = 0;
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRPPixelConverter::HRPPixelConverter(const HRPPixelConverter& pi_rObj)
    {
    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRPPixelConverter::~HRPPixelConverter()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another pixel type, doing a deep copy.
//-----------------------------------------------------------------------------
HRPPixelConverter& HRPPixelConverter::operator=(const HRPPixelConverter& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        DeepDelete();
        DeepCopy(pi_rObj);
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// Convert
//-----------------------------------------------------------------------------
void HRPPixelConverter::ConvertLostChannel(const void*  pi_pSourceRawData,
                                void*        pio_pDestRawData,
                                size_t        pi_PixelsCount,
                                const bool* pi_pChannelsMask) const
    {
    // does nothing by default. this is normal
    }

//-----------------------------------------------------------------------------
// Compose
//-----------------------------------------------------------------------------
void HRPPixelConverter::Compose(const void* pi_pSourceRawData,
                                void*       pio_pDestRawData,
                                size_t      pi_PixelsCount) const
    {
    // this method must be implemented in the child pixel type
    // if there is alpha in the source pixel type
    HPRECONDITION(m_pSourcePixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE);

    // by default, call the convert method
    Convert(pi_pSourceRawData, pio_pDestRawData, pi_PixelsCount);
    }

//-----------------------------------------------------------------------------
// SetDestinationPixelType
//-----------------------------------------------------------------------------
void HRPPixelConverter::SetDestinationPixelType(const HRPPixelType* pi_pDestPixelType)
    {
    HPRECONDITION(pi_pDestPixelType != 0);

    m_pDestPixelType= pi_pDestPixelType;

    // update possible data used for conversion if the two pixel types are defined
    if(m_pSourcePixelType)
        Update();
    }

//-----------------------------------------------------------------------------
// SetSourcePixelType
//-----------------------------------------------------------------------------
void HRPPixelConverter::SetSourcePixelType(const HRPPixelType* pi_pSourcePixelType)
    {
    HPRECONDITION(pi_pSourcePixelType != 0);

    m_pSourcePixelType= pi_pSourcePixelType;

    // update possible data used for conversion if the two pixel types are defined
    if(m_pDestPixelType)
        Update();
    }

//-----------------------------------------------------------------------------
// Update possible private data for conversion
//-----------------------------------------------------------------------------
void HRPPixelConverter::Update()
    {
    }

//-----------------------------------------------------------------------------
// Deletes everything owned by the object
//-----------------------------------------------------------------------------
void HRPPixelConverter::DeepDelete()
    {
    }

//-----------------------------------------------------------------------------
// DeepCopy
//-----------------------------------------------------------------------------
void HRPPixelConverter::DeepCopy (const HRPPixelConverter& pi_rObj)
    {
    m_pSourcePixelType = pi_rObj.m_pSourcePixelType;
    m_pDestPixelType   = pi_rObj.m_pDestPixelType;
    }
