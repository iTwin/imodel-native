/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gpa_parser.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

GraphicsPointArray::Parser::Parser (GraphicsPointArrayCP gpa)
    : m_gpa (gpa)
    {
    Reset ();
    }

void GraphicsPointArray::Parser::Attach (GraphicsPointArrayCP gpa)
    {
    m_gpa = gpa;
    Reset ();
    }

GraphicsPointArray::Parser::Parser (GraphicsPointArray::Parser const & other)
    : m_gpa (other.m_gpa)
    {
    m_i0 = other.m_i0;
    m_i1 = other.m_i1;
    m_curveType = other.m_curveType;
    }

GraphicsPointArray::Parser::Parser (GraphicsPointArrayCP gpa, size_t i0, size_t i1, int curveType)
    : m_gpa (gpa)
    {
    m_i0 = i0;
    m_i1 = i1;
    m_curveType = curveType;
    }


void GraphicsPointArray::Parser::Reset ()
    {
    m_i0 = m_i1 = 0;
    m_curveType = 0;    
    }

void GraphicsPointArray::Parser::ResetAtEnd ()
    {
    m_i0 = m_i1 = m_gpa->vbArray_hdr.size ();
    m_curveType = 0;    
    }

bool GraphicsPointArray::Parser::IsValid () const
    {
    return m_i1 > m_i0 && m_gpa->IsValidIndex (m_i1);
    }

bool GraphicsPointArray::Parser::HasMore () const
    {
    // EDL 6/26/10 this was just m_i1 < size.
    // This allowed the final parser cursor postion to look live.
    return m_i1 + 1 < m_gpa->vbArray_hdr.size ();
    }


void GEOMDLLIMPEXP GraphicsPointArray::MarkBreak ()
    {
    size_t n = vbArray_hdr.size ();
    if (n > 0)
        vbArray_hdr[n-1].mask |= HPOINT_MASK_BREAK;
    }

