//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DTransfoModel
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


// The class declaration must be the last include file.
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>   // special case include turn around
#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DUniverse.h>

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DTransfoModel::HGF2DTransfoModel()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HGF2DTransfoModel::HGF2DTransfoModel(const HGF2DTransfoModel& pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DTransfoModel::~HGF2DTransfoModel()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.
//-----------------------------------------------------------------------------
HGF2DTransfoModel& HGF2DTransfoModel::operator=(const HGF2DTransfoModel& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        }

    // Return reference to self
    return (*this);
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
bool HGF2DTransfoModel::IsConvertDirectThreadSafe() const   { return _IsConvertDirectThreadSafe();}
bool HGF2DTransfoModel::IsConvertInverseThreadSafe() const  { return _IsConvertInverseThreadSafe(); }

StatusInt HGF2DTransfoModel::ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const { return _ConvertDirect(pio_pXInOut, pio_pYInOut); }

StatusInt HGF2DTransfoModel::ConvertDirect(double  pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_aXOut, double* po_aYOut) const 
    {
    return _ConvertDirect(pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_aXOut, po_aYOut);
    }
StatusInt HGF2DTransfoModel::ConvertDirect(double  pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    return _ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
    }
StatusInt HGF2DTransfoModel::ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    return _ConvertDirect(pi_NumLoc, pio_aXInOut, pio_aYInOut);
    }

StatusInt HGF2DTransfoModel::ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const { return _ConvertInverse(pio_pXInOut, pio_pYInOut); }

StatusInt HGF2DTransfoModel::ConvertInverse(double  pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_aXOut, double* po_aYOut) const
    {
    return _ConvertInverse(pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_aXOut, po_aYOut);
    }
StatusInt HGF2DTransfoModel::ConvertInverse(double  pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    return _ConvertInverse(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
    }
StatusInt HGF2DTransfoModel::ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    return _ConvertInverse(pi_NumLoc, pio_aXInOut, pio_aYInOut);
    }

bool HGF2DTransfoModel::IsIdentity() const {return _IsIdentity();}
bool HGF2DTransfoModel::IsStretchable(double pi_AngleTolerance) const { return _IsStretchable(pi_AngleTolerance); }
void HGF2DTransfoModel::GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, HGF2DDisplacement* po_pDisplacement) const { return _GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement); }

bool               HGF2DTransfoModel::HasDomain() const { return _HasDomain(); }
HFCPtr<HGF2DShape> HGF2DTransfoModel::GetDirectDomain() const { return _GetDirectDomain(); }
HFCPtr<HGF2DShape> HGF2DTransfoModel::GetInverseDomain() const { return _GetInverseDomain(); }

HGF2DTransfoModel* HGF2DTransfoModel::Clone() const {return _Clone();}

HFCPtr<HGF2DTransfoModel> HGF2DTransfoModel::ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const { return _ComposeInverseWithDirectOf(pi_rModel); }
HFCPtr<HGF2DTransfoModel> HGF2DTransfoModel::ComposeInverseWithInverseOf(const HGF2DTransfoModel& pi_rModel) const { return _ComposeInverseWithInverseOf(pi_rModel); }

bool                      HGF2DTransfoModel::CanBeRepresentedByAMatrix() const { return _CanBeRepresentedByAMatrix(); }
HFCMatrix<3, 3>           HGF2DTransfoModel::GetMatrix() const { return _GetMatrix(); }

HFCPtr<HGF2DTransfoModel> HGF2DTransfoModel::CreateSimplifiedModel() const { return _CreateSimplifiedModel(); }

bool HGF2DTransfoModel::PreservesLinearity() const { return _PreservesLinearity(); }
bool HGF2DTransfoModel::PreservesParallelism() const { return _PreservesParallelism(); }
bool HGF2DTransfoModel::PreservesShape() const { return _PreservesShape(); }
bool HGF2DTransfoModel::PreservesDirection() const { return _PreservesDirection(); }

void HGF2DTransfoModel::Reverse() { _Reverse(); }

/** -----------------------------------------------------------------------------
This method reverses the current transformation model. The operation of
reversing implies that results produced by the ConvertDirect() method will
from now on be produced by ConvertInverse() and vice-versa. The different
parameters related to the model are modified. The units used for the direct
and inverse output channels are also swapped.

-----------------------------------------------------------------------------
*/
void HGF2DTransfoModel::_Reverse() 
    {
    Prepare();
    }

/** -----------------------------------------------------------------------------
    This method creates a new transformation model as a composition of the
    present model, and the given one. The returned model is always a new
    one. The returned model is the simplest possible, and can be of a completely
    different type from self, or the other given transformation model.
    The process of composition for this method binds the inverse channels of
    self with the direct channels of given transformation model, resulting in
    a new transformation model.

    @param pi_rModel IN A constant reference to a transformation model to compose
                     with self

    @return A smart pointer to a newly allocated transformation model resulting
            from composition.


    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel>  HGF2DTransfoModel::_ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // All models unknown ask other for composition
    return(CallComposeOf(pi_rModel));
    }


/** -----------------------------------------------------------------------------
    This method converts direct the given coordinates by the relation of
    the HGF2DTransfoModel. The transformation model really transforms
    distances, therefore, the methods must be capable of transforming the
    given input into distances. Since the numbers given are raw values,
    then the values are interpreted in the units set in the model at
    its construction. This therefore prevents requiring the convert method
    of performing a unit conversion, but lets the user check that the
    values refer to the proper units.

    @param pi_XIn A double containing a value that will be interpreted in the unit
                  of the Direct X dimension to transform.

    @param pi_YIn A double containing a value that will be interpreted in the unit
                  of the Direct Y dimension to transform.

    @param po_pXOut A pointer to double that receives the converted value expressed
                    in the units of the inverse X dimension units.

    @param po_pYOut A pointer to double that receives the converted value expressed
                    in the units of the inverse Y dimension units.


    @see ConvertInverse()
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::_ConvertDirect(double   pi_XIn,
                                           double   pi_YIn,
                                           double*  po_pXOut,
                                           double*  po_pYOut) const
    {
    // Return value vars must be provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Copy values
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return ConvertDirect (po_pXOut, po_pYOut);
    }


/** -----------------------------------------------------------------------------
    This method converts direct the given coordinates by the relation of the
    HGF2DTransfoModel. The transformation model really transforms distances,
    therefore, the methods must be capable of transforming the given input into
    distances. Since the numbers given are raw values, then the values are
    interpreted in the units set in the model at its construction. This therefore
    prevents requiring the convert method of performing a unit conversion,
    but lets the user check that the values refer to the proper units.
    This conversion method permits to convert many coordinates having the same input Y.

    @param pi_Yin An double indicating the Y dimension of coordinates to transform.

    @param pi_XInStart An double indicating the first X dimension coordinate of coordinates
                       to transform.

    @param pi_XInStep An double containing the X step between each coordinates for
                      batch conversion.

    @param pi_NumLoc A positive number indicating the number of coordinates that are
                     to be converted.

    @param pi_aXOut An array of double that receives the X coordinates of the converted
                    values. This array must have at least pi_NumLoc elements.

    @param pi_aYOut An array of double that receives the Y coordinates of the converted
                    values. This array must have at least pi_NumLoc elements.

    @see ConvertInverse()
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::_ConvertDirect(double    pi_YIn,
                                           double    pi_XInStart,
                                           size_t    pi_NumLoc,
                                           double    pi_XInStep,
                                           double*   po_aXOut,
                                           double*   po_aYOut) const
    {
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    StatusInt status = SUCCESS;

    uint32_t Index;
    double X;
    for (Index = 0L, X = pi_XInStart; Index < pi_NumLoc ; Index++, X+=pi_XInStep)
        {
        po_aXOut[Index] = X;
        po_aYOut[Index] = pi_YIn;

        StatusInt tempStatus = ConvertDirect (&(po_aXOut[Index]), &(po_aYOut[Index]));

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }

/** -----------------------------------------------------------------------------
    This method converts direct the given coordinates by the relation of the
    HGF2DTransfoModel. The transformation model really transforms distances,
    therefore, the methods must be capable of transforming the given input into
    distances. Since the numbers given are raw values, then the values are
    interpreted in the units set in the model at its construction. This therefore
    prevents requiring the convert method of performing a unit conversion,
    but lets the user check that the values refer to the proper units.
    This conversion method permits to convert many coordinates at the time.
    The coordinates do not need to have the same Y coordinate.

    @param pi_NumLoc A positive number indicating the number of coordinates that are
                     to be converted.

    @param pio_aXInOut An array of double that contains the X coordinates to convert and that 
                       receives the X coordinates of the converted
                       values. This array must have at least pi_NumLoc elements.

    @param pio_aYInOut An array of double that contains the Y coordinates to convert and that 
                       receives the Y coordinates of the converted
                       values. This array must have at least pi_NumLoc elements.
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::_ConvertDirect(size_t    pi_NumLoc,
                                           double*   pio_aXInOut,
                                           double*   pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    uint32_t Index;
    for (Index = 0L; Index < pi_NumLoc ; Index++)
        {
        StatusInt tempStatus = ConvertDirect (&(pio_aXInOut[Index]), &(pio_aYInOut[Index]));

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }


/** -----------------------------------------------------------------------------
    This method converts inverse the given coordinates by the relation of
    the HGF2DTransfoModel. The transformation model really transforms
    distances, therefore, the methods must be capable of transforming the
    given input into distances. Since the numbers given are raw values,
    then the values are interpreted in the units set in the model at
    its construction. This therefore prevents requiring the convert method
    of performing a unit conversion, but lets the user check that the
    values refer to the proper units.

    @param pi_XIn A double containing a value that will be interpreted in the unit
                  of the inverse X dimension to transform.

    @param pi_YIn A double containing a value that will be interpreted in the unit
                  of the inverse Y dimension to transform.

    @param po_pXOut A pointer to double that receives the converted value expressed
                    in the units of the direct X dimension units.

    @param po_pYOut A pointer to double that receives the converted value expressed
                    in the units of the direct Y dimension units.


    @see ConvertDirect()
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::_ConvertInverse(double  pi_XIn,
                                            double  pi_YIn,
                                            double* po_pXOut,
                                           double* po_pYOut) const
    {
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Copy values
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    // Use other method for transformations
    return ConvertInverse (po_pXOut, po_pYOut);
    }


/** -----------------------------------------------------------------------------
    This method converts direct the given coordinate

    @see ConvertInverse()
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::ConvertPosDirect(HGF2DPosition* pio_rpCoord) const
    {
    // Return value vars must be provided
    HPRECONDITION(pio_rpCoord != 0);

    double X = pio_rpCoord->GetX();
    double Y = pio_rpCoord->GetY();

    StatusInt status = ConvertDirect (&X, &Y);

    pio_rpCoord->SetX(X);
    pio_rpCoord->SetY(Y);

    return status;
    }


/** -----------------------------------------------------------------------------
    This method converts inverse the given coordinates by the relation of the
    HGF2DTransfoModel. The transformation model really transforms distances,
    therefore, the methods must be capable of transforming the given input into
    distances. Since the numbers given are raw values, then the values are
    interpreted in the units set in the model at its construction. This therefore
    prevents requiring the convert method of performing a unit conversion,
    but lets the user check that the values refer to the proper units.
    This conversion method permits to convert many coordinates having the same input Y.

    @param pi_Yin An double indicating the Y dimension of coordinates to transform.

    @param pi_XInStart An double indicating the first X dimension coordinate of coordinates
                       to transform.

    @param pi_XInStep An double containing the X step between each coordinates for
                      batch conversion.

    @param pi_NumLoc A positive number indicating the number of coordinates that are
                     to be converted.

    @param pi_aXOut An array of double that receives the X coordinates of the converted
                    values. This array must have at least pi_NumLoc elements.

    @param pi_aYOut An array of double that receives the Y coordinates of the converted
                    values. This array must have at least pi_NumLoc elements.

    @see ConvertDirect()
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::_ConvertInverse (double    pi_YIn,
                                             double    pi_XInStart,
                                             size_t    pi_NumLoc,
                                             double    pi_XInStep,
                                             double*   po_aXOut,
                                             double*   po_aYOut) const
    {
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);


    StatusInt status = SUCCESS;

    uint32_t Index;
    double X;

    for (Index = 0L, X = pi_XInStart; Index < pi_NumLoc; Index++, X+=pi_XInStep)
        {
        po_aXOut[Index] = X;
        po_aYOut[Index] = pi_YIn;

        StatusInt tempStatus = ConvertInverse (&(po_aXOut[Index]), &(po_aYOut[Index]));

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }
    return status;
    }

/** -----------------------------------------------------------------------------
    This method converts inverse the given coordinates by the relation of the
    HGF2DTransfoModel. The transformation model really transforms distances,
    therefore, the methods must be capable of transforming the given input into
    distances. Since the numbers given are raw values, then the values are
    interpreted in the units set in the model at its construction. This therefore
    prevents requiring the convert method of performing a unit conversion,
    but lets the user check that the values refer to the proper units.
    This conversion method permits to convert many coordinates at the time.
    The coordinates do not need to have the same Y coordinate.

    @param pi_NumLoc A positive number indicating the number of coordinates that are
                     to be converted.

    @param pio_aXInOut An array of double that contains the X coordinates to convert and that
                       receives the X coordinates of the converted
                       values. This array must have at least pi_NumLoc elements.

    @param pio_aYInOut An array of double that contains the Y coordinates to convert and that
                       receives the Y coordinates of the converted
                       values. This array must have at least pi_NumLoc elements.
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::_ConvertInverse(size_t    pi_NumLoc,
                                            double*   pio_aXInOut,
                                            double*   pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    uint32_t Index;
    for (Index = 0L; Index < pi_NumLoc ; Index++)
        {
        StatusInt tempStatus = ConvertInverse (&(pio_aXInOut[Index]), &(pio_aYInOut[Index]));

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }


/** -----------------------------------------------------------------------------
    This method converts direct the given coordinate

    @see ConvertInverse()
    -----------------------------------------------------------------------------
*/
StatusInt HGF2DTransfoModel::ConvertPosInverse(HGF2DPosition* pio_rpCoord) const
    {
    // Return value vars must be provided
    HPRECONDITION(pio_rpCoord != 0);

    double X = pio_rpCoord->GetX();
    double Y = pio_rpCoord->GetY();

    StatusInt status = ConvertInverse(&X, &Y);

    pio_rpCoord->SetX(X);
    pio_rpCoord->SetY(Y);
       
    return status;
    }

