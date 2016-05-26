//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DProjective.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DProjective
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HGFAngle.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DDCTransfoModel.h>

#include <Imagepp/all/h/HGF2DProjective.h>

/** -----------------------------------------------------------------------------
    Default Constructor
    Initializes the model to no transformation (Scaling = 1.0, Translation = none ,
    no rotation and no anorthogonality ) with units for all channels set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DProjective::HGF2DProjective()
    : HGF2DTransfoModel()
    {
    m_D00 = 1.0;
    m_D01 = 0.0;
    m_D02 = 0.0;
    m_D10 = 0.0;
    m_D11 = 1.0;
    m_D12 = 0.0;
    m_D20 = 0.0;
    m_D21 = 0.0;
    m_D22 = 1.0;

    Prepare();

    HINVARIANTS;
    }

/** -----------------------------------------------------------------------------
    Constructor by matrix
    Initializes the model to the transoformation implied by the given transformation matrix.
    The units are set to meters for inverse and direct channels.

    @param pi_rMatrix IN A constant reference to a matrix containing the transformation.
                         The determinant of the matrix may not be 0.

    -----------------------------------------------------------------------------
*/
HGF2DProjective::HGF2DProjective(const HFCMatrix<3, 3>& pi_rMatrix)
    : HGF2DTransfoModel()
    {
    // Make sure that global scaling is different from 0.0
    HPRECONDITION(pi_rMatrix[2][2] != 0);

    // Make sure that provided parameters are valid
    // This implies that the upper left parameters follow certain rules:
// HChk AR 2003-08-25 In fact, the determinant of the matrix may not be 0 which
// implies all of the other following rules.
    // One of [0,0] and [0,1] must different from 0.0
    // One of [0,0] and [1,0] must different from 0.0
    // One of [0,1] and [1,1] must different from 0.0
    // One of [1,0] and [1,1] must different from 0.0
    HPRECONDITION((pi_rMatrix[0][0] != 0.0) || (pi_rMatrix[0][1] != 0.0));
    HPRECONDITION((pi_rMatrix[0][0] != 0.0) || (pi_rMatrix[1][0] != 0.0));
    HPRECONDITION((pi_rMatrix[0][1] != 0.0) || (pi_rMatrix[1][1] != 0.0));
    HPRECONDITION((pi_rMatrix[1][0] != 0.0) || (pi_rMatrix[1][1] != 0.0));

    // Copy matrix parameters
    m_D00 = pi_rMatrix[0][0] / pi_rMatrix[2][2];
    m_D01 = pi_rMatrix[0][1] / pi_rMatrix[2][2];
    m_D02 = pi_rMatrix[0][2] / pi_rMatrix[2][2];
    m_D10 = pi_rMatrix[1][0] / pi_rMatrix[2][2];
    m_D11 = pi_rMatrix[1][1] / pi_rMatrix[2][2];
    m_D12 = pi_rMatrix[1][2] / pi_rMatrix[2][2];
    m_D20 = pi_rMatrix[2][0] / pi_rMatrix[2][2];
    m_D21 = pi_rMatrix[2][1] / pi_rMatrix[2][2];
    m_D22 = 1.0;

    // Prepare acceleration attributes
    Prepare();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor by raw matrix
//-----------------------------------------------------------------------------
HGF2DProjective::HGF2DProjective(double const  pi_pMatrix[3][3])
    : HGF2DTransfoModel()
    {
    // Make sure that global scaling is different from 0.0
    HPRECONDITION(pi_pMatrix[2][2] != 0);

    // Make sure that provided parameters are valid
    // This implies that the upper left parameters follow certain rules:
    // One of [0,0] and [0,1] must different from 0.0
    // One of [0,0] and [1,0] must different from 0.0
    // One of [0,1] and [1,1] must different from 0.0
    // One of [1,0] and [1,1] must different from 0.0
    HPRECONDITION((pi_pMatrix[0][0] != 0.0) || (pi_pMatrix[0][1] != 0.0));
    HPRECONDITION((pi_pMatrix[0][0] != 0.0) || (pi_pMatrix[1][0] != 0.0));
    HPRECONDITION((pi_pMatrix[0][1] != 0.0) || (pi_pMatrix[1][1] != 0.0));
    HPRECONDITION((pi_pMatrix[1][0] != 0.0) || (pi_pMatrix[1][1] != 0.0));

    // Copy matrix parameters
    m_D00 = pi_pMatrix[0][0] / pi_pMatrix[2][2];
    m_D01 = pi_pMatrix[0][1] / pi_pMatrix[2][2];
    m_D02 = pi_pMatrix[0][2] / pi_pMatrix[2][2];
    m_D10 = pi_pMatrix[1][0] / pi_pMatrix[2][2];
    m_D11 = pi_pMatrix[1][1] / pi_pMatrix[2][2];
    m_D12 = pi_pMatrix[1][2] / pi_pMatrix[2][2];
    m_D20 = pi_pMatrix[2][0] / pi_pMatrix[2][2];
    m_D21 = pi_pMatrix[2][1] / pi_pMatrix[2][2];
    m_D22 = 1.0;

    // Prepare acceleration attributes
    Prepare();

    HINVARIANTS;
    }



/** -----------------------------------------------------------------------------
    Constructor
    This constructor sets directly most transformation parameters. The dimension
    units are then taken from the displacement(translation) provided.

    @param pi_rTranslation IN A constant reference to an HGF2DDisplacement object
                            containing the description of the translation
                            component of the model.

    @param pi_rRotation IN Rotation component of the model.

    @param pi_ScalingX IN The scaling factor for x coordinates. This scaling
                            must be different from 0.0.

    @param pi_ScalingY IN The scaling factor for y coordinates. This scaling
                            must be different from 0.0.

    @param pi_rAnorthogonality IN The anorthogonality of the axes (shearing).
                            This angle must be in the interval )- pi/2, pi/2( exclusive
                            or any equivalent angle.

    @code
        double            MyRotation = PI;
        double            MyAnortho = 0.00001;

        HGF2DDisplacement   MyTranslation(10, 10);
        HGF2DProjective     MyThirdModel(MyTranslation,
                                         MyRotation, 2.0, 2.1, MyAnortho);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DProjective::HGF2DProjective(const HGF2DDisplacement& pi_rTranslation,
                                 double                  pi_rRotation,
                                 double                  pi_ScaleX,
                                 double                  pi_ScaleY,
                                 double                  pi_rAnorthogonality)
    : HGF2DTransfoModel()
    {
    // Scaling factors must not be 0.0
    HPRECONDITION(pi_ScaleX != 0.0);
    HPRECONDITION(pi_ScaleY != 0.0);
    HDEBUGCODE(double MyTrigoValue = CalculateNormalizedTrigoValue(pi_rAnorthogonality));
    HPRECONDITION((MyTrigoValue < PI/2) || (MyTrigoValue > (3*PI/2)));

    m_D00 = pi_ScaleX * cos(pi_rRotation);
    m_D01 = -pi_ScaleY * sin(pi_rRotation +  pi_rAnorthogonality);
    m_D02 = pi_rTranslation.GetDeltaX();
    m_D10 = pi_ScaleX * sin(pi_rRotation);
    m_D11 = pi_ScaleY * cos(pi_rRotation + pi_rAnorthogonality);
    m_D12 = pi_rTranslation.GetDeltaY();
    m_D20 = 0.0;
    m_D21 = 0.0;
    m_D22 = 1.0;

    Prepare();

    HINVARIANTS;
    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DProjective::HGF2DProjective(const HGF2DProjective& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Copy (pi_rObj);

    HINVARIANTS;
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
HGF2DProjective::HGF2DProjective(unsigned short pi_NumberOfPoints,
                                 const double*  pi_pTiePoints)
    : HGF2DTransfoModel()
    {
    HPRECONDITION (pi_NumberOfPoints >= PROJECTIVE_MIN_NB_TIE_PTS );

    double Matrix[4][4];
    HGF2DDCTransfoModel::GetProjectiveTransfoMatrixFromScaleAndTiePts(Matrix, pi_NumberOfPoints, pi_pTiePoints);

    m_D00 = Matrix[0][0];
    m_D01 = Matrix[0][1];
    m_D02 = Matrix[0][3];
    m_D10 = Matrix[1][0];
    m_D11 = Matrix[1][1];
    m_D12 = Matrix[1][3];
    m_D20 = Matrix[3][0];
    m_D21 = Matrix[3][1];
    m_D22 = Matrix[3][3];
    Prepare();
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DProjective::~HGF2DProjective()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DProjective& HGF2DProjective::operator=(const HGF2DProjective& pi_rObj)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DProjective::IsConvertDirectThreadSafe() const 
    { 
    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DProjective::IsConvertInverseThreadSafe() const 
    { 
    return true; 
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertDirect(double* pio_pXInOut,
                                    double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    // Compute the denominator part
    double Denominator = (m_D20 * X) + (m_D21 * Y) + m_D22;

    // The denominator must be different from 0.0
    HASSERT(Denominator != 0.0);

    if (0.0L == Denominator)
        return ERROR;

    // Transform coordinates
    *pio_pXInOut = ((X * m_D00) + (Y * m_D01) + m_D02) / Denominator;
    *pio_pYInOut = ((X * m_D10) + (Y * m_D11) + m_D12) / Denominator;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertDirect(double    pi_YIn,
                                         double    pi_XInStart,
                                         size_t    pi_NumLoc,
                                         double    pi_XInStep,
                                         double*   po_pXOut,
                                         double*   po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    uint32_t Index;

    StatusInt status = SUCCESS;
    double X;

    // Compute the denominator part
    double Denominator;
    double PartialDenominator = (m_D21 * pi_YIn) + m_D22;

    // Precalculate the Y components
    double ByProdY1 = ((pi_YIn * m_D01) + m_D02);
    double ByProdY2 = ((pi_YIn * m_D11) + m_D12);

    for (Index = 0, X = pi_XInStart; Index < pi_NumLoc ; ++Index, X+=pi_XInStep)
        {
        Denominator = (m_D20 * X) + PartialDenominator;

        HASSERT(Denominator != 0.0);

        // Note that even though a coordinate is invalid we continue with others.
        if (0.0L == Denominator)
            status = ERROR;
        else
            {
            po_pXOut[Index] = ((X * m_D00) + ByProdY1) / Denominator;
            po_pYOut[Index] = ((X * m_D10) + ByProdY2) / Denominator;
            }
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertDirect(size_t    pi_NumLoc,
                                         double*   pio_aXInOut,
                                         double*   pio_aYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);


    StatusInt status = SUCCESS;

    double X;
    double Y;

    double Denominator;

    double ByProdY1;
    double ByProdY2;

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        ByProdY1 = ((Y * m_D01) + m_D02);
        ByProdY2 = ((Y * m_D11) + m_D12);

        Denominator = (m_D20 * X) + (m_D21 * Y) + m_D22;

        HASSERT(Denominator != 0.0);

        // Note that even though a coordinate is invalid we continue with others.
        if (0.0L == Denominator)
            status = ERROR;
        else
            {
            pio_aXInOut[i] = ((X * m_D00) + ByProdY1) / Denominator;
            pio_aYInOut[i] = ((X * m_D10) + ByProdY2) / Denominator;
            }
        }

    return status;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertDirect(double   pi_XIn,
                                         double   pi_YIn,
                                         double*  po_pXOut,
                                         double*  po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Compute the denominator part
    double Denominator = (m_D20 * pi_XIn) + (m_D21 * pi_YIn) + m_D22;

    // The denominator must be different from 0.0
    HASSERT(Denominator != 0.0);

    if (0.0L == Denominator)
        return ERROR;

    // Transform coordinates
    *po_pXOut = ((pi_XIn * m_D00) + (pi_YIn * m_D01) + m_D02) / Denominator;
    *po_pYOut = ((pi_XIn * m_D10) + (pi_YIn * m_D11) + m_D12) / Denominator;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertInverse(double* pio_pXInOut,
                                          double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    // Compute the denominator part
    double Denominator = (m_I20 * X) + (m_I21 * Y) + m_I22;

    // The denominator must be different from 0.0
    HASSERT(Denominator != 0.0);

    if (0.0L == Denominator)
        return ERROR;

    // Transform coordinates
    *pio_pXInOut = ((X * m_I00) + (Y * m_I01) + m_I02) / Denominator;
    *pio_pYInOut = ((X * m_I10) + (Y * m_I11) + m_I12) / Denominator;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertInverse(double    pi_YIn,
                                          double    pi_XInStart,
                                          size_t    pi_NumLoc,
                                          double    pi_XInStep,
                                          double*   po_pXOut,
                                          double*   po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    StatusInt status = SUCCESS;

    uint32_t Index;
    double X;

    // Compute the denominator part
    double Denominator;
    double PartialDenominator = (m_I21 * pi_YIn) + m_I22;

    // Precalculate the Y components
    double ByProdY1 = ((pi_YIn * m_I01) + m_I02);
    double ByProdY2 = ((pi_YIn * m_I11) + m_I12);

    for (Index = 0, X = pi_XInStart; Index < pi_NumLoc ; ++Index, X+=pi_XInStep)
        {
        Denominator = (m_I20 * X) + PartialDenominator;

        HASSERT(Denominator != 0.0);

        // If a single coordinate is on vanishing line we willr eturn error yet convert all other 
        // valid coordinates.
        if (0.0L == Denominator)
            status = ERROR;
        else
            {
            po_pXOut[Index] = ((X * m_I00) + ByProdY1) / Denominator;
            po_pYOut[Index] = ((X * m_I10) + ByProdY2) / Denominator;
            }
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertInverse(size_t    pi_NumLoc,
                                          double*   pio_aXInOut,
                                          double*   pio_aYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    double X;
    double Y;

    double Denominator;

    double ByProdY1;
    double ByProdY2;

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        Denominator = (m_I20 * X) + (m_I21 * Y) + m_I22;

        HASSERT(Denominator != 0.0);

        ByProdY1 = ((Y * m_I01) + m_I02);
        ByProdY2 = ((Y * m_I11) + m_I12);

        // If a single coordinate is on vanishing line we will return error yet convert all other 
        // valid coordinates.
        if (0.0L == Denominator)
            status = ERROR;
        else
            {
            pio_aXInOut[i] = ((X * m_I00) + ByProdY1) / Denominator;
            pio_aYInOut[i] = ((X * m_I10) + ByProdY2) / Denominator;
            }
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjective::ConvertInverse(double  pi_XIn,
                                          double  pi_YIn,
                                          double* po_pXOut,
                                          double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Compute the denominator part
    double Denominator = (m_I20 * pi_XIn) + (m_I21 * pi_YIn) + m_I22;

    // The denominator must be different from 0.0
    HASSERT(Denominator != 0.0);

    if (0.0L == Denominator)
        return ERROR;

    // Transform coordinates
    *po_pXOut = ((pi_XIn * m_I00) + (pi_YIn * m_I01) + m_I02) / Denominator;
    *po_pYOut = ((pi_XIn * m_I10) + (pi_YIn * m_I11) + m_I12) / Denominator;

    return SUCCESS;
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DProjective::PreservesLinearity () const
    {
    HINVARIANTS;

    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DProjective::PreservesParallelism() const
    {
    HINVARIANTS;

    // A projective preserves linearity if perspective parameters
    // are null
    return (m_D20 == 0.0 && m_D21 == 0.0);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DProjective::PreservesShape() const
    {
    HINVARIANTS;

    // Shape is preserved only if there is no anisotropic scaling implied
    // and no anorthogonality
    return ((m_D00 == m_D11) &&
            (m_D20 == 0.0 && m_D21 == 0.0));
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DProjective::PreservesDirection() const
    {
    HINVARIANTS;

    // Direction is preserved only if there is no anisotropic scaling implied
    // no rotation nor anorthogonality
    return ((m_D00 == m_D11) &&
            (m_D10 == 0.0 && m_D01 == 0.0) &&
            (m_D20 == 0.0 && m_D21 == 0.0));
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DProjective::CanBeRepresentedByAMatrix() const
    {
    HINVARIANTS;

    return(true);
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DProjective::IsIdentity () const
    {
    HINVARIANTS;

    return((m_D00 == 1.0) && (m_D01 == 0.0) && (m_D02 == 0.0) &&
           (m_D10 == 0.0) && (m_D11 == 1.0) && (m_D12 == 0.0) &&
           (m_D20 == 0.0) && (m_D21 == 0.0) && (m_D22 == 1.0));
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DProjective::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    // There must be no rotation nor shearing
    // There must be no perspective
    return(HDOUBLE_EQUAL(GetRotation(), 0.0, pi_AngleTolerance) &&
           HDOUBLE_EQUAL(GetAnorthogonality(), 0.0, pi_AngleTolerance) &&
           (m_D20 == 0.0) && (m_D21 == 0.0) &&
           (m_D22 == 1.0));
    }


//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DProjective::GetStretchParams (double* po_pScaleFactorX,
                                        double* po_pScaleFactorY,
                                        HGF2DDisplacement* po_pDisplacement) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);


    *po_pScaleFactorX = GetXScaling();
    *po_pScaleFactorY = GetYScaling();

    po_pDisplacement->SetDeltaX(m_D02);
    po_pDisplacement->SetDeltaY(m_D12);
    }

/** -----------------------------------------------------------------------------
    This method extracts the angle of the rotation component of the model.

    @return The rotation component of the model.

    @code
        HGF2DProjective MyModel;
        double          MyRotationAngle = 13.5;
        MyModel.AddRotation(MyRotationAngle);

        double TheAngleOfRotation = MyModel.GetRotation ();
    @end

    @see AddRotation()
    -----------------------------------------------------------------------------
*/
double HGF2DProjective::GetRotation () const
    {
    HINVARIANTS;

    double MyRotation;

//HChk AR ???? should make fHGFAntan2 work
    if (m_D00 != 0.0)
        MyRotation = atan(m_D10/m_D00);
    else
        MyRotation = PI/2;

    return(MyRotation);
    }

/** -----------------------------------------------------------------------------
    This method extracts the angle of the anorthogonality component of the model.

    @return A constant reference to the anorthogonality component of the model.
            This value is between - PI/2 and  PI/2 (in radians) exclusively.

    @code
        HGF2DProjective MyModel;
        ...
        double TheAngleOfAnortho = MyModel.GetAnorthogonality();
    @end

    -----------------------------------------------------------------------------
*/
double HGF2DProjective::GetAnorthogonality() const
    {
    HINVARIANTS;

    double        MyAnorthogonality;
    double        Rotation = GetRotation();

//HChk AR ???? should make fHGFAntan2 work
    if (m_D11 != 0.0)
        MyAnorthogonality = atan(-m_D01/m_D11) - Rotation;
    else
        MyAnorthogonality = PI/2 - Rotation;

    // Normalize anorthogonality
    // This makes sure that the anorthogonality is in the range -PI/2 to PI/2
    // Addition or substraction of PI simply results in reversal of the sign of
    // the Y scaling

    // Obtain normalized value
    double NormAnortho = CalculateNormalizedTrigoValue(MyAnorthogonality);

    // Precalculate right angle (optimization)
    double RightAngle = PI / 2;

    if ((NormAnortho >= RightAngle) && (NormAnortho <= 3*RightAngle))
        MyAnorthogonality -= PI;

    return(MyAnorthogonality);
    }

/** -----------------------------------------------------------------------------
    This method extracts the X scaling components of the model.

    @return The X scaling factor

    @code
        HGF2DProjective MyModel;
        MyModel.AddIsotropicScaling(12.3);
        ...
        double ScaleFactorX = MyModel.GetXScaling ();
    @end

    @see GetYScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
double HGF2DProjective::GetXScaling() const
    {
    HINVARIANTS;

    double         ScaleX;

    double         MyRotation = GetRotation();

    // Compute X scale
    double     MyCos = cos(MyRotation);
    double     MySin = sin(MyRotation);

    /*
    ** Take the largest value of the Sine or the Cosine of the angle to avoid
    ** dividing by zero.  Both values for MySin and MyCos cannot be both zero,
    ** since sin(a)*sin(a) + cos(a)*cos(a) = 1 for all values of a.
    ** Taking the largest value ensures we stay away from a division by zero, and
    ** as a bonus increases the precision of the result.
    */
    if (fabs(MyCos) > fabs(MySin))
        ScaleX = m_D00 / MyCos;
    else
        ScaleX = m_D10 / MySin;

    return(ScaleX);
    }


