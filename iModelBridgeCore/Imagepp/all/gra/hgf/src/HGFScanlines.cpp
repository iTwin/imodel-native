//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFScanlines.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFScanLines.h>

//:Ignore
////////////////////////////
// HGFScanlines
////////////////////////////
//:End Ignore

//-----------------------------------------------------------------------------
// Pixel selection strategies
//-----------------------------------------------------------------------------
class PixelSelectionStrategy : public IPixelSelectionStrategy
    {
public:

    PixelSelectionStrategy() {};
    virtual ~PixelSelectionStrategy() {};

    // The first X pixel coordinate has its center (0.5) past the received real value.
    virtual int32_t ConvertXMin(double pi_XMin) const {
        return (int32_t) (pi_XMin + 0.5);
        }

    // The last useful X pixel coordinate has its center (0.5) before the received real value.
    // We return the coordinate of the first pixel AFTER. For example, receiving the value 3.8,
    // the pixel 3 would be taken since its center (3.5) is before 3.8. We would then return 4 as the limit of the range.
    virtual int32_t ConvertXMax(double pi_XMax) const {
        return (int32_t) (pi_XMax + 0.5);
        }

    // The first Y pixel coordinate has its center (0.5) past the received real value.
    virtual int32_t ConvertYMin(double pi_YMin) const {
        return (int32_t) (pi_YMin + 0.5);
        }

    // The last useful Y pixel coordinate has its center (0.5) before the received real value.
    // We return the coordinate of the first pixel AFTER. For example, receiving the value 6.7,
    // the pixel 6 would be taken since its center (6.5) is before 6.7. We would then return 7 as the limit of the range.
    virtual int32_t ConvertYMax(double pi_YMax) const {
        return (int32_t) (pi_YMax + 0.5);
        }

    // Test if a run is valid or not. A run is valid if it is at least one pixel wide.
    // A pixel is inside a run if its center (0.5) is between the minimum and maximum X values.
    virtual bool   IsAValidRun(double pi_XMin, double pi_XMax) const {
        return (((int32_t)(pi_XMax+0.5)) - ((int32_t)(pi_XMin+0.5)) >= 1);
        }

    // Return if it is a loose strategy
    virtual bool   IsLoose () const {
        return false;
        }

private:

    // Disabled methods
    PixelSelectionStrategy(const PixelSelectionStrategy& pi_rObj);
    PixelSelectionStrategy& operator=(const PixelSelectionStrategy& pi_rObj);
    };

