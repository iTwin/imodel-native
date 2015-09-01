//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARaster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes any graphical object of raster type (graphics that
// are composed of pixels).  Abstract class.
//-----------------------------------------------------------------------------
#pragma once

#include "HGFRaster.h"
#include "HFCAccessMode.h"
#include "HRAIteratorOptions.h"
#include "HRPChannelType.h"
#include "HMGMacros.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARasterIterator;
class HRARasterEditor;
class HPMObjectStore;
class HPMPool;
class HRARepPalParms;
class HRPHistogram;
class HRAHistogramOptions;
class HRAClearOptions;
class HMDContext;
class HRADrawOptions;
class HRACopyFromLegacyOptions;
class HRPPixelType;
class HRPPixelPalette;
class HVEShape;
class HRABitmap;
class HRAStoredRaster;
class IHPMMemoryManager;
class HRPFilter;
struct HRACopyFromOptions;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class HNOVTABLEINIT HRARaster : public HGFRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRARasterId)

public:

    friend class HRARasterEditor;
    friend class HRARasterIterator;

    // Primary methods

    virtual         ~HRARaster();

    // Inherited from HGFGraphicObject

    virtual HGF2DExtent       GetExtent() const override;
    virtual HGF2DExtent       GetExtentInCs(HFCPtr<HGF2DCoordSys> pi_pCoordSys) const;

    // Other methods

    virtual void    CopyFromLegacy    (const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions) = 0;

    virtual void    CopyFromLegacy    (const HFCPtr<HRARaster>& pi_pSrcRaster) = 0;

    virtual void    Clear() = 0;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) = 0;

    virtual HRARasterEditor*     CreateEditor   (HFCAccessMode   pi_Mode)= 0;

    virtual HRARasterEditor*     CreateEditor   (const HVEShape& pi_rShape,
                                                 HFCAccessMode   pi_Mode) = 0;

    virtual HRARasterEditor*     CreateEditorUnShaped (HFCAccessMode pi_Mode)= 0;

    /** -----------------------------------------------------------------------------
        Allocates a new raster iterator that can be used to access the physical rasters
        composing this raster object.  The type of raster iterator will be the type specifically
        defined for the type of current raster.  You are responsible of deleting the raster
        iterator object when you are finished with it.

        If you explicitly specify a shape in the options parameter, the created iterator will
        iterate only on pixels that lie within the intersection of the specified shape
        and the effective shape of the raster.

        If the ClipUsingEffectiveShape setting is set to false (in the options),
        the iterator will assume that the specified shape was already intersected
        with its effective shape. This is only an optimization, and the default setting is
        true, meaning that the iterator will always intersect the specified shape with its
        own effective shape.

        The physical coordinate system given in the options parameter describes the resolution
        to which the most appropriate raster resolution level must be obtained. If the raster
        is not a multi-resolution raster, buffered raster or other rendering raster, then the parameter
        may be ignored. If no coordinate system is provided, then the raster should create
        an it;erator upon the highest resolution possible.

        @h3{Inheritance notes:}
        This method is implemented only in creatable raster classes.
        @end

        @param pi_rOptions  Optional parameter. A constant reference to an HRAIteratorOptions
                            object which defines the iterating parameters
        @end

        @return A pointer to the newly allocated raster iterator object.  This iterator is ready to use.
                It must be deleted by the user.
        @end

        @h3{Word Related Documentation:}
        @list{<a href = "..\..\all\gra\hra\doc\HRARaster.doc">HRARaster previous documentation </a>}
        @end
        -----------------------------------------------------------------------------
    */
    virtual HRARasterIterator*     CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual HFCPtr<HRPPixelType>   GetPixelType() const = 0;

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                             Byte                      pi_Id = 0) const = 0;

    virtual bool   HasSinglePixelType() const = 0;

    virtual bool   IsStoredRaster  () const;


    virtual const HVEShape&         GetShape    () const;
    virtual void    SetShape    (const HVEShape& pi_rShape);

    virtual HFCPtr<HVEShape>    GetEffectiveShape () const = 0;

    virtual HGF2DExtent GetAveragePixelSize () const = 0;

    /** -----------------------------------------------------------------------------
       This method returns the minimum and maximum pixel sizes of the raster. The pixel sizes
    are returned in HGF2DExtent objects that can be expressed in any coordinate system pertinent
    to the representation of the pixel size. Usually, the extent representing the pixel
    size will have sizes 1, 1 and be expressed in the physical coordinate system of the
    component raster of the raster that posseses the smallest/greatest pixel size. If the raster is virtual
    such as an HRAReferenceToRaster, there is no physical storage and thus no physical coordinate
    system. In such case, the raster (an therefore any raster) can elect to use any other coordinate
    system pertinent to the task of representing the pixel dimensions. The caller must
    not however consider that the extent is oriented according to the actual orientation of
    pixel blocks. The extent may be rotated(or sheared or any geometric transformation)
    according to this orientation. If the raster is a composite or a virtual copy of many raster
    to which different geometric transformation have been applied, the pixel size may be an approximation
    of the pixel size. The term "exact pixel dimensions in X and Y" is meaningless in the
    context shearing or projection transformation are applied. The best approximation that can then
    be obtained is the respect of the area occupied by the pixel with or without equivalent
    distribution to the X and Y dimensions. Note that the X and Y orientation of the pixel sizes
    may become meaningless even with a simple rotation or the rasters. If projection is applied,
    the raster may need to transform the original pixel size of a source into another coordinate
    system (such as is the case with a HRAReferenceToRaster class) that may or may not be
    related to the source physical coordinate system through simple stretching. In such case, the raster
    is only required to return a pixel size that has the approximative area across the image source
    only. For example, a raster transformed using projective may have small pixels on one end
    and large pixels on the other. It follows that pixels sizes must be considered approximations
    or more exactly hints concerning the raster resolution. Be careful about usage.<P>

    @PARAM po_rMinimum : An HGF2DExtent that will receive the extent representing the
                         smallest pixel size of the raster.
    @PARAM po_rMaximum : An HGF2DExtent that will receive the extent representing the
                         greatest pixel size of the raster.
    For more details concerning raster file capabilities refer to the HRA User's Guide <P>


    <h3>see </h3>
    <LI><a href = "../../../doc/HRARaster.doc"> HRARaster previous documentation </a></LI>
    -----------------------------------------------------------------------------
    */
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const = 0;

    // If the raster has been created with a Store, you can specify
    // an other in parameters, else you can't.
    virtual HPMPersistentObject* Clone() const override;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const =0;


    virtual unsigned short GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

    virtual bool   StartHistogramEditMode();

    IMAGEPP_EXPORT const HRAHistogramOptions* GetHistogram() const;

    // Debug function
    virtual void    PrintState(ostream& po_rOutput) const;

    // Notifications
    bool   NotifyContentChanged (const HMGMessage& pi_rMessage);

    // Drawing function.
    virtual void        Rotate(double               pi_Angle,
                               const HGF2DLocation& pi_rOrigin) = 0;
    virtual void        Scale(double pi_ScaleFactor,
                              const HGF2DLocation& pi_rOrigin)
        {
        Scale(pi_ScaleFactor, pi_ScaleFactor, pi_rOrigin);
        }
    virtual void        Scale(double pi_ScaleFactorX,
                              double pi_ScaleFactorY,
                              const HGF2DLocation& pi_rOrigin) = 0;

    // Other inherited from HGFGraphicObject
    virtual Location    Locate(const HGF2DLocation& pi_rPoint) const override;

    // LookAhead Methods
    virtual bool   HasLookAhead() const;
    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    virtual bool   IsOpaque() const;
    
    void Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const;

    // This method must be Private, but HRSObjectStore must use it, to load
    // a Raster form a file.
    void            InitializeHistogram(const HRPHistogram&     pi_rHistogram,
                                        const HRPPixelType&     pi_rPixelType);

    //Context related methods
    virtual void                      SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext>        GetContext();

    virtual void                      InvalidateRaster();

    IMAGEPP_EXPORT ImagePPStatus CopyFrom(HRARaster& srcRaster);
    IMAGEPP_EXPORT ImagePPStatus CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options);
        
    ImagePPStatus BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options);

