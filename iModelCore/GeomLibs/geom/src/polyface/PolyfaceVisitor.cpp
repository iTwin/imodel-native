/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceVisitor.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

DPoint3dCP                          PolyfaceVisitor::GetPointCP () const                { return m_point.GetCP(); }
DVec3dCP                            PolyfaceVisitor::GetNormalCP () const               { return m_normal.GetCP(); }
DPoint2dCP                          PolyfaceVisitor::GetParamCP () const                { return m_param.GetCP(); }
uint32_t const*                     PolyfaceVisitor::GetIntColorCP () const             { return (uint32_t const*)m_intColor.GetCP(); }
FacetFaceDataCP                     PolyfaceVisitor::GetFaceDataCP () const             { return m_faceData.GetCP(); }
PolyfaceAuxDataCPtr                 PolyfaceVisitor::GetAuxDataCP() const               { return _GetAuxDataCP(); }

BlockedVectorDPoint3dR              PolyfaceVisitor::Point ()                           { return m_point; }
BlockedVectorDPoint2dR              PolyfaceVisitor::Param ()                           { return m_param; }
BlockedVectorDVec3dR                PolyfaceVisitor::Normal ()                          { return m_normal; }
BlockedVectorUInt32R                PolyfaceVisitor::IntColor ()                        { return m_intColor; }

BlockedVectorIntR                   PolyfaceVisitor::ClientPointIndex  ()               { return m_pointIndex; }
BlockedVectorIntR                   PolyfaceVisitor::ClientParamIndex  ()               { return m_paramIndex; }
BlockedVectorIntR                   PolyfaceVisitor::ClientNormalIndex  ()              { return m_normalIndex; }
BlockedVectorIntR                   PolyfaceVisitor::ClientColorIndex  ()               { return m_colorIndex; }
BlockedVectorIntR                   PolyfaceVisitor::ClientFaceIndex ()                 { return m_faceIndex; }

int32_t const*                      PolyfaceVisitor::GetClientPointIndexCP () const     { return m_pointIndex.GetCP(); }
int32_t const*                      PolyfaceVisitor::GetClientParamIndexCP() const      { return m_paramIndex.GetCP(); }
int32_t const*                      PolyfaceVisitor::GetClientNormalIndexCP() const     { return m_normalIndex.GetCP(); }
int32_t const*                      PolyfaceVisitor::GetClientColorIndexCP() const      { return m_colorIndex.GetCP(); }
int32_t const*                      PolyfaceVisitor::GetClientFaceIndexCP() const       { return m_faceIndex.GetCP(); }
int32_t const*                      PolyfaceVisitor::GetClientAuxIndexCP() const        { return m_auxData.IsValid() ? m_auxData->GetIndices().data() : nullptr; }

bool                                PolyfaceVisitor::GetTwoSided() const                { return m_twoSided; }


template<typename TargetType>
bool PushIndexDataFromMesh      // carefully get data by way of in index array.  index array might go through default.
        (
        size_t readIndex,
        BlockedVector<TargetType> &dataDest,        // destination data in visitor.
        BlockedVectorInt &indexDest,              // destination index in visitor
        TargetType const *data, size_t nData,       // contiguous data in source.
        int const *index,                           // index array in source.
        int const *defaultIndex,                  // default index array source.
        size_t nIndex                             // size of index arrays (same size for both index and defaultIndex)
        )
    {
    if (readIndex >= nIndex)
        return false;
    if (data == nullptr)
        return false;

    if (index != nullptr)
        {
        int oneBasedIndex = index[readIndex];
        if (oneBasedIndex == 0)
            return false;
        size_t zeroBasedIndex = (size_t)abs (oneBasedIndex) - 1;
        if (zeroBasedIndex > nData)
            return false;
        dataDest.push_back (data[zeroBasedIndex]);
        indexDest.push_back ((int)zeroBasedIndex);
        return true;
        }
    else if (defaultIndex != nullptr)
        {
        int oneBasedIndex = defaultIndex[readIndex];
        if (oneBasedIndex == 0)
            return false;
        size_t zeroBasedIndex = (size_t)abs (oneBasedIndex) - 1;
        if (zeroBasedIndex > nData)
            return false;
        dataDest.push_back (data[zeroBasedIndex]);
        indexDest.push_back ((int)zeroBasedIndex);
        return true;
        }

    return false;
    }

bool PolyfaceVisitor::_AddVertexByReadIndex (size_t readIndex) {return false;}

bool PolyfaceVisitor::TryGetClientZeroBasedPointIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex, bool &visible)
    {
    if (m_pointIndex.Active () && zeroBasedVisitorIndex >= 0 && zeroBasedVisitorIndex < (int)m_pointIndex.size ())
        {
        zeroBasedIndex = m_pointIndex[zeroBasedVisitorIndex];
        visible = m_visible[zeroBasedVisitorIndex];
        return true;
        }
    visible = false;
    zeroBasedIndex = 0;
    return false;
    }

//! access zero-based normal index for an vertex within the curent face.
bool PolyfaceVisitor::TryGetClientZeroBasedNormalIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex)
    {
    if (m_normalIndex.Active () && zeroBasedVisitorIndex >= 0 && zeroBasedVisitorIndex < (int)m_normalIndex.size ())
        {
        zeroBasedIndex = m_normalIndex[zeroBasedVisitorIndex];
        return true;
        }
    zeroBasedIndex = 0;
    return false;
    }

