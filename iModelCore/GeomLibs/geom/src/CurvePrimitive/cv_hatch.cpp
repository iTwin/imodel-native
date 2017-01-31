/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cv_hatch.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//! Return a curve vector (of type BOUNDARY_TYPE_None) containing hatch sticks.
GEOMDLLIMPEXP CurveVectorPtr CurveVector::CreateXYHatch (
CurveVectorCR        boundary,      //!< [in] boundary curves.
DPoint3dCR           startPoint,    //!< [in] Start point for hatch lines
double               angleRadians,  //!< [in] angle from X axis.
double               spacing,       //!< [in] spacing perpendicular to hatch direction
int                  selectRule     //!< 0 for parity rules, 1 for longest possible strokes (first to last crossings), 2 for leftmsot and rightmost of parity set.
)
    {
    GraphicsPointArray boundaryGPA, collectorGPA;
    Transform localToWorld;
    double c = cos (angleRadians);
    double s = sin (angleRadians);
    localToWorld.InitFromRowValues
        (
        c * spacing, 0, -s * spacing, startPoint.x,
        s * spacing, 0,  c * spacing, startPoint.y,
        0,      spacing, 0.0,         startPoint.z
        );

    boundaryGPA.AddCurves (boundary, true);
    jmdlGraphicsPointArray_addTransformedCrossHatchExt (&collectorGPA, &boundaryGPA, &localToWorld, selectRule);
    CurveVectorPtr sticks = CurveVector::Create (BOUNDARY_TYPE_None);
    // Sticks are alternating 
    for (size_t i = 1; i < collectorGPA.GetGraphicsPointCount (); i+=2)
        {
        DSegment3d segment;
        if (   collectorGPA.GetNormalizedPoint (i-1, segment.point[0])
            && collectorGPA.GetNormalizedPoint (i, segment.point[1])
            )
            sticks->push_back (ICurvePrimitive::CreateLine (segment));
        }
    return sticks;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
