/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/TiledTriangulatorValidator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include "DcStmCorePCH.h"
//#include "ScalableMeshATPPch.h"
#include "TiledTriangulatorValidator.h"
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCoreAPI.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH
    
#define MAX_POINTS_PER_DTM   10000
#define MAX_FEATURES_PER_DTM UINT_MAX


//BEGIN_GEODTMAPP_NAMESPACE


/*----------------------------------------------------------------------------+
|ITiledTriangulatorValidator Method Definition Section - Begin
+----------------------------------------------------------------------------*/
/*ITiledTriangulatorValidatorPtr ITiledTriangulatorValidator::CreateFor (RefCountedPtr<BcDTM> memDtmPtr)
    {
    return new TiledTriangulatorValidator(memDtmPtr);
    }

int ITiledTriangulatorValidator::CompareMemDTMwithTileDTM(const DTMPtr& tileDtmPtr)
    {
    return _CompareMemDTMwithTileDTM(tileDtmPtr);
    }

int ITiledTriangulatorValidator::GetLastTileStat(unsigned __int64& nbComparedTriangles, unsigned __int64& nbWrongTriangles) const
    {
    return _GetLastTileStat(nbComparedTriangles, nbWrongTriangles);
    }

int ITiledTriangulatorValidator::GetTotalStat(unsigned __int64& totalNbComparedTiles,
                                              unsigned __int64& totalNbWrongTiles,
                                              unsigned __int64& totalNbComparedTriangles,
                                              unsigned __int64& totalNbWrongTriangles)  const
    {
    return _GetTotalStat(totalNbComparedTiles,
                         totalNbWrongTiles,
                         totalNbComparedTriangles,
                         totalNbWrongTriangles);
    }

int ITiledTriangulatorValidator::SetOuputInActiveModel(bool outputIncorrectTriangles)
    {
    return _SetOuputInActiveModel(outputIncorrectTriangles);
    }

/*----------------------------------------------------------------------------+
|ITiledTriangulatorValidator Method Definition Section - End
+----------------------------------------------------------------------------*/

