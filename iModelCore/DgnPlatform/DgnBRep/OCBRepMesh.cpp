/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/OCBRepMesh.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/OCBRep.h>

static const double s_maxFacetAngleTol     = 1.00; // radians
static const double s_minFacetAngleTol     = 0.10; // radians
static const double s_defaultFacetAngleTol = 0.39; // radians

static const double DEFAULT_CREASE_DEGREES = 45.0; // From SimplifyViewDrawGeom.

//=======================================================================================
//! @bsiclass                                                   Ray.Bentley     02/2016
//=======================================================================================
struct EdgeIndices
{
int m_low;
int m_high;

EdgeIndices () {} 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeIndices (int i0, int i1)
    {
    if (i0 < i1)
        {
        m_low  = i0;
        m_high = i1;
        }
    else
        {
        m_low  = i1;
        m_high = i0;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (EdgeIndices const& other) const
    {
    if (m_low == other.m_low)
        return m_high < other.m_high;

    return m_low < other.m_low;
    }

}; // EdgeIndices

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
static double restrictAngleTol (double radians, double defaultRadians, double minRadians, double maxRadians)
    {
    if (radians <= 0.0)
        return defaultRadians;
    else if (radians < minRadians)
        return minRadians;
    else if (radians > maxRadians)
        return maxRadians;
    else
        return radians;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr OCBRep::IncrementalMesh(TopoDS_Shape const& shape, IFacetOptionsR facetOptions, bool cleanShape)
    {
    if (shape.IsNull())
        return nullptr;

    double angularDeflection = restrictAngleTol(facetOptions.GetAngleTolerance(), s_defaultFacetAngleTol, s_minFacetAngleTol, s_maxFacetAngleTol);
    double linearDeflection = facetOptions.GetChordTolerance();

    if (linearDeflection <= 0.0)
        {
        BeAssert(false && "Chord tolerance required - BRepMesh_IncrementalMesh behaves poorly/slowly otherwise.");
        return nullptr;
        }

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);
    bool isInParallel = false;

    BRepMesh_IncrementalMesh(shape, linearDeflection, false, angularDeflection, isInParallel);

    if (facetOptions.GetNormalsRequired())
        BRepLib::EnsureNormalConsistency(shape, Angle::DegreesToRadians(DEFAULT_CREASE_DEGREES), true);

    for (TopExp_Explorer ex (shape, TopAbs_FACE); ex.More(); ex.Next())
        {
        Transform                   locationTransform;
        bmap <int,int>              indexMap, uvIndexMap, normalIndexMap;
        bset <EdgeIndices>          edgeIndicesSet;
        TopLoc_Location             location;
        TopoDS_Face const&          face = TopoDS::Face(ex.Current()); 
        Handle(Poly_Triangulation)  polyTriangulation = BRep_Tool::Triangulation (face, location);
        bool                        doLocationTransform, doReverse = TopAbs_REVERSED == face.Orientation();

        if (polyTriangulation.IsNull())
            continue;

        if (false != (doLocationTransform = !location.IsIdentity()))
            locationTransform = OCBRep::ToTransform(location.Transformation());

        for (TopExp_Explorer edgeExplorer (face, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next())
            {
            Handle (Poly_PolygonOnTriangulation)    edgeTriangulation = BRep_Tool::PolygonOnTriangulation (TopoDS::Edge (edgeExplorer.Current()), polyTriangulation, location);
            TColStd_Array1OfInteger const&          edgeNodes = edgeTriangulation->Nodes();  
                     
            if (!edgeTriangulation.IsNull())
                for (int i = edgeNodes.Lower(); i<= edgeNodes.Upper()-1; i++)
                    edgeIndicesSet.insert (EdgeIndices (edgeNodes.Value(i), edgeNodes.Value(i+1)));
            }

        TColgp_Array1OfPnt const& points = polyTriangulation->Nodes();
        for (int i=1; i <= points.Length(); i++)
            {
            DPoint3d    point = OCBRep::ToDPoint3d(points.Value(i));

            if (doLocationTransform)
                locationTransform.Multiply(point);

            indexMap[i] = (int) polyfaceBuilder->FindOrAddPoint(point);
            }

        if (polyTriangulation->HasNormals() && facetOptions.GetNormalsRequired())
            {
            TShort_Array1OfShortReal const& normals = polyTriangulation->Normals();

            for (int i=1, j=1; i <= normals.Length(); i += 3, j++)
                {
                DVec3d    normal = DVec3d::From(normals.Value(i), normals.Value(i+1), normals.Value(i+2));

                normalIndexMap[j] = (int) polyfaceBuilder->FindOrAddNormal(normal);
                }
            }

        if (polyTriangulation->HasUVNodes() && facetOptions.GetParamsRequired())
            {
            TColgp_Array1OfPnt2d const& uvNodes = polyTriangulation->UVNodes();
            DRange2d                    uvRange;
            DVec2d                      uvDelta;

            BRepTools::UVBounds(face, uvRange.low.x, uvRange.high.x, uvRange.low.y, uvRange.high.y);
            uvDelta.DifferenceOf(uvRange.high, uvRange.low);

            for (int i=1; i <= uvNodes.Length(); i++)
                uvIndexMap[i] = (int) polyfaceBuilder->FindOrAddParam(DPoint2d::From((uvNodes.Value(i).X() - uvRange.low.x) / uvDelta.x, (uvNodes.Value(i).Y() - uvRange.low.y) / uvDelta.y));
            }

        Poly_Array1OfTriangle const& triangles = polyTriangulation->Triangles();

        for (int i=1; i <= polyTriangulation->NbTriangles(); i++)
            {
            Poly_Triangle   triangle = triangles.Value(i);

            if (doReverse)
                {
                int     tmp = triangle.Value(1);

                triangle.ChangeValue(1) = triangle.Value(3);
                triangle.ChangeValue(3) = tmp;
                }

            for (int j=0; j < 3; j++)
                polyfaceBuilder->AddPointIndex (indexMap[triangle.Value(j+1)], edgeIndicesSet.find(EdgeIndices(triangle.Value(j+1), triangle.Value(1 + (j+1) % 3))) != edgeIndicesSet.end());

            polyfaceBuilder->AddPointIndexTerminator();    

            if (!normalIndexMap.empty())
                {
                for (int j=1; j <= 3; j++)
                    polyfaceBuilder->AddNormalIndex(normalIndexMap[triangle.Value(j)]);

                polyfaceBuilder->AddNormalIndexTerminator();
                }
            
            if (!uvIndexMap.empty())
                {
                for (int j=1; j <= 3; j++)
                    polyfaceBuilder->AddParamIndex(uvIndexMap[triangle.Value(j)]);

                polyfaceBuilder->AddParamIndexTerminator();
                }
            }
        }

    if (cleanShape)
        BRepTools::Clean(shape); // Don't leave triangulation on shape...

    return polyfaceBuilder->GetClientMeshPtr();
    }
