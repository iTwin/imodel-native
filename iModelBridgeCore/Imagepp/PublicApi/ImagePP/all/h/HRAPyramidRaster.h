//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPyramidRaster.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAPyramidRaster
//-----------------------------------------------------------------------------
// This class describes an image buffer.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HRAStoredRaster.h"
#include "HGF2DGrid.h"
#include "HGSTypes.h"

BEGIN_IMAGEPP_NAMESPACE
class HIMOnDemandMosaic;
class HRABitmapBase;
class HRAPyramidRasterIterator;
class HRSObjectStore;
class HRABitmapBase;
class HRAClearOptions;
class HRABitmap;
class HRATiledRaster;

class HRAPyramidRaster : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAPyramidRasterId)

    friend class HIMOnDemandMosaic;
    friend class HRAPyramidRasterIterator;
    friend class HRSObjectStore;

public:

    // Used by the constructor, to describe each sub-images.
    class SubImageDescription
        {
    public:
        SubImageDescription()
            {
            ResolutionComputed=true;
            UseDimension=false;
            Resolution=0.0;
            }
        ~SubImageDescription() {};


        // false --> this resolution is not compute or not synchronize
        //           with the main image.
        //DMx
        //  Presently, all sub-image after are considered not compute.
        //
        bool               ResolutionComputed;
        double             Resolution;

        bool                 UseDimension;   // true must have this dimension
        uint32_t            DimX;           // Use to synchronize with a file
        uint32_t            DimY;
        uint32_t            DimBlockX;
        uint32_t            DimBlockY;
        HGSResampling::ResamplingMethod ResamplingType;
        HFCPtr<HRATiledRaster> pTiledRaster;
        };


    // Primary methods

    HRAPyramidRaster ();
    HRAPyramidRaster (HFCPtr<HRATiledRaster>& pi_pRasterModel,
                      uint64_t                pi_WidthPixels,
                      uint64_t                pi_HeightPixels,
                      SubImageDescription*    pi_pSubImageDesc,
                      uint16_t                pi_NumberOfSubImage,
                      HPMObjectStore*         pi_pStore=0,
                      HPMPool*                pi_pLog=0,
                      HFCPtr<HRATiledRaster>  pi_pMainImageRasterModel = 0,
                      bool                    pi_DisableTileStatus = false);   // Optimization if Raster ReadOnly


    HRAPyramidRaster(const HRAPyramidRaster& pi_rObj);

    virtual ~HRAPyramidRaster();

    HRAPyramidRaster& operator=(const HRAPyramidRaster& pi_rObj);


    // Overriden from HRAStoredRaster

    HRARasterIterator*
    CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const override;

    uint16_t GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms) override;

    void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false) override;

    bool   StartHistogramEditMode() override;

    // Special cas
    void    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels) override;

    void    SetShape         (const HVEShape& pi_rShape) override;


    // Catch it, and call the parent
    void    SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl) override;

    HPMPersistentObject* Clone () const override;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    bool   HasSinglePixelType  () const override;

    void            UseOnlyFirstResolution(bool pi_UseOnlyFirst);

    // Include the main image.
    virtual uint16_t CountSubImages              () const;
    virtual double GetSubImagesResolution      (uint16_t pi_SubImageIndex) const;
    IMAGEPP_EXPORT    HGSResampling::ResamplingMethod
    GetResamplingForSubResolution(int32_t pi_SubImageIndex = -1) const;
    virtual bool   SetResamplingForSubResolution(const HGSResampling::ResamplingMethod pi_ResamplingType,
                                                  bool                                 pi_RecomputeSubResolution = false,
                                                  int32_t                              pi_SubImageIndex = -1);
    void            UpdateNextRes               (int32_t pi_SubResolution,
                                                 uint64_t pi_TileIndex,
                                                 HRABitmapBase* pi_pTile);
    IMAGEPP_EXPORT void     UpdateSubResolution         (int32_t           pi_SubResolution = -1,
                                                 const HGF2DExtent* pi_pExtent=0);

    uint64_t       GetNbSubResTilesToUpdate(uint16_t pi_ResIndex, HGF2DExtent& pi_rSurfaceExtent);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;
  
    void    SetContext(const HFCPtr<HMDContext>& pi_rpContext) override;

    void    InvalidateRaster() override;

    // LookAhead Methods
    bool   HasLookAhead() const override;

    void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false) override;

    void            SetLookAheadImpl(const HVEShape& pi_rShape,
                                     uint32_t        pi_ConsumerID,
                                     bool           pi_Async,
                                     int32_t           pi_ResIndex = -1);


    virtual void    GetRasterTileIDList(bvector<TileIdListInfo>& po_rTileIdListInfo, 
                                        const HVEShape&          pi_rShape) override;

    // Message Handler...
    bool           NotifyContentChanged (const HMGMessage& pi_rMessage);
    bool           NotifyProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyHRAProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);
    bool           NotifyGeometryChanged (const HMGMessage& pi_rMessage);
    bool           NotifyModifiedTileNotSaved(const HMGMessage& pi_rMessage);


    // STx: Think of MultiResRaster for the future
    // GetSubImage should be private.
    void            EnableSubImageComputing(bool pi_Enable);
    HFCPtr<HRATiledRaster> GetSubImage (uint16_t pi_Index) const;

    IMAGEPP_EXPORT void            SaveAndFlushAllTiles();

