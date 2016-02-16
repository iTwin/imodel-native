/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/vuPolygonClassifier.h $
|    $RCSfile: vuPolygonClassifier.h,v $
|   $Revision: 1.14 $
|       $Date: 2009/04/20 15:27:58 $
|     $Author: Earlin.Lutz $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//#include <STDVectorDPoint3d.h>
#include <Vu/vuApi.h>

typedef bvector<DPoint3d> vuVectorDPoint3d;
typedef vuVectorDPoint3d& vuVectorDPoint3dR;




#ifdef MSVUPRINT_AVAILABLE
#include <msvuprint.fdf>
#endif

#ifndef FaceModeDefined
    typedef enum
        {
        FaceMode_Unconnected,
        FaceMode_Monotone,
        FaceMode_Triangulate,
        FaceMode_BridgeEdges,
        FaceMode_Convex
        } VuFaceMode;
#endif


#define MASK_BOUNDARY_A VU_KNOT_EDGE
#define MASK_BOUNDARY_B VU_SEAM_EDGE
#define MASK_EXTERIOR_A VU_RULE_EDGE
#define MASK_EXTERIOR_B VU_GRID_EDGE



// Context manager to hold a vu graph and do polygon booleans on request.
//
// The following example code demonstrates
//      (1) Allocate a classifier context (on stack, so destructor is called on scope exit to free the graph)
//      (2) Referencing polygons stored as two arrays mXYZ[0] and mXYZ[1] with counts mCount[0] and mCount[1],
//          the classifier context is used to form (A intersect B), (A-B), (B-A), and (A union B).
//      (3) After each computation, there is a loop to retrieve the computed polygons.  Note that HOLES
//          appear as polygons with negative area.
//   --------------------------------------------------------------------------------------------
//            VuPolygonClassifier vu  = VuPolygonClassifier ();
//            DPoint3d xyz[MAX_VERTICES];
//            int n;
//            double areaA = 0.0;
//            double areaB = 0.0;
//            double areaAB = 0.0;
//            double areaUnion = 0.0;
//            double ai = 0.0;
//            vu.ClassifyAIntersectB (mXYZ[0], mCount[0], mXYZ[1], mCount[1]);
//            for (;vu.GetFace (xyz, n, MAX_VERTICES);)
//                {
//                drawPolygon (xyz, n, 4, 0);
//                areaAB += (ai = bsiGeom_getXYPolygonArea (xyz, n));
//                printf ("                      tile %15.7lf %s\n", ai, ai < 0.0 ? "HOLE" : "    ");
//                }
//            printf ("   areaAB    %15.7lf\n", areaAB);
//            vu.ClassifyAMinusB (mXYZ[0], mCount[0], mXYZ[1], mCount[1]);
//            for (;vu.GetFace (xyz, n, MAX_VERTICES);)
//                {
//                drawPolygon (xyz, n, 3, 0);
//                areaA += (ai = bsiGeom_getXYPolygonArea (xyz, n));
//                printf ("                      tile %15.7lf %s\n", ai, ai < 0.0 ? "HOLE" : "    ");
//                }
//            printf ("   areaA     %15.7lf\n", areaA);
//            vu.ClassifyAMinusB (mXYZ[1], mCount[1], mXYZ[0], mCount[0]);
//            for (;vu.GetFace (xyz, n, MAX_VERTICES);)
//                {
//                drawPolygon (xyz, n, 2, 0);
//                areaB += (ai = bsiGeom_getXYPolygonArea (xyz, n));
//                printf ("                      tile %15.7lf %s\n", ai, ai < 0.0 ? "HOLE" : "    ");
//                }
//            printf ("   areaB     %15.7lf\n", areaB);
//            vu.ClassifyAUnionB (mXYZ[1], mCount[1], mXYZ[0], mCount[0]);
//            for (;vu.GetFace (xyz, n, MAX_VERTICES);)
//                {
//                areaUnion += (ai = bsiGeom_getXYPolygonArea (xyz, n));
//                printf ("                      tile %15.7lf %s\n", ai, ai < 0.0 ? "HOLE" : "    ");
//                }
//            printf ("   areaUnion %15.7lf\n", areaUnion);
//            printf ("   error     %15.7lf\n", areaA + areaB + areaAB - areaUnion);
//            mNumPolygon = 0;
//            }
//    -----------------------------------------------------------------------------------
// Usage notes:
//    Polygons with holes may be entered by "disconnect" points
//   (Use bsiDPoint3d_initDisconnect(&xyz) or xyz.initDisconnect to initialize).
//
//    A polygon with n edges is passed as n+1 point array, with first point duplicated at end.
//
//    Holes appear as negative area faces.
//
class VuPolygonClassifier
{
public:
VuSetP mpGraph;
VuArrayP mpFaceArray;

public:

