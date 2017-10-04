/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidTopo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetBodyFaces (bvector<PK_FACE_t>& faces, PK_BODY_t body)
    {
    int         faceCount = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_faces (body, &faceCount, &pFaceTagArray) || 0 == faceCount)
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

    if (SUCCESS != PK_BODY_ask_edges (body, &edgeCount, &pEdgeTagArray) || 0 == edgeCount)
        return ERROR;

    edges.resize (edgeCount);
    for (int i=0; i<edgeCount; i++)
        edges[i] = pEdgeTagArray[i];

    PK_MEMORY_free (pEdgeTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetBodyVertices (bvector<PK_VERTEX_t>& vertices, PK_BODY_t body)
    {
    int           vertexCount = 0;
    PK_VERTEX_t*  pVertexTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_vertices (body, &vertexCount, &pVertexTagArray) || 0 == vertexCount)
        return ERROR;

    vertices.resize (vertexCount);
    for (int i=0; i<vertexCount; i++)
        vertices[i] = pVertexTagArray[i];

    PK_MEMORY_free (pVertexTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetBodyEdgesAndFaces (bvector<PK_ENTITY_t>& edgesAndFaces, PK_BODY_t body)
    {
    int         i, edgeCount = 0, faceCount = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_edges (body, &edgeCount, &pEdgeTagArray);
    PK_BODY_ask_faces (body, &faceCount, &pFaceTagArray);

    edgesAndFaces.resize (edgeCount + faceCount);

    for (i=0; i<edgeCount; i++)
        edgesAndFaces[i] = pEdgeTagArray[i];

    for (int j=0; j<faceCount; i++, j++)
        edgesAndFaces[i] = pFaceTagArray[j];

    PK_MEMORY_free (pEdgeTagArray);
    PK_MEMORY_free (pFaceTagArray);

    return edgesAndFaces.empty() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetFaceEdges (bvector<PK_EDGE_t>& edges, PK_FACE_t face)
    {
    int         nEdges = 0;
    PK_EDGE_t*  edgesP = NULL;

    if (SUCCESS != PK_FACE_ask_edges (face, &nEdges, &edgesP) || 0 == nEdges)
        return ERROR;

    edges.resize (nEdges);
    for (int i=0; i<nEdges; i++)
        edges[i] = edgesP[i];

    PK_MEMORY_free (edgesP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetFaceVertices (bvector<PK_VERTEX_t>& vertices, PK_FACE_t face)
    {
    int           nVertex = 0;
    PK_VERTEX_t*  verticesP = NULL;

    if (SUCCESS != PK_FACE_ask_vertices (face, &nVertex, &verticesP) || 0 == nVertex)
        return ERROR;

    vertices.resize (nVertex);
    for (int i=0; i<nVertex; i++)
        vertices[i] = verticesP[i];

    PK_MEMORY_free (verticesP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetFaceLoops (bvector<PK_LOOP_t>& loops, PK_FACE_t face)
    {
    int         loopCount = 0;
    PK_LOOP_t*  pLoopTagArray = NULL;

    if (SUCCESS != PK_FACE_ask_loops (face, &loopCount, &pLoopTagArray) || 0 == loopCount)
        return ERROR;

    loops.resize (loopCount);
    for (int i=0; i<loopCount; i++)
        loops[i] = pLoopTagArray[i];

    PK_MEMORY_free (pLoopTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetEdgeFaces (bvector<PK_FACE_t>& faces, PK_EDGE_t edge)
    {
    int         faceCount = 0;
    PK_FACE_t*  pFaceTagArray = 0;

    if (SUCCESS != PK_EDGE_ask_faces (edge, &faceCount, &pFaceTagArray) || 0 == faceCount)
        return ERROR;

    faces.resize (faceCount);
    for (int i=0; i<faceCount; i++)
        faces[i] = pFaceTagArray[i];

    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetEdgeVertices (bvector<PK_VERTEX_t>& vertices, PK_EDGE_t edge)
    {
    PK_VERTEX_t vertexTags[2];

    if (SUCCESS != PK_EDGE_ask_vertices (edge, vertexTags))
        return ERROR;

    if (PK_ENTITY_null != vertexTags[0])
        vertices.push_back(vertexTags[0]);

    if (PK_ENTITY_null != vertexTags[1])
        vertices.push_back(vertexTags[1]);

    return (0 != vertices.size() ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetEdgeFins (bvector<PK_FIN_t>& fins, PK_EDGE_t edge)
    {
    int         finCount = 0;
    PK_FIN_t*   pFinTagArray = NULL;

    if (SUCCESS != PK_EDGE_ask_fins (edge, &finCount, &pFinTagArray) || 0 == finCount)
        return ERROR;

    fins.resize (finCount);
    for (int i=0; i<finCount; i++)
        fins[i] = pFinTagArray[i];

    PK_MEMORY_free (pFinTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetVertexFaces (bvector<PK_FACE_t>& faces, PK_VERTEX_t vertex)
    {
    int         faceCount = 0;
    PK_FACE_t*  pFaceTagArray = 0;

    if (SUCCESS != PK_VERTEX_ask_faces (vertex, &faceCount, &pFaceTagArray) || 0 == faceCount)
        return ERROR;

    faces.resize (faceCount);
    for (int i=0; i<faceCount; i++)
        faces[i] = pFaceTagArray[i];

    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetVertexEdges (bvector<PK_EDGE_t>& edges, PK_VERTEX_t vertex)
    {
    int         edgeCount = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    if (SUCCESS != PK_VERTEX_ask_oriented_edges (vertex, &edgeCount, &pEdgeTagArray, nullptr) || 0 == edgeCount)
        return ERROR;

    edges.resize (edgeCount);
    for (int i=0; i<edgeCount; i++)
        edges[i] = pEdgeTagArray[i];

    PK_MEMORY_free (pEdgeTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetLoopFins (bvector<PK_FIN_t>& fins, PK_LOOP_t loop)
    {
    int         finCount = 0;
    PK_FIN_t*   pFinTagArray = NULL;

    if (SUCCESS != PK_LOOP_ask_fins (loop, &finCount, &pFinTagArray) || 0 == finCount)
        return ERROR;

    fins.resize (finCount);
    for (int i=0; i<finCount; i++)
        fins[i] = pFinTagArray[i];

    PK_MEMORY_free (pFinTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetTangentBlendEdges(bvector<PK_EDGE_t>& smoothEdges, PK_EDGE_t edgeTag)
    {
    int         nBlend = 0;
    PK_EDGE_t*  blendsP = NULL;
    PK_EDGE_set_blend_constant_o_t options;

    PK_EDGE_set_blend_constant_o_m(options);

    options.properties.propagate = PK_blend_propagate_yes_c;

    if (SUCCESS != PK_EDGE_set_blend_constant(1, &edgeTag, 1.0, &options, &nBlend, &blendsP) || 0 == nBlend)
        return ERROR;

    for (int i=0; i < nBlend; i++)
        {
        smoothEdges.push_back(blendsP[i]);
        PK_EDGE_remove_blend(blendsP[i]);
        }

    PK_MEMORY_free(blendsP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus findEdgeFinForFace(PK_FIN_t& fin, PK_EDGE_t edge, PK_FACE_t face)
    {
    bvector<PK_FIN_t> edgeFins;
    
    PSolidTopo::GetEdgeFins(edgeFins, edge);

    for (PK_FIN_t testFin : edgeFins)
        {
        PK_FACE_t testFace;

        if (SUCCESS == PK_FIN_ask_face(testFin, &testFace) && testFace == face)
            {
            fin = testFin;

            return SUCCESS;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetLoopEdgesFromEdge(bvector<PK_EDGE_t>& loopEdges, PK_EDGE_t edgeTag, PK_FACE_t faceTag)
    {
    PK_FIN_t    finTag;

    if (SUCCESS != findEdgeFinForFace(finTag, edgeTag, faceTag))
        return ERROR;
           
    PK_LOOP_t   loopTag;

    if (SUCCESS != PK_FIN_ask_loop(finTag, &loopTag))
        return ERROR;

    int         nEdgeTags;
    PK_EDGE_t*  edgeTags = NULL;

    if (SUCCESS != PK_LOOP_ask_edges(loopTag, &nEdgeTags, &edgeTags) || 0 == nEdgeTags)
        return ERROR;
    
    loopEdges.resize(nEdgeTags);
    for (int i=0; i < nEdgeTags; i++)
        loopEdges[i] = edgeTags[i];

    PK_MEMORY_free(edgeTags);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopo::GetCurveOfEdge
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

