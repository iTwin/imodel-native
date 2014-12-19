//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSTypes.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSTypes
//-----------------------------------------------------------------------------
// This is the type used by the HGS
//-----------------------------------------------------------------------------

#pragma once


// MemoryAlignment
class HGSMemoryAlignment
    {
    HDECLARE_SEALEDCLASS_ID(8200)

public:

    enum MemoryAlignment
        {
        BYTE,
        DWORD
        };

    HGSMemoryAlignment(MemoryAlignment  pi_Alignment);
    HGSMemoryAlignment(const void*      pi_pRawData,
                       uint32_t         pi_BytesPerRow);
    HGSMemoryAlignment(const HGSMemoryAlignment& pi_rMemoryAlignment);
    ~HGSMemoryAlignment();

    HGSMemoryAlignment&
    operator=(const HGSMemoryAlignment& pi_rObj);
    bool           operator==(const HGSMemoryAlignment& pi_rObj) const;
    bool           operator!=(const HGSMemoryAlignment& pi_rObj) const;

    bool           Supports(const HGSMemoryAlignment& pi_rObj) const;

    HGSMemoryAlignment::MemoryAlignment
    GetMemoryAlignment() const;

private:

    // members
    MemoryAlignment m_MemoryAlignment;

    // disabled method
    HGSMemoryAlignment();
    };

class HGSSurfaceType
    {
    HDECLARE_SEALEDCLASS_ID(8201)

public:

    enum SurfaceType
        {
        MEMORY,
        SCREEN,
        PRINTER,
        METAFILE
        };

    HGSSurfaceType(SurfaceType pi_SurfaceType);
    HGSSurfaceType(const HGSSurfaceType& pi_rSurfaceType);
    ~HGSSurfaceType();

    HGSSurfaceType& operator=(const HGSSurfaceType& pi_rObj);
    bool           operator==(const HGSSurfaceType& pi_rObj) const;
    bool           operator!=(const HGSSurfaceType& pi_rObj) const;

    bool           Supports(const HGSSurfaceType& pi_rObj) const;

    HGSSurfaceType::SurfaceType
    GetSurfaceType() const;

private:

    // members
    SurfaceType m_SurfaceType;

    // disabled method
    HGSSurfaceType();
    };

class HGSTransform
    {
    HDECLARE_SEALEDCLASS_ID(8202)

public:

    enum TransformMethod
        {
        VERTICAL_FLIP
        };

    HGSTransform(TransformMethod pi_TransformMethod);
    HGSTransform(const HGSTransform& pi_rObj);
    ~HGSTransform();

    HGSTransform&
    operator=(const HGSTransform& pi_rObj);
    bool           operator==(const HGSTransform& pi_rObj) const;
    bool           operator!=(const HGSTransform& pi_rObj) const;

    bool           Supports(const HGSTransform& pi_rObj) const;

    HGSTransform::TransformMethod
    GetTransformMethod() const;

private:

    // members
    TransformMethod m_TransformMethod;

    // disabled method
    HGSTransform();
    };


class HGSResampling
    {
    HDECLARE_SEALEDCLASS_ID(8203)

public:

    // IMPORTANT: The neighborhood size must be added in the
    // following GetNeighborhoodSize method.
    enum ResamplingMethod
        {
        NEAREST_NEIGHBOUR=0,
        AVERAGE=1,
        VECTOR_AWARENESS=2,
        CUBIC_CONVOLUTION=3,
        HMR_CUBIC_CONVOLUTION=4,
        DESCARTES_CUBIC_CONVOLUTION=5,
        UNDEFINED=6,
        ORING4=7,
        NONE=8,
        BILINEAR=9
        };

    HGSResampling(ResamplingMethod pi_ResamplingMethod);
    HGSResampling(const HGSResampling& pi_rObj);
    ~HGSResampling();

    HGSResampling&  operator=(const HGSResampling& pi_rObj);
    bool           operator==(const HGSResampling& pi_rObj) const;
    bool           operator!=(const HGSResampling& pi_rObj) const;

    bool           Supports(const HGSResampling& pi_rObj) const;

    HGSResampling::ResamplingMethod
    GetResamplingMethod() const;

    /** ---------------------------------------------------------------------------
        Retrieve the needed pixels on each side of a sample for the
        current resampling mode. Coded here to remind that the size must
        be added to the array when a new method is added to the enum.
        ---------------------------------------------------------------------------
    */
    uint32_t        GetNeighborhoodSize() const
        {
        static uint32_t aSizes[] = {0,  // NEAREST_NEIGHBOUR
                                  0,  // AVERAGE
                                  0,  // VECTOR_AWARENESS
                                  2,  // CUBIC_CONVOLUTION
                                  2,  // HMR_CUBIC_CONVOLUTION
                                  2,  // DESCARTES_CUBIC_CONVOLUTION
                                  0,  // UNDEFINED
                                  0,  // ORING4
                                  0,  // NONE
                                  1
                                 }; // BILINEAR

        return aSizes[m_ResamplingMethod];
        };

private:

    // members
    ResamplingMethod    m_ResamplingMethod;

    // disabled method
    HGSResampling();
    };



