//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMSeamlessMosaic.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HIMMosaic.h"
#include "HFCPtr.h"
#include "HVE2DPolySegment.h"

class HGF2DTransfoModel;
class HRPPixelType;
class HGF2DCoordSys;
class HIMMosaicIterator;
class HIMColorBalancedImage;


/** -----------------------------------------------------------------------------
    This is an interface for a clipping topology system. The user of an
    HIMSeamlessMosaic class must implement a child of the interface to give
    the clipping service to the HIMSeamlessMosaic.

    @see HIMSeamlessMosaic
    -----------------------------------------------------------------------------
*/
class HIMClippingTopology : public HFCShareableObject<HIMClippingTopology>
    {
public:
    virtual ~HIMClippingTopology() {}
    virtual HFCPtr<HVEShape>
    GetPolygon(const HFCPtr<HRARaster>& pi_rpImage) = 0;

    virtual HFCPtr<HVE2DPolySegment>
    GetBoundary(const HFCPtr<HRARaster>& pi_rpImage1,
                const HFCPtr<HRARaster>& pi_rpImage2) = 0;
    };


/** -----------------------------------------------------------------------------
    This is a null clipping manager. It systematically returns null for
    every clipping request. It is used to replace the original manager when the
    mosaic is saved and reloaded (persistence). We don't want to force
    clip managers to be persistent for now.

    @see HIMSeamlessMosaic
    @see HIMClippingTopology
    -----------------------------------------------------------------------------
*/
class HIMNullClipManager : public HIMClippingTopology
    {
public:

    virtual HFCPtr<HVEShape>
    GetPolygon(const HFCPtr<HRARaster>& pi_rpImage)
        {
        return 0;
        };

    virtual HFCPtr<HVE2DPolySegment>
    GetBoundary(const HFCPtr<HRARaster>& pi_rpImage1,
                const HFCPtr<HRARaster>& pi_rpImage2)
        {
        return 0;
        };
    };


/** -----------------------------------------------------------------------------
    This class describes a mosaic that does color balancing and
    image blending.

    The image clipping and frontier determination will be done through an
    object that derives from the HIMClippingTopology interface. This object must
    be implemented by the user and given to the mosaic. Each image will be clipped
    separately, and the frontier between two images will be used to compute the
    blending corridor between the images.

    The following rules must be respected:
    - Images must be 8 bits grayscale or 24 bits RGB. No mixing is allowed.
    - Images must be dodged.
    - Images must all have the same resolution.
    - Each image must have a homogeneous pixelsize and orientation (no mosaics)

    There are two color balancing alrogithms that can be turned on or off
    separately. The global algorithm adjusts each pixel to a global statistical
    distribution. The positional algorithm applies a transformation to each pixel
    depending on the position of the pixel inside the image. The pixel is adjusted
    based on the statistical distributions of the image compared to its close
    neighbors. The positional method takes more time than the global method,
    but gives good results, especially when combined with the global method.

    Blend corridors are computed for each pairs of images, to attenuate the
    visible frontier between the images. The width of the corridors is variable,
    specified by calling SetBlendWidth(). The width is specified in the mosaic's
    logical coordinate system. The default is 1.0.

    @see HIMMosaic
    @see HIMColorBalancedImage
    @see HIMBlendCorridor
    @see HIMClippingTopology
    -----------------------------------------------------------------------------
*/
class HIMSeamlessMosaic : public HIMMosaic
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1272)

public:

    friend class HIMSeamlessMosaicIterator;

    enum SamplingQuality
        {
        SAMPLING_FAST,
        SAMPLING_NORMAL,
        SAMPLING_HIGH_QUALITY
        };

    typedef void* IteratorHandle;

    //:> Primary methods

    HIMSeamlessMosaic();

    HIMSeamlessMosaic(const HFCPtr<HGF2DCoordSys>&       pi_rpCoordSys,
                      const HFCPtr<HIMClippingTopology>& pi_rpClipManager,
                      HPMPool*                           pi_pPool);

    HIMSeamlessMosaic(const HIMSeamlessMosaic& pi_rObj);

    virtual ~HIMSeamlessMosaic();

    HIMSeamlessMosaic& operator=(const HIMSeamlessMosaic& pi_rObj);

    //:> Overriden from HGFGraphicObject

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);
#if (0)
    virtual void    Rotate(double        pi_Angle);
