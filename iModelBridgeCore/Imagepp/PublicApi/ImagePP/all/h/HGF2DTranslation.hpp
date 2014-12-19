//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTranslation.hpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This method extracts the translation component of the model.

    @return The translation component of the model.

    @code
        HGF2DTranslation    MyModel();
        HGF2DDisplacement   MyDisp = MyModel.GetTranslation ();
    @end

    @see SetTranslation()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DTranslation::GetTranslation () const
    {
    return (HGF2DDisplacement(m_XTranslation, m_YTranslation));
    }




