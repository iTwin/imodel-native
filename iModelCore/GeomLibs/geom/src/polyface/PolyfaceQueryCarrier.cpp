/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PolyfaceQueryCarrier::PolyfaceQueryCarrier (
    uint32_t numPerFace,
    bool   twoSided,
    size_t indexCount,
    size_t pointCount,      DPoint3dCP pPoint, int32_t const* pPointIndex,
    size_t normalCount,     DVec3dCP  pNormal, int32_t const* pNormalIndex,
    size_t paramCount,      DPoint2dCP pParam, int32_t const* pParamIndex,
    size_t colorCount,      int32_t const* pColorIndex, uint32_t const* pIntColor, void const* unused,
    uint32_t                meshStyle,
    uint32_t                numPerRow,
    uint32_t                expectedClosure
    )
    {
    m_numPerFace        = numPerFace;
    m_pointPtr          = pPoint;
    m_normalPtr         = pNormal;
    m_paramPtr          = pParam;
    m_intColorPtr       = pIntColor;
    m_pointIndexPtr     = pPointIndex;
    m_normalIndexPtr    = pNormalIndex;
    m_paramIndexPtr     = pParamIndex;
    m_colorIndexPtr     = pColorIndex;
    m_twoSided          = twoSided;
    m_pointCount        = pointCount;
    m_paramCount        = paramCount;
    m_normalCount       = normalCount;
    m_colorCount        = colorCount;
    m_indexCount        = indexCount;
    m_meshStyle         = meshStyle;
    m_numPerRow         = numPerRow;
    m_faceCount         = 0;
    m_edgeChainCount    = 0;
    m_faceDataPtr       = NULL;
    m_faceIndexPtr      = NULL;
    m_edgeChainsPtr     = NULL;
    m_expectedClosure   = expectedClosure;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void PolyfaceQueryCarrier::SetFacetFaceData (FacetFaceDataCP facetFaceData, size_t n)
    {
    m_faceDataPtr = facetFaceData;
    m_faceCount = n;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void PolyfaceQueryCarrier::SetFaceIndex (int32_t const *indexArray)
    {
    m_faceIndexPtr = indexArray;
    }
 
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void PolyfaceQueryCarrier::RemoveNormals()
    {
    m_normalPtr = nullptr;
    m_normalIndexPtr = nullptr;
    m_normalCount = 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void PolyfaceQueryCarrier::RemoveParams()
    {
    m_paramPtr = nullptr;
    m_paramIndexPtr = nullptr;
    m_paramCount = 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void PolyfaceQueryCarrier::RemoveColors()
    {
    m_intColorPtr = nullptr;
    m_colorIndexPtr = nullptr;
    m_colorCount = 0;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
