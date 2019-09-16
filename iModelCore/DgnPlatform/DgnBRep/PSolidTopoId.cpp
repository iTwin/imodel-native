/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
BentleyStatus PSolidTopoId::AttachEntityId
(
PK_ENTITY_t     entityTagIn,        // => input entity tag to which to attach tag
uint32_t        nodeIdIn,           // => input node id of body
uint32_t        entityIdIn          // => input entity id of entity
)
    {
    int         failureCode;
    PK_ATTDEF_t entityIdAttribDefTag = PK_ENTITY_null;
    PK_ATTRIB_t currEntityIdAttribTag = PK_ENTITY_null;

    if (entityIdIn == KRN_INVALID_FACE_ID)
        return ERROR;

    if (SUCCESS == (failureCode = PK_ATTDEF_find (PKI_ENTITY_ID_ATTRIB_NAME, &entityIdAttribDefTag)) && PK_ENTITY_null != entityIdAttribDefTag &&
        SUCCESS == (failureCode = PK_ATTRIB_create_empty (entityTagIn, entityIdAttribDefTag, &currEntityIdAttribTag)))
        {
        int     entityIdAttribData[2];

        entityIdAttribData[0] = nodeIdIn;
        entityIdAttribData[1] = entityIdIn;

        failureCode = PK_ATTRIB_set_ints (currEntityIdAttribTag, 0, 2, entityIdAttribData);
        }

    return (BentleyStatus) failureCode;
    }

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
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidTopoId::DeleteEntityId (PK_ENTITY_t entityTag)
    {
    int           numAttrib = 0;
    PK_ATTRIB_t*  attribArray = NULL;

    PSolidAttrib::GetAttrib (&numAttrib, &attribArray, entityTag, PKI_ENTITY_ID_ATTRIB_NAME);

    if (0 != numAttrib)
        PK_ENTITY_delete (numAttrib, attribArray);

    PK_MEMORY_free (attribArray);
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
* @bsimethod                                                    Brien.Bastings  05/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::ExtractEntityIdForNodeId (uint32_t& foundEntityId, bool& uniformNodeIds, PK_ENTITY_t entityTag, uint32_t findNodeId, bool useHighest)
    {
    if (!entityTag || !findNodeId)
        return ERROR;

    int             numAttrib = 0;
    uint32_t        firstNodeId = 0;
    PK_ATTRIB_t*    pAttribArray = NULL;
    BentleyStatus   status = ERROR;

    PSolidAttrib::GetAttrib (&numAttrib, &pAttribArray, entityTag, PKI_ENTITY_ID_ATTRIB_NAME);
    uniformNodeIds = true;

    for (int i=0; i < numAttrib; i++)
        {
        uint32_t nodeId = 0, entityId = 0;

        PSolidTopoId::AskEntityId (&nodeId, &entityId, pAttribArray[i]);

        if (i == 0)
            firstNodeId = nodeId;
        else if (firstNodeId != nodeId)
            uniformNodeIds = false;

        if (nodeId != findNodeId)
            continue;

        if ((i == 0) || (useHighest && entityId > foundEntityId) || (!useHighest && entityId < foundEntityId))
            {
            foundEntityId = entityId;
            status = SUCCESS;
            }
        }

    PK_MEMORY_free (pAttribArray);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
int PSolidTopoId::EntityAttribCompare (PK_ENTITY_t entityTag1, PK_ENTITY_t entityTag2)
    {
    int         status = 0;
    FaceId      faceId1, faceId2;

    if (entityTag1 && entityTag2 &&
        SUCCESS == PSolidTopoId::IdFromEntity (faceId1, entityTag1, true) &&
        SUCCESS == PSolidTopoId::IdFromEntity (faceId2, entityTag2, true))
        {
        if (faceId1.nodeId < faceId2.nodeId)
            status = -1;
        else if (faceId1.nodeId > faceId2.nodeId)
            status = 1;
        else if (faceId1.entityId < faceId2.entityId)
            status = -1;
        else if (faceId1.entityId > faceId2.entityId)
            status = 1;
        }
    else if (!entityTag1 && entityTag2)
        {
        status = 1;
        }
    else if (!entityTag2 && entityTag1)
        {
        status = -1;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int entityAttribCompare (const void* entity1P, const void* entity2P)
    {
    return PSolidTopoId::EntityAttribCompare (*((PK_ENTITY_t const*) entity1P), *((PK_ENTITY_t const*) entity2P));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus facesAroundFace (bvector<PK_FACE_t>& faceTags, PK_FACE_t faceTag, uint32_t nodeId = 0)
    {
    int         nEdges = 0;
    PK_EDGE_t*  edges = NULL;

    if (SUCCESS != PK_FACE_ask_edges (faceTag, &nEdges, &edges))
        return ERROR;

    for (int iEdge = 0; iEdge < nEdges; ++iEdge)
        {
        int         nFaces = 0;
        PK_FACE_t*  faces = NULL;

        if (SUCCESS == PK_EDGE_ask_faces (edges[iEdge], &nFaces, &faces))
            {
            for (int iFace = 0; iFace < nFaces; ++iFace)
                {
                bool      uniformNodeIds = false;
                uint32_t  entityId;

                if (nodeId && SUCCESS != PSolidTopoId::ExtractEntityIdForNodeId (entityId, uniformNodeIds, faces[iFace], nodeId, true))
                    continue;

                faceTags.push_back (faces[iFace]);
                }
            }

        PK_MEMORY_free (faces);
        }

    PK_MEMORY_free (edges);

    return (0 != faceTags.size () ? SUCCESS : ERROR);
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct EntityAttribComparer
{
bool operator () (PK_ENTITY_t entityTag1, PK_ENTITY_t entityTag2) const
    {
    return (-1 == PSolidTopoId::EntityAttribCompare (entityTag1, entityTag2));
    }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct SurroundingFaceComparer
{
uint32_t m_lastNodeId;

bool operator () (PK_FACE_t faceTag1, PK_FACE_t faceTag2) const
    {
    if (!faceTag1 || !faceTag2)
        return (faceTag1 && !faceTag2);

    bvector<PK_FACE_t>  faceTagsAround1, faceTagsAround2;

    facesAroundFace (faceTagsAround1, faceTag1, m_lastNodeId);
    facesAroundFace (faceTagsAround2, faceTag2, m_lastNodeId);

    if (0 != faceTagsAround1.size () && 0 != faceTagsAround2.size ())
        {
        for (size_t iFace = 0; iFace < faceTagsAround1.size (); ++iFace)
            {
            bvector<PK_FACE_t>::iterator  location;

            if (faceTagsAround2.end () == (location = std::find (faceTagsAround2.begin (), faceTagsAround2.end (), faceTagsAround1.at (iFace))))
                continue;

            faceTagsAround1[iFace] = 0;
            faceTagsAround2[location - faceTagsAround2.begin ()] = 0;
            }
        }

    std::sort (faceTagsAround1.begin (), faceTagsAround1.end (), EntityAttribComparer ());
    std::sort (faceTagsAround2.begin (), faceTagsAround2.end (), EntityAttribComparer ());

    PK_FACE_t  faceCompareTag1 = (0 != faceTagsAround1.size () ? faceTagsAround1.back () : 0);
    PK_FACE_t  faceCompareTag2 = (0 != faceTagsAround2.size () ? faceTagsAround2.back () : 0);

    bool      uniformNodeIds;
    uint32_t entityId1, entityId2;

    if (SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (entityId1, uniformNodeIds, faceCompareTag1, m_lastNodeId, true) &&
        SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (entityId2, uniformNodeIds, faceCompareTag2, m_lastNodeId, true))
        return (entityId1 < entityId2);

    return (faceCompareTag1 && !faceCompareTag2);
    }

SurroundingFaceComparer (uint32_t lastNodeId) {m_lastNodeId = lastNodeId;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/98
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6385 6386) // static analysis thinks we exceed the bounds of pSameNodeIdArray, pSameFaceIdArray, and pEntityIdArray... I don't see how.
static void resolveDuplicateFaceIds (int numFace, PK_FACE_t* pFaceTagArray, uint32_t lastNodeId)
    {
    PK_FACE_t*  pSameNodeIdArray = (PK_FACE_t*) malloc (numFace * sizeof (PK_FACE_t));;
    PK_FACE_t*  pSameFaceIdArray = (PK_FACE_t*) malloc (numFace * sizeof (PK_FACE_t));;
    uint32_t*   pEntityIdArray   = (uint32_t*)  malloc (numFace * sizeof (uint32_t));;

    for (int outerIndex = 0; outerIndex < numFace; outerIndex++)
        {
        if (!pFaceTagArray[outerIndex])
            continue; // Already checked...

        FaceId  thisFaceId;

        if (SUCCESS != PSolidTopoId::IdFromEntity (thisFaceId, pFaceTagArray[outerIndex], true))
            continue;

        int     facesOfNodeId = 0;

        for (int i = 0; i < numFace; i++)
            {
            if (!pFaceTagArray[i])
                continue; // Already checked...

            FaceId  faceId;

            if (SUCCESS != PSolidTopoId::IdFromEntity (faceId, pFaceTagArray[i], true) || faceId.nodeId != thisFaceId.nodeId)
                continue;

            pSameNodeIdArray[facesOfNodeId] = pFaceTagArray[i];
            pEntityIdArray[facesOfNodeId++] = faceId.entityId;
            pFaceTagArray[i] = 0; // Don't check this face again...
            }

        int     facesOfFaceId = 0;

        for (int innerIndex = 0; innerIndex < facesOfNodeId-1; innerIndex++)
            {
            if (!pSameNodeIdArray[innerIndex])
                continue; // Already checked...

            bool    duplicateFound = false;

            for (int i = innerIndex+1; i < facesOfNodeId; i++)
                {
                if (!pSameNodeIdArray[i])
                    continue; // Already checked...

                if (pEntityIdArray[i] != pEntityIdArray[innerIndex])
                    continue;

                if (!duplicateFound)
                    {
                    pSameFaceIdArray[facesOfFaceId++] = pSameNodeIdArray[innerIndex];
                    duplicateFound = true;
                    }

                pSameFaceIdArray[facesOfFaceId++] = pSameNodeIdArray[i];
                pSameNodeIdArray[i] = 0; // Don't check this again...
                }
            }

        if (facesOfFaceId > 1)
            {
            if (lastNodeId) // Yuck...horribly complicated/inefficient sort that only matters for feature solids...
                std::sort (pSameFaceIdArray, pSameFaceIdArray + facesOfFaceId, SurroundingFaceComparer (lastNodeId));

            std::sort (pEntityIdArray, pEntityIdArray + facesOfNodeId);

            uint32_t highestEntityId = pEntityIdArray[facesOfNodeId-1]; // Choose highest entity id from faces for this node id...

            for (int i = 0; i < facesOfFaceId; i++)
                PSolidTopoId::AttachEntityId (pSameFaceIdArray[i], thisFaceId.nodeId, ++highestEntityId);
            }
        }

    free (pSameNodeIdArray);
    free (pSameFaceIdArray);
    free (pEntityIdArray);
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::ResolveDuplicateFaceIds (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 == numFaces)
        return ERROR;

    resolveDuplicateFaceIds (numFaces, pFaceTagArray, nodeId);

    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void assignFaceIds (int numFaces, PK_FACE_t* pFaceTagArray, uint32_t nodeId, bool overrideExisting)
    {
    if (overrideExisting)
        {
        for (int faceInd = 0; faceInd < numFaces; faceInd++)
            {
            PSolidTopoId::DeleteEntityId (pFaceTagArray[faceInd]);
            PSolidTopoId::AttachEntityId (pFaceTagArray[faceInd], nodeId, faceInd+1);
            }

        return;
        }

    uint32_t            highestEntityId = 0;
    FaceId              entityId;
    bvector<PK_FACE_t>  facesToAssign;

    for (int faceInd = 0; faceInd < numFaces; faceInd++)
        {
        if (SUCCESS == PSolidTopoId::IdFromEntity (entityId, pFaceTagArray[faceInd], true))
            {
            if (entityId.nodeId == nodeId && entityId.entityId > highestEntityId)
                highestEntityId = entityId.entityId;
            }
        else
            {
            facesToAssign.push_back (pFaceTagArray[faceInd]);
            }
        }

    int nextEntityId = highestEntityId + 1;
    
    for (PK_FACE_t faceTag : facesToAssign)
        PSolidTopoId::AttachEntityId (faceTag, nodeId, nextEntityId++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AssignFaceIds (PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting)
    {
    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 == numFaces)
        return ERROR;

    assignFaceIds (numFaces, pFaceTagArray, nodeId, overrideExisting);
    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void assignEdgeIds (int numEdges, PK_EDGE_t* pEdgeTagArray, uint32_t nodeId, bool overrideExisting)
    {
    if (overrideExisting)
        {
        for (int edgeInd = 0; edgeInd < numEdges; edgeInd++)
            {
            PSolidTopoId::DeleteEntityId (pEdgeTagArray[edgeInd]);
            PSolidTopoId::AttachEntityId (pEdgeTagArray[edgeInd], nodeId, edgeInd+1);
            }
        }
    else
        {
        uint32_t            highestEntityId = 0;
        FaceId              entityId;
        bvector<PK_EDGE_t>  edgesToAssign;

        for (int edgeInd = 0; edgeInd < numEdges; edgeInd++)
            {
            if (SUCCESS == PSolidTopoId::IdFromEntity (entityId, pEdgeTagArray[edgeInd], true))
                {
                if (entityId.nodeId == nodeId && entityId.entityId > highestEntityId)
                    highestEntityId = entityId.entityId;
                }
            else
                {
                edgesToAssign.push_back (pEdgeTagArray[edgeInd]);
                }
            }

        int nextEntityId = highestEntityId + 1;
    
        for (PK_EDGE_t edgeTag : edgesToAssign)
            PSolidTopoId::AttachEntityId (edgeTag, nodeId, nextEntityId++);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AssignEdgeIds (PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting)
    {
    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 == numEdges)
        return ERROR;

    assignEdgeIds (numEdges, pEdgeTagArray, nodeId, overrideExisting);
    PK_MEMORY_free (pEdgeTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::FindNodeIdRange (PK_BODY_t bodyTag, uint32_t& highestNodeId, uint32_t& lowestNodeId)
    {
    highestNodeId = 0;
    lowestNodeId  = 0xffffffff;

    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 != numFaces)
        {
        for (int faceInd = 0; faceInd < numFaces; faceInd++)
            {
            FaceId  entityId;

            if (SUCCESS != PSolidTopoId::IdFromEntity (entityId, pFaceTagArray[faceInd], true))
                continue;

            if (entityId.nodeId > highestNodeId)
                highestNodeId = entityId.nodeId;

            if (entityId.nodeId < lowestNodeId)
                lowestNodeId = entityId.nodeId;
            }

        PK_MEMORY_free (pFaceTagArray);
        }

    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 != numEdges)
        {
        for (int edgeInd = 0; edgeInd < numEdges; edgeInd++)
            {
            FaceId  entityId;

            if (SUCCESS != PSolidTopoId::IdFromEntity (entityId, pEdgeTagArray[edgeInd], true))
                continue;

            if (entityId.nodeId > highestNodeId)
                highestNodeId = entityId.nodeId;

            if (entityId.nodeId < lowestNodeId)
                lowestNodeId = entityId.nodeId;
            }

        PK_MEMORY_free (pEdgeTagArray);
        }

    return (highestNodeId > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AddNodeIdAttributes (PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting)
    {
    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 != numFaces)
        {
        assignFaceIds (numFaces, pFaceTagArray, nodeId, overrideExisting);

        if (!overrideExisting)
            resolveDuplicateFaceIds (numFaces, pFaceTagArray, nodeId);

        PK_MEMORY_free (pFaceTagArray);
        }

    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 != numEdges)
        {
        assignEdgeIds (numEdges, pEdgeTagArray, nodeId, overrideExisting);
        PK_MEMORY_free (pEdgeTagArray);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AddNewNodeIdAttributes (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    // NOTE: This method attempted to assign new face ids by sorting existing face ids,
    //       i.e. collapse a multi-node id solid to a single node id solid and have the ids
    //       be a little more stable than just assigning them sequentially. Feature solids
    //       would use this when a smart solid was used as a leaf node...
    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 != numFaces)
        {
        // NOTE: Ugh, qsort can change order even if all compares are 0...
	    qsort (&pFaceTagArray[0], numFaces, sizeof (PK_FACE_t), entityAttribCompare);

        assignFaceIds (numFaces, pFaceTagArray, nodeId, true);
        PK_MEMORY_free (pFaceTagArray);
        }

    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 != numEdges)
        {
        assignEdgeIds (numEdges, pEdgeTagArray, nodeId, true);
        PK_MEMORY_free (pEdgeTagArray);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::DeleteNodeIdAttributes (PK_BODY_t bodyTag)
    {
    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 != numEdges)
        {
        for (int edgeInd = 0; edgeInd < numEdges; edgeInd++)
            PSolidTopoId::DeleteEntityId (pEdgeTagArray[edgeInd]);
        }

    PK_MEMORY_free (pEdgeTagArray);

    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 != numFaces)
        {
        for (int faceInd = 0; faceInd < numFaces; faceInd++)
            PSolidTopoId::DeleteEntityId (pFaceTagArray[faceInd]);
        }

    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/94
+---------------+---------------+---------------+---------------+---------------+------*/
static void incrementIdAttribute (PK_ENTITY_t entityTag, int32_t increment)
    {
    int           numAttrib = 0;
    PK_ATTRIB_t*  pAttribArray = NULL;

    PSolidAttrib::GetAttrib (&numAttrib, &pAttribArray, entityTag, PKI_ENTITY_ID_ATTRIB_NAME);

    for (int i = 0; i < numAttrib; i++)
        {
        uint32_t nodeId, entityId;

        PSolidTopoId::AskEntityId (&nodeId, &entityId, pAttribArray[i]);
        PSolidTopoId::AttachEntityId (entityTag, nodeId + increment, entityId);
        PK_ENTITY_delete (1, &pAttribArray[i]);
        }

    PK_MEMORY_free (pAttribArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::IncrementNodeIdAttributes (PK_BODY_t bodyTag, int32_t increment)
    {
    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 != numFaces)
        {
        for (int faceInd = 0; faceInd < numFaces; faceInd++)
            incrementIdAttribute (pFaceTagArray[faceInd], increment);

        PK_MEMORY_free (pFaceTagArray);
        }

    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 != numEdges)
        {
        for (int edgeInd = 0; edgeInd < numEdges; edgeInd++)
            incrementIdAttribute (pEdgeTagArray[edgeInd], increment);

        PK_MEMORY_free (pEdgeTagArray);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void changeIdAttribute (PK_ENTITY_t entityTag, uint32_t newNodeId, uint32_t oldNodeId)
    {
    int           numAttrib = 0;
    PK_ATTRIB_t*  pAttribArray = NULL;

    PSolidAttrib::GetAttrib (&numAttrib, &pAttribArray, entityTag, PKI_ENTITY_ID_ATTRIB_NAME);

    for (int i = 0; i < numAttrib; i++)
        {
        uint32_t nodeId, entityId;

        PSolidTopoId::AskEntityId (&nodeId, &entityId, pAttribArray[i]);

        if (nodeId == newNodeId)
            continue;

        if (nodeId == oldNodeId)
            PSolidTopoId::AttachEntityId (entityTag, newNodeId, entityId);

        PK_ENTITY_delete (1, &pAttribArray[i]);
        }

    PK_MEMORY_free (pAttribArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::ChangeNodeIdAttributes (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    uint32_t highestNodeId, lowestNodeId;

    if (SUCCESS != PSolidTopoId::FindNodeIdRange(bodyTag, highestNodeId, lowestNodeId))
        return ERROR;

    int         numFaces = 0;
    PK_FACE_t*  pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFaces, &pFaceTagArray);

    if (0 != numFaces)
        {
        for (int faceInd = 0; faceInd < numFaces; faceInd++)
            changeIdAttribute (pFaceTagArray[faceInd], nodeId, highestNodeId);

        PK_MEMORY_free (pFaceTagArray);
        }

    int         numEdges = 0;
    PK_EDGE_t*  pEdgeTagArray = NULL;

    PK_BODY_ask_edges (bodyTag, &numEdges, &pEdgeTagArray);

    if (0 != numEdges)
        {
        for (int edgeInd = 0; edgeInd < numEdges; edgeInd++)
            changeIdAttribute (pEdgeTagArray[edgeInd], nodeId, highestNodeId);

        PK_MEMORY_free (pEdgeTagArray);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AssignConeFaceIds (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    int         faceInd, numFace=0, *pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFace, &pFaceTagArray);

    for (faceInd = 0; faceInd < numFace; faceInd++)
        {
        int             currSurfaceTag;
        PK_CLASS_t      surfaceClass;
        PK_LOGICAL_t    faceSense;

        if (SUCCESS == PK_FACE_ask_oriented_surf (pFaceTagArray[faceInd], &currSurfaceTag, &faceSense) &&
            SUCCESS == PK_ENTITY_ask_class (currSurfaceTag, &surfaceClass))
            {
            int     currFaceId = KRN_INVALID_FACE_ID;

            if (surfaceClass == PK_CLASS_plane)
                {
                PK_PLANE_sf_t   currPlaneStruct;

                if (SUCCESS == PK_PLANE_ask (currSurfaceTag, &currPlaneStruct))
                    {
                    PK_VECTOR1_t    *pFaceNormal = &currPlaneStruct.basis_set.axis;

                    if (faceSense != PK_LOGICAL_true)
                        {
                        pFaceNormal->coord[0] *= -1.0;
                        pFaceNormal->coord[1] *= -1.0;
                        pFaceNormal->coord[2] *= -1.0;
                        }

                    if (DoubleOps::WithinTolerance (pFaceNormal->coord[0], 0.0, 1e-08) &&
                        DoubleOps::WithinTolerance (pFaceNormal->coord[1], 0.0, 1e-08) &&
                        DoubleOps::WithinTolerance (pFaceNormal->coord[2], 1.0, 1e-08))
                        {
                        currFaceId = KRN_CONE_TOP_FACE_ID;
                        }
                    else
                        {
                        currFaceId = KRN_CONE_BOTTOM_FACE_ID;
                        }
                    }
                }
            else
                {
                currFaceId = KRN_CONE_LATERAL_FACE_ID;
                }

            PSolidTopoId::AttachEntityId (pFaceTagArray[faceInd], nodeId, currFaceId);
            }
        }

    if (numFace)
        PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AssignSlabFaceIds (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    int         faceInd, numFace=0, *pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFace, &pFaceTagArray);

    for (faceInd = 0; faceInd < numFace; faceInd++)
        {
        int             currSurfaceTag;
        PK_LOGICAL_t    faceSense;
        PK_PLANE_sf_t   currPlaneStruct;

        if (SUCCESS == PK_FACE_ask_oriented_surf (pFaceTagArray[faceInd], &currSurfaceTag, &faceSense) &&
            SUCCESS == PK_PLANE_ask (currSurfaceTag, &currPlaneStruct))
            {
            int             currFaceId = KRN_INVALID_FACE_ID;
            PK_VECTOR1_t    *pFaceNormal = &currPlaneStruct.basis_set.axis;

            if (faceSense != PK_LOGICAL_true)
                {
                pFaceNormal->coord[0] *= -1.0;
                pFaceNormal->coord[1] *= -1.0;
                pFaceNormal->coord[2] *= -1.0;
                }

            if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], 0.0, 1e-08) &&
                DoubleOps::WithinTolerance(pFaceNormal->coord[1], 0.0, 1e-08) &&
                DoubleOps::WithinTolerance(pFaceNormal->coord[2], -1.0, 1e-08))
                {
                currFaceId = KRN_SLAB_TOP_FACE_ID;
                }
            else if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], 0.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[1], 0.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[2], 1.0, 1e-08))
                {
                currFaceId = KRN_SLAB_BOTTOM_FACE_ID;
                }
            else if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], 0.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[1], -1.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[2], 0.0, 1e-08))
                {
                currFaceId = KRN_SLAB_FRONT_FACE_ID;
                }
            else if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], 1.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[1], 0.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[2], 0.0, 1e-08))
                {
                currFaceId = KRN_SLAB_LEFT_FACE_ID;
                }
            else if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], 0.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[1], 1.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[2], 0.0, 1e-08))
                {
                currFaceId = KRN_SLAB_BACK_FACE_ID;
                }
            else if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], -1.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[1], 0.0, 1e-08) &&
                     DoubleOps::WithinTolerance(pFaceNormal->coord[2], 0.0, 1e-08))
                {
                currFaceId = KRN_SLAB_RIGHT_FACE_ID;
                }

            PSolidTopoId::AttachEntityId (pFaceTagArray[faceInd], nodeId, currFaceId);
            }
        }

    if (numFace)
        PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::AssignTorusFaceIds (PK_BODY_t bodyTag, uint32_t nodeId)
    {
    int         faceInd, numFace=0, *pFaceTagArray = NULL;

    PK_BODY_ask_faces (bodyTag, &numFace, &pFaceTagArray);

    for (faceInd = 0; faceInd < numFace; faceInd++)
        {
        int             currSurfaceTag;
        PK_CLASS_t      surfaceClass;
        PK_LOGICAL_t    faceSense;

        if (SUCCESS == PK_FACE_ask_oriented_surf (pFaceTagArray[faceInd], &currSurfaceTag, &faceSense) &&
            SUCCESS == PK_ENTITY_ask_class (currSurfaceTag, &surfaceClass))
            {
            int     currFaceId = KRN_INVALID_FACE_ID;

            if (surfaceClass == PK_CLASS_plane)
                {
                PK_PLANE_sf_t   currPlaneStruct;

                if (SUCCESS == PK_PLANE_ask (currSurfaceTag, &currPlaneStruct))
                    {
                    PK_VECTOR1_t    *pFaceNormal = &currPlaneStruct.basis_set.axis;

                    if (faceSense != PK_LOGICAL_true)
                        {
                        pFaceNormal->coord[0] *= -1.0;
                        pFaceNormal->coord[1] *= -1.0;
                        pFaceNormal->coord[2] *= -1.0;
                        }

                    if (DoubleOps::WithinTolerance(pFaceNormal->coord[0], 0.0, 1e-08) &&
                        DoubleOps::WithinTolerance(pFaceNormal->coord[1], -1.0, 1e-08) &&
                        DoubleOps::WithinTolerance(pFaceNormal->coord[2], 0.0, 1e-08))
                        {
                        currFaceId = KRN_TORUS_ENDCAP1_FACE_ID;
                        }
                    else
                        {
                        currFaceId = KRN_TORUS_ENDCAP2_FACE_ID;
                        }
                    }
                }
            else
                {
                currFaceId = KRN_TORUS_FACE_ID;
                }

            PSolidTopoId::AttachEntityId (pFaceTagArray[faceInd], nodeId, currFaceId);
            }
        }

    if (numFace)
        PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::AssignProfileBodyIds (PK_BODY_t bodyTag, uint32_t nodeId, bool singleHoleLoopPriority)
    {
    PK_BODY_type_t  bodyType = 0;
    unsigned long   edgeEntityId = 1;

    PK_BODY_ask_type (bodyTag, &bodyType);

    if (PK_BODY_type_sheet_c == bodyType)
        {
        int faceInd, numFace=0, *pFaceTagArray = NULL;

        PK_BODY_ask_faces (bodyTag, &numFace, &pFaceTagArray);
        edgeEntityId = numFace*2+1;

        for (faceInd = 0; faceInd < numFace; faceInd++)
            {
            int loopInd, numLoop = 0, *pLoopTagArray = NULL;

            PK_FACE_ask_loops (pFaceTagArray[faceInd], &numLoop, &pLoopTagArray);

            if (numLoop > 1)
                {
                PK_LOOP_type_t priorityLoopType = PK_LOOP_type_outer_c;

                if (singleHoleLoopPriority && 2 == numLoop)
                    priorityLoopType = PK_LOOP_type_inner_c;

                for (loopInd = 0; loopInd < numLoop; loopInd++)
                    {
                    PK_LOOP_type_t      loopType;

                    PK_LOOP_ask_type (pLoopTagArray[loopInd], &loopType);

                    if (loopType == priorityLoopType)
                        {
                        PK_LOOP_t tmpLoopTag = pLoopTagArray[loopInd];

                        pLoopTagArray[loopInd] = pLoopTagArray[0];
                        pLoopTagArray[0] = tmpLoopTag;
                        break;
                        }
                    }
                }

            for (loopInd = 0; loopInd < numLoop; loopInd++)
                {
                int coedgeInd=0, coedgeCount=0, numCoedge = 0, *pCoedgeTagArray = NULL;

                PK_LOOP_ask_fins (pLoopTagArray[loopInd], &numCoedge, &pCoedgeTagArray);

                if (numCoedge)
                    {
                    PK_LOGICAL_t coedgeSense;

                    PK_FIN_is_positive (pCoedgeTagArray[0], &coedgeSense);

                    do
                        {
                        int edgeTag = 0;

                        if (SUCCESS == PK_FIN_ask_edge (pCoedgeTagArray[coedgeInd], &edgeTag))
                            {
                            int nFins = 0;

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
        int edgeInd, numEdges = 0, *pEdgeTagArray = NULL;

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
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        PSolidTopoId::AssignSweptProfileLateralIds (int nLaterals, int* baseArray, int* laterals)
    {
    FaceId      faceId;

    memset (&faceId, 0, sizeof (FaceId));

    for (int i=0; i < nLaterals; i++)
        {
        if (SUCCESS != PSolidTopoId::IdFromEntity (faceId, baseArray[i], true))
            continue;

        PSolidTopoId::AttachEntityId (laterals[i], faceId.nodeId, faceId.entityId);
        }

    return faceId.nodeId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::LowestUniqueIdFromEdge (EdgeId& edgeId, PK_EDGE_t edgeTag)
    {
    if (!edgeTag)
        return ERROR;

    int         nEdgeFaces = 0;
    PK_FACE_t*  edgeFaces = NULL;
    PK_FACE_t   faceTag0, faceTag1;

    PK_EDGE_ask_faces (edgeTag, &nEdgeFaces, &edgeFaces);

    faceTag0 = (nEdgeFaces > 0 ? edgeFaces[0] : PK_ENTITY_null);
    faceTag1 = (nEdgeFaces > 1 ? edgeFaces[1] : PK_ENTITY_null);

    PK_MEMORY_free (edgeFaces);

    if (nEdgeFaces < 2)
        return ERROR;

    FaceId      faceId;
    uint32_t   preferedNodeId = 0L;

    if (SUCCESS == PSolidTopoId::IdFromEntity (faceId, edgeTag, true))
        preferedNodeId = faceId.nodeId;

    bool        uniformNodeIds = false;
    PK_BODY_t   bodyTag = PSolidUtil::GetBodyForEntity (edgeTag);

    edgeId.faces[0].nodeId = preferedNodeId;
    edgeId.faces[1].nodeId = preferedNodeId;

    if ((SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[0].entityId, uniformNodeIds, faceTag0, preferedNodeId, false) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[0], faceTag0, false)) &&
        (SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[1].entityId, uniformNodeIds, faceTag1, preferedNodeId, false) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[1], faceTag1, false)))
        {
        bvector<PK_EDGE_t>  edgeVector;

        if (SUCCESS == PSolidTopoId::EdgesFromId (edgeVector, edgeId, bodyTag) && 1 == edgeVector.size ())
            return SUCCESS;
        }

    edgeId.faces[0].nodeId = preferedNodeId;
    edgeId.faces[1].nodeId = preferedNodeId;

    if ((SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[0].entityId, uniformNodeIds, faceTag0, preferedNodeId, false) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[0], faceTag0, false)) &&
        (SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[1].entityId, uniformNodeIds, faceTag1, preferedNodeId, true) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[1], faceTag1, true)))
        {
        bvector<PK_EDGE_t>  edgeVector;

        if (SUCCESS == PSolidTopoId::EdgesFromId (edgeVector, edgeId, bodyTag) && 1 == edgeVector.size ())
            return SUCCESS;
        }

    edgeId.faces[0].nodeId = preferedNodeId;
    edgeId.faces[1].nodeId = preferedNodeId;

    if ((SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[0].entityId, uniformNodeIds, faceTag0, preferedNodeId, true) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[0], faceTag0, true)) &&
        (SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[1].entityId, uniformNodeIds, faceTag1, preferedNodeId, false) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[1], faceTag1, false)))
        {
        bvector<PK_EDGE_t>  edgeVector;

        if (SUCCESS == PSolidTopoId::EdgesFromId (edgeVector, edgeId, bodyTag) && 1 == edgeVector.size ())
            return SUCCESS;
        }

    edgeId.faces[0].nodeId = preferedNodeId;
    edgeId.faces[1].nodeId = preferedNodeId;

    if ((SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[0].entityId, uniformNodeIds, faceTag0, preferedNodeId, true) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[0], faceTag0, true)) &&
        (SUCCESS == PSolidTopoId::ExtractEntityIdForNodeId (edgeId.faces[1].entityId, uniformNodeIds, faceTag1, preferedNodeId, true) ||
         SUCCESS == PSolidTopoId::IdFromFace (edgeId.faces[1], faceTag1, true)))
        {
        return SUCCESS; // Don't check for uniqueness...
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::FaceFromId (PK_FACE_t& faceTag, FaceId const& faceId, PK_BODY_t bodyTag)
    {
    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;
    
    PK_BODY_ask_faces (bodyTag, &numFaces, &faces);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numFaces; i++)
        {
        if (PSolidTopoId::EntityMatchesId (faceId, faces[i]))
            {
            faceTag = faces[i];
            status  = SUCCESS;
            }
        
        if (SUCCESS == status)
            break;
        }

    PK_MEMORY_free (faces);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::FacesFromNodeId (bvector<PK_FACE_t>& faceVector, uint32_t nodeId, PK_BODY_t bodyTag)
    {
    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;
    
    PK_BODY_ask_faces (bodyTag, &numFaces, &faces);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numFaces; i++)
        {
        FaceId      faceId;

        if (SUCCESS != PSolidTopoId::IdFromFace (faceId, faces[i], true) || faceId.nodeId != nodeId)
            continue;

        faceVector.push_back (faces[i]);
        status = SUCCESS;
        }

    PK_MEMORY_free (faces);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidTopoId::FacesFromId (bvector<PK_FACE_t>& faceVector, FaceId const& faceId, PK_BODY_t bodyTag)
    {
    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;
    
    PK_BODY_ask_faces (bodyTag, &numFaces, &faces);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numFaces; i++)
        {
        if (!PSolidTopoId::EntityMatchesId (faceId, faces[i]))
            continue;

        faceVector.push_back (faces[i]);
        status = SUCCESS;
        }

    PK_MEMORY_free (faces);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::EdgeFromId (PK_EDGE_t& edgeTag, EdgeId const& edgeId, PK_BODY_t bodyTag)
    {
    int         numEdges = 0;
    PK_EDGE_t*  edges = NULL;
    
    PK_BODY_ask_edges (bodyTag, &numEdges, &edges);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numEdges; i++)
        {
        int         numFaces = 0;
        PK_FACE_t*  faces = NULL;

        PK_EDGE_ask_faces (edges[i], &numFaces, &faces);

        if (numFaces >= 2)
            {
            if ((PSolidTopoId::EntityMatchesId (edgeId.faces[0], faces[0]) && PSolidTopoId::EntityMatchesId (edgeId.faces[1], faces[1])) ||
                (PSolidTopoId::EntityMatchesId (edgeId.faces[1], faces[0]) && PSolidTopoId::EntityMatchesId (edgeId.faces[0], faces[1])))
                {
                edgeTag = edges[i];
                status  = SUCCESS;
                }
            }

        PK_MEMORY_free (faces);

        if (SUCCESS == status)
            break;
        }

    PK_MEMORY_free (edges);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::EdgesFromId (bvector<PK_EDGE_t>& edgeVector, EdgeId const& edgeId, PK_BODY_t bodyTag)
    {
    int         numEdges = 0;
    PK_EDGE_t*  edges = NULL;
    
    PK_BODY_ask_edges (bodyTag, &numEdges, &edges);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numEdges; i++)
        {
        int         numFaces = 0;
        PK_FACE_t*  faces = NULL;

        PK_EDGE_ask_faces (edges[i], &numFaces, &faces);

        if (numFaces >= 2)
            {
            if (!((PSolidTopoId::EntityMatchesId (edgeId.faces[0], faces[0]) && PSolidTopoId::EntityMatchesId (edgeId.faces[1], faces[1])) ||
                  (PSolidTopoId::EntityMatchesId (edgeId.faces[1], faces[0]) && PSolidTopoId::EntityMatchesId (edgeId.faces[0], faces[1]))))
                continue;

            edgeVector.push_back (edges[i]);
            status = SUCCESS;
            }

        PK_MEMORY_free (faces);
        }

    PK_MEMORY_free (edges);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::VertexFromId (PK_VERTEX_t& vertexTag, VertexId const& vertexId, PK_BODY_t bodyTag)
    {
    int         numVertices = 0;
    PK_EDGE_t*  vertices = NULL;
    
    PK_BODY_ask_vertices (bodyTag, &numVertices, &vertices);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numVertices; i++)
        {
        int         numFaces = 0;
        PK_FACE_t*  faces = NULL;

        PK_VERTEX_ask_faces (vertices[i], &numFaces, &faces);

        if (numFaces >= 2)
            {
            if ((PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[2])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[1])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[2])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[0])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[1])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[0])))
                {
                vertexTag = vertices[i];
                status    = SUCCESS;
                }
            }

        PK_MEMORY_free (faces);

        if (SUCCESS == status)
            break;
        }

    PK_MEMORY_free (vertices);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidTopoId::VerticesFromId (bvector<PK_VERTEX_t>& vertexVector, VertexId const& vertexId, PK_BODY_t bodyTag)
    {
    int         numVertices = 0;
    PK_EDGE_t*  vertices = NULL;
    
    PK_BODY_ask_vertices (bodyTag, &numVertices, &vertices);

    BentleyStatus   status = ERROR;

    for (int i=0; i < numVertices; i++)
        {
        int         numFaces = 0;
        PK_FACE_t*  faces = NULL;

        PK_VERTEX_ask_faces (vertices[i], &numFaces, &faces);

        if (numFaces >= 2)
            {
            if ((PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[2])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[1])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[2])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[0])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[0]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[1])) ||
                (PSolidTopoId::EntityMatchesId (vertexId.faces[0], faces[2]) && PSolidTopoId::EntityMatchesId (vertexId.faces[1], faces[1]) && PSolidTopoId::EntityMatchesId (vertexId.faces[2], faces[0])))
                {
                vertexVector.push_back (vertices[i]);
                status = SUCCESS;
                }
            }

        PK_MEMORY_free (faces);
        }

    PK_MEMORY_free (vertices);

    return status;
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






