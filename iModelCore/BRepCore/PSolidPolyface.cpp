/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>
#include <Mtg/MtgApi.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

static int s_debug = 0;

static char const*tokenToString (int eClass)
    {
    if (eClass == PK_CLASS_vertex)
        return "VRTX";
    if (eClass == PK_CLASS_edge)
        return "EDGE";
    if (eClass == PK_CLASS_face)
        return "FACE";
    if (eClass == PK_CLASS_loop)
        return "LOOP";
    if (eClass == PK_CLASS_shell)
        return "SHELL";
    if (eClass == PK_CLASS_body)
        return "BODY";
    if (eClass == PK_TOPOL_sense_none_c)
        return " ";
    if (eClass == PK_TOPOL_sense_positive_c)
        return "+";
    if (eClass == PK_TOPOL_sense_negative_c)
        return "-";
    return "????";
    }

class MTGPSD_TopologyBuilder
{
private:
    EmbeddedIntArray *m_pEntityClassArray;
    EmbeddedIntArray *m_pParentArray;
    EmbeddedIntArray *m_pChildArray;
    EmbeddedIntArray *m_pSenseArray;

    EmbeddedIntArray *m_pNodeIdToFaceIndexArray;
    EmbeddedIntArray *m_pNodeIdToVertexIndexArray;
    EmbeddedIntArray *m_pNodeIdToEdgeIndexArray;

    int *m_pTagArray;

    MTGFacets *m_pFacets;
    MTGGraph *m_pGraph;

    MTGMask m_faceSeedMask;
    MTGMask m_edgeSeedMask;
    MTGMask m_vertexSeedMask;

    // unused - int m_numNode;
    int m_numExteriorFace;
    bool    m_bClosed;
    double  m_volume;

    // If node is already mapped to topology by given index array, return false.
    // Otherwise, create a new topology index of specified class, and set mask on the node.
    bool    mapNodeToNewTopology
            (
            MTGNodeId nodeId,
            EmbeddedIntArray *pIndexArray,
            int eClass,
            MTGMask seedMask,
            int *pIndex
            )
        {
        int index;
        // MTG_EXTERIOR_MASK test removed here!!!
        if (!jmdlEmbeddedIntArray_getInt (pIndexArray, &index, nodeId)
            || index >= 0)
            return false;
        *pIndex = addTopologicalEntity (eClass);
        jmdlMTGGraph_setMask (m_pGraph, nodeId, seedMask);
        return true;
        }


    bool    derefVertex
            (
            int nodeId,
            DPoint3d *pXYZ,
            int *pTag
            )
        {
        int vertexIndex;
        if (jmdlEmbeddedIntArray_getInt (m_pNodeIdToVertexIndexArray, &vertexIndex, nodeId)
            && m_pTagArray != NULL
            && vertexIndex >= 0
            && vertexIndex < jmdlEmbeddedIntArray_getCount (m_pEntityClassArray))
            {
            *pTag = m_pTagArray [vertexIndex];
            jmdlMTGFacets_getNodeCoordinates (m_pFacets, pXYZ, nodeId);
            return true;
            }
        return false;
        }

    bool    derefEdge
            (
            int nodeId,
            int *pTag
            )
        {
        int index;
        if (   jmdlEmbeddedIntArray_getInt (m_pNodeIdToEdgeIndexArray, &index, nodeId)
           && m_pTagArray != NULL
           && index >= 0
           && index < jmdlEmbeddedIntArray_getCount (m_pEntityClassArray)
           )
            {
            *pTag = m_pTagArray [index];
            return true;
            }
        return false;
        }

    bool    derefFace
            (
            int nodeId,
            int *pTag
            )
        {
        int index;
        if (   jmdlEmbeddedIntArray_getInt (m_pNodeIdToFaceIndexArray, &index, nodeId)
           && m_pTagArray != NULL
           && index >= 0
           && index < jmdlEmbeddedIntArray_getCount (m_pEntityClassArray)
           )
            {
            *pTag = m_pTagArray [index];
            return true;
            }
        return false;
        }

