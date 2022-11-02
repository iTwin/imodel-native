/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef abc

/*=================================================================================**//**
* lexical edge comparison by <low vertex, high vertex>, with indices assumed zero based.
+===============+===============+===============+===============+===============+======*/
struct EdgeSortIndices
{
MTGNodeId nodeId;
int vertexIndexA;
int vertexIndexB;

EdgeSortIndices (int vertexIndexA, int vertexIndexB)
    {
    }

void GetLowHigh (int &low, int &high)
    {
    if (vertexIndexA < vertexIndexB)
        {
        low = vertexIndexA;
        high = vertexIndexB;
        }
    else
        {
        low = vertexIndexB;
        high = vertexIndexA;
        }
    }

};

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static bool cb_LessThan_EdgeSortIndices (EdgeSortIndices const &edgeA, EdgeSortIndices const & edgeB)
    {
    int lowA, lowB, highA, highB;
    edgeA.GetLowHigh (lowA, highA);
    edgeB.GetLowHigh (lowB, highB);
    if (lowA < lowB)
        return true;
    if (lowA > lowB)
        return false;
    return highA < highB;
    }

bool PolyfaceToMTG_FromPolyfaceConnectivity
(
PolyfaceHeaderR  polyface,
MTGGraph *graph,
unsigned int readIndexLabel
)
    {
    // Build an MTG face loop for each face.
    // These loops have "exterior sides" that need to be eliminated.
    // 
    bvector <EdgeSortIndices> sortArray;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (source, true);
    visitor->SetNumWrap (1);
    bvector <size_t> &indexPosition = visitor->IndexPosition ();
    bvector <size_t> &pointIndex = visitor->PointIndex ();
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        size_t numEdge = visitor->NumEdgesThisFace ();
        MTGNodeId baseNodeId = MTG_NULL_NODEID;
        MTGNodeId leftNodeId, rightNodeId;
        for (size_t i = 0; i < numEdge; i++)
            {
            graph->Splitedge (baseNodeId, leftNodeId, rightNodeId);
            graph->SetMask (rightNodeId, MTG_EXTERIOR_MASK);
            sortArray.push_back (EdgeSortIndices (leftNodeId, pointIndex[i], pointIndex[i+1]);
            graph->SetLabel (leftNodeId, readIndexLabel, indexPosition[i]);
            graph->SetLabel (rightNodeId, readIndexLabel, indexPosition[i+1]);
            }
        }
        
    std::Sort (sortArray.begin (), sortArray.end (), cb_LessThan_EdgeSortIndices);
    

    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE


