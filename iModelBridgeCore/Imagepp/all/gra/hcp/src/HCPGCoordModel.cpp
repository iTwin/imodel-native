//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGCoordModel.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>                // must be first for PreCompiledHeader Option
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HCPGCoordUtility.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HFCException.h>


#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HCPGCoordModel::HCPGCoordModel ()
    :HGF2DTransfoModel ()
    {
    // The DLLs must be loaded to use this class.
    if (!GCSServices->_IsAvailable())
        throw HFCFileException(HFC_DLL_NOT_FOUND_EXCEPTION, GCSServices->_GetServiceName());

    Prepare ();

    HINVARIANTS;
    }

/** -----------------------------------------------------------------------------
    Creates a GCoord reprojection model based on two provided
    GCoord projections. The units of the model are based on the units
    of the projections given.

    @param pi_SourceGEOCS A Valid GCoord projection representing the
                          projection of the direct channel of the model.

    @param pi_DestinationGEOCS A Valid GCoord projection representing the
                               projection of the inversechannel of the model.


    -----------------------------------------------------------------------------
*/
HCPGCoordModel::HCPGCoordModel
(
    IRasterBaseGcsPtr  pi_SourceGEOCS,
    IRasterBaseGcsPtr  pi_DestinationGEOCS
)
    :HGF2DTransfoModel (),
     m_SourceGEOCS (pi_SourceGEOCS),
     m_DestinationGEOCS (pi_DestinationGEOCS)
    {
    // The DLLs must be loaded to use this class.
    if (!GCSServices->_IsAvailable ())
        throw HFCFileException(HFC_DLL_NOT_FOUND_EXCEPTION, GCSServices->_GetServiceName());

    // The two projections must be valid
    HPRECONDITION (m_SourceGEOCS->IsValid ());
    HPRECONDITION (m_DestinationGEOCS->IsValid ());
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HCPGCoordModel::HCPGCoordModel (const HCPGCoordModel& pi_rObj)
    :HGF2DTransfoModel (pi_rObj)
    {
    // The DLLs must be loaded to use this class.
    if (!GCSServices->_IsAvailable())
        throw HFCFileException(HFC_DLL_NOT_FOUND_EXCEPTION, GCSServices->_GetServiceName());

    Copy (pi_rObj);

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HCPGCoordModel::~HCPGCoordModel ()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HCPGCoordModel& HCPGCoordModel::operator=(const HCPGCoordModel& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModel::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }


/** -----------------------------------------------------------------------------
    Studies the reversibility of the model over a region using the given step.
    Since GCoord models are notably un-reversible when region of
    operation is far from usual region of application, it is recommended
    to estimate the reversibility of the model before using. The method
    will sample coordinate transformation by converting direct then inverse
    this result. The deviation from the original value is used in the
    calculation of mean and maximum error which are returned.

    @param pi_rPrecisionArea An extent over which to perform the study. The
                             area may not be empty.

    @param pi_Step The step used in X and Y for sampling. This value must be
                   greater than 0.0


    @param po_pMeanError Pointer to double that receives the mean error.

    @param po_pMaxError  Pointer to double that receives the maximum error.

    @param po_pScaleChangeMean Pointer to double that receives the mean scale change

    @param po_pScaleChangeMax Pointer to double that receives the max scale change

    @param pi_ScaleThreshold Value indicating the scale that will result in a stop
                             of study. Even if study is stopped, at least one
                             sample has been completely processed and thus
                             all stats are valid. The threshold is specified as
                             a change of scale from 1.0 (no scale change). This parameter
                             is optional and defaults to 1.0 meaning a factor of 2.0
                             will stop the process

    -----------------------------------------------------------------------------
*/
void HCPGCoordModel::StudyReversibilityPrecisionOver
(
    const HGF2DLiteExtent& pi_PrecisionArea,
    double                pi_Step,
    double*               po_pMeanError,
    double*               po_pMaxError,
    double*               po_pScaleChangeMean,
    double*               po_pScaleChangeMax,
    double                pi_ScaleTreshold
) const
    {
    // The extent of area must not be empty
    HPRECONDITION (pi_PrecisionArea.GetWidth () != 0.0);
    HPRECONDITION (pi_PrecisionArea.GetHeight () != 0.0);

    // The step may not be null nor negative
    HPRECONDITION (pi_Step > 0.0);

    // Recipient variables must be provided
    HPRECONDITION (po_pMeanError != NULL);
    HPRECONDITION (po_pMaxError != NULL);

    // Convert in temporary variables
    double TempX;
    double TempY;

    double MaxError = 0.0;
    double MaxScaleChange = 1.0;
    uint32_t ScaleNumSamples = 0;
    double ScaleChangeSum = 0.0;
    double StatSumX = 0.0;
    double StatSumY = 0.0;
    uint32_t StatNumSamples = 0;

    double TempX1;
    double TempY1;

    double CurrentX;
    double CurrentY;
    double PreviousX=0.0;
    double PreviousY=0.0;
    double PreviousConvertedX=0.0;
    double PreviousConvertedY=0.0;

    for (CurrentY = pi_PrecisionArea.GetYMin () ;
         CurrentY < pi_PrecisionArea.GetYMax () && (fabs (MaxScaleChange - 1.0) < pi_ScaleTreshold) ;
         CurrentY += pi_Step)
        {
        bool Initialized = false;
        for (CurrentX = pi_PrecisionArea.GetXMin () , PreviousX = CurrentX;
             CurrentX < pi_PrecisionArea.GetXMax () && (fabs (MaxScaleChange - 1.0) < pi_ScaleTreshold)  ;
             CurrentX += pi_Step)
            {
            // Convert one way
            ConvertDirect (CurrentX, CurrentY, &TempX, &TempY);

            // Compute scale change calculations
            // Scale calculations only occur if not first time (requires 2 points)
            //            if (PreviousY != CurrentY || PreviousX != CurrentX)
            if (Initialized)
                {
                double DistanceInverse = sqrt ((PreviousConvertedX - TempX) * (PreviousConvertedX - TempX) +
                                                (PreviousConvertedY - TempY) * (PreviousConvertedY - TempY));
                double DistanceDirect = sqrt ((PreviousX - CurrentX) * (PreviousX - CurrentX) +
                                               (PreviousY - CurrentY) * (PreviousY - CurrentY));

                double ScaleChange = DistanceInverse / DistanceDirect;

                ScaleChangeSum += ScaleChange;
                if (fabs (MaxScaleChange - 1.0) < fabs (ScaleChange - 1.0))
                    MaxScaleChange = ScaleChange;

                ScaleNumSamples++;
                }

            // Save previous conversion for next scale change calculations
            PreviousConvertedX = TempX;
            PreviousConvertedY = TempY;
            PreviousX = CurrentX;
            PreviousY = CurrentY;
            Initialized = true;

            // Convert back
            ConvertInverse (TempX, TempY, &TempX1, &TempY1);

            // Compute difference (drift)
            double DeltaX = fabs (CurrentX - TempX1);
            double DeltaY = fabs (CurrentY - TempY1);

            // Add deltas
            StatSumX += DeltaX;
            StatSumY += DeltaY;
            StatNumSamples++;

            MaxError = max (MaxError, max (DeltaX, DeltaY));
            }
        }

    // Compute precision results
    *po_pMaxError = MaxError;
    *po_pMeanError = (StatNumSamples > 0 ? (StatSumX + StatSumY) / (2 * StatNumSamples) : 0.0);
    *po_pScaleChangeMax = MaxScaleChange;
    *po_pScaleChangeMean = (ScaleNumSamples > 0 ? ScaleChangeSum / ScaleNumSamples : 1.0);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HCPGCoordModel::ConvertDirect
(
    double* pio_pXInOut,
    double* pio_pYInOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (pio_pXInOut != NULL);
    HPRECONDITION (pio_pYInOut != NULL);


#if (0)
    DPoint3d inCartesian;
    StatusInt   status = SUCCESS;
    StatusInt   stat1;
    StatusInt   stat2;
    StatusInt   stat3;


    inCartesian.x = *pio_pXInOut;
    inCartesian.y = *pio_pYInOut;
    inCartesian.z = 0.0;

    GeoPoint inLatLong;
    stat1 = m_SourceGEOCS->GetBaseGCS()->LatLongFromCartesian (inLatLong, inCartesian);

    GeoPoint outLatLong;
    stat2 = m_SourceGEOCS->GetBaseGCS()->LatLongFromLatLong(outLatLong, inLatLong, *m_DestinationGEOCS->GetBaseGCS());


    DPoint3d outCartesian;
    stat3 = m_DestinationGEOCS->GetBaseGCS()->CartesianFromLatLong(outCartesian, outLatLong);

    if (SUCCESS == status)
        {
        // Status returns hardest error found in the three error statuses
        // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
        if (SUCCESS != stat1)
            status = stat1;
        if ((SUCCESS != stat2) && ((SUCCESS == status) || (1 == status))) // If stat2 has error and status not already hard error
            {
            if (0 > stat2) // If stat2 is negative ... this is the one ...
                status = stat2;
            else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                status = (stat2 > status ? stat2 : status);
            }
        if ((SUCCESS != stat3) && ((SUCCESS == status) || (1 == status))) // If stat3 has error and status not already hard error
            {
            if (0 > stat3) // If stat3 is negative ... this is the one ...
                status = stat3;
            else  // Both are positive (status may be SUCCESS) we use the highest value
                status = (stat3 > status ? stat3 : status);
            }
        }
    


    *pio_pXInOut = outCartesian.x;
    *pio_pYInOut = outCartesian.y;
#else
    double XOut;
    double YOut;

    m_SourceGEOCS->Reproject(&XOut, &YOut, *pio_pXInOut, *pio_pYInOut, *m_DestinationGEOCS);
    *pio_pXInOut = XOut;
    *pio_pYInOut = YOut;

#endif
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HCPGCoordModel::ConvertDirect
(
    double  pi_XIn,
    double  pi_YIn,
    double* po_pXOut,
    double* po_pYOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    ConvertDirect (po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HCPGCoordModel::ConvertDirect
(
    double    pi_YIn,
    double    pi_XInStart,
    size_t    pi_NumLoc,
    double    pi_XInStep,
    double*   po_pXOut,
    double*   po_pYOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        ConvertDirect (X, pi_YIn, pCurrentX, pCurrentY);
        }
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
void HCPGCoordModel::ConvertInverse
(
    double* pio_pXInOut,
    double* pio_pYInOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (pio_pXInOut != NULL);
    HPRECONDITION (pio_pYInOut != NULL);

#if (0)


    DPoint3d inCartesian;
    StatusInt   status = SUCCESS;
    StatusInt   stat1;
    StatusInt   stat2;
    StatusInt   stat3;


    inCartesian.x = *pio_pXInOut;
    inCartesian.y = *pio_pYInOut;
    inCartesian.z = 0.0;

    GeoPoint inLatLong;
    stat1 = m_DestinationGEOCS->GetBaseGCS()->LatLongFromCartesian (inLatLong, inCartesian);

    GeoPoint outLatLong;
    stat2 = m_DestinationGEOCS->GetBaseGCS()->LatLongFromLatLong(outLatLong, inLatLong, *m_SourceGEOCS->GetBaseGCS());


    DPoint3d outCartesian;
    stat3 = m_SourceGEOCS->GetBaseGCS()->CartesianFromLatLong(outCartesian, outLatLong);

    if (SUCCESS == status)
        {
        // Status returns hardest error found in the three error statuses
        // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
        if (SUCCESS != stat1)
            status = stat1;
        if ((SUCCESS != stat2) && ((SUCCESS == status) || (1 == status))) // If stat2 has error and status not already hard error
            {
            if (0 > stat2) // If stat2 is negative ... this is the one ...
                status = stat2;
            else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                status = (stat2 > status ? stat2 : status);
            }
        if ((SUCCESS != stat3) && ((SUCCESS == status) || (1 == status))) // If stat3 has error and status not already hard error
            {
            if (0 > stat3) // If stat3 is negative ... this is the one ...
                status = stat3;
            else  // Both are positive (status may be SUCCESS) we use the highest value
                status = (stat3 > status ? stat3 : status);
            }
        }

    *pio_pXInOut = outCartesian.x;
    *pio_pYInOut = outCartesian.y;

#else                                                                                                                                               
    double XOut;
    double YOut;

    m_DestinationGEOCS->Reproject(&XOut, &YOut, *pio_pXInOut, *pio_pYInOut, *m_SourceGEOCS);

    *pio_pXInOut = XOut;
    *pio_pYInOut = YOut;

#endif

    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
void HCPGCoordModel::ConvertInverse
(
    double pi_XIn,
    double pi_YIn,
    double* po_pXOut,
    double* po_pYOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    ConvertInverse (po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
void HCPGCoordModel::ConvertInverse
(
    double    pi_YIn,
    double    pi_XInStart,
    size_t     pi_NumLoc,
    double    pi_XInStep,
    double*   po_pXOut,
    double*   po_pYOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        ConvertInverse (X, pi_YIn, pCurrentX, pCurrentY);
        }
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesLinearity () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesParallelism () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesShape () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesDirection () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HCPGCoordModel::CanBeRepresentedByAMatrix () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HCPGCoordModel::IsIdentity () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HCPGCoordModel::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HCPGCoordModel::GetStretchParams
(
    double* po_pScaleFactorX,
    double* po_pScaleFactorY,
    HGF2DDisplacement* po_pDisplacement
) const
    {
    HINVARIANTS;

    HPRECONDITION (po_pScaleFactorX != NULL);
    HPRECONDITION (po_pScaleFactorY != NULL);
    HPRECONDITION (po_pDisplacement != NULL);

    // Do not use!
    HASSERT (0);

    HFCPtr<HGF2DTransfoModel> pModel;

    pModel->GetStretchParams (po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HCPGCoordModel::GetMatrix () const
    {
    HINVARIANTS;

    // Should not be called
    HASSERT (0);

    HFCMatrix<3, 3> Matrix;

    return (Matrix);
    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HCPGCoordModel::Reverse ()
    {
    HINVARIANTS;

    // Swap GEOCSs
    IRasterBaseGcsPtr  TempGEOCS;
    TempGEOCS = m_DestinationGEOCS;
    m_DestinationGEOCS = m_SourceGEOCS;
    m_SourceGEOCS = TempGEOCS;

    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModel::Reverse ();
    }


//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HCPGCoordModel::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if it is identity
    if (pi_rModel.IsIdentity ())
        {
        // Model is identity ... return copy of self
        pResultModel = new HCPGCoordModel (*this);

        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf (pi_rModel);
        }

    return (pResultModel);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HCPGCoordModel::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return (new HCPGCoordModel (*this));
    }

//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HCPGCoordModel::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Type is not known ... build a complex
    // To do this we call the ancester ComposeYourself
    pResultModel = HGF2DTransfoModel::ComposeYourself (pi_rModel);

    return (pResultModel);
    }

//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HCPGCoordModel::Prepare ()
    {
    // Obtain conversion ratio for direct X to inverse X units
    }

//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HCPGCoordModel::Copy (const HCPGCoordModel& pi_rObj)
    {
    // Copy master data
    m_SourceGEOCS      = pi_rObj.m_SourceGEOCS;
    m_DestinationGEOCS = pi_rObj.m_DestinationGEOCS;
    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGCoordModel::CreateSimplifiedModel () const
    {
    HINVARIANTS;

    if (m_SourceGEOCS == m_DestinationGEOCS)
        return new HGF2DIdentity();

    // If we get here, no simplification is possible.
    return 0;
    }

//-----------------------------------------------------------------------------
// GetSourceGEOCS
// Returns the source projection
//-----------------------------------------------------------------------------
IRasterBaseGcsPtr HCPGCoordModel::GetSourceGEOCS() const
    {
    return m_SourceGEOCS;
    }

//-----------------------------------------------------------------------------
// GetDestinationGEOCS
// Returns the destination projection
//-----------------------------------------------------------------------------
IRasterBaseGcsPtr HCPGCoordModel::GetDestinationGEOCS() const
    {
    return m_DestinationGEOCS;
    }

#ifdef HVERIFYCONTRACT
void               HCPGCoordModel::ValidateInvariants() const
    {
    HASSERT(m_SourceGEOCS->IsValid());
    HASSERT(m_DestinationGEOCS->IsValid());
    }
#endif