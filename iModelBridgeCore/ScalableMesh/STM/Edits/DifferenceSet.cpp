#include "ScalableMeshPCH.h" 
#include "../ImagePPHeaders.h"
#include "DifferenceSet.h"
#include <Vu/VuApi.h>
#include <TerrainModel\TerrainModel.h>
#include <TerrainModel/Core/DTMIterators.h>
#include <array>
USING_NAMESPACE_BENTLEY_TERRAINMODEL
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


size_t DifferenceSet::WriteToBinaryStream(void*& serialized)
    {
    size_t ct = sizeof(int32_t) + 7 * sizeof(uint64_t) + addedVertices.size()*sizeof(DPoint3d) + addedFaces.size()*sizeof(int32_t) + removedVertices.size()*sizeof(int32_t) + removedFaces.size() * sizeof(int32_t) + addedUvs.size()*sizeof(DPoint2d) + addedUvIndices.size()*sizeof(int32_t)+ sizeof(bool);
    serialized = malloc(ct);
    size_t offset = 0;
    memcpy((uint8_t*)serialized + offset, &clientID, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy((uint8_t*)serialized+offset, &firstIndex, sizeof(int32_t));
    offset += sizeof(int32_t);
    uint64_t arraySize = addedVertices.size();
    memcpy((uint8_t*)serialized + offset, &arraySize, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    if (addedVertices.size() > 0) memcpy((uint8_t*)serialized + offset, &addedVertices[0], addedVertices.size()*sizeof(DPoint3d));
    offset += addedVertices.size()*sizeof(DPoint3d);
    arraySize = addedFaces.size();
    memcpy((uint8_t*)serialized + offset, &arraySize, sizeof(uint64_t));
    offset += sizeof(uint64_t);
   if(addedFaces.size() > 0) memcpy((uint8_t*)serialized + offset, &addedFaces[0], addedFaces.size()*sizeof(int32_t));
    offset += addedFaces.size()*sizeof(int32_t);
    arraySize = removedVertices.size();
    memcpy((uint8_t*)serialized + offset, &arraySize, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    if (removedVertices.size() > 0)memcpy((uint8_t*)serialized + offset, &removedVertices[0], removedVertices.size()*sizeof(int32_t));
    offset += removedVertices.size()*sizeof(int32_t);
    arraySize = removedFaces.size();
    memcpy((uint8_t*)serialized + offset, &arraySize, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    if (removedFaces.size() > 0) memcpy((uint8_t*)serialized + offset, &removedFaces[0], removedFaces.size()*sizeof(int32_t));
    offset += removedFaces.size()*sizeof(int32_t);
    arraySize = addedUvs.size();
    memcpy((uint8_t*)serialized + offset, &arraySize, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    if (addedUvs.size() > 0) memcpy((uint8_t*)serialized + offset, &addedUvs[0], addedUvs.size()*sizeof(DPoint2d));
    offset += addedUvs.size()*sizeof(DPoint2d);
    arraySize = addedUvIndices.size();
    memcpy((uint8_t*)serialized + offset, &arraySize, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    if (addedUvIndices.size() > 0) memcpy((uint8_t*)serialized + offset, &addedUvIndices[0], addedUvIndices.size()*sizeof(int32_t));
    offset += addedUvIndices.size()*sizeof(int32_t);
    memcpy((uint8_t*)serialized + offset, &toggledForID, sizeof(bool));
    offset += sizeof(bool);
    return ct;
    }

void DifferenceSet::LoadFromBinaryStream(void* serialized, size_t ct)
    {
    addedVertices = bvector<DPoint3d>();
    addedFaces = bvector<int32_t>();
    removedVertices = bvector<int32_t>();
    removedFaces = bvector<int32_t>();
    addedUvs = bvector<DPoint2d>();
    addedUvIndices = bvector<int32_t>();
    memcpy(&clientID, serialized, sizeof(uint64_t));
    memcpy(&firstIndex, (uint8_t*)serialized+sizeof(uint64_t), sizeof(int32_t));
    size_t size;
    memcpy(&size, (uint8_t*)serialized + sizeof(uint64_t)+sizeof(int32_t), sizeof(uint64_t));
    assert(size <= ct);
    addedVertices.resize(size);
    memcpy(&addedVertices[0], (uint8_t*)serialized + 2*sizeof(uint64_t) + sizeof(int32_t), size*sizeof(DPoint3d));
    size_t offset = 2 * sizeof(uint64_t) + sizeof(int32_t) + size*sizeof(DPoint3d);
    memcpy(&size, (uint8_t*)serialized + offset, sizeof(uint64_t));
    assert(size <= ct);
    addedFaces.resize(size);
    memcpy(&addedFaces[0], (uint8_t*)serialized + offset + sizeof(uint64_t), size*sizeof(int32_t));
    offset += sizeof(uint64_t) + size*sizeof(int32_t);
    memcpy(&size, (uint8_t*)serialized + offset, sizeof(uint64_t));
    assert(size <= ct);
    removedVertices.resize(size);
    memcpy(&removedVertices[0], (uint8_t*)serialized + offset + sizeof(uint64_t), size*sizeof(int32_t));
    offset += sizeof(uint64_t) + size*sizeof(int32_t);
    memcpy(&size, (uint8_t*)serialized + offset, sizeof(uint64_t));
    assert(size <= ct);
    removedFaces.resize(size);
    memcpy(&removedFaces[0], (uint8_t*)serialized + offset + sizeof(uint64_t), size*sizeof(int32_t));
    offset += sizeof(uint64_t) + size*sizeof(int32_t);
    memcpy(&size, (uint8_t*)serialized + offset, sizeof(uint64_t));
    assert(size <= ct);
    addedUvs.resize(size);
    memcpy(&addedUvs[0], (uint8_t*)serialized + offset + sizeof(uint64_t), size*sizeof(DPoint2d));
    offset += sizeof(uint64_t) + size*sizeof(DPoint2d);
    memcpy(&size, (uint8_t*)serialized + offset, sizeof(uint64_t));
    assert(size <= ct);
    addedUvIndices.resize(size);
    memcpy(&addedUvIndices[0], (uint8_t*)serialized + offset + sizeof(uint64_t), size*sizeof(int32_t));
    offset += sizeof(uint64_t) + size*sizeof(int32_t);
    memcpy(&toggledForID, (uint8_t*)serialized + offset, sizeof(bool));
    offset += sizeof(bool);
    }


void DifferenceSet::ApplySet(DifferenceSet& d, int firstIndx)
    {
    assert(addedUvIndices.size() == 0 || addedFaces.size() == addedUvIndices.size());
    size_t firstId = addedVertices.size() + firstIndex;
 //   uint64_t upperId = (d.clientID >> 32);
   // if (upperId == 0) return;
    addedVertices.insert(addedVertices.end(), d.addedVertices.begin(), d.addedVertices.end());
  //  if (upperId == 0) removedVertices.insert(removedVertices.end(), d.removedVertices.begin(), d.removedVertices.end());
    for (int32_t idx : d.removedFaces)
        {
        removedFaces.push_back(idx + firstIndx);
        }
    //vector<bool> targeted(d.firstIndex, false);
    //size_t nOfAddedFaces = addedFaces.size();
    for (int32_t idx : d.addedFaces)
        {
        if (idx < d.firstIndex) addedFaces.push_back(idx + firstIndx);
        else addedFaces.push_back(idx - d.firstIndex + (int32_t)firstId);
        //if (upperId != 0 && idx < d.firstIndex) targeted[idx - 1] = true;
        assert(addedFaces.back() != 0);
        }
    size_t originalNUVs = addedUvs.size();
    addedUvs.insert(addedUvs.end(), d.addedUvs.begin(), d.addedUvs.end());
    size_t originalNUVIndices = addedUvIndices.size();
    addedUvIndices.insert(addedUvIndices.end(), d.addedUvIndices.begin(), d.addedUvIndices.end());
    for (size_t i = originalNUVIndices; i < addedUvIndices.size(); ++i)
        {
        addedUvIndices[i] += (int)originalNUVs;
        assert(addedUvIndices[i] <= addedUvs.size());
        }
    assert(addedUvIndices.size() == 0 || addedFaces.size() == addedUvIndices.size());
 /*   if (upperId != 0)
        {
        for (size_t i = 0; i < d.removedVertices.size(); ++i) targeted[d.removedVertices[i]] = true;
        for (size_t i = 0; i < nOfAddedFaces; i += 3)
            {
            if ((addedFaces[i] < firstIndex && targeted[addedFaces[i] - 1])
                || (addedFaces[i + 1] < firstIndex && targeted[addedFaces[i + 1] - 1])
                || (addedFaces[i + 2] < firstIndex && targeted[addedFaces[i + 2] - 1]))
                {
                std::string s;
                s += std::to_string(i);
                addedFaces.erase(&addedFaces[i], &addedFaces[i + 3]);
                i -= 3;
                nOfAddedFaces -= 3;
                }

            }
        }*/
    }


void DifferenceSet::ApplyMapped(DifferenceSet& d, const int* idxMap)
    {
    size_t firstId = addedVertices.size() + firstIndex;
    addedVertices.insert(addedVertices.end(), d.addedVertices.begin(), d.addedVertices.end());
    removedVertices.insert(removedVertices.end(), d.removedVertices.begin(), d.removedVertices.end());
    for (int32_t idx : d.removedFaces)
        {
        removedFaces.push_back(idxMap[idx-1]);
        }
    for (int32_t idx : d.addedFaces)
        {
        if (idx < d.firstIndex) addedFaces.push_back(idxMap[idx - 1]);
        else addedFaces.push_back(idx - d.firstIndex + (int32_t)firstId);
        assert(addedFaces.back() != 0);
        }
    }

bool DifferenceSet::ConflictsWith(DifferenceSet& d)
    {
    if (d.firstIndex != firstIndex) return false;
    for (auto f : d.addedFaces)
        for (auto val : addedFaces)
            if (f < firstIndex && val < firstIndex && f == val) return true;
    return false;
    }

void AddFacesFromIntersectingSets(DifferenceSet& outSet, vector<vector<int32_t>>& setA, vector<vector<int32_t>>& setB, DifferenceSet& diffA, DifferenceSet& diffB, const DPoint3d* vertices, const int32_t vtxId)
    {
    size_t n = 0;
    for (auto& facet1 : setA)
        {
        if (facet1.size() < 3) continue;
        VuSetP graphP = vu_newVuSet(0);
        std::string s1;
        s1 += " FACET 0 " + std::to_string(facet1[0]) + " FACET 1 " + std::to_string(facet1[1]) + " FACET 2 " + std::to_string(facet1[2]);
        DPoint3d facetV1[3] = { facet1[0] < diffA.firstIndex ? vertices[facet1[0] - 1] : diffA.addedVertices[facet1[0] - diffA.firstIndex],
            facet1[1] < diffA.firstIndex ? vertices[facet1[1] - 1] : diffA.addedVertices[facet1[1] - diffA.firstIndex],
            facet1[2] < diffA.firstIndex ? vertices[facet1[2] - 1] : diffA.addedVertices[facet1[2] - diffA.firstIndex] };
        for (auto& facet : setB)
            {
            if (facet.size() < 3) continue;
            std::string s;
            s += " FACET 0 " + std::to_string(facet[0]) + " FACET 1 " + std::to_string(facet[1]) + " FACET 2 " + std::to_string(facet[2]);
            DPoint3d facetV[3] = { facet[0] < diffB.firstIndex ? vertices[facet[0] - 1] : diffB.addedVertices[facet[0] - diffB.firstIndex],
                facet[1] < diffB.firstIndex ? vertices[facet[1] - 1] : diffB.addedVertices[facet[1] - diffB.firstIndex],
                facet[2] < diffB.firstIndex ? vertices[facet[2] - 1] : diffB.addedVertices[facet[2] - diffB.firstIndex] };
            vu_makeLoopFromArray3d(graphP, facetV, 3, true, true);

            // Resolve loop criss-cross(es) using parity rules
            vu_mergeLoops(graphP);
            vu_regularizeGraph(graphP);
            vu_markAlternatingExteriorBoundaries(graphP, true);
            }
        vu_stackPush(graphP);
        vu_makeLoopFromArray3d(graphP, facetV1, 3, true, true);
        vu_mergeLoops(graphP);
        vu_regularizeGraph(graphP);
        vu_markAlternatingExteriorBoundaries(graphP, true);
        vu_stackPopWithOperation(graphP, vu_andLoops, NULL);
        vu_triangulateMonotoneInteriorFaces(graphP, false);
        VuArrayP faceArrayP = vu_grabArray(graphP);
        vu_collectInteriorFaceLoops(faceArrayP, graphP);
        vu_arrayOpen(faceArrayP);
        VuP         faceP;
        vector<DPoint3d> facePoints;
        for (; vu_arrayRead(faceArrayP, &faceP);)
            {
            facePoints.clear();

            VU_FACE_LOOP(P, faceP)
                {
                DPoint3d xyz;
                vu_getDPoint3d(&xyz, P);
                facePoints.push_back(xyz);
                }
            END_VU_FACE_LOOP(P, faceP)
                if (facePoints.size() != 3) continue;
            ++n;
            for (auto& pt : facePoints)
                {
                if (bsiDPoint3d_pointEqualTolerance(&pt, &vertices[vtxId], 1e-5)) outSet.addedFaces.push_back(vtxId + 1);
                else
                    {
                    outSet.addedFaces.push_back(outSet.firstIndex + (int)outSet.addedVertices.size());
                    outSet.addedVertices.push_back(pt);
                    }
                }
            }
        if (faceArrayP)
            vu_returnArray(graphP, faceArrayP);
        vu_freeVuSet(graphP);
        }
    std::string s;
    s += "FACES ADDED " + std::to_string(n);
    }

DifferenceSet DifferenceSet::MergeSetWith(DifferenceSet& d, const DPoint3d* vertices)
    {
    vector<std::array<vector<vector<int32_t>>, 2>> faces(d.firstIndex);
    DifferenceSet newDiff;
    newDiff.firstIndex = d.firstIndex;
    for (size_t f = 0; f < addedFaces.size(); f += 3)
        {
        for (int i = 0; i < 3;++i)
        if (addedFaces[f+i] < firstIndex)
            {
            faces[addedFaces[f + i] - 1][0].push_back(vector<int32_t>());
            faces[addedFaces[f+i] - 1][0].back().push_back(addedFaces[f]);
            faces[addedFaces[f + i] - 1][0].back().push_back(addedFaces[f + 1]);
            faces[addedFaces[f + i] - 1][0].back().push_back(addedFaces[f + 2]);
            }
        }
    for (size_t f = 0; f < d.addedFaces.size(); f += 3)
        {
        for (int i = 0; i < 3; ++i)
            if (d.addedFaces[f + i]  < firstIndex)
                {
                faces[d.addedFaces[f + i] - 1][1].push_back(vector<int32_t>());
                faces[d.addedFaces[f + i] - 1][1].back().push_back(d.addedFaces[f]);
                faces[d.addedFaces[f + i] - 1][1].back().push_back(d.addedFaces[f + 1]);
                faces[d.addedFaces[f + i] - 1][1].back().push_back(d.addedFaces[f + 2]);
                }
        }
    for (auto& vtx : faces)
        {
        if (vtx[0].size() == 0 || vtx[1].size() == 0) continue;
        AddFacesFromIntersectingSets(newDiff, vtx[1], vtx[0], d, *this, vertices, (int32_t)(&vtx - &faces[0]));
        //AddFacesFromIntersectingSets(newDiff, vtx[0], vtx[1], *this, d, vertices, (int32_t)(&vtx - &faces[0]));
        /*for (auto& facet1 : vtx[1])
            {
            if (facet1.size() < 3) continue;
            VuSetP graphP = vu_newVuSet(0);
            std::string s1;
            s1 += " FACET 0 " + std::to_string(facet1[0]) + " FACET 1 " + std::to_string(facet1[1]) + " FACET 2 " + std::to_string(facet1[2]);
            DPoint3d facetV1[3] = { facet1[0] < firstIndex ? vertices[facet1[0] - 1] : d.addedVertices[facet1[0] - firstIndex],
            facet1[1] < firstIndex ? vertices[facet1[1] - 1] : d.addedVertices[facet1[1] - firstIndex],
            facet1[2] < firstIndex ? vertices[facet1[2] - 1] : d.addedVertices[facet1[2] - firstIndex] };
            for (auto& facet : vtx[0])
            {
            if (facet.size() < 3) continue;
            std::string s;
            s += " FACET 0 " + std::to_string(facet[0]) + " FACET 1 " + std::to_string(facet[1]) + " FACET 2 " + std::to_string(facet[2]);
            DPoint3d facetV[3] = { facet[0] < firstIndex ? vertices[facet[0] - 1] : addedVertices[facet[0] - firstIndex],
            facet[1] < firstIndex ? vertices[facet[1] - 1] : addedVertices[facet[1] - firstIndex],
            facet[2] < firstIndex ? vertices[facet[2] - 1] : addedVertices[facet[2] - firstIndex] };
            vu_makeLoopFromArray3d(graphP, facetV, 3, true, true);

            // Resolve loop criss-cross(es) using parity rules
            vu_mergeLoops(graphP);
            vu_regularizeGraph(graphP);
            vu_markAlternatingExteriorBoundaries(graphP, true);
            }
            vu_stackPush(graphP);
            vu_makeLoopFromArray3d(graphP, facetV1, 3, true, true);
            vu_mergeLoops(graphP);
            vu_regularizeGraph(graphP);
            vu_markAlternatingExteriorBoundaries(graphP, true);
            vu_stackPopWithOperation(graphP, vu_andLoops, NULL);
            vu_triangulateMonotoneInteriorFaces(graphP, false);
            VuArrayP faceArrayP = vu_grabArray(graphP);
            vu_collectInteriorFaceLoops(faceArrayP, graphP);
            vu_arrayOpen(faceArrayP);
            VuP         faceP;
            vector<DPoint3d> facePoints;
            for (; vu_arrayRead(faceArrayP, &faceP);)
            {
            facePoints.clear();

            VU_FACE_LOOP(P, faceP)
            {
            DPoint3d xyz;
            vu_getDPoint3d(&xyz, P);
            facePoints.push_back(xyz);
            }
            END_VU_FACE_LOOP(P, faceP)
            if (facePoints.size() != 3) continue;
            for (auto& pt : facePoints)
            {
            if (bsiDPoint3d_pointEqualTolerance(&pt, &vertices[&vtx - &faces[0]], 1e-5)) newDiff.addedFaces.push_back(&vtx - &faces[0] + 1);
            else
            {
            newDiff.addedFaces.push_back(newDiff.firstIndex + (int)newDiff.addedVertices.size());
            newDiff.addedVertices.push_back(pt);
            }
            }
            }
            if (faceArrayP)
            vu_returnArray(graphP, faceArrayP);
            vu_freeVuSet(graphP);
            }
            }*/
        }
    return newDiff;
    }

DifferenceSet DifferenceSet::MergeSetWith(DifferenceSet& d, const DPoint3d* vertices, bvector<DPoint3d>& clip1, bvector<DPoint3d>& clip2)
    {
    vector<std::array<bool, 2>> faces(d.firstIndex, { { false, false } });
    vector<bool> neighborsOfMarkedVertices(d.firstIndex, false);
    DifferenceSet newDiff;
    newDiff.firstIndex = d.firstIndex;
    bvector<DPoint3d> allPoints;
    map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    for (size_t f = 0; f < addedFaces.size(); f += 3)
        {
        for (int i = 0; i < 3; ++i)
            {
            DPoint3d pt = addedFaces[f + i] < firstIndex ? vertices[addedFaces[f + i] - 1] : addedVertices[addedFaces[f + i] - firstIndex];
            allPoints.push_back(pt);
            if (addedFaces[f + i] < firstIndex)
                {
                faces[addedFaces[f + i] - 1][0] = true;
                mapOfPoints[vertices[addedFaces[f + i] - 1]] = (int)(addedFaces[f + i] - 1);
                }
            }
        }
    for (size_t f = 0; f < d.addedFaces.size(); f += 3)
        {
        for (int i = 0; i < 3; ++i)
            {
            DPoint3d pt = d.addedFaces[f + i] < firstIndex ? vertices[d.addedFaces[f + i] - 1] : d.addedVertices[d.addedFaces[f + i] - firstIndex];
            allPoints.push_back(pt);
            if (d.addedFaces[f + i] < firstIndex)
                {
                faces[d.addedFaces[f + i] - 1][1] = true;
                mapOfPoints[vertices[d.addedFaces[f + i] - 1]] = (int)(d.addedFaces[f + i] - 1);
                }
            }
        }
    for (auto& mask : faces) if (mask[0] && mask[1]) newDiff.removedVertices.push_back(&mask - &faces[0]);
    /*for (size_t f = 0; f < addedFaces.size(); f += 3)
        {
        bool markFace = false;
        for (size_t i = 0; i < 3; ++i)
            if (addedFaces[f + i] < firstIndex && faces[addedFaces[f + i] - 1][0] == true && faces[addedFaces[f + i] - 1][1] == true) markFace = true;
        if (markFace)
            {
            for (size_t i = 0; i < 3; ++i)
                if (addedFaces[f + i] < firstIndex) neighborsOfMarkedVertices[addedFaces[f + i] - 1] = true;
            }
        }
    for (size_t f = 0; f < d.addedFaces.size(); f += 3)
        {
        bool markFace = false;
        for (size_t i = 0; i < 3; ++i)
            if (d.addedFaces[f + i] < firstIndex && faces[d.addedFaces[f + i] - 1][0] == true && faces[d.addedFaces[f + i] - 1][1] == true) markFace = true;
        if (markFace)
            {
            for (size_t i = 0; i < 3; ++i)
                if (d.addedFaces[f + i] < firstIndex) neighborsOfMarkedVertices[d.addedFaces[f + i] - 1] = true;
            }
        }*/
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        BcDTMPtr bcDtmObjPtr;
#ifdef VANCOUVER_API
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(*bcDtmP);
#else
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(bcDtmP);
#endif
        dtmPtr = bcDtmObjPtr.get();
        }
    bcdtmObject_storeDtmFeatureInDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), DTMFeatureType::RandomSpots, dtmPtr->GetBcDTM()->GetTinHandle()->nullUserTag, 1, &dtmPtr->GetBcDTM()->GetTinHandle()->nullFeatureId, &allPoints[0], (long)allPoints.size());
    bcdtmObject_triangulateDtmObject(dtmPtr->GetBcDTM()->GetTinHandle());
    DTMUserTag    userTag = 0;
    DTMFeatureId* textureRegionIdsP = 0;
    long          numRegionTextureIds = 0;
    bcdtmInsert_internalDtmFeatureMrDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(),
                                                       DTMFeatureType::Region,
                                                       1,
                                                       2,
                                                       userTag,
                                                       &textureRegionIdsP,
                                                       &numRegionTextureIds,
                                                       &clip1[0],
                                                       (long)clip1.size());
    userTag = 1;
    bcdtmInsert_internalDtmFeatureMrDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(),
                                              DTMFeatureType::Region,
                                              1,
                                              2,
                                              userTag,
                                              &textureRegionIdsP,
                                              &numRegionTextureIds,
                                              &clip2[0],
                                              (long)clip2.size());
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtmPtr->GetBcDTM());

    en->SetExcludeAllRegions();
    en->SetMaxTriangles(dtmPtr->GetBcDTM()->GetTrianglesCount() * 2);
    for (PolyfaceQueryP pf : *en)
        {
        for (PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(*(pf)); addedFacets->AdvanceToNextFace();)
            {
            DPoint3d face[3];
            int32_t idx[3] = { -1, -1, -1 };
            for (size_t i = 0; i < 3; ++i)
                {
                face[i] = addedFacets->GetPointCP()[i];
                idx[i] = mapOfPoints.count(face[i]) != 0 ? mapOfPoints[face[i]] : -1;
                }
            if (((idx[0] != -1 && ((faces[idx[0]][0] && faces[idx[0]][1]) || neighborsOfMarkedVertices[idx[0]])) ||
                (idx[1] != -1 && ((faces[idx[1]][0] && faces[idx[1]][1]) || neighborsOfMarkedVertices[idx[1]])) ||
                (idx[2] != -1 && ((faces[idx[2]][0] && faces[idx[2]][1]) || neighborsOfMarkedVertices[idx[2]])) && (idx[0] == -1 || idx[1] == -1 || idx[2] == -1)) ||
                (idx[0]== -1 && idx[1] == -1 && idx[2] == -1))
                {
                for (size_t i = 0; i < 3; ++i)
                    {
                    if (idx[i] == -1)
                        {
                        idx[i] = (int)newDiff.addedVertices.size() + newDiff.firstIndex;
                        newDiff.addedVertices.push_back(face[i]);
                        }
                    else
                        {
                        //neighborsOfMarkedVertices[idx[i]] = true;
                        idx[i]++;
                        }
                    }
                newDiff.addedFaces.push_back(idx[0]);
                newDiff.addedFaces.push_back(idx[1]);
                newDiff.addedFaces.push_back(idx[2]);
                }
            }
        }
    return newDiff;
    }
    DifferenceSet  DifferenceSet::FromPolyface(PolyfaceHeaderPtr& polyMesh, map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> & mapOfPoints, size_t maxPtIdx)
    {
    DifferenceSet d;
    d.firstIndex = (int)maxPtIdx;
    if (polyMesh == nullptr) return d;
    size_t originalNFaces = d.addedFaces.size();
    for (PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(*polyMesh); addedFacets->AdvanceToNextFace();)
        {
        DPoint3d face[3];
        int32_t idx[3] = { -1, -1, -1 };
        for (size_t i = 0; i < 3; ++i)
            {
            face[i] = addedFacets->GetPointCP()[i];
            idx[i] = mapOfPoints.count(face[i]) != 0 ? mapOfPoints[face[i]] : -1;
            }
        for (size_t i = 0; i < 3; ++i)
            {
            if (idx[i] == -1)
                {
                idx[i] = (int)d.addedVertices.size() + d.firstIndex;
                d.addedVertices.push_back(face[i]);
                }
            else idx[i]++;
            }
        d.addedFaces.push_back(idx[0]);
        d.addedFaces.push_back(idx[1]);
        d.addedFaces.push_back(idx[2]);
        }
    if (polyMesh->GetParamCP() != nullptr && polyMesh->GetParamIndexCP() != nullptr)
        {
        size_t originalNUVs = d.addedUvs.size();
        d.addedUvs.insert(d.addedUvs.end(), polyMesh->GetParamCP(), polyMesh->GetParamCP() + polyMesh->GetParamCount());
        size_t originalSize = d.addedUvIndices.size();
        d.addedUvIndices.insert(d.addedUvIndices.end(), polyMesh->GetParamIndexCP(), polyMesh->GetParamIndexCP() + (int)(d.addedFaces.size() - originalNFaces));
        for (size_t i = originalSize; i < d.addedUvIndices.size(); ++i)
            {
            d.addedUvIndices[i] += (int)originalNUVs;
            assert(d.addedUvs.size() >= d.addedUvIndices[i]);
            }
        }
    return d;
    }

    DifferenceSet  DifferenceSet::FromPolyfaceSet(bvector<PolyfaceHeaderPtr>& polyMeshes, const DPoint3d* vertices, size_t nVertices)
    {
    DifferenceSet d;
    d.firstIndex = (int)nVertices+1;
    if (polyMeshes.size() == 0) return d;
    map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    for (size_t i = 0; i < nVertices; ++i)
        mapOfPoints[vertices[i]] = (int)i;
    return DifferenceSet::FromPolyfaceSet(polyMeshes, mapOfPoints, nVertices+1);
    }

    DifferenceSet  DifferenceSet::FromPolyfaceSet(bvector<PolyfaceHeaderPtr>& polyMeshes, map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> & mapOfPoints, size_t maxPtIdx)
    {
    DifferenceSet d;
    d.firstIndex = (int)maxPtIdx;
    if (polyMeshes.size() == 0) return d;
    d = DifferenceSet::FromPolyface(polyMeshes[0], mapOfPoints,maxPtIdx);
    for (size_t n = 1; n < polyMeshes.size(); ++n)
        {
        DifferenceSet d1 = DifferenceSet::FromPolyface(polyMeshes[n], mapOfPoints, maxPtIdx);
        d.ApplySet(d1, 0);
        }
    return d;
    }


    PolyfaceHeaderPtr DifferenceSet::ToPolyfaceMesh(const DPoint3d* points, size_t nOfPoints)
        {
        bvector<int32_t> indices;
        bvector<DPoint3d> pts;
        bmap<int, int> mapOfPts;
        size_t currentPt = 0;
        if (addedFaces.size() >= 3 && addedFaces.size() < 1024 * 1024)
            {

            for (int i = 0; i + 2 < addedFaces.size(); i += 3)
                {
                if (!(addedFaces[i] - 1 >= 0 && addedFaces[i] - 1 < nOfPoints + addedVertices.size() && addedFaces[i + 1] - 1 >= 0 && addedFaces[i + 1] - 1 < nOfPoints + addedVertices.size()
                    && addedFaces[i + 2] - 1 >= 0 && addedFaces[i + 2] - 1 < nOfPoints + addedVertices.size()))
                    {
                    continue;
                    }
                assert(addedFaces[i] - 1 >= 0 && addedFaces[i] - 1 < nOfPoints + addedVertices.size() && addedFaces[i + 1] - 1 >= 0 && addedFaces[i + 1] - 1 < nOfPoints + addedVertices.size()
                       && addedFaces[i + 2] - 1 >= 0 && addedFaces[i + 2] - 1 < nOfPoints + addedVertices.size());
                for (size_t j = 0; j < 3;  ++j)
                    {
                    int32_t idx = (int32_t)(addedFaces[i + j] >= firstIndex ? addedFaces[i + j] - firstIndex + nOfPoints + 1 : addedFaces[i + j]);
                    assert(idx > 0 && idx <= nOfPoints + addedVertices.size());
                    if (mapOfPts.count(idx) == 0)
                        {
                        mapOfPts[idx] = (int)currentPt;
                        pts.push_back(addedFaces[i + j] >= firstIndex ? addedVertices[addedFaces[i + j] - firstIndex] : points[addedFaces[i + j] - 1]);
                        currentPt++;
                        }
                    indices.push_back(mapOfPts[idx]+1);
                    }
                }
            }
#ifndef VANCOUVER_API
        return PolyfaceHeader::CreateIndexedMesh(3,pts, indices);
#else
        auto headerP = PolyfaceHeader::New();
        auto polyP = new PolyfaceQueryCarrier(3, false, pts.size(), indices.size(), &pts[0], &indices[0]);
        headerP->CopyFrom(*polyP);
        return headerP;
#endif
        }
END_BENTLEY_SCALABLEMESH_NAMESPACE