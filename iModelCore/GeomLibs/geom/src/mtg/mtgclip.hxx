/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// An edge is classified relative to the section plane by an MTG_SectionEdgeClassification.
// This is in some situations viewed as an (unorderd) enumeration and in others
// as a bit map.  ABOVE and BELOW have separate bits that cannot be on together.  However,
// all combinations of 0 or 1 of those plus the ON bit are allowed, and the enumerated values
// turn end up covering all bit combinations from 0 to 5.  The MTG_SECTIONEDGE_SIDEBITS macro
// has both side bits set so a logical OR operation can extract whichever bit is set in a
// particular classification value.

#define MTG_SECTIONEDGE_ONBIT 0x01
#define MTG_SECTIONEDGE_ABOVEBIT 0x02
#define MTG_SECTIONEDGE_BELOWBIT 0x04

#define MTG_SECTIONEDGE_SIDEBITS (MTG_SECTIONEDGE_ABOVEBIT | MTG_SECTIONEDGE_BELOWBIT)
typedef enum
    {
    MTG_SectionEdge_Unclassified = 0,
    MTG_SectionEdge_On          = MTG_SECTIONEDGE_ONBIT,                            // Edge and face neighborhood are both ON
    MTG_SectionEdge_Above       = MTG_SECTIONEDGE_ABOVEBIT,                         // Entire edge is above
    MTG_SectionEdge_OnAbove    = MTG_SECTIONEDGE_ONBIT | MTG_SECTIONEDGE_ABOVEBIT,  // Edge is ON, face neighborhood is above
    MTG_SectionEdge_Below       = MTG_SECTIONEDGE_BELOWBIT,                         // Entire edge is below
    MTG_SectionEdge_OnBelow    = MTG_SECTIONEDGE_ONBIT | MTG_SECTIONEDGE_BELOWBIT   // Edge is ON, face neighborhood is below
    } MTG_SectionEdgeClassification;




/*----------------------------------------------------------------------+
| An instance of the OmdlMTG_MergeHandler class receives callbacks      |
| at key points during boolean ops.                                     |
|                                                                       |
| The goal of the split is for each FACE to be uniformly classified     |
| as IN or OUT, so that selected faces can be purged at the end.        |
|                                                                       |
| For CLIP INSIDE, things can be deleted as soon as each plane is       |
| processed.  Hence there is no need for long-term markup.              |
|                                                                       |
| For CLIP OUTSIDE, a marker bit is initially FALSE.  After each        |
| plane is processed, the OUT face set increases with each step.        |
| A single mask bit identifies whether each edges is part of a face     |
| in the marked set.                                                    |
+----------------------------------------------------------------------*/