/** -----------------------------------------------------------------------------
    This method creates a new transformation model as a composition of the
    present model, and the given one. The returned model is always a new one.
    The returned model is the simplest possible, and can be of a completely
    different type from self, or the other given transformation model.
    The process of composition for this method binds the inverse channels of self
    with the inverse channels of given transformation model, resulting in a
    new transformation model.

    @param pi_rModel IN A constant reference to a transformation model to compose
                     with self

    @return A smart pointer to a newly allocated transformation model resulting
            from composition.

    @see Reverse()
    @see ComposeInverseWithDirectOf()
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel>  HGF2DTransfoModel::_ComposeInverseWithInverseOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Make a duplicate of given
    HAutoPtr<HGF2DTransfoModel> pNewModel(pi_rModel.Clone());

    // Reverse the model
    pNewModel->Reverse();

    // Compose
    HFCPtr<HGF2DTransfoModel> pResultModel = ComposeInverseWithDirectOf(*pNewModel);

    return (pResultModel);
    }


//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given as failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DTransfoModel::_ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // The model is of an unknown type --> create complex
    // Allocate new complex transformation model
    HFCPtr<HGF2DComplexTransfoModel> pMyComplex(new HGF2DComplexTransfoModel ());

    // Add transformation models
    pMyComplex->AddModel(pi_rModel);
    pMyComplex->AddModel(*this);

    return((HFCPtr<HGF2DTransfoModel>&)pMyComplex);
    }

/** -----------------------------------------------------------------------------
    This method permits to extract the stretch parameters at a specific location
    in the direct channel coordinate space. These stretch parameters
    are the translation and scaling components of the model. The user must call
    the IsStretchable() method in order to know if the stretch parameters are
    the only transformation components in the model. If this is not the case,
    the values returned are the best stretch parameters that can approximate
    the model at the given location.

    @param po_pScaleFactorX OUT Pointer to double that receives the scaling factor
                            component in the first dimension for the transformation
                            model.

    @param po_pScaleFactorY OUT Pointer to double that receives the scaling factor
                            component in the second dimension for the transformation
                            model.

    @param po_pDisplacement OUT Pointer to an HGF2DDisplacement object that receives
                            the translation component of the model.

    @param pi_XLocation IN X coordinate of the location in the direct channel coordinate
                           space on which to obtain or estimate the stretch parameters.

    @param pi_YLocation IN Y coordinate of the location in the direct channel coordinate
                           space on which to obtain or estimate the stretch parameters.

    @param pi_AreaSize IN OPTIONAL The size of the area upon which scale factors are
                           evaluated. If provided this size should be greater than 0.
                           If the value is not provided, a value will be estimated
                           based upon the value of coordinates. The default estimation
                           may not work properly when scale factors are great.

    @see IsStretchable()
    @see GetStretchParams()
    -----------------------------------------------------------------------------
*/
void HGF2DTransfoModel::GetStretchParamsAt(double*  po_pScaleFactorX,
                                           double*  po_pScaleFactorY,
                                           HGF2DDisplacement* po_pDisplacement,
                                           double   pi_XLocation,
                                           double   pi_YLocation,
                                           double   pi_AreaSize) const
    {
    // Recipient variables must be provided
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    if(PreservesParallelism())
        {

        GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
        }
    else
        {

        double NewX1;
        double NewY1;
        double NewX2;
        double NewY2;

        double LocalEpsilon = pi_AreaSize / 2.0;

        if (LocalEpsilon <= 0.0)
            {
            LocalEpsilon = 100 * HNumeric<double>::GLOBAL_EPSILON();

            LocalEpsilon = MAX(LocalEpsilon, MAX(HNumeric<double>::EPSILON_MULTIPLICATOR() * pi_XLocation,
                                                 HNumeric<double>::EPSILON_MULTIPLICATOR() * pi_YLocation));
            }

        // Convert location with delta Xs
        ConvertDirect(pi_XLocation - LocalEpsilon, pi_YLocation, &NewX1, &NewY1);
        ConvertDirect(pi_XLocation + LocalEpsilon, pi_YLocation, &NewX2, &NewY2);

        // Measure result distance
        double DistX = sqrt(((NewX2 - NewX1) * (NewX2 - NewX1)) + ((NewY2 - NewY1) * (NewY2 - NewY1)));

        // Compute ratio of distances
        *po_pScaleFactorX = (DistX / (2 * LocalEpsilon));

        // We need to put a sign to the scaling factor
        if (NewX2 < NewX1)
            *po_pScaleFactorX *= -1;

        // Convert location with delta Ys
        ConvertDirect(pi_XLocation, pi_YLocation - LocalEpsilon, &NewX1, &NewY1);
        ConvertDirect(pi_XLocation, pi_YLocation + LocalEpsilon, &NewX2, &NewY2);

        // Measure result distance
        double DistY = sqrt(((NewX2 - NewX1) * (NewX2 - NewX1)) + ((NewY2 - NewY1) * (NewY2 - NewY1)));

        // Compute ratio of distances
        *po_pScaleFactorY = (DistY / (2 * LocalEpsilon));

        if (NewY2 < NewY1)
            *po_pScaleFactorY *= -1;

        // Convert specified location exactly
        ConvertDirect(pi_XLocation, pi_YLocation, &NewX1, &NewY1);

        // Convert for unit differences

        double TransX = NewX1 - (pi_XLocation * (*po_pScaleFactorX));
        double TransY = NewY1 - (pi_YLocation * (*po_pScaleFactorY));
        po_pDisplacement->SetDeltaX(TransX);
        po_pDisplacement->SetDeltaY(TransY);
        }
    }

