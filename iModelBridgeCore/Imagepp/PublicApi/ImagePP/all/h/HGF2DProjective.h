//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DProjective.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DProjective
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model.
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModel.h"
#include "HFCMatrix.h"

#define PROJECTIVE_MIN_NB_TIE_PTS 4

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a coordinate transformation model of type projective.
    An object of this class expresses the bidirectional projective transformations
    to apply to coordinates. A projective transformation performs a translation,
    a rotation ,two scalings and apply anorthogonality of the axes.
    Coordinates can be transformed directly or inverse. Concretely, a projective
    transformation applies a rotation around the origin, two
    scalings (independant in X and Y), the anorthogonality of the
    axes (also called shearing). All of these movement are independant from each
    other. After these, the translation is applied. The translation here
    defined is a post-translation.

    Usually, users of transformation models do not know the primary rotations
    and scaling. Instead, they want to specify an origin of rotation or
    scaling. Furthermore, the user may know many different, independant
    transformations to apply, all of them applicable inside a single
    projective transformation model. For example, a user must express a rotation
    around the pair (1.0, 1.0), then a scaling around (2.0, 2.0), and finaly a
    translation. Note that the order of application of the 3 transformations is
    important, since all of them contain a translation component. For all of these
    reasons, methods were provided permitting to add individual non-primary rotation
    and scalings to a projective. The user may use the methods AddRotation(),
    AddIsotropicScaling(), AddAnisotropicScaling() , AddTranslation() AddHorizontalFlip()
    and AddVerticalFlip() to specify or modify an existing projective transformation
    model. These perform the process of creating the specified similitude transformations,
    and composing into the current projective.

    The matrix is of the following form:
       A00 A01 A02
       A10 A11 A12
       A20 A21 A22

    used in the equations as:
            A00*X + A01*Y + A02
     X1 =  ---------------------
            A20*X + A21*Y + A22

            A10*X + A11*Y + A12
     Y1 =  ---------------------
            A20*X + A21*Y + A22

    Where, by normalization, all values can be divided by A22 without changing the transformation.

    To represent a valid transformation, the determinant of the matrix must be
    different from 0.

    -----------------------------------------------------------------------------
*/
class HGF2DProjective : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DProjectiveId_Base, HGF2DTransfoModel)

