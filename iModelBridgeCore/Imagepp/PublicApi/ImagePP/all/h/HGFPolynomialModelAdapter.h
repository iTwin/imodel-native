//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFPolynomialModelAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFPolynomialModelAdapter
//-----------------------------------------------------------------------------
// Description of 2D affine transformation model adapter.
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DTransfoModelAdapter.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DShape.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HFCMatrix.h>

#define POLYNOMIAL_DEGREE 3
#define NUMBER_OF_COEFFICIENTS 10
#define MINIMUM_NUMBER_OF_TIE_POINTS 50

#define __USE_COMPOSE_OPTIMISATION__ 1 // Compose stretchable models directly into the polynomial function coefficients instead of using
                                       // HGF2DCompolexTransfoModel. It is recommended as it is much faster.

BEGIN_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------------------
// <h2>HGFPolynomialModelAdapter</h2>
//
// The HGFPolynomialModelAdapter is a model adapter, meaning it is meant to replace, or
// adapt, a slower exact transfo model. It is originally design to adapt HCPGCoordModel,
// but can be used with any other transfo model.
//
// This model can approximate complex curvature of 2D spaces using 3rd degree polynomial
// functions, in our case bivariate polynomials (see http://mathworld.wolfram.com/BivariatePolynomial.html).
// Such functions takes a coordinate (x, y) as input and outputs a number. Two functions are
// used: one function f_x maps from (x, y) to x' and another function f_y maps from (x, y) to y', (x', y') being
// the coordinate analogous to the (x, y) coordinate in the destination coordinate system.
//
// To find f_x and f_y, a grid of tie points is generated. Least square fitting is then used to
// compute the optimal coefficients of f_x and f_y. See http://mathworld.wolfram.com/LeastSquaresFittingPolynomial.html.
//
// @bsimethod                                                 Alexandre.Gariepy   06/15
//+---------------+---------------+---------------+---------------+---------------+------
class HGFPolynomialModelAdapter : public HGF2DTransfoModelAdapter
    {
public:

    //----------------------------------------------------------------------------------------------------------------
    // @param pi_ExactTransfoModel      IN  The HGF2DTransfoModel to be adapted. This class is intended
    //                                      to approximate slow models, like the HCPGCoordModel.
    //
    // @param pi_ApplicationArea        IN  This is the area to be approximated by the polynomial model
    //                                      in the destination (inverse) coordinate system. A grid of points in this
    //                                      area is generated and used as tie points to approximate a polynomial
    //                                      model. Note that this area is not a hard limit for the domain of
    //                                      the model, but points outside this area won't have been considered
    //                                      in the polynomial approximation and are prone to have more error.
    //
    // @param pi_StepX pi_StepY         IN  As said above, a grid of tie points is generated and used as tie points
    //                                      of points. Those parameters are the steps used to generate those poitns.
    //                                      For example, if the area is a rectangle of 100 units per 200 units,
    //                                      a pi_StepX of 10 and a pi_StepY of 10 means generating a grid of
    //                                      10 points per 20 points, for a total of 200 points. Mathematically,
    //                                      10 points are required to have a unique polynomial function. Altough,
    //                                      to minimize the error, a hard limit on the number of points has been set
    //                                      to the constant MINIMUM_NUMBER_OF_TIE_POINTS. The higher the number of tie points,
    //                                      the more distributed will be the error across all the application area.
    //
    // @param pi_UsePolynomialForDirect IN  This parameter is used to set in which direction the polynomial approximation
    //                                      is used. Because the 3rd degree polynomial function is a non-reversible function,
    //                                      the polynomial approximation can only be used in one direction. In the other
    //                                      direction, the ExactTransfoModel is used. For exemple, if pi_UsePolynomialForDirect
    //                                      is true, the polynomial approximation will be used when calling ConvertDirect
    //                                      and the exact model will be used when calling ConvertInverse. Usually, you
    //                                      would like the use the polynomial approximation in the direction that the pixel
    //                                      are converted. That way, the exact (slower) model is only used to convert the extents
    //                                      while the polynomial approximation (faster) is used for the vast majority of the convert
    //                                      calls.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+-------------------------------
    IMAGEPP_EXPORT                       HGFPolynomialModelAdapter(const HGF2DTransfoModel&  pi_ExactTransfoModel,
                                                                   const HGF2DShape&         pi_ApplicationArea,
                                                                   double                    pi_StepX,
                                                                   double                    pi_StepY,
                                                                   bool                      pi_UsePolynomialForDirect=false);


    IMAGEPP_EXPORT                       HGFPolynomialModelAdapter(const HGFPolynomialModelAdapter& pi_rObj);
    IMAGEPP_EXPORT virtual               ~HGFPolynomialModelAdapter();
    HGFPolynomialModelAdapter&          operator=(const HGFPolynomialModelAdapter& pi_rObj);

    const HGF2DShape&                   GetApplicationArea() const {m_ApplicationArea;}
    double                              GetStepX() const {return m_StepX;}
    double                              GetStepY() const {return m_StepY;}

    // Conversion interface
    virtual StatusInt                   ConvertDirect(double*   pio_pXInOut,
                                                      double*   pio_pYInOut) const override;

    virtual StatusInt                   ConvertDirect(double    pi_YIn,
                                                      double    pi_XInStart,
                                                      size_t    pi_NumLoc,
                                                      double    pi_XInStep,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    virtual StatusInt                   ConvertDirect(double    pi_XIn,
                                                      double    pi_YIn,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    virtual StatusInt                   ConvertDirect(size_t    pi_NumLoc,
                                                      double*   pio_aXInOut,
                                                      double*   pio_aYInOut) const override;

    virtual StatusInt                   ConvertInverse(double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    virtual StatusInt                   ConvertInverse(double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t    pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt                   ConvertInverse(double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt                   ConvertInverse(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;
#if __USE_COMPOSE_OPTIMISATION__
    virtual HFCPtr<HGF2DTransfoModel>   ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;
    virtual HFCPtr<HGF2DTransfoModel>   ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
#endif

    // Miscalenious
    virtual bool                        IsIdentity      () const {return false;}
    virtual bool                        IsStretchable   (double pi_AngleTolerance = 0) const {return false;}
    virtual void                        GetStretchParams(double*           po_pScaleFactorX,
                                                         double*           po_pScaleFactorY,
                                                         HGF2DDisplacement* po_pDisplacement) const;

    virtual HGF2DTransfoModel*          Clone () const override;


    // Model definition
    // Model definition
    virtual bool                        CanBeRepresentedByAMatrix() const {return false;}
    virtual HFCMatrix<3, 3>             GetMatrix() const;

    // Geometric properties
    virtual bool                        PreservesLinearity() const {return false;}
    virtual bool                        PreservesParallelism() const {return false;}
    virtual bool                        PreservesShape() const {return false;}
    virtual bool                        PreservesDirection() const {return false;}

    // Operations
    virtual void                        Reverse ();

    IMAGEPP_EXPORT StatusInt            GetMeanError(double* po_pMeanError, double* po_pMaxError, HGF2DPosition* po_pMaxErrorPosition,
                                                     double* po_pMinError, HGF2DPosition* po_pMinErrorPosition);

    virtual bool IsConvertDirectThreadSafe() const override;
    virtual bool IsConvertInverseThreadSafe() const override;

    IMAGEPP_EXPORT bool HasEnoughTiePoints() {return m_EnoughTiePoints;}

private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HASSERT(m_StepX > 0.0);
        HASSERT(m_StepY > 0.0);

        HGF2DTransfoModelAdapter::ValidateInvariants();
        }
#endif

    // Private methods
    HGFPolynomialModelAdapter(); //Not used

    inline void TransposePointUsingPolynomial(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_YOut) const;
    inline void TransposePointsUsingPolynomial(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep,
                                               double* po_pXOut, double* po_pYOut) const;

    void Copy (const HGFPolynomialModelAdapter& pi_rObj);
    void CalculatePolynomialCoefficients(const HGF2DShape& pi_rShape, double pi_StepX, double pi_StepY);

    void AddStretchBeforePolynomial(double pi_TranslationX, double pi_TranslationY, double pi_ScalingX, double pi_ScalingY);
    void AddStretchAfterPolynomial(double pi_TranslationX, double pi_TranslationY, double pi_ScalingX, double pi_ScalingY);

    HFCPtr<HGF2DShape> m_ApplicationArea;
    double m_StepX;
    double m_StepY;
    bool   m_UsePolynomialForDirect;
    double m_CoefficientsX[NUMBER_OF_COEFFICIENTS];
    double m_CoefficientsY[NUMBER_OF_COEFFICIENTS];
    bool   m_EnoughTiePoints;
    };

END_IMAGEPP_NAMESPACE