/** -----------------------------------------------------------------------------
    This method extracts the Y scaling components of the model.

    @return The Y scaling factor

    @code
        HGF2DProjective MyModel;
        MyModel.AddIsotropicScaling(12.3);

        double ScaleFactorY = MyModel.GetYScaling ();
    @end

    @see GetXScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
double HGF2DProjective::GetYScaling() const
    {
    HINVARIANTS;

    double         ScaleY;

    double        MyRotation = GetRotation();
    double        MyAnorthogonality = GetAnorthogonality();

    // Compute Y scale
    double     MyCos = cos(MyRotation + MyAnorthogonality);
    double     MySin = sin(MyRotation + MyAnorthogonality);

    /*
    ** Take the largest value of the Sine or the Cosine of the angle to avoid
    ** dividing by zero.  Both values for MySin and MyCos cannot be both zero,
    ** since sin(a)*sin(a) + cos(a)*cos(a) = 1 for all values of a, and that
    ** both MySin and MyCos values are added with the same value.
    ** Taking the largest value ensures we stay away from a division by zero, and
    ** as a bonus increases the precision of the result.
    */
    if (fabs(MyCos) > fabs(MySin))
        ScaleY = m_D11 / MyCos;
    else
        ScaleY = - m_D01 / MySin;

    return(ScaleY);

    }


