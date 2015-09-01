//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPDEMFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRPPixelNeighbourhood.h"
#include <Imagepp/all/h/HRAImageOp.h>    

IMAGEPP_REF_COUNTED_PTR(HRAImageOpDEMFilter)

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;
class HGSMemorySurfaceDescriptor;
class DEMFilterImplementation;
class HGF2DTransfoModel;
class HRPChannelType;

//-----------------------------------------------------------------------------
// HRPDEMFilter
//-----------------------------------------------------------------------------
class HRPDEMFilter : public HFCShareableObject<HRPDEMFilter>
    {
public:

    enum Style
        {
        //&&Backlog We persisted a new type which is Style_Intensity. It's the same as elevation but need a different IDs for DC GUI. 
        // Eval de possibility to remove the persisted enum from IPP and store the persisted enum in rastercore. 
        // The IPP version should renamed Style_Elevation to Style_Value. rastercore would do the mapping.
        // >> if we don't, make sure we do not instantiate 2 versions of the elevation processing.
        // NEVER CHANGE THIS ENUM NUMBERS THEY ARE PERSISTENT.
        Style_Unknown       = 0,
        Style_Elevation     = 1,
        Style_SlopePercent  = 2,
        Style_Aspect        = 3,        
        Style_Intensity     = 4,
        };

    struct IMAGEPP_EXPORT RangeInfo
        {
        RangeInfo()
            {
            m_rgb[0] = 0;
            m_rgb[1] = 0;
            m_rgb[2] = 0;
            m_IsOn = true;
            }

        RangeInfo(Byte pi_Red, Byte pi_Green, Byte pi_Blue, bool pi_IsOn)
            {
            m_rgb[0] = pi_Red;
            m_rgb[1] = pi_Green;
            m_rgb[2] = pi_Blue;
            m_IsOn = pi_IsOn;
            }
        Byte      m_rgb[3];
        bool       m_IsOn;
        };

    struct  HillShadingSettings
        {
        IMAGEPP_EXPORT HillShadingSettings();
        IMAGEPP_EXPORT HillShadingSettings(unsigned short pi_AltitudeAngle, unsigned short pi_AzimuthDegree);

        IMAGEPP_EXPORT bool       GetHillShadingState() const;
        IMAGEPP_EXPORT void       SetHillShadingState(bool state);
        IMAGEPP_EXPORT unsigned short GetAltitudeAngle() const;
        IMAGEPP_EXPORT void       SetAltitudeAngle(unsigned short pi_AltitudeAngle);
        IMAGEPP_EXPORT unsigned short GetAzimuthDegree() const;
        IMAGEPP_EXPORT void       SetAzimuthDegree(unsigned short pi_AltitudeAngle);


    private:
        unsigned short m_AltitudeAngle;
        unsigned short m_AzimuthDegree;
        bool   m_HillShadingState;
        };

    typedef map<double, RangeInfo, lessDoubleEpsilon> UpperRangeValues;

    IMAGEPP_EXPORT                          HRPDEMFilter(const HillShadingSettings& settings, Style pi_Style = Style_Unknown);

    IMAGEPP_EXPORT                          HRPDEMFilter(HRPDEMFilter const& object);

    IMAGEPP_EXPORT virtual                  ~HRPDEMFilter();

    IMAGEPP_EXPORT bool                    GetClipToEndValues() const;
    IMAGEPP_EXPORT void                     SetClipToEndValues(bool pi_ClipToEndValues);

    IMAGEPP_EXPORT const HillShadingSettings& GetHillShadingSettings() const;
    IMAGEPP_EXPORT void                    SetHillShadingSettings(const HillShadingSettings& pi_HillShading);
    IMAGEPP_EXPORT unsigned short          GetVerticalExaggeration() const;
    IMAGEPP_EXPORT void                    SetVerticalExaggeration(unsigned short pi_VerticalExaggeration);

    IMAGEPP_EXPORT Style                    GetStyle() const;
    IMAGEPP_EXPORT void                     SetStyle(Style pi_Style);

    IMAGEPP_EXPORT const Byte*            GetDefaultColor() const;
    IMAGEPP_EXPORT void                     SetDefaultColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue, Byte pi_Alpha);

    IMAGEPP_EXPORT UpperRangeValues const&  GetUpperRangeValues() const;
    IMAGEPP_EXPORT void                     SetUpperRangeValues(UpperRangeValues const& pi_rUpperRangeValues);

    IMAGEPP_EXPORT HFCPtr<HRPPixelType>     GetOutputPixelType() const;

    HRPPixelNeighbourhood           GetNeighbourhood() const;

    void                            ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                                  double pi_ScaleFactorX, double pi_ScaleFactorY);

    void                            SetFor(HFCPtr<HRPPixelType> const& pi_pPixelType, double pi_PixelSizeX, double pi_PixelSizeY, HFCPtr<HGF2DTransfoModel> pi_pOrientationTransfo);

