//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP/all/h/HGF2DWorld.h>

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

    //Redirection to CreateBestAdaptedModel
    IMAGEPP_EXPORT static HFCPtr<HGF2DTransfoModel>
        CreateGCoordBestAdaptedModel(GeoCoordinates::BaseGCSCR      pi_SourceProjection,
                                     GeoCoordinates::BaseGCSCR      pi_DestinationProjection,
                                     const HGF2DLiteExtent&         pi_rExtent,
                                     double                         pi_Step,
                                     double                         pi_ExpectedMeanError,
                                     double                         pi_ExpectedMaxError,
                                     double*                        po_pAdaptationMeanError = nullptr,
                                     double*                        po_pAdaptationMaxError = nullptr,
                                     double*                        po_pReversibilityMeanError = nullptr,
                                     double*                        po_pReversibilityMaxError = nullptr);

    // This method creates the best adapted GCoord model between Stretch, Affine, Projective and polynomial. 
    // If the error target is not attained, then the nullptr is returned
    IMAGEPP_EXPORT static HFCPtr<HGF2DTransfoModel> CreateBestAdaptedModel(const HGF2DTransfoModel& pi_rTransforModel,
                                                                           const HGF2DLiteExtent& pi_rExtent,
                                                                           double  pi_Step,
                                                                           double  pi_ExpectedMeanError,
                                                                           double  pi_ExpectedMaxError,
                                                                           double* po_pAdaptationMeanError = nullptr,
                                                                           double* po_pAdaptationMaxError = nullptr,
                                                                           double* po_pReversibilityMeanError = nullptr,
                                                                           double* po_pReversibilityMaxError = nullptr);

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



    /** -----------------------------------------------------------------------------

        Constructor from ERM strings. This constructor takes the definition of the
        projection, datum and units as used by ER Mapper file format and
        attempts to build a set of geokeys if the projection CS type is a known
        EPSG value. If it is not then the strings are converted into a WKT (internally
        using OGR) the remainder of the procedure and behavior is exactly the same
        as the WKT based constructor defined above.

        @param pi_rErmProjection IN The ER Mapper projection string. This string
                             should be one of the normal ER Mapper projection including
                             RAW or LOCAL (Geographic).

        @param pi_rErmProjection IN The ER Mapper datum string. This string
                             should be one of the normal ER Mapper datum

        @param pi_rErmUnit IN The ER Mapper unit. This UNIT string must be one of
                             "METERS"
                             "FEET" or "U.S. SURVEY FOOT" for US Survey foot
                             "IFEET" for international foot
                             "DEGREES"
                             "IMPERIAL YARD" for the British Imperial Yard 1895 definition

        -----------------------------------------------------------------------------
    */
    static GeoCoordinates::BaseGCSPtr CreateRasterGcsFromERSIDS(uint32_t EPSGCode, CharCP pErmProjection, CharCP pErmDatum, CharCP pErmUnits);

    static bool GetUnitsFromMeters(double& unitFromMeter, uint32_t EPSGUnitCode);

    
    /*---------------------------------------------------------------------------------**//**
    * This static method analyses the inputs and modifies the provided transformation model
    * to take into account the unit scaling. The returned model will apply unit change TO
    * meters based on provided information.
    * @return   A pointer to a transformation model taking unti transformation to meters.
    * @param    pi_pModel    IN The model to obtain a modified version to transform units to meters.
    * @param    pi_factorModelToMeter  IN 
    *                      This parameter indicates what are the units of the output of the transformation model
    *                      to meter. Again I beleive this is ridiculous since if we knew the factor we would simply
    *                      apply it, yet for some forsaken reason we support strange client datasets that specify
    *                      units then override them. I would get rid of this parameter if I were me ... but 
    *                      then I am so I probably will in a near future.

    * @param po_DefaultUnitWasFound IN, OPTIONAL Luckily it was found which implies it was lost. Now that it is found
    *                      it is me who is lost.
    * @bsimethod                                                    Alain Robert 2015/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    static HFCPtr<HGF2DTransfoModel> TranslateToMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                                                       double                           pi_FactorModelToMeter);
     static HFCPtr<HGF2DTransfoModel> TranslateFromMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                                                          double                           pi_FactorModelToMeter,
                                                          bool*                            po_DefaultUnitWasFound=0);
private:
    static void     CompareModelsOver( const HGF2DTransfoModel& ExactModel,
                                const HGF2DTransfoModel& ModelToTest,
                                const HGF2DLiteExtent& pi_PrecisionArea,
                                double                pi_Step,
                                double*               po_pMeanError,
                                double*               po_pMaxError);

    };

END_IMAGEPP_NAMESPACE
