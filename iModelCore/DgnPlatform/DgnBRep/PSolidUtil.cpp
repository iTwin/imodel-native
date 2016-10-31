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
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void clipBodyByShape (bvector<PK_BODY_t>& output, bool& clipped, size_t& errorCount, PK_BODY_t body, ClipPrimitiveCR clipPrimitive, TransformCR clipToBody, ClipVectorCR clipVector, size_t nextPrimitiveIndex)
    {                     
    Transform           bodyToShape, shapeToBody = (NULL != clipPrimitive.GetTransformFromClip()) ? Transform::FromProduct (clipToBody, *clipPrimitive.GetTransformFromClip()) : clipToBody;
    PK_BODY_t           clipBody;

    bodyToShape.InverseOf (shapeToBody);
#if defined (NOT_NOW_NEEDSWORK)
    if (SUCCESS != bodyFromSingleParallelClip (clipBody, clipPrimitive, body, shapeToBody, bodyToShape))
#else
    if (true)
#endif
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
