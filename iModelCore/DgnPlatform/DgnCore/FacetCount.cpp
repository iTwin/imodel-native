/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/FacetCount.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

#define TRIANGLE_MULTIPLIER 2

USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getTorusFacetCount(DgnTorusPipeDetailCR data, IFacetOptionsR facetOptions)
    {
    // Use an ellipse to calculate the stroke count and approximate facets, center and normal doesn't matter
    // as we only care for the radius and its stroke count based on the facet options
    DEllipse3d pipeSection;
    pipeSection.InitFromCenterNormalRadius(data.m_center, data.m_vectorY, data.m_minorRadius);
    size_t pipeStrokes = facetOptions.FullEllipseStrokeCount(pipeSection);

    // Initialize the primary circle as an ellipse (might not be a closed loop), again, the center, orientation
    // and theta (parametric angle of start point) doesn't matter as we only care for the stroke count
    // IMPORTANT NOTE: DEllipse3d::GetStrokeCount doesn't seem to take into account the sweep angle, assumes a full ellipse
    DEllipse3d primaryCircle;
    primaryCircle.InitFromCenterNormalRadius(data.m_center, data.m_vectorX, data.m_majorRadius);
    primaryCircle.SetSweep(0, data.m_sweepAngle);
    size_t primaryStrokes = facetOptions.EllipseStrokeCount(primaryCircle);

    // Assume that each stroke of the pipe section will be connected to each stroke of the primary circle
    return pipeStrokes * primaryStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getConeFacetCount(DgnConeDetailCR data, IFacetOptionsR facetOptions)
    {
    // Use the greatest radius to approximate the facet count using a cylinder
    double radius = std::max(data.m_radiusA, data.m_radiusB);
    double minRadius = std::min(data.m_radiusA, data.m_radiusB);

    // Use an ellipse to calculate the stroke count and approximate facets, center and normal doesn't matter
    // as we only care for the radius and its stroke count based on the facet options
    DEllipse3d ellipse;
    ellipse.InitFromCenterNormalRadius(data.m_centerA, data.m_vector90, radius);
    size_t strokes = facetOptions.FullEllipseStrokeCount(ellipse);

    // The cylinder base and top circles have the amount of "strokes", so assume each stroke is a triangle that goes from the cylinder's center to the stroke
    // That gives 2 * strokes facets for only the base and top, now add strokes amount of facets for the quads that connect the strokes from base to top
    // If the minimum radius is 0, we don't have a top or a base face, so don't take it into account
    // If it's not capped, only take the sides
    return data.m_capped ? ((minRadius == 0.0 ? 2 : 3) * strokes) : strokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getBoxFacetCount(DgnBoxDetailCR data, IFacetOptionsR facetOptions)
    {
    // Get biggest box sizes
    double biggestX = std::max(data.m_baseX, data.m_topX);
    double biggestY = std::max(data.m_baseY, data.m_topY);
    // Use the sizes to get an approximate facet count for a face of the box
    size_t faceFacets = facetOptions.DistanceStrokeCount(biggestX) * facetOptions.DistanceStrokeCount(biggestY);
    // 6 faces in a box, 2 triangles each
    return (TRIANGLE_MULTIPLIER * 6 * faceFacets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getSphereFacetCount(DgnSphereDetailCR data, IFacetOptionsR facetOptions)
    {
    DPoint3d center;
    DVec3d unitX, unitY, unitZ;
    double radius1, radius2;

    data.IsTrueRotationAroundZ(center, unitX, unitY, unitZ, radius1, radius2);

    // Get half the ellipse defining the radius of the sphere
    DEllipse3d perimeter;
    perimeter.InitFromCenterNormalRadius(center, unitY, radius1);
    perimeter.SetSweep(0, msGeomConst_pi);
    // Get count of facets in the perimeter
    size_t perimeterFacetCount = facetOptions.EllipseStrokeCount(perimeter);

    // Get the "equator" of the ellipsoid/sphere and use the stroke count
    // to multiply the perimeter strokes and get an approximation of the facets
    DEllipse3d equator;
    equator.InitFromCenterNormalRadius(center, unitX, radius2);
    size_t equatorFacetCount = facetOptions.FullEllipseStrokeCount(equator);

    return equatorFacetCount * perimeterFacetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getExtrusionFacetCount(DgnExtrusionDetailCR data, IFacetOptionsR facetOptions)
    {
    // Swept profile facet count approximation
    size_t curveStrokes = FacetCountUtil::GetFacetCount(*data.m_baseCurve, facetOptions);
    size_t extrusionStrokes = facetOptions.DistanceStrokeCount(data.m_extrusionVector.Magnitude());

    // TO-DO: Right now, I'm using the curve strokes of the profile as a count of what should actually be the face's facet count
    size_t extrusionFacets = curveStrokes * extrusionStrokes;
    // Multiplying by the triangle multiplier (2) based on parasolid's output
    return data.m_capped ? (TRIANGLE_MULTIPLIER * extrusionFacets + 2 * curveStrokes)  : TRIANGLE_MULTIPLIER * extrusionFacets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getRotationalSweepFacetCount(DgnRotationalSweepDetailCR data, IFacetOptionsR facetOptions)
    {
    // Get strokes from the swept curve
    size_t curveStrokes = FacetCountUtil::GetFacetCount(*data.m_baseCurve, facetOptions);

    // Obtain the maximum radius to try to generate an over-estimated facet count
    double radius;
    data.GetRadius(radius, DgnRotationalSweepDetail::RadiusType::Maximum);
    // Get ellipse strokes that give us an approximation of each potential segments
    DEllipse3d rotationEllipse;
    rotationEllipse.InitFromCenterNormalRadius(data.m_axisOfRotation.origin, data.m_axisOfRotation.direction, radius);
    rotationEllipse.SetSweep(0, data.m_sweepAngle);
    size_t rotationStrokes = facetOptions.EllipseStrokeCount(rotationEllipse);

    size_t sweepFacets = curveStrokes * rotationStrokes;

    // TO-DO: Right now, I'm using the curve strokes of the profile as a count of what should actually be the face's facet count
    return data.m_capped ? (sweepFacets + 2 * curveStrokes) : sweepFacets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t getRuledSweepFacetCount(DgnRuledSweepDetailCR data, IFacetOptionsR facetOptions)
    {
    size_t maxCurveFacets = 0;

    // Get the maximum number of facets
    for (CurveVectorPtr curve : data.m_sectionCurves)
        maxCurveFacets = std::max(maxCurveFacets, FacetCountUtil::GetFacetCount(*curve, facetOptions));

    // TO-DO: We may need to multiply the facets by the distance facets between the curves instead
    return maxCurveFacets * data.m_sectionCurves.size();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCountUtil::GetFacetCount (ISolidPrimitiveCR solidPrimitive, IFacetOptionsR facetOptions)
    {
    switch (solidPrimitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail data;
            solidPrimitive.TryGetDgnTorusPipeDetail(data);
            return getTorusFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail data;
            solidPrimitive.TryGetDgnConeDetail(data);
            return getConeFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail data;
            solidPrimitive.TryGetDgnBoxDetail(data);
            return getBoxFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail data;
            solidPrimitive.TryGetDgnSphereDetail(data);
            return getSphereFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail data;
            solidPrimitive.TryGetDgnExtrusionDetail(data);
            return getExtrusionFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail data;
            solidPrimitive.TryGetDgnRotationalSweepDetail(data);
            return getRotationalSweepFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail data;
            solidPrimitive.TryGetDgnRuledSweepDetail(data);
            return getRuledSweepFacetCount(data, facetOptions);
            }
        case SolidPrimitiveType_None:
        default:
            return 0;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCountUtil::GetFacetCount (CurveVectorCR curveVector, IFacetOptionsR facetOptions)
    {
    bvector<DPoint3d> strokePoints;
    curveVector.AddStrokePoints(strokePoints, facetOptions);
    return strokePoints.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCountUtil::GetFacetCount (MSBsplineSurfaceCR surface, IFacetOptionsR facetOptions, bool useMax)
    {
    static int      s_numSteps = 3; // Number of Iso Curves used to obtain an approximate
    static double   s_stepSize = 1.0 / ((double)s_numSteps-1);

    size_t facetCount = 0;

    if (useMax)
        {
        size_t maxU = 0;
        size_t maxV = 0;

        for (double i = 0.0; i <= 1.0; i+=s_stepSize)
            {
            MSBsplineCurvePtr curveU = surface.GetIsoUCurve(i);
            MSBsplineCurvePtr curveV = surface.GetIsoVCurve(i);

            maxU = std::max(facetOptions.BsplineCurveStrokeCount(*curveU), maxU);
            maxV = std::max(facetOptions.BsplineCurveStrokeCount(*curveV), maxV);
            }

        facetCount = maxU * maxV;
        }
    else
        {
        size_t sumU = 0;
        size_t sumV = 0;

        for (double i = 0.0; i <= 1.0; i+=s_stepSize)
            {
            MSBsplineCurvePtr curveU = surface.GetIsoUCurve(i);
            MSBsplineCurvePtr curveV = surface.GetIsoVCurve(i);

            sumU += facetOptions.BsplineCurveStrokeCount(*curveU);
            sumV += facetOptions.BsplineCurveStrokeCount(*curveV);
            }

        double avgU = ((double)sumU / s_numSteps);
        double avgV = ((double)sumV / s_numSteps);

        facetCount = (size_t)(avgV * avgU);
        }

    return facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCountUtil::GetFacetCount (IGeometryCR geometry, IFacetOptionsR facetOptions)
    {
    CurveVectorPtr          curveVector;
    ISolidPrimitivePtr      solidPrimitive;
    MSBsplineSurfacePtr     bsplineSurface;
    PolyfaceHeaderPtr       polyface;
    
    if ((curveVector = geometry.GetAsCurveVector()).IsValid())
        return GetFacetCount (*curveVector, facetOptions);

    if ((solidPrimitive = geometry.GetAsISolidPrimitive()).IsValid())
        return GetFacetCount (*solidPrimitive, facetOptions);

    if ((bsplineSurface = geometry.GetAsMSBsplineSurface ()).IsValid())
        return GetFacetCount (*bsplineSurface, facetOptions);

    if ((polyface = geometry.GetAsPolyfaceHeader()).IsValid())
        return polyface->GetNumFacet();

    BeAssert (false);
    return 0;
    }

