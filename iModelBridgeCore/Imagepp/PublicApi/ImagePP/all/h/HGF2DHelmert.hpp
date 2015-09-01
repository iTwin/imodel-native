//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DHelmert.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    This method extracts the translation component of the model.

    @return The translation component of the model.

    @code
        HGF2DHelmert      MyModel();
        HGF2DDisplacement MyTrans = MyModel.GetTranslation();
    @end

    @see SetTranslation()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DHelmert::GetTranslation () const
    {
    return (HGF2DDisplacement(m_XTranslation, m_YTranslation));
    }


/** -----------------------------------------------------------------------------
    This method extracts the angle of the rotation component of the model.

    @return A constant reference to the rotation component of the model

    @code
        HGF2DHelmert    MyModel;
        double          MyRotationAngle = 13.5;

        MyModel.SetRotation (MyRotationAngle);

        double TheAngleOfRotation = MyModel.GetRotation ();
    @end

    @see SetRotation()
    -----------------------------------------------------------------------------
*/
inline double HGF2DHelmert::GetRotation () const
    {
    return (m_Rotation);
    }


/** -----------------------------------------------------------------------------
    This method returns the minimum number of control points that are necessary
    to create a Helmert transformation model

    @return The minimum number of control points
    -----------------------------------------------------------------------------
*/
inline uint32_t HGF2DHelmert::GetMinimumNumberOfTiePoints()
    {
    return HELMERT_MIN_NB_TIE_PTS;
    }



END_IMAGEPP_NAMESPACE