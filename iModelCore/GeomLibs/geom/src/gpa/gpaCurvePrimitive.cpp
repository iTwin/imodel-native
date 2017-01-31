/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gpaCurvePrimitive.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* Load B-Spline curve into a graphics point array -- only handles easy
* special cases.
* @bsimethod                                                    RayBentley      06/01
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   addBsplineCurveSpecialCases (GraphicsPointArrayP pGPA, MSBsplineCurveCP pCurve)
    {
    GraphicsPoint   gp;
    int             order = pCurve->params.order;

    // linear B-spline curves are stored as linestrings
    if (order == 2)
        {
        if (pCurve->params.closed)
            {
            if (pCurve->params.numKnots == pCurve->params.numPoles - 1)
                {
                /* In addition to the indicated interior knot count,
                    the knot array is sandwiched by one start/end knot and one
                    wraparound knot at each end. */
                double      *pKnotBuffer    = pCurve->knots + order - 1;
                DPoint3d    *pPoleBuffer    = pCurve->poles;
                double      *pWeightBuffer  = pCurve->weights;
                int         i;
                for (i = 0; i < pCurve->params.numPoles; i++)
                    {
                    gp.point.x  = pPoleBuffer[i].x;
                    gp.point.y  = pPoleBuffer[i].y;
                    gp.point.z  = pPoleBuffer[i].z;
                    gp.point.w  = pCurve->rational ? pWeightBuffer[i] : 1.0;
                    gp.mask     = 0;
                    gp.userData = 0;
                    gp.a        = pKnotBuffer[i];
                    pGPA->Add (gp);
                    }

                /* last pole is first */
                gp.point.x  = pPoleBuffer[0].x;
                gp.point.y  = pPoleBuffer[0].y;
                gp.point.z  = pPoleBuffer[0].z;
                gp.point.w  = pCurve->rational ? pWeightBuffer[0] : 1.0;
                gp.mask     = 0;
                gp.userData = 0;
                gp.a        = pKnotBuffer[pCurve->params.numPoles];
                pGPA->Add (gp);
                pGPA->MarkBreak ();
                return SUCCESS;
                }
            }
        else /* open */
            {
            if (pCurve->params.numKnots == pCurve->params.numPoles - order)
                {
                /* In addition to the indicated interior knot count,
                    the knot array contains double knots at start and end. */
                double      *pKnotBuffer    = pCurve->knots + order - 1;
                DPoint3d    *pPoleBuffer    = pCurve->poles;
                double      *pWeightBuffer  = pCurve->weights;
                int         i;
                for (i = 0; i < pCurve->params.numPoles; i++)
                    {
                    gp.point.x  = pPoleBuffer[i].x;
                    gp.point.y  = pPoleBuffer[i].y;
                    gp.point.z  = pPoleBuffer[i].z;
                    gp.point.w  = pCurve->rational ? pWeightBuffer[i] : 1.0;
                    gp.mask     = 0;
                    gp.userData = 0;
                    gp.a        = pKnotBuffer[i];
                    pGPA->Add (gp);
                    }

                pGPA->MarkBreak ();
                return SUCCESS;
                }
            }
        }
    return ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AppendCurveVectorToGraphicsPointArray (CurveVectorCR curves, GraphicsPointArrayP gpa, bool splinesAsBezier)
    {
    if (1 > curves.size ())
        return ERROR;

    CurveVector::BoundaryType boundaryType = curves.GetBoundaryType ();
    bool        majorBreaksBetweenVectorsRequired    = (CurveVector::BOUNDARY_TYPE_Outer == boundaryType || CurveVector::BOUNDARY_TYPE_Inner == boundaryType);
    bool        majorBreaksBetweenPrimitivesRequired = (CurveVector::BOUNDARY_TYPE_None == boundaryType);
    bool        isFirst = true;

    for(ICurvePrimitivePtr curve: curves)
        {
        if (!curve.IsValid ())
            return ERROR;

        if (!isFirst && majorBreaksBetweenPrimitivesRequired)
            gpa->MarkMajorBreak ();

        isFirst = false;

        switch (curve->GetCurvePrimitiveType ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3d  segment = *curve->GetLineCP ();

                gpa->AddArray (&segment.point[0], 2);
                gpa->MarkBreak ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d> const* points = curve->GetLineStringCP ();

                gpa->AddArray (&points->front (), (int) points->size ());
                gpa->MarkBreak ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3d  ellipse = *curve->GetArcCP ();

                gpa->Add (ellipse);
                gpa->MarkBreak ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                MSBsplineCurveCP bcurve = curve->GetBsplineCurveCP ();
        
                if (SUCCESS != addBsplineCurveSpecialCases (gpa, bcurve))
                    gpa->AddBsplineCurve (*bcurve, splinesAsBezier);

                gpa->MarkBreak ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                {
                MSInterpolationCurveCP  fitCurve = curve->GetInterpolationCurveCP ();
                MSBsplineCurve          curve;
        
                if (SUCCESS != mdlBspline_convertInterpolationToBspline (&curve, const_cast <MSInterpolationCurveP> (fitCurve)))
                    return ERROR;

                gpa->AddBsplineCurve (curve, splinesAsBezier);
                gpa->MarkBreak ();

                curve.ReleaseMem ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                {
                bvector<DPoint3d> const* points = curve->GetAkimaCurveCP ();

                MSBsplineCurve  curve;

                if (SUCCESS != curve.InitAkima (&points->front (), points->size ()))
                    return ERROR;

                gpa->AddBsplineCurve (curve, splinesAsBezier);
                gpa->MarkBreak ();

                curve.ReleaseMem ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                {
                bvector<DPoint3d> const* points = curve->GetPointStringCP ();

                gpa->AddArray (&points->front (), (int) points->size ());
                gpa->MarkBreak ();
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                if (majorBreaksBetweenVectorsRequired)
                    {
                    gpa->MarkMajorBreak ();
                    majorBreaksBetweenVectorsRequired = false;
                    }

                CurveVector const* child = curve->GetChildCurveVectorCP ();

                AppendCurveVectorToGraphicsPointArray (*child, gpa, splinesAsBezier);
                break;
                }

            default:
                {
                MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP ();
				if (bcurve != nullptr)
					{
	                if (SUCCESS != addBsplineCurveSpecialCases (gpa, bcurve))
	                    gpa->AddBsplineCurve (*bcurve, splinesAsBezier);

	                gpa->MarkBreak ();
	                break;
					}
                }			
                return ERROR;
            }
        }

    if (majorBreaksBetweenVectorsRequired)
        gpa->MarkMajorBreak ();

    return SUCCESS;
    }



GEOMDLLIMPEXP BentleyStatus GraphicsPointArray::AddCurves (CurveVectorCR curveArray, bool splinesAsBezier)
    {
    return AppendCurveVectorToGraphicsPointArray (curveArray, this, splinesAsBezier);
    }

struct GPAToCurveVectorContext
{
GPAToCurveVectorContext ()
    {
    
    }

CurveVectorPtr TranslateToMajorBreak (GraphicsPointArrayCP gpa, GraphicsPointArray::Parser &parser)
    {
    size_t majorBreakIndex;
    bool hasMajorBreaks = gpa->FindMajorBreakAfter (0, majorBreakIndex);
    CurveVectorPtr path = CurveVector::Create
                (hasMajorBreaks ? CurveVector::BOUNDARY_TYPE_Outer
                                : CurveVector::BOUNDARY_TYPE_Open);
    bvector<DPoint3d> points;
    bvector<DPoint4d> poles;
    size_t unknownTypes = 0;
    double a0, a1;
    while (parser.MoveToNextFragment ())
        {
        MSBsplineCurve bcurve;
        size_t readIndex = parser.GetReadIndex ();
        size_t breakIndex = parser.GetReadIndexTail ();
        size_t tailIndex;
        DSegment3d segment;
        DEllipse3d ellipse;
        DConic4d   conic;
        if (gpa->IsLineString (readIndex, tailIndex))
            {
            if (tailIndex == readIndex + 1
                && gpa->GetDSegment3d (readIndex, tailIndex, segment))
                {
                path->push_back (ICurvePrimitive::CreateLine (segment));
                }
            else
                {
                points.clear ();
                for (size_t i = readIndex; i <= tailIndex; i++)
                    {
                    DPoint3d xyz;
                    gpa->GetNormalizedPoint (i, xyz);
                    points.push_back (xyz);
                    }
                path->push_back (ICurvePrimitive::CreateLineString (points));
                points.clear ();
                }
            }
        else if (gpa->GetDEllipse3d (readIndex, tailIndex, ellipse))
            {
            path->push_back (ICurvePrimitive::CreateArc (ellipse));
            }
        else if (gpa->GetDConic4d (readIndex, tailIndex, conic))
            {
            unknownTypes++;
            }
        else if (gpa->GetBsplineCurve (readIndex, bcurve))
            {
            path->push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));
            bcurve.ReleaseMem ();
            }
        else if (gpa->GetBezier (readIndex, tailIndex, poles, a0, a1))
            {
            int order = (int)poles.size ();
            bcurve.InitFromDPoint4dArray (poles.data (), order, order);
            path->push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));
            bcurve.ReleaseMem ();
            }
        else
            {
            unknownTypes++;
            continue;       // hm .. skip?
            }

        if (gpa->IsMajorBreak (breakIndex))
            {
            return path;
            }
        }
    return path;
    }

CurveVectorPtr Translate (GraphicsPointArrayCP gpa)
    {
    GraphicsPointArray::Parser parser (gpa);
    CurveVectorPtr loop = TranslateToMajorBreak (gpa, parser);
    if (!parser.HasMore ())
        return loop;    // Just one loop.   It can return as is.
    // More loops coming.   Assemble all into a parity region.
    CurveVectorPtr parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*loop));
    for (;;)
        {
        loop = TranslateToMajorBreak (gpa, parser);
        if (!loop.IsValid () || loop->size () == 0)
            return parityRegion;
        parityRegion->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*loop));
        }
    }
};

CurveVectorPtr GraphicsPointArray::CreateCurveVector () const
    {
    GPAToCurveVectorContext context;
    return context.Translate (this);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
