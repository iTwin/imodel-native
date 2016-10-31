/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidCreate.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/11
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr PSolidUtil::GetAsCurvePrimitive (PK_CURVE_t curve, PK_INTERVAL_t interval, bool reverseDirection)
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

            if (SUCCESS != PSolidUtil::CreateMSBsplineCurveFromCurve (bCurve, curve, interval, reverseDirection))
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
BentleyStatus   PSolidUtil::EdgeToCurvePrimitive (ICurvePrimitivePtr& curvePrimitive, PK_EDGE_t edgeTag)
    {
    PK_CURVE_t      curveTag = 0;
    PK_LOGICAL_t    orientation;
    PK_INTERVAL_t   interval;

    if (SUCCESS != PK_EDGE_ask_oriented_curve (edgeTag, &curveTag, &orientation) ||
        SUCCESS != PK_EDGE_find_interval (edgeTag, &interval))
        return ERROR;

    curvePrimitive = PSolidUtil::GetAsCurvePrimitive (curveTag, interval, !orientation);
    
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
            primitive = PSolidUtil::GetAsCurvePrimitive (curveTag, interval, !orientation);
        else if (SUCCESS == PK_FIN_is_positive (fins[iFin], &isPositiveFin) &&
                 SUCCESS == PK_EDGE_ask_oriented_curve (edgeTag, &curveTag, &orientation) &&
                 SUCCESS == PK_EDGE_find_interval (edgeTag, &interval))
            primitive = PSolidUtil::GetAsCurvePrimitive (curveTag, interval, orientation != isPositiveFin);

        if (!primitive.IsValid ())
            continue;

        if (NULL != idMap)
            {
            FaceId  faceId;

            if (SUCCESS == PSolidUtil::IdFromEntity (faceId, edgeTag, true))
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
CurveVectorPtr PSolidUtil::PlanarFaceToCurveVector(PK_FACE_t faceTag, EdgeToCurveIdMap const* idMap)
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
CurveVectorPtr  PSolidUtil::PlanarSheetBodyToCurveVector (IBRepEntityCR entity)
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

    CurveVectorPtr  curves = PlanarFaceToCurveVector (faceTag);

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

    baseCurve->push_back (PSolidUtil::GetAsCurvePrimitive (sfSwept.curve, interval, false));

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
ISolidPrimitivePtr PSolidUtil::FaceToSolidPrimitive(PK_FACE_t faceTag, CurveVectorPtr* uvBoundaries)
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

    if (SUCCESS != PSolidUtil::EdgeToCurvePrimitive (primitive, edgeTag))
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
BentleyStatus   PSolidUtil::BodyToCurveVectors (bvector<CurveVectorPtr>& curves, IBRepEntityCR entity, EdgeToCurveIdMap const* idMap)
    {
    int         nFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_BODY_ask_faces (PSolidUtil::GetEntityTag (entity), &nFaces, &faces);

    for (int i=0; i<nFaces; i++)
        {
        CurveVectorPtr  curveVector = PSolidUtil::PlanarFaceToCurveVector (faces[i], idMap);

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
CurveVectorPtr  PSolidUtil::WireBodyToCurveVector (IBRepEntityCR entity)
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





