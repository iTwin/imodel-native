//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAStoredRaster
//-----------------------------------------------------------------------------
// Class that defines physical stored rasters
//-----------------------------------------------------------------------------


#pragma once

#include "HRARaster.h"
#include "HGF2DExtent.h"
#include "HGF2DCoordSys.h"
#include "HVEShape.h"

BEGIN_IMAGEPP_NAMESPACE
class HRATransactionRecorder;
class HRATransaction;
class HCDPacketRLE;
class HRPFilter;
class HFCInclusiveGrid;
//class IHPMMemoryManager;
class HRAStoredRasterTransaction;

class HRAStoredRaster : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAStoredRasterId_Base)

public:

    // Primary methods
    HRAStoredRaster(HRPPixelType*                   pi_pPixelType=NULL,
                    bool                            pi_Resizable = false);
    HRAStoredRaster(uint64_t                        pi_WidthPixels,
                    uint64_t                        pi_HeightPixels,
                    const HGF2DTransfoModel*        pi_pModelCSp_CSl,
                    const HFCPtr<HGF2DCoordSys>&    pi_rpLogicalCoordSys,
                    const HFCPtr<HRPPixelType>&     pi_rpType,
                    bool                            pi_Resizable = false);

    HRAStoredRaster(const HRAStoredRaster& pi_rObj);

    virtual          ~HRAStoredRaster();
    HRAStoredRaster& operator=(const HRAStoredRaster& pi_rObj);

    virtual void                    Saved();
    // From HGFGraphicObject

    void                    Move    (const HGF2DDisplacement&   pi_rDisplacement) override;
    void                    Rotate  (double                     pi_Angle,
                                             const HGF2DLocation&       pi_rOrigin) override;
    void                    Scale   (double                     pi_ScaleFactorX,
                                             double                     pi_ScaleFactorY,
                                             const HGF2DLocation&       pi_rOrigin) override;


    // Inherited from HRARaster

    HGF2DExtent             GetAveragePixelSize () const override;
    void                    GetPixelSizeRange(HGF2DExtent& po_rMinimum,
                                                      HGF2DExtent& po_rMaximum) const override;

    // Catch it, and call the parent
    void                    SetShape    (const HVEShape& pi_rShape) override;

    virtual HFCPtr<HRPPixelType>    GetPixelType        () const override;

    virtual bool                    ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                                              Byte                      pi_Id = 0) const override;

    bool                    HasSinglePixelType  () const override;
    bool                            IsStoredRaster      () const override;

    // Other methods

    virtual const HFCPtr<HGF2DCoordSys>&
                                    GetPhysicalCoordSys () const;

    const HFCPtr<HGF2DTransfoModel>&
                                    GetTransfoModel     () const;

    //&&Backlog review the 2 SetTransfoModel. One is virtual and the other is not. Do we need to override both? look for HRAStoredRaster::SetTransfoModel calls.
    virtual void                    SetTransfoModel     (const HGF2DTransfoModel& pi_rModelCSp_CSl);
    void SetTransfoModel     (const HGF2DTransfoModel& pi_rModelCSp_CSl,
                                                         const HFCPtr<HGF2DCoordSys>& pi_rpLogicalCoordSys);


    virtual void                    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels);
    virtual void                    GetSize(uint64_t* po_pWidthPixels, uint64_t* po_pHeightPixels) const;
    virtual HGF2DExtent             GetPhysicalExtent() const;
    HFCPtr<HVEShape>        GetEffectiveShape   () const override;

    // re sizable method
    bool                            IsResizable() const;
    virtual HGF2DExtent             GetRasterExtent() const;
    virtual void                    SetRasterExtent(const HGF2DExtent& pi_rRasterExtent);
    virtual void                    GetRasterSize(uint64_t*      po_pWidthPixels,
                                                  uint64_t*      po_pHeightPixels,
                                                  uint64_t*      po_pPosX = 0,
                                                  uint64_t*      po_pPosY = 0) const;



    uint16_t          GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms) override;

    // Debug function
    void                    PrintState(ostream& po_rOutput) const override;

    //Context Methods
    void                    SetContext(const HFCPtr<HMDContext>& pi_rpContext) override;
    HFCPtr<HMDContext>      GetContext() override;
    void                    InvalidateRaster() override;

    // Undo/redo interface
    virtual void                    SetTransactionRecorder(HFCPtr<HRATransactionRecorder>& pi_rpTransactionRecorder);
    virtual const HFCPtr<HRATransactionRecorder>&
    GetTransactionRecorder() const;
    virtual HSTATUS                 StartTransaction();
    virtual HSTATUS                 EndTransaction();
    virtual void                    ClearAllRecordedData();

    virtual void                    SetCurrentTransaction(HFCPtr<HRATransaction>& pi_rpTransaction);
    virtual HFCPtr<HRATransaction>& GetCurrentTransaction();

    virtual bool                    CanUndo() const;
    virtual void                    Undo(bool pi_RecordRedo = true);
    virtual bool                    CanRedo() const;
    virtual void                    Redo();

    ImageSinkNodePtr GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType> pReplacingPixelType);
    
