//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTransfoModel.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DTransfoModel (inline methods)
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    This method reverses the current transformation model. The operation of
    reversing implies that results produced by the ConvertDirect() method will
    from now on be produced by ConvertInverse() and vice-versa. The different
    parameters related to the model are modified. The units used for the direct
    and inverse output channels are also swapped.

    -----------------------------------------------------------------------------
*/
inline void    HGF2DTransfoModel::Reverse()
    {
    // Prepare
    Prepare();

    }


//-----------------------------------------------------------------------------
// CallComposeOf
// This protected method permits to call the protected method of another
// transfo model not necessarely of the same type.
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DTransfoModel> HGF2DTransfoModel::CallComposeOf(const HGF2DTransfoModel& pi_rModel) const
    {
    return (pi_rModel.ComposeYourself(*this));
    }


END_IMAGEPP_NAMESPACE