    VuFaceMode mFaceMode;
    int      mNoisy;
protected:


// Search an array for "disconnect" points separating multiple loops of a polygon with holes.
// Add each loop section to the vu graph.
// Each edge is marked with VU_BOUNDARY_EDGE and the extra mask ...
void InsertLoopsWithDisconnects (vuVectorDPoint3dR xyzIn, VuMask extraMask = 0)
    {
    int i1;
    int numXYZ = (int)xyzIn.size ();
    for (int i0 = 0; i0 < numXYZ - 1; i0 = ++i1)
        {
        i1 = i0;
        while (i1 < numXYZ && !bsiDPoint3d_isDisconnect (&xyzIn[i1]))
            i1++;
//        int numThisLoop = i1 - i0;
        VuP pFirstA = NULL;
        VuP pPreviousB = NULL;
        for (int i = i0 + 1; i < i1; i++)
            {
            VuP pNodeA, pNodeB;
            vu_makePair (mpGraph, &pNodeA, &pNodeB);
            vu_setDPoint3d (pNodeA, &xyzIn[i-1]);
            vu_setDPoint3d (pNodeB, &xyzIn[i]);
            vu_setMask (pNodeA, VU_BOUNDARY_EDGE | extraMask);
            vu_setMask (pNodeB, VU_BOUNDARY_EDGE | extraMask);
            if (pPreviousB == NULL)
                {
                pFirstA = pNodeA;
                }
            else
                {
                vu_vertexTwist (mpGraph, pPreviousB, pNodeA);
                }
            pPreviousB = pNodeB;
            }
        if (pFirstA != NULL && pPreviousB != NULL)
            {
            DPoint3d xyzEnd, xyzStart;
            vu_getDPoint3d (&xyzStart, pFirstA);
            vu_getDPoint3d (&xyzEnd, pPreviousB);            
            if (bsiDPoint3d_pointEqual (&xyzEnd, &xyzStart))
                {
                // First, last points match.  Just twist the dangling edges together..
                vu_vertexTwist (mpGraph, pPreviousB, pFirstA);
                }
            else
                {
                // Make a new edge to close the last-to-first gap ..
                VuP pNodeA, pNodeB;
                vu_makePair (mpGraph, &pNodeA, &pNodeB);
                vu_setDPoint3d (pNodeA, &xyzEnd);
                vu_setDPoint3d (pNodeB, &xyzStart);
                vu_setMask (pNodeA, VU_BOUNDARY_EDGE | extraMask);
                vu_setMask (pNodeB, VU_BOUNDARY_EDGE | extraMask);
                vu_vertexTwist (mpGraph, pPreviousB, pNodeA);
                vu_vertexTwist (mpGraph, pFirstA, pNodeB);
                }
            }
        }
    }

// Search an array for "disconnect" points separating multiple loops of a polygon with holes.
// Add each loop section to the vu graph.
void InsertEdgesWithDisconnects (DPoint3d *pXYZArray, void **pEdgeData, int numXYZ)
    {
    for (int i0 = 0; i0 < numXYZ - 1; i0++)
        {
        int i1 = i0 + 1;
        DPoint3d xyz0 = pXYZArray[i0];
        DPoint3d xyz1 = pXYZArray[i1];
        if (!bsiDPoint3d_isDisconnect (&xyz0)
            && !bsiDPoint3d_isDisconnect (&xyz1))
            {
            VuP pStart, pEnd;
            vu_makePair (mpGraph, &pStart, &pEnd);
            vu_setMask (pStart, VU_BOUNDARY_EDGE);
            vu_setMask (pEnd,   VU_BOUNDARY_EDGE);
            vu_setDPoint3d (pStart, &xyz0);
            vu_setDPoint3d (pEnd,   &xyz1);
            if (NULL != pEdgeData)
                {
                vu_setUserDataP (pStart, pEdgeData[i0]);
                vu_setUserDataP (pEnd,   pEdgeData[i1]);
                }
            }
        }
    }

int HandleVuCMessage (VuMessageType type, VuSetP pGraph, VuP pNodeA, VuP pNodeB, void *pArg)
    {
    if (type == VU_MESSAGE_ANNOUNCE_POST_SPLIT_EDGE)
        {
        VuP pPredA = vu_fpred (pNodeA);
        VuP pPredB = vu_fpred (pNodeB);
        void *pDataA = vu_getUserDataP (pPredA);
        void *pDataB = vu_getUserDataP (pPredB);
        vu_setUserDataP (pNodeA, pDataA);
        vu_setUserDataP (pNodeB, pDataB);
        }
    return SUCCESS;
    }

static int DispatchVuCMessage (VuMessageType type, VuSetP pGraph, VuP pNodeA, VuP pNodeB, void *pArg, VuPolygonClassifier *pClassifier)
    {
    return pClassifier->HandleVuCMessage (type, pGraph, pNodeA, pNodeB, pArg);
    }

private:
// ASSUME graph is triangulated.
// We want to expand to convex.
// The expander looks at EXTERIOR bits, it should look at BOUNDARY.
// Do save/restore sequence on the exteriors around the call ...
void CullTriangulatedToConvex ()
    {
    // SAVE ....
    VuMask savedExterior = vu_grabMask (mpGraph);
    VU_SET_LOOP (pCurr, mpGraph)
        {
        vu_writeMask (pCurr, savedExterior, vu_getMask (pCurr, VU_EXTERIOR_EDGE));
        vu_writeMask (pCurr, VU_EXTERIOR_EDGE, vu_getMask (pCurr, VU_BOUNDARY_EDGE));
        }
    END_VU_SET_LOOP (pCurr, mpGraph)
    // Remove edges ...
    vu_removeEdgesToExpandConvexInteriorFaces (mpGraph);
    // RESTORE ...
    VU_SET_LOOP (pCurr, mpGraph)
        {
        vu_writeMask (pCurr, VU_EXTERIOR_EDGE, vu_getMask (pCurr, savedExterior));
        }
    END_VU_SET_LOOP (pCurr, mpGraph)
    vu_returnMask (mpGraph, savedExterior);
    }
// Add or remove interior edges to support the face mode.
// Caller is ASSUMED to have regularized and marked exterior faces -- i.e. monotone faces
void ApplyFaceMode ()
    {
    if (mFaceMode == FaceMode_Monotone)
        {
        // Should be in place ...
        }
    else if (mFaceMode == FaceMode_Triangulate
            || mFaceMode == FaceMode_Convex)
        {
        vu_triangulateMonotoneInteriorFaces (mpGraph, false);
        vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
        if (mFaceMode == FaceMode_Convex)
            {
            CullTriangulatedToConvex ();
            }
        }
    else if (mFaceMode == FaceMode_BridgeEdges)
        {
        vu_expandFacesToBarrier (mpGraph, VU_BOUNDARY_EDGE);
        }
    }
public:
// Reset the face array for visits by the GetFace() function
void SetupForLoopOverInteriorFaces ()
    {
    vu_arrayClear (mpFaceArray);
    vu_collectInteriorFaceLoops (mpFaceArray, mpGraph);
    vu_arrayOpen (mpFaceArray);
    }

// Constructor.  Create vu graph and face array to be reused ...
VuPolygonClassifier (double graphAbsTol = 0, double graphRelTol = 1.0e-7)
    {
    mpGraph = vu_newVuSet (0);
    mpFaceArray = vu_grabArray (mpGraph);
    vu_setTol (mpGraph, graphAbsTol, graphRelTol);
    vu_setCMessageFunction (mpGraph, (VuMessageFunction)DispatchVuCMessage, this);
    vu_setUserDataPIsVertexProperty (mpGraph, false);
    mNoisy = 0;
    mFaceMode = FaceMode_BridgeEdges;
    }

// Set default value of user data in nodes.
// If not called, the default is a null pointer.
// If your user data is integer indices, you probably need to call this function
// and change the default to ((void*)-1) so that 0 can be distinguished as a valid user data 
void SetDefaultEdgeData (void *pDefaultEdgeData)
    {
    vu_setDefaultUserDataPAsInt (mpGraph, (int)0);
    }
// Destructor.  Dispose of array and graph ...
~VuPolygonClassifier ()
    {
    vu_returnArray (mpGraph, mpFaceArray);
    vu_freeVuSet (mpGraph);
    }

// Perform a planar merge of all edges in the array. (Disconnects as separators)
// On exit, the face array is prepped for looping over all faces with GetFace ()
void ClassifyPlane (DPoint3d *pXYZArrayA, void **pEdgeData, int numXYZA)
    {
    vu_reinitializeVuSet (mpGraph);
    InsertEdgesWithDisconnects (pXYZArrayA, pEdgeData, numXYZA);
    vu_mergeOrUnionLoops (mpGraph, VUUNION_UNION);
    vu_regularizeGraph (mpGraph);
    vu_floodFromNegativeAreaFaces (mpGraph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    ApplyFaceMode ();
    SetupForLoopOverInteriorFaces ();
    }

// <param "bAOutB">If true, retain faces of A outside of B</param>
// <param "bBoutA">If true, retain faces of B outside of A</param>
// <param "bAB">If true, return faces of A intersect B</param>
// <param "bMergeFaces">If true, elide boundary markings on edges that separate retained among retained faces.
//       When doing a union or subtraction in FaceMode_BridgeEdges, this step makes largest possible faces.
//      When doing union or subtraction in FaceMode_Triangulate, this allows the triangulation to
//      "flip" edges to go across original edges that are no longer boundaries after the boolean.
//              </param>
void ClassifyAB (vuVectorDPoint3dR xyzA, vuVectorDPoint3dR xyzB,
    bool bKeepAOutB,
    bool bKeepBOutA,
    bool bKeepAB,
    bool bMergeFaces
    )
    {
    vu_reinitializeVuSet (mpGraph);
    InsertLoopsWithDisconnects (xyzA, MASK_BOUNDARY_A);
    InsertLoopsWithDisconnects (xyzB, MASK_BOUNDARY_B);
    //printMarkup (mpGraph, "RAW LOOPS");
    vu_mergeOrUnionLoops (mpGraph, VUUNION_UNION);
    // Always connect ...
    vu_regularizeGraph (mpGraph);

    vu_clearMaskInSet (mpGraph, MASK_EXTERIOR_A | MASK_EXTERIOR_B);
    if (mFaceMode == FaceMode_Triangulate)
        {
        // Triangulation does not preserve the classification masks.
        // Premark global exterior -- we'll clear it before leaving this block
        vu_floodFromNegativeAreaFaces (mpGraph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
        vu_triangulateMonotoneInteriorFaces (mpGraph, false);
        vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
        vu_clearMaskInSet (mpGraph, VU_EXTERIOR_EDGE);
        }

    //printMarkup (mpGraph, "START PARITY");
    vu_markExteriorByParity (mpGraph, MASK_BOUNDARY_A, MASK_EXTERIOR_A);
    //printMarkup (mpGraph, "A MARKED");
    vu_markExteriorByParity (mpGraph, MASK_BOUNDARY_B, MASK_EXTERIOR_B);
    //printMarkup (mpGraph, "B MARKED");

    VuMask visitMask = vu_grabMask (mpGraph);
    vu_clearMaskInSet (mpGraph, visitMask | VU_EXTERIOR_EDGE);
    VU_SET_LOOP (pCurr, mpGraph)
        {
        if (!vu_getMask (pCurr, visitMask))
            {
            bool bA = !vu_getMask (pCurr, MASK_EXTERIOR_A);
            bool bB = !vu_getMask (pCurr, MASK_EXTERIOR_B);
            //double areas = vu_area (pCurr);
            bool bKeepThisFace = true;
            if (bA && bB)
                {
                bKeepThisFace = bKeepAB;
                }
            else if (bA)
                {
                bKeepThisFace = bKeepAOutB;
                }
            else if (bB)
                {
                bKeepThisFace = bKeepBOutA;
                }
            else
                {
                bKeepThisFace = false;
                }

            if (!bKeepThisFace)
                vu_setMaskAroundFace (pCurr, VU_EXTERIOR_EDGE);
            vu_setMaskAroundFace (pCurr, visitMask);
            }
        }
    END_VU_SET_LOOP (pCurr, mpGraph)
    vu_returnMask (mpGraph, visitMask);

    if (bMergeFaces)
        {
        VuMask clearMask = VU_BOUNDARY_EDGE | MASK_BOUNDARY_A | MASK_BOUNDARY_B | MASK_EXTERIOR_A | MASK_EXTERIOR_B;
        VU_SET_LOOP (pCurr, mpGraph)
            {
            VuP pMate = vu_edgeMate (pCurr);
            if (    vu_getMask (pCurr, VU_BOUNDARY_EDGE)
                &&  vu_getMask (pMate, VU_BOUNDARY_EDGE)
                && !vu_getMask (pCurr, VU_EXTERIOR_EDGE)
                && !vu_getMask (pMate, VU_EXTERIOR_EDGE)
                )
                {
                vu_clrMask (pCurr, clearMask);
                vu_clrMask (pMate, clearMask);
                }
            }
        END_VU_SET_LOOP (pCurr, mpGraph)
        }


    if (mFaceMode == FaceMode_Triangulate)
        {
        // Flip the triangles again to allow buried edges to move.
        vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
        }


    if (mFaceMode == FaceMode_BridgeEdges)
        {
        vu_expandFacesToBarrier (mpGraph, VU_BOUNDARY_EDGE);
        vu_deleteDanglingEdges (mpGraph);
        }
    }

void SetupForLoopOverBooleanSubsetOfFaces (bool bAOnly, bool bBOnly, bool bAB)
    {
    vu_arrayClear (mpFaceArray);
    VuMask visitMask = vu_grabMask (mpGraph);
    vu_clearMaskInSet (mpGraph, visitMask);
    VU_SET_LOOP (pCurr, mpGraph)
        {
        if (!vu_getMask (pCurr, visitMask))
            {
            vu_setMaskAroundFace (pCurr, visitMask);
            if (!vu_getMask (pCurr, MASK_EXTERIOR_A))
                {
                if (!vu_getMask (pCurr, MASK_EXTERIOR_B))
                    {
                    if (bAB)
                        vu_arrayAdd (mpFaceArray, pCurr);
                    }
                else
                    {
                    if (bAOnly)
                        vu_arrayAdd (mpFaceArray, pCurr);
                    }
                }
            else if (!vu_getMask (pCurr, MASK_EXTERIOR_B))
                {
                if (bBOnly)
                    vu_arrayAdd (mpFaceArray, pCurr);
                }
            }
        }
    END_VU_SET_LOOP (pCurr, mpGraph)
    vu_arrayOpen (mpFaceArray);
    vu_returnMask (mpGraph, visitMask);
    }

// Analyze two polygons A and B.
// On exit, the face array is prepped for looping over faces (A Intersect B) with GetFace ()
void ClassifyAIntersectB (vuVectorDPoint3dR xyzA, vuVectorDPoint3dR xyzB)
    {
    ClassifyAB (xyzA, xyzB, false, false, true, false);
    SetupForLoopOverInteriorFaces  ();
    }



// Analyze two polygons A and B.
// On exit, the face array is prepped for looping over faces (A Union B) with GetFace ()
void ClassifyAUnionB (vuVectorDPoint3dR xyzA, vuVectorDPoint3dR xyzB)
    {
    ClassifyAB (xyzA, xyzB, true, true, true, true);
    SetupForLoopOverInteriorFaces  ();
    }


// Analyze two polygons A and B.
// On exit, the face array is prepped for looping over faces (A Minus B) with GetFace ()
void ClassifyAMinusB (vuVectorDPoint3dR xyzA, vuVectorDPoint3dR xyzB)
    {
    ClassifyAB (xyzA, xyzB, true, false, false, true);
    SetupForLoopOverInteriorFaces  ();
    }

// Analyze two polygons A and B.
// On exit, the face array is prepped for looping over faces (B Minus A) with GetFace ()
void ClassifyBMinusA (vuVectorDPoint3dR xyzA, vuVectorDPoint3dR xyzB)
    {
    ClassifyAB (xyzA, xyzB, false, true, false, true);
    SetupForLoopOverInteriorFaces  ();
    }

// Retrieve coordinates the "next" face of the graph.
// This is to be called in a loop
//     for (;GetFace (xyz, edgeData, num, max);)
//          {
//          for (int i = 0; i < num; i++)
//              {
//              .. vertex coordinates xyz[i]
//              .. with edgeData[i]
//          }
//  Once the faces are visited, use SetupForLoopOverInteriorFaces to reset for additional visits.
bool GetFace (vuVectorDPoint3dR xyzOut, bvector <void *> * pEdgeDataArray)
    {
    xyzOut.clear ();
    if (pEdgeDataArray)
        pEdgeDataArray->clear ();
    VuP pFaceSeed;
    // We're going to ignore faces with just 2 edges or zero area ..
    //   have to be in a loop even though we expect to "usually" read
    //   only once.
    for (;vu_arrayRead (mpFaceArray, &pFaceSeed);)
        {
        VU_FACE_LOOP (pCurr, pFaceSeed)
            {
            DPoint3d xyz;
            vu_getDPoint3d (&xyz, pCurr);
            xyzOut.push_back (xyz);
            if (NULL != pEdgeDataArray)
                pEdgeDataArray->push_back (vu_getUserDataP (pCurr));
            }
        END_VU_FACE_LOOP (pCurr, pFaceSeed)

        if (xyzOut.size () > 2 && vu_area (pFaceSeed) != 0.0)
            {
            DPoint3d xyz = xyzOut[0];
            xyzOut.push_back (xyz);
            if (pEdgeDataArray != NULL)
                pEdgeDataArray->push_back (NULL);
            return true;
            }
        if (pEdgeDataArray)
            pEdgeDataArray->clear ();
        xyzOut.clear ();
        }
    return false;
    }

// Retrieve the "next" face of the graph.
// This is to be called in a loop
//     for (;GetFace (xyz, num, max);)
//          {
//          }
//  Once the faces are visited, use SetupForLoopOverInteriorFaces to reset for additional visits.
bool GetFace (vuVectorDPoint3dR xyz)
    {
    return GetFace (xyz, NULL);
   }


};

