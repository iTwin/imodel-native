//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DHelmert.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DHelmert
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DDCTransfoModel.h>

#include <Imagepp/all/h/HGF2DHelmert.h>


/** -----------------------------------------------------------------------------
    Default Constructor
    Initializes the model to no transformation with units for all channels
    set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DHelmert::HGF2DHelmert()
    : HGF2DTransfoModel(),
      m_Rotation(0.0),
      m_XTranslation(0.0),
      m_YTranslation(0.0)

    {
    Prepare();
    }


/** -----------------------------------------------------------------------------
    Constructor
    Initializes the model to given components. The units are extracted from
    the displacement and assigned to both direct and inverse channels.

    @param pi_rTranslation IN A constant reference to an HGF2DDisplacement object
                              containing the description of the translation
                              component to set into the Helmert.

    @param pi_rRotation IN The rotation component to set into the Helmert.

    @code
        double              MyRotation = 3.14159265;
        HGF2DDisplacement   MyTranslation (10, 10);
        HGF2DHelmert    MyModel (MyTranslation, MyRotation, 2.0);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DHelmert::HGF2DHelmert(const HGF2DDisplacement& pi_rTranslation,
                           double                  pi_rRotation)
    : HGF2DTransfoModel(),
    m_Rotation (pi_rRotation),
    m_XTranslation (pi_rTranslation.GetDeltaX()),
    m_YTranslation (pi_rTranslation.GetDeltaY())
    {
    Prepare();
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
HGF2DHelmert::HGF2DHelmert(unsigned short pi_NumberOfPoints,
                           const double* pi_pTiePoints)
    : HGF2DTransfoModel()
    {
    HPRECONDITION (pi_NumberOfPoints >= HELMERT_MIN_NB_TIE_PTS );

    double Matrix[4][4];
    HGF2DDCTransfoModel::GetHelmertTransfoMatrixFromScaleAndTiePts(Matrix, pi_NumberOfPoints, pi_pTiePoints);

    SetByMatrixParameters(Matrix[0][3], Matrix[0][0], Matrix[0][1], Matrix[1][3], Matrix[1][0], Matrix[1][1]);

    }


//-----------------------------------------------------------------------------
//  SetByMatrixParameters
//  Sets the components of the affine by matrix parameters
//-----------------------------------------------------------------------------
void HGF2DHelmert::SetByMatrixParameters(double pi_A0,
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



    // Compute prepared settings
    Prepare ();
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DHelmert::HGF2DHelmert(const HGF2DHelmert& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Copy (pi_rObj);
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DHelmert::~HGF2DHelmert()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DHelmert& HGF2DHelmert::operator=(const HGF2DHelmert& pi_rObj)
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
bool HGF2DHelmert::IsConvertDirectThreadSafe() const 
    { 
    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DHelmert::IsConvertInverseThreadSafe() const 
    { 
    return true; 
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DHelmert::ConvertDirect(double* pio_pXInOut,
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
StatusInt HGF2DHelmert::ConvertDirect (double    pi_YIn,
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
StatusInt HGF2DHelmert::ConvertDirect (size_t    pi_NumLoc,
                                       double*   pio_aXInOut,
                                       double*   pio_aYInOut) const
    {
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
StatusInt HGF2DHelmert::ConvertDirect(double   pi_XIn,
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
StatusInt HGF2DHelmert::ConvertInverse(double* pio_pXInOut,
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
StatusInt HGF2DHelmert::ConvertInverse (double    pi_YIn,
                                        double    pi_XInStart,
                                        size_t     pi_NumLoc,
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
StatusInt HGF2DHelmert::ConvertInverse (size_t     pi_NumLoc,
                                        double*    pio_aXInOut,
                                        double*    pio_aYInOut) const
    {
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
StatusInt HGF2DHelmert::ConvertInverse(double  pi_XIn,
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
bool   HGF2DHelmert::PreservesLinearity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DHelmert::PreservesParallelism() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DHelmert::PreservesShape() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DHelmert::PreservesDirection() const
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
bool HGF2DHelmert::CanBeRepresentedByAMatrix() const
    {
    return true;
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DHelmert::IsIdentity () const
    {
    return ((m_Rotation == 0.0) &&
            (m_XTranslation == 0.0) &&
            (m_YTranslation == 0.0));
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DHelmert::IsStretchable (double pi_AngleTolerance) const
    {
    return (HDOUBLE_EQUAL(m_Rotation, 0.0, pi_AngleTolerance));
    }



//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DHelmert::GetStretchParams (double* po_pScaleFactorX,
                                     double* po_pScaleFactorY,
                                     HGF2DDisplacement* po_pDisplacement) const
    {
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);

    // Extract stretch parameters
    *po_pScaleFactorX = 1.0;
    *po_pScaleFactorY = 1.0;

    *po_pDisplacement = GetTranslation();
    }

/** -----------------------------------------------------------------------------
    This method sets the translation component of the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing
                              the new translation component of the model.

    @code
        HGF2DHelmert    MyModel;
        HGF2DDisplacement  Translation (10.2, 30.0);

        MyModel.SetTranslation  (Translation);
    @end

    @see GetTranslation()
    @see AddTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DHelmert::SetTranslation (const HGF2DDisplacement& pi_rTranslation)
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
        HGF2DHelmert    MyModel;
        HGF2Ddisplacement    Translation (10.2, 20.3);

        MyModel.AddTranslation  (Translation);
    @end

    @see GetTranslation()
    @see SetTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DHelmert::AddTranslation (const HGF2DDisplacement& pi_rTranslation)
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
        HGF2DHelmert    MyModel;
        double          MyRotationAngle = 13.5;

        MyModel.SetRotation (MyRotationAngle);
    @end

    @see GetRotation()
    @see SetRotation()
    -----------------------------------------------------------------------------
*/
void HGF2DHelmert::SetRotation (double pi_Angle)
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
        HGF2DHelmert MyModel;
        double       MyRotationAngle = 13.5 // radians

        MyModel.AddRotation (MyRotationAngle, 10.0, 10.0);
    @end

    @see GetRotation()
    @see SetRotation()
    -----------------------------------------------------------------------------
