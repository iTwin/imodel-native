/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Map signed, nonperiodic source integers onto an unsigned periodic integer target space.
// The source integers are a nominal sequence [0,1,...sourceCount-1].
// An integer i in the source space (but NOT limited to the [0..sourceCount-1] interval) is mapped via
//!<ul>
//!<li>An unbounded linear mapping {k = targetIndex0 + i * targetStep}.  This can be negative.
//!<li>Restrict to periodic limits: {result = (k % targetPeriod)} where the modulo operator allows negatives.
//!</ul>
//!
struct CyclicIndexMap
{
private:
size_t m_sourceCount;

size_t m_targetIndex0;
size_t m_targetStep;
size_t m_targetPeriod;

public:
CyclicIndexMap (size_t sourceCount, size_t targetIndex0, int targetStep, size_t targetPeriod)
    : m_sourceCount (sourceCount),
      m_targetIndex0 (targetIndex0),
      m_targetPeriod (targetPeriod),
      m_targetStep (targetStep)
    {
    if (m_targetPeriod < 1)
        m_targetPeriod = 1;
    }

//! Return the nominal number of source indices.
size_t SourceCount () { return m_sourceCount;}

//! Test if sourceIndex is in the nominal [0..sourceCount-1} range.
bool IsValidSourceIndex (ptrdiff_t sourceIndex)
    {
    return 0 <= sourceIndex && sourceIndex < (ptrdiff_t)m_sourceCount;
    }

//! Map (unrestricted) sourceIndex to the (restricted) target space.
size_t Map (ptrdiff_t sourceIndex)
    {
    ptrdiff_t index = m_targetIndex0 + sourceIndex * m_targetStep;
    // We expect the most common case will require no periodic adjustment.
    // We expect that adjustment by a single period is next most common.
    // Adjustment by multiple periods is EXTREMELY uncommon.
    // Hence write the code using single period add and subtract rather than the (more expensive) modulo operator.
    if (index < 0)
        {
        do
            {
            index += m_targetPeriod;
            } while (index < 0);
        }
    else if (index > (ptrdiff_t)m_targetPeriod)
        {
        do
            {
            index -= m_targetPeriod;
            } while (index > (ptrdiff_t)m_targetPeriod);
        }
    return (size_t)index;
    }

size_t MapFirst () {return Map (0);}
size_t MapLast () {return Map (m_sourceCount - 1);}
};
#ifdef CompileAllPolylineIntersections
struct PolylineIntersector
{
bvector<DPoint3d>const &m_pointA;
bvector<DPoint3d>const &m_pointB;
bvector<double> *m_paramA;
bvector<double> *m_paramB;
bvector<DPoint3d>*m_intersectionA;
bvector<DPoint3d>*m_intersectionB;

PolylineIntersector (){}

void AddIntersection (double paramA, DPoint3dCR intersectionPointA, double paramB, DPoint3dCR intersectionPointB)
    {
    if (NULL != m_paramA)
        m_paramA->push_back (paramA);
    if (NULL != m_intersectionA)
        m_intersectionA->push_back (intersectionPointA);

    if (NULL != m_paramB)
        m_paramB->push_back (paramB);
    if (NULL != m_intersectionB)
        m_intersectionB->push_back (intersectionPointB);
    }

void DoIntersection_IncreasingX
(
DPoint3dCR pointA0,
DPoint3dCR pointA1,
CyclicIndexMap &indexA,
size_t iA1,
DPoint3dCR pointB0,
DPoint3dCR pointB1,
CyclicIndexMap &indexB,
size_t iB1
)
    {
    if (pointB0.x > pointA1.x)
        return;
    if (pointA0.x > pointB1.x)
        return;
    }

void IntersectionIncreasingX (CyclicIndexMap &indexA, CyclicIndexMap &indexB)
    {
    DPoint3d pointA0 = m_pointA[indexA.MapFirst ()];
    DPoint3d pointA1 = m_pointA[indexA.MapLast ()];
    DPoint3d pointB0 = m_pointB[indexB.MapFirst ()];
    DPoint3d pointB1 = m_pointB[indexB.MapLast ()];
    if (pointA1.x < pointB0.x)
        return;
    if (pointB1.x < pointA0.x)
        return;
    size_t nA = indexA.SourceCount ();
    size_t nB = indexB.SourceCount ();
    size_t iA1 = 1;
    size_t iB1 = 1;
    for (;iA1 < nA && iB1 < nB;)
        {
        pointA1 = m_pointA[indexA.Map (iA1)];
        pointB1 = m_pointA[indexA.Map (iB1)];
        DoIntersection_IncreasingX (pointA0, pointA1, indexA, iA1, pointB0, pointB1, indexB, iB1);
        if (pointA0.x <= pointB0.x)
            {
            if (pointB1.x < pointA1.x)
                {
                pointB0 = pointB1;
                iB1++;
                }
            else
                {
                pointA0 = pointA1;
                iA1++;
                }
            }
        else
            {
            if (pointA1.x < pointB1.x)
                {
                pointA0 = pointA1;
                iA1++;
                }
            else
                {
                pointB0 = pointB1;
                iB1++;
                }
            }
        }
    }


};
#endif
END_BENTLEY_GEOMETRY_NAMESPACE