//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordModel.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HGF2DTransfoModel.h>

class HGF2DDisplacement;

// ----------------------------------------------------------------------------
//  HCPGCoordModel
// ----------------------------------------------------------------------------
class HCPGCoordModel : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(1363, HGF2DTransfoModel)

public:
    // Primary methods
    _HDLLg                      HCPGCoordModel();

    _HDLLg                      HCPGCoordModel(IRasterBaseGcsPtr pi_SourceGEOCS,
                                               IRasterBaseGcsPtr pi_DestinationGEOCS);


    _HDLLg                      HCPGCoordModel(const HCPGCoordModel& pi_rObj);
    _HDLLg virtual              ~HCPGCoordModel();
    _HDLLg HCPGCoordModel&      operator=(const HCPGCoordModel& pi_rObj);

    // GCoord model specific interface
    _HDLLg IRasterBaseGcsPtr    GetSourceGEOCS() const;
    _HDLLg IRasterBaseGcsPtr    GetDestinationGEOCS() const;

    void                        StudyReversibilityPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                                                double                pi_Step,
                                                                double*               po_pMeanError,
                                                                double*               po_pMaxError,
                                                                double*               po_pScaleChangeMean,
                                                                double*               po_pScaleChangeMax,
                                                                double                pi_ScaleThreshold = 1.0) const;

    // HGF2DTransfoModel interface

    // Conversion interface
    _HDLLg virtual void         ConvertDirect(double*   pio_pXInOut,
                                              double*   pio_pYInOut) const;

    _HDLLg virtual void         ConvertDirect(double    pi_YIn,
                                              double    pi_XInStart,
                                              size_t     pi_NumLoc,
                                              double    pi_XInStep,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const;

    _HDLLg virtual void         ConvertDirect(double    pi_XIn,
                                              double    pi_YIn,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const;

    _HDLLg virtual void         ConvertInverse(double*   pio_pXInOut,
                                               double*   pio_pYInOut) const;

    _HDLLg virtual void         ConvertInverse(double    pi_YIn,
                                               double    pi_XInStart,
                                               size_t     pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const;

    _HDLLg virtual void         ConvertInverse(double    pi_XIn,
                                               double    pi_YIn,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const;
											   

    // Miscellaneous
    _HDLLg virtual bool         IsIdentity      () const;
    _HDLLg virtual bool         IsStretchable   (double pi_AngleTolerance = 0) const;
    _HDLLg virtual void         GetStretchParams(double*           po_pScaleFactorX,
                                                 double*           po_pScaleFactorY,
                                                 HGF2DDisplacement* po_pDisplacement) const;

    _HDLLg virtual HGF2DTransfoModel* 
                                Clone () const override;
    _HDLLg virtual HFCPtr<HGF2DTransfoModel>
                                ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;


    // Model definition
    _HDLLg virtual bool         CanBeRepresentedByAMatrix() const;
    _HDLLg virtual HFCMatrix<3, 3>
                                GetMatrix() const;

    _HDLLg virtual HFCPtr<HGF2DTransfoModel>
                                CreateSimplifiedModel() const;

    // Geometric properties
    _HDLLg virtual bool         PreservesLinearity() const;
    _HDLLg virtual bool         PreservesParallelism() const;
    _HDLLg virtual bool         PreservesShape() const;
    _HDLLg virtual bool         PreservesDirection() const;

    // Operations
    _HDLLg virtual void         Reverse ();

protected:

    virtual void                Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>
                                ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
private:
    enum ErrorTolerance
        {
        TOLERANCE_NONE,
        TOLERANCE_WARNING,
        TOLERANCE_ERROR
        };

#ifdef HVERIFYCONTRACT
    void                        ValidateInvariants() const;
#endif

    // Private methods
    void                        Copy (const HCPGCoordModel& pi_rObj);

    // Primary attributes
    IRasterBaseGcsPtr     m_SourceGEOCS;
    IRasterBaseGcsPtr     m_DestinationGEOCS;
    };