class HGSColorConversion
    {
    HDECLARE_SEALEDCLASS_ID(8204)

public:

    enum ColorConversionMethod
        {
        COMPOSE,
        };

    HGSColorConversion(ColorConversionMethod pi_ColorConversionMethod);
    HGSColorConversion(const HGSColorConversion& pi_rObj);
    ~HGSColorConversion();

    HGSColorConversion&
    operator=(const HGSColorConversion& pi_rObj);
    bool           operator==(const HGSColorConversion& pi_rObj) const;
    bool           operator!=(const HGSColorConversion& pi_rObj) const;

    bool           Supports(const HGSColorConversion& pi_rObj) const;

    HGSColorConversion::ColorConversionMethod
    GetColorConversionMethod() const;

private:

    // members
    ColorConversionMethod   m_ColorConversionMethod;

    // disabled method
    HGSColorConversion();
    };

class HGSColor
    {
    HDECLARE_SEALEDCLASS_ID(8205)

public:
    HGSColor();
    HGSColor(const Byte* pi_pRGBAValue);
    HGSColor(const HGSColor& pi_rObj);
    ~HGSColor();

    HGSColor&
    operator=(const HGSColor& pi_rObj);
    bool           operator==(const HGSColor& pi_rObj) const;
    bool           operator!=(const HGSColor& pi_rObj) const;

    bool           Supports(const HGSColor& pi_rObj) const;

    const Byte*   GetValue() const;

private:

    // members
    HArrayAutoPtr<Byte> m_pRGBAValue;
    };

class HGSScanlineMethod
    {
    HDECLARE_SEALEDCLASS_ID(8206)

public:

    enum ScanlineMethod
        {
        GRID,
        };
    HGSScanlineMethod(ScanlineMethod  pi_ScanlineMethod);
    HGSScanlineMethod(const HGSScanlineMethod& pi_rObj);
    ~HGSScanlineMethod();

    HGSScanlineMethod&
    operator=(const HGSScanlineMethod& pi_rObj);
    bool           operator==(const HGSScanlineMethod& pi_rObj) const;
    bool           operator!=(const HGSScanlineMethod& pi_rObj) const;

    bool           Supports(const HGSScanlineMethod& pi_rObj) const;

    HGSScanlineMethod::ScanlineMethod
    GetScanlineMethod() const;

private:

    // members
    ScanlineMethod  m_ScanlineMethod;
    };

class HGSPurpose
    {
    HDECLARE_SEALEDCLASS_ID(8207)

public:

    enum Purposes
        {
        OVERVIEWS,
        };
    HGSPurpose(Purposes  pi_Purpose);
    HGSPurpose(const HGSPurpose& pi_rObj);
    ~HGSPurpose();

    HGSPurpose&
    operator=(const HGSPurpose& pi_rObj);
    bool           operator==(const HGSPurpose& pi_rObj) const;
    bool           operator!=(const HGSPurpose& pi_rObj) const;

    bool           Supports(const HGSPurpose& pi_rObj) const;

    HGSPurpose::Purposes
    GetPurpose() const;

private:

    // members
    Purposes  m_Purpose;
    };

#include "HGSTypes.hpp"