class OmdlMTG_MergeHandler
    {
    public:
        OmdlMTG_MergeHandler
            (
            MTGFacets *pFacetHeader,
            MTGMask cutMask
            );

        GEOMAPI_VIRTUAL ~OmdlMTG_MergeHandler ();

        GEOMAPI_VIRTUAL void applyTransform(
        const Transform *pTransform
        );

        GEOMAPI_VIRTUAL bool    boolean_getLocalRange(
        DRange3d *pRange
        );

        GEOMAPI_VIRTUAL bool    boolean_getLocalFaceRange(
        DRange3d *pRange,
        MTGNodeId seedNodeId
        );

        GEOMAPI_VIRTUAL void splitFaceByLocalZ(
        MTGNodeId seedNodeId
        );

        GEOMAPI_VIRTUAL bool isShortCircuited(
        );

        GEOMAPI_VIRTUAL void startConvexSet(
        const DPoint4d *pHPlane,
        int      numPlane
        );

        GEOMAPI_VIRTUAL void endConvexSet(
        const DPoint4d *pHPlane,
        int      numPlane
        );

        GEOMAPI_VIRTUAL void pushClipType(
        MTGClipOp clipType
        );

        GEOMAPI_VIRTUAL MTGClipOp popClipType(
        );

        GEOMAPI_VIRTUAL void startPlaneOfConvexSet(
        const DPoint4d *pHPlane,
        int      numPlane,
        int      i
        );

        GEOMAPI_VIRTUAL void endPlaneOfConvexSet(
        const DPoint4d *pHPlane,
        int      numPlane,
        int      i
        );

        GEOMAPI_VIRTUAL void announceAllIn(
        );

        GEOMAPI_VIRTUAL void announceAllOut(
        );

        GEOMAPI_VIRTUAL void announceAllOn(
        );

        GEOMAPI_VIRTUAL void announceEdgeSplit(
        MTGNodeId   belowNodeId,     // The node below the section plane
        MTGNodeId   upNodeId,         // The (new) node pointing up from the section plane
        MTGNodeId   downNodeId        // The (new) node pointing down from the section plane
        );

        GEOMAPI_VIRTUAL void announceJoin(
        MTGNodeId   leftNodeId,       // The preexisting start node
        MTGNodeId   rightNodeId,     // The preexisting end node
        MTGNodeId   startNodeId,     // The new start node
        MTGNodeId   endNodeId,        // The new end node
        bool        markCut    // TRUE to invoke application of m_cutMask.
        );

        GEOMAPI_VIRTUAL void removeExtraNodes(
        );

        void    announceEdgeClassification
        (
        EmbeddedIntArray  *pEdgeClassification
        );

        // Indicates start of search for edges to be split.
        void startEdgeSplit
        (
        );

        // Default returns TRUE -- any edge is splitable.
        bool    isEdgeSplitable
        (
        MTGNodeId  baseNodeId      // base node of edge.
        );

    protected:

        MTGFacets *m_pFacetHeader;

        EmbeddedDPoint3dArray *m_pAltitudeHeader;     // (grabbed)
        EmbeddedIntArray *m_pFaceSeedNodeArray;  // (grabbed)

        MTGGraph  *m_pGraph;
        int         m_vertexLabelOffset;
        bool        m_isShortCircuited;
        MTGMask     m_opMask;                    // Mask to apply in operation-specific fashion.
        MTGMask     m_splitMask;         // Mask to apply to newly-split edge nodes
        MTGMask     m_joinMask;                  // Mask to apply to join edges.
        MTGMask     m_returnMask;                // Mask(s) to return to the graph at end.
        MTGMask     m_edgeSplitTransferMask;
        MTGMask     m_faceSplitTransferMask;
        MTGClipOp   m_clipType;
        MTGMask     m_cutMask;                   // Mask to apply to join edges that are ON.
    };


/*----------------------------------------------------------------------+
| An OmdlMTG_PolygonPunchHandler customizes sectioning so for the       |
| special case of punching a polygonal shape.                           |
+----------------------------------------------------------------------*/
class OmdlMTG_PolygonPunchHandler
     : public OmdlMTG_MergeHandler
    {
    public:
        OmdlMTG_PolygonPunchHandler
        (
        MTGFacets *pFacetHeader
        );

        GEOMAPI_VIRTUAL ~OmdlMTG_PolygonPunchHandler ();

        // Start the entire polygon
        StatusInt   boolean_startPolygon
        (
        const DPoint3d  *pPointArray,
              int       numPoint,
        const DPoint3d *pDirection
        );

        // Finish
        void    endPolygon
        (
        const DPoint3d  *pPointArray,
              int       numPoint,
        const DPoint3d *pDirection
        );

        // Start a particular edge
        bool    boolean_applyPolygonEdge
        (
              int i,                    // The edge index
        const DPoint3d  *pPointArray,
              int numPoint,
        const DPoint3d *pDirection
        );


    private:
        DPoint3d m_startPoint;
        DPoint3d m_unitNormal;
        DPoint3d m_edgeVector;
        double   m_alphaMin;
        double   m_alphaMax;
        double   planeTol;
        double   tangentTol;

        Transform m_sweepTransform;
    };


END_BENTLEY_GEOMETRY_NAMESPACE