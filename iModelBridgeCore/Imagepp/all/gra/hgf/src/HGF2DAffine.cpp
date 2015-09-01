//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DAffine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DAffine
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFAngle.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DDCTransfoModel.h>

#include <Imagepp/all/h/HGF2DAffine.h>


/** -----------------------------------------------------------------------------
    Default Constructor
    Initializes the model to no transformation with units for all channels
    set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DAffine::HGF2DAffine()
    : HGF2DTransfoModel(),
      m_ScaleX(1.0),
      m_ScaleY(1.0),
      m_XTranslation(0.0),
      m_YTranslation(0.0),
      m_Rotation(0.0),
      m_Anorthogonality(0.0)
    {
    Prepare();
    }


/** -----------------------------------------------------------------------------
    Constructor
    This constructor sets directly the transformation parameters. The dimension
    units are then taken from the displacement provided.

    @param pi_rTranslation IN A constant reference to an HGF2DDisplacement object
                            containing the description of the translation
                            component of the affine.
    @param pi_rRotation IN  Rotation component of the affine.
    @param pi_ScalingX IN The scaling factor for x coordinates. This scaling
                            must be different from 0.0.
    @param pi_ScalingY IN The scaling factor for y coordinates. This scaling
                            must be different from 0.0.
    @param pi_rAnorthogonality IN The anorthogonality of the axes (shearing).
                            This angle must be in the interval )- pi/2, pi/2( exclusive
                            or any equivalent angle.

    @code
        double              MyRotation = PI;
        double              MyAnortho = 0.00001;

        HGF2DDisplacement   MyTranslation(10, 10);
        HGF2DAffine         MyThirdModel(MyTranslation,
                                         MyRotation,2.0,2.1,MyAnortho);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DAffine::HGF2DAffine(const HGF2DDisplacement& pi_rTranslation,
                         double                  pi_rRotation,
                         double                  pi_ScaleX,
                         double                  pi_ScaleY,
                         double                  pi_rAnorthogonality)
    : HGF2DTransfoModel(),
    m_Rotation (pi_rRotation),
    m_Anorthogonality(pi_rAnorthogonality),
    m_XTranslation (pi_rTranslation.GetDeltaX()),
    m_YTranslation (pi_rTranslation.GetDeltaY()),
    m_ScaleX(pi_ScaleX),
    m_ScaleY(pi_ScaleY)
    {
    HPRECONDITION(pi_ScaleX != 0.0);
    HPRECONDITION(pi_ScaleY != 0.0);
    HDEBUGCODE(double MyTrigoValue = CalculateNormalizedTrigoValue(pi_rAnorthogonality));
    HPRECONDITION((MyTrigoValue < PI/2) || (MyTrigoValue > (3*PI/2)));

    Prepare();
    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DAffine::HGF2DAffine(const HGF2DAffine& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Copy(pi_rObj);
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
HGF2DAffine::HGF2DAffine(unsigned short pi_NumberOfPoints,
                         const double*  pi_pTiePoints)
    : HGF2DTransfoModel()
    {
    HPRECONDITION (pi_NumberOfPoints >= AFFINE_MIN_NB_TIE_PTS );

    double Matrix[4][4];
    HGF2DDCTransfoModel::GetAffineTransfoMatrixFromScaleAndTiePts(Matrix, pi_NumberOfPoints, pi_pTiePoints);

    SetByMatrixParameters(Matrix[0][3], Matrix[0][0], Matrix[0][2], Matrix[1][3], Matrix[1][0], Matrix[1][1]);
    }


//-----------------------------------------------------------------------------
// The destroyer.
//-----------------------------------------------------------------------------
HGF2DAffine::~HGF2DAffine()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DAffine& HGF2DAffine::operator=(const HGF2DAffine& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModel::operator=(pi_rObj);
        Copy(pi_rObj);
        }

    // Return reference to self
    return (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DAffine::IsConvertDirectThreadSafe() const 
    { 
    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DAffine::IsConvertInverseThreadSafe() const 
    { 
    return true; 
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DAffine::ConvertDirect(double* pio_pXInOut,
                                     double* pio_pYInOut) const
    {
    // Make sure variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    // Duplicate values since needed in both equations
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
StatusInt HGF2DAffine::ConvertDirect (double    pi_YIn,
                                      double    pi_XInStart,
                                      size_t     pi_NumLoc,
                                      double    pi_XInStep,
                                      double*   po_pXOut,
                                      double*   po_pYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    // Prepare byproducts
    double ByProdY1 = pi_YIn * -m_PreparedDirectB1 + m_XTranslationPrime;
    double ByProdY2 = pi_YIn * m_PreparedDirectA2 + m_YTranslationPrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = (X * m_PreparedDirectA1) + ByProdY1;
        *pCurrentY = (X * m_PreparedDirectB2) + ByProdY2;
        }
    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DAffine::ConvertDirect (size_t     pi_NumLoc,
                                      double*    pio_aXInOut,
                                      double*    pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    for (uint32_t i = 0; i < pi_NumLoc ; i++)
        {
        double X = pio_aXInOut[i];
        double Y = pio_aYInOut[i];

        pio_aXInOut[i] = (X * m_PreparedDirectA1) + Y * -m_PreparedDirectB1 + m_XTranslationPrime;
        pio_aYInOut[i] = (X * m_PreparedDirectB2) + Y * m_PreparedDirectA2 + m_YTranslationPrime;
        }
    return SUCCESS;
    }



//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DAffine::ConvertDirect(double   pi_XIn,
                                     double   pi_YIn,
                                     double*  po_pXOut,
                                     double*  po_pYOut) const
    {
    // Make sure recipient variables are provided
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
StatusInt HGF2DAffine::ConvertInverse(double* pio_pXInOut,
                                      double* pio_pYInOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    // Make a duplicate of values since used in both equations
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
StatusInt HGF2DAffine::ConvertInverse (double    pi_YIn,
                                       double    pi_XInStart,
                                       size_t     pi_NumLoc,
                                       double    pi_XInStep,
                                       double*   po_pXOut,
                                       double*   po_pYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    uint32_t Index;
    double  X;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

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
StatusInt HGF2DAffine::ConvertInverse (size_t     pi_NumLoc,
                                       double*    pio_aXInOut,
                                       double*    pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    for (uint32_t i = 0; i < pi_NumLoc; ++i)
        {
        double X = pio_aXInOut[i];
        double Y = pio_aYInOut[i];

        pio_aXInOut[i] = (X * m_PreparedInverseA1) + Y * -m_PreparedInverseB1 + m_XTranslationInversePrime;
        pio_aYInOut[i] = (X * m_PreparedInverseB2) + Y * m_PreparedInverseA2 + m_YTranslationInversePrime;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DAffine::ConvertInverse(double  pi_XIn,
                                      double  pi_YIn,
                                      double* po_pXOut,
                                      double* po_pYOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Transform coordinates
    *po_pXOut = (pi_XIn * m_PreparedInverseA1) + (pi_YIn * -m_PreparedInverseB1) +
                m_XTranslationInversePrime;
    *po_pYOut = (pi_XIn * m_PreparedInverseB2) + (pi_YIn * m_PreparedInverseA2) +
                m_YTranslationInversePrime;

    return SUCCESS;
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
// An affine transform always preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DAffine::PreservesLinearity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
// An affine transform always preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DAffine::PreservesParallelism() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DAffine::PreservesShape() const
    {
    // Shape is preserved only if there is no anisotropic scaling implied
    // and no anorthogonality
    return ((m_ScaleX == m_ScaleY) && (m_Anorthogonality == 0.0));
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DAffine::PreservesDirection() const
    {
    // Direction is preserved only if there is no anisotropic scaling implied
    // no rotation nor anorthogonality
    return ((m_Rotation == 0.0) &&
            (m_ScaleX == m_ScaleY) &&
            (m_Anorthogonality == 0.0));
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
// in the case of an affine this is always true
//-----------------------------------------------------------------------------
bool HGF2DAffine::CanBeRepresentedByAMatrix() const
    {
    return(true);
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DAffine::IsIdentity () const
    {

    return ((m_Rotation == 0.0) &&
            (m_XTranslation == 0.0) &&
            (m_YTranslation == 0.0) &&
            (m_ScaleX == 1.0) &&
            (m_ScaleY == 1.0) &&
            (m_Anorthogonality == 0.0));
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DAffine::IsStretchable (double pi_AngleTolerance) const
    {
    // For the model to be stretchable it must have no rotation and no anorthogonality
    return ((HDOUBLE_EQUAL(m_Anorthogonality, 0.0, pi_AngleTolerance)) &&
            (HDOUBLE_EQUAL(m_Rotation, 0.0, pi_AngleTolerance)));
    }



//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DAffine::GetStretchParams (double*           po_pScaleFactorX,
                                    double*           po_pScaleFactorY,
                                    HGF2DDisplacement* po_pDisplacement) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    // Extract stretch parameters
    *po_pScaleFactorX = m_ScaleX;
    *po_pScaleFactorY = m_ScaleY;

    // Obtain translation
    *po_pDisplacement = GetTranslation();
    }


//-----------------------------------------------------------------------------
//  SetByMatrixParameters
//  Sets the components of the affine by matrix parameters
//-----------------------------------------------------------------------------
void HGF2DAffine::SetByMatrixParameters(double pi_A0,
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

    // Compute anorthogonality
    // If B2 is 0, then cos of rotation plus anorthogonality is 0, therefore
    // their sum is equal to PI/2 or -PI/2. Either choice will do, but a selection will
    // reverse the sign of the Y scaling compared to the other. If B2 is 0, then
    // the rotation cannot be 0, since this would result in an anorthogonality
    // of PI/2 or -PI/2, but this is trapped by the preconditions.
    if (pi_B2 != 0.0)
        m_Anorthogonality = atan(-pi_A2/pi_B2) - m_Rotation;
    else
        m_Anorthogonality = PI/2 - m_Rotation;

    // Normalize anorthogonality
    // This makes sure that the anorthogonality is in the range -PI/2 to PI/2
    // Addition or substraction of PI simply results in reversal of the sign of
    // the Y scaling
    if ((CalculateNormalizedTrigoValue(m_Anorthogonality) >= PI/2) &&
        (CalculateNormalizedTrigoValue(m_Anorthogonality) <= 3*PI/2))
        m_Anorthogonality -= PI;

    // Compute X scale
    double     MyCos = cos(m_Rotation);

    // Check that cos is not 0. This implies that the new rotation is PI/2 or -PI/2
    // which is valid, but renders the first equation unsolvable. The second resolves
    // the scale in that case. Normaly a 0 cos implies an A1 equal to 0, but insures that
    // B1 is not equal to 0.
    if (!HDOUBLE_EQUAL_EPSILON(MyCos, 0.0) && pi_A1 != 0.0)
        m_ScaleX = pi_A1 / MyCos;
    else
        m_ScaleX = pi_B1 / sin(m_Rotation);

    // Compute Y scale
    double     MyCosAnortho = cos(m_Rotation + m_Anorthogonality);

    // Check that cos is not 0. This implies that the new rotation plus new anorthogonality is PI/2 or -PI/2
    // which is valid, but renders the first equation unsolvable. The second resolves
    // the scale in that case. Normaly a 0 cos implies an B2 equal to 0, but insures that
    // A2 is not equal to 0.
    if (!HDOUBLE_EQUAL_EPSILON(MyCosAnortho, 0.0) && pi_B2 != 0.0)
        m_ScaleY = pi_B2 / MyCosAnortho;
    else
        m_ScaleY = -pi_A2 / sin(m_Rotation + m_Anorthogonality);

    // Compute prepared settings
    Prepare ();
    }


//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the affine by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DAffine::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    ReturnedMatrix[0][0] = m_ScaleX * cos(m_Rotation);
    ReturnedMatrix[0][1] = -m_ScaleY * sin(m_Rotation + m_Anorthogonality);
    ReturnedMatrix[0][2] = m_XTranslation;
    ReturnedMatrix[1][0] = m_ScaleX * sin(m_Rotation);
    ReturnedMatrix[1][1] = m_ScaleY * cos(m_Rotation + m_Anorthogonality);
    ReturnedMatrix[1][2] = m_YTranslation;
    ReturnedMatrix[2][0] = 0.0;
    ReturnedMatrix[2][1] = 0.0;
    ReturnedMatrix[2][2] = 1.0;

    return(ReturnedMatrix);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the affine by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DAffine::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    po_rRecipient[0][0] = m_ScaleX * cos(m_Rotation);
    po_rRecipient[0][1] = -m_ScaleY * sin(m_Rotation + m_Anorthogonality);
    po_rRecipient[0][2] = m_XTranslation;
    po_rRecipient[1][0] = m_ScaleX * sin(m_Rotation);
    po_rRecipient[1][1] = m_ScaleY * cos(m_Rotation + m_Anorthogonality);
    po_rRecipient[1][2] = m_YTranslation;
    po_rRecipient[2][0] = 0.0;
    po_rRecipient[2][1] = 0.0;
    po_rRecipient[2][2] = 1.0;

    return(po_rRecipient);
    }

/** -----------------------------------------------------------------------------
    This method sets the translation component of the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing
                              the new translation component of the model.

    @code
        HGF2DAffine            MyModel;
        HGF2DDisplacement   Translation (10.2, 20.0);

        MyModel.SetTranslation(Translation);
    @end

    @see GetTranslation()
    @see AddTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::SetTranslation (const HGF2DDisplacement& pi_rTranslation)
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
        HGF2DDisplacement   Translation (10.2, -20.0);

        MyModel.AddTranslation  (Translation);
    @end

    @see GetTranslation()
    @see SetTranslation()
    @see AddRotation()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::AddTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    // Set translation
    m_XTranslation += pi_rTranslation.GetDeltaX();
    m_YTranslation += pi_rTranslation.GetDeltaY();

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method sets the anorthogonality component of the model. The angle
    of the anorthogonality must be in the interval ]-PI/2, PI/2[ exclusive,
    or in the interval ]3PI/2, 2PI ] or in the interval
    [0, PI/2[ or any equivalent angle.

    @param rAngle    The angle of the new anorthogonality component of the model.
                    This angle must be in the interval ]- PI/2,  PI/2[ exclusive
                    or any equivalent angle

    @code
        HGF2DAffine     MyModel;
        double          MyAnorthoAngle = 0.05; // Radians);

        MyModel.SetAnorthogonality (MyAnorthoAngle);
    @end

    @see GetAnorthogonality()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::SetAnorthogonality(double pi_rAnorthogonality)
    {
    // This condition enforces the rule that limits the anorthogonality
    // to the domain of value -PI/2 to PI/2 exclusive.
    HDEBUGCODE(double MyTrigoValue = CalculateNormalizedTrigoValue(pi_rAnorthogonality));
    HPRECONDITION((MyTrigoValue < PI/2) || (MyTrigoValue > (3*PI/2)));

    // Set rotation
    m_Anorthogonality = pi_rAnorthogonality;

    // Compute prepared settings
    Prepare ();
    }



/** -----------------------------------------------------------------------------
    This method sets the rotation component of the model.

    @param pi_Angle IN The angle of the new rotation component of the model.

    @code
        HGF2DAffine     MyModel;
        double          MyRotationAngle = 13.5; // Radians);

        MyModel.SetRotation (MyRotationAngle);
    @end

    @see GetRotation()
    @see AddRotation()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::SetRotation (double pi_Angle)
    {
    // Set rotation
    m_Rotation = pi_Angle;

    // Compute prepared settings
    Prepare ();

    }


/** -----------------------------------------------------------------------------
    This method adds a rotation around a specified center of rotation to the
    current components of the model.
    If center of rotation is omitted, then the origin is used.
    Remember that when specifying a center different than the origin,
    a translation component is also added to the model.

    @param pi_Angle IN The angle of the rotation component to add to the model.

    @param pi_rXCenter IN A double that contains
                          the X value of the center of the rotation to add to
                          the model.

    @param pi_rYCenter IN A double that contains
                          the Y value of the center of the rotation to add
                          to the model.

    @see GetRotation()
    @see SetRotation()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::AddRotation (double  pi_Angle,
                               double  pi_rXCenter,
                               double  pi_rYCenter)
    {
    // Add rotation component of the added rotation
    m_Rotation += pi_Angle;

    // Intermediate results
    double MySin = sin(pi_Angle);
    double MyCos = cos(pi_Angle);

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


    // Compute prepared settings
    Prepare();
    }



/** -----------------------------------------------------------------------------
    This method sets scaling component of the model in the X direction.

    @param pi_ScaleX IN The scaling factor of the scaling component of the model
                        in the X direction.
                        This factor must be different than 0.0

    @code
        HGF2DAffine  MyModel;
        MyModel.SetXScaling (34.5);
    @end

    @see GetXScaling()
    @see SetYScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::SetXScaling (double pi_ScaleX)
    {
    HPRECONDITION(pi_ScaleX != 0.0);

    // Set scale
    m_ScaleX = pi_ScaleX;

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method sets scaling component of the model in the Y direction.

    @param pi_ScaleY IN The scaling factor of the scaling component of the model
                        in the Y direction.
                        This factor must be different than 0.0

    @code
        HGF2DAffine  MyModel;
        MyModel.SetYScaling (34.5);
    @end

    @see GetXYcaling()
    @see SetXScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::SetYScaling (double pi_ScaleY)
    {
    HPRECONDITION(pi_ScaleY != 0.0);

    // Set scale
    m_ScaleY = pi_ScaleY;

    // Compute prepared settings
    Prepare ();
    }




/** -----------------------------------------------------------------------------
    This method adds an isotropic scaling around a specified center of scaling
    to the current components of the model.
    If center omitted, then the origin is used.
    Remember that when specifying a center different than the origin, a
    translation component is also added to the model.

    @param pi_Scaler IN The scaling factor of the scaling component to add
                        to the model. This factor must be different from 0.0.


    @param pi_rXCenter IN OPTIONAL A double that
                                contains the X value of the center of the scaling
                                to add to the model.
    @param pi_rYCenter IN OPTIONAL A double that
                                contains the Y value of the center of the scaling
                                to add to the model.


    @see GetXScaling()
    @see GetYScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::AddIsotropicScaling (double  pi_Scale,
                                       double  pi_rXCenter,
                                       double  pi_rYCenter)
    {
    HPRECONDITION(pi_Scale != 0.0);

    // Add scaling component
    m_ScaleX *= pi_Scale;
    m_ScaleY *= pi_Scale;

    // calculate new translation component
    double TransX = ((m_XTranslation * pi_Scale) +
                     (pi_rXCenter + pi_rYCenter * pi_Scale - pi_rXCenter * pi_Scale));
    double TransY = ((m_YTranslation * pi_Scale) +
                     (pi_rYCenter - pi_rYCenter * pi_Scale - pi_rXCenter * pi_Scale));

    // Set translation
    m_XTranslation = TransX;
    m_YTranslation = TransY;

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method adds an anisotropic scaling (different in X and Y dimensions)
    around a specified center of scaling to the current components of the model.
    The center is expressed as raw numbers that will be interpreted in the
    X and Y units of the direct channels of the transformation model.
    If center is omitted, then the origin is used.
    Remember that when specifying a center different than the origin, a
    translation component is also added to the model.

    @param pi_ScaleX IN The scaling factor of the scaling component to
                        add to the model in the X dimension.
                        This factor must be different from 0.0.

    @param pi_ScaleY IN The scaling factor of the scaling component to
                        add to the model in the Y dimension.
                        This factor must be different from 0.0.

    @param pi_XCenter IN OPTIONAL The X value of the center of the scaling to
                            add, that will be interpreted in the X
                            direct channel units.

    @param pi_YCenter IN OPTIONAL The Y value of the center of the scaling to
                            add, that will be interpreted in the Y
                            direct channel units.

    @see GetXScaling()
    @see GetYScaling()
    @see AddIsotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::AddAnisotropicScaling (double pi_ScaleX,
                                         double pi_ScaleY,
                                         double pi_rXCenter,
                                         double pi_rYCenter)
    {
    HPRECONDITION(pi_ScaleX != 0.0);
    HPRECONDITION(pi_ScaleY != 0.0);

    // Add scaling component
    m_ScaleX *= pi_ScaleX;
    m_ScaleY *= pi_ScaleY;

    // calculate new translation component
    double TransX = ((m_XTranslation * pi_ScaleX) +
                     (pi_rXCenter - pi_rXCenter * pi_ScaleX));
    double TransY = ((m_YTranslation * pi_ScaleY) +
                     (pi_rYCenter - pi_rYCenter * pi_ScaleY));

    // Set translation
    m_XTranslation = TransX;
    m_YTranslation = TransY;

    // Compute prepared settings
    Prepare ();
    }




/** -----------------------------------------------------------------------------
    This method adds a horizontal flip around a specified mirror position
    to the current components of the model. The mirror position is expressed
    as a raw number that will be interpreted in the X units of the direct channels
    of the transformation model. If mirror position is omitted, the origin
    is used.
    Remember that when specifying a mirror position different than the Y-axis,
    a translation component is added to the model in addition to the
    negative scaling factor.

    @param pi_MirrorPos IN OPTIONAL The X value of the mirror position of the flip
                            to add, that will be interpreted in the X direct
                            channel units.


    @code
        HGF2DAffine MyModel;

        MyModel.AddHorizontalFlip (10.0);
    @end

    @see AddVerticalFlip()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::AddHorizontalFlip (double pi_rXMirrorPos)
    {
    // Set parameters
    m_XTranslation = m_XTranslation + (2.0 * pi_rXMirrorPos);
    m_ScaleX *= -1.0;

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method adds a vertical flip around a specified mirror position to the
    current components of the model. The mirror position is expressed as a raw
    number, that will be interpreted in the Y units of the direct channels of
    the transformation model. If mirror position is omitted, then the origin is
    used.
    Remember that when specifying a mirror position different than the X axis,
    a translation component is added to the model in addition to the
    negative scaling factor.

    @param pi_RawYMirrorPos IN OPTIONAL The Y value of the mirror position of the flip
                            to add, that will be interpreted in the Y direct
                            channel units.

    @code
        HGF2DAffine MyModel;

        MyModel.AddVerticalFlip (10.0);
    @end

    @see AddHorizontalFlip()
    -----------------------------------------------------------------------------
*/
void HGF2DAffine::AddVerticalFlip (double pi_rYMirrorPos)
    {
    // Set parameters
    m_YTranslation = m_YTranslation + (2.0 * pi_rYMirrorPos);
    m_ScaleY *= -1.0;

    // Compute prepared settings
    Prepare ();
    }



