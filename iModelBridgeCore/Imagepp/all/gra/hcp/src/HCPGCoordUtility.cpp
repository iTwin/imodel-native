//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGCoordUtility.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>                // must be first for PreCompiledHeader Option
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCPGCoordUtility.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DLinearModelAdapter.h>
#include <Imagepp/all/h/HGF2DLocalProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>
#include <Imagepp/all/h/HRFRasterFile.h>

#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

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
    const IRasterBaseGcsPtr& pi_rpBaseGeoCoord1,
    const IRasterBaseGcsPtr& pi_rpBaseGeoCoord2
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
    IRasterBaseGcsPtr pi_SourceProjection,
    IRasterBaseGcsPtr pi_DestinationProjection
) const
    {
    // Create GCoord model
    return (new HCPGCoordModel (pi_SourceProjection, pi_DestinationProjection));
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
HFCPtr<HGF2DTransfoModel> HCPGCoordUtility::CreateGCoordAdaptedModel
(
    IRasterBaseGcsPtr pi_SourceProjection,
    IRasterBaseGcsPtr pi_DestinationProjection,
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

        HFCPtr<HGF2DTransfoModel> pTempModel(new HCPGCoordModel (pi_SourceProjection, pi_DestinationProjection));

        // Check if reversibility must be studied
        if (po_pReversibilityMeanError != 0 || po_pReversibilityMaxError != 0)
            {
            // The caller desires knowledge about reversibility. study
            HFCPtr<HCPGCoordModel> pBMModel = static_cast<HCPGCoordModel*>(&(*pTempModel));

            double ReverseMeanError;
            double ReverseMaxError;
            double ScaleChangeMean;
            double ScaleChangeMax;
            pBMModel->StudyReversibilityPrecisionOver ( EnlargedExtent,
                                                        pi_Step,
                                                        &ReverseMeanError,
                                                        &ReverseMaxError,
                                                        &ScaleChangeMean,
                                                        &ScaleChangeMax);

            // If mean error is desired ... set it
            if (po_pReversibilityMeanError != 0)
                *po_pReversibilityMeanError = ReverseMeanError;

            // If max error is desired ... set it
            if (po_pReversibilityMaxError != 0)
                *po_pReversibilityMaxError = ReverseMaxError;

            }


        // Attempt acceleration with a projective adapter
        HFCPtr<HGF2DLinearModelAdapter> pAdaptedModel = new HGF2DLinearModelAdapter (*pTempModel,
                                                                                     EnlargedExtent,
                                                                                     pi_Step);

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

                pGridModel = new HGF2DLocalProjectiveGrid (*pTempModel, EnlargedExtent, NbOfTilesX, NbOfTilesY);

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
    IRasterBaseGcsPtr               pi_SourceProjection,
    IRasterBaseGcsPtr               pi_DestinationProjection,
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

        HGF2DLiteExtent ImageExtent (min (FirstPointX, min (SecondPointX, min (ThirdPointX, FourthPointX))),
                                     min (FirstPointY, min (SecondPointY, min (ThirdPointY, FourthPointY))),
                                     max (FirstPointX, max (SecondPointX, max (ThirdPointX, FourthPointX))),
                                     max (FirstPointY, max (SecondPointY, max (ThirdPointY, FourthPointY))));

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

            // If max error is desired ... set it
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

