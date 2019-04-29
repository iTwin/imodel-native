/*----------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+----------------------------------------------------------------------*/
#include <map>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct ClipEdgeData
{
int m_vertexA;
int m_vertexB;
int m_planeIndex;
size_t m_succ;
size_t m_pred;
bool m_flag;

ClipEdgeData (int vertexIndexA, int vertexIndexB, int planeIndex);

void SetNeighbors (size_t succ, size_t pred);
};

struct ClipEdgeDataArray : bvector <ClipEdgeData>
{
static bool compare_PlaneIdVertexAVertexB (ClipEdgeData const& edge0, ClipEdgeData const& edge1);
// Find endIndex so {i :: beginIndex <= i < endIndex} have same plane id as beginIndex.
size_t FindEndOfPlaneCluster (size_t beginIndex);

bool IsValid (size_t index);
void SetFlag (size_t beginIndex, size_t endIndex, bool flag);
void SetFlag (size_t index, bool flag);
bool GetFlag (size_t index);

void ExtendChain
    (
    size_t beginIndex,
    size_t endIndex,
    size_t index,
    bvector <size_t>&chainIndices,
    bool includeFinalVertex
    );

void SortByPlaneVertexAVertexB ();
size_t CountVertexA (size_t vertexA, size_t beginIndex, size_t endIndex, size_t &lastMatch);
size_t CountVertexB (size_t vertexB, size_t beginIndex, size_t endIndex, size_t &lastMatch);
bool AnalyzeCutPlane (size_t beginIndex, size_t endIndex, bvector <size_t> &startIndices, bvector<size_t> &endIndices);

};


END_BENTLEY_GEOMETRY_NAMESPACE

