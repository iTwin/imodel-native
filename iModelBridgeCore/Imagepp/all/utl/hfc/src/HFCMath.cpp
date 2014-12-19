//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMath.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMath
//----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCMath.h>


HFC_IMPLEMENT_SINGLETON(HFCMath)

/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HFCMath::HFCMath ()
    {
    m_QuotientsShort = new unsigned char[USHRT_MAX+1];
    for (long i=0; i<USHRT_MAX+1; ++i)
        m_QuotientsShort[i] = (Byte)(i / 255);

    m_MultiBy0X01010101 = new uint32_t[256];
    for (long i=0; i<256; ++i)
        m_MultiBy0X01010101[i] = i * 0x01010101;
    }

/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HFCMath::~HFCMath()
    {
    }
