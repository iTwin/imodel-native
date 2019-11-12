/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

static const double TOLERANCE_CircleAxisRatio = 1.0E-8;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/11
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr PSolidGeom::GetAsCurvePrimitive (PK_CURVE_t curve, PK_INTERVAL_t interval, bool reverseDirection)
    {
    PK_CLASS_t  curveClass;

    PK_ENTITY_ask_class (curve, &curveClass);

    switch (curveClass)
        {
        case PK_CLASS_line:
            {
            PK_LINE_sf_t    sfLine;

            if (SUCCESS != PK_LINE_ask (curve, &sfLine))
                return NULL;

            DSegment3d  segment;

            segment.point[0].SumOf (*((DPoint3dCP) sfLine.basis_set.location.coord),*((DVec3dCP) sfLine.basis_set.axis.coord), interval.value[reverseDirection ? 1 : 0]);
            segment.point[1].SumOf (*((DPoint3dCP) sfLine.basis_set.location.coord),*((DVec3dCP) sfLine.basis_set.axis.coord), interval.value[reverseDirection ? 0 : 1]);

            return ICurvePrimitive::CreateLine (segment);
            }

        case PK_CLASS_circle:
            {
            PK_CIRCLE_sf_t  sfCircle;

            if (SUCCESS != PK_CIRCLE_ask (curve, &sfCircle))
                return NULL;

            double      start, sweep;
            DVec3d      xVector, yVector;

            xVector.Scale (*((DVec3dP) &sfCircle.basis_set.ref_direction.coord), sfCircle.radius);
            yVector.CrossProduct (*((DVec3dP) sfCircle.basis_set.axis.coord), *((DVec3dP) &xVector));

            PSolidUtil::ExtractStartAndSweepFromInterval (start, sweep, interval, reverseDirection);

            DEllipse3d  ellipse;

            ellipse.InitFromVectors (*(DPoint3dP) sfCircle.basis_set.location.coord, xVector, yVector, start, sweep);

            return ICurvePrimitive::CreateArc (ellipse);
            }

        case PK_CLASS_ellipse:
            {
            PK_ELLIPSE_sf_t  sfEllipse;

            if (SUCCESS != PK_ELLIPSE_ask (curve, &sfEllipse))
                return NULL;

            double      start, sweep;
            DVec3d      xVector, yVector;

            xVector.Init (sfEllipse.basis_set.ref_direction.coord[0], sfEllipse.basis_set.ref_direction.coord[1], sfEllipse.basis_set.ref_direction.coord[2]);
            yVector.CrossProduct (*((DVec3dP) sfEllipse.basis_set.axis.coord), xVector);
            xVector.Scale (sfEllipse.R1);
            yVector.Scale (sfEllipse.R2);

            PSolidUtil::ExtractStartAndSweepFromInterval (start, sweep, interval, reverseDirection);

            DEllipse3d  ellipse;

            ellipse.InitFromVectors (*(DPoint3dP) sfEllipse.basis_set.location.coord, xVector, yVector, start, sweep);

            return ICurvePrimitive::CreateArc (ellipse);
            }

        default:
            {
            MSBsplineCurve  bCurve;

            if (SUCCESS != PSolidGeom::CreateMSBsplineCurveFromCurve (bCurve, curve, interval, reverseDirection))
                return NULL;

            ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateBsplineCurve (bCurve);

            bCurve.ReleaseMem ();

            return primitive;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::EdgeToCurvePrimitive (ICurvePrimitivePtr& curvePrimitive, PK_EDGE_t edgeTag)
    {
    PK_CURVE_t      curveTag = PK_ENTITY_null;
    PK_FIN_t        finTag = PK_ENTITY_null;
    PK_LOGICAL_t    orientation = PK_LOGICAL_true;
    PK_INTERVAL_t   interval;

    if (SUCCESS == PK_EDGE_ask_oriented_curve(edgeTag, &curveTag, &orientation) && PK_ENTITY_null != curveTag)
        {
        if (SUCCESS != PK_EDGE_find_interval(edgeTag, &interval))
            PK_CURVE_ask_interval(curveTag, &interval);
        }
    else if (SUCCESS == PK_EDGE_ask_first_fin(edgeTag, &finTag) && SUCCESS == PK_FIN_ask_oriented_curve(finTag, &curveTag, &orientation) && PK_ENTITY_null != curveTag)
        {
        if (SUCCESS != PK_FIN_find_interval(finTag, &interval))
            PK_CURVE_ask_interval(curveTag, &interval);
        }
    else
        {
        return ERROR;
        }

    curvePrimitive = PSolidGeom::GetAsCurvePrimitive (curveTag, interval, !orientation);

    return curvePrimitive.IsValid() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr planarFaceLoopToCurveVector (PK_LOOP_t loopTag, EdgeToCurveIdMap const* idMap)
    {
    CurveVectorPtr  childLoop;
    int             nFins = 0;
    PK_FIN_t*       fins = NULL;
    PK_LOOP_type_t  loopType;

    PK_LOOP_ask_type (loopTag, &loopType);

    bool    isHoleLoop = (PK_LOOP_type_inner_c == loopType || PK_LOOP_type_inner_sing_c == loopType || PK_LOOP_type_likely_inner_c == loopType);

    if (SUCCESS != PK_LOOP_ask_fins (loopTag, &nFins, &fins) || 0 == nFins)
        return childLoop;

    childLoop = CurveVector::Create (isHoleLoop ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer);

    for (int iFin = 0; iFin < nFins; iFin++)
        {
        PK_EDGE_t   edgeTag = PK_ENTITY_null;

        if (SUCCESS != PK_FIN_ask_edge (fins[iFin], &edgeTag))
            continue;

        PK_CURVE_t          curveTag = PK_ENTITY_null;
        PK_LOGICAL_t        isPositiveFin;
        PK_INTERVAL_t       interval;
        ICurvePrimitivePtr  primitive;
        PK_LOGICAL_t        orientation;

        if (SUCCESS == PK_FIN_ask_oriented_curve (fins[iFin], &curveTag, &orientation) &&
            SUCCESS == PK_FIN_find_interval (fins[iFin], &interval))
            primitive = PSolidGeom::GetAsCurvePrimitive (curveTag, interval, !orientation);
        else if (SUCCESS == PK_FIN_is_positive (fins[iFin], &isPositiveFin) &&
                 SUCCESS == PK_EDGE_ask_oriented_curve (edgeTag, &curveTag, &orientation) &&
                 SUCCESS == PK_EDGE_find_interval (edgeTag, &interval))
            primitive = PSolidGeom::GetAsCurvePrimitive (curveTag, interval, orientation != isPositiveFin);

        if (!primitive.IsValid ())
            continue;

        if (NULL != idMap)
            {
            FaceId  faceId;

            if (SUCCESS == PSolidTopoId::IdFromEntity (faceId, edgeTag, true))
                {
                EdgeToCurveIdMap::const_iterator found = idMap->find (faceId.nodeId);

                if (found != idMap->end())
                    primitive->SetId (CurvePrimitiveId::Create (*found->second).get());
                }
            }

        childLoop->push_back (primitive);
        }

    PK_MEMORY_free (fins);

    return childLoop;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPlanarSurfaceClass(PK_CLASS_t surfaceClass)
    {
    switch (surfaceClass)
        {
        case PK_CLASS_plane:
        case PK_CLASS_circle:
        case PK_CLASS_ellipse:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr PSolidGeom::PlanarFaceToCurveVector(PK_FACE_t faceTag, EdgeToCurveIdMap const* idMap)
    {
    PK_SURF_t       surfaceTag;
    PK_CLASS_t      surfaceClass;
    int             nLoops = 0;
    PK_LOOP_t*      loops = NULL;
    PK_LOGICAL_t    orientation;

    if (SUCCESS != PK_FACE_ask_oriented_surf(faceTag, &surfaceTag, &orientation) ||
        SUCCESS != PK_ENTITY_ask_class(surfaceTag, &surfaceClass) || !isPlanarSurfaceClass(surfaceClass) ||
        SUCCESS != PK_FACE_ask_loops(faceTag, &nLoops, &loops) || nLoops < 1)
        return nullptr;

    bvector<CurveVectorPtr> curveLoops;

    for (int iLoop = 0; iLoop < nLoops; iLoop++)
        {
        CurveVectorPtr  childLoop = planarFaceLoopToCurveVector (loops[iLoop], idMap);

        if (!childLoop.IsValid() || childLoop->empty())
            continue;

        // NOTE: I don't believe there is any real requirement for outer loop to be first in partity region...
        if (CurveVector::BOUNDARY_TYPE_Outer == childLoop->GetBoundaryType ())
            curveLoops.insert (curveLoops.begin (), childLoop);
        else
            curveLoops.push_back (childLoop);
        }

    PK_MEMORY_free (loops);

    switch (curveLoops.size ())
        {
        case 0:
            return NULL;

        case 1:
            return curveLoops.front ();

        default:
            {
            CurveVectorPtr  curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);

            for (CurveVectorPtr childLoop: curveLoops)
                curveVector->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*childLoop));

            return curveVector;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  PSolidGeom::PlanarSheetBodyToCurveVector (IBRepEntityCR entity)
    {
    PK_ENTITY_t     entityTag = PSolidUtil::GetEntityTag (entity);
    PK_BODY_type_t  bodyType;

    PK_BODY_ask_type (entityTag, &bodyType);

    if (PK_BODY_type_sheet_c != bodyType)
        return NULL;

    int         nFaces = 0;

    if (SUCCESS != PK_BODY_ask_faces (entityTag, &nFaces, NULL) || 1 != nFaces)
        return NULL;

    PK_FACE_t   faceTag = PK_ENTITY_null;

    if (SUCCESS != PK_BODY_ask_first_face (entityTag, &faceTag) || PK_ENTITY_null == faceTag)
        return NULL;

    CurveVectorPtr  curves = PSolidGeom::PlanarFaceToCurveVector (faceTag);

    if (!curves.IsValid ())
        return NULL;

    curves->TransformInPlace (entity.GetEntityTransform ());

    return curves;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void flipBoundaries (CurveVectorPtr* uvBoundaries, bool flipU, bool flipV)
    {
    if (NULL == uvBoundaries || !uvBoundaries->IsValid() || (!flipU && !flipV))
        return;

    Transform transform = Transform::FromIdentity();

    if (flipU)
        {
        transform.ScaleMatrixColumns (transform, -1.0, 1.0, 1.0);
        transform.TranslateInLocalCoordinates (transform, -1.0, 0.0, 0.0);
        }

    if (flipV)
        {
        transform.ScaleMatrixColumns (transform, 1.0, -1.0, 1.0);
        transform.TranslateInLocalCoordinates (transform, 0.0, -1.0, 0.0);
        }

    (*uvBoundaries)->TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void swapBoundariesXY (CurveVectorPtr* uvBoundaries)
    {
    if (NULL == uvBoundaries || !uvBoundaries->IsValid())
        return;

    Transform swapXY = Transform::From (RotMatrix::From2Vectors (DVec3d::From (0.0, 1.0, 0.0), DVec3d::From (1.0, 0.0, 0.0)));

    (*uvBoundaries)->TransformInPlace (swapXY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr solidPrimitiveFromConePararameters
(
PK_AXIS2_sf_t&  basis_set,
PK_UVBOX_t&     uvBox,
double          radius,
double          semiAngle,
bool            reversed,
CurveVectorPtr* uvBoundaries
)
    {
    double      topRadius, baseRadius, sweepAngle = uvBox.param[2] - uvBox.param[0];

    topRadius = radius + uvBox.param[1] * tan (semiAngle);
    baseRadius = radius + uvBox.param[3] * tan (semiAngle);

    DVec3d      xVector, yVector, zVector;
    RotMatrix   rMatrix;

    xVector = *((DVec3dP) basis_set.ref_direction.coord);
    zVector = *((DVec3dP) basis_set.axis.coord);

    yVector.CrossProduct (zVector, xVector);
    rMatrix.InitFromColumnVectors (xVector, yVector, zVector);

    DPoint3d    basePoint, topPoint;

    topPoint.SumOf (*((DPoint3dP) basis_set.location.coord), *((DVec3dP) basis_set.axis.coord), uvBox.param[1]);
    basePoint.SumOf (*((DPoint3dP) basis_set.location.coord), *((DVec3dP) basis_set.axis.coord), uvBox.param[3]);

    if (fabs (sweepAngle) >= msGeomConst_2pi)
        {
        if (reversed)
            {
            double      tmpRadius = topRadius;
            DPoint3d    tmpPoint = topPoint;

            topRadius = baseRadius;
            topPoint = basePoint;

            basePoint = tmpPoint;
            baseRadius = tmpRadius;
            }
        else
            {
            // The top and base are reversed above - so need to flip V parameters (unless reverse already flipped).
            flipBoundaries (uvBoundaries, false, true);
            }


        DgnConeDetail detail (basePoint, topPoint, rMatrix, baseRadius, topRadius, false);

        return ISolidPrimitive::CreateDgnCone (detail);
        }

    double      startAngle;

    if (reversed)
        {
        startAngle = uvBox.param[0];
        }
    else
        {
        startAngle = uvBox.param[2];
        sweepAngle = -sweepAngle;
        }

    flipBoundaries (uvBoundaries, !reversed, true);      // Always flip V to account for topPoint mismatch retained from SS3.  U flipped only if not reversed.  Ick.

    DVec3d      m0, m1;

    rMatrix.GetColumn (m0, 0);
    rMatrix.GetColumn (m1, 1);

    DEllipse3d  baseEllipse, topEllipse;

    baseEllipse.InitFromDGNFields3d (basePoint, m0, m1, baseRadius, baseRadius, startAngle, sweepAngle);
    topEllipse.InitFromDGNFields3d (topPoint, m0, m1, topRadius, topRadius, startAngle, sweepAngle);

    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr  topCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    baseCurve->push_back (ICurvePrimitive::CreateArc (baseEllipse));
    topCurve->push_back (ICurvePrimitive::CreateArc (topEllipse));

    DgnRuledSweepDetail detail (baseCurve, topCurve, false);

    return ISolidPrimitive::CreateDgnRuledSweep (detail);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr solidPrimitiveFromConeFace (PK_FACE_t face, PK_UVBOX_t uvBox, CurveVectorPtr* uvBoundaries)
    {
    PK_SURF_t       surface;
    PK_CLASS_t      surfaceClass;
    PK_LOGICAL_t    orientation;

    PK_FACE_ask_oriented_surf (face, &surface, &orientation);
    PK_ENTITY_ask_class (surface, &surfaceClass);

    if (PK_CLASS_cyl == surfaceClass)
        {
        PK_CYL_sf_t sfCylinder;

        if (SUCCESS != PK_CYL_ask (surface, &sfCylinder))
            return NULL;

        return solidPrimitiveFromConePararameters (sfCylinder.basis_set, uvBox, sfCylinder.radius, 0.0, PK_LOGICAL_false == orientation, uvBoundaries);
        }

    PK_CONE_sf_t sfCone;

    if (SUCCESS != PK_CONE_ask (surface, &sfCone))
        return NULL;

    return solidPrimitiveFromConePararameters (sfCone.basis_set, uvBox, sfCone.radius, sfCone.semi_angle, PK_LOGICAL_false == orientation, uvBoundaries);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr solidPrimitiveFromTorusFace (PK_FACE_t face, PK_UVBOX_t uvBox, CurveVectorPtr* uvBoundaries)
    {
    PK_SURF_t       surface;
    PK_LOGICAL_t    orientation;

    PK_FACE_ask_oriented_surf (face, &surface, &orientation);

    PK_TORUS_sf_t   sfTorus;

    if (SUCCESS != PK_TORUS_ask (surface, &sfTorus))
        return NULL;

    DVec3d      xVector, yVector, zVector;
    RotMatrix   rMatrix, rotateRMatrix;

    xVector = *((DVec3dP) sfTorus.basis_set.ref_direction.coord);
    zVector = *((DVec3dP) sfTorus.basis_set.axis.coord);

    yVector.CrossProduct (zVector, xVector);
    rMatrix.InitFromColumnVectors (xVector, yVector, zVector);

    rotateRMatrix.InitFromAxisAndRotationAngle (2, uvBox.param[0]);
    rMatrix.InitProduct (rMatrix, rotateRMatrix);

    DVec3d      m0, m1, m2;

    rMatrix.GetColumn (m0, 0);
    rMatrix.GetColumn (m1, 1);
    rMatrix.GetColumn (m2, 2);

    double      sweepAngle = uvBox.param[2] - uvBox.param[0];
    double      arcStart = uvBox.param[1], arcEnd = uvBox.param[3], arcSweep;

    swapBoundariesXY (uvBoundaries);

    if (PK_LOGICAL_false == orientation)
        {
        arcSweep = arcEnd - arcStart;
        }
    else
        {
        arcSweep = arcStart - arcEnd;
        arcStart = arcEnd;

        flipBoundaries (uvBoundaries, true, false);
        }

    if (arcSweep >= msGeomConst_2pi && (NULL == uvBoundaries || !uvBoundaries->IsValid()))
        {
        DgnTorusPipeDetail  detail (*((DPoint3dP) (sfTorus.basis_set.location.coord)), m0, m1, sfTorus.major_radius, sfTorus.minor_radius, sweepAngle, false);
        return ISolidPrimitive::CreateDgnTorusPipe (detail);
        }

    DPoint3d center;

    center.SumOf (*((DPoint3dP) sfTorus.basis_set.location.coord), m0, sfTorus.major_radius);

    DEllipse3d      ellipse;
    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    ellipse.InitFromDGNFields3d (center, m0, m2, sfTorus.minor_radius, sfTorus.minor_radius, arcStart, arcSweep);
    baseCurve->push_back (ICurvePrimitive::CreateArc (ellipse));

    DgnRotationalSweepDetail  detail (baseCurve, *((DPoint3dP) sfTorus.basis_set.location.coord), m2, sweepAngle, false);

    return ISolidPrimitive::CreateDgnRotationalSweep (detail);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr solidPrimitiveFromSphereFace (PK_FACE_t face, PK_UVBOX_t uvBox, CurveVectorPtr* uvBoundaries)
    {
    PK_SURF_t       surface;
    PK_LOGICAL_t    orientation;

    PK_FACE_ask_oriented_surf (face, &surface, &orientation);

    PK_SPHERE_sf_t  sfSphere;

    if (SUCCESS != PK_SPHERE_ask (surface, &sfSphere))
        return NULL;

    DVec3d      xVector, yVector, zVector;
    RotMatrix   rMatrix, rotateRMatrix;

    xVector = *((DVec3dP) sfSphere.basis_set.ref_direction.coord);
    zVector = *((DVec3dP) sfSphere.basis_set.axis.coord);

    yVector.CrossProduct (zVector, xVector);
    rMatrix.InitFromColumnVectors (xVector, yVector, zVector);

    rotateRMatrix.InitFromAxisAndRotationAngle (2, uvBox.param[0]);
    rMatrix.InitProduct (rMatrix, rotateRMatrix);

    DVec3d      m0, m1, m2;

    rMatrix.GetColumn (m0, 0);
    rMatrix.GetColumn (m1, 1);
    rMatrix.GetColumn (m2, 2);

    double      sweepAngle = uvBox.param[2] - uvBox.param[0];
    double      arcStart = uvBox.param[1], arcEnd = uvBox.param[3], arcSweep;

    swapBoundariesXY (uvBoundaries);

    if (PK_LOGICAL_false == orientation)
        {
        arcSweep = arcEnd - arcStart;
        }
    else
        {
        arcSweep = arcStart - arcEnd;
        arcStart = arcEnd;

        flipBoundaries (uvBoundaries, true, false);
        }

    DEllipse3d      ellipse;
    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    ellipse.InitFromDGNFields3d (*((DPoint3dP) sfSphere.basis_set.location.coord), m0, m2, sfSphere.radius, sfSphere.radius, arcStart, arcSweep);
    baseCurve->push_back (ICurvePrimitive::CreateArc (ellipse));

    DgnRotationalSweepDetail  detail (baseCurve, *((DPoint3dP) sfSphere.basis_set.location.coord), m2, sweepAngle, false);

    return ISolidPrimitive::CreateDgnRotationalSweep (detail);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr solidPrimitiveFromSweptFace (PK_FACE_t face, PK_UVBOX_t uvBox, CurveVectorPtr* uvBoundaries)
    {
    PK_SURF_t       surface;
    PK_LOGICAL_t    orientation;

    PK_FACE_ask_oriented_surf (face, &surface, &orientation);

    PK_SWEPT_sf_t   sfSwept;

    if (SUCCESS != PK_SWEPT_ask (surface, &sfSwept))
        return NULL;

    double  startSweepParam = uvBox.param[1];
    double  endSweepParam   = uvBox.param[3];
    DVec3d  extrudeDir = *((DVec3dP) sfSwept.direction.coord);

    extrudeDir.ScaleToLength (endSweepParam - startSweepParam);

    PK_INTERVAL_t   interval;

    interval.value[0] = uvBox.param[0];
    interval.value[1] = uvBox.param[2];

    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    baseCurve->push_back (PSolidGeom::GetAsCurvePrimitive (sfSwept.curve, interval, false));

    // Adjust location of base profile to start param...
    if (fabs (startSweepParam) > 1.0e-12)
        {
        DVec3d      shiftDir = *((DVec3dP) sfSwept.direction.coord);

        shiftDir.ScaleToLength (startSweepParam);

        Transform   shiftTransform = Transform::From (shiftDir);

        baseCurve->TransformInPlace (shiftTransform);
        }

    DgnExtrusionDetail  detail (baseCurve, extrudeDir, false);

    return ISolidPrimitive::CreateDgnExtrusion (detail);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidPrimitivePtr PSolidGeom::FaceToSolidPrimitive(PK_FACE_t faceTag, CurveVectorPtr* uvBoundaries)
    {
    PK_LOGICAL_t    isBox;
    PK_UVBOX_t      uvBox;

    if (SUCCESS != PK_FACE_is_uvbox (faceTag, &isBox, &uvBox) || (!isBox && NULL == uvBoundaries))
        return NULL;

    if (!isBox && (SUCCESS != PK_FACE_find_uvbox (faceTag, &uvBox) || ! (*uvBoundaries = FaceToUVCurveVector (faceTag, &uvBox, false)).IsValid()))
        return NULL; // This can occur on some faces.... Error code is typically 1008 (failed to produce trimmed surface).

    PK_SURF_t   surface;
    PK_CLASS_t  surfaceClass;

    PK_FACE_ask_surf (faceTag, &surface);
    PK_ENTITY_ask_class (surface, &surfaceClass);

    switch (surfaceClass)
        {
        case PK_CLASS_cyl:
        case PK_CLASS_cone:
            return solidPrimitiveFromConeFace (faceTag, uvBox, uvBoundaries);

        case PK_CLASS_torus:
            return solidPrimitiveFromTorusFace (faceTag, uvBox, uvBoundaries);

        case PK_CLASS_sphere:
            return solidPrimitiveFromSphereFace (faceTag, uvBox, uvBoundaries);

        case PK_CLASS_swept:
            return solidPrimitiveFromSweptFace (faceTag, uvBox, uvBoundaries);

        default:
            return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void addEdgeToCurveVector (CurveVectorR curveVector, PK_EDGE_t edgeTag)
    {
    ICurvePrimitivePtr  primitive;

    if (SUCCESS != PSolidGeom::EdgeToCurvePrimitive (primitive, edgeTag))
        return;

    DPoint3d    startPt, endPt;

    // If appending to existing curve, make sure it's connected head to tail...
    if (curveVector.GetStartEnd (startPt, endPt))
        {
        DPoint3d    thisStartPt, thisEndPt;

        if (!primitive->GetStartEnd (thisStartPt, thisEndPt))
            return;

        if (thisStartPt.IsEqual (endPt, 1.0e-8))
            {
            curveVector.push_back (primitive);
            }
        else if (thisEndPt.IsEqual (startPt, 1.0e-8))
            {
            curveVector.insert (curveVector.begin (), primitive);
            }
        else
            {
            BeAssert (false); // Disjoint wire body?!?
            curveVector.SetBoundaryType (CurveVector::BOUNDARY_TYPE_None);
            curveVector.push_back (primitive);
            }

        return;
        }

    curveVector.push_back (primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool appendConnectedEdge (bvector<PK_EDGE_t>& connectedEdges, bvector<PK_EDGE_t>& unorderedEdges, PK_VERTEX_t& firstVertexTag, PK_VERTEX_t& lastVertexTag)
    {
    for (PK_EDGE_t& thisEdge : unorderedEdges)
        {
        PK_VERTEX_t  vertices[2];

        if (SUCCESS != PK_EDGE_ask_vertices (thisEdge, vertices))
            continue;

        if (PK_ENTITY_null != vertices[0] && vertices[0] == lastVertexTag)
            {
            lastVertexTag = vertices[1];
            connectedEdges.push_back (thisEdge);
            unorderedEdges.erase (std::remove (unorderedEdges.begin (), unorderedEdges.end (), thisEdge), unorderedEdges.end ());

            return true;
            }
        else if (PK_ENTITY_null != vertices[1] && vertices[1] == firstVertexTag)
            {
            firstVertexTag = vertices[0];
            connectedEdges.insert (connectedEdges.begin (), thisEdge);
            unorderedEdges.erase (std::remove (unorderedEdges.begin (), unorderedEdges.end (), thisEdge), unorderedEdges.end ());

            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void getOrderedEdges (PK_EDGE_t* edges, int nEdges)
    {
    if (nEdges < 2)
        return;

    PK_VERTEX_t  vertices[2];

    if (SUCCESS != PK_EDGE_ask_vertices (edges[0], vertices) || (PK_ENTITY_null == vertices[0] || PK_ENTITY_null == vertices[1]))
        return;

    bvector<PK_EDGE_t>  connectedEdges, unorderedEdges;

    connectedEdges.reserve (nEdges);
    connectedEdges.push_back (edges[0]);
    unorderedEdges.insert (unorderedEdges.begin (), &edges[1], &edges[nEdges]);

    do
        {
        if (!appendConnectedEdge (connectedEdges, unorderedEdges, vertices[0], vertices[1]))
            {
            BeAssert (false); // Disjoint wire bodies aren't supported...
            return;
            }

        } while ((int)connectedEdges.size () < nEdges);

    memcpy (edges, &connectedEdges.front (), nEdges * sizeof (*edges));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::BodyToCurveVectors (bvector<CurveVectorPtr>& curves, IBRepEntityCR entity, EdgeToCurveIdMap const* idMap)
    {
    int         nFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_BODY_ask_faces (PSolidUtil::GetEntityTag (entity), &nFaces, &faces);

    for (int i=0; i<nFaces; i++)
        {
        CurveVectorPtr  curveVector = PSolidGeom::PlanarFaceToCurveVector (faces[i], idMap);

        if (curveVector.IsValid () && curveVector->size () > 0)
            {
            curveVector->TransformInPlace (entity.GetEntityTransform ());

            curves.push_back (curveVector);
            }
        }

    PK_MEMORY_free (faces);

    PK_REGION_t*    regions = NULL;
    int             nRegions = 0;

    if (SUCCESS == PK_BODY_ask_regions (PSolidUtil::GetEntityTag (entity), &nRegions, &regions))
        {
        for (int i = nRegions > 1 ? 1 : 0; i < nRegions; i++)
            {
            PK_SHELL_t*  shells = NULL;
            int          nShells = 0;

            if (SUCCESS == PK_REGION_ask_shells (regions[i], &nShells, &shells))
                {
                for (int i=0; i<nShells; i++)
                    {
                    PK_EDGE_t*  edges = NULL;
                    int         nEdges = 0;

                    if (SUCCESS == PK_SHELL_ask_wireframe_edges (shells[i], &nEdges, &edges) && nEdges > 0)
                        {
                        CurveVectorPtr  curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

                        getOrderedEdges (edges, nEdges);

                        for (int i=0; i<nEdges; i++)
                            addEdgeToCurveVector (*curveVector, edges[i]);

                        PK_MEMORY_free (edges);

                        if (curveVector.IsValid () && curveVector->size () > 0)
                            {
                            curveVector->TransformInPlace (entity.GetEntityTransform ());

                            curves.push_back (curveVector);
                            }
                        }
                    }

                PK_MEMORY_free (shells);
                }
            }

        PK_MEMORY_free (regions);
        }

    return (curves.size () > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  PSolidGeom::WireBodyToCurveVector (IBRepEntityCR entity)
    {
    PK_BODY_type_t  bodyType;

    PK_BODY_ask_type (PSolidUtil::GetEntityTag (entity), &bodyType);

    if (PK_BODY_type_wire_c != bodyType)
        return NULL;

    int             nEdges = 0;
    PK_EDGE_t*      edges = NULL;
    CurveVectorPtr  curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    PK_BODY_ask_edges (PSolidUtil::GetEntityTag (entity), &nEdges, &edges);
    getOrderedEdges (edges, nEdges);

    for (int i=0; i<nEdges; i++)
        addEdgeToCurveVector (*curveVector, edges[i]);

    PK_MEMORY_free (edges);

    if (!curveVector.IsValid () || curveVector->size () < 1)
        return NULL;

    curveVector->TransformInPlace (entity.GetEntityTransform ());

    return curveVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus triangulatedBodyFromNonPlanarPolygon (PK_BODY_t& bodyTag, CurveVectorCR curves, TransformCR uorToBodyTransform)
    {
    if (!curves.IsAnyRegionType ())
        return ERROR;

    if (curves.ContainsNonLinearPrimitive ())
        return ERROR;

    Transform                   localToWorld, worldToLocal;
    DRange3d                    range;

    if (curves.IsPlanar (localToWorld, worldToLocal, range))
        return ERROR;

    IFacetOptionsPtr            facetOptions = IFacetOptions::Create ();
    IPolyfaceConstructionPtr    builder = IPolyfaceConstruction::Create (*facetOptions);

    builder->AddRegion(curves);

    return PSolidGeom::BodyFromPolyface (bodyTag, builder->GetClientMeshR(), uorToBodyTransform);
    }

// Default values for gap closure options ...
static double s_defaultEqualPointTolerance = 1.0e-10;   // should be "like" PSD resabs
                                                        // BUT ... it seems to be good to make this SMALLER so that
                                                        //     we call in "move the endpoints" machinery to REALLY close the gaps instead of
                                                        //     just hoping we "really" understand what PSD will close up.
static double s_defaultMaxDirectAdjust     = 1.0e-4;    // gaps this large can be closed by just moving endpoints (i.e. without gap segment).
                                                        // (And this adjustemnt can be away from the curve direction)
static double s_defaultMaxAdjustAlongCurve = 1.0e-3;    // motion along the curve by this much is permitted.

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   06/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int pki_make_minimal_body(int* pBodyOut)
    {
    PK_POINT_t      point;
    PK_POINT_sf_t   pointSF;

    memset (&pointSF, 0, sizeof (pointSF));
    PK_POINT_create (&pointSF, &point);

    return PK_POINT_make_minimum_body (point, pBodyOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int pki_scribe_curve_on_body
(
int             *pEdgeTagOut,           /* <= (optional) output edge tag (in body) created  */
int             bodyTagInOut,           /* <=> body tag on which to scribe  */
int             curveTagIn,             /* => input curve to scribe  */
double          *startParamInP,         /* => input start parameter on curve (or NULL for start)   */
double          *endParamInP            /* => input end parameter on curve (or NULL for end)  */
)
    {
    int             failureCode, numNewEdge = 0, numNewFace = 0;
    PK_EDGE_t       *pNewEdges = NULL;
    PK_FACE_t       *pNewFaces = NULL;
    PK_INTERVAL_t   bounds;

    PK_CURVE_ask_interval (curveTagIn, &bounds);

    if (NULL != startParamInP)
        bounds.value[0] = *startParamInP;

    if (NULL != endParamInP)
        bounds.value[1] = *endParamInP;

    failureCode = PK_BODY_imprint_curve (bodyTagInOut, curveTagIn, bounds, &numNewEdge, &pNewEdges, &numNewFace, &pNewFaces);

    if (NULL != pNewEdges && numNewEdge >= 1)
        {
        if (NULL != pEdgeTagOut)
            *pEdgeTagOut = *pNewEdges;
        }
    else
        {
        /* Ignore "point-like" segments */
        if (failureCode == PK_ERROR_curve_too_short)
            failureCode = SUCCESS;
        else
            failureCode = failureCode ? failureCode : ERROR;
        }

    PK_MEMORY_free (pNewEdges);
    PK_MEMORY_free (pNewFaces);

    return failureCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_cover_face_with_surface(int faceTagIn)
    {
    PK_LOGICAL_t        localCheckFlag = PK_LOGICAL_false;
    PK_local_check_t    localCheckStatus;

    return PK_FACE_attach_surf_fitting (faceTagIn, localCheckFlag, &localCheckStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::CoverWires (PK_BODY_t body)
    {
    int         numFace = 0, *faces = NULL;

    if (SUCCESS != PK_BODY_ask_faces (body, &numFace, &faces))
        return ERROR;

    for (int i=0; i < numFace; i++)
        pki_cover_face_with_surface (faces[i]);

    PK_MEMORY_free (faces);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::ImprintSegment (PK_BODY_t body, PK_EDGE_t* edge, DPoint3dCP segment)
    {
    if (segment[0].IsEqual (segment[1]))
        return SUCCESS; // Nothing to do.

    PK_LINE_t       line;
    PK_LINE_sf_t    sfLine;

    sfLine.basis_set.location.coord[0] = segment[0].x;
    sfLine.basis_set.location.coord[1] = segment[0].y;
    sfLine.basis_set.location.coord[2] = segment[0].z;

    double      length = ((DVec3d *) &sfLine.basis_set.axis)->NormalizedDifference (segment[1], segment[0]);
    int         nNewEdges, nNewFaces;

    if (SUCCESS != PK_LINE_create (&sfLine, &line))
        return ERROR;

    PK_INTERVAL_t   bounds;

    bounds.value[0] = 0.0;
    bounds.value[1] = length;

    PK_EDGE_t*      edges = NULL;
    StatusInt       status;

    if (SUCCESS == (status = PK_BODY_imprint_curve (body, line, bounds, &nNewEdges, &edges, &nNewFaces, NULL)) && nNewEdges > 0)
        *edge = edges[0];

    PK_MEMORY_free (edges);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nick.Shulga     04/97
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_make_ellipse_curve_from_standard_data
(
int             *pCurveTagOut,          /* <= output curve tag created  */
DPoint3d        *pLocation,
DPoint3d        *p_zAxis,
DPoint3d        *p_refDirection,
double          majorAxis,
double          minorAxis
)
    {
    /* this function uses arguments similar to Parasolid standard form.  */
    /* the same data happens to be used in STEP.  */
    if (pCurveTagOut && pLocation && p_zAxis && p_refDirection && majorAxis > 0 && minorAxis > 0)
        {
        int     failureCode = ERROR;

        if (DoubleOps::WithinTolerance(minorAxis, majorAxis, 1e-08))
            {
            /* create circle  */
            PK_CIRCLE_sf_t circleCurve;

            circleCurve.basis_set.location.coord[0] = pLocation->x;
            circleCurve.basis_set.location.coord[1] = pLocation->y;
            circleCurve.basis_set.location.coord[2] = pLocation->z;

            circleCurve.basis_set.ref_direction.coord[0] = p_refDirection->x;
            circleCurve.basis_set.ref_direction.coord[1] = p_refDirection->y;
            circleCurve.basis_set.ref_direction.coord[2] = p_refDirection->z;

            circleCurve.basis_set.axis.coord[0] = p_zAxis->x;
            circleCurve.basis_set.axis.coord[1] = p_zAxis->y;
            circleCurve.basis_set.axis.coord[2] = p_zAxis->z;

            circleCurve.radius = majorAxis;

            failureCode = PK_CIRCLE_create (&circleCurve, pCurveTagOut);
            }
        else
            {
            /* create ellipse  */
            PK_ELLIPSE_sf_t ellipseCurve;

            ellipseCurve.basis_set.location.coord[0] = pLocation->x;
            ellipseCurve.basis_set.location.coord[1] = pLocation->y;
            ellipseCurve.basis_set.location.coord[2] = pLocation->z;

            ellipseCurve.basis_set.axis.coord[0] = p_zAxis->x;
            ellipseCurve.basis_set.axis.coord[1] = p_zAxis->y;
            ellipseCurve.basis_set.axis.coord[2] = p_zAxis->z;

            /* If major axis is less than minor axis - swap because parasolid doesnt handle it */
            if (majorAxis < minorAxis)
                {
                DPoint3d    majorAxisVec;

                ellipseCurve.R1 = minorAxis;
                ellipseCurve.R2 = majorAxis;

                /* cross z and ref direction for majorAxis Vec */
                majorAxisVec.CrossProduct (*p_zAxis, *p_refDirection);
                ellipseCurve.basis_set.ref_direction.coord[0] = majorAxisVec.x;
                ellipseCurve.basis_set.ref_direction.coord[1] = majorAxisVec.y;
                ellipseCurve.basis_set.ref_direction.coord[2] = majorAxisVec.z;
                }
            else
                {
                ellipseCurve.R1 = majorAxis;
                ellipseCurve.R2 = minorAxis;

                ellipseCurve.basis_set.ref_direction.coord[0] = p_refDirection->x;
                ellipseCurve.basis_set.ref_direction.coord[1] = p_refDirection->y;
                ellipseCurve.basis_set.ref_direction.coord[2] = p_refDirection->z;
                }

            failureCode = PK_ELLIPSE_create (&ellipseCurve, pCurveTagOut);
            }

        return failureCode;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::MakeEllipseCurve (PK_CURVE_t* curve, double* startParam, double* endParam, DPoint3dP center, RotMatrixP rMatrix, double x1, double x2, double startAngle, double sweepAngle)
    {
    double      radiusRatio;
    DVec3d      normal;
    DVec3d      majorAxisVec;
    double      r1, r2;

    rMatrix->GetColumn(normal,  2);
    rMatrix->GetColumn(majorAxisVec,  0);

    r1 = x1;
    r2 = x2;

    radiusRatio = x1 ? x2 / x1 : 0.0;

    if (fabs (1.0 - radiusRatio) < TOLERANCE_CircleAxisRatio)
        {
        x2 = x1;
        radiusRatio = 1.0;
        }

    /* Correct parametrization to "theta" angles */
    if (sweepAngle != msGeomConst_2pi)
        {
        *startParam = startAngle;
        *endParam   = startAngle + sweepAngle;
        }
    else
        {
        *startParam = 0.0;
        *endParam   = sweepAngle;
        }

    if (sweepAngle < 0.0)
        {
        *startParam = - *startParam;
        *endParam   = - *endParam;

        normal.Scale (normal,  -1.0);
        }

    if (*startParam > *endParam)
        *endParam += msGeomConst_2pi;

    /* If Major axis is less than minor axis swap because Parasolid cant handle it */
    if (x2 > x1)
        {
        *startParam -= msGeomConst_piOver2;
        *endParam   -= msGeomConst_piOver2;
        }

    return (BentleyStatus) pki_make_ellipse_curve_from_standard_data (curve, center, &normal, &majorAxisVec, r1, r2);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromCurveVector (PK_BODY_t& bodyTag, PK_VERTEX_t* startVertexP, CurveVectorCR curves, TransformCR uorToBodyTransform, bool coverClosed, EdgeToCurveIdMap* idMap)
    {
    if (1 > curves.size ())
        return ERROR;

    StatusInt   status = SUCCESS;

    if (curves.IsUnionRegion ())
        {
        bvector<PK_BODY_t> regions;

        bodyTag = PK_ENTITY_null;

        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (false && "Unexpected entry in union region.");

                return ERROR; // Each loop must be a child curve bvector (a closed loop or parity region)...
                }

            CurveVector const* childCurves = curve->GetChildCurveVectorCP ();

            PK_BODY_t   childBodyTag = PK_ENTITY_null;

            if (SUCCESS != (status = bodyFromCurveVector (childBodyTag, startVertexP, *childCurves, uorToBodyTransform, coverClosed, idMap)))
                break;

            if (PK_ENTITY_null == bodyTag)
                bodyTag = childBodyTag;
            else
                regions.push_back (childBodyTag);
            }

        if (SUCCESS == status && regions.size () > 0)
            status = PSolidUtil::Boolean (NULL, PK_boolean_unite, false, bodyTag, &regions[0], (int) regions.size (), PKI_BOOLEAN_OPTION_AllowDisjoint);

        if (SUCCESS != status)
            {
            PK_ENTITY_delete (1, &bodyTag);
            PK_ENTITY_delete ((int) regions.size (), &regions[0]);
            }
        }
    else if (curves.IsParityRegion ())
        {
        bvector<PK_BODY_t> holes;

        bodyTag = PK_ENTITY_null;

        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (false && "Unexpected entry in parity region.");

                return ERROR; // Each loop must be a child curve bvector (a closed loop)...
                }

            CurveVector const* childCurves = curve->GetChildCurveVectorCP ();

            PK_BODY_t   childBodyTag = PK_ENTITY_null;

            if (SUCCESS != (status = bodyFromCurveVector (childBodyTag, startVertexP, *childCurves, uorToBodyTransform, coverClosed, idMap)))
                break;

            if (CurveVector::BOUNDARY_TYPE_Outer == childCurves->GetBoundaryType ())
                {
                BeAssert (PK_ENTITY_null == bodyTag && "Parity region should have a single outer loop");
                bodyTag = childBodyTag;
                }
            else
                {
                holes.push_back (childBodyTag);
                }
            }

        if (SUCCESS == status && holes.size () > 0)
            status = PSolidUtil::Boolean (NULL, coverClosed ? PK_boolean_subtract : PK_boolean_unite, !coverClosed, bodyTag, &holes[0], (int) holes.size (), PKI_BOOLEAN_OPTION_AllowDisjoint);

        if (SUCCESS != status)
            {
            PK_ENTITY_delete (1, &bodyTag);
            PK_ENTITY_delete ((int) holes.size (), &holes[0]);
            }
        }
    else
        {
        pki_make_minimal_body (&bodyTag);

        PK_VERTEX_t     startVertex;

        if (NULL == startVertexP)
            startVertexP = &startVertex;

        *startVertexP = PK_ENTITY_null;

        PK_EDGE_t   edgeTag = PK_ENTITY_null;

        unsigned long   id = 1;

        if (NULL != idMap)
            {
            for (EdgeToCurveIdMap::iterator curr = idMap->begin(); curr != idMap->end(); curr++)
                if (curr->first >= id)
                    id = curr->first;
            }

        // NOTE: Scribe curves in reverse order to match SS3 behavior (required for entity id assignment)...
        for (size_t iCurve = curves.size (); iCurve > 0; --iCurve)
            {
            ICurvePrimitivePtr  curve = curves.at (iCurve-1);

            if (!curve.IsValid ())
                continue;

            switch (curve->GetCurvePrimitiveType ())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    curve = ICurvePrimitive::CreateLineString (&curve->GetLineCP ()->point[0], 2);

                    // FALL THROUGH...
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> points = *curve->GetLineStringCP ();

                    uorToBodyTransform.Multiply (&points.front (), (int) points.size ());

                    static double   s_minimumSegmentLength = 1.0E-7;

                    // NOTE: Scribe segments in reverse order to match SS3 behavior (required for entity id assignment)...
                    for (size_t j = points.size ()-1; j > 0 && SUCCESS == status; --j)
                        {
                        DSegment3d  segment = DSegment3d::From (points[j-1], points[j]);

                        if (segment.Length () <= s_minimumSegmentLength)
                            {
                            // Make sure that the last (closure) point is used and that interior small segments are consolidated...
                            points[j-1] = points[j];
                            continue;
                            }

                        if (SUCCESS != (status = PSolidUtil::ImprintSegment (bodyTag, &edgeTag, segment.point)))
                            break;
                        }
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3d      ellipse = *curve->GetArcCP ();
                    double          r0, r1, theta0, sweep;
                    DPoint3d        center;
                    RotMatrix       rMatrix;

                    uorToBodyTransform.Multiply (ellipse, ellipse);
                    ellipse.GetScaledRotMatrix (center, rMatrix, r0, r1, theta0, sweep);

                    PK_CURVE_t      curveTag = 0;
                    double          startParam, endParam;

                    if (SUCCESS == (status = PSolidUtil::MakeEllipseCurve (&curveTag, &startParam, &endParam, &center, &rMatrix, r0, r1, theta0, sweep)))
                        status = pki_scribe_curve_on_body (&edgeTag, bodyTag, curveTag, &startParam, &endParam);

                    if (startVertexP != &startVertex && PK_ENTITY_null == *startVertexP && ellipse.IsFullEllipse ())
                        {
                        PK_VERTEX_t     newVertex;
                        PK_EDGE_t       newEdge;

                        PK_EDGE_split_at_param (edgeTag, 0.0, &newVertex, &newEdge);
                        }

                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    MSBsplineCurveCP  bcurve = curve->GetProxyBsplineCurveCP ();
                    MSBsplineCurve    curve;

                    curve.CopyTransformed (*bcurve, uorToBodyTransform);

                    PK_CURVE_t      curveTag = 0;

                    if (SUCCESS == (status = PSolidGeom::CreateCurveFromMSBsplineCurve (&curveTag, curve)))
                        status = pki_scribe_curve_on_body (&edgeTag, bodyTag, curveTag, NULL, NULL);

                    if (SUCCESS != status)
                        {
                        PSolidUtil::NormalizeBsplineCurve (curve);

                        if (SUCCESS == (status = PSolidGeom::CreateCurveFromMSBsplineCurve (&curveTag, curve)))
                            status = pki_scribe_curve_on_body (&edgeTag, bodyTag, curveTag, NULL, NULL);
                        }

                    curve.ReleaseMem ();
                    break;
                    }

                default:
                    {
                    BeAssert (false && "Unexpected entry in CurveVector.");
                    break;
                    }
                }

            if (SUCCESS != status)
                break;

            if (NULL != idMap && NULL != curve->GetId())
                {
                PSolidTopoId::AttachEntityId (edgeTag, id, 0);
                (*idMap)[id++] = curve->GetId();
                }
            }

        if (SUCCESS == status)
            {
            if (PK_ENTITY_null != edgeTag)
                {
                PK_VERTEX_t  vertices[2];

                PK_EDGE_ask_vertices (edgeTag, vertices);
                *startVertexP = vertices[0];
                }

            if (coverClosed && curves.IsClosedPath ())
                {
                // TR#270092 - Cover sometimes creates bad bodies with cone rather than planar surfaces that cause "hangs" when queried?!?
                status = ((SUCCESS == PSolidUtil::CoverWires (bodyTag) && PSolidUtil::HasOnlyPlanarFaces (bodyTag)) ? SUCCESS : ERROR);
                }
            else
                {
                PK_FACE_t   faceTag;

                // Make sure physically closed open profiles are returned as wire boides
                if (SUCCESS == PK_BODY_ask_first_face (bodyTag, &faceTag) && PK_ENTITY_null != faceTag)
                    PK_FACE_delete_from_sheet_body (faceTag);
                }

            }

        if (SUCCESS != status)
            PK_ENTITY_delete (1, &bodyTag);
        }

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::BodyFromCurveVector (PK_BODY_t& bodyTag, PK_VERTEX_t* startVertexP, CurveVectorCR rawCurves, TransformCR uorToBodyTransform, bool coverClosed, EdgeToCurveIdMap* idMap)
    {
    if (1 > rawCurves.size ())
        return ERROR;

    double          a = uorToBodyTransform.ColumnXMagnitude ();
    double          b;

    DoubleOps::SafeDivide (b, 1.0, a, 1.0);

    double          uorEqualPoint = s_defaultEqualPointTolerance * b;
    double          uorDirectAdjust = s_defaultMaxDirectAdjust * b;
    double          uorAlongCurveAdust = s_defaultMaxAdjustAlongCurve * b;
    CurveGapOptions options (uorEqualPoint, uorDirectAdjust, uorAlongCurveAdust);
    CurveVectorPtr  curvesNoGaps = rawCurves.CloneWithGapsClosed (options);
    CurveVectorCP   curves = curvesNoGaps.get ();
    double          resolvedGap = curvesNoGaps->MaxGapWithinPath ();

    if (resolvedGap > uorEqualPoint)
        curves = &rawCurves;

    if (coverClosed && NULL == startVertexP && SUCCESS == triangulatedBodyFromNonPlanarPolygon (bodyTag, *curves, uorToBodyTransform))
        return SUCCESS;

    if (coverClosed && curves->IsClosedPath ())
        {
        switch (curves->HasSingleCurvePrimitive ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                {
                Transform   localToWorld, worldToLocal;

                if (!curves->GetAnyFrenetFrame (localToWorld) || !worldToLocal.InverseOf (localToWorld))
                    break;

                DRange3d    localRange;

                if (!curves->GetRange (localRange, worldToLocal))
                    break;

                double  planarTolerance = (localRange.low.DistanceXY (localRange.high)) * 1.0e-3;

                if (fabs (localRange.high.z - localRange.low.z) > planarTolerance)
                    coverClosed = false; // Don't flatten or try to cover REALLY non-planar closed bcurves...
                break;
                }
            }
        }

    Transform   compositeTransform = uorToBodyTransform;
    double      area;
    DVec3d      normal;
    DPoint3d    centroid;

    if (coverClosed && curves->IsAnyRegionType () && curves->CentroidNormalArea (centroid, normal, area))
        {
        Transform   flattenTransform;

        flattenTransform.InitFromProjectionToPlane (centroid,normal);
        compositeTransform.InitProduct (compositeTransform, flattenTransform);
        }

    return bodyFromCurveVector (bodyTag, startVertexP, *curves, compositeTransform, coverClosed, idMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidUtil::GetTransforms(TransformR solidToUor, TransformR uorToSolid, DPoint3dCP origin, double solidScale)
    {
    solidToUor.InitIdentity();

    if (nullptr != origin)
        solidToUor.SetTranslation(*origin);

    solidToUor.ScaleMatrixColumns(solidToUor, solidScale, solidScale, solidScale);
    uorToSolid.InverseOf(solidToUor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidGeom::BodyFromCurveVector (IBRepEntityPtr& entityOut, CurveVectorCR curveVector, TransformCP curveToDgn, uint32_t nodeId, EdgeToCurveIdMap* idMap)
    {
    DPoint3d    startPt;

    if (!curveVector.GetStartPoint (startPt))
        return ERROR;

    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    Transform   solidToDgn, dgnToSolid, curveToBody, bodyToCurve;

    PSolidUtil::GetTransforms (solidToDgn, dgnToSolid);

    curveToBody = (NULL == curveToDgn) ? dgnToSolid : Transform::FromProduct (dgnToSolid, *curveToDgn);
    curveToBody.TranslateInLocalCoordinates (curveToBody, -startPt.x, -startPt.y, -startPt.z);
    bodyToCurve.InverseOf (curveToBody);

    PK_BODY_t   bodyTag;

    if (SUCCESS != PSolidGeom::BodyFromCurveVector (bodyTag, NULL, curveVector, curveToBody, true, idMap))
        return ERROR;

    if (nodeId)
        PSolidTopoId::AssignProfileBodyIds (bodyTag, nodeId);

    entityOut = PSolidUtil::CreateNewEntity(bodyTag, bodyToCurve);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int pki_create_solid_by_capping(PK_BODY_t* pBodyInOut)
    {
    int               failureCode;
    int               numFace = 0, nBodies = 0;
    PK_FACE_t*        pFaceArray = NULL;
    PK_BODY_t*        pBodies = NULL;
    PK_LOGICAL_t      localCheckFlag = PK_LOGICAL_false;
    PK_local_check_t* localCheckStatus;

    PK_BODY_ask_faces(*pBodyInOut, &numFace, &pFaceArray);

    failureCode = PK_FACE_make_solid_bodies(numFace, pFaceArray, PK_FACE_heal_cap_c, localCheckFlag, &nBodies, &pBodies, &localCheckStatus);

    if (nBodies >= 1 && pBodies)
        {
        PK_ENTITY_delete(1, pBodyInOut);
        *pBodyInOut = pBodies[0];

        if (nBodies > 1)
            PK_ENTITY_delete(nBodies-1, &pBodies[1]);

        PK_MEMORY_free(pBodies);
        }

    PK_MEMORY_free(pFaceArray);

    return failureCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void getDefaultBasis(PK_AXIS2_sf_t* basisP)
    {
    memset (basisP, 0, sizeof (*basisP));

    basisP->axis.coord[2] = 1.0;
    basisP->ref_direction.coord[0] = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   06/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_make_cone
(
int             *pBodyOut,                  /* <= output tag of body created */
double          bottomRadiusIn,             /* => input bottom radius */
double          topRadiusIn,                /* => input top radius */
double          heightIn                    /* => input height */
)
    {
    int             failureCode;
    PK_AXIS2_sf_t   basis;

    *pBodyOut = NULTAG;

    if (DoubleOps::WithinTolerance(heightIn, 0.0, 1e-08) || (DoubleOps::WithinTolerance(bottomRadiusIn, 0.0, 1e-08) && DoubleOps::WithinTolerance(topRadiusIn, 0.0, 1e-08)))
        return ERROR;

    if (0.0 > bottomRadiusIn)
        bottomRadiusIn *= -1.0;

    if (0.0 > topRadiusIn)
        topRadiusIn *= -1.0;

    getDefaultBasis (&basis);
    basis.location.coord[2] = -heightIn/2.0;

    if (DoubleOps::WithinTolerance(bottomRadiusIn, topRadiusIn, 1e-08))
        {
        failureCode = PK_BODY_create_solid_cyl (bottomRadiusIn, heightIn, &basis, pBodyOut);
        }
    else
        {
        double      angle, lesserRadius;

        if (bottomRadiusIn < topRadiusIn)
            {
            angle = atan ((topRadiusIn - bottomRadiusIn) / heightIn);
            lesserRadius = bottomRadiusIn;
            basis.location.coord[2] = heightIn/2.0;
            basis.axis.coord[2] = -1.0;
            }
        else
            {
            angle = atan ((bottomRadiusIn - topRadiusIn) / heightIn);
            lesserRadius = topRadiusIn;
            }

        failureCode = PK_BODY_create_solid_cone (lesserRadius, heightIn, angle, &basis, pBodyOut);
        }

    return failureCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::BodyFromCone (PK_BODY_t& bodyTag, RotMatrixCR rMatrixIn, DPoint3dCR topCenter, DPoint3dCR bottomCenter, double topRadius, double bottomRadius, bool capped, uint32_t nodeId)
    {
    double      height;
    DVec3d      normal, baseNormal, skewVector;
    DPoint3d    center;
    RotMatrix   rMatrix = rMatrixIn;
    Transform   transform, rotateTransform;

    rMatrix.GetColumn(baseNormal, 2);
    normal.DifferenceOf (topCenter, bottomCenter);
    skewVector.CrossProduct (normal, baseNormal);

    if (skewVector.Magnitude () > 1.0e-6)
        {
        MSBsplineSurface  surface;

        if (SUCCESS != bspconv_coneToSurface (&surface, topRadius, bottomRadius, &rMatrix, const_cast<DPoint3dP>(&topCenter), const_cast<DPoint3dP>(&bottomCenter)))
            return ERROR;

        StatusInt   status;

        if (SUCCESS == (status = PSolidGeom::BodyFromMSBsplineSurface (bodyTag, surface)) && capped)
            status = pki_create_solid_by_capping (&bodyTag);

        surface.ReleaseMem ();

        if (nodeId && SUCCESS == status)
            PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

        return (BentleyStatus) status;
        }

    height = normal.Normalize ();

    if (baseNormal.DotProduct (normal) < 0.0)
        {
        RotMatrix   rotate180;

        rotate180.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (), msGeomConst_pi, 0.0, 0.0);
        rMatrix.InitProduct(rMatrix, rotate180);
        }

    if (SUCCESS != pki_make_cone (&bodyTag, topRadius, bottomRadius, height))
        return ERROR;

    if (nodeId)
        {
        PSolidTopoId::AssignConeFaceIds (bodyTag, nodeId);
        PSolidTopoId::AssignEdgeIds (bodyTag, nodeId, true);
        }

    if (!capped)
        {
        int         nFaces;
        PK_FACE_t*  faces = NULL;

        PK_BODY_ask_faces (bodyTag, &nFaces, &faces);

        for (int i=0; i < nFaces; i++)
            {
            PK_SURF_t   surface;
            PK_CLASS_t  surfaceClass;

            PK_FACE_ask_surf (faces[i], &surface);
            PK_ENTITY_ask_class (surface, &surfaceClass);

            if (PK_CLASS_cone == surfaceClass || PK_CLASS_cyl == surfaceClass)
                {
                PK_BODY_t   sheetBody;

                if (SUCCESS == PK_FACE_make_sheet_body (1, &faces[i], &sheetBody))
                    {
                    PK_ENTITY_delete (1, &bodyTag);
                    bodyTag = sheetBody;
                    }

                break;
                }
            }

        PK_MEMORY_free (faces);
        }

    center.SumOf (bottomCenter, topCenter);
    center.Scale (center, 0.5);

    rotateTransform.InitFrom (rMatrix);
    transform.InitFrom (center.x, center.y, center.z);
    transform.InitProduct (transform, rotateTransform);

    PSolidUtil::TransformBody (bodyTag, transform);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::BodyFromLoft (PK_BODY_t& bodyTag, PK_BODY_t* profiles, PK_VERTEX_t* startVertices, size_t nProfiles, PK_BODY_t* guides, size_t nGuides, bool periodic)
    {
    if ((nProfiles < 1) || (nProfiles < (periodic ? 3 : 2) && nGuides < 1))
        return ERROR;

    PK_BODY_make_lofted_body_o_t    options;
    PK_BODY_tracked_loft_r_t        result;

    memset (&result, 0, sizeof (result));
    PK_BODY_make_lofted_body_o_m (options);
    options.end_conditions.periodic = periodic ? PK_PARAM_periodic_yes_c : PK_PARAM_periodic_no_c;

    if (nGuides)
        {
        options.n_guide_wires = (int) nGuides;
        options.guide_wires = guides;
        }

    StatusInt   status;

    if (SUCCESS == (status = PK_BODY_make_lofted_body ((int) nProfiles, profiles, startVertices, &options, &result)))
        {
        if (PK_ENTITY_null == result.body || PK_BODY_loft_ok_c != result.status.fault)
            {
            PK_ENTITY_delete (1, &result.body);
            status = ERROR;
            }
        else
            {
            bodyTag = result.body;
            }
        }

    PK_BODY_tracked_loft_r_f (&result);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getPathVertices (PK_VERTEX_t* vertices, PK_BODY_t pathTag)
    {
    PK_BODY_type_t bodyType;

    PK_BODY_ask_type (pathTag, &bodyType);

    if (PK_BODY_type_wire_c != bodyType)
        return ERROR;

    bvector <PK_EDGE_t> edges;
    PK_VERTEX_t startVertices[2], endVertices[2];

    if (SUCCESS != PSolidTopo::GetBodyEdges (edges, pathTag) || edges.empty() ||
        SUCCESS != PK_EDGE_ask_vertices (edges.front(), startVertices) ||
        SUCCESS != PK_EDGE_ask_vertices (edges.back(), endVertices) ||
        PK_ENTITY_null == startVertices[0] ||
        PK_ENTITY_null == endVertices[1])
        return ERROR;

    vertices[0] = startVertices[0];
    vertices[1] = endVertices[1];

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::BodyFromSweep (PK_BODY_t& bodyTag, PK_BODY_t profileTag, PK_BODY_t pathTag, PK_VERTEX_t pathVertexTag, bool alignParallel, bool selfRepair, DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint)

    {
    PK_BODY_make_swept_body_2_o_t   options;
    PK_BODY_tracked_sweep_2_r_t     result;

    PK_BODY_make_swept_body_2_o_m (options);
    memset (&result, 0, sizeof (result));

    options.alignment = alignParallel ? PK_BODY_sweep_align_parallel_c : PK_BODY_sweep_align_normal_c;
    options.topology_form = PK_BODY_topology_grid_c;

    if (lockDirection)
        {
        options.have_lock_direction = true;
        options.lock_type = PK_sweep_lock_path_and_dir_c;

        options.lock_direction.coord[0] = lockDirection->x;
        options.lock_direction.coord[1] = lockDirection->y;
        options.lock_direction.coord[2] = lockDirection->z;
        }

    if (NULL != twistAngle)
        {
        options.twist.law_set.vertices = new PK_VERTEX_t[2];
        options.twist.law_set.values = new double[2];   // From 3dtube.c.... Is this leaked?

        if (SUCCESS != getPathVertices (options.twist.law_set.vertices, pathTag))
            return ERROR;

        options.twist.law_set.n_vertices = 2;
        options.twist.law_set.values [0] = 0.0;
        options.twist.law_set.values [1] = *twistAngle;
        options.twist.law_type = PK_BODY_sweep_law_discrete_c;
        }

    if (NULL != scale)
        {
        if (0.0 == *scale || NULL == scalePoint)
            return  ERROR;

        options.scale_type = PK_BODY_sweep_scale_size_c;

        options.scale_point.coord[0] = scalePoint->x;
        options.scale_point.coord[1] = scalePoint->y;
        options.scale_point.coord[2] = scalePoint->z;
        options.scale.law_type = PK_BODY_sweep_law_discrete_c;

        options.scale.law_set.n_vertices = 2;                    // From 3dtube.c.... Is this leaked?
        options.scale.law_set.vertices = new PK_VERTEX_t[2];

        if (SUCCESS != getPathVertices (options.scale.law_set.vertices, pathTag))
            return ERROR;

        options.scale.law_set.values = new double[2];
        options.scale.law_set.values [0] = 1;
        options.scale.law_set.values [1] = *scale;
        }

    // unused - static bool     s_doRepair;

    if (selfRepair)
        options.repair = PK_sweep_repair_yes_c;

    StatusInt   status;

    if (SUCCESS == (status = PK_BODY_make_swept_body_2 (1, &profileTag, pathTag, &pathVertexTag, &options, &result)))
        {
        if (PK_ENTITY_null == result.body || PK_BODY_sweep_ok_c != result.status.fault)
            {
            PK_ENTITY_delete (1, &result.body);
            status = ERROR;
            }
        else
            {
            bodyTag = result.body;
            }
        }

    PK_BODY_tracked_sweep_2_r_f (&result);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus degeneratePointFromCurveVector (PK_BODY_t& bodyTag, CurveVectorCR curves, TransformCR uorToBodyTransform)
    {
    if (1 > curves.size () || curves.FastLength () > 1.0e-10)
        return ERROR;

    DPoint3d    startPoint;

    if (!curves.GetStartPoint (startPoint))
        return ERROR;

    PK_POINT_t  point;

    uorToBodyTransform.Multiply (startPoint);
    PK_POINT_create ((PK_POINT_sf_t*) &startPoint, &point);

    return (SUCCESS == PK_POINT_make_minimum_body (point, &bodyTag) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6386) // static analysis thinks we exceed the bounds of startVertices in the SolidPrimitiveType_DgnRuledSweep case... I don't see how.
BentleyStatus PSolidGeom::BodyFromSolidPrimitive (IBRepEntityPtr& entityOut, ISolidPrimitiveCR primitive, uint32_t nodeId)
    {
    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    Transform   solidToDgn, dgnToSolid;

    switch (primitive.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail  detail;

            if (!primitive.TryGetDgnTorusPipeDetail (detail))
                return ERROR;

            DPoint3d    center;
            DVec3d      axis;
            double      sweepRadians;

            if (!detail.TryGetRotationAxis (center, axis, sweepRadians))
                return ERROR;

            PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &center);

            DEllipse3d      minorHoop = detail.VFractionToUSectionDEllipse3d (0.0);
            CurveVectorPtr  curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

            curve->push_back (ICurvePrimitive::CreateArc (minorHoop));

            PK_BODY_t   bodyTag;

            if (SUCCESS != PSolidGeom::BodyFromCurveVector (bodyTag, NULL, *curve, dgnToSolid, primitive.GetCapped ()))
                return ERROR;

            if (nodeId)
                PSolidTopoId::AssignProfileBodyIds (bodyTag, nodeId);

            DPoint3d    zeroPoint = {0.0, 0.0, 0.0};

            dgnToSolid.MultiplyMatrixOnly (axis);

            if (SUCCESS != PSolidUtil::SweepBodyAxis (bodyTag, axis, zeroPoint, sweepRadians))
                {
                PK_ENTITY_delete (1, &bodyTag);

                return ERROR;
                }

            if (nodeId)
                PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, false);

            entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

            return SUCCESS;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail  detail;

            if (!primitive.TryGetDgnConeDetail (detail))
                return ERROR;

            DPoint3d    centerA, centerB;
            RotMatrix   rMatrix;
            double      radiusA, radiusB;
            bool        capped;

            if (!detail.IsCircular (centerA, centerB, rMatrix, radiusA, radiusB, capped))
                return ERROR;

            PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &centerA);

            DPoint3d    zeroPoint = {0.0, 0.0, 0.0}, delta;

            delta.DifferenceOf (centerB, centerA);
            dgnToSolid.MultiplyMatrixOnly (delta);

            PK_BODY_t   bodyTag;

            if (SUCCESS != PSolidGeom::BodyFromCone (bodyTag, rMatrix, zeroPoint, delta, radiusA, radiusB, capped, nodeId))
                return ERROR;

            entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

            return SUCCESS;
            }

        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail  detail;

            if (!primitive.TryGetDgnBoxDetail (detail))
                return ERROR;

            bvector<DPoint3d>  corners;

            detail.GetCorners (corners);

            DPoint3d  baseRectangle[5];

            baseRectangle[0] = corners[0];
            baseRectangle[1] = corners[1];
            baseRectangle[2] = corners[3];
            baseRectangle[3] = corners[2];
            baseRectangle[4] = corners[0];

            DPoint3d  topRectangle[5];

            topRectangle[0] = corners[4];
            topRectangle[1] = corners[5];
            topRectangle[2] = corners[7];
            topRectangle[3] = corners[6];
            topRectangle[4] = corners[4];

            CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
            CurveVectorPtr  topCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

            baseCurve->push_back (ICurvePrimitive::CreateLineString (baseRectangle, 5));
            topCurve->push_back (ICurvePrimitive::CreateLineString (topRectangle, 5));

            DgnRuledSweepDetail  ruleDetail (baseCurve, topCurve, primitive.GetCapped ());
            ISolidPrimitivePtr   rulePrimitive = ISolidPrimitive::CreateDgnRuledSweep (ruleDetail);

            return BodyFromSolidPrimitive (entityOut, *rulePrimitive, nodeId);
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail  detail;

            if (!primitive.TryGetDgnSphereDetail (detail))
                return ERROR;

            DPoint3d    origin;
            double      sweepRadians;
            DVec3d      axis;

            if (!detail.TryGetRotationAxis (origin, axis, sweepRadians))
                return ERROR;

            PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &origin);

            DEllipse3d  ellipse = detail.UFractionToVSectionDEllipse3d (0.0);

            if (ellipse.IsCircular ()) // Special case for Sphere...
                {
                double      r0, r1, theta0, sweep;
                DPoint3d    center;
                RotMatrix   rMatrix;
                DEllipse3d  tmpEllipse;


                dgnToSolid.Multiply (tmpEllipse, ellipse);
                tmpEllipse.GetScaledRotMatrix (center, rMatrix, r0, r1, theta0, sweep);

                PK_BODY_t   bodyTag;

                if (SUCCESS == PK_BODY_create_solid_sphere (r0, NULL, &bodyTag))
                    {
                    Transform   sphereTransform;

                    sphereTransform.InitFrom (rMatrix, center);
                    PSolidUtil::TransformBody (bodyTag, sphereTransform);

                    if (nodeId)
                        PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

                    entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

                    return SUCCESS;
                    }
                }

            CurveVectorPtr  curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open); // Only true sphere should be a solid...

            curve->push_back (ICurvePrimitive::CreateArc (ellipse));

            PK_BODY_t   bodyTag;

            if (SUCCESS != PSolidGeom::BodyFromCurveVector (bodyTag, NULL, *curve, dgnToSolid))
                return ERROR;

            if (nodeId)
                PSolidTopoId::AssignProfileBodyIds (bodyTag, nodeId);

            DPoint3d    zeroPoint = {0.0, 0.0, 0.0};

            dgnToSolid.MultiplyMatrixOnly (axis);

            if (SUCCESS != PSolidUtil::SweepBodyAxis (bodyTag, axis, zeroPoint, sweepRadians))
                {
                PK_ENTITY_delete (1, &bodyTag);

                return ERROR;
                }

            if (nodeId)
                PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, false);

            entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

            return SUCCESS;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail  detail;

            if (!primitive.TryGetDgnExtrusionDetail (detail))
                return ERROR;

            DPoint3d    origin;

            detail.m_baseCurve->GetStartPoint (origin);

            PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &origin);

            PK_BODY_t   bodyTag;

            if (SUCCESS != PSolidGeom::BodyFromCurveVector (bodyTag, NULL, *detail.m_baseCurve, dgnToSolid, primitive.GetCapped ()))
                return ERROR;

            if (nodeId)
                PSolidTopoId::AssignProfileBodyIds (bodyTag, nodeId);

            DVec3d      extrusion = detail.m_extrusionVector;

            dgnToSolid.MultiplyMatrixOnly (extrusion);

            double      distance = extrusion.Normalize ();

            if (SUCCESS != PSolidUtil::SweepBodyVector (bodyTag, extrusion, distance))
                {
                PK_ENTITY_delete (1, &bodyTag);

                return ERROR;
                }

            if (nodeId)
                PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, false);

            entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

            return SUCCESS;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail  detail;

            if (!primitive.TryGetDgnRotationalSweepDetail (detail))
                return ERROR;

            DPoint3d    origin;
            DVec3d      axis;
            double      sweepRadians;

            if (!detail.TryGetRotationAxis (origin, axis, sweepRadians))
                return ERROR;

            PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &origin);

            PK_BODY_t   bodyTag;

            if (SUCCESS != PSolidGeom::BodyFromCurveVector (bodyTag, NULL, *detail.m_baseCurve, dgnToSolid, primitive.GetCapped ()))
                return ERROR;

            if (nodeId)
                PSolidTopoId::AssignProfileBodyIds (bodyTag, nodeId);

            DPoint3d    zeroPoint = {0.0, 0.0, 0.0};

            dgnToSolid.MultiplyMatrixOnly (axis);

            if (sweepRadians > msGeomConst_2pi)
                sweepRadians = msGeomConst_2pi;
            else if (sweepRadians < -msGeomConst_2pi)
                sweepRadians = -msGeomConst_2pi;

            if (SUCCESS != PSolidUtil::SweepBodyAxis (bodyTag, axis, zeroPoint, sweepRadians))
                {
                PK_ENTITY_delete (1, &bodyTag);

                return ERROR;
                }

            if (nodeId)
                PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, false);

            entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

            return SUCCESS;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail  detail;

            if (!primitive.TryGetDgnRuledSweepDetail (detail))
                return ERROR;

            DPoint3d    origin;

            if (!detail.m_sectionCurves.front ()->GetStartPoint (origin))
                return ERROR;

            PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &origin);

            DVec3d      translation;

            if (2 == detail.m_sectionCurves.size () && detail.GetSectionCurveTranslation (translation, 0, 1))
                {
                PK_BODY_t   bodyTag;

                if (SUCCESS != PSolidGeom::BodyFromCurveVector (bodyTag, NULL, *detail.m_sectionCurves.front (), dgnToSolid, primitive.GetCapped ()))
                    return ERROR;

                if (nodeId)
                    PSolidTopoId::AssignProfileBodyIds (bodyTag, nodeId);

                dgnToSolid.MultiplyMatrixOnly (translation);

                double      distance = translation.Normalize ();

                if (SUCCESS != PSolidUtil::SweepBodyVector (bodyTag, translation, distance))
                    {
                    PK_ENTITY_delete (1, &bodyTag);

                    return ERROR;
                    }

                if (nodeId)
                    PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, false);

                entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

                return SUCCESS;
                }
            else if (detail.m_sectionCurves.front()->IsParityRegion())
                {
                // NOTE: PK_BODY_make_lofted_body doesn't support sheets with holes, try to convert by stitching faces...
                StatusInt            status = ERROR;
                DgnRuledSweepDetail  correctedDetail;

                correctedDetail.m_capped = primitive.GetCapped ();

                double a = dgnToSolid.ColumnXMagnitude (), b;

                DoubleOps::SafeDivide (b, 1.0, a, 1.0);

                CurveGapOptions     options (1.0e-10 * b, 1.0e-4 * b, 1.0e-3 * b);

                // Create new solid/surface with profiles cleaned up to remove gaps/silver faces...
                for (CurveVectorPtr& profile : detail.m_sectionCurves)
                    correctedDetail.m_sectionCurves.push_back (profile->CloneWithGapsClosed (options));

                ISolidPrimitivePtr correctedPrimitive = ISolidPrimitive::CreateDgnRuledSweep (correctedDetail);

                bvector <SolidLocationDetail::FaceIndices> faceIndices;

                correctedPrimitive->GetFaceIndices (faceIndices);

                if (0 != faceIndices.size ())
                    {
                    bvector<IBRepEntityPtr> tools;

                    for (SolidLocationDetail::FaceIndices const& thisFace: faceIndices)
                        {
                        IGeometryPtr faceGeom = correctedPrimitive->GetFace (thisFace);

                        if (!faceGeom.IsValid ())
                            continue;

                        IBRepEntityPtr faceEntity;

                        switch (faceGeom->GetGeometryType ())
                            {
                            case IGeometry::GeometryType::CurveVector:
                                {
                                CurveVectorPtr thisFace = faceGeom->GetAsCurveVector();
                                PK_BODY_t thisSurfTag;

                                if (SUCCESS != PSolidGeom::BodyFromCurveVector (thisSurfTag, nullptr, *thisFace, dgnToSolid, true))
                                    break;

                                IBRepEntityPtr faceEntity = PSolidUtil::CreateNewEntity (thisSurfTag, solidToDgn);

                                tools.push_back (faceEntity);
                                break;
                                }

                            case IGeometry::GeometryType::SolidPrimitive:
                                {
                                // NOTE: Avoids recursion and likely failure...
                                bvector<MSBsplineSurfacePtr> surfaces;

                                if (!MSBsplineSurface::CreateTrimmedSurfaces (surfaces, *faceGeom->GetAsISolidPrimitive()) || 1 != surfaces.size())
                                    break;

                                if (SUCCESS != surfaces.front()->TransformSurface (&dgnToSolid))
                                    break;

                                PK_BODY_t thisSurfTag;

                                if (SUCCESS != PSolidGeom::BodyFromMSBsplineSurface (thisSurfTag, *surfaces.front()))
                                    break;

                                IBRepEntityPtr faceEntity = PSolidUtil::CreateNewEntity (thisSurfTag, solidToDgn);

                                tools.push_back (faceEntity);
                                break;
                                }

                            case IGeometry::GeometryType::BsplineSurface:
                                {
                                MSBsplineSurfacePtr thisFace = faceGeom->GetAsMSBsplineSurface();

                                if (SUCCESS != thisFace->TransformSurface (&dgnToSolid))
                                    break;

                                PK_BODY_t thisSurfTag;

                                if (SUCCESS != PSolidGeom::BodyFromMSBsplineSurface (thisSurfTag, *thisFace))
                                    break;

                                IBRepEntityPtr faceEntity = PSolidUtil::CreateNewEntity (thisSurfTag, solidToDgn);

                                tools.push_back (faceEntity);
                                break;
                                }

                            default:
                                {
                                BeAssert (false);
                                break;
                                }
                            }
                        }

                    if (tools.size() == faceIndices.size())
                        {
                        bvector<IBRepEntityPtr> sewn;
                        bvector<IBRepEntityPtr> unsewn;

                        if (SUCCESS == BRepUtil::Modify::SewBodies (sewn, unsewn, tools, 1.0))
                            {
                            if (0 == sewn.size())
                                return ERROR;

                            entityOut = sewn.front();

                            if (primitive.GetCapped ())
                                {
                                if (1 != sewn.size() || 0 != unsewn.size())
                                    return ERROR;
                                }
                            else
                                {
                                if (sewn.size() > 1)
                                    unsewn.insert (unsewn.begin (), sewn.begin () + 1, sewn.end ());

                                if (0 != unsewn.size())
                                    {
                                    if (SUCCESS != PSolidUtil::DoBoolean(entityOut, &unsewn.front(), unsewn.size(), PK_boolean_unite, PKI_BOOLEAN_OPTION_AllowDisjoint))
                                        return ERROR;
                                    }
                                }

                            PK_TOPOL_delete_redundant (PSolidUtil::GetEntityTag (*entityOut));

                            if (nodeId)
                                PSolidTopoId::AddNodeIdAttributes (PSolidUtil::GetEntityTag (*entityOut), nodeId, true);

                            if ((primitive.GetCapped() && IBRepEntity::EntityType::Solid == entityOut->GetEntityType()) ||
                                (!primitive.GetCapped() && IBRepEntity::EntityType::Sheet == entityOut->GetEntityType()))
                                status = SUCCESS;
                            }
                        }
                    }

                return (BentleyStatus) status;
                }

            size_t          nProfiles = detail.m_sectionCurves.size ();
            PK_BODY_t*      profileBodies = (PK_BODY_t*) _alloca (nProfiles * sizeof (PK_BODY_t));
            PK_VERTEX_t*    startVertices = (PK_VERTEX_t*) _alloca (nProfiles * sizeof (PK_VERTEX_t));

            memset (profileBodies, 0, nProfiles * sizeof (PK_BODY_t));

            StatusInt       status = ERROR;

            for (size_t iProfile = 0; iProfile < nProfiles; iProfile++)
                {
                // NOTE: Need all open profiles for multi-stage linear transition even though each loft is done 2 profiles at a time.
                //       A loft of co-planar closed profiles (ex. concentric circles) isn't valid. We will instead create a
                //       surface and attempt to cap it at the end...
                bool    coverClosed = (nProfiles > 2 ? false : (primitive.GetCapped () && (0 == iProfile || iProfile+1 == nProfiles)));

                // Degenerate point profile is allowed for first or last (Scale of 0 in both X and Y)...
                if ((0 == iProfile || iProfile+1 == nProfiles) && SUCCESS == (status = degeneratePointFromCurveVector (profileBodies[iProfile], *detail.m_sectionCurves.at (iProfile), dgnToSolid)))
                    {
                    startVertices[iProfile] = PK_ENTITY_null;
                    continue;
                    }

                if (SUCCESS != (status = PSolidGeom::BodyFromCurveVector (profileBodies[iProfile], &startVertices[iProfile], *detail.m_sectionCurves.at (iProfile), dgnToSolid, coverClosed)))
                    break;
                }

            if (SUCCESS == status)
                {
                PK_BODY_make_lofted_body_o_t    options;
                PK_BODY_tracked_loft_r_t        result;

                PK_BODY_make_lofted_body_o_m (options);
                memset (&result, 0, sizeof (result));

                bvector<PK_BODY_t>  resultTags;

                for (size_t iProfilePair = 0; iProfilePair < nProfiles-1; iProfilePair++)
                    {
                    if (SUCCESS == PK_BODY_make_lofted_body (2, &profileBodies[iProfilePair], &startVertices[iProfilePair], &options, &result))
                        {
                        if (PK_ENTITY_null == result.body || PK_BODY_loft_ok_c != result.status.fault)
                            status = ERROR;

                        resultTags.push_back (result.body);
                        }

                    PK_BODY_tracked_loft_r_f (&result);
                    }

                if (SUCCESS == status)
                    {
                    if (0 == resultTags.size ())
                        {
                        status = ERROR;
                        }
                    else
                        {
                        PK_BODY_t   bodyTag = resultTags.front ();

                        if (resultTags.size () > 1)
                            {
                            status = PSolidUtil::Boolean (NULL, PK_boolean_unite, false, bodyTag, &resultTags.at (1), (int) resultTags.size ()-1, PKI_BOOLEAN_OPTION_None);

                            if (SUCCESS == status && primitive.GetCapped ())
                                pki_create_solid_by_capping (&bodyTag);
                            }

                        if (SUCCESS == status)
                            {
                            if (nodeId)
                                PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

                            entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);
                            }
                        }
                    }

                if (SUCCESS != status && 0 != resultTags.size ())
                    PK_ENTITY_delete ((int) resultTags.size (), &resultTags.front ());
                }

            PK_ENTITY_delete ((int) nProfiles, profileBodies);

            return (BentleyStatus) status;
            }

        default:
            return ERROR;
        }
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hopen.He                        03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static void addVerticesToEdge(PK_EDGE_t edge, int addVertexCount)
    {
    PK_CURVE_t curve;
    if (SUCCESS != PK_EDGE_ask_curve(edge, &curve))
        return;

    PK_VERTEX_t vertics[2];
    if (SUCCESS != PK_EDGE_ask_vertices(edge, vertics))
        return;

    PK_POINT_t psPt1, psPt2;
    PK_POINT_sf_t psSf1, psSf2;

    PK_VERTEX_ask_point(vertics[0], &psPt1);
    PK_VERTEX_ask_point(vertics[1], &psPt2);
    PK_POINT_ask(psPt1, &psSf1);
    PK_POINT_ask(psPt2, &psSf2);

    PK_INTERVAL_t interval;
    if (SUCCESS != PK_CURVE_find_vector_interval(curve, psSf1.position, psSf2.position, &interval))
        return;

    double everyPoint = (interval.value[1] - interval.value[0]) / (addVertexCount + 1);
    PK_EDGE_t currEdge = PK_ENTITY_null;

    for (int i = 1; i <= addVertexCount; i++)
        {
        double        u = interval.value[0] + i * everyPoint;
        PK_VECTOR_t   startVec;
        PK_POINT_sf_t pointSF;
        PK_POINT_t    pointTag = PK_ENTITY_null;
        PK_EDGE_t     newEdge = PK_ENTITY_null;
        PK_VERTEX_t   newVertex = PK_ENTITY_null;

        PK_CURVE_eval(curve, u, 0, &startVec);
        pointSF.position.coord[0] = startVec.coord[0];
        pointSF.position.coord[1] = startVec.coord[1];
        pointSF.position.coord[2] = startVec.coord[2];

        if (SUCCESS != PK_POINT_create(&pointSF, &pointTag))
            continue;

        if ((PK_ENTITY_null != currEdge && SUCCESS == PK_EDGE_imprint_point(currEdge, pointTag, &newVertex, &newEdge)) ||
            (currEdge != edge && SUCCESS == PK_EDGE_imprint_point(edge, pointTag, &newVertex, &newEdge)))
            currEdge = newEdge;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hopen.He                        03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static void addVerticesToEdges(PK_EDGE_t const* edges, int edgeNum, int addVertexCount)
    {
    if (nullptr == edges || edgeNum < 1 || addVertexCount < 1)
        return;

    for (int i = 0; i < edgeNum; i++)
        addVerticesToEdge(edges[i], addVertexCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hopen.He                        03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static void addProfileVertices(PK_BODY_t body, int targetVertexCount)
    {
    PK_BODY_type_t bodyType;
    if (SUCCESS != PK_BODY_ask_type(body, &bodyType) || PK_BODY_type_minimum_c == bodyType)
        return; // Leave minimal bodies alone...

    int numVertex = 0;
    if (SUCCESS != PK_BODY_ask_vertices(body, &numVertex, nullptr) || numVertex >= targetVertexCount)
        return;

    int edgeNum = 0;
    PK_EDGE_t* edges = nullptr;
    if (SUCCESS != PK_BODY_ask_edges(body, &edgeNum, &edges) || 0 == edgeNum)
        return;

    int addCount = targetVertexCount - numVertex;
    if (addCount == edgeNum)
        {
        addVerticesToEdges(edges, edgeNum, 1); // Add a single vertex to every edge...
        }
    else if (addCount > edgeNum)
        {
        int everyNum = addCount / edgeNum;
        int remainNum = addCount - (everyNum * edgeNum);

        addVerticesToEdges(edges, edgeNum, everyNum); // Add everyNum vertices to every edge...

        if (remainNum > 0)
            {
            PK_MEMORY_free(edges);
            edges = nullptr;
            edgeNum = 0;

            if (SUCCESS == PK_BODY_ask_edges(body, &edgeNum, &edges) && edgeNum >= remainNum)
                addVerticesToEdges(edges, remainNum, 1); // Add a single vertex to the first remainNum edges...
            }
        }
    else
        {
        addVerticesToEdges(edges, addCount, 1); // Add a single vertex to the first addNum edges...
        }

    PK_MEMORY_free(edges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hopen.He                        03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeProfilesWithUniformVertexCount(PK_BODY_t const* bodies, size_t nBodies)
    {
    if (nullptr == bodies || nBodies < 2)
        return;

    int maxVertex = 0;

    for (size_t iBody = 0; iBody < nBodies; iBody++)
        {
        int nVertex = 0;

        if (SUCCESS == PK_BODY_ask_vertices(bodies[iBody], &nVertex, nullptr) && nVertex > maxVertex)
            maxVertex = nVertex;
        }

    if (0 == maxVertex)
        return;

    for (size_t iBody = 0; iBody < nBodies; iBody++)
        addProfileVertices(bodies[iBody], maxVertex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6385) // static analysis thinks we can go off the end of profileBodies
BentleyStatus PSolidGeom::BodyFromLoft(IBRepEntityPtr& out, CurveVectorPtr* profiles, size_t nProfiles, CurveVectorPtr* guides, size_t nGuides, bool periodic, uint32_t nodeId)
    {
    // NOTE: Parity/Union regions not currently supported...
    if (nullptr == profiles || 0 == nProfiles)
        return ERROR;

    DPoint3d    origin;

    if (!profiles[0]->GetStartPoint (origin))
        return ERROR;

    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    Transform   solidToDgn, dgnToSolid;

    PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &origin);

    PK_BODY_t*      profileBodies = (PK_BODY_t*) _alloca (nProfiles * sizeof (PK_BODY_t));
    PK_VERTEX_t*    startVertices = (PK_VERTEX_t*) _alloca (nProfiles * sizeof (PK_VERTEX_t));

    memset (profileBodies, 0, nProfiles * sizeof (PK_BODY_t));

    BentleyStatus   status = ERROR;

    for (size_t iProfile = 0; iProfile < nProfiles; iProfile++)
        {
        bool coverClosed = (0 == iProfile || (!periodic && iProfile+1 == nProfiles)); // NOTE: Only end caps may be sheet bodies...

        // Degenerate point profile is allowed for first or last profile...
        if (coverClosed && SUCCESS == (status = degeneratePointFromCurveVector(profileBodies[iProfile], *profiles[iProfile], dgnToSolid)))
            {
            startVertices[iProfile] = PK_ENTITY_null;
            continue;
            }

        if (SUCCESS != (status = PSolidGeom::BodyFromCurveVector (profileBodies[iProfile], &startVertices[iProfile], *profiles[iProfile], dgnToSolid, coverClosed)))
            break;
        }

    if (SUCCESS == status)
        {
        PK_BODY_t*   guideBodies = (nGuides ? (PK_BODY_t*) _alloca (nGuides * sizeof (PK_BODY_t)) : NULL);

        if (nGuides)
            {
            memset (guideBodies, 0, nGuides * sizeof (PK_BODY_t));

            for (size_t iGuide = 0; iGuide < nGuides; iGuide++)
                PSolidGeom::BodyFromCurveVector (guideBodies[iGuide], NULL, *guides[iGuide], dgnToSolid, false);
            }

        PK_BODY_t   bodyTag = PK_ENTITY_null;

        makeProfilesWithUniformVertexCount(profileBodies, nProfiles);

        if (SUCCESS == (status = PSolidGeom::BodyFromLoft (bodyTag, profileBodies, startVertices, nProfiles, guideBodies, nGuides, periodic)))
            {
            if (nodeId)
                PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

            out = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn, true);
            }

        PK_ENTITY_delete ((int) nGuides, guideBodies);
        }

    PK_ENTITY_delete ((int) nProfiles, profileBodies);

    return status;
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidGeom::BodyFromSweep(IBRepEntityPtr& out, CurveVectorCR profile, CurveVectorCR path, bool alignParallel, bool selfRepair, bool createSheet, DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint, uint32_t nodeId)
    {
    DPoint3d    origin;

    if (!path.GetStartPoint (origin))
        return ERROR;

    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    Transform   solidToDgn, dgnToSolid;

    PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &origin);

    PK_BODY_t   pathTag = PK_ENTITY_null;
    PK_VERTEX_t pathVertexTag = PK_ENTITY_null;

    if (SUCCESS != PSolidGeom::BodyFromCurveVector (pathTag, &pathVertexTag, path, dgnToSolid, false))
        return ERROR;

    PK_BODY_t   profileTag = PK_ENTITY_null;

    if (SUCCESS != PSolidGeom::BodyFromCurveVector (profileTag, NULL, profile, dgnToSolid, profile.IsAnyRegionType () && !createSheet))
        {
        PK_ENTITY_delete (1, &pathTag);

        return ERROR;
        }

    DVec3d      unitLockDir;

    if (lockDirection)
        {
        unitLockDir = *lockDirection;
        dgnToSolid.MultiplyMatrixOnly (unitLockDir);
        unitLockDir.Normalize ();
        }

    DPoint3d    solidScalePoint;
    if (NULL != scalePoint)
        dgnToSolid.Multiply (solidScalePoint, *scalePoint);

    BentleyStatus   status;
    PK_BODY_t       bodyTag = PK_ENTITY_null;

    if (SUCCESS == (status = PSolidGeom::BodyFromSweep (bodyTag, profileTag, pathTag, pathVertexTag, alignParallel, selfRepair, lockDirection ? &unitLockDir : NULL, twistAngle, scale, NULL == scalePoint ? NULL : &solidScalePoint)))
        {
        if (nodeId)
            PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

        out = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn, true);
        }

    PK_ENTITY_delete (1, &pathTag);
    PK_ENTITY_delete (1, &profileTag);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidGeom::BodyFromExtrusionToBody(IBRepEntityPtr& target, IBRepEntityCR extrudeTo, IBRepEntityCR profile, bool reverseDirection, uint32_t nodeId)
    {
    PK_ENTITY_t extrudeToTag = 0, profileTag = 0, extrusionTag = 0;
    Transform   invTargetTransform, profileTransform;
    DVec3d      direction;

    if (0 == (extrudeToTag = PSolidUtil::GetEntityTag (extrudeTo)) ||
        0 == (profileTag = PSolidUtil::GetEntityTag (profile)))
        return ERROR;

    PK_ENTITY_copy (profileTag, &profileTag);

    invTargetTransform.InverseOf (extrudeTo.GetEntityTransform ());
    profileTransform.InitProduct (invTargetTransform, profile.GetEntityTransform ());
    PSolidUtil::TransformBody (profileTag, profileTransform);

    bvector <PK_FACE_t> profileFaces;

    if (SUCCESS != PSolidTopo::GetBodyFaces (profileFaces, profileTag) ||
        SUCCESS != PSolidUtil::GetPlanarFaceData (NULL, &direction, profileFaces.front()))
        return ERROR;

    direction.Normalize();

    if (reverseDirection)
        direction.Negate();

    PK_BODY_extrude_o_t     options;
    PK_TOPOL_track_r_t      tracking;
    PK_TOPOL_local_r_t      results;
    PK_VECTOR1_t            path;
    int                     pkStatus;

    PK_BODY_extrude_o_m (options);
    memset (&tracking, 0, sizeof (tracking));
    memset (&results, 0, sizeof (results));

    options.end_bound.bound = PK_bound_body_c;
    options.end_bound.forward = PK_LOGICAL_true;
    options.allow_disjoint = PK_LOGICAL_true;
    options.end_bound.entity = extrudeToTag;

    direction.GetComponents (path.coord[0], path.coord[1], path.coord[2]);

    BentleyStatus status = (SUCCESS == (pkStatus = PK_BODY_extrude (profileTag, path, &options, &extrusionTag, &tracking, &results))) ? SUCCESS : ERROR;

    if (SUCCESS == status)
        {
        if (nodeId)
            PSolidTopoId::AddNodeIdAttributes (extrusionTag, nodeId, true);

        target = PSolidUtil::CreateNewEntity (extrusionTag, extrudeTo.GetEntityTransform (), true);
        }

    PK_TOPOL_track_r_f (&tracking);
    PK_TOPOL_local_r_f (&results);

    return status;
    }



