//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGCoordUtility.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>                // must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HCPGCoordUtility.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DLinearModelAdapter.h>
#include <Imagepp/all/h/HGF2DLocalProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <Imagepp/all/h/HRFGdalUtilities.h>

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
* Extract units from meters form a GeoTiffKeys List 
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  HCPGCoordUtility::GetUnitsFromMeters
(
double&                unitFromMeter, 
HCPGeoTiffKeys const&  geoTiffKeys,
bool                   pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation
)
    {
    unitFromMeter=1.0;
    bool isUnitWasFound(false);

    //&&AR GCS validate that geocoord is init?
//     IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
//     if (pService == NULL)
//         return isUnitWasFound;

    GeoCoordinates::BaseGCSPtr pInputGeocoding = GeoCoordinates::BaseGCS::CreateGCS();
    if(pInputGeocoding.IsNull() || SUCCESS != pInputGeocoding->InitFromGeoTiffKeys (nullptr, nullptr, &geoTiffKeys))
        return false;

    if (pInputGeocoding.IsValid() && pInputGeocoding->IsValid())
        {
        //TRICKY: By default, GeoGCS will use the ProjLinearUnits geokey to override default unit found in PCS.
        //If user ask otherwise (pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation==false)
        //we will remove ProjLinearUnits from geokeys and re-create geocoding to get unit from the original PCS.
        //We do this only temporarily here because setting can change during session...
        if (!pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation && geoTiffKeys.HasKey(ProjLinearUnits))
            {
            HFCPtr<HCPGeoTiffKeys> pNewKeys = geoTiffKeys.Clone(); 
            pNewKeys->EraseKey(ProjLinearUnits);
            GeoCoordinates::BaseGCSPtr pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
            
            if(pBaseGcs.IsValid() && SUCCESS == pBaseGcs->InitFromGeoTiffKeys (nullptr, nullptr, pNewKeys.GetPtr()))
                pInputGeocoding = pBaseGcs;
            }

        if (pInputGeocoding != NULL && pInputGeocoding->IsValid())
            {
            unitFromMeter = pInputGeocoding->UnitsFromMeters();
            isUnitWasFound = true;
            }
        }
    else if (geoTiffKeys.HasKey(ProjLinearUnits))
        {
        //The GCS is not valid but there is a unit defined by the geokeys
        //Interpret this unit.
        //scan all supported unit name and then check for corresponding EPSG code
        uint32_t unitCode;
        geoTiffKeys.GetValue(ProjLinearUnits, &unitCode);

        isUnitWasFound = GetUnitsFromMeters(unitFromMeter,unitCode);
        }

    return isUnitWasFound;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCSPtr HCPGCoordUtility::CreateRasterGcsFromERSIDS(uint32_t pi_EPSGCode, WStringCR pi_rErmProjection, WStringCR pi_rErmDatum, WStringCR pi_rErmUnits)
    {
    //&&AR GCS validate that geocoord is init?
//     IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
//     if(pService == nullptr)
//         return nullptr;

    // First part ... we try to set the geokeys by ourselves ...
    if (pi_rErmProjection == L"RAW")
        return nullptr;// no geotiff info

    GeoCoordinates::BaseGCSPtr pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
    if(pBaseGcs == nullptr)
        return nullptr;

    uint32_t ModelType;

    if (pi_rErmProjection == L"GEODETIC" || pi_rErmProjection == L"LOCAL")
        ModelType = TIFFGeo_ModelTypeGeographic;
    else if (pi_rErmProjection == L"GEOCENTRIC")
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

    // Set Unit field
    if ((pi_rErmUnits == L"FEET") || (pi_rErmUnits == L"U.S. SURVEY FOOT"))
        ProjLinearUnit = TIFFGeo_Linear_Foot_US_Survey; // US Foot
    else if (pi_rErmUnits ==  L"IFEET")
        ProjLinearUnit = TIFFGeo_Linear_Foot; // Foot
    else if (pi_rErmUnits ==  L"DEGREES")
        GeogAngularUnit = 9102; // Degree
    else if (pi_rErmUnits == L"METERS")
        ProjLinearUnit = TIFFGeo_Linear_Meter; // Meter
    else if (pi_rErmUnits == L"IMPERIAL YARD")
        ProjLinearUnit = TIFFGeo_British_Yard_1895;
    else
        {
        HASSERT_DATA(0); //Check if it isn't a new unit that should be handled by a special case
        }

    HFCPtr<HCPGeoTiffKeys> pGeokeys(new HCPGeoTiffKeys());

    if (pi_EPSGCode != TIFFGeo_UserDefined)
        {
        pGeokeys->AddKey(GTModelType, ModelType);

        if (ModelType == TIFFGeo_ModelTypeGeographic)
            {
            pGeokeys->AddKey(GeographicType, pi_EPSGCode);
            }
        else
            {
            if (pi_EPSGCode > USHRT_MAX)
                {
                pGeokeys->AddKey(ProjectedCSTypeLong, pi_EPSGCode);
                }
            else
                {
                pGeokeys->AddKey(ProjectedCSType, pi_EPSGCode);
                }
            }

        if (ProjLinearUnit != 0)
            {
            pGeokeys->AddKey(ProjLinearUnits, ProjLinearUnit);
            }

        if (GeogAngularUnit != 0)
            {
            pGeokeys->AddKey(GeogAngularUnits, GeogAngularUnit);
            }
        }

    if(SUCCESS != pBaseGcs->InitFromGeoTiffKeys(nullptr, nullptr, pGeokeys.GetPtr()))
        {
        WString wkt;

        HRFGdalUtilities::ConvertERMToOGCWKT(wkt, pi_rErmProjection, pi_rErmDatum, pi_rErmUnits);
        
        if (WString::IsNullOrEmpty(wkt.c_str()))
            {
            pGeokeys->AddKey(GTModelType, ModelType);

            if (ModelType == TIFFGeo_ModelTypeGeographic)
                {
                pGeokeys->AddKey(GeographicType, pi_EPSGCode);
                }
            else
                {
                HASSERT(pi_EPSGCode == TIFFGeo_UserDefined);
                pGeokeys->AddKey(ProjectedCSType, pi_EPSGCode);
                }

            if (ProjLinearUnit != 0)
                {
                pGeokeys->AddKey(ProjLinearUnits, ProjLinearUnit);
                }

            if (GeogAngularUnit != 0)
                {
                pGeokeys->AddKey(GeogAngularUnits, GeogAngularUnit);
                }
            }
        else // Fallback solution only available if baseGeoCoord is loaded
            {
            //&&AR GCS review: why not use the GCS object that we just created?
            GeoCoordinates::BaseGCSPtr pBaseGeoCoord = GeoCoordinates::BaseGCS::CreateGCS();

            //&&AR GCS review: if it is ok to use flavor 0 (none in csmap)? if yes add it to BaseGCS::WktFlavor enum?
            StatusInt Status = pBaseGeoCoord->InitFromWellKnownText(nullptr, nullptr, (GeoCoordinates::BaseGCS::WktFlavor)0, wkt.c_str());

            if (SUCCESS != Status)
                {
                pGeokeys->AddKey(GTModelType, ModelType);

                if (ModelType == TIFFGeo_ModelTypeGeographic)
                    {
                    pGeokeys->AddKey(GeographicType, pi_EPSGCode);
                    }
                else
                    {
                    if (pi_EPSGCode > USHRT_MAX)
                        {   
                        pGeokeys->AddKey(ProjectedCSTypeLong, pi_EPSGCode);
                        }
                    else
                        {
                        pGeokeys->AddKey(ProjectedCSType, pi_EPSGCode);
                        }                        
                    }

                if (ProjLinearUnit != 0)
                    {
                    pGeokeys->AddKey(ProjLinearUnits, ProjLinearUnit);
                    }        

                if (GeogAngularUnit != 0)
                    {
                    pGeokeys->AddKey(GeogAngularUnits, GeogAngularUnit);
                    }  
                }
            }
        }

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    if(SUCCESS != pBaseGcs->InitFromGeoTiffKeys(nullptr, nullptr, pGeokeys.GetPtr()))
        return NULL;

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
* @bsimethod                                                Alain.Robert    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeSpecified (vector<HGF2DCoord<double> >&     shape, 
                              double              minimumLongitude,
                              double              maximumLongitude,
                              double              minimumLatitude,
                              double              maximumLatitude)
    {
    shape.push_back(HGF2DCoord<double>(minimumLongitude, minimumLatitude));
    shape.push_back(HGF2DCoord<double>(minimumLongitude, maximumLatitude));
    shape.push_back(HGF2DCoord<double>(maximumLongitude, maximumLatitude));
    shape.push_back(HGF2DCoord<double>(maximumLongitude, minimumLatitude));
    shape.push_back(HGF2DCoord<double>(minimumLongitude, minimumLatitude));
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutPrimeMeridianAndEquator (vector<HGF2DCoord<double> >&     shape, 
                                                double              allowedDeltaAboutPrimeMeridian,
                                                double              allowedDeltaAboutEquator)
    {
    shape.push_back(HGF2DCoord<double>(-allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(-allowedDeltaAboutPrimeMeridian, allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(allowedDeltaAboutPrimeMeridian, allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(-allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndEquator  (vector<HGF2DCoord<double> >&     shape, 
                                            double              specifiedMeridian,
                                            double              allowedDeltaAboutMeridian,
                                            double              allowedDeltaAboutEquator)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    const double minLatitude = -allowedDeltaAboutEquator;
    const double maxLatitude = allowedDeltaAboutEquator; 

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndParallel (vector<HGF2DCoord<double> >&     shape, 
                                            double              specifiedMeridian,
                                            double              allowedDeltaAboutMeridian,
                                            double              specifiedParallel,
                                            double              allowedDeltaAboutParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    const double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    const double maxLatitude = specifiedParallel + allowedDeltaAboutParallel;   

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndBoundParallel    (vector<HGF2DCoord<double> >&     shape, 
                                                    double              specifiedMeridian,
                                                    double              allowedDeltaAboutMeridian,
                                                    double              specifiedParallel,
                                                    double              allowedDeltaAboutParallel,
                                                    double              southMostAllowedParallel,
                                                    double              northMostAllowedParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;

    double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    if (minLatitude < southMostAllowedParallel)
        minLatitude = southMostAllowedParallel;

    double maxLatitude = specifiedParallel + allowedDeltaAboutParallel;
    if (maxLatitude > northMostAllowedParallel)
        maxLatitude = northMostAllowedParallel;

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndTwoStandardBoundParallel (vector<HGF2DCoord<double> >&     shape, 
                                                            double              specifiedMeridian,
                                                            double              allowedDeltaAboutMeridian,
                                                            double              standardParallel1,
                                                            double              standardParallel2,
                                                            double              allowedDeltaAboutParallels,
                                                            double              southMostAllowedParallel,
                                                            double              northMostAllowedParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;

    double minLatitude;
    double maxLatitude;
    if (standardParallel1 < standardParallel2)
        {
        minLatitude = standardParallel1 - allowedDeltaAboutParallels;
        if (minLatitude < southMostAllowedParallel)
            minLatitude = southMostAllowedParallel;
        maxLatitude = standardParallel2 + allowedDeltaAboutParallels;
        if (maxLatitude > northMostAllowedParallel)
            maxLatitude = northMostAllowedParallel;
        }
    else
        {
        minLatitude = standardParallel2 - allowedDeltaAboutParallels;
        if (minLatitude < southMostAllowedParallel)
            minLatitude = southMostAllowedParallel;
        maxLatitude = standardParallel1 + allowedDeltaAboutParallels;
        if (maxLatitude > northMostAllowedParallel)
            maxLatitude = northMostAllowedParallel;
        }



    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutBoundMeridianAndBoundParallel   (vector<HGF2DCoord<double> >&     shape, 
                                                        double              specifiedMeridian,
                                                        double              allowedDeltaAboutMeridian,
                                                        double              westMostAllowedMeridian,
                                                        double              eastMostAllowedMeridian,
                                                        double              specifiedParallel,
                                                        double              allowedDeltaAboutParallel,
                                                        double              southMostAllowedParallel,
                                                        double              northMostAllowedParallel)
    {
    double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    if (minLongitude < westMostAllowedMeridian)
        minLongitude = westMostAllowedMeridian;
    double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    if (maxLongitude > eastMostAllowedMeridian)
        maxLongitude = eastMostAllowedMeridian;
    double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    if (minLatitude < southMostAllowedParallel) 
        minLatitude = southMostAllowedParallel;   
    double maxLatitude = specifiedParallel + allowedDeltaAboutParallel; 
    if (maxLatitude > northMostAllowedParallel)
        maxLatitude = northMostAllowedParallel;   

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline double GetUTMZoneCenterMeridian(int zoneNumber)
    {
    return (zoneNumber - 30) * 6;
    }


/*---------------------------------------------------------------------------------**//**
* Returns the domain of application for GCS. This domain is the math domain intersected
* with the logical domain if one is set.
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt HCPGCoordUtility::GetGeoDomain
(
GeoCoordinates::BaseGCSCR rasterGcs,
vector<HGF2DCoord<double> >&    shape
) 
    {
    using namespace GeoCoordinates;

    
//    BaseGCSCP pGcs = rasterGcs.GetBaseGCS();
//
//    if (pGcs == NULL) 
//        return ERROR;
//
//    BaseGCSCR gcs = *pGcs;
    GeoCoordinates::BaseGCSCR gcs = rasterGcs;
    // Some explanation about the values specified below and their intent.
    // First it must be inderstood that the current implementation is in progress.
    // The present implementation fixes some reported issues related to the
    // display and management of rasters when reprojection is invloved.
    // The principle attempts to define the geo domain of a specific projection using
    // extent defined as min and max longitude and latitude. Such definition is adequate
    // for many projections but not all. For example Lamber Comformal Conic domain is
    // domain is correctle defined using such definition. For transverse mercator and derivatives
    // the domain can likewise be defined using this method. Others like Oblique Mercator
    // or stereo graphic projections cannot as their area definition is not alligned
    // to latitude and longitudes. We assume that an smaller area can be defined using
    // plain geo extent but we are not sure. When the North and South pole are included we
    // have not yet defined a way to indicate this representation other than by specifying
    // exact min or max to either North or Sout pole latitude but the actual
    // case never occured so the implementation has currently been postponed
    // till more adequate research can be done.
    //
    // Concerning the definition of Transverse Mercators and derivative the mathematical domain
    // is usually defined from North to South pole on a longitude with of some
    // specific value ... We provide a very large area in this case. In practice we have had
    // cases where the datum shift during the reprojection process shifted the North and South pole
    // sufficiently that a longitude located on one side of the Earth became in the other datum
    // on the other size of the pole (17E Longitude became 163W Longitude)
    // For this reason we have decided to limit the upper and lower latitudes for all
    // projections to 89.9 degrees (any greater values resulted in the problem in our case)
    // This means that the zone will remain about 12 kilometers from the poles. For cartography
    // made in the pole areas, other projection methods will have to be used.

    // If datum transformation method is limitative by nature we will use the user-defined domain.
    GeoCoordinates::WGS84ConvertCode datumConvert = gcs.GetDatumConvertMethod();

    if ((GeoCoordinates::WGS84ConvertCode::ConvertType_MREG  == datumConvert) ||
        (GeoCoordinates::WGS84ConvertCode::ConvertType_NAD27 == datumConvert) ||
        (GeoCoordinates::WGS84ConvertCode::ConvertType_HPGN  == datumConvert) ||  
        (GeoCoordinates::WGS84ConvertCode::ConvertType_AGD66 == datumConvert) ||  
        (GeoCoordinates::WGS84ConvertCode::ConvertType_AGD84 == datumConvert) ||
        (GeoCoordinates::WGS84ConvertCode::ConvertType_NZGD4 == datumConvert) ||   
        (GeoCoordinates::WGS84ConvertCode::ConvertType_ATS77 == datumConvert) ||  
        (GeoCoordinates::WGS84ConvertCode::ConvertType_CSRS  == datumConvert) ||   
        (GeoCoordinates::WGS84ConvertCode::ConvertType_TOKYO == datumConvert) ||   
        (GeoCoordinates::WGS84ConvertCode::ConvertType_RGF93 == datumConvert) ||  
        (GeoCoordinates::WGS84ConvertCode::ConvertType_ED50  == datumConvert) ||    
        (GeoCoordinates::WGS84ConvertCode::ConvertType_DHDN  == datumConvert) ||
#ifdef GEOCOORD_ENHANCEMENT //&&AR CLEANUP
        (GeoCoordinates::WGS84ConvertCode::ConvertType_GENGRID == datumConvert) ||
#else
        (27 == datumConvert) ||
#endif
        (GeoCoordinates::WGS84ConvertCode::ConvertType_CHENYX == datumConvert))
        {
        double minLongitude = gcs.GetMinimumUsefulLongitude();
        double maxLongitude = gcs.GetMaximumUsefulLongitude();
        double minLatitude = gcs.GetMinimumUsefulLatitude();
        double maxLatitude = gcs.GetMaximumUsefulLatitude();
        if ((minLongitude != maxLongitude) && (minLatitude != minLongitude))
            {
            return GetRangeSpecified(shape, minLongitude, maxLongitude, minLatitude, maxLatitude);
            }
        }


    GeoCoordinates::BaseGCS::ProjectionCodeValue projectionCode = gcs.GetProjectionCode();
    switch (projectionCode)
        {
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvCassini : // Not so sure about this one ... check http://www.radicalcartography.net/?projectionref
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEckertIV :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEckertVI :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvMillerCylindrical :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvUnity :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvGoodeHomolosine :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvModifiedStereographic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEqualAreaAuthalicNormal :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEqualAreaAuthalicTransverse :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvSinusoidal :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvVanderGrinten :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvRobinsonCylindrical :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvWinkelTripel :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEquidistantCylindrical :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEquidistantCylindricalEllipsoid :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvPlateCarree :
            // good around the globe          
            return GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvMercatorScaleReduction :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvMercator :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvPopularVisualizationPseudoMercator :
            // good pretty close 90 degrees east and west of central meridian
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian (), 179.999999, 
                                                   80.0);
        
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertEquidistantAzimuthal :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvAzimuthalEquidistantElevatedEllipsoid :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertEqualAreaAzimuthal :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvOrthographic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvObliqueStereographic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvSnyderObliqueStereographic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvPolarStereographic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvPolarStereographicStandardLatitude :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvGnomonic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvBipolarObliqueConformalConic :
            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

#if NOT_YET
            // This one is a bit complicated by the fact the hemisphere can be centered anywhere on earth. 
            // If centered at a pole, the domain extends from 0 to -90 in latitude and around the globe in longitude
            // If centered somewhere on the equator, then it is valid from North to South pole but 90 degrees east and west of center
            // If centered elsewhere, the area is not easily representable in the form of min max of lat long...
            return GetRangeAboutBoundMeridianAndBoundParallel(shape,
                                                              gcs.GetOriginLongitude(), 90.0,
                                                              -180.0, 180.0,
                                                              gcs.GetOriginLatitude(), 90.0,
                                                              -90.0, 90.0);
#endif   

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercator :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvGaussKrugerTranverseMercator :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvSouthOrientedTransverseMercator :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorAffinePostProcess :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorMinnesota :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorWisconsin:
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorKruger :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian(), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTotalTransverseMercatorBF :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian(), 30.0, 
                                                   89.9);
#endif

        // The following are close enough to TM but require latitude origin
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvObliqueCylindricalHungary :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorOstn97 :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorOstn02 :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetOriginLongitude(), 30.0, 
                                                   89.9);

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvCzechKrovak :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvCzechKrovakObsolete :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvCzechKrovak95 :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvCzechKrovak95Obsolete :
            // Hard-coded domain as origin longitude give 17.39W which couldn't be used as a central meridian for this
            // area. According to Alain Robert, these projections are oblique/conical and this strange origin longitude
            // could have been used as a mean of correction for non-standard prime meridian (not greenwich) used.
            return GetRangeAboutMeridianAndParallel(shape, 
                                                    17.5, 7.5, 
                                                    49.5, 2.5);
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorDenmarkSys34 :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorDenmarkSys3499 :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTransverseMercatorDenmarkSys3401 :
            {
            int region = gcs.GetDanishSys34Region();

            // 1  ==> jylland
            // 2  ==> sjælland
            // 3  ==> bornholm

            if (1 == region)
                {
                shape.push_back(HGF2DCoord<double>(8.2930, 54.7757));
                shape.push_back(HGF2DCoord<double>(7.9743, 55.0112));
                shape.push_back(HGF2DCoord<double>(7.5544, 56.4801));
                shape.push_back(HGF2DCoord<double>(8.0280, 57.1564));
                shape.push_back(HGF2DCoord<double>(10.4167, 58.0417));
                shape.push_back(HGF2DCoord<double>(10.9897, 57.7786));
                shape.push_back(HGF2DCoord<double>(11.5395, 57.1551));
                shape.push_back(HGF2DCoord<double>(12.0059, 56.5088));
                shape.push_back(HGF2DCoord<double>(11.7200, 54.9853));
                shape.push_back(HGF2DCoord<double>(10.5938, 54.5951));
                shape.push_back(HGF2DCoord<double>(8.2930, 54.7757)); 
                }
            else if (2 == region)
                {
                shape.push_back(HGF2DCoord<double>(11.5108, 54.4367));
                shape.push_back(HGF2DCoord<double>(10.2526, 54.6795));
                shape.push_back(HGF2DCoord<double>(9.6333, 55.0286));
                shape.push_back(HGF2DCoord<double>(9.6157, 55.3831));
                shape.push_back(HGF2DCoord<double>(10.0748, 56.0823));
                shape.push_back(HGF2DCoord<double>(11.5664, 56.9520));
                shape.push_back(HGF2DCoord<double>(13.2099, 565104));
                shape.push_back(HGF2DCoord<double>(13.2097, 54.8276));
                shape.push_back(HGF2DCoord<double>(12.8531, 54.6593));
                shape.push_back(HGF2DCoord<double>(12.1009, 54.5007));
                shape.push_back(HGF2DCoord<double>(11.5108, 54.4367));
                }
            else 
                {
                assert (3 == region);
                shape.push_back(HGF2DCoord<double>(14.510, 54.942));
                shape.push_back(HGF2DCoord<double>(14.510, 55.431));
                shape.push_back(HGF2DCoord<double>(15.300, 55.431));
                shape.push_back(HGF2DCoord<double>(15.300, 54.942));
                shape.push_back(HGF2DCoord<double>(14.510, 54.942));
                }


            }  
            return BSISUCCESS;

        // The conic
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvAmericanPolyconic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvModifiedPolyconic :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndBoundParallel(shape,
                                                         gcs.GetCentralMeridian(), 89.999999,
                                                         gcs.GetOriginLatitude(), 30.0,
                                                         -89.9, 89.9);
                                                                  
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertTangential :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertConformalConicOneParallel :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvSnyderTransverseMercator :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndBoundParallel(shape, 
                                                         gcs.GetOriginLongitude(), 89.999999,
                                                         gcs.GetOriginLatitude(), 30.0,
                                                         -89.9, 89.9);

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvBonne :
            return GetRangeAboutMeridianAndBoundParallel(shape, 
                                                         gcs.GetOriginLongitude(), 170.999999,
                                                         gcs.GetOriginLatitude(), 60.0,
                                                         -89.9, 89.9);

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvEquidistantConic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvAlbersEqualArea :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertConformalConicTwoParallel :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertConformalConicWisconsin :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertConformalConicBelgian :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertConformalConicMinnesota:
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvLambertConformalConicAffinePostProcess :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndTwoStandardBoundParallel(shape, 
                                                                    gcs.GetOriginLongitude(), 89.9999,
                                                                    gcs.GetStandardParallel1(), gcs.GetStandardParallel2(), 30.0,
                                                                    -80.0, 80.0);

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvObliqueCylindricalSwiss :
            // This projection is usually only used in Switzerland but can also be used in Hungary
            // we cannot hard code the extent based on the Switzerland extent but must instead compute the
            // extent based on the latitude and longitude of origin.
            return GetRangeAboutMeridianAndParallel(shape, 
                                                    gcs.GetOriginLongitude(), 6.0, 
                                                    gcs.GetOriginLatitude(), 6.0);


        // Other local projections
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvHotineObliqueMercator :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvNewZealandNationalGrid :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvMollweide :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvRectifiedSkewOrthomorphic :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvRectifiedSkewOrthomorphicCentered :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvRectifiedSkewOrthomorphicOrigin :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvHotineObliqueMercator1UV :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvHotineObliqueMercator1XY :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvHotineObliqueMercator2UV :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvHotineObliqueMercator2XY :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvObliqueMercatorMinnesota :
            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

#if NOT_YET
            // This one is a bit complicated by the fact the hemisphere can be centered anywhere on earth. 
            // If centered at a pole, the domain extends from 0 to -90 in latitude and around the globe in longitude
            // If centered somewhere on the equator, then it is valid from North to South pole but 90 degrees east and west of center
            // If centered elsewhere, the area is not easily representable in the form of min max of lat long...
            return GetRangeAboutBoundMeridianAndBoundParallel(shape,
                                                              gcs.GetOriginLongitude(), 30.0,
                                                              -180.0, 180.0,
                                                              gcs.GetOriginLatitude(), 30.0,
                                                              -89.9, 89.9);
#endif

        // Other
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvNonEarth :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvNonEarthScaleRotation :
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvObliqueConformalConic :
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvUniversalTransverseMercator :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   GetUTMZoneCenterMeridian(gcs.GetUTMZone()), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
        case GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvTotalUniversalTransverseMercatorBF :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   GetUTMZoneCenterMeridian(gcs.GetUTMZone()), 30.0, 
                                                   89.9);

#endif //TOTAL_SPECIAL
        default:
            break;
        }

    HASSERT(!"Not implemented ... please do so");
    return BSIERROR; 
    }


