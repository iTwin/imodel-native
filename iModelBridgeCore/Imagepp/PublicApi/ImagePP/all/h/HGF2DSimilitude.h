//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DSimilitude
//-----------------------------------------------------------------------------
// Description of 2D similitude transformation model.
//-----------------------------------------------------------------------------

#pragma once


#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE

#define SIMILITUDE_MIN_NB_TIE_PTS 2
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class encapsulates a coordinate transformation model of type
    similitude. An object of this class expresses the bi-directional
    similitude transformations to apply to coordinates. A similitude
    transformation performs, a rotation and a scaling (in either order) then
    afterwards applies a translation. Coordinates can therefore be transformed
    directly or inverse.
    -----------------------------------------------------------------------------
*/
class HGF2DSimilitude : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DSimilitudeId, HGF2DTransfoModel)

public:

    // Primary methods
    IMAGEPP_EXPORT              HGF2DSimilitude();
    IMAGEPP_EXPORT              HGF2DSimilitude (const HGF2DDisplacement& pi_rTranslation,
                                                 double                   pi_rRotation,
                                                 double                   pi_Scaling);
    IMAGEPP_EXPORT              HGF2DSimilitude (const HGF2DSimilitude& pi_rObj);
                                HGF2DSimilitude(uint16_t pi_NumberOfPoints,
                                                const double*  pi_pTiePoints);
    IMAGEPP_EXPORT virtual      ~HGF2DSimilitude();

    IMAGEPP_EXPORT HGF2DSimilitude& operator= (const HGF2DSimilitude& pi_rObj);

    static  uint32_t            GetMinimumNumberOfTiePoints();

    // Model definition
    IMAGEPP_EXPORT void               SetTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT HGF2DDisplacement  GetTranslation() const;

    IMAGEPP_EXPORT void         SetRotation(double pi_Angle);
    IMAGEPP_EXPORT double       GetRotation() const;

    IMAGEPP_EXPORT void         SetScaling(double pi_Scale);
    IMAGEPP_EXPORT double       GetScaling() const;

    // High level model definition
    IMAGEPP_EXPORT void         AddTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT void         AddRotation(double pi_Angle,
                                            double pi_XCenter = 0.0,
                                            double pi_wYCenter = 0.0);
    IMAGEPP_EXPORT void         AddScaling(double pi_ScalingFactor,
                                           double pi_XCenter = 0.0,
                                           double pi_YCenter = 0.0);
    IMAGEPP_EXPORT void         SetByMatrixParameters(double pi_A0,
                                                      double pi_A1,
                                                      double pi_A2,
                                                      double pi_B0,
                                                      double pi_B1,
                                                      double pi_B2);

protected:

    // Conversion interface
    virtual bool                _IsConvertDirectThreadSafe()  const override;
    virtual bool                _IsConvertInverseThreadSafe() const override;

    virtual StatusInt           _ConvertDirect (double*   pio_pXInOut,
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

    // Miscellaneous
    virtual bool                _IsIdentity() const override;
    virtual bool                _IsStretchable(double pi_AngleTolerance) const override;
    virtual void                _GetStretchParams(double*           po_pScaleFactorX,
                                                 double*           po_pScaleFactorY,
                                                 HGF2DDisplacement* po_pDisplacement) const override;

    virtual HGF2DTransfoModel*  _Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>     
        _ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const override;
    virtual bool                _CanBeRepresentedByAMatrix() const override;

    virtual HFCMatrix<3, 3>     _GetMatrix() const override;

    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;


    // Geometric properties
    virtual bool                _PreservesLinearity() const override;
    virtual bool                _PreservesParallelism() const override;
    virtual bool                _PreservesShape() const override;
    virtual bool                _PreservesDirection() const override;


    // Operations
    virtual void                _Reverse() override;

    virtual void                _Prepare() override;
    virtual HFCPtr<HGF2DTransfoModel>
        _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;
private:

    // Primary attributes

    // HGF2DDisplacement
    double m_XTranslation;
    double m_YTranslation;

    double m_Scale;
    double m_Rotation;

    // Acceleration attributes
    double m_XTranslationPrime;
    double m_YTranslationPrime;
    double m_XTranslationInverse;
    double m_YTranslationInverse;
    double m_XTranslationInversePrime;
    double m_YTranslationInversePrime;

    double m_PreparedDirectA1;
    double m_PreparedDirectB1;
    double m_PreparedInverseA1;
    double m_PreparedInverseB1;
    double m_PreparedDirectA2;
    double m_PreparedDirectB2;
    double m_PreparedInverseA2;
    double m_PreparedInverseB2;


    // Private methods
    void            Copy (const HGF2DSimilitude& pi_rObj);

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DSimilitude.hpp"
