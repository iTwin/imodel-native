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

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void addIsoCurve(Render::GraphicBuilderR graphic, Adaptor3d_CurveOnSurface const& curveOnSurf)
    {
    // NEEDSWORK: CurveTopologyId...
    double param1 = curveOnSurf.FirstParameter();
    double param2 = curveOnSurf.LastParameter();

    if (GeomAbs_Line == curveOnSurf.GetType())
        {
        DSegment3d segment = DSegment3d::From(OCBRep::ToDPoint3d(curveOnSurf.Value(param1)), OCBRep::ToDPoint3d(curveOnSurf.Value(param2)));
        graphic.AddLineString(2, segment.point);
        }
    else if (GeomAbs_Circle == curveOnSurf.GetType())
        {
        DEllipse3d arc = OCBRep::ToDEllipse3d(curveOnSurf.Circle(), param1, param2);
        graphic.AddArc(arc, false, false);
        }
    else if (GeomAbs_Ellipse == curveOnSurf.GetType())
        {
        DEllipse3d arc = OCBRep::ToDEllipse3d(curveOnSurf.Ellipse(), param1, param2);
        graphic.AddArc(arc, false, false);
        }
    else if (GeomAbs_BSplineCurve == curveOnSurf.GetType())
        {
        ICurvePrimitivePtr curve = OCBRep::ToCurvePrimitive(curveOnSurf.BSpline(), param1, param2);

        if (!curve.IsValid())
            return;

        graphic.AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
        }
    else
        {
        // Approximate everything else (Other, Bezier etc.) as a bspline...guess this ok...it's not like this api makes a lick of sense???
        Handle(Adaptor3d_HCurve) hCurveAdaptor = curveOnSurf.Trim(param1, param2, Precision::Confusion());
        GeomConvert_ApproxCurve approxCurve(hCurveAdaptor, Precision::Confusion(), GeomAbs_C2, 1000, 9);
  
        if (!approxCurve.HasResult())
            return;

        ICurvePrimitivePtr curve = OCBRep::ToCurvePrimitive(approxCurve.Curve(), param1, param2);

        if (!curve.IsValid())
            return;

        graphic.AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static Standard_Real getAdjustedRadialStartParam(Standard_Real minParam, Standard_Real step)
    {
    Standard_Real startParam = 0.0; // Radial u/v parameters aren't always between 0 and 2pi...

    if (minParam > 0.0)
        {
        do
            {
            startParam += step;

            } while (startParam < minParam);
        }
    else if (minParam < 0.0)
        {
        do
            {
            startParam -= step;

            } while (startParam > minParam);
        }

    return startParam;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Standard_Integer numIsoFromAngle(Standard_Real angle, Standard_Integer numIso)
    {
    if (fabs(angle) < 0.1)
        return 0;

    Standard_Integer n = (Standard_Integer) floor(0.5 + (Standard_Real) numIso * (fabs(angle) - mgds_fc_epsilon) / msGeomConst_2pi);

    return (n > 0 ? (n + 1) : 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Standard_Integer numIsoFromCurve(Handle(Geom_Curve) const& curve)
    {
    if (ShapeAnalysis_Curve::IsClosed(curve))
        return 4;

    TColgp_SequenceOfPnt pnts;

    if (!ShapeAnalysis_Curve::GetSamplePoints(curve, curve->FirstParameter(), curve->LastParameter(), pnts) || pnts.Size() < 3)
        return 0;

    int         iPt = 0;
    double      angle = 0.0;
    DVec3d      lastDir;
    DPoint3d    lastPt;

    for (gp_Pnt gPt : pnts)
        {
        DVec3d   dir = DVec3d::From(0.0, 0.0, 0.0);
        DPoint3d pt = OCBRep::ToDPoint3d(gPt);

        if (iPt > 0)
            dir.NormalizedDifference(lastPt, pt);

        if (iPt > 1)
            angle += fabs(dir.AngleTo(lastDir));

        lastDir = dir;
        lastPt  = pt;
        iPt++;
        }

    return numIsoFromAngle(angle, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void OCBRepUtil::HatchFace(Render::GraphicBuilderR graphic, Geom2dHatch_Hatcher& hatcher, TopoDS_Face const& face)
    {
    // NOTE: Adapted from DBRep_IsoBuilder...
    TopLoc_Location location;
    Handle(Geom_Surface) const& surface = BRep_Tool::Surface(face, location);

    if (surface.IsNull() || surface->IsKind(STANDARD_TYPE(Geom_Plane)))
        return;

    Standard_Integer numUIsos = 0, numVIsos = 0;
    Standard_Real stepU = 0.0, stepV = 0.0, uParamStart = 0.0, vParamStart = 0.0;
    Standard_Real faceUMin = 0.0, faceUMax = 0.0, faceVMin = 0.0, faceVMax = 0.0; 

    BRepTools::UVBounds(face, faceUMin, faceUMax, faceVMin, faceVMax);

    Standard_Real deltaU = Abs(faceUMax - faceUMin);
    Standard_Real deltaV = Abs(faceVMax - faceVMin);
    Standard_Real confusion = Min(deltaU, deltaV) * 1.e-8;

    Handle(Standard_Type) kindOfSurface = surface->DynamicType();

    if (STANDARD_TYPE(Geom_RectangularTrimmedSurface) == kindOfSurface)
        kindOfSurface = Handle(Geom_RectangularTrimmedSurface)::DownCast(surface)->BasisSurface()->DynamicType();

    if (STANDARD_TYPE(Geom_CylindricalSurface) == kindOfSurface || STANDARD_TYPE(Geom_ConicalSurface) == kindOfSurface)
        {
        numUIsos = 4; // Trim will exclude iso if outside face bounds...
        stepU = msGeomConst_piOver2;
        uParamStart = getAdjustedRadialStartParam(faceUMin, stepU);
        }
    else if (STANDARD_TYPE(Geom_ToroidalSurface) == kindOfSurface || STANDARD_TYPE(Geom_SphericalSurface) == kindOfSurface)
        {
        numUIsos = numVIsos = 4; // Trim will exclude iso if outside face bounds...
        stepU = stepV = msGeomConst_piOver2;
        uParamStart = getAdjustedRadialStartParam(faceUMin, stepU);
        vParamStart = getAdjustedRadialStartParam(faceVMin, stepV);
        }
    else if (STANDARD_TYPE(Geom_SurfaceOfRevolution) == kindOfSurface)
        {
        numUIsos = 4; // Trim will exclude iso if outside face bounds...
        stepU = msGeomConst_piOver2;
        uParamStart = getAdjustedRadialStartParam(faceUMin, stepU);

        Handle(Geom_Curve) uCurve = surface->UIso(faceUMin);

        if (!uCurve.IsNull())
            {
            numVIsos = numIsoFromCurve(uCurve);
            stepV = (numVIsos > 0 ? (deltaV / (Standard_Real) numVIsos) : 0.0);
            vParamStart = faceVMin + stepV;
            }
        }
    else
        {
        Handle(Geom_Curve) uCurve = surface->UIso(faceUMin);

        if (!uCurve.IsNull())
            {
            numVIsos = numIsoFromCurve(uCurve);
            stepV = (numVIsos > 0 ? (deltaV / (Standard_Real) numVIsos) : 0.0);
            vParamStart = faceVMin + stepV;
            }

        Handle(Geom_Curve) vCurve = surface->VIso(faceVMin);

        if (!vCurve.IsNull())
            {
            numUIsos = numIsoFromCurve(vCurve);
            stepU = (numUIsos > 0 ? (deltaU / (Standard_Real) numUIsos) : 0.0);
            uParamStart = faceUMin + stepU;
            }
        }

    if (0 == numUIsos && 0 == numVIsos)
        return;

    Handle(GeomAdaptor_HSurface) hSurfAdaptor = new GeomAdaptor_HSurface(surface);
    Adaptor3d_CurveOnSurface curveOnSurf(hSurfAdaptor);

    hatcher.Clear();
    hatcher.Confusion3d(confusion);

    // Retrieving the edges and loading them into the hatcher.
    TopExp_Explorer expEdges;

    for (expEdges.Init(face.Oriented(TopAbs_FORWARD), TopAbs_EDGE); expEdges.More(); expEdges.Next())
        {
        Standard_Real u1, u2;
        TopoDS_Edge const& edge = TopoDS::Edge(expEdges.Current());
        const Handle(Geom2d_Curve) pcurve = BRep_Tool::CurveOnSurface(edge, face, u1, u2);

        if (pcurve.IsNull() || Abs(u1 - u2) <= Precision::PConfusion())
            continue;

        // Check if a TrimmedCurve is necessary.
        if (Abs(pcurve->FirstParameter()-u1) <= Precision::PConfusion() && Abs(pcurve->LastParameter()-u2) <= Precision::PConfusion())
            {
            hatcher.AddElement(pcurve, edge.Orientation());
            }
        else
            {
            if (!pcurve->IsPeriodic())
                {
                Handle(Geom2d_TrimmedCurve) trimPCurve = Handle(Geom2d_TrimmedCurve)::DownCast(pcurve);

                if (!trimPCurve.IsNull())
                    {
                    if (trimPCurve->BasisCurve()->FirstParameter() - u1 > Precision::PConfusion() ||
                        trimPCurve->BasisCurve()->FirstParameter() - u2 > Precision::PConfusion() ||
                        u1 - trimPCurve->BasisCurve()->LastParameter()  > Precision::PConfusion() ||
                        u2 - trimPCurve->BasisCurve()->LastParameter()  > Precision::PConfusion())
                        {
                        hatcher.AddElement(pcurve, edge.Orientation());
                        continue;
                        }
                    }
                }
            else
                {
                if (pcurve->FirstParameter() - u1 > Precision::PConfusion())
                    u1 = pcurve->FirstParameter();

                if (pcurve->FirstParameter() - u2 > Precision::PConfusion())
                    u2 = pcurve->FirstParameter();

                if (u1 - pcurve->LastParameter() > Precision::PConfusion())
                    u1 = pcurve->LastParameter();

                if (u2 - pcurve->LastParameter() > Precision::PConfusion())
                    u2 = pcurve->LastParameter();
                }

            // if U1 and U2 coincide-->do nothing
            if (Abs(u1 - u2) <= Precision::PConfusion())
                continue;

            Handle(Geom2d_TrimmedCurve) trimPCurve = new Geom2d_TrimmedCurve(pcurve, u1, u2);
            Geom2dAdaptor_Curve trimCurve(trimPCurve);
            hatcher.AddElement(trimCurve, edge.Orientation());
            }
        }

    // Loading and trimming the hatchings.
    TColStd_Array1OfReal faceUParam(numUIsos > 0 ? 1 : 0, numUIsos);
    TColStd_Array1OfReal faceVParam(numVIsos > 0 ? 1 : 0, numVIsos);
    TColStd_Array1OfInteger faceUInd(numUIsos > 0 ? 1 : 0, numUIsos);
    TColStd_Array1OfInteger faceVInd(numVIsos > 0 ? 1 : 0, numVIsos);

    faceUInd.Init(0);
    faceVInd.Init(0);
            
    if (stepU > confusion)
        {
        Standard_Real uParam = uParamStart;
        gp_Dir2d dir(0.0, 1.0);

        for (Standard_Integer iIso = 1; iIso <= numUIsos; iIso++)
            {
            faceUParam(iIso) = uParam;

            if (!((fabs(uParam - faceUMin) < confusion) || (fabs(uParam - faceUMax) < confusion)))
                {
                gp_Pnt2d origin(uParam, 0.0);
                Geom2dAdaptor_Curve hCurve(new Geom2d_Line(origin, dir));
                faceUInd(iIso) = hatcher.AddHatching(hCurve);
                }

            uParam += stepU;
            }
        }

    if (stepV > confusion)
        {
        Standard_Real vParam = vParamStart;
        gp_Dir2d dir(1.0, 0.0);

        for (Standard_Integer iIso = 1; iIso <= numVIsos ; iIso++)
            {
            faceVParam(iIso) = vParam;

            if (!((fabs(vParam - faceVMin) < confusion) || (fabs(vParam - faceVMax) < confusion)))
                {
                gp_Pnt2d origin(0.0, vParam);
                Geom2dAdaptor_Curve hCurve(new Geom2d_Line(origin, dir));
                faceVInd(iIso) = hatcher.AddHatching(hCurve);
                }

            vParam += stepV;
            }
        }

    // Computation.
    hatcher.Trim();

    for (Standard_Integer iIso = 1; iIso <= numUIsos ; iIso++)
        {
        Standard_Integer index = faceUInd(iIso);

        if (0 != index && hatcher.TrimDone(index) && !hatcher.TrimFailed(index))
            hatcher.ComputeDomains(index);
        }

    for (Standard_Integer iIso = 1; iIso <= numVIsos ; iIso++)
        {
        Standard_Integer index = faceVInd(iIso);

        if (0 != index && hatcher.TrimDone(index) && !hatcher.TrimFailed(index))
            hatcher.ComputeDomains(index);
        }

    // Iso curve output...
    for (Standard_Integer uIso = faceUParam.Lower(); uIso <= faceUParam.Upper(); uIso++)
        {
        Standard_Integer uInd = faceUInd.Value(uIso);

        if (0 == uInd || !hatcher.IsDone(uInd))
            continue;

	    Standard_Integer nbDom = hatcher.NbDomains(uInd);

	    for (Standard_Integer iDom = 1; iDom <= nbDom; iDom++)
            {
	        HatchGen_Domain const& dom = hatcher.Domain(uInd, iDom);
	        Standard_Real v1 = dom.HasFirstPoint()  ? dom.FirstPoint().Parameter()  : faceVMin;
	        Standard_Real v2 = dom.HasSecondPoint() ? dom.SecondPoint().Parameter() : faceVMax;

            if (v2 == v1 && v1 == faceVMin)
                v2 = faceVMax; // physical closure in v...

            Geom2dAdaptor_Curve const& curve2d = hatcher.HatchingCurve(uInd);
            Handle(Geom2dAdaptor_HCurve) hCurve2dAdaptor = new Geom2dAdaptor_HCurve(curve2d.Curve(), v1, v2);
            curveOnSurf.Load(hCurve2dAdaptor);

            addIsoCurve(graphic, curveOnSurf);
            }
        }

    for (Standard_Integer vIso = faceVParam.Lower(); vIso <= faceVParam.Upper(); vIso++)
        {
        Standard_Integer vInd = faceVInd.Value(vIso);

        if (0 == vInd || !hatcher.IsDone(vInd))
            continue;
    
        Standard_Integer nbDom = hatcher.NbDomains(vInd);

        for (Standard_Integer iDom = 1; iDom <= nbDom ; iDom++)
            {
	        HatchGen_Domain const& dom = hatcher.Domain(vInd, iDom);
	        Standard_Real u1 = dom.HasFirstPoint()  ? dom.FirstPoint().Parameter()  : faceUMin;
	        Standard_Real u2 = dom.HasSecondPoint() ? dom.SecondPoint().Parameter() : faceUMax;

            if (u2 == u1 && u1 == faceUMin)
                u2 = faceUMax; // physical closure in u (ex. full sweep surface of revolution)...

            Geom2dAdaptor_Curve const& curve2d = hatcher.HatchingCurve(vInd);
            Handle(Geom2dAdaptor_HCurve) hCurve2dAdaptor = new Geom2dAdaptor_HCurve(curve2d.Curve(), u1, u2);
            curveOnSurf.Load(hCurve2dAdaptor);

            addIsoCurve(graphic, curveOnSurf);
            }
        }
    }
    
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
        // Some chord tolerance required - BRepMesh_IncrementalMesh behaves poorly/slowly otherwise...
        Bnd_Box box;
        Standard_Real maxDimension = 0.0;

        BRepBndLib::Add(shape, box);
        BRepMesh_ShapeTool::BoxMaxDimension(box, maxDimension);
        linearDeflection = (0.1 * maxDimension);
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
