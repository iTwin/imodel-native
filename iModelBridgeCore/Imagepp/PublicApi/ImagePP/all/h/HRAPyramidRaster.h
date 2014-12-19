//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPyramidRaster.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HIMOnDemandMosaic;
class HRABitmapBase;
class HRAPyramidRasterIterator;
class HRSObjectStore;
class HRABitmapBase;
class HRAClearOptions;

class HRAPyramidRaster : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1041)

    friend class HIMOnDemandMosaic;
    friend class HRAPyramidRasterIterator;
    friend class HRSObjectStore;

    //Data and objects used for drawing. Can be precomputed during the PreDraw call.
    struct DrawingData
        {
    public :

        unsigned short m_ResolutionIndex;
        };

public:

    // Used by the constructor, to discribe each sub-images.
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

        bool               UseDimension;   // true must have this dimension
        uint32_t            DimX;           // Use to synchronise with a file
        uint32_t            DimY;
        uint32_t            DimBlockX;
        uint32_t            DimBlockY;
        HGSResampling::ResamplingMethod
        ResamplingType;
        HFCPtr<HRAStoredRaster>
        pRasterModel;
        };


    // Primary methods

    HRAPyramidRaster ();
    _HDLLg /*IppImaging_Needs*/  HRAPyramidRaster (HFCPtr<HRAStoredRaster>& pi_pRasterModel,
                                                  uint64_t                pi_WidthPixels,
                                                  uint64_t                pi_HeightPixels,
                                                  SubImageDescription*     pi_pSubImageDesc,
                                                  unsigned short          pi_NumberOfSubImage,
                                                  HPMObjectStore*          pi_pStore=0,
                                                  HPMPool*                 pi_pLog=0,
                                                  HFCPtr<HRAStoredRaster>  pi_pMainImageRasterModel = 0,
                                                  bool                    pi_DisableTileStatus = false);   // Optimization if Raster ReadOnly


    HRAPyramidRaster(const HRAPyramidRaster& pi_rObj);

    virtual ~HRAPyramidRaster();

    HRAPyramidRaster& operator=(const HRAPyramidRaster& pi_rObj);


    // Overriden from HRAStoredRaster

    virtual const void*
    GetRawDataPtr () const;

    virtual HRARasterIterator*
    CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual HRARasterEditor*
    CreateEditor(HFCAccessMode pi_Mode);
    virtual HRARasterEditor*
    CreateEditor(const HVEShape& pi_rShape,
                 HFCAccessMode   pi_Mode);
    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual unsigned short GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

    virtual bool   StartHistogramEditMode();

    // Special cas
    virtual void    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels);

    virtual void    SetShape         (const HVEShape& pi_rShape);


    // Catch it, and call the parent
    virtual void    SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl);

    virtual HPMPersistentObject* Clone () const;

    virtual HRARaster*
    Clone (HPMObjectStore* pi_pStore,
           HPMPool*        pi_pLog=0) const;

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const;

    virtual bool   HasSinglePixelType  () const;

    void            UseOnlyFirstResolution(bool pi_UseOnlyFirst);

    // Include the main image.
    virtual unsigned short CountSubImages              () const;
    virtual double GetSubImagesResolution      (unsigned short pi_SubImageIndex) const;
    _HDLLg    HGSResampling::ResamplingMethod
    GetResamplingForSubResolution(int32_t pi_SubImageIndex = -1) const;
    virtual bool   SetResamplingForSubResolution(const HGSResampling::ResamplingMethod pi_ResamplingType,
                                                  bool                                 pi_RecomputeSubResolution = false,
                                                  int32_t                              pi_SubImageIndex = -1);
    void            UpdateNextRes               (int32_t pi_SubResolution,
                                                 uint64_t pi_TileIndex,
                                                 HRABitmapBase* pi_pTile);
    _HDLLg void     UpdateSubResolution         (int32_t           pi_SubResolution = -1,
                                                 const HGF2DExtent* pi_pExtent=0);

    uint64_t       GetNbSubResTilesToUpdate(unsigned short pi_ResIndex, HGF2DExtent& pi_rSurfaceExtent);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;

    // CopyFrom methods
    virtual void    CopyFrom   (const HFCPtr<HRARaster>& pi_rpSrcRaster,
                                const HRACopyFromOptions& pi_rOptions);
    virtual void    CopyFrom   (const HFCPtr<HRARaster>& pi_rpSrcRaster);

    virtual void    PreDraw       (HRADrawOptions* pi_pOptions);

    virtual void    SetContext(const HFCPtr<HMDContext>& pi_rpContext);

    virtual void    InvalidateRaster();

    // LookAhead Methods
    virtual bool   HasLookAhead() const;

    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    void            SetLookAheadImpl(const HVEShape& pi_rShape,
                                     uint32_t        pi_ConsumerID,
                                     bool           pi_Async,
                                     int32_t           pi_ResIndex = -1);

    // Message Handler...
    bool           NotifyContentChanged (const HMGMessage& pi_rMessage);
    bool           NotifyProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyHRAProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);
    bool           NotifyGeometryChanged (const HMGMessage& pi_rMessage);
    bool           NotifyModifiedTileNotSaved(const HMGMessage& pi_rMessage);


    // STx: Think of MultiResRaster for the future
    // GetSubImage should be private.
    _HDLLg /*IppImaging_Needs*/ void            EnableSubImageComputing(bool pi_Enable);
    HFCPtr<HRAStoredRaster>
    GetSubImage             (unsigned short pi_Index) const;

    _HDLLg void            SaveAndFlushAllTiles();

protected:

    // From HGFGraphicObject
    virtual void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys);

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
        HFCPtr<HRAStoredRaster> m_pSubImage;
        double                 m_ImageResolution;
        double                 m_PhysicalImageResolution;
        bool                   m_ComputeResWithOptimizedMethod;
        bool                   m_SubResolutionIsDirty;         // true--> the image have been dirty
        HGSResampling::ResamplingMethod m_ResamplingType;       // Resampling method to compute the sub-resolutions
        // See define value in the CPP
        };
    // Members

    // No persistent
    HPMPool*                m_pLog;

    //Precomputed data storing ID
    uint32_t                  m_StoringID;

    // Copy of the Model with an Extent (1,1)
    HFCPtr<HRAStoredRaster>             m_pRasterModel;

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

    void            Constructor    (const HFCPtr<HRAStoredRaster>&  pi_rpRasterModel,
                                    const HFCPtr<HRAStoredRaster>&  pi_rpMainImageRasterModel,
                                    uint64_t                        pi_WidthPixels,
                                    uint64_t                        pi_HeightPixels,
                                    SubImageDescription*            pi_pSubImageDesc,
                                    unsigned short                 pi_NumberOfSubImage);

    HFCPtr<HRAStoredRaster>
    CreateSubResRaster(const HFCPtr<HRAStoredRaster>&
                       pi_rpRasterModel,
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
                                             unsigned short       pi_NumberOfSubImage);

    void            UpdateDirtyFlags(const HVEShape& pi_rShape, short pi_Res);

    void            UpdateResolution(int32_t           pi_Resolution,
                                     const HGF2DExtent* pi_pExtent);


    SubImageDescription*
    MakeSubImageDescriptor();

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg)

    };

#include "HRAPyramidRaster.hpp"

