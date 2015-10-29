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
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

#include <GeoCoord\BaseGeoCoord.h>
#include <ImagePP/all/h/HFCRasterGeoCoordinateServices.h>


HFC_IMPLEMENT_SINGLETON (HCPGCoordUtility)
#define MAX_GRIDSIZE (300)

//-----------------------------------------------------------------------------
// HCPGCoordUtility
//-----------------------------------------------------------------------------
HCPGCoordUtility::HCPGCoordUtility ()
    {
    }

//-----------------------------------------------------------------------------
// ~HCPGCoordUtility
//-----------------------------------------------------------------------------
HCPGCoordUtility::~HCPGCoordUtility ()
    {
    }


/** -----------------------------------------------------------------------------
This method compares two coordinate systems. It returns true if they are
the same.

@param pi_rpBaseGeoCoord1 A valid BaseGeoCoord pointer, representing the first
file to compare.

@param pi_rpBaseGeoCoord2 A valid BaseGeoCoord pointer, representing the second
file to compare.

@return bool representing true if the coordinate systems are the same and
false if they are not.
-----------------------------------------------------------------------------
*/
bool HCPGCoordUtility::AreBaseGCSEquivalent
(
    IRasterBaseGcsCP pi_rpBaseGeoCoord1,
    IRasterBaseGcsCP pi_rpBaseGeoCoord2
)
    {
    if (pi_rpBaseGeoCoord1==NULL && pi_rpBaseGeoCoord2==NULL)
        return true;
    else if(pi_rpBaseGeoCoord1==NULL || pi_rpBaseGeoCoord2==NULL)
        return false;

    return pi_rpBaseGeoCoord1->IsEquivalent(*pi_rpBaseGeoCoord2);
    }