*/

void HGF2DHelmert::AddRotation (double pi_Angle,
                                double pi_rXCenter,
                                double pi_rYCenter)
    {
    // Add rotation component of the added rotation
    m_Rotation += pi_Angle;

    // Intermediate results
    double     MySin = sin(pi_Angle);
    double     MyCos = cos(pi_Angle);

    // Calculate new translation
    double TransX = ((m_YTranslation * MySin) +
                     (m_XTranslation * MyCos) +
                     (pi_rXCenter + pi_rYCenter * MySin - pi_rXCenter * MyCos));
     double TransY = ((m_YTranslation * MyCos) -
                      (m_XTranslation * MySin) +
                      (pi_rYCenter - pi_rYCenter * MyCos - pi_rXCenter * MySin));

    // Set result translation
    m_XTranslation = TransX;
    m_YTranslation = TransY;


    Prepare();
    }






//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transfomation mdoel
//-----------------------------------------------------------------------------
void    HGF2DHelmert::Reverse()
    {
    // Reverse transformation parameters
    m_XTranslation = m_XTranslationInverse;
    m_YTranslation = m_YTranslationInverse;
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
HFCPtr<HGF2DTransfoModel>  HGF2DHelmert::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID  TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY, TRANSLATION and Helmert
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DHelmert(*this);
        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be Helmert
        HAutoPtr<HGF2DHelmert> pMySimi(new HGF2DHelmert());

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

        // Assign to smart pointer
        pResultModel = pMySimi.release();
        }
    else if (TheModelType == HGF2DHelmert::CLASS_ID)
        {
        // We have two Helmert models ... compose a Helmert
        // Allocate
        HAutoPtr<HGF2DHelmert> pMySimi(new HGF2DHelmert());

        // Cast model as a Helmert
        const HGF2DHelmert* pModelPrime = static_cast<const HGF2DHelmert*>(&pi_rModel);

        double     MySin = sin(pModelPrime->GetRotation());
        double     MyCos = cos(pModelPrime->GetRotation());

        // Compute composed translation
        double TransX = (-(m_YTranslation *
                            MySin) +
                          (m_XTranslation *
                           MyCos) +
                          pModelPrime->GetTranslation().GetDeltaX());
        double TransY = ((m_YTranslation *
                          MyCos) +
                         (m_XTranslation *
                          MySin) +
                         pModelPrime->GetTranslation().GetDeltaY());

        // Set parameters
        pMySimi->SetTranslation(HGF2DDisplacement(TransX, TransY));
        pMySimi->SetRotation (m_Rotation + pModelPrime->GetRotation());

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
HGF2DTransfoModel* HGF2DHelmert::Clone () const
    {
    // Allocate object as copy and return
    return (new HGF2DHelmert (*this));
    }



//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DHelmert::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY, TRANSLATION, and Helmert, but Helmert
    // would have processed itself so not processed.
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DHelmert(*this);
        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be Helmert
        HAutoPtr<HGF2DHelmert> pMySimi(new HGF2DHelmert());

        // Cast model as a Helmert
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        double     MyScaledSin = sin(m_Rotation);
        double     MyScaledCos = cos(m_Rotation);

        // Compute composed translation
        double TransX = pModelPrime->GetTranslation().GetDeltaX() * MyScaledCos -
                        pModelPrime->GetTranslation().GetDeltaY() * MyScaledSin +
                        m_XTranslation;
        double TransY = pModelPrime->GetTranslation().GetDeltaY() * MyScaledCos +
                        pModelPrime->GetTranslation().GetDeltaX() * MyScaledSin +
                        m_YTranslation;

        // Set parameters
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
void HGF2DHelmert::Prepare ()
    {
    double     MySin = sin(m_Rotation);
    double     MyCos = cos(m_Rotation);
    double     MyNegSin = sin(-m_Rotation);
    double     MyNegCos = cos(-m_Rotation);

    // Prepare transformation factor for direct and inverse transformation
    // GetRotation() is not used because the compiler refused to inline it
    m_PreparedDirectA1 = MyCos;
    m_PreparedDirectB1 = MySin;
    m_PreparedDirectA2 = MyCos;
    m_PreparedDirectB2 = MySin;
    m_PreparedInverseA1 = (MyNegCos);
    m_PreparedInverseB1 = (MyNegSin);
    m_PreparedInverseA2 = (MyNegCos);
    m_PreparedInverseB2 = (MyNegSin); 

    // Convert direct translation to inverse translation (negative value)
    // GetTranslation() is not used since compiler refused to inline it
    
    m_XTranslationInverse = ((-m_YTranslation * MySin) - (m_XTranslation * MyCos));

    m_YTranslationInverse = ((-m_YTranslation * MyCos) + (m_XTranslation * MySin));

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
void HGF2DHelmert::Copy(const HGF2DHelmert& pi_rObj)
    {
    // Copy Helmert parameters
    m_Rotation = pi_rObj.m_Rotation;
    m_XTranslation = pi_rObj.m_XTranslation;
    m_YTranslation = pi_rObj.m_YTranslation;

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
HFCMatrix<3, 3> HGF2DHelmert::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    double MySin = sin(m_Rotation);
    double MyCos = cos(m_Rotation);

    ReturnedMatrix[0][2] = m_XTranslation;
    ReturnedMatrix[1][2] = m_YTranslation;
    ReturnedMatrix[0][0] = MyCos;
    ReturnedMatrix[0][1] = -MySin;
    ReturnedMatrix[1][0] = MySin;
    ReturnedMatrix[1][1] = MyCos;
    ReturnedMatrix[2][0] = 0.0;
    ReturnedMatrix[2][1] = 0.0;
    ReturnedMatrix[2][2] = 1.0;

    return(ReturnedMatrix);
    }


//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DHelmert::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    double MySin = sin(m_Rotation);
    double MyCos = cos(m_Rotation);

    po_rRecipient[0][2] = m_XTranslation;
    po_rRecipient[1][2] = m_YTranslation;
    po_rRecipient[0][0] = MyCos;
    po_rRecipient[0][1] = -MySin;
    po_rRecipient[1][0] = MySin;
    po_rRecipient[1][1] = MyCos;
    po_rRecipient[2][0] = 0.0;
    po_rRecipient[2][1] = 0.0;
    po_rRecipient[2][2] = 1.0;

    return(po_rRecipient);
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DHelmert::CreateSimplifiedModel() const
    {
    if (m_Rotation == 0.0)
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

    // If we get here, no simplification is possible.
    return 0;
    }
