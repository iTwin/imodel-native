/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "CoordinateMaps.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_defaultRelTol = 1.0e-12;
static double s_defaultAbsTol = 1.0e-14;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template<typename T>
static bool sort_zyx
(
T xA,
T yA,
T zA,
T xB,
T yB,
T zB,
double absTol = s_defaultAbsTol,
double relTol = s_defaultRelTol
)
    {
    double tol = absTol;
    
    if (0.0 !=  relTol)
        tol +=  relTol * 
            ( fabs (xA) + fabs (xB)
            + fabs (yA) + fabs (yB)
            + fabs (zA) + fabs (zB));

    double ntol = - tol;

    T dz = zB - zA;

    if (dz > tol)
        return false;
    if (dz < ntol)
        return true;

    T dy = yB - yA;

    if (dy > tol)
        return false;
    if (dy < ntol)
        return true;

    T dx = xB - xA;

    if (dx > tol)
        return false;
    if (dx < ntol)
        return true;

    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3dZYXTolerancedSortComparison::DPoint3dZYXTolerancedSortComparison (double absTol, double relTol)
    : m_absTol (absTol), m_relTol (relTol)
    {
    }
   

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DPoint3dZYXTolerancedSortComparison::operator() (const DPoint3d& pointA, const DPoint3d &pointB) const
    {
    return sort_zyx<double> (pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z, m_absTol, m_relTol);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3dZYXTolerancedSortComparison::DVec3dZYXTolerancedSortComparison (double absTol, double relTol)
    : m_absTol (absTol), m_relTol (relTol)
    {
    }
   

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DVec3dZYXTolerancedSortComparison::operator() (const DVec3d& pointA, const DVec3d &pointB) const
    {
    return sort_zyx<double> (pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z, m_absTol, m_relTol);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DVec3dZYXSortComparison::operator () (const DVec3d& vecA, const DVec3d &vecB) const
    {
    return sort_zyx<double> (vecA.x, vecA.y, vecA.z, vecB.x, vecB.y, vecB.z);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DPoint2dYXSortComparison::operator () (const DPoint2d& pointA, const DPoint2d &pointB) const
    {
    return sort_zyx<double>  (pointA.x, pointA.y, 0.0, pointB.x, pointB.y, 0.0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool UInt32SortComparison::operator () (uint32_t const &colorA, uint32_t const &colorB) const
    {
    return colorA < colorB;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceCoordinateMap::PolyfaceCoordinateMap (PolyfaceHeaderR polyface) : m_polyface (polyface), m_currentParamZ(0.0)
    {
    m_pointMap = new PolyfaceZYXMap (DPoint3dZYXTolerancedSortComparison (s_defaultAbsTol, s_defaultRelTol));
    m_paramMap = new PolyfaceZYXMap (DPoint3dZYXTolerancedSortComparison (s_defaultAbsTol, s_defaultRelTol));
    m_normalMap = new PolyfaceZYXDVec3dMap (DVec3dZYXTolerancedSortComparison (s_defaultAbsTol, s_defaultRelTol));
    }


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceZYXMap::PolyfaceZYXMap (DPoint3dZYXTolerancedSortComparison const &compare)
        : bmap <DPoint3d, size_t, DPoint3dZYXTolerancedSortComparison> (compare)
        {
        }

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceZYXDVec3dMap::PolyfaceZYXDVec3dMap (DVec3dZYXTolerancedSortComparison const &compare)
        : bmap <DVec3d, size_t, DVec3dZYXTolerancedSortComparison> (compare)
        {
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceCoordinateMap::PolyfaceCoordinateMap
(
PolyfaceHeaderR polyface,
double xyzAbsTol,
double xyzRelTol,
double paramAbsTol,
double paramRelTol,
double normalAbsTol,
double normalRelTol
)  : m_polyface (polyface), m_currentParamZ(0.0)
    {
    m_pointMap = new PolyfaceZYXMap (DPoint3dZYXTolerancedSortComparison (xyzAbsTol,   xyzRelTol  ));
    m_paramMap = new PolyfaceZYXMap (DPoint3dZYXTolerancedSortComparison (paramAbsTol, paramRelTol));
    m_normalMap = new PolyfaceZYXDVec3dMap (DVec3dZYXTolerancedSortComparison (normalAbsTol, normalRelTol));
    }


PolyfaceCoordinateMap::~PolyfaceCoordinateMap ()
    {
    delete m_pointMap;
    delete m_paramMap;
    delete m_normalMap;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceCoordinateMap::SetCurrentParamZ (double data) {m_currentParamZ = data;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceCoordinateMap::GetCurrentParamZ () const {return m_currentParamZ;}



#ifdef ImplementFaceParameterRemappingMethods

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceCoordinateMap::RemapCurrentFaceParamsToRange (DRange2dCR newRange)
    {
    size_t index0 = m_currentFaceBaseParamIndex;
    size_t index1 = m_polyface.Param ().size ();
    if (index0 < index1 && !newRange.IsNull ())
        {
        DRange2d currentRange = GetCurrentFaceParamRange ();
        bvector<DPoint2d> &params = m_polyface.Param ();
        double xScale, yScale;
        bsiTrig_safeDivide (&xScale, (newRange.high.x - newRange.low.x), (currentRange.high.x - currentRange.low.x), 1.0);
        bsiTrig_safeDivide (&yScale, (newRange.high.y - newRange.low.y), (currentRange.high.y - currentRange.low.y), 1.0);
        for (size_t i = index0; i < index1; i++)
            {
            DPoint2d uvOld = params[i], uvNew = newRange.low;
            uvNew.x += (uvOld.x - currentRange.low.x) * xScale;
            uvNew.y += (uvOld.y - currentRange.low.y) * yScale;
            params[i] = uvNew;
            }
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange2d PolyfaceCoordinateMap::GetCurrentFaceParamRange () const
    {
    DRange2d range = DRange2d::NullRange ();
    size_t index0 = m_currentFaceBaseParamIndex;
    size_t index1 = m_polyface.Param ().size ();
    bvector<DPoint2d> &params = m_polyface.Param ();
    for (size_t i = index0; i < index1; i++)
        range.Extend (params[i]);
    return range;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceCoordinateMap::ApplyParamOptionsToDistanceParams (IFacetOptionsR options)
    {
    DRange2d targetRange;
    DRange2d currentRange = GetCurrentFaceParamRange ();
    FacetParamMode newMode = options.GetParamMode ();
    if (newMode == FACET_PARAM_ZeroOne)
        targetRange= DRange2d::From (0,0,1,1);
    else
        {
        targetRange.low = DPoint2d::From (0,0);
        DVec2d diagonal = DVec2d::FromStartEnd (currentRange.low, currentRange.high);
        double scale = options.GetParamDistanceScale ();
        if (scale != 0.0)
            diagonal.Scale (scale);
        targetRange.high.SumOf (targetRange.low, diagonal);
        }
    RemapCurrentFaceParamsToRange (targetRange);
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceCoordinateMapPtr PolyfaceCoordinateMap::New (PolyfaceHeaderR polyface)
    {
    PolyfaceCoordinateMapPtr header = new PolyfaceCoordinateMap (polyface);
    return header;    
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceCoordinateMapPtr PolyfaceCoordinateMap::Create(PolyfaceHeaderR polyface)
    {
    PolyfaceCoordinateMapPtr header = new PolyfaceCoordinateMap (polyface);
    return header;    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceCoordinateMap::AddPoint (DPoint3dCR point)
    {
    size_t index;
    if (FindPoint (point, index))
        return index;
    (*m_pointMap)[point] = index = m_polyface.Point ().AppendAndReturnIndex (point);
    return index;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceCoordinateMap::FindPoint (DPoint3dCR point, size_t &index)
    {
    index = SIZE_MAX;
    PolyfaceZYXMap::iterator key = m_pointMap->find (point);
    if (key == m_pointMap->end ())
        return false;
    index = key->second;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceCoordinateMap::TryGetPoint (size_t index, DPoint3dR point) const
    {
    if (index < m_polyface.Point ().size ())
        {
        point = m_polyface.Point ()[index];
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceCoordinateMap::AddParam (DPoint2dCR xy)
    {
    size_t index;
    if (FindParam (xy, index))
        return index;
    DPoint3d xyz = DPoint3d::FromXYZ (xy.x, xy.y, m_currentParamZ);
    (*m_paramMap)[xyz] = index = m_polyface.Param ().AppendAndReturnIndex (xy);;
    return index;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceCoordinateMap::FindParam (DPoint2dCR xy, size_t &index)
    {
    index = SIZE_MAX;
    DPoint3d xyz = DPoint3d::FromXYZ (xy.x, xy.y, m_currentParamZ);
    PolyfaceZYXMap::iterator key = m_paramMap->find (xyz);
    if (key == m_paramMap->end ())
        return false;
    index = key->second;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceCoordinateMap::AddNormal (DVec3dCR normal)
    {
    size_t index;
    if (FindNormal (normal, index))
        return index;
    (*m_normalMap)[normal] = index = m_polyface.Normal ().AppendAndReturnIndex (normal);
    return index;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceCoordinateMap::FindNormal (DVec3dCR normal, size_t &index)
    {
    index = SIZE_MAX;
    auto key = (*m_normalMap).find (normal);
    if (key == (*m_normalMap).end ())
        return false;
    index = key->second;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceCoordinateMap::AddIntColor (uint32_t intColor)
    {
    size_t index;
    if (FindIntColor (intColor, index))
        return index;
    m_intColorMap[intColor] = index = m_polyface.IntColor ().AppendAndReturnIndex (intColor);
    return index;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceCoordinateMap::FindIntColor (uint32_t intColor, size_t &index)
    {
    index = SIZE_MAX;
    bmap<uint32_t, size_t, UInt32SortComparison>::iterator key = m_intColorMap.find (intColor);
    if (key == m_intColorMap.end ())
        return false;
    index = key->second;
    return true;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceCoordinateMap::AddPolygon
(
int n,
DPoint3dCP points,
DVec3dCP normals,
DPoint2dCP params,
int * intColor
)
    {
    // eliminate trailing duplicates ...
    while (n > 1 && points[n - 1].IsEqual (points[0]))
        {
        n--;
        }
    for (int i = 0; i < n; i++)
        {
        m_polyface.PointIndex ().push_back ((int)AddPoint (points[i]) + 1);
        if (normals != NULL)
            m_polyface.NormalIndex ().push_back ((int)AddNormal (normals[i]) + 1);
        if (params != NULL)
            m_polyface.ParamIndex ().push_back ((int)AddParam (params[i]) + 1);
        if (intColor != NULL)
            m_polyface.ColorIndex ().push_back ((int)AddIntColor (intColor[i]) + 1);
        }
    m_polyface.PointIndex ().push_back (0);
    if (normals != nullptr)
        m_polyface.NormalIndex ().push_back (0);
    if (params != nullptr)
        m_polyface.ParamIndex ().push_back (0);
    if (intColor != nullptr)
        m_polyface.ColorIndex ().push_back (0);
    }

void PolyfaceCoordinateMap::AddPolygon
(
bvector<DPoint3d> const &points,
bvector<DVec3d> const *normals,
bvector<DPoint2d> const *params,
bvector<int> const *intColor
)
    {
    size_t n = points.size ();
    // eliminate trailing duplicates ...
    while (n > 1 && points[n - 1].IsEqual (points[0]))
        {
        n--;
        }
    for (size_t i = 0; i < n; i++)
        {
        m_polyface.PointIndex ().push_back ((int)AddPoint (points[i]) + 1);
        if (normals != NULL)
            m_polyface.NormalIndex ().push_back ((int)AddNormal (normals->at(i)) + 1);
        if (params != NULL)
            m_polyface.ParamIndex ().push_back ((int)AddParam (params->at(i)) + 1);
        if (intColor != NULL)
            m_polyface.ColorIndex ().push_back ((int)AddIntColor (intColor->at(i)) + 1);
        }
    m_polyface.PointIndex ().push_back (0);
    if (normals != nullptr)
        m_polyface.NormalIndex ().push_back (0);
    if (params != nullptr)
        m_polyface.ParamIndex ().push_back (0);
    if (intColor != nullptr)
        m_polyface.ColorIndex ().push_back (0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceCoordinateMap::AddVisitorPartialFace
(
PolyfaceVisitor &source,
size_t i0,
size_t n
)
    {    
    for (size_t i = 0; i < n; i++)
        {
        size_t readIndex = i0 + i;
        int pointIndex = 1 + (int)AddPoint (source.Point()[readIndex]);
        if (!source.Visible()[readIndex])
            pointIndex = - pointIndex;
        m_polyface.PointIndex ().push_back (pointIndex);
        if (source.Normal ().Active ())
            m_polyface.NormalIndex ().push_back (1 + (int)AddNormal (source.Normal()[readIndex]));
        if (source.Param ().Active ())
            m_polyface.ParamIndex ().push_back (1 + (int)AddParam (source.Param ()[readIndex]));
        if (source.IntColor().Active ())
            m_polyface.ColorIndex ().push_back (1 + (int)AddIntColor (source.IntColor ()[readIndex]));
        }
    m_polyface.TerminateAllActiveIndexVectors ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceCoordinateMap::AddVisitorFace (PolyfaceVisitor &source)
    {    
    AddVisitorPartialFace (source, 0, source.NumEdgesThisFace ());
    }



PolyfaceHeader& PolyfaceCoordinateMap::GetPolyfaceHeaderR ()
    {
    return m_polyface;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceCoordinateMap::ClearData ()
    {
    m_pointMap->clear ();
    m_normalMap->clear ();
    m_paramMap->clear ();
    m_intColorMap.clear ();
    m_polyface.ClearAllVectors ();
    }

//===================================================================================
//===================================================================================
PolyfaceCoordinateAverageContext::PolyfaceCoordinateAverageContext ()
    : m_map (DPoint3dZYXTolerancedSortComparison (s_defaultAbsTol, s_defaultRelTol)),
    m_numPoints (0)
    {
    }

//! Create a new context.
//! The reference mesh is consulted to determine which arrays (uv, normal, color, etc) need to be averaged/
//! The reference mesh is NOT retained.
PolyfaceCoordinateAverageContextPtr PolyfaceCoordinateAverageContext::Create ()
    {
    return new PolyfaceCoordinateAverageContext ();
    }

void AccumulateDoubleColor (DPoint4dR sums, int color)
    {
    sums.x += color & 0xFF;
    sums.y += (color >> 8) & 0xff;
    sums.z += (color >> 16) & 0xff;
    sums.w += (color >> 24) & 0xff;
    }

//! Announce a facet whose vertices contribute to the averages.
void PolyfaceCoordinateAverageContext::AnnounceFacet (PolyfaceVisitorR visitor, TransformCP worldToLocal)
    {
    size_t numVertex = visitor.NumEdgesThisFace ();
    for (size_t i = 0; i < numVertex; i++)
        {
        size_t index = SIZE_MAX;
        DPoint3d xyz = visitor.Point ()[i];
        if (nullptr != worldToLocal)
            worldToLocal->Multiply (xyz);
        PolyfaceZYXMap::iterator key = m_map.find (xyz);

        if (key == m_map.end ())
            {
            index = m_numPoints++;
            m_numIncident.push_back (0);
            m_param.push_back (DPoint2d::FromZero ());
            m_normal.push_back (DVec3d::From (0,0,0));
            m_intColor.push_back (0);
            m_doubleColor.push_back (DPoint4d::From (0,0,0,0));
            m_map[xyz] = index;
            }
        else
            {
            index = key->second;
            }
        m_numIncident[index]++;
        if (visitor.Param ().Active ())
            m_param[index].Add (visitor.Param ()[i]);
        if (visitor.Normal ().Active () && i < visitor.Normal ().size ())
            m_normal[index].Add (visitor.Normal ()[i]);
        // don't try to average int colors . . .
        if (visitor.IntColor ().Active () && i < visitor.IntColor ().size ())
            {
            AccumulateDoubleColor (m_doubleColor[index], visitor.IntColor ()[i]);
            m_intColor[index] = visitor.IntColor ()[i];
            }
        }
    }

//! look up a point.
ValidatedSize PolyfaceCoordinateAverageContext::FindPoint (DPoint3dCR xyz1, TransformCP worldToLocal)
    {
    DPoint3d xyz = xyz1;
    if (nullptr != worldToLocal)
        worldToLocal->Multiply (xyz);
    PolyfaceZYXMap::iterator key = m_map.find (xyz);
    if (key == m_map.end ())
        {
        return ValidatedSize (0, false);
        }
    else
        {
        return ValidatedSize (key->second, true);
        }
    }

//! return the averaged parameter.
DPoint2d PolyfaceCoordinateAverageContext::GetAverageParam (size_t index) const
    {
    DPoint2d result = DPoint2d::From (0,0);
    if (index < m_param.size () && m_numIncident[index] > 0)
        {
        result = m_param[index];
        result.Scale (1.0 / m_numIncident[index]);
        }
    return result;
    }

//! return the averaged parameter.
DVec3d PolyfaceCoordinateAverageContext::GetAverageNormal (size_t index) const
    {
    DVec3d result = DVec3d::From (0,0);
    if (index < m_normal.size () && m_numIncident[index] > 0)
        {
        result = m_normal[index];
        result.Scale (1.0 / m_numIncident[index]);
        }
    return result;
    }

//! return the (last) intColor.
int PolyfaceCoordinateAverageContext::GetIntColor (size_t index) const
    {
    if (index < m_intColor.size ())
        return m_intColor [index];
    return 0;
    }

//! return int color averaged in each channel
int PolyfaceCoordinateAverageContext::GetAverageIntColor (size_t index) const
    {
    if (index < m_doubleColor.size () && m_numIncident[index] > 0)
        {
        DPoint4d average = m_doubleColor[index];
        average.Scale (m_doubleColor[index], 1.0 / (double)m_numIncident[index]);
        // Each value summed was between 0 and 127.
        // So the average must be between 0 and 127.
        // Just to be sure, blast the bits as inserted ..
        int result =
              (((int)average.x)  & 0xFF)
            + ((((int)average.y) & 0xFF) << 8)
            + ((((int)average.z) & 0xFF) << 16)
            + ((((int)average.w) & 0xFF) << 24);
        return result;
        }
    return 0;
    }


bool PolyfaceCoordinateAverageContext:: LoadVisitor
(
PolyfaceVisitorR visitor,
bvector<DPoint3d> const &xyz
)
    {
    size_t numXYZ = xyz.size ();
    visitor.Point ().clear ();
    for (size_t i = 0; i < numXYZ; i++)
        visitor.Point ().push_back (xyz[i]);
    visitor.m_numEdgesThisFace = (uint32_t)numXYZ;
    size_t numFail = 0;
    visitor.Normal ().clear ();
    visitor.Param ().clear ();
    visitor.IntColor ().clear ();
    for (size_t i = 0; i < numXYZ; i++)
        {
        ValidatedSize k = FindPoint (xyz[i]);
        if (k.IsValid ())
            {
            // remark: if active flags disagree with what data is available, just accept the zeros.
            visitor.Visible ().push_back (true);
            if (visitor.Param ().Active ())
                visitor.Param ().push_back (GetAverageParam (k));
            if (visitor.Normal ().Active ())
                visitor.Normal ().push_back (GetAverageNormal (k));
            if (visitor.IntColor ().Active ())
                visitor.IntColor ().push_back (GetIntColor (k));
            }
        else
            numFail++;
        }
    return numFail == 0;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
