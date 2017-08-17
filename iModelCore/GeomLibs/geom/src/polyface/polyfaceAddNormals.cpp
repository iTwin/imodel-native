/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/polyfaceAddNormals.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <assert.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct ApproximateVertexNormalContext
{
MTGGraphP graph;
MTGFacetsP oldFacets;
MTGMask barrierEdgeMask;
MTGMask visitMask;
int     readIndexLabel;
PolyfaceHeaderR mesh;

ApproximateVertexNormalContext (PolyfaceHeaderR mesh1)
    : mesh (mesh1)
    {
    oldFacets = jmdlMTGFacets_new ();
    graph = jmdlMTGFacets_getGraph (oldFacets);
    barrierEdgeMask = graph->GrabMask ();
    visitMask       = graph->GrabMask ();
    readIndexLabel = -1;
    }
    
~ApproximateVertexNormalContext ()
    {
    graph->DropMask (barrierEdgeMask);
    graph->DropMask (visitMask);
    jmdlMTGFacets_free (oldFacets);    
    }

bool TryGetNormal (MTGNodeId nodeId, DVec3dR normal)
    {
    int readIndex;
    return graph->TryGetLabel (nodeId, readIndexLabel, readIndex)
        && mesh.TryGetNormalAtReadIndex ((size_t)readIndex, normal);
    }

bool TryGetNormalIndex (MTGNodeId nodeId, bvector<int> const &normalIndexArray, int &normalIndex)
    {
    int readIndex;
    if (graph->TryGetLabel (nodeId, readIndexLabel, readIndex)
        && (size_t)readIndex < normalIndexArray.size()
        )
        {
        normalIndex = normalIndexArray[(size_t)readIndex];
        return true;
        }
    return false;
    }

bool TrySetVisibility (MTGNodeId nodeId, bool visible)
    {
    int readIndex;
    if (   graph->TryGetLabel (nodeId, readIndexLabel, readIndex)
        && (size_t)readIndex < mesh.PointIndex ().size()
        )
        {
        int s = visible ? 1 : -1;
        mesh.PointIndex ()[readIndex] = s * abs (mesh.PointIndex ()[readIndex]);
        }
    return false;
    }

void Report (size_t num0, size_t numInterior, size_t numHidden)
    {
    }
bool ComputeFaceNormals (double maxSingleEdgeAngle, double maxAccumulatedAngle, bool markTransitionsVisible)
    {
    bvector<DVec3d> newNormal;
    // Complete copy of prior indices ...
    bvector<int>    newNormalIndex = mesh.NormalIndex ();
    // Make existing indices negative so we can see if they are not changed ...
    // (but 0 remains zero -- these are one-based indices)
    for (size_t i = 0, n = newNormalIndex.size (); i < n; i++)
        newNormalIndex[i] = - abs (newNormalIndex[i]);
        
    bvector<MTGNodeId> baseNodes;
    graph->ClearMask (barrierEdgeMask);

    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        MTGNodeId nodeB = graph->FSucc (nodeA);
        MTGNodeId nodeC = graph->VSucc (nodeB);
        MTGNodeId nodeD = graph->FSucc (nodeC);
        DVec3d normalA, normalD;
        bool isBarrier = false;
        if (graph->GetMaskAt (nodeA, MTG_EXTERIOR_MASK))
            {   // exterior side of boundary
            isBarrier = true;   // but do NOT record as base !!!
            }
        else if (graph->GetMaskAt (nodeC, MTG_EXTERIOR_MASK))
            {   // interior side of boundary
            isBarrier = true;
            baseNodes.push_back (nodeA);
            }
        else
            {   // interior edge
            if (!TryGetNormal (nodeA, normalA)
                || !TryGetNormal (nodeD, normalD))
                return false;
            double theta = normalA.AngleTo (normalD);
            if (theta > maxSingleEdgeAngle)
                {
                isBarrier = true;
                baseNodes.push_back (nodeA);
                }
            }
        if (isBarrier)
            graph->SetMaskAt (nodeA, barrierEdgeMask);

        }
    MTGARRAY_END_SET_LOOP (nodeA, graph)


    // Find purely interior vertex loops with no barriers -- record a representaitve as a baseNode.
    graph->ClearMask (visitMask);
    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        if (!graph->GetMaskAt (nodeA, visitMask))
            {
            // this is the first visit to this vertex.
            graph->SetMaskAroundVertex (nodeA, visitMask);
            if (0 == graph->CountMaskAroundVertex (nodeA, barrierEdgeMask))
                baseNodes.push_back (nodeA);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA, graph)
        
    
    DVec3d accumulatedNormal;
    bvector<DVec3d> extendedSectorNormal;
    bvector<MTGNodeId> extendedSectorNode;
    for (size_t i = 0, n = baseNodes.size (); i < n; i++)
        {
        // Walk forward to accumulate the candidate nodes around this vertex ...
        extendedSectorNode.clear ();
        extendedSectorNormal.clear ();
        MTGNodeId nodeId = baseNodes[i];
        MTGNodeId nodeId0 = nodeId;
        do {
            DVec3d normal;
            if (!TryGetNormal (nodeId, normal))
                return false;   // should never happen
            extendedSectorNormal.push_back (normal);
            extendedSectorNode.push_back (nodeId);
            nodeId = graph->VSucc (nodeId);
            } while (nodeId != nodeId0 && !graph->GetMaskAt (nodeId, barrierEdgeMask));
            
            
            
        size_t numSector = extendedSectorNode.size ();
        for (size_t k0 = 0; k0 < numSector; )
            {
            double accumulatedAngle = 0.0;
            accumulatedNormal = extendedSectorNormal[k0];
            size_t k1 = k0 + 1;
            for (; k1 < numSector;)
                {
                DVec3d normal1 = extendedSectorNormal[k1];
                double delta = extendedSectorNormal[k1-1].AngleTo (normal1);
                accumulatedAngle += delta;
                accumulatedNormal.Add (normal1); // NEEDS WORK -- weight by sector size?
                k1++;
                }
            // [k0..k1-1] is a sequence of {k1-k0) sectors that share a normal.
            // crate the new normal, index back to it, and mark the graph edge as a transition.
            DVec3d averageNormalA, averageNormalB;
            averageNormalA.Normalize (accumulatedNormal);    // don't have to divide by count -- normalize 
            averageNormalB.Scale (accumulatedNormal, 1.0 / (double)numSector);
            double dot = averageNormalA.DotProduct (averageNormalB);
            static double s_averageNormalDot = 0.92;
            if (dot > s_averageNormalDot && accumulatedAngle < maxAccumulatedAngle) // QV cutoff for 
                {
                newNormal.push_back (averageNormalA);
                graph->SetMaskAt (extendedSectorNode [k0], barrierEdgeMask);
                size_t oneBasedNormalIndex = newNormal.size ();

                for (size_t k = k0; k < k1; k++)
                    {
                    int readIndexI;
                    if (!graph->TryGetLabel (extendedSectorNode[k], readIndexLabel, readIndexI))
                        return false;
                    size_t readIndex = (size_t)readIndexI;
                    if (readIndex >= newNormalIndex.size ())
                        return false;
                    newNormalIndex[readIndex] = static_cast<int>(oneBasedNormalIndex);
                    }
                }
            else
                {
                for (size_t k = k0; k < k1; k++)
                    {
                    newNormal.push_back (extendedSectorNormal[k]);
                    graph->SetMaskAt (extendedSectorNode [k], barrierEdgeMask);
                    size_t oneBasedNormalIndex = newNormal.size ();
                    int readIndexI;
                    if (!graph->TryGetLabel (extendedSectorNode[k], readIndexLabel, readIndexI))
                        return false;
                    size_t readIndex = (size_t)readIndexI;
                    if (readIndex >= newNormalIndex.size ())
                        return false;
                    newNormalIndex[readIndex] = static_cast<int>(oneBasedNormalIndex);
                    }
                }

            k0 = k1;
            }
        }

    // confirm that all normal indices have been touched ...
    for (size_t i = 0, n = newNormalIndex.size (); i < n; i++)
        {
        if (newNormalIndex[i] < 0)
            return false;
        }

    // Ah !!! time to install ...
    bvector<DVec3d> &meshNormal = mesh.Normal ();
    bvector<int> &meshNormalIndex = mesh.NormalIndex ();
    
    meshNormal.clear ();
    // normals are "new"  --- probably smaller.  clear and copy.
    for (size_t i = 0, n = newNormal.size (); i < n; i++)
        meshNormal.push_back (newNormal[i]);
    // indices are one-to-one replacements ...
    assert (newNormalIndex.size () == meshNormalIndex.size ());
    for (size_t i = 0, n = newNormalIndex.size (); i < n; i++)
        meshNormalIndex[i] = newNormalIndex[i];
    
    if (markTransitionsVisible)
        {
        bvector<int> &pointIndex = mesh.PointIndex ();
        for (size_t i = 0; i < pointIndex.size (); i++)
            pointIndex[i] = abs (pointIndex[i]);
        size_t numTotal = 0;
        size_t numInteriorSector = 0;
        size_t numHidden = 0;
        MTGARRAY_SET_LOOP (nodeB, graph)
            {
            numTotal++;
            if (!graph->GetMaskAt (nodeB, MTG_EXTERIOR_MASK))
                {
                numInteriorSector++;
                MTGNodeId nodeA = graph->VPred (nodeB);
                if (!graph->GetMaskAt (nodeA, MTG_EXTERIOR_MASK))
                    {
                    int normalIndexA, normalIndexB;
                    if (  TryGetNormalIndex (nodeA, meshNormalIndex, normalIndexA)
                       && TryGetNormalIndex (nodeB, meshNormalIndex, normalIndexB)
                       && normalIndexA == normalIndexB
                       )
                        {
                        TrySetVisibility (nodeB, false);
                        numHidden++;
                        }
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeB, graph)
        Report (numTotal, numInteriorSector, numHidden);
        }
    return true;
    }

bool TryGetEdgeVector (MTGNodeId nodeA, MTGMask skipMask, DVec3dR edgeVector, MTGNodeId &mateId)
    {
    if (graph->GetMaskAt (nodeA, skipMask))
        return false;
    MTGNodeId nodeB = graph->FSucc (nodeA);
    mateId = graph->VSucc (nodeB);

    if (graph->GetMaskAt (mateId, MTG_EXTERIOR_MASK))
        return false;
    edgeVector = GetEdgeVector (nodeA);
    return true;
    }
void MarkPathContinuations (double continuationRadians, bool clearFirst)
    {
    UsageSums angle0, angle1;
    if (clearFirst)
        {
        MTGARRAY_SET_LOOP (nodeA, graph)
            {
            DVec3d vectorA;
            MTGNodeId mateA;
            // Walk around the vertex at the far end.
            // Mark edges visible if within continuationRadians of this edge.
            if (TryGetEdgeVector (nodeA, MTG_EXTERIOR_MASK, vectorA, mateA))
                TrySetVisibility (nodeA, false);
            else
                TrySetVisibility (nodeA, true);
            }
        MTGARRAY_END_SET_LOOP (nodeB, graph)
        }
    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        DVec3d vectorA;
        MTGNodeId mateA;
        // Walk around the vertex at the far end.
        // Mark edges visible if within continuationRadians of this edge.
        if (TryGetEdgeVector (nodeA, MTG_EXTERIOR_MASK, vectorA, mateA))
            {
            MTGARRAY_VERTEX_LOOP (nodeQ, graph, mateA)
                {
                DVec3d vectorQ;
                MTGNodeId mateQ;
                if (nodeQ != mateA && TryGetEdgeVector (nodeQ, MTG_EXTERIOR_MASK, vectorQ, mateQ))
                    {
                    double theta = vectorA.AngleTo (vectorQ);
                    if (theta < continuationRadians)
                        {
                        angle0.Accumulate (theta);
                        TrySetVisibility (nodeA, true);
                        TrySetVisibility (mateA, true);
                        TrySetVisibility (nodeQ, true);
                        TrySetVisibility (mateQ, true);
                        }
                    else
                        angle1.Accumulate (theta);
                    }
                }
            MTGARRAY_END_VERTEX_LOOP (nodeQ, graph, mateA)
            }
        }
    MTGARRAY_END_SET_LOOP (nodeB, graph)
    BeConsole::Printf (" (%d avg %g std %g)\n", (int)angle0.Count (), angle0.Mean (), angle0.StandardDeviation ());
    BeConsole::Printf (" (%d avg %g std %g)\n", (int)angle1.Count (), angle1.Mean (), angle1.StandardDeviation ());
    }
struct SectorData
{
MTGNodeId   m_node;
ptrdiff_t   m_normalIndex;    // 0 if exterior
DVec3d      m_vector;
MTGNodeId   m_baseNode;   // first node sharing normal in a sector.
size_t      m_count;

SectorData (MTGNodeId node, ptrdiff_t normalIndex, DVec3dCR vector, MTGNodeId baseNode, size_t count = 0)
    : m_node (node), m_normalIndex (normalIndex), m_vector(vector), m_baseNode (baseNode), m_count (count)
    {
    }
SectorData ()
    : m_node (MTG_NULL_NODEID), m_normalIndex (0), m_vector(DVec3d::From (0,0,0)), m_baseNode (MTG_NULL_NODEID), m_count (0)
    {
    }
};
// Collect distinct normal data around the vertex.
// Return the number of nodes around the vertex.
// BUT (!!)) the array has a multiple of the number of nodes -- to allow easy wraparound.
void CollectSectorDataAroundVertex (
MTGNodeId vertexSeed,
bvector<int> &meshNormalIndex,  // ONE BASED normal index array from mesh.
bvector<DVec3d> &meshNormal,
double offsetDistance,
bvector<SectorData> &data
)
    {
    data.clear ();
    MTGARRAY_VERTEX_LOOP (node, graph, vertexSeed)
        {
        if (!graph->GetMaskAt (node, MTG_EXTERIOR_MASK))
            {
            int normalIndex;
            DVec3d normal;
            if (   TryGetNormalIndex (node, meshNormalIndex, normalIndex)
                && TryGetNormal (node, normal)
                )
                {
                data.push_back (SectorData (node, normalIndex, normal * offsetDistance, node));
                }
            else
                {
                data.push_back (SectorData (node, 0, DVec3d::From (0,0,0), node));
                }
            }
        }
    MTGARRAY_END_VERTEX_LOOP (node, graph, vertexSeed)

    }
// Compute an offset direction for 3 given normals
// distances are multiples of the normals !!!!
ValidatedDVec3d SolveOffset (DVec3dCR normal0, double distance0, DVec3dCR normal1, double distance1, DVec3dCR normal2, double distance2)
    {
    RotMatrix matrix = RotMatrix::FromRowVectors (normal0, normal1, normal2);
    DVec3d rhs = DVec3d::From (distance0 * normal0.MagnitudeSquared (), distance1 * normal1.MagnitudeSquared (), distance2 * normal2.MagnitudeSquared ());
    DVec3d solution;
    if (matrix.Solve (solution, rhs))
        {
        return ValidatedDVec3d (solution, true);
        }
    return ValidatedDVec3d (normal0, false);
    }

// Add copies 
// find contiguous entries with the same normal index.
// within the cluster, assign the base node back to first.
// Return index of of the first cluster start.
// On return the array has added entries so that simple indexing can be used for numReference entries beginning at the break index.
size_t WrapAndAssignBaseNodes (bvector<SectorData> &data, size_t numReference)
    {
    // Find an entry whose normal index differs from predecessor ... (0 if all identical)
    size_t breakIndex = 0;
    size_t numNode = data.size ();
    size_t priorIndex = numNode - 1;
    for (size_t i = 0; i < numNode; priorIndex = i++)
        {
        if (data[priorIndex].m_normalIndex != data[i].m_normalIndex)
            {
            breakIndex = i;
            break;
            }
        }
    data.reserve (breakIndex + numReference);
    for (size_t i = 0; i < numReference; i++)
        data.push_back (data[i]);
        
    return breakIndex;
    }
// ASSUME nodeToOffset is sized (and initialized) for all nodeId's
void RecordNodeOffsets (bvector<SectorData> &nodeToOffset, DVec3dCR vector, bvector<SectorData> const &source, size_t index0, size_t count)
    {
    size_t numNode = nodeToOffset.size ();
    for (size_t i = index0; i < index0 + count; i++)
        {
        MTGNodeId nodeId = source[i].m_node;
        if (nodeId >= 0 && nodeId < (int)numNode)
            nodeToOffset[nodeId] = SectorData (source[i].m_node, 0, vector, source[i].m_node);
        }
    }

// bound member for AdjustOffset ...
double m_tangentHalfChisel;
void SetChiselMembers (Angle maxChiselAngle)
    {
    m_tangentHalfChisel = sqrt (2.0) * tan (0.5 * maxChiselAngle.Radians ());
    }
// Returned vector is always set to either
//<ul>
//<li>true if the candidateOffset unchanged
//<li>false if a restricted offset was computed.
//</ul>
ValidatedDVec3d CorrectedOffset (DVec3dCR baseOffset, DVec3dCR computedOffset, DVec3dCP edgeVector0 = nullptr, DVec3dCP edgeVector1 = nullptr)
    {
    double a1 = baseOffset.Magnitude ();
    DVec3d shift1 = DVec3d::FromStartEnd (baseOffset, computedOffset);
    if (    edgeVector0 != nullptr 
        &&  edgeVector1 != nullptr 
        &&  shift1.DotProduct (*edgeVector0) >= 0.0
        &&  shift1.DotProduct (*edgeVector1) >= 0.0
        )
        return ValidatedDVec3d (computedOffset, true);
    double a2 = shift1.Magnitude ();
    double maxShift = m_tangentHalfChisel * a1;
    if (a2 <= maxShift)
        return ValidatedDVec3d (computedOffset, true);
    double fraction = maxShift / a2;
    return ValidatedDVec3d (DVec3d::FromInterpolate (baseOffset, fraction, computedOffset), false);
    }

DVec3d GetEdgeVector (MTGNodeId node)
    {
    DPoint3d xyz0, xyz1;
    oldFacets->NodeToVertexCoordinates (node, xyz0);
    oldFacets->NodeToVertexCoordinates (graph->FSucc (node), xyz1);
    return DVec3d::FromStartEnd (xyz0, xyz1);
    }

// On input: The graph has been marked up with shared meshNormalIndex    
void ComputeOffsetVectors
(
double distance,
Angle maxChiselAngle,           // max angle
MTGMask   complexVertexMask,    // mask to apply at vertices that do not offset to single point
MTGMask   complexEdgeMask,      // mask to apply at (base of) edge where the chisel condition is exceeded
bvector<SectorData> &offsetVectors
    // List of all node+offsetVector pairs.
)
    {
    bvector<DVec3d> &meshNormal = mesh.Normal ();
    bvector<int> &meshNormalIndex = mesh.NormalIndex ();
    SetChiselMembers (maxChiselAngle);
    double crossLength = fabs (distance);
    MTGMask visitMask = graph->GrabMask ();
    graph->ClearMask (visitMask);
    bvector<SectorData> sectors;
    bvector<SectorData> baseSectors;
    bvector<SectorData> correctedBaseSectors;
    offsetVectors.clear ();
    offsetVectors.resize (graph->GetNodeIdCount ());
    ValidatedDVec3d offsetResult;
    UsageSums baseCount;
    UsageSums fullCount;
    SmallIntegerHistogram baseCountList (20);
    MTGARRAY_SET_LOOP (vertexSeed, graph)
        {
        if (!graph->GetMaskAt (vertexSeed, visitMask))
            {
            graph->SetMaskAroundVertex (vertexSeed, visitMask);
            sectors.clear ();
            baseSectors.clear ();
            correctedBaseSectors.clear ();
            CollectSectorDataAroundVertex (vertexSeed, meshNormalIndex, meshNormal, distance, sectors);
            size_t numNode = sectors.size ();
            size_t position0 = WrapAndAssignBaseNodes (sectors, numNode + 3);
            size_t position1 = position0 + numNode;
            for (size_t positionA = position0; positionA < position1;)
                {
                size_t positionB = positionA + 1;
                while (positionB < position1 && sectors[positionB].m_normalIndex == sectors[positionA].m_normalIndex)
                    positionB++;
                baseSectors.push_back (sectors[positionA]);
                baseSectors.back ().m_normalIndex = positionA;
                baseSectors.back ().m_count = positionB - positionA;
                positionA = positionB;
                correctedBaseSectors.push_back (baseSectors.back ());
                }
            baseCount.Accumulate (baseSectors.size ());
            baseCountList.Record (baseSectors.size ());
            fullCount.Accumulate (sectors.size ());

            if (DoubleOps::AlmostEqual (distance, 0.0))
                {
                // the offsets are all zero.  Leave them in place
                }
            // Each sector now has a perpendicular offset (with offset distance applied)
            // Compute corrected offsets from offsets of adjacennt planes.   Restrict each by chisel effects.
            else if (baseSectors.size () == 1)
                {
                // no change to the singleton
                }
            else if (baseSectors.size () == 2)
                {
                DVec3d commonNormal = DVec3d::FromCrossProduct (baseSectors[0].m_vector, baseSectors[1].m_vector);
                commonNormal.ScaleToLength (crossLength);
                auto offset = SolveOffset (baseSectors[0].m_vector, 1.0, baseSectors[1].m_vector, 1.0, commonNormal, 0.0);
                if (offset.IsValid ())
                    {
                    correctedBaseSectors[0].m_vector = CorrectedOffset (baseSectors[0].m_vector, offset.Value ());
                    correctedBaseSectors[1].m_vector = CorrectedOffset (baseSectors[1].m_vector, offset.Value ());
                    }
                }
            else    // There are 3 ore more sectors.  Each contiguous group of 3 (overlapping) generates a candidate offset for its middle sector.
                {
                // Each batch of 3 generates an offset for its middle sector
                size_t numBaseSector = baseSectors.size ();
                for (size_t i0 = 0; i0 < numBaseSector; i0++)
                    {
                    size_t i1 = (i0 + 1) % numBaseSector;
                    size_t i2 = (i0 + 2) % numBaseSector;
                    DVec3d edgeVector0 = GetEdgeVector (baseSectors[i1].m_node);
                    DVec3d edgeVector1 = GetEdgeVector (baseSectors[i2].m_node);
                    auto offset = SolveOffset (baseSectors[i0].m_vector, 1.0, baseSectors[i1].m_vector, 1.0, baseSectors[i2].m_vector, 1.0);
                    if (offset.IsValid ())
                        correctedBaseSectors[i1].m_vector = CorrectedOffset (baseSectors[i1].m_vector, offset.Value (), &edgeVector0, &edgeVector1);
                    }
                }

            // distribute offsets to all nodes in the output array.
            for (auto &data : correctedBaseSectors)
                {
                RecordNodeOffsets (offsetVectors, data.m_vector, sectors, data.m_normalIndex, data.m_count);
                }

            // verify that updates happend in all sectors around this vertex
            size_t numUntouched = 0;
            for (auto &sector : sectors)
                {
                size_t node = sector.m_node;
                if (offsetVectors[node].m_node == MTG_NULL_NODEID)
                    numUntouched++;
                }
            }
        }
    MTGARRAY_END_SET_LOOP (vertexSeed, graph)
    graph->DropMask (visitMask);
    }

bool PushCoordinates (bvector<DPoint3d> &xyz, bvector<SectorData> &offsetVectors, MTGNodeId node)
    {
    DPoint3d xyzA;
    if (!graph->GetMaskAt (node, MTG_EXTERIOR_MASK)
        && oldFacets->NodeToVertexCoordinates (node, xyzA))
        {
        DPoint3d xyzB = xyzA + offsetVectors[node].m_vector;
        xyz.push_back (xyzB);
        return true;
        }
    return false;
    }

void PackDistinctCoordinates (bvector<DPoint3d> &xyz)
    {
    while (xyz.size () > 1 && xyz.back ().AlmostEqual (xyz.front()))
        xyz.pop_back();
    size_t numDistinct = 1;
    DPoint3d xyzA = xyz.front ();
    for (size_t i = 1; i < xyz.size (); i++)
        {
        if (!xyzA.AlmostEqual (xyz[i]))
            xyzA = xyz[numDistinct++] = xyz[i];
        }
    xyz.resize (numDistinct);
    }

// output faces with coordinate shift !!!
void AnnounceShiftedMeshToBuilder
(
IPolyfaceConstructionPtr &meshBuilder,
MTGMask   complexVertexMask,    // mask to apply at vertices that do not offset to single point
MTGMask   complexEdgeMask,      // mask to apply at (base of) edge where the chisel condition is exceeded
bvector<SectorData> &offsetVectors
)
    {
    MTGMask visitMask = graph->GrabMask ();
    graph->ClearMask (visitMask);

    bvector<DPoint3d> xyz;      // points around a single facet.
    SmallIntegerHistogram faceCounts(10), vertexCounts(20), edgeCounts(5);
    // FACE offsets . . .
    MTGARRAY_SET_LOOP (faceSeed, graph)
        {
        if (!graph->GetMaskAt (faceSeed, visitMask))
            {
            graph->SetMaskAroundFace (faceSeed, visitMask);
            if (!graph->GetMaskAt (faceSeed, MTG_EXTERIOR_MASK))
                {
                xyz.clear ();
                MTGARRAY_FACE_LOOP (node, graph, faceSeed)
                    {
                    PushCoordinates (xyz, offsetVectors, node);
                    }
                MTGARRAY_END_FACE_LOOP (node, graph, faceSeed)
                PackDistinctCoordinates (xyz);
                meshBuilder->AddTriangulation (xyz);
                faceCounts.Record (xyz.size ());
                }
            }
        }
    MTGARRAY_END_SET_LOOP (faceSeed, graph)


    // EDGE offsets ...
    // There are 4 nodes around the edge.
    // They may have been offset to another edge.
    // But either or both ends may have been split.
    graph->ClearMask (visitMask);
    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        if (!graph->GetMaskAt (nodeA, visitMask))
            {
            graph->SetMaskAroundEdge (nodeA, visitMask);
            xyz.clear ();
            MTGNodeId nodeB = graph->FSucc (nodeA);
            MTGNodeId nodeC = graph->VSucc (nodeB);
            MTGNodeId nodeD = graph->FSucc (nodeC);
            if (PushCoordinates (xyz, offsetVectors, nodeA)
                && PushCoordinates (xyz, offsetVectors, nodeD)
                && PushCoordinates (xyz, offsetVectors, nodeC)
                && PushCoordinates (xyz, offsetVectors, nodeB)
                )
                {
                PackDistinctCoordinates (xyz);
                if (xyz.size () > 2)
                    meshBuilder->AddTriangulation (xyz);
                edgeCounts.Record(xyz.size ());
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA, graph)

    // VERTEZX offsets ...
    // If there are 3 or more nodes around a vertex ...
    // Generate each offset point.
    // triangulate the polygon.
    auto maxPerFaceSave = meshBuilder->GetFacetOptionsR ().GetMaxPerFace ();
    meshBuilder->GetFacetOptionsR ().SetMaxPerFace (3);
    graph->ClearMask (visitMask);
    MTGARRAY_SET_LOOP (vertexSeed, graph)
        {
        if (!graph->GetMaskAt (vertexSeed, visitMask))
            {
            graph->SetMaskAroundVertex (vertexSeed, visitMask);
            xyz.clear ();
            MTGARRAY_VERTEX_LOOP (node, graph, vertexSeed)
                {
                PushCoordinates (xyz, offsetVectors, node);
                }
            MTGARRAY_END_VERTEX_LOOP (node, graph, vertexSeed)
            PackDistinctCoordinates (xyz);
            if (xyz.size () > 2)
                meshBuilder->AddTriangulation (xyz);
            vertexCounts.Record (xyz.size ());
            }
        }
    MTGARRAY_END_SET_LOOP (vertexSeed, graph)
    meshBuilder->GetFacetOptionsR ().SetMaxPerFace (maxPerFaceSave);



    graph->DropMask (visitMask);

    }


// output faces created by sweep of exterior edges
void AnnounceEdgeSplitsToBuilder
(
IPolyfaceConstructionPtr &meshBuilder,
MTGMask   complexVertexMask,    // mask to apply at vertices that do not offset to single point
MTGMask   complexEdgeMask,      // mask to apply at (base of) edge where the chisel condition is exceeded
bvector<SectorData> &offsetVectorsA,
bvector<SectorData> &offsetVectorsB
)
    {
    auto maxPerFaceSave = meshBuilder->GetFacetOptionsR ().GetMaxPerFace ();
    meshBuilder->GetFacetOptionsR ().SetMaxPerFace (4);
    // Find interior edges adjacent to exterior.
    // output quad between them (builder's triangulation method can detect if offsets produced nonplanar quads)
    bvector<DPoint3d> xyz;
    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        MTGNodeId nodeB = graph->FSucc (nodeA);
        MTGNodeId nodeC = graph->VSucc (nodeB);
        if  (  !graph->GetMaskAt (nodeA, MTG_EXTERIOR_MASK)
            && graph->GetMaskAt (nodeC, MTG_EXTERIOR_MASK)
            )
            {
            xyz.clear ();
            if (   PushCoordinates (xyz, offsetVectorsA, nodeA)
                && PushCoordinates (xyz, offsetVectorsB, nodeA)
                && PushCoordinates (xyz, offsetVectorsB, nodeB)
                && PushCoordinates (xyz, offsetVectorsA, nodeB)
                )
                {
                PackDistinctCoordinates (xyz);
                if (xyz.size () > 2)
                    meshBuilder->AddTriangulation (xyz);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA, graph)
    meshBuilder->GetFacetOptionsR ().SetMaxPerFace (maxPerFaceSave);
    }

// output faces created by split neighborhood sweep of exterior vertices
void AnnounceVertexSplitsToBuilder
(
IPolyfaceConstructionPtr &meshBuilder,
MTGMask   complexVertexMask,    // mask to apply at vertices that do not offset to single point
MTGMask   complexEdgeMask,      // mask to apply at (base of) edge where the chisel condition is exceeded
bvector<SectorData> &offsetVectorsA,
bvector<SectorData> &offsetVectorsB
)
    {
    bvector<DPoint3d> xyz;
    // Find exterior vertex (with outgoing and incoming exterior edges)
    // The nodes to either side may have been offset differently.
    // If so, generate sweep
    MTGARRAY_SET_LOOP (nodeX, graph)
        {
        if (graph->GetMaskAt (nodeX, MTG_EXTERIOR_MASK))
            {
            MTGNodeId nodeA = graph->VSucc (nodeX);
            MTGNodeId nodeB = graph->VPred (nodeX);
            if (   !graph->GetMaskAt (nodeA, MTG_EXTERIOR_MASK)
                && !graph->GetMaskAt (nodeB, MTG_EXTERIOR_MASK))
                {
                xyz.clear ();
                if (   PushCoordinates (xyz, offsetVectorsA, nodeA)
                    && PushCoordinates (xyz, offsetVectorsA, nodeB)
                    && PushCoordinates (xyz, offsetVectorsB, nodeB)
                    && PushCoordinates (xyz, offsetVectorsB, nodeA)
                    )
                    {
                    PackDistinctCoordinates (xyz);
                    if (xyz.size () > 2)
                        meshBuilder->AddTriangulation (xyz);
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeX, graph)
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool go (double maxSingleEdgeAngle, double maxAccumulatedAngle, bool markAllTransitionsVisible)
    {    
    static double s_pathContinuationRadians = 0.0;

    if (!mesh.BuildPerFaceNormals ())
        return false;
    if (!PolyfaceToMTG_FromPolyfaceConnectivity (oldFacets, mesh))
        return false;
    if (!graph->TrySearchLabelTag (MTG_LABEL_TAG_POLYFACE_READINDEX, readIndexLabel))
        return false;

    bool normalsOK = ComputeFaceNormals (maxSingleEdgeAngle, maxAccumulatedAngle, markAllTransitionsVisible);
    if (s_pathContinuationRadians > 0.0)
        MarkPathContinuations (s_pathContinuationRadians, true);
    return normalsOK;
    }

PolyfaceHeaderPtr goOffset
(
double maxSingleEdgeAngle,
double maxAccumulatedAngle,
bool markAllTransitionsVisible, 
ValidatedDouble positiveOrientationOffset,
ValidatedDouble negativeOrientationOffset,
Angle maxChamaferAngle,
bool outputOffset1,
bool outputOffset2,
bool outputSideFacets
)
    {
    if (go (maxSingleEdgeAngle, maxAccumulatedAngle, markAllTransitionsVisible))
        {
        bvector<SectorData> offsetVectorsA, offsetVectorsB;
        MTGMask complexVertexMask = graph->GrabMask ();
        MTGMask complexEdgeMask = graph->GrabMask ();
        graph->ClearMask (complexVertexMask);
        graph->ClearMask (complexEdgeMask);
        auto options = IFacetOptions::Create ();
        IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
        // Side facets depend on both offsets.
        // 1) if either of the offsets is not active, suppress the side.
        // 2) if side is requested, both primaries have to be computed even if not being output.
        // 2) if side is not requested and only one primary is requested, the other primary can be skipped.

        if (!negativeOrientationOffset.IsValid () || !positiveOrientationOffset.IsValid ())
            outputSideFacets = false;

        if (positiveOrientationOffset.IsValid () && (outputOffset1 || outputSideFacets))
            {
            ComputeOffsetVectors (positiveOrientationOffset.Value (),
                    maxChamaferAngle, complexVertexMask, complexEdgeMask, offsetVectorsA);
            if (outputOffset1)
                AnnounceShiftedMeshToBuilder (meshBuilder, complexEdgeMask, complexVertexMask, offsetVectorsA);
            }

        if (negativeOrientationOffset.IsValid () && (outputOffset2 || outputSideFacets))
            {
            ComputeOffsetVectors (negativeOrientationOffset.Value (),
                    maxChamaferAngle, complexVertexMask, complexEdgeMask, offsetVectorsB);
            if (outputOffset2)
                {
                meshBuilder->ToggleIndexOrderAndNormalReversal ();
                AnnounceShiftedMeshToBuilder (meshBuilder, complexEdgeMask, complexVertexMask, offsetVectorsB);
                meshBuilder->ToggleIndexOrderAndNormalReversal ();
                }
            }

        if (positiveOrientationOffset.IsValid () && negativeOrientationOffset.IsValid ())
            {
            if (outputSideFacets)
                {
                AnnounceEdgeSplitsToBuilder (meshBuilder, complexEdgeMask, complexVertexMask, offsetVectorsA, offsetVectorsB);
                AnnounceVertexSplitsToBuilder (meshBuilder, complexEdgeMask, complexVertexMask, offsetVectorsA, offsetVectorsB);
                }
            }
        graph->DropMask (complexEdgeMask);
        graph->DropMask (complexVertexMask);
        return meshBuilder->GetClientMeshPtr ();
        }
    return nullptr;
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildApproximateNormals (double maxSingleEdgeAngle, double maxAccumulatedAngle, bool markAllTransitionsVisible)
    {

    ApproximateVertexNormalContext context (*this);
    return context.go (maxSingleEdgeAngle, maxAccumulatedAngle, markAllTransitionsVisible);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2016
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::ComputeOffset
(
OffsetOptions const &options,
double distance1,            //!< [in] offset distance for first surface.
double distance2,           //!< [in] offset distance for second surface.
bool outputOffset1,         //!< [in] true to output the (positive oriented) offset at distance1
bool outputOffset2,         //!< [in] true to output the (negatively oriented) offset at distance2
bool outputSideFacets       //!< [in] true to output side facets where boundary edges are swept
)
    {
    PolyfaceHeaderPtr nonDuplicatedMesh = this->CloneWithIndexedDuplicatesRemoved ();
    ApproximateVertexNormalContext context (*nonDuplicatedMesh);
    return context.goOffset (options.m_maxSingleEdgeAngle.Radians (), options.m_maxAccumulatedAngle.Radians (), options.m_useStoredNormals,
        ValidatedDouble (distance1, true),
        ValidatedDouble (distance2, true),
        options.m_maxChamferAngle,
        outputOffset1, outputOffset2, outputSideFacets
        );
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildPerFaceParameters (LocalCoordinateSelect selector)
    {
    size_t numIndex = m_pointIndex.size ();
    // We don't know how to do this for non-indexed ...
    if (m_pointIndex.size () == 0)
        return false;
        
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    BentleyApi::Transform worldToLocal, localToWorld;
    visitor->SetNumWrap (0);
    bvector <size_t> &indexPosition = visitor->IndexPosition ();
    bvector <DPoint3d> &visitorPoint = visitor->Point ();
    
    // We will have distinct params on every corner of every face.  Don't know how many ...
    ClearParameters (true);
    // There has to be a param for each point index ..
    m_paramIndex.reserve (numIndex);
    for (size_t i = 0; i < numIndex; i++)
        m_paramIndex.push_back (0);

    for (size_t i = 0; i < m_faceData.size (); i++)
        m_faceData[i].m_paramRange = DRange2d::NullRange ();
        
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        bool frameOK = visitor->TryGetLocalFrame (localToWorld, worldToLocal, selector);

        for (size_t i = 0, n = visitor->NumEdgesThisFace (); i < n; i++)
            {
            DPoint3d uvw;
            DPoint2d uv;
            if (frameOK)
                worldToLocal.Multiply (uvw, visitorPoint[i]);
            else
                uvw.Zero ();
            size_t newParamIndex = m_param.size ();
            uv.x = uvw.x;
            uv.y = uvw.y;
            m_param.push_back (uv);
            size_t readPos = indexPosition[i];
            if (readPos < numIndex)   // really really better be...
                m_paramIndex[readPos] = static_cast <int>(newParamIndex + 1);   // ONE BASED !!!!
            size_t faceIndex;
            if (m_faceIndex.Active ()
                && readPos < m_faceIndex.size ()
                && (faceIndex = m_faceIndex[readPos]) < m_faceData.size ()
                )
                {
                m_faceData[faceIndex].m_paramRange.Extend (uv);
                }
            }
        }
    
    return true;
    }  

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearNormals (bool active)
    {
    Normal ().clear ();
    NormalIndex ().clear ();
    Normal ().SetActive (active);
    NormalIndex ().SetActive (active);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearParameters (bool active)
    {
    Param ().clear ();
    ParamIndex ().clear ();
    Param ().SetActive (active);
    ParamIndex ().SetActive (active);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildPerFaceNormals ()
    {
    if (!ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ())
        return false;
     size_t numIndex = m_pointIndex.size ();
    // We don't know how to do this for non-indexed ...
    if (m_pointIndex.size () == 0)
        return false;
        
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    BentleyApi::Transform worldToLocal, localToWorld;
    visitor->SetNumWrap (0);
    bvector <size_t> &indexPosition = visitor->IndexPosition ();
    
    ClearNormals (true);
    
    // There has to be a param for each point index ..
    m_normalIndex.reserve (numIndex);
    for (size_t i = 0; i < numIndex; i++)
        m_normalIndex.push_back (0);
    DVec3d zvector;
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        if (visitor->TryGetLocalFrame (localToWorld, worldToLocal, LOCAL_COORDINATE_SCALE_UnitAxesAtStart))
            {
            localToWorld.GetMatrixColumn (zvector, 2);
            size_t newNormalIndex = m_normal.size ();
            m_normal.push_back (zvector);
            
            for (size_t i = 0, n = visitor->NumEdgesThisFace (); i < n; i++)
                {
                size_t readPos = indexPosition[i];
                if (readPos < numIndex)   // really really better be...
                    m_normalIndex[readPos] = static_cast <int>(newNormalIndex + 1);   // ONE BASED !!!!
                }
            }
        }
    
    return true;
    }
 
 
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildPerFaceFaceData ()
    {
    FaceIndex().SetActive(true);

    if (0 !=  GetParamCount())
        {
        SetNewFaceData (NULL, GetPointIndexCount());

        for (FacetFaceDataR faceData : m_faceData)
            faceData.m_paramRange.InitFrom (0.0, 0.0, 1.0, 1.0);
        }
    else
        {
        FacetFaceData       faceData;
        PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (*this, true);

        for (visitor->Reset (); visitor->AdvanceToNextFace (); )
            {
            SetNewFaceData (&faceData, visitor->GetReadIndex() + visitor->NumEdgesThisFace() + (0 == GetNumPerFace() ? 1 : 0));
            faceData.Init ();
            }
        }


    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2017
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildXYParameters (LocalCoordinateSelect selector, TransformR localToWorld, TransformR worldToLocal)
    {
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    if (0 == m_point.size ())
        return false;

    DRange3d range = DRange3d::From (m_point);
    DPoint3d points [5];
    double z = range.low.z;
    points[0] = DPoint3d::From (range.low.x, range.low.y, z);
    points[1] = DPoint3d::From (range.high.x, range.low.y, z);
    points[2] = DPoint3d::From (range.high.x, range.high.y, z);
    points[3] = DPoint3d::From (range.low.x, range.high.y, z);
    points[4] = points[0];

    if (!PolygonOps::CoordinateFrame
        (
        points, 5,
        localToWorld, worldToLocal, selector
        ))
        return false;
    return BuildParametersFromTransformedPoints (worldToLocal);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2017
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildParametersFromTransformedPoints (TransformCR worldToLocal)
    {
    if (0 == m_point.size ())
        return false;

    // * one param per point.
    m_param.SetActive (true);
    m_param.clear ();
    for (auto xyz : m_point    )
        {
        m_param.push_back (DPoint2d::From (xyz.x, xyz.y));
        }
    worldToLocal.Multiply (m_param, m_param);

    // * param index matches point index.
    m_paramIndex.SetActive (m_pointIndex.Active ());
    if (m_paramIndex.Active ())
        {
        m_paramIndex.clear ();
        m_paramIndex.insert (m_paramIndex.begin (), m_pointIndex.begin (), m_pointIndex.end());
        }

    return true;
    }
   
END_BENTLEY_GEOMETRY_NAMESPACE
