/** -----------------------------------------------------------------------------
    Studies the reversibility of the model over a region using the given step.
    Since mzGCoord models are notably un-reversible when region of
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

    @param po_pScaleChangeMax Pointer to double that receives the MAX scale change

    @param pi_ScaleThreshold Value indicating the scale that will result in a stop
                             of study. Even if study is stopped, at least one
                             sample has been completely proecessed and thus
                             all stats are valid. The threshold is specified as
                             a change of scale from 1.0 (no scale change). This parameter
                             is optional and defaults to 1.0 meaning a factor of 2.0
                             will stop the process

    -----------------------------------------------------------------------------
*/
void HGF2DTransfoModel::StudyReversibilityPrecisionOver
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
    HPRECONDITION (po_pMeanError != 0);
    HPRECONDITION (po_pMaxError != 0);

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

            MaxError = MAX (MaxError, MAX (DeltaX, DeltaY));
            }
        }

    // Compute precision results
    *po_pMaxError = MaxError;
    *po_pMeanError = (StatNumSamples > 0 ? (StatSumX + StatSumY) / (2 * StatNumSamples) : 0.0);
    *po_pScaleChangeMax = MaxScaleChange;
    *po_pScaleChangeMean = (ScaleNumSamples > 0 ? ScaleChangeSum / ScaleNumSamples : 1.0);

    }