protected:       

    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options) = 0;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) = 0;


    // Primary methods
    HRARaster   ();
    HRARaster   (const HFCPtr<HGF2DCoordSys>& pi_rCoordSys);
    HRARaster   (const HRARaster& pi_rObj);

    HRARaster&      operator=(const HRARaster& pi_rObj);


    // Inherited from HGFGraphicObject
    virtual void    SetCoordSysImplementation   (const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys);


    virtual void    SetShapeImpl                (const HVEShape& pi_rShape);
    virtual void    RecalculateEffectiveShape   ();

    void            UpdateRepPalCache           (unsigned short pi_CountUsed, const HRPPixelPalette& pi_rPalette);
    void            InvalidateRepPalCache       ();
    unsigned short GetRepresentativePaletteCache(
        HRARepPalParms* pio_pRepPalParms);

    void            SetHistogram(const HRAHistogramOptions* pi_pHistogram);

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const = 0;

private:

    // Members

    // Representative palette cache  (Not persistent)
    HAutoPtr<HRPPixelPalette>   m_pRepPalCache;
    bool                       m_RepPalValide;
    unsigned short             m_RepPalCountUsed;

    // (Not persistent) for the moment.
    // Cache the Histogram, for optimize, or for load from a file.
    // Pour etre sauvegarder dans un fichier, il faut que l'histogramme
    // soit a 100% de quality.
    HAutoPtr<HRAHistogramOptions>
    m_pHistogram;

    // Logical Shape on the raster
    HFCPtr<HVEShape>    m_pShape;

    // Methods

    void                InitRepPalCache ();

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };
END_IMAGEPP_NAMESPACE

