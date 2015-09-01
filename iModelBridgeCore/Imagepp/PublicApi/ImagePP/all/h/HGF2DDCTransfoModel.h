//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DDCTransfoModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DDCTransfoModel
//-----------------------------------------------------------------------------
// Description of 2D transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCPtr.h>
BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Stephane Cote



    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGF2DDCTransfoModel : public HFCShareableObject<HGF2DDCTransfoModel>
    {
    HDECLARE_SEALEDCLASS_ID(HGF2DDCTransfoModelId_Base)

public:

    IMAGEPP_EXPORT HGF2DDCTransfoModel();
    IMAGEPP_EXPORT ~HGF2DDCTransfoModel();

    IMAGEPP_EXPORT static uint32_t GetAlignTransfoMatrixFromScaleAndTiePts (double        po_pMatrix[4][4],
                                                                   unsigned short pi_NbVal_GeoTiePoint,
                                                                   const double* pi_pVal_GeoTiePoint);
    IMAGEPP_EXPORT static uint32_t GetHelmertTransfoMatrixFromScaleAndTiePts (double        po_pMatrix[4][4],
                                                                     unsigned short pi_NbVal_GeoTiePoint,
                                                                     const double* pi_pVal_GeoTiePoint);
    IMAGEPP_EXPORT static uint32_t GetSimilitudeTransfoMatrixFromScaleAndTiePts (double        po_pMatrix[4][4],
                                                                        unsigned short pi_NbVal_GeoTiePoint,
                                                                        const double* pi_pVal_GeoTiePoint);
    IMAGEPP_EXPORT static uint32_t GetAffineTransfoMatrixFromScaleAndTiePts (double        po_pMatrix[4][4],
                                                                    unsigned short pi_NbVal_GeoTiePoint,
                                                                    const double* pi_pVal_GeoTiePoint);
    IMAGEPP_EXPORT static uint32_t GetProjectiveTransfoMatrixFromScaleAndTiePts (double        po_pMatrix[4][4],
                                                                        unsigned short pi_NbVal_GeoTiePoint,
                                                                        const double* pi_pVal_GeoTiePoint);


protected:

private:


    };

END_IMAGEPP_NAMESPACE