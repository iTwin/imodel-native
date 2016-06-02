//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DProjective.hpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    This method extracts the translation component of the model.

    @return The translation component of the model.

    @code
        HGF2DProjective MyModel();
        HGF2DDisplacement MyTrans = MyModel.GetTranslation ();
    @end

    @see AddTranslation()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DProjective::GetTranslation () const
    {
    HINVARIANTS;

    return (HGF2DDisplacement(m_D02, m_D12));
    }



/** -----------------------------------------------------------------------------
    This method returns the minimum number of control points that are necessary
    to create a Projective transformation model

    @return The minimum number of control points
    -----------------------------------------------------------------------------
*/
inline uint32_t HGF2DProjective::GetMinimumNumberOfTiePoints()
    {
    return PROJECTIVE_MIN_NB_TIE_PTS;
    }




END_IMAGEPP_NAMESPACE