#include "ScalableMeshPCH.h"
#include "ScalableMeshGraph.h"
//#define ENABLE_EXTRA_ASSERTS

#ifndef NDEBUG
#define NDEBUG
#undef DEBUG
#endif
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
size_t FastCountNodesAroundFace(MTGGraph* graphP, MTGNodeId id)
    {
    size_t n = 0;
    MTGARRAY_FACE_LOOP(faceIter, graphP, id)
        {
        if (++n > 3) return n;
        }
    MTGARRAY_END_FACE_LOOP(faceIter, graphP, id)
        return n;
    }


bool IsOutsideEdge(MTGGraph* graphP, MTGNodeId id)
    {
    size_t countNodes = FastCountNodesAroundFace(graphP, id), countMates = FastCountNodesAroundFace(graphP,graphP->EdgeMate(id));
    //size_t countNodes = graphP->CountNodesAroundFace(id), countMates = graphP->CountNodesAroundFace(graphP->EdgeMate(id));
    assert(countNodes == 3 || countMates == 3);
    if (countNodes == 3 && countMates > 3) return false;
    else if (countMates == 3 && countNodes > 3) return true;
    else //gets trickier: 3 outside faces, 3 inside faces, not necessarily a triangle
        {
        if (!graphP->GetMaskAt(id, MTG_BOUNDARY_MASK)) return false;
        else
            {
            MTGARRAY_FACE_LOOP(faceIter, graphP, id)
                {
                if (!graphP->GetMaskAt(faceIter, MTG_BOUNDARY_MASK)) return false;
                }
            MTGARRAY_END_FACE_LOOP(faceIter, graphP, id)
                return true;
            }
        }
    }

