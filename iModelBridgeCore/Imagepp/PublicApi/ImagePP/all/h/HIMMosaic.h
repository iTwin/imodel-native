//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMMosaic.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRARaster.h"
#include "HFCPtr.h"

#include "HVEVSRelativeIndex.h"
#include "HIDXComplexIndex.h"
#include "HGFHVVHSpatialIndex.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DTransfoModel;
class HRPPixelType;
class HGF2DCoordSys;
class HIMMosaicIterator;
class HRAClearOptions;

/** -----------------------------------------------------------------------------
    This class describes a mosaic, a group of images. The images may be
    heterogeneous in size, shape and colorspace. The mosaic simply groups them
    together and is then viewed as a single graphical object.

    The images are arranged in layers, and they may overlap one another. It this
    case, the sources on the top layers will hide parts of the sources that are
    on the lower layers. The order of the sources can be changed using BringToFront,
    SendToBack, Sink and Float.

    Each image contained in the mosaic must inherit from HRARaster. They must also
    use a coordinate system that has a relation with the mosaic's coordinate system.

    Since the mosaic does not have data of its own, the majority of the mosaic
    services work on the source images.
    -----------------------------------------------------------------------------
*/
class HIMMosaic : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HIMMosaicId_Base)

    friend class HIMMosaicEditor;
    friend class HIMMosaicIterator;

    //:> Types used for the internal index. They must be defined here
    //:> because we use one of their internal types.
    typedef HGFHVVHSpatialIndex< HFCPtr<HRARaster> > SpatialIndexType;
    typedef HVEVSRelativeIndex< HFCPtr<HRARaster>, SpatialIndexType > RelativeIndexType;
    typedef HIDXComplexIndex< HFCPtr<HRARaster>, RelativeIndexType, SpatialIndexType> IndexType;

    typedef IndexType::ObjectList RasterList;

public:

    typedef void* IteratorHandle;

    //:> Primary methods

    IMAGEPP_EXPORT HIMMosaic();
    IMAGEPP_EXPORT HIMMosaic(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT HIMMosaic(const HIMMosaic& pi_rObj);

    IMAGEPP_EXPORT virtual ~HIMMosaic();

    HIMMosaic& operator=(const HIMMosaic& pi_rObj);

    //:> Overriden from HGFGraphicObject

    virtual HGF2DExtent
    GetExtent() const;

    virtual HGF2DExtent
    GetExtentInCs(HFCPtr<HGF2DCoordSys> pi_pCoordSys) const;

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);
#if (0)
    virtual void    Rotate(double pi_Angle);
#endif
    virtual void    Rotate(double pi_Angle,
                           const HGF2DLocation& pi_rOrigin);
    virtual void    Scale(double pi_ScaleFactorX,
                          double pi_ScaleFactorY,
                          const HGF2DLocation& pi_rOrigin);

    //Context Methods
    virtual void                      SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext>        GetContext();

    virtual void                      InvalidateRaster();

    //:> Overriden from HRARaster
    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions);

    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;

    virtual HRARasterEditor*
    CreateEditor(HFCAccessMode p_Mode);
    virtual HRARasterEditor*
    CreateEditor(const HVEShape& pi_rShape,
                 HFCAccessMode   pi_Mode);
    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual bool HasSinglePixelType() const;
    virtual HFCPtr<HRPPixelType> GetPixelType() const;
    IMAGEPP_EXPORT         void                 SetPixelType(HFCPtr<HRPPixelType> const& pi_pPixelType);

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const;

    virtual HFCPtr<HVEShape>
    GetEffectiveShape () const;

    virtual HGF2DExtent
    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const;

    virtual HPMPersistentObject* Clone () const;
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    //:> Added methods

    virtual void    Add(const HFCPtr<HRARaster>& pi_pRaster);
    virtual void    Add(const RasterList& pi_rRasters);

    virtual void    AddBefore(const HFCPtr<HRARaster>& pi_pRaster,
                              const HFCPtr<HRARaster>& pi_pBefore = 0);
    virtual void    AddAfter(const HFCPtr<HRARaster>& pi_pRaster,
                             const HFCPtr<HRARaster>& pi_pAfter = 0);

    virtual void    Remove(const HFCPtr<HRARaster>& pi_pRaster);

    virtual void    RemoveAll();

    virtual HFCPtr<HRARaster>
    GetAt(const HGF2DLocation& pi_rPosition) const;

    virtual void    BringToFront(const HFCPtr<HRARaster>& pi_pRaster);
    virtual void    SendToBack(const HFCPtr<HRARaster>& pi_pRaster);

    virtual void    Sink(const HFCPtr<HRARaster>& pi_pRaster);
    virtual void    Float(const HFCPtr<HRARaster>& pi_pRaster);

    virtual bool    Contains(const HFCPtr<HRARaster>& pi_pRaster,
                              bool                    pi_DeepCheck = false) const;

    //:> Iteration on full rasters composing the mosaic

    IteratorHandle  StartIteration(void) const;
    const HFCPtr<HRARaster>
    Iterate(IteratorHandle pi_Handle) const;
    const HFCPtr<HRARaster>
    GetElement(IteratorHandle pi_Handle) const;
    const HFCPtr<HVEShape>
    GetVisiblePart(IteratorHandle pi_Handle) const;
    void            StopIteration(IteratorHandle pi_Handle) const;
    uint32_t        CountElements(IteratorHandle pi_Handle) const;

    //:> LookAhead Methods
    virtual bool   HasLookAhead() const;
    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    //:> Message Handlers
    bool NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);
    bool NotifyGeometryChanged (const HMGMessage& pi_rMessage);
    bool NotifyPaletteChanged (const HMGMessage& pi_rMessage);

protected:

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options) override;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

    virtual void    RecalculateEffectiveShape ();

    // The images
    IndexType*      m_pIndex;

private:

    // Unlink from all rasters in the mosaic
    void            UnlinkFromAll(void);

    // Link to all rasters in the mosaic
    void            LinkToAll(void);

    // Cached effective shape
    mutable HFCPtr<HVEShape>
    m_pEffectiveShape;
    mutable HAutoPtr<HGF2DExtent>
    m_pEffectiveExtent;

    HFCPtr<HRPPixelType> m_pPixelType;

    // Note if we support a LookAhead (cached result)
    // 1  = Yes, we support a LookAhead
    // 0  = No, we don't
    // -1 = Result not cached, we don't know...
    // Use the HIMMOSAIC_Xxx macros defined in the .cpp file,
    // DON'T USE BOOLEAN VALUES WITH THIS VARIABLE!
    mutable int32_t  m_HasLookAhead;

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };

END_IMAGEPP_NAMESPACE
#include "HIMMosaic.hpp"

