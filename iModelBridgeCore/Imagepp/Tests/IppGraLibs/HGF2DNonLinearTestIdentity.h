//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DNonLinearTestIdentity.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DNonLinearTestIdentity
//-----------------------------------------------------------------------------
// Description of 2D transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include <imagepp/all/h/HGF2DTransfoModel.h>

class HGFDistance;


class HGF2DNonLinearTestIdentity : public HGF2DTransfoModel
    {
public:

    // Primary methods
                                HGF2DNonLinearTestIdentity();
                                HGF2DNonLinearTestIdentity(const HGF2DNonLinearTestIdentity& pi_rObj);
    virtual                     ~HGF2DNonLinearTestIdentity();
    HGF2DNonLinearTestIdentity& operator=(const HGF2DNonLinearTestIdentity& pi_rObj);


    virtual bool IsConvertDirectThreadSafe() const override {return false;}
    virtual bool IsConvertInverseThreadSafe() const override {return false;}

    // Conversion interface
    virtual StatusInt           ConvertDirect(double*   pio_pXInOut,
                                              double*   pio_pYInOut) const override;

    virtual StatusInt           ConvertDirect (double    pi_YIn,
                                               double    pi_XInStart,
                                               size_t    pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_aXOut,
                                               double*   po_aYOut) const override;

    virtual StatusInt           ConvertDirect(double    pi_XIn,
                                              double    pi_YIn,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const override;


    virtual StatusInt           ConvertDirect(size_t    pi_NumLoc,
                                              double*   pio_aXInOut,
                                              double*   pio_aYInOut) const override;

    virtual StatusInt           ConvertInverse(double*   pio_pXInOut,
                                               double*   pio_pYInOut) const override;

    virtual StatusInt           ConvertInverse(double    pi_YIn,
                                               double    pi_XInStart,
                                               size_t    pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_aXOut,
                                               double*   po_aYOut) const override;


    virtual StatusInt           ConvertInverse(double    pi_XIn,
                                               double    pi_YIn,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const override;

    virtual StatusInt           ConvertInverse(size_t    pi_NumLoc,
                                               double*   pio_aXInOut,
                                               double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                IsIdentity() const;
    virtual bool                IsStretchable(double pi_AngleTolerance = 0) const;
    virtual void                GetStretchParams(double*  po_pScaleFactorX,
                                                 double*  po_pScaleFactorY,
                                                 HGF2DDisplacement* po_pDisplacement) const;

    virtual HGF2DTransfoModel*  Clone() const override;

    virtual HFCPtr<HGF2DTransfoModel>
                                ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;
    virtual bool                CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>     GetMatrix() const;
    virtual HFCMatrix<3, 3>&    GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;

    // Geometric properties
    virtual bool                PreservesLinearity() const;
    virtual bool                PreservesParallelism() const;
    virtual bool                PreservesShape() const;
    virtual bool                PreservesDirection() const;

    // Operations
    virtual void                Reverse ();

protected:

    virtual void                Prepare ();

    HFCPtr<HGF2DTransfoModel>   ComposeYourself(const HGF2DTransfoModel& pi_rModel) const;

private:

    double         m_XDirectRatio;
    double         m_YDirectRatio;
    double         m_XInverseRatio;
    double         m_YInverseRatio;

    };
