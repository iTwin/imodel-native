//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DSimilitude.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DSimilitude
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DDCTransfoModel.h>

#include <Imagepp/all/h/HGF2DSimilitude.h>


/** -----------------------------------------------------------------------------
    Default Constructor
    Initializes the model to no transformation with units for all channels
    set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DSimilitude::HGF2DSimilitude()
    : HGF2DTransfoModel(),
      m_Scale(1.0),
      m_XTranslation(0.0),
      m_YTranslation(0.0),
      m_Rotation(0.0)
    {
    Prepare();
    }


/** -----------------------------------------------------------------------------
    Constructor
    Initializes the model to given c0omponents. The units are extracted from
    the displacement and assigned to both direct and inverse channels.

    @param pi_rTranslation IN A constant reference to an HGF2DDisplacement object
                              containing the description of the translation
                              component to set into the similitude.

    @param pi_rRotation IN The rotation component to set into the similitude.

    @param pi_Scale IN The scaling factor. This scaling must be different than 0.0.

    @code
        HGF2DDisplacement   MyTranslation (10.0, 10.0);
        HGF2DSimilitude    MyModel (MyTranslation, 3.14159265, 2.0);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DSimilitude::HGF2DSimilitude(const HGF2DDisplacement& pi_rTranslation,
                                 double                   pi_Rotation,
                                 double                   pi_Scale)
    : HGF2DTransfoModel(),
    m_Rotation (pi_Rotation),
    m_XTranslation (pi_rTranslation.GetDeltaX()),
    m_YTranslation (pi_rTranslation.GetDeltaY()),
    m_Scale(pi_Scale)
    {
    // The scale may not be 0.0 ... it can be very small or very large
    // however
    HPRECONDITION(pi_Scale != 0.0);
    Prepare();
    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DSimilitude::HGF2DSimilitude(const HGF2DSimilitude& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Copy (pi_rObj);
    }


/** -----------------------------------------------------------------------------
    Constructor
    Initializes the model based on sets of control points. The units are estimated
    from the control points and assigned to both direct and inverse channels.

    Prerequisite: there must be at least GetMinimumNumberOfTiePoints() points
    in the list.

    @param pi_NumberOfPoints IN Number of points in the lists
    @param pi_pTiePoints     IN A constant pointer to an array of positions
                                giving the source (bad) and destination (good) points.

    -----------------------------------------------------------------------------
*/
HGF2DSimilitude::HGF2DSimilitude(unsigned short pi_NumberOfPoints,
                                 const double*  pi_pTiePoints)
    : HGF2DTransfoModel()
    {
    HPRECONDITION (pi_NumberOfPoints >= SIMILITUDE_MIN_NB_TIE_PTS );

    double Matrix[4][4];
    HGF2DDCTransfoModel::GetSimilitudeTransfoMatrixFromScaleAndTiePts(Matrix, pi_NumberOfPoints, pi_pTiePoints);

    SetByMatrixParameters(Matrix[0][3], Matrix[0][0], Matrix[0][1], Matrix[1][3], Matrix[1][0], Matrix[1][1]);
    }


