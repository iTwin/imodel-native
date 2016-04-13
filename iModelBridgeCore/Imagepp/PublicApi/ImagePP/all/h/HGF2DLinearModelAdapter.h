//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLinearModelAdapter.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLinearModelAdapter
//-----------------------------------------------------------------------------
// Description of 2D affine transformation model adapter.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModelAdapter.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert (${mailto:Alain.Robert@altaphoto.com})

    The purpose of the linear model adapter is to linearize a non-linear model in order
    to accelerate the conversion process by providing an approximation of an affine or projective
    over a specific area. The linear model adapter is a transformation model
    like any other, and in fact encapsulates a simple affine or projective. Since the
    adaptation is an approximation of the transformation model, an error may be
    introduced by adaptation using such a this adapter. The error can
    be studied using the StudyPrecisionOver() method.

    Since the linear model adapter is an adapter that linearizes a non-linear model
    it does so for a specific area only. The result model is defined for any
    coordinate pair, but the approximation is evaluated in a specific area only.

    The user is responsible of providing the normal area of operation of the
    adapter as well as a step. This step is used in the sampling of transformations
    in the area provided. The step must be at least twice as small as both the height
    and width of the area to insure that at least 4 points are included in the
    sampling.

    Study of precision can be performed on the original area or on any other area.

    As a result of Composition of the present model, a linear model adapter may be returned
    of a simple affine or projective thus losing the adaptation parameters.

    -----------------------------------------------------------------------------
*/
class HGF2DLinearModelAdapter : public HGF2DTransfoModelAdapter
    {
    HDECLARE_CLASS_ID(HGF2DLinearId_ModelAdapter, HGF2DTransfoModelAdapter)

public:

    // Primary methods
    IMAGEPP_EXPORT                       HGF2DLinearModelAdapter();

    IMAGEPP_EXPORT                       HGF2DLinearModelAdapter(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                               const HGF2DLiteExtent&    pi_ApplicationArea,
                                                               double                    pi_Step,
                                                               bool                      pi_AdaptAsAffine = false);


    IMAGEPP_EXPORT                       HGF2DLinearModelAdapter(const HGF2DLinearModelAdapter& pi_rObj);
    IMAGEPP_EXPORT virtual               ~HGF2DLinearModelAdapter();
    IMAGEPP_EXPORT HGF2DLinearModelAdapter& operator=(const HGF2DLinearModelAdapter& pi_rObj);

    const HGF2DLiteExtent&              GetApplicationArea() const;
    double                              GetStep() const {return m_Step;}

protected:

    virtual bool _IsConvertDirectThreadSafe() const override {return false;}
    virtual bool _IsConvertInverseThreadSafe() const override {return false;}

    // Conversion interface
    virtual StatusInt                   _ConvertDirect(double*   pio_pXInOut,
                                                      double*   pio_pYInOut) const override;

    virtual StatusInt                   _ConvertDirect(double    pi_YIn,
                                                      double    pi_XInStart,
                                                      size_t    pi_NumLoc,
                                                      double    pi_XInStep,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    virtual StatusInt                   _ConvertDirect(double    pi_XIn,
                                                      double    pi_YIn,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    virtual StatusInt                   _ConvertDirect(size_t    pi_NumLoc,
                                                      double*   pio_aXInOut,
                                                      double*   pio_aYInOut) const override;

    virtual StatusInt                   _ConvertInverse(double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    virtual StatusInt                   _ConvertInverse(double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t    pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt                   _ConvertInverse(double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt                   _ConvertInverse(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                        _IsIdentity      () const override;
    virtual bool                        _IsStretchable   (double pi_AngleTolerance) const override;
    virtual void                        _GetStretchParams(double*           po_pScaleFactorX,
                                                         double*           po_pScaleFactorY,
                                                         HGF2DDisplacement* po_pDisplacement) const override;

    virtual HGF2DTransfoModel*          _Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>   _ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const override;


    // Model definition
    // Model definition
    virtual bool                        _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3>             _GetMatrix() const override;

    virtual HFCPtr<HGF2DTransfoModel>   _CreateSimplifiedModel() const override;

    // Geometric properties
    virtual bool                        _PreservesLinearity() const override;
    virtual bool                        _PreservesParallelism() const override;
    virtual bool                        _PreservesShape() const override;
    virtual bool                        _PreservesDirection() const override;

    // Operations
    virtual void                        _Reverse () override;


    virtual void                        _Prepare () override;
    virtual HFCPtr<HGF2DTransfoModel>   _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;

    HGF2DLinearModelAdapter(const HGF2DTransfoModel& pi_rNonLinearTransfo,
                            const HGF2DTransfoModel& pi_rPreTransfo,
                            const HGF2DTransfoModel& pi_rPostTransfo,
                            const HGF2DLiteExtent&   pi_ApplicationArea,
                            double                  pi_Step,
                            bool                    pi_AdaptAsAffine = false);


private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HASSERT(m_Step > 0.0);

        HGF2DTransfoModelAdapter::ValidateInvariants();

        }
#endif

    // Private methods
    HGF2DLinearModelAdapter(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                            const HGF2DTransfoModel& pi_rLinearModelApproximation,
                            const HGF2DTransfoModel& pi_rPreTransfo,
                            const HGF2DTransfoModel& pi_rPostTransfo,
                            const HGF2DLiteExtent&   pi_ApplicationArea,
                            double                  pi_Step,
                            bool                    pi_AdaptAsAffine = false);
    void               Copy (const HGF2DLinearModelAdapter& pi_rObj);
    HFCPtr<HGF2DTransfoModel>             CreateDirectModelFromExtent(const HGF2DLiteExtent& pi_rExtent,
                                double                pi_Step) const;

    // Primary attributes
    mutable HFCPtr<HGF2DTransfoModel> m_pLinearModel;

    HGF2DLiteExtent m_ApplicationArea;
    double         m_Step;

    HFCPtr<HGF2DTransfoModel> m_pPreTransfoModel;
    HFCPtr<HGF2DTransfoModel> m_pPostTransfoModel;
    bool                     m_AdaptAsAffine;
    };

END_IMAGEPP_NAMESPACE

