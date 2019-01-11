/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceVectors.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

size_t                  PolyfaceVectors::_GetPointCount       ()     const  {return m_point.size ();}
size_t                  PolyfaceVectors::_GetNormalCount      ()     const  {return m_normal.size ();}
size_t                  PolyfaceVectors::_GetParamCount       ()     const  {return m_param.size ();}
size_t                  PolyfaceVectors::_GetColorCount       ()     const  
    {
    if (m_intColor.Active ())
        return m_intColor.size ();
    return 0;
    }
size_t                  PolyfaceVectors::_GetFaceCount        ()     const  {return m_faceData.size(); } 
size_t                  PolyfaceVectors::_GetPointIndexCount  ()     const  {return m_pointIndex.size ();}
size_t                  PolyfaceVectors::_GetFaceIndexCount  ()      const  {return m_faceIndex.size ();}
size_t                  PolyfaceVectors::_GetEdgeChainCount  ()      const  {return m_edgeChain.size ();}
DPoint3dCP              PolyfaceVectors::_GetPointCP ()              const  {return m_point.GetCP();}
DVec3dCP                PolyfaceVectors::_GetNormalCP ()             const  {return m_normal.GetCP();}
DPoint2dCP              PolyfaceVectors::_GetParamCP ()              const  {return m_param.GetCP();}
FacetFaceDataCP         PolyfaceVectors::_GetFaceDataCP ()           const  {return m_faceData.GetCP(); }
PolyfaceEdgeChainCP     PolyfaceVectors::_GetEdgeChainCP ()          const  {return m_edgeChain.GetCP(); }
PolyfaceAuxDataCPtr     PolyfaceVectors::_GetAuxDataCP()             const  {return m_auxData; }
uint32_t const*         PolyfaceVectors::_GetIntColorCP ()           const  {return (uint32_t const*)m_intColor.GetCP();}

/** For Color, Param, and normal indices, resolveToDefaults allows caller to request using
PointIndex (or other default decision) if respective index is same as PointIndex. */
int32_t const*          PolyfaceVectors::_GetPointIndexCP  ()        const  {return m_pointIndex.GetCP ();  }
int32_t const*          PolyfaceVectors::_GetColorIndexCP  ()        const  {return m_colorIndex.GetCP ();  }
int32_t const*          PolyfaceVectors::_GetParamIndexCP  ()        const  {return m_paramIndex.GetCP ();  }
int32_t const*          PolyfaceVectors::_GetNormalIndexCP ()        const  {return m_normalIndex.GetCP (); }
int32_t const*          PolyfaceVectors::_GetFaceIndexCP ()          const  {return m_faceIndex.GetCP ();   }
bool                    PolyfaceVectors::_GetTwoSided ()             const  {return m_twoSided;}
uint32_t                PolyfaceVectors::_GetNumPerFace ()           const  {return m_numPerFace;}
uint32_t                PolyfaceVectors::_GetNumPerRow ()            const  {return m_numPerRow;}
uint32_t                PolyfaceVectors::_GetMeshStyle()             const  {return m_meshStyle;}

PolyfaceVectors* PolyfaceVectors::_AsPolyfaceVectorsP () const { return const_cast <PolyfaceVectors*> (this);}
void    PolyfaceVectors::SetTwoSided (bool twoSided)        { m_twoSided = twoSided; }
void    PolyfaceVectors::SetNumPerFace (uint32_t numPerFace)  { m_numPerFace = numPerFace; }
void    PolyfaceVectors::SetNumPerRow (uint32_t numPerRow)    { m_numPerRow = numPerRow; }
void    PolyfaceVectors::SetMeshStyle (uint32_t meshStyle)    { m_meshStyle  = meshStyle;     }
void    PolyfaceVectors::SetAuxData(PolyfaceAuxDataPtr& auxData) { m_auxData = auxData; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceVectors::PolyfaceVectors() :
    // array constructions: numPerStruct, structsPerRow, tag, indexedBy, indexFamily, active
    m_point             (3, 1, 0, 0, 0, false),
    m_pointIndex        (1, 1, 0, 0, 0, false),

    m_param             (2, 1, 0, 0, 0, false),
    m_paramIndex        (1, 1, 0, 0, 0, false),

    m_normal            (3, 1, 0, 0, 0, false),
    m_normalIndex       (1, 1, 0, 0, 0, false),

    m_intColor          (1, 1, 0, 0, 0, false),
    m_colorIndex        (1, 1, 0, 0, 0, false),
    m_faceIndex         (1, 1, 0, 0, 0, false),
    m_faceData (),
    m_numPerFace (0),
    m_twoSided (false),
    m_meshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVectors::CopyAllActiveFlagsFrom (PolyfaceVectorsCR source)
    {
    m_pointIndex.SetActive (source.m_pointIndex.Active ());
    m_param.SetActive (source.m_param.Active ());
    m_paramIndex.SetActive (source.m_paramIndex.Active ());
    m_normal.SetActive (source.m_normal.Active ());
    m_normalIndex.SetActive (source.m_normalIndex.Active ());

    m_colorIndex.SetActive (source.m_colorIndex.Active ());
    m_intColor.SetActive (source.m_intColor.Active ());

    m_faceData.SetActive (source.m_faceData.Active ());
    m_faceIndex.SetActive (source.m_faceIndex.Active ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVectors::ClearAllArrays ()
    {
    m_point.clear ();
    m_pointIndex.clear ();
    m_param.clear ();
    m_paramIndex.clear ();
    m_normal.clear ();
    m_normalIndex.clear ();
    m_colorIndex.clear ();
    m_intColor.clear ();
    m_faceData.clear ();
    m_faceIndex.clear ();
    m_auxData = nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceVectors::CopyAllActiveFlagsFromQuery (PolyfaceQueryCR source)
    {
    m_point.SetActive (source.GetPointCP () != NULL);
    m_pointIndex.SetActive (source.GetPointIndexCP () != NULL);
    m_param.SetActive (source.GetParamCP () != NULL);
    m_paramIndex.SetActive (source.GetParamIndexCP () != NULL);
    m_normal.SetActive (source.GetNormalCP () != NULL);
    m_normalIndex.SetActive (source.GetNormalIndexCP () != NULL);
    m_colorIndex.SetActive (source.GetColorIndexCP () != NULL);
    m_intColor.SetActive (source.GetIntColorCP () != NULL);
    m_faceData.SetActive (source.GetFaceDataCP () != NULL);
    m_faceIndex.SetActive (source._GetFaceIndexCP () != NULL);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
