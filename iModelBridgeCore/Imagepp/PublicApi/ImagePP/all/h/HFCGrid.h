//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCGrid.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCGrid
//-----------------------------------------------------------------------------

#pragma once


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


#include "HFCGrid.hpp"

