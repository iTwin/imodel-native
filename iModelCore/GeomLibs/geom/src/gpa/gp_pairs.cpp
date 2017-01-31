/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_pairs.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static bool rangesIntersect (DRange3dCR range0, DRange3dCR range1, int workdim)
    {
    if (range0.high.x < range1.low.x)
        return false;
    if (range0.low.x > range1.high.x)
        return false;
    if (workdim < 2)
        return true;

    if (range0.high.y < range1.low.y)
        return false;
    if (range0.low.y > range1.high.y)
        return false;
    if (workdim < 3)
        return true;

    if (range0.high.z < range1.low.z)
        return false;
    if (range0.low.z > range1.high.z)
        return false;
    return true;
    }



TaggedBezierDPoint4d::TaggedBezierDPoint4d (GraphicsPointArrayCP pSource)
    {
    m_pSource = pSource;
    m_isNullInterval = false;
    m_knot0 = 0.0;
    m_knot1 = 1.0;
    m_primitiveIndex = 0;
    m_intervalIndex = 0;
    m_order = 0;
    }

bool TaggedBezierDPoint4d::LoadSingleBezier (size_t primitiveIndex)
    {
    m_knot0 = 0.0;
    m_knot1 = 1.0;
    m_primitiveIndex = primitiveIndex;
    m_intervalIndex = 0;
    m_isNullInterval = false;
    int readIndex = (int)primitiveIndex;
    return jmdlGraphicsPointArray_getBezier (
            m_pSource,
            &readIndex, m_poles, &m_order,
                                MAX_BEZIER_CURVE_ORDER)
        ? true : false;
    
    }

bool TaggedBezierDPoint4d::LoadBsplineSpan (size_t primitiveIndex, size_t spanIndex)
    {
    m_primitiveIndex = primitiveIndex;
    m_intervalIndex = spanIndex;
    return m_pSource->GetBezierSpanFromBsplineCurve (primitiveIndex, spanIndex,
                            m_poles, m_order, MAX_BEZIER_CURVE_ORDER, m_isNullInterval, m_knot0, m_knot1);
    }




TaggedBezierDPoint4d::TaggedBezierDPoint4d  (GraphicsPointArrayCP pSource,
        size_t index,
        DPoint4dCP poles, int order)
    {
    m_pSource = pSource;
    m_knot0 = 0.0;
    m_knot1 = 1.0;
    m_primitiveIndex = index;
    m_intervalIndex = 0;
    m_isNullInterval = false;
    m_order = order;
    memcpy (m_poles, poles, order * sizeof (DPoint4d));
    }

double TaggedBezierDPoint4d::LocalToGlobal (double u) const
    {
    return m_knot0 + u * (m_knot1 - m_knot0);
    }