/*
//#define DBL_COMPARE_TOL 0.00001
static double DBL_COMPARE_TOL = 0.01;

static bool s_displayIncorrectVertex = false;
static int  s_nbXbins = 50;
static int  s_nbYbins = 50;
static double s_tolScaleForIndexing = 2;

static bool s_useTriSearchNewCode = true;

/*
AddFace(TreeNode& m_RootNode, )
    {
    if (m_RootNode.m_childNodes.size() > 0)
        {

        }

    }

struct TreeNode
    {
    DRange2d                    m_dRange2d;
    vector<FaceWithProperties>  m_faces;
    vector<TreeNode>            m_childNodes;
    };
    */

    /*
TiledTriangulatorValidator::TiledTriangulatorValidator(RefCountedPtr<BcDTM> memDtmPtr)
    {
    m_outputIncorrectTriangles = false;

    m_totalNbComparedTiles = 0;
    m_totalNbWrongTiles = 0;

    m_totalNbComparedTriangles = 0;
    m_totalNbWrongTriangles = 0;

    m_nbComparedTrianglesLastTile = 0;
    m_nbWrongTrianglesLastTile = 0;

    assert(memDtmPtr->GetTrianglesCount() > 0);

    m_memDtmPtr = memDtmPtr;

    BcDTMMeshPtr mesh = memDtmPtr->GetMesh(TRUE, memDtmPtr->GetTrianglesCount(), NULL, 0);

    if (mesh == 0)
        {
        throw ERROR;
        }

    getAllMeshFacesAndComputeBoundingCircles(mesh, m_memDtmFaces);

    //mesh->Release();

    DPoint3d minRange;
    DPoint3d maxRange;
    DRange3d range;
    memDtmPtr->GetRange(range);
    minRange = range.low;
    maxRange = range.high;

    m_indexRangeX = maxRange.x - minRange.x;
    m_indexRangeY = maxRange.y - minRange.y;

    m_indexStepX = m_indexRangeX / s_nbXbins;
    m_indexStepY = m_indexRangeY / s_nbYbins;


    //MST TBD - Should take into account unit - e.g. 0.02 for lat long is awfully big.
    assert(m_indexStepX > DBL_COMPARE_TOL * s_tolScaleForIndexing * 10);
    assert(m_indexStepY > DBL_COMPARE_TOL * s_tolScaleForIndexing * 10);

    m_memDtmIndexedFaces.resize(s_nbXbins * s_nbYbins);

    vector<FaceWithProperties>::const_iterator faceIter(m_memDtmFaces.begin());
    vector<FaceWithProperties>::const_iterator faceIterEnd(m_memDtmFaces.end());

    int indXforTol = -1;
    int indYforTol = -1;

    double toleranceForIndexing = DBL_COMPARE_TOL * s_tolScaleForIndexing;

    for (size_t faceInd = 0; faceInd < m_memDtmFaces.size(); faceInd++)
        {
        for (size_t coordInd = 0; coordInd < 3; coordInd++)
            {
            int indX = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).x -  minRange.x) / m_indexStepX);
            indX = max(0, min(indX, s_nbXbins - 1));

            indXforTol = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).x + toleranceForIndexing - minRange.x) / m_indexStepX);
            indXforTol = max(0, min(indXforTol, s_nbXbins - 1));

            if (indXforTol == indX)
                {
                indXforTol = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).x - toleranceForIndexing - minRange.x) / m_indexStepX);
                indXforTol = max(0, min(indXforTol, s_nbXbins - 1));

                if (indXforTol == indX)
                    {
                    indXforTol = -1;
                    }
                }
    #ifndef NDEBUG
            else
                {
                int indXforTolTemp = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).x - toleranceForIndexing -  minRange.x) / m_indexStepX);
                indXforTolTemp = max(0, min(indXforTolTemp, s_nbXbins - 1));
                assert(indXforTolTemp == indX);
                }
    #endif

            int indY = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).y - minRange.y) / m_indexStepY);
            indY = max(0, min(indY, s_nbYbins - 1));

            indYforTol = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).y + toleranceForIndexing - minRange.y) / m_indexStepY);
            indYforTol = max(0, min(indYforTol, s_nbYbins - 1));

            if (indYforTol == indY)
                {
                indYforTol = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).y - toleranceForIndexing - minRange.y) / m_indexStepY);
                indYforTol = max(0, min(indYforTol, s_nbYbins - 1));

                if (indYforTol == indY)
                    {
                    indYforTol = -1;
                    }
                }
    #ifndef NDEBUG
            else
                {
                int indYforTolTemp = (int)((m_memDtmFaces[faceInd].GetFace()->GetCoordinates((int)coordInd).y - toleranceForIndexing -  minRange.y) / m_indexStepY);
                indYforTolTemp = max(0, min(indYforTolTemp, s_nbYbins - 1));
                assert(indYforTolTemp == indY);
                }
    #endif

            m_memDtmIndexedFaces[indX + s_nbXbins * indY].push_back(faceInd);

            if (indXforTol != -1)
                {
                m_memDtmIndexedFaces[indXforTol + s_nbXbins * indY].push_back(faceInd);

                if (indYforTol != -1)
                    {
                    m_memDtmIndexedFaces[indX + s_nbXbins * indYforTol].push_back(faceInd);
                    m_memDtmIndexedFaces[indXforTol + s_nbXbins * indYforTol].push_back(faceInd);
                    }
                }
            else
            if (indYforTol != -1)
                {
                m_memDtmIndexedFaces[indX + s_nbXbins * indYforTol].push_back(faceInd);
                }
            }

        faceIter++;
        }
    }

TiledTriangulatorValidator::~TiledTriangulatorValidator()
    {
    }

 int TiledTriangulatorValidator::_GetLastTileStat(unsigned __int64& nbComparedTriangles, unsigned __int64& nbWrongTriangles) const
    {
    nbComparedTriangles = m_nbComparedTrianglesLastTile;
    nbWrongTriangles = m_nbWrongTrianglesLastTile;

    return SUCCESS;
    }

int TiledTriangulatorValidator::_GetTotalStat(unsigned __int64& totalNbComparedTiles,
                                              unsigned __int64& totalNbWrongTiles,
                                              unsigned __int64& totalNbComparedTriangles,
                                              unsigned __int64& totalNbWrongTriangles) const
    {
    totalNbComparedTiles = m_totalNbComparedTiles;
    totalNbWrongTiles = m_totalNbWrongTiles;
    totalNbComparedTriangles = m_totalNbComparedTriangles;
    totalNbWrongTriangles = m_totalNbWrongTriangles;

    return SUCCESS;
    }

//MST TBD - Not sure we want to consider that as valid.
static bool s_considerPointsAddedOnLinearFeature = false;

int TiledTriangulatorValidator::_CompareMemDTMwithTileDTM(const DTMPtr& tileDtmPtr)
    {
    int status = ERROR;

    m_totalNbComparedTiles++;

    m_nbComparedTrianglesLastTile = tileDtmPtr->GetBcDTM()->GetTrianglesCount();
    m_nbWrongTrianglesLastTile  = 0;

    m_totalNbComparedTriangles += m_nbComparedTrianglesLastTile;

    assert((tileDtmPtr != 0) && (tileDtmPtr->GetBcDTM()->GetTrianglesCount()));
    /*
    vector<FaceWithProperties>::iterator facesInMemIter(facesInMem.begin());
    vector<FaceWithProperties>::iterator facesInMemIterEnd(facesInMem.end());

    vector<FaceWithProperties>::iterator facesInTileIter(facesInTile.begin());
    vector<FaceWithProperties>::iterator facesInTileIterEnd(facesInTile.end());
    */