//! access zero-based param index for an vertex within the curent face.
bool PolyfaceVisitor::TryGetClientZeroBasedParamIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex)
    {
    if (m_paramIndex.Active () && zeroBasedVisitorIndex >= 0 && zeroBasedVisitorIndex < (int)m_paramIndex.size ())
        {
        zeroBasedIndex = m_paramIndex[zeroBasedVisitorIndex];
        return true;
        }
    zeroBasedIndex = 0;
    return false;
    }

//! access zero-based color index for an vertex within the curent face.
bool PolyfaceVisitor::TryGetClientZeroBasedColorIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex)
    {
    if (m_paramIndex.Active () && zeroBasedVisitorIndex >= 0 && zeroBasedVisitorIndex < (int)m_paramIndex.size ())
        {
        zeroBasedIndex = m_paramIndex[zeroBasedVisitorIndex];
        return true;
        }
    zeroBasedIndex = 0;
    return false;
    }

//=======================================================================================
//! @bsiclass
//=======================================================================================
// Polyface visitor that makes a parent with blocked coordinates appear as ZERO-BASED indexed
class _PolyfaceVisitor_BlockedCoordinatesToIndexed : public PolyfaceVisitor
{
protected:
    PolyfaceQueryCR m_parentMesh;
    uint32_t m_nextFaceIndex;
    uint32_t m_currentFaceIndex;
    uint32_t m_numPerFaceInParent;

/*__PUBLISH_SECTION_END__*/
void _Reset () override       { m_nextFaceIndex = m_currentFaceIndex = 0;     }
PolyfaceQueryCR _GetClientPolyfaceQueryCR () const override {return m_parentMesh;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _MoveToFacetByReadIndex (size_t readIndex) override 
    {
    m_nextFaceIndex = (uint32_t)readIndex;
    return _AdvanceToNextFace ();
    }
size_t _GetReadIndex () const override {return (size_t)m_currentFaceIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AdvanceToNextFace () override
    {
    uint32_t i0 = m_nextFaceIndex * m_numPerFaceInParent;
    uint32_t i1 = i0 + m_numPerFaceInParent;
    m_numEdgesThisFace = m_numPerFaceInParent;
    if (i1 <= m_parentMesh.GetPointCount ())
        {
        // Hmm.. what about missing arrays?  Ignore them -- the
        // copiers will stop quietly.
        m_point.ClearAndAppendBlock      (m_parentMesh.GetPointCP (), m_parentMesh.GetPointCount (),   i0, m_numPerFaceInParent, m_numWrap);
        m_pointIndex.AddSequentialBlock (i0, m_numPerFaceInParent, m_numWrap, 0, true);
        m_visible.clear ();
        m_indexPosition.clear ();
        for (size_t i = 0; i < m_numEdgesThisFace + m_numWrap; i++)
            {
            m_visible.push_back (true);
            m_indexPosition.push_back (i0 + i);
            }

        if (m_allData)
            {
            m_normal.ClearAndAppendBlock     (m_parentMesh.GetNormalCP (),          m_parentMesh.GetNormalCount (),     i0, m_numPerFaceInParent, m_numWrap);
            m_param.ClearAndAppendBlock      (m_parentMesh.GetParamCP (),           m_parentMesh.GetParamCount (),      i0, m_numPerFaceInParent, m_numWrap);
            m_intColor.ClearAndAppendBlock   (m_parentMesh.GetIntColorCP (),        m_parentMesh.GetColorCount (),      i0, m_numPerFaceInParent, m_numWrap);

            m_paramIndex.AddSequentialBlock (i0, m_numPerFaceInParent, m_numWrap, 0, true);
            m_normalIndex.AddSequentialBlock (i0, m_numPerFaceInParent, m_numWrap, 0, true);
            m_colorIndex.AddSequentialBlock (i0, m_numPerFaceInParent, m_numWrap, 0, true);
            m_faceIndex.AddSequentialBlock (i0, m_numPerFaceInParent, m_numWrap, 0, true);
            }
        m_currentFaceIndex = m_nextFaceIndex;
        m_nextFaceIndex++;
        return true;
        }
    return false;
    }
public:
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
// Constructor.  This is private, for use by static factory method -- assumes the factory
//    method has fully vetted the caller.
_PolyfaceVisitor_BlockedCoordinatesToIndexed (PolyfaceQueryCR parentMesh, int numPerFace, bool allData)
    : m_parentMesh(parentMesh)
    {
    SetNumPerFace (m_parentMesh.GetNumPerFace ());
    SetTwoSided (m_parentMesh.GetTwoSided ());
    SetMeshStyle (m_parentMesh.GetMeshStyle ());
    m_numPerFaceInParent = numPerFace;
    m_allData = allData;
    m_pointIndex.SetActive (true);
    Reset ();
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template<typename T>
static void AppendIndexed (bvector<T> &data,
    bvector<int> &dataIndices,
    T const *source,
    size_t sourceCount,
    size_t const *indices, size_t n)
    {
    for (size_t i = 0; i < n; i++)
        {
        size_t sourceIndex = indices[i];
        data.push_back (source[sourceIndex]);
        dataIndices.push_back ((int)sourceIndex);
        }
    }

//=======================================================================================
//! @bsiclass
//=======================================================================================
// Polyface visitor that makes a parent with quad grid appear as ZERO-BASED indexed
class _PolyfaceVisitor_QuadGridToIndexed : public PolyfaceVisitor
{
protected:
    PolyfaceQueryCR m_parentMesh;
    uint32_t m_nextFaceIndex;
    uint32_t m_currentFaceIndex;
    bool   m_triangulate;

/*__PUBLISH_SECTION_END__*/
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Reset () override
    {
    m_nextFaceIndex = m_currentFaceIndex = 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceQueryCR _GetClientPolyfaceQueryCR () const override {return m_parentMesh;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _MoveToFacetByReadIndex (size_t readIndex) override 
    {
    m_nextFaceIndex = (uint32_t)readIndex;
    return _AdvanceToNextFace ();
    }
size_t _GetReadIndex () const override {return (size_t)m_currentFaceIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AdvanceToNextFace () override
    {
    size_t numVertexPerRow = GetNumPerRow ();
    size_t numFacePerRow   = numVertexPerRow - 1;
    size_t quadIndex = m_nextFaceIndex;
    size_t triangleSelect = 0;
    if (m_triangulate)
        {
        quadIndex = m_nextFaceIndex >> 1;
        triangleSelect = m_nextFaceIndex & 0x01;
        }
    size_t rowIndex = quadIndex / numFacePerRow;
    size_t colIndex = quadIndex - rowIndex * numFacePerRow;

    size_t i00 = rowIndex * numVertexPerRow + colIndex;
    size_t i10 = i00 + 1;
    size_t i01 = i00 + numVertexPerRow;
    size_t i11 = i01 + 1;
#define MAX_WRAP 10
    size_t indices[4 + MAX_WRAP];
    
    if (m_triangulate)
        {
        m_numEdgesThisFace = 3;
        if (triangleSelect == 0)
            {
            indices[0] = i00;
            indices[1] = i10;
            indices[2] = i01;
            }
        else
            {
            indices[0] = i01;
            indices[1] = i10;
            indices[2] = i11;
            }
        }
    else
        {
        m_numEdgesThisFace = 4;
        indices[0] = i00;
        indices[1] = i10;
        indices[2] = i11;
        indices[3] = i01;
        }
    int numWrap = m_numWrap;
    if (numWrap > MAX_WRAP)
        numWrap = MAX_WRAP;
    for (int i = 0; i < numWrap; i++)
        indices[m_numEdgesThisFace++] = indices[i];

    if (i11 <= m_parentMesh.GetPointCount ())
        {
        m_point.clear ();
        m_pointIndex.clear ();
        m_normal.clear ();
        m_normalIndex.clear ();
        m_param.clear();
        m_paramIndex.clear ();
        m_visible.clear ();
        m_indexPosition.clear ();
        m_colorIndex.clear ();
        m_faceIndex.clear();
        m_intColor.clear ();
        for (size_t i = 0; i < m_numEdgesThisFace; i++)
            m_visible.push_back (true);

        if (i11 <= m_parentMesh.GetPointCount ())
            AppendIndexed <DPoint3d> (m_point, m_pointIndex, m_parentMesh.GetPointCP (), m_parentMesh.GetPointCount (), indices, m_numEdgesThisFace);
        if (i11 <= m_parentMesh.GetNormalCount ())
            AppendIndexed <DVec3d> (m_normal, m_normalIndex, m_parentMesh.GetNormalCP (), m_parentMesh.GetNormalCount (), indices, m_numEdgesThisFace);
        if (i11 <= m_parentMesh.GetParamCount ())
            AppendIndexed <DPoint2d> (m_param, m_paramIndex, m_parentMesh.GetParamCP (), m_parentMesh.GetParamCount (), indices, m_numEdgesThisFace);
        if (i11 <= m_parentMesh.GetFaceCount ())
            AppendIndexed <FacetFaceData> (m_faceData, m_faceIndex, m_parentMesh.GetFaceDataCP (), m_parentMesh.GetFaceCount (), indices, m_numEdgesThisFace);
        if (i11 <= m_parentMesh.GetColorCount () && m_parentMesh.GetIntColorCP () != NULL)
            AppendIndexed <uint32_t> (m_intColor, m_colorIndex, m_parentMesh.GetIntColorCP (), m_parentMesh.GetColorCount (), indices, m_numEdgesThisFace);

        m_numEdgesThisFace -= numWrap;
        m_currentFaceIndex = m_nextFaceIndex;
        m_nextFaceIndex++;
        return true;
        }
    return false;
    }

public:
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
// Constructor.  This is private, for use by static factory method -- assumes the factory
//    method has fully vetted the caller.
_PolyfaceVisitor_QuadGridToIndexed (PolyfaceQueryCR parentMesh, bool triangulate, bool allData)
    : m_parentMesh(parentMesh),
    m_triangulate (triangulate)
    {
    SetNumPerFace (0);  // We mutate it to indexed.
    SetTwoSided (m_parentMesh.GetTwoSided ());
    SetNumPerRow (parentMesh.GetNumPerRow ());
    SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    m_allData = allData;
    Reset ();
    }
};



// Polyface visitor that makes a parent with blocked coordinates appear as ZERO-BASED indexed
class _PolyfaceVisitor_IndexedPolyfaceQueryToIndexed: public PolyfaceVisitor
{
protected:
    PolyfaceQueryCR m_parentMesh;
    uint32_t m_currentReadIndex;
    uint32_t m_nextReadIndex;
    uint32_t m_numPerFaceInParent;
/*__PUBLISH_SECTION_END__*/
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Reset () override
    {
    m_nextReadIndex = m_currentReadIndex = 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DelimitIndicesInFace (uint32_t readIndex, uint32_t &numThisFace, uint32_t &firstReadIndexThisFace, uint32_t &nextReadIndex)
    {
    int const *pointIndex = m_parentMesh.GetPointIndexCP ();
    size_t arraySize = m_parentMesh.GetPointIndexCount ();
    if (readIndex >= arraySize)
        return false;

    numThisFace = 0;
    if (m_numPerFaceInParent > 1)
        {
        firstReadIndexThisFace = readIndex; // hm... what if there are leading zeros?
        while (numThisFace < m_numPerFaceInParent
                && readIndex + numThisFace < arraySize
                && pointIndex[readIndex + numThisFace] != 0)
            numThisFace++;
        nextReadIndex = readIndex + m_numPerFaceInParent;
        }
    else
        {
        // skip over null faces?
        firstReadIndexThisFace = readIndex;
        while (firstReadIndexThisFace < arraySize && pointIndex[firstReadIndexThisFace] == 0)
            firstReadIndexThisFace++;
        
        while (  firstReadIndexThisFace + numThisFace < arraySize
              && pointIndex[firstReadIndexThisFace + numThisFace] != 0)
            numThisFace++;
        nextReadIndex = firstReadIndexThisFace + numThisFace + 1;
        }

    //assert (numThisFace > 0); // This can occur for a malformed index array (successive terminators) is present.

    return numThisFace > 0;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceQueryCR _GetClientPolyfaceQueryCR () const override {return m_parentMesh;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _MoveToFacetByReadIndex (size_t readIndex) override 
    {
    m_nextReadIndex = (uint32_t)readIndex;
    return _AdvanceToNextFace ();
    }
size_t _GetReadIndex () const override {return m_currentReadIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AdvanceToNextFace () override
    {
    uint32_t i0, i2;
    if (DelimitIndicesInFace (m_nextReadIndex, m_numEdgesThisFace, i0, i2))
        {
        m_nextReadIndex = i0; // skip over duplicate nulls !!!
        m_point.ClearAndAppendByOneBasedIndices (m_pointIndex,      NULL,
                m_parentMesh.GetPointCP (), m_parentMesh.GetPointCount (),
                m_parentMesh.GetPointIndexCP (), m_parentMesh.GetPointIndexCount (),
                i0, m_numEdgesThisFace, m_numWrap);
        m_visible.clear ();
        m_indexPosition.clear ();
        // Disaster if this happens??
        if (m_pointIndex.size () < m_numEdgesThisFace)
            return false;
        for (size_t i = 0; i < m_numEdgesThisFace; i++)
            {
            m_visible.push_back (m_parentMesh.GetPointIndexCP()[m_nextReadIndex + i] > 0);
            m_indexPosition.push_back (m_nextReadIndex + i);
            }
            
        for (size_t i1 = 0; i1 < m_numWrap; i1++)
            {
            int i0 = i1 % m_numEdgesThisFace;
            m_visible.push_back (m_parentMesh.GetPointIndexCP()[m_nextReadIndex + i0] > 0);
            m_indexPosition.push_back (m_nextReadIndex + i0);            
            }
            

        if (m_allData)
            {

            // normals and params default to point index if neededed (Do you believe in the tooth fairy???)
            if (m_parentMesh.GetNormalIndexCP () == NULL && m_parentMesh.GetNormalCount () == m_parentMesh.GetPointCount ())
                m_normal.ClearAndAppendByOneBasedIndices (m_normalIndex,    NULL,
                    m_parentMesh.GetNormalCP (), m_parentMesh.GetNormalCount (),
                    m_parentMesh.GetPointIndexCP (), m_parentMesh.GetPointIndexCount (),
                    i0, m_numEdgesThisFace, m_numWrap);
            else
                m_normal.ClearAndAppendByOneBasedIndices (m_normalIndex,    NULL,
                    m_parentMesh.GetNormalCP (), m_parentMesh.GetNormalCount (),
                    m_parentMesh.GetNormalIndexCP (), m_parentMesh.GetPointIndexCount (),
                    i0, m_numEdgesThisFace, m_numWrap);


            if (m_parentMesh.GetParamIndexCP () == NULL && m_parentMesh.GetParamCount () == m_parentMesh.GetPointCount ())
                m_param.ClearAndAppendByOneBasedIndices (m_paramIndex,    NULL,
                    m_parentMesh.GetParamCP (), m_parentMesh.GetParamCount (),
                    m_parentMesh.GetPointIndexCP (), m_parentMesh.GetPointIndexCount (),
                    i0, m_numEdgesThisFace, m_numWrap);
            else
                m_param.ClearAndAppendByOneBasedIndices (m_paramIndex,      NULL,
                    m_parentMesh.GetParamCP (), m_parentMesh.GetParamCount (),
                    m_parentMesh.GetParamIndexCP (), m_parentMesh.GetPointIndexCount (),
                   i0, m_numEdgesThisFace, m_numWrap);


            m_faceData.ClearAndAppendByOneBasedIndices (m_faceIndex,      NULL,
                    m_parentMesh.GetFaceDataCP (), m_parentMesh.GetFaceCount (),
                    m_parentMesh.GetFaceIndexCP (), m_parentMesh.GetFaceIndexCount (),
                   i0, m_numEdgesThisFace, m_numWrap);

            if (m_parentMesh.GetIntColorCP () != NULL)
                m_intColor.ClearAndAppendByOneBasedIndices (m_colorIndex,   NULL,
                    m_parentMesh.GetIntColorCP (), m_parentMesh.GetColorCount (),
                    m_parentMesh.GetColorIndexCP (), m_parentMesh.GetPointIndexCount (),
                    i0, m_numEdgesThisFace, m_numWrap);

            if (m_auxData.IsValid())
                m_auxData->AdvanceVisitorToNextFace(*m_parentMesh.GetAuxDataCP(), i0, m_numEdgesThisFace, m_numWrap);
            }
        m_currentReadIndex = m_nextReadIndex;
        m_nextReadIndex = i2;
        return true;
        }

    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AddVertexByReadIndex (size_t readIndex) override
    {
    size_t numIndex = m_parentMesh.GetPointIndexCount ();
    if (readIndex >= numIndex)
        return false;

    size_t numPoint = m_parentMesh.GetPointCount ();
    int oneBasedPointIndex = m_parentMesh.GetPointIndexCP ()[readIndex];
    size_t zeroBasedPointIndex = (size_t)abs (oneBasedPointIndex) - 1;
    if (oneBasedPointIndex == 0)
        return false;
    if (zeroBasedPointIndex >= numPoint)
        return false;

    m_point.push_back (m_parentMesh.GetPointCP ()[zeroBasedPointIndex]);
    m_pointIndex.push_back ((int)zeroBasedPointIndex);

    m_visible.push_back (oneBasedPointIndex > 0);
    m_indexPosition.push_back (readIndex);

    if (m_allData)
        {
        if (m_normal.Active ())
            if (!(PushIndexDataFromMesh<DVec3d> (readIndex,
                m_normal,
                m_normalIndex,
                m_parentMesh.GetNormalCP (),
                m_parentMesh.GetNormalCount (),
                m_parentMesh.GetNormalIndexCP (),
                m_parentMesh.GetPointIndexCP (), 
                numIndex
                )))
                return false;

        if (m_param.Active ())
            if (!(PushIndexDataFromMesh<DPoint2d> (readIndex,
                m_param,
                m_paramIndex,
                m_parentMesh.GetParamCP (),
                m_parentMesh.GetParamCount (),
                m_parentMesh.GetParamIndexCP (),
                m_parentMesh.GetPointIndexCP (), 
                numIndex
                )))
                return false;

        if (m_intColor.Active ())
            if (!(PushIndexDataFromMesh<uint32_t> (readIndex,
                m_intColor,
                m_colorIndex,
                m_parentMesh.GetIntColorCP (),
                m_parentMesh.GetColorCount (),
                m_parentMesh.GetColorIndexCP (),
                m_parentMesh.GetPointIndexCP (), 
                numIndex
                )))
                return false;
        }
    m_numEdgesThisFace++;
    return true;
    }

public:
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
// Constructor.  This is private, for use by static factory method -- assumes the factory
//    method has fully vetted the caller.
_PolyfaceVisitor_IndexedPolyfaceQueryToIndexed (PolyfaceQueryCR parentMesh, bool allData)
    : m_parentMesh(parentMesh)
    {
    SetNumPerFace (m_parentMesh.GetNumPerFace ());
    SetTwoSided (m_parentMesh.GetTwoSided ());
    SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);//(m_parentMesh.MeshStyle ());
    PolyfaceVectorsP parentVectors = parentMesh._AsPolyfaceVectorsP ();
    if (nullptr != parentVectors)
        CopyAllActiveFlagsFrom (*parentVectors);
    else
        CopyAllActiveFlagsFromQuery (m_parentMesh);

    m_numPerFaceInParent = m_parentMesh.GetNumPerFace ();
    if (m_parentMesh.GetAuxDataCP().IsValid())
        m_auxData = m_parentMesh.GetAuxDataCP()->CreateForVisitor();


    m_allData = allData;
    Reset ();
    }
};




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceVisitor::PushFaceData (PolyfaceVisitor &source, size_t index)
    {
    size_t outIndex = m_point.size ();
    m_point.push_back (source.m_point [index]);
    m_visible.push_back (source.m_visible [index]);
    if (m_normal.Active ())
        m_normal.push_back (source.m_normal[index]);
    if (m_param.Active ())
        m_param.push_back (source.m_param[index]);
    if (m_intColor.Active ())
        m_intColor.push_back (source.m_intColor[index]);
    PushIndexData (source, index);
    return outIndex;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVisitor::PushIndexData (PolyfaceVisitor &source, size_t index)
    {
    if (m_pointIndex.Active ())
        m_pointIndex.push_back (source.m_pointIndex[index]);
    if (m_normalIndex.Active ())
        m_normalIndex.push_back (source.m_normalIndex[index]);
    if (m_paramIndex.Active ())
        m_paramIndex.push_back (source.m_paramIndex[index]);
    if (m_colorIndex.Active ())
        m_colorIndex.push_back (source.m_colorIndex[index]);
    if (m_faceIndex.Active ())
        m_faceIndex.push_back (source.m_faceIndex[index]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVisitor::TrimFaceData (size_t index0, size_t count)
    {
    m_point.Trim (index0, count);
    m_normal.Trim (index0, count);
    m_param.Trim (index0, count);
    m_intColor.Trim (index0, count);
    size_t n = m_visible.size ();
    size_t n1 = 0;
    while (n1 < count && index0 + n1 < n)
        {
        m_visible[n1] = m_visible[index0 + n1];
        n1++;
        }
    m_visible.resize (n1);    
    }


bool const* PolyfaceVisitor::GetVisibleCP () const      { return &m_visible[0]; }
bvector <bool> &PolyfaceVisitor::Visible ()             { return m_visible;}
bvector <size_t> &PolyfaceVisitor::IndexPosition()      { return m_indexPosition;}

// Hidden virtual dispatch for polyface services ...
void PolyfaceVisitor::Reset () {_Reset ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AdvanceToNextFace () {return _AdvanceToNextFace();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
uint32_t PolyfaceVisitor::NumEdgesThisFace() const {return m_numEdgesThisFace;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
uint32_t PolyfaceVisitor::GetNumWrap () const {return m_numWrap;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceVisitor::GetReadIndex () const {return _GetReadIndex ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::MoveToFacetByReadIndex (size_t readIndex) {return _MoveToFacetByReadIndex (readIndex);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVisitor::SetNumWrap (uint32_t numWrap) {m_numWrap = numWrap;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceVisitor::PolyfaceVisitor(uint32_t numWrap) {m_numWrap = numWrap;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceQueryCR PolyfaceVisitor::GetClientPolyfaceQueryCR () const
    {return _GetClientPolyfaceQueryCR();}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVisitor::CopyData (size_t fromIndex, size_t toIndex)
    {
    if (fromIndex != toIndex)
        {
        m_point[toIndex] = m_point[fromIndex];
        m_pointIndex[toIndex] = m_pointIndex[fromIndex];
        m_indexPosition[toIndex] = m_indexPosition[fromIndex];
        m_visible[toIndex] = m_visible[fromIndex];
        if (m_allData)
            {
            m_normal.CopyData (fromIndex, toIndex);
            m_param.CopyData (fromIndex, toIndex);
            m_faceData.CopyData (fromIndex, toIndex);
            m_intColor.CopyData (fromIndex, toIndex);
            m_paramIndex.CopyData (fromIndex, toIndex);
            m_normalIndex.CopyData (fromIndex, toIndex);
            m_faceIndex.CopyData (fromIndex, toIndex);
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVisitor::TrimData (size_t newSize)
    {
    if (newSize < m_point.size ())
        {
        m_point.resize (newSize);
        m_pointIndex.resize (newSize);
        m_indexPosition.resize (newSize);
        m_visible.resize (newSize);
        if (m_allData)
            {
            m_normal.resize (newSize);
            m_param.resize (newSize);
            m_intColor.resize (newSize);
            m_paramIndex.resize (newSize);
            m_normalIndex.resize (newSize);
            m_colorIndex.resize (newSize);
            m_faceIndex.resize (newSize);
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVisitor::CompressClosePoints (double tolerance)
    {
    // Reduce count for trailing duplicates of first point ...
    bvector<DPoint3d>&points = Point();
    double toleranceSquared = tolerance * tolerance;
    size_t tailCount = points.size ();
    for (size_t i = tailCount - 1;
            i > 0 && points[i].Distance (points[0]) < toleranceSquared; i--)
        {
        tailCount--;
        }

    // Copy distinct data forward at interior dupliates ...
    size_t i0 = 0;
    size_t numAccept = 1;
    for (size_t i1 = 1; i1 < tailCount; i1++)
        {
        if (points[i1].DistanceSquared (points[i0]) < toleranceSquared)
            {
            // i1 duplicates i0.  Skip it.
            }
        else
            {
            // i1 is a new distinct point.  Accept it as start of continued search/
            CopyData (i1, numAccept);
            i0 = numAccept++;
            }
        }
    TrimData (numAccept);        
    }






/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceVisitor::PushInterpolatedFaceData (PolyfaceVisitor &source, size_t i0, double fraction, size_t i1, bool suppressVisibility)
    {
    DPoint3d point;
    point.Interpolate (source.m_point[i0], fraction, source.m_point[i1]);
    m_visible.push_back (suppressVisibility ? false : source.m_visible[i0]);
    size_t outIndex = m_point.size ();
    m_point.push_back (point);

    if (m_normal.Active ())
        {
        DVec3d normal;
        normal.Interpolate (source.m_normal[i0], fraction, source.m_normal[i1]);
        normal.Normalize ();
        m_normal.push_back (normal);
        }

    if (m_param.Active ())
        {
        DPoint2d param;
        param.Interpolate (source.m_param[i0], fraction, source.m_param[i1]);
        m_param.push_back (param);        
        }

    if (m_intColor.Active ())
        {
        RgbFactor color;
        RgbFactor color0 = RgbFactor::FromIntColor (source.m_intColor[i0]);
        RgbFactor color1 = RgbFactor::FromIntColor (source.m_intColor[i1]);
        double f0 = 1.0 - fraction;
        color.red   = f0 * color0.red   + fraction * color1.red;
        color.green = f0 * color0.green + fraction * color1.green;
        color.blue  = f0 * color0.blue  + fraction * color1.blue;
        m_intColor.push_back (color.ToIntColor ());
        }

    // Index data carries unchanged ..
    PushIndexData (source, i0);
    return outIndex;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryGetFacetCentroidNormalAndArea (DPoint3dR centroid, DVec3dR normal, double &area) const
    {
    return PolygonOps::CentroidNormalAndArea (&m_point[0], m_numEdgesThisFace, centroid, normal, area);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryGetFacetAreaMomentProducts (DPoint3dCR origin, DMatrix4dR products) const
    {
    return PolygonOps::SecondAreaMomentProducts (m_point, origin, products);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryGetEdgePoint (size_t edgeIndex, double f, DPoint3dR xyz) const
    {
    if (m_numEdgesThisFace < 1)
        return false;
    size_t i0 = (edgeIndex % m_numEdgesThisFace);
    size_t i1 = ((edgeIndex + 1) % m_numEdgesThisFace);
    xyz.Interpolate (m_point[i0], f, m_point [i1]);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2012
+--------------------------------------------------------------------------------------*/
bool   PolyfaceVisitor::TryGetNormalizedParameter (size_t index, DPoint2dR normalizedParam) const
    {
    if (index >= NumEdgesThisFace())
        return false;

    DPoint2dCP        param; 
    FacetFaceDataCP   faceData;

    if (NULL == (param = GetParamCP()) ||
        NULL == (faceData = GetFaceDataCP()))
        return false;

   
    faceData[index].ConvertParamToNormalized (normalizedParam, param[index]);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                     Ray.Bentley     06/2012
+--------------------------------------------------------------------------------------*/
bool   PolyfaceVisitor::TryGetDistanceParameter (size_t index, DPoint2dR distanceParam) const

    {
    if (index >= NumEdgesThisFace())
        return false;

    DPoint2dCP        param; 
    FacetFaceDataCP   faceData;

    if (NULL == (param = GetParamCP()) ||
        NULL == (faceData = GetFaceDataCP()))
        return false;
   
    faceData[index].ConvertParamToDistance (distanceParam, param[index]);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceVisitorPtr PolyfaceVisitor::Attach (PolyfaceQueryCR parentMesh, bool allData)
    {
     if (parentMesh.GetMeshStyle () == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
         {
         return new _PolyfaceVisitor_BlockedCoordinatesToIndexed (parentMesh, 3, allData);
         }
     else if (parentMesh.GetMeshStyle () == MESH_ELM_STYLE_COORDINATE_QUADS)
         {
         return new _PolyfaceVisitor_BlockedCoordinatesToIndexed (parentMesh, 4, allData);
         }
     else if (parentMesh.GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
         {
         return new _PolyfaceVisitor_IndexedPolyfaceQueryToIndexed (parentMesh, allData);
         }
     else if (parentMesh.GetMeshStyle () == MESH_ELM_STYLE_QUAD_GRID)
         {
        return new _PolyfaceVisitor_QuadGridToIndexed (parentMesh, false, allData);
         }
     else if (parentMesh.GetMeshStyle () == MESH_ELM_STYLE_TRIANGLE_GRID)
         {
         return new _PolyfaceVisitor_QuadGridToIndexed (parentMesh, true, allData);
         }
    return NULL;
//    return new _PolyfaceVisitor_IndexedPolyfaceQueryToIndexed (parentMesh, allData);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
PolyfaceVisitorNormalOrientationFilter::PolyfaceVisitorNormalOrientationFilter
(
bool acceptPositive,    //! [in] true to accept facets with positive normal direction
bool acceptNegative,    //! [in] true to accept faces with negative normal direction.
bool acceptSide,        //! [in] true to accept facets with side normal direction.
double normalTolerance  //! [in] tolerance for normal test.
)
    : m_normalTolerance (normalTolerance),
      m_acceptNegative (acceptNegative),
      m_acceptSide (acceptSide),
      m_acceptPositive (acceptPositive)
    {
    m_positiveDirectionVector = DVec3d::From (0,0,1);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitorNormalOrientationFilter::SetPositiveDirectionVector (DVec3dCR vector)
    {
    auto unitVector = vector.ValidatedNormalize ();
    if (unitVector.IsValid ())
        m_positiveDirectionVector = unitVector.Value ();
    return unitVector.IsValid ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitorNormalOrientationFilter::TestFacet (PolyfaceVisitorCR visitor)
    {
    DPoint3d centroid;
    DVec3d   normal;
    double area;
    if (visitor.TryGetFacetCentroidNormalAndArea (centroid, normal, area))
        {
        double a = normal.DotProduct (m_positiveDirectionVector);
        if (fabs (a) <= m_normalTolerance)
            return m_acceptSide;
        if (a < 0.0)
            return m_acceptNegative;
        if (a > 0.0)
            return m_acceptPositive;
        }
    return false;
    }
END_BENTLEY_GEOMETRY_NAMESPACE






#ifdef UNUSED_HANDLED_THROUGH_POLYFACEQUERY
//=======================================================================================
//! @bsiclass
//=======================================================================================
// Polyface visitor that makes a parent with blocked coordinates appear as ZERO-BASED indexed
class _PolyfaceVisitor_IndexedToIndexed: public PolyfaceVisitor
{
protected:
    PolyfaceQueryCR m_parentMesh;
    uint32_t m_nextReadIndex;
    uint32_t m_currentReadIndex;
    uint32_t m_numPerFaceInParent;
/*__PUBLISH_SECTION_END__*/
void _Reset () override
    {
    m_currentReadIndex = 0; // Not valid
    m_nextReadIndex = 0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CountIndicesInFace(uint32_t readIndex, uint32_t &numThisFace, uint32_t &nextReadIndex)
    {
    bvector<int>& pointIndex = m_parentMesh.PointIndex();
    size_t arraySize = pointIndex.size ();
    if (readIndex >= arraySize)
        return false;

    numThisFace = 0;
    if (m_numPerFaceInParent > 1)
        {
        while (numThisFace < m_numPerFaceInParent
                && readIndex + numThisFace < arraySize
                && pointIndex[readIndex + numThisFace] != 0)
            numThisFace++;
        nextReadIndex = readIndex + m_numPerFaceInParent;
        }
    else
        {
        while (  readIndex + numThisFace < arraySize
              && pointIndex[readIndex + numThisFace] != 0)
            numThisFace++;
        nextReadIndex = readIndex + numThisFace + 1;
        }
    return true;
    }

PolyfaceQueryCR _GetClientPolyfaceQueryCR () const override {return m_parentMesh;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _MoveToFacetByReadIndex (size_t readIndex) override 
    {
    m_nextReadIndex = (uint32_t)readIndex;
    return _AdvanceToNextFace ();
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t _GetReadIndex () const override {return m_currentReadIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AdvanceToNextFace () override
    {
    uint32_t i0 = m_nextReadIndex;
    uint32_t i2;
    if (CountIndicesInFace (m_nextReadIndex, m_numEdgesThisFace, i2))
        {
        m_point.ClearAndAppendByOneBasedIndices (m_pointIndex,      NULL, m_parentMesh.Point (),   m_parentMesh.PointIndex (),       i0, m_numEdgesThisFace, m_numWrap);
        m_visible.clear ();
        m_indexPosition.clear ();
        for (size_t i = 0; i < m_numEdgesThisFace; i++)
            {
            m_visible.push_back (m_parentMesh.PointIndex ()[m_nextReadIndex + i] > 0);
            m_indexPosition.push_back (m_nextReadIndex + i);
            }
        for (size_t i = 0; i < m_numWrap; i++)
            {
            m_visible.push_back (m_visible [i]);
            m_indexPosition.push_back (m_indexPosition [i]);
            }
        if (m_allData)
            {
            m_normal.ClearAndAppendByOneBasedIndices (m_normalIndex,      NULL, m_parentMesh.Normal (), m_parentMesh.NormalIndex (),      i0, m_numEdgesThisFace, m_numWrap);
            m_param.ClearAndAppendByOneBasedIndices (m_paramIndex,        NULL, m_parentMesh.Param (),   m_parentMesh.ParamIndex (),       i0, m_numEdgesThisFace, m_numWrap);
            m_faceData.ClearAndAppendByOneBasedIndices (m_faceIndex,      NULL, m_parentMesh.FaceData (),   m_parentMesh.FaceIndex (),       i0, m_numEdgesThisFace, m_numWrap);
            // NEEDS WORK: variant cases for color indexer?
            m_doubleColor.ClearAndAppendByOneBasedIndices (m_colorIndex,  NULL, m_parentMesh.DoubleColor (), m_parentMesh.ColorIndex (),   i0, m_numEdgesThisFace, m_numWrap);
            m_intColor.ClearAndAppendByOneBasedIndices (m_colorIndex,     NULL, m_parentMesh.IntColor (), m_parentMesh.ColorIndex (),   i0, m_numEdgesThisFace, m_numWrap);
            m_colorTable.ClearAndAppendByOneBasedIndices (m_colorIndex,   NULL, m_parentMesh.ColorTable (), m_parentMesh.ColorIndex (),   i0, m_numEdgesThisFace, m_numWrap);
            }
        m_currentReadIndex = m_nextReadIndex;
        m_nextReadIndex = i2;
        return true;
        }

    return false;
    }
public:
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
// Constructor.  This is private, for use by static factory method -- assumes the factory
//    method has fully vetted the caller.
_PolyfaceVisitor_IndexedToIndexed (PolyfaceQueryCR parentMesh, bool allData)
    : m_parentMesh(parentMesh)
    {
    SetNumPerFace (m_parentMesh.GetNumPerFace ());
    SetTwoSided (m_parentMesh.GetTwoSided ());
    SetMeshStyle (m_parentMesh.GetMeshStyle ());
    PolyfaceVectorsP parentVectors = parentMesh._AsPolyfaceVectorsP ();
    if (nullptr != parentVectors)
        CopyAllActiveFlagsFrom (*parentVectors);
    else
        CopyAllActiveFlagsFromQuery (m_parentMesh);
    m_numPerFaceInParent = m_parentMesh.GetNumPerFace ();
    m_allData = allData;
    Reset ();
    }
};


void PolyfaceVisitor::CollectReadIndices (bvector<size_t> &indices)
    {
    size_t savedIndex = GetReadIndex ();
    indices.clear ();
    for (Reset (); AdvanceToNextFace ();)
        {
        indices.push_back (GetReadIndex ());
        }
    MoveToFacetByReadIndex (savedIndex);
    }

#endif
void PolyfaceVisitor::ClearFacet ()
    {
    // Clear the base class ...
    ClearAllArrays ();
    // And additional arrays in the visitor ...
    m_visible.clear ();
    m_indexPosition.clear ();
    m_numEdgesThisFace = 0;
    }

bool PolyfaceVisitor::TryAddVertexByReadIndex (size_t readIndex)
    {
    return _AddVertexByReadIndex (readIndex);
    }

bool PolyfaceVisitor::TryRecomputeNormals ()
    {
    DPoint3d centroid;
    DVec3d normal;
    double area;
    if (m_normal.Active ()
        && PolygonOps::CentroidNormalAndArea (&m_point[0], m_numEdgesThisFace, centroid, normal, area))
        {
        for (size_t i = 0; i < m_normal.size (); i++)
            m_normal[i] = normal;
        return true;
        }
    return false;
    }

