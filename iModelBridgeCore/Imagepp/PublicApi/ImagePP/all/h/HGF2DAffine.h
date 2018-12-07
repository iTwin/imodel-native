//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DAffine.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DAffine
//-----------------------------------------------------------------------------
// Description of 2D affine transformation model.
//-----------------------------------------------------------------------------
#pragma once


#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModel.h"

#define AFFINE_MIN_NB_TIE_PTS 3

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a coordinate transformation model of type affine.
    An object of this class expresses the bi-directional affine transformations
    to apply to coordinates. An affine transformation permits to perform a
    translation, a rotation ,two scalings and apply anorthogonality of the
    axes. Coordinates can be transformed directly or inverse. Concretely,
    an affine transformation applies a rotation around the origin, two
    scalings (independent in X and Y), the anorthogonality of the axes
    (also called shearing). All of these movements are independent from each
    other. After these, the translation is applied. The translation here
    defined is a post-translation.

    This is the reason why the equation and parameter of the first set of
    equation is privileged in the API. This forces us to make choices in the
    reversing of the transformation model. Many possible solutions are possible
    for the inverse parameters. We will select those which result from the condition
    that is small which is equivalent to saying that the rotation of the inverse is
    in the quadrant opposite in the trigonometric space to the rotation of the
    direct equations. Furthermore, if the anorthogonality is set to either - /2 or  /2,
    then the Y-axis and the X-axis are confounded, and so are we. Therefore,
    the anorthogonality it forced to be - /2 and  /2, but not equal to either one.

    Any affine transformation can be reversed into another affine transformation.
    Furthermore, an affine can compose any transformation of type translation,
    identity, stretch and similitude, and still result in an affine transformation.

    -----------------------------------------------------------------------------
*/
class HGF2DAffine : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DAffineId, HGF2DTransfoModel)

public:

    // Primary methods

    IMAGEPP_EXPORT                   HGF2DAffine();
    IMAGEPP_EXPORT                   HGF2DAffine(const HGF2DDisplacement& pi_rTranslation,
                                                double                   pi_rRotation,
                                                double                   pi_ScalingX,
                                                double                   pi_ScalingY,
                                                double                   pi_rAnorthogonality);
    IMAGEPP_EXPORT                   HGF2DAffine(const HGF2DAffine& pi_rObj);
    IMAGEPP_EXPORT                   HGF2DAffine(uint16_t pi_NumberOfPoints,
                                                const double*  pi_pTiePoints);
    IMAGEPP_EXPORT virtual           ~HGF2DAffine ();
    IMAGEPP_EXPORT HGF2DAffine& operator=(const HGF2DAffine& pi_rObj);

    static uint32_t                 GetMinimumNumberOfTiePoints();

    // Model definition
    IMAGEPP_EXPORT void              SetTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT HGF2DDisplacement GetTranslation() const;
    IMAGEPP_EXPORT void              SetRotation(double pi_Angle);
    double                          GetRotation() const;
    IMAGEPP_EXPORT void              SetXScaling(double pi_Scale);
    IMAGEPP_EXPORT void              SetYScaling(double pi_Scale);
    double                          GetXScaling() const;
    double                          GetYScaling() const;
    IMAGEPP_EXPORT void              SetAnorthogonality(double pi_Angle);
    double                          GetAnorthogonality() const;

    // TEMPORARY for HPS etc...
    IMAGEPP_EXPORT void              SetByMatrixParameters(double pi_A0,
                                                          double pi_A1,
                                                          double pi_A2,
                                                          double pi_B0,
                                                          double pi_B1,
                                                          double pi_B2);
    // High level model definition
    IMAGEPP_EXPORT void              AddTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT void              AddRotation(double         pi_Angle,
                                                double         pi_RawXCenter=0.0,
                                                double         pi_RawYCenter=0.0);
    IMAGEPP_EXPORT void              AddIsotropicScaling(double pi_ScalingFactor,
                                                        double pi_XCenter=0.0,
                                                        double pi_YCenter=0.0);
    IMAGEPP_EXPORT void              AddAnisotropicScaling(double pi_ScalingFactorX,
                                                          double pi_ScalingFactorY,
                                                          double pi_XCenter=0.0,
                                                          double pi_YCenter=0.0);


    IMAGEPP_EXPORT void              AddHorizontalFlip(double pi_RawXMirrorPos=0.0);
    IMAGEPP_EXPORT void              AddVerticalFlip (double pi_RawYMirrorPos=0.0);