void GetGraphExteriorBoundary(MTGGraph* graphP, bvector<bvector<DPoint3d>>& vecBound, const DPoint3d* points, bool pruneTriangleInnerLoops)
    {
    MTGMask visitedMask = graphP->GrabMask();
    std::map<int,int> pts;
    size_t numLoop = 0;
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        pts.clear();
        if (graphP->GetMaskAt(edgeID, MTG_EXTERIOR_MASK) && !graphP->GetMaskAt(edgeID, visitedMask))
            {
            std::string s;
            s += "LOOP N " + std::to_string(numLoop) + "\n";
            bvector<DPoint3d> bound;
            bvector<bvector<DPoint3d>> boundLoops;
            std::vector<MTGNodeId> edges;
            MTGARRAY_FACE_LOOP(extID, graphP, edgeID)
                {
                int v = -1;
                if (graphP->GetMaskAt(extID, visitedMask) || !graphP->IsValidNodeId(extID)) break;
                graphP->SetMaskAt(extID, visitedMask);
                graphP->TryGetLabel(extID, 0, v);
                s += "POINT N " + std::to_string(v) + "\n";
                if (v <= 0 )continue;
                if (pts.count(v) != 0 && pts[v] < bound.size()) //detected a loop
                    {
                    bvector<DPoint3d> innerLoop;
                    innerLoop.insert(innerLoop.end(), bound.begin() + pts[v], bound.end());
                    innerLoop.push_back(innerLoop.front()); //close loop
                    s += "STOP LOOP SIZE IS " + std::to_string(innerLoop.size()) + "\n";
                    int idx = pts[v];
                    if (pruneTriangleInnerLoops && innerLoop.size() <= 4)
                        {
                        for (size_t i = idx; i < edges.size(); ++i)
                            {
                            graphP->DropEdge(edges[i]);
                            }

                        }
                    else  boundLoops.push_back(innerLoop);
                    for (auto it = pts.begin(); it != pts.end(); ++it)
                        if (it->second > idx && it->second < bound.size())
                            {
                            pts.erase(it);
                            it = pts.begin();
                            }
                    bound.resize(idx+1); //we keep the looping point
                    edges.resize(idx + 1);
                    }
                else
                    {
                    pts[v] = (int) bound.size();
                    edges.push_back(extID);
                    bound.push_back(points[v - 1]);
                    }
                }
            MTGARRAY_END_FACE_LOOP(extID, graphP, edgeID)
//                int v = -1;
            if (pruneTriangleInnerLoops && bound.size() <= 3)
                {
                for (size_t i = 0; i < edges.size(); ++i)
                    {
                    graphP->DropEdge(edges[i]);
                    }
                bound.clear();
                }
            //graphP->TryGetLabel(edgeID, 0, v);
            //if (v <= 0)continue;
            //bound.push_back(points[v - 1]);
//            break;
            //for (auto& b : boundLoops) if (b.size() > bound.size()) bound = b;
            vecBound.push_back(bound);
            for (auto& b : boundLoops) vecBound.push_back(b);
            numLoop++;
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
        graphP->ClearMask(visitedMask);
    graphP->DropMask(visitedMask);
        /*volatile size_t size = bound.size();
    size = size;*/
    }


bool ProcessFaceToAdd(MTGGraph* graphP, const int* vtx, std::vector<int>& componentLabels, const DPoint3d* points, std::vector<std::pair<long, MTGNodeId>>** edgeLabels)
    {
    //std::ofstream log;
    MTGNodeId faceNodes[3] = { -1, -1, -1 }, existing[3] = { -1, -1, -1 };
    assert(vtx[0] != vtx[1] && vtx[1] != vtx[2] && vtx[0] != vtx[2]);
    bool edgeTest[3] = {
        edgeLabels[vtx[0] - 1] != nullptr ? std::find_if(edgeLabels[vtx[0] - 1]->begin(), edgeLabels[vtx[0] - 1]->end(), [&vtx] (const std::pair<long, MTGNodeId>& a) { return a.first == vtx[1]; }) != edgeLabels[vtx[0] - 1]->end() : false,
        edgeLabels[vtx[1] - 1] != nullptr ? std::find_if(edgeLabels[vtx[1] - 1]->begin(), edgeLabels[vtx[1] - 1]->end(), [&vtx] (const std::pair<long, MTGNodeId>& a) { return a.first == vtx[2]; }) != edgeLabels[vtx[1] - 1]->end() : false,
        edgeLabels[vtx[2] - 1] != nullptr ? std::find_if(edgeLabels[vtx[2] - 1]->begin(), edgeLabels[vtx[2] - 1]->end(), [&vtx] (const std::pair<long, MTGNodeId>& a) { return a.first == vtx[0]; }) != edgeLabels[vtx[2] - 1]->end() : false
        };

    int nExisting = 0;
    bool invalid = false;
    int isFlipped = -1;
    int currentComponentID = -1;
    int componentLabel = 1, vertexLabel = 0;
    int existingIDs[3] = { -1, -1, -1 };
    for (int edge = 0; edge < 3; edge++)
        {
        if (edgeTest[edge])
            {
            existing[edge] = std::find_if(edgeLabels[vtx[edge] - 1]->begin(), edgeLabels[vtx[edge] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { int t = edge == 2 ? 0 : edge + 1; return a.first == vtx[t]; })->second;
            assert(graphP->IsValidNodeId(existing[edge]));
            for (int edge2 = 0; edge2 < 3; edge2++) if (graphP->FSucc(existing[edge]) == existing[edge2] && graphP->FSucc(graphP->EdgeMate(existing[edge])) == existing[edge2]) invalid = true;
            graphP->TryGetLabel(existing[edge], componentLabel, (int&)existingIDs[edge]);
            if (existingIDs[edge] != -1 && existingIDs[edge] >= (int)componentLabels.size()) componentLabels.resize(existingIDs[edge], -1);
            if (currentComponentID == -1 && existingIDs[edge] != -1) currentComponentID = existingIDs[edge];
            else if (existingIDs[edge] != -1 && points != nullptr)
                {
                int toReplace = componentLabels[currentComponentID];
                if (points[componentLabels[existingIDs[edge]]].x > points[componentLabels[currentComponentID]].x)
                    toReplace = componentLabels[existingIDs[edge]];
                else currentComponentID = existingIDs[edge];
                if (toReplace != componentLabels[currentComponentID])
                    {
                    for (auto& pt : componentLabels)
                        {
                        //                  int val = &pt - &componentLabels[0];
                        if (pt == toReplace)
                            {
                            //                      log << "REPLACING " + std::to_string(toReplace) + " AT " + std::to_string(val) + " WITH " + std::to_string(componentLabels[currentComponentID]) << endl;
                            pt = componentLabels[currentComponentID];
                            }
                        }
                    }
                }
            if (!graphP->GetMaskAt(existing[edge], MTG_BOUNDARY_MASK))
                {
                //existing[edge] = -1;//invalid = true; //only add 2 faces per edge
                //continue;
                invalid = true;
                }
            graphP->SetMaskAroundEdge(existing[edge], MTG_NULL_MASK); //interior edge
            int flipped = IsOutsideEdge(graphP, existing[edge]) ? 0 : 1; //make sure if several edges are shared, the interior triangle face can still be kept
            // if (nExisting == 1 && flipped == 1) invalid = true;
            if (flipped == 1)isFlipped = flipped;
            nExisting++;
            }
        }
    if (invalid) return false; //adding this face would result in a non-manifold mesh, ignore it
    if ((nExisting == 2 && isFlipped == 1))
        {
        return false;
        }
    MTGNodeId tmp;
    if (edgeTest[0] && edgeTest[1] && edgeTest[2])
        {
        bool canAdd = graphP->GetMaskAt(graphP->FSucc(existing[0]), MTG_BOUNDARY_MASK) && graphP->GetMaskAt(graphP->FPred(existing[0]), MTG_BOUNDARY_MASK)
            && graphP->GetMaskAt(graphP->FSucc(existing[1]), MTG_BOUNDARY_MASK) && graphP->GetMaskAt(graphP->FPred(existing[1]), MTG_BOUNDARY_MASK)
            && graphP->GetMaskAt(graphP->FSucc(existing[2]), MTG_BOUNDARY_MASK) && graphP->GetMaskAt(graphP->FPred(existing[2]), MTG_BOUNDARY_MASK);
        if (!canAdd)
            {
            return false;
            }
       // log.open("D:\\0test.txt", ios_base::app);
        if (graphP->FSucc(existing[2]) != existing[0])
            {
            graphP->VertexTwist(existing[0], graphP->FSucc(existing[2]));
            }
        if (graphP->FSucc(existing[0]) != existing[1])
            {
            graphP->VertexTwist(existing[1], graphP->FSucc(existing[0]));
            }
        if (graphP->FSucc(existing[1]) != existing[2])
            {
            graphP->VertexTwist(existing[2], graphP->FSucc(existing[1]));
            }
        graphP->ClearMaskAroundEdge(existing[0], MTG_BOUNDARY_MASK);
        graphP->ClearMaskAroundEdge(existing[1], MTG_BOUNDARY_MASK);
        graphP->ClearMaskAroundEdge(existing[2], MTG_BOUNDARY_MASK);
        for (int m = 0; m < 3; m++)
            {
            int v = -1, vn = -1;
            assert(graphP->TryGetLabel(existing[m], vertexLabel, v) && v != -1);
            assert(graphP->TryGetLabel(graphP->FSucc(existing[m]), vertexLabel, vn) && vn != -1 && vn != v);
            assert(graphP->TryGetLabel(graphP->EdgeMate(existing[m]), vertexLabel, v) && v != -1);
            assert(graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(existing[m])), vertexLabel, vn) && vn != -1 && vn != v);
            }
       /* log << "CLEARBOUNDARY2 EDGE " + std::to_string(vtx[0]) + "/" + std::to_string(vtx[1]) + "EDGE ID IS " + std::to_string(existing[0]) + " MATE ID IS " + std::to_string(graphP->EdgeMate(existing[0])) + " MASK IS " +
            std::string(graphP->GetMaskAt(existing[0], MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") << endl;
        log << "CLEARBOUNDARY2 EDGE " + std::to_string(vtx[1]) + "/" + std::to_string(vtx[2]) + "EDGE ID IS " + std::to_string(existing[1]) + " MATE ID IS " + std::to_string(graphP->EdgeMate(existing[1])) + " MASK IS " +
            std::string(graphP->GetMaskAt(existing[1], MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") << endl;
        log << "CLEARBOUNDARY2 EDGE " + std::to_string(vtx[2]) + "/" + std::to_string(vtx[0]) + "EDGE ID IS " + std::to_string(existing[2]) + " MATE ID IS " + std::to_string(graphP->EdgeMate(existing[2])) + " MASK IS " +
            std::string(graphP->GetMaskAt(existing[2], MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") << endl;
        log.close();*/

        return true; //remove duplicated faces
        }
  /*  log.open("D:\\0test.txt", ios_base::app);
           log << " KEPT FACE IS " + std::to_string(vtx[0]) + "/" + std::to_string(vtx[1]) + "/" + std::to_string(vtx[2]) + " HAS CURRENT ID " + std::to_string(currentComponentID) +
    " HAS EXISTING EDGES: " + std::to_string(nExisting) << endl;
           log.close();*/

    if (currentComponentID == -1)
        {
        currentComponentID = (int)componentLabels.size();
        componentLabels.push_back(-1);
        }
    //create triangle
    graphP->CreateEdge(faceNodes[0], tmp);
    graphP->TrySetLabel(faceNodes[0], componentLabel, currentComponentID);
    graphP->TrySetLabel(tmp, componentLabel, currentComponentID);
    graphP->SplitEdge(faceNodes[1], tmp, faceNodes[0]);
    graphP->Join(tmp, faceNodes[2], faceNodes[0], graphP->EdgeMate(faceNodes[1]), 0, 0);
    //at this point faceNodes[0]->faceNodes[1]->faceNodes[2] is a face loop.
    assert(graphP->FSucc(faceNodes[0]) == faceNodes[1] && graphP->FSucc(faceNodes[1]) == faceNodes[2] && graphP->FSucc(faceNodes[2]) == faceNodes[0]);
    assert(graphP->CountNodesAroundFace(faceNodes[0]) == 3 && graphP->CountNodesAroundFace(tmp) == 3);

    int oldId = componentLabels[currentComponentID];
    if (points != nullptr)
        {
        if (componentLabels[currentComponentID] == -1 || points[componentLabels[currentComponentID]].x > points[/*pointComponents[*/vtx[0] - 1/*]*/].x) componentLabels[currentComponentID] = /*pointComponents[*/vtx[0] - 1/*]*/;
        assert(currentComponentID < componentLabels.size() && currentComponentID >= 0);
        if (componentLabels[currentComponentID] >= 0 && points[componentLabels[currentComponentID]].x > points[/*pointComponents[*/vtx[1] - 1/*]*/].x) componentLabels[currentComponentID] = /*pointComponents[*/vtx[1] - 1/*]*/;
        if (componentLabels[currentComponentID] >= 0 && points[componentLabels[currentComponentID]].x > points[/*pointComponents[*/vtx[2] - 1/*]*/].x) componentLabels[currentComponentID] = /*pointComponents[*/vtx[2] - 1/*]*/;
        for (int id = 0; id < 3; id++)
            {
            if (existingIDs[id] != -1 && (int)componentLabels.size() > existingIDs[id] && points[componentLabels[currentComponentID]].x > points[componentLabels[existingIDs[id]]].x) componentLabels[currentComponentID] = componentLabels[existingIDs[id]];
            }
        }
    int isTriangleLoopFlipped = 0;
    int flippedEdges[2][3] = { { 0, 1, 2 }, { 2, 1, 0 } };
    MTGNodeId flippedFaceNodes[3] = { -1, -1, -1 }; //faceNodes is the loop vtx[0]->vtx[1]->vtx[2]. flippedFaceNodes is vtx[2]->vtx[1]->vtx[0]
    flippedFaceNodes[0] = graphP->EdgeMate(faceNodes[1]);
    flippedFaceNodes[1] = graphP->EdgeMate(faceNodes[0]);
    flippedFaceNodes[2] = graphP->EdgeMate(faceNodes[2]);
    //we are now looking for shared edges
    for (int edge = 0; edge < 3; edge++)
        {
        if (edgeTest[edge])
            {
            //merge faceNodes[edge] (new edge) with existing[edge] (old edge). Annotate all vertices accordingly.
            // assert(graphP->CountNodesAroundFace(graphP->EdgeMate(existing[edge])) == 3 || graphP->CountNodesAroundFace(existing[edge]) == 3);
            //for (int edge2 = 0; edge2 < 3; edge2++) assert(graphP->FSucc(existing[edge]) != faceNodes[edge2] && graphP->FSucc(graphP->EdgeMate(existing[edge])) != faceNodes[edge2]);
            if (existingIDs[edge] != -1)
                {
                graphP->TrySetLabel(existing[edge], componentLabel, currentComponentID);
                }
            bool existingEdgeIsExterior = IsOutsideEdge(graphP, existing[edge]);
            MTGNodeId exteriorEdge = existingEdgeIsExterior ? existing[edge] : graphP->EdgeMate(existing[edge]);
            MTGNodeId triangleEdge = existingEdgeIsExterior ? graphP->EdgeMate(faceNodes[edge]) : faceNodes[edge];
            MTGNodeId newEdge = graphP->EdgeMate(triangleEdge);
            
        /*    std::string tlog;
            if (vtx[0] == 5491 && vtx[1] == 368 && vtx[2] == 3628)
                {
                tlog += std::string("EXTERIOR: ");
                MTGARRAY_FACE_LOOP(nId, graphP, exteriorEdge)
                    {
                    int v;
                    graphP->TryGetLabel(nId, 0, v);
                    tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                    }
                MTGARRAY_END_FACE_LOOP(nId, graphP, exteriorEdge)
                    tlog += std::string("EXISTING: ");
                    MTGARRAY_FACE_LOOP(nId, graphP, existing[edge])
                    {
                    int v;
                    graphP->TryGetLabel(nId, 0, v);
                    tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                    }
                MTGARRAY_END_FACE_LOOP(nId, graphP, existing[edge])
                }*/

            bool flipped = !existingEdgeIsExterior;
            if (flipped) isTriangleLoopFlipped = 1;

            graphP->VertexTwist(exteriorEdge, graphP->FSucc(triangleEdge));
       /*     if (vtx[0] == 692 && vtx[1] == 953 && vtx[2] == 694)
                {
                tlog += std::string("TRIANGLE INTERMEDIATE: ");
                    MTGARRAY_FACE_LOOP(nId, graphP, triangleEdge)
                    {
                    int v;
                    graphP->TryGetLabel(nId, 0, v);
                    tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                    }
                    MTGARRAY_END_FACE_LOOP(nId, graphP, triangleEdge)
                        tlog += std::string("NEW INTERMEDIATE: ");
                    MTGARRAY_FACE_LOOP(nId, graphP, newEdge)
                    {
                    int v;
                    graphP->TryGetLabel(nId, 0, v);
                    tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                    }
                MTGARRAY_END_FACE_LOOP(nId, graphP, newEdge)
                }*/
            graphP->VertexTwist(triangleEdge, graphP->FSucc(exteriorEdge));
           /* if (vtx[0] == 692 && vtx[1] == 953 && vtx[2] == 694)
                {
                tlog += std::string("TRIANGLE: ");
                MTGARRAY_FACE_LOOP(nId, graphP, triangleEdge)
                    {
                    int v;
                    graphP->TryGetLabel(nId, 0, v);
                    tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                    }
                MTGARRAY_END_FACE_LOOP(nId, graphP, triangleEdge)
                    tlog += std::string("NEW EXISTING: ");
                    MTGARRAY_FACE_LOOP(nId, graphP, existing[edge])
                    {
                    int v;
                    graphP->TryGetLabel(nId, 0, v);
                    tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                    }
                    MTGARRAY_END_FACE_LOOP(nId, graphP, existing[edge])
                }*/
            graphP->ExciseSliverFace(triangleEdge);

            assert(graphP->CountNodesAroundFace(newEdge) == 3);
            assert(graphP->CountNodesAroundFace(graphP->EdgeMate(newEdge)) == 3);
           /* faceNodes[edge] = newEdge;
            faceNodes[(edge + 1) % 3] = graphP->FSucc(faceNodes[edge]);
            faceNodes[(edge + 2) % 3] = graphP->FSucc(faceNodes[(edge + 1) % 3]);*/
            int edgeMap[3] = { 1, 0, 2 };
            if (flipped)
                {
                flippedFaceNodes[edgeMap[edge]] = newEdge;
                flippedFaceNodes[(edgeMap[edge] + 1) % 3] = graphP->FSucc(newEdge);
                flippedFaceNodes[(edgeMap[edge] + 2) % 3] = graphP->FSucc(flippedFaceNodes[(edgeMap[edge] + 1) % 3]);
                faceNodes[edge] = graphP->EdgeMate(newEdge);
                faceNodes[(edge + 1) % 3] = graphP->EdgeMate(flippedFaceNodes[edgeMap[(edge + 1) % 3]]);
                faceNodes[(edge + 2) % 3] = graphP->EdgeMate(flippedFaceNodes[edgeMap[(edge + 2) % 3]]);
                }
            else
                {
                faceNodes[edge] = newEdge;
                faceNodes[(edge + 1) % 3] = graphP->FSucc(faceNodes[edge]);
                faceNodes[(edge + 2) % 3] = graphP->FSucc(faceNodes[(edge + 1) % 3]);
                flippedFaceNodes[0] = graphP->EdgeMate(faceNodes[1]);
                flippedFaceNodes[1] = graphP->EdgeMate(faceNodes[0]);
                flippedFaceNodes[2] = graphP->EdgeMate(faceNodes[2]);
                }
            //assert(graphP->FSucc(faceNodes[0]) == faceNodes[1] && graphP->FSucc(faceNodes[1]) == faceNodes[2] && graphP->FSucc(faceNodes[2]) == faceNodes[0]);
            graphP->SetMaskAroundEdge(newEdge, MTG_NULL_MASK); //interior edge
            graphP->ClearMaskAroundEdge(newEdge, MTG_BOUNDARY_MASK);
            graphP->ClearMaskAroundEdge(newEdge, MTG_EXTERIOR_MASK);
        /*    log.open("D:\\0test.txt", ios_base::app);
            log << "CLEARBOUNDARY EDGE " + std::to_string(vtx[edge]) + "/" + std::to_string(vtx[((edge + 1) % 3)]) + "EDGE ID IS " + std::to_string(faceNodes[edge]) + " MATE ID IS " + std::to_string(graphP->EdgeMate(faceNodes[edge])) + " MASK IS " +
                std::string(graphP->GetMaskAt(faceNodes[edge], MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") << endl;
            log.close();*/
            assert(graphP->IsValidNodeId(faceNodes[edge]));
            assert(graphP->IsValidNodeId(faceNodes[(edge + 1) % 3]));
            assert(graphP->IsValidNodeId(faceNodes[(edge + 2) % 3]));

            
            std::find_if(edgeLabels[vtx[edge] - 1]->begin(), edgeLabels[vtx[edge] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { int t = edge == 2 ? 0 : edge + 1; return a.first == vtx[t]; })->second = faceNodes[edge];
            std::find_if(edgeLabels[vtx[((edge + 1) % 3)] - 1]->begin(), edgeLabels[vtx[((edge + 1) % 3)] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { return a.first == vtx[edge]; })->second = graphP->EdgeMate(faceNodes[edge]);

            auto edge1Iterator = std::find_if(edgeLabels[vtx[((edge + 1) % 3)] - 1]->begin(), edgeLabels[vtx[((edge + 1) % 3)] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { int t = (edge + 2) % 3; return a.first == vtx[t]; });
            if (edge1Iterator != edgeLabels[vtx[((edge + 1) % 3)] - 1]->end()) edge1Iterator->second = faceNodes[(edge + 1) % 3];
            else edgeLabels[vtx[((edge + 1) % 3)] - 1]->push_back(std::make_pair(vtx[((edge + 2) % 3)], faceNodes[(edge + 1) % 3]));

            if (edgeLabels[vtx[((edge + 2) % 3)] - 1] != nullptr)
                {
                auto iterator = std::find_if(edgeLabels[vtx[((edge + 2) % 3)] - 1]->begin(), edgeLabels[vtx[((edge + 2) % 3)] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { int t = (edge + 1) % 3; return a.first == vtx[t]; });
                if (iterator != edgeLabels[vtx[((edge + 2) % 3)] - 1]->end()) iterator->second = graphP->EdgeMate(faceNodes[(edge + 1) % 3]);
                else edgeLabels[vtx[((edge + 2) % 3)] -1 ]->push_back(std::make_pair(vtx[((edge + 1) % 3)],  graphP->EdgeMate(faceNodes[(edge + 1) % 3])));

                iterator = std::find_if(edgeLabels[vtx[((edge + 2) % 3)] - 1]->begin(), edgeLabels[vtx[((edge + 2) % 3)] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { return a.first == vtx[edge]; });
                if (iterator != edgeLabels[vtx[((edge + 2) % 3)] - 1]->end()) iterator->second = faceNodes[(edge + 2) % 3];
                else edgeLabels[vtx[((edge + 2) % 3)] - 1]->push_back(std::make_pair(vtx[edge], faceNodes[(edge + 2) % 3]));
                }
            else
                {
                edgeLabels[vtx[((edge + 2) % 3)] - 1] = new std::vector<std::pair<long, MTGNodeId>>();
                edgeLabels[vtx[((edge + 2) % 3)] - 1]->push_back(std::make_pair(vtx[((edge + 1) % 3)],  graphP->EdgeMate(faceNodes[(edge + 1) % 3])));
                edgeLabels[vtx[((edge + 2) % 3)] - 1]->push_back(std::make_pair(vtx[edge], faceNodes[(edge + 2) % 3]));
                }
            auto edge2Iterator = std::find_if(edgeLabels[vtx[edge] - 1]->begin(), edgeLabels[vtx[edge] - 1]->end(), [&vtx, &edge] (const std::pair<long, MTGNodeId>& a) { int t = (edge + 2) % 3; return a.first == vtx[t]; });
            if (edge2Iterator != edgeLabels[vtx[edge] - 1]->end()) edge2Iterator->second = graphP->EdgeMate(faceNodes[(edge + 2) % 3]);
            else edgeLabels[vtx[edge] - 1]->push_back(std::make_pair(vtx[((edge + 2) % 3)], graphP->EdgeMate(faceNodes[(edge + 2) % 3])));
            //if (!edgeTest[(edge + 1) % 3]) graphP->SetMaskAroundEdge(faceNodes[(edge + 1) % 3], MTG_BOUNDARY_MASK);
           // if (!edgeTest[(edge + 2) % 3]) graphP->SetMaskAroundEdge(faceNodes[(edge + 2) % 3], MTG_BOUNDARY_MASK);
            }
        else
            {
            assert(graphP->IsValidNodeId(faceNodes[edge]));
            //set boundary mask on edge
            graphP->SetMaskAroundEdge(faceNodes[edge], MTG_BOUNDARY_MASK);
            //add edge & edge-mate to edge list
            if (edgeLabels[vtx[edge] - 1] == nullptr)
                {
                edgeLabels[vtx[edge] - 1] = new std::vector<std::pair<long, MTGNodeId>>();
                edgeLabels[vtx[edge] - 1]->reserve(10);
                }
            edgeLabels[vtx[edge] - 1]->push_back(std::make_pair(vtx[((edge + 1) % 3)], faceNodes[edge]));
            if (edgeLabels[vtx[((edge + 1) % 3)] - 1] == nullptr)
                {
                edgeLabels[vtx[((edge + 1) % 3)] - 1] = new std::vector<std::pair<long, MTGNodeId>>();
                edgeLabels[vtx[((edge + 1) % 3)] - 1]->reserve(10);
                }
            edgeLabels[vtx[((edge + 1) % 3)] - 1]->push_back(std::make_pair(vtx[edge], graphP->EdgeMate(faceNodes[edge])));
            }
        }
    int pointsToUpdate[4] = { -99, -99, -99, -99 };
    bool toUpdate = false;
    MTGNodeId* innerLoopNodes = isTriangleLoopFlipped == 1? flippedFaceNodes : faceNodes;
   // std::string test = "";
  //  test += (innerLoopNodes == flippedFaceNodes) ? "FLIPPED" : "NOT";
    assert(graphP->FSucc(innerLoopNodes[0]) == innerLoopNodes[1] && graphP->FSucc(innerLoopNodes[1]) == innerLoopNodes[2] && graphP->FSucc(innerLoopNodes[2]) == innerLoopNodes[0]);
    for (int edgeT = 0; edgeT < 3; edgeT++)
        {
        assert(graphP->IsValidNodeId(innerLoopNodes[edgeT]));
        assert(graphP->EdgeMate(innerLoopNodes[edgeT]) != graphP->FSucc(innerLoopNodes[edgeT]));
        assert(edgeTest[0] == !graphP->GetMaskAt(faceNodes[0], MTG_BOUNDARY_MASK));
        assert(edgeTest[1] == !graphP->GetMaskAt(faceNodes[1], MTG_BOUNDARY_MASK));
        assert(edgeTest[2] == !graphP->GetMaskAt(faceNodes[2], MTG_BOUNDARY_MASK));
        graphP->TrySetLabel(innerLoopNodes[edgeT], vertexLabel, vtx[flippedEdges[isTriangleLoopFlipped][edgeT]]);
        graphP->TrySetLabel(graphP->EdgeMate(innerLoopNodes[edgeT]), vertexLabel, vtx[flippedEdges[isTriangleLoopFlipped][(edgeT + 1) % 3]]);
        graphP->TrySetLabel(innerLoopNodes[edgeT], componentLabel, currentComponentID);
        graphP->TrySetLabel(graphP->EdgeMate(innerLoopNodes[edgeT]), componentLabel, currentComponentID);
        /*log.open("D:\\0test.txt", ios_base::app);
        log << " EDGE " + std::to_string(vtx[edge]) + "/" + std::to_string(vtx[((edge + 1) % 3)]) + "EDGE ID IS " + std::to_string(faceNodes[edge]) + " MATE ID IS " + std::to_string(graphP->EdgeMate(faceNodes[edge]))+" MASK IS " +
        std::string(graphP->GetMaskAt(faceNodes[edge], MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") << endl;
         log.close();*/
       /* if (!graphP->IsValidNodeId(faceNodes[edgeT]))
            {
            log.open("D:\\0test.txt", ios_base::app);
            log << "FACE EDGE " + std::to_string(vtx[edgeT]) + "/" + std::to_string(vtx[((edgeT + 1) % 3)]) + "EDGE ID IS " + std::to_string(faceNodes[edgeT]) + " EDGE IS INVALID!!" << endl;
            log.close();
            }
        MTGNodeId foundEdge = std::find_if(edgeLabels[vtx[edgeT] - 1]->begin(), edgeLabels[vtx[edgeT] - 1]->end(), [&vtx, &edgeT] (const std::pair<long, MTGNodeId>& a) { return a.first == vtx[(edgeT + 1) % 3]; })->second;
        if (!graphP->IsValidNodeId(foundEdge))
            {
            log.open("D:\\0test.txt", ios_base::app);
            log << "LINKED VERTEX EDGE " + std::to_string(vtx[edge]) + "/" + std::to_string(vtx[((edge + 1) % 3)]) + "EDGE ID IS " + std::to_string(foundEdge) + " EDGE IS INVALID!!" << endl;
            log.close();
            }*/
        }
  
    if (componentLabels[currentComponentID] != oldId && oldId != -1)
        {
        pointsToUpdate[0] = oldId;
        toUpdate = true;
        }
    for (int id = 0; id < 3; id++)
        {
        if (existingIDs[id] != -1 && componentLabels[existingIDs[id]] && componentLabels[existingIDs[id]] != componentLabels[currentComponentID])
            {
            pointsToUpdate[id + 1] = (componentLabels[existingIDs[id]]);
            toUpdate = true;
            componentLabels[existingIDs[id]] = componentLabels[currentComponentID];
            }
        }
    if (toUpdate)
        {
        for (auto& pt : componentLabels)
            {
            if (pointsToUpdate[0] == pt || pointsToUpdate[1] == pt || pointsToUpdate[2] == pt || pointsToUpdate[3] == pt)
                {
                pt = componentLabels[currentComponentID];
                }
            }
        }
#ifndef NDEBUG
    assert(innerLoopNodes[0] != innerLoopNodes[1] && innerLoopNodes[1] != innerLoopNodes[2] && innerLoopNodes[2] != innerLoopNodes[0]);
    MTGARRAY_FACE_LOOP(currentEdge, graphP, innerLoopNodes[0])
        {

              int v=-1,vn=-1;
     /*         std::string tlog = "CURRENT EDGE ";
              MTGARRAY_FACE_LOOP(nId, graphP, currentEdge)
                  {
                  int v;
                  graphP->TryGetLabel(nId, 0, v);
                  tlog += std::to_string(nId)+":"+std::to_string((int)v) + " ";
                  }
              MTGARRAY_END_FACE_LOOP(nId, graphP, currentEdge)
                  tlog += " EDGE MATE: ";
              MTGARRAY_FACE_LOOP(nId, graphP, graphP->EdgeMate(currentEdge))
                  {
                  int v;
                  graphP->TryGetLabel(nId, 0, v);
                  tlog += std::to_string(nId) + ":" + std::to_string((int)v) + " ";
                  }
              MTGARRAY_END_FACE_LOOP(nId, graphP, graphP->EdgeMate(currentEdge))*/
        assert(graphP->TryGetLabel(currentEdge, vertexLabel, v) && v != -1 );
        assert(graphP->TryGetLabel(graphP->FSucc(currentEdge), vertexLabel, vn) && vn != -1  && vn != v);
        assert(graphP->TryGetLabel(graphP->EdgeMate(currentEdge), vertexLabel, v) && v != -1);
        assert(graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(currentEdge)), vertexLabel, vn) && vn != -1 && vn != v);
        //assert(graphP->TryGetLabel(graphP->EdgeMate(currentEdge), vertexLabel, v) && v != -1 && v <= pointCount);
        assert(graphP->CountNodesAroundFace(graphP->EdgeMate(currentEdge)) == 3 || graphP->CountNodesAroundFace(currentEdge) == 3);
        assert(graphP->GetMaskAt(currentEdge, MTG_BOUNDARY_MASK) == graphP->GetMaskAt(graphP->EdgeMate(currentEdge), MTG_BOUNDARY_MASK));

        if (graphP->GetMaskAt(currentEdge, MTG_BOUNDARY_MASK) && graphP->CountNodesAroundFace(currentEdge) > 3)
            {
            }
        else if (graphP->GetMaskAt(graphP->EdgeMate(currentEdge), MTG_BOUNDARY_MASK) && graphP->CountNodesAroundFace(graphP->EdgeMate(currentEdge)) > 3)
            {
            MTGARRAY_FACE_LOOP(bound, graphP, graphP->EdgeMate(currentEdge))
                {
                //                    MTGNodeId edge = bound;
                  assert(graphP->GetMaskAt(bound, MTG_BOUNDARY_MASK));
                if (!graphP->GetMaskAt(bound, MTG_BOUNDARY_MASK))
                    {
                 /*   log.open("D:\\0test.txt", ios_base::app);
                     log << " WRONG BOUNDARY IDENTIFICATION! AT " + std::to_string(bound) + " FACE HAS " + std::to_string(graphP->CountNodesAroundFace(bound)) +
                    " MATE HAS " + std::to_string(graphP->CountNodesAroundFace(graphP->EdgeMate(bound))) << endl;
                    log << std::to_string(graphP->FPred(bound)) + "->[" + std::to_string(bound) + "]->" + std::to_string(graphP->FSucc(bound))
                    + "->" + std::to_string(graphP->FSucc(graphP->FSucc(bound))) << endl;
                    MTGARRAY_FACE_LOOP(eId, graphP, bound)
                        {
                        log << std::to_string(eId) + "->";
                        }
                    MTGARRAY_END_FACE_LOOP(eId, graphP, bound)
                        log << endl;
                    log.close();*/
                    }
                }
            MTGARRAY_END_FACE_LOOP(bound, graphP, graphP->EdgeMate(currentEdge))
            }
        else if (nExisting == 0 && graphP->CountNodesAroundFace(currentEdge) == 3 && graphP->CountNodesAroundFace(graphP->EdgeMate(currentEdge)) == 3)
            {
            assert(graphP->GetMaskAt(graphP->EdgeMate(currentEdge), MTG_BOUNDARY_MASK) && graphP->GetMaskAt(graphP->EdgeMate(graphP->FSucc(currentEdge)), MTG_BOUNDARY_MASK)
                   && graphP->GetMaskAt(graphP->EdgeMate(graphP->FSucc(graphP->FSucc(currentEdge))), MTG_BOUNDARY_MASK) && graphP->GetMaskAt(currentEdge, MTG_BOUNDARY_MASK)
                   && graphP->GetMaskAt(graphP->FSucc(currentEdge), MTG_BOUNDARY_MASK) && graphP->GetMaskAt(graphP->FSucc(graphP->FSucc(currentEdge)), MTG_BOUNDARY_MASK));
            }
        }
    MTGARRAY_END_FACE_LOOP(currentEdge, graphP, innerLoopNodes[0])
#ifdef ENABLE_EXTRA_ASSERTS
        MTGARRAY_SET_LOOP(allEdges, graphP)
        {
        int v = -1, vn = -1;
        assert(graphP->TryGetLabel(allEdges, vertexLabel, v) && v != -1);
        assert(graphP->TryGetLabel(graphP->FSucc(allEdges), vertexLabel, vn) && vn != -1 && vn != v);
        assert(graphP->TryGetLabel(graphP->EdgeMate(allEdges), vertexLabel, v) && v != -1);
        assert(graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(allEdges)), vertexLabel, vn) && vn != -1 && vn != v);
        }
    MTGARRAY_END_SET_LOOP(allEdges, graphP)
#endif
#endif
        assert(graphP->FSucc(innerLoopNodes[0]) == innerLoopNodes[1] && graphP->FSucc(innerLoopNodes[1]) == innerLoopNodes[2] && graphP->FSucc(innerLoopNodes[2]) == innerLoopNodes[0]);
        return true;
    }


void ExtractFaceIndexListFromGraph(std::vector<int>& faceIndexBuffer, MTGGraph* graphP)
    {
    MTGMask visitedMask = graphP->GrabMask();
    MTGARRAY_SET_LOOP(start, graphP)
        {
        if (graphP->GetMaskAt(start, MTG_EXTERIOR_MASK)) continue;
        if (graphP->GetMaskAt(start, visitedMask)) continue;
        assert(graphP->CountNodesAroundFace(start) == 3);
        std::vector<int> faceIndices;
        MTGARRAY_FACE_LOOP(faceId, graphP, start)
            {
            int vIndex = -1;
            graphP->TryGetLabel(faceId, 0, vIndex);
            assert(vIndex != -1);
            assert(vIndex > 0);
            faceIndexBuffer.push_back(vIndex);
            faceIndices.push_back(vIndex);
            graphP->SetMaskAt(faceId, visitedMask);
            }
        MTGARRAY_END_FACE_LOOP(faceId, graphP, start)
            assert(faceIndices[0] != faceIndices[1] && faceIndices[1] != faceIndices[2] && faceIndices[2] != faceIndices[0]);
        }
    MTGARRAY_END_SET_LOOP(start, graphP)
    graphP->ClearMask(visitedMask);
    graphP->DropMask(visitedMask);
    }


void AddFacesToGraph(MTGGraph* destGraphP, std::vector<int>& faces, std::vector<std::vector<std::pair<long, MTGNodeId>>*>& existingEdges, std::vector<int>& indexMapping, std::vector<DPoint3d>& pointsInGraph, std::vector<DPoint3d>& pointsInFaces, bvector<int>& componentContours)
    {
   // std::ofstream log;
   // log.open("D:\\dctest.txt", ios_base::app);
   // log.open("d:\\0test.txt", ios_base::app);
    int nFrem = 0;
    std::vector<int> componentLabels(componentContours.size());
  //  log << "PROCESSING TILE WITH " + std::to_string(faces.size()) + " FACE DEFINITIONS" << endl;
    //int componentLabel = 1;
    for (int i = 0; i < (int)faces.size(); i += 3)
        {
        int j = i % 3 < 2 ? i + 1 : i / 3;
        int k = j + 1;
        assert(faces[i] != faces[j] && faces[j] != faces[k] && faces[i] != faces[k]);
       // MTGNodeId faceNodes[3] = { -1, -1, -1 }, existing[3] = { -1, -1, -1 };
        int vtx[3] = { faces[i], faces[j], faces[k] };
        for (int v = 0; v < 3; v++)
            {
            if (vtx[v] < indexMapping.size() && indexMapping[vtx[v] - 1] != -1 /*&& indexMapping[vtx[v] - 1] < (int)pointsInGraph.size()*/) vtx[v] = indexMapping[vtx[v] - 1] + 1;
            else
                {
                pointsInGraph.push_back(pointsInFaces[vtx[v] - 1]);
                if (vtx[v] - 1 >= indexMapping.size())  indexMapping.resize(vtx[v],-1);
                indexMapping[vtx[v] - 1] = (int)pointsInGraph.size() - 1;
                vtx[v] = indexMapping[vtx[v] - 1] + 1;
                }
            assert(vtx[v] <= (int)pointsInGraph.size());
            if (vtx[v] - 1 >= existingEdges.size()) existingEdges.resize(vtx[v], nullptr);
            }
        if (vtx[0] == vtx[1] || vtx[1] == vtx[2] || vtx[2] == vtx[0]) continue; //NEEDS_WORK_SM: see what's up with this case
        if (!ProcessFaceToAdd(destGraphP, (int*)vtx, componentLabels, &pointsInGraph[0], &existingEdges[0])) nFrem++;
        continue;
        }
        std::set<int> boundaryPts;
        for (int pointID : componentLabels)
            {
            boundaryPts.insert(pointID);
            }
        for (int pointID : boundaryPts)
            {
            componentContours.push_back(pointID);
            }
        MTGNodeId exteriorEdge = -1;
        MTGARRAY_SET_LOOP(boundaryEdge, destGraphP)
            {
            if (destGraphP->GetMaskAt(boundaryEdge, MTG_BOUNDARY_MASK))
                {
                assert(destGraphP->GetMaskAt(destGraphP->EdgeMate(boundaryEdge), MTG_EXTERIOR_MASK) || destGraphP->GetMaskAt(destGraphP->EdgeMate(boundaryEdge), MTG_BOUNDARY_MASK));
                exteriorEdge = boundaryEdge;
               /* if (destGraphP->CountNodesAroundFace(boundaryEdge) != 3 && destGraphP->CountNodesAroundFace(destGraphP->EdgeMate(boundaryEdge)) != 3)
                    {
                    log << " [EXAMPLE] edge " + std::to_string((int)boundaryEdge) + " has number of nodes " + std::to_string(destGraphP->CountNodesAroundFace(boundaryEdge)) +
                        " masks are " + std::to_string((int)destGraphP->FPred(boundaryEdge)) + "(" + std::string(destGraphP->GetMaskAt(destGraphP->FPred(boundaryEdge), MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") + ")->"
                        + std::to_string((int)boundaryEdge) + "(" + std::string(destGraphP->GetMaskAt(boundaryEdge, MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") + std::string(")->")
                        + std::to_string((int)destGraphP->FSucc(boundaryEdge)) + std::string("(") + std::string(destGraphP->GetMaskAt(destGraphP->FSucc(boundaryEdge), MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") + ")";
                    log << endl;
                    log << std::to_string(boundaryEdge) + "=" + std::to_string(destGraphP->EdgeMate(boundaryEdge)) + "(" + std::to_string(destGraphP->CountNodesAroundFace(destGraphP->EdgeMate(boundaryEdge))) + ")(" + std::string(destGraphP->GetMaskAt(destGraphP->EdgeMate(boundaryEdge), MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") + ")";
                    log << " FACE " + std::to_string(destGraphP->FPred(destGraphP->EdgeMate(boundaryEdge))) + "-" + std::to_string(destGraphP->EdgeMate(boundaryEdge)) + "-" + std::to_string(destGraphP->FSucc(destGraphP->EdgeMate(boundaryEdge)));
                    log << endl;
                    log << std::to_string(destGraphP->FSucc(boundaryEdge)) + "=" + std::to_string(destGraphP->EdgeMate(destGraphP->FSucc(boundaryEdge))) + "(" + std::to_string(destGraphP->CountNodesAroundFace(destGraphP->EdgeMate(destGraphP->FSucc(boundaryEdge)))) + ")(" + std::string(destGraphP->GetMaskAt(destGraphP->EdgeMate(destGraphP->FSucc(boundaryEdge)), MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") + ")";
                    log << " FACE " + std::to_string(destGraphP->FPred(destGraphP->EdgeMate(destGraphP->FSucc(boundaryEdge)))) + "-" + std::to_string(destGraphP->EdgeMate(destGraphP->FSucc(boundaryEdge))) + "-" + std::to_string(destGraphP->FSucc(destGraphP->EdgeMate(destGraphP->FSucc(boundaryEdge))));
                    log << endl;
                    log << std::to_string(destGraphP->FPred(boundaryEdge)) + "=" + std::to_string(destGraphP->EdgeMate(destGraphP->FPred(boundaryEdge))) + "(" + std::to_string(destGraphP->CountNodesAroundFace(destGraphP->EdgeMate(destGraphP->FPred(boundaryEdge)))) + ")(" + std::string(destGraphP->GetMaskAt(destGraphP->EdgeMate(destGraphP->FPred(boundaryEdge)), MTG_BOUNDARY_MASK) ? "BOUNDARY" : "INTERIOR") + ")";
                    log << " FACE " + std::to_string(destGraphP->FPred(destGraphP->EdgeMate(destGraphP->FPred(boundaryEdge)))) + "-" + std::to_string(destGraphP->EdgeMate(destGraphP->FPred(boundaryEdge))) + "-" + std::to_string(destGraphP->FSucc(destGraphP->EdgeMate(destGraphP->FPred(boundaryEdge))));
                    log << endl << endl;
                    }*/
                assert(destGraphP->CountNodesAroundFace(boundaryEdge) == 3 || destGraphP->CountNodesAroundFace(destGraphP->EdgeMate(boundaryEdge)) == 3);
                if (destGraphP->CountNodesAroundFace(boundaryEdge) == 3) exteriorEdge = destGraphP->EdgeMate(boundaryEdge);
                if (!destGraphP->GetMaskAt(exteriorEdge, MTG_BOUNDARY_MASK)) continue;
                if (destGraphP->GetMaskAt(destGraphP->EdgeMate(exteriorEdge), MTG_EXTERIOR_MASK)) continue;
                MTGARRAY_FACE_LOOP(currentEdge, destGraphP, exteriorEdge)
                    {
                    // assert(!graphP->GetMaskAt(graphP->EdgeMate(currentEdge), MTG_EXTERIOR_MASK));
                    if (!destGraphP->GetMaskAt(currentEdge, MTG_BOUNDARY_MASK)) break;
                    destGraphP->SetMaskAt(currentEdge, MTG_EXTERIOR_MASK);
                    destGraphP->ClearMaskAt(currentEdge, MTG_BOUNDARY_MASK);
                    }
                MTGARRAY_END_FACE_LOOP(currentEdge, destGraphP, exteriorEdge)
                    //break;
                }
            }
        MTGARRAY_END_SET_LOOP(boundaryEdge, destGraphP)
    //        log << " DONE ADDING FACES TO GRAPH. FACES REMOVED: " + std::to_string(nFrem) + " OF WHICH IGNORED " + std::to_string(nFi) + " for a total of " + std::to_string(faces.size()) << endl;
    //       log.close();
    }


void CreateGraphFromIndexBuffer(MTGGraph* graphP, const long* meshFaces, int count, int pointCount, bvector<int>& componentContours, const DPoint3d* points)
    {
    std::ofstream log;
    int nFrem = 0;
    int vertexLabel = graphP->DefineLabel(0, MTG_LabelMask_VertexProperty, -1);
    vertexLabel = vertexLabel;
    std::vector<std::pair<long, MTGNodeId>>** edgeLabels = new std::vector<std::pair<long, MTGNodeId>>*[pointCount]();
    std::fill_n(edgeLabels, pointCount, nullptr);
    std::vector<int> componentLabels;
    int componentLabel = graphP->DefineLabel(1, MTG_LabelMask_FaceProperty, -1);
    int* pointComponents = new int[pointCount];
    std::vector<int> extraFaces;
   // log.open("D:\\0test.txt", ios_base::app);
  //  log << "PROCESSING TILE WITH " + std::to_string(count) + " FACE DEFINITIONS" << endl;
  //  log.close();
    for (int i = 0; i < pointCount; i++) pointComponents[i] = i;
    for (int i = 0; i < count; i += 3)
        {
        int j = i % 3 < 2 ? i + 1 : i / 3;
        int k = j + 1;
        assert(meshFaces[i] != meshFaces[j] && meshFaces[j] != meshFaces[k] && meshFaces[k] != meshFaces[i]);
        assert(meshFaces[i] <= pointCount && meshFaces[j] <= pointCount  && meshFaces[k] <= pointCount);
        /*      log << " FACE IS " + std::to_string(meshFaces[i]) + "/" + std::to_string(meshFaces[j]) + "/" + std::to_string(meshFaces[k]) << endl;
              log << " FACE POINTS ARE (" + std::to_string(points[meshFaces[i] - 1].x) + "," + std::to_string(points[meshFaces[i] - 1].y) + "," + std::to_string(points[meshFaces[i] - 1].z) + ")("
              + std::to_string(points[meshFaces[j] - 1].x) + "," + std::to_string(points[meshFaces[j] - 1].y) + "," + std::to_string(points[meshFaces[j] - 1].z) + ")("
              + std::to_string(points[meshFaces[k] - 1].x) + "," + std::to_string(points[meshFaces[k] - 1].y) + "," + std::to_string(points[meshFaces[k] - 1].z) << endl;*/
        //       MTGNodeId faceNodes[3] = { -1, -1, -1 }, existing[3] = { -1, -1, -1 };
        if (!ProcessFaceToAdd(graphP, (const int*)meshFaces + i, componentLabels, points, (std::vector<std::pair<long, MTGNodeId>>**)edgeLabels)) nFrem++;
        continue;
        }
  /*   log.open("D:\\0test.txt", ios_base::app);
     log << " DONE CREATING GRAPH FOR MESH. FACES REMOVED: "+std::to_string(nFrem)+" OF WHICH IGNORED "+std::to_string(nFi) +" for a total of "+std::to_string(count)<< endl;
     log.close();*/
    //after graph is done, add "exterior" mask to exterior loop (outer "boundary" half-edge)
    MTGNodeId exteriorEdge = -1;
    /*
    MTGARRAY_SET_LOOP(boundaryEdge, graphP)
        {
        assert(graphP->CountNodesAroundFace(boundaryEdge) == 3 || graphP->CountNodesAroundFace(graphP->EdgeMate(boundaryEdge)) == 3);
        if (graphP->CountNodesAroundFace(boundaryEdge) > 3)
            {
            MTGARRAY_FACE_LOOP(currentEdge, graphP, boundaryEdge)
                {
                assert(graphP->CountNodesAroundFace(graphP->EdgeMate(currentEdge)) == 3);
                }
            MTGARRAY_END_FACE_LOOP(currentEdge, graphP, boundaryEdge)
            }
        }
    MTGARRAY_END_SET_LOOP(boundaryEdge, graphP)*/
   // log.open("D:\\0test.txt", ios_base::app);
        MTGARRAY_SET_LOOP(boundaryEdge, graphP)
        {
        if (graphP->GetMaskAt(boundaryEdge, MTG_BOUNDARY_MASK))
            {
            assert(graphP->GetMaskAt(graphP->EdgeMate(boundaryEdge), MTG_EXTERIOR_MASK) || graphP->GetMaskAt(graphP->EdgeMate(boundaryEdge), MTG_BOUNDARY_MASK));
            exteriorEdge = boundaryEdge;
            assert(graphP->CountNodesAroundFace(boundaryEdge) == 3 || graphP->CountNodesAroundFace(graphP->EdgeMate(boundaryEdge)) == 3);
           // if (graphP->CountNodesAroundFace(boundaryEdge) == 3 && graphP->CountNodesAroundFace(graphP->EdgeMate(boundaryEdge)) == 3)
           //     {
            if (graphP->CountNodesAroundFace(boundaryEdge) == 3) exteriorEdge = graphP->EdgeMate(boundaryEdge);
                if (!graphP->GetMaskAt(graphP->FSucc(exteriorEdge), MTG_BOUNDARY_MASK)) exteriorEdge = graphP->EdgeMate(exteriorEdge);
           //     }
           // else if (graphP->CountNodesAroundFace(boundaryEdge) == 3) exteriorEdge = graphP->EdgeMate(boundaryEdge);
            if (!graphP->GetMaskAt(exteriorEdge, MTG_BOUNDARY_MASK)) continue;
            if (graphP->GetMaskAt(graphP->EdgeMate(exteriorEdge), MTG_EXTERIOR_MASK)) continue;
            int initCompId = -1;
            graphP->TryGetLabel(exteriorEdge, componentLabel, initCompId);
            MTGARRAY_FACE_LOOP(currentEdge, graphP, exteriorEdge)
                {
                int v1=-1, v2=-1;
                graphP->TryGetLabel(currentEdge, 0, v1);
                graphP->TryGetLabel(graphP->EdgeMate(currentEdge), 0, v2);
                int extCompId = -1;
                graphP->TryGetLabel(currentEdge, componentLabel, extCompId);
                graphP->SetMaskAt(currentEdge, MTG_EXTERIOR_MASK);
               /* log << "SETTING EXTERIOR MASK: " + std::to_string(currentEdge) + " (|" + std::to_string(graphP->EdgeMate(currentEdge)) + ")" << endl;
                log << "EDGE FIRST POINT: (" + std::to_string(points[v1 - 1].x) + "," + std::to_string(points[v1 - 1].y) + "," + std::to_string(points[v1 - 1].z) + ")" << endl;
                log << std::to_string(graphP->FPred(currentEdge)) + "->" + std::to_string(currentEdge) + std::to_string(graphP->FSucc(currentEdge)) << endl;*/
                graphP->ClearMaskAt(currentEdge, MTG_BOUNDARY_MASK);
                }
            MTGARRAY_END_FACE_LOOP(currentEdge, graphP, exteriorEdge)
                //break;
            }
        //assert(graphP->CountNodesAroundFace(boundaryEdge) == 3 || graphP->CountNodesAroundFace(graphP->EdgeMate(boundaryEdge)) == 3);
        }
        MTGARRAY_END_SET_LOOP(boundaryEdge, graphP)
        //    log << "DONE SETTING EXTERIOR MASKS." << endl;
        //    log.close();
        std::set<int> boundaryPts;
    for (int& pointID : componentLabels)
        {
  //      log << " POINT CHOSEN " + std::to_string(pointID) + " FOR COMPONENT " + std::to_string(&pointID - &componentLabels[0]) << endl;
        if (pointID >= 0) boundaryPts.insert(pointID);
        }
    for (int pointID : boundaryPts)
        {
 //       log << " SELECTED POINT AT " + std::to_string(pointID) + " IDX " + std::to_string(pointID + 1);
 //       log << " COORDS ARE (" + std::to_string(points[pointID].x) + "," + std::to_string(points[pointID].y) + "," + std::to_string(points[pointID].z) + ")" << endl;
        componentContours.push_back(pointID);
        }
 //   log.close();

        //test
#ifndef NDEBUG
    for (int i = 0; i < pointCount; i++) if (edgeLabels[i] != nullptr) edgeLabels[i]->clear();
   // std::vector<std::map<int, int>> maps(pointCount);
        MTGARRAY_SET_LOOP(boundaryEdge, graphP)
        {
        if (graphP->GetMaskAt(boundaryEdge, MTG_BOUNDARY_MASK))
            {
            assert(graphP->CountNodesAroundFace(boundaryEdge) == 3);
            }
        else if (!graphP->GetMaskAt(boundaryEdge, MTG_EXTERIOR_MASK)) assert(graphP->CountNodesAroundFace(boundaryEdge) == 3);
        else assert(graphP->CountNodesAroundFace(graphP->EdgeMate(boundaryEdge)) == 3);

        int v=-1,vx=-1;
        assert(graphP->TryGetLabel(boundaryEdge, vertexLabel, v) && v != -1 && v <= pointCount);
        assert(graphP->TryGetLabel(graphP->FSucc(boundaryEdge), vertexLabel, vx) && vx != -1 && vx != v && vx <= pointCount);
        auto it = std::find_if(edgeLabels[v - 1]->begin(), edgeLabels[v - 1]->end(), [&vx] (std::pair<long,MTGNodeId>& test)->bool { return vx - 1 == test.first; });
        if (it == edgeLabels[v - 1]->end())
            {
            edgeLabels[v - 1]->push_back(std::make_pair(vx - 1,boundaryEdge));
            //maps[v - 1][vx - 1] = 1;
            }
        else
            {
            //assert(it->second == graphP->EdgeMate(boundaryEdge));
            //maps[v - 1][vx - 1]++;
            assert(false);
            }
        //assert(maps[v - 1][vx - 1] <= 2);
        assert(graphP->TryGetLabel(boundaryEdge, componentLabel, v) && v != -1);
        assert(graphP->TryGetLabel(graphP->FSucc(boundaryEdge), componentLabel, vx) && vx != -1);
        }
    MTGARRAY_END_SET_LOOP(boundaryEdge, graphP)
#endif
        for (int i = 0; i < pointCount; i++) if (edgeLabels[i] != nullptr) delete edgeLabels[i];
    delete[] edgeLabels;
    delete[] pointComponents;
    }

void RemovePolygonInteriorFromGraph(MTGGraph* graphP, bvector<DPoint3d>& polygon, const DPoint3d* points, DPoint3d& minCorner, DPoint3d& maxCorner)
    {
    MTGMask toDeleteMask = graphP->GrabMask();
    //start traversing destgraphP triangles
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        DPoint3d face[3];
        MTGNodeId faceEdges[3];
        DPoint3d midpointEdge = DPoint3d::FromXYZ(0, 0, 0);
        int n = 0, vIndex = -1;
        MTGARRAY_FACE_LOOP(currentID, graphP, edgeID)
            {
            if (n > 2)
                {
                n++;
                break;
                }
            graphP->TryGetLabel(currentID, 0, vIndex);
            assert(vIndex > 0);
            faceEdges[n] = currentID;
            face[n] = points[vIndex - 1];

            n++;
            }
        MTGARRAY_END_FACE_LOOP(currentID, graphP, edgeID)
            if (n > 3) continue;

        DSegment3d edge1 = DSegment3d::From(face[0], face[1]);
        DSegment3d edge2 = DSegment3d::From(face[1], face[2]);
        DSegment3d edge3 = DSegment3d::From(face[2], face[0]);
        double lengthline;
        if (edge3.LengthSquared() >= edge2.LengthSquared() && edge3.LengthSquared() >= edge1.LengthSquared()) edge3.WireCentroid(lengthline, midpointEdge);
        else if (edge2.LengthSquared() >= edge1.LengthSquared() && edge2.LengthSquared() >= edge3.LengthSquared())edge2.WireCentroid(lengthline, midpointEdge);
        else edge1.WireCentroid(lengthline, midpointEdge);
        bvector<DPoint3d> line;
        bvector<DPoint3d> line2;
        line.push_back(midpointEdge);
        line2.push_back(midpointEdge);

        line.push_back(minCorner);
        line2.push_back(maxCorner);
        bvector<CurveLocationDetail> intersectionsA, intersectionsB;
        //for each triangle, intersect line from triangle centroid to box corners of extent

        PolylineOps::CollectIntersectionsAndCloseApproachesXY(line, nullptr, polygon, nullptr, intersectionsA, intersectionsB, 0);
        if (intersectionsA.size() % 2 == 1)
            {
            for (int i = 0; i < 3; i++) graphP->SetMaskAt(faceEdges[i], toDeleteMask);
            continue;
            }
        PolylineOps::CollectIntersectionsAndCloseApproachesXY(line2, nullptr, polygon, nullptr, intersectionsA, intersectionsB, 0);
        if (intersectionsA.size() % 2 == 1)
            {
            for (int i = 0; i < 3; i++) graphP->SetMaskAt(faceEdges[i], toDeleteMask);
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
        //drop all edges where both mates were marked for deletion
        MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        if (graphP->GetMaskAt(edgeID, toDeleteMask))
            {
            if (graphP->GetMaskAt(graphP->EdgeMate(edgeID), toDeleteMask)) graphP->DropEdge(edgeID);
            else if (graphP->GetMaskAt(graphP->EdgeMate(edgeID), MTG_EXTERIOR_MASK)) graphP->DropEdge(edgeID);
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
        graphP->ClearMask(toDeleteMask);
    graphP->DropMask(toDeleteMask);

    }

void RemoveTrianglesWithinExtent(MTGGraph* graphP, const DPoint3d* points, DPoint3d& minCorner, DPoint3d& maxCorner)
    {
    MTGMask toDeleteMask = graphP->GrabMask();
    //start traversing destgraphP triangles
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        DPoint3d face[3];
        MTGNodeId faceEdges[3];
        int n = 0, vIndex = -1;
        MTGARRAY_FACE_LOOP(currentID, graphP, edgeID)
            {
            if (n > 2)
                {
                n++;
                break;
                }
            graphP->TryGetLabel(currentID, 0, vIndex);
            assert(vIndex > 0);
            faceEdges[n] = currentID;
            face[n] = points[vIndex - 1];

            n++;
            }
        MTGARRAY_END_FACE_LOOP(currentID, graphP, edgeID)
            if (n > 3) continue;

        DSegment3d edge1 = DSegment3d::From(face[0], face[1]);
        DSegment3d edge2 = DSegment3d::From(face[1], face[2]);
        DSegment3d edge3 = DSegment3d::From(face[2], face[0]);

        DSegment3d bottom = DSegment3d::From(DPoint3d::From(minCorner.x, minCorner.y, 0), DPoint3d::From(maxCorner.x, minCorner.y, 0));
        DSegment3d top = DSegment3d::From(DPoint3d::From(minCorner.x, maxCorner.y, 0), DPoint3d::From(maxCorner.x, maxCorner.y, 0));
        DSegment3d left = DSegment3d::From(DPoint3d::From(minCorner.x, minCorner.y, 0), DPoint3d::From(minCorner.x, maxCorner.y, 0));
        DSegment3d right = DSegment3d::From(DPoint3d::From(maxCorner.x, minCorner.y, 0), DPoint3d::From(maxCorner.x, maxCorner.y, 0));

        double param1, param2;
        DPoint3d pt1, pt2;

        if ((DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge1, top) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1) || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge2, top) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1)
            || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge3, top) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1) || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge1, bottom) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1)
            || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge2, bottom) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1) || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge3, bottom) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1)
            || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge1, left) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1) || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge2, left) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1)
            || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge3, left) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1) || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge1, right) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1)
            || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge2, right) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1) || (DSegment3d::IntersectXY(param1, param2, pt1, pt2, edge3, right) && param1 >= 0 && param1 <= 1 && param2 >= 0 && param2 <= 1))
           for(size_t i =0; i < 3; i++) graphP->SetMaskAt(faceEdges[i], toDeleteMask);
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
        //drop all edges where both mates were marked for deletion
        MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        if (graphP->GetMaskAt(edgeID, toDeleteMask))
            {
            if (graphP->GetMaskAt(graphP->EdgeMate(edgeID), toDeleteMask)) graphP->DropEdge(edgeID);
            else if (graphP->GetMaskAt(graphP->EdgeMate(edgeID), MTG_EXTERIOR_MASK)) graphP->DropEdge(edgeID);
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
        graphP->ClearMask(toDeleteMask);
    graphP->DropMask(toDeleteMask);

    }


