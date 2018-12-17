//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCGrid.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCGrid
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HNOVTABLEINIT HFCGrid
    {
public:

    // Primary methods
    HFCGrid (double      pi_Precision = HGLOBAL_EPSILON);

    HFCGrid (double      pi_XMin,
             double      pi_YMin,
             double      pi_XMax,
             double      pi_YMax,
             double      pi_Precision = HGLOBAL_EPSILON);

    ~HFCGrid();

    HFCGrid(const HFCGrid& pi_rObj);
    HFCGrid&      operator=(const HFCGrid& pi_rObj);


    // Set/Get extent
    void            SetExtent(double   pi_XMin,
                              double   pi_YMin,
                              double   pi_XMax,
                              double   pi_YMax);

    void            GetExtent(double*  po_pXMin,
                              double*  po_pYMin,
                              double*  po_pXMax,
                              double*  po_pYMax) const;


    // Set/Get precision
    void            SetPrecision(double pi_Precision);
    double         GetPrecision() const;

    // Coordinate management
    int64_t       GetXMin() const;
    int64_t       GetYMin() const;
    int64_t       GetXMax() const;
    int64_t       GetYMax() const;

    // Dimension measurement
    int64_t       GetWidth() const;
    int64_t       GetHeight() const;


    // Coord conversion
    static int32_t ConvertValue(double    pi_Value,
                                double    pi_Precision = HGLOBAL_EPSILON);

    static double  ConvertValue(int32_t  pi_Value);

protected:

private:

    // members
    double         m_XMin;
    double         m_YMin;
    double         m_XMax;
    double         m_YMax;

    double         m_Precision;
    };

//=======================================================================================
//! @bsiclass                                                     
//=======================================================================================
class HFCInclusiveGrid
    {
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 06/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static HFCInclusiveGrid FromIntersectionOf(HFCInclusiveGrid const& first, HFCInclusiveGrid const& second)
        {
        HFCInclusiveGrid grid; 
        grid.InitFromIntersectionOf(first, second);
        return grid;
        }


    // Primary methods
    HFCInclusiveGrid(double xMinContinuous, double yMinContinuous, double xMaxContinuous, double yMaxContinuous)
        :m_XMinContinuous(xMinContinuous), m_YMinContinuous(yMinContinuous), 
         m_XMaxContinuous(xMaxContinuous), m_YMaxContinuous(yMaxContinuous),
         m_XMin(ContinuousToMinDiscrete(xMinContinuous)), m_YMin(ContinuousToMinDiscrete(yMinContinuous)),
         m_XMax(ContinuousToMaxDiscrete(xMaxContinuous)), m_YMax(ContinuousToMaxDiscrete(yMaxContinuous))
        {        
        BeAssert(GetXMin() <= GetXMax());
        BeAssert(GetYMin() <= GetYMax());
        }
    explicit HFCInclusiveGrid() { InitEmpty(); }
        
    ~HFCInclusiveGrid(){};

    // Init
    void InitFromLenght(double xMinContinuous, double yMinContinuous, uint64_t width, uint64_t height)
        {
        m_XMinContinuous = xMinContinuous;
        m_YMinContinuous = yMinContinuous;
        m_XMin = ContinuousToMinDiscrete(xMinContinuous);
        m_YMin = ContinuousToMinDiscrete(yMinContinuous);

        // Use the discrete coordinate to accommodate requested width/height
        m_XMaxContinuous = (double)GetXMin() + (double)width;
        m_YMaxContinuous = (double)GetYMin() + (double)height;
        m_XMax = ContinuousToMaxDiscrete(m_XMaxContinuous);
        m_YMax = ContinuousToMaxDiscrete(m_YMaxContinuous);

        BeAssert(width == GetWidth() && height == GetHeight());
        BeAssert(GetXMin() <= GetXMax());
        BeAssert(GetYMin() <= GetYMax());
        }

    //! Test for intersection between 2 grid.
    bool HasIntersect(HFCInclusiveGrid const& grid) const
        {
        // Check if rectangles are disjoint
        // The maxes required +1 because they represent the beginning of the last pixel so its full 
        // continuous region is MAX+1.  It fixes cases with a length of 1: (525, 525), (400, 525)
         if (grid.GetXMin() >= (GetXMax() + 1) || (grid.GetXMax() + 1) <= GetXMin() ||
             grid.GetYMin() >= (GetYMax() + 1) || (grid.GetYMax() + 1) <= GetYMin())
            return false;
            
        return true;
        }

    void InitFromIntersectionOf(HFCInclusiveGrid const& first, HFCInclusiveGrid const& second)
        {
        m_XMinContinuous = MAX(first.m_XMinContinuous, second.m_XMinContinuous);
        m_YMinContinuous = MAX(first.m_YMinContinuous, second.m_YMinContinuous);
        m_XMaxContinuous = MIN(first.m_XMaxContinuous, second.m_XMaxContinuous);
        m_YMaxContinuous = MIN(first.m_YMaxContinuous, second.m_YMaxContinuous);

        if (m_XMinContinuous > m_XMaxContinuous || m_YMinContinuous > m_YMaxContinuous)
            {
            InitEmpty();
            }
        else
            {
            m_XMin = ContinuousToMinDiscrete(m_XMinContinuous);
            m_YMin = ContinuousToMinDiscrete(m_YMinContinuous);
            m_XMax = ContinuousToMaxDiscrete(m_XMaxContinuous);
            m_YMax = ContinuousToMaxDiscrete(m_YMaxContinuous);
            }
        }

    void InitEmpty()
        {
        m_XMin = 0;
        m_YMin = 0;
        m_XMax = 0;
        m_YMax = 0;

        m_XMinContinuous = 0;
        m_YMinContinuous = 0;
        m_XMaxContinuous = 0;
        m_YMaxContinuous = 0;
        }

    // Coordinate management
    int64_t GetXMin() const { return m_XMin; }
    int64_t GetYMin() const { return m_YMin; }
    int64_t GetXMax() const { return m_XMax; }
    int64_t GetYMax() const { return m_YMax; }

    // Dimension measurement
    uint64_t GetWidth() const  {return (GetXMax() - GetXMin()) + 1;}
    uint64_t GetHeight() const {return (GetYMax() - GetYMin()) + 1;}

    bool operator!=(const HFCInclusiveGrid& rhs) const {return !(*this == rhs);}

    bool operator== (const HFCInclusiveGrid& rhs) const
        {
        // N.B. we do not care about Continuous values. They are kept for debugging purpose.
        return rhs.m_XMin == m_XMin && rhs.m_XMax == m_XMax &&
               rhs.m_YMin == m_YMin && rhs.m_YMax == m_YMax;
        }

private:
    int64_t ContinuousToMinDiscrete(double val) const { return (int64_t)floor(val + 0.00001); }
    int64_t ContinuousToMaxDiscrete(double val) const { return (int64_t)floor(val - 0.00001); }


    int64_t m_XMin;
    int64_t m_YMin;
    int64_t m_XMax;
    int64_t m_YMax;

    // For now, keep continuous values for debugging purpose only.
    double m_XMinContinuous;
    double m_YMinContinuous;
    double m_XMaxContinuous;
    double m_YMaxContinuous;
    };

END_IMAGEPP_NAMESPACE

#include "HFCGrid.hpp"