    // assumes only m_pFacets and m_pGraph are set;
    // sets m_bClosed, m_volume and m_numExteriorFace;
    void getVolumeCharacteristics (bool    ensureNonnegativeVolume)
        {
        bvector<MTGNodeId>  faceSeeds;
        // unused - MTGNodeId           faceSeedId;
        // unused - int                 numFace;

        m_bClosed = false;
        m_volume = 0.0;
        m_numExteriorFace = 0;

        // count exterior faces
        m_pGraph->CollectFaceLoops (faceSeeds);
        for (MTGNodeId faceSeedId : faceSeeds)
            if (jmdlMTGGraph_getMask (m_pGraph, faceSeedId, MTG_EXTERIOR_MASK))
                m_numExteriorFace++;

        if (m_numExteriorFace <= 0)
            {
            jmdlMTGFacets_volume (m_pFacets, &m_volume);
            if (ensureNonnegativeVolume && m_volume < 0.0)
                {
                jmdlMTGFacets_reverseOrientation (m_pFacets);
                m_volume *= -1.0;
                }
            m_bClosed = true;
            }
        }

public:
    MTGPSD_TopologyBuilder (MTGFacets *pFacets)
        {
        m_pFacets = pFacets;
        m_pGraph = &pFacets->graphHdr;

        m_pEntityClassArray = jmdlEmbeddedIntArray_grab ();
        m_pParentArray = jmdlEmbeddedIntArray_grab ();
        m_pChildArray = jmdlEmbeddedIntArray_grab ();
        m_pSenseArray = jmdlEmbeddedIntArray_grab ();

        m_pNodeIdToFaceIndexArray = jmdlEmbeddedIntArray_grab ();
        m_pNodeIdToVertexIndexArray = jmdlEmbeddedIntArray_grab ();
        m_pNodeIdToEdgeIndexArray = jmdlEmbeddedIntArray_grab ();

        m_vertexSeedMask = jmdlMTGGraph_grabMask (m_pGraph);
        m_faceSeedMask = jmdlMTGGraph_grabMask (m_pGraph);
        m_edgeSeedMask = jmdlMTGGraph_grabMask (m_pGraph);
        m_pTagArray = NULL;

        // TR #164577: reverse facet set here b/c reversing in buildRelations doesn't work
        getVolumeCharacteristics (true);
#ifdef CompileMTGPrint
        if (s_debug >= 1000)
            jmdlMTGGraph_printFaceLoops (m_pGraph);
#endif
        }

    ~MTGPSD_TopologyBuilder ()
        {
        jmdlEmbeddedIntArray_drop (m_pEntityClassArray);
        jmdlEmbeddedIntArray_drop (m_pParentArray);
        jmdlEmbeddedIntArray_drop (m_pChildArray);
        jmdlEmbeddedIntArray_drop (m_pSenseArray);
        jmdlEmbeddedIntArray_drop (m_pNodeIdToVertexIndexArray);
        jmdlEmbeddedIntArray_drop (m_pNodeIdToFaceIndexArray);
        jmdlEmbeddedIntArray_drop (m_pNodeIdToEdgeIndexArray);

        jmdlMTGGraph_dropMask (m_pGraph, m_faceSeedMask);
        jmdlMTGGraph_dropMask (m_pGraph, m_edgeSeedMask);
        jmdlMTGGraph_dropMask (m_pGraph, m_vertexSeedMask);

        if (m_pTagArray)
            delete [] m_pTagArray;
        }


    void addRelation (int parentIndex, int childIndex, int sense = PK_TOPOL_sense_none_c)
        {
        jmdlEmbeddedIntArray_addInt (m_pParentArray, parentIndex);
        jmdlEmbeddedIntArray_addInt (m_pChildArray, childIndex);
        jmdlEmbeddedIntArray_addInt (m_pSenseArray, sense);
        if (s_debug >= 10)
            {
            int parentClass, childClass;
            jmdlEmbeddedIntArray_getInt (m_pEntityClassArray, &parentClass, parentIndex);
            jmdlEmbeddedIntArray_getInt (m_pEntityClassArray, &childClass,  childIndex);
            printf (" (%s:%3d,%s:%3d,%s)\n",
                        tokenToString (parentClass), parentIndex,
                        tokenToString (childClass), childIndex,
                        tokenToString (sense));
            }
        }

    int addTopologicalEntity (int eClass)
        {
        int index = jmdlEmbeddedIntArray_getCount (m_pEntityClassArray);
        jmdlEmbeddedIntArray_addInt (m_pEntityClassArray, eClass);
        if (s_debug >= 10)
            printf (" %3d:%s\n", index, tokenToString (eClass));
        return index;
        }