/** -----------------------------------------------------------------------------
    This method sets the transformation model by giving the matrix or equation
    parameters.

    @param pi_rMatrix IN A properly configured matrix containing the transformation
                         data.

    @code
        HGF2DProjective     MyModel;
        HFCMatrix<3, 3>     MyMatrix;
        MyMatrix[0][0] = 1.0;
        MyMatrix[0][0] = 0.0;
        MyMatrix[0][2] = 10.0;
        MyMatrix[1][0] = 0.0;
        MyMatrix[1][1] = 1.0;
        MyMatrix[1][2] = 10.0;
        MyMatrix[2][2] = 1.0;

        MyModel.SetByMatrix(MyMatrix);
    @end

    @see GetMatrix()
    -----------------------------------------------------------------------------
*/
void HGF2DProjective::SetByMatrix(const HFCMatrix<3, 3>& pi_rMatrix)
    {
    HINVARIANTS;

    // These are the conditions that prevent any scale factor of being 0, or
    // the anorthogonality of being equal to PI/2 or -PI/2
//HChk AR ????? Cannot implement determinant calculations yet
//    HPRECONDITION(pi_rMatrix.CalculateDeterminant() != 0.0);

    HPRECONDITION(pi_rMatrix[2][2] != 0);

    // Copy matrix

    m_D00 = pi_rMatrix[0][0] / pi_rMatrix[2][2];
    m_D01 = pi_rMatrix[0][1] / pi_rMatrix[2][2];
    m_D02 = pi_rMatrix[0][2] / pi_rMatrix[2][2];
    m_D10 = pi_rMatrix[1][0] / pi_rMatrix[2][2];
    m_D11 = pi_rMatrix[1][1] / pi_rMatrix[2][2];
    m_D12 = pi_rMatrix[1][2] / pi_rMatrix[2][2];
    m_D20 = pi_rMatrix[2][0] / pi_rMatrix[2][2];
    m_D21 = pi_rMatrix[2][1] / pi_rMatrix[2][2];
    m_D22 = 1.0;


    // Compute prepared settings
    Prepare ();

    // Check state again
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DProjective::GetMatrix() const
    {
    HINVARIANTS;

    HFCMatrix<3, 3> Matrix;

    Matrix[0][0] = m_D00;
    Matrix[0][1] = m_D01;
    Matrix[0][2] = m_D02;
    Matrix[1][0] = m_D10;
    Matrix[1][1] = m_D11;
    Matrix[1][2] = m_D12;
    Matrix[2][0] = m_D20;
    Matrix[2][1] = m_D21;
    Matrix[2][2] = m_D22;

    return(Matrix);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DProjective::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    HINVARIANTS;

    po_rRecipient[0][0] = m_D00;
    po_rRecipient[0][1] = m_D01;
    po_rRecipient[0][2] = m_D02;
    po_rRecipient[1][0] = m_D10;
    po_rRecipient[1][1] = m_D11;
    po_rRecipient[1][2] = m_D12;
    po_rRecipient[2][0] = m_D20;
    po_rRecipient[2][1] = m_D21;
    po_rRecipient[2][2] = m_D22;

    return(po_rRecipient);
    }

/** -----------------------------------------------------------------------------
    This method adds the specified translation component to the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing the
                              translation component to add to the model.

    @code
        HGF2DProjective     MyModel;
        HGF2DDisplacement   Translation (10.2, 20.0);

        MyModel.AddTranslation  (Translation);
    @end

    @see GetTranslation()
    @see AddRotation()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DProjective::AddTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    HINVARIANTS;

    // Set translation
    m_D02 += pi_rTranslation.GetDeltaX();
    m_D12 += pi_rTranslation.GetDeltaY();

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
        HGF2DProjective MyModel;
        double          MyRotationAngle = 13.5;

        MyModel.AddRotation(MyRotationAngle, 10.0, 10.0);
    @end


    @see GetRotation()
    @see AddTranslation()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DProjective::AddRotation(double pi_Angle,
                                  double pi_rXCenter,
                                  double pi_rYCenter)
    {
    HINVARIANTS;

    // Obtain matrix
    HFCMatrix<3, 3> AddedMatrix;

    // Calculate sin and cos
    double MySin = sin(pi_Angle);
    double MyCos = cos(pi_Angle);

    // Set matrix to represent rotation
    AddedMatrix[0][0] = MyCos;
    AddedMatrix[0][1] = -MySin;
    AddedMatrix[0][2] = pi_rXCenter +
                        pi_rYCenter * MySin -
                        pi_rXCenter * MyCos;
    AddedMatrix[1][0] = MySin;
    AddedMatrix[1][1] = MyCos;
    AddedMatrix[1][2] = pi_rYCenter -
                        pi_rYCenter * MyCos -
                        pi_rXCenter * MySin;
    AddedMatrix[2][0] = 0.0;
    AddedMatrix[2][1] = 0.0;
    AddedMatrix[2][2] = 1.0;

    // Get current matrix
    HFCMatrix<3, 3> CurrentMatrix = GetMatrix();

    // Add Rotation
    HFCMatrix<3, 3> NewMatrix = AddedMatrix * CurrentMatrix;

    SetByMatrix(NewMatrix);
    }

/** -----------------------------------------------------------------------------
    This method adds an isotropic scaling around a specified center of scaling
    to the current components of the model.
    The center is expressed as raw numbers that will be interpreted in the
    X and Y units of the direct channels of the transformation model.
    If center omitted, then the origin is used.
    Remember that when specifying a center different than the origin, a
    translation component is also added to the model.

    @param pi_Scaler IN The scaling factor of the scaling component to add
                        to the model. This factor must be different from 0.0.

    @param pi_XCenter IN OPTIONAL The X value of the center of the scaling to
                         add, that will be interpreted in the X direct channel units.

    @param pi_YCenter IN OPTIONAL The Y value of the center of the scaling to
                         add, that will be interpreted in the Y direct channel units.

    @code
        HGF2DProjective MyModel;
        MyModel.AddIsotropicScaling (34.5, 2.0, 3.0);
    @end

    @see GetXScaling()
    @see GetYScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DProjective::AddIsotropicScaling (double pi_Scale,
                                           double pi_XCenter,
                                           double pi_YCenter)
    {
    HINVARIANTS;

    HPRECONDITION(pi_Scale != 0.0);

    // Obtain matrix
    HFCMatrix<3, 3> AddedMatrix;

    // Set matrix to represent rotation
    AddedMatrix[0][0] = pi_Scale;
    AddedMatrix[0][1] = 0.0;
    AddedMatrix[0][2] = pi_XCenter - pi_XCenter * pi_Scale;
    AddedMatrix[1][0] = 0.0;
    AddedMatrix[1][1] = pi_Scale;
    AddedMatrix[1][2] = pi_YCenter - pi_YCenter * pi_Scale;
    AddedMatrix[2][0] = 0.0;
    AddedMatrix[2][1] = 0.0;
    AddedMatrix[2][2] = 1.0;

    // Get current matrix
    HFCMatrix<3, 3> CurrentMatrix = GetMatrix();

    // Add Scaling
    HFCMatrix<3, 3> NewMatrix = AddedMatrix * CurrentMatrix;

    SetByMatrix(NewMatrix);
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
void HGF2DProjective::AddAnisotropicScaling(double pi_ScaleX,
                                            double pi_ScaleY,
                                            double pi_XCenter,
                                            double pi_YCenter)
    {
    HINVARIANTS;

    HPRECONDITION(pi_ScaleX != 0.0);
    HPRECONDITION(pi_ScaleY != 0.0);

    // Obtain matrix
    HFCMatrix<3, 3> AddedMatrix;

    // Set matrix to represent rotation
    AddedMatrix[0][0] = pi_ScaleX;
    AddedMatrix[0][1] = 0.0;
    AddedMatrix[0][2] = pi_XCenter - pi_XCenter * pi_ScaleX;
    AddedMatrix[1][0] = 0.0;
    AddedMatrix[1][1] = pi_ScaleY;
    AddedMatrix[1][2] = pi_YCenter - pi_YCenter * pi_ScaleY;
    AddedMatrix[2][0] = 0.0;
    AddedMatrix[2][1] = 0.0;
    AddedMatrix[2][2] = 1.0;

    // Get current matrix
    HFCMatrix<3, 3> CurrentMatrix = GetMatrix();

    // Add Scaling
    HFCMatrix<3, 3> NewMatrix = AddedMatrix * CurrentMatrix;

    SetByMatrix(NewMatrix);
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

    @param pi_RawXMirrorPos IN OPTIONAL The X value of the mirror position of the flip
                            to add, that will be interpreted in the X direct
                            channel units.


    @code
        HGF2DProjective MyModel;

        MyModel.AddHorizontalFlip (10.0);
    @end

    @see AddVerticalFlip()
    -----------------------------------------------------------------------------
*/
void HGF2DProjective::AddHorizontalFlip (double pi_rXMirrorPos)
    {
    HINVARIANTS;

    // Obtain matrix
    HFCMatrix<3, 3> AddedMatrix;

    // Set matrix to represent mirror
    AddedMatrix[0][0] = -1.0;
    AddedMatrix[0][1] = 0.0;
    AddedMatrix[0][2] = 2 * pi_rXMirrorPos;
    AddedMatrix[1][0] = 0.0;
    AddedMatrix[1][1] = 1.0;
    AddedMatrix[1][2] = 0.0;
    AddedMatrix[2][0] = 0.0;
    AddedMatrix[2][1] = 0.0;
    AddedMatrix[2][2] = 1.0;

    // Get current matrix
    HFCMatrix<3, 3> CurrentMatrix = GetMatrix();

    // Add Flip
    HFCMatrix<3, 3> NewMatrix = AddedMatrix * CurrentMatrix;

    SetByMatrix(NewMatrix);
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

    @param pi_YMirrorPos IN OPTIONAL The Y value of the mirror position of the flip
                            to add, that will be interpreted in the Y direct
                            channel units.

    @code
        HGF2DProjective MyModel;

        MyModel.AddVerticalFlip (10.0);
    @end

    @see AddHorizontalFlip()
    -----------------------------------------------------------------------------
*/
void HGF2DProjective::AddVerticalFlip (double pi_YMirrorPos)
    {
    HINVARIANTS;

    // Obtain matrix
    HFCMatrix<3,3> AddedMatrix;

    // Set matrix to represent mirror
    AddedMatrix[0][0] = 1.0;
    AddedMatrix[0][1] = 0.0;
    AddedMatrix[0][2] = 0.0;
    AddedMatrix[1][0] = 0.0;
    AddedMatrix[1][1] = -1.0;
    AddedMatrix[1][2] = (2 * pi_YMirrorPos);
    AddedMatrix[2][0] = 0.0;
    AddedMatrix[2][1] = 0.0;
    AddedMatrix[2][2] = 1.0;

    // Get current matrix
    HFCMatrix<3, 3> CurrentMatrix = GetMatrix();

    // Add flip
    HFCMatrix<3, 3> NewMatrix = AddedMatrix * CurrentMatrix;

    SetByMatrix(NewMatrix);
    }



//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DProjective::Reverse()
    {
    HINVARIANTS;

    // Copy other parameters

    // Compute inverse matrix parameters (The adjoint)
    double I00 = (m_D11 * m_D22 - m_D12 * m_D21);
    double I01 = (m_D02 * m_D21 - m_D01 * m_D22);
    double I02 = (m_D01 * m_D12 - m_D02 * m_D11);
    double I10 = (m_D12 * m_D20 - m_D10 * m_D22);
    double I11 = (m_D00 * m_D22 - m_D02 * m_D20);
    double I12 = (m_D02 * m_D10 - m_D00 * m_D12);
    double I20 = (m_D10 * m_D21 - m_D11 * m_D20);
    double I21 = (m_D01 * m_D20 - m_D00 * m_D21);
    double I22 = (m_D00 * m_D11 - m_D01 * m_D10);

    // Copy to master data

    m_D00 = I00 / I22;
    m_D01 = I01 / I22;
    m_D02 = I02 / I22;
    m_D10 = I10 / I22;
    m_D11 = I11 / I22;
    m_D12 = I12 / I22;
    m_D20 = I20 / I22;
    m_D21 = I21 / I22;
    m_D22 = I22 / I22;

    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModel::Reverse();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjective::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;


    // Check if the type of the given model can be represented by a matrix
    if (pi_rModel.CanBeRepresentedByAMatrix())
        {
        HFCMatrix<3, 3> MyGivenMatrix(pi_rModel.GetMatrix());

        // Compose the two matrix together
        pResultModel = new HGF2DProjective();

        ((HGF2DProjective*)(&(*pResultModel)))->SetByMatrix(MyGivenMatrix * GetMatrix());

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
HGF2DTransfoModel* HGF2DProjective::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return (new HGF2DProjective (*this));
    }



//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjective::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsIdentity())
        {
        pResultModel = new HGF2DProjective(*this);
        }

    // Check if the type of the given model can be represented by a matrix
    else if (pi_rModel.CanBeRepresentedByAMatrix())
        {
        HFCMatrix<3, 3> MySelfMatrix(GetMatrix());

        // Compose the two matrix together
        pResultModel = new HGF2DProjective();

        ((HGF2DProjective*)(&(*pResultModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());

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
void HGF2DProjective::Prepare ()
    {

    // Compute inverse matrix parameters (The adjoint)
    // Calculate global scale
    double GlobalInverseScale = (m_D00 * m_D11 - m_D01 * m_D10);

    // Determinant must be different than 0.0
    HASSERT (GlobalInverseScale != 0.0);

    // This condition will simply prevent crashes that unfortunately occur. Normally this condition cannot
    // occur since provided parameters have already been tested ... but due to computation errors, it occurs
    // We just add this test to bypass such fortuitous errors.
    if (GlobalInverseScale != 0.0)
        {
        m_I00 = (m_D11 * m_D22 - m_D12 * m_D21) / GlobalInverseScale;
        m_I01 = (m_D02 * m_D21 - m_D01 * m_D22) / GlobalInverseScale;
        m_I02 = (m_D01 * m_D12 - m_D02 * m_D11) / GlobalInverseScale;
        m_I10 = (m_D12 * m_D20 - m_D10 * m_D22) / GlobalInverseScale;
        m_I11 = (m_D00 * m_D22 - m_D02 * m_D20) / GlobalInverseScale;
        m_I12 = (m_D02 * m_D10 - m_D00 * m_D12) / GlobalInverseScale;
        m_I20 = (m_D10 * m_D21 - m_D11 * m_D20) / GlobalInverseScale;
        m_I21 = (m_D01 * m_D20 - m_D00 * m_D21) / GlobalInverseScale;
        m_I22 = 1.0;
        }
    }


//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DProjective::Copy(const HGF2DProjective& pi_rObj)
    {
    // Copy master data
    m_D00 = pi_rObj.m_D00;
    m_D01 = pi_rObj.m_D01;
    m_D02 = pi_rObj.m_D02;
    m_D10 = pi_rObj.m_D10;
    m_D11 = pi_rObj.m_D11;
    m_D12 = pi_rObj.m_D12;
    m_D20 = pi_rObj.m_D20;
    m_D21 = pi_rObj.m_D21;
    m_D22 = pi_rObj.m_D22;

    // Copy inverse matrix
    m_I00 = pi_rObj.m_I00;
    m_I01 = pi_rObj.m_I01;
    m_I02 = pi_rObj.m_I02;
    m_I10 = pi_rObj.m_I10;
    m_I11 = pi_rObj.m_I11;
    m_I12 = pi_rObj.m_I12;
    m_I20 = pi_rObj.m_I20;
    m_I21 = pi_rObj.m_I21;
    m_I22 = pi_rObj.m_I22;
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DProjective::CreateSimplifiedModel() const
    {
    HINVARIANTS;

    // Declare recipient variable
    HFCPtr<HGF2DTransfoModel> pSimplifiedModel;

    // In order to be simplified at all, projection parameters must be null
    if (m_D20 == 0.0 && m_D21 == 0.0)
        {
        // No perspective ... surely can be simplified

        // Check if any rotation and anorthogonality can be detected
        if (m_D10 == 0.0 && m_D01 == 0.0)
            {
            // No rotation neither anorthogonality
            // The model is at worst a stretch

            // Check if scaling factors are unitary
            if ((m_D00 == 1.0) && (m_D11 == 1.0))
                {
                // No scaling

                // Check if there are any translation
                if (m_D02 == 0.0 && m_D12 == 0.0)
                    {
                    // No translation

                    // Therefore the model is an identity since it describes no
                    // transformation. Only unit factors are preserved
                    pSimplifiedModel = new HGF2DIdentity();
                    }
                else
                    {
                    // There is only a translation
                    pSimplifiedModel = new HGF2DTranslation(GetTranslation());

                    }
                }
            else
                {
                // Scaling and translation are present

                pSimplifiedModel = new HGF2DStretch(GetTranslation(), GetXScaling(), GetYScaling());
                }
            }
        else
            {
            // Rotation and / or anorthogonality is present

            // Check if there are any anorthogonality or if scaling is isotropic
            // In order to do so, the [0,0] and [1,1] factor must be equal
            // and the [0,1] and [1,0] must be of inverse sign
            if ((m_D00 == m_D11) && (m_D10 == -m_D01))
                {
                // Isotropic scaling and no arthogonality
                // we have and affine

                pSimplifiedModel = new HGF2DSimilitude(GetTranslation(), GetRotation(), GetXScaling());
                }
            else
                {
                // Anisotropic

                pSimplifiedModel = new HGF2DAffine(GetTranslation(),
                                                   GetRotation(),
                                                   GetXScaling(),
                                                   GetYScaling(),
                                                   GetAnorthogonality());

                }
            }
        }

    // If we get here, no simplification is possible.
    return pSimplifiedModel;
    }
