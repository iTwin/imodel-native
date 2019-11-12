//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAUnlimitedResolutionRaster
//-----------------------------------------------------------------------------
// This class describes an unlimited resolution raster.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HRAStoredRaster.h"
#include "HGF2DGrid.h"
#include "HGSTypes.h"

BEGIN_IMAGEPP_NAMESPACE
class HRAUnlimitedResolutionRasterIterator;
class HRSObjectStore;
class HRABitmap;
class HRATiledRaster;

class HRAUnlimitedResolutionRaster : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAUnlimitedResolutionRasterId)

    friend class HRAUnlimitedResolutionRasterIterator;
    friend class HRSObjectStore;

public:

    // Primary methods
    HRAUnlimitedResolutionRaster ();
    HRAUnlimitedResolutionRaster (HFCPtr<HRATiledRaster>& pi_pRasterModel,
                                  uint64_t                pi_WidthPixels,
                                  uint64_t                pi_HeightPixels,
                                  uint64_t                pi_MinWidth,
                                  uint64_t                pi_MinHeight,
                                  uint64_t                pi_MaxWidth,
                                  uint64_t                pi_MaxHeight,
                                  HPMObjectStore*          pi_pStore=0,
                                  HPMPool*                 pi_pLog=0);

    HRAUnlimitedResolutionRaster(const HRAUnlimitedResolutionRaster& pi_rObj);

    virtual ~HRAUnlimitedResolutionRaster();

    HRAUnlimitedResolutionRaster& operator=(const HRAUnlimitedResolutionRaster& pi_rObj);


    // Overriden from HRAStoredRaster

    HRARasterIterator*      CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const override;

    uint16_t         GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms) override;

    void                    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                                     bool                pi_ForceRecompute = false) override;

    bool                   StartHistogramEditMode() override;

    virtual void                    GetMaxResolutionSize(uint64_t* po_pMaxWidth,
                                                         uint64_t* po_pMaxHeight);
    // Special case
    void                    InitSize(uint64_t pi_WidthPixels,
                                             uint64_t pi_HeightPixels) override;

    void                    SetShape(const HVEShape& pi_rShape) override;


    // Catch it, and call the parent
    void                    SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl) override;

    HPMPersistentObject* Clone () const override;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    //Context Methods
    void                    SetContext(const HFCPtr<HMDContext>& pi_rpContext) override;

    void                    InvalidateRaster() override;

    bool                   HasSinglePixelType  () const override;


    // Include the main image.
    virtual void                    Clear() override;
    virtual void                    Clear(const HRAClearOptions& pi_rOptions) override;

    // LookAhead Methods
    bool                   HasLookAhead() const override;
    void                 SetLookAhead(const HVEShape& pi_rShape,
                                                 uint32_t        pi_ConsumerID,
                                                 bool           pi_Async = false) override;
    // Message Handler...
    bool           NotifyContentChanged (const HMGMessage& pi_rMessage);
    bool           NotifyProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyHRAProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);
    bool           NotifyGeometryChanged (const HMGMessage& pi_rMessage);


protected:

    //! Editing of unlimited is not supported.
    virtual ImageSinkNodePtr _GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType) override;
    
    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

    // From HGFGraphicObject
    void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys) override;

    // Used by HRSObjectStore to enable the LookAhead
    // mechanism for this raster. Should only be called
    // when the underlying RasterFile supports the
    // LookAhead mechanism.
    void            EnableLookAhead(bool pi_ByBlock);

private:

    // Members

    // No persistent
    HPMPool*                        m_pLog;
    HFCPtr<HRATiledRaster>         m_pCurrentResolutionRaster;

    bool                           m_LookAheadEnabled;
    bool                           m_LookAheadByBlockEnabled;
    double                         m_CurResolution;

    // Copy of the Model with an Extent (1,1)
    HFCPtr<HRATiledRaster>         m_pTiledRaster;


    // true--> Histogram will be updated after each content modification
    bool                           m_HistogramEditMode;

    // true if all resolution has the same pixel type, false otherwise
    bool                           m_SinglePixelType;

    uint64_t                       m_MinWidth;
    uint64_t                       m_MinHeight;
    uint64_t                       m_MaxWidth;
    uint64_t                       m_MaxHeight;
    double                         m_MinResolution;
    double                         m_MaxResolution;

    // Methods

    void                Constructor             (const HFCPtr<HRATiledRaster>&         pi_rpRasterModel,
                                                 uint64_t                              pi_WidthPixels,
                                                 uint64_t                              pi_HeightPixels,
                                                 uint64_t                              pi_MinWidth,
                                                 uint64_t                              pi_MinHeight,
                                                 uint64_t                              pi_MaxWidth,
                                                 uint64_t                              pi_MaxHeight);

    void                DeepCopy                (const HRAUnlimitedResolutionRaster&    pi_rdRaster,
                                                 HPMObjectStore*                        pi_pStore =0,
                                                 HPMPool*                               pi_pLog = 0);

    void                DeepDelete      ();

    double             FindTheBestResolution   (const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                                 const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys = 0) const;
    void                ChangeResolution        (double pi_Resolution);
    HFCPtr<HRARaster>   CreateSubImage          (double pi_Resolution) const;


    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)

    };
END_IMAGEPP_NAMESPACE

