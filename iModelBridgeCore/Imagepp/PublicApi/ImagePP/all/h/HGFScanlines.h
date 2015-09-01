//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFScanlines.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HGF2DCoordSys.h"
#include "HFCPtr.h"
#include "HGF2DPosition.h"

BEGIN_IMAGEPP_NAMESPACE
class HNOVTABLEINIT IPixelSelectionStrategy
    {
public:
    virtual int32_t ConvertXMin(double pi_XMin) const = 0;
    virtual int32_t ConvertXMax(double pi_XMax) const = 0;
    virtual int32_t ConvertYMin(double pi_YMin) const = 0;
    virtual int32_t ConvertYMax(double pi_YMax) const = 0;
    virtual bool   IsAValidRun(double pi_XMin, double pi_XMax) const = 0;
    virtual bool   IsLoose() const = 0;
    };

class HGFScanLines
    {
protected:
    typedef vector<double> SpanArray;

public:
    //:> Primary methods

    IMAGEPP_EXPORT HGFScanLines(bool pi_gridMode = false,
                        const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys = 0);

    IMAGEPP_EXPORT virtual ~HGFScanLines();


    //:> Info extraction

    IMAGEPP_EXPORT double         GetYMin() const;
    IMAGEPP_EXPORT double         GetYMax() const;
    IMAGEPP_EXPORT HSINTX          GetScanLineStart() const;
    IMAGEPP_EXPORT HSINTX          GetScanLineEnd() const;
    IMAGEPP_EXPORT bool           IsRectangle() const;
    IMAGEPP_EXPORT size_t          GetScanlineCount() const;

    IMAGEPP_EXPORT const HFCPtr<HGF2DCoordSys>&
    GetScanlinesCoordSys() const;

    //:> Scanline usage (not reentrant)

    IMAGEPP_EXPORT bool           GotoFirstRun();
    IMAGEPP_EXPORT bool           GotoNextRun();
    IMAGEPP_EXPORT bool           NextRunIsOnSameScanline() const;
    IMAGEPP_EXPORT HSINTX          GetCurrentScanLine() const;
    IMAGEPP_EXPORT HSINTX          GetCurrentRunXMin() const;
    IMAGEPP_EXPORT size_t          GetCurrentRunLength() const;
    IMAGEPP_EXPORT void            GetCurrentRun(HSINTX* po_pXMin, HSINTX* po_pScanline, size_t* po_pLength) const;

    IMAGEPP_EXPORT double         AdjustXValueForCurrentRun(double pi_X) const;
    IMAGEPP_EXPORT double         AdjustYValueForCurrentRun(double pi_Y) const;


    //:> Range setting

    IMAGEPP_EXPORT void     SetLimits(double pi_YMin,
                              double pi_YMax,
                              size_t  pi_ProjectedMaxCrossingsPerLine,
                              bool   pi_IsRectangle = false,
                              bool   pi_Reserve = true);

    IMAGEPP_EXPORT void            ResetLimits();

    IMAGEPP_EXPORT bool           LimitsAreSet() const;


    //:> Scanline construction interface

    IMAGEPP_EXPORT double         AdjustEndpointYValue(double pi_YValue) const;

    IMAGEPP_EXPORT double         GetFirstScanlinePosition() const;

    IMAGEPP_EXPORT void            SetScanlinesCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    IMAGEPP_EXPORT bool           AddCrossingPoint(HSINTX pi_Scanline, double pi_XCrossingPoint);

    IMAGEPP_EXPORT void            AddCrossingPointsForSegment(const HGF2DPosition& pi_rStart,
                                                       const HGF2DPosition& pi_rEnd);

    void AddCrossingPointsForSegment(const HGF2DPositionCollection& pi_rPoints);



    IMAGEPP_EXPORT SpanArray*                        GetSpanArray() const;
    IMAGEPP_EXPORT void                              SetStrategy(IPixelSelectionStrategy* pi_pStrategy);

protected:


    //:> Disabled methods
    HGFScanLines(const HGFScanLines& pi_rObj);
    HGFScanLines&   operator=(const HGFScanLines& pi_rObj);


    // Coordinate system the scanlines are in
    HFCPtr<HGF2DCoordSys> m_pCoordSys;

    // The selected pixel selection strategy
    HAutoPtr<IPixelSelectionStrategy>
    m_pPixelSelector;

    // Specified parameters
    double         m_YMin;
    double         m_YMax;
    bool           m_IsRectangle;
    bool           m_LimitsAreSet;

    // Optimizations
    HSINTX          m_FirstScanline;
    HSINTX          m_NumberOfScanlines;
    HGF2DPosition*  m_pPreviousStart;
    HGF2DPosition*  m_pPreviousEnd;

    // Iteration state
    HSINTX          m_CurrentScanLine;
    HSINTX          m_PositionInScanline;

    // The arrays of crossing points
    HArrayAutoPtr<SpanArray>
    m_pScanLines;
    };

END_IMAGEPP_NAMESPACE
#include "HGFScanLines.hpp"
