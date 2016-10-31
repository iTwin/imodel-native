/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidTopoId.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

#define KRN_INVALID_FACE_ID                    -1

#define KRN_SLAB_TOP_FACE_ID                    1
#define KRN_SLAB_BOTTOM_FACE_ID                 2
#define KRN_SLAB_FRONT_FACE_ID                  3
#define KRN_SLAB_RIGHT_FACE_ID                  4
#define KRN_SLAB_BACK_FACE_ID                   5
#define KRN_SLAB_LEFT_FACE_ID                   6

#define KRN_CONE_TOP_FACE_ID                    1
#define KRN_CONE_BOTTOM_FACE_ID                 2
#define KRN_CONE_LATERAL_FACE_ID                3

#define KRN_TORUS_ENDCAP1_FACE_ID               1
#define KRN_TORUS_ENDCAP2_FACE_ID               2
#define KRN_TORUS_FACE_ID                       3

#define KRN_SWEEP_TOP_FACE_ID                   1
#define KRN_SWEEP_BOTTOM_FACE_ID                2
#define KRN_SWEEP_FIRST_LATERAL_FACE_ID         3

#define KRN_SHELL_FIRST_OFFSET_FACE_ID          1

#define KRN_RIB_TOP_FACE_ID                     1
#define KRN_RIB_SIDE1_FACE_ID                   2
#define KRN_RIB_SIDE2_FACE_ID                   3

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/97
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidTopoId::AskEntityId
(
uint32_t*   pNodeIdOut,         // <= output node ids of entity
uint32_t*   pEntityIdOut,       // <= output entity id of entity
PK_ATTRIB_t attribTagIn         // => input attribute tag for which to get data
)
    {
    int         numEntityIdInt = 0, *pEntityIdAttribData = 0;

    PK_ATTRIB_ask_ints (attribTagIn, 0, &numEntityIdInt, &pEntityIdAttribData);

    if (pNodeIdOut)
        *pNodeIdOut = pEntityIdAttribData [0];

    if (pEntityIdOut)
        *pEntityIdOut = pEntityIdAttribData [1];

    PK_MEMORY_free (pEntityIdAttribData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::IdFromEntity (FaceId& faceId, PK_ENTITY_t entityTag, bool useHighestId)
    {
    int             numAttribs = 0;
    PK_ATTRIB_t*    attribs = NULL;

    memset (&faceId, 0, sizeof (faceId));
    PSolidAttrib::GetAttrib (&numAttribs, &attribs, entityTag, PKI_ENTITY_ID_ATTRIB_NAME);

    for (int i=0; i < numAttribs; i++)
        {
        uint32_t   nodeId = 0, entityId = 0;

        PSolidTopoId::AskEntityId (&nodeId, &entityId, attribs[i]);

        if (i == 0)
            {
            faceId.entityId = entityId;
            faceId.nodeId   = nodeId;
            }
        else
            {
            if (useHighestId)
                {
                faceId.entityId = ((nodeId == faceId.nodeId && entityId < faceId.entityId) || (nodeId < faceId.nodeId) ? faceId.entityId : entityId);
                faceId.nodeId   = (nodeId < faceId.nodeId ? faceId.nodeId : nodeId);
                }
            else
                {
                faceId.nodeId   = (nodeId < faceId.nodeId ? faceId.nodeId : nodeId);
                }
            }
        }

    PK_MEMORY_free (attribs);

    return (numAttribs > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PSolidTopoId::EntityMatchesId (FaceId const& faceId, PK_ENTITY_t entityTag)
    {
    int             numAttribs = 0;
    PK_ATTRIB_t*    attribs = NULL;

    PSolidAttrib::GetAttrib (&numAttribs, &attribs, entityTag, PKI_ENTITY_ID_ATTRIB_NAME);

    bool        found = false;

    for (int i=0; i < numAttribs; i++)
        {
        uint32_t   nodeId = 0, entityId = 0;

        PSolidTopoId::AskEntityId (&nodeId, &entityId, attribs[i]);

        if (nodeId == faceId.nodeId && entityId == faceId.entityId)
            {
            found = true;
            break;
            }
        }

    PK_MEMORY_free (attribs);

    return found;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::IdFromFace (FaceId& faceId, PK_FACE_t faceTag, bool useHighestId)
    {
    return PSolidTopoId::IdFromEntity (faceId, faceTag, useHighestId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::IdFromEdge (EdgeId& edgeId, PK_EDGE_t edgeTag, bool useHighestId)
    {
    if (!edgeTag)
        return ERROR;

    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_EDGE_ask_faces (edgeTag, &numFaces, &faces);

    BentleyStatus status = ERROR;

    if (numFaces >= 2)
        {
        if (SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[0], faces[0], useHighestId) && 
            SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[1], faces[1], useHighestId))
            status = SUCCESS;
        }

    PK_MEMORY_free (faces);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::IdFromVertex (VertexId& vertexId, PK_VERTEX_t vertexTag, bool useHighestId)
    {
    if (!vertexTag)
        return ERROR;

    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;
    
    PK_VERTEX_ask_faces (vertexTag, &numFaces, &faces);

    BentleyStatus status = ERROR;

    if (numFaces >= 3)
        {
        if (SUCCESS == PSolidTopoId::IdFromFace (vertexId.faces[0], faces[0], useHighestId) &&
            SUCCESS == PSolidTopoId::IdFromFace (vertexId.faces[1], faces[1], useHighestId) &&
            SUCCESS == PSolidTopoId::IdFromFace (vertexId.faces[2], faces[2], useHighestId))
            status = SUCCESS;
        }

    PK_MEMORY_free (faces);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::AssignProfileBodyIds (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    PK_BODY_type_t  bodyType = 0;
    unsigned long   edgeEntityId = 1;

    PK_BODY_ask_type (bodyTag, &bodyType);

    if (PK_BODY_type_sheet_c == bodyType)
        {
        int     faceInd, numFace=0, *pFaceTagArray = NULL;

        PK_BODY_ask_faces (bodyTag, &numFace, &pFaceTagArray);
        edgeEntityId = numFace*2+1;

        for (faceInd = 0; faceInd < numFace; faceInd++)
            {
            int     loopInd, numLoop = 0, *pLoopTagArray = NULL;

            PK_FACE_ask_loops (pFaceTagArray[faceInd], &numLoop, &pLoopTagArray);

            if (numLoop > 1)
                {
                for (loopInd = 0; loopInd < numLoop; loopInd++)
                    {
                    PK_LOOP_type_t      loopType;

                    PK_LOOP_ask_type (pLoopTagArray[loopInd], &loopType);

                    if (loopType == PK_LOOP_type_outer_c)
                        {
                        PK_LOOP_t   outerLoopTag = pLoopTagArray[loopInd];

                        pLoopTagArray[loopInd] = pLoopTagArray[0];
                        pLoopTagArray[0]       = outerLoopTag;
                        break;
                        }
                    }
                }

            for (loopInd = 0; loopInd < numLoop; loopInd++)
                {
                int     coedgeInd=0, coedgeCount=0, numCoedge = 0, *pCoedgeTagArray = NULL;

                PK_LOOP_ask_fins (pLoopTagArray[loopInd], &numCoedge, &pCoedgeTagArray);

                if (numCoedge)
                    {
                    PK_LOGICAL_t    coedgeSense;

                    PK_FIN_is_positive (pCoedgeTagArray[0], &coedgeSense);

                    do
                        {
                        int     edgeTag = 0;

                        if (SUCCESS == PK_FIN_ask_edge (pCoedgeTagArray[coedgeInd], &edgeTag))
                            {
                            int     nFins = 0;

                            PK_EDGE_ask_fins (edgeTag, &nFins, NULL);

                            if (nFins == 1)
                                PSolidTopoId::AttachEntityId (edgeTag, nodeId, edgeEntityId++);
                            }

                        if (coedgeSense == PK_LOGICAL_true)
                            {
                            coedgeInd += 1;
                            }
                        else
                            {
                            if (coedgeInd == 0)
                                coedgeInd = numCoedge-1;
                            else
                                coedgeInd -= 1;
                            }

                        coedgeCount += 1;

                        } while (coedgeCount < numCoedge);

                    PK_MEMORY_free (pCoedgeTagArray);
                    }
                }

            if (numLoop)
                PK_MEMORY_free (pLoopTagArray);
            }

        if (numFace)
            PK_MEMORY_free (pFaceTagArray);

        return SUCCESS;
        }
    else if (PK_BODY_type_wire_c == bodyType)
        {
        int     edgeInd, numEdges = 0, *pEdgeTagArray = NULL;

        PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

        for (edgeInd = 0; edgeInd < numEdges; edgeInd++)
            PSolidTopoId::AttachEntityId (pEdgeTagArray[edgeInd], nodeId, edgeEntityId++);

        if (numEdges)
            PK_MEMORY_free (pEdgeTagArray);

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::CurveTopologyIdFromEdge (CurveTopologyId& id, PK_EDGE_t edgeTag, bool useHighestId)
    {
    id.Clear();

    if (!edgeTag)
        return ERROR;

    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_EDGE_ask_faces (edgeTag, &numFaces, &faces);

    BentleyStatus status = ERROR;

    if (numFaces >= 2)              // Shared edge.
        {
        FaceId  faceIds[2];

        if (SUCCESS == PSolidTopoId::IdFromFace (faceIds[0], faces[0], useHighestId) && 
            SUCCESS == PSolidTopoId::IdFromFace (faceIds[1], faces[1], useHighestId))
            {
            id = CurveTopologyId::FromBRepSharedEdge (faceIds[0], faceIds[1]);
            status = SUCCESS;
            }
        }
    else if (1 == numFaces)         // Sheet edge.
        {
        FaceId  edgeFaceId;

        if (SUCCESS == PSolidTopoId::IdFromEntity (edgeFaceId, faces[0], useHighestId))
            {
            id = CurveTopologyId::FromBRepSheetEdge (edgeFaceId);
            status = SUCCESS;
            }
        }

    PK_MEMORY_free (faces);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/97
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::AttachEntityId
(
PK_ENTITY_t     entityTagIn,        // => input entity tag to which to attach tag
uint32_t        nodeIdIn,           // => input node id of body
uint32_t        entityIdIn          // => input entity id of entity
)
    {
    int         failureCode;
    PK_ATTDEF_t entityIdAttribDefTag;
    PK_ATTRIB_t currEntityIdAttribTag;

    if (entityIdIn == KRN_INVALID_FACE_ID)
        return ERROR;

    if (SUCCESS == (failureCode = PK_ATTDEF_find (PKI_ENTITY_ID_ATTRIB_NAME, &entityIdAttribDefTag)) &&
        SUCCESS == (failureCode = PK_ATTRIB_create_empty (entityTagIn, entityIdAttribDefTag, &currEntityIdAttribTag)))
        {
        int     entityIdAttribData[2];

        entityIdAttribData[0] = nodeIdIn;
        entityIdAttribData[1] = entityIdIn;

        failureCode = PK_ATTRIB_set_ints (currEntityIdAttribTag, 0, 2, entityIdAttribData);
        }

    return (BentleyStatus) failureCode;
    }




