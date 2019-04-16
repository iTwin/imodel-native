/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct SectorIndices
{
    // (unique) id to keep track of original order of sectors ...
    size_t counter;
    // Existing indices in the polyface data ...
    int vertex;
    int normal;
    int param;
    int color;
    int face;
    // "new" index assigned after lexical sort of existing indices ...
    int newIndex;
    // bit mask of boolean properties ...
    uint32_t flags;
    SectorIndices (size_t counterA, int defaultIndex = -1)
        {
        counter = counterA;
        vertex = normal = param = color = face = newIndex = defaultIndex;
        flags = 0;
        }
// bit masks for accessing flags ...        
static const uint32_t s_visibleBit = 0x01;
static const uint32_t s_lastInFaceBit = 0x02;

void SetFlags (bool value, uint32_t mask)
    {
    if (value)
        flags |= mask;
    else
        flags &= ~mask;
    }
void SetVisible (bool value) {SetFlags (value, s_visibleBit);}
void SetLastInFace (bool value) { SetFlags (value, s_lastInFaceBit);}

bool IsVisible    () { return 0 != (flags & s_visibleBit);}
bool IsLastInFace () { return 0 != (flags & s_lastInFaceBit);}

int GetOneBasedPointIndex  ()   { return IsVisible () ? (vertex + 1) : - (vertex + 1);}
int GetOneBasedNormalIndex ()   { return normal + 1;}
int GetOneBasedParamIndex  ()   { return param + 1;}
int GetOneBasedColorIndex  ()   { return color + 1;}
int GetOneBasedFaceIndex   ()   { return face + 1;}
int GetOneBasedNewIndexWithVisibilitySign  ()   { return IsVisible () ? (newIndex + 1) : - (newIndex + 1);}
int GetOneBasedNewIndex  ()   {return newIndex + 1;}
};

static bool cb_compareByIndices (SectorIndices const &dataA, SectorIndices const &dataB)
    {
    if (dataA.vertex < dataB.vertex)
        return true;
    if (dataB.vertex < dataA.vertex)
        return false;

    if (dataA.normal < dataB.normal)
        return true;
    if (dataB.normal < dataA.normal)
        return false;
     
    if (dataA.param < dataB.param)
        return true;
    if (dataB.param < dataA.param)
        return false;

    if (dataA.color < dataB.color)
        return true;
    if (dataB.color < dataA.color)
        return false;

    if (dataA.face < dataB.face)
        return true;
    if (dataB.face < dataA.face)
        return false;
        
    return false;        
    }

static bool cb_compareByCounter (SectorIndices const &dataA, SectorIndices const &dataB)
    {
    return dataA.counter < dataB.counter;
    }
    
struct IndexUnificationContext
{
bvector<SectorIndices> m_indices;
PolyfaceQueryCR m_source;
PolyfaceHeaderR m_dest;

IndexUnificationContext (PolyfaceQueryCR source, PolyfaceHeaderR dest)
    : m_source (source), m_dest (dest)
    {
    dest.Normal ().SetActive (source.GetNormalCount () > 0);
    dest.Param ().SetActive (source.GetParamCount () > 0);
    dest.IntColor ().SetActive (source.GetColorCount () > 0  && source.GetIntColorCP () != NULL);
    dest.NormalIndex ().SetActive (source.GetNormalCount () > 0);
    dest.ParamIndex ().SetActive (source.GetParamCount () > 0);
    dest.ColorIndex ().SetActive (source.GetColorCount () > 0  && source.GetIntColorCP () != NULL);
    }
    
void BuildSortData ()
    {
    int defaultIndex = -1;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (m_source);
    m_indices.clear();
    size_t counter = 0;

    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t numEdgesThisFace = (size_t)visitor->NumEdgesThisFace ();
        for (size_t i = 0; i < numEdgesThisFace; i++)
            {
            SectorIndices indices (counter++, defaultIndex);
            int data;
            bool visible;
            if (visitor->TryGetClientZeroBasedPointIndex ((int)i, data, visible))
                {
                indices.vertex = data;
                indices.SetVisible (visible);
                }
            if (visitor->TryGetClientZeroBasedNormalIndex ((int)i, data))
                indices.normal = data;
            if (visitor->TryGetClientZeroBasedParamIndex ((int)i, data))
                indices.param = data;
            if (visitor->TryGetClientZeroBasedColorIndex ((int)i, data))
                indices.color = data;
//            if (visitor->TryGetClientZeroBasedFaceIndex ((int)i, data))
//                indices.face = data;

            m_indices.push_back (indices);
            }
        m_indices.back ().SetLastInFace (true);            
        }
    }