//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DAffine::Reverse()
    {

    HFCMatrix<3, 3> MyMatrix = GetMatrix();


    // Compute inverse of matrix
    HFCMatrix<3, 3> InverseMatrix;

    double Determinant = MyMatrix[0][0] * MyMatrix[1][1] - MyMatrix[1][0] * MyMatrix[0][1];

    // Make sure the determinant is different from 0.0
    // Exact compare operation os voluntary
    HASSERT(Determinant != 0.0);

    InverseMatrix[0][2] = (MyMatrix[0][1] * MyMatrix[1][2] - MyMatrix[1][1] * MyMatrix[0][2]) / Determinant;
    InverseMatrix[1][2] = (MyMatrix[1][0] * MyMatrix[0][2] - MyMatrix[0][0] * MyMatrix[1][2]) / Determinant;
    InverseMatrix[0][0] = MyMatrix[1][1] / Determinant;
    InverseMatrix[0][1] = -MyMatrix[0][1] / Determinant;
    InverseMatrix[1][0] = -MyMatrix[1][0] / Determinant;
    InverseMatrix[1][1] = MyMatrix[0][0] / Determinant;
    InverseMatrix[2][2] = MyMatrix[2][2];



    // Set translation components
    m_XTranslation = InverseMatrix[0][2];
    m_YTranslation = InverseMatrix[1][2];

    // Compute rotation
    // if A1 is zero, then the cos of rotation is 0 therefore the angle is either
    // PI/2 or -PI/2. Any one will do, but the sign of the scalings will be the reverse
    // for each possible values of rotation
    if (InverseMatrix[0][0] != 0.0)
        m_Rotation = atan(InverseMatrix[1][0]/InverseMatrix[0][0]);
    else
        m_Rotation = PI/2;

    // Compute anorthogonality
    // If B2 is 0, then cos of rotation plus anorthogonality is 0, therefore
    // their sum is equal to PI/2 or -PI/2. Either choice will do, but a selection will
    // reverse the sign of the Y scaling compared to the other. If B2 is 0, then
    // the rotation cannot be 0, since this would result in an anorthogonality
    // of PI/2 or -PI/2, but this is trapped by the preconditions.
    if (InverseMatrix[1][1] != 0.0)
        m_Anorthogonality = atan(-InverseMatrix[0][1]/InverseMatrix[1][1]) - m_Rotation;
    else
        m_Anorthogonality = PI/2 - m_Rotation;

    // Normalize anorthogonality
    // This makes sure that the anorthogonality is in the range -PI/2 to PI/2
    // Addition or substraction of PI simply results in reversal of the sign of
    // the Y scaling
    if ((CalculateNormalizedTrigoValue(m_Anorthogonality) >= PI/2) &&
        (CalculateNormalizedTrigoValue(m_Anorthogonality) <= 3*PI/2))
        m_Anorthogonality -= PI;

    // Compute X scale
    double     MyCos = cos(m_Rotation);
    double     MySin = sin(m_Rotation);

    /*
    ** Take the largest value of the Sine or the Cosine of the angle to avoid
    ** dividing by zero.  Both values for MySin and MyCos cannot be both zero,
    ** since sin(a)*sin(a) + cos(a)*cos(a) = 1 for all values of a.
    ** Taking the largest value ensures we stay away from a division by zero, and
    ** as a bonus increases the precision of the result.
    */
    if ( fabs(MyCos) > fabs(MySin) )
        m_ScaleX = InverseMatrix[0][0] / MyCos;
    else
        m_ScaleX = InverseMatrix[1][0] / MySin;

    // Compute Y scale
    double     MyCosAnortho = cos(m_Rotation + m_Anorthogonality);
    double     MySinAnortho = sin(m_Rotation + m_Anorthogonality);

    /*
    ** Take the largest value of the Sine or the Cosine of the angle to avoid
    ** dividing by zero.  Both values for MySin and MyCos cannot be both zero,
    ** since sin(a)*sin(a) + cos(a)*cos(a) = 1 for all values of a, and that
    ** both MySin and MyCos values are added with the same value.
    ** Taking the largest value ensures we stay away from a division by zero, and
    ** as a bonus increases the precision of the result.
    */
    if ( fabs(MyCosAnortho) > fabs(MySinAnortho) )
        m_ScaleY = InverseMatrix[1][1] / MyCosAnortho;
    else
        m_ScaleY = -InverseMatrix[0][1] / MySinAnortho;


    // Invoque reversing of ancester
    HGF2DTransfoModel::Reverse();

    // Prepare
    Prepare();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DAffine::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY TRANSLATION, SIMILITUDE and AFFINE
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DAffine(*this);

        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be affine
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());


        // Cast model as a affine
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        // Compute composed translation
        double TransX = m_XTranslation + pModelPrime->GetTranslation().GetDeltaX();
        double TransY = m_YTranslation + pModelPrime->GetTranslation().GetDeltaY();

        // Set parameters
        pMyAffine->m_Rotation = m_Rotation;
        pMyAffine->m_ScaleX = m_ScaleX;
        pMyAffine->m_ScaleY = m_ScaleY;
        pMyAffine->m_Anorthogonality = m_Anorthogonality;
        pMyAffine->m_XTranslation = TransX;
        pMyAffine->m_YTranslation = TransY;
        pMyAffine->Prepare();

        // Assign to smart pointer
        pResultModel = pMyAffine.release();
        }
    else if (TheModelType == HGF2DSimilitude::CLASS_ID)
        {
        // Allocate
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());


        // Cast model as a affine
        const HGF2DSimilitude* pModelPrime = static_cast<const HGF2DSimilitude*>(&pi_rModel);

        // Calculate trigonometric values of similitude
        double MySin = sin(pModelPrime->GetRotation());
        double MyCos = cos(pModelPrime->GetRotation());

        // Calculate new translation
        double TransX = m_XTranslation * pModelPrime->GetScaling() * MyCos -
                             m_YTranslation * pModelPrime->GetScaling() * MySin +
                             pModelPrime->GetTranslation().GetDeltaX();
        double TransY = m_XTranslation * pModelPrime->GetScaling() * MySin +
                             m_YTranslation * pModelPrime->GetScaling() * MyCos +
                             pModelPrime->GetTranslation().GetDeltaY();

        // Set parameters
        pMyAffine->m_XTranslation = TransX;
        pMyAffine->m_YTranslation = TransY;
        pMyAffine->m_Rotation = m_Rotation + pModelPrime->GetRotation();
        pMyAffine->m_ScaleX = m_ScaleX * pModelPrime->GetScaling();
        pMyAffine->m_ScaleY = m_ScaleY * pModelPrime->GetScaling();
        pMyAffine->m_Anorthogonality = m_Anorthogonality;
        pMyAffine->Prepare();

        // Assign to smart pointer
        pResultModel = pMyAffine.release();
        }
    else if (TheModelType == HGF2DAffine::CLASS_ID)
        {

        // We have two affine models ... compose a affine
        // Allocate
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());

        HFCMatrix<3, 3> MyGivenMatrix(pi_rModel.GetMatrix());

        // Compose the two matrix together


        HFCMatrix<3, 3> NewMatrix = MyGivenMatrix * GetMatrix();

        // The result projection parameters must be null
        HASSERT(HDOUBLE_EQUAL_EPSILON(NewMatrix[2][0], 0.0));
        HASSERT(HDOUBLE_EQUAL_EPSILON(NewMatrix[2][1], 0.0));

        // The global scale must be 1
        HASSERT(HDOUBLE_EQUAL_EPSILON(NewMatrix[2][2], 1.0));

        pMyAffine->SetByMatrixParameters(NewMatrix[0][2],
                                         NewMatrix[0][0],
                                         NewMatrix[0][1],
                                         NewMatrix[1][2],
                                         NewMatrix[1][0],
                                         NewMatrix[1][1]);


        // Assign to smart pointer
        pResultModel = pMyAffine.release();
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
HGF2DTransfoModel* HGF2DAffine::Clone () const
    {
    // Allocate object as copy and return
    return(new HGF2DAffine(*this));
    }



