//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DNonLinearTestIdentity
//-----------------------------------------------------------------------------
// Description of 2D transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF2DTransfoModel.h>

class HGFDistance;


class HGF2DNonLinearTestIdentity : public HGF2DTransfoModel
    {
public:

    // Primary methods
                                HGF2DNonLinearTestIdentity();
                                HGF2DNonLinearTestIdentity(const HGF2DNonLinearTestIdentity& pi_rObj);
    virtual                     ~HGF2DNonLinearTestIdentity();
    HGF2DNonLinearTestIdentity& operator=(const HGF2DNonLinearTestIdentity& pi_rObj);


    virtual bool _IsConvertDirectThreadSafe() const override {return false;}
    virtual bool _IsConvertInverseThreadSafe() const override {return false;}

    // Conversion interface
    virtual StatusInt           _ConvertDirect(double*   pio_pXInOut,
                                              double*   pio_pYInOut) const override;

    virtual StatusInt           _ConvertDirect (double    pi_YIn,
                                               double    pi_XInStart,
                                               size_t    pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_aXOut,
                                               double*   po_aYOut) const override;

    virtual StatusInt           _ConvertDirect(double    pi_XIn,
                                              double    pi_YIn,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const override;


    virtual StatusInt           _ConvertDirect(size_t    pi_NumLoc,
                                              double*   pio_aXInOut,
                                              double*   pio_aYInOut) const override;

    virtual StatusInt           _ConvertInverse(double*   pio_pXInOut,
                                               double*   pio_pYInOut) const override;

    virtual StatusInt           _ConvertInverse(double    pi_YIn,
                                               double    pi_XInStart,
                                               size_t    pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_aXOut,
                                               double*   po_aYOut) const override;


    virtual StatusInt           _ConvertInverse(double    pi_XIn,
                                               double    pi_YIn,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const override;

    virtual StatusInt           _ConvertInverse(size_t    pi_NumLoc,
                                               double*   pio_aXInOut,
                                               double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                _IsIdentity() const override;
    virtual bool                _IsStretchable(double pi_AngleTolerance = 0) const override;
    virtual void                _GetStretchParams(double*  po_pScaleFactorX,
                                                 double*  po_pScaleFactorY,
                                                 HGF2DDisplacement* po_pDisplacement) const override;

    virtual HGF2DTransfoModel*  _Clone() const override;

    virtual HFCPtr<HGF2DTransfoModel>
        _ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const override;
    virtual bool                _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3>     _GetMatrix() const override;
    
    // Geometric properties
    virtual bool                _PreservesLinearity() const override;
    virtual bool                _PreservesParallelism() const override;
    virtual bool                _PreservesShape() const override;
    virtual bool                _PreservesDirection() const override;

    // Operations
    virtual void                _Reverse () override;

protected:
    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override { return nullptr; };

    virtual void                _Prepare () override;

    HFCPtr<HGF2DTransfoModel>   _ComposeYourself(const HGF2DTransfoModel& pi_rModel) const override;
    };
