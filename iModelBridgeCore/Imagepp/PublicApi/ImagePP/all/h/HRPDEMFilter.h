//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPDEMFilter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRPPixelNeighbourhood.h"

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
        // NEVER CHANGE THIS ENUM NUMBERS THEY ARE PERSISTENT.
        Style_Unknown       = 0,
        Style_Elevation     = 1,
        Style_SlopePercent  = 2,
        Style_Aspect        = 3,
        };

    struct _HDLLg RangeInfo
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
        _HDLLg HillShadingSettings();
        _HDLLg HillShadingSettings(unsigned short pi_AltitudeAngle, unsigned short pi_AzimuthDegree);

        _HDLLg bool       GetHillShadingState() const;
        _HDLLg void       SetHillShadingState(bool state);
        _HDLLg unsigned short GetAltitudeAngle() const;
        _HDLLg void       SetAltitudeAngle(unsigned short pi_AltitudeAngle);
        _HDLLg unsigned short GetAzimuthDegree() const;
        _HDLLg void       SetAzimuthDegree(unsigned short pi_AltitudeAngle);


    private:
        unsigned short m_AltitudeAngle;
        unsigned short m_AzimuthDegree;
        bool   m_HillShadingState;
        };

    typedef map<double, RangeInfo, lessDoubleEpsilon> UpperRangeValues;

    _HDLLg                          HRPDEMFilter(const HillShadingSettings& settings, Style pi_Style = Style_Unknown);

    _HDLLg                          HRPDEMFilter(HRPDEMFilter const& object);

    _HDLLg virtual                  ~HRPDEMFilter();

    _HDLLg bool                    GetClipToEndValues() const;
    _HDLLg void                     SetClipToEndValues(bool pi_ClipToEndValues);

    _HDLLg const HillShadingSettings& GetHillShadingSettings() const;
    _HDLLg void                    SetHillShadingSettings(const HillShadingSettings& pi_HillShading);
    _HDLLg unsigned short          GetVerticalExaggeration() const;
    _HDLLg void                    SetVerticalExaggeration(unsigned short pi_VerticalExaggeration);

    _HDLLg Style                    GetStyle() const;
    _HDLLg void                     SetStyle(Style pi_Style);

    _HDLLg const Byte*            GetDefaultColor() const;
    _HDLLg void                     SetDefaultColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue, Byte pi_Alpha);

    //&&MM review this. Not sure is a good idea to provide only get/set this way. It involves too many copy of the map.
    _HDLLg UpperRangeValues const&  GetUpperRangeValues() const;
    _HDLLg void                     SetUpperRangeValues(UpperRangeValues const& pi_rUpperRangeValues);

    _HDLLg HFCPtr<HRPPixelType>     GetOutputPixelType() const;

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