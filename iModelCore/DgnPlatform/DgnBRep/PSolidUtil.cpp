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
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::GetBodyEdges (bvector<PK_EDGE_t>& edges, PK_BODY_t body)
    {
    int         edgeCount = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_edges (body, &edgeCount, &pEdgeTagArray))
        return ERROR;

    edges.resize (edgeCount);
    for (int i=0; i<edgeCount; i++)
        edges[i] = pEdgeTagArray[i];

    PK_MEMORY_free (pEdgeTagArray);

    return SUCCESS;
    }

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

