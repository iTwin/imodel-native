//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordUtility.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCMacros.h>
#include <Imagepp/all/h/HGF2DWorld.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DWorldCluster;
class HRFRasterFile;
class HGF2DTransfoModel;
class HCPGCoordModel;
class HCPGeoTiffKeys;

// ----------------------------------------------------------------------------
//  HGFGCoordException
// ----------------------------------------------------------------------------
class HCPGCoordUtility
    {
public:

    // This method creates an adapted GCoord model from projection description
    static HFCPtr<HGF2DTransfoModel>
    CreateGCoordAdaptedModel(GeoCoordinates::BaseGCSCR      pi_SourceProjection,
                             GeoCoordinates::BaseGCSCR      pi_DestinationProjection,
                             const HGF2DLiteExtent&         pi_rExtent,
                             double                         pi_Step,
                             double                         pi_ExpectedMeanError,
                             double                         pi_ExpectedMaxError,
                             double*                        po_pAdaptationMeanError = 0,
                             double*                        po_pAdaptationMaxError = 0,
                             double*                        po_pReversibilityMeanError = 0,
                             double*                        po_pReversibilityMaxError = 0);

    IMAGEPP_EXPORT static HFCPtr<HGF2DTransfoModel> CreateAdaptedModel(HGF2DTransfoModel& transforModel,
                                                                   const HGF2DLiteExtent& pi_rExtent,
                                                                   double  pi_Step,
                                                                   double  pi_ExpectedMeanError,
                                                                   double  pi_ExpectedMaxError,
                                                                   double* po_pAdaptationMeanError,
                                                                   double* po_pAdaptationMaxError,
                                                                   double* po_pReversibilityMeanError,
                                                                   double* po_pReversibilityMaxError);

    // This method creates an adapted GCoord model from projection description
    // raster file extent and threshold errors.
    static HFCPtr<HGF2DTransfoModel>
    CreateGCoordAdaptedModel(GeoCoordinates::BaseGCSCR      pi_SourceProjection,
                             GeoCoordinates::BaseGCSCR      pi_DestinationProjection,
                             const HFCPtr<HRFRasterFile>&   pi_rpRasterFile,
                             uint32_t                       pi_Page,
                             const HGF2DWorldCluster&       pi_rCluster,
                             HGF2DWorldIdentificator        pi_DestinationBaseWorld,
                             double                         pi_Step,
                             double                         pi_ExpectedMeanError,
                             double                         pi_ExpectedMaxError,
                             double*                        po_pAdaptationMeanError = 0,
                             double*                        po_pAdaptationMaxError = 0,
                             double*                        po_pReversibilityMeanError = 0,
                             double*                        po_pReversibilityMaxError = 0);


    //&&AR can we get that from geocoord? remove IMAGEPP_EXPORT if we can.
    IMAGEPP_EXPORT static StatusInt GetGeoDomain(GeoCoordinates::BaseGCSCR rasterGcs, vector<HGF2DCoord<double>>& shape);

    static GeoCoordinates::BaseGCSPtr CreateRasterGcsFromERSIDS(uint32_t pi_EPSGCode, WStringCR pi_rErmProjection, WStringCR pi_rErmDatum, WStringCR pi_rErmUnits);

    static bool GetUnitsFromMeters(double& unitFromMeter, uint32_t EPSGUnitCode);
    static bool GetUnitsFromMeters(double& unitFromMeter, HCPGeoTiffKeys const& geoTiffKeys, bool pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation);

    };

END_IMAGEPP_NAMESPACE