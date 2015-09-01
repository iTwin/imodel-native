//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/NonLinearTransforms.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRARasterCopyFromTester
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DTransfoModel.h>

//-----------------------------------------------------------------------------
//  This class is a 2DTransfoModel defined for testing purposes only.
//  It defines a non-linear that is composed with other transform models
//  via 2DComplexTransfoModel. 
//
//  It remaps the Y coordinates in a parabolic shape. The X coordinates stays
//  unchanged.
//  
//  This model is used to test a edge case where, after transforming a shape,
//  there are pixels to copy at both extremes of a stripe of the destination,
//  but no valid pixels in the middle. 
//
//  -------------                   ---                            
//  |           |     ==>         /     \                  
//  |           |                /       \                                   
//  |           |     ==>       /   ---   \                                      
//  |           |              |   /   \  |               
//  |           |     ==>      |  /     \ |               
//  -------------              | /       \|                 
//-----------------------------------------------------------------------------
class ParabolaTransfoModel : public HGF2DTransfoModel
    {
    public:
    ParabolaTransfoModel()
        : HGF2DTransfoModel()
        {
        m_reverse = false;
        }

    ParabolaTransfoModel(const ParabolaTransfoModel& pi_rObj)
        : HGF2DTransfoModel (pi_rObj)
        {
        Copy(pi_rObj);
        }

    virtual ~ParabolaTransfoModel() {}

    ParabolaTransfoModel& operator=(const ParabolaTransfoModel& pi_rObj)
        {
        if (this != &pi_rObj)
            {
            HGF2DTransfoModel::operator=(pi_rObj);
            Copy(pi_rObj);
            }
        return (*this);
        }

    // Conversion interface
    virtual bool       IsConvertDirectThreadSafe()  const override {return true;}
    virtual bool       IsConvertInverseThreadSafe() const override {return true;}

    virtual StatusInt  ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const
        {
        RemapCoordinate(*pio_pXInOut, *pio_pYInOut, pio_pXInOut, pio_pYInOut, m_reverse);
        return SUCCESS;
        }

    virtual StatusInt  ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc,
                                     double pi_XInStep, double* po_pXOut, double* po_pYOut) const
        {
        double  X;
        uint32_t Index;
        double* pCurrentX = po_pXOut;
        double* pCurrentY = po_pYOut;


        for (Index = 0, X = pi_XInStart;
             Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
            {
            RemapCoordinate(X, pi_YIn, pCurrentX, pCurrentY, m_reverse);
            }
        return SUCCESS;
        }


    virtual StatusInt ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
        {
        for (uint32_t i = 0; i < pi_NumLoc; i++)
            {
            ConvertDirect(pio_aXInOut + i, pio_aYInOut + i);
            }
        return SUCCESS;
        }

    virtual StatusInt  ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, 
                                     double* po_pYOut) const
        {
        RemapCoordinate(pi_XIn, pi_YIn, po_pXOut, po_pYOut, m_reverse);
        return SUCCESS;
        }

    virtual StatusInt  ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const
        {
        RemapCoordinate(*pio_pXInOut, *pio_pYInOut, pio_pXInOut, pio_pYInOut, !m_reverse);
        return SUCCESS;
        }

    virtual StatusInt  ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc,
                                      double pi_XInStep, double* po_pXOut, double* po_pYOut) const
        {
        double  X;
        uint32_t Index;
        double* pCurrentX = po_pXOut;
        double* pCurrentY = po_pYOut;


        for (Index = 0, X = pi_XInStart;
             Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
            {
            RemapCoordinate(X, pi_YIn, pCurrentX, pCurrentY, !m_reverse);
            }
        return SUCCESS;
        }

    virtual StatusInt ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
        {
        for (uint32_t i = 0; i < pi_NumLoc; i++)
            {
            ConvertDirect(pio_aXInOut + i, pio_aYInOut + i);
            }
        return SUCCESS;
        }

    virtual StatusInt  ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut,
                                      double* po_pYOut) const
        {
        RemapCoordinate(pi_XIn, pi_YIn, po_pXOut, po_pYOut, !m_reverse);
        return SUCCESS;
        }

    virtual bool IsIdentity() const {return false;}

    virtual bool IsStretchable(double pi_AngleTolerance = 0) const {return false;}

    virtual void GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY,
                                  HGF2DDisplacement* po_pDisplacement) const
        {
        HGF2DTransfoModel::GetStretchParamsAt(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement, 0.0, 0.0);
        }

    virtual HGF2DTransfoModel* Clone () const override
        {
        return new ParabolaTransfoModel(*this);
        }

    virtual bool CanBeRepresentedByAMatrix() const {return false;}

    virtual HFCMatrix<3, 3> GetMatrix() const
        {
        HFCMatrix<3, 3> m;
        return m;
        }

    // Geometric properties
    virtual bool PreservesLinearity() const {return false;}
    virtual bool PreservesParallelism() const {return false;}
    virtual bool PreservesShape() const {return false;}
    virtual bool PreservesDirection() const {return false;}

    // Operations
    virtual void Reverse ()
        {
        m_reverse = !m_reverse;
        }

    virtual void Prepare () {}

    bool m_reverse;

    void Copy (const ParabolaTransfoModel& pi_rObj)
        {
        m_reverse = pi_rObj.m_reverse;
        }

    //---------------------------------------------------------------------------------------
    // @description   Remaps a coordinates following a parabolic shape. X coordinates
    //                stays unchanged. Y coordinates are remaped in function of X.
    //
    // @bsimethod                                                    Alexandre.Gariepy  06/15 
    //+---------------+---------------+---------------+---------------+---------------+------
    void RemapCoordinate(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut,
                         bool reverse) const
        {
        *po_pXOut = pi_XIn;
        if (reverse)
            *po_pYOut = pi_YIn + (0.0004 * pow(pi_XIn, 2.0) - 0.8 * pi_XIn + 200);
        else
            *po_pYOut = pi_YIn - (0.0004 * pow(pi_XIn, 2.0) - 0.8 * pi_XIn + 200);
        }
    };