void GEOMDLLIMPEXP GraphicsPointArray::MarkMajorBreak ()
    {
    size_t n = vbArray_hdr.size ();
    if (n > 0)
        {
        vbArray_hdr[n-1].mask |= HPOINT_MASK_MAJOR_BREAK | HPOINT_MASK_BREAK;
        SetArrayMask (HPOINT_ARRAYMASK_HAS_MAJOR_BREAKS);
        }
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsMajorBreak (size_t i) const
    {
    size_t n = vbArray_hdr.size ();
    return      i < n
            && (vbArray_hdr[i].mask & HPOINT_MASK_MAJOR_BREAK) != 0;
    
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsBreak (size_t i) const
    {
    size_t n = vbArray_hdr.size ();
    return      i < n
            && (vbArray_hdr[i].mask & HPOINT_MASK_BREAK) != 0;    
    }


size_t GEOMDLLIMPEXP GraphicsPointArray::ResolveIndex (int i) const
    {
    if (i >= 0)
        return i;
    size_t n = vbArray_hdr.size ();
    if (n > 0)
        return n - 1;
    return 0;
    }

size_t GraphicsPointArray::Parser::GetReadIndex () const {return m_i0;}
size_t GraphicsPointArray::Parser::GetReadIndexTail () const {return m_i1;}
int GraphicsPointArray::Parser::GetCurveType () const    {return m_curveType;}
bool GraphicsPointArray::Parser::SameIndicesAndCurveType
    (
    GraphicsPointArray::Parser const &other
    ) const
    {
    return m_i0 == other.m_i0
        && m_i1 == other.m_i1
        && m_curveType == other.m_curveType;
    }
bool GraphicsPointArray::FindCurvePointTypeBefore (size_t startIndex,
            int curveType, int pointType, size_t &foundAt) const
    {
    size_t i = startIndex;
    size_t n = vbArray_hdr.size ();
    if (i >= n)
        i = n - 1;
        
    int mask = HPOINT_MASK_CURVE_BITS;
    int targetMask = curveType | pointType;
    while (i > 0)
        {
        i--;
        if ((vbArray_hdr[i].mask & mask) == targetMask)
            {
            foundAt = i;
            return true;
            }
        }        
    return i < startIndex;
    }

bool GraphicsPointArray::FindCurvePointTypeAfter (size_t startIndex,
            int curveType, int pointType, size_t &foundAt) const
    {
    size_t i = startIndex + 1;
    size_t n = vbArray_hdr.size ();
    int mask = HPOINT_MASK_CURVE_BITS;
    int targetMask = curveType | pointType;
    while (i < n)
        {
        if ((vbArray_hdr[i].mask & mask) == targetMask)
            {
            foundAt = i;
            return true;
            }
        i++;
        }
    return i > startIndex;
    }


bool GraphicsPointArray::Parser::MoveToPrimitive (size_t startIndex)
    {
    size_t tailIndex;
    if      (m_gpa->IsLineSegment (startIndex, tailIndex))
        Set (startIndex, tailIndex, 0);
    else if (m_gpa->IsConic (startIndex, tailIndex))
        Set (startIndex, tailIndex, HPOINT_MASK_CURVETYPE_ELLIPSE);
    else if (m_gpa->IsBsplineCurve (startIndex, tailIndex))
        Set (startIndex, tailIndex, HPOINT_MASK_CURVETYPE_BSPLINE);
    else if (m_gpa->IsBezierCurve (startIndex, tailIndex))
        Set (startIndex, tailIndex, HPOINT_MASK_CURVETYPE_BEZIER);
    else
        return false;
    return true;
    }

void GraphicsPointArray::Parser::Set (size_t i0, size_t i1, int curveType)
    {
    m_i0 = i0;
    m_i1 = i1;
    m_curveType = curveType;
    }


bool GraphicsPointArray::Parser::MoveToNextPrimitive ()
    {
    size_t i0;
    //size_t n = m_gpa->size ();
    GraphicsPoint gp0, gp1, gp2;
    // Make i0 = tentative start of next primitive, trusting m_curveType ...
    if (m_i0 == 0 && m_i1 == 0)
        {
        i0 = 0;
        }
    else if (m_curveType == 0)  // Continuing in linestring ...
        {
        // Is i1 internal to a linestring?
        if (   m_gpa->GetGraphicsPoint (m_i1, gp1)
            && gp1.GetCurveType () == 0
            && !m_gpa->IsBreak (m_i1)
            && m_gpa->GetGraphicsPoint(m_i1 + 1, gp2)
            && gp2.GetCurveType () == 0)
            {
            i0 = m_i1;
            }
        else
            {
            i0 = m_i1 + 1;
            }
        }
    else
        {
        i0 = m_i1 + 1;
        }

    if (!m_gpa->GetGraphicsPoint(i0, gp0))
        return false;

    m_curveType = gp0.GetCurveType ();
    if (m_curveType == 0)
        {
        m_i0 = i0;
        m_i1 = i0 + 1;
        return m_gpa->GetGraphicsPoint (m_i1, gp1)
                && gp1.GetCurveType () == 0;
        }

    if (m_curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        m_i0 = i0;
        m_i1 = i0 + 4;
        return true;
        }

    if (m_curveType == HPOINT_MASK_CURVETYPE_BEZIER)
        {
        m_i0 = i0;
        return m_gpa->IsBezierCurve (m_i0, m_i1);
        }

    if (m_curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        m_i0 = i0;
        return m_gpa->IsBsplineCurve (m_i0, m_i1);
        }

    return false;
    }

bool GraphicsPointArray::Parser::MoveToNextFragment ()
    {
    if (MoveToNextPrimitive ())
        {
        GraphicsPoint gp;
        if (m_curveType == 0
            && m_gpa->GetGraphicsPoint (m_i1, gp))
            {
            while (   !gp.IsCurveBreak ()
                   && m_gpa->GetGraphicsPoint (m_i1 + 1, gp)
                   && gp.GetCurveType () == 0)
                {
                m_i1++;
                }
            }
        return true;
        }
    return false;
    }



bool GraphicsPointArray::Parser::MoveToPreviousFragment ()
    {
    size_t i1;
    size_t n = m_gpa->vbArray_hdr.size ();
    GraphicsPoint gp;
    // Make i0 = tentative start of next primitive, trusting m_curveType ...
    if (m_i0 == n && m_i1 == n)
        {
        i1 = n - 1;
        }
    else
        {
        i1 = m_i0 - 1;
        }

    if (!m_gpa->GetGraphicsPoint(i1, gp))
        return false;

    m_curveType = gp.GetCurveType ();
    if (m_curveType == 0)
        {
        m_i1 = i1;
        m_i0 = i1;
        while (m_i0 > 0)
            {
            size_t i0 = m_i0 - 1;
            GraphicsPoint gp = m_gpa->vbArray_hdr[i0];
            if (gp.GetCurveType () == 0
                    && !gp.IsCurveBreak ())
                m_i0--;
            else
                break;
            }
        return m_i0 < m_i1;
        }

    if (m_curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        m_i1 = i1;
        m_i0 = i1 - 4;
        return m_gpa->IsConic (m_i0, i1);
        }

    //int targetMask = HPOINT_MASK_CURVETYPE_BEZIER;
    if (m_curveType == HPOINT_MASK_CURVETYPE_BEZIER)
        {
        m_i1 = i1;
        return m_gpa->FindCurvePointTypeBefore (m_i1, 
                    HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_STARTEND, m_i0);
        }

    if (m_curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        m_i1 = i1;
        return m_gpa->FindCurvePointTypeBefore (m_i1, 
                    HPOINT_MASK_CURVETYPE_BSPLINE, HPOINT_MASK_BSPLINE_STARTEND, m_i0);
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsPrimitiveIndex (size_t index) const
    {
    size_t i1;
    return IsLineSegment (index, i1)
        || IsConic (index, i1)
        || IsBezierCurve (index, i1)
        || IsBsplineCurve (index);
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetLastPrimitiveIndex (size_t &index) const
    {
    Parser parser (this);
    parser.ResetAtEnd ();
    if (parser.MoveToPreviousPrimitive ())
        {
        index = parser.GetReadIndex ();
        return true;
        }
    return false;
    }

bool GraphicsPointArray::Parser::MoveToPreviousPrimitive ()
    {
    size_t i0, i1;
    size_t n = m_gpa->vbArray_hdr.size ();
    GraphicsPoint gp;
    if (m_i0 == 0)
        return false;
    // Naively, we move m_i1 back to m_i0 -1, and then push m_i0 beyond that.
    // But we have to allow for chained linestrings ...
    // Make i1 = tentative end of next primitive (but direct backup in linestring
    if (m_i0 >= n || m_i1 >= n)
        {
        i1 = n - 1;
        }
    else
        {
        if (   m_gpa->GetGraphicsPoint(m_i0, gp)
            && gp.GetCurveType () == 0
            && !gp.IsCurveBreak ()
            && m_gpa->GetGraphicsPoint (m_i0 - 1, gp)
            && gp.GetCurveType () == 0
            && !gp.IsCurveBreak ())
            {
            m_i1 = m_i0--;
            m_curveType = 0;
            return true;
            }
        i1 = m_i0 - 1;
        }

    if (!m_gpa->GetGraphicsPoint(i1, gp))
        return false;

    m_curveType = gp.GetCurveType ();
    if (m_curveType == 0)
        {
        i0 = i1 - 1;
        if (m_gpa->GetGraphicsPoint (i0, gp)
            && gp.GetCurveType () == 0)
            {
            m_i1 = i1;
            m_i0 = i0;
            m_curveType = 0;
            return true;
            }
        }
    else if (m_curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        m_i1 = i1;
        m_i0 = i1 - 4;
        return m_gpa->IsConic (m_i0, i1);
        }
    else if (m_curveType == HPOINT_MASK_CURVETYPE_BEZIER)
        {
        m_i1 = i1;
        return m_gpa->FindCurvePointTypeBefore (m_i1, 
                    HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_STARTEND, m_i0);
        }
    else if (m_curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        m_i1 = i1;
        return m_gpa->FindCurvePointTypeBefore (m_i1, 
                    HPOINT_MASK_CURVETYPE_BSPLINE, HPOINT_MASK_BSPLINE_STARTEND, m_i0);
        }
    return false;
    }


bool GEOMDLLIMPEXP GraphicsPointArray::GetPointMask (size_t i, int mask, int &returnedMask) const
    {
    GraphicsPoint gp;
    if (!GetGraphicsPoint (i, gp))
        {
        returnedMask = 0;
        return false;
        }
    returnedMask = mask & gp.mask;
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::SetPointMask (size_t i, int mask)
    {
    GraphicsPoint gp;
    if (i < vbArray_hdr.size ())
        {
        vbArray_hdr[i].mask |= mask;
        return true;
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetCheckedGraphicsPoint (size_t i, GraphicsPointR gp, int curveType, int pointType) const
    {
    if (i >= vbArray_hdr.size ())
        {
        memset (&gp, 0, sizeof (gp));
        return false;
        }
    gp = vbArray_hdr[i];
    int mask0 = (curveType | pointType);
    int mask1 = gp.mask & (HPOINT_MASK_CURVETYPE_BITS | HPOINT_MASK_POINTTYPE_BITS);
    return mask0 == mask1;
    }


bool GEOMDLLIMPEXP GraphicsPointArray::IsCurvePointType
(
size_t readIndex,
int    curveType,
int    pointType
) const
    {
    size_t n = vbArray_hdr.size ();
    if (readIndex < n)
        {
        int mask = vbArray_hdr[readIndex].mask;
        return ((mask & HPOINT_MASK_CURVETYPE_BITS) == curveType)
            && ((mask & HPOINT_MASK_POINTTYPE_BITS) == pointType);
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsLineSegment
(
size_t readIndex,
size_t &tailIndex
) const
    {
    size_t n = vbArray_hdr.size ();
    tailIndex = readIndex + 1;
    if (tailIndex < n)
        {
        int mask0 = vbArray_hdr[readIndex].mask;
        if ((mask0 & HPOINT_MASK_CURVETYPE_BITS) != 0)
            return false;
        int mask1 = vbArray_hdr[tailIndex].mask;
        if ((mask1 & HPOINT_MASK_CURVETYPE_BITS) != 0)
            return false;
        return true;
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsLineString
(
size_t readIndex,
size_t &tailIndex
) const
    {
    size_t n = vbArray_hdr.size ();
    int mask0 = vbArray_hdr[readIndex].mask;
    if ((mask0 & HPOINT_MASK_CURVETYPE_BITS) != 0)
        return false;

    tailIndex = readIndex + 1;
    for (tailIndex = readIndex;
         tailIndex < n;
         tailIndex++)
        {
        int mask1 = vbArray_hdr[tailIndex].mask;
        // Have we walked past the linestring part?
        if ((mask1 & HPOINT_MASK_CURVETYPE_BITS) != 0)
            break;
        // last point in linestring?
        if ((mask1 & HPOINT_MASK_BREAK) != 0)
            {
            tailIndex++;
            break;
            }
        }
    // tailIndex is AFTER the end of linestring...  
    tailIndex--;
    return tailIndex > readIndex;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetNextPrimitiveReadIndex (size_t readIndex, size_t &nextReadIndex) const
    {
    Parser parser (this, readIndex, readIndex, 0);
    if (parser.MoveToNextPrimitive ())
        {
        nextReadIndex = parser.GetReadIndex ();
        return true;
        }
    nextReadIndex = vbArray_hdr.size ();
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetPreviousPrimitiveReadIndex (size_t readIndex, size_t &previousReadIndex) const
    {
    Parser parser (this, readIndex, readIndex, 0);
    if (parser.MoveToPreviousPrimitive ())
        {
        previousReadIndex = parser.GetReadIndex ();
        return true;
        }
    previousReadIndex = readIndex;
    return false;
    }



bool GEOMDLLIMPEXP GraphicsPointArray::IsConic
(
size_t readIndex,
size_t &tailIndex
) const
    {
    GraphicsPoint gp;
    tailIndex = readIndex + 4;
    if (   IsCurvePointType (readIndex,
                HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_STARTEND)
       && IsCurvePointType (readIndex +1,
                HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_VECTOR)
       && IsCurvePointType (readIndex + 2,
                HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_CENTER)
       && IsCurvePointType (readIndex + 3,
                HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_VECTOR)
       && IsCurvePointType (readIndex + 4,
                HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_STARTEND)
        )
        {
        return true;
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsBezierCurve
(
size_t readIndex,
size_t &tailIndex
) const
    {
    GraphicsPoint gp0, gp1;
    if (!GetCheckedGraphicsPoint (readIndex, gp0,
            HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_STARTEND))
        return false;
    tailIndex = readIndex + 1;
    while (GetCheckedGraphicsPoint (tailIndex, gp1,
            HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_POLE))
        {
        tailIndex++;
        }
    // Should last point have break bit?
    if (GetCheckedGraphicsPoint (tailIndex, gp1,
            HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_STARTEND))
        return true;
    // hmm.. bad data if we got here.
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsBsplineCurveTail (size_t i1, size_t &i0, bool checkAll) const
    {
    GraphicsPoint gp0, gp1;
    if (!GetGraphicsPoint (i1, gp1))
        return false;
    i0 = i1 - 1;
    for (i0 = i1;
           i1 > 0
        && GetGraphicsPoint (--i0, gp0)
        && gp0.GetCurveType () == HPOINT_MASK_CURVETYPE_BSPLINE
        ;)
        {
        if (gp0.GetPointType () == HPOINT_MASK_BSPLINE_STARTEND)
            {
            if (!checkAll)
                return true;
            size_t j1;
            return IsBsplineCurve (i0, j1, checkAll) && j1 == i1;
            }
        }
    return false;    
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsBsplineCurve (size_t i0) const
    {
    size_t i1;
    return IsBsplineCurve (i0, i1, false);
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsBsplineCurve (size_t i0, size_t &i1, bool checkAll) const
    {
    GraphicsPoint gp0, gp1;
    i1 = i0;
    if (!GetGraphicsPoint (i0, gp0))
        return false;
    if (gp0.index <= 0)
        return false;
    i1 = i0 + gp0.index;
    if (!GetGraphicsPoint (i1, gp1))
        return false;
    int order = gp0.GetOrder ();
    if (   gp0.GetCurveType () != HPOINT_MASK_CURVETYPE_BSPLINE
        || gp0.GetPointType () != HPOINT_MASK_BSPLINE_STARTEND
        || gp1.GetCurveType () != HPOINT_MASK_CURVETYPE_BSPLINE
        || gp1.GetPointType () != HPOINT_MASK_BSPLINE_STARTEND
        || gp1.index != 0
        || !gp1.IsCurveBreak ())
        return false;

    if (!checkAll)
        return true;

    // Confirm:  (i0..i1) is all bspline curve
    // no need to test against the array size -- i1 is already know safe.
    int targetPointType = HPOINT_MASK_BSPLINE_EXTRA_POLE;
    size_t firstPoleIndex = i0 + order - 1;
    for (size_t i = i0 + 1; i < i1; i++)
        {
        GraphicsPoint gp = vbArray_hdr[i];
        if (HPOINT_GET_CURVETYPE_BITS (gp.mask) != HPOINT_MASK_CURVETYPE_BSPLINE)
            return false;
        if (i >= firstPoleIndex)
            targetPointType = HPOINT_MASK_BSPLINE_POLE;
        if (HPOINT_GET_POINTTYPE_BITS (gp.mask) != targetPointType)
            return false;
        if (gp.GetOrder () != order)
            return false;
        if (i + gp.index != i1)
            return false;
        }

    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::AppendPrimitiveFrom
(
GraphicsPointArrayCR source,
size_t index
)
    {
    GraphicsPointArray::Parser parser (&source);
    if (parser.MoveToPrimitive (index))
        {
        for (size_t i = parser.m_i0; i <= parser.m_i1; i++)
            vbArray_hdr.push_back (source.vbArray_hdr[i]);
        arrayMask |= source.arrayMask & HPOINT_ARRAYMASK_CURVES;
        return true;
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::AppendIntervalFrom
(
GraphicsPointArrayCR source,
size_t index0,
double fraction0,
size_t index1,
double fraction1
)
    {
#ifdef abc
    if (index0 == index1)
        return AppendPartialPrimitveFrom (source, index0, fraction0, fraction1);
    else if (index0 < index1)
        {
        return AppendPartialPrimitiveFrom (source, index0, fraction0, 1.0);
        return AppendPartialPrimitiveFrom (source, index1, 0.0, fraction1);
        }
    else
        {
        return AppendPartialPrimitiveFrom (source, index1, fraction0, 0.0);
        for (
        return AppendPartialPrimitiveFrom (source, index0, 1.0, fraction1);
        }
#else
    return jmdlGraphicsPointArray_appendInterval (this, &source,
                        (int)index0, fraction0, (int)index1, fraction1)
        ? true : false;
#endif
    }

bool GraphicsPointArray::FindMajorBreakAfter (size_t index, size_t &foundIndex) const
    {
    size_t n = vbArray_hdr.size ();
    for (size_t i = index; i < n; i++)
        {
        if (vbArray_hdr[i].IsLoopBreak ())
            {
            foundIndex = i;
            return true;
            }
        }
    foundIndex = n - 1;
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