/** -----------------------------------------------------------------------------
    This method creates a GCoord model based on given projection. It is also
    possible to directly instantiate a HCPGCoordModel object directly
    with the same parameters.

    @param pi_SourceProjection A Valid GCoord projection representing the
                               projection of the direct channel of the model.

    @param pi_DestinationProjection A Valid GCoord projection representing the
                                    projection of the inverse channel of the model.


    @return a smart pointer to a GCoord transformation model created or a
            null pointer if for some reason the model could not be created.

    @see HCPGCoordModel
    -----------------------------------------------------------------------------
*/
HFCPtr<HCPGCoordModel> HCPGCoordUtility::CreateGCoordModel
(
    IRasterBaseGcsR pi_SourceProjection,
    IRasterBaseGcsR pi_DestinationProjection
) const
    {
    // Create GCoord model
    return (new HCPGCoordModel (pi_SourceProjection, pi_DestinationProjection));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateGCoordAdaptedModel
(
    IRasterBaseGcsR pi_SourceProjection,
    IRasterBaseGcsR pi_DestinationProjection,
    const HGF2DLiteExtent& pi_rExtent,
    double  pi_Step,
    double  pi_ExpectedMeanError,
    double  pi_ExpectedMaxError,
    double* po_pAdaptationMeanError,
    double* po_pAdaptationMaxError,
    double* po_pReversibilityMeanError,
    double* po_pReversibilityMaxError
) const
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
                        units (@see IRasterBaseGcsPtr::GetUnit()). This step
                        is used for the sampling in the creation of the adapters
                        as well as for the check in precision.

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
                        units (@see IRasterBaseGcsPtr::GetUnit()). This step
                        is used for the sampling in the creation of the adapters
                        as well as for the check in precision.

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
    IRasterBaseGcsR                pi_SourceProjection,
    IRasterBaseGcsR                pi_DestinationProjection,
    const HFCPtr<HRFRasterFile>&    pi_rpRasterFile,
    uint32_t                       pi_Page,
    const HGF2DWorldCluster&        pi_rCluster,
    HGF2DWorldIdentificator         pi_DestinationBaseWorld,
    double                          pi_Step,
    double                          pi_ExpectedMeanError,
    double                          pi_ExpectedMaxError,
    double*                         po_pAdaptationMeanError,
    double*                         po_pAdaptationMaxError,
    double*                         po_pReversibilityMeanError,
    double*                         po_pReversibilityMaxError
) const
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
            HFCPtr<HCPGCoordModel> pBMModel = static_cast<HCPGCoordModel*>(&(*pTempModel));

            double ReverseMeanError;
            double ReverseMaxError;
            double ScaleChangeMean;
            double ScaleChangeMax;
            pBMModel->StudyReversibilityPrecisionOver (ImageExtent, pi_Step, &ReverseMeanError, &ReverseMaxError, &ScaleChangeMean, &ScaleChangeMax);

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
IRasterBaseGcsCR                rasterGcs,
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
    IRasterBaseGcsCR gcs = rasterGcs;
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
    ImagePP::WGS84ConvertCode datumConvert = gcs.GetDatumConvertMethod();

    if ((ImagePP::ConvertType_MREG  == datumConvert) ||
        (ImagePP::ConvertType_NAD27 == datumConvert) ||
        (ImagePP::ConvertType_HPGN  == datumConvert) ||  
        (ImagePP::ConvertType_AGD66 == datumConvert) ||  
        (ImagePP::ConvertType_AGD84 == datumConvert) ||
        (ImagePP::ConvertType_NZGD4 == datumConvert) ||   
        (ImagePP::ConvertType_ATS77 == datumConvert) ||  
        (ImagePP::ConvertType_CSRS  == datumConvert) ||   
        (ImagePP::ConvertType_TOKYO == datumConvert) ||   
        (ImagePP::ConvertType_RGF93 == datumConvert) ||  
        (ImagePP::ConvertType_ED50  == datumConvert) ||    
        (ImagePP::ConvertType_DHDN  == datumConvert) ||
        (ImagePP::ConvertType_GENGRID == datumConvert) ||
        (ImagePP::ConvertType_CHENYX == datumConvert))
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


    const ImagePP::ProjectionCodeValue projectionCode = gcs.GetProjectionCode();
    switch (projectionCode)
        {
        case ImagePP::pcvCassini : // Not so sure about this one ... check http://www.radicalcartography.net/?projectionref
        case ImagePP::pcvEckertIV :
        case ImagePP::pcvEckertVI :
        case ImagePP::pcvMillerCylindrical :
        case ImagePP::pcvUnity :
        case ImagePP::pcvGoodeHomolosine :
        case ImagePP::pcvModifiedStereographic :
        case ImagePP::pcvEqualAreaAuthalicNormal :
        case ImagePP::pcvEqualAreaAuthalicTransverse :
        case ImagePP::pcvSinusoidal :
        case ImagePP::pcvVanderGrinten :
        case ImagePP::pcvRobinsonCylindrical :
        case ImagePP::pcvWinkelTripel :
        case ImagePP::pcvEquidistantCylindrical :
        case ImagePP::pcvEquidistantCylindricalEllipsoid :
        case ImagePP::pcvPlateCarree :
            // good around the globe          
            return GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);

        case ImagePP::pcvMercatorScaleReduction :
        case ImagePP::pcvMercator :
        case ImagePP::pcvPopularVisualizationPseudoMercator :
            // good pretty close 90 degrees east and west of central meridian
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian (), 179.999999, 
                                                   80.0);
        
        case ImagePP::pcvLambertEquidistantAzimuthal :
        case ImagePP::pcvAzimuthalEquidistantElevatedEllipsoid :
        case ImagePP::pcvLambertEqualAreaAzimuthal :
        case ImagePP::pcvOrthographic :
        case ImagePP::pcvObliqueStereographic :
        case ImagePP::pcvSnyderObliqueStereographic :
        case ImagePP::pcvPolarStereographic :
        case ImagePP::pcvPolarStereographicStandardLatitude :
        case ImagePP::pcvGnomonic :
        case ImagePP::pcvBipolarObliqueConformalConic :
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

        case ImagePP::pcvTransverseMercator :
        case ImagePP::pcvGaussKrugerTranverseMercator :
        case ImagePP::pcvSouthOrientedTransverseMercator :
        case ImagePP::pcvTransverseMercatorAffinePostProcess :
        case ImagePP::pcvTransverseMercatorMinnesota :
        case ImagePP::pcvTransverseMercatorWisconsin:
        case ImagePP::pcvTransverseMercatorKruger :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian(), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
        case ImagePP::pcvTotalTransverseMercatorBF :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian(), 30.0, 
                                                   89.9);
