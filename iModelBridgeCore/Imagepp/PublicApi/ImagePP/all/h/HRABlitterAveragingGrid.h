//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABlitterAveragingGrid.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRABlitterAveragingGrid
//-----------------------------------------------------------------------------

#pragma once


class HNOVTABLEINIT HRABlitterAveragingGrid
    {
public:

    // Primary methods
    HRABlitterAveragingGrid (double pi_XMin,
                             double pi_YMin,
                             double pi_XMax,
                             double pi_YMax,
                             uint32_t pi_XLimit = ULONG_MAX,
                             uint32_t pi_YLimit = ULONG_MAX);

    ~HRABlitterAveragingGrid();

    HRABlitterAveragingGrid(const HRABlitterAveragingGrid& pi_rObj);
    HRABlitterAveragingGrid&
    operator=(const HRABlitterAveragingGrid& pi_rObj);


    // Set/Get extent
    void            SetExtent(double   pi_XMin,
                              double   pi_YMin,
                              double   pi_XMax,
                              double   pi_YMax);

    void            GetExtent(double*  po_pXMin,
                              double*  po_pYMin,
                              double*  po_pXMax,
                              double*  po_pYMax) const;

    void            SetLimits(uint32_t pi_XLimit,
                              uint32_t pi_YLimit);

    void            TranslateX(double pi_Translation);

    // Coordinate management
    uint32_t        GetXMin() const;
    uint32_t        GetYMin() const;
    uint32_t        GetXMax() const;
    uint32_t        GetYMax() const;

    // Dimension measurement
    uint32_t        GetWidth() const;
    uint32_t        GetHeight() const;


    // Coord conversion
    static uint32_t ConvertValue(double pi_Value,
                                 double pi_Precision = HGLOBAL_EPSILON);

    static double  ConvertValue(uint32_t pi_Value);

protected:

private:

    void            ComputeResults();


    // members
    double         m_XMin;
    double         m_YMin;
    double         m_XMax;
    double         m_YMax;

    uint32_t        m_XLimit;
    uint32_t        m_YLimit;

    // Precomputed results
    uint32_t        m_ResultXMin;
    uint32_t        m_ResultYMin;
    uint32_t        m_ResultXMax;
    uint32_t        m_ResultYMax;
    };


#include "HRABlitterAveragingGrid.hpp"

