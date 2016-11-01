/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidTopo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopo::GetBodyFaces (bvector<PK_FACE_t>& faces, PK_BODY_t body)
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
BentleyStatus   PSolidTopo::GetBodyEdges (bvector<PK_EDGE_t>& edges, PK_BODY_t body)
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
* @bsimethod                                                    Brien.Bastings  02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopo::GetCurveOfEdge
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