void ReadFeatureEndTags(MTGGraph * graphP, std::vector<int>& pointToDestPointsMap, bvector<TaggedEdge>& featureEdges)
    {
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        int tag = -1;
        graphP->TryGetLabel(edgeID, 3, tag);
        if (tag != -1)
            {
            TaggedEdge feature;
            feature.tag = tag;
            graphP->TryGetLabel(edgeID, 0, feature.vtx1);
            graphP->TryGetLabel(graphP->EdgeMate(edgeID), 0, feature.vtx2);
            if (feature.vtx1 < pointToDestPointsMap.size() && pointToDestPointsMap[feature.vtx1 - 1] != -1) feature.vtx1 = pointToDestPointsMap[feature.vtx1 - 1];
            else feature.vtx1--;
            if (feature.vtx2 < pointToDestPointsMap.size() && pointToDestPointsMap[feature.vtx2 - 1] != -1) feature.vtx2 = pointToDestPointsMap[feature.vtx2 - 1];
            else feature.vtx2--;

            featureEdges.push_back(feature);
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
    }

void ReadFeatureTags(MTGGraph * graphP, std::vector<int>& pointToDestPointsMap, bvector<bvector<int32_t>>& features, std::map<int, int>& componentForPoints)
    {
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        int tag = -1;
        graphP->TryGetLabel(edgeID, 2, tag);
        if (tag != -1)
            {
            bvector<int32_t> feature(3);
            feature[0] = tag;
            graphP->TryGetLabel(edgeID, 0, feature[1]);
            graphP->TryGetLabel(graphP->EdgeMate(edgeID), 0, feature[2]);
            if (feature[1] < pointToDestPointsMap.size() && pointToDestPointsMap[feature[1] - 1] != -1) feature[1] = pointToDestPointsMap[feature[1] - 1];
            else feature[1]--;
            if (feature[2] < pointToDestPointsMap.size() && pointToDestPointsMap[feature[2] - 1] != -1) feature[2] = pointToDestPointsMap[feature[2] - 1];
            else feature[2]--;

            int component = -1;
            graphP->TryGetLabel(edgeID, 3, component);
            if (component != -1)
                {
                componentForPoints[feature[1]] = component;
                componentForPoints[feature[2]] = component;
                }

            features.push_back(feature);
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
    }

void ReadFeatureTags(MTGGraph * graphP, std::vector<int>& pointToDestPointsMap, bvector<bvector<int32_t>>& features)
    {
    std::map<int, int> pts;
    ReadFeatureTags(graphP, pointToDestPointsMap, features, pts);
    }

void ApplyEndTags(MTGGraph * graphP, bvector<TaggedEdge>& featureEdges)
    {
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        int v = -1;
        graphP->TryGetLabel(edgeID, 0, v);
        bool tagSet = false;
        for (auto& edge : featureEdges)
            {
            if ((v == edge.vtx1 || v == edge.vtx2))
                {
                MTGARRAY_VERTEX_LOOP(edgeId2, graphP, edgeID)
                    {
                    int v1 = -1;
                    graphP->TryGetLabel(graphP->FSucc(edgeId2), 0, v1);
                    if (v1 == edge.vtx1 || v1 == edge.vtx2)
                        {
                        graphP->TrySetLabel(edgeId2, 3, edge.tag);
                        tagSet = true;
                        break;
                        }
                    }
                MTGARRAY_END_VERTEX_LOOP(edgeId2, graphP, edgeID)
                    if (tagSet) break;
                }
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
    }

    void MergeGraphs(MTGGraph * destGraphP, std::vector<DPoint3d>& destPoints, MTGGraph * srcGraphP, std::vector<DPoint3d>& inPoints, DPoint3d minCorner, DPoint3d maxCorner, std::vector<int>& pointToDestPointsMap, bvector<int>& componentContours)
    {
    bvector<bvector<DPoint3d>> contours;
    std::set<MTGNodeId>* exterior = new std::set<MTGNodeId>();
    int i = 0;
#ifndef NDEBUG
   /* MTGARRAY_SET_LOOP(edgeID, destGraphP)
        {
        assert(destGraphP->CountNodesAroundFace(edgeID) == 3 || destGraphP->CountNodesAroundFace(destGraphP->EdgeMate(edgeID)) == 3);      
        }
    MTGARRAY_END_SET_LOOP(edgeID, destGraphP)*/
#endif
    //get contours 
    MTGARRAY_SET_LOOP(boundaryEdgeID, srcGraphP)
        {
        if (srcGraphP->GetMaskAt(boundaryEdgeID, MTG_EXTERIOR_MASK) && exterior->count(boundaryEdgeID) == 0)
            {
            contours.resize(i + 1);
            MTGARRAY_FACE_LOOP(currentID, srcGraphP, boundaryEdgeID)
                {
                if (!srcGraphP->GetMaskAt(currentID, MTG_EXTERIOR_MASK))
                    {
                    srcGraphP->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
                        srcGraphP->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
                    }
                exterior->insert(currentID);
                int vIndex = -1;
                srcGraphP->TryGetLabel(currentID, 0, vIndex);
                assert(vIndex > 0);
                DPoint3d pt = inPoints[vIndex - 1];
                contours[i].push_back(pt);
                }
            MTGARRAY_END_FACE_LOOP(currentID, srcGraphP, boundaryEdgeID)
                i++;
            }
        }
    MTGARRAY_END_SET_LOOP(boundaryEdgeID, srcGraphP)
        delete exterior;
    MTGMask toDeleteMask = destGraphP->GrabMask();

        std::vector<std::vector<std::pair<long,MTGNodeId>>*> remainingEdgesForStitchContour(destPoints.size());
    std::fill_n(remainingEdgesForStitchContour.begin(), remainingEdgesForStitchContour.size(), nullptr);
    MTGMask stitchedEdgeMask = destGraphP->GrabMask();
    //drop all edges where both mates were marked for deletion
    //destGraphP->ClearMask(MTG_EXTERIOR_MASK);
    //destGraphP->ClearMask(MTG_BOUNDARY_MASK);
    //std::ofstream log;
    //log.open("D:\\dctest2.txt", ios_base::app);
    //log << "-~-~-~-~-~~-~-~-~-~-~-~" << endl;
    MTGARRAY_SET_LOOP(edgeID, destGraphP)
        {
        if (destGraphP->GetMaskAt(edgeID, toDeleteMask))
            {
            if (destGraphP->GetMaskAt(destGraphP->EdgeMate(edgeID), toDeleteMask)) destGraphP->DropEdge(edgeID);
            else if (destGraphP->GetMaskAt(destGraphP->EdgeMate(edgeID), MTG_EXTERIOR_MASK))destGraphP->DropEdge(edgeID);
            else        //gather mapping from points to edge to make adding faster later
                {
                int vIndex = -1,index2=-1;
                destGraphP->TryGetLabel(edgeID, 0, vIndex);
                destGraphP->TryGetLabel(destGraphP->EdgeMate(edgeID), 0, index2);
                assert(vIndex > 0 && index2 > 0 && vIndex <= (int)destPoints.size() && index2 <= (int)destPoints.size());
                if (remainingEdgesForStitchContour[vIndex - 1] == nullptr) remainingEdgesForStitchContour[vIndex - 1] = new std::vector<std::pair<long, MTGNodeId>>();
                remainingEdgesForStitchContour[vIndex - 1]->push_back(std::make_pair(index2,edgeID));
                if (remainingEdgesForStitchContour[index2 - 1] == nullptr) remainingEdgesForStitchContour[index2 - 1] = new std::vector<std::pair<long, MTGNodeId>>();
                remainingEdgesForStitchContour[index2 - 1]->push_back(std::make_pair(vIndex, edgeID));
                destGraphP->SetMaskAt(edgeID, stitchedEdgeMask);
                destGraphP->SetMaskAroundEdge(edgeID, MTG_BOUNDARY_MASK);
              /*  log << "REMAINING CONTOUR EDGE " + std::to_string(edgeID) + " AS " + std::to_string(destGraphP->FPred(edgeID)) + "->";
                 log << std::to_string(edgeID) + "->" + std::to_string(destGraphP->FSucc(edgeID)) + " COUNT IS "+ std::to_string(destGraphP->CountNodesAroundFace(edgeID));
                log << endl;*/
                }
            }
        if (destGraphP->GetMaskAt(edgeID, MTG_BOUNDARY_MASK))destGraphP->SetMaskAroundEdge(edgeID, MTG_BOUNDARY_MASK);
        }
    MTGARRAY_END_SET_LOOP(edgeID, destGraphP)
    //--------------------------
    destGraphP->ClearMask(toDeleteMask);
    destGraphP->ClearMask(MTG_EXTERIOR_MASK);
    //destGraphP->ClearMask(MTG_BOUNDARY_MASK);
    destGraphP->DropMask(toDeleteMask);


    //for triangles from original mesh, add triangle to stitched mesh and points to destination points

        std::vector<int> faces;
        ExtractFaceIndexListFromGraph(faces, srcGraphP);
        //get tagged edges
        AddFacesToGraph(destGraphP, faces, remainingEdgesForStitchContour, pointToDestPointsMap, destPoints, inPoints, componentContours);
        bvector<bvector<int32_t>> features;
        ReadFeatureTags(srcGraphP, pointToDestPointsMap, features);
       // bvector<TaggedEdge> featureEndEdges;
        //ReadFeatureEndTags(srcGraphP, pointToDestPointsMap, featureEndEdges);
        for (auto& feature : features)
            TagFeatureEdges(destGraphP, (DTMFeatureType)feature[0], feature.size() - 1, &feature[1], false);
        //ApplyEndTags(destGraphP, featureEndEdges);
       
        for (size_t i = 0; i < destPoints.size(); i++) if (remainingEdgesForStitchContour[i] != nullptr) delete remainingEdgesForStitchContour[i];

    MTGMask visitedMask = destGraphP->GrabMask();
    MTGARRAY_SET_LOOP(edgeID, destGraphP)
        {
        if (destGraphP->GetMaskAt(edgeID, visitedMask)) continue;
        int n = 0;
        MTGARRAY_FACE_LOOP(faceEdgeID, destGraphP, edgeID)
            {
            if (n > 2)
                {
                ++n; 
                break;
                }
            ++n;
            }
        MTGARRAY_END_FACE_LOOP(faceEdgeID, destGraphP, edgeID)
        /*if (n <= 3)
            {
            MTGARRAY_FACE_LOOP(faceEdgeID, destGraphP, edgeID)
                {
                if (!destGraphP->GetMaskAt(faceEdgeID, MTG_BOUNDARY_MASK)) destGraphP->SetMaskAt(faceEdgeID, MTG_NULL_MASK);
                destGraphP->SetMaskAt(faceEdgeID, visitedMask);
            MTGARRAY_END_FACE_LOOP(faceEdgeID, destGraphP, edgeID)
            }*/
        if (n > 3)
            {
            MTGARRAY_FACE_LOOP(faceEdgeID, destGraphP, edgeID)
                {
                destGraphP->SetMaskAt(faceEdgeID, MTG_EXTERIOR_MASK);
                destGraphP->SetMaskAt(faceEdgeID, visitedMask);
                if (destGraphP->GetMaskAt(destGraphP->EdgeMate(faceEdgeID), MTG_EXTERIOR_MASK))
                    {
                   /* log << "[ASSERT] Exterior edge " + std::to_string(faceEdgeID);
                    log << " prev " + std::to_string(destGraphP->FPred(faceEdgeID)) + " succ " + std::to_string(destGraphP->FSucc(faceEdgeID));
                    log << endl;*/
                    }
                destGraphP->SetMaskAt(destGraphP->EdgeMate(faceEdgeID), MTG_BOUNDARY_MASK);
                }
                MTGARRAY_END_FACE_LOOP(faceEdgeID, destGraphP, edgeID)
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, destGraphP)
    destGraphP->ClearMask(stitchedEdgeMask);
    destGraphP->DropMask(stitchedEdgeMask);
    destGraphP->ClearMask(visitedMask);
    destGraphP->DropMask(visitedMask);

    //-----------------------------
    //log << "-~-~-~-~-~~-~-~-~-~-~-~" << endl;
   // log.close();
    }

    void GetFaceDefinition(MTGGraph* graphP, int* outTriangle, MTGNodeId& edge)
        {
        int i = 0;
        MTGARRAY_FACE_LOOP(faceEdgeId, graphP, edge)
            {
            if (i >= 3) return;
            int v = -1;
            graphP->TryGetLabel(faceEdgeId, 0, v);
            outTriangle[i] = v;
            i++;
            }
        MTGARRAY_END_FACE_LOOP(faceEdgeId, graphP, edge)
        }

    MTGNodeId FindFaceInGraph(MTGGraph* graphP, int vertex1, int vertex2, int vertex3)
        {
        /* size_t nNodesInGraph = (size_t)(graphP)->GetNodeIdCount();
         bool found = false;
         int minV = std::min(vertex1, std::min(vertex2, vertex3));
         MTGNodeId edgeId = (MTGNodeId)nNodesInGraph / 2;
         int upper = (MTGNodeId)nNodesInGraph;
         int lower = 0;
         while (!found)
         {
         if (edgeId == -1 || edgeId == (MTGNodeId)nNodesInGraph) break;
         int v = -1, v1 = -1;
         graphP->TryGetLabel(edgeId, 0,v);
         if (v < minV)
         {
         lower = edgeId;
         if (edgeId == (MTGNodeId)nNodesInGraph - 1) edgeId = (MTGNodeId)nNodesInGraph;
         else edgeId = lower+ (upper-lower) / 2;
         continue;
         }
         else if (v > minV)
         {
         upper = edgeId;
         if (edgeId == 1) edgeId = 0;
         else if (edgeId == 0) edgeId = -1;
         else
         {
         edgeId = lower + (upper - lower) / 2;
         }
         continue;
         }*/

        MTGARRAY_SET_LOOP(edgeId, graphP)
            {
            int v = -1;
            graphP->TryGetLabel(edgeId, 0, v);
            if ((v == vertex1 || v == vertex2 || v == vertex3))
                {
                MTGARRAY_VERTEX_LOOP(edgeId2, graphP, edgeId)
                    {
                    int v1 = -1;
                    graphP->TryGetLabel(graphP->FSucc(edgeId2), 0, v1);
                    if ((v1 == vertex1 || v1 == vertex2 || v1 == vertex3))
                        {
                        int v2 = -1;
                        graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(edgeId2)), 0, v2);
                        if (v2 == vertex1 || v2 == vertex2 || v2 == vertex3) return edgeId2;
                        graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(graphP->EdgeMate(edgeId2))), 0, v2);
                        if (v2 == vertex1 || v2 == vertex2 || v2 == vertex3) return graphP->EdgeMate(edgeId2);
                        //return -1;
                        }
                    }
                MTGARRAY_END_VERTEX_LOOP(edgeId2, graphP, edgeId)
                }
            }
        MTGARRAY_END_SET_LOOP(edgeId, graphP)
            return -1;
        }

    //NEEDSWORK_SM: separate out all the extra info (segment, linePts etc) into a different function so that core code is reusable across the utility function for FollowPolylineOnGraph 
    //and the API function for finding a triangle in a direction
    bool FindNextTriangleOnRay(MTGNodeId& edge, DPoint3d& lastVertex, int* segment, MTGGraph* graphP, DRay3d toEdgeRay, const DPoint3d* linePts, const DPoint3d* points, bvector<bvector<DPoint3d>>& projectedPoints, int* nIntersect, const size_t nPoints, bool is3d)
        {
        DRange1d fraction;
        DSegment3d segmentRay;
        DRange3d triExt;
        volatile MTGNodeId bestEdge = -1;
        double closestFraction = DBL_MAX;
        DPoint3d pt, pt2;
        DPoint3d lastIntersect = DPoint3d::From(0,0,0);
        double paray1 = 0, paray2;
        DPoint3d proj1, proj2;
        DRay3d ray;
        if (segment != NULL)
            {
            ray = DRay3d::FromOriginAndVector(linePts[*segment + 1], DVec3d::From(0, 0, -1));
            bsiDRay3d_closestApproach(&paray1, &paray2, &proj1, &proj2, &toEdgeRay, &ray);
            }
        MTGNodeId extEdge = -1;
        MTGMask visitedMask = -1;
        if (is3d){
            visitedMask = graphP->GrabMask();
            graphP->ClearMask(visitedMask);
            }
        do
            {
            extEdge = -1;
            MTGARRAY_SET_LOOP(edgeID, graphP)
                {
                 if (is3d && graphP->GetMaskAt(edgeID, visitedMask)) continue;
                if (graphP->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
                    {
                    extEdge = edgeID;
                    break;
                    }
                }
            MTGARRAY_END_SET_LOOP(edgeID, graphP)

                MTGARRAY_FACE_LOOP(edgeID, graphP, extEdge)
                {
                 if (is3d) graphP->SetMaskAt(edgeID, visitedMask);
             
                //if (!graphP->GetMaskAt(edgeID, MTG_EXTERIOR_MASK)) continue;
                /* if (edgeID == edge || graphP->EdgeMate(edgeID) == edge || graphP->FSucc(edgeID) == edge || graphP->FSucc(graphP->FSucc(edgeID)) == edge
                     || edgeID == graphP->EdgeMate(edge) || graphP->FSucc(edgeID) == graphP->EdgeMate(edge) || graphP->FSucc(graphP->FSucc(edgeID)) == graphP->EdgeMate(edge)
                     || graphP->EdgeMate(edgeID) == edge || graphP->FSucc(graphP->EdgeMate(edgeID)) == edge || graphP->FSucc(graphP->FSucc(graphP->EdgeMate(edgeID))) == edge) continue;*/
                int triangle[3] = { 0 };
                graphP->TryGetLabel(graphP->EdgeMate(edgeID), 0, (int&)triangle[0]);
                graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(edgeID)), 0, (int&)triangle[1]);
                graphP->TryGetLabel(graphP->EdgeMate(graphP->FSucc(graphP->EdgeMate(edgeID))), 0, (int&)triangle[2]);
                if (triangle[0] < 1 || triangle[0] > (int)nPoints || triangle[1] < 1 || triangle[1] > (int)nPoints || triangle[2] < 1 || triangle[2] > (int)nPoints)
                    {
                    edgeID = graphP->FSucc(edgeID);
              
                    continue;
                    }
                DPoint3d pts[3] = { points[triangle[0] - 1], points[triangle[1] - 1], points[triangle[2] - 1] };
                triExt = DRange3d::From(pts[0], pts[1], pts[2]);

                triExt.low.z = -DBL_MAX; //Don't consider z.
                triExt.high.z = DBL_MAX; //NEEDS_WORK_SM: this is probably dependent on the specific drape direction (clip should be negated along the ray direction not just in z). Not sure how to modify that for the general case.
                if (!toEdgeRay.ClipToRange(triExt, segmentRay, fraction))
                    {
                    edgeID = graphP->FSucc(edgeID);
                    continue;
                    }
                if (edgeID == edge || graphP->FSucc(graphP->EdgeMate(edgeID)) == graphP->EdgeMate(edge) ||
                    graphP->FSucc(graphP->FSucc(graphP->EdgeMate(edgeID))) == graphP->EdgeMate(edge))
                    {
                    edgeID = graphP->FSucc(edgeID);
                    continue;
                    }
                bool intersectsTri = false;
                double params[3] = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
                double param, param2, lastParam = DBL_MAX;
                MTGNodeId prevLastEdge = bestEdge;
                DPoint3d prevLastIntersect = lastIntersect;
                double prevLastParam = lastParam;
                int nIntersects = 0;
                for (size_t i = 0; i < 3; i++)
                    {
                    DRay3d triEdgeRay = DRay3d::From(DSegment3d::From(pts[i], pts[(i + 1) % 3]));
                    triEdgeRay.direction.z = 0;
                    toEdgeRay.direction.z = 0;
                    if (bsiDRay3d_closestApproach(&param, &param2, &pt, &pt2, &toEdgeRay, &triEdgeRay) && param > -1e-8 && param2 >= 0 && param2 <= 1)
                        {
                        DRay3d drapeRay = DRay3d::FromOriginAndVector(pt, DVec3d::From(0, 0, -1));
                        DPoint3d pt1;
                        triEdgeRay.direction.z = pts[(i + 1) % 3].z - pts[i].z;
                        //there is a closest approach, now find out whether there is an intersection along the projection(drape) direction
                        if (bsiDRay3d_closestApproach(NULL, &param2, &pt1, &pt2, &drapeRay, &triEdgeRay) && param2 >= 0 && param2 <= 1 && DVec3d::FromStartEnd(pt1, pt2).MagnitudeSquared() < 0.01)
                            {
                            intersectsTri = true;
                            nIntersects++;
                            params[i] = param;
                            if (param < lastParam)lastParam = param;
                            if (lastParam < closestFraction)
                                {
                                bestEdge = graphP->EdgeMate(edgeID);
                                for (size_t j = 0; j < i; ++j) bestEdge = graphP->FSucc(bestEdge);
                                /*if (bestEdge == edge || bestEdge == graphP->EdgeMate(edge))
                                    {
                                    bestEdge = lastEdge;
                                    break;
                                    }*/
                                lastIntersect = pt2;
                                closestFraction = lastParam;
                                }
                            }
                        }
                    if (i == 2 && intersectsTri && lastParam != prevLastParam && params[0] <= lastParam && params[1] <= lastParam && params[2] <= lastParam && nIntersects <= 1)
                        {
                        if (lastParam >= 0.01 && nIntersect != NULL) *nIntersect = 1;
                        else
                            {
                            bestEdge = prevLastEdge;
                            lastIntersect = prevLastIntersect;
                            lastParam = prevLastParam;
                            }
                        }
                    }

                }
            MTGARRAY_END_FACE_LOOP(edgeID, graphP, extEdge)

            }
        while (is3d && extEdge != -1);

        if (is3d)
            {
            graphP->ClearMask(visitedMask);
            graphP->DropMask(visitedMask);
            }
            if (bestEdge == -1) return false;
        if (segment != NULL && closestFraction > paray1)
            {
            projectedPoints[*segment].push_back(proj2);
            ++(*segment);//segment ends before triangle is met
            lastIntersect = proj2;
            }
        edge = bestEdge;
        lastVertex = lastIntersect;
        return true;
        }

    bool FollowPolylineOnGraph(MTGGraph* graphP, DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, MTGNodeId& triangleStartEdge, int* segment, const DPoint3d* linePoints, int nLinePts, DPoint3d startPt, const size_t nPoints, bool is3d)
        {
        bool stopProgression = false;
        MTGNodeId currentTriangle = triangleStartEdge;
        int triangle[3];
        DVec3d drapeDirection = DVec3d::From(0, 0, -1);//NEEDS_WORK_SM: We may want to support different directions
        graphP->TryGetLabel(currentTriangle, 0, triangle[0]);
        graphP->TryGetLabel(graphP->FSucc(currentTriangle), 0, triangle[1]);
        graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(currentTriangle)), 0, triangle[2]);
        DPoint3d pts[3] = { points[triangle[0]-1], points[triangle[1]-1], points[triangle[2]-1] };
        DRay3d ray = DRay3d::FromOriginAndVector(startPt, drapeDirection); 
        DPoint3d bary, projectedPt;
        double param;
        bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, pts);
        DPoint3d currentVertex = projectedPt;
        if (*segment >= nLinePts - 1) return true;
        projectedPoints[*segment].push_back(currentVertex);
        int lastVtx1 = -1, lastVtx2 = -1;
        DVec3d currentLineSegmentDirection = DVec3d::FromStartEnd(linePoints[*segment], linePoints[*segment + 1]);
        currentLineSegmentDirection.Normalize();
        ray = DRay3d::FromOriginAndVector(linePoints[*segment + 1], drapeDirection);
        /*DPlane3d plane = DPlane3d::From3Points(pts[0], pts[1], pts[2]);
        ray.intersect(&projectedPt, &param, &plane);
        DPoint3d currentSegmentEnd = projectedPt;*/
        while (!stopProgression && *segment < nLinePts-1)
            {
            double p;
            DPoint3d pt2;
            pt2.SumOf(currentVertex, currentLineSegmentDirection);
            DRay3d toTriPlane = DRay3d::FromOriginAndVector(pt2, drapeDirection);
            bsiDRay3d_intersectTriangle(&toTriPlane, &pt2, &bary, &p, pts);
            DVec3d projDirection = DVec3d::FromStartEnd(currentVertex, pt2);
            //projectedPoints[*segment].push_back(pt2);
            DRay3d toNextPoint = DRay3d::FromOriginAndVector(currentVertex, projDirection);
            bool intersectFound = false;
            DPoint3d intersectPt = {0,0,0};
            size_t i = 0;
            bool backwards = false;
            DPoint3d lastPt = DPoint3d::From(0,0,0);
            double lastParam = -1;
            int intersectedEdge = -1;
            for (; i < 3 && !intersectFound; i++)
                {
                DSegment3d triEdge = DSegment3d::From(pts[i], pts[(i + 1) % 3]);
                DRay3d edgeRay = DRay3d::From(triEdge);
                double param2;
                DPoint3d intersectPt2;
                if (bsiDRay3d_closestApproach(&param, &param2, &intersectPt2, &intersectPt, &toNextPoint, &edgeRay))
                    {
                    if ((lastVtx1 == triangle[i] || lastVtx1 == triangle[(i + 1) % 3]) && (lastVtx2 == triangle[i] || lastVtx2 == triangle[(i + 1) % 3])) continue;
                    if (param2 < 0 || param2 > 1 || param < 1.0e-8) continue;
                    if(lastVtx1 != -1 && lastVtx2 != -1) intersectFound = true;
                    else if (lastParam == -1)
                        {
                        lastPt = intersectPt;
                        lastParam = param;
                        intersectedEdge = (int)i;
                        }
                    else if (lastParam > param)
                        {
                        intersectPt = lastPt;
                        param = lastParam;
                        intersectFound = true;
                        i = intersectedEdge;
                        }
                    else intersectFound = true;
                    }
                }
            if (intersectFound) intersectedEdge = (int)i;
            if (!intersectFound && lastParam != -1)
                {
                intersectFound = true;
                i = intersectedEdge+1;
                param = lastParam;
                intersectPt = lastPt;
                }
            if (!intersectFound)
                {
                for (size_t j = 0; j < 3; ++j)
                    {
                    if (bsiDPoint3d_pointEqualTolerance(&currentVertex, &pts[j], 1.0e-8))
                        {
                        intersectFound = true;
                        //compare the two edges to pick the one that is in direction of ray
                        DPoint3d pt,pt2;
                        double paramA, paramB;
                        toNextPoint.ProjectPointUnbounded(pt, paramA, pts[(j + 1) % 3]);
                        toNextPoint.ProjectPointUnbounded(pt2, paramB, pts[(j + 2) % 3]);
                        intersectPt = currentVertex;
                        param = 0;
                        if (lastVtx1 == triangle[(i + 2) % 3] || lastVtx2 == triangle[(i + 2) % 3] || (lastVtx1 != triangle[(i + 1) % 3] && lastVtx2 != triangle[(i + 1) % 3] && paramA > paramB))
                            {
                            i = j + 1;
                            if (bsiDPoint3d_pointEqualTolerance(&pt, &pts[(j + 1) % 3], 1.0e-8)) //second point also on ray
                                if (paramA > 0) intersectPt = pt;
                            }
                        else
                            {
                            i = ((j + 2) % 3) + 1;
                            if (bsiDPoint3d_pointEqualTolerance(&pt2, &pts[(j + 2) % 3], 1.0e-8)) //second point also on ray
                                if (paramB > 0) intersectPt = pt2;
                            }
                        break;
                        }
                    }
                if (!intersectFound)
                for (i=0; i < 3 && !intersectFound; i++)
                    {
                    DSegment3d triEdge = DSegment3d::From(pts[i], pts[(i + 1) % 3]);
                    DRay3d edgeRay = DRay3d::From(triEdge);
                    double param2;
                    DPoint3d intersectPt2;
                    if (bsiDRay3d_closestApproach(&param, &param2, &intersectPt2, &intersectPt, &toNextPoint, &edgeRay))
                        {
                        if ((lastVtx1 == triangle[i] || lastVtx1 == triangle[(i + 1) % 3]) && (lastVtx2 == triangle[i] || lastVtx2 == triangle[(i + 1) % 3])) continue;
                        if (param2 < 0 || param2 > 1 || param < -1.0e-8) continue;
                        intersectFound = true;
                        }
                    }
                }
            backwards = param <0;
            //assert(intersectFound);
            DRay3d toEdgeRay;
            if (!intersectFound)
                {
                //DSegment3d toTriEdge = DSegment3d::From(currentVertex, intersectPt);
                //toEdgeRay = DRay3d::From(toTriEdge);
                toEdgeRay.origin = currentVertex;//intersectPt;
                toEdgeRay.direction = currentLineSegmentDirection;
                //currentVertex = intersectPt;
               // projectedPoints[*segment].push_back(currentVertex);
                lastVtx1 = -1;
                lastVtx2 = -1;
                int inter = 0;
                if (!FindNextTriangleOnRay(currentTriangle, intersectPt, segment, graphP, toEdgeRay, linePoints, points, projectedPoints, &inter, nPoints, is3d))
                    {
                    stopProgression = true;
                    continue;
                    }
                else
                    {
                    if (!graphP->TryGetLabel(currentTriangle, 0, triangle[0])) assert(false && "WRONG CURRENTTRIANGLE");
                    if (!graphP->TryGetLabel(graphP->FSucc(currentTriangle), 0, triangle[1])) assert(false && "WRONG NEXT EDGE");
                    if (!graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(currentTriangle)), 0, triangle[2])) assert(false && "WRONG 2nd EDGE");
                currentVertex = intersectPt;
                projectedPoints[*segment].push_back(currentVertex);
                pts[0] = points[triangle[0] - 1];
                pts[1] = points[triangle[1] - 1];
                pts[2] = points[triangle[2] - 1];
                    continue;
                    }
                }
            if (fabs(param) > 0.000001)
                {
                DSegment3d toTriEdge = DSegment3d::From(currentVertex, intersectPt);
                toEdgeRay = DRay3d::From(toTriEdge);
                double paramray;
                DPoint3d projectedEnd, projectedEnd2;
                //toTriEdge.PointToFractionParameter(param, linePoints[*segment + 1]);
                if (bsiDRay3d_closestApproach(&param, &paramray, &projectedEnd, &projectedEnd2, &toEdgeRay, &ray) && param >= 1.0e-8 && param <= 1+1.0e-8 /*&& paramray > 0*/) //segment ends before triangle edge is reached
                    {
                    //currentVertex.Interpolate(currentVertex, param, intersectPt);
                    currentVertex = projectedEnd;
                    //currentVertex.z = intersectPt.z;
                    projectedPoints[*segment].push_back(currentVertex);
                    ++(*segment);
                    if (*segment == nLinePts - 1)
                        {
                        stopProgression = true; //reached the end of the line
                        }
                    else
                        {
                        lastVtx1 = -1;
                        lastVtx2 = -1;
                        currentLineSegmentDirection = DVec3d::FromStartEnd(linePoints[*segment], linePoints[*segment + 1]);
                        currentLineSegmentDirection.Normalize();
                        ray = DRay3d::FromOriginAndVector(linePoints[*segment + 1], DVec3d::From(0, 0, -1));
                        }
                    continue;
                    }
                else if (lastParam != param && lastParam != -1 && *segment < nLinePts - 1) projectedPoints[*segment].push_back(lastPt);
                }
            else
                {
                toEdgeRay = DRay3d::FromOriginAndVector(currentVertex, projDirection);
                double paramray;
                DPoint3d projectedEnd;
                if (ray.ProjectPointUnbounded(projectedEnd, paramray, currentVertex) && bsiDPoint3d_pointEqualTolerance(&projectedEnd, &currentVertex, 1.0e-7))
                    {
                    projectedPoints[*segment].push_back(currentVertex);
                    ++(*segment);
                    if (*segment == nLinePts - 1)
                        {
                        stopProgression = true; //reached the end of the line
                        }
                    else
                        {
                        lastVtx1 = -1;
                        lastVtx2 = -1;
                        currentLineSegmentDirection = DVec3d::FromStartEnd(linePoints[*segment], linePoints[*segment + 1]);
                        currentLineSegmentDirection.Normalize();
                        ray = DRay3d::FromOriginAndVector(linePoints[*segment + 1], DVec3d::From(0, 0, -1));
                        }
                    continue;
                    }
                }
                //next triangle
            lastVtx1 = triangle[(i - 1) % 3];
            lastVtx2 = triangle[i % 3];
