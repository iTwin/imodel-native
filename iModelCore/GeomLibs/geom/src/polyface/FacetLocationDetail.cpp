/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Single bit masks for active flags
// Data become active when present on a Load step.
#define FLD_PointActive 0x01
#define FLD_ParamActive 0x02
#define FLD_NormalActive 0x04

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::Zero ()
    {
    isInteriorPoint = false;
    activeMask = 0;
    readIndex = 0;
    a = 0.0;
    memset (&point, 0, sizeof (point));
    memset (&param, 0, sizeof (param));
    memset (&normal, 0, sizeof (normal));
    numSourceIndex = 0;

    memset (&dXdu, 0, sizeof (dXdu));
    memset (&dXdv, 0, sizeof (dXdv));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::AccumulateScaledData
(
FacetLocationDetailCR source,
double fraction
)
    {
    activeMask |= source.activeMask;    // hmmm... if source has data, just accept it.
    point.SumOf  (point,  source.point, fraction);
    param.SumOf  (param,  source.param, fraction);
    normal.SumOf (normal, source.normal, fraction);
    for (int i = 0; i < source.numSourceIndex && numSourceIndex < MAX_FACET_LOCATION_INDEX; i++)
        {
        int k = numSourceIndex++;
        sourceIndex[k] = source.sourceIndex[i];
        sourceFraction[k] = fraction * source.sourceFraction[i];
        intColor[k] = source.intColor[i];
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::LoadVertexData (FacetLocationDetailR detail, size_t index)
    {
    detail.Zero ();
    if (index >= m_point.size ())
        return false;
    detail.activeMask |= FLD_PointActive;
    detail.point = m_point[index];
    detail.readIndex = GetReadIndex ();

    if (index < m_param.size ())
        detail.SetParam (m_param[index]);

    if (index < m_normal.size ())
        detail.SetNormal (m_normal[index]);

    detail.numSourceIndex = 1;
    detail.sourceIndex[0] = index;
    detail.sourceFraction[0] = 1.0;
    detail.intColor[0] = 
            index < m_intColor.size () ? m_intColor[index] : -1;
    detail.colorIndex[0] = 
            index < m_colorIndex.size () ? m_colorIndex[index] : -1;
    return true;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::LoadCyclicVertexData(FacetLocationDetailR detail, size_t index)
    {
    size_t n = this->NumEdgesThisFace();
    if (n == 0)
        return false;
    index = (index % n);
    return LoadVertexData (detail, index);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FacetLocationDetail::FacetLocationDetail()
    {
    Zero ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FacetLocationDetail::FacetLocationDetail(size_t readIndexIn, double aIn)
    {
    Zero ();
    readIndex = readIndexIn;
    a = aIn;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::TryGetPoint (DPoint3dR data) const
    {
    data = point;
    return 0 != (activeMask & FLD_PointActive);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::TryGetParam (DPoint2dR data) const
    {
    data = param;
    return 0 != (activeMask & FLD_ParamActive);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::TryGetNormal (DVec3dR data) const
    {
    data = normal;
    return 0 != (activeMask & FLD_NormalActive);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::SetNormal (DVec3dCR data)
    {
    normal = data;
    activeMask |= FLD_NormalActive;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::SetParam (DPoint2dCR data)
    {
    param = data;
    activeMask |= FLD_ParamActive;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t FacetLocationDetail::GetNumWeights () const{return numSourceIndex;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::TryGetWeight (size_t index, double &data) const
    {
    if (index < (size_t)numSourceIndex)
        return false;
    data = sourceFraction[index];
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::TryGetVertexIndex (size_t index, size_t &data) const
    {
    if (index < (size_t)numSourceIndex)
        return false;
    data = sourceIndex[index];
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::TryGetIntColor (size_t index, uint32_t &data) const
    {
    if (index < (size_t)numSourceIndex)
        return false;
    data = intColor[index];
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::GetIsInterior () const {return isInteriorPoint;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t FacetLocationDetail::GetReadIndex () const {return readIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::SetIsInterior (bool value)  {isInteriorPoint = value;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::SetReadIndex (size_t value) {readIndex = value;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::CompareUV (FacetLocationDetail const &dataB) const
    {
    if (param.x < dataB.param.x)
        return true;
    if (param.x > dataB.param.x)
        return false;
    if (param.y < dataB.param.y)
        return true;
    if (param.y > dataB.param.y)
        return true;
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static bool cb_CompareUV (FacetLocationDetail const &dataA, FacetLocationDetail const &dataB)
    {
    return dataA.CompareUV (dataB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::SortUV (bvector<FacetLocationDetail> &data)
    {
    std::sort (data.begin (), data.end (), cb_CompareUV);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FacetLocationDetail::CompareA (FacetLocationDetail const &dataB) const {return a < dataB.a;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static bool cb_CompareA (FacetLocationDetail const &dataA, FacetLocationDetail const &dataB) {return dataA.CompareA (dataB);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FacetLocationDetail::SortA (bvector<FacetLocationDetail> &data)
    {
    std::sort (data.begin (), data.end (), cb_CompareA);
    }



END_BENTLEY_GEOMETRY_NAMESPACE
