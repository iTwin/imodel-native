//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFScanlines.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HGF2DCoordSys.h"
#include "HFCPtr.h"
#include "HGF2DPosition.h"

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

    _HDLLg HGFScanLines(bool pi_gridMode = false,
                        const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys = 0);

    _HDLLg virtual ~HGFScanLines();


    //:> Info extraction

    _HDLLg double         GetYMin() const;
    _HDLLg double         GetYMax() const;
    _HDLLg HSINTX          GetScanLineStart() const;
    _HDLLg HSINTX          GetScanLineEnd() const;
    _HDLLg bool           IsRectangle() const;
    _HDLLg size_t          GetScanlineCount() const;

    _HDLLg const HFCPtr<HGF2DCoordSys>&
    GetScanlinesCoordSys() const;

    //:> Scanline usage (not reentrant)

    _HDLLg bool           GotoFirstRun();
    _HDLLg bool           GotoNextRun();
    _HDLLg bool           NextRunIsOnSameScanline() const;
    _HDLLg HSINTX          GetCurrentScanLine() const;
    _HDLLg HSINTX          GetCurrentRunXMin() const;
    _HDLLg size_t          GetCurrentRunLength() const;
    _HDLLg void            GetCurrentRun(HSINTX* po_pXMin, HSINTX* po_pScanline, size_t* po_pLength) const;

    _HDLLg double         AdjustXValueForCurrentRun(double pi_X) const;
    _HDLLg double         AdjustYValueForCurrentRun(double pi_Y) const;


    //:> Range setting

    _HDLLg void     SetLimits(double pi_YMin,
                              double pi_YMax,
                              size_t  pi_ProjectedMaxCrossingsPerLine,
                              bool   pi_IsRectangle = false,
                              bool   pi_Reserve = true);

    _HDLLg void            ResetLimits();

    _HDLLg bool           LimitsAreSet() const;


    //:> Scanline construction interface

    _HDLLg double         AdjustEndpointYValue(double pi_YValue) const;

    _HDLLg double         GetFirstScanlinePosition() const;

    _HDLLg void            SetScanlinesCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    _HDLLg bool           AddCrossingPoint(HSINTX pi_Scanline, double pi_XCrossingPoint);

    _HDLLg void            AddCrossingPointsForSegment(const HGF2DPosition& pi_rStart,
                                                       const HGF2DPosition& pi_rEnd);

    void AddCrossingPointsForSegment(const HGF2DPositionCollection& pi_rPoints);



    _HDLLg SpanArray*                        GetSpanArray() const;
    _HDLLg void                              SetStrategy(IPixelSelectionStrategy* pi_pStrategy);

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


#include "HGFScanLines.hpp"