/** -----------------------------------------------------------------------------
    Studies the reversibility of the model over a region using the given step.
    Since mzGCoord models are notably un-reversible when region of
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

    @param po_pScaleChangeMax Pointer to double that receives the MAX scale change

    @param pi_ScaleThreshold Value indicating the scale that will result in a stop
                             of study. Even if study is stopped, at least one
                             sample has been completely proecessed and thus
                             all stats are valid. The threshold is specified as
                             a change of scale from 1.0 (no scale change). This parameter
                             is optional and defaults to 1.0 meaning a factor of 2.0
                             will stop the process

    -----------------------------------------------------------------------------
*/
void HGF2DTransfoModel::GetEquivalenceToOver (const HGF2DTransfoModel& pi_rModel,
                                              const HGF2DLiteExtent& pi_PrecisionArea,
                                              double                pi_Step,
                                              double*               po_pMeanError,
                                              double*               po_pMaxError) const
    {
    // The extent of area must not be empty
    HPRECONDITION (pi_PrecisionArea.GetWidth () != 0.0);
    HPRECONDITION (pi_PrecisionArea.GetHeight () != 0.0);

    // The step may not be null nor negative
    HPRECONDITION (pi_Step > 0.0);

    // Recipient variables must be provided
    HPRECONDITION (po_pMeanError != 0);
    HPRECONDITION (po_pMaxError != 0);

    // Convert in temporary variables
    double TempX;
    double TempY;

    double MaxError = 0.0;

    double StatSumX = 0.0;
    double StatSumY = 0.0;
    uint32_t StatNumSamples = 0;

    double TempX1;
    double TempY1;

    double CurrentX;
    double CurrentY;


    for (CurrentY = pi_PrecisionArea.GetYMin () ;
         CurrentY < pi_PrecisionArea.GetYMax () ;
         CurrentY += pi_Step)
        {
        for (CurrentX = pi_PrecisionArea.GetXMin () ;
             CurrentX < pi_PrecisionArea.GetXMax () ;
             CurrentX += pi_Step)
            {
            // Convert using this
            ConvertDirect (CurrentX, CurrentY, &TempX, &TempY);

            // Convert using given
            pi_rModel.ConvertDirect (CurrentX, CurrentY, &TempX1, &TempY1);

            // Compute difference
            double DeltaX = fabs (CurrentX - TempX1);
            double DeltaY = fabs (CurrentY - TempY1);

            // Add deltas
            StatSumX += DeltaX;
            StatSumY += DeltaY;
            StatNumSamples++;

            MaxError = MAX (MaxError, MAX (DeltaX, DeltaY));
            }
        }

    // Compute precision results
    *po_pMaxError = MaxError;
    *po_pMeanError = (StatNumSamples > 0 ? (StatSumX + StatSumY) / (2 * StatNumSamples) : 0.0);
    }
/** -----------------------------------------------------------------------------
    @bsimethod                                         Alain Robert 2014/06
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DShape>  HGF2DTransfoModel::_GetDirectDomain() const
    {
    // Default implementation has no domain implying there is no limit
    return new HGF2DUniverse();
    }


/** -----------------------------------------------------------------------------
    @bsimethod                                         Alain Robert 2014/06
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DShape>  HGF2DTransfoModel::_GetInverseDomain() const
    {
    // Default implementation has no domain implying there is no limit
    return new HGF2DUniverse();
    }
