//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMOnDemandMosaic.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"

#include "HGFHVVHSpatialIndex.h"

#include "HIDXComplexIndex.h"

#include "HRAOnDemandRaster.h"
#include "HRAPyramidRaster.h"

#include "HVEVSRelativeIndex.h"
#include "HRAReferenceToRaster.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DTransfoModel;
class HRPPixelType;
class HGF2DCoordSys;
class HIMMosaicIterator;

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
class HIMOnDemandMosaic : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HIMMosaicId_OnDemand)

    friend class HIMMosaicEditor;
    friend class HIMMosaicIterator;

    //:> Types used for the internal index. They must be defined here
    //:> because we use one of their internal types.
    typedef HGFHVVHSpatialIndex< HFCPtr<HRAOnDemandRaster> > SpatialIndexType;
    typedef HVEVSRelativeIndex< HFCPtr<HRAOnDemandRaster>, SpatialIndexType > RelativeIndexType;
    typedef HIDXComplexIndex< HFCPtr<HRAOnDemandRaster>, RelativeIndexType, SpatialIndexType> IndexType;

public:

    typedef IndexType::ObjectList RasterList;

    typedef void* IteratorHandle;

    //:> Primary methods

    IMAGEPP_EXPORT HIMOnDemandMosaic();
        IMAGEPP_EXPORT HIMOnDemandMosaic(const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys, 
                                 const HFCPtr<HFCURL>&           pi_pPSSCacheFileUrl,
                                 const HFCPtr<HRAPyramidRaster>&           pi_rpSubRes, 
                                 HRFDownSamplingMethod::DownSamplingMethod pi_cacheFileDownSamplingMethod);        
               
    IMAGEPP_EXPORT HIMOnDemandMosaic(const HIMOnDemandMosaic& pi_rObj);

    IMAGEPP_EXPORT virtual ~HIMOnDemandMosaic();

    HIMOnDemandMosaic& operator=(const HIMOnDemandMosaic& pi_rObj);

    //:> Overriden from HGFGraphicObject
    virtual HGF2DExtent
    GetExtent() const;

    virtual HGF2DExtent
    GetExtentInCs(HFCPtr<HGF2DCoordSys> pi_pCoordSys) const;

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);

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

    virtual HRARasterEditor*    CreateEditor(HFCAccessMode p_Mode);
    virtual HRARasterEditor*    CreateEditor(const HVEShape& pi_rShape,
                 HFCAccessMode   pi_Mode);
    virtual HRARasterEditor*    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual bool                HasSinglePixelType() const;
    virtual HFCPtr<HRPPixelType> GetPixelType() const;

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const;

    virtual HFCPtr<HVEShape>    GetEffectiveShape () const;

    virtual HGF2DExtent    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum,
                                      HGF2DExtent& po_rMaximum) const;

    static void     GetPixelSizeRange(HFCPtr<HRARaster>            pi_pRaster,
                                      const HFCPtr<HGF2DCoordSys>& pi_rpMosaicCoordSys,
                                      bool&                        pio_rInitialized,
                                      double&                      pio_rMinimumArea,
                                      double&                      pio_rMaximumArea,
                                      HGF2DExtent&                 pio_rMinimum,
                                      HGF2DExtent&                 pio_rMaximum);

    virtual HPMPersistentObject* Clone () const;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    void            SetPixelTypeInfo(bool                        pi_HasSinglePixelType,
                                     const HFCPtr<HRPPixelType>& pi_rpRepresentativePixelType);

    void            SetPrecomputedPixelSizeRange(HGF2DExtent& pi_rMinimum,
                                                 HGF2DExtent& pi_rMaximum);

    //:> Added methods
    virtual void    Add(const RasterList& pi_rRasters);
    bool            Add(const string&                    pi_rOnDemandRastersInfo, 
                        HPMPool*                         pi_pMemPool,
                        const HFCPtr<HGF2DWorldCluster>& pi_rAppWorldCluster,
                        const HFCPtr<HFCURL>&            pi_rpPSSUrl = HFCPtr<HFCURL>());

    //:> Iteration on full rasters composing the mosaic

    IMAGEPP_EXPORT IteratorHandle  StartIteration(void) const;
    IMAGEPP_EXPORT const HFCPtr<HRARaster>    Iterate(IteratorHandle pi_Handle) const;
    IMAGEPP_EXPORT const HFCPtr<HRARaster>    GetElement(IteratorHandle pi_Handle) const;
    const HFCPtr<HVEShape>    GetVisiblePart(IteratorHandle pi_Handle) const;
    IMAGEPP_EXPORT void     StopIteration(IteratorHandle pi_Handle) const;
    IMAGEPP_EXPORT uint32_t CountElements(IteratorHandle pi_Handle) const;

    //:> LookAhead Methods
    virtual bool   HasLookAhead() const;
    virtual void   SetLookAhead(const HVEShape& pi_rShape,
                                uint32_t       pi_ConsumerID,
                                bool            pi_Async = false); 

    //:> Message Handlers
    bool NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);
    bool NotifyGeometryChanged (const HMGMessage& pi_rMessage);
    bool NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    //:> Serialization Methods
    IMAGEPP_EXPORT void  GetOnDemandRastersInfo(Utf8String* po_pOnDemandRastersInfoUTF8) const;

    void  SetRepresentativePSSForWorlds(WString& pi_rPSSDescriptiveNode);

    //:> Cache Methods
        IMAGEPP_EXPORT bool                                     GetSourceFileURLs(ListOfRelatedURLs& po_rRelatedURLs);

        IMAGEPP_EXPORT void CreateCacheFile(const WString&                            pi_rCacheFileName, 
                                    const HFCPtr<HGF2DWorldCluster>&          pi_pWorldCluster, 
                                    HPMPool*                                  pi_pMemoryPool, 
                                    HRFDownSamplingMethod::DownSamplingMethod pi_DownSamplingMethod);

        IMAGEPP_EXPORT HRFDownSamplingMethod::DownSamplingMethod GetCacheFileDownSamplingMethod() const;

               bool                                      HasCache();

        IMAGEPP_EXPORT bool                                      IsCacheFileUpToDate();        
                
        IMAGEPP_EXPORT bool                                      IsSupportingCacheFile();                        

               void  RemoveCache();        

    IMAGEPP_EXPORT bool  SetCacheFile(const WString&                   pi_rCacheFileName,
                                  const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster, 
                                  HPMPool*                         pi_pMemoryPool);

    //:> Miscellaneous Methods
    IMAGEPP_EXPORT void  CreatePssFile(const WString& pi_rFileName) const;

    IMAGEPP_EXPORT void  GetRasterList(HIMOnDemandMosaic::RasterList& po_rRasters) const;
        
           bool  IsDataChangingWithResolution() const;

    IMAGEPP_EXPORT bool  HasUnlimitedRasterSource() const;

    IMAGEPP_EXPORT void  RemoveAll();

