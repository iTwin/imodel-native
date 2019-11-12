/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

#define TRIANGLE_MULTIPLIER 2

USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnTorusPipeDetailCR data) const
    {
    // Use an ellipse to calculate the stroke count and approximate facets, center and normal doesn't matter
    // as we only care for the radius and its stroke count based on the facet options
    DEllipse3d pipeSection;
    pipeSection.InitFromCenterNormalRadius(data.m_center, data.m_vectorY, data.m_minorRadius);
    size_t pipeStrokes = m_facetOptions->FullEllipseStrokeCount(pipeSection);

    // Initialize the primary circle as an ellipse (might not be a closed loop), again, the center, orientation
    // and theta (parametric angle of start point) doesn't matter as we only care for the stroke count
    // NOTE: The radius used is the sum of both radius to get the farthest point of the torus as a radius
    DEllipse3d primaryCircle;
    primaryCircle.InitFromCenterNormalRadius(data.m_center, data.m_vectorX, data.m_majorRadius + data.m_minorRadius);
    primaryCircle.SetSweep(0, data.m_sweepAngle);
    size_t primaryStrokes = m_facetOptions->EllipseStrokeCount(primaryCircle);

    // Assume that each stroke of the pipe section will be connected to each stroke of the primary circle
    // Also, if the torus does not do a full sweep, add the faces if it's capped
    double torusStrokes = m_faceMultiplier * static_cast<double>(pipeStrokes * primaryStrokes);
    double faceStrokes  = data.m_sweepAngle == msGeomConst_2pi || !data.m_capped ? 0.0 : pipeStrokes * 2.0;

    return static_cast<size_t>(torusStrokes + faceStrokes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnConeDetailCR data) const
    {
    // Use the greatest radius to approximate the facet count using a cylinder
    double radius = std::max(data.m_radiusA, data.m_radiusB);
    double minRadius = std::min(data.m_radiusA, data.m_radiusB);

    // Use an ellipse to calculate the stroke count and approximate facets, center and normal doesn't matter
    // as we only care for the radius and its stroke count based on the facet options
    DEllipse3d ellipse;
    ellipse.InitFromCenterNormalRadius(data.m_centerA, data.m_vector90, radius);
    size_t strokes = m_facetOptions->FullEllipseStrokeCount(ellipse);

    // The cylinder base and top circles have the amount of "strokes", so assume each stroke is a triangle that goes from the cylinder's center to the stroke
    // That gives 2 * strokes facets for only the base and top, now add strokes amount of facets for the quads that connect the strokes from base to top
    // If the minimum radius is 0, we don't have a top or a base face, so don't take it into account
    // If it's not capped, only take the sides
    return data.m_capped ? ((minRadius == 0.0 ? 2 : 3) * (m_faceMultiplier * strokes)) : (m_faceMultiplier * strokes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnBoxDetailCR data) const
    {
    // Get biggest box sizes
    double biggestX = std::max(data.m_baseX, data.m_topX);
    double biggestY = std::max(data.m_baseY, data.m_topY);
    // Use the sizes to get an approximate facet count for a face of the box
    size_t faceFacets = m_facetOptions->DistanceStrokeCount(biggestX) * m_facetOptions->DistanceStrokeCount(biggestY);
    // 6 faces in a box, 2 triangles each
    return (m_faceMultiplier * 6 * faceFacets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnSphereDetailCR data) const
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
    size_t perimeterFacetCount = m_facetOptions->EllipseStrokeCount(perimeter);

    // Get the "equator" of the ellipsoid/sphere and use the stroke count
    // to multiply the perimeter strokes and get an approximation of the facets
    DEllipse3d equator;
    equator.InitFromCenterNormalRadius(center, unitX, radius2);
    size_t equatorFacetCount = m_facetOptions->FullEllipseStrokeCount(equator);

    return equatorFacetCount * m_faceMultiplier * perimeterFacetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnExtrusionDetailCR data) const
    {
    // Swept profile facet count approximation
    size_t curveStrokes = GetFacetCount(*data.m_baseCurve);
    size_t extrusionStrokes = m_facetOptions->DistanceStrokeCount(data.m_extrusionVector.Magnitude());

    // To overstimate the amount of facets in each of the extrusion faces, add 2 times the strokes of the profile
    //  when the extrusion profile is concave, there is no way of telling exactly the amount of facets, so only count the strokes
    //  2 times per face to overstimate
    size_t extrusionFacets = curveStrokes * extrusionStrokes;
    return data.m_capped ? (m_faceMultiplier * extrusionFacets + 4 * curveStrokes) : m_faceMultiplier * extrusionFacets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnRotationalSweepDetailCR data) const
    {
    // Get strokes from the swept curve
    size_t curveStrokes = GetFacetCount(*data.m_baseCurve);

    // Obtain the maximum radius to try to generate an over-estimated facet count
    double radius;
    data.GetRadius(radius, DgnRotationalSweepDetail::RadiusType::Maximum);
    // Get ellipse strokes that give us an approximation of each potential segments
    DEllipse3d rotationEllipse;
    rotationEllipse.InitFromCenterNormalRadius(data.m_axisOfRotation.origin, data.m_axisOfRotation.direction, radius);
    rotationEllipse.SetSweep(0, data.m_sweepAngle);
    size_t rotationStrokes = m_facetOptions->EllipseStrokeCount(rotationEllipse);

    size_t sweepFacets = curveStrokes * rotationStrokes;

    // To overstimate the amount of facets in each of the rotational sweep end faces, add 2 times the strokes of the profile
    //  when the profile is concave, there is no way of telling exactly the amount of facets, so only count the strokes
    //  2 times per face to overstimate
    return data.m_capped ? (m_faceMultiplier * sweepFacets + 4 * curveStrokes) : m_faceMultiplier * sweepFacets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(DgnRuledSweepDetailCR data) const
    {
    size_t maxCurveFacets = 0;

    // Get the maximum number of facets
    for (CurveVectorPtr curve : data.m_sectionCurves)
        maxCurveFacets = std::max(maxCurveFacets, GetFacetCount(*curve));

    // TO-DO: We may need to multiply the facets by the distance facets between the curves instead
    return m_faceMultiplier * maxCurveFacets * data.m_sectionCurves.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount (ISolidPrimitiveCR solidPrimitive) const
    {
    switch (solidPrimitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail data;
            solidPrimitive.TryGetDgnTorusPipeDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail data;
            solidPrimitive.TryGetDgnConeDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail data;
            solidPrimitive.TryGetDgnBoxDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail data;
            solidPrimitive.TryGetDgnSphereDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail data;
            solidPrimitive.TryGetDgnExtrusionDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail data;
            solidPrimitive.TryGetDgnRotationalSweepDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail data;
            solidPrimitive.TryGetDgnRuledSweepDetail(data);
            return GetFacetCount(data);
            }
        case SolidPrimitiveType_None:
        default:
            return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount (CurveVectorCR curveVector) const
    {
    bvector<DPoint3d> strokePoints;
    curveVector.AddStrokePoints(strokePoints, *m_facetOptions);
    return strokePoints.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(ICurvePrimitiveCR primitive) const
    {
    bvector<DPoint3d> strokePoints;
    primitive.AddStrokes(strokePoints, *m_facetOptions, true, 0.0, 1.0);
    return strokePoints.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount (MSBsplineSurfaceCR surface, bool useMax) const
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

            maxU = std::max(m_facetOptions->BsplineCurveStrokeCount(*curveU), maxU);
            maxV = std::max(m_facetOptions->BsplineCurveStrokeCount(*curveV), maxV);
            }

        facetCount = static_cast<size_t>(m_faceMultiplier) * maxU * maxV;
        }
    else
        {
        size_t sumU = 0;
        size_t sumV = 0;

        for (double i = 0.0; i <= 1.0; i+=s_stepSize)
            {
            MSBsplineCurvePtr curveU = surface.GetIsoUCurve(i);
            MSBsplineCurvePtr curveV = surface.GetIsoVCurve(i);

            sumU += m_facetOptions->BsplineCurveStrokeCount(*curveU);
            sumV += m_facetOptions->BsplineCurveStrokeCount(*curveV);
            }

        double avgU = ((double)sumU / s_numSteps);
        double avgV = ((double)sumV / s_numSteps);

        facetCount = static_cast<size_t>(m_faceMultiplier * avgV * avgU);
        }

    return facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(PolyfaceQueryCR geom) const
    {
    return geom.GetNumFacet();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount (IGeometryCR geometry) const
    {
    CurveVectorPtr          curveVector;
    ISolidPrimitivePtr      solidPrimitive;
    MSBsplineSurfacePtr     bsplineSurface;
    PolyfaceHeaderPtr       polyface;
    ICurvePrimitivePtr      curvePrimitive;

    if ((curveVector = geometry.GetAsCurveVector()).IsValid())
        return GetFacetCount (*curveVector);

    if ((solidPrimitive = geometry.GetAsISolidPrimitive()).IsValid())
        return GetFacetCount (*solidPrimitive);

    if ((bsplineSurface = geometry.GetAsMSBsplineSurface ()).IsValid())
        return GetFacetCount (*bsplineSurface);

    if ((polyface = geometry.GetAsPolyfaceHeader()).IsValid())
        return GetFacetCount(*polyface);

    if ((curvePrimitive = geometry.GetAsICurvePrimitive()).IsValid())
        return GetFacetCount(*curvePrimitive);

    BeAssert (false);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(TextStringCR geom) const
    {
    return 0; // ###TODO_FACET_COUNT: TextStrings...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(GeometricPrimitiveCR geom) const
    {
    switch (geom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:      return GetFacetCount(*geom.GetAsICurvePrimitive());
        case GeometricPrimitive::GeometryType::CurveVector:         return GetFacetCount(*geom.GetAsCurveVector());
        case GeometricPrimitive::GeometryType::SolidPrimitive:      return GetFacetCount(*geom.GetAsISolidPrimitive());
        case GeometricPrimitive::GeometryType::BsplineSurface:      return GetFacetCount(*geom.GetAsMSBsplineSurface());
        case GeometricPrimitive::GeometryType::Polyface:            return GetFacetCount(*geom.GetAsPolyfaceHeader());
        case GeometricPrimitive::GeometryType::BRepEntity:   return GetFacetCount(*geom.GetAsIBRepEntity());
        case GeometricPrimitive::GeometryType::TextString:          return GetFacetCount(*geom.GetAsTextString());
        default:                                                    BeAssert(false); return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr FacetCounter::CreateDefaultFacetOptions()
    {
    auto options = IFacetOptions::Create();

    static const double s_chordTolerance = 0.01;
    options->SetChordTolerance(s_chordTolerance);
    options->SetMaxPerFace(3);
    options->SetCurvedSurfaceMaxPerFace(3);
    options->SetNormalsRequired(false);
    options->SetParamsRequired(false);

    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FacetCounter::GetFacetCount(IBRepEntityCR entity) const
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag(entity);

    if (0 == entityTag)
        return 0;

    int         nFaces = 0;
    PK_FACE_t*  faceTags = nullptr;

    if (SUCCESS != PK_BODY_ask_faces(entityTag, &nFaces, &faceTags))
        return 0;

    size_t facetCount = 0;

    for (int iFace = 0; iFace < nFaces; iFace++)
        {
        PK_ENTITY_t faceTag = faceTags[iFace];

        PK_SURF_t       surface;
        PK_CLASS_t      surfaceClass;
        PK_LOGICAL_t    orientation;

        PK_FACE_ask_oriented_surf (faceTag, &surface, &orientation);
        PK_ENTITY_ask_class (surface, &surfaceClass);

        switch (surfaceClass)
            {
            case PK_CLASS_plane:
            case PK_CLASS_circle:
            case PK_CLASS_ellipse:
                {
                CurveVectorPtr  curveVector;

                if ((curveVector = PSolidGeom::PlanarFaceToCurveVector (faceTag)).IsValid())
                    facetCount += GetFacetCount(*curveVector);
                break;
                }

            case PK_CLASS_cyl:
            case PK_CLASS_cone:
            case PK_CLASS_torus:
            case PK_CLASS_sphere:
            case PK_CLASS_swept:
                {
                ISolidPrimitivePtr  solidPrimitive;

                if ((solidPrimitive = PSolidGeom::FaceToSolidPrimitive (faceTag, nullptr)).IsValid())
                    facetCount += GetFacetCount(*solidPrimitive);
                break;
                }

            default:
                {
                MSBsplineSurfacePtr bSplineSurface = MSBsplineSurface::CreatePtr() ;

                if (SUCCESS == PSolidGeom::CreateMSBsplineSurfaceFromSurface (*bSplineSurface, surface, nullptr, nullptr, nullptr, 0, 0, 1.0E-6, false))
                    facetCount += GetFacetCount(*bSplineSurface);

                break;
                }
            }
        }

    PK_MEMORY_free(faceTags);

    return facetCount;
#else
    return 0;
#endif
    }