//-----------------------------------------------------------------------------
// LoosePixelSelectionStrategy
//-----------------------------------------------------------------------------
class LoosePixelSelectionStrategy : public PixelSelectionStrategy
    {
public:

    LoosePixelSelectionStrategy() {};
    ~LoosePixelSelectionStrategy() {};

    // The first X pixel coordinate has its center (0.5) past the received real value.
    virtual int32_t ConvertXMin(double pi_XMin) const {
        return (int32_t) (pi_XMin + HGLOBAL_EPSILON);
        }

    // The last useful X pixel coordinate has its center (0.5) before the received real value.
    // We return the coordinate of the first pixel AFTER. For example, receiving the value 3.8,
    // the pixel 3 would be taken since its center (3.5) is before 3.8. We would then return 4 as the limit of the range.
    virtual int32_t ConvertXMax(double pi_XMax) const
        {
        // Converting to XMax with a value of 0.0 fails because we fall into the negative.
        // Ex. 0.0 will end-up with a MAX of 1 instead of 0
        // ((Int32)(0.0 - EPS)) + 1 = (Int32)(-EPS) + 1   = (0) + 1 = 1 <- wrong
        // ((Int32)(1.0 - EPS)) + 1 = (Int32)(0.9999) + 1 = (0) + 1 = 1 <- good
        // Also see IsAValidRun() and ConvertYMax()
        if(HDOUBLE_EQUAL_EPSILON(pi_XMax, 0.0))
            return 0;

        return ((int32_t) (pi_XMax - HGLOBAL_EPSILON)) + 1;
        }

    // The first Y pixel coordinate has its center (0.5) past the received real value.
    virtual int32_t ConvertYMin(double pi_YMin) const {
        return (int32_t) (pi_YMin + HGLOBAL_EPSILON);
        }

    // The last useful Y pixel coordinate has its center (0.5) before the received real value.
    // We return the coordinate of the first pixel AFTER. For example, receiving the value 6.7,
    // the pixel 6 would be taken since its center (6.5) is before 6.7. We would then return 7 as the limit of the range.
    virtual int32_t ConvertYMax(double pi_YMax) const
        {
        // Converting to YMax with a value of 0.0 fails because we fall into the negative.
        // Ex. 0.0 will end-up with a MAX of 1 instead of 0
        // ((Int32)(0.0 - EPS)) + 1 = (Int32)(-EPS) + 1   = (0) + 1 = 1 <- wrong
        // ((Int32)(1.0 - EPS)) + 1 = (Int32)(0.9999) + 1 = (0) + 1 = 1 <- good
        // Also see IsAValidRun() and ConvertYMax()
        if(HDOUBLE_EQUAL_EPSILON(pi_YMax, 0.0))
            return 0;

        return ((int32_t) (pi_YMax - HGLOBAL_EPSILON)) + 1;
        }

    // Test if a run is valid or not. A run is valid if it is at least one pixel wide.
    // A pixel is inside a run if its center (0.5) is between the minimum and maximum X values.
    virtual bool   IsAValidRun(double pi_XMin, double pi_XMax) const
        {
        // We use convertXXX() methods so a XMax value of 0.0 is handled properly.
        return ConvertXMax(pi_XMax) > ConvertXMin(pi_XMin);
        }

    // Return if it is a loose strategy
    virtual bool   IsLoose () const {
        return true;
        }

private:

    // Disabled methods
    LoosePixelSelectionStrategy(const LoosePixelSelectionStrategy& pi_rObj);
    LoosePixelSelectionStrategy& operator=(const LoosePixelSelectionStrategy& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Strategy Creators
//-----------------------------------------------------------------------------
struct PixelSelectionStrategyCreator
    {
    virtual PixelSelectionStrategy* operator ()() const = 0;
    };

struct DefaultPixelSelectionStrategyCreator : public PixelSelectionStrategyCreator
    {
public:
    PixelSelectionStrategy* operator ()() const {
        return new PixelSelectionStrategy();
        }
    };

struct LoosePixelSelectionStrategyCreator : public PixelSelectionStrategyCreator
    {
public:
    PixelSelectionStrategy* operator ()() const {
        return new LoosePixelSelectionStrategy();
        }
    };
// Strategy factory
//-----------------------------------------------------------------------------
struct PixelSelectionStrategyFactory
    {
public:
    PixelSelectionStrategyFactory(bool pi_gridMode)
        {
        if (pi_gridMode)
            {
            m_pCreator = new LoosePixelSelectionStrategyCreator();
            }
        if (!m_pCreator)
            m_pCreator = new DefaultPixelSelectionStrategyCreator();
        }

    virtual ~PixelSelectionStrategyFactory() {};

    virtual  PixelSelectionStrategy* CreateStrategy () const
        {
        HPRECONDITION (m_pCreator.get() != 0);
        return (*m_pCreator)();
        }

private:
    HAutoPtr<PixelSelectionStrategyCreator> m_pCreator;
    };


/** -----------------------------------------------------------------------------
    Constructor. The coordsys parameter serves for validation purposes only.
    -----------------------------------------------------------------------------
*/
HGFScanLines::HGFScanLines(bool pi_gridMode,
                           const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_pCoordSys(pi_rpCoordSys)
    {
    // Initilaze the object properly.
    ResetLimits();

    PixelSelectionStrategyFactory Factory (pi_gridMode);
    m_pPixelSelector = Factory.CreateStrategy();
    }


/** -----------------------------------------------------------------------------
    Destructor.
    -----------------------------------------------------------------------------
*/
HGFScanLines::~HGFScanLines()
    {
    }


/** -----------------------------------------------------------------------------
    This method must be called once to set the scanline range that will be
    generated. Subsequent calls will destroy all previously entered crossing
    points.

    @param pi_YMin The minimum Y value that the scanlines will cover.
    @param pi_YMax The maximum Y value that the scanlines will cover.
    @param pi_ProjectedMaxCrossingsPerLine A hint used to preallocate space
                                           for the crossing points on each line.
    @param pi_IsRectangle Indicate if the shape to scan is a rectangle, in
                          which case we don't need to keep crossing points for
                          each scanline.
    -----------------------------------------------------------------------------
*/
void HGFScanLines::SetLimits(double pi_YMin,
                             double pi_YMax,
                             size_t  pi_ProjectedMaxCrossingsPerLine,
                             bool   pi_IsRectangle,
                             bool   pi_Reserve)
    {
    HPRECONDITION(HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_YMax, pi_YMin));

    double YMin = AdjustEndpointYValue(pi_YMin);
    double YMax = AdjustEndpointYValue(pi_YMax);
    m_YMin = pi_YMin;
    m_YMax = pi_YMax;
    m_IsRectangle = pi_IsRectangle;

    m_LimitsAreSet = true;

    // The first line has YMin smaller than its center (0.5)
    m_FirstScanline = m_pPixelSelector->ConvertYMin(YMin);

    m_NumberOfScanlines = m_pPixelSelector->ConvertYMax(YMax) - m_FirstScanline;
    if (m_NumberOfScanlines > 0)
        {
        // Allocate and prepare the array of intersection points

        if (m_IsRectangle)
            {
            // Allocate only one line, and keep two intersection points
            m_pScanLines = new SpanArray[1];

            if (pi_Reserve)
                m_pScanLines->reserve(2);
            }
        else
            {
            // Allocate all necessary lines
            try
                {
                m_pScanLines = new SpanArray[m_NumberOfScanlines];
                if (pi_Reserve)
                    {
                    for (HSINTX i = 0 ; i < m_NumberOfScanlines ; ++i)
                        {
                        m_pScanLines[i].reserve(pi_ProjectedMaxCrossingsPerLine);
                        }
                    }
                }
            catch(...)
                {
                // Something went awfully wrong, probably allocated
                // too much memory. Can't do our job.

                m_LimitsAreSet = false;

                m_pScanLines = 0;
                }
            }
        }
    }


/** -----------------------------------------------------------------------------
    Add a crossing point to one of the scanlines. The specified scanline must
    be between GetScanLineStart() and GetScanLineEnd(). Since this class uses
    the parity rule to create runs, there must be an even number of crossing
    points on each scanline, but they need not be inserted in order. This
    implies that the scanned shapes can be convex, concave or have holes, but
    they cannot cross themselves.

    @param pi_Scanline The scanline the crossing point belongs to.
    @param pi_XCrossingPoint The crossing point on the X axis.

    @return true if the specified scanline exists, false otherwise.

    @see GetScanLineStart()
    @see GetScanLineEnd()
    @see SetLimits()
    -----------------------------------------------------------------------------
*/
bool HGFScanLines::AddCrossingPoint(HSINTX pi_Scanline, double pi_XCrossingPoint)
    {
    HASSERT(m_LimitsAreSet);

    HSINTX Position = pi_Scanline - m_FirstScanline;

    if (Position >= 0 && Position < m_NumberOfScanlines)
        {
        if (m_IsRectangle)
            m_pScanLines->push_back(pi_XCrossingPoint);
        else
            m_pScanLines[Position].push_back(pi_XCrossingPoint);

        return true;
        }
    else
        {
        // Invalid position
        return false;
        }
    }


/** -----------------------------------------------------------------------------
    Start iterating on the runs. Iteration will begin at GetScanLineStart().
    There can be many runs on the same scanline, depending on the number of
    crossings. The runs will be returned from smallest to biggest on the
    X axis.

    @return true if there is a first run, false otherwise.

    @see GotoNextRun()
    @see GetCurrentScanLine()
    @see GetCurrentRun()
    -----------------------------------------------------------------------------
*/
bool HGFScanLines::GotoFirstRun()
    {
    HASSERT(m_LimitsAreSet);

    bool Result;

    if (m_LimitsAreSet &&
        m_NumberOfScanlines > 0)
        {
        // There is at least one run. Start there...

        m_CurrentScanLine = 0;
        m_PositionInScanline = 0;

        sort(m_pScanLines[0].begin(), m_pScanLines[0].end());

        Result = true;

        // If we don't get a valid first run, try advancing
        if (m_pScanLines[0].size() < 2 ||
            !m_pPixelSelector->IsAValidRun(m_pScanLines[0].at(0),
                                           m_pScanLines[0].at(1)))
            {
            if (m_IsRectangle)
                Result = false;     // The only computed scanline is invalid. Abort
            else
                Result = GotoNextRun();
            }
        }
    else
        {
        m_CurrentScanLine = m_NumberOfScanlines;

        Result = false;
        }

    return Result;
    }


/** -----------------------------------------------------------------------------
    Go to the next run. Iteration must have been previously started by calling
    GotoFirstRun().

    @return true if there is a run, false if iteration has ended.

    @see GotoFirstRun()
    @see GetCurrentScanLine()
    @see GetCurrentRun()
    -----------------------------------------------------------------------------
*/
bool HGFScanLines::GotoNextRun()
    {
    HASSERT(m_LimitsAreSet);

    bool Result = true;
    bool Found = false;

    if (m_CurrentScanLine >= m_NumberOfScanlines)
        return false;

    HSINTX Scanline = m_CurrentScanLine;
    if (m_IsRectangle)
        Scanline = 0;

    while (Result && !Found)
        {
        if (m_PositionInScanline + 3 < (HSINTX)m_pScanLines[Scanline].size())
            {
            // We have at least two more points

            m_PositionInScanline += 2;

            if (m_pPixelSelector->IsAValidRun(m_pScanLines[Scanline].at(m_PositionInScanline),
                                              m_pScanLines[Scanline].at(m_PositionInScanline+1)))
                Found = true;
            }
        else
            {
            // No more runs here, advance to next scanline

            // If we get an odd number of points, display a message
            HDEBUGCODE(if (m_PositionInScanline + 2 < (HSINTX)m_pScanLines[Scanline].size()))
                HDEBUGCODE( {)
                            HDEBUGCODE(    WChar Message[80];)
                            HDEBUGCODE(    BeStringUtilities::Snwprintf(Message, L"Error: Odd number of crossings in scanline %ld\n", m_FirstScanline + m_CurrentScanLine)); // Error: Odd number of crossings in scanline %ld\n
                            HDEBUGCODE(    HDEBUGTEXT(Message);)
                            HDEBUGCODE(
                            })

                ++m_CurrentScanLine;

            if (m_CurrentScanLine < m_NumberOfScanlines)
                {
                if (!m_IsRectangle)
                    {
                    ++Scanline;
                    sort(m_pScanLines[Scanline].begin(), m_pScanLines[Scanline].end());
                    }

                m_PositionInScanline = 0;

                // There is a following scanline. Check if the first run is valid
                if (m_PositionInScanline + 1 < (HSINTX)m_pScanLines[Scanline].size() &&
                    m_pPixelSelector->IsAValidRun(m_pScanLines[Scanline].at(m_PositionInScanline),
                                                  m_pScanLines[Scanline].at(m_PositionInScanline+1)))
                    Found = true;
                }
            else
                Result = false;
            }
        }

    return Found;
    }


/** -----------------------------------------------------------------------------
    Test if the next available run is on the same scanline or not.
    -----------------------------------------------------------------------------
*/
bool HGFScanLines::NextRunIsOnSameScanline() const
    {
    HASSERT(m_CurrentScanLine < m_NumberOfScanlines);

    HSINTX Scanline = m_CurrentScanLine;

    if (m_IsRectangle)
        Scanline = 0;

    // We need two more points, and the run must be valid
    return (m_PositionInScanline + 3 < (HSINTX)m_pScanLines[Scanline].size() &&
            m_pPixelSelector->IsAValidRun(m_pScanLines[Scanline].at(m_PositionInScanline+2),
                                          m_pScanLines[Scanline].at(m_PositionInScanline+3)));

    }


/** -----------------------------------------------------------------------------
Compute and add all crossing points for the specified segment. Crossings
will be computed for all scanlines using a variation of the Bresenham
technique.

@see SetLimits()
-----------------------------------------------------------------------------
*/
void HGFScanLines::AddCrossingPointsForSegment(const HGF2DPositionCollection& pi_rPoints)
    {
    HPRECONDITION(pi_rPoints.size());
    HPRECONDITION(m_LimitsAreSet);
    HPRECONDITION(!m_IsRectangle);    // To simplify subsequent crossings recording

    typedef struct
        {
        Byte   Orientation;
        HSINTX  StartScanline;
        HSINTX  EndScanline;
        } Segment;

    // first, scan all segment and compute the scanline
    size_t FirstSegIndex = -1;
    Segment CurSegment;
    vector<Segment> Segments;
    HGF2DPositionCollection::const_iterator Itr(pi_rPoints.begin());
    HGF2DPositionCollection::const_iterator ItrNext(pi_rPoints.begin() + 1);
    while (ItrNext != pi_rPoints.end())
        {
        if (!HDOUBLE_EQUAL_EPSILON(Itr->GetY() - ItrNext->GetY(), 0.0))
            {
            // the segment is not horizontal
            if (Itr->GetY() < ItrNext->GetY())
                {
                // the Y values increase, set the orientation to 1
                CurSegment.Orientation = 1;
                CurSegment.StartScanline = m_pPixelSelector->ConvertYMin(AdjustEndpointYValue(Itr->GetY())) - m_FirstScanline;
                CurSegment.EndScanline = m_pPixelSelector->ConvertYMax(AdjustEndpointYValue(ItrNext->GetY())) - m_FirstScanline;
                }
            else
                {
                // the Y values decrease, set the orientation to -1
                CurSegment.Orientation = -1;
                CurSegment.StartScanline = m_pPixelSelector->ConvertYMax(AdjustEndpointYValue(Itr->GetY())) - m_FirstScanline;
                CurSegment.EndScanline = m_pPixelSelector->ConvertYMin(AdjustEndpointYValue(ItrNext->GetY())) - m_FirstScanline;
                }

            if (FirstSegIndex == -1)
                FirstSegIndex = Segments.size();    // keep the first not horizontal segment
            }
        else
            {
            // the segment is horizontal, set the orientation to 0
            CurSegment.Orientation = 0;
            }

        // push the new scanline
        Segments.push_back(CurSegment);
        Itr = ItrNext;
        ItrNext++;
        }

    // pLastSegment must start with a no horizontal segment
    const Segment* pLastSegment(&Segments[FirstSegIndex]);
    size_t CurSegIndex(FirstSegIndex + 1 == Segments.size() ? 0 : FirstSegIndex + 1); // set the current index to the next segment
    const Segment* pCurSegment;
    HSINTX StartScanline;
    HSINTX EndScanline;
    HSINTX FirstScanlinePos;
    HSINTX LastScanlinePos;

    const HGF2DPosition* pStart;
    const HGF2DPosition* pEnd;
    double XDiff;
    double YDiff;

    size_t SegmentCount = Segments.size();

    // keep the last not horizontal segment orientation
    Byte LastSegOrientation = pLastSegment->Orientation;
    while (SegmentCount != 0)
        {
        pCurSegment = &Segments[CurSegIndex];
        if (pCurSegment->Orientation != (Byte)0)   // check if the segment is horizontal
            {
            if (pCurSegment->Orientation == LastSegOrientation)
                {
                // the current segment is on the same orientation than the previous one,
                // check if we have an overlap between the previous end and the current start
                if (pCurSegment->Orientation == (Byte)1)
                    {
                    StartScanline = (pLastSegment->EndScanline > pCurSegment->StartScanline ? pLastSegment->EndScanline : pCurSegment->StartScanline);
                    EndScanline = pCurSegment->EndScanline;
                    }
                else
                    {
                    HPRECONDITION(pCurSegment->Orientation == (Byte)-1);
                    StartScanline = (pLastSegment->EndScanline < pCurSegment->StartScanline ? pLastSegment->EndScanline : pCurSegment->StartScanline);
                    EndScanline = pCurSegment->EndScanline;
                    }
                }
            else
                {
                // the current segment orientation is different than the previous one
                // no overlap is possible
                StartScanline = pCurSegment->StartScanline;
                EndScanline = pCurSegment->EndScanline;
                }

            if (pCurSegment->Orientation == (Byte)1)
                {
                // y values increase, no need to swap
                pStart = &pi_rPoints[CurSegIndex];
                pEnd = &pi_rPoints[CurSegIndex + 1];
                FirstScanlinePos = StartScanline;
                LastScanlinePos = EndScanline;
                }
            else
                {
                // y values decrease, swap segment orientation
                pStart = &pi_rPoints[CurSegIndex + 1];
                pEnd = &pi_rPoints[CurSegIndex];
                FirstScanlinePos = EndScanline;
                LastScanlinePos = StartScanline;
                }

            // now, generate scanline for the current segment
            XDiff = pEnd->GetX() - pStart->GetX();
            YDiff = pEnd->GetY() - pStart->GetY();

            if (HDOUBLE_EQUAL_EPSILON(XDiff, 0.0))
                {
                for (HSINTX i = FirstScanlinePos; i < LastScanlinePos; ++i)
                    m_pScanLines[i].push_back(pStart->GetX());
                }
            else
                {
                double XDelta = XDiff / YDiff;

                // Adjust X position so it matches the Y value of the first scanline
                double CurrentX = pStart->GetX() + XDelta * ((double)(FirstScanlinePos + m_FirstScanline) + 0.5 - pStart->GetY());

                double MinX = MIN(pStart->GetX(), pEnd->GetX());
                double MaxX = MAX(pStart->GetX(), pEnd->GetX());

                for (HSINTX i = FirstScanlinePos; i < LastScanlinePos; ++i)
                    {
                    m_pScanLines[i].push_back(MAX(MIN(CurrentX, MaxX), MinX));

                    CurrentX += XDelta;
                    }
                }

            LastSegOrientation = pCurSegment->Orientation;
            pLastSegment = pCurSegment;
            }

        --SegmentCount;
        ++CurSegIndex;
        if (CurSegIndex == Segments.size())
            CurSegIndex = 0;
        }
    }



/** -----------------------------------------------------------------------------
    Compute and add all crossing points for the specified segment. Crossings
    will be computed for all scanlines using a variation of the Bresenham
    technique.

    @see SetLimits()
    -----------------------------------------------------------------------------
*/
void HGFScanLines::AddCrossingPointsForSegment(const HGF2DPosition& pi_rStart,
                                               const HGF2DPosition& pi_rEnd)
    {
    HASSERT(m_LimitsAreSet);
    HASSERT(!m_IsRectangle);    // To simplify subsequent crossings recording

    const HGF2DPosition& Start = pi_rStart.GetY() < pi_rEnd.GetY() ? pi_rStart : pi_rEnd;
    const HGF2DPosition& End   = pi_rStart.GetY() < pi_rEnd.GetY() ? pi_rEnd : pi_rStart;

    HASSERT(Start.GetY() >= m_YMin);
    HASSERT(End.GetY() <= m_YMax);

    HSINTX FirstScanlinePos = m_pPixelSelector->ConvertYMin(AdjustEndpointYValue(Start.GetY())) - m_FirstScanline;
    HSINTX LastScanlinePos = m_pPixelSelector->ConvertYMax(AdjustEndpointYValue(End.GetY())) - m_FirstScanline;


    double XDiff = End.GetX() - Start.GetX();
    double YDiff = End.GetY() - Start.GetY();

    if (HDOUBLE_GREATER_EPSILON(YDiff, 0.0))    // We don't process horizontal segments
        {
        if (HDOUBLE_EQUAL_EPSILON(XDiff, 0.0))
            {
            double CurrentX = Start.GetX();
            for (HSINTX i = FirstScanlinePos ; i < LastScanlinePos ; ++i)
                {
                m_pScanLines[i].push_back(CurrentX);
                }
            }
        else
            {
            double XDelta = XDiff / YDiff;

            // Adjust X position so it matches the Y value of the first scanline
            double CurrentX = Start.GetX() + XDelta * ((double)(FirstScanlinePos + m_FirstScanline) + 0.5 - Start.GetY());

            double MinX = MIN(Start.GetX(), End.GetX());
            double MaxX = MAX(Start.GetX(), End.GetX());

            for (HSINTX i = FirstScanlinePos; i < LastScanlinePos ; ++i)
                {
                m_pScanLines[i].push_back(MAX(MIN(CurrentX, MaxX), MinX));

                CurrentX += XDelta;
                }
            }
        }
    }


/** -----------------------------------------------------------------------------
    Provide a way to invalidate the Object when an error occur like rasterizing
    a void shape..
    -----------------------------------------------------------------------------
*/

void HGFScanLines::ResetLimits()
    {
    m_YMin                = 0.0;
    m_YMax                = 0.0;
    m_IsRectangle         = false;
    m_LimitsAreSet        = false;

    m_FirstScanline       = 0;
    m_NumberOfScanlines   = 0;

    m_CurrentScanLine     = 0;
    m_PositionInScanline  = 0;
    }