class ThirdDegreeTransfoModel : public HGF2DTransfoModel
    {
    static const uint32_t NUMBER_OF_COEFFICIENT = 10;

    public:
    ThirdDegreeTransfoModel()
        : HGF2DTransfoModel()
        {
        Prepare();
        }

    ThirdDegreeTransfoModel(const ThirdDegreeTransfoModel& pi_rObj)
        : HGF2DTransfoModel (pi_rObj)
        {
        Copy(pi_rObj);
        }

    virtual ~ThirdDegreeTransfoModel() {}

    ThirdDegreeTransfoModel& operator=(const ThirdDegreeTransfoModel& pi_rObj)
        {
        if (this != &pi_rObj)
            {
            HGF2DTransfoModel::operator=(pi_rObj);
            Copy(pi_rObj);
            }
        return (*this);
        }

    // Conversion interface
    virtual bool       IsConvertDirectThreadSafe() const override {return true;}
    virtual bool       IsConvertInverseThreadSafe() const override {return true;}

    virtual StatusInt  ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const
        {
        RemapCoordinate(*pio_pXInOut, *pio_pYInOut, m_CoefficientsA, m_CoefficientsB, pio_pXInOut, pio_pYInOut);
        return SUCCESS;
        }

    virtual StatusInt  ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc,
                                     double pi_XInStep, double* po_pXOut, double* po_pYOut) const
        {
        double  X;
        uint32_t Index;
        double* pCurrentX = po_pXOut;
        double* pCurrentY = po_pYOut;


        for (Index = 0, X = pi_XInStart;
             Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
            {
            RemapCoordinate(X, pi_YIn, m_CoefficientsA, m_CoefficientsB, pCurrentX, pCurrentY);
            }
        return SUCCESS;
        }

    virtual StatusInt ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
        {
        for (uint32_t i = 0; i < pi_NumLoc; i++)
            {
            ConvertDirect(pio_aXInOut + i, pio_aYInOut + i);
            }
        return SUCCESS;
        }

    virtual StatusInt  ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, 
                                     double* po_pYOut) const
        {
        RemapCoordinate(pi_XIn, pi_YIn, m_CoefficientsA, m_CoefficientsB, po_pXOut, po_pYOut);
        return SUCCESS;
        }

    virtual StatusInt  ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const
        {
        RemapCoordinate(*pio_pXInOut, *pio_pYInOut, m_CoefficientsC, m_CoefficientsD, pio_pXInOut, pio_pYInOut);
        return SUCCESS;
        }

    virtual StatusInt  ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc,
                                      double pi_XInStep, double* po_pXOut, double* po_pYOut) const
        {
        double  X;
        uint32_t Index;
        double* pCurrentX = po_pXOut;
        double* pCurrentY = po_pYOut;


        for (Index = 0, X = pi_XInStart;
             Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
            {
            RemapCoordinate(X, pi_YIn, m_CoefficientsC, m_CoefficientsD, pCurrentX, pCurrentY);
            }
        return SUCCESS;
        }

    virtual StatusInt ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
        {
        for (uint32_t i = 0; i < pi_NumLoc; i++)
            {
            ConvertInverse(pio_aXInOut + i, pio_aYInOut + i);
            }
        return SUCCESS;
        }

    virtual StatusInt  ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut,
                                      double* po_pYOut) const
        {
        RemapCoordinate(pi_XIn, pi_YIn, m_CoefficientsC, m_CoefficientsD, po_pXOut, po_pYOut);
        return SUCCESS;
        }

    virtual bool IsIdentity() const {return false;}

    virtual bool IsStretchable(double pi_AngleTolerance = 0) const {return false;}

    virtual void GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY,
                                  HGF2DDisplacement* po_pDisplacement) const
        {
        HGF2DTransfoModel::GetStretchParamsAt(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement, 0.0, 0.0);
        }

    virtual HGF2DTransfoModel* Clone () const override
        {
        return new ThirdDegreeTransfoModel(*this);
        }

    virtual bool CanBeRepresentedByAMatrix() const {return false;}

    virtual HFCMatrix<3, 3> GetMatrix() const
        {
        HFCMatrix<3, 3> m;
        return m;
        }

    // Geometric properties
    virtual bool PreservesLinearity() const {return false;}
    virtual bool PreservesParallelism() const {return false;}
    virtual bool PreservesShape() const {return false;}
    virtual bool PreservesDirection() const {return false;}

    // Operations
    virtual void Reverse ()
        {
        double temp;
        for(uint32_t i = 0; i < NUMBER_OF_COEFFICIENT; i++)
            {
            temp = m_CoefficientsA[i];
            m_CoefficientsA[i] = m_CoefficientsC[i];
            m_CoefficientsC[i] = temp;

            temp = m_CoefficientsB[i];
            m_CoefficientsB[i] = m_CoefficientsD[i];
            m_CoefficientsD[i] = temp;
            }
        }

    virtual void Prepare () 
        {
        // direct
        m_CoefficientsA[0] = -16.4702606071;
        m_CoefficientsA[1] = 1.08495512903;
        m_CoefficientsA[2] = -0.110560165049;
        m_CoefficientsA[3] = 0.000486333384684;
        m_CoefficientsA[4] = -0.000287862019029;
        m_CoefficientsA[5] = 0.000254048348424;
        m_CoefficientsA[6] = -1.66296009185e-06;
        m_CoefficientsA[7] = 1.36878799736e-05;
        m_CoefficientsA[8] = 2.02609232737e-05;
        m_CoefficientsA[9] = -2.52581933774e-05;
        m_CoefficientsB[0] = -0.971177631908;
        m_CoefficientsB[1] = -0.0803951619276;
        m_CoefficientsB[2] = 2.63926791411;
        m_CoefficientsB[3] = 0.000531060765708;
        m_CoefficientsB[4] = -0.000316640027824;
        m_CoefficientsB[5] = -0.000819491003157;
        m_CoefficientsB[6] = 5.9281780232e-06;
        m_CoefficientsB[7] = 1.13119819866e-05;
        m_CoefficientsB[8] = -9.01608885036e-06;
        m_CoefficientsB[9] = -0.000262932484968;
        // inverse
        m_CoefficientsC[0] = -6.22520663363;
        m_CoefficientsC[1] = 1.07339140877;
        m_CoefficientsC[2] = 0.986209385692;
        m_CoefficientsC[3] = 0.00192858780738;
        m_CoefficientsC[4] = -0.00116840686154;
        m_CoefficientsC[5] = -0.00331531587693;
        m_CoefficientsC[6] = 3.73171636348e-06;
        m_CoefficientsC[7] = 1.79684086869e-05;
        m_CoefficientsC[8] = -5.1252205754e-05;
        m_CoefficientsC[9] = -0.000165714030617;
        m_CoefficientsD[0] = -1.69894036029;
        m_CoefficientsD[1] = 0.101327351309;
        m_CoefficientsD[2] = 0.791255754119;
        m_CoefficientsD[3] = -0.000255731466368;
        m_CoefficientsD[4] = -0.00102090500494;
        m_CoefficientsD[5] = 0.000545436809207;
        m_CoefficientsD[6] = -4.11659389314e-06;
        m_CoefficientsD[7] = -7.70300249626e-06;
        m_CoefficientsD[8] = -1.93773594071e-06;
        m_CoefficientsD[9] = 1.44434726089e-05;
        }

    void Copy (const ThirdDegreeTransfoModel& pi_rObj)
        {
        for (uint32_t i = 0; i < NUMBER_OF_COEFFICIENT; i++)
            {
            m_CoefficientsA[i] = pi_rObj.m_CoefficientsA[i];
            m_CoefficientsB[i] = pi_rObj.m_CoefficientsB[i];
            m_CoefficientsC[i] = pi_rObj.m_CoefficientsC[i];
            m_CoefficientsD[i] = pi_rObj.m_CoefficientsD[i];
            }
        }

    void RemapCoordinate(double pi_XIn, double pi_YIn, 
                                             const double pi_XCoefficients[NUMBER_OF_COEFFICIENT], 
                                             const double pi_YCoefficients[NUMBER_OF_COEFFICIENT],
                                             double* po_pXOut, double* po_pYOut) const
        {
        double x_result = 0;
        double y_result = 0;

        uint32_t coefficient_index = 0;
        for(uint32_t i = 0; i <= 3; i++)
            {
            for(uint32_t j = 0; j <= i; j++)
                {
                x_result += pi_XCoefficients[coefficient_index] * pow(pi_XIn, (i - j)) * pow(pi_YIn, j);
                y_result += pi_YCoefficients[coefficient_index] * pow(pi_XIn, (i - j)) * pow(pi_YIn, j);
                coefficient_index++;
                }
            }
        *po_pXOut = x_result;
        *po_pYOut = y_result;
        }

    // Coefficients A used to transform the x component of coordinates from direct
    double m_CoefficientsA[NUMBER_OF_COEFFICIENT];
    // Coefficients B used to transform the y component of coordinates from direct
    double m_CoefficientsB[NUMBER_OF_COEFFICIENT];
    // Coefficients C used to transform the x component of coordinates from inverse
    double m_CoefficientsC[NUMBER_OF_COEFFICIENT];
    // Coefficients D used to transform the y component of coordinates from inverse
    double m_CoefficientsD[NUMBER_OF_COEFFICIENT];

    };
