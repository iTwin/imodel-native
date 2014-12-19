//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAStoredRaster.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
#include <ImagePP/h/HAutoPtr.h>
#include "HFCStack.h"
#include "HRATransaction.h"

class HRAStoredRaster;
class HRAStoredRasterEditor;
class HRATransactionRecorder;
class HCDPacketRLE;
class HRPFilter;

class HRAStoredRasterTransaction : public HFCStackItem
    {
public:
    HRAStoredRasterTransaction(const HFCPtr<HRATransaction>& pi_pUndo,
                               const HFCPtr<HRATransaction>& pi_pRedo = 0);
    virtual ~HRAStoredRasterTransaction();

    bool                   HasUndoTransaction() const;
    HFCPtr<HRATransaction>  GetUndoTransaction() const;
    void                    SetUndoTransaction(const HFCPtr<HRATransaction>& pi_rpUndo);

    bool                   HasRedoTransaction() const;
    HFCPtr<HRATransaction>  GetRedoTransaction() const;
    void                    SetRedoTransaction(const HFCPtr<HRATransaction>& pi_rpRedo);

    void                    Clear();

    void                    SetSaveBookmark();
    void                    RemoveSaveBookmark();
    bool                   GetSaveBookmark() const;

private:

    bool                       m_SavedBookmark;
    HFCPtr<HRATransaction>      m_pUndo;
    HFCPtr<HRATransaction>      m_pRedo;
    };



class HRAStoredRaster : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1001)

public:

    friend class HRAStoredRasterEditor;

    // Primary methods
    HRAStoredRaster(HRPPixelType*                   pi_pPixelType=NULL,
                    bool                           pi_Resizable = false);
    HRAStoredRaster(uint64_t                       pi_WidthPixels,
                    uint64_t                       pi_HeightPixels,
                    const HGF2DTransfoModel*        pi_pModelCSp_CSl,
                    const HFCPtr<HGF2DCoordSys>&    pi_rpLogicalCoordSys,
                    const HFCPtr<HRPPixelType>&     pi_rpType,
                    bool                           pi_Resizable = false);

    HRAStoredRaster(const HRAStoredRaster& pi_rObj);

    virtual          ~HRAStoredRaster();
    HRAStoredRaster& operator=(const HRAStoredRaster& pi_rObj);


    virtual void                    Saved();
    // From HGFGraphicObject

    virtual void                    Move    (const HGF2DDisplacement&   pi_rDisplacement);
    virtual void                    Rotate  (double                     pi_Angle,
                                             const HGF2DLocation&       pi_rOrigin);
    virtual void                    Scale   (double                     pi_ScaleFactorX,
                                             double                     pi_ScaleFactorY,
                                             const HGF2DLocation&       pi_rOrigin);


    // Inherited from HRARaster

    virtual void                    CopyFrom(const HFCPtr<HRARaster>&   pi_pSrcRaster,
                                             const HRACopyFromOptions&  pi_rOptions);

    virtual void                    CopyFrom(const HFCPtr<HRARaster>&   pi_pSrcRaster);

    virtual void                    PreDraw (HRADrawOptions* pio_pOptions);

    virtual HGF2DExtent             GetAveragePixelSize () const;
    virtual void                    GetPixelSizeRange(HGF2DExtent& po_rMinimum,
                                                      HGF2DExtent& po_rMaximum) const;

    // Catch it, and call the parent
    virtual void                    SetShape    (const HVEShape& pi_rShape);

    virtual HFCPtr<HRPPixelType>    GetPixelType        () const override;

    virtual bool                   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                                              Byte                      pi_Id = 0) const override;

    virtual bool                   HasSinglePixelType  () const;
    bool                           IsStoredRaster      () const;

    // Other methods

    virtual const HFCPtr<HGF2DCoordSys>&
    GetPhysicalCoordSys () const;

    const HFCPtr<HGF2DTransfoModel>&
    GetTransfoModel     () const;
    virtual void                    SetTransfoModel     (const HGF2DTransfoModel& pi_rModelCSp_CSl);
    _HDLLg /*IppImaging_Needs*/void SetTransfoModel     (const HGF2DTransfoModel& pi_rModelCSp_CSl,
                                                         const HFCPtr<HGF2DCoordSys>& pi_rpLogicalCoordSys);


    virtual void                    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels);
    virtual void                    GetSize(uint64_t* po_pWidthPixels, uint64_t* po_pHeightPixels) const;
    virtual HGF2DExtent             GetPhysicalExtent() const;
    virtual HFCPtr<HVEShape>        GetEffectiveShape   () const;

    // re sizable method
    bool                           IsResizable() const;
    virtual HGF2DExtent             GetRasterExtent() const;
    virtual void                    SetRasterExtent(const HGF2DExtent& pi_rRasterExtent);
    virtual void                    GetRasterSize(uint64_t*      po_pWidthPixels,
                                                  uint64_t*      po_pHeightPixels,
                                                  uint64_t*      po_pPosX = 0,
                                                  uint64_t*      po_pPosY = 0) const;



    virtual unsigned short         GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms);

    // Debug function
    virtual void                    PrintState(ostream& po_rOutput) const;

    //Context Methods
    virtual void                    SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext>      GetContext();
    virtual void                    InvalidateRaster();

    // Undo/redo interface
    virtual void                    SetTransactionRecorder(HFCPtr<HRATransactionRecorder>& pi_rpTransactionRecorder);
    virtual const HFCPtr<HRATransactionRecorder>&
    GetTransactionRecorder() const;
    virtual HSTATUS                 StartTransaction();
    virtual HSTATUS                 EndTransaction();
    virtual void                    ClearAllRecordedData();

    virtual void                    SetCurrentTransaction(HFCPtr<HRATransaction>& pi_rpTransaction);
    virtual HFCPtr<HRATransaction>& GetCurrentTransaction();

    virtual bool                   CanUndo() const;
    virtual void                    Undo(bool pi_RecordRedo = true);
    virtual bool                   CanRedo() const;
    virtual void                    Redo();

protected:

    // From HGFGraphicObject
    virtual void        SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpOldCoordSys);

    // From HRARaster
    virtual void        RecalculateEffectiveShape ();

private:

    // Members

    // Smart pointer to the pixel type object.
    HFCPtr<HRPPixelType>    m_pPixelType;

    // Smart pointer to the physical and logical CoordSys.
    HFCPtr<HGF2DCoordSys>   m_pPhysicalCoordSys;

    // TransfoModel between model.
    HFCPtr<HGF2DTransfoModel>   m_pTransfoModel;

protected:   // Could not move this member to keep compatibility with previous version V8i
    //Non persistent changeable layer information
    mutable HFCPtr<HMDContext>          m_pContext;

private:

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
    HFCStack                m_UndoStack;
    HFCStack                m_RedoStack;
    HFCPtr<HRATransactionRecorder>
    m_pTransactionRecorder;

    // Methodes

    void DeepCopy           (const HRAStoredRaster& pi_rObj);
    void DeepDelete         ();

    void SetModelCSp_CSl    (const HGF2DTransfoModel& pi_rModel,
                             bool                    pi_AdjustLogicalShape);

    void ApplyTransaction   (const HFCPtr<HRATransaction>&  pi_rpTransaction);
    };

#include "HRAStoredRaster.hpp"

