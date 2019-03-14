//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DTransfoModel.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DTransfoModel (inline methods)
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void HGF3DTransfoModel::ConvertDirect(double    pi_XIn,
                                             double    pi_YIn,
                                             double    pi_ZIn,
                                             double*   po_pXOut,
                                             double*   po_pYOut,
                                             double*   po_pZOut) const
    {
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;
    *po_pZOut = pi_ZIn;

    ConvertDirect(po_pXOut, po_pYOut, po_pZOut);
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void HGF3DTransfoModel::ConvertInverse(double    pi_XIn,
                                              double    pi_YIn,
                                              double    pi_ZIn,
                                              double*   po_pXOut,
                                              double*   po_pYOut,
                                              double*   po_pZOut) const
    {
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;
    *po_pZOut = pi_ZIn;

    ConvertInverse(po_pXOut, po_pYOut, po_pZOut);
    }


END_IMAGEPP_NAMESPACE