protected:       
    
    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions) override;

    //! Editable raster must implement this methods.
    virtual ImageSinkNodePtr _GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType) = 0;

    // From HGFGraphicObject
    void        SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpOldCoordSys) override;

    // From HRARaster
    void        RecalculateEffectiveShape () override;

    static double EvaluateScaleFactor(HFCPtr<HGF2DCoordSys> const& srcCS, HFCPtr<HGF2DCoordSys> const& dstCS, HVEShape const& shape);
    static uint32_t EvaluateScaleFactorPowerOf2(HFCPtr<HGF2DCoordSys> const& srcCS, HFCPtr<HGF2DCoordSys> const& dstCS, HVEShape const& shape);

    //Non persistent changeable layer information
    mutable HFCPtr<HMDContext>  m_pContext;

private:

    // Members

    // Smart pointer to the pixel type object.
    HFCPtr<HRPPixelType>    m_pPixelType;

    // Smart pointer to the physical and logical CoordSys.
    HFCPtr<HGF2DCoordSys>   m_pPhysicalCoordSys;

    // TransfoModel between model.
    HFCPtr<HGF2DTransfoModel>   m_pTransfoModel;

    // Physical shape for the raster
    //  Physical CoordSys.
    HFCPtr<HVEShape>            m_pPhysicalRect;

    // re sizable support
    bool                       m_Resizable;
    HFCPtr<HVEShape>            m_pRasterPhysicalRect;

    // Keep the copy of the current effective shape only for the performance.
    //  Physical CoordSys.
    HFCPtr<HVEShape>            m_pEffectiveShape;

    // Temporary Members, not save in the HOD
    // Only use to optimize the CopyFrom

    // For optimisation only, use to skip the call to SetShape on each tiles
    bool                   m_HasSetShape;

    // Undo/Redo stack
    HFCPtr<HRATransaction>  m_pCurrentTransaction;
    std::stack<HFCPtr<HRAStoredRasterTransaction>> m_UndoStack;
    std::stack<HFCPtr<HRAStoredRasterTransaction>> m_RedoStack;
    HFCPtr<HRATransactionRecorder> m_pTransactionRecorder;

    // Methodes

    void DeepCopy           (const HRAStoredRaster& pi_rObj);
    void DeepDelete         ();

    void SetModelCSp_CSl    (const HGF2DTransfoModel& pi_rModel,
                             bool                    pi_AdjustLogicalShape);

    void ApplyTransaction   (const HFCPtr<HRATransaction>&  pi_rpTransaction);
    };
END_IMAGEPP_NAMESPACE

#include "HRAStoredRaster.hpp"