bool TaggedBezierDPoint4d::IsNullInterval () const {return m_isNullInterval;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    processPairs_bezierDPoint4dXXX
(
void *pContext,
TaggedBezierDPoint4d &bezier0,
GraphicsPointArrayCP pSource1,
GPAPairFunc_DSegment4dBezierDPoint4dTagged        cbDSegment4dBezierDPoint4d,
GPAPairFunc_DConic4dBezierDPoint4dTagged          cbDConic4dBezierDPoint4d,
GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged    cbBezierDPoint4dBezierDPoint4d
)
    {
    int readIndex;
    int index10, index11;
    int curveType;
    for (index10 = index11 = -1;
        jmdlGraphicsPointArray_parsePrimitiveAfter (pSource1, &index10, &index11, NULL, NULL, &curveType, index11);
        )
        {
        if (curveType == 0)
            {
            DSegment4d segment1;
            if (readIndex = index10,
                jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex, &segment1))
                {
                if ( cbDSegment4dBezierDPoint4d &&
                    !cbDSegment4dBezierDPoint4d
                            (
                            pContext,
                            pSource1, index10, &segment1, bezier0
                            ))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            DConic4d conic1;
            if (readIndex = index10,
                jmdlGraphicsPointArray_getDConic4d (pSource1, &readIndex, &conic1,
                                                        NULL, NULL, NULL, NULL))
                {
                if ( cbDConic4dBezierDPoint4d &&
                    !cbDConic4dBezierDPoint4d
                            (
                            pContext,
                            pSource1, index10, &conic1,
                            bezier0
                            ))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            TaggedBezierDPoint4d bezier1 (pSource1);
            if (bezier1.LoadSingleBezier (index10))
                {
                if ( cbBezierDPoint4dBezierDPoint4d &&
                    !cbBezierDPoint4dBezierDPoint4d
                            (
                            pContext, bezier0, bezier1
                            ))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            TaggedBezierDPoint4d bezier1 (pSource1);
            for (size_t k = 0; bezier1.LoadBsplineSpan (index10, k); k++)
                {
                if ( !bezier1.IsNullInterval ()
                    && cbBezierDPoint4dBezierDPoint4d
                    && !cbBezierDPoint4dBezierDPoint4d
                            (
                            pContext, bezier0, bezier1
                            ))
                    return false;
                }
            }

        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    processPairs_DConic4dXXX
(
void *pContext,
GraphicsPointArrayCP pSource0,
int index00,
GraphicsPointArrayCP pSource1,
GPAPairFunc_DSegment4dDConic4d              cbDSegment4dDConic4d,
GPAPairFunc_DConic4dDConic4d                cbDConic4dDConic4d,
GPAPairFunc_DConic4dBezierDPoint4dTagged    cbDConic4dBezierDPoint4d
)
    {
    int readIndex;
    DConic4d conic0;
    if (readIndex = index00,
        jmdlGraphicsPointArray_getDConic4d (pSource0, &readIndex, &conic0,
                                                NULL, NULL, NULL, NULL))
        {
        int index10, index11;
        int curveType;
        for (index10 = index11 = -1;
            jmdlGraphicsPointArray_parsePrimitiveAfter (pSource1, &index10, &index11, NULL, NULL, &curveType, index11);
            )
            {
            if (curveType == 0)
                {
                DSegment4d segment1;
                if (readIndex = index10,
                    jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex, &segment1))
                    {
                    if ( cbDSegment4dDConic4d &&
                        !cbDSegment4dDConic4d
                                (
                                pContext,
                                pSource1, index10, &segment1,
                                pSource0, index00, &conic0
                                ))
                        return false;
                    }
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
                {
                DConic4d conic1;
                if (readIndex = index10,
                    jmdlGraphicsPointArray_getDConic4d (pSource1, &readIndex, &conic1,
                                                            NULL, NULL, NULL, NULL))
                    {
                    if ( cbDConic4dDConic4d &&
                        !cbDConic4dDConic4d
                                (
                                pContext,
                                pSource0, index00,  &conic0,
                                pSource1, index10, &conic1
                                ))
                        return false;
                    }
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE CODED
                {
                TaggedBezierDPoint4d bezier1 (pSource1);
                if (bezier1.LoadSingleBezier (index10))
                    {
                    if ( cbDConic4dBezierDPoint4d &&
                        !cbDConic4dBezierDPoint4d
                                (
                                pContext,
                                pSource0, index00, &conic0,
                                bezier1
                                ))
                        return false;
                    }
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
                {
                TaggedBezierDPoint4d bezier1 (pSource1);
                for (size_t k = 0;bezier1.LoadBsplineSpan (index10, k); k++)
                    {
                    if (  !bezier1.IsNullInterval ()
                       && cbDConic4dBezierDPoint4d
                       && !cbDConic4dBezierDPoint4d
                                (
                                pContext,
                                pSource0, index00, &conic0,
                                bezier1
                                ))
                        return false;
                    }
                }

            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    processPairs_DSegment4dXXX
(
void *pContext,
GraphicsPointArrayCP pSource0,
int index00,
GraphicsPointArrayCP pSource1,
GPAPairFunc_DSegment4dDSegment4d            cbDSegment4dDSegment4d,
GPAPairFunc_DSegment4dDConic4d              cbDSegment4dDConic4d,
GPAPairFunc_DSegment4dBezierDPoint4dTagged  cbDSegment4dBezierDPoint4d
)
    {
    int readIndex;
    DSegment4d segment0;
    if (readIndex = index00,
        jmdlGraphicsPointArray_getDSegment4d (pSource0,
                            &readIndex, &segment0))
        {
        int index10, index11;
        int curveType;
        for (index10 = index11 = -1;
            jmdlGraphicsPointArray_parsePrimitiveAfter (pSource1, &index10, &index11, NULL, NULL, &curveType, index11);
            )
            {
            if (curveType == 0)
                {
                DSegment4d segment1;
                if (readIndex = index10,
                    jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex, &segment1))
                    {
                    if ( cbDSegment4dDSegment4d &&
                        !cbDSegment4dDSegment4d
                                (
                                pContext,
                                pSource0, index00,  &segment0,
                                pSource1, index10, &segment1
                                ))
                        return false;
                    }
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
                {
                DConic4d conic1;
                if (readIndex = index10,
                    jmdlGraphicsPointArray_getDConic4d (pSource1, &readIndex, &conic1,
                                                            NULL, NULL, NULL, NULL))
                    {
                    if ( cbDSegment4dDConic4d &&
                        !cbDSegment4dDConic4d
                                (
                                pContext,
                                pSource0, index00,  &segment0,
                                pSource1, index10, &conic1
                                ))
                        return false;
                    }
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE CODED
                {
                TaggedBezierDPoint4d bezier1 (pSource1);
                if (bezier1.LoadSingleBezier (index10))
                    {
                    if ( cbDSegment4dBezierDPoint4d &&
                        !cbDSegment4dBezierDPoint4d
                                (
                                pContext,
                                pSource0, index00,  &segment0, bezier1
                                ))
                        return false;
                    }
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
                {
                TaggedBezierDPoint4d bezier1 (pSource1);
                for (size_t k = 0;bezier1.LoadBsplineSpan (index10, k); k++)
                    {
                    if (  !bezier1.IsNullInterval ()
                       &&  cbDSegment4dBezierDPoint4d
                       && !cbDSegment4dBezierDPoint4d
                                (
                                pContext,
                                pSource0, index00,  &segment0, bezier1
                                ))
                        return false;
                    }
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlGraphicsPointArray_processPrimitivePair
(
void			*pContext,
GraphicsPointArrayCP pSource0,
int index0,
GraphicsPointArrayCP pSource1,
int index1,
GPAPairFunc_DSegment4dDSegment4d	    cbDSegment4dDSegment4d,
GPAPairFunc_DSegment4dDConic4d		    cbDSegment4dDConic4d,
GPAPairFunc_DSegment4dBezierDPoint4dTagged	    cbDSegment4dBezierDPoint4d,
GPAPairFunc_DConic4dDConic4d		    cbDConic4dDConic4d,
GPAPairFunc_DConic4dBezierDPoint4dTagged	    cbDConic4dBezierDPoint4d,
GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged    cbBezierDPoint4dBezierDPoint4d
)
    {
    int readIndex0, readIndex1;
    DSegment4d segment0, segment1;
    DConic4d conic0, conic1;

    
    // OUTER CASE -- SEGMENT
    if (readIndex0 = index0,
	jmdlGraphicsPointArray_getDSegment4d (pSource0, &readIndex0, &segment0))
	{
        if (readIndex1 = index1,
            jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex1, &segment1))
            {
            return (NULL != cbDSegment4dDSegment4d)
                && cbDSegment4dDSegment4d
                (
                pContext,
                pSource0, index0, &segment0,
                pSource1, index1, &segment1
                );
            }
        else if (readIndex1 = index1,
	        jmdlGraphicsPointArray_getDConic4d (pSource1, &readIndex1, &conic1,
						        NULL, NULL, NULL, NULL))
                {
                return (NULL != cbDSegment4dDConic4d)
                    &&  cbDSegment4dDConic4d
                    (
                    pContext,
                    pSource0, index0,  &segment0,
                    pSource1, index1, &conic1
                    );
                }
        else
            {
            TaggedBezierDPoint4d bezier1 (pSource1);
            if (bezier1.LoadSingleBezier (index1))
                {
                return (NULL != cbDSegment4dBezierDPoint4d)
                    && cbDSegment4dBezierDPoint4d
                        (
                        pContext,
                        pSource0, index0,  &segment0,
                        bezier1
                        );
                }
            }
        }
    // OUTER CASE -- CONIC
    else if (readIndex0 = index0,
             jmdlGraphicsPointArray_getDConic4d (pSource0, &readIndex0, &conic0, NULL, NULL, NULL, NULL))
        {
        if (readIndex1 = index1,
            jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex1, &segment1))
            {
            return (NULL != cbDSegment4dDConic4d)
                && cbDSegment4dDConic4d
                (
                pContext,
                pSource1, index1, &segment1,
                pSource0, index0, &conic0
                );
            }
        else if (readIndex1 = index1,
	    jmdlGraphicsPointArray_getDConic4d (pSource1, &readIndex1, &conic1,
						    NULL, NULL, NULL, NULL))
            {
            return (NULL != cbDConic4dDConic4d)
                &&  cbDConic4dDConic4d
                (
                pContext,
                pSource0, index0, &conic0,
                pSource1, index1, &conic1
                );
            }
        else
            {
            TaggedBezierDPoint4d bezier1 (pSource1);
            if (bezier1.LoadSingleBezier (index1))
                {
                return (NULL != cbDConic4dBezierDPoint4d)
                    && cbDConic4dBezierDPoint4d
                        (
                        pContext,
                        pSource0, index0, &conic0,
                        bezier1
                        );
                }
            }
        }
    else
        {
        TaggedBezierDPoint4d bezier0 (pSource0);
        if (bezier0.LoadSingleBezier (index0))
            {
            if (readIndex1 = index1,
                jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex1, &segment1))
                {
                return (NULL != cbDSegment4dBezierDPoint4d)
                    && cbDSegment4dBezierDPoint4d
                    (
                    pContext,
                    pSource1, index1, &segment1,
                    bezier0
                    );
                }
            else if (readIndex1 = index1,
	        jmdlGraphicsPointArray_getDConic4d (pSource1, &readIndex1, &conic1,
						        NULL, NULL, NULL, NULL))
                {
                return (NULL != cbDConic4dBezierDPoint4d)
                    &&  cbDConic4dBezierDPoint4d
                    (
                    pContext,
                    pSource1, index1, &conic1,
                    bezier0
                    );
                }
            else
                {
                TaggedBezierDPoint4d bezier1 (pSource1);
                if (bezier1.LoadSingleBezier (index1))
                    {
                    return (NULL != cbBezierDPoint4dBezierDPoint4d)
                        && cbBezierDPoint4dBezierDPoint4d
                            (
                            pContext, bezier0, bezier1
                            );
                    }
                }
            }
        }
    return false;
    }

void verifyRangeRejection ()
    {
    }
/*---------------------------------------------------------------------------------**//**
* Process primitive from two GPA's pairwise -- every primitive from pSource0 with
*   every primitive from pSource1.
* @bsimethod                                                    BentleySystems  03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_processPairsWithRangeIntersection
(
void			*pContext,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
int workdim,
double rangeExpansion,
GPAPairFunc_DSegment4dDSegment4d	    cbDSegment4dDSegment4d,
GPAPairFunc_DSegment4dDConic4d		    cbDSegment4dDConic4d,
GPAPairFunc_DSegment4dBezierDPoint4dTagged	    cbDSegment4dBezierDPoint4d,
GPAPairFunc_DConic4dDConic4d		    cbDConic4dDConic4d,
GPAPairFunc_DConic4dBezierDPoint4dTagged	    cbDConic4dBezierDPoint4d,
GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged    cbBezierDPoint4dBezierDPoint4d
)
    {
    static bool s_acceptAll = false;
    bvector <DRange3dInt> range0;
    bvector <DRange3dInt> range1;

    jmdlGraphicsPointArray_collectPrimitiveRanges (pSource0, range0);
    jmdlGraphicsPointArray_collectPrimitiveRanges (pSource1, range1);

    if (rangeExpansion < 0.0)
        {
        DRange3d range;
        range.Init ();
        for (size_t i = 0, n = range0.size (); i < n; i++)
            range.Extend (range0[i].range);

        for (size_t i = 0, n = range1.size (); i < n; i++)
            range.Extend (range1[i].range);

        rangeExpansion = fabs (rangeExpansion) * range.low.Distance (range.high);
        }

    for (size_t i = 0; i < range0.size (); i++)
        range0[i].range.Extend (rangeExpansion);

    for (size_t i = 0; i < range1.size (); i++)
        range1[i].range.Extend (rangeExpansion);
    
    size_t numHit = 0;
    //size_t numCandidate = range0.size () * range1.size ();
    for (size_t i0 = 0, n0 = range0.size (); i0 < n0; i0++)
        {
        DRange3dInt r0 = range0[i0];    // will not be changed.
        for (size_t i1 = 0, n1 = range1.size (); i1 < n1; i1++)
            {
            DRange3dInt r1 = range1[i1];        // will not be changed.
            if (!rangesIntersect (r0.range, r1.range, workdim))
                {
                verifyRangeRejection ();
                }
            if (s_acceptAll || rangesIntersect (r0.range, r1.range, workdim))
                {
                numHit++;
                jmdlGraphicsPointArray_processPrimitivePair
                    (
                    pContext,
                    pSource0, r0.index,
                    pSource1, r1.index,
                    cbDSegment4dDSegment4d,
                    cbDSegment4dDConic4d,
                    cbDSegment4dBezierDPoint4d,
                    cbDConic4dDConic4d,
                    cbDConic4dBezierDPoint4d,
                    cbBezierDPoint4dBezierDPoint4d
                    );
                }
            }
        }
    //double f = numHit / (double)numCandidate;
    }


/*---------------------------------------------------------------------------------**//**
* Process primitive pairs with intersecting ranges from a single gpa.  Each pair (i,j) is
* processed only once.
* @param pContext IN caller context
* @param pSource IN curves
* @param includeSelfIntersect IN true to include pairs (i,i).
* @param workdim IN 2 for 2d range tests, 3 for 3d
* @param rangeExpansion IN all ranges are expanded by this in all directions.
* @bsimethod                                                    BentleySystems  03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_processPairsWithRangeIntersectionSingleSource
(
void			*pContext,
GraphicsPointArrayCP pSource,
bool includeSelfIntersect,
int workdim,
double rangeExpansion,
GPAPairFunc_DSegment4dDSegment4d	    cbDSegment4dDSegment4d,
GPAPairFunc_DSegment4dDConic4d		    cbDSegment4dDConic4d,
GPAPairFunc_DSegment4dBezierDPoint4dTagged	    cbDSegment4dBezierDPoint4d,
GPAPairFunc_DConic4dDConic4d		    cbDConic4dDConic4d,
GPAPairFunc_DConic4dBezierDPoint4dTagged	    cbDConic4dBezierDPoint4d,
GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged    cbBezierDPoint4dBezierDPoint4d
)
    {
    bvector <DRange3dInt> ranges;

    jmdlGraphicsPointArray_collectPrimitiveRanges (pSource, ranges);

    for (size_t i = 0, n = ranges.size (); i< n; i++)
        ranges[i].range.Extend (rangeExpansion);

    size_t numHit = 0;
    //size_t numCandidate = ranges.size () * ranges.size ();
    size_t n = ranges.size ();
    for (size_t i0 = 0; i0 < n; i0++)
        {
        DRange3dInt r0 = ranges[i0];
        for (size_t i1 = includeSelfIntersect ? i0 : i0 + 1; i1 < n; i1++)
            {
            DRange3dInt r1 = ranges[i1];
            if (rangesIntersect (r0.range, r1.range, workdim))
                {
                numHit++;
                jmdlGraphicsPointArray_processPrimitivePair
                    (
                    pContext,
                    pSource, r0.index,
                    pSource, r1.index,
                    cbDSegment4dDSegment4d,
                    cbDSegment4dDConic4d,
                    cbDSegment4dBezierDPoint4d,
                    cbDConic4dDConic4d,
                    cbDConic4dBezierDPoint4d,
                    cbBezierDPoint4dBezierDPoint4d
                    );
                }
            }
        }
//    double f = numHit / (double)numCandidate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processAllPairs
(
void                    *pContext,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
GPAPairFunc_DSegment4dDSegment4d            cbDSegment4dDSegment4d,
GPAPairFunc_DSegment4dDConic4d              cbDSegment4dDConic4d,
GPAPairFunc_DSegment4dBezierDPoint4dTagged        cbDSegment4dBezierDPoint4d,
GPAPairFunc_DConic4dDConic4d                cbDConic4dDConic4d,
GPAPairFunc_DConic4dBezierDPoint4dTagged          cbDConic4dBezierDPoint4d,
GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged    cbBezierDPoint4dBezierDPoint4d
)
    {
    int curr0, curr1;
    int curveType;
    bool    boolstat = true;

    for (curr0 = curr1 = -1;
            boolstat
        &&  jmdlGraphicsPointArray_parsePrimitiveAfter
                (pSource0, &curr0, &curr1, NULL, NULL, &curveType, curr1)
        ;)
        {
        if (curveType == 0)
            {
            boolstat = processPairs_DSegment4dXXX
                            (
                            pContext,
                            pSource0, curr0,
                            pSource1,
                            cbDSegment4dDSegment4d,
                            cbDSegment4dDConic4d,
                            cbDSegment4dBezierDPoint4d
                            );
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            boolstat = processPairs_DConic4dXXX
                            (
                            pContext,
                            pSource0, curr0,
                            pSource1,
                            cbDSegment4dDConic4d,
                            cbDConic4dDConic4d,
                            cbDConic4dBezierDPoint4d
                            );
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            TaggedBezierDPoint4d bezier0 (pSource0);
            if (bezier0.LoadSingleBezier (curr0))
                boolstat = processPairs_bezierDPoint4dXXX
                            (
                            pContext, bezier0,
                            pSource1,
                            cbDSegment4dBezierDPoint4d,
                            cbDConic4dBezierDPoint4d,
                            cbBezierDPoint4dBezierDPoint4d
                            );
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
                TaggedBezierDPoint4d bezier0 (pSource0);
                for (size_t k = 0;bezier0.LoadBsplineSpan (curr0, k); k++)
                    {
                    if (!bezier0.IsNullInterval ())
                        boolstat = processPairs_bezierDPoint4dXXX
                            (
                            pContext, bezier0,
                            pSource1,
                            cbDSegment4dBezierDPoint4d,
                            cbDConic4dBezierDPoint4d,
                            cbBezierDPoint4dBezierDPoint4d
                            );
                    }
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitive from two GPA's pairwise, proceeding in lockstep through both arrays.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processCorrespondingIndexPairs
(
void                    *pContext,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
GPAPairFunc_IndexIndex  cbIndexIndex
)
    {
    int index00, index01, index10, index11;
    bool    boolstat = true;

    for (index00 = index01 = index10 = index11 = -1;
            boolstat
        &&  jmdlGraphicsPointArray_parsePrimitiveAfter
                (pSource0, &index00, &index01, NULL, NULL, NULL, index01)
        && jmdlGraphicsPointArray_parsePrimitiveAfter
                (pSource1, &index10, &index11, NULL, NULL, NULL, index11);
        )
            {
            boolstat = cbIndexIndex (pContext, pSource0, index00, pSource1, index10);
            }

    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitive from two GPA's pairwise -- every primitive from pSource0 with
*   every primitive from pSource1.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processAllIndexedPairs
(
void                    *pContext,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
GPAPairFunc_IndexIndex  cbIndexIndex
)
    {
    int index00, index01, index10, index11;
    bool    boolstat = true;

    for (index00 = index01 = -1;
            boolstat
        &&  jmdlGraphicsPointArray_parsePrimitiveAfter
                (pSource0, &index00, &index01, NULL, NULL, NULL, index01);)
        {
        for (index10 = index11 = -1;
               boolstat
            && jmdlGraphicsPointArray_parsePrimitiveAfter
                    (pSource1, &index10, &index11, NULL, NULL, NULL, index11);)
            {
            boolstat = cbIndexIndex (pContext, pSource0, index00, pSource1, index10);
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitive from two GPA's pairwise -- every primitive from pSource0 with
*   every primitive from pSource1.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processAllIndexedTriples
(
void                    *pContext,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
GraphicsPointArrayCP pSource2,
GPATripleFunc_IndexIndexIndex  cbIndexIndexIndex
)
    {
    int index00, index01, index10, index11, index20, index21;
    bool    boolstat = true;

    for (index00 = index01 = -1;
            boolstat
        &&  jmdlGraphicsPointArray_parsePrimitiveAfter
                (pSource0, &index00, &index01, NULL, NULL, NULL, index01);)
        {
        for (index10 = index11 = -1;
               boolstat
            && jmdlGraphicsPointArray_parsePrimitiveAfter
                    (pSource1, &index10, &index11, NULL, NULL, NULL, index11);)
            {
            for (index20 = index21 = -1;
                   boolstat
                && jmdlGraphicsPointArray_parsePrimitiveAfter
                        (pSource2, &index20, &index21, NULL, NULL, NULL, index21);)
                {
                boolstat = cbIndexIndexIndex
                                    (
                                    pContext,
                                    pSource0, index00,
                                    pSource1, index10,
                                    pSource2, index20
                                    );
                }
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitives from one GPA.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processPrimitives
(
void                    *pContext,
GraphicsPointArrayCP pSource,
GPAFunc_DSegment4d              cbDSegment4dD,
GPAFunc_DConic4d                cbDConic4d,
GPAFunc_BezierDPoint4dTagged      cbBezierDPoint4d
)
    {
    int readIndex;
    int index0, index1;
    int curveType;
    for (index0 = index1 = -1;
        jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &index0, &index1, NULL, NULL, &curveType, index1);
        )
        {
        if (curveType == 0)
            {
            DSegment4d segment;
            if (readIndex = index0,
                jmdlGraphicsPointArray_getDSegment4d (pSource, &readIndex, &segment))
                {
                if ( cbDSegment4dD &&
                    !cbDSegment4dD
                            (
                            pContext,
                            pSource, index0, &segment
                            ))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            DConic4d conic;
            if (readIndex = index0,
                jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                                        NULL, NULL, NULL, NULL))
                {
                if ( cbDConic4d &&
                    !cbDConic4d (pContext, pSource, index0, &conic))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            TaggedBezierDPoint4d bezier (pSource);
            if (bezier.LoadSingleBezier (index0))
                {
                if ( cbBezierDPoint4d &&
                    !cbBezierDPoint4d (pContext, bezier))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            TaggedBezierDPoint4d bezier (pSource);
            for (size_t k = 0; bezier.LoadBsplineSpan (index0, k); k++)
                {
                if ( cbBezierDPoint4d &&
                    !cbBezierDPoint4d (pContext, bezier))
                    return false;
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitives from one GPA.  Start at the primitive "after" readIndex, stop
* at primitive with major break.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processPrimitivesToMajorBreakExt
(
void                            *pContext,
GraphicsPointArrayCP pSource,
int                             *pNextReadIndex,
int                             firstReadIndex,
GPAFunc_DSegment4d              cbDSegment4dD,
GPAFunc_DConic4d                cbDConic4d,
GPAFunc_BezierDPoint4dTagged          cbBezierDPoint4d
)
    {
    int readIndex;
    int index0, index1;
    int curveType;
    int count = 0;
    if (pNextReadIndex)
        *pNextReadIndex = firstReadIndex;
    for (index0 = index1 = firstReadIndex;
        jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &index0, &index1, NULL, NULL, &curveType, index1);
        )
        {
        count++;
        *pNextReadIndex = index1;
        if (curveType == 0)
            {
            DSegment4d segment;
            if (readIndex = index0,
                jmdlGraphicsPointArray_getDSegment4d (pSource, &readIndex, &segment))
                {
                if ( cbDSegment4dD &&
                    !cbDSegment4dD
                            (
                            pContext,
                            pSource, index0, &segment
                            ))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            DConic4d conic;
            if (readIndex = index0,
                jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                                        NULL, NULL, NULL, NULL))
                {
                if ( cbDConic4d &&
                    !cbDConic4d (pContext, pSource, index0, &conic))
                    return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER)  //BSPLINE_CODED
            {
            TaggedBezierDPoint4d bezier (pSource);
            if (bezier.LoadSingleBezier (index0))
                {
                if ( cbBezierDPoint4d &&
                    !cbBezierDPoint4d (pContext, bezier))
                return false;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            TaggedBezierDPoint4d bezier (pSource);
            for (size_t k = 0;
                bezier.LoadBsplineSpan (index0, k);
                k++)
                {
                if ( cbBezierDPoint4d &&
                    !cbBezierDPoint4d (pContext, bezier))
                    return false;
                }
            }

        if (jmdlGraphicsPointArray_isMajorBreak (pSource, index1))
            break;
        }

    return count > 0;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitives from one GPA.  Start at the primitive "after" readIndex, stop
* at primitive with major break.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_processPrimitivesToMajorBreak
(
void                            *pContext,
GraphicsPointArrayCP pSource,
int                             firstReadIndex,
GPAFunc_DSegment4d              cbDSegment4d,
GPAFunc_DConic4d                cbDConic4d,
GPAFunc_BezierDPoint4dTagged          cbBezierDPoint4d
)
    {
    int nextReadIndex;
    return jmdlGraphicsPointArray_processPrimitivesToMajorBreakExt
                (
                pContext,
                pSource,
                &nextReadIndex,
                firstReadIndex,
                cbDSegment4d,
                cbDConic4d,
                cbBezierDPoint4d
                );
    }

/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
#define MAX_TANGENTIAL_INTEGRAL 10

typedef struct
     {
    double sum[MAX_TANGENTIAL_INTEGRAL];
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int numPole;

    int numIntegral;        /* From caller */
    void *pCallerContext;   /* From caller */
    GPAFunc_TangentialIntegrand callerFunction; /* From caller */
    PFVectorIntegrand newtonCotesFunction;  /* => local function -- will couple to caller function. */
    } TangentialIntegrationContext;

/*---------------------------------------------------------------------------------**//**
* Compute path integrands at specified parameter on a bezier
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   cb_DPoint4dBezierTangentialIntegrands
(
double  *pF,
double  parameter,
TangentialIntegrationContext *pContext,
int     numFunc
)
    {
    DPoint3d point, tangent;
    bsiBezierDPoint4d_evaluateDPoint3dArray
                (
                &point, &tangent,
                pContext->poleArray, pContext->numPole,
                &parameter,
                1
                );
    pContext->callerFunction (pContext->pCallerContext, pF, numFunc, &point, &tangent);
    }




static void tangentialIntegrationDriver
(
TangentialIntegrationContext   *pContext,
double                      x0,
double                      x1
)
    {
    double integral[MAX_TANGENTIAL_INTEGRAL], error[MAX_TANGENTIAL_INTEGRAL];
    int count, i;
    static double s_absTol = 0.0;
    static double s_relTol = 1.0e-12;

    bsiMath_recursiveNewtonCotes5Vector (integral, error, &count,
                        x0, x1, s_absTol, s_relTol,
                        pContext->newtonCotesFunction,
                        pContext, pContext->numIntegral);
    for (i = 0; i < pContext->numIntegral; i++)
        {
        pContext->sum[i] += integral[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for conic tangential integrals.  Repackage
* and dispatch to more generic integrators. (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbTangentialIntegralDConic4d
(
        TangentialIntegrationContext        *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DConic4d                    *pConic
)
    {
#define MAX_POLE 5

    DPoint4d poleArray[MAX_POLE];
    DPoint3d circlePoleArray[MAX_POLE];
    int  numPole;
    int  numSpan;
    int i;
    int order = 3;
    int basePoleIndex;

    bsiDConic4d_getQuadricBezierPoles (pConic, poleArray, circlePoleArray, &numPole, &numSpan, MAX_POLE);
    pContext->newtonCotesFunction = (PFVectorIntegrand)cb_DPoint4dBezierTangentialIntegrands;
    for (i = basePoleIndex = 0; i < numSpan; i++, basePoleIndex += order - 1)
        {
        memcpy (pContext->poleArray, poleArray + basePoleIndex, order * sizeof (DPoint4d));
        pContext->numPole = order;
        tangentialIntegrationDriver
                    (
                    pContext,
                    0.0,
                    1.0
                    );
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for DPoint4d bezier tangential integrals.  Repackage
* and dispatch to more generic integrators. (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbTangentialIntegralBezierDPoint4d
(
        TangentialIntegrationContext    *pContext,
    TaggedBezierDPoint4d &bezier
)
    {
    pContext->numPole = bezier.m_order;
    if (bezier.m_order <= MAX_BEZIER_CURVE_ORDER)
        {
        memcpy (pContext->poleArray, bezier.m_poles, bezier.m_order * sizeof (DPoint4d));
        pContext->newtonCotesFunction = (PFVectorIntegrand)cb_DPoint4dBezierTangentialIntegrands;
        tangentialIntegrationDriver
                    (
                    pContext, 0.0, 1.0
                    );
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for DSegment4d tangential integrals.  Repackage
* and dispatch to more generic integrators. (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbTangentialIntegralDSegment4d
(
        TangentialIntegrationContext *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DSegment4d                  *pSegment
)
    {
    pContext->numPole = 2;
    pContext->poleArray[0] = pSegment->point[0];
    pContext->poleArray[1] = pSegment->point[1];
    pContext->newtonCotesFunction = (PFVectorIntegrand)cb_DPoint4dBezierTangentialIntegrands;
    tangentialIntegrationDriver
                    (
                    pContext, 0.0, 1.0
                    );
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Computes integrals primitive-by-primitive through a major break.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_integratePrimitivesToMajorBreak
(
GraphicsPointArrayCP pSource,
int                             *pNextReadIndex,
double                          *pIntegral,
int                             numIntegral,
int                             firstReadIndex,
GPAFunc_TangentialIntegrand     cbTangentialIntegrand,
void                            *pContext
)
    {
    bool    boolstat = false;
    TangentialIntegrationContext params;
    memset (pIntegral, 0, numIntegral * sizeof (double));

    if (numIntegral > MAX_TANGENTIAL_INTEGRAL)
        return false;

    memset (&params, 0, sizeof (TangentialIntegrationContext));
    params.numIntegral = numIntegral;
    params.pCallerContext = pContext;
    params.callerFunction = cbTangentialIntegrand;

    boolstat = jmdlGraphicsPointArray_processPrimitivesToMajorBreakExt
                    (
                    &params,
                    pSource,
                    pNextReadIndex,
                    firstReadIndex,
                    (GPAFunc_DSegment4d)cbTangentialIntegralDSegment4d,
                    (GPAFunc_DConic4d)cbTangentialIntegralDConic4d,
                    (GPAFunc_BezierDPoint4dTagged)cbTangentialIntegralBezierDPoint4d
                    );

    memcpy (pIntegral, params.sum, numIntegral * sizeof (double));
    return boolstat;
    }



END_BENTLEY_GEOMETRY_NAMESPACE