#endif
    virtual void    Rotate(double               pi_Angle,
                           const HGF2DLocation& pi_rOrigin);
    virtual void    Scale(double                pi_ScaleFactorX,
                          double                pi_ScaleFactorY,
                          const HGF2DLocation&  pi_rOrigin);

    //:> Overriden from HRARaster

    virtual void    CopyFrom(  const HFCPtr<HRARaster>& pi_pSrcRaster,
                               const HRACopyFromOptions& pi_rOptions);

    virtual void    CopyFrom(const HFCPtr<HRARaster>& pi_pSrcRaster);

    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;


    //:> From HIMMosaic

    virtual void    Add(const HFCPtr<HRARaster>& pi_pRaster);
    virtual void    Add(const RasterList& pi_rRasters);

    virtual void    AddBefore(const HFCPtr<HRARaster>& pi_pRaster,
                              const HFCPtr<HRARaster>& pi_pBefore = 0);
    virtual void    AddAfter(const HFCPtr<HRARaster>& pi_pRaster,
                             const HFCPtr<HRARaster>& pi_pAfter = 0);

    virtual void    Remove(const HFCPtr<HRARaster>& pi_pRaster);

    virtual void    RemoveAll();

    virtual bool    Contains(const HFCPtr<HRARaster>& pi_pRaster,
                              bool                    pi_DeepCheck = false) const;

    virtual HFCPtr<HRARaster>
    GetAt(const HGF2DLocation& pi_rPosition) const;

    IteratorHandle  StartIteration(void) const;
    const HFCPtr<HRARaster>
    Iterate(IteratorHandle pi_Handle) const;
    const HFCPtr<HRARaster>
    GetElement(IteratorHandle pi_Handle) const;
    const HFCPtr<HVEShape>
    GetVisiblePart(IteratorHandle pi_Handle) const;
    void            StopIteration(IteratorHandle pi_Handle) const;
    uint32_t        CountElements(IteratorHandle pi_Handle) const;

    //:> Message Handlers
    bool NotifyContentChanged (const HMGMessage& pi_rMessage);
    bool NotifyGeometryChanged (const HMGMessage& pi_rMessage);
    bool NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    //:> Color balancing

    void            SetSamplingResolution(unsigned short pi_SamplingResolution);
    void            SetBlendWidth(double pi_BlendWidth);
    double         GetBlendWidth() const;

    void            SetSamplingQuality(SamplingQuality pi_Quality);
    SamplingQuality GetSamplingQuality() const;

    void            ApplyGlobalAlgorithm(bool pi_Apply);
    void            ApplyPositionalAlgorithm(bool pi_Apply);
    bool           GlobalAlgorithmApplied() const;
    bool           PositionalAlgorithmApplied() const;

    void            UpdateClippingOf(const HFCPtr<HRARaster>& pi_rpImage);

protected:


private:

    //////////////////////
    //:> Methods

    //:> Balancing stuff
    void            SetupFilterFor(const HFCPtr<HIMColorBalancedImage>& pi_rpRaster) const;

    HFCPtr<HRARaster>
    AddInternalLayers(const HFCPtr<HRARaster>& pi_rpRaster) const;

    void            UpdateNeighbors(const HFCPtr<HVEShape>& pi_rpUpdateShape);

    bool           RecomputeRegion(const HFCPtr<HVEShape>& pi_rpUpdateShape);

    void            SelectNeighbors(const HFCPtr<HIMColorBalancedImage>& pi_rpRaster,
                                    HFCPtr<HGF2DCoordSys>&               pi_rpPhysicalCS,
                                    HFCPtr<HRARaster>& po_rpLeftNeighbor,
                                    HFCPtr<HRARaster>& po_rpRightNeighbor,
                                    HFCPtr<HRARaster>& po_rpTopNeighbor,
                                    HFCPtr<HRARaster>& po_rpBottomNeighbor) const;

    //:> Synchronize the blends with the images
    void            ManageBlends(const HFCPtr<HVEShape>& pi_rpUpdateShape);

    void            InvalidateBlends();

    void            SetGlobalDispersion();

    HFCPtr<HGF2DCoordSys>
    ObtainPhysicalCoordSys(const HFCPtr<HRARaster>& pi_rpRaster) const;

    bool           IsAValidSource(const HFCPtr<HRARaster>& pi_rpRaster) const;


    //////////////////////
    // Attributes

    /** -------------------------------------------------------------------
        The object that gives us the clipping information
        @note This member is not persistent. Therefore, if the
              mosaic is saved and re-loaded, it will not have the
              clip manager anymore. A default manager will be used, and
              this manager will not give any clipping. If we really want
              the mosaic to be persistent, we'll have to make the
              HIMClippingTopology object persistent.
        -------------------------------------------------------------------
    */
    HFCPtr<HIMClippingTopology>
    m_pClipManager;

    //:> Note which balancing algorithm to apply
    bool           m_ApplyGlobalAlgorithm;
    bool           m_ApplyPositionalAlgorithm;

    // The quality/speed of sampling when computing histograms
    SamplingQuality m_SamplingQuality;

    // The width of the blend corridors
    double         m_BlendWidth;

    // Map the given (external) rasters to the internal representation
    typedef map< HFCPtr<HRARaster>, HFCPtr<HRARaster> > RasterMap;
    RasterMap       m_ImageMap;

    // The blend corridors
    SpatialIndexType*
    m_pBlends;

    // The memory manager to give to the blend corridors
    HPMPool*        m_pPool;

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLNone)
    };


#include "HIMSeamlessMosaic.hpp"

