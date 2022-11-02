/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Vu/VuMultiClip.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double sRelTol = 1.0e-9;


// Implementation of PolygonMarkedMultiClip with VU.
//
class VuMultiClip : public PolygonMarkedMultiClip
{
private:
VuSetP m_pGraph;
VuArrayP m_pFaceArray;
friend class VuMultiClipFactory;
int    m_debug;

static const int s_PRIMARY_BOUNDARY_MASK = VU_RULE_EDGE  ;

static const int s_PRIMARY_EXTERIOR_MASK = VU_EXTERIOR_EDGE  ;

static const int s_SECONDARY_BOUNDARY_MASK = VU_KNOT_EDGE;
static const int s_SECONDARY_EXTERIOR_MASK = VU_SEAM_EDGE ;
static const int s_SECONDARY_COMPOSITE_EXTERIOR_MASK = VU_GRID_EDGE ;

static const int s_CALLER_MARK_MASK = VU_DISCONTINUITY_EDGE ;
public:
VuMultiClip ()
    {
    m_pGraph = vu_newVuSet (0);
    m_pFaceArray = vu_grabArray (m_pGraph);
    vu_setTol (m_pGraph, 0.0, sRelTol); 
    m_debug = 0;
    }

GEOMAPI_VIRTUAL ~VuMultiClip ()
    {
    vu_returnArray (m_pGraph, m_pFaceArray);
    vu_freeVuSet (m_pGraph);
    }

void Initialize () override
    {
    vu_reinitializeVuSet (m_pGraph);
    }

void SetDebug (int level) override {m_debug = level;}
private:
void AddMarkedPolygon (DPoint3d *pXYZArray, bool *pMarkBit, int numXYZ,
    VuMask baseMask, VuMask markMask)
    {
    VuP pBase = NULL;
    VuP pLeft, pRight;
    while (numXYZ > 1
            && pXYZArray[0].IsEqual (pXYZArray[numXYZ-1]))
            numXYZ--;
    if (numXYZ < 3)
        return;
    for (int i = 0; i < numXYZ; i++)
        {
        vu_splitEdge (m_pGraph, pBase, &pLeft, &pRight);
        vu_setDPoint3d (pLeft,  &pXYZArray[i]);
        vu_setDPoint3d (pRight, &pXYZArray[i]);
        vu_setMask (pLeft, baseMask);
        vu_setMask (pRight, baseMask);
        // Mark mask may have been preserved by split -- it has to be
        // explicitly cleared.
        if (NULL != pMarkBit)
            vu_writeMask (pLeft, markMask, pMarkBit[i] ? 1 : 0);
        pBase = pLeft;
        }
    // transfer the mark bit to mates.
    VU_FACE_LOOP (pCurr, pBase)
        {
        vu_writeMask (vu_edgeMate (pCurr), markMask,
                vu_getMask (pCurr, markMask) ? 1 : 0);
        }
    END_VU_FACE_LOOP (pCurr, pBase)
    }
public:
bool AddBasePolygon (DPoint3d *pXYZArray, bool *pMarkBit, int numXYZ) override
    {
    AddMarkedPolygon (pXYZArray, pMarkBit, numXYZ,
                s_PRIMARY_BOUNDARY_MASK, s_CALLER_MARK_MASK);
    vu_mergeOrUnionLoops (m_pGraph, VUUNION_UNION);
    vu_regularizeGraph (m_pGraph);
    vu_parityFloodFromNegativeAreaFaces (m_pGraph,
                    s_PRIMARY_BOUNDARY_MASK, s_PRIMARY_EXTERIOR_MASK);
    if (m_debug > 3)
        vu_printFaceLabels (m_pGraph, "AddBasePolygon");
    return true;
    }

bool AddClipper (DPoint3d *pXYZArray, bool *pMarkBit, int numXYZ) override
    {
    vu_stackPush (m_pGraph);
    AddMarkedPolygon (pXYZArray, pMarkBit, numXYZ,
                        s_SECONDARY_BOUNDARY_MASK, s_CALLER_MARK_MASK);
    
    vu_mergeLoops (m_pGraph);
    vu_regularizeGraph (m_pGraph);

    vu_parityFloodFromNegativeAreaFaces (m_pGraph,
                        s_SECONDARY_BOUNDARY_MASK, s_SECONDARY_EXTERIOR_MASK);
    // Should just match, but do it for debugging....
    vu_windingFloodFromNegativeAreaFaces (m_pGraph,
                    s_SECONDARY_EXTERIOR_MASK, s_SECONDARY_COMPOSITE_EXTERIOR_MASK);

    if (m_debug > 3)
        vu_printFaceLabels (m_pGraph, "AddClipper");
    return false;
    }


void FinishClip (FaceMode faceMode) override
    {
    int initialStackDepth = vu_stackSize (m_pGraph);
    vu_stackPopNEntries (m_pGraph, initialStackDepth - 1);
    vu_clearMaskInSet (m_pGraph, s_PRIMARY_EXTERIOR_MASK);

    // Merge among all clippers ..
    vu_mergeOrUnionLoops (m_pGraph, VUUNION_UNION);
    vu_regularizeGraph (m_pGraph);
    // WARNING -- triangulation logic assumes usual BOUNDARY and EXTERIOR masks !!!
    if (m_debug > 3)
        vu_printFaceLabels (m_pGraph, "Clipper Union");

    vu_stackPopAll (m_pGraph);
    if (m_debug > 3)
        vu_printFaceLabels (m_pGraph, "Clipper Union Popped to Stock");
    vu_clearMaskInSet (m_pGraph, s_PRIMARY_EXTERIOR_MASK);
    vu_mergeOrUnionLoops (m_pGraph, VUUNION_UNION);
    vu_regularizeGraph (m_pGraph);

    vu_parityFloodFromNegativeAreaFaces (m_pGraph,
            s_PRIMARY_BOUNDARY_MASK, s_PRIMARY_EXTERIOR_MASK);

    if (faceMode == FaceMode_Default)
        {
        }
    else
        {
        vu_triangulateMonotoneInteriorFaces (m_pGraph, false);
        vu_flipTrianglesToImproveQuadraticAspectRatio (m_pGraph);
        if (faceMode == FaceMode_Convex)
            {
            vu_removeEdgesToExpandConvexFaces (m_pGraph, VU_BOUNDARY_EDGE);
            }
        }

    //vu_parityFloodFromNegativeAreaFaces (m_pGraph,
    //                    s_SECONDARY_BOUNDARY_MASK, s_SECONDARY_EXTERIOR_MASK);
    vu_windingFloodFromNegativeAreaFaces (m_pGraph,
                    s_SECONDARY_EXTERIOR_MASK, s_SECONDARY_COMPOSITE_EXTERIOR_MASK);

    if (m_debug > 0)
        vu_printFaceLabels (m_pGraph, "Final");
        
    }

void SetupForLoopOverFaces (bool primarySelect, bool clipperSelect) override
    {
    VuMask visitMask = vu_grabMask (m_pGraph);
    vu_clearMaskInSet (m_pGraph, visitMask);
    vu_arrayClear (m_pFaceArray);
    double totalArea = 0.0;
    double area;
    VU_SET_LOOP (pCurr, m_pGraph)
        {
        if (!vu_getMask (pCurr, visitMask))
            {
            vu_setMaskAroundFace (pCurr, visitMask);

            if (   (primarySelect == (0 == vu_getMask (pCurr, s_PRIMARY_EXTERIOR_MASK)))
                && (clipperSelect == (0 == vu_getMask (pCurr, s_SECONDARY_COMPOSITE_EXTERIOR_MASK))))
                {
                area = vu_area (pCurr);
                totalArea += area;
                vu_arrayAdd (m_pFaceArray, pCurr);
                }
            }
        }
    END_VU_SET_LOOP (pCurr, m_pGraph)
    vu_returnMask (m_pGraph, visitMask);
    vu_arrayOpen (m_pFaceArray);
    }

bool GetFace (DPoint3d *pXYZBuffer, bool *pMarkBits, int &numXYZ, int maxXYZ, bool repeatFirstPoint) override
    {
    numXYZ = 0;
    VuP pFaceSeed;
    if (!vu_arrayRead (m_pFaceArray, &pFaceSeed))
        return false;

    VU_FACE_LOOP (pCurr, pFaceSeed)
        {
        if (numXYZ < maxXYZ)
            {
            vu_getDPoint3d (&pXYZBuffer[numXYZ], pCurr);
            if (pMarkBits)
                {
                pMarkBits[numXYZ] = 0 != (vu_getMask (pCurr, s_CALLER_MARK_MASK));
                }
            numXYZ++;
            }
        }
    END_VU_FACE_LOOP (pCurr, pFaceSeed)

    if (repeatFirstPoint && numXYZ < maxXYZ)
        {
        pXYZBuffer[numXYZ] = pXYZBuffer[0];
        if (pMarkBits)
            pMarkBits[numXYZ] = pMarkBits[0];
        numXYZ++;
        }

    return true;
    }
};


PolygonMarkedMultiClipP VuMultiClipFactory::NewPolygonMarkedMultiClip ()
    {
    return new VuMultiClip ();
    }

void VuMultiClipFactory::FreePolygonMarkedMultiClip (PolygonMarkedMultiClipP pClipper)
    {
    delete pClipper;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
