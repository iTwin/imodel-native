/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidCreate.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

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
    PK_CURVE_t      curveTag = 0;
    PK_LOGICAL_t    orientation;
    PK_INTERVAL_t   interval;

    if (SUCCESS != PK_EDGE_ask_oriented_curve (edgeTag, &curveTag, &orientation) ||
        SUCCESS != PK_EDGE_find_interval (edgeTag, &interval))
        return ERROR;

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
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr PSolidGeom::PlanarFaceToCurveVector(PK_FACE_t faceTag, EdgeToCurveIdMap const* idMap)
    {
    PK_SURF_t       surfaceTag;
    PK_CLASS_t      surfaceClass;
    int             nLoops = 0;
    PK_LOOP_t*      loops = NULL;
    PK_LOGICAL_t    orientation;

    if (SUCCESS != PK_FACE_ask_oriented_surf (faceTag, &surfaceTag, &orientation) ||
        SUCCESS != PK_ENTITY_ask_class (surfaceTag, &surfaceClass) || surfaceClass != PK_CLASS_plane ||
        SUCCESS != PK_FACE_ask_loops (faceTag, &nLoops, &loops) || nLoops < 1)
        return NULL;

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
            status = PSolidUtil::Boolean (NULL, NULL, PK_boolean_unite, false, bodyTag, &regions[0], (int) regions.size (), PKI_BOOLEAN_OPTION_AllowDisjoint);

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
            status = PSolidUtil::Boolean (NULL, NULL, coverClosed ? PK_boolean_subtract : PK_boolean_unite, !coverClosed, bodyTag, &holes[0], (int) holes.size (), PKI_BOOLEAN_OPTION_AllowDisjoint);

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




