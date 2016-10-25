/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::GetBodyFaces (bvector<PK_FACE_t>& faces, PK_BODY_t body)
    {
    int         faceCount = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_faces (body, &faceCount, &pFaceTagArray))
        return ERROR;

    faces.resize (faceCount);
    for (int i=0; i<faceCount; i++)
        faces[i] = pFaceTagArray[i];

    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVector::BoundaryType getBoundaryType (CurveVectorCR curveVector)
    {
    double      area;
    DPoint3d    centroid;

    curveVector.CentroidAreaXY (centroid, area);

    return area < 0.0 ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr PSolidUtil::FaceToUVCurveVector(PK_FACE_t faceTag, PK_UVBOX_t* uvBox, bool splineParameterization)
    {
    PK_FACE_output_surf_trimmed_o_t options;

    PK_FACE_output_surf_trimmed_o_m (options);

    options.want_geoms  = PK_LOGICAL_true;
    options.want_topols = PK_LOGICAL_false;
    options.trim_surf   = splineParameterization ? PK_FACE_trim_surf_bsurf_c : PK_FACE_trim_surf_own_c;

    PK_SURF_t           surfaceTag;
    PK_LOGICAL_t        sense;
    PK_SURF_trim_data_t trimData;
    PK_GEOM_t*          geometryP = NULL;
    PK_INTERVAL_t*      intervalP = NULL;
    PK_TOPOL_t*         topologyP = NULL;
    
    if (SUCCESS != PK_FACE_output_surf_trimmed (faceTag, &options, &surfaceTag, &sense, &trimData, &geometryP, &intervalP, &topologyP))
        return nullptr; // This can occur on some faces.... Error code is typically 1008 (failed to produce trimmed surface).

    int                 currentLoopTag = -1;
    CurveVectorPtr      currentLoop, curveVector;

    for (size_t i=0; i < (size_t) trimData.n_spcurves; i++)
        {
        if (trimData.trim_loop[i] != currentLoopTag && currentLoop.IsValid())
            {
            if (!curveVector.IsValid())
                curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
            
            currentLoop->SetBoundaryType (getBoundaryType (*currentLoop));
            curveVector->Add (currentLoop);
            currentLoop = NULL;
            }

        currentLoopTag = trimData.trim_loop[i];

        PK_SPCURVE_sf_t     sfSpCurve;
        PK_BCURVE_t         psBCurveTag;
        PK_LOGICAL_t        isExact;
        MSBsplineCurve      bCurve;

        if (SUCCESS != PK_SPCURVE_ask (trimData.spcurves[i], &sfSpCurve) ||
            SUCCESS != PK_CURVE_make_bcurve (sfSpCurve.curve, trimData.intervals[i], PK_LOGICAL_false, PK_LOGICAL_false, 1.0E-6, &psBCurveTag, &isExact) ||
            SUCCESS != PSolidUtil::CreateMSBsplineCurveFromBCurve (bCurve, psBCurveTag))
            {
            BeAssert (false);
            curveVector = NULL;
            currentLoop = NULL;
            break;
            }

        if (!currentLoop.IsValid())
            currentLoop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

        currentLoop->Add (ICurvePrimitive::CreateBsplineCurve (bCurve));

        PK_ENTITY_delete (1, &psBCurveTag);
        }

    if (currentLoop.IsValid())
        {
        currentLoop->SetBoundaryType (getBoundaryType (*currentLoop));

        if (curveVector.IsValid())
            curveVector->Add (currentLoop);
        else
            curveVector = currentLoop;
        }

    Transform rangeTransform;

    if (NULL != uvBox && 
        curveVector.IsValid() &&
        Transform::TryRangeMapping (DRange2d::From (uvBox->param[0], uvBox->param[1], uvBox->param[2], uvBox->param[3]), DRange2d::From (0.0, 0.0, 1.0, 1.0), rangeTransform))
        curveVector->TransformInPlace (rangeTransform);

    if (trimData.n_spcurves)
        {
        PK_ENTITY_delete (trimData.n_spcurves, trimData.spcurves);

        PK_MEMORY_free (trimData.spcurves);
        PK_MEMORY_free (trimData.intervals);
        PK_MEMORY_free (trimData.trim_loop);
        PK_MEMORY_free (trimData.trim_set);
        }

    if (geometryP)
        {
        PK_ENTITY_delete (trimData.n_spcurves, geometryP);
        PK_MEMORY_free (geometryP);
        }

    if (intervalP)
        PK_MEMORY_free (intervalP);

    return curveVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr  planarFaceLoopToCurveVector (PK_LOOP_t loopTag, EdgeToCurveIdMap const* idMap)
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
* @bsimethod                                                    Brien.Bastings  02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::GetCurveOfEdge
(
PK_CURVE_t&     curveTagOut,
double*         startParamP,
double*         endParamP,
bool*           reversedP,
PK_EDGE_t       edgeTagIn
)
    {
    PK_FIN_t        fin;
    PK_INTERVAL_t   interval;
    PK_LOGICAL_t    sense = PK_LOGICAL_true;

    curveTagOut = NULTAG;

    if (SUCCESS == PK_EDGE_ask_oriented_curve (edgeTagIn, &curveTagOut, &sense) && curveTagOut != NULTAG)
        {
        if (startParamP || endParamP)
            {
            if (SUCCESS != PK_EDGE_find_interval (edgeTagIn, &interval))
                PK_CURVE_ask_interval (curveTagOut, &interval);
            }
        }
    else if (SUCCESS == PK_EDGE_ask_first_fin (edgeTagIn, &fin))
        {
        PK_FIN_ask_oriented_curve (fin, &curveTagOut, &sense);

        if ((startParamP || endParamP) && curveTagOut != NULTAG)
            PK_FIN_find_interval (fin, &interval);
        }

    if (startParamP)
        *startParamP = interval.value[sense == PK_LOGICAL_true ? 0 : 1];

    if (endParamP)
        *endParamP = interval.value[sense == PK_LOGICAL_true ? 1 : 0];

    if (reversedP)
        *reversedP = (sense == PK_LOGICAL_true ? false : true);

    return (curveTagOut != NULTAG ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::HasCurvedFacesOrEdges (PK_BODY_t entity)
    {
    if (!entity)
        return false;

    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_BODY_ask_faces (entity, &numFaces, &faces);

    for (int i=0; i < numFaces; i++)
        {
        PK_SURF_t       surfaceTag;
        PK_LOGICAL_t    orientation;
        bool            isPlanar = false;

        if (SUCCESS == PK_FACE_ask_oriented_surf (faces[i], &surfaceTag, &orientation))
            {
            PK_CLASS_t  entityClass;

            PK_ENTITY_ask_class (surfaceTag, &entityClass);

            switch (entityClass)
                {
                case PK_CLASS_plane:
                case PK_CLASS_circle:
                case PK_CLASS_ellipse:
                    {
                    isPlanar = true;
                    break;
                    }
                }
            }

        if (!isPlanar)
            {
            PK_MEMORY_free (faces);

            return true;
            }
        }

    PK_MEMORY_free (faces);

    int         numEdges = 0;
    PK_EDGE_t*  edges = NULL;

    PK_BODY_ask_edges (entity, &numEdges, &edges);

    for (int i=0; i < numEdges; i++)
        {
        PK_CURVE_t      curveTag;
        bool            isStraight = false;

        if (SUCCESS == PSolidUtil::GetCurveOfEdge (curveTag, NULL, NULL, NULL, edges[i]))
            {
            PK_CLASS_t  entityClass;

            PK_ENTITY_ask_class (curveTag, &entityClass);
            isStraight = (PK_CLASS_line == entityClass);
            }

        if (!isStraight)
            {
            PK_MEMORY_free (edges);

            return true;
            }
        }

    PK_MEMORY_free (edges);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidUtil::ExtractStartAndSweepFromInterval (double& start, double& sweep, PK_INTERVAL_t const& interval, bool reverse)
    {
    if (interval.value[0] < interval.value[1])
        {
        start = interval.value[0];
        sweep = interval.value[1] - start;
        }
    else
        {
        start = interval.value[1];
        sweep = interval.value[0] - start;
        }

    if (sweep < 0.0)
        sweep += msGeomConst_2pi;

    if (reverse)
        {
        start += sweep;
        sweep =- sweep;
        }
    }

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
