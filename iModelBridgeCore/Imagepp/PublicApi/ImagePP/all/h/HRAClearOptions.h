//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAClearOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAClearOptions
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HVEShape;
class HGFScanlines;
class HCDPacketRLE;

class HNOVTABLEINIT HRAClearOptions
    {
public:

    // Primary methods

    IMAGEPP_EXPORT              HRAClearOptions();

    IMAGEPP_EXPORT              HRAClearOptions(const HRAClearOptions& pi_rOptions);

    IMAGEPP_EXPORT              ~HRAClearOptions();


    // Operators

    IMAGEPP_EXPORT HRAClearOptions&  operator=(const HRAClearOptions& pi_rObj);


    // Filters
    IMAGEPP_EXPORT bool                       HasShape() const;
    IMAGEPP_EXPORT const HVEShape*             GetShape() const;
    IMAGEPP_EXPORT void                        SetShape(const HVEShape*    pi_pShape);

    IMAGEPP_EXPORT bool                       HasApplyRasterClipping() const;
    IMAGEPP_EXPORT void                        SetApplyRasterClipping(bool pi_ApplyRasterClipping);

    IMAGEPP_EXPORT bool                       HasScanlines() const;
    IMAGEPP_EXPORT void                        SetScanlines(const HGFScanLines* pi_pScanlines);
    IMAGEPP_EXPORT const HGFScanLines*         GetScanlines() const;

    IMAGEPP_EXPORT bool                       HasRLEMask() const;
    IMAGEPP_EXPORT void                        SetRLEMask(const HCDPacketRLE* pi_pRLEMask);
    IMAGEPP_EXPORT const HCDPacketRLE*         GetRLEMask() const;

    IMAGEPP_EXPORT const void*                 GetRawDataValue() const;
    IMAGEPP_EXPORT void                        SetRawDataValue(const void* pi_pRawData);

    IMAGEPP_EXPORT bool                       HasLoadingData() const;
    IMAGEPP_EXPORT void                        SetLoadingData(bool pi_ForceLoadingData);


protected:

private:

    const HVEShape*             m_pShape;
    bool                       m_ApplyRasterClipping;
    const HGFScanLines*         m_pScanlines;
    const HCDPacketRLE*         m_pRLEMask;
    const void*                 m_pRawData;
    bool                       m_ForceLoadingData;
    };

END_IMAGEPP_NAMESPACE

#include "HRAClearOptions.hpp"

