//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARepPalParms.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRARepPalParms
//-----------------------------------------------------------------------------
// Representative Palette Parameters object definition.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"
#include "HRASamplingOptions.h"
#include "HRPHistogram.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARepPalParms
    {
public:

    // Primary methods
    IMAGEPP_EXPORT                 HRARepPalParms( const HRARepPalParms&       pi_rRepPalParms);
    IMAGEPP_EXPORT                 HRARepPalParms( const HFCPtr<HRPPixelType>& pi_pPixelType,
                                           bool                       pi_ComputeHistogram = false,
                                           uint32_t                    pi_MaxEntries = 0);
    IMAGEPP_EXPORT                 HRARepPalParms( const HFCPtr<HRPPixelType>& pi_pPixelType,
                                           const HRASamplingOptions&   pi_rOptions,
                                           bool                       pi_ComputeHistogram = false,
                                           uint32_t                    pi_MaxEntries = 0);
    IMAGEPP_EXPORT virtual         ~HRARepPalParms();

    // Settings
    const HFCPtr<HRPPixelType>&
    GetPixelType() const;
    void            SetPixelType(const HFCPtr<HRPPixelType>& pi_pPixelType);

    void            SetMaxEntries(uint32_t pi_MaxEntries);
    uint32_t        GetMaxEntries() const;

    IMAGEPP_EXPORT HRPQuantizedPalette*
    CreateQuantizedPalette() const;

    bool           UseCache() const;
    void            SetCacheUse(bool pi_State);

    const HFCPtr<HRPHistogram>&
    GetHistogram() const;
    void            SetHistogram(const HFCPtr<HRPHistogram>& pio_rpHistogram);

    const HRASamplingOptions&
    GetSamplingOptions() const;
    void            SetSamplingOptions(const HRASamplingOptions& pi_rOptions);

protected:

    // Primary methods
    HRARepPalParms();
    HRARepPalParms& operator=(const HRPHistogram& pi_rObj);


private:

    // private members
    HFCPtr<HRPPixelType>    m_pPixelType;
    HRASamplingOptions      m_SamplingOptions;
    bool                   m_UseCache;
    HFCPtr<HRPHistogram>    m_pHistogram;
    uint32_t                m_MaxEntries;

    // private methods
    void                    DeepCopy(const HRARepPalParms& pi_rRepPalParms);
    void                    DeepDelete();
    void                    InitObject( const HFCPtr<HRPPixelType>& pi_pPixelType,
                                        bool                       pi_ComputeHistogram,
                                        uint32_t                    pi_MaxEntries);
    };
END_IMAGEPP_NAMESPACE

#include "HRARepPalParms.hpp"