protected:
    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;
    
    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options) override;

    static bool    HasSinglePixelType(const HFCPtr<HRARaster>& pi_rpSourceRaster,
                                      HFCPtr<HRPPixelType>&    pio_rpPrevFoundPixelType);

    virtual void    RecalculateEffectiveShape ();

    // The images
    IndexType*      m_pIndex;

    struct OpenRaster
        {
        HFCPtr<HRAOnDemandRaster> m_pOnDemandRaster;
        HFCPtr<HRARaster>         m_pRaster;
        };

    typedef list<OpenRaster> RURasterList;

    mutable RURasterList m_RecentlyUsedRasters;

private:

    void GetRasterSizeInPixel(const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster,
                              uint64_t&                         po_rHeight,
                              uint64_t&                         po_rWidth);

    bool HasSomeRasterLastLoadFailed();

    // Unlink from all rasters in the mosaic
    void              UnlinkFromAll(void);

    IMAGEPP_EXPORT HFCPtr<HRARaster> GetRaster(const HFCPtr<HRAOnDemandRaster>& pi_rpOnDemandRaster) const;

    // Cached effective shape
    mutable HFCPtr<HVEShape>      m_pEffectiveShape;
    mutable HAutoPtr<HGF2DExtent> m_pEffectiveExtent;

    HFCPtr<HRPPixelType>          m_pSetPixelType;
    HAutoPtr<bool>                m_pHasSinglePixelType;

    mutable HAutoPtr<HGF2DExtent> m_pMinimumPixelSizeRange;
    mutable HAutoPtr<HGF2DExtent> m_pMaximumPixelSizeRange;

    WString                       m_WorldDescriptivePSS;
                                    
    //On-demand mosaic cache
    HRFDownSamplingMethod::DownSamplingMethod m_CacheFileDownSamplingMethod;
    double                                    m_onDemandMosaicCacheInfoVersion;
    HFCPtr<HFCURL>                m_pPSSCacheFileUrl;
    HFCPtr<HRAReferenceToRaster>  m_pSubRes;        
                
    // Cached bool value macros. 
    //
    // 1  = True
    // 0  = False
    // -1 = Unknown value
    // Use the HIMONDEMANDMOSAIC_Xxx macros defined in the .cpp file,
    // DON'T USE BOOLEAN VALUES WITH THOSE VARIABLES!
    mutable int32_t m_hasLookAhead;
    mutable int32_t m_isDataChangingWithResolution;
    mutable int32_t m_hasUnlimitedRasterSource;


    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };

END_IMAGEPP_NAMESPACE
#include "HIMOnDemandMosaic.hpp"