size_t CreateNewSector (SectorIndices const &oldIndices)
    {
    std::sort (m_indices.begin (), m_indices.end (), cb_compareByIndices);
    size_t newIndex = m_dest.Point ().size ();
    m_dest.Point ().push_back (m_source.GetPointCP() [oldIndices.vertex]);
    if (oldIndices.normal >= 0)
        m_dest.Normal ().push_back (m_source.GetNormalCP()[(size_t)oldIndices.normal]);
    if (oldIndices.param>= 0)
        m_dest.Param ().push_back (m_source.GetParamCP()[(size_t)oldIndices.param]);
    if (oldIndices.color >= 0)
        {
        if (NULL != m_source.GetIntColorCP ())
            m_dest.IntColor ().push_back (m_source.GetIntColorCP()[(size_t)oldIndices.color]);
        }
    //if (oldIndices.face >= 0)
    //    m_dest.FaceIndex ().push_back (m_source.FaceIndex()[(size_t)oldIndices.face]);                        
    return newIndex;        
    }

void AssignNewIndicesAndCoordinateArrays ()
    {
    if (m_indices.size () > 0)
        {
        SectorIndices oldIndices (0, -2);  // A unique set of indices !!
        size_t currentSectorIndex = SIZE_MAX;
        for (size_t i = 0; i < m_indices.size (); i++)
            {
            if (cb_compareByIndices (oldIndices, m_indices[i]))
                {
                currentSectorIndex = CreateNewSector (m_indices[i]);
                oldIndices = m_indices[i];
                }
            m_indices[i].newIndex = (int)currentSectorIndex;
            }
        }
    }

void BuildNewIndices ()
    {
    std::sort (m_indices.begin (), m_indices.end (), cb_compareByCounter);
    for (size_t i = 0; i < m_indices.size (); i++)
        {
        int newIndex = m_indices[i].GetOneBasedNewIndex ();
        int newIndexWithSign = m_indices[i].GetOneBasedNewIndexWithVisibilitySign ();
        if (m_indices[i].vertex >= 0)
            m_dest.PointIndex ().push_back (newIndexWithSign);
        if (m_indices[i].normal >= 0)
            m_dest.NormalIndex ().push_back (newIndex);
        if (m_indices[i].param >= 0)
            m_dest.ParamIndex ().push_back (newIndex);
        if (m_indices[i].color >= 0)
            m_dest.ColorIndex ().push_back (newIndex);
        //if (m_indices[i].face >= 0)
        //    m_dest.Face.push_back (m_indices[i].GetOneBasedColorIndex ());

        if (m_indices[i].IsLastInFace ())
            {
            if (m_indices[i].vertex >= 0)
                m_dest.PointIndex ().push_back (0);
            if (m_indices[i].normal >= 0)
                m_dest.NormalIndex ().push_back (0);
            if (m_indices[i].param >= 0)
                m_dest.ParamIndex ().push_back (0);
            if (m_indices[i].color >= 0)
                m_dest.ColorIndex ().push_back (0);
            //if (m_indices[i].face >= 0)
            //    m_dest.Face.push_back (0);
            }
        }
    }
        
void Go ()
    {
    BuildSortData ();
    AssignNewIndicesAndCoordinateArrays ();
    BuildNewIndices ();
    }
    
};

PolyfaceHeaderPtr PolyfaceHeader::CreateUnifiedIndexMesh (PolyfaceQueryCR source)
    {
    PolyfaceHeaderPtr dest = PolyfaceHeader::CreateVariableSizeIndexed ();
    IndexUnificationContext context (source, *dest);
    context.Go ();
    return dest;
    }
    
END_BENTLEY_GEOMETRY_NAMESPACE