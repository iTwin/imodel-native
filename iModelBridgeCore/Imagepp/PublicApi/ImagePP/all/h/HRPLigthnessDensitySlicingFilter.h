//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessDensitySlicingFilter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPLigthnessDensitySlicingFilter
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

// Forward declaration.
class HGFLightnessColorSpace;

class HRPLigthnessDensitySlicingFilter : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(1360, HRPFunctionFilter)
    

public:                       // Primary methods
    HRPLigthnessDensitySlicingFilter();
    HRPLigthnessDensitySlicingFilter(const HFCPtr<HRPPixelType>&     pi_pFilterPixelType);
    HRPLigthnessDensitySlicingFilter(const HRPLigthnessDensitySlicingFilter& pi_rFilter);

    virtual              ~HRPLigthnessDensitySlicingFilter();

    // Cloning
    HRPFilter*     Clone() const override;

    // Get/Set methods
    virtual int32_t    AddSlice (int32_t pi_StartIndex,
                                int32_t pi_EndIndex,
                                uint32_t pi_StartColor,
                                uint32_t pi_EndColor,
                                int32_t pi_Opacity);

    virtual bool     ModifySlice (int32_t pi_StartIndex,
                                   int32_t pi_EndIndex,
                                   uint32_t pi_StartColor,
                                   uint32_t pi_EndColor,
                                   int32_t pi_Opacity);

    virtual int32_t    GetSliceIndex (int32_t pi_StartIndex,
                                     int32_t pi_EndIndex) const;

    virtual bool     GetSliceInfo(int32_t pi_SliceIndex,
                                   int32_t* po_pStartIndex,
                                   int32_t* po_pEndIndex,
                                   int32_t* po_pStartColor,
                                   int32_t* po_pEndColor,
                                   int32_t* po_pOpacity) const;

    virtual void      RemoveSlice (int32_t pi_StartIndex,
                                   int32_t pi_EndIndex);

    virtual void      RemoveSlice (int32_t pi_SliceIndex);

    virtual void      RemoveAll();

    virtual int32_t    GetSliceCount() const;

    _HDLLg  virtual void      SetDesaturationFactor(double pi_DesaturationFactor);
    virtual double   GetDesaturationFactor() const;

protected:
    // methods
    virtual void      Function ( const void*  pi_pSrcRawData,
                                 void*  po_pDestRawData,
                                 uint32_t pi_PixelsCount) const;
    struct SliceInfo
        {
        int32_t m_StartIndex;
        int32_t m_EndIndex;

        uint32_t m_StartColor;
        uint32_t m_EndColor;

        int32_t m_Opacity;
        };

    // members
    vector<SliceInfo > m_SliceList;

    unsigned short     m_Channels;
    unsigned short     m_ChannelWidth;
    int                m_MaxSampleValue;

    double            m_DesaturationFactor;
    HGFLightnessColorSpace* m_pColorSpaceConverter;

private:
    HRPLigthnessDensitySlicingFilter&
    operator=(const HRPLigthnessDensitySlicingFilter& pi_rFilter);
    void            DeepCopy(const HRPLigthnessDensitySlicingFilter& pi_rSrc);

    void            FunctionN8( const void*  pi_pSrcRawData,
                                void*  po_pDestRawData,
                                uint32_t pi_PixelsCount) const;

    void            FunctionN16( const void*  pi_pSrcRawData,
                                 void*  po_pDestRawData,
                                 uint32_t pi_PixelsCount) const;

    void            FunctionPalette( const void*  pi_pSrcRawData,
                                     void*  po_pDestRawData,
                                     uint32_t pi_PixelsCount) const;
    };


