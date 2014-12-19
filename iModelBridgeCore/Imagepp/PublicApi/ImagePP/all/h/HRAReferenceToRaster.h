//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAReferenceToRaster.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAReferenceToRaster
//-----------------------------------------------------------------------------
// This class describes a reference to another raster object.
//-----------------------------------------------------------------------------
#pragma once

#include "HRARaster.h"

class HGF2DTransfoModel;
class HGF2DRectangle;
class HRPPixelType;
class HGF2DCoordSys;
class HRAClearOptions;

class HRAReferenceToRaster : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1016)

public:

    // Primary methods

    _HDLLg HRAReferenceToRaster();     // Do not call directly

    _HDLLg HRAReferenceToRaster(const HFCPtr<HRARaster>&     pi_pSource,
                                bool                        pi_Composable = false);
    _HDLLg HRAReferenceToRaster(const HFCPtr<HRARaster>&     pi_pSource,
                                const HVEShape&              pi_rShape,
                                bool                        pi_Composable = false);
    _HDLLg HRAReferenceToRaster(const HFCPtr<HRARaster>&     pi_pSource,
                                const HFCPtr<HGF2DCoordSys>& pi_pCoordSys,
                                bool                        pi_Composable = false);
    _HDLLg HRAReferenceToRaster(const HFCPtr<HRARaster>&     pi_pSource,
                                const HFCPtr<HGF2DCoordSys>& pi_pCoordSys,
                                const HVEShape&              pi_rShape,
                                bool                        pi_Composable = false);

    _HDLLg HRAReferenceToRaster(const HRAReferenceToRaster& pi_rObj);

    _HDLLg virtual ~HRAReferenceToRaster();

    HRAReferenceToRaster& operator=(const HRAReferenceToRaster& pi_rObj);

    // Overriden from HGFGraphicObject

    virtual HGF2DExtent
    GetExtent() const;

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void    Rotate(double               pi_Angle,
                           const HGF2DLocation& pi_rOrigin);
    virtual void    Scale(double pi_ScaleFactorX,
                          double pi_ScaleFactorY,
                          const HGF2DLocation& pi_rOrigin);

    // Overriden from HRARaster

    virtual void    CopyFrom(const HFCPtr<HRARaster>& pi_pSrcRaster,
                             const HRACopyFromOptions& pi_rOptions);

    virtual void    CopyFrom(const HFCPtr<HRARaster>& pi_pSrcRaster);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rClearOptions) override;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions);

    virtual HRARasterEditor*
    CreateEditor(HFCAccessMode pi_Mode);
    virtual HRARasterEditor*
    CreateEditor(const HVEShape& pi_rShape,
                 HFCAccessMode   pi_Mode);
    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual bool HasSinglePixelType() const;
    virtual HFCPtr<HRPPixelType> GetPixelType() const;

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const;

    virtual bool   IsStoredRaster  () const;

    virtual HRARasterIterator*
    CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual unsigned short GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

    // virtual const void*
    //                 GetRawDataPtr () const;

    virtual void    SetShape    (const HVEShape& pi_rShape);

    virtual HFCPtr<HVEShape>
    GetEffectiveShape () const;

    virtual HGF2DExtent
    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const;

    virtual HPMPersistentObject* Clone () const;

    virtual HRARaster*
    Clone (HPMObjectStore* pi_pStore,
           HPMPool*        pi_pLog=0) const;

    // Added methods

    _HDLLg const HFCPtr<HRARaster>& GetSource() const;

    HRAReferenceToRaster*
    TryToCompose(const HVEShape&              pi_rShape);
    HRAReferenceToRaster*
    TryToCompose(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);
    HRAReferenceToRaster*
    TryToCompose(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys,
                 const HVEShape&              pi_rShape);

    // LookAhead Methods
    virtual bool   HasLookAhead() const;
    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    // Debug function
#ifdef __HMR_PRINTSTATE
    virtual void    PrintState(ostream& po_rOutput) const;
#endif

    // Message handlers
    virtual bool NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);
    virtual bool NotifyGeometryChanged       (const HMGMessage& pi_rMessage);
    virtual bool NotifyProgressImageChanged  (const HMGMessage& pi_rMessage);

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const;

    //Context Methods
    virtual void                      SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext>        GetContext();
    virtual void                      InvalidateRaster();

protected:

    virtual void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys);

    virtual void    RecalculateEffectiveShape();

    // Cached effective shape
    HFCPtr<HVEShape>
    m_pEffectiveShape;
    bool           m_EffectiveShapeDirty;

    // Source raster object
    HFCPtr<HRARaster>
    m_pSource;

    // Tell if the reference has a geometry different of the source.
    // i.e. different coordinate system and shape.
    bool           m_ShapeChanged;
    bool           m_CoordSysChanged;

    bool           m_Composable;

    // Set reference's shape to its default value
    void            SetDefaultShape();

private:

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg)
    };