#ifndef NDEBUG
            std::string triDebug = "";
            triDebug += "CURRENT TRIANGLE EDGE: " + std::to_string(currentTriangle)+"\r\n";
            triDebug += ""+ std::to_string(currentTriangle) + "|" + std::to_string(graphP->EdgeMate(currentTriangle)) + "->" + std::to_string(graphP->FSucc(currentTriangle))
                + "|" + std::to_string(graphP->EdgeMate(graphP->FSucc(currentTriangle))) + "->" + std::to_string(graphP->FSucc(graphP->FSucc(currentTriangle))) + "|"
                + std::to_string(graphP->EdgeMate(graphP->FSucc(graphP->FSucc(currentTriangle)))) + "\r\n";
            int tmp[3] = { -1, -1, -1 };
            graphP->TryGetLabel(graphP->EdgeMate(currentTriangle), 0, tmp[0]);
            graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(currentTriangle)), 0, tmp[1]);
            graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(graphP->EdgeMate(currentTriangle))), 0, tmp[2]);
            triDebug += "TRILABELS:" + std::to_string(triangle[0]) + "->" + std::to_string(triangle[1]) + "->" + std::to_string(triangle[2]) + "|" +
                std::to_string(tmp[0]) + "->" + std::to_string(tmp[1]) + "->" + std::to_string(tmp[2]) + "\r\n";
            graphP->TryGetLabel(graphP->EdgeMate(graphP->FSucc(currentTriangle)), 0, tmp[0]);
            graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(graphP->FSucc(currentTriangle))), 0, tmp[1]);
            graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(graphP->EdgeMate(graphP->FSucc(currentTriangle)))), 0, tmp[2]);
            triDebug += "TRILABELS:" + std::to_string(triangle[1]) + "->" + std::to_string(triangle[2]) + "->" + std::to_string(triangle[0]) + "|" +
                std::to_string(tmp[0]) + "->" + std::to_string(tmp[1]) + "->" + std::to_string(tmp[2]) + "\r\n";
            graphP->TryGetLabel(graphP->EdgeMate(graphP->FSucc(graphP->FSucc(currentTriangle))), 0, tmp[0]);
            graphP->TryGetLabel(graphP->FSucc(graphP->EdgeMate(graphP->FSucc(graphP->FSucc(currentTriangle)))), 0, tmp[1]);
            graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(graphP->EdgeMate(graphP->FSucc(graphP->FSucc(currentTriangle))))), 0, tmp[2]);
            triDebug += "TRILABELS:" + std::to_string(triangle[2]) + "->" + std::to_string(triangle[0]) + "->" + std::to_string(triangle[1]) + "|" +
                std::to_string(tmp[0]) + "->" + std::to_string(tmp[1]) + "->" + std::to_string(tmp[2]) + "\r\n";
