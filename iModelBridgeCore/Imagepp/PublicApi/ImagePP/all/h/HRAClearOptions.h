//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAClearOptions.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAClearOptions
//-----------------------------------------------------------------------------
#pragma once


class HVEShape;
class HGFScanlines;
class HCDPacketRLE;

class HNOVTABLEINIT HRAClearOptions
    {
public:

    // Primary methods

    _HDLLg              HRAClearOptions();

    _HDLLg              HRAClearOptions(const HRAClearOptions& pi_rOptions);

    _HDLLg              ~HRAClearOptions();


    // Operators

    _HDLLg HRAClearOptions&  operator=(const HRAClearOptions& pi_rObj);


    // Filters
    _HDLLg bool                       HasShape() const;
    _HDLLg const HVEShape*             GetShape() const;
    _HDLLg void                        SetShape(const HVEShape*    pi_pShape);

    _HDLLg bool                       HasApplyRasterClipping() const;
    _HDLLg void                        SetApplyRasterClipping(bool pi_ApplyRasterClipping);

    _HDLLg bool                       HasScanlines() const;
    _HDLLg void                        SetScanlines(const HGFScanLines* pi_pScanlines);
    _HDLLg const HGFScanLines*         GetScanlines() const;

    _HDLLg bool                       HasRLEMask() const;
    _HDLLg void                        SetRLEMask(const HCDPacketRLE* pi_pRLEMask);
    _HDLLg const HCDPacketRLE*         GetRLEMask() const;

    _HDLLg const void*                 GetRawDataValue() const;
    _HDLLg void                        SetRawDataValue(const void* pi_pRawData);

    _HDLLg bool                       HasLoadingData() const;
    _HDLLg void                        SetLoadingData(bool pi_ForceLoadingData);


protected:

private:

    const HVEShape*             m_pShape;
    bool                       m_ApplyRasterClipping;
    const HGFScanLines*         m_pScanlines;
    const HCDPacketRLE*         m_pRLEMask;
    const void*                 m_pRawData;
    bool                       m_ForceLoadingData;
    };

#include "HRAClearOptions.hpp"