//-----------------------------------------------------------------------------
//  SetByMatrixParameters
//  Sets the components of the affine by matrix parameters
//-----------------------------------------------------------------------------
void HGF2DSimilitude::SetByMatrixParameters(double pi_A0,
                                            double pi_A1,
                                            double pi_A2,
                                            double pi_B0,
                                            double pi_B1,
                                            double pi_B2)
    {
    // These are the conditions that prevent any scale factor of being 0, or
    // the anorthogonality of being equal to PI/2 or -PI/2
    HPRECONDITION((pi_A1 != 0.0) || (pi_A2 != 0.0));
    HPRECONDITION((pi_B1 != 0.0) || (pi_B2 != 0.0));
    HPRECONDITION((pi_A1 != 0.0) || (pi_B1 != 0.0));
    HPRECONDITION((pi_A2 != 0.0) || (pi_B2 != 0.0));

    // Set translation components
    m_XTranslation = pi_A0;
    m_YTranslation = pi_B0;

    // Compute rotation
    // if A1 is zero, then the cos of rotation is 0 therefore the angle is either
    // PI/2 or -PI/2. Any one will do, but the sign of the scalings will be the reverse
    // for each possible values of rotation
    if (pi_A1 != 0.0)
        m_Rotation = atan(pi_B1/pi_A1);
    else
        m_Rotation = PI/2;


    // Compute X scale
    double     MyCos = cos(m_Rotation);

    // Check that cos is not 0. This implies that the new rotation is PI/2 or -PI/2
    // which is valid, but renders the first equation unsolvable. The second resolves
    // the scale in that case. Normaly a 0 cos implies an A1 equal to 0, but insures that
    // B1 is not equal to 0.
    if (!HDOUBLE_EQUAL_EPSILON(MyCos, 0.0) && pi_A1 != 0.0)
        m_Scale = pi_A1 / MyCos;
    else
        m_Scale = pi_B1 / sin(m_Rotation);


    // Compute prepared settings
    Prepare ();
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DSimilitude::~HGF2DSimilitude()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DSimilitude& HGF2DSimilitude::operator=(const HGF2DSimilitude& pi_rObj)
    {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DSimilitude::IsConvertDirectThreadSafe() const 
    { 
    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DSimilitude::IsConvertInverseThreadSafe() const 
    { 
    return true; 
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertDirect(double* pio_pXInOut,
                                         double* pio_pYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    // Transform coordinates
    *pio_pXInOut = (X * m_PreparedDirectA1) + (Y * -m_PreparedDirectB1) +
                   m_XTranslationPrime;
    *pio_pYInOut = (X * m_PreparedDirectB2) + (Y * m_PreparedDirectA2) +
                   m_YTranslationPrime;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertDirect (double    pi_YIn,
                                          double    pi_XInStart,
                                          size_t    pi_NumLoc,
                                          double    pi_XInStep,
                                          double*   po_aXOut,
                                          double*   po_aYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    double ByProdY1 = pi_YIn * -m_PreparedDirectB1 + m_XTranslationPrime;
    double ByProdY2 = pi_YIn * m_PreparedDirectA2 + m_YTranslationPrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; Index++, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = (X * m_PreparedDirectA1) + ByProdY1;
        *pCurrentY = (X * m_PreparedDirectB2) + ByProdY2;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertDirect (size_t    pi_NumLoc,
                                          double*   pio_aXInOut,
                                          double*   pio_aYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    double X;
    double Y;

    double ByProdY1;
    double ByProdY2;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        ByProdY1 = Y * -m_PreparedDirectB1 + m_XTranslationPrime;
        ByProdY2 = Y * m_PreparedDirectA2 + m_YTranslationPrime;

        pio_aXInOut[i] = (X * m_PreparedDirectA1) + ByProdY1;
        pio_aYInOut[i] = (X * m_PreparedDirectB2) + ByProdY2;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertDirect(double   pi_XIn,
                                         double   pi_YIn,
                                         double*  po_pXOut,
                                         double*  po_pYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Transform coordinates
    *po_pXOut = (pi_XIn * m_PreparedDirectA1) + (pi_YIn * -m_PreparedDirectB1) +
                m_XTranslationPrime;
    *po_pYOut = (pi_XIn * m_PreparedDirectB2) + (pi_YIn * m_PreparedDirectA2) +
                m_YTranslationPrime;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertInverse(double* pio_pXInOut,
                                          double* pio_pYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    // Transform coordinates
    *pio_pXInOut = (X * m_PreparedInverseA1) + (Y * -m_PreparedInverseB1) +
                   m_XTranslationInversePrime;
    *pio_pYInOut = (X * m_PreparedInverseB2) + (Y * m_PreparedInverseA2) +
                   m_YTranslationInversePrime;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertInverse (double    pi_YIn,
                                           double    pi_XInStart,
                                           size_t    pi_NumLoc,
                                           double    pi_XInStep,
                                           double*   po_aXOut,
                                           double*   po_aYOut) const
    {

    // Make sure that recipient variables are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
    double X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    double ByProdY1 = pi_YIn * -m_PreparedInverseB1 + m_XTranslationInversePrime;
    double ByProdY2 = pi_YIn * m_PreparedInverseA2 + m_YTranslationInversePrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = (X * m_PreparedInverseA1) + ByProdY1;
        *pCurrentY = (X * m_PreparedInverseB2) + ByProdY2;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertInverse (size_t    pi_NumLoc,
                                           double*   pio_aXInOut,
                                           double*   pio_aYInOut) const
    {

    // Make sure that recipient variables are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    double X;
    double Y;

    double ByProdY1;
    double ByProdY2;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        ByProdY1 = Y * -m_PreparedInverseB1 + m_XTranslationInversePrime;
        ByProdY2 = Y * m_PreparedInverseA2 + m_YTranslationInversePrime;

        pio_aXInOut[i] = (X * m_PreparedInverseA1) + ByProdY1;
        pio_aYInOut[i] = (X * m_PreparedInverseB2) + ByProdY2;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DSimilitude::ConvertInverse(double  pi_XIn,
                                          double  pi_YIn,
                                          double* po_pXOut,
                                          double* po_pYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    *po_pXOut = (pi_XIn * m_PreparedInverseA1) + (pi_YIn * -m_PreparedInverseB1) +
                m_XTranslationInversePrime;
    *po_pYOut = (pi_XIn * m_PreparedInverseB2) + (pi_YIn * m_PreparedInverseA2) +
                m_YTranslationInversePrime;

    return SUCCESS;
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DSimilitude::PreservesLinearity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DSimilitude::PreservesParallelism() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DSimilitude::PreservesShape() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DSimilitude::PreservesDirection() const
    {
    // Direction is preserved only if there is no rotation implied
    // Note that no tolerance is applied since exact equality to 0.0 is
    // needed
    return (m_Rotation == 0.0);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DSimilitude::CanBeRepresentedByAMatrix() const
    {
    return true;
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DSimilitude::IsIdentity () const
    {
    return ((m_Rotation == 0.0) &&
            (m_XTranslation == 0.0) &&
            (m_YTranslation == 0.0) &&
            (m_Scale == 1.0));
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DSimilitude::IsStretchable (double pi_AngleTolerance) const
    {
    return (HDOUBLE_EQUAL(m_Rotation, 0.0, pi_AngleTolerance));
    }



//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DSimilitude::GetStretchParams (double* po_pScaleFactorX,
                                        double* po_pScaleFactorY,
                                        HGF2DDisplacement* po_pDisplacement) const
    {
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);

    // Extract stretch parameters
    *po_pScaleFactorX = GetScaling();
    *po_pScaleFactorY = GetScaling();

    *po_pDisplacement = GetTranslation();
    }

/** -----------------------------------------------------------------------------
    This method sets the translation component of the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing
                              the new translation component of the model.

    @code
        HGF2DDisplacement  Translation (10.2, 20.0);

        MyModel.SetTranslation  (Translation);
    @end

    @see GetTranslation()
    @see AddTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DSimilitude::SetTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    // Set translation
    m_XTranslation = pi_rTranslation.GetDeltaX();
    m_YTranslation = pi_rTranslation.GetDeltaY();

    // Compute prepared settings
    Prepare ();
    }

/** -----------------------------------------------------------------------------
    This method adds the specified translation component to the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing the
                              translation component to add to the model.

    @code
        HGF2Ddisplacement    Translation (10.2, 30.2);

        MyModel.AddTranslation  (Translation);
    @end

    @see GetTranslation()
    @see SetTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DSimilitude::AddTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    // Set translation
    m_XTranslation += pi_rTranslation.GetDeltaX();
    m_YTranslation += pi_rTranslation.GetDeltaY();

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method sets the rotation component of the model.

    @param pi_Angle The angle of the new rotation component of the model.

    @code
        MyModel.SetRotation (13.2);
    @end

    @see GetRotation()
    @see SetRotation()
    -----------------------------------------------------------------------------
*/
void HGF2DSimilitude::SetRotation (double pi_Angle)
    {
    // Set rotation
    m_Rotation = pi_Angle;

    // Compute prepared settings
    Prepare ();

    }

/** -----------------------------------------------------------------------------
    This method adds a rotation around a specified center of rotation to the
    current components of the model.
    The center is expressed as raw numbers that will be
    interpreted in the X and Y units of the direct channels of the
    transformation model. If omitted, then the origin is used.
    Remember that when specifying a center different than the origin,
    a translation component is also added to the model.

    @param pi_Angle IN The angle of the rotation component to add to the model.

    @param pi_XCenter IN The X value of the center of the rotation to add,
                            that will be interpreted in the X direct channel units.

    @param pi_YCenter IN The Y value of the center of the rotation to add,
                            that will be interpreted in the Y direct channel units.

    @code
        HGF2DSimilitude MyModel;
        double        MyRotationAngle = 13.5;

        MyModel.AddRotation (MyRotationAngle, 10.0, 10.0);
    @end

    @see GetRotation()
    @see SetRotation()
    -----------------------------------------------------------------------------
*/
void HGF2DSimilitude::AddRotation (double         pi_Angle,
                                   double         pi_XCenter,
                                   double         pi_YCenter)
    {
    // Add rotation component of the added rotation
    m_Rotation += pi_Angle;

    // Intermediate results
    double     MySin = sin(pi_Angle);
    double     MyCos = cos(pi_Angle);

    // Calculate new translation
    double TransX = ((m_YTranslation * MySin) +
                     (m_XTranslation * MyCos) +
                     (pi_XCenter + pi_YCenter * MySin - pi_XCenter * MyCos));
    double TransY = ((m_YTranslation * MyCos) -
                     (m_XTranslation * MySin) +
                     (pi_YCenter - pi_YCenter * MyCos - pi_XCenter * MySin));

    // Set result translation
    m_XTranslation = TransX;
    m_YTranslation = TransY;


    Prepare();
    }



/** -----------------------------------------------------------------------------
    This method sets scaling component of the model.

    @param pi_ScaleFactor IN The scaling factor of the scaling component of the model.
                             This factor must be different than 0.0

    @code
        HGF2DSimilitude   MyModel;
        MyModel.SetScaling (34.5);
    @end

    @see GetScaling()
    @see AddScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DSimilitude::SetScaling (double pi_Scale)
    {
    HPRECONDITION(pi_Scale != 0.0);

    // Set scale
    m_Scale = pi_Scale;

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method adds a scaling around a specified center of scaling to the current
    components of the model. The center is expressed as raw numbers that will be
    interpreted in the X and Y units of the direct channels of the transformation model.
    If omitted, then the origin is used.
    Remember that when specifying a center different than the origin,
    a translation component is also added to the model.

    @param pi_ScaleFactor IN The scaling factor of the scaling component to add
                             to the model. This factor must be different from 0.0.

    @param pi_XCenter IN The X value of the center of the scaling to add,
                            that will be interpreted in the X direct channel units.

    @param pi_YCenter IN The Y value of the center of the scaling to add,
                            that will be interpreted in the Y direct channel units.

    @code
        HGF2DSimilitude MyModel;
        MyModel.AddScaling (34.5, 2.0, 3.0);
    @end

    @see GetScaling()
    @see SetScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DSimilitude::AddScaling (double pi_Scale,
                                  double pi_XCenter,
                                  double pi_YCenter)
    {
    HPRECONDITION(pi_Scale != 0.0);

    // Add scaling component
    m_Scale *= pi_Scale;

    // Add translation component
    double TransX = ((m_XTranslation * pi_Scale) +
                          (pi_XCenter + pi_YCenter * pi_Scale - pi_XCenter * pi_Scale));
    double TransY = ((m_YTranslation * pi_Scale) +
                          (pi_YCenter - pi_YCenter * pi_Scale - pi_XCenter * pi_Scale));

    m_XTranslation = TransX;
    m_YTranslation = TransY;

    Prepare();
    }





//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transfomation mdoel
//-----------------------------------------------------------------------------
void    HGF2DSimilitude::Reverse()
    {
    // Reverse transformation parameters
    m_XTranslation = m_XTranslationInverse;
    m_YTranslation = m_YTranslationInverse;
    m_Scale = 1 / m_Scale;
    m_Rotation = -m_Rotation;

    // Invoque reversing of ancester
    HGF2DTransfoModel::Reverse();

    // Prepare
    Prepare();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DSimilitude::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID  TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY, TRANSLATION and SIMILITUDE
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DSimilitude(*this);
        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be similitude
        HAutoPtr<HGF2DSimilitude> pMySimi(new HGF2DSimilitude());

        // Cast model as a translation
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        // Compute composed translation
        double TransX = m_XTranslation +
                        pModelPrime->GetTranslation().GetDeltaX();
        double TransY = m_YTranslation +
                        pModelPrime->GetTranslation().GetDeltaY();

        // Set parameters
        pMySimi->SetTranslation(HGF2DDisplacement(TransX, TransY));
        pMySimi->SetRotation (m_Rotation);
        pMySimi->SetScaling (m_Scale);

        // Assign to smart pointer
        pResultModel = pMySimi.release();
        }
    else if (TheModelType == HGF2DSimilitude::CLASS_ID)
        {
        // We have two similitude models ... compose a similitude
        // Allocate
        HAutoPtr<HGF2DSimilitude> pMySimi(new HGF2DSimilitude());

        // Cast model as a similitude
        const HGF2DSimilitude* pModelPrime = static_cast<const HGF2DSimilitude*>(&pi_rModel);

        double     MySin = sin(pModelPrime->GetRotation());
        double     MyCos = cos(pModelPrime->GetRotation());

        // Compute composed translation
        double TransX = (-(m_YTranslation *
                           pModelPrime->GetScaling() *
                           MySin) +
                          (m_XTranslation *
                           pModelPrime->GetScaling() *
                           MyCos) +
                          pModelPrime->GetTranslation().GetDeltaX());
        double TransY = ((m_YTranslation *
                          pModelPrime->GetScaling() *
                          MyCos) +
                         (m_XTranslation *
                          pModelPrime->GetScaling() *
                          MySin) +
                         pModelPrime->GetTranslation().GetDeltaY());

        // Set parameters
        pMySimi->SetTranslation(HGF2DDisplacement(TransX, TransY));
        pMySimi->SetRotation (m_Rotation + pModelPrime->GetRotation());
        pMySimi->SetScaling (m_Scale * pModelPrime->GetScaling());

        // Assign to smart pointer
        pResultModel = pMySimi.release();
        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf(pi_rModel);
        }

    return (pResultModel);
    }




//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
    HGF2DTransfoModel* HGF2DSimilitude::Clone () const
    {
    // Allocate object as copy and return
    return (new HGF2DSimilitude (*this));
    }



//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DSimilitude::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY, TRANSLATION, and SIMILITUDE, but similitude
    // would have processed itself so not processed.
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DSimilitude(*this);
        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be similitude
        HAutoPtr<HGF2DSimilitude> pMySimi(new HGF2DSimilitude());

        // Cast model as a similitude
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        double     MyScaledSin = m_Scale * sin(m_Rotation);
        double     MyScaledCos = m_Scale * cos(m_Rotation);

        // Compute composed translation
        double TransX = pModelPrime->GetTranslation().GetDeltaX() * MyScaledCos -
                         pModelPrime->GetTranslation().GetDeltaY() * MyScaledSin +
                         m_XTranslation;
        double TransY = pModelPrime->GetTranslation().GetDeltaY() * MyScaledCos +
                         pModelPrime->GetTranslation().GetDeltaX() * MyScaledSin +
                         m_YTranslation;

        // Set parameters
        pMySimi->m_Scale = m_Scale;
        pMySimi->m_Rotation = m_Rotation;
        pMySimi->SetTranslation(HGF2DDisplacement(TransX, TransY));

        // Assign to smart pointer
        pResultModel = pMySimi.release();
        }
    else
        {
        // Type is not known ... build a complex
        // To do this we call the ancester ComposeYourself
        pResultModel = HGF2DTransfoModel::ComposeYourself (pi_rModel);
        }

    return (pResultModel);


    }


//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HGF2DSimilitude::Prepare ()
    {

    double     MySin = sin(m_Rotation);
    double     MyCos = cos(m_Rotation);
    double     MyNegSin = sin(-m_Rotation);
    double     MyNegCos = cos(-m_Rotation);

    // Prepare transformation factor for direct and inverse transformation
    // GetRotation() is not used because the compiler refused to inline it
    m_PreparedDirectA1 = MyCos * m_Scale;
    m_PreparedDirectB1 = MySin * m_Scale;
    m_PreparedDirectA2 = MyCos * m_Scale;
    m_PreparedDirectB2 = MySin * m_Scale;
    m_PreparedInverseA1 = (MyNegCos / m_Scale);
    m_PreparedInverseB1 = (MyNegSin / m_Scale);
    m_PreparedInverseA2 = (MyNegCos / m_Scale);
    m_PreparedInverseB2 = (MyNegSin / m_Scale);

    // Convert direct translation to inverse translation (negative value)
    // GetTranslation() is not used since compiler refused to inline it
    m_XTranslationInverse = ((-m_YTranslation * MySin) - (m_XTranslation * MyCos)) / m_Scale;
    m_YTranslationInverse = ((-m_YTranslation * MyCos) + (m_XTranslation * MySin)) / m_Scale;


    // Set prime translations
    m_XTranslationInversePrime = m_XTranslationInverse;
    m_YTranslationInversePrime = m_YTranslationInverse;

    m_XTranslationPrime = m_XTranslation;
    m_YTranslationPrime = m_YTranslation;
    }



//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DSimilitude::Copy(const HGF2DSimilitude& pi_rObj)
    {
    // Copy similitude parameters
    m_Rotation = pi_rObj.m_Rotation;
    m_XTranslation = pi_rObj.m_XTranslation;
    m_YTranslation = pi_rObj.m_YTranslation;
    m_Scale = pi_rObj.m_Scale;

    // Copy prepared items
    m_PreparedDirectA1 = pi_rObj.m_PreparedDirectA1;
    m_PreparedDirectB1 = pi_rObj.m_PreparedDirectB1;
    m_PreparedInverseA1 = pi_rObj.m_PreparedInverseA1;
    m_PreparedInverseB1 = pi_rObj.m_PreparedInverseB1;
    m_PreparedDirectA2 = pi_rObj.m_PreparedDirectA2;
    m_PreparedDirectB2 = pi_rObj.m_PreparedDirectB2;
    m_PreparedInverseA2 = pi_rObj.m_PreparedInverseA2;
    m_PreparedInverseB2 = pi_rObj.m_PreparedInverseB2;
    m_XTranslationInverse = pi_rObj.m_XTranslationInverse;
    m_YTranslationInverse = pi_rObj.m_YTranslationInverse;
    m_XTranslationInversePrime = pi_rObj.m_XTranslationInversePrime;
    m_YTranslationInversePrime = pi_rObj.m_YTranslationInversePrime;
    m_XTranslationPrime = pi_rObj.m_XTranslationPrime;
    m_YTranslationPrime = pi_rObj.m_YTranslationPrime;
    }


//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DSimilitude::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    double MySin = sin(m_Rotation);
    double MyCos = cos(m_Rotation);

    ReturnedMatrix[0][2] = m_XTranslation;
    ReturnedMatrix[1][2] = m_YTranslation;
    ReturnedMatrix[0][0] = m_Scale * MyCos;
    ReturnedMatrix[0][1] = -m_Scale * MySin;
    ReturnedMatrix[1][0] = m_Scale * MySin;
    ReturnedMatrix[1][1] = m_Scale * MyCos;
    ReturnedMatrix[2][0] = 0.0;
    ReturnedMatrix[2][1] = 0.0;
    ReturnedMatrix[2][2] = 1.0;

    return(ReturnedMatrix);
    }


//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DSimilitude::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    double MySin = sin(m_Rotation);
    double MyCos = cos(m_Rotation);

    po_rRecipient[0][2] = m_XTranslation;
    po_rRecipient[1][2] = m_YTranslation;
    po_rRecipient[0][0] = m_Scale * MyCos;
    po_rRecipient[0][1] = -m_Scale * MySin;
    po_rRecipient[1][0] = m_Scale * MySin;
    po_rRecipient[1][1] = m_Scale * MyCos;
    po_rRecipient[2][0] = 0.0;
    po_rRecipient[2][1] = 0.0;
    po_rRecipient[2][2] = 1.0;

    return(po_rRecipient);
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DSimilitude::CreateSimplifiedModel() const
    {
    if (m_Rotation == 0.0)
        {
        if (m_Scale == 1.0)
            {
            if ((m_XTranslation == 0.0) &&
                (m_YTranslation == 0.0))
                {
                return new HGF2DIdentity();
                }
            else
                {
                return new HGF2DTranslation(GetTranslation());
                }
            }
        else
            {
            // Stretch is faster than keeping a similitude when
            // there is no rotation...
            return new HGF2DStretch(GetTranslation(), m_Scale, m_Scale);
            }
        }

    // If we get here, no simplification is possible.
    return 0;
    }