/*
    long numTriangles = tileDtmPtr->GetBcDTM()->GetTrianglesCount();

    if (numTriangles == 0) return ERROR;

    BcDTMMeshPtr mesh = tileDtmPtr->GetBcDTM()->GetMesh(TRUE, numTriangles, NULL, 0);

    vector<FaceWithProperties> facesInTile;
    getAllMeshFacesAndComputeBoundingCircles(mesh, facesInTile);

   // mesh->Release();

    vector<FaceWithProperties> invalidTriangles;

    DPoint3d minRange;
    DPoint3d maxRange;
    DRange3d range;

    m_memDtmPtr->GetRange(range);
    minRange = range.low;
    maxRange = range.high;

    int indXforTol;
    int indYforTol;

    double toleranceForIndexing = DBL_COMPARE_TOL * s_tolScaleForIndexing;

#ifdef NDEBUG
//    #pragma omp parallel for
#endif
    for (int faceInTileInd = 0; faceInTileInd < (int)facesInTile.size(); faceInTileInd++)
        {
        vector<size_t>::const_iterator faceIter;
        vector<size_t>::const_iterator faceIterEnd;

        for (size_t coordInd = 0; coordInd < 3; coordInd++)
            {
//            size_t faceInMemInd = 0;

            int indX = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).x - minRange.x) / m_indexStepX);
            indX = max(0, min(indX, s_nbXbins - 1));

            int indY = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).y - minRange.y) / m_indexStepY);
            indY = max(0, min(indY, s_nbYbins - 1));

    //MST TBD - Should not be duplicated
    //MST TBD - Probably not necessary
            indXforTol = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).x + toleranceForIndexing - minRange.x) / m_indexStepX);
            indXforTol = max(0, min(indXforTol, s_nbXbins - 1));

            if (indXforTol == indX)
                {
                indXforTol = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).x - toleranceForIndexing - minRange.x) / m_indexStepX);
                indXforTol = max(0, min(indXforTol, s_nbXbins - 1));

                if (indXforTol == indX)
                    {
                    indXforTol = -1;
                    }
                }
    #ifndef NDEBUG
            else
                {
                int indXforTolTemp = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).x - toleranceForIndexing -  minRange.x) / m_indexStepX);
                indXforTolTemp = max(0, min(indXforTolTemp, s_nbXbins - 1));
                assert(indXforTolTemp == indX);
                }
    #endif

            indYforTol = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).y + toleranceForIndexing - minRange.y) / m_indexStepY);
            indYforTol = max(0, min(indYforTol, s_nbYbins - 1));

            if (indYforTol == indY)
                {
                indYforTol = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).y - toleranceForIndexing - minRange.y) / m_indexStepY);
                indYforTol = max(0, min(indYforTol, s_nbYbins - 1));

                if (indYforTol == indY)
                    {
                    indYforTol = -1;
                    }
                }
    #ifndef NDEBUG
            else
                {
                int indYforTolTemp = (int)((facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).y - toleranceForIndexing -  minRange.y) / m_indexStepY);
                indYforTolTemp = max(0, min(indYforTolTemp, s_nbYbins - 1));
                assert(indYforTolTemp == indY);
                }
    #endif

    /////------------------
            int indType = 0;
            int currentIndX = indX;
            int currentIndY = indY;

            while (indType <= 3)
                {
                faceIter = m_memDtmIndexedFaces[currentIndX + s_nbXbins * currentIndY].begin();
                faceIterEnd = m_memDtmIndexedFaces[currentIndX + s_nbXbins * currentIndY].end();

                while (faceIter != faceIterEnd)
                    {
                    const FaceWithProperties& faceWithProperties(m_memDtmFaces[*faceIter]);

                    if ((fabs(faceWithProperties.GetFace()->GetCoordinates((int)coordInd).x - facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).x) <= DBL_COMPARE_TOL) &&
                        (fabs(faceWithProperties.GetFace()->GetCoordinates((int)coordInd).y - facesInTile[faceInTileInd].GetFace()->GetCoordinates((int)coordInd).y) <= DBL_COMPARE_TOL) &&
                        (fabs(faceWithProperties.GetProperties().GetCircumRadiusSquared() - facesInTile[faceInTileInd].GetProperties().GetCircumRadiusSquared()) <= DBL_COMPARE_TOL))
                        {
                        break;
                        }

                    faceIter++;
                    }

                if (faceIter != faceIterEnd)
                    {
                    break;
                    }

                indType++;

                if (indType == 1)
                    {
                    if (indXforTol != -1)
                        {
                        currentIndX = indXforTol;
                        currentIndY = indY;
                        }
                    else
                        {
                        indType++;
                        }
                    }

                if (indType == 2)
                    {
                    if (indYforTol != -1)
                        {
                        currentIndX = indXforTol;
                        currentIndY = indY;
                        }
                    else
                        {
                        indType++;
                        }
                    }

                if (indType == 3)
                    {
                    if (indXforTol != -1 && indYforTol != -1)
                        {
                        currentIndX = indXforTol;
                        currentIndY = indYforTol;
                        }
                    else
                        {
                        indType++;
                        }
                    }
                }

            if (faceIter != faceIterEnd)
                {
                break;
                }
            }

        if (faceIter == faceIterEnd)
            {
            invalidTriangles.push_back(facesInTile[faceInTileInd]);
            }
        }

    if (invalidTriangles.size() == 0)
        {
        status = SUCCESS;
        }
    else
        {
       // int indXforTol = -1;
       // int indYforTol = -1;

        double toleranceForIndexing = DBL_COMPARE_TOL * s_tolScaleForIndexing;

        vector<FaceWithProperties>::iterator invalidTriIter(invalidTriangles.begin());

        long firstCall   = true;
        long maxMeshSize = m_memDtmPtr->GetTrianglesCount();
        DPoint3d fencePts[5];
        long numFencePts = 5;

        /*
        DPoint3d minRange;
        DPoint3d maxRange;

        int status = m_memDtmPtr->getRange(&minRange, &maxRange);
        */
/*
        while (invalidTriIter != invalidTriangles.end())
            {
            if (s_useTriSearchNewCode)
                {
                double minX = DBL_MAX;
                double maxX = -DBL_MAX;
                double minY = DBL_MAX;
                double maxY = -DBL_MAX;

                for (size_t coordInd = 0; coordInd < 3; coordInd++)
                    {
                    minX = min(minX, (invalidTriIter)->GetFace()->GetCoordinates((int)coordInd).x);
                    maxX = max(maxX, (invalidTriIter)->GetFace()->GetCoordinates((int)coordInd).x);
                    minY = min(minY, (invalidTriIter)->GetFace()->GetCoordinates((int)coordInd).y);
                    maxY = max(maxY, (invalidTriIter)->GetFace()->GetCoordinates((int)coordInd).y);
                    }

                minX -= toleranceForIndexing;
                maxX += toleranceForIndexing;
                minY -= toleranceForIndexing;
                maxY += toleranceForIndexing;

                fencePts[0].x = minX;
                fencePts[0].y = minY;
                fencePts[0].z = 0;

                fencePts[1].x = maxX;
                fencePts[1].y = minY;
                fencePts[1].z = 0;

                fencePts[2].x = maxX;
                fencePts[2].y = maxY;
                fencePts[2].z = 0;

                fencePts[3].x = minX;
                fencePts[3].y = maxY;
                fencePts[3].z = 0;

                fencePts[4].x = minX;
                fencePts[4].y = minY;
                fencePts[4].z = 0;

                BcDTMMeshPtr pMesh = m_memDtmPtr->GetMesh(firstCall, maxMeshSize, fencePts, numFencePts);

                if (pMesh == 0)
                    {
                    invalidTriIter++;
                    }
                else
                    {
                    const int numFaces = pMesh->GetFaceCount();

                    int i = 0;

                    //IBcDTMMeshFacePtr pFace;

                        for (; i < numFaces; i++)
                        {
                        //pFace = pMesh->GetFace(i);
                        IBcDTMMeshFacePtr pFace = pMesh->GetFace(i);

                        double x0x0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).x - pFace->GetCoordinates(0).x);
                        double x0x1 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).x - pFace->GetCoordinates(1).x);
                        double x0x2 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).x - pFace->GetCoordinates(2).x);

                        double x1x0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).x - pFace->GetCoordinates(0).x);
                        double x1x1 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).x - pFace->GetCoordinates(1).x);
                        double x1x2 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).x - pFace->GetCoordinates(2).x);

                        double x2x0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).x - pFace->GetCoordinates(0).x);
                        double x2x1 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).x - pFace->GetCoordinates(1).x);
                        double x2x2 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).x - pFace->GetCoordinates(2).x);

                        if (((x0x0 < DBL_COMPARE_TOL) ||
                             (x0x1 < DBL_COMPARE_TOL) ||
                             (x0x2 < DBL_COMPARE_TOL)) &&
                             //Cood 1
                            ((x1x0 < DBL_COMPARE_TOL) ||
                             (x1x1 < DBL_COMPARE_TOL) ||
                             (x1x2 < DBL_COMPARE_TOL)) &&
                             //Cood 2
                            ((x2x0 < DBL_COMPARE_TOL) ||
                             (x2x1 < DBL_COMPARE_TOL) ||
                             (x2x2 < DBL_COMPARE_TOL)))
                            {
                            double y0y0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).y - pFace->GetCoordinates(0).y);
                            double y0y1 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).y - pFace->GetCoordinates(1).y);
                            double y0y2 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).y - pFace->GetCoordinates(2).y);

                            double y1y0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).y - pFace->GetCoordinates(0).y);
                            double y1y1 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).y - pFace->GetCoordinates(1).y);
                            double y1y2 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).y - pFace->GetCoordinates(2).y);

                            double y2y0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).y - pFace->GetCoordinates(0).y);
                            double y2y1 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).y - pFace->GetCoordinates(1).y);
                            double y2y2 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).y - pFace->GetCoordinates(2).y);

                            if (((y0y0 < DBL_COMPARE_TOL) ||
                                 (y0y1 < DBL_COMPARE_TOL) ||
                                 (y0y2 < DBL_COMPARE_TOL)) &&
                                 //Cood 1
                                ((y1y0 < DBL_COMPARE_TOL) ||
                                 (y1y1 < DBL_COMPARE_TOL) ||
                                 (y1y2 < DBL_COMPARE_TOL)) &&
                                 //Cood 2
                                ((y2y0 < DBL_COMPARE_TOL) ||
                                 (y2y1 < DBL_COMPARE_TOL) ||
                                 (y2y2 < DBL_COMPARE_TOL)))
                                {
                                break;
                                }
                            }
                        }
                    //pFace->Release();
                    if (i < numFaces)
                        {
                        invalidTriIter = invalidTriangles.erase(invalidTriIter);
                        }
                    else
                        {
                        invalidTriIter++;
                        }

                    //pMesh->Release();
                    }
                }
            else
                {
                    /*
                vector<vector<FaceWithProperties>>::const_iterator faceBinIter(m_memDtmIndexedFaces.begin());
                vector<vector<FaceWithProperties>>::const_iterator faceBinIterEnd(m_memDtmIndexedFaces.end());

                while (faceBinIter != faceBinIterEnd)
                    {
                    vector<FaceWithProperties>::const_iterator faceIter(faceBinIter->begin());
                    vector<FaceWithProperties>::const_iterator faceIterEnd(faceBinIter->end());

                    while (faceIter != faceIterEnd)
                        {
                        double x0x0 = fabs(invalidTriIter->GetFace()->GetCoordinates(0).x - faceIter->GetFace()->GetCoordinates(0).x);
                        double x0x1 = fabs(invalidTriIter->GetFace()->GetCoordinates(0).x - faceIter->GetFace()->GetCoordinates(1).x);
                        double x0x2 = fabs(invalidTriIter->GetFace()->GetCoordinates(0).x - faceIter->GetFace()->GetCoordinates(2).x);

                        double x1x0 = fabs(invalidTriIter->GetFace()->GetCoordinates(1).x - faceIter->GetFace()->GetCoordinates(0).x);
                        double x1x1 = fabs(invalidTriIter->GetFace()->GetCoordinates(1).x - faceIter->GetFace()->GetCoordinates(1).x);
                        double x1x2 = fabs(invalidTriIter->GetFace()->GetCoordinates(1).x - faceIter->GetFace()->GetCoordinates(2).x);

                        double x2x0 = fabs(invalidTriIter->GetFace()->GetCoordinates(2).x - faceIter->GetFace()->GetCoordinates(0).x);
                        double x2x1 = fabs(invalidTriIter->GetFace()->GetCoordinates(2).x - faceIter->GetFace()->GetCoordinates(1).x);
                        double x2x2 = fabs(invalidTriIter->GetFace()->GetCoordinates(2).x - faceIter->GetFace()->GetCoordinates(2).x);

                        if (((x0x0 < DBL_COMPARE_TOL) ||
                             (x0x1 < DBL_COMPARE_TOL) ||
                             (x0x2 < DBL_COMPARE_TOL)) &&
                             //Cood 1
                            ((x1x0 < DBL_COMPARE_TOL) ||
                             (x1x1 < DBL_COMPARE_TOL) ||
                             (x1x2 < DBL_COMPARE_TOL)) &&
                             //Cood 2
                            ((x2x0 < DBL_COMPARE_TOL) ||
                             (x2x1 < DBL_COMPARE_TOL) ||
                             (x2x2 < DBL_COMPARE_TOL)))
                            {
                            double y0y0 = fabs(invalidTriIter->GetFace()->GetCoordinates(0).y - faceIter->GetFace()->GetCoordinates(0).y);
                            double y0y1 = fabs(invalidTriIter->GetFace()->GetCoordinates(0).y - faceIter->GetFace()->GetCoordinates(1).y);
                            double y0y2 = fabs(invalidTriIter->GetFace()->GetCoordinates(0).y - faceIter->GetFace()->GetCoordinates(2).y);

                            double y1y0 = fabs(invalidTriIter->GetFace()->GetCoordinates(1).y - faceIter->GetFace()->GetCoordinates(0).y);
                            double y1y1 = fabs(invalidTriIter->GetFace()->GetCoordinates(1).y - faceIter->GetFace()->GetCoordinates(1).y);
                            double y1y2 = fabs(invalidTriIter->GetFace()->GetCoordinates(1).y - faceIter->GetFace()->GetCoordinates(2).y);

                            double y2y0 = fabs(invalidTriIter->GetFace()->GetCoordinates(2).y - faceIter->GetFace()->GetCoordinates(0).y);
                            double y2y1 = fabs(invalidTriIter->GetFace()->GetCoordinates(2).y - faceIter->GetFace()->GetCoordinates(1).y);
                            double y2y2 = fabs(invalidTriIter->GetFace()->GetCoordinates(2).y - faceIter->GetFace()->GetCoordinates(2).y);

                            if (((y0y0 < DBL_COMPARE_TOL) ||
                                 (y0y1 < DBL_COMPARE_TOL) ||
                                 (y0y2 < DBL_COMPARE_TOL)) &&
                                 //Cood 1
                                ((y1y0 < DBL_COMPARE_TOL) ||
                                 (y1y1 < DBL_COMPARE_TOL) ||
                                 (y1y2 < DBL_COMPARE_TOL)) &&
                                 //Cood 2
                                ((y2y0 < DBL_COMPARE_TOL) ||
                                 (y2y1 < DBL_COMPARE_TOL) ||
                                 (y2y2 < DBL_COMPARE_TOL)))
                                {
                                break;
                                }
                            }



    /*
                        if ((bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(0), &faceIter->GetFace()->GetCoordinates(0), DBL_COMPARE_TOL) ||
                             bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(0), &faceIter->GetFace()->GetCoordinates(1), DBL_COMPARE_TOL) ||
                             bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(0), &faceIter->GetFace()->GetCoordinates(2), DBL_COMPARE_TOL)) &&
                             //Coord 1
                            (bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(1), &faceIter->GetFace()->GetCoordinates(0), DBL_COMPARE_TOL) ||
                             bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(1), &faceIter->GetFace()->GetCoordinates(1), DBL_COMPARE_TOL) ||
                             bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(1), &faceIter->GetFace()->GetCoordinates(2), DBL_COMPARE_TOL)) &&
                             //Coord 2
                            (bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(2), &faceIter->GetFace()->GetCoordinates(0), DBL_COMPARE_TOL) ||
                             bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(2), &faceIter->GetFace()->GetCoordinates(1), DBL_COMPARE_TOL) ||
                             bsiDPoint3d_pointEqualTolerance(&invalidTriIter->GetFace()->GetCoordinates(2), &faceIter->GetFace()->GetCoordinates(2), DBL_COMPARE_TOL)))
                            {
                            break;
                            }

                        faceIter++;

                        }

                    if (faceIter != faceIterEnd)
                        {
                        break;
                        }

                    faceBinIter++;
                    }

                if (faceBinIter != faceBinIterEnd)
                    {
                    invalidTriIter = invalidTriangles.erase(invalidTriIter);
                    }
                else
                    {
                    invalidTriIter++;
                    }
                    */
/*                }
            }
        }


    if (invalidTriangles.size() == 0)
        {
        status = SUCCESS;
        }
    else
        {
        if (s_considerPointsAddedOnLinearFeature)
            {
            vector<FaceWithProperties>::iterator invalidTriIter(invalidTriangles.begin());
            vector<DPoint3d> pointsToCheck;

            while (invalidTriIter != invalidTriangles.end())
                {
                long nbVertices = (long)m_memDtmPtr->GetPointCount ();
                bool vert0Found = false;
                bool vert1Found = false;
                bool vert2Found = false;

                DPoint3d pt;

                for (long ptInd = 0; ptInd < nbVertices; ptInd++)
                    {
                    int status = m_memDtmPtr->GetPoint(ptInd, pt);
                    assert(status == SUCCESS);

                    if (vert0Found == false)
                        {
                        double x0x0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).x - pt.x);
                        double y0y0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(0).y - pt.y);

                        if (x0x0 < DBL_COMPARE_TOL && y0y0 < DBL_COMPARE_TOL)
                            {
                            vert0Found = true;
                            }
                        }

                    if (vert1Found == false)
                        {
                        double x1x0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).x - pt.x);
                        double y1y0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(1).y - pt.y);

                        if (x1x0 < DBL_COMPARE_TOL && y1y0 < DBL_COMPARE_TOL)
                            {
                            vert1Found = true;
                            }
                        }

                    if (vert2Found == false)
                        {
                        double x2x0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).x - pt.x);
                        double y2y0 = fabs((invalidTriIter)->GetFace()->GetCoordinates(2).y - pt.y);

                        if (x2x0 < DBL_COMPARE_TOL && y2y0 < DBL_COMPARE_TOL)
                            {
                            vert2Found = true;
                            }
                        }

                    if (vert0Found == true && vert1Found == true && vert2Found == true)
                        {
                        break;
                        }
                    }

                //MST TBD - Should add check that the vertice is on a linear feature.
                if (vert0Found == false || vert1Found == false || vert2Found == false)
                    {
                    invalidTriIter = invalidTriangles.erase(invalidTriIter);
                    }
                else
                    {
                    invalidTriIter++;
                    }
                }
            }
        }

    m_nbWrongTrianglesLastTile = invalidTriangles.size();
    m_totalNbWrongTriangles += m_nbWrongTrianglesLastTile;

    if (invalidTriangles.size() == 0)
        {
        status = SUCCESS;
        }
    else
        {
        m_totalNbWrongTiles++;

        if (m_outputIncorrectTriangles)
            {
            vector<FaceWithProperties>::const_iterator invalidTriIter(invalidTriangles.begin());
            vector<FaceWithProperties>::const_iterator invalidTriIterEnd(invalidTriangles.end());

            std::vector<DPoint3d> lineStringPoints;

            const double uorPerMeter = ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP());//dgnModel_getUorPerMeter(ACTIVEMODEL);//mdlModelRef_getUorPerMeter(ACTIVEMODEL);

            Transform uorToMeter;
            bsiTransform_initFromRowValues(&uorToMeter, uorPerMeter, 0.0, 0.0, 0.0, 0.0, uorPerMeter, 0.0, 0.0, 0.0, 0.0, uorPerMeter, 0.0);

            while (invalidTriIter != invalidTriIterEnd)
                {

                // NEEDS_WOKR_SM : comment powerplatform dependance. Need to find a way to fix it
//                MSElement       lineElm;
//                MSElementDescrP lineElmdscrP;

                lineStringPoints.clear();

                IBcDTMMeshFacePtr pMeshFace((invalidTriIter)->GetFace());

                lineStringPoints.push_back(pMeshFace->GetCoordinates(0));
                lineStringPoints.push_back(pMeshFace->GetCoordinates(1));
                lineStringPoints.push_back(pMeshFace->GetCoordinates(2));
                lineStringPoints.push_back(pMeshFace->GetCoordinates(0));

                bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &lineStringPoints[0], (int)lineStringPoints.size());

//                mdlLineString_create(&lineElm, NULL, &lineStringPoints[0], (int)lineStringPoints.size());

//                mdlElmdscr_new (&lineElmdscrP, NULL, &lineElm);


/*                UInt32 color = 4;
                UInt32 weight = 5;
                Int32 style = 0;*/

