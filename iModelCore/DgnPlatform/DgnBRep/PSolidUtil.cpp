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
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::HasCurvedFaceOrEdge (PK_BODY_t entity)
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

        if (SUCCESS == PSolidTopo::GetCurveOfEdge (curveTag, NULL, NULL, NULL, edges[i]))
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
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::HasOnlyPlanarFaces (PK_BODY_t entity)
    {
    if (!entity)
        return true;

    bool        isPlanar = true;
    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_BODY_ask_faces (entity, &numFaces, &faces);

    for (int i=0; i < numFaces && isPlanar; i++)
        {
        PK_SURF_t       surfaceTag;
        PK_LOGICAL_t    orientation;

        if (SUCCESS == PK_FACE_ask_oriented_surf (faces[i], &surfaceTag, &orientation))
            {
            PK_CLASS_t  entityClass;

            PK_ENTITY_ask_class (surfaceTag, &entityClass);

            switch (entityClass)
                {
                case PK_CLASS_plane:
                case PK_CLASS_circle:
                case PK_CLASS_ellipse:
                    break;

                default:
                    isPlanar = false;
                    break;
                }
            }
        }

    PK_MEMORY_free (faces);

    return isPlanar;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::IsSmoothEdge (PK_ENTITY_t edge)
    {
    PK_LOGICAL_t smooth;
    static double smoothTolerance = 1.0e-5;

    return (SUCCESS == PK_EDGE_is_smooth (edge, smoothTolerance, &smooth)) && smooth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::GetPlanarFaceData (DPoint3dP pointOut, DVec3dP normalOut, PK_FACE_t faceTag)
    {
    if (!faceTag)
        return ERROR;

    PK_SURF_t       surfaceTag = 0;
    PK_LOGICAL_t    orientation;

    PK_FACE_ask_oriented_surf (faceTag, &surfaceTag, &orientation);

    PK_UVBOX_t      uvBox;

    if (!surfaceTag || SUCCESS != PK_FACE_find_uvbox (faceTag, &uvBox))
        return ERROR;

    PK_UV_t         uv;

    uv.param[0] = (uvBox.param[2]+uvBox.param[0])/2.0;
    uv.param[1] = (uvBox.param[3]+uvBox.param[1])/2.0;

    PK_LOGICAL_t    triangular = PK_LOGICAL_false;
    PK_VECTOR_t     point, normal;

    PK_SURF_eval_with_normal (surfaceTag, uv, 0, 0, triangular, &point, &normal);

    if (pointOut)
        {
        pointOut->x = point.coord[0];
        pointOut->y = point.coord[1];
        pointOut->z = point.coord[2];
        }

    if (normalOut)
        {
        normalOut->x = normal.coord[0];
        normalOut->y = normal.coord[1];
        normalOut->z = normal.coord[2];

        if (orientation == PK_LOGICAL_false)
            normalOut->Negate ();
        }

    PK_CLASS_t      entityClass;

    PK_ENTITY_ask_class (surfaceTag, &entityClass);

    switch (entityClass)
        {
        case PK_CLASS_plane:
        case PK_CLASS_circle:
        case PK_CLASS_ellipse:
            return SUCCESS;

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
PK_BODY_t PSolidUtil::GetBodyForEntity (PK_ENTITY_t entityTag)
    {
    PK_BODY_t   bodyTag = PK_ENTITY_null;
    PK_CLASS_t  entityClass = 0;

    PK_ENTITY_ask_class (entityTag, &entityClass);

    switch (entityClass)
        {
        case PK_CLASS_edge:
            PK_EDGE_ask_body (entityTag, &bodyTag);
            break;

        case PK_CLASS_face:
            PK_FACE_ask_body (entityTag, &bodyTag);
            break;

        case PK_CLASS_vertex:
            PK_VERTEX_ask_body (entityTag, &bodyTag);
            break;

        case PK_CLASS_body:
            bodyTag = entityTag;
            break;
        }

    return bodyTag;
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
CurveVectorPtr PSolidGeom::FaceToUVCurveVector(PK_FACE_t faceTag, PK_UVBOX_t* uvBox, bool splineParameterization)
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
            SUCCESS != PSolidGeom::CreateMSBsplineCurveFromBCurve (bCurve, psBCurveTag))
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
* @bsimethod                                                    Brien.Bastings  07/97
+---------------+---------------+---------------+---------------+---------------+------*/
static  int     pki_get_body_box
(
DPoint3d        *pLo,
DPoint3d        *pHi,
int             entityTagIn
)
    {
    int         status = ERROR;
    PK_BOX_t    entityBox;

    if (PK_ERROR_no_errors == PK_TOPOL_find_box (entityTagIn, &entityBox))
        {
        pLo->x = entityBox.coord[0];
        pLo->y = entityBox.coord[1];
        pLo->z = entityBox.coord[2];
        pHi->x = entityBox.coord[3];
        pHi->y = entityBox.coord[4];
        pHi->z = entityBox.coord[5];

        status = SUCCESS;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::GetEntityRange (DRange3dR range, PK_TOPOL_t entity)
    {
    return (SUCCESS == pki_get_body_box(&range.low, &range.high, entity) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void copyToSheet (PK_BODY_t* outBody, PK_BODY_t inBody, bool setHiddenEdges)
    {
    PK_ENTITY_copy (inBody, outBody);
    PSolidUtil::ConvertSolidBodyToSheet (*outBody);

    if (setHiddenEdges)
        {
        int                     nEdges;
        PK_EDGE_t*              edges;
        bool                    hidden;

        PK_BODY_ask_edges (*outBody, &nEdges, &edges);

        for (int i=0; i<nEdges; i++)
            if (SUCCESS != PSolidAttrib::GetHiddenAttribute (hidden, edges[i]))
                PSolidAttrib::SetHiddenAttribute (edges[i], false);

        PK_MEMORY_free (edges);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void setHiddenEdgeAttributes (PK_BODY_t body)
    {
    int         nEdges;
    PK_EDGE_t*  edges;
    bool        isHidden;

    PK_BODY_ask_edges (body, &nEdges, &edges);

    for (int i=0; i<nEdges; i++)
        {
        if (SUCCESS == PSolidAttrib::GetHiddenAttribute (isHidden, edges[i]))
            {
            if (!isHidden)
                PSolidAttrib::DeleteHiddenAttribute (edges[i]);
            }
        else
            {
            PSolidAttrib::SetHiddenAttribute (edges[i], TRUE);
            }
        }

    PK_MEMORY_free (edges);
    }

static void clipBodyByClipVector (bvector<PK_BODY_t>& output, bool& clipped, size_t& errorCount, PK_BODY_t body,  TransformCR clipToBody, ClipVectorCR clip, size_t nextPrimitiveIndex);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void clipBodyByPlanes
(
bvector<PK_BODY_t>&         output,
bool&                       clipped,
size_t&                     errorCount, 
PK_BODY_t                   body,
ClipPlaneCP                 clipPlane,
ClipPlaneCP                 endPlane,
TransformCR                 clipToBody,
ClipVectorCR                clipVector, 
size_t                      nextPrimitiveIndex
)
    {
    if (clipPlane >= endPlane)
        {
        clipBodyByClipVector (output, clipped, errorCount, body, clipToBody, clipVector, nextPrimitiveIndex);
        return;
        }
    DPlane3d            bodyPlane;
    DPoint3d            bodyCorners[8];
    DRange3d            range;

    bodyPlane.normal = clipPlane->GetNormal();
    bodyPlane.origin.Scale (*(&clipPlane->GetNormal()), clipPlane->GetDistance());

    clipToBody.Multiply (bodyPlane, bodyPlane);
    bodyPlane.Normalize ();

    double          planeDistance = bodyPlane.normal.DotProduct (bodyPlane.origin), minDistance=0.0, maxDistance=0.0;

    PSolidUtil::GetEntityRange (range, body);
    range.Get8Corners (bodyCorners);

    for (size_t i=0; i<8; i++)
        {
        double          cornerDistance = bodyPlane.normal.DotProduct (bodyCorners[i]) - planeDistance;

        if (0 == i)
            {
            minDistance = maxDistance = cornerDistance;
            }
        else
            {
            if (cornerDistance < minDistance)
                minDistance = cornerDistance;
            else if (cornerDistance > maxDistance)
                maxDistance = cornerDistance;
            }
        }

    if (minDistance  > -1.0E-8)     // Entire body in front.
        {
        clipBodyByPlanes (output, clipped, errorCount, body, clipPlane+1, endPlane, clipToBody, clipVector, nextPrimitiveIndex);
        return;
        }

    if (maxDistance < 0.0)      // Entire body behind plane (we're done).
        {
        clipped = true;
        return;
        }

    PK_PLANE_sf_t       planeSurfaceSF;
    PK_PLANE_t          planeSurface;
    RotMatrix           planeMatrix;

    planeMatrix.InitFrom1Vector (bodyPlane.normal, 2, false);

    planeSurfaceSF.basis_set.location.coord[0] = bodyPlane.origin.x;
    planeSurfaceSF.basis_set.location.coord[1] = bodyPlane.origin.y;
    planeSurfaceSF.basis_set.location.coord[2] = bodyPlane.origin.z;

    planeSurfaceSF.basis_set.axis.coord[0] = bodyPlane.normal.x;
    planeSurfaceSF.basis_set.axis.coord[1] = bodyPlane.normal.y;
    planeSurfaceSF.basis_set.axis.coord[2] = bodyPlane.normal.z;
    // copy column 0 (x vector) 
    planeSurfaceSF.basis_set.ref_direction.coord[0] = planeMatrix.form3d[0][0];
    planeSurfaceSF.basis_set.ref_direction.coord[1] = planeMatrix.form3d[1][0];
    planeSurfaceSF.basis_set.ref_direction.coord[2] = planeMatrix.form3d[2][0];

    PK_BODY_section_o_t     options;
    PK_section_r_t          results;

    PK_BODY_section_o_m (options);
    memset (&results, 0, sizeof (results));

   if (SUCCESS != PK_PLANE_create (&planeSurfaceSF, &planeSurface))
        {    
        clipBodyByPlanes (output, clipped, ++errorCount, body, clipPlane+1, endPlane, clipToBody, clipVector, nextPrimitiveIndex);
        return;
        }

    PK_BODY_t   sheetBody;

    copyToSheet (&sheetBody, body, !clipPlane->IsVisible());

    if (SUCCESS == PK_BODY_section_with_surf (sheetBody, planeSurface, &options, &results))
        {
        if (0 != results.back_bodies.length)
            clipped = true;

        for (int i=0; i<results.front_bodies.length; i++)
            {
            if (!clipPlane->IsVisible())
                setHiddenEdgeAttributes (results.front_bodies.array[i]);

            clipBodyByPlanes (output, clipped, errorCount, results.front_bodies.array[i], clipPlane+1, endPlane, clipToBody, clipVector, nextPrimitiveIndex);
            }

        PK_ENTITY_delete (results.front_bodies.length, results.front_bodies.array);
        PK_ENTITY_delete (results.back_bodies.length, results.back_bodies.array);
        PK_section_r_f (&results);
        }
    else
        {
        clipBodyByPlanes (output, clipped, ++errorCount, body, clipPlane+1, endPlane, clipToBody, clipVector, nextPrimitiveIndex);
        PK_ENTITY_delete (1, &sheetBody);
        }

    PK_ENTITY_delete (1, &planeSurface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void computeZRangeFromBody (double& zMin, double& zMax, PK_BODY_t entityTag, TransformCR solidToClip)
    {
    zMin = -1.0e20;
    zMax = 1.0e20;

    PK_BOX_t    box;

    if (PK_ERROR_no_errors != PK_TOPOL_find_box (entityTag, &box))
        return;

    DRange3d    range;

    range.InitFrom (box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);
    solidToClip.Multiply (range, range);
 
    // Avoid coincident geometry and ensure a minimum path length for sweep...
    double      s_uorClearFactor = .5;

    zMin = range.low.z  - s_uorClearFactor;
    zMax = range.high.z + s_uorClearFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromSingleParallelClip (PK_BODY_t& clipBody, ClipPrimitiveCR clip, PK_BODY_t entityToClip, TransformCR clipToSolid, TransformCR solidToClip)
    {
    GPArrayCP   gpa;

    if (NULL == (gpa = clip.GetGPA (false)))
        return ERROR;

    double      zMin, zMax;

    computeZRangeFromBody (zMin, zMax, entityToClip, solidToClip);

    if (clip.ClipZLow() || clip.ClipZHigh() && (zMin < clip.GetZLow() || zMax > clip.GetZHigh()))
        {
        if (clip.ClipZLow ())
            zMin = clip.GetZLow ();

        if (clip.ClipZHigh())
            zMax = clip.GetZHigh();
        }

    Transform   clipZTranslation = Transform::From (0.0, 0.0, zMin), compound;
    DPoint3d    clipSweep = DPoint3d::From (0.0, 0.0, zMax - zMin), solidSweep;

    compound.InitProduct (clipToSolid, clipZTranslation);
                                               
    if (SUCCESS != PSolidGeom::BodyFromGPA (&clipBody, NULL, gpa, compound, true))
        return ERROR;
    
    clipToSolid.MultiplyMatrixOnly (solidSweep, clipSweep);

    // Need a minimum path length for sweep
    if (solidSweep.Magnitude () < mgds_fc_epsilon)
        solidSweep.ScaleToLength (mgds_fc_epsilon);

    PK_VECTOR_t path;

    path.coord[0] = solidSweep.x;
    path.coord[1] = solidSweep.y;
    path.coord[2] = solidSweep.z;

    int               nLaterals;
    PK_local_check_t  localCheck;

    if (SUCCESS != PK_BODY_sweep (clipBody, path, false, &nLaterals, NULL, NULL, &localCheck))
        {
        PK_ENTITY_delete (1, &clipBody);

        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void clipBodyByShape (bvector<PK_BODY_t>& output, bool& clipped, size_t& errorCount, PK_BODY_t body, ClipPrimitiveCR clipPrimitive, TransformCR clipToBody, ClipVectorCR clipVector, size_t nextPrimitiveIndex)
    {                     
    Transform           bodyToShape, shapeToBody = (NULL != clipPrimitive.GetTransformFromClip()) ? Transform::FromProduct (clipToBody, *clipPrimitive.GetTransformFromClip()) : clipToBody;
    PK_BODY_t           clipBody;

    bodyToShape.InverseOf (shapeToBody);
    if (SUCCESS != bodyFromSingleParallelClip (clipBody, clipPrimitive, body, shapeToBody, bodyToShape))
        {
        clipBodyByClipVector (output, clipped, ++errorCount, body, clipToBody, clipVector, nextPrimitiveIndex);
        return;
        }

#ifdef NOTYET
    if (mdlSolid_hasHiddenFace ((BODY*) body))          // Changed to only do the (unreliable) clipBodyWithHiddenEntities if faces (unification) are present.
        {                                               // If only hidden entities are present (as arises from previous clipping, these will be handled by propagating hidden edge attribute below.
        PK_BODY_t   clipBodyCopy, *clippedBodies = 0;
        StatusInt   status;
        int         numClippedBodies = 0;

        PK_ENTITY_copy (clipBody, &clipBodyCopy);

        if (SUCCESS == (status = clipBodyWithHiddenEntities (&clippedBodies, &numClippedBodies, clipped, body, clipBodyCopy, clip->hdr.flags.outside != 0, clip->hdr.flags.dontDisplayCut)))
            {
            for (int i = 0; i < numClippedBodies; i++)
                clipBodyByClipDescr (clippedBodies[i], clip->hdr.next, clipToBody, outputFunction, userArg, clipped);

            PK_ENTITY_delete (1, &clipBody);
            PK_ENTITY_delete (numClippedBodies, clippedBodies);
            PK_MEMORY_free (clippedBodies);
            return;
            }

        PK_ENTITY_delete (1, &clipBodyCopy);
        }
#endif

    PK_BODY_t               sheetBody;
    PK_BODY_boolean_o_t     options;
    PK_boolean_r_t          results;
    PK_TOPOL_track_r_t      tracking;

    PK_BODY_boolean_o_m (options);
    options.function        = clipPrimitive.IsMask() ? PK_boolean_subtract_c : PK_boolean_intersect_c;
    options.merge_imprinted = PK_LOGICAL_true;

    memset (&results, 0, sizeof (results));
    memset (&tracking, 0, sizeof (tracking));

    bool        dontDisplayCut = clipPrimitive.GetInvisible();

    copyToSheet (&sheetBody, body, dontDisplayCut);      
    if (dontDisplayCut)
        {
        int                     nEdges;
        PK_EDGE_t*              edges;
        bool                    hidden;

        PK_BODY_ask_edges (sheetBody, &nEdges, &edges);

        for (int i=0; i<nEdges; i++)
            if (SUCCESS != PSolidAttrib::GetHiddenAttribute (hidden, edges[i]))
                PSolidAttrib::SetHiddenAttribute (edges[i], false);

        PK_MEMORY_free (edges);
        }

#ifdef DEBUG_CLIP
    PK_PART_transmit_o_t transmitOptions;

    PK_PART_transmit_o_m (transmitOptions);
    PK_PART_transmit_u (1, &sheetBody, (PK_UCHAR_t const*) L"d:\\tmp\\SheetBody.xmt", &transmitOptions);
    PK_PART_transmit_u (1, &clipBody, (PK_UCHAR_t const*) L"d:\\tmp\\ClipBody.xmt", &transmitOptions);
#endif

    if (SUCCESS == PK_BODY_boolean_2 (sheetBody, 1, &clipBody, &options, &tracking, &results) && PK_boolean_result_failed_c != results.result)
        {
        if (0 == results.n_bodies ||  results.result != PK_boolean_result_no_clash_c)
            clipped = true;

#ifdef DEBUG_CLIP
        PK_PART_transmit_u (results.n_bodies, &results.bodies[0], (PK_UCHAR_t const*) L"d:\\tmp\\Result.xmt", &transmitOptions);
#endif
        for (int i=0; i<results.n_bodies; i++)
            {
            if (dontDisplayCut)
                setHiddenEdgeAttributes (results.bodies[i]);

            clipBodyByClipVector (output, clipped, errorCount, results.bodies[i], clipToBody, clipVector, nextPrimitiveIndex);
            }

        PK_ENTITY_delete (results.n_bodies, results.bodies);
        }
    else
        {
        PK_ENTITY_delete (1, &sheetBody);
        PK_ENTITY_delete (1, &clipBody);

        clipBodyByClipVector (output, clipped, ++errorCount, body, clipToBody, clipVector, nextPrimitiveIndex);
        }

    PK_boolean_r_f (&results);
    PK_TOPOL_track_r_f (&tracking);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void clipBodyByClipVector (bvector<PK_BODY_t>& output, bool& clipped, size_t& errorCount, PK_BODY_t body, TransformCR clipToBody, ClipVectorCR clipVector, size_t primitiveIndex)
    {
    if (primitiveIndex >= clipVector.size())
        {
        PK_BODY_t outputBody;

        PK_ENTITY_copy (body, &outputBody);
        output.push_back (outputBody);
        return;
        }

    ClipPrimitiveCP primitive = clipVector.at(primitiveIndex++).get();

    if (NULL == primitive)
        {
        BeAssert (false);       // Should never happen.
        return;
        }

    if (NULL != primitive->GetPolygon())
        return clipBodyByShape (output, clipped, errorCount, body, *primitive, clipToBody, clipVector, primitiveIndex);

    if (NULL == primitive->GetClipPlanes())
        {
        BeAssert (false); // Should never happen.
        return;
        }

    for (ConvexClipPlaneSetCR convexSet: *primitive->GetClipPlanes())
        clipBodyByPlanes (output, clipped, errorCount, body, &convexSet.front(), &convexSet.front() + convexSet.size(), clipToBody, clipVector, primitiveIndex);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::ClipCurveVector(bvector<CurveVectorPtr>& output, CurveVectorCR input, ClipVectorCR clipVector, TransformCP transformToDgn)
    {
    BentleyStatus       status;
    IBRepEntityPtr      entity;
    EdgeToCurveIdMap    idMap;

    if (SUCCESS != (status = PSolidGeom::BodyFromCurveVector (entity, input, transformToDgn, 0L, &idMap)))
        return status;

    bool                clipped = false;
    bvector <PK_BODY_t> outBodies;
    size_t              errorCount = 0;
    Transform           clipToBody;

    clipToBody.InverseOf (entity->GetEntityTransform());
    clipBodyByClipVector (outBodies, clipped, errorCount, PSolidUtil::GetEntityTag (*entity), clipToBody, clipVector, 0);

    if (!clipped)
        {
        output.push_back (input.Clone());
        }
    else
        {
        for (PK_BODY_t body: outBodies)
            {
            IBRepEntityPtr clippedEntity = PSolidUtil::CreateNewEntity(body, entity->GetEntityTransform(), false);

            PSolidGeom::BodyToCurveVectors (output, *clippedEntity, &idMap);
            }
        }

    if (!outBodies.empty())
        PK_ENTITY_delete ((int) outBodies.size(), &outBodies[0]);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::ClipBody(bvector<IBRepEntityPtr>& output, bool& clipped, IBRepEntityCR input, ClipVectorCR clipVector)
    {
    size_t              errorCount = 0;
    Transform           clipToBody;
    bvector <PK_BODY_t> outBodies;

    clipToBody.InverseOf(input.GetEntityTransform());
    clipBodyByClipVector(outBodies, clipped, errorCount, PSolidUtil::GetEntityTag(input), clipToBody, clipVector, 0); 

    for (PK_BODY_t body : outBodies)
        output.push_back(PSolidUtil::CreateNewEntity(body, input.GetEntityTransform()));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::MassProperties
(
double*         amountOut,
double*         peripheryOut,
DPoint3dP       centroidOut,
double          inertiaOut[3][3],
PK_BODY_t       bodyTag,
TransformCP     transform,
double          tolerance
)
    {
    if (tolerance > 100.0)
        tolerance = 100.0;
    else if (tolerance < 0.0)
        tolerance = 0.0;

    double      accuracy = 1.0 - (tolerance / 100.0);

    PK_TOPOL_eval_mass_props_o_t  propOptions;

    PK_TOPOL_eval_mass_props_o_m (propOptions);

    propOptions.mass = PK_mass_mass_c;

    if (centroidOut)
        propOptions.mass = PK_mass_c_of_g_c;

    if (inertiaOut)
        propOptions.mass = PK_mass_m_of_i_c;

    double      mass, amount, periphery, centroid[3], inertia[9];
    PK_BODY_t   copyBodyTag = PK_ENTITY_null;

    if (NULL != transform)
        {
        PK_TRANSF_t  transformTag = NULTAG;

        if (SUCCESS == PSolidUtil::CreateTransf (transformTag, *transform) && NULTAG != transformTag)
            {
            if (SUCCESS == PK_ENTITY_copy (bodyTag, &copyBodyTag))
                PSolidUtil::ApplyTransform (copyBodyTag, transformTag);

            PK_ENTITY_delete (1, &transformTag);
            }
        }

    if (SUCCESS != PK_TOPOL_eval_mass_props (1, PK_ENTITY_null != copyBodyTag ? &copyBodyTag : &bodyTag, accuracy, &propOptions, &amount, &mass, centroid, inertia, &periphery))
        {
        PK_ENTITY_delete (1, &copyBodyTag);

        return ERROR;
        }

    PK_ENTITY_delete (1, &copyBodyTag);

    if (amountOut)
        *amountOut = amount;

    if (peripheryOut)
        *peripheryOut = periphery;

    if (inertiaOut)
        memcpy ((double *) inertiaOut, inertia, sizeof (double) * 9);

    if (centroidOut)
        centroidOut->Init (centroid[0], centroid[1], centroid[2]);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::CheckBody (PK_BODY_t body, bool checkGeometry, bool checkTopology, bool checkSize)
    {
    int                 nFaults = 0;
    PK_check_fault_t    *faultsP = NULL;
    PK_BODY_check_o_t   options;

    if (PK_ENTITY_null == body)
        return ERROR;

    PK_BODY_check_o_m (options);

    options.max_faults = 0;

    if (!checkGeometry)
        {
        options.geom  = PK_check_geom_no_c;
        options.bgeom = PK_check_bgeom_no_c;
        }

    if (!checkTopology)
        {
        options.top_geo = PK_check_top_geo_no_c;
        options.fa_X    = PK_check_fa_X_no_c;
        options.loops   = PK_check_loops_no_c;
        options.fa_fa   = PK_check_fa_fa_no_c;
        options.sh      = PK_check_sh_no_c;
        options.corrupt = PK_check_corrupt_no_c;
        }

    if (!checkSize)
        {
        options.size_box = PK_check_size_box_no_c;
        }

    return (SUCCESS == PK_BODY_check (body, &options, &nFaults, &faultsP) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PSolidUtil::AreBodiesEqual (PK_BODY_t body1, PK_BODY_t body2, double tolerance, TransformCP deltaTransform1To2)
    {
    // Transforming the body can be expensive (particularly with bodies containing B-Splines)
    // so do some tests here to try to avoid it unless we are fairly certain the bodies are equal.

    int         nVertices1, nVertices2;
    PK_VERTEX_t *vertices1 = NULL, *vertices2 = NULL;

    PK_BODY_ask_vertices (body1, &nVertices1, &vertices1);
    PK_BODY_ask_vertices (body2, &nVertices2, &vertices2);

    bool        verticesEqual;

    if (true == (verticesEqual = (nVertices1 == nVertices2)))
        {
        for (int i=0; i<nVertices1 && verticesEqual; i++)
            {
            PK_POINT_t  point1Tag, point2Tag;
            DPoint3d    point1, point2;

            if (SUCCESS == PK_VERTEX_ask_point (vertices1[i], &point1Tag) &&
                SUCCESS == PK_VERTEX_ask_point (vertices2[i], &point2Tag) &&
                SUCCESS == PK_POINT_ask (point1Tag, (PK_POINT_sf_t*) &point1) &&
                SUCCESS == PK_POINT_ask (point2Tag, (PK_POINT_sf_t*) &point2))
                {
                if (NULL != deltaTransform1To2)
                    deltaTransform1To2->Multiply(point1);

                verticesEqual = point1.IsEqual (point2, tolerance);
                }
            }
        }

    PK_MEMORY_free (vertices1);
    PK_MEMORY_free (vertices2);

    if (!verticesEqual)
        return false;

    // The face test (evaluating and testing a point at each face) was added to address TRE# 281976 which had
    // two rotated symettric bodes that were not distinguisued correctly by PK_DEBUG_BODY_compare.
    int         nFaces1, nFaces2;
    PK_FACE_t   *faces1 = NULL, *faces2 = NULL;

    PK_BODY_ask_faces (body1, &nFaces1, &faces1);
    PK_BODY_ask_faces (body2, &nFaces2, &faces2);

    bool        facesEqual;

    if (true == (facesEqual = (nFaces1 == nFaces2)))
        {
        for (int i=0; i<nFaces1 && facesEqual; i++)
            {
            PK_SURF_t       surf1, surf2;
            PK_PARAM_sf_t   params1[2], params2[2];

            if (SUCCESS == PK_FACE_ask_surf (faces1[i], &surf1) &&
                SUCCESS == PK_FACE_ask_surf (faces2[i], &surf2) &&
                SUCCESS == PK_SURF_ask_params (surf1, params1) &&
                SUCCESS == PK_SURF_ask_params (surf2, params2))
                {
                PK_UV_t         uv1 = {(params1[0].range.value[0] + params1[0].range.value[1])/2.0, (params1[1].range.value[1] + params1[1].range.value[1])/2.0},
                                uv2 = {(params2[0].range.value[0] + params2[0].range.value[1])/2.0, (params2[1].range.value[1] + params2[1].range.value[1])/2.0};
                DPoint3d        p1[4], p2[4];

                if (SUCCESS == PK_SURF_eval (surf1, uv1, 1, 1, true, (PK_VECTOR_t*) p1) &&
                    SUCCESS == PK_SURF_eval (surf2, uv2, 1, 1, true, (PK_VECTOR_t*) p2))
                    {
                    if (NULL != deltaTransform1To2)
                        deltaTransform1To2->Multiply(p1[0]);

                    facesEqual = p1[0].IsEqual (p2[0], tolerance);

                    for (int j=1; j<3 && facesEqual; j++)
                        {
                        if (NULL != deltaTransform1To2)
                            deltaTransform1To2->MultiplyMatrixOnly(p1[j]);

                        facesEqual = p1[j].IsEqual (p2[j], 1.0E-8);
                        }
                    }
                }
            }
        }

    PK_MEMORY_free (faces1);
    PK_MEMORY_free (faces2);

    // The face test (evaluating and testing a point at each face) was added to address TRE# 281976 which had
    // two rotated symettric bodes that were not distinguisued correctly by PK_DEBUG_BODY_compare.
    int         nEdges1, nEdges2;
    PK_EDGE_t   *edges1 = NULL, *edges2 = NULL;

    PK_BODY_ask_edges (body1, &nEdges1, &edges1);
    PK_BODY_ask_edges (body2, &nEdges2, &edges2);

    bool        edgesEqual;

    if (true == (edgesEqual = (nEdges1 == nEdges2)))
        {
        for (int i=0; i<nEdges1 && edgesEqual; i++)
            {
            PK_CURVE_t      curve1, curve2;
            DPoint3d        p1, p2;

            if (SUCCESS == PK_EDGE_ask_curve (edges1[i], &curve1) &&
                SUCCESS == PK_EDGE_ask_curve (edges2[i], &curve2) &&
                SUCCESS == PK_CURVE_eval (curve1, 0.0, 0, (PK_VECTOR_t*) &p1) && 
                SUCCESS == PK_CURVE_eval (curve2, 0.0, 0, (PK_VECTOR_t*) &p2))
                {
                if (NULL != deltaTransform1To2)
                    deltaTransform1To2->Multiply(p1);

                edgesEqual = p1.IsEqual (p2, tolerance);
                }
            }
        }

    PK_MEMORY_free (edges1);
    PK_MEMORY_free (edges2);

    if (!edgesEqual)
        return false;

    if (NULL != deltaTransform1To2)
        PSolidUtil::TransformBody (body1, *deltaTransform1To2);

    DRange3d        box1, box2;

    if (SUCCESS != pki_get_body_box (&box1.low, &box1.high, body1) ||
        SUCCESS != pki_get_body_box (&box2.low, &box2.high, body2))
        return false;

    if (!box1.IsEqual (box2, tolerance))
        return false;

    PK_DEBUG_BODY_compare_o_t   options;
    PK_DEBUG_BODY_compare_r_t   results;

    PK_DEBUG_BODY_compare_o_m (options);

    if (SUCCESS != PK_DEBUG_BODY_compare (body1, body2, &options, &results))
        return false;

    bool    isEqual = (PK_DEBUG_global_res_no_diffs_c == results.global_result);

    PK_DEBUG_BODY_compare_r_f (&results);

    return isEqual;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::DisjoinBody (bvector<PK_BODY_t>& bodiesOut, PK_BODY_t bodyTag)
    {
    int         nBodies = 0;
    PK_BODY_t*  bodies = nullptr;

    if (SUCCESS != PK_BODY_disjoin(bodyTag, &nBodies, &bodies) || nBodies < 1)
        return ERROR;

    for (int iBody=0; iBody < nBodies; iBody++)
        bodiesOut.push_back(bodies[iBody]);

    PK_MEMORY_free(bodies);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::CreateTransf(PK_TRANSF_t& transfTag, TransformCR transform)
    {
    if (transform.IsIdentity())
        {
        transfTag = NULTAG;

        return SUCCESS;
        }

    RotMatrix rMatrix = RotMatrix::From(transform);

    // Scale the whole transform so the rotational part has determinant +-1.
    // The scale factor goes back in at the 33 spot.
    // It's ok for leading determinant to be negative.
    // It's not ok for the scale factor to be negative.
    double det = rMatrix.Determinant();
    double scale = pow(fabs(det), 1.0 / 3.0);
    double a = 1.0 / scale;

    PK_TRANSF_sf_t transSF;

    for (int i = 0; i < 3; i++)
        {
        for (int j = 0; j < 4; j++)
            transSF.matrix[i][j] = transform.form3d[i][j] * a;
        }

    transSF.matrix[3][0] = 0.0;
    transSF.matrix[3][1] = 0.0;
    transSF.matrix[3][2] = 0.0;
    transSF.matrix[3][3] = a;

    return (SUCCESS == PK_TRANSF_create(&transSF, &transfTag) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/97
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::ApplyTransform(PK_BODY_t bodyTag, PK_TRANSF_t transfTag)
    {
    int                   failureCode;
    PK_TOPOL_track_r_t    tracking;
    PK_TOPOL_local_r_t    results;
    PK_BODY_transform_o_t options;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    PK_BODY_transform_o_m(options);

    failureCode = PK_BODY_transform_2(bodyTag, transfTag, 1.0e-05, &options, &tracking, &results);

    if (PK_ERROR_no_errors == failureCode)
        {
        if (PK_local_status_ok_c != results.status && PK_local_status_nocheck_c != results.status)
            failureCode = ERROR;
        }

    PK_TOPOL_track_r_f(&tracking);
    PK_TOPOL_local_r_f(&results);

    return (SUCCESS == failureCode ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::TransformBody(PK_BODY_t bodyTag, TransformCR transform)
    {
    PK_TRANSF_t transfTag;

    if (SUCCESS != PSolidUtil::CreateTransf(transfTag, transform))
        return ERROR;

    if (NULTAG == transfTag)
        return SUCCESS;

    BentleyStatus status = PSolidUtil::ApplyTransform(bodyTag, transfTag);
    PK_ENTITY_delete(1, &transfTag);

    return status;
    }

