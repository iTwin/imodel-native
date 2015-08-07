//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMColorBalancedImage.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


#include "HRAImageView.h"

class HRPPixelBuffer;
class HRPHistogram;

/** -----------------------------------------------------------------------------
    This class applies a color balancing "filter" to an image. It is used
    inside the HIMSeamlessMosaic class.

    It first applies a global balance to the image, then does a positional
    algorithm based on the left, right, top and bottom neighbors of the image.

    Supported images are 8 bits grayscale and 24 bits RGB, no compression.

    @see HIMSeamlessMosaic
    -----------------------------------------------------------------------------
*/
class HIMColorBalancedImage : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1273)

    friend class HIMColorBalancedImageIterator;

public:

    enum SamplingQuality
        {
        SAMPLING_FAST,
        SAMPLING_NORMAL,
        SAMPLING_HIGH_QUALITY
        };

    struct RGBDispersion
        {
        RGBDispersion()
            {
            //:> Special case for means computing. The Stddevs must
            //:> NOT stay null.
            MeanRed     = 0.0;
            StddevRed   = 0.0;
            MeanGreen   = 0.0;
            StddevGreen = 0.0;
            MeanBlue    = 0.0;
            StddevBlue  = 0.0;
            }

        RGBDispersion(double pi_MeanRed,
                      double pi_StddevRed,
                      double pi_MeanGreen,
                      double pi_StddevGreen,
                      double pi_MeanBlue,
                      double pi_StddevBlue)
            {
            MeanRed     = pi_MeanRed;
            StddevRed   = pi_StddevRed >= 1.0 ? pi_StddevRed : 1.0;
            MeanGreen   = pi_MeanGreen;
            StddevGreen = pi_StddevGreen >= 1.0 ? pi_StddevGreen : 1.0;
            MeanBlue    = pi_MeanBlue;
            StddevBlue  = pi_StddevBlue >= 1.0 ? pi_StddevBlue : 1.0;
            }

        void SetValues(double pi_MeanRed,
                       double pi_StddevRed,
                       double pi_MeanGreen,
                       double pi_StddevGreen,
                       double pi_MeanBlue,
                       double pi_StddevBlue)
            {
            MeanRed     = pi_MeanRed;
            StddevRed   = pi_StddevRed >= 1.0 ? pi_StddevRed : 1.0;
            MeanGreen   = pi_MeanGreen;
            StddevGreen = pi_StddevGreen >= 1.0 ? pi_StddevGreen : 1.0;
            MeanBlue    = pi_MeanBlue;
            StddevBlue  = pi_StddevBlue >= 1.0 ? pi_StddevBlue : 1.0;
            }

        RGBDispersion& operator+=(const RGBDispersion& pi_rObj)
            {
            MeanRed     += pi_rObj.MeanRed;
            StddevRed   += pi_rObj.StddevRed;
            MeanGreen   += pi_rObj.MeanGreen;
            StddevGreen += pi_rObj.StddevGreen;
            MeanBlue    += pi_rObj.MeanBlue;
            StddevBlue  += pi_rObj.StddevBlue;

            return *this;
            }

        RGBDispersion& operator/=(int pi_Divider)
            {
            MeanRed     /= pi_Divider;
            StddevRed   /= pi_Divider;
            MeanGreen   /= pi_Divider;
            StddevGreen /= pi_Divider;
            MeanBlue    /= pi_Divider;
            StddevBlue  /= pi_Divider;

            return *this;
            }

        bool IsEmpty() const
            {
            return (MeanRed   == 0 && StddevRed   == 0 &&
                    MeanGreen == 0 && StddevGreen == 0 &&
                    MeanBlue  == 0 && StddevBlue  == 0);
            }

        double MeanRed;
        double StddevRed;
        double MeanGreen;
        double StddevGreen;
        double MeanBlue;
        double StddevBlue;
        };

    // Precomputed deltas.
    struct Deltas
        {
        union
            {
            double Gray[256];
            double Red[256];
            };
        };
    struct RGBDeltas : public Deltas
        {
        double Green[256];
        double Blue[256];
        };

    // Color map.
    struct ColorMap
        {
        union
            {
            Byte Gray[256];
            Byte Red[256];
            };

        Byte Green[256];
        Byte Blue[256];
        };

    //:> Primary methods
    HIMColorBalancedImage();

    HIMColorBalancedImage(const HFCPtr<HRARaster>& pi_pSource,
                          bool pi_ApplyGlobalBalance,
                          bool pi_ApplyPositionalBalance);

    HIMColorBalancedImage(const HIMColorBalancedImage& pi_rDitheredImage);

    virtual         ~HIMColorBalancedImage();


    //:> Overriden methods
    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    virtual HRARasterIterator*
    CreateIterator (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions);

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const;

    bool           NotifyContentChanged(HMGMessage& pi_rMessage);

    //:> Added methods

    virtual bool   IsStoredRaster  () const;

    void            InitObject();

    void            SetApplicationShape(const HFCPtr<HVEShape>& pi_rpShape);
    const HFCPtr<HVEShape>&
    GetApplicationShape() const;

    void            ClearNeighbors();

    void            SetLeftNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor);
    void            SetRightNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor);
    void            SetTopNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor);
    void            SetBottomNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor);

    bool           HasLeftNeighbor() const;
    bool           HasRightNeighbor() const;
    bool           HasTopNeighbor() const;
    bool           HasBottomNeighbor() const;
    size_t          GetNumberOfNeighbors() const;

    //:> Preparation methods
    void            SetGlobalDispersion(const RGBDispersion& pi_Dispersion);

    void            RecomputeHistograms();

    void            ApplyPositionalAlgorithm(bool pi_Apply);
    bool           PositionalAlgorithmApplied() const;
    void            ApplyGlobalAlgorithm(bool pi_Apply);
    bool           GlobalAlgorithmApplied() const;

    const RGBDispersion&
    GetLocalDispersionForGlobalAlgorithm();

    bool           Uses(const HFCPtr<HRARaster>& pi_rpRaster) const;

    void            SetSamplingQuality(SamplingQuality pi_Quality);
    SamplingQuality GetSamplingQuality() const;


