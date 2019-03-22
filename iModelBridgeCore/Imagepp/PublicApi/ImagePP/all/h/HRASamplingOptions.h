//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRASamplingOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"
#include "HRPFilter.h"
#include "HFCPtr.h"
#include "HVEShape.h"

/** -----------------------------------------------------------------------------
    This class is a parameter object. It is used to specify sampling
    attributes such as region of interest and percent of coverage.

    @note This class is not meant to be kept for a long time. It is used to
    pass a bunch of parameters together. The filter parameters are not
    owned by this object and are not cloned. Therefore, they could be
    deleted before they are used in a later time.
    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HRASamplingOptions
    {
public:

    //:> Primary methods
    IMAGEPP_EXPORT                 HRASamplingOptions();
    IMAGEPP_EXPORT                  HRASamplingOptions( const HRASamplingOptions& pi_rSamplingOptions);
    IMAGEPP_EXPORT  virtual         ~HRASamplingOptions();

    IMAGEPP_EXPORT  HRASamplingOptions&
    operator=(const HRASamplingOptions& pi_rObj);

    //:> Settings
    void            SetPixelsToScan(Byte m_PixelsToScan);
    Byte          GetPixelsToScan() const;

    void            SetTilesToScan(Byte m_TilesToScan);
    Byte          GetTilesToScan() const;

    void            SetPyramidImageSize(Byte m_ImageSize);
    Byte          GetPyramidImageSize() const;

    const HFCPtr<HRPPixelType>&
    GetSrcPixelTypeReplacer() const;
    void            SetSrcPixelTypeReplacer(const HFCPtr<HRPPixelType>& pi_pPixelType);

    const HFCPtr<HVEShape>&
    GetRegionToScan() const;
    void            SetRegionToScan(const HFCPtr<HVEShape>& pi_rpRegion);

protected:

private:

    //:> private members
    Byte                  m_PixelsToScan;
    Byte                  m_TilesToScan;
    Byte                  m_PyramidImageSize;
    HRPFilter*              m_pFilter;
    HFCPtr<HRPPixelType>    m_pSrcPixelTypeReplacer;

    HFCPtr<HVEShape>        m_pRegionToScan;

    //:> private methods
    void                    DeepCopy(const HRASamplingOptions& pi_rSamplingOptions);
    };
END_IMAGEPP_NAMESPACE

#include "HRASamplingOptions.hpp"