    // Establish the index and count structure:
    // ** m_pNodeIdToFaceIndexArray gives the node's face index in the class array
    // ** m_pNodeIdToVertexIndexArray gives the node's vertex index in the class array
    // ** m_pNodeIdToEdgeIndexArray gives the node's edge index in the class array
    // ** For each edge, exactly one node id is marked as its seed node.
    // ** For each face, exactly one node id is marked as its seed node.
    // ** For each vertex, exactly one node id is marked as its seed node.
    // ** Global edge, face, vertex, and exterior edge counts
    void buildClassIndexArrays ()
        {
        int m_numNode = jmdlMTGGraph_getNodeIdCount (m_pGraph);
        jmdlEmbeddedIntArray_setConstant (m_pNodeIdToEdgeIndexArray, -1, m_numNode);
        jmdlEmbeddedIntArray_setConstant (m_pNodeIdToVertexIndexArray, -1, m_numNode);
        jmdlEmbeddedIntArray_setConstant (m_pNodeIdToFaceIndexArray, -1, m_numNode);
        jmdlMTGGraph_clearMaskInSet (m_pGraph, m_faceSeedMask | m_vertexSeedMask | m_edgeSeedMask);
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            recordNodeIdAtVertex (nodeId);
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)

        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            recordNodeIdAtFace (nodeId);
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)

        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            recordNodeIdAtEdge (nodeId);
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }

    void recordNodeIdAtVertex (MTGNodeId nodeId)
        {
        int vertexIndex;
        if (!mapNodeToNewTopology
                        (
                        nodeId,
                        m_pNodeIdToVertexIndexArray,
                        PK_CLASS_vertex,
                        m_vertexSeedMask,
                        &vertexIndex
                        ))
            return;

        MTGARRAY_VERTEX_LOOP (currNodeId, m_pGraph, nodeId)
            {
            jmdlEmbeddedIntArray_setInt (m_pNodeIdToVertexIndexArray, vertexIndex, currNodeId);
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, m_pGraph, nodeId)
        jmdlMTGGraph_setMask (m_pGraph, nodeId, m_vertexSeedMask);
        }

    void recordNodeIdAtFace (MTGNodeId nodeId)
        {
        int faceIndex;
        if (!mapNodeToNewTopology
                        (
                        nodeId,
                        m_pNodeIdToFaceIndexArray,
                        PK_CLASS_face,
                        m_faceSeedMask,
                        &faceIndex))
            return;

        MTGARRAY_FACE_LOOP (currNodeId, m_pGraph, nodeId)
            {
            jmdlEmbeddedIntArray_setInt (m_pNodeIdToFaceIndexArray, faceIndex, currNodeId);
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, m_pGraph, nodeId)
        jmdlMTGGraph_setMask (m_pGraph, nodeId, m_faceSeedMask);
        }

    void recordNodeIdAtEdge (MTGNodeId nodeId)
        {
        int edgeIndex;
        if (!mapNodeToNewTopology
                        (
                        nodeId,
                        m_pNodeIdToEdgeIndexArray,
                        PK_CLASS_edge,
                        m_edgeSeedMask,
                        &edgeIndex))
            return;

        jmdlEmbeddedIntArray_setInt (m_pNodeIdToEdgeIndexArray, edgeIndex, nodeId);

        MTGNodeId mateNodeId = jmdlMTGGraph_getEdgeMate (m_pGraph, nodeId);
        jmdlEmbeddedIntArray_setInt (m_pNodeIdToEdgeIndexArray, edgeIndex, mateNodeId);
        }

    void buildRelations (int bodyIndex, int shellIndex, bool    bReverseFaceLoops)
        {
        // BAD -- shell index should have sense to allow void.
        if (bodyIndex >= 0)
            addRelation (bodyIndex, shellIndex);

        // For each EDGE SEED ....
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            if (jmdlMTGGraph_getMask (m_pGraph, nodeId, m_edgeSeedMask))
                {
                int edgeIndex, vertexIndex0, vertexIndex1;
                // Make edge relations
                jmdlEmbeddedIntArray_getInt (m_pNodeIdToEdgeIndexArray, &edgeIndex, nodeId);
                /*MTGNodeId mateNodeId =*/ jmdlMTGGraph_getEdgeMate (m_pGraph, nodeId);
                jmdlEmbeddedIntArray_getInt (m_pNodeIdToVertexIndexArray, &vertexIndex0, nodeId);
                jmdlEmbeddedIntArray_getInt (m_pNodeIdToVertexIndexArray, &vertexIndex1,
                                                jmdlMTGGraph_getFSucc (m_pGraph, nodeId));
                addRelation (edgeIndex, vertexIndex0);
                addRelation (edgeIndex, vertexIndex1);
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)

        // For each FACE SEED ....
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            if (jmdlMTGGraph_getMask (m_pGraph, nodeId, m_faceSeedMask))
                {
                int faceIndex, loopIndex;
                // .... put the face in the shell
                jmdlEmbeddedIntArray_getInt (m_pNodeIdToFaceIndexArray, &faceIndex, nodeId);
                addRelation (shellIndex, faceIndex);
                // .... a loop in the face
                loopIndex = addTopologicalEntity (PK_CLASS_loop);
                addRelation (faceIndex, loopIndex);
                // .... and insert edges in the loop
                if (bReverseFaceLoops)
                    {
                    // ... going backwards
                    MTGNodeId currNodeId = nodeId;
                    do
                        {
                        int edgeIndex;
                        jmdlEmbeddedIntArray_getInt (m_pNodeIdToEdgeIndexArray, &edgeIndex, currNodeId);
                        addRelation (loopIndex, edgeIndex,
                                jmdlMTGGraph_getMask (m_pGraph, currNodeId, m_edgeSeedMask)
                                    ? PK_TOPOL_sense_negative_c : PK_TOPOL_sense_positive_c);
                        currNodeId = jmdlMTGGraph_getFPred (m_pGraph, currNodeId);
                        } while (currNodeId != nodeId);
                    }
                else
                    {
                    // .... or going forward
                    MTGARRAY_FACE_LOOP (currNodeId, m_pGraph, nodeId)
                        {
                        int edgeIndex;
                        jmdlEmbeddedIntArray_getInt (m_pNodeIdToEdgeIndexArray, &edgeIndex, currNodeId);
                        addRelation (loopIndex, edgeIndex,
                                jmdlMTGGraph_getMask (m_pGraph, currNodeId, m_edgeSeedMask)
                                    ? PK_TOPOL_sense_positive_c : PK_TOPOL_sense_negative_c);
                        }
                    MTGARRAY_END_FACE_LOOP (currNodeId, m_pGraph, nodeId)
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }

/*---------------------------------------------------------------------------------**//**
@param pIsClosed OUT true if closed body (no boundaries)
@param pVolume OUT volume.  0 if not closed.
* @bsimethod                                                    Earlin.Lutz     06/04
+---------------+---------------+---------------+---------------+---------------+------*/
    int go_pki (int *pBodyTag)
        {
        PK_BODY_fault_t fault;
        int faultIndex;
        int numTopol = jmdlEmbeddedIntArray_getCount (m_pEntityClassArray);
        m_pTagArray = new int [numTopol];

        int result = 1;

        if (m_bClosed)
            result = PK_BODY_create_solid_topology
                (
                numTopol,
                jmdlEmbeddedIntArray_getPtr (m_pEntityClassArray, 0),
                jmdlEmbeddedIntArray_getCount (m_pParentArray),
                jmdlEmbeddedIntArray_getPtr (m_pParentArray, 0),
                jmdlEmbeddedIntArray_getPtr (m_pChildArray, 0),
                jmdlEmbeddedIntArray_getPtr (m_pSenseArray, 0),
                pBodyTag,
                m_pTagArray,
                &fault,
                &faultIndex);
        else
            result = PK_BODY_create_sheet_topology
                (
                numTopol,
                jmdlEmbeddedIntArray_getPtr (m_pEntityClassArray, 0),
                jmdlEmbeddedIntArray_getCount (m_pParentArray),
                jmdlEmbeddedIntArray_getPtr (m_pParentArray, 0),
                jmdlEmbeddedIntArray_getPtr (m_pChildArray, 0),
                jmdlEmbeddedIntArray_getPtr (m_pSenseArray, 0),
                pBodyTag,
                m_pTagArray,
                &fault,
                &faultIndex);

        if (s_debug > 0)
            printf (" FAULT %d at index %d\n", fault, faultIndex);

        if (result != 0 || fault != PK_BODY_fault_no_fault_c)
            return 1;

        int vertexTag0, vertexTag1;
        int edgeTag, faceTag;

        // Create PSD POINT for each VERTEX ....
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            DPoint3d xyz;
            if (jmdlMTGGraph_getMask (m_pGraph, nodeId, m_vertexSeedMask))
                {
                if (!derefVertex (nodeId, &xyz, &vertexTag0))
                    return 1;
                // SEND TO PSD:
                //      (vertexTag0) has coordinates (xyz)
                PK_POINT_sf_t   point_sf;
                PK_POINT_t      pointTag;

                point_sf.position.coord[0] = xyz.x;
                point_sf.position.coord[1] = xyz.y;
                point_sf.position.coord[2] = xyz.z;

                if (PK_ERROR_no_errors == PK_POINT_create (&point_sf, &pointTag))
                    {
                    PK_VERTEX_attach_points (1, (PK_VERTEX_t*)&vertexTag0, &pointTag);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)

        // Create PSD LINE for each EDGE ....
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            DPoint3d xyz0, xyz1;
            if (jmdlMTGGraph_getMask (m_pGraph, nodeId, m_edgeSeedMask))
                {
                if (  !derefVertex (nodeId, &xyz0, &vertexTag0)
                   || !derefVertex (jmdlMTGGraph_getFSucc (m_pGraph, nodeId), &xyz1, &vertexTag1)
                   || !derefEdge (nodeId, &edgeTag)
                   )
                   return 1;
                // SEND TO PSD:
                //      (edgeTag) has coordinates (xyz)
                PK_LINE_sf_t        line_sf;
                PK_LINE_t           lineTag;

                line_sf.basis_set.location.coord[0]  = xyz0.x;
                line_sf.basis_set.location.coord[1]  = xyz0.y;
                line_sf.basis_set.location.coord[2]  = xyz0.z;

                xyz1.Subtract (xyz0);
                xyz1.Normalize ();
                line_sf.basis_set.axis.coord[0]      = xyz1.x;
                line_sf.basis_set.axis.coord[1]      = xyz1.y;
                line_sf.basis_set.axis.coord[2]      = xyz1.z;

                if (PK_ERROR_no_errors == PK_LINE_create (&line_sf, &lineTag))
                    {
                    PK_EDGE_attach_curves (1, (PK_EDGE_t *) &edgeTag, (PK_CURVE_t *) &lineTag);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)

#define MAX_PER_FACE 1000
        DPoint3d xyzArray[MAX_PER_FACE];

        // Create PSD PLANE for each face ....
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            DPoint3d currXYZ;
            if (!jmdlMTGGraph_getMask (m_pGraph, nodeId, m_faceSeedMask))
                {
                // not a seed, ignore it..
                }
            else if (jmdlMTGGraph_getMask (m_pGraph, nodeId, MTG_EXTERIOR_MASK))
                {
                if (!derefFace (nodeId, &faceTag))
                    return 1;
                // LU HAN --- delete the faceTag here!!
                PK_FACE_delete_from_sheet_body (faceTag);
                }
            else
                {
                int numVertex = 0;
                //  copy out up to MAX_PER_FACE coordinates...
                MTGARRAY_FACE_LOOP (currNodeId, m_pGraph, nodeId)
                    {
                    if (numVertex < MAX_PER_FACE
                        && derefVertex (currNodeId, &currXYZ, &vertexTag0))
                        {
                        xyzArray[numVertex++] = currXYZ;
                        }
                    }
                MTGARRAY_END_FACE_LOOP (currNodeId, m_pGraph, nodeId)
                // SEND TO PSD:
                //      Compute plane origin and normal from xyzArray[0..numVertex]
                //      Make PSD plane, attach to face
                if (!derefFace (nodeId, &faceTag))
                    return 1;

                DVec3d normal0, xVec, yVec, zVec;
                DPoint3d basePt;
                // "best fit" plane normal ...
                bsiGeom_polygonNormal (&normal0, &basePt, xyzArray, numVertex);
                normal0.Normalize ();
                // use the best fit to trigger sequence of vectors that favor first edge ...
                xVec.NormalizedDifference(xyzArray[1], xyzArray[0]);
                yVec.NormalizedCrossProduct (normal0, xVec);
                zVec.NormalizedCrossProduct (xVec, yVec);

                PK_PLANE_sf_t   plane_sf;
                PK_PLANE_t      planeTag = PK_ENTITY_null;
                PK_LOGICAL_t    senseTag = PK_LOGICAL_true;

                plane_sf.basis_set.location.coord[0] = basePt.x;
                plane_sf.basis_set.location.coord[1] = basePt.y;
                plane_sf.basis_set.location.coord[2] = basePt.z;

                plane_sf.basis_set.axis.coord[0] = zVec.x;
                plane_sf.basis_set.axis.coord[1] = zVec.y;
                plane_sf.basis_set.axis.coord[2] = zVec.z;

                plane_sf.basis_set.ref_direction.coord[0] = xVec.x;
                plane_sf.basis_set.ref_direction.coord[1] = xVec.y;
                plane_sf.basis_set.ref_direction.coord[2] = xVec.z;

                if (PK_ERROR_no_errors == PK_PLANE_create (&plane_sf, &planeTag))
                    {
                    PK_FACE_attach_surfs (1, (PK_FACE_t *) &faceTag, (PK_SURF_t *) &planeTag, &senseTag);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)

        return 0;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/97
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   BodyFromMTGFacets
(
int                 *pBodyTag,          // <= tag of created body
MTGFacets *         pFacets
)
    {
    // ASSUME the facets are a single shell ...
    MTGPSD_TopologyBuilder builder = MTGPSD_TopologyBuilder (pFacets);

    int bodyIndex  = builder.addTopologicalEntity (PK_CLASS_body);
    int shellIndex = builder.addTopologicalEntity (PK_CLASS_shell);

    builder.buildClassIndexArrays ();

    // TR #164577: we reversed the facet set in the ctor; reversing in buildRelations doesn't work
    builder.buildRelations (bodyIndex, shellIndex, false);

    // Destructor frees various arrays and masks.
    if (0 != builder.go_pki (pBodyTag))
        return ERROR;

    PK_TOPOL_delete_redundant (*pBodyTag);

    return SUCCESS;
    }

static BentleyStatus BodyFromPolyface_byConnectivity
(
PK_BODY_t &bodyTag,
PolyfaceQueryCR polyface,
TransformCR dgnToSolid
)
    {
    BentleyStatus status = ERROR;
    MTGFacetsP mtgFacets = jmdlMTGFacets_new ();

    if (PolyfaceToMTG_FromPolyfaceConnectivity (mtgFacets, polyface, true))
        {
        jmdlMTGFacets_transform (mtgFacets, &dgnToSolid, false);
        status = BodyFromMTGFacets (&bodyTag, mtgFacets);
        }
    jmdlMTGFacets_free (mtgFacets);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/97
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidGeom::BodyFromPolyface(PK_BODY_t& bodyTag, PolyfaceQueryCR polyface, TransformCR dgnToSolid)
    {
    static double s_relTol = 1.0e-10;
    static double s_absTol = 1.0e-12;
    BentleyStatus status = BodyFromPolyface_byConnectivity (bodyTag, polyface, dgnToSolid);

    if (SUCCESS != status)
        {
        MTGFacetsP facets = jmdlMTGFacets_new ();

        if (PolyfaceToMTG (facets, NULL, NULL, polyface, true, s_absTol, s_relTol))
            {
            jmdlMTGFacets_transform (facets, &dgnToSolid, false);
            status = BodyFromMTGFacets (&bodyTag, facets);
            }

        jmdlMTGFacets_free (facets);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidGeom::BodyFromPolyface (IBRepEntityPtr& entityOut, PolyfaceQueryCR meshData, uint32_t nodeId)
    {
    if (meshData.GetPointCount () < 3)
        return ERROR;

    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    Transform   solidToDgn, dgnToSolid;

    PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, meshData.GetPointCP());

    PK_BODY_t   bodyTag;

    if (SUCCESS != PSolidGeom::BodyFromPolyface (bodyTag, meshData, dgnToSolid))
        return ERROR;

    if (nodeId)
        PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

    entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

    return SUCCESS;
    }