private:

    template<class T>
    DEMFilterImplementation*        CreateFilterImplementation(const HRPChannelType* pi_pChannelType, double pi_PixelSizeX, double pi_PixelSizeY) const;

    template<class T>
    void                            ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource);

    // Disabled
    HRPDEMFilter& operator =(const HRPDEMFilter& object);

    Style                   m_Style;
    UpperRangeValues        m_RangeValues;
    Byte                  m_DefaultRGBAColor[4];

    unsigned short         m_VerticalExaggeration;

    bool                   m_ClipToEndValues;

    HAutoPtr<DEMFilterImplementation>    m_pFilterImpl;

    HillShadingSettings m_HillShadingSettings;  

    };
  
/*----------------------------------------------------------------------------+
|struct HRAImageOpDEMFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpDEMFilter: public HRAImageOp
{
public:

    struct DEMFilterProcessor
        {
        virtual void _ProcessPixels(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) = 0;
        virtual void _ProcessPixelsWithShading(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) = 0;
        };

    static HRAImageOpDEMFilterPtr CreateDEMFilter(HRPDEMFilter::Style style, HRPDEMFilter::UpperRangeValues const& upperRangeValues,
                                                  double unitSizeX, double unitSizeY, HGF2DTransfoModel const& orientation);
        
    // DEM filter settings.
    Byte const* GetDefaultRGBA() const;
    void        SetDefaultRGBA(Byte const* rgba);

    unsigned short GetVerticalExaggeration() const;
    void        SetVerticalExaggeration(unsigned short newExaggeration);

    bool        GetClipToEndValue() const;
    void        SetClipToEndValue(bool clipToEnd);

    HRPDEMFilter::HillShadingSettings const&  GetHillShadingSettings() const;
    void                        SetHillShadingSettings(HRPDEMFilter::HillShadingSettings const& pi_HillShading);

    HRPDEMFilter::UpperRangeValues const&  GetUpperRangeValues() const;

    double GetUnitSizeX() const;
    double GetUnitSizeY() const;

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;


private:

    HRAImageOpDEMFilter(HRPDEMFilter::Style style, HRPDEMFilter::UpperRangeValues const& pi_rUpperRangeValues,
                        double unitSizeX, double unitSizeY, HGF2DTransfoModel const& orientation);

    virtual ~HRAImageOpDEMFilter();

    template<class Src_T> DEMFilterProcessor* CreateProcessor(HRPChannelType const& srcChannelType);

    void UpdatePixelNeighbourhood();
    ImagePPStatus UpdateProcessor();
    
    HRPDEMFilter::Style         m_style; 
    uint32_t                    m_defaultRGBA;
    double                      m_unitSizeX;
    double                      m_unitSizeY;
    HFCPtr<HGF2DTransfoModel>   m_pOrientationTransfo;
    std::unique_ptr<DEMFilterProcessor> m_pDEMFilterProcessor;

    HRPDEMFilter::UpperRangeValues    m_upperRangeValues;
    unsigned short              m_verticalExaggeration;
    HRPDEMFilter::HillShadingSettings m_hillShadingSettings;
    bool                        m_clipToEndValue;
    };

END_IMAGEPP_NAMESPACE
