//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageOpMapFilters.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HRAImageOp.h>


IMAGEPP_REF_COUNTED_PTR(HRAImageOpContrastStretchFilter)

BEGIN_IMAGEPP_NAMESPACE 

/*----------------------------------------------------------------------------+
|struct HRAImageOpMapFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpMapFilter : public HRAImageOp
{
public:
    struct MapFilter
        {
        MapFilter();
        virtual ~MapFilter();

        virtual ImagePPStatus _ApplyMap(HRAImageSampleR out, HRAImageSampleCR inputData) const = 0;

        virtual void*       _AllocateChannelMap() = 0;

        virtual void        _SetMapReference(void* pMapRef, uint32_t channelIndex) = 0;
        };

    
protected:
    HRAImageOpMapFilter();
    virtual ~HRAImageOpMapFilter();

    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;

    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) = 0;

    virtual bool IsSupportedPixeltype(HRPPixelType const& pixeltype) const;

    virtual ImagePPStatus GetMatchingPixelType(HFCPtr<HRPPixelType>& pixelType, const HFCPtr<HRPPixelType> pixelTypeToMatch);

    std::unique_ptr<MapFilter> m_pFilter;  
private:
     
    HRAImageOpMapFilter::MapFilter* CreateFilter(HRPPixelType const& pixelType) const;

    ImagePPStatus UpdateFilter();
    };

/*----------------------------------------------------------------------------+
|struct HRAImageOpGammaFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpGammaFilter : public HRAImageOpMapFilter
{
public:
    //! Create a gamma filter. A gamma of 0 is illegal.
    static HRAImageOpPtr CreateGammaFilter(double gamma);

protected: 
    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) override;

private:

    template<class Data_T> void CreateGammaMap_T(Data_T* pMapBuffer);

    HRAImageOpGammaFilter(double gamma);
    virtual ~HRAImageOpGammaFilter();

    double m_gamma;
    std::unique_ptr<Byte> m_pGammaMap;
};


/*----------------------------------------------------------------------------+
|struct HRAImageOpBrightnessFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpBrightnessFilter : public HRAImageOpMapFilter
{
public:
    //! Create a brightness filter. The range of values is [-1, 1]. Where 0 is no brightness.
    static HRAImageOpPtr CreateBrightnessFilter(double brightness);

protected: 
    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) override;

private:
    template<class Data_T> void CreateBrightnessMap_T(Data_T* pMapBuffer, double brightness);
    
    HRAImageOpBrightnessFilter(double brightness);
    virtual ~HRAImageOpBrightnessFilter();

    double m_brightness;
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpContrastFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpContrastFilter : public HRAImageOpMapFilter
{
public:
    //! Create a contrast filter. The range of values is [-1, 1]. Where 0 is no contrast.
    static HRAImageOpPtr CreateContrastFilter(double contrast);

protected: 
    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) override;

private:

    template<class Data_T> void CreateContrastMap_T(Data_T* pMapBuffer);
    
    HRAImageOpContrastFilter(double contrast);
    virtual ~HRAImageOpContrastFilter();

    double m_contrast;  
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpContrastStretchFilter
| In order to create an HistogramScalingFilter:
|       - With InputRangeClipping, use GetInterval/SetInterval.
|       - With OutputRangeCompression, use GetContrastInterval/SetContrastInterval.
+----------------------------------------------------------------------------*/
struct HRAImageOpContrastStretchFilter : public HRAImageOpMapFilter
{
public:
    //! Create a contrast stretch filter. Interval values must be provided in 'intervalPixelType' space.
    //! @remark for now, only 8bit/16bit unsigned values are supported.
    static HRAImageOpContrastStretchFilterPtr CreateContrastStretchFilter(HRPPixelType const& intervalPixelType);

    //! Internal value must be provided in 'intervalPixelType'. 
    //!  8 bits range is [0, 256]. 
    //! 16 bits range is [0, 65535].
    void GetInterval(uint16_t channelIndex, uint16_t& minValue, uint16_t& maxValue) const;
    void SetInterval(uint16_t channelIndex, uint16_t minValue, uint16_t maxValue);

    //! Internal value must be provided in 'intervalPixelType'. 
    //!  8 bits range is [0, 256]. 
    //! 16 bits range is [0, 65535].
    void GetContrastInterval(uint16_t channelIndex, uint16_t& minContrastValue, uint16_t& maxContrastValue) const;
    void SetContrastInterval(uint16_t channelIndex, uint16_t minContrastValue, uint16_t maxContrastValue);

    //! Gamma factor. Default is 0.0 which is no gamma.
    double GetGammaFactor(uint16_t channelIndex) const;
    void SetGammaFactor(uint16_t channelIndex, double gammaFactor);
    
protected: 
    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) override;

    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual bool IsSupportedPixeltype(HRPPixelType const& pixeltype) const override;

private:
    
    template<class Data_T> void CreateContrastStretchMap_T(Data_T* pMapBuffer, Data_T minValue, Data_T maxValue, Data_T minContrast, Data_T maxContrast, double gamma);
    
    HRAImageOpContrastStretchFilter(HRPPixelType const& pixelType);
    virtual ~HRAImageOpContrastStretchFilter();

    struct Values
        {
        union
            {
            Byte values[1];
            uint8_t value8[4];      // At most 4-channels
            uint16_t value16[4];       
            };
        };

    Values               m_minValues;
    Values               m_maxValues;
    Values               m_minContrastValues;
    Values               m_maxContrastValues;
    double               m_gammaFactor[4];      // At most 4-channels    
    HFCPtr<HRPPixelType> m_intervalPixeltype;  
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpInvertFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpInvertFilter : public HRAImageOpMapFilter
{
public:
    //! Create an invert filter.
    static HRAImageOpPtr CreateInvertFilter();

protected: 
    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) override;

private:
    template<class Data_T> void CreateInvertMap_T(Data_T* pMapBuffer);
    
    HRAImageOpInvertFilter();
    virtual ~HRAImageOpInvertFilter();
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpTintFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpTintFilter : public HRAImageOpMapFilter
{
public:
    //! Create a tint filter. No tint would be (255,255,255)
    static HRAImageOpPtr CreateTintFilter(Byte const* pRgbColor);

protected: 
    virtual bool _SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType) override;

    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual bool IsSupportedPixeltype(HRPPixelType const& pixeltype) const override;

private:
    template<class Data_T> void CreateTintMap_T(Data_T* pMapBuffer, Data_T tintFactor);
    
    HRAImageOpTintFilter(Byte const* pRgbColor);
    virtual ~HRAImageOpTintFilter();
    
    Byte m_rgbColor[3];
};

END_IMAGEPP_NAMESPACE