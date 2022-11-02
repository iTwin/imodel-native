/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


bool UnionFind::IsValidClusterIndex (bvector <size_t> &clusterData, size_t index)
    {
    return index < clusterData.size ();
    }

size_t UnionFind::AddClusters (bvector<size_t> &clusterData, size_t numAdd)
    {
    size_t n0 = clusterData.size ();
    for (size_t i = 0; i < numAdd; i++)
        clusterData.push_back (n0 + i);
    return n0 + numAdd;
    }

size_t UnionFind::NewClusterIndex (bvector<size_t> &clusterData)
    {
    size_t value = clusterData.size ();
    clusterData.push_back (value);
    return value;
    }

size_t UnionFind::FindClusterRoot (bvector<size_t> &clusterData, size_t start)
    {
    if (!IsValidClusterIndex (clusterData, start))
        return start;
    // Sweep once to find root ...
    // currIndex is always valid.
    // nextIndex is suspect
    size_t nextIndex = clusterData[start];
    size_t currIndex = start;
    while (IsValidClusterIndex (clusterData, nextIndex) && nextIndex != currIndex)
        {
        currIndex = nextIndex;
        nextIndex = clusterData[currIndex];
        }
    // Sweep again to fixup ...
    size_t rootIndex = currIndex;
    for (currIndex = start;currIndex != rootIndex;currIndex = nextIndex)
        {
        nextIndex = clusterData[currIndex];
        clusterData[currIndex] = rootIndex;
        }
    return rootIndex;
    }

size_t UnionFind::MergeClusters (bvector<size_t> &clusterData, size_t index0, size_t index1)
    {
    size_t root0 = FindClusterRoot (clusterData, index0);
    size_t root1 = FindClusterRoot (clusterData, index1);
    if (root0 == root1)
        return root0;
    clusterData[root0] = root1;
    size_t root2 = FindClusterRoot (clusterData, index0);
    return root2;   // which is really root1..
    }
END_BENTLEY_GEOMETRY_NAMESPACE
