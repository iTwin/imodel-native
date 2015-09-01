//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAHistogramOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRPHistogram.h"
#include "HRASamplingOptions.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelPalette;
class HRPPixelType;

/** -----------------------------------------------------------------------------
    This class is a parameter object. It is used to specify histogram computing
    options, as well as to hold the resulting histogram.

    @note This class makes use of an HRASamplingOptions object. This latter
    object has restrictions on its use: we should not keep it for a long time.
    Therefore, an HRAHistogramOptions object should also have a short life.
    Please refer to HRASamplingOptions documentation for details.

  @see HRASamplingOptions
    -----------------------------------------------------------------------------
*/
class HRAHistogramOptions
    {
public:
    //:> Primary methods
    IMAGEPP_EXPORT                 HRAHistogramOptions( const HRAHistogramOptions&     pi_rOptions);

    IMAGEPP_EXPORT                 HRAHistogramOptions( const HRPPixelType*            pi_pPixelType,
                                                HRPHistogram::COLOR_SPACE      pi_ColorSpace = HRPHistogram::NATIVE);

    IMAGEPP_EXPORT                 HRAHistogramOptions( const HFCPtr<HRPHistogram>&    pio_rpHistogram,
                                                const HRPPixelType*            pi_pPixelType);

    IMAGEPP_EXPORT                 HRAHistogramOptions( const HFCPtr<HRPHistogram>&    pio_rpHistogram,
                                                const HRPPixelType*            pi_pPixelType,
                                                const HRASamplingOptions&      pi_rOptions);

    IMAGEPP_EXPORT virtual         ~HRAHistogramOptions();

    //:> Parameters
    HRASamplingOptions&
    GetSamplingOptions() const;
    IMAGEPP_EXPORT void            SetSamplingOptions(const HRASamplingOptions& pi_rOptions);
    const HRPPixelType*
    GetPixelType() const;
    void            SetHistogram(const HFCPtr<HRPHistogram>& pi_rpHistogram);
    const HFCPtr<HRPHistogram>&
    GetHistogram() const;

    void            ClearHistogram();

    void            SetSamplingColorSpace(HRPHistogram::COLOR_SPACE pi_ColorSpace);
    const HRPHistogram::COLOR_SPACE
    GetSamplingColorSpace() const;

    bool           CanBeUsedInPlaceOf(const HRAHistogramOptions& pi_rHistogramOptions) const;

protected:

private:

    //:> private members
    HRASamplingOptions          m_SamplingOptions;
    HFCPtr<HRPPixelType>        m_pPixelType;
    HFCPtr<HRPHistogram>        m_pHistogram;
    HRPHistogram::COLOR_SPACE   m_ColorSpace;

    //:> private methods
    void                    DeepCopy(const HRAHistogramOptions& pi_rObj);

    //:> primary methods not available
    HRAHistogramOptions();
    HRAHistogramOptions&
    operator=(const HRAHistogramOptions& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HRAHistogramOptions.hpp"