#endif
                if (i == 1) currentTriangle = graphP->EdgeMate(currentTriangle);
                else if (i == 2) currentTriangle = graphP->EdgeMate(graphP->FSucc(currentTriangle));
                else currentTriangle = graphP->EdgeMate(graphP->FSucc(graphP->FSucc(currentTriangle)));
                if (!graphP->TryGetLabel(currentTriangle, 0, triangle[0])) assert(false && "WRONG CURRENTTRIANGLE");
                if (!graphP->TryGetLabel(graphP->FSucc(currentTriangle), 0, triangle[1])) assert(false && "WRONG NEXT EDGE");
                if (!graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(currentTriangle)), 0, triangle[2])) assert(false && "WRONG 2nd EDGE");
                if (graphP->GetMaskAt(currentTriangle, MTG_EXTERIOR_MASK) || FastCountNodesAroundFace(graphP, currentTriangle) != 3 || //reached mesh exterior -- is it a hole?
                    ((lastVtx1 != triangle[0] && lastVtx1 != triangle[1] && lastVtx1 != triangle[2]) && (lastVtx2 != triangle[0] && lastVtx2 != triangle[1] && lastVtx2 != triangle[2]))) //this should not happen, temp fix
                    {
                    if (is3d) //we need to at least try look in the tiles above and below
                        {
                        triangleStartEdge = currentTriangle;
                        currentVertex = intersectPt;
                        projectedPoints[*segment].push_back(intersectPt);
                        stopProgression = true;
                        break;
                        }
                    toEdgeRay.origin = /*backwards ? */intersectPt;// : currentVertex;
                    toEdgeRay.direction = currentLineSegmentDirection;
                    currentVertex = intersectPt;
                    projectedPoints[*segment].push_back(currentVertex);
                    lastVtx1 = -1;
                    lastVtx2 = -1;
                    int lastSegment = *segment;
                    int inter = 0;
                    if (!FindNextTriangleOnRay(currentTriangle, intersectPt, segment, graphP, toEdgeRay, linePoints, points, projectedPoints, &inter, nPoints,is3d))
                        {
                        stopProgression = true;
                        }
                    else if (*segment != lastSegment && *segment < nLinePts-1)
                        {
                        currentLineSegmentDirection = DVec3d::FromStartEnd(linePoints[*segment], linePoints[*segment + 1]);
                        currentLineSegmentDirection.Normalize();
                        ray = DRay3d::FromOriginAndVector(linePoints[*segment + 1], DVec3d::From(0, 0, -1));
                        toEdgeRay.origin = intersectPt;
                        toEdgeRay.direction = currentLineSegmentDirection;
                        currentTriangle = -1;
                        if (!FindNextTriangleOnRay(currentTriangle, intersectPt, segment, graphP, toEdgeRay, linePoints, points, projectedPoints,&inter, nPoints,is3d))
                            {
                            stopProgression = true;
                            }
                        }
                    if (!stopProgression && inter != 1)
                        {
                        graphP->TryGetLabel(currentTriangle, 0, lastVtx1);
                        graphP->TryGetLabel(graphP->FSucc(currentTriangle), 0, lastVtx2);
                        }
                    if (*segment == nLinePts - 1)
                        {
                        //reached the end of the line
                        currentVertex = linePoints[nLinePts - 1];
                        break;
                        }
                    if (!graphP->TryGetLabel(currentTriangle, 0, triangle[0])) assert(false && "WRONG CURRENTTRIANGLE");
                    if (!graphP->TryGetLabel(graphP->FSucc(currentTriangle), 0, triangle[1])) assert(false && "WRONG NEXT EDGE");
                    if (!graphP->TryGetLabel(graphP->FSucc(graphP->FSucc(currentTriangle)), 0, triangle[2])) assert(false && "WRONG 2nd EDGE");
                    }
                currentVertex = intersectPt;
                projectedPoints[*segment].push_back(currentVertex);
                pts[0] = points[triangle[0]-1];
                pts[1] = points[triangle[1]-1];
                pts[2] = points[triangle[2]-1];

            }
        endPt = currentVertex;
        return true;
        }

    void ExtractMeshIndicesFromGraph(std::vector<int32_t>& indices, MTGGraph* graphP)
        {
        MTGMask visitedMask = graphP->GrabMask();
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            if (!graphP->GetMaskAt(edgeID, MTG_EXTERIOR_MASK) && !graphP->GetMaskAt(edgeID, visitedMask))
                {
                std::vector<MTGNodeId> faceNodes;
                if (FastCountNodesAroundFace(graphP,edgeID) != 3) continue;
                MTGARRAY_FACE_LOOP(faceID, graphP, edgeID)
                    {
                    int vIndex = -1;
                    graphP->TryGetLabel(faceID, 0, vIndex);
                    assert(vIndex > 0);
                    indices.push_back(vIndex);
                    graphP->SetMaskAt(faceID, visitedMask);
                    }
                MTGARRAY_END_FACE_LOOP(faceID, graphP, edgeID)
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            graphP->ClearMask(visitedMask);
        graphP->DropMask(visitedMask);
        }

    size_t CountExteriorFaces(MTGGraph* graphP)
        {
        MTGMask visitedMask = graphP->GrabMask();
        size_t nFaces = 0;
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            if (!graphP->GetMaskAt(edgeID, visitedMask))
                {
                if (graphP->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
                    {
                    nFaces++;
                    MTGARRAY_FACE_LOOP(faceID, graphP, edgeID)
                        {
                        graphP->SetMaskAt(faceID, visitedMask);
                        }
                    MTGARRAY_END_FACE_LOOP(faceID, graphP, edgeID)
                    }
                else graphP->SetMaskAt(edgeID, visitedMask);
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            graphP->ClearMask(visitedMask);
        graphP->DropMask(visitedMask);
        return nFaces;
        }

    void PrintGraph(Utf8String path, Utf8String name, MTGGraph* graphP)
        {
        Utf8String fileName = path + name + "_graph.log";
        std::ofstream f;
        f.open(fileName.c_str(), std::ios_base::trunc);
        f << " GRAPH HAS " + std::to_string(graphP->GetActiveNodeCount()) + " NODES" << std::endl;
        f << " WALKING GRAPH " << std::endl;
        MTGMask visitedMask = graphP->GrabMask();
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            if (!graphP->GetMaskAt(edgeID, visitedMask))
                {
                f << " NEW FACE " << std::endl;
                size_t nNodes = 0;
                MTGARRAY_FACE_LOOP(faceID, graphP, edgeID)
                    {
                    nNodes++;
                    int vIndex = -1, vIndex2 = -1;
                    graphP->TryGetLabel(faceID, 0, vIndex);
                    graphP->TryGetLabel(graphP->EdgeMate(faceID), 0, vIndex2);
                    f << " EDGE " + std::to_string(faceID) + " MATE " + std::to_string(graphP->EdgeMate(faceID)) + " SOURCE " + std::to_string(vIndex) + " TARGET " + std::to_string(vIndex2);
                    int featureTag = -1;
                    graphP->TryGetLabel(faceID, 2, featureTag);
                    if (featureTag != -1) f << " FEATURE! ";
                    f<< std::endl;
                    graphP->SetMaskAt(faceID, visitedMask);
                    }
                MTGARRAY_END_FACE_LOOP(faceID, graphP, edgeID)
                    f << " END FACE  SIZE WAS " + std::to_string(nNodes);
                if (nNodes != 3) f << " NOT A TRIANGLE ";
                    f<< std::endl << std::endl;
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            graphP->ClearMask(visitedMask);
        graphP->DropMask(visitedMask);
        f.close();
        }

    void PrintGraphWithPointInfo(Utf8String path, Utf8String name, MTGGraph* graphP, const DPoint3d* pts, const size_t npts)
        {
        Utf8String fileName = path + name + "_graph.log";
        std::ofstream f;
        f.open(fileName.c_str(), std::ios_base::trunc);
        f << " POINT DATA IS " << std::endl;
        for (size_t i = 0; i < npts; ++i)
            {
            f << " POINT AT " + std::to_string(i + 1) + ": (";
            f << std::to_string(pts[i].x) + ",";
            f << std::to_string(pts[i].y) + ",";
            f << std::to_string(pts[i].z) + ")";
            f << std::endl;
            }
        f << " GRAPH HAS " + std::to_string(graphP->GetActiveNodeCount()) + " NODES" << std::endl;
        f << " WALKING GRAPH " << std::endl;
        MTGMask visitedMask = graphP->GrabMask();
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            if (!graphP->GetMaskAt(edgeID, visitedMask))
                {
                f << " NEW FACE " << std::endl;
                size_t nNodes = 0;
                MTGARRAY_FACE_LOOP(faceID, graphP, edgeID)
                    {
                    nNodes++;
                    int vIndex = -1, vIndex2 = -1;
                    graphP->TryGetLabel(faceID, 0, vIndex);
                    graphP->TryGetLabel(graphP->EdgeMate(faceID), 0, vIndex2);
                    f << " EDGE " + std::to_string(faceID) + " MATE " + std::to_string(graphP->EdgeMate(faceID)) + " SOURCE " + std::to_string(vIndex) + " TARGET " + std::to_string(vIndex2) << std::endl;
                    graphP->SetMaskAt(faceID, visitedMask);
                    }
                MTGARRAY_END_FACE_LOOP(faceID, graphP, edgeID)
                    f << " END FACE  SIZE WAS " + std::to_string(nNodes);
                if (nNodes != 3) f << " NOT A TRIANGLE ";
                f << std::endl << std::endl;
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            graphP->ClearMask(visitedMask);
        graphP->DropMask(visitedMask);
        f.close();
        }

    void UntieLoopsFromPolygon(bvector<DPoint3d>& polygon)
        {
        std::map<DPoint3d, int, DPoint3dZYXTolerancedSortComparison> pts(DPoint3dZYXTolerancedSortComparison(1e-6, 0));
        bvector<DPoint3d> bound;
        bvector<bvector<DPoint3d>> boundLoops;
        for (auto v : polygon)
            {
            if (pts.count(v) != 0 && pts[v] < bound.size()) //detected a loop
                {
                bvector<DPoint3d> innerLoop;
                innerLoop.insert(innerLoop.end(), bound.begin() + pts[v], bound.end());
                innerLoop.push_back(innerLoop.front()); //close loop
                boundLoops.push_back(innerLoop);
                int idx = pts[v];
                for (auto it = pts.begin(); it != pts.end(); ++it)
                    if (it->second > idx && it->second < bound.size())
                        {
                        pts.erase(it);
                        it = pts.begin();
                        }
                bound.resize(idx + 1); //we keep the looping point
                }
            else
                {
                pts[v] = (int)bound.size();
                bound.push_back(v);
                }
            }
        if (DVec3d::FromStartEnd(bound.back(),bound.front()).Magnitude() > 10e-3) bound.clear(); //not closed
        for (auto& b : boundLoops) if (b.size() > bound.size()) bound = b;
        polygon = bound;
        }

    bool RemoveKnotsFromGraph(MTGGraph* graphP, std::vector<DPoint3d>& ptsToModify)
        {
        bool anyKnotsRemoved = false;
        MTGMask visitedMask = graphP->GrabMask();
        std::map<int, int> pts;
       // std::string logstr = std::to_string(ptsToModify.size()) + "\n";
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            pts.clear();
            if (FastCountNodesAroundFace(graphP, edgeID) > 3 && !graphP->GetMaskAt(edgeID, visitedMask))
                {
                size_t n = 0;
                MTGARRAY_FACE_LOOP(extID, graphP, edgeID)
                    {
                    int v = -1;
                    graphP->SetMaskAt(extID, visitedMask);
                    graphP->TryGetLabel(extID, 0, v);
                    if (pts.count(v) != 0 && pts[v] < n) //detected a loop
                        {
                        anyKnotsRemoved = true;
                        //logstr += " POINT " + std::to_string(v) + " FOUND IN FACE AT POSITIONS " + std::to_string(pts[v]) + " AND " + std::to_string(n) + "\n";
                        //we split the vertex causing the pinch
                        DPoint3d ptVal = ptsToModify[v - 1];
                        ptsToModify.push_back(ptVal);
                        //MTGNodeId newID = graphP->EdgeMate(graphP->FPred(extID));
                        MTGNodeId initID = extID;//graphP->GetMaskAt(graphP->EdgeMate(graphP->VSucc(extID)), MTG_EXTERIOR_MASK) && graphP->VSucc(extID) != newID ? newID : extID;
                        MTGNodeId endID = graphP->VPred(extID);//initID == newID ? extID : newID;
                       // logstr += " SETTING ID FOR EDGE " + std::to_string(initID) + " TO " + std::to_string(ptsToModify.size()) + "\nEDGES: ";
                        for (auto it = initID; it != endID; it = graphP->VSucc(it))
                            {
                            graphP->TrySetLabel(it, 0, (int)ptsToModify.size());
                           // logstr += std::to_string(it) + " ";
                            }
                        graphP->TrySetLabel(endID, 0, (int)ptsToModify.size());
                      //  logstr += "\n";
                        /*int idx = pts[v];
                        for (auto it = pts.begin(); it != pts.end(); ++it)
                            if (it->second > idx && it->second < n)
                                {
                                pts.erase(it);
                                it = pts.begin();
                                }*/
                        pts.erase(v);
                        n++;
                        //n = idx + 1;
                        }
                    else
                        {
                        pts[v] = (int)n;
                        n++;
                        }
                    }
                MTGARRAY_END_FACE_LOOP(extID, graphP, edgeID)
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            graphP->ClearMask(visitedMask);
        graphP->DropMask(visitedMask);
        if (anyKnotsRemoved)
            {
           /* std::ofstream f;
            f.open("E:\\output\\scmesh\\2016-01-08\\removed.log", std::ios_base::app);
            f << " NEW GRAPH " << std::endl;
            f << logstr;
            f << std::endl;
            f.close();*/
            }
        return anyKnotsRemoved;
        }


void ResolveUnmergedBoundaries(MTGGraph * graphP)
    {
    MTGMask visitedMask = graphP->GrabMask();
    std::map<std::pair<int,int>, MTGNodeId> map;
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        if (graphP->GetMaskAt(edgeID, MTG_EXTERIOR_MASK) && !graphP->GetMaskAt(edgeID, visitedMask))
            {
            MTGARRAY_FACE_LOOP(extID, graphP, edgeID)
                {
                if (graphP->GetMaskAt(extID, visitedMask)) break;
                graphP->SetMaskAt(extID, visitedMask);
                MTGNodeId edgeMate = graphP->EdgeMate(extID);
                int v = -1, v1 = -1;
                graphP->TryGetLabel(edgeMate, 0, v);
                graphP->TryGetLabel(extID, 0, v1);
                if (v == -1 || v1 == -1) break;
                if (map.count(std::make_pair(v,v1)) > 0) 
                    {
                    MTGNodeId newMateId = map[std::make_pair(v, v1)];
                    MTGNodeId newExtID = graphP->FSucc(extID);
                    graphP->VertexTwist(extID, graphP->FSucc(newMateId));
                    graphP->VertexTwist(newMateId, graphP->FSucc(extID));
                    extID = newExtID;
                    graphP->ExciseSliverFace(newMateId);
                    graphP->ClearMaskAroundEdge(edgeMate, MTG_EXTERIOR_MASK);
                    graphP->ClearMaskAroundEdge(edgeMate, MTG_BOUNDARY_MASK);
                    map.erase(std::make_pair(v, v1));
                    continue;
                    }
                else map.insert(std::make_pair(std::make_pair(v1,v), extID));
                }
            MTGARRAY_END_FACE_LOOP(extID, graphP, edgeID)
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
    graphP->ClearMask(visitedMask);
    graphP->DropMask(visitedMask);
    RecomputeExterior(graphP);
    }


void RecomputeExterior(MTGGraph * graphP)
    {
    MTGMask visitedMask = graphP->GrabMask();
    MTGARRAY_SET_LOOP(edgeID, graphP)
        {
        if (graphP->GetMaskAt(edgeID, visitedMask)) continue;
        int n = 0;
        MTGARRAY_FACE_LOOP(faceEdgeID, graphP, edgeID)
            {
            if (n > 2)
                {
                ++n;
                break;
                }
            ++n;
            }
        MTGARRAY_END_FACE_LOOP(faceEdgeID, graphP, edgeID)
            if (n > 3)
                {
                MTGARRAY_FACE_LOOP(faceEdgeID, graphP, edgeID)
                    {
                    graphP->SetMaskAt(faceEdgeID, MTG_EXTERIOR_MASK);
                    graphP->SetMaskAt(faceEdgeID, visitedMask);
                    if (graphP->GetMaskAt(graphP->EdgeMate(faceEdgeID), MTG_EXTERIOR_MASK))
                        {

                        }
                    graphP->SetMaskAt(graphP->EdgeMate(faceEdgeID), MTG_BOUNDARY_MASK);
                    graphP->SetMaskAt(graphP->EdgeMate(faceEdgeID), visitedMask);
                    }
                MTGARRAY_END_FACE_LOOP(faceEdgeID, graphP, edgeID)
                }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graphP)
        graphP->ClearMask(visitedMask);
    graphP->DropMask(visitedMask);
    }

bool HasOverlapWithNeighborsXY(MTGGraph* graphP, MTGNodeId boundaryId, const DPoint3d* pts)
    {
    size_t edgeAroundFace = 0;
    DPoint3d facePoints[3];
    MTGNodeId nodes[3];
    MTGARRAY_FACE_LOOP(aroundFaceIndex, graphP, boundaryId)
        {
        if (edgeAroundFace < 3)
            {
            int vIndex = -1;
            graphP->TryGetLabel(aroundFaceIndex, 0, vIndex);
            assert(vIndex > 0);
            if (vIndex <= 0)break;
            facePoints[edgeAroundFace] = pts[vIndex - 1];
            nodes[edgeAroundFace] = aroundFaceIndex;
            }
        edgeAroundFace++;
        if (edgeAroundFace > 3) break;
        }
    MTGARRAY_END_FACE_LOOP(aroundFaceIndex, graphP, boundaryId)
        DSegment3d triangle[3] = { DSegment3d::From(facePoints[0], facePoints[1]),
        DSegment3d::From(facePoints[1], facePoints[2]),
        DSegment3d::From(facePoints[2], facePoints[0]) };
    std::vector<DSegment3d> neighbors[3];
    neighbors[0].reserve(20);
    neighbors[1].reserve(20);
    neighbors[2].reserve(20);
    for (size_t i = 0; i < 3; ++i)
        {
        if (FastCountNodesAroundFace(graphP, graphP->EdgeMate(nodes[i])) > 3) continue;
        MTGARRAY_FACE_LOOP(aroundFaceIndex, graphP, graphP->EdgeMate(nodes[i]))
            {
            if (aroundFaceIndex != graphP->EdgeMate(nodes[i]))
                {
                int vIndex1, vIndex2;
                graphP->TryGetLabel(aroundFaceIndex, 0, vIndex1);
                graphP->TryGetLabel(graphP->EdgeMate(aroundFaceIndex), 0, vIndex2);
                neighbors[i].push_back(DSegment3d::From(pts[vIndex1 - 1], pts[vIndex2 - 1]));
                }
            }
        MTGARRAY_END_FACE_LOOP(aroundFaceIndex, graphP, graphP->EdgeMate(nodes[i]))
        }
    for (size_t i = 0; i < 3; ++i)
        {
        for (auto& seg : neighbors[i])
            {
            double fracA, fracB;
            DPoint3d ptA, ptB;
            if (DSegment3d::IntersectXY(fracA, fracB, ptA, ptB, seg, triangle[(i + 1) % 3]) && fracA > 1e-8 && fracB > 1e-8 && fracA < (1 - 1e-8) && fracB < (1 - 1e-8)) return true;
            if (DSegment3d::IntersectXY(fracA, fracB, ptA, ptB, seg, triangle[(i + 2) % 3]) && fracA > 1e-8 && fracB > 1e-8 && fracA < (1 - 1e-8) && fracB < (1 - 1e-8)) return true;
            }
        }
    return false;
    }

void LookForFirstTaggedEdge(MTGGraph* graphP, MTGNodeId& start, int& currentLastId, size_t featureSize, const int32_t* featureData)
    {
    int vertex1 = featureData[currentLastId - 1] == INT_MAX ? INT_MAX : featureData[currentLastId - 1] + 1;
    int vertex2 = featureData[currentLastId] == INT_MAX ? INT_MAX : featureData[currentLastId] + 1;
    while (start == MTG_NULL_NODEID && currentLastId < featureSize)
        {
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            int v = -1;
            graphP->TryGetLabel(edgeID, 0, v);
            if ((v == vertex1 || v == vertex2))
                {
                MTGARRAY_VERTEX_LOOP(edgeId2, graphP, edgeID)
                    {
                    int v1 = -1;
                    graphP->TryGetLabel(graphP->FSucc(edgeId2), 0, v1);
                    if (v1 == vertex1 || v1 == vertex2)
                        {
                        start = edgeId2;
                        if (v1 == vertex1) start = graphP->EdgeMate(start);
                        break;
                        }
                    }
                MTGARRAY_END_VERTEX_LOOP(edgeId2, graphP, edgeID)
                    if (start != MTG_NULL_NODEID) break;
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            if (start == MTG_NULL_NODEID)
                {
                do
                    {
                    if (currentLastId + 1 < featureSize)
                        {
                        vertex1 = featureData[currentLastId] == INT_MAX ? INT_MAX : featureData[currentLastId] + 1;
                        vertex2 = featureData[currentLastId + 1] == INT_MAX ? INT_MAX : featureData[currentLastId + 1] + 1;
                        }
                    ++currentLastId;
                    }
                while (currentLastId < featureSize && (vertex1 == INT_MAX || vertex2 == INT_MAX));
                }
        }
    }

void TagFeatureEdges(MTGGraph* graphP, DTMFeatureType tagValue, size_t featureSize, const int32_t* featureData, bool tagEnds)
    {
    if (featureSize < 2) return;

    int vertex1 = featureData[0] == INT_MAX? INT_MAX : featureData[0] + 1;
    int vertex2 = featureData[1] == INT_MAX ? INT_MAX : featureData[1] + 1;
    int currentLastId = 1;
    while ((vertex1 == INT_MAX || vertex2 == INT_MAX) && currentLastId + 1 < featureSize)
        {
        vertex1 = featureData[currentLastId] == INT_MAX? INT_MAX: featureData[currentLastId] + 1;
        vertex2 = featureData[currentLastId+1] == INT_MAX ? INT_MAX : featureData[currentLastId + 1] + 1;
        currentLastId++;
        }
    if (vertex1 == INT_MAX || vertex2 == INT_MAX) return;

    MTGNodeId start = MTG_NULL_NODEID; 
    LookForFirstTaggedEdge(graphP, start, currentLastId, featureSize, featureData);
  /*  while (start == MTG_NULL_NODEID && currentLastId < featureSize)
        {
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            int v = -1;
            graphP->TryGetLabel(edgeId, 0, v);
            if ((v == vertex1 || v == vertex2))
                {
                MTGARRAY_VERTEX_LOOP(edgeId2, graphP, edgeID)
                    {
                    int v1 = -1;
                    graphP->TryGetLabel(graphP->FSucc(edgeId2), 0, v1);
                    if (v1 == vertex1 || v1 == vertex2)
                        {
                        start = edgeId2;
                        if (v1 == vertex1) start = graphP->EdgeMate(start);
                        break;
                        }
                    }
                MTGARRAY_END_VERTEX_LOOP(edgeId2, graphP, edgeID)
                    if (start != MTG_NULL_NODEID) break;
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)
            if (start == MTG_NULL_NODEID)
                {
                do
                    {
                    if (currentLastId + 1 < featureSize)
                        {
                        vertex1 = featureData[currentLastId] == INT_MAX? INT_MAX : featureData[currentLastId] + 1;
                        vertex2 = featureData[currentLastId+1] == INT_MAX ? INT_MAX : featureData[currentLastId + 1] + 1;
                        }
                    ++currentLastId;
                    }
                while (currentLastId < featureSize && (vertex1 == INT_MAX || vertex2 == INT_MAX));
                }
        }*/

        int featureIdLabel = -1;
    if (!graphP->TrySearchLabelTag(2, featureIdLabel))featureIdLabel = graphP->DefineLabel(2, MTG_LabelMask_EdgeProperty, -1);
    int nextInFeatureLabel = -1;
    if (!graphP->TrySearchLabelTag(3, nextInFeatureLabel))nextInFeatureLabel = graphP->DefineLabel(3, MTG_LabelMask_EdgeProperty, -1);
    graphP->TrySetLabel(start, featureIdLabel, (int)tagValue);
    graphP->TrySetLabel(graphP->EdgeMate(start), featureIdLabel, (int)tagValue);
    MTGNodeId currentEdge = start;
    int componentId = 0;

    if (tagEnds)
        {
        graphP->TrySetLabel(start, nextInFeatureLabel, (int)componentId);
        graphP->TrySetLabel(graphP->EdgeMate(start), nextInFeatureLabel, (int)componentId);
        }
    for (size_t i = currentLastId; i + 1 < featureSize; ++i)
        {
        currentEdge = graphP->FSucc(currentEdge);
        int v = -1;
        bool found = false;
        graphP->TryGetLabel(graphP->EdgeMate(currentEdge), 0, v);
        if (featureData[i + 1] + 1 == v)
            {
            graphP->TrySetLabel(currentEdge, featureIdLabel, (int)tagValue);
            graphP->TrySetLabel(graphP->EdgeMate(currentEdge), featureIdLabel, (int)tagValue);
            found = true;
            }
        else
            {
            MTGARRAY_VERTEX_LOOP(featureEdge, graphP, currentEdge)
                {
                graphP->TryGetLabel(graphP->EdgeMate(featureEdge), 0, v);
                if (featureData[i + 1] + 1 == v)
                    {
                    graphP->TrySetLabel(featureEdge, featureIdLabel, (int)tagValue);
                    graphP->TrySetLabel(graphP->EdgeMate(featureEdge), featureIdLabel, (int)tagValue);
                    currentEdge = featureEdge;
                    found = true;
                    break;
                    }
                }
            MTGARRAY_END_VERTEX_LOOP(featureEdge, graphP, currentEdge)

                if (!found)
                    {
                    MTGNodeId lastEdge = currentEdge;
                    currentLastId = (int)i+1;//can't find next vertex, will try the one after it
                    while (currentLastId+1 < featureSize && lastEdge == currentEdge)
                        {
                        currentLastId++;
                        LookForFirstTaggedEdge(graphP, currentEdge, currentLastId, featureSize, featureData);
                        }
                    if (currentEdge == lastEdge) break;
                    i = currentLastId - 1;

                    componentId++;
                    if (tagEnds)
                        {
                        graphP->TrySetLabel(lastEdge, nextInFeatureLabel, (int)componentId);
                        graphP->TrySetLabel(graphP->EdgeMate(lastEdge), nextInFeatureLabel, (int)componentId);
                        graphP->TrySetLabel(currentEdge, nextInFeatureLabel, (int)componentId);
                        graphP->TrySetLabel(graphP->EdgeMate(currentEdge), nextInFeatureLabel, (int)componentId);
                        }
                    }
            }
        }

    }

END_BENTLEY_SCALABLEMESH_NAMESPACE