/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//
#include <bsibasegeomPCH.h>
#include <stdio.h>
#include <Bentley/bvector.h>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef struct
    {
    DPoint3d xyz;
    int nextInCluster;
    int originalIndex;
    double sortCoordinate;
    } XYZSortKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool cb_XYZSortKey_LT_XYZSortKey (XYZSortKey const &keyA, XYZSortKey const &keyB)
{
    return keyA.sortCoordinate < keyB.sortCoordinate;
}


/*=================================================================================**//**
* @bsiclass                                                     EarlinLutz      02/2006
+===============+===============+===============+===============+===============+======*/
class XYZSortKeyArray : public bvector <XYZSortKey>
{
public:
void Sort ()
    {
    std::sort (begin (), end (), cb_XYZSortKey_LT_XYZSortKey);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    keysInSameCluster
(
XYZSortKey *pKeyA,
XYZSortKey *pKeyB,
double    abstol,
bool      bXYZ
)
    {
    // Changed from distance test to coordinate compares to avoid bottleneck in tile
    // generation decimation (Scene_3d) -- Ray B.  01/2018
#ifdef DISTANCE_TEST
    double dd = bXYZ ? pKeyA->xyz.DistanceSquared (pKeyB->xyz)
                    : pKeyA->xyz.DistanceSquaredXY (pKeyB->xyz);
    return dd < abstol * abstol;
#else
    return fabs(pKeyA->xyz.x - pKeyB->xyz.x) < abstol &&
           fabs(pKeyA->xyz.y - pKeyB->xyz.y) < abstol &&
           (!bXYZ || fabs(pKeyA->xyz.z - pKeyB->xyz.z) < abstol);
#endif    
    }


/**
* @description Identify clusters of nearly-identical coordinates.
* @param xyzArray                   IN      array of xyz coordinatss.
* @param pClusterArray              OUT     array of NULL-deliminted blocks of nodes within tolerance
* @param packedArray                OUT     optional array to receive single representatives
* @param absTol                     IN      absolute tolerance for xy-coordinate comparison
* @param bReassignXYZ               IN      true to immediately update points to identical coordinates.
* @param bXYZ                       IN      true for 3d, false for xy
* @bsimethod                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
DPoint3dP xyzArray,
size_t numXYZ,
bvector<int> & blockedIndexArray,
bvector<DPoint3d> *packedArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ,
bvector<size_t> *oldToNew
)
    {
    double      epsilon, epsilon2;
    XYZSortKey   sortInfo;
    DVec3d    sortVector;
    DPoint3d    xyz;

    //static double   s_defaultRelTol = 1.0e-6;
    //static double   s_defaultAbsTol = 1.0e-14;

    sortVector.Init ( 0.13363333323, 1.41423, 0.128);
    if (!bXYZ)
        sortVector.z = 0.0;
    sortVector.Normalize ();
    epsilon = absTol;
    epsilon2 = sqrt (3.0) * epsilon;   // l_infinity vertex tol mapped to linear l_2 tol along random vector


    XYZSortKeyArray sortArray = XYZSortKeyArray ();
    blockedIndexArray.clear ();
    if (oldToNew)
        {
        oldToNew->clear ();
        for (size_t i = 0; i < numXYZ; i++)
            oldToNew->push_back (SIZE_MAX);
        }

    for (unsigned int i = 0; i < numXYZ; i++)
        {
        xyz = sortInfo.xyz = xyzArray[i];
        sortInfo.originalIndex = i;
        sortInfo.nextInCluster = -1;
        sortInfo.sortCoordinate = xyz.x * sortVector.x + xyz.y * sortVector.y + xyz.z * sortVector.z;
        sortArray.push_back (sortInfo);
        }

    sortArray.Sort ();

    /* Sorting along the single direction does a good but not complete job of bringing
        geometrically clustered points together in the sort array.
        Do linear search "within epsilon" in the sort order to complete the filtering.
        Push nodeId's back to NULL as nodes are gathered into clusters. */
#define NOT_YET_CLUSTERED(_pSortBuffer_,_index_) (_pSortBuffer_[_index_].nextInCluster == -1)
    size_t nodeCount = sortArray.size ();
    size_t newIndex = 0;
    for (size_t pivotIndex = 0; pivotIndex < nodeCount; pivotIndex++)
        {
        if (NOT_YET_CLUSTERED (sortArray, pivotIndex))
            {
            double testLimit = sortArray[pivotIndex].sortCoordinate + epsilon2;
            /* Link to self as singleton linked list */
            sortArray[pivotIndex].nextInCluster = (int)pivotIndex;
            DPoint3d xyzPivot = sortArray[pivotIndex].xyz;
            blockedIndexArray.push_back (sortArray[pivotIndex].originalIndex);
            if (NULL != packedArray)
                packedArray->push_back (sortArray[pivotIndex].xyz);
            xyz = sortArray[pivotIndex].xyz;
            if (oldToNew != nullptr)
                oldToNew->at((size_t)sortArray[pivotIndex].originalIndex) = newIndex;
            for (size_t testIndex = pivotIndex + 1;
                 testIndex < nodeCount && sortArray[testIndex].sortCoordinate <= testLimit;
                 testIndex++
                 )
                {
                if (NOT_YET_CLUSTERED(sortArray, testIndex))
                    {
                    if (keysInSameCluster (&sortArray[pivotIndex], &sortArray[testIndex], epsilon, bXYZ))
                        {
                        sortArray[testIndex].nextInCluster = sortArray[pivotIndex].nextInCluster;
                        sortArray[pivotIndex].nextInCluster = (int)testIndex;
                        int originalIndex = sortArray[testIndex].originalIndex;
                        blockedIndexArray.push_back (originalIndex);
                        if (oldToNew != nullptr)
                            oldToNew->at((size_t)originalIndex) = newIndex;
                        if (bReassignXYZ)
                            xyzArray[sortArray[testIndex].originalIndex] = xyzPivot;
                        }
                    }
                }
            blockedIndexArray.push_back (-1);
            newIndex++;
            }
        }
    }



/**
* @description Identify clusters of nearly-identical coordinates.
* @param xyzArray                   IN      array of xyz coordinatss.
* @param pClusterArray              OUT     array of NULL-deliminted blocks of nodes within tolerance
* @param absTol                     IN      absolute tolerance for xy-coordinate comparison
* @param bReassignXYZ               IN      true to immediately update points to identical coordinates.
* @param bXYZ                       IN      true for 3d, false for xy
* @bsimethod                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
bvector<DPoint3d> & xyzArray,
bvector<int> & blockedIndexArray,
bvector<DPoint3d> *packedArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ,
bvector<size_t> *oldToNew
)
    {
    blockedIndexArray.clear ();
    if (xyzArray.size () > 0)
        {
        bsiDPoint3dArray_findClusters (&xyzArray[0], xyzArray.size (), blockedIndexArray, packedArray, absTol, bReassignXYZ, bXYZ, oldToNew);
        }
    }

Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
bvector<DPoint3d> & xyzArray,
bvector<int> & blockedIndexArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ
)
    {
    bsiDPoint3dArray_findClusters (xyzArray, blockedIndexArray, nullptr, absTol, bReassignXYZ, bXYZ, nullptr);
    }

Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
DPoint3dP xyzArray,
size_t numXYZ,
bvector<int> & blockedIndexArray,
bvector<DPoint3d> *packedArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ
)
    {
    bsiDPoint3dArray_findClusters (xyzArray, numXYZ, blockedIndexArray, packedArray, absTol, bReassignXYZ, bXYZ, nullptr);
    }

	
END_BENTLEY_GEOMETRY_NAMESPACE
	