//                EditElementHandle lineElement(lineElmdscrP, true, true);

//                mdlElement_setSymbology(lineElement.GetElementP(), &color, &weight, &style);

//                lineElement.AddToModel();

/*                invalidTriIter++;
                }
            }

        if (s_displayIncorrectVertex)
            {
//MST TBD - Deactivated for now
#if 0
            //MST TBD - Could use a set for optimization
            vector<vector<DPoint3d>> pointInMemIndexed;

            pointInMemIndexed.resize(s_nbXbins * s_nbYbins);

            vector<vector<FaceWithProperties>>::const_iterator faceBinIter(m_memDtmIndexedFaces.begin());
            vector<vector<FaceWithProperties>>::const_iterator faceBinIterEnd(m_memDtmIndexedFaces.end());

            int indXforTol = -1;
            int indYforTol = -1;

            double toleranceForIndexing = DBL_COMPARE_TOL * s_tolScaleForIndexing;

            while (faceBinIter != faceBinIterEnd)
                {
                vector<FaceWithProperties>::const_iterator faceIter(faceBinIter->begin());
                vector<FaceWithProperties>::const_iterator faceIterEnd(faceBinIter->end());

                while (faceIter != faceIterEnd)
                    {
                    for (size_t coordInd = 0; coordInd < 3; coordInd++)
                        {
                        DPoint3d ptToAdd = faceIter->GetFace()->GetCoordinates(coordInd);

                        int indX = (int)((ptToAdd.x -  minRange.x) / m_indexStepX);
                        indX = max(0, min(indX, s_nbXbins - 1));

                        indXforTol = (int)((ptToAdd.x + toleranceForIndexing -  minRange.x) / m_indexStepX);
                        indXforTol = max(0, min(indXforTol, s_nbXbins - 1));

                        if (indXforTol == indX)
                            {
                            indXforTol = (int)((ptToAdd.x - toleranceForIndexing -  minRange.x) / m_indexStepX);
                            indXforTol = max(0, min(indXforTol, s_nbXbins - 1));

                            if (indXforTol == indX)
                                {
                                indXforTol = -1;
                                }
                            }
                #ifndef NDEBUG
                        else
                            {
                            int indXforTolTemp = (int)((ptToAdd.x - toleranceForIndexing -  minRange.x) / m_indexStepX);
                            indXforTolTemp = max(0, min(indXforTolTemp, s_nbXbins - 1));
                            assert(indXforTolTemp == indXforTol);
                            }
                #endif

                        int indY = (int)((ptToAdd.y -  minRange.y) / m_indexStepY);
                        indY = max(0, min(indY, s_nbYbins - 1));

                        indYforTol = (int)((ptToAdd.y + toleranceForIndexing -  minRange.y) / m_indexStepY);
                        indYforTol = max(0, min(indYforTol, s_nbYbins - 1));

                        if (indYforTol == indY)
                            {
                            indYforTol = (int)((ptToAdd.y - toleranceForIndexing -  minRange.y) / m_indexStepY);
                            indYforTol = max(0, min(indYforTol, s_nbYbins - 1));

                            if (indYforTol == indY)
                                {
                                indYforTol = -1;
                                }
                            }
                #ifndef NDEBUG
                        else
                            {
                            int indYforTolTemp = (int)((ptToAdd.y - toleranceForIndexing -  minRange.y) / m_indexStepY);
                            indYforTolTemp = max(0, min(indYforTolTemp, s_nbYbins - 1));
                            assert(indYforTolTemp == indYforTol);
                            }
                #endif

                        pointInMemIndexed[indX + s_nbXbins * indY].push_back(ptToAdd);

                        if (indXforTol != -1)
                            {
                            pointInMemIndexed[indXforTol + s_nbXbins * indY].push_back(ptToAdd);

                            if (indYforTol != -1)
                                {
                                pointInMemIndexed[indX + s_nbXbins * indYforTol].push_back(ptToAdd);
                                pointInMemIndexed[indXforTol + s_nbXbins * indYforTol].push_back(ptToAdd);
                                }
                            }
                        else
                        if (indYforTol != -1)
                            {
                            pointInMemIndexed[indX + s_nbXbins * indYforTol].push_back(ptToAdd);
                            }
                        }

                    faceIter++;
                    }

                faceBinIter++;
                }

            vector<DPoint3d> invalidPoints;

            for (int faceInTileInd = 0; faceInTileInd < (int)facesInTile.size(); faceInTileInd++)
                {
                for (size_t coordInd = 0; coordInd < 3; coordInd++)
                    {
                    DPoint3d ptToCheck = facesInTile[faceInTileInd].GetFace()->GetCoordinates(coordInd);

                    int indX = (int)((ptToCheck.x -  minRange.x) / m_indexStepX);
                    indX = max(0, min(indX, s_nbXbins - 1));

                    int indY = (int)((ptToCheck.y -  minRange.y) / m_indexStepY);
                    indY = max(0, min(indY, s_nbYbins - 1));

                    vector<DPoint3d>::const_iterator ptIter(pointInMemIndexed[indX + s_nbXbins * indY].begin());
                    vector<DPoint3d>::const_iterator ptIterEnd(pointInMemIndexed[indX + s_nbXbins * indY].end());

                    while (ptIter != ptIterEnd)
                        {
                        if ((fabs(ptIter->x - ptToCheck.x) <= DBL_COMPARE_TOL) &&
                            (fabs(ptIter->y - ptToCheck.y) <= DBL_COMPARE_TOL))
                            {
                            break;
                            }

                        ptIter++;
                        }

                    if (ptIter == ptIterEnd)
                        {
                        invalidPoints.push_back(ptToCheck);
                        }
                    }
                }

            vector<DPoint3d>::const_iterator ptIter(invalidPoints.begin());
            vector<DPoint3d>::const_iterator ptIterEnd(invalidPoints.end());

            std::vector<DPoint3d> lineStringPoints;

            const double uorPerMeter = mdlModelRef_getUorPerMeter(ACTIVEMODEL);

            Transform uorToMeter;
            bsiTransform_initFromRowValues(&uorToMeter, uorPerMeter, 0.0, 0.0, 0.0, 0.0, uorPerMeter, 0.0, 0.0, 0.0, 0.0, uorPerMeter, 0.0);

            while (ptIter != ptIterEnd)
                {
                MSElement       lineElm;
                MSElementDescrP lineElmdscrP;

                lineStringPoints.clear();

                lineStringPoints.push_back(*ptIter);
                lineStringPoints.push_back(*ptIter);

                bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &lineStringPoints[0], lineStringPoints.size());

                mdlLineString_create(&lineElm, NULL, &lineStringPoints[0], lineStringPoints.size());

                mdlElmdscr_new (&lineElmdscrP, NULL, &lineElm);

                EditElementHandle lineElement(lineElmdscrP, true, true);

                UInt32 color = 4;
                UInt32 weight = 5;
                Int32 style = 0;

                mdlElement_setSymbology(lineElement.GetElementP(), &color, &weight, &style);

                lineElement.AddToModel(ACTIVEMODEL);

                ptIter++;
                }
#endif
            }
        }

    return status;
    }

int TiledTriangulatorValidator::_SetOuputInActiveModel(bool outputIncorrectTriangles)
    {
    m_outputIncorrectTriangles = outputIncorrectTriangles;
    return SUCCESS;
    }

//END_GEODTMAPP_NAMESPACE
#endif