protected:
    // Conversion interface
    virtual bool                    _IsConvertDirectThreadSafe()  const override;
    virtual bool                    _IsConvertInverseThreadSafe() const override;

    virtual StatusInt               _ConvertDirect(double*   pio_pXInOut,
                                                   double*   pio_pYInOut) const override;

    virtual StatusInt               _ConvertDirect(double    pi_YIn,
                                                   double    pi_XInStart,
                                                   size_t    pi_NumLoc,
                                                   double    pi_XInStep,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

    virtual StatusInt               _ConvertDirect(double    pi_XIn,
                                                   double    pi_YIn,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

    virtual StatusInt               _ConvertDirect(size_t    pi_NumLoc,
                                                   double*   pio_aXInOut,
                                                   double*   pio_aYInOut) const override;

    virtual StatusInt               _ConvertInverse(double*   pio_pXInOut,
                                                    double*   pio_pYInOut) const override;

    virtual StatusInt               _ConvertInverse(double    pi_YIn,
                                                    double    pi_XInStart,
                                                    size_t    pi_NumLoc,
                                                    double    pi_XInStep,
                                                    double*   po_pXOut,
                                                    double*   po_pYOut) const override;

    virtual StatusInt               _ConvertInverse(double    pi_XIn,
                                                    double    pi_YIn,
                                                    double*   po_pXOut,
                                                    double*   po_pYOut) const override;


    virtual StatusInt               _ConvertInverse(size_t    pi_NumLoc,
                                                    double*   pio_aXInOut,
                                                    double*   pio_aYInOut) const override;

    // Miscellaneous
    virtual bool                    _IsIdentity() const override;
    virtual bool                    _IsStretchable(double pi_AngleTolerance) const override;
    virtual void                    _GetStretchParams(double*           po_pScaleFactorX,
                                                      double*           po_pScaleFactorY,
                                                      HGF2DDisplacement* po_pDisplacement) const override;

    // Geometric properties
    virtual bool                    _PreservesLinearity() const override;
    virtual bool                    _PreservesParallelism() const override;
    virtual bool                    _PreservesShape() const override;
    virtual bool                    _PreservesDirection() const override;


    virtual bool                    _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3>         _GetMatrix() const override;

    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;

    virtual HGF2DTransfoModel*      _Clone() const override;

    virtual HFCPtr<HGF2DTransfoModel> _ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const override;

    virtual void                    _Prepare () override;
    virtual HFCPtr<HGF2DTransfoModel>   _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;

    // Operations
    virtual void                    _Reverse() override;
private:

    // Primary attributes
    double m_XTranslation;
    double m_YTranslation;

    double     m_ScaleX;
    double     m_ScaleY;
    double     m_Rotation;
    double     m_Anorthogonality;

    // Acceleration attributes
    double m_XTranslationPrime;
    double m_YTranslationPrime;
    double m_XTranslationInverse;
    double m_YTranslationInverse;
    double m_XTranslationInversePrime;
    double m_YTranslationInversePrime;

    double     m_PreparedDirectA1;
    double     m_PreparedDirectB1;
    double     m_PreparedInverseA1;
    double     m_PreparedInverseB1;
    double     m_PreparedDirectA2;
    double     m_PreparedDirectB2;
    double     m_PreparedInverseA2;
    double     m_PreparedInverseB2;

    // Private methods
    void            Copy (const HGF2DAffine& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DAffine.hpp"
