//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>                // must be first for PreCompiledHeader Option

#include <ImagePP/all/h/HCPGCoordUtility.h>
#include <ImagePP/all/h/HCPGCoordModel.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DLinearModelAdapter.h>
#include <ImagePP/all/h/HGF2DLocalProjectiveGrid.h>
#include <ImagePP/all/h/HGF2DWorldCluster.h>
#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>
#include <ImagePP/all/h/HRFGdalUtilities.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGFPolynomialModelAdapter.h>

#define MAX_GRIDSIZE (300)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HCPGCoordUtility::GetUnitsFromMeters(double& unitFromMeter, uint32_t EPSGUnitCode)
    {
    bool  isUnitWasFound(false);

    unitFromMeter=1.0;
    T_WStringVector* pAllUnitName = GeoCoordinates::BaseGCS::GetUnitNames();
    for (T_WStringVector::const_iterator itr = pAllUnitName->begin(); itr != pAllUnitName->end(); ++itr)
        {
        GeoCoordinates::UnitCP pUnit(GeoCoordinates::Unit::FindUnit(itr->c_str()));
        if (pUnit->GetEPSGCode() == EPSGUnitCode)
            {
            unitFromMeter = 1.0/pUnit->GetConversionFactor();
            isUnitWasFound = true;
            break;
            }
        }

    return isUnitWasFound;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCSPtr HCPGCoordUtility::CreateRasterGcsFromERSIDS(uint32_t EPSGCode, CharCP pErmProjection, CharCP pErmDatum, CharCP pErmUnits)
    {
    // To have a GCS we need the engine initialized
    if (!GeoCoordinates::BaseGCS::IsLibraryInitialized())
        return nullptr;

    // First part ... we try to set the geokeys by ourselves ...
    if (BeStringUtilities::Stricmp(pErmProjection, "RAW") == 0)
        return nullptr;// no geotiff info

    GeoCoordinates::BaseGCSPtr pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();

    uint32_t ModelType;

    if (BeStringUtilities::Stricmp(pErmProjection, "GEODETIC") == 0 || BeStringUtilities::Stricmp(pErmProjection, "LOCAL") == 0)
        ModelType = TIFFGeo_ModelTypeGeographic;
    else if (BeStringUtilities::Stricmp(pErmProjection, "GEOCENTRIC") == 0)
        {
        ModelType = TIFFGeo_ModelTypeGeocentric;
        HASSERT(ModelType != 3); // not supported for now, see next statement!

        // Should be TIFFGeo_ModelTypeGeocentric but it is not supported by HRF, so treat file as projected
        ModelType = TIFFGeo_ModelTypeProjected; // assure it will be accepted by HRF!
        }
    else
        ModelType = TIFFGeo_ModelTypeProjected; // projected

    // We compute the unit key
    uint32_t ProjLinearUnit = 0;
    uint32_t GeogAngularUnit = 0;

    if (0 != *pErmUnits) // We do not always have units.
        {
        // Set Unit field
        if (BeStringUtilities::Stricmp(pErmUnits, "FEET") == 0 || BeStringUtilities::Stricmp(pErmUnits, "U.S. SURVEY FOOT") == 0)
            ProjLinearUnit = TIFFGeo_Linear_Foot_US_Survey; // US Foot
        else if (BeStringUtilities::Stricmp(pErmUnits, "IFEET") == 0)
            ProjLinearUnit = TIFFGeo_Linear_Foot; // Foot
        else if (BeStringUtilities::Stricmp(pErmUnits, "DEGREES") == 0)
            GeogAngularUnit = 9102; // Degree
        else if (BeStringUtilities::Stricmp(pErmUnits, "METERS") == 0)
            ProjLinearUnit = TIFFGeo_Linear_Meter; // Meter
        else if (BeStringUtilities::Stricmp(pErmUnits, "IMPERIAL YARD") == 0)
            ProjLinearUnit = TIFFGeo_British_Yard_1895;
        else
            {
            HASSERT_DATA(0); //Check if it isn't a new unit that should be handled by a special case
            }
        }

    // We build a set of geoTIFF keys according to ECW ids we obtain...
    HCPGeoTiffKeys geoKeys;

    geoKeys.AddKey(GTModelType, ModelType);

    if (ModelType == TIFFGeo_ModelTypeGeographic)
        geoKeys.AddKey(GeographicType, EPSGCode);
    else
        {
        if (EPSGCode > USHRT_MAX)
            geoKeys.AddKey(ProjectedCSTypeLong, EPSGCode);
        else
            geoKeys.AddKey(ProjectedCSType, EPSGCode);
        }

    if (ProjLinearUnit != 0)
        geoKeys.AddKey(ProjLinearUnits, ProjLinearUnit);

    if (GeogAngularUnit != 0)
        geoKeys.AddKey(GeogAngularUnits, GeogAngularUnit);
    
    if (EPSGCode != TIFFGeo_UserDefined)
        {
        // First tentative to obtain a valid BaseGCS using the GeoTIFF keys we generated using
        // ECW identifiers given it is not user defined.
        if(SUCCESS != pBaseGcs->InitFromGeoTiffKeys(nullptr, nullptr, &geoKeys, true))
            {
            // First attempt from geotiff keys failed ... we use a fallback solution
            // using GDAL we convert ECW ids into WKT
#ifdef IPP_HAVE_GDAL_SUPPORT
            AString wkt;
            HRFGdalUtilities::ConvertERMToOGCWKT(wkt, pErmProjection, pErmDatum, pErmUnits);
        
            if (!wkt.empty())
                {
                WString wktWide(wkt.c_str(), BentleyCharEncoding::Locale);
                // WKT was obtained from GDAL
                StatusInt Status = pBaseGcs->InitFromWellKnownText(nullptr, nullptr, GeoCoordinates::BaseGCS::wktFlavorOGC, wktWide.c_str());
                if (SUCCESS != Status)
                    {
                    // Sometimes the suposedly OGC compliant WKT from GDAL is not OGC compliant ... we try with wktFlavorUnknown flavor in case it may work
                    Status = pBaseGcs->InitFromWellKnownText(nullptr, nullptr, GeoCoordinates::BaseGCS::wktFlavorUnknown, wktWide.c_str());
        
                    if (SUCCESS != Status)
                        return NULL; // Too bad!
                    }
                }
#endif
            }
        }
    else 
        {
        // Extraction of a WKT from GDAL failed ... We patch up using IDs we have even if user-defined
        if(SUCCESS != pBaseGcs->InitFromGeoTiffKeys(nullptr, nullptr, &geoKeys, true))
            return NULL;
        }

    if (!pBaseGcs->IsValid())
        return nullptr;

    return pBaseGcs;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateGCoordAdaptedModel
(
    GeoCoordinates::BaseGCSCR pi_SourceProjection,
    GeoCoordinates::BaseGCSCR pi_DestinationProjection,
    const HGF2DLiteExtent& pi_rExtent,
    double  pi_Step,
    double  pi_ExpectedMeanError,
    double  pi_ExpectedMaxError,
    double* po_pAdaptationMeanError,
    double* po_pAdaptationMaxError,
    double* po_pReversibilityMeanError,
    double* po_pReversibilityMaxError
)
    {
    HCPGCoordModel GCoordModel(pi_SourceProjection, pi_DestinationProjection);
    return HCPGCoordUtility::CreateAdaptedModel(GCoordModel,pi_rExtent,pi_Step,pi_ExpectedMeanError,pi_ExpectedMaxError,po_pAdaptationMeanError,po_pAdaptationMaxError,po_pReversibilityMeanError,po_pReversibilityMaxError);
    }




/** -----------------------------------------------------------------------------
    This methods creates an adapted GCoord model based on given projection.
    The purpose of the method is to create a GCoord model and adapt it
    with a transformation model adapter such as HGF2DLinearModelAdapter
    or HGF2DLocalProjectiveGrid in order to linearize and accelerate the
    transformation model for performance reason. Since adaptation implies an
    approximation of the GCoord model, the caller must provide
    an extent representing the normal region of application of the model
    as well an an expected step in source projection units (usually meters)
    of linearity. In addition, the caller gives threshold values for the
    approximation to be perfomed.

    The method will first attempt the creation of a projective adapter upon the
    model in the inidcated region. The step is then usefull for the sampling
    of the region to generate a projective. After this adaptation, the precision of
    the approximation is perfomed to check if threshold values are not exceeded.
    If they are, then a projective grid adapter is created upon the same region
    at the indicated step. The precision is likewise checked. If the model
    adaptation is yet too imprecise, then a null pointer will be returned.

    @param pi_SourceProjection A Valid GCoord projection representing the
                               projection of the direct channel of the model.

    @param pi_DestinationProjection A Valid GCoord projection representing the
                                    projection of the inverse channel of the model.

    @param pi_rExtent   Constant reference to HGF2DLiteExtent containing
                        the expected area of application of the model. This
                        extent may not be empty.

    @param pi_Step      The step in X and Y in source coordinate system normal
                        units. This step is used for the sampling in the creation 
                        of the adapters as well as for the check in precision.

    @param pi_ExpectedMeanError The threshold value for the mean of errors introduced by the
                                approximation. If this condition is not met then no model is
                                returned. This value must be greater or equal to 0.0.

    @param pi_ExpectedMaxError  The threshold value for the maximum error introduced by the
                                approximation. If this condition is not met, then no model is
                                returned. This value must be greater or equal to 0.0.

    @param po_pAdaptationMeanError Optional parameter. Returns the mean error introduced by
                                   the transformation model adapter. This value is expressed
                                   in the target projection units. This value is returned even
                                   if expected precisions are not met and no model is created.
                                   If this value is not needed, 0 can be provided.

    @param po_pAdatationMaxError Optional Parameter. Returns the maximum error introduced by
                                 the transformation model adapter. This value is expressed
                                 in the target projection units.This value is returned even
                                 if expected precisions are not met and no model is created.
                                 If this value is not needed, 0 can be provided.

    @param po_pReversibilityMeanError Optional parameter. Returns the mean error between
                                     transformation of a coordintate direct then back
                                     by the GCoord model only. This is an indication
                                     of the applicability of the reprojection in the
                                     specified area. This value is expressed
                                     in the source projection units.This value is returned even
                                     if expected precisions are not met and no model is created.
                                     If this value is not needed, 0 can be provided.


    @param po_pReversibilityMaxError Optional parameter. Returns the maximum error between
                                     transformation of a coordintate direct then back
                                     by the GCoord model only. This is an indication
                                     of the applicability of the reprojection in the
                                     specified area. This value is expressed
                                     in the source projection units.This value is returned even
                                     if expected precisions are not met and no model is created.
                                     If this value is not needed, 0 can be provided.

    @return a smart pointer to transformation model of some appropriate kind or NULL
            if the model could not be created such as when precision is not met.

    @see HCPGCoordModel
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateAdaptedModel
(
    HGF2DTransfoModel& transforModel,
    const HGF2DLiteExtent& pi_rExtent,  // in src unit 
    double  pi_Step,                    // in src unit 
    double  pi_ExpectedMeanError,       // in dst unit
    double  pi_ExpectedMaxError,        // in dst unit
    double* po_pAdaptationMeanError,
    double* po_pAdaptationMaxError,
    double* po_pReversibilityMeanError,
    double* po_pReversibilityMaxError
)
    {
    // The expected error must be greater or equal to 0.0
    HPRECONDITION (pi_ExpectedMeanError >= 0.0);
    HPRECONDITION (pi_ExpectedMaxError >= 0.0);

    HFCPtr<HGF2DTransfoModel> pResultModel;
    try
        {
        double width  = pi_rExtent.GetWidth() / 1000.0;
        double height = pi_rExtent.GetHeight() / 1000.0;
        HGF2DLiteExtent EnlargedExtent(pi_rExtent.GetXMin() - width, pi_rExtent.GetYMin() - height,
                                       pi_rExtent.GetXMax() + width, pi_rExtent.GetYMax() + height);


        // Check if reversibility must be studied
        if (po_pReversibilityMeanError != 0 || po_pReversibilityMaxError != 0)
            {
            // The caller desires knowledge about reversibility. study
            double ReverseMeanError;
            double ReverseMaxError;
            double ScaleChangeMean;
            double ScaleChangeMax;
            transforModel.StudyReversibilityPrecisionOver (EnlargedExtent, pi_Step, 
                                                           &ReverseMeanError, &ReverseMaxError, 
                                                           &ScaleChangeMean, &ScaleChangeMax);

            // If mean error is desired ... set it
            if (po_pReversibilityMeanError != 0)
                *po_pReversibilityMeanError = ReverseMeanError;

            // If max error is desired ... set it
            if (po_pReversibilityMaxError != 0)
                *po_pReversibilityMaxError = ReverseMaxError;
            }

        // Attempt acceleration with a projective adapter
        HFCPtr<HGF2DLinearModelAdapter> pAdaptedModel = new HGF2DLinearModelAdapter (transforModel, EnlargedExtent, pi_Step);

        // Check precision of adapter
        double MeanError;
        double MaxError;
        pAdaptedModel->StudyPrecisionOver (EnlargedExtent, pi_Step, &MeanError, &MaxError);

        // Check if precision is respected
        if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
            {
            pResultModel = pAdaptedModel;

            pAdaptedModel = NULL;

            // The expected precision is not attained ... try something else
            int32_t NbOfTilesX (1);
            int32_t NbOfTilesY (1);
            HFCPtr<HGF2DLocalProjectiveGrid> pGridModel (NULL);

            while (((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError)) && (NbOfTilesX < MAX_GRIDSIZE || NbOfTilesY < MAX_GRIDSIZE))
                {
                NbOfTilesX +=2;
                NbOfTilesY +=2;

                pGridModel = new HGF2DLocalProjectiveGrid (transforModel, EnlargedExtent, NbOfTilesX, NbOfTilesY);

                // Check precision of adapter
                pGridModel->StudyPrecisionOver (EnlargedExtent, pi_Step, &MeanError, &MaxError);
                }

            // Check if precision is respected
            if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
                {
                // Precision still not attained ...  return empty model
                pResultModel = 0;
                }
            else
                {
                // Return the projective grid
                pResultModel = pGridModel;
                }
            }
        else
            {
            pResultModel = pAdaptedModel;
            }

        // Check if adaptation errors must be returned and set if needed
        if (po_pAdaptationMeanError != 0)
            *po_pAdaptationMeanError = MeanError;

        if (po_pAdaptationMaxError != 0)
            *po_pAdaptationMaxError = MaxError;
        }
    catch(...)
        {
        HASSERT(!"Error in HCPGCoordUtility::CreateGCoordAdaptedModel");
        pResultModel = NULL;
        }

    return (pResultModel);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                           Laurent.Robert-Veillette  03/2016
// Redirection to HCPGCoordUtility::CreateBestAdaptedModel
//----------------------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateGCoordBestAdaptedModel
(
    GeoCoordinates::BaseGCSCR pi_SourceProjection,
    GeoCoordinates::BaseGCSCR pi_DestinationProjection,
    const HGF2DLiteExtent& pi_rExtent,
    double  pi_Step,
    double  pi_ExpectedMeanError,
    double  pi_ExpectedMaxError,
    double* po_pAdaptationMeanError,
    double* po_pAdaptationMaxError,
    double* po_pReversibilityMeanError,
    double* po_pReversibilityMaxError
    )
    {
    HCPGCoordModel GCoordModel(pi_SourceProjection, pi_DestinationProjection);
    return HCPGCoordUtility::CreateBestAdaptedModel(GCoordModel,
                                                    pi_rExtent,
                                                    pi_Step,
                                                    pi_ExpectedMeanError,
                                                    pi_ExpectedMaxError,
                                                    po_pAdaptationMeanError,
                                                    po_pAdaptationMaxError,
                                                    po_pReversibilityMeanError,
                                                    po_pReversibilityMaxError);
    }

/** -----------------------------------------------------------------------------
This methods creates the best adapted GCoord model based on given projection.
The method creates a HGF2DLinearModelAdapter only once but then uses the matrix
of the transformation with well placed zeros to test the following models in 
complexity order : Stretch, Affine and Projective. If none of these respect the
given expected errors, we then try to adapt with a HGFPolynomialModelAdapter.
If this model also does not respect the expected errors, the method returns 
nullptr. 

The purpose of the method is to create a adapted model in order to linearize 
and accelerate the transformation model for performance reason. Since adaptation 
implies an approximation of the GCoord exact model, the caller must provide
an extent representing the normal region of application of the model
as well an an expected step in source projection units (usually meters)
of linearity. In addition, the caller gives threshold values for the
approximation to be perfomed.

@param pi_rTransforModel reference to a model to approximate. WARNING! Pay
attention to the direction of the transformation. The INVERSE conversion MUST
REPRESENT THE CONVERSION OF THE PIXELS. This restriction comes from the 
polynomial model which is nos bijective (not valid in both directions).

@param pi_rExtent   Constant reference to HGF2DLiteExtent containing
the expected area of application of the model. This
extent may not be empty.

@param pi_Step      The step in X and Y in source coordinate system normal
units. This step is used for the sampling in the creation
of the adapters as well as for the check in precision. It must be
at least twice as small as both the height and width of the area to insure 
that at least 4 points are included in the sampling.

@param pi_ExpectedMeanError The threshold value for the mean of errors introduced by the
approximation. If this condition is not met then no model is
returned. This value must be greater or equal to 0.0.

@param pi_ExpectedMaxError  The threshold value for the maximum error introduced by the
approximation. If this condition is not met, then no model is
returned. This value must be greater or equal to 0.0.

@param po_pAdaptationMeanError Optional parameter. Returns the mean error introduced by
the transformation model adapter. This value is expressed
in the target projection units. This value is returned even
if expected precisions are not met and no model is created.
If this value is not needed, nullptr can be provided.

@param po_pAdatationMaxError Optional Parameter. Returns the maximum error introduced by
the transformation model adapter. This value is expressed
in the target projection units.This value is returned even
if expected precisions are not met and no model is created.
If this value is not needed, nullptr can be provided.

@param po_pReversibilityMeanError Optional parameter. Returns the mean error between
transformation of a coordintate direct then back
by the GCoord model only. This is an indication
of the applicability of the reprojection in the
specified area. This value is expressed
in the source projection units.This value is returned even
if expected precisions are not met and no model is created.
If this value is not needed, nullptr can be provided.

@param po_pReversibilityMaxError Optional parameter. Returns the maximum error between
transformation of a coordintate direct then back
by the GCoord model only. This is an indication
of the applicability of the reprojection in the
specified area. This value is expressed
in the source projection units.This value is returned even
if expected precisions are not met and no model is created.
If this value is not needed, nullptr can be provided.

@return a smart pointer to transformation model of some appropriate kind or nullptr
if the model could not be created such as when precision is not met.

@see HCPGCoordModel
                                        Laurent Robert-Veillette     03/2016
-----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateBestAdaptedModel(const HGF2DTransfoModel& pi_rTransforModel,
                                                 const HGF2DLiteExtent& pi_rExtent,
                                                 double  pi_Step,
                                                 double  pi_ExpectedMeanError,
                                                 double  pi_ExpectedMaxError,
                                                 double* po_pAdaptationMeanError,
                                                 double* po_pAdaptationMaxError,
                                                 double* po_pReversibilityMeanError,
                                                 double* po_pReversibilityMaxError)
    {
    // The expected error must be greater or equal to 0.0
    HPRECONDITION(pi_ExpectedMeanError >= 0.0);
    HPRECONDITION(pi_ExpectedMaxError >= 0.0);
    HPRECONDITION(pi_Step > 0);

    HFCPtr<HGF2DTransfoModel> pResultModel;
    try
        {
        // Check if reversibility must be studied
        if (po_pReversibilityMeanError && po_pReversibilityMaxError)
            {
            // The caller desires knowledge about reversibility. study
            double ReverseMeanError;
            double ReverseMaxError;
            double ScaleChangeMean;
            double ScaleChangeMax;
            pi_rTransforModel.StudyReversibilityPrecisionOver(pi_rExtent, pi_Step,
                                                          &ReverseMeanError, &ReverseMaxError,
                                                          &ScaleChangeMean, &ScaleChangeMax);

            // If mean error is desired ... set it
            if (po_pReversibilityMeanError != 0)
                *po_pReversibilityMeanError = ReverseMeanError;

            // If max error is desired ... set it
            if (po_pReversibilityMaxError != 0)
                *po_pReversibilityMaxError = ReverseMaxError;
            }

        // Create the Linear model adapter to extract his parameters (scale, translation, rotation, etc.)
        HFCPtr<HGF2DLinearModelAdapter> pLinearModel = new HGF2DLinearModelAdapter(pi_rTransforModel, pi_rExtent, pi_Step);
        HFCMatrix<3, 3> linearMatrix = pLinearModel->GetMatrix();
        HFCPtr<HGF2DProjective> pProjectiveModel = new HGF2DProjective(linearMatrix);

        //Create the STRETCH and test the precision
        HFCPtr<HGF2DStretch> pStretchModel = new HGF2DStretch(pProjectiveModel->GetTranslation(),
                                                              pProjectiveModel->GetXScaling(),
                                                              pProjectiveModel->GetYScaling());

        double MeanError, MaxError;
        CompareModelsOver(pi_rTransforModel, *pStretchModel, pi_rExtent, pi_Step, &MeanError, &MaxError);
        if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
            {
            //Create the AFFINE model and test the precision
            HFCPtr<HGF2DAffine> pAffineModel = new HGF2DAffine(pProjectiveModel->GetTranslation(),
                                                               pProjectiveModel->GetRotation(),
                                                               pProjectiveModel->GetXScaling(),
                                                               pProjectiveModel->GetYScaling(),
                                                               pProjectiveModel->GetAnorthogonality());

            CompareModelsOver(pi_rTransforModel, *pAffineModel, pi_rExtent, pi_Step, &MeanError, &MaxError);

            if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
                {
                //Test the precision of the PROJECTIVE model
                CompareModelsOver(pi_rTransforModel, *pProjectiveModel, pi_rExtent, pi_Step, &MeanError, &MaxError);

                if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
                    {
                    //Create the extent in the destination
                    double minX, minY, maxX, maxY;
                    pi_rTransforModel.ConvertDirect(pi_rExtent.GetXMin(), pi_rExtent.GetYMin(), &minX, &minY);
                    pi_rTransforModel.ConvertDirect(pi_rExtent.GetXMax(), pi_rExtent.GetYMax(), &maxX, &maxY);
                    HGF2DRectangle rectangle(minX, minY, maxX, maxY);
                    //Create the POLYNOMIAL model and test the precision
                    HFCPtr<HGFPolynomialModelAdapter> pPolynomialModel = new HGFPolynomialModelAdapter(pi_rTransforModel,
                                                                                                       rectangle,
                                                                                                       pi_rExtent.GetWidth() / 15,
                                                                                                       pi_rExtent.GetHeight() / 15,
                                                                                                       false);

                    HGF2DPosition MaxErrorPosition, MinErrorPosition;
                    double MinError;
                    pPolynomialModel->GetMeanError(&MeanError, &MaxError, &MaxErrorPosition, &MinError, &MinErrorPosition);

                    if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
                        pResultModel = nullptr;
                    else
                        pResultModel = pPolynomialModel;
                    }
                else
                    pResultModel = pProjectiveModel;
                }
            else
                pResultModel = pAffineModel;
            }
        else
            pResultModel = pStretchModel;

        // Check if adaptation errors must be returned and set if needed
        if (po_pAdaptationMeanError)
            *po_pAdaptationMeanError = MeanError;

        if (po_pAdaptationMaxError)
            *po_pAdaptationMaxError = MaxError;
        }
    catch (...)
        {
        HASSERT(!"Error in HCPGCoordUtility::CreateBestAdaptedModel");
        pResultModel = nullptr;
        }

    return pResultModel;
    }

/** -----------------------------------------------------------------------------
    This methods creates an adapted GCoord model based on given projection.
    an provided parameters. The model returned will describe the full transformation
    from the future logical coordinate system of the raster to the base
    world indicated including usual logical space transformation to this base
    if different as returned by the raster file world identifier
    (@see HRFRasterFile::GetWorldIdentificator())
    The purpose of the method is to create a GCoord model and adapt it
    with a transformation model adapter such as HGF2DLinearModelAdapter
    or HGF2DLocalProjectiveGrid in order to linearize and accelerate the
    transformation model for performance reason. Since adaptation implies an
    approximation of the GCoord model, the caller must provide
    a raster and page number from which will be extracted the normal region of
    application of the new model. Must also be provided an expected step
    in source projection units (usually meters) of linearity.
    In addition, the caller gives threshold values for the
    approximation to be perfomed.

    The method will first attempt the creation of a projective adapter upon the
    model in the inidcated region. The step is then useful for the sampling
    of the region to generate a projective. After this adaptation, the precision of
    the approximation is perfomed to check if threshold values are not exceeded.
    If they are, then a projective grid adapter is created upon the same region
    at the indicated step. The precision is likewise checked. If the model
    adaptation is yet too imprecise, then a null pointer will be returned.

    @param pi_SourceProjection A Valid GCoord projection representing the
                               projection of the direct channel of the model.

    @param pi_DestinationProjection A Valid GCoord projection representing the
                                    projection of the inverse channel of the model.

    @param pi_rpRasterFile A smart pointer to an opened raster file that must have
                           the page refered to by pi_Page from which will be
                           extracted world identifier and extent in logical space.

    @param pi_Page The number of the page to obtain model for. This number starts
                   at 0 for the first page and so on. The indicated page must exist.

    @param pi_rCluster A world cluster that must contain the base world as well as
                       the world identified by the raster file world identifier.

    @param pi_DestinationBaseWorld A world identifier that represents the destination
                                   world the model will transform to. This world
                                   must be known by the cluster.

    @param pi_Step      The step in X and Y in source coordinate system normal
                        units. This step is used for the sampling in the creation of 
                        the adapters as well as for the check in precision.

    @pi_ExpectedMeanError The threshold value for the mean of errors introduced by the
                          approximation. If this condition is not met then no model is
                          returned.

    @pi_ExpectedMaxError  The threshold value for the maximum error introduced by the
                          approximation. If this condition is not met, then no model is
                          returned.

    @param po_pAdaptationMeanError Optional parameter. Returns the mean error introduced by
                                   the transformation model adapter. This value is expressed
                                   in the target projection units. This value is returned even
                                   if expected precisions are not met and no model is created.
                                   If this value is not needed, 0 can be provided.

    @param po_pAdatationMaxError Optional Parameter. Returns the maximum error introduced by
                                 the transformation model adapter. This value is expressed
                                 in the target projection units.This value is returned even
                                 if expected precisions are not met and no model is created.
                                 If this value is not needed, 0 can be provided.

    @param po_pReversibilityMeanError Optional parameter. Returns the mean error between
                                     transformation of a coordintate direct then back
                                     by the GCoord model only. This is an indication
                                     of the applicability of the reprojection in the
                                     specified area. This value is expressed
                                     in the source projection units.This value is returned even
                                     if expected precisions are not met and no model is created.
                                     If this value is not needed, 0 can be provided.


    @param po_pReversibilityMaxError Optional parameter. Returns the maximum error between
                                     transformation of a coordintate direct then back
                                     by the GCoord model only. This is an indication
                                     of the applicability of the reprojection in the
                                     specified area. This value is expressed
                                     in the source projection units.This value is returned even
                                     if expected precisions are not met and no model is created.
                                     If this value is not needed, 0 can be provided.


    @return a smart pointer to transformation model of some appropriate kind or NULL
            if the model could not be created such as when precision is not met.

    @see HCPGCoordModel
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateGCoordAdaptedModel
(
    GeoCoordinates::BaseGCSCR       pi_SourceProjection,
    GeoCoordinates::BaseGCSCR       pi_DestinationProjection,
    const HFCPtr<HRFRasterFile>&    pi_rpRasterFile,
    uint32_t                        pi_Page,
    const HGF2DWorldCluster&        pi_rCluster,
    HGF2DWorldIdentificator         pi_DestinationBaseWorld,
    double                          pi_Step,
    double                          pi_ExpectedMeanError,
    double                          pi_ExpectedMaxError,
    double*                         po_pAdaptationMeanError,
    double*                         po_pAdaptationMaxError,
    double*                         po_pReversibilityMeanError,
    double*                         po_pReversibilityMaxError
)
    {
    // The expected error must be greater or equal to 0.0
    HPRECONDITION (pi_ExpectedMeanError >= 0.0);
    HPRECONDITION (pi_ExpectedMaxError >= 0.0);

    // The page must be valid
    HPRECONDITION (pi_rpRasterFile->CountPages () > pi_Page);

    // The world of the raster file must be known by the cluster
    HPRECONDITION (pi_rCluster.HasWorld (pi_rpRasterFile->GetWorldIdentificator ()));

    // The base world must be known by the cluster
    HPRECONDITION (pi_rCluster.HasWorld (pi_DestinationBaseWorld));


    HFCPtr<HGF2DTransfoModel> pResultModel;

    try
        {
        // Create reprojection model
        HFCPtr<HGF2DTransfoModel> pTempModel = new HCPGCoordModel (pi_SourceProjection, pi_DestinationProjection);

        // Extract information from raster file
        HFCPtr<HGF2DTransfoModel> pFileModel = new HGF2DIdentity ();

        if (pi_rpRasterFile->GetPageDescriptor (pi_Page)->HasTransfoModel ())
            {
            pFileModel = pi_rpRasterFile->GetPageDescriptor (pi_Page)->GetTransfoModel ();
            }

        // Obtain the cluster coordinate system representing the world ID of file
        HFCPtr<HGF2DCoordSys> pLogicalByID = pi_rCluster.GetCoordSysReference (pi_rpRasterFile->GetWorldIdentificator ());

        // Obtain the transformation from this world to base world
        HFCPtr<HGF2DTransfoModel> pIDToBaseTransfo = pLogicalByID->GetTransfoModelTo (pi_rCluster.GetCoordSysReference (pi_DestinationBaseWorld));

        HFCPtr<HGF2DTransfoModel> pFileModelToBase = pFileModel->ComposeInverseWithDirectOf (*pIDToBaseTransfo);

        // Obtain Image width and height

        CHECK_HUINT64_TO_HDOUBLE_CONV(pi_rpRasterFile->GetPageDescriptor(pi_Page)->
                                      GetResolutionDescriptor (0)->GetWidth())
        CHECK_HUINT64_TO_HDOUBLE_CONV(pi_rpRasterFile->GetPageDescriptor(pi_Page)->
                                      GetResolutionDescriptor (0)->GetHeight())

        double ImageWidth = (double)pi_rpRasterFile->GetPageDescriptor (pi_Page)->GetResolutionDescriptor (0)->GetWidth ();
        double ImageHeight = (double)pi_rpRasterFile->GetPageDescriptor (pi_Page)->GetResolutionDescriptor (0)->GetHeight ();

        // Convert four coordinates
        double FirstPointX;
        double FirstPointY;
        double SecondPointX;
        double SecondPointY;
        double ThirdPointX;
        double ThirdPointY;
        double FourthPointX;
        double FourthPointY;

        double TempX = 0.0;
        double TempY = 0.0;
        pFileModelToBase->ConvertDirect (TempX, TempY, &FirstPointX, &FirstPointY);

        TempX = 0.0;
        TempY = ImageHeight;
        pFileModelToBase->ConvertDirect (TempX, TempY, &SecondPointX, &SecondPointY);

        TempX = ImageWidth;
        TempY = ImageHeight;
        pFileModelToBase->ConvertDirect (TempX, TempY, &ThirdPointX, &ThirdPointY);

        TempX = ImageWidth;
        TempY = 0.0;
        pFileModelToBase->ConvertDirect (TempX, TempY, &FourthPointX, &FourthPointY);

        HGF2DLiteExtent ImageExtent (MIN (FirstPointX, MIN (SecondPointX, MIN (ThirdPointX, FourthPointX))),
                                     MIN (FirstPointY, MIN (SecondPointY, MIN (ThirdPointY, FourthPointY))),
                                     MAX (FirstPointX, MAX (SecondPointX, MAX (ThirdPointX, FourthPointX))),
                                     MAX (FirstPointY, MAX (SecondPointY, MAX (ThirdPointY, FourthPointY))));

        // Check if reversibility must be studied
        if (po_pReversibilityMeanError != 0 || po_pReversibilityMaxError != 0)
            {
            // The caller desires knowledge about reversibility. study
            double ReverseMeanError;
            double ReverseMaxError;
            double ScaleChangeMean;
            double ScaleChangeMax;
            pTempModel->StudyReversibilityPrecisionOver (ImageExtent, pi_Step, &ReverseMeanError, &ReverseMaxError, &ScaleChangeMean, &ScaleChangeMax);

            // If mean error is desired ... set it
            if (po_pReversibilityMeanError != 0)
                *po_pReversibilityMeanError = ReverseMeanError;

            // If MAX error is desired ... set it
            if (po_pReversibilityMaxError != 0)
                *po_pReversibilityMaxError = ReverseMaxError;

            }



        // Attempt acceleration with a projective adapter
        HFCPtr<HGF2DLinearModelAdapter> pAdaptedModel = new HGF2DLinearModelAdapter (*pTempModel,
                                                                                     ImageExtent,
                                                                                     pi_Step);

        // Check precision of adapter
        double MeanError;
        double MaxError;
        pAdaptedModel->StudyPrecisionOver (ImageExtent, pi_Step, &MeanError, &MaxError);

        if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
            {
            pResultModel = pAdaptedModel;

            pAdaptedModel = NULL;

            // The expected precision is not attained ... try something else
            int32_t NbOfTilesX (1);
            int32_t NbOfTilesY (1);
            HFCPtr<HGF2DLocalProjectiveGrid> pGridModel (NULL);

            while ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError) && (NbOfTilesX < MAX_GRIDSIZE || NbOfTilesY < MAX_GRIDSIZE))
                {
                NbOfTilesX +=2;
                NbOfTilesY +=2;

                pGridModel = new HGF2DLocalProjectiveGrid (*pTempModel, ImageExtent, NbOfTilesX, NbOfTilesY);

                // Check precision of adapter
                pGridModel->StudyPrecisionOver (ImageExtent, pi_Step, &MeanError, &MaxError);
                }

            // Check if precision is respected
            if ((MeanError > pi_ExpectedMeanError) || (MaxError > pi_ExpectedMaxError))
                {
                // Precision still not attained ...  return empty model
                pResultModel = 0;
                }
            else
                {
                // Return the projective grid
                pResultModel = pGridModel;
                }
            }
        else
            {
            pResultModel = pAdaptedModel;
            }


        // Check if a model of sufficient precision was obtained...
        if (pResultModel != 0)
            {
            // Add the transformation from logical to pseudo base.
            pResultModel = pIDToBaseTransfo->ComposeInverseWithDirectOf (*pResultModel);
            }

        // Check if adaptation errors must be returned and set if needed
        if (po_pAdaptationMeanError != 0)
            *po_pAdaptationMeanError = MeanError;

        if (po_pAdaptationMaxError != 0)
            *po_pAdaptationMaxError = MaxError;
        }
    catch (...)
        {
        HASSERT(!"Error in HCPGCoordUtility::CreateGCoordAdaptedModel");
        pResultModel = NULL;
        }

    return (pResultModel);
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::TranslateToMeter
(
const HFCPtr<HGF2DTransfoModel>& pi_pModel,
double                           pi_FactorModelToMeter
) 
    {
    HPRECONDITION(pi_pModel != 0);
    HFCPtr<HGF2DTransfoModel> pTransfo = pi_pModel;

    double effectiveFactorModelToMeter = pi_FactorModelToMeter;
    
    // Apply to Matrix
    if (effectiveFactorModelToMeter != 1.0)
        {
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();

        pScaleModel->SetXScaling(effectiveFactorModelToMeter);
        pScaleModel->SetYScaling(effectiveFactorModelToMeter);

        pTransfo = pTransfo->ComposeInverseWithDirectOf(*pScaleModel);
        }

    return pTransfo;
    }


///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Marc.Bedard                     01/2014
//+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::TranslateFromMeter
(
const HFCPtr<HGF2DTransfoModel>& pi_pModel,
double                           pi_FactorModelToMeter,
bool*                            po_pDefaultUnitWasFound
) 
    {
    HPRECONDITION(pi_pModel != 0);

    double FactorModelToMeter = 1.0;
    bool   isUnitWasFound = false;

    if (po_pDefaultUnitWasFound != NULL)
        {
        *po_pDefaultUnitWasFound = isUnitWasFound;
        }
    
    HFCPtr<HGF2DTransfoModel> pTransfo = pi_pModel;
    
    // Apply inverse factor to Matrix
    if (FactorModelToMeter != 1.0)
        {
        HASSERT(FactorModelToMeter != 0.0);
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
        pScaleModel->SetXScaling(1.0 / FactorModelToMeter);
        pScaleModel->SetYScaling(1.0 / FactorModelToMeter);

        pTransfo = pTransfo->ComposeInverseWithDirectOf(*pScaleModel);
        }

    return pTransfo;
    }

/*---------------------------------------------------------------------------------**//*
//* @bsimethod                                    Laurent.Robert-Veillette        03/2016
//Private method to compare two models over a specified area in the direct direction
//+---------------+---------------+---------------+---------------+---------------+------*/
void     HCPGCoordUtility::CompareModelsOver(const HGF2DTransfoModel& ExactModel,
                           const HGF2DTransfoModel& ModelToTest,
                           const HGF2DLiteExtent& pi_PrecisionArea,
                           double                pi_Step,
                           double*               po_pMeanError,
                           double*               po_pMaxError)
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_PrecisionArea.GetWidth() != 0.0);
    HPRECONDITION(pi_PrecisionArea.GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // Recipient variables must be provided
    HPRECONDITION(po_pMeanError != nullptr);
    HPRECONDITION(po_pMaxError != nullptr);

    // Convert in temporary variables
    double TempX;
    double TempY;

    double MaxDirectError = 0.0;
    double DirectStatSumX = 0.0;
    double DirectStatSumY = 0.0;
    uint32_t DirectStatNumSamples = 0;

    double TempX1;
    double TempY1;

    double CurrentX;
    double CurrentY;

    for (CurrentY = pi_PrecisionArea.GetYMin(); CurrentY < pi_PrecisionArea.GetYMax(); CurrentY += pi_Step)
        {
        for (CurrentX = pi_PrecisionArea.GetXMin(); CurrentX < pi_PrecisionArea.GetXMax(); CurrentX += pi_Step)
            {
            //Ignore point that cannot be converted, we don't want to compute model precision using point outside valid domain
            if ((SUCCESS == ExactModel.ConvertDirect(CurrentX, CurrentY, &TempX, &TempY)) &&
                (SUCCESS == ModelToTest.ConvertDirect(CurrentX, CurrentY, &TempX1, &TempY1)))
                {
                // Compute difference
                double DeltaX = fabs(TempX - TempX1);
                double DeltaY = fabs(TempY - TempY1);

                // Add deltas
                DirectStatSumX += DeltaX;
                DirectStatSumY += DeltaY;
                DirectStatNumSamples++;

                MaxDirectError = MAX(MaxDirectError, MAX(DeltaX, DeltaY));
                }               
            }
        }

    // Assign precision results
    *po_pMaxError = MaxDirectError;
    *po_pMeanError = (DirectStatNumSamples > 0 ? (DirectStatSumX + DirectStatSumY) / (2 * DirectStatNumSamples) : 0.0);
    }