protected:

    enum ColorMode
        {
        COLORMODE_GRAY,
        COLORMODE_RGB
        };

    ColorMode       GetColorMode() const;

private:

    void            ComputeLeftHistograms();
    void            ComputeRightHistograms();
    void            ComputeTopHistograms();
    void            ComputeBottomHistograms();


    void            ComputeDispersion(HFCPtr<HRPHistogram>& pi_rpHistogram,
                                      RGBDispersion&        pi_rDispersion);

    void            ComputePrimeBasedOnHistogram(HFCPtr<HRPHistogram>& pi_rpHistogram,
                                                 const RGBDispersion&  pi_rLocalDispersionForGlobal,
                                                 RGBDispersion&        po_rDispersionPrime);

    Deltas*         ComputeDeltas(const RGBDispersion& pi_rDispersion,
                                  const RGBDispersion& pi_rNeighborDispersion);

    void            ComputeGlobalMap();

    HFCPtr<HRPHistogram>
    GetHistogramInfoFor(const HFCPtr<HIMColorBalancedImage>& pi_rpImage,
                        const HFCPtr<HVEShape>&  pi_rpShape) const;

    // General balance application (global + positional)
    void                ApplyColorBalanceRGB(HRPPixelBuffer* pi_pSourcePixels,
                                             HRPPixelBuffer* pi_pDestPixels,
                                             int32_t pi_OriginX,
                                             int32_t pi_OriginY,
                                             const HGF2DExtent& pi_rTotalExtent,
                                             int32_t pi_DisplacementX,
                                             int32_t pi_DisplacementY) const;

    // General balance application (positional)
    void                ApplyColorBalanceRGBPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                           HRPPixelBuffer* pi_pDestPixels,
                                                           int32_t pi_OriginX,
                                                           int32_t pi_OriginY,
                                                           const HGF2DExtent& pi_rTotalExtent,
                                                           int32_t pi_DisplacementX,
                                                           int32_t pi_DisplacementY) const;

    // Balance application specific for 4 neighbors (global + positional)
    void                ApplyColorBalanceRGB4(HRPPixelBuffer* pi_pSourcePixels,
                                              HRPPixelBuffer* pi_pDestPixels,
                                              int32_t pi_OriginX,
                                              int32_t pi_OriginY,
                                              const HGF2DExtent& pi_rTotalExtent,
                                              int32_t pi_DisplacementX,
                                              int32_t pi_DisplacementY) const;

    // Balance application specific for 4 neighbors (positional)
    void                ApplyColorBalanceRGB4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                            HRPPixelBuffer* pi_pDestPixels,
                                                            int32_t pi_OriginX,
                                                            int32_t pi_OriginY,
                                                            const HGF2DExtent& pi_rTotalExtent,
                                                            int32_t pi_DisplacementX,
                                                            int32_t pi_DisplacementY) const;

    // Balance application (global)
    void                ApplyColorBalanceRGBGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                       HRPPixelBuffer* pi_pDestPixels,
                                                       int32_t pi_DisplacementX,
                                                       int32_t pi_DisplacementY) const;

    // General balance application (global + positional)
    void                ApplyColorBalanceGray(HRPPixelBuffer* pi_pSourcePixels,
                                              HRPPixelBuffer* pi_pDestPixels,
                                              int32_t pi_OriginX,
                                              int32_t pi_OriginY,
                                              const HGF2DExtent& pi_rTotalExtent,
                                              int32_t pi_DisplacementX,
                                              int32_t pi_DisplacementY) const;

    // General balance application (positional)
    void                ApplyColorBalanceGrayPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                            HRPPixelBuffer* pi_pDestPixels,
                                                            int32_t pi_OriginX,
                                                            int32_t pi_OriginY,
                                                            const HGF2DExtent& pi_rTotalExtent,
                                                            int32_t pi_DisplacementX,
                                                            int32_t pi_DisplacementY) const;

    // Balance application specific for 4 neighbors (global + positional)
    void                ApplyColorBalanceGray4(HRPPixelBuffer* pi_pSourcePixels,
                                               HRPPixelBuffer* pi_pDestPixels,
                                               int32_t pi_OriginX,
                                               int32_t pi_OriginY,
                                               const HGF2DExtent& pi_rTotalExtent,
                                               int32_t pi_DisplacementX,
                                               int32_t pi_DisplacementY) const;

    // Balance application specific for 4 neighbors (positional)
    void                ApplyColorBalanceGray4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                             HRPPixelBuffer* pi_pDestPixels,
                                                             int32_t pi_OriginX,
                                                             int32_t pi_OriginY,
                                                             const HGF2DExtent& pi_rTotalExtent,
                                                             int32_t pi_DisplacementX,
                                                             int32_t pi_DisplacementY) const;

    // Balance application (global)
    void                ApplyColorBalanceGrayGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                        HRPPixelBuffer* pi_pDestPixels,
                                                        int32_t pi_DisplacementX,
                                                        int32_t pi_DisplacementY) const;



    //:> Left neighborhood
    RGBDispersion   m_Left;
    RGBDispersion   m_LeftNeighbor;
    RGBDispersion   m_LeftPrime;
    RGBDispersion   m_LeftNeighborPrime;
    HFCPtr<HRPHistogram>
    m_pLeftHistogram;
    HFCPtr<HRPHistogram>
    m_pLeftNeighborHistogram;
    HFCPtr<HIMColorBalancedImage>
    m_pLeftNeighbor;
    bool           m_HasLeftNeighbor;

    //:> Right neighborhood
    RGBDispersion   m_Right;
    RGBDispersion   m_RightNeighbor;
    RGBDispersion   m_RightPrime;
    RGBDispersion   m_RightNeighborPrime;
    HFCPtr<HRPHistogram>
    m_pRightHistogram;
    HFCPtr<HRPHistogram>
    m_pRightNeighborHistogram;
    HFCPtr<HIMColorBalancedImage>
    m_pRightNeighbor;
    bool           m_HasRightNeighbor;

    //:> Top neighborhood
    RGBDispersion   m_Top;
    RGBDispersion   m_TopNeighbor;
    RGBDispersion   m_TopPrime;
    RGBDispersion   m_TopNeighborPrime;
    HFCPtr<HRPHistogram>
    m_pTopHistogram;
    HFCPtr<HRPHistogram>
    m_pTopNeighborHistogram;
    HFCPtr<HIMColorBalancedImage>
    m_pTopNeighbor;
    bool           m_HasTopNeighbor;

    //:> Bottom neighborhood
    RGBDispersion   m_Bottom;
    RGBDispersion   m_BottomNeighbor;
    RGBDispersion   m_BottomPrime;
    RGBDispersion   m_BottomNeighborPrime;
    HFCPtr<HRPHistogram>
    m_pBottomHistogram;
    HFCPtr<HRPHistogram>
    m_pBottomNeighborHistogram;
    HFCPtr<HIMColorBalancedImage>
    m_pBottomNeighbor;
    bool           m_HasBottomNeighbor;

    //:> Global dispersion information
    RGBDispersion   m_AllGlobal;
    RGBDispersion   m_MyGlobal;

    ColorMap        m_GlobalMap;

    //:> Computed deltas for positional algorithm
    Deltas*         m_pLeftDeltas;
    Deltas*         m_pRightDeltas;
    Deltas*         m_pTopDeltas;
    Deltas*         m_pBottomDeltas;

    bool           m_ApplyGlobal;
    bool           m_ApplyPositional;

    SamplingQuality m_SamplingQuality;

    // Variable region for the positional algorithm
    HFCPtr<HVEShape>
    m_pApplicationShape;

    // Note the pixeltype of our source
    ColorMode       m_ColorMode;
    };

#include "HIMColorBalancedImage.hpp"