#endif

        // The following are close enough to TM but require latitude origin
        case ImagePP::pcvObliqueCylindricalHungary :
        case ImagePP::pcvTransverseMercatorOstn97 :
        case ImagePP::pcvTransverseMercatorOstn02 :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetOriginLongitude(), 30.0, 
                                                   89.9);

        case ImagePP::pcvCzechKrovak :
        case ImagePP::pcvCzechKrovakObsolete :
        case ImagePP::pcvCzechKrovak95 :
        case ImagePP::pcvCzechKrovak95Obsolete :
            // Hard-coded domain as origin longitude give 17.39W which couldn't be used as a central meridian for this
            // area. According to Alain Robert, these projections are oblique/conical and this strange origin longitude
            // could have been used as a mean of correction for non-standard prime meridian (not greenwich) used.
            return GetRangeAboutMeridianAndParallel(shape, 
                                                    17.5, 7.5, 
                                                    49.5, 2.5);
        case ImagePP::pcvTransverseMercatorDenmarkSys34 :
        case ImagePP::pcvTransverseMercatorDenmarkSys3499 :
        case ImagePP::pcvTransverseMercatorDenmarkSys3401 :
            {
            int region = gcs.GetDanishSys34Region();

            // 1  ==> jylland
            // 2  ==> sj�lland
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
        case ImagePP::pcvAmericanPolyconic :
        case ImagePP::pcvModifiedPolyconic :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndBoundParallel(shape,
                                                         gcs.GetCentralMeridian(), 89.999999,
                                                         gcs.GetOriginLatitude(), 30.0,
                                                         -89.9, 89.9);
                                                                  
        case ImagePP::pcvLambertTangential :
        case ImagePP::pcvLambertConformalConicOneParallel :
        case ImagePP::pcvSnyderTransverseMercator :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndBoundParallel(shape, 
                                                         gcs.GetOriginLongitude(), 89.999999,
                                                         gcs.GetOriginLatitude(), 30.0,
                                                         -89.9, 89.9);

        case ImagePP::pcvBonne :
            return GetRangeAboutMeridianAndBoundParallel(shape, 
                                                         gcs.GetOriginLongitude(), 170.999999,
                                                         gcs.GetOriginLatitude(), 60.0,
                                                         -89.9, 89.9);

        case ImagePP::pcvEquidistantConic :
        case ImagePP::pcvAlbersEqualArea :
        case ImagePP::pcvLambertConformalConicTwoParallel :
        case ImagePP::pcvLambertConformalConicWisconsin :
        case ImagePP::pcvLambertConformalConicBelgian :
        case ImagePP::pcvLambertConformalConicMinnesota:
        case ImagePP::pcvLambertConformalConicAffinePostProcess :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndTwoStandardBoundParallel(shape, 
                                                                    gcs.GetOriginLongitude(), 89.9999,
                                                                    gcs.GetStandardParallel1(), gcs.GetStandardParallel2(), 30.0,
                                                                    -80.0, 80.0);

        case ImagePP::pcvObliqueCylindricalSwiss :
            // This projection is usually only used in Switzerland but can also be used in Hungary
            // we cannot hard code the extent based on the Switzerland extent but must instead compute the
            // extent based on the latitude and longitude of origin.
            return GetRangeAboutMeridianAndParallel(shape, 
                                                    gcs.GetOriginLongitude(), 6.0, 
                                                    gcs.GetOriginLatitude(), 6.0);


        // Other local projections
        case ImagePP::pcvHotineObliqueMercator :
        case ImagePP::pcvNewZealandNationalGrid :
        case ImagePP::pcvMollweide :
        case ImagePP::pcvRectifiedSkewOrthomorphic :
        case ImagePP::pcvRectifiedSkewOrthomorphicCentered :
        case ImagePP::pcvRectifiedSkewOrthomorphicOrigin :
        case ImagePP::pcvHotineObliqueMercator1UV :
        case ImagePP::pcvHotineObliqueMercator1XY :
        case ImagePP::pcvHotineObliqueMercator2UV :
        case ImagePP::pcvHotineObliqueMercator2XY :
        case ImagePP::pcvObliqueMercatorMinnesota :
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
        case ImagePP::pcvNonEarth :
        case ImagePP::pcvNonEarthScaleRotation :
        case ImagePP::pcvObliqueConformalConic :
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

        case ImagePP::pcvUniversalTransverseMercator :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   GetUTMZoneCenterMeridian(gcs.GetUTMZone()), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
        case ImagePP::pcvTotalUniversalTransverseMercatorBF :
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