protected:
  
    ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

    uint16_t EvaluateResolution(double Resolution) const;

    virtual ImageSinkNodePtr _GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType) override;

    // From HGFGraphicObject
    void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys) override;

    // Used by HRSObjectStore to enable the LookAhead
    // mechanism for this raster. Should only be called
    // when the underlying RasterFile supports the
    // LookAhead mechanism.
    void            EnableLookAhead(bool pi_ByBlock);

    // Find the best resolution
    double         FindTheBestResolution   (const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                             const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys = 0) const;
private:

    struct ResolutionInfo
        {
        HFCPtr<HRATiledRaster> m_pSubImage;
        double                 m_ImageResolution;
        double                 m_PhysicalImageResolution;
        double                 m_ScaleFromMainX;
        double                 m_ScaleFromMainY;
        bool                   m_ComputeResWithOptimizedMethod;
        bool                   m_SubResolutionIsDirty;         // true--> the image have been dirty
        HGSResampling::ResamplingMethod m_ResamplingType;       // Resampling method to compute the sub-resolutions
        // See define value in the CPP
        ~ResolutionInfo() {m_pSubImage = NULL;}

        };
    // Members

    // No persistent
    HPMPool*                m_pLog;

    // Copy of the Model with an Extent (1,1)
    HFCPtr<HRATiledRaster>             m_pRasterModel;

    // One entry by SubImage, the main image is included.
    struct SubImageList
        {
        SubImageList() : pData(0), BufSize(0) {}
        ResolutionInfo*   pData;
        size_t            BufSize;
        } m_pSubImageList;

    bool                               m_UseOnlyFirstRes;

    bool                               m_ComputeSubImage;


    // Note if our internal RasterFile supports the mechanism.
    // Set by the EnableLookAhead method, default to false.
    bool                               m_LookAheadEnabled;

    // true--> Histogram will be updated after each content modification
    bool                               m_HistogramEditMode;

    // true if all resolution has the same pixel type, false otherwise
    bool                               m_SinglePixelType;

    // Optimzation, should be set true if the raster loaded is readOnly.
    bool                               m_DisableTileStatus;

    // Methods

    void            Constructor    (const HFCPtr<HRATiledRaster>&  pi_rpRasterModel,
                                    const HFCPtr<HRATiledRaster>&  pi_rpMainImageRasterModel,
                                    uint64_t                        pi_WidthPixels,
                                    uint64_t                        pi_HeightPixels,
                                    SubImageDescription*            pi_pSubImageDesc,
                                    uint16_t                 pi_NumberOfSubImage);

    HFCPtr<HRATiledRaster> CreateSubResRaster (const HFCPtr<HRATiledRaster>& pi_rpRasterModel,
                                               uint64_t             pi_Width,
                                               uint64_t             pi_Height,
                                               uint64_t             pi_BlockWidth,
                                               uint64_t             pi_BlockHeight,
                                               bool                 pi_UseBlockSize,
                                               uint64_t             pi_TileID) const;


    void            DeepCopy        (const HRAPyramidRaster& pi_rdRaster,
                                     HPMObjectStore*         pi_pStore =0,
                                     HPMPool*                pi_pLog = 0);

    void            DeepDelete      ();
    void            ReplaceObject   (HFCPtr<HRAPyramidRaster>& pio_pRaster);

    void            SetSubImageNotComputed  (SubImageDescription*  pi_pSubImageDesc,
                                             uint16_t       pi_NumberOfSubImage);

    void            UpdateDirtyFlags(const HVEShape& pi_rShape, uint16_t pi_Res);

    void            UpdateResolution(int32_t           pi_Resolution,
                                     const HGF2DExtent* pi_pExtent);


    SubImageDescription*
    MakeSubImageDescriptor();

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)

    };
END_IMAGEPP_NAMESPACE

#include "HRAPyramidRaster.hpp"

