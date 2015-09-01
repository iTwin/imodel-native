//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFPolynomialModelAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGFPolynomialModelAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
#include <Imagepp/all/h/HGF2DTransfoModelAdapter.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DPosition.h>
#include <Geom/GeomApi.h>

#include <Imagepp/all/h/HGFPolynomialModelAdapter.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HGFPolynomialModelAdapter::HGFPolynomialModelAdapter(const HGF2DTransfoModel&  pi_ExactTransfoModel, const HGF2DShape& pi_rApplicationArea,
                                                     double pi_StepX, double pi_StepY, bool pi_UsePolynomialForDirect)
    : HGF2DTransfoModelAdapter(pi_ExactTransfoModel)
    {
    // The width nor the height of extent may be null
    HPRECONDITION(pi_rApplicationArea.GetExtent().GetWidth() > 0.0);
    HPRECONDITION(pi_rApplicationArea.GetExtent().GetHeight() > 0.0);

    HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rApplicationArea.GetExtent().GetWidth(), (pi_StepX * 10));
    HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rApplicationArea.GetExtent().GetHeight(), (pi_StepY * 10));
    m_ApplicationArea = HFCPtr<HGF2DShape>(static_cast<HGF2DShape*>(pi_rApplicationArea.Clone()));
    m_StepX = pi_StepX;
    m_StepY = pi_StepY;
    m_UsePolynomialForDirect = pi_UsePolynomialForDirect;
    m_EnoughTiePoints = false;

    CalculatePolynomialCoefficients(*m_ApplicationArea, pi_StepX, pi_StepY);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HGFPolynomialModelAdapter::HGFPolynomialModelAdapter(const HGFPolynomialModelAdapter& pi_rObj)
    : HGF2DTransfoModelAdapter(pi_rObj)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HGFPolynomialModelAdapter::~HGFPolynomialModelAdapter()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HGFPolynomialModelAdapter& HGFPolynomialModelAdapter::operator=(const HGFPolynomialModelAdapter& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModelAdapter::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGFPolynomialModelAdapter::IsConvertDirectThreadSafe() const
    {
    // The polynomial is thread safe, but not the transform that uses gcoord
    return m_UsePolynomialForDirect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGFPolynomialModelAdapter::IsConvertInverseThreadSafe() const
    {
    // The polynomial is thread safe, but not the transform that uses gcoord
    return !m_UsePolynomialForDirect;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const
    {
    ConvertDirect(*pio_pXInOut, *pio_pYInOut, pio_pXInOut, pio_pYInOut);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut,
                                                   double* po_pYOut) const
    {
    if (m_UsePolynomialForDirect)
        {
        TransposePointsUsingPolynomial(pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_pXOut, po_pYOut);
        }
    else
        {
        uint32_t Index;
        double X;

        for (Index = 0, X = pi_XInStart; Index < pi_NumLoc ; ++Index, X+=pi_XInStep)
            {
            m_pAdaptedTransfoModel->ConvertDirect(X, pi_YIn, po_pXOut + Index, po_pYOut + Index);
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    if (m_UsePolynomialForDirect)
        {
        TransposePointUsingPolynomial(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    else
        {
        m_pAdaptedTransfoModel->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        ConvertDirect(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);
        }
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const
    {
    ConvertInverse(*pio_pXInOut, *pio_pYInOut, pio_pXInOut, pio_pYInOut);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut,
                                                    double* po_pYOut) const
    {
    if (!m_UsePolynomialForDirect)
        {
        TransposePointsUsingPolynomial(pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_pXOut, po_pYOut);
        }
    else
        {
        uint32_t Index;
        double X;

        for (Index = 0, X = pi_XInStart; Index < pi_NumLoc ; ++Index, X+=pi_XInStep)
            {
            m_pAdaptedTransfoModel->ConvertInverse(X, pi_YIn, po_pXOut + Index, po_pYOut + Index);
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    if (!m_UsePolynomialForDirect)
        {
        TransposePointUsingPolynomial(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    else
        {
        m_pAdaptedTransfoModel->ConvertInverse(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGFPolynomialModelAdapter::ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        ConvertInverse(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);
        }
    return SUCCESS;
    }

inline void HGFPolynomialModelAdapter::TransposePointsUsingPolynomial(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep,
                                                                      double* po_pXOut, double* po_pYOut) const
    {
    //Pre-compute some factors that are the same for every pixel
    double y = pi_YIn;
    double y2 = y * y;
    double y3 = y2 * y;

    double a2_x       = m_CoefficientsX[2] * y;
    double partial4_x = m_CoefficientsX[4] * y;
    double a5_x       = m_CoefficientsX[5] * y2;
    double partial7_x = m_CoefficientsX[7] * y;
    double partial8_x = m_CoefficientsX[8] * y2;
    double a9_x       = m_CoefficientsX[9] * y3;

    double a2_y       = m_CoefficientsY[2] * y;
    double partial4_y = m_CoefficientsY[4] * y;
    double a5_y       = m_CoefficientsY[5] * y2;
    double partial7_y = m_CoefficientsY[7] * y;
    double partial8_y = m_CoefficientsY[8] * y2;
    double a9_y       = m_CoefficientsY[9] * y3;


    uint32_t index;
    double x;
    double x2;
    double x3;
    for (index = 0, x = pi_XInStart; index < pi_NumLoc ; ++index, x+=pi_XInStep)
        {
        x2 = x * x;
        x3 = x2 * x;

        *(po_pXOut + index) = m_CoefficientsX[0]
                            + m_CoefficientsX[1] * x
                            + a2_x
                            + m_CoefficientsX[3] * x2
                            + partial4_x * x
                            + a5_x
                            + m_CoefficientsX[6] * x3
                            + partial7_x * x2
                            + partial8_x * x
                            + a9_x;

        *(po_pYOut + index) = m_CoefficientsY[0]
                            + m_CoefficientsY[1] * x
                            + a2_y
                            + m_CoefficientsY[3] * x2
                            + partial4_y * x
                            + a5_y
                            + m_CoefficientsY[6] * x3
                            + partial7_y * x2
                            + partial8_y * x
                            + a9_y;
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
inline void HGFPolynomialModelAdapter::TransposePointUsingPolynomial(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    double x = pi_XIn;
    double y = pi_YIn;
    double xy = x * y;
    double x2 = x * x;
    double y2 = y * y;
    double x2y = x2 * y;
    double xy2 = x * y2;
    double x3 = x2 * x;
    double y3 = y2 * y;

    *po_pXOut = m_CoefficientsX[0]
              + m_CoefficientsX[1] * x
              + m_CoefficientsX[2] * y
              + m_CoefficientsX[3] * x2
              + m_CoefficientsX[4] * xy
              + m_CoefficientsX[5] * y2
              + m_CoefficientsX[6] * x3
              + m_CoefficientsX[7] * x2y
              + m_CoefficientsX[8] * xy2
              + m_CoefficientsX[9] * y3;

    *po_pYOut = m_CoefficientsY[0]
              + m_CoefficientsY[1] * x
              + m_CoefficientsY[2] * y
              + m_CoefficientsY[3] * x2
              + m_CoefficientsY[4] * xy
              + m_CoefficientsY[5] * y2
              + m_CoefficientsY[6] * x3
              + m_CoefficientsY[7] * x2y
              + m_CoefficientsY[8] * xy2
              + m_CoefficientsY[9] * y3;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void HGFPolynomialModelAdapter::Copy (const HGFPolynomialModelAdapter& pi_rObj)
    {
    m_StepX = pi_rObj.m_StepX;
    m_StepY = pi_rObj.m_StepY;
    m_ApplicationArea = pi_rObj.m_ApplicationArea;
    m_UsePolynomialForDirect = pi_rObj.m_UsePolynomialForDirect;
    for(uint32_t i = 0; i < NUMBER_OF_COEFFICIENTS; i++)
        {
        m_CoefficientsX[i] = pi_rObj.m_CoefficientsX[i];
        m_CoefficientsY[i] = pi_rObj.m_CoefficientsY[i];
        }
    m_EnoughTiePoints = pi_rObj.m_EnoughTiePoints;
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGFPolynomialModelAdapter::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return(new HGFPolynomialModelAdapter(*this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void HGFPolynomialModelAdapter::Reverse ()
    {
    HGF2DTransfoModelAdapter::Reverse();
    m_UsePolynomialForDirect = !m_UsePolynomialForDirect;
    }

//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGFPolynomialModelAdapter::GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, HGF2DDisplacement* po_pDisplacement) const
    {
    HGF2DTransfoModel::GetStretchParamsAt(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement, 0.0, 0.0);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the affine by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGFPolynomialModelAdapter::GetMatrix() const
    {
    HFCMatrix<3, 3> m;
    return m;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void InitializeLinearAlgebraProblem(RowMajorMatrix& matrixX, RowMajorMatrix& vectorRightHandSideX,
                                    RowMajorMatrix& matrixY, RowMajorMatrix& vectorRightHandSideY, double* pTiePointsSrc,
                                    double* pTiePointsDest, uint32_t numberOfPoints, bool reverse)
    {
    double* pPolyFunctionDomainPoints;
    double* pPolyFunctionCodomainPoints;

    if (reverse)
        {
        pPolyFunctionDomainPoints = pTiePointsDest;
        pPolyFunctionCodomainPoints = pTiePointsSrc;
        }
    else
        {
        pPolyFunctionDomainPoints = pTiePointsSrc;
        pPolyFunctionCodomainPoints = pTiePointsDest;
        }


    double xDom;
    double yDom;
    double xCodom;
    double yCodom;

    for (uint32_t point = 0; point < numberOfPoints; point++)
        {
        xDom = pPolyFunctionDomainPoints[point * 2];
        yDom = pPolyFunctionDomainPoints[point * 2 + 1];
        xCodom = pPolyFunctionCodomainPoints[point * 2];
        yCodom = pPolyFunctionCodomainPoints[point * 2 + 1];

        uint32_t col = 0;
        for (uint32_t j = 0; j <= POLYNOMIAL_DEGREE; j++)
            {
            for (uint32_t i = 0; i <= j; i++)
                {
                double cell = pow(xDom, (j - i)) * pow(yDom, i);
                matrixX.Set(point, col, cell);
                matrixY.Set(point, col, cell);
                ++col;
                }
            }
        vectorRightHandSideX.Set(point, 0, xCodom);
        vectorRightHandSideY.Set(point, 0, yCodom);
        }
    }

// -----------------------------------------------------------------------------
//  PRIVATE METHOD
//  This method performs the generation of a third degree polymial by sampling of the
//  transformation of the non-linear model.
//
//  @param pi_rShape The application area over which sampling is performed.
//
//  @param pi_StepX The sampling step used in X.
//  @param pi_StepY The sampling step used in Y.
// -----------------------------------------------------------------------------
 void HGFPolynomialModelAdapter::CalculatePolynomialCoefficients(const HGF2DShape& pi_rShape, double pi_StepX, double pi_StepY)
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_rShape.GetExtent().GetWidth() != 0.0);
    HPRECONDITION(pi_rShape.GetExtent().GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_StepX > 0.0);
    HPRECONDITION(pi_StepY > 0.0);

    // Calculate the maximum number of tie points needed (some may be in the extent but out of the shape)
    uint32_t maxNumberOfPoints = (uint32_t)(((pi_rShape.GetExtent().GetHeight() / pi_StepY) + 2) * ((pi_rShape.GetExtent().GetWidth() / pi_StepX) + 2));


    // Allocate Tie Points
    HArrayAutoPtr<double> pTiePointsSrc(new double[maxNumberOfPoints * 2]);
    HArrayAutoPtr<double> pTiePointsDest(new double[maxNumberOfPoints * 2]);

    uint32_t currentTiePointVal = 0;
    uint32_t effectiveNumberOfPoints = 0;
    double currentX;
    double currentY;
    double tempX;
    double tempY;

    for (currentY = pi_rShape.GetExtent().GetYMin() ; currentY < pi_rShape.GetExtent().GetYMax() ; currentY += pi_StepY)
        {
        for (currentX = pi_rShape.GetExtent().GetXMin() ; currentX < pi_rShape.GetExtent().GetXMax() ; currentX += pi_StepX)
            {
            if(pi_rShape.IsPointIn(HGF2DPosition(currentX, currentY)))
                {
                if(m_pAdaptedTransfoModel->ConvertInverse(currentX, currentY, &tempX, &tempY) == SUCCESS)
                    {
                    pTiePointsDest[currentTiePointVal] = currentX;
                    pTiePointsDest[currentTiePointVal + 1] = currentY;


                    pTiePointsSrc[currentTiePointVal] = tempX;
                    pTiePointsSrc[currentTiePointVal + 1] = tempY;

                    currentTiePointVal += 2;
                    effectiveNumberOfPoints++;
                    }
                }
            }
        }
    if(effectiveNumberOfPoints >= MINIMUM_NUMBER_OF_TIE_POINTS)
        m_EnoughTiePoints = true;

    // The problem of finding coefficients for the polynomial approximation function is split in two subproblems:
    // finding the coefficent vector for the x and finding the coefficient vector for y.
    RowMajorMatrix matrixOverdeterminedX(effectiveNumberOfPoints, NUMBER_OF_COEFFICIENTS);
    RowMajorMatrix matrixOverdeterminedY(effectiveNumberOfPoints, NUMBER_OF_COEFFICIENTS);
    RowMajorMatrix vectorRightHandSideOverdeterminedX(effectiveNumberOfPoints, 1);
    RowMajorMatrix vectorRightHandSideOverdeterminedY(effectiveNumberOfPoints, 1);

    InitializeLinearAlgebraProblem(matrixOverdeterminedX, vectorRightHandSideOverdeterminedX, matrixOverdeterminedY, vectorRightHandSideOverdeterminedY,
                                   pTiePointsSrc, pTiePointsDest, effectiveNumberOfPoints, !m_UsePolynomialForDirect);

    //Compute square matrix Ax and Ay
    size_t bufferSize = matrixOverdeterminedX.NumRows() * matrixOverdeterminedX.NumColumns();

    vector<double> matrixXDataBuffer(bufferSize);
    vector<double> transposedMatrixXDataBuffer(bufferSize);
    vector<double> finalMatrixXDataBuffer(NUMBER_OF_COEFFICIENTS * NUMBER_OF_COEFFICIENTS);
    vector<double> matrixYDataBuffer(bufferSize);
    vector<double> transposedMatrixYDataBuffer(bufferSize);
    vector<double> finalMatrixYDataBuffer(NUMBER_OF_COEFFICIENTS * NUMBER_OF_COEFFICIENTS);

    matrixOverdeterminedX.GetAll(matrixXDataBuffer.data(), bufferSize, false/*transpose*/);
    matrixOverdeterminedX.GetAll(transposedMatrixXDataBuffer.data(), bufferSize, true/*transpose*/);
    matrixOverdeterminedY.GetAll(matrixYDataBuffer.data(), bufferSize, false/*transpose*/);
    matrixOverdeterminedY.GetAll(transposedMatrixYDataBuffer.data(), bufferSize, true/*transpose*/);

    bsiLinAlg_multiplyDenseRowMajorMatrixMatrix(finalMatrixXDataBuffer.data(), NUMBER_OF_COEFFICIENTS, NUMBER_OF_COEFFICIENTS,
                                                transposedMatrixXDataBuffer.data(), effectiveNumberOfPoints, matrixXDataBuffer.data());
    bsiLinAlg_multiplyDenseRowMajorMatrixMatrix(finalMatrixYDataBuffer.data(), NUMBER_OF_COEFFICIENTS, NUMBER_OF_COEFFICIENTS,
                                                transposedMatrixYDataBuffer.data(), effectiveNumberOfPoints, matrixYDataBuffer.data());

    //Compute vector b
    vector<double> rightHandSideXDataBuffer(effectiveNumberOfPoints);
    vector<double> finalVectorXDataBuffer(NUMBER_OF_COEFFICIENTS);
    vector<double> rightHandSideYDataBuffer(effectiveNumberOfPoints);
    vector<double> finalVectorYDataBuffer(NUMBER_OF_COEFFICIENTS);
    vectorRightHandSideOverdeterminedX.GetAll(rightHandSideXDataBuffer.data(), effectiveNumberOfPoints, false/*transpose*/);
    vectorRightHandSideOverdeterminedY.GetAll(rightHandSideYDataBuffer.data(), effectiveNumberOfPoints, false/*transpose*/);

    bsiLinAlg_multiplyDenseRowMajorMatrixMatrix(finalVectorXDataBuffer.data(), NUMBER_OF_COEFFICIENTS, 1,
                                                transposedMatrixXDataBuffer.data(), effectiveNumberOfPoints, rightHandSideXDataBuffer.data());
    bsiLinAlg_multiplyDenseRowMajorMatrixMatrix(finalVectorYDataBuffer.data(), NUMBER_OF_COEFFICIENTS, 1,
                                                transposedMatrixYDataBuffer.data(), effectiveNumberOfPoints, rightHandSideYDataBuffer.data());

    //Rebuild RowMajorMatrix objects
    RowMajorMatrix A_x(NUMBER_OF_COEFFICIENTS, NUMBER_OF_COEFFICIENTS, finalMatrixXDataBuffer.data());
    RowMajorMatrix b_x(NUMBER_OF_COEFFICIENTS, 1, finalVectorXDataBuffer.data());
    RowMajorMatrix A_y(NUMBER_OF_COEFFICIENTS, NUMBER_OF_COEFFICIENTS, finalMatrixYDataBuffer.data());
    RowMajorMatrix b_y(NUMBER_OF_COEFFICIENTS, 1, finalVectorYDataBuffer.data());

    //Solve Ax = b
    double temp;
    LinearAlgebra::SolveInplaceGaussFullPivot(A_x, b_x, temp);
    LinearAlgebra::SolveInplaceGaussFullPivot(A_y, b_y, temp);

    //Store results
    b_x.GetAll(m_CoefficientsX, NUMBER_OF_COEFFICIENTS);
    b_y.GetAll(m_CoefficientsY, NUMBER_OF_COEFFICIENTS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HGFPolynomialModelAdapter::GetMeanError(double* po_pMeanError, double* po_pMaxError, HGF2DPosition* po_pMaxErrorPosition,
                                                  double* po_pMinError, HGF2DPosition* po_pMinErrorPosition)
    {
    *po_pMaxError = -1;
    *po_pMinError = numeric_limits<double>::max();
    double sumOfError = 0;
    double numberOfTestedPoints = 0;

    double stepX = m_StepX / 13;
    double stepY = m_StepY / 13;

    double currentX;
    double currentY;
    double currentError;
    double tempX;
    double tempY;
    double polyTransformedX;
    double polyTransformedY;
    StatusInt s;
    for (currentY = m_ApplicationArea->GetExtent().GetYMin() ; currentY < m_ApplicationArea->GetExtent().GetYMax() ; currentY += stepY)
        {
        for (currentX = m_ApplicationArea->GetExtent().GetXMin() ; currentX < m_ApplicationArea->GetExtent().GetXMax() ; currentX += stepX)
            {
            if(m_ApplicationArea->IsPointIn(HGF2DPosition(currentX, currentY)))
                {
                if (m_UsePolynomialForDirect)
                    {
                    ConvertInverse(currentX, currentY, &tempX, &tempY);
                    ConvertDirect(tempX, tempY, &polyTransformedX, &polyTransformedY);
                    s = m_pAdaptedTransfoModel->ConvertDirect(tempX, tempY, &currentX, &currentY);
                    currentError = sqrt(pow(polyTransformedX - currentX, 2) + pow(polyTransformedY - currentY, 2));
                    }
                else
                    {
                    ConvertInverse(currentX, currentY, &polyTransformedX, &polyTransformedY);
                    s = m_pAdaptedTransfoModel->ConvertInverse(currentX, currentY, &tempX, &tempY);
                    currentError = sqrt(pow(polyTransformedX - tempX, 2) + pow(polyTransformedY - tempY, 2));
                    }
                if(s != SUCCESS)
                    return ERROR;
                if(currentError < *po_pMinError)
                    {
                    *po_pMinError = currentError;
                    *po_pMinErrorPosition = HGF2DPosition(currentX, currentY);
                    }
                if(currentError > *po_pMaxError)
                    {
                    *po_pMaxError = currentError;
                    *po_pMaxErrorPosition = HGF2DPosition(currentX, currentY);
                    }
                sumOfError += currentError;
                numberOfTestedPoints++;
                }
            }
        }
    *po_pMeanError = sumOfError / numberOfTestedPoints;
    return SUCCESS;
    }


#if __USE_COMPOSE_OPTIMISATION__
//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGFPolynomialModelAdapter::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsStretchable())
        {
        HFCPtr<HGFPolynomialModelAdapter> pPolynomial(new HGFPolynomialModelAdapter(*this));

        double scaleFactorX, scaleFactorY;
        HGF2DDisplacement displacement;

        pPolynomial->m_pAdaptedTransfoModel = pPolynomial->m_pAdaptedTransfoModel->ComposeInverseWithDirectOf(pi_rModel);

        if(m_UsePolynomialForDirect)
            {
            pi_rModel.GetStretchParams(&scaleFactorX, &scaleFactorY, &displacement);

            pPolynomial->AddStretchAfterPolynomial(displacement.GetDeltaX(), displacement.GetDeltaY(), scaleFactorX, scaleFactorY);
            }
        else
            {
            HFCPtr<HGF2DTransfoModel> pModelPrimeReversed = pi_rModel.Clone();
            pModelPrimeReversed->Reverse();

            pModelPrimeReversed->GetStretchParams(&scaleFactorX, &scaleFactorY, &displacement);

            pPolynomial->AddStretchBeforePolynomial(displacement.GetDeltaX(), displacement.GetDeltaY(), scaleFactorX, scaleFactorY);
            }

        pResultModel = pPolynomial;
        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf(pi_rModel);
        }

    return pResultModel;
    }


//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGFPolynomialModelAdapter::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsStretchable())
        {
        HFCPtr<HGFPolynomialModelAdapter> pPolynomial(new HGFPolynomialModelAdapter(*this));

        double scaleFactorX, scaleFactorY;
        HGF2DDisplacement displacement;

        pPolynomial->m_pAdaptedTransfoModel = pi_rModel.ComposeInverseWithDirectOf(*pPolynomial->m_pAdaptedTransfoModel);

        if(m_UsePolynomialForDirect)
            {
            pi_rModel.GetStretchParams(&scaleFactorX, &scaleFactorY, &displacement);

            pPolynomial->AddStretchBeforePolynomial(displacement.GetDeltaX(), displacement.GetDeltaY(), scaleFactorX, scaleFactorY);
            }
        else
            {
            HFCPtr<HGF2DTransfoModel> pModelPrimeReversed = pi_rModel.Clone();
            pModelPrimeReversed->Reverse();

            pModelPrimeReversed->GetStretchParams(&scaleFactorX, &scaleFactorY, &displacement);

            pPolynomial->AddStretchAfterPolynomial(displacement.GetDeltaX(), displacement.GetDeltaY(), scaleFactorX, scaleFactorY);
            }

        pResultModel = pPolynomial;
        }
    else
        {
        // Type is not known ... build a complex
        // To do this we call the ancester ComposeYourself
        pResultModel = HGF2DTransfoModel::ComposeYourself (pi_rModel);
        }

    return (pResultModel);
    }
#endif


//--------------------------------------------------------------------------------------------------------------
// It is possible to integrate stretch parameters directly into the coefficients of the polynomial functions. This
// method integrates stretch parameters after the polynomial, which means, if apply seperatly, we would first apply the
// stretch (scaling + translation), and then use the produced coordinate in the polynomial.
//
// More precisely, we have two polynomial functions, one that transforms the point (x,y) into x' and another one
// that transforms the same point (x,y) into y'. Applying those two funcitons give us the transformed coordinate (x', y')
// Lets call those functions f_x and f_y.
// We also have two stretch functions, one for the scaling and translation in x, and a second one for the scaling and translation in y:
// - s_x(x) = s_1 * x + t_1
// - s_y(y) = s_2 * y + t_2
//
// The new polynomials functions that we obtain after combining the stretch functions before the polynomials are the two functions:
// - f_xprime(x, y) = f_x(s_x(x), s_y(y)) which maps from (x, y) to x'
// - f_yprime(x, y) = f_y(s_x(x), s_y(y)) which maps from (x, y) to y'
//
// The formulas below can be obtained using a symbolic math tool, like Wolfram Alpha, Maple, SymPy, etc. Here is a short script written
// in python using SymPy that generates the coefficients:
//
//        import sympy
//
//        #Declare symbols
//        x, y = sympy.symbols('x,y') #variables
//        a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 = sympy.symbols('a0,a1,a2,a3,a4,a5,a6,a7,a8,a9') #coefficients
//        b_x, c_x, b_y, c_y = sympy.symbols('b_x,c_x,b_y,c_y') #translation and scaling
//
//        #Declare de expression
//        expression = a0 \
//                   + a1 * (b_x * x + c_x) \
//                   + a2 * (b_y * y + c_y) \
//                   + a3 * (b_x * x + c_x)**2 \
//                   + a4 * (b_x * x + c_x) * (b_y * y + c_y) \
//                   + a5 * (b_y * y + c_y)**2 \
//                   + a6 * (b_x * x + c_x)**3 \
//                   + a7 * (b_x * x + c_x)**2 * (b_y * y + c_y)  \
//                   + a8 * (b_x * x + c_x) * (b_y * y + c_y)**2 \
//                   + a9 * (b_y * y + c_y)**3
//
//        #Reformat as polynomial and get coefficients
//        poly = sympy.Poly(expression, (x,y))
//
//        print "new a0:", poly.coeff_monomial((0,0))
//        print "new a1:", poly.coeff_monomial((1,0))
//        print "new a2:", poly.coeff_monomial((0,1))
//        print "new a3:", poly.coeff_monomial((2,0))
//        print "new a4:", poly.coeff_monomial((1,1))
//        print "new a5:", poly.coeff_monomial((0,2))
//        print "new a6:", poly.coeff_monomial((3,0))
//        print "new a7:", poly.coeff_monomial((2,1))
//        print "new a8:", poly.coeff_monomial((1,2)) #where (1,2) means coefficient of x**1 * y**2
//        print "new a9:", poly.coeff_monomial((0,3))
//
//--------------------------------------------------------------------------------------------------------------
void HGFPolynomialModelAdapter::AddStretchBeforePolynomial(double pi_TranslationX, double pi_TranslationY, double pi_ScalingX, double pi_ScalingY)
    {
    double temp [10] = { };

    double b_x = pi_ScalingX;
    double b2_x = b_x * b_x;
    double b3_x = b2_x * b_x;
    double c_x = pi_TranslationX;
    double c2_x = c_x * c_x;
    double c3_x = c2_x * c_x;

    double b_y = pi_ScalingY;
    double b2_y = b_y * b_y;
    double b3_y = b2_y * b_y;
    double c_y = pi_TranslationY;
    double c2_y = c_y * c_y;
    double c3_y = c2_y * c_y;

    //Calculate new X coefficients
    temp[0] = m_CoefficientsX[0]              + m_CoefficientsX[1] * c_x  + m_CoefficientsX[2] * c_y  + m_CoefficientsX[3] * c2_x
            + m_CoefficientsX[4] * c_x * c_y  + m_CoefficientsX[5] * c2_y + m_CoefficientsX[6] * c3_x + m_CoefficientsX[7] * c2_x * c_y
            + m_CoefficientsX[8] * c_x * c2_y + m_CoefficientsX[9] * c3_y;

    temp[1] = (m_CoefficientsX[1] * b_x)            + (2 * m_CoefficientsX[3] * b_x * c_x)       + (m_CoefficientsX[4] * b_x * c_y)
            + (3 * m_CoefficientsX[6] * b_x * c2_x) + (2 * m_CoefficientsX[7] * b_x * c_x * c_y) + (m_CoefficientsX[8] * b_x * c2_y);

    temp[2] = (m_CoefficientsX[2] * b_y)        + (m_CoefficientsX[4] * b_y * c_x)           + (2 * m_CoefficientsX[5] * b_y * c_y)
            + (m_CoefficientsX[7] * b_y * c2_x) + (2 * m_CoefficientsX[8] * b_y * c_x * c_y) + (3 * m_CoefficientsX[9] * b_y * c2_y);

    temp[3] = (m_CoefficientsX[3] * b2_x) + (3 * m_CoefficientsX[6] * b2_x * c_x) + (m_CoefficientsX[7] * b2_x * c_y);
    temp[4] = (m_CoefficientsX[4] * b_x * b_y) + (2 * m_CoefficientsX[7] * b_x * b_y * c_x) + (2 * m_CoefficientsX[8] * b_x * b_y * c_y);
    temp[5] = (m_CoefficientsX[5] * b2_y) + (m_CoefficientsX[8] * b2_y * c_x) + (3 * m_CoefficientsX[9] * b2_y * c_y);
    temp[6] = m_CoefficientsX[6] * b3_x;
    temp[7] = m_CoefficientsX[7] * b2_x * b_y;
    temp[8] = m_CoefficientsX[8] * b_x * b2_y;
    temp[9] = m_CoefficientsX[9] * b3_y;

    m_CoefficientsX[0] = temp[0];
    m_CoefficientsX[1] = temp[1];
    m_CoefficientsX[2] = temp[2];
    m_CoefficientsX[3] = temp[3];
    m_CoefficientsX[4] = temp[4];
    m_CoefficientsX[5] = temp[5];
    m_CoefficientsX[6] = temp[6];
    m_CoefficientsX[7] = temp[7];
    m_CoefficientsX[8] = temp[8];
    m_CoefficientsX[9] = temp[9];

    //Calculate new Y coefficients
    temp[0] = m_CoefficientsY[0]              + m_CoefficientsY[1] * c_x  + m_CoefficientsY[2] * c_y  + m_CoefficientsY[3] * c2_x
            + m_CoefficientsY[4] * c_x * c_y  + m_CoefficientsY[5] * c2_y + m_CoefficientsY[6] * c3_x + m_CoefficientsY[7] * c2_x * c_y
            + m_CoefficientsY[8] * c_x * c2_y + m_CoefficientsY[9] * c3_y;

    temp[1] = (m_CoefficientsY[1] * b_x)            + (2 * m_CoefficientsY[3] * b_x * c_x)       + (m_CoefficientsY[4] * b_x * c_y)
            + (3 * m_CoefficientsY[6] * b_x * c2_x) + (2 * m_CoefficientsY[7] * b_x * c_x * c_y) + (m_CoefficientsY[8] * b_x * c2_y);

    temp[2] = (m_CoefficientsY[2] * b_y)        + (m_CoefficientsY[4] * b_y * c_x)           + (2 * m_CoefficientsY[5] * b_y * c_y)
            + (m_CoefficientsY[7] * b_y * c2_x) + (2 * m_CoefficientsY[8] * b_y * c_x * c_y) + (3 * m_CoefficientsY[9] * b_y * c2_y);

    temp[3] = (m_CoefficientsY[3] * b2_x) + (3 * m_CoefficientsY[6] * b2_x * c_x) + (m_CoefficientsY[7] * b2_x * c_y);
    temp[4] = (m_CoefficientsY[4] * b_x * b_y) + (2 * m_CoefficientsY[7] * b_x * b_y * c_x) + (2 * m_CoefficientsY[8] * b_x * b_y * c_y);
    temp[5] = (m_CoefficientsY[5] * b2_y) + (m_CoefficientsY[8] * b2_y * c_x) + (3 * m_CoefficientsY[9] * b2_y * c_y);
    temp[6] = m_CoefficientsY[6] * b3_x;
    temp[7] = m_CoefficientsY[7] * b2_x * b_y;
    temp[8] = m_CoefficientsY[8] * b_x * b2_y;
    temp[9] = m_CoefficientsY[9] * b3_y;

    m_CoefficientsY[0] = temp[0];
    m_CoefficientsY[1] = temp[1];
    m_CoefficientsY[2] = temp[2];
    m_CoefficientsY[3] = temp[3];
    m_CoefficientsY[4] = temp[4];
    m_CoefficientsY[5] = temp[5];
    m_CoefficientsY[6] = temp[6];
    m_CoefficientsY[7] = temp[7];
    m_CoefficientsY[8] = temp[8];
    m_CoefficientsY[9] = temp[9];
    }


//--------------------------------------------------------------------------------------------------------------
// It is possible to integrate stretch parameters directly into the coefficients of the polynomial functions. This
// method integrates stretch parameters after the polynomial, which means, if apply seperatly, we would transform
// the point (x,y) using the polynomial function, then apply the stretch (scale + translation).
//
// More precisely, we have two polynomial functions, one that transforms the point (x,y) into x' and another one
// that transforms the same point (x,y) into y'. Applying those two funcitons give us the transformed coordinate (x', y')
// Lets call those functions f_x and f_y.
// We also have two stretch functions, one for the scaling and translation in x, and a second one for the scaling and translation in y:
// - s_x(x) = s_1 * x + t_1
// - s_y(y) = s_2 * y + t_2
//
// The new polynomials functions that we obtain after combining the stretch functions after the polynomials are the two functions:
// - f_xprime(x, y) = s_x(f_x(x, y)) which maps from (x, y) to x'
// - f_yprime(x, y) = s_y(f_y(x, y)) which maps from (x, y) to y'
//--------------------------------------------------------------------------------------------------------------
void HGFPolynomialModelAdapter::AddStretchAfterPolynomial(double pi_TranslationX, double pi_TranslationY, double pi_ScalingX, double pi_ScalingY)
    {
    m_CoefficientsX[0] = m_CoefficientsX[0] * pi_ScalingX + pi_TranslationX;
    m_CoefficientsX[1] = m_CoefficientsX[1] * pi_ScalingX;
    m_CoefficientsX[2] = m_CoefficientsX[2] * pi_ScalingX;
    m_CoefficientsX[3] = m_CoefficientsX[3] * pi_ScalingX;
    m_CoefficientsX[4] = m_CoefficientsX[4] * pi_ScalingX;
    m_CoefficientsX[5] = m_CoefficientsX[5] * pi_ScalingX;
    m_CoefficientsX[6] = m_CoefficientsX[6] * pi_ScalingX;
    m_CoefficientsX[7] = m_CoefficientsX[7] * pi_ScalingX;
    m_CoefficientsX[8] = m_CoefficientsX[8] * pi_ScalingX;
    m_CoefficientsX[9] = m_CoefficientsX[9] * pi_ScalingX;

    m_CoefficientsY[0] = m_CoefficientsY[0] * pi_ScalingY + pi_TranslationY;
    m_CoefficientsY[1] = m_CoefficientsY[1] * pi_ScalingY;
    m_CoefficientsY[2] = m_CoefficientsY[2] * pi_ScalingY;
    m_CoefficientsY[3] = m_CoefficientsY[3] * pi_ScalingY;
    m_CoefficientsY[4] = m_CoefficientsY[4] * pi_ScalingY;
    m_CoefficientsY[5] = m_CoefficientsY[5] * pi_ScalingY;
    m_CoefficientsY[6] = m_CoefficientsY[6] * pi_ScalingY;
    m_CoefficientsY[7] = m_CoefficientsY[7] * pi_ScalingY;
    m_CoefficientsY[8] = m_CoefficientsY[8] * pi_ScalingY;
    m_CoefficientsY[9] = m_CoefficientsY[9] * pi_ScalingY;
    }
