//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLinearModelAdapter.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    Returns the step in direct channel value used in the sampling of the
    adapted model to generate the linear model approximation

    @return The step in direct channel raw value.
    -----------------------------------------------------------------------------
*/
inline double HGF2DLinearModelAdapter::GetStep() const
    {
    return(m_Step);
    }










