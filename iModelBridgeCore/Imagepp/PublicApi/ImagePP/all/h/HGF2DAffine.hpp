//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DAffine.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    This method extracts the translation component of the model.

    @return The translation component of the model.

    @code
        HGF2DAffine     MyModel();
        HGF2DDisplacement MyTrans = MyModel.GetTranslation();
    @end

    @see SetTranslation()
    @see AddTranslation()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DAffine::GetTranslation () const
    {
    return (HGF2DDisplacement(m_XTranslation, m_YTranslation));
    }


/** -----------------------------------------------------------------------------
    This method extracts the angle of the rotation component of the model.

    @return A constant reference to the rotation component of the model.

    @see SetRotation()
    @see AddRotation()
    -----------------------------------------------------------------------------
*/
inline double  HGF2DAffine::GetRotation () const
    {
    return (m_Rotation);
    }


/** -----------------------------------------------------------------------------
    This method extracts the angle of the anorthogonality component of the model.

    @return A constant reference to the anorthogonality component of the model.
            This value is between - /2 and  /2 (in radians) exclusively.

    @code
        HGF2DAffine     MyModel;
        double          MyAnorthoAngle = 0.05; // Radians

        MyModel.SetAnorthonality (MyRotationAngle);

        double TheAngleOfAnortho = MyModel.GetAnorthogonality();
    @end

    @see SetAnorthogonality()
    -----------------------------------------------------------------------------
*/
inline double HGF2DAffine::GetAnorthogonality() const
    {
    return (m_Anorthogonality);
    }


/** -----------------------------------------------------------------------------
    This method extracts the X scaling components of the model.

    @return The X scaling factor

    @code
        HGF2DAffine MyModel;
        MyModel.SetXScaling (34.5);

        double ScaleFactorX = MyModel.GetXScaling ();
    @end

    @see SetXScaling()
    @see SetYScaling()
    @see GetYScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
inline double HGF2DAffine::GetXScaling () const
    {
    return (m_ScaleX);
    }

/** -----------------------------------------------------------------------------
    This method extracts the Y scaling components of the model.

    @return The Y scaling factor

    @code
        HGF2DAffine MyModel;
        MyModel.SetYScaling (34.5);

        double ScaleFactorY = MyModel.GetYScaling ();
    @end

    @see SetYScaling()
    @see SetXScaling()
    @see GetXScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
inline double HGF2DAffine::GetYScaling () const
    {
    return (m_ScaleY);
    }



/** -----------------------------------------------------------------------------
    This method returns the minimum number of control points that are necessary
    to create an Affine transformation model

    @return The minimum number of control points
    -----------------------------------------------------------------------------
*/
inline uint32_t HGF2DAffine::GetMinimumNumberOfTiePoints()
    {
    return AFFINE_MIN_NB_TIE_PTS;
    }

END_IMAGEPP_NAMESPACE