//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCacheFileCreator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFCacheFileCreator
//-----------------------------------------------------------------------------
// This class describes the stretcher implementation
//-----------------------------------------------------------------------------

#pragma once

#include "HFCMacros.h"
#include "HFCPtr.h"

#include "HRFTypes.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFile;
class HRFRasterFileCapabilities;

//-----------------------------------------------------------------------------
// This is a helper class to instantiate a cache file object
// without knowing the different cache file format.
//-----------------------------------------------------------------------------
class HRFCacheFileCreator
    {
public:
    // Constructor
    HRFCacheFileCreator();
    virtual void PostConstructor();

    // Destructor
    virtual ~HRFCacheFileCreator();

    // This methods allow to know if we have a cache file for the specified raster file.
    virtual bool HasCacheFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                              uint32_t                      pi_Page = -1) const = 0;

    // This factory methods allow to instantiate the cache file for the specified raster file.
    virtual HFCPtr<HRFRasterFile> GetCacheFileFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile,
                                                  uint32_t               pi_Page = -1) const = 0;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const = 0;

    // Pixel Type interface
    virtual uint32_t     CountPixelType() const;
    virtual HCLASS_ID  GetPixelType(uint32_t pi_Index) const;

    // Codec interface
    virtual uint32_t     CountCodecsFor(HCLASS_ID  pi_PixelType) const;
    virtual HCLASS_ID  GetCodecFor   (HCLASS_ID  pi_PixelType,
                                      uint32_t     pi_Index) const;

    virtual void         SelectCodecFor(HCLASS_ID  pi_PixelType,
                                        HCLASS_ID  pi_Codec);


    virtual HCLASS_ID  GetSelectedCodecFor(HCLASS_ID  pi_PixelType) const;

    virtual uint32_t     CountCompressionStepFor(HCLASS_ID  pi_PixelType) const;

    virtual void         SelectCompressionQualityFor(HCLASS_ID  pi_PixelType,
                                                     uint32_t     pi_Quality);

    virtual uint32_t     GetSelectedCompressionQualityFor(HCLASS_ID  pi_PixelType) const;

    // Sub Res Codec interface
    virtual void         SelectSubResCodecFor(HCLASS_ID  pi_PixelType,
                                              HCLASS_ID  pi_Codec);

    virtual HCLASS_ID  GetSelectedSubResCodecFor(HCLASS_ID  pi_PixelType) const;

    virtual void         SelectSubResCompressionQualityFor(HCLASS_ID  pi_PixelType,
                                                           uint32_t     pi_Quality);

    virtual uint32_t     GetSelectedSubResCompressionQualityFor(HCLASS_ID  pi_PixelType) const;

    virtual void         SetDownSamplingMethodForValuesPixelType
    (HRFDownSamplingMethod    pi_DownSamplingMethod);

    virtual void         SetDownSamplingMethodForIndexedPixelType
    (HRFDownSamplingMethod    pi_DownSamplingMethod);

    virtual void         SetDownSamplingMethodFor1BitPixelType
    (HRFDownSamplingMethod    pi_DownSamplingMethod);

    virtual HRFDownSamplingMethod
    GetDownSamplingMethodForValuesPixelType() const;

    virtual HRFDownSamplingMethod
    GetDownSamplingMethodForIndexedPixelType() const;

    virtual HRFDownSamplingMethod
    GetDownSamplingMethodFor1BitPixelType() const;

protected:
    // Selected Codec List Compression
    typedef vector<HCLASS_ID>
    SelectedCodecs;

    typedef vector<uint32_t>
    SelectedCodecsQuality;

    // Implementation Creators registry
    HFCPtr<HRFRasterFileCapabilities>   m_ListOfPixelType;
    SelectedCodecs                      m_SelectedCodecs;
    SelectedCodecsQuality               m_SelectedCodecsQuality;

    // Selected sub resolution codec
    SelectedCodecs                      m_SelectedSubResCodecs;
    SelectedCodecsQuality               m_SelectedSubResCodecsQuality;

    HRFDownSamplingMethod               m_DownSamplingMethodForValuesPixelType;
    HRFDownSamplingMethod               m_DownSamplingMethodForIndexedPixelType;
    HRFDownSamplingMethod               m_DownSamplingMethodFor1BitPixelType;

private:
    // Disabled methods
    HRFCacheFileCreator(const HRFCacheFileCreator&);
    HRFCacheFileCreator& operator=(const HRFCacheFileCreator&);
    };
END_IMAGEPP_NAMESPACE
