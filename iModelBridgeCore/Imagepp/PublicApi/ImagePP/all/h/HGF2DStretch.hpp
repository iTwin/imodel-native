//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DStretch.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    This method extracts the translation component of the model.

    @return The translation component of the model.

    @code
        HGF2DStretch    MyModel();
        HGF2DDisplacement MyTrans = MyModel.GetTranslation ();
    @end

    @see SetTranslation()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DStretch::GetTranslation () const
    {
    return(HGF2DDisplacement(m_XTranslation, m_YTranslation));
    }


/** -----------------------------------------------------------------------------
    Returns the X scaling component of the stretch

    @return The X scaling component of the stretch

    @code
        HGF2DStretch    MyModel;
        MyModel.SetXScaling (34.5);

        double ScaleFactorX = MyModel.GetXScaling ();
    @end

    @see GetYScaling()
    @see SetXScaling()
    -----------------------------------------------------------------------------
*/
inline double HGF2DStretch::GetXScaling () const
    {
    return(m_ScaleX);
    }

/** -----------------------------------------------------------------------------
    Returns the Y scaling component of the stretch

    @return The Y scaling component of the stretch

    @code
        HGF2DStretch    MyModel;
        MyModel.SetYScaling (34.5);

        double ScaleFactorY = MyModel.GetYScaling ();
    @end

    @see GetXScaling()
    @see SetYScaling()
    -----------------------------------------------------------------------------
*/
inline double HGF2DStretch::GetYScaling () const
    {
    return(m_ScaleY);
    }


END_IMAGEPP_NAMESPACE

