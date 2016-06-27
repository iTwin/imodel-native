//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMath.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMath
//----------------------------------------------------------------------------
#include <ImageppInternal.h>

#include <Imagepp/all/h/HFCMath.h>


HFC_IMPLEMENT_SINGLETON(HFCMath)

/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HFCMath::HFCMath ()
    {
    m_QuotientsShort = new unsigned char[USHRT_MAX+1];
    int32_t Max255 = 256*255;
    for (int32_t i=0; i<Max255; ++i)
        m_QuotientsShort[i] = (Byte)(i / 255);
    memset(&(m_QuotientsShort[Max255]), 255, USHRT_MAX-Max255+1);

    m_MultiBy0X01010101 = new uint32_t[256];
    for (uint32_t i=0; i<256; ++i)
        m_MultiBy0X01010101[i] = i * 0x01010101;
    }

/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HFCMath::~HFCMath()
    {
    }