//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DAffine::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY TRANSLATION, SIMILITUDE, and AFFINE but affine
    // would have processed itself so not processed.
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DAffine(*this);
        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be affine
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());

        // Cast model as a affine
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        // Compute various sin and cos required from added model
        double MySin = sin(m_Rotation);
        double MyCos = cos(m_Rotation);
        double MySinAnortho = sin(m_Rotation + m_Anorthogonality);
        double MyCosAnortho = cos(m_Rotation + m_Anorthogonality);

        // Compute composed translation
        double TransX = pModelPrime->GetTranslation().GetDeltaX() * m_ScaleX * MyCos -
                             pModelPrime->GetTranslation().GetDeltaY() * m_ScaleY * MySinAnortho +
                             m_XTranslation;
        double TransY = pModelPrime->GetTranslation().GetDeltaY() * m_ScaleY * MyCosAnortho +
                             pModelPrime->GetTranslation().GetDeltaX() * m_ScaleX * MySin +
                             m_YTranslation;

        // Set parameters
        pMyAffine->m_ScaleX = m_ScaleX;
        pMyAffine->m_ScaleY = m_ScaleY;
        pMyAffine->m_Rotation = m_Rotation;
        pMyAffine->m_Anorthogonality = m_Anorthogonality;
        pMyAffine->SetTranslation(HGF2DDisplacement(TransX, TransY));

        // Assign to smart pointer
        pResultModel = pMyAffine.release();
        }
    else if (TheModelType == HGF2DSimilitude::CLASS_ID)
        {
        double      NewRotation;
        double      NewAnorthogonality;
        double      NewScaleX;
        double      NewScaleY;

        // Allocate
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());

        // Cast model as a affine
        const HGF2DSimilitude* pModelPrime = static_cast<const HGF2DSimilitude*>(&pi_rModel);

        // Compute various sin and cos required
        double MySin = sin(pModelPrime->GetRotation());
        double MyCos = cos(pModelPrime->GetRotation());

        double MyPreviousSin = sin(m_Rotation);
        double MyPreviousCos = cos(m_Rotation);
        double MyPreviousSinAnortho = sin(m_Rotation + m_Anorthogonality);
        double MyPreviousCosAnortho = cos(m_Rotation + m_Anorthogonality);


        // Calculate new rotation
        if (HDOUBLE_EQUAL_EPSILON(MySin, 0.0))
            NewRotation = m_Rotation;
        else
            NewRotation = atan2(((m_ScaleX * MyCos * MyPreviousSin) +
                                 (m_ScaleY * MySin * MyPreviousCosAnortho)),
                                ((m_ScaleX * MyCos * MyPreviousCos) -
                                 (m_ScaleY * MyPreviousSinAnortho * MySin)));

        // Calculate new anorthogonality
        if (HDOUBLE_EQUAL_EPSILON(MySin, 0.0))
            NewAnorthogonality = m_Rotation + m_Anorthogonality - NewRotation;
        else
            NewAnorthogonality = atan2((m_ScaleX * MyPreviousCos * MySin +
                                        m_ScaleY * MyCos * MyPreviousSinAnortho),
                                       (m_ScaleY * MyCos * MyPreviousCosAnortho -
                                        m_ScaleX * MySin * MyPreviousSin)) - NewRotation;

        // Normalize anorthogonality
        if ((CalculateNormalizedTrigoValue(NewAnorthogonality) >= PI/2) &&
            (CalculateNormalizedTrigoValue(NewAnorthogonality) <= 3*PI/2))
            NewAnorthogonality -= PI;

        // Compute sin and cos for new rotation and anorthogonality
        double MyNewSin = sin(NewRotation);
        double MyNewCos = cos(NewRotation);
        double MyNewSinAnortho = sin(NewRotation + NewAnorthogonality);
        double MyNewCosAnortho = cos(NewRotation + NewAnorthogonality);

        // Calculate new X scale
        if (!HDOUBLE_EQUAL_EPSILON(MyNewCos, 0.0))
            NewScaleX = pModelPrime->GetScaling() * ((m_ScaleX * MyCos * MyPreviousCos -
                                                      m_ScaleY * MyPreviousSinAnortho * MySin) /
                                                     MyNewCos);
        else
            NewScaleX = pModelPrime->GetScaling() * ((m_ScaleX * MyPreviousSin * MyCos +
                                                      m_ScaleY * MyPreviousCosAnortho * MySin) /
                                                     MyNewSin);

        // Calculate new Y scale
        if (!HDOUBLE_EQUAL_EPSILON(MyNewCosAnortho,0.0))
            NewScaleY = pModelPrime->GetScaling() * ((m_ScaleY * MyPreviousCosAnortho * MyCos -
                                                      m_ScaleX * MyPreviousSin * MySin) /
                                                     MyNewCosAnortho);
        else
            NewScaleY = pModelPrime->GetScaling() * ((m_ScaleX * MyPreviousCos * MySin +
                                                      m_ScaleY * MyPreviousSinAnortho * MyCos) /
                                                     MyNewSinAnortho);


        // Calculate new translation
        double TransX = pModelPrime->GetTranslation().GetDeltaX() * m_ScaleX * MyPreviousCos -
                        pModelPrime->GetTranslation().GetDeltaY() * m_ScaleY * MyPreviousSinAnortho +
                        m_XTranslation;

        double TransY = pModelPrime->GetTranslation().GetDeltaX() * m_ScaleX * MyPreviousSin +
                        pModelPrime->GetTranslation().GetDeltaY() * m_ScaleY * MyPreviousCosAnortho +
                        m_YTranslation;


        // Set parameters
        pMyAffine->m_XTranslation = TransX;
        pMyAffine->m_YTranslation = TransY;
        pMyAffine->m_Rotation = NewRotation;
        pMyAffine->m_ScaleX = NewScaleX;
        pMyAffine->m_ScaleY = NewScaleY;
        pMyAffine->m_Anorthogonality = NewAnorthogonality;
        pMyAffine->Prepare();

        // Assign to smart pointer
        pResultModel = pMyAffine.release();
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
void HGF2DAffine::Prepare ()
    {
    double      InvRotation;
    double      InvScaleX;
    double      InvScaleY;

    double MySin = sin(m_Rotation);
    double MyCos = cos(m_Rotation);
    double MySinAnortho = sin(m_Rotation + m_Anorthogonality);
    double MyCosAnortho = cos(m_Rotation + m_Anorthogonality);


    // Compute reverse anorthogonality
    // We first compute the matrix parameters
    HFCMatrix<3, 3> MyMatrix(GetMatrix());


    // Compute adjoint of matrix
    // Compute inverse of matrix

    double Determinant = MyMatrix[0][0] * MyMatrix[1][1] - MyMatrix[1][0] * MyMatrix[0][1];

    double InvTx = (MyMatrix[0][1] * MyMatrix[1][2] - MyMatrix[1][1] * MyMatrix[0][2]) / Determinant;
    double InvTy = (MyMatrix[1][0] * MyMatrix[0][2] - MyMatrix[0][0] * MyMatrix[1][2]) / Determinant;
    double InvA11 = MyMatrix[1][1] / Determinant;
    double InvA12 = -MyMatrix[0][1] / Determinant;
    double InvA21 = -MyMatrix[1][0] / Determinant;
    double InvA22 = MyMatrix[0][0] / Determinant;

    double InvTxDist = InvTx;
    double InvTyDist = InvTy;

    m_XTranslationInverse = InvTxDist;
    m_YTranslationInverse = InvTyDist;

    // Compute rotation
    // if A1 is zero, then the cos of rotation is 0 therefore the angle is either
    // PI/2 or -PI/2. Any one will do, but the sign of the scalings will be the reverse
    // for each possible values of rotation
    if (InvA11 != 0.0)
        InvRotation = atan(InvA21/InvA11);
    else
        InvRotation = PI/2;

    // Compute anorthogonality
    double InvAnorthogonality;

    // If B2 is 0, then cos of rotation plus anorthogonality is 0, therefore
    // their sum is equal to PI/2 or -PI/2. Either choice will do, but a selection will
    // reverse the sign of the Y scaling compared to the other. If B2 is 0, then
    // the rotation cannot be 0, since this would result in an anorthogonality
    // of PI/2 or -PI/2, but this is trapped by the preconditions.
    if (InvA22 != 0.0)
        InvAnorthogonality = atan(-InvA12/InvA22) - InvRotation;
    else
        InvAnorthogonality = PI/2 - InvRotation;

    // Normalize anorthogonality
    // This makes sure that the anorthogonality is in the range -PI/2 to PI/2
    // Addition or substraction of PI simply results in reversal of the sign of
    // the Y scaling
    if ((CalculateNormalizedTrigoValue(InvAnorthogonality) >= PI/2) &&
        (CalculateNormalizedTrigoValue(InvAnorthogonality) <= 3*PI/2))
        InvAnorthogonality -= PI;

    double     MySinInv = sin(InvRotation);
    double     MyCosInv = cos(InvRotation);
    double     MySinInvAnortho = sin(InvRotation + InvAnorthogonality);
    double     MyCosInvAnortho = cos(InvRotation + InvAnorthogonality);


    // Check that cos is not 0. This implies that the new rotation is PI/2 or -PI/2
    // which is valid, but renders the first equation unsolvable. The second resolves
    // the scale in that case. Normaly a 0 cos implies an A1 equal to 0, but insures that
    // B1 is not equal to 0.
    if (!HDOUBLE_EQUAL_EPSILON(MyCosInv, 0.0) && InvA11 != 0.0)
        InvScaleX = InvA11 / MyCosInv;
    else
        InvScaleX = InvA21 / sin(InvRotation);

    // Check that cos is not 0. This implies that the new rotation plus new anorthogonality is PI/2 or -PI/2
    // which is valid, but renders the first equation unsolvable. The second resolves
    // the scale in that case. Normaly a 0 cos implies an B2 equal to 0, but insures that
    // A2 is not equal to 0.
    if (!HDOUBLE_EQUAL_EPSILON(MyCosInvAnortho, 0.0) && InvA22 != 0.0)
        InvScaleY = InvA22 / MyCosInvAnortho;
    else
        InvScaleY = -InvA12 / sin(InvRotation + InvAnorthogonality);

    // Prepare transformation factor for direct and inverse transformation
    m_PreparedDirectA1 = MyCos * m_ScaleX;
    m_PreparedDirectB1 = MySinAnortho * m_ScaleY;
    m_PreparedDirectA2 = MyCosAnortho * m_ScaleY;
    m_PreparedDirectB2 = MySin * m_ScaleX;

    m_PreparedInverseA1 = (MyCosInv * InvScaleX);
    m_PreparedInverseB1 = (MySinInvAnortho * InvScaleY);
    m_PreparedInverseA2 = (MyCosInvAnortho * InvScaleY);
    m_PreparedInverseB2 = (MySinInv * InvScaleX);

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
void HGF2DAffine::Copy(const HGF2DAffine& pi_rObj)
    {
    // Copy affine parameters
    m_Rotation = pi_rObj.m_Rotation;
    m_XTranslation = pi_rObj.m_XTranslation;
    m_YTranslation = pi_rObj.m_YTranslation;
    m_ScaleX = pi_rObj.m_ScaleX;
    m_ScaleY = pi_rObj.m_ScaleY;
    m_Anorthogonality = pi_rObj.m_Anorthogonality;

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
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DAffine::CreateSimplifiedModel() const
    {
    if (m_Anorthogonality == 0.0)
        {
        if (m_Rotation == 0.0)
            {
            if (m_ScaleX == 1.0 && m_ScaleY == 1.0)
                {
                if ((m_XTranslation == 0.0) && (m_YTranslation == 0.0))
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
                return new HGF2DStretch(GetTranslation(), m_ScaleX, m_ScaleY);
                }
            }
        else
            {
            // We can only simplify if we have an isotropic scaling.
            if (m_ScaleX == m_ScaleY)
                return new HGF2DSimilitude(GetTranslation(), m_Rotation, m_ScaleX);
            }
        }

    // If we get here, no simplification is possible.
    return 0;
    }