public:

    // Primary methods
    IMAGEPP_EXPORT                  HGF2DProjective();
    IMAGEPP_EXPORT                  HGF2DProjective(const HGF2DDisplacement& pi_rTranslation,
                                                    double                   pi_rRotation,
                                                    double                   pi_ScalingX,
                                                    double                   pi_ScalingY,
                                                    double                   pi_rAnorthogonality);
    IMAGEPP_EXPORT                  HGF2DProjective(const HGF2DProjective& pi_rObj);
    IMAGEPP_EXPORT                  HGF2DProjective(const HFCMatrix<3, 3>& pi_rMatrix);
                                    HGF2DProjective(uint16_t pi_NumberOfPoints,
                                                    const double*  pi_pTiePoints);
    IMAGEPP_EXPORT                   HGF2DProjective(double const           pi_pMatrix[3][3]);

    IMAGEPP_EXPORT virtual           ~HGF2DProjective();
    IMAGEPP_EXPORT HGF2DProjective&  operator=(const HGF2DProjective& pi_rObj);

    static uint32_t                 GetMinimumNumberOfTiePoints();

    // Model definition
    IMAGEPP_EXPORT HGF2DDisplacement    GetTranslation() const;
    IMAGEPP_EXPORT double               GetRotation() const;
    IMAGEPP_EXPORT double               GetXScaling() const;
    IMAGEPP_EXPORT double               GetYScaling() const;
    IMAGEPP_EXPORT double               GetAnorthogonality() const;

    void                            SetByMatrix(const HFCMatrix<3, 3>& pi_rMatrix);

    // High level model definition
    IMAGEPP_EXPORT  void            AddTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT  void            AddRotation(double pi_Angle,
                                                double pi_XCenter = 0.0,
                                                double pi_YCenter = 0.0);
    IMAGEPP_EXPORT  void            AddIsotropicScaling(double pi_ScalingFactor,
                                                        double pi_XCenter = 0.0,
                                                        double pi_YCenter = 0.0);
    IMAGEPP_EXPORT void             AddAnisotropicScaling(double pi_ScalingFactorX,
                                                          double pi_ScalingFactorY,
                                                          double pi_XCenter = 0.0,
                                                          double pi_YCenter = 0.0);

    IMAGEPP_EXPORT void             AddHorizontalFlip(double pi_XMirrorPos = 0.0);
    IMAGEPP_EXPORT void             AddVerticalFlip(double pi_YMirrorPos = 0.0);

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
    virtual bool                    _IsIdentity      () const override;
    virtual bool                    _IsStretchable   (double pi_AngleTolerance) const override;
    virtual void                    _GetStretchParams(double*            po_pScaleFactorX,
                                                     double*            po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const override;
    virtual HGF2DTransfoModel*      _Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>    
        _ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const override;


    virtual bool                    _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3>         _GetMatrix() const override;

    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;

    // Geometric properties
    virtual bool                    _PreservesLinearity() const override;
    virtual bool                    _PreservesParallelism() const override;
    virtual bool                    _PreservesShape() const override;
    virtual bool                    _PreservesDirection() const override;

    // Operations
    virtual void                    _Reverse () override;

    virtual void                    _Prepare () override;
    virtual HFCPtr<HGF2DTransfoModel>    
                                    _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;
private:

#ifdef HVERIFYCONTRACT
    void                    ValidateInvariants() const
        {
        // Make sure that m_D22 equals 1.0
        HASSERT(m_D22 == 1.0);

        // Make sure that direct and inverse global scale equal 1.0
        HASSERT(m_D22 == 1.0);
        HASSERT(m_I22 == 1.0);

        // Make sure that model is valid
        // This implies that the upper left parameters follow certain rules:
        // One of [0,0] and [0,1] must different from 0.0
        // One of [0,0] and [1,0] must different from 0.0
        // One of [0,1] and [1,1] must different from 0.0
        // One of [1,0] and [1,1] must different from 0.0
        HASSERT((m_D00 != 0.0) || (m_D01 != 0.0));
        HASSERT((m_D00 != 0.0) || (m_D10 != 0.0));
        HASSERT((m_D01 != 0.0) || (m_D11 != 0.0));
        HASSERT((m_D10 != 0.0) || (m_D11 != 0.0));

        // The same applies to inverse parameters

        HASSERT((m_I00 != 0.0) || (m_I01 != 0.0));
        HASSERT((m_I00 != 0.0) || (m_I10 != 0.0));
        HASSERT((m_I01 != 0.0) || (m_I11 != 0.0));
        HASSERT((m_I10 != 0.0) || (m_I11 != 0.0));
        }
#endif

    // Primary attributes
    // These attributes represent the different factors of a transformation array
    // | A00 A01 A02 |
    // | A10 A11 A12 |
    // | A20 A21 A22 |
    // Where the interpretation of these are :
    // The block composing the upper left portion A00, A01, A10 and A11
    // contain the factors related to rotation anisotropic scaling and shearing
    // The factors A02 and A12 are the translation component
    // The A20 and A21 are the perspective parameter
    // Finally A22 represents the overall scaling
    // This matrix contains the transformation matrix for input and output direct units
    // After transformation of a coordinate, the inverse units must be applied to result
//        HFCMatrix<3, 3>     m_Matrix;

    // The direct matrix
    double         m_D00;
    double         m_D01;
    double         m_D02;   // Translation X
    double         m_D10;
    double         m_D11;
    double         m_D12;   // Translation Y
    double         m_D20;   // Perspective component
    double         m_D21;   // Perspective component
    double         m_D22;   // Overall scaling ... this can be = 1 at all times

    // The inverse matrix
    double         m_I00;
    double         m_I01;
    double         m_I02;   // Translation X Expressed in direct X units
    double         m_I10;
    double         m_I11;
    double         m_I12;   // Translation Y Expressed in direct Y units
    double         m_I20;   // Perspective component in inverse of dist unit X (direct)
    double         m_I21;   // Perspective component in inverse of dist unit Y (direct)
    double         m_I22;   // Overall scaling ... this can be = 1 at all times

    // Private methods
    void            Copy (const HGF2DProjective& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DProjective.hpp"
