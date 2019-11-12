/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include "BcDtmProvider.h"
#include <TerrainModel/Formats/TerrainImporter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL


BEGIN_GROUND_DETECTION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProviderPtr BcDtmProvider::Create()
    {
    return new BcDtmProvider();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProviderPtr BcDtmProvider::CreateFrom(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM& bcDtm)
    {
    return new BcDtmProvider(bcDtm);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProviderPtr BcDtmProvider::CreateFrom(WChar* filename, WChar* name)
    {
    BcDTMPtr pBcDtm = LoadTerrainModel(filename, name);
    if (pBcDtm != nullptr)
        {
        return new BcDtmProvider(*pBcDtm);
        }
    return new BcDtmProvider();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProvider::BcDtmProvider() :m_pBcDtm(BcDTM::Create())
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProvider::BcDtmProvider(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM& bcDtm) :m_pBcDtm(&bcDtm)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProvider::~BcDtmProvider()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre               12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr BcDtmProvider::GetBcDTM()
    {
    return m_pBcDtm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDTMPtr BcDtmProvider::LoadTerrainModel(WChar* filename, WChar* name)
    {
    TerrainImporterPtr reader = TerrainImporter::CreateImporter(filename);
    WString surfaceName;

    if (name == nullptr)
        {
        const TerrainInfoList& surfaceInfos = reader->GetTerrains();

        if (surfaceInfos.size() == 0)
            return nullptr;

        surfaceName = surfaceInfos[0].GetName();
        }
    else
        surfaceName = name;

    ImportedTerrain dtm = reader->ImportTerrain(surfaceName.c_str());
    return dtm.GetTerrain();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProvider::BcDtmProviderIteratorImpl::BcDtmProviderIteratorImpl(BcDTMMesh& mesh, size_t currentItr)
:m_pMesh(&mesh), 
m_currentFace(currentItr),
m_currentIsDirty(true)
    {
    BeAssert(m_pMesh.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BcDtmProvider::BcDtmProviderIteratorImpl::~BcDtmProviderIteratorImpl()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BcDtmProvider::BcDtmProviderIteratorImpl::_IsDifferent(IDtmProvider::IDtmProviderIteratorImpl const& rhs)  const 
    {
    BcDtmProvider::BcDtmProviderIteratorImpl const* pRhs = dynamic_cast<BcDtmProvider::BcDtmProviderIteratorImpl const*>(&rhs);
    if (pRhs == nullptr)
        return true;
    return pRhs->m_currentFace != m_currentFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BcDtmProvider::BcDtmProviderIteratorImpl::_MoveToNext() 
    {
    m_currentFace++;
    m_currentIsDirty=true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IDtmProvider::IDtmProviderIteratorImpl::ReturnType& BcDtmProvider::BcDtmProviderIteratorImpl::_GetCurrent() const
    {
    BeAssert(m_pMesh.IsValid());

    if (m_currentIsDirty)
        {
        BcDTMMeshFacePtr face = m_pMesh->GetFace((long) m_currentFace);
        DPoint3d pt1 = face->GetCoordinates(0);
        DPoint3d pt2 = face->GetCoordinates(1);
        DPoint3d pt3 = face->GetCoordinates(2);
        m_current = Triangle(pt1, pt2, pt3);
        m_currentIsDirty=false;
        }

    return m_current;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BcDtmProvider::BcDtmProviderIteratorImpl::_IsAtEnd() const
    {
    BeAssert(m_pMesh.IsValid());
    return m_currentFace >= ((size_t)m_pMesh->GetFaceCount()/3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IDtmProvider::const_iterator BcDtmProvider::_begin() const 
    {
    BeAssert(m_pMesh.IsValid());
    RefCountedPtr<BcDtmProvider::BcDtmProviderIteratorImpl> impl = new BcDtmProvider::BcDtmProviderIteratorImpl(*m_pMesh, 0);
    return IDtmProvider::const_iterator(*impl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IDtmProvider::const_iterator BcDtmProvider::_end() const 
    {
    BeAssert(m_pMesh.IsValid());
    RefCountedPtr<BcDtmProvider::BcDtmProviderIteratorImpl> impl = new BcDtmProvider::BcDtmProviderIteratorImpl(*m_pMesh, m_pMesh->GetFaceCount()/3);
    return IDtmProvider::const_iterator(*impl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BcDtmProvider::_size() const 
    {
    BeAssert(m_pMesh.IsValid());

    return m_pMesh->GetFaceCount()/3;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BcDtmProvider::_GetTriangleCount() const
    {
    if (!m_pMesh.IsValid())
        return 0;

    return m_pMesh->GetFaceCount()/3;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BcDtmProvider::_GetPointCount() const
    {
    if (!m_pBcDtm.IsValid())
        return 0;

    return m_pBcDtm->GetPointCount();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  BcDtmProvider::_AddPoint(DPoint3d const& point) 
    {
    m_pBcDtm->AddPoint(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BcDtmProvider::_GetDTMPoints(DPoint3d* pPoints) const
    {
    if (!m_pMesh.IsValid())
        return ERROR;

    memcpy(pPoints, m_pMesh->GetPointReference(), m_pMesh->GetPointCount() * sizeof (DPoint3d));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
long BcDtmProvider::_ComputeTriangulation() 
    {
    DTMStatusInt dtmStatus;
    static long   edgeOption = 2;         // ==> Triangulation Edge Option <1,2,3> Usually 2
    static double maxSideLength = 250;    // ==> Max Triangle Side Length For External Triangles ** Required For Edge Option = 3
    static double p2pTolerance = 0.0001;  // ==> Triangulation Point to Point Tolerance Usually 0.0001

    if (m_pBcDtm->GetPointCount() < 3)
        return 0; //not enough point

    // Seed Triangulation delaunay
    if (DTM_SUCCESS != (dtmStatus = m_pBcDtm->SetTriangulationParameters(p2pTolerance, p2pTolerance, edgeOption, maxSideLength)))
        return 0; //Error, stop iteration

    if (DTM_SUCCESS != (dtmStatus = m_pBcDtm->Triangulate()))
        return 0; //Error, stop iteration

    if (DTM_SUCCESS != (dtmStatus = m_pBcDtm->CheckTriangulation()))
        return 0;  //Error, stop iteration

    long newTrianglesCount(m_pBcDtm->GetTrianglesCount());
    
    bvector<DPoint3d> meshPts;
    bvector<long> meshFaces;

    DTMStatusInt status = (DTMStatusInt)bcdtmLoad_tinMeshFromDtmObject(m_pBcDtm->GetTinHandle(), 1, newTrianglesCount, NULL, DTMFenceType::Block, DTMFenceOption::Overlap, NULL, 0, meshPts, meshFaces);

    BeAssert(status == DTM_SUCCESS);

    m_pMesh = BcDTMMesh::Create(meshPts, meshFaces);

    BeAssert(m_pMesh.IsValid());

    return newTrianglesCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BcDtmProvider::_FindNearestTriangleDistanceFromPoint(Triangle* pTri, double& distance, DPoint3d const& point) const
    {
    bool found(false);
    BC_DTM_OBJ* dtmObject = m_pBcDtm->GetTinHandle();

    long fndTypeP, pnt1, pnt2, pnt3;
    double Zs;
    StatusInt status = bcdtmFind_triangleForPointDtmObject(dtmObject, point.x, point.y, &Zs, &fndTypeP, &pnt1, &pnt2, &pnt3);
    if (status != DTM_SUCCESS)
        return NULL;

    distance = fabs(point.z - Zs);

    switch (fndTypeP)
        {
        case 0: //Point External To Dtm
            {
            found = false;
            }
            break;
        case 1: //Point Coincident with Point pnt1
            {
            found = true;
            }
            break;
        case 2: //Point On Line pnt1 - Ppnt2P
            {
            found = true;
            }
            break;
        case 3: // Point On Hull Line pnt1-pnt2
            {
            found = false;
            }
            break;
        case 4: //Point In Triangle pnt1-pnt2-pnt3
            {
            found = true;

            if (pnt1 == dtmObject->nullPnt || pnt2 == dtmObject->nullPnt || pnt3 == dtmObject->nullPnt)
                return false;

            DPoint3d p1;
            p1.x = pointAddrP(dtmObject, pnt1)->x;
            p1.y = pointAddrP(dtmObject, pnt1)->y;
            p1.z = pointAddrP(dtmObject, pnt1)->z;

            DPoint3d p2;
            p2.x = pointAddrP(dtmObject, pnt2)->x;
            p2.y = pointAddrP(dtmObject, pnt2)->y;
            p2.z = pointAddrP(dtmObject, pnt2)->z;

            DPoint3d p3;
            p3.x = pointAddrP(dtmObject, pnt3)->x;
            p3.y = pointAddrP(dtmObject, pnt3)->y;
            p3.z = pointAddrP(dtmObject, pnt3)->z;

            Triangle foundTri(p1, p2, p3);

            //Return value if asked for
            if (pTri!=nullptr)
                *pTri = foundTri;

            //Distance returned by  bcdtmFind_triangleForPointDtmObject is vertical projection
            //We want true distance from triangle plane
            //Compute it
            DPlane3d planeFromTriangle = foundTri.GetPlane();
            if (planeFromTriangle.normal.z < 0)
                planeFromTriangle.normal.Negate();
            if (planeFromTriangle.Normalize()) //true if normal vector has nonzero length
                {
                //If the plane normal is a unit vector, this is the true distance from the
                //plane to the point.  If not, it is a scaled distance.
                //The plane is normalized, so it is the true distance.
                double distanceFound = planeFromTriangle.Evaluate(point);
                distance = distanceFound;
                }
            }
            break;
        }

    return found;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elenie.Godzaridis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BcDtmProvider::AddFaceToEdge(T_EdgeMap& edgeMap, int idx1, int idx2, int i) const
    {
    auto idx1_it = std::find_if(edgeMap[idx1].begin(), edgeMap[idx1].end(), [idx2](T_PairIndexAndIndexCollection& edge) { return edge.first == idx2; });
    if (idx1_it == edgeMap[idx1].end())
        {
        T_IndexCollection indexCollection;
        indexCollection.push_back(i);
        edgeMap[idx1].push_back(make_pair(idx2, indexCollection));
        }
    else
        {
        idx1_it->second.push_back(i);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elenie.Godzaridis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BcDtmProvider::_ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats) const
    {
    if (!m_pMesh.IsValid())
        return;

    // get all triangles

    int meshPointCount(m_pMesh->GetPointCount());
    T_EdgeMap edgeMap(meshPointCount);
    size_t nbFaces(m_pMesh->GetFaceCount()/3); //One face contains three indices, but GetfaceCount returns number of indices.
    for (size_t i = 0; i < nbFaces; ++i)
        {
        auto face = m_pMesh->GetFace((long) i); //GetFace will multiply index by 3 to point to corresponding face.
        int idx1 = face->GetMeshPointIndex(0) - 1;
        int idx2 = face->GetMeshPointIndex(1) - 1;
        int idx3 = face->GetMeshPointIndex(2) - 1;
        BeAssert(idx1 >= 0 && idx2 >= 0 && idx3 >= 0 && idx1 < meshPointCount && idx2 < meshPointCount && idx3 < meshPointCount);
        AddFaceToEdge(edgeMap, idx1, idx2, idx3);
        AddFaceToEdge(edgeMap, idx1, idx3, idx2);
        AddFaceToEdge(edgeMap, idx2, idx1, idx3);
        AddFaceToEdge(edgeMap, idx2, idx3, idx1);
        AddFaceToEdge(edgeMap, idx3, idx1, idx2);
        AddFaceToEdge(edgeMap, idx3, idx2, idx1);
        }

    size_t i=0;
    for (auto itr = edgeMap.begin(); itr != edgeMap.end(); ++itr,++i)
        {
        for (auto jItr = itr->begin(); jItr != itr->end(); ++jItr)
            {
            if (jItr->second.size() <= 1)
                continue;
            DPoint3d pt1 = m_pMesh->GetPoint((int)i);
            DPoint3d pt2 = m_pMesh->GetPoint(jItr->first);
            DPoint3d pt3 = m_pMesh->GetPoint(jItr->second[0]);
            DPoint3d pt4 = m_pMesh->GetPoint(jItr->second[1]);

            DVec3d vec1 = DPlane3d::From3Points(pt1, pt2, pt3).normal;
            DVec3d vec2 = DPlane3d::From3Points(pt1, pt2, pt4).normal;
            double angleValue(vec1.SmallerUnorientedAngleTo(vec2));
            angleStats.IncrementCountFromValue(angleValue);
            double heightValue = fabs(pt1.z - pt2.z);
            heightStats.IncrementCountFromValue(heightValue);
            heightValue = fabs(pt3.z - pt2.z);
            heightStats.IncrementCountFromValue(heightValue);
            heightValue = fabs(pt3.z - pt1.z);
            heightStats.IncrementCountFromValue(heightValue);
            heightValue = fabs(pt4.z - pt2.z);
            heightStats.IncrementCountFromValue(heightValue);
            heightValue = fabs(pt4.z - pt1.z);
            heightStats.IncrementCountFromValue(heightValue);
            }
        }
    }




END_GROUND_DETECTION_NAMESPACE
