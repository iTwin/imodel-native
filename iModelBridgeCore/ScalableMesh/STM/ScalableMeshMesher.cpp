//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.cpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.cpp,v $
//:>   $Revision: 1.13 $
//:>       $Date: 2011/06/27 14:53:05 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
        
           
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

/*----------------------------------------------------------------------+
| Include MicroStation SDK header files
+----------------------------------------------------------------------*/
//#include <toolsubs.h>
/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/

#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshMesher.h"
#include "ScalableMeshMesher.hpp"
         
//template class ScalableMesh2DDelaunayMesher<DPoint3d, ISMStore::Extent3d64f>;
 
//template class ScalableMesh3DDelaunayMesher<DPoint3d, ISMStore::Extent3d64f>;






template class ScalableMesh2DDelaunayMesher<DPoint3d, DRange3d>;

void ProcessFeatureDefinitions(bvector<bvector<DPoint3d>>& voidFeatures, bvector<DTMFeatureType>& types, bvector<bvector<DPoint3d>>& islandFeatures, const std::vector<DPoint3d>& nodePoints, BC_DTM_OBJ* dtmObjP, bvector<bvector<int32_t>>& featureDefs)
    {
    for (size_t i = 0; i < featureDefs.size(); ++i)
        {
        bvector<DPoint3d> feature;
        for (size_t j = 1; j < featureDefs[i].size(); ++j)
            {
            if (featureDefs[i][j] < nodePoints.size()) feature.push_back(nodePoints[featureDefs[i][j]]);
            }
        if (feature.empty()) continue;
        if ((DTMFeatureType)featureDefs[i][0] == DTMFeatureType::Void || (DTMFeatureType)featureDefs[i][0] == DTMFeatureType::Hole
            || (DTMFeatureType)featureDefs[i][0] == DTMFeatureType::BreakVoid || (DTMFeatureType)featureDefs[i][0] == DTMFeatureType::DrapeVoid)
            {

            if (featureDefs[i].size() > 4)
                {
                if (!feature.back().AlmostEqual(feature.front())) feature.push_back(feature.front());
                voidFeatures.push_back(feature);
                }
            types.push_back((DTMFeatureType)featureDefs[i][0]);
            }
        else if ((DTMFeatureType)featureDefs[i][0] == DTMFeatureType::Island)
            {
            if (!feature.back().AlmostEqual(feature.front())) feature.push_back(feature.front());
            islandFeatures.push_back(feature);
            }
		else
		    {
			DTMFeatureType type = (DTMFeatureType)featureDefs[i][0];
			bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, type, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &feature[0], (long)feature.size());
		    }
		}
    }

int AddPolygonsToDTMObject(bvector<bvector<DPoint3d>>& polygons, DTMFeatureType type, BC_DTM_OBJ* dtmObjP)
    {
    int status = DTM_SUCCESS;
    for (auto& polygon : polygons)
        {
        if (polygon.size() > 2)
            {
            if (!bsiDPoint3d_pointEqualTolerance(&polygon.front(), &polygon.back(), 1e-8)) polygon.push_back(polygon.front());
            if (type == DTMFeatureType::Void)
                {
                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());
                assert(status == SUCCESS);
                }
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, type, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());
            assert(status == SUCCESS);
            }
        }
    return status;
    }

//Limited case. If a void and an island exactly overlap, remove them both.
void PruneFeatures(bvector<bvector<DPoint3d>>& islandFeatures, bvector<bvector<DPoint3d>>& voidFeatures, bvector<DTMFeatureType> types)
    {

	bvector<bool> deleteIslands(islandFeatures.size());
	bvector<bool> deleteVoids(voidFeatures.size());
	for (auto& island : islandFeatures)
	{
		ICurvePrimitivePtr curvePtr;
		CurveVectorPtr curveVectorPtr;

		curvePtr = ICurvePrimitive::CreateLineString(island);
		curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);

		for (auto& hole : voidFeatures)
	    {

			bool allInsidePolygon = true;
			for (size_t i = 0; i < hole.size() && allInsidePolygon; ++i)
			{
				auto classif = curveVectorPtr->PointInOnOutXY(hole[i]);
				if (classif == CurveVector::InOutClassification::INOUT_Out) allInsidePolygon = false;
			}

			if (allInsidePolygon)
			{

				bool allInsideVoid= true;
				ICurvePrimitivePtr curvePtrVoid;
				CurveVectorPtr curveVectorPtrVoid;

				curvePtrVoid = ICurvePrimitive::CreateLineString(hole);
				curveVectorPtrVoid = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtrVoid);
				for (size_t i = 0; i < island.size() && allInsidePolygon; ++i)
				{
					auto classif = curveVectorPtrVoid->PointInOnOutXY(island[i]);
					if (classif == CurveVector::InOutClassification::INOUT_Out) allInsideVoid = false;
				}
				deleteIslands[&island - &islandFeatures[0]] = true;
				deleteVoids[&hole - &voidFeatures[0]] = true;
			}
				
        }

		}


	bvector<bvector<DPoint3d>> newIslands;
	bvector<bvector<DPoint3d>> newVoids;
	bvector<DTMFeatureType> newTypes;
	for (auto& island : islandFeatures)
	{
		if (!deleteIslands[&island - &islandFeatures[0]])
			newIslands.push_back(island);
	}

	for (auto& hole : voidFeatures)
	{
		if (!deleteVoids[&hole - &voidFeatures[0]])
		{
			newVoids.push_back(hole);
			newTypes.push_back(types[&hole - &voidFeatures[0]]);
		}
	}

	islandFeatures.swap(newIslands);
	voidFeatures.swap(newVoids);
	types.swap(newTypes);
    }

int AddIslandsToDTMObject(bvector<bvector<DPoint3d>>& islandFeatures, bvector<bvector<DPoint3d>>& voidFeatures, bvector<bvector<DPoint3d>>& boundary, BC_DTM_OBJ* dtmObjP)
    {
    VuPolygonClassifier vu(1e-8, 0);
    int status = DTM_SUCCESS;
	ICurvePrimitivePtr curvePtr;
	CurveVectorPtr curveVectorPtr;
	if (!boundary.empty())
	{
		curvePtr = ICurvePrimitive::CreateLineString(boundary.front());
		curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
	}

    for (auto& island : islandFeatures)
        {
        bool intersect = false;
		if (!boundary.empty())
		{
			bool allInsidePolygon = true;
			for (size_t i = 0; i < island.size() && allInsidePolygon; ++i)
			{
				auto classif = curveVectorPtr->PointInOnOutXY(island[i]);
				if (classif == CurveVector::InOutClassification::INOUT_Out) allInsidePolygon = false;
			}

			if (allInsidePolygon)
				continue;
		}
        for (auto& hole : voidFeatures)
            {
            vu.ClassifyAIntersectB(island, hole);
            bvector<DPoint3d> xyz;
            for (; vu.GetFace(xyz);)
                {
                intersect = true;
				if (!boundary.empty())
				{
					bool allInsidePolygon = true;
					for (size_t i = 0; i < xyz.size() && allInsidePolygon; ++i)
					{
						auto classif = curveVectorPtr->PointInOnOutXY(xyz[i]);
						if (classif == CurveVector::InOutClassification::INOUT_Out) allInsidePolygon = false;
					}

					if (allInsidePolygon)
						continue;
				}
                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Island, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(xyz[0]), (long)xyz.size());
                assert(status == SUCCESS);
                }

            }
        if (!intersect)
            {
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Island, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(island[0]), (long)island.size());
            
            }
        }
    return status;
    }

void AddLoopsFromShape(bvector<bvector<DPoint3d>>& polygons, const HGF2DShape* shape, std::function<void(const bvector<DPoint3d>& element)> afterPolygonAdded)
    {

    if (shape->IsComplex())
        {
        for (auto& elem : shape->GetShapeList())
            {
            AddLoopsFromShape(polygons, elem, afterPolygonAdded);
            }
        }
    else if (!shape->IsEmpty())
        {
        HGF2DPositionCollection thePoints;
        shape->Drop(&thePoints, shape->GetTolerance());

        bvector<DPoint3d> vec(thePoints.size());

        for (size_t idx = 0; idx < thePoints.size(); idx++)
            {
            vec[idx].x = thePoints[idx].GetX();
            vec[idx].y = thePoints[idx].GetY();
            vec[idx].z = 0; // As mentionned below the Z is disregarded
            }

        polygons.push_back(vec);
        afterPolygonAdded(vec);
        }
    }

void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons)
    {
    return MergePolygonSets(polygons, [] (const size_t i, const bvector<DPoint3d>& element)
        {
        return true;
        }, [] (const bvector<DPoint3d>&element) {});
    }

void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons, std::function<bool(const size_t i, const bvector<DPoint3d>& element)> choosePolygonInSet, std::function<void(const bvector<DPoint3d>& element)> afterPolygonAdded)
    {
    bvector<bvector<DPoint3d>> newUnifiedPoly;
    HFCPtr<HGF2DCoordSys>   coordSysPtr(new HGF2DCoordSys());
    HFCPtr<HVEShape> allPolyShape = new HVEShape(coordSysPtr);
    bvector<bool> used(polygons.size(),false);

	bvector<bool> available(polygons.size(), false);
	for (auto& poly : polygons)
	{
		available[&poly - &polygons.front()] = choosePolygonInSet(&poly - &polygons.front(), poly);
	}

	//Apparently, intersection on a single vertex, even though it has no bearing on the "inside" section of voids, trips up the Civil triangulation.
	//So we find out and disconnect single vertex intersections first, since they cannot be unified.
	for (auto& poly : polygons)
	{
		if (!available[&poly-&polygons.front()]) continue;
		DRange3d range = DRange3d::From(poly);
		if (poly.empty()) continue;
		bvector<DPoint3d> poly_2d = poly;
		for (auto&pt : poly_2d) pt.z = 0;
		for (auto& poly2 : polygons)
		{
			if (!available[&poly2 - &polygons.front()]) continue;
			if (&poly == &poly2) continue;
			if (poly2.empty()) continue;
			if (!DRange3d::From(poly2).IntersectsWith(range)) continue;
			bvector<DPoint3d> poly2_2d = poly2;
			for (auto&pt : poly2_2d) pt.z = 0;

			//There are cases where the clash functions on non-coplanar 3d polygons says 2 polygons which share a vertex don't clash.
			if (bsiDPoint3dArray_polygonClashXYZ(&poly.front(), (int)poly.size(), &poly2.front(), (int)poly2.size()) ||
				bsiDPoint3dArray_polygonClashXYZ(&poly_2d.front(), (int)poly_2d.size(), &poly2_2d.front(), (int)poly2_2d.size()))
			{
				VuPolygonClassifier vu(1e-8, 0);
				vu.ClassifyAUnionB(poly, poly2);
				bvector<DPoint3d> xyz;
				bvector<bvector<DPoint3d>> faces;
				for (; vu.GetFace(xyz);)
				{
					if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
					else
					{
						//  postFeatureBoundary.push_back(xyz);
						faces.push_back(xyz);

					}

				}
				if (faces.size() == 1)
					continue;
				//compute intersects on single vertices
				bmap<DPoint3d, size_t, DPoint3dZYXTolerancedSortComparison> setOfPts(DPoint3dZYXTolerancedSortComparison(1e-8, 0));
				bvector<DPoint3d> intersectingVertices;
				bvector<bpair<bpair<DSegment3d, DSegment3d>, bpair<DSegment3d, DSegment3d>>> intersectingContext;
				int minConsecutiveIntersectingVertices = INT_MAX;
				int consecutiveIntersectingVertices = 0;
				int nPtsSeen = 0;
				int loopNPts = 0;
				for (auto pt : poly)
				{
					pt.z = 0;
					setOfPts.insert(make_bpair(pt, &pt - &poly[0]));
				}
				for (auto& pt : poly2)
				{
					++nPtsSeen;
					DPoint3d pt2d = pt;
					pt2d.z = 0;
					if (setOfPts.count(pt2d))
					{
						DSegment3d lastSegOn1, nextSegOn1, lastSegOn2, nextSegOn2;
						lastSegOn1 = DSegment3d::From(setOfPts[pt2d] == 0 ? poly[poly.size() - 2] : poly[setOfPts[pt2d] - 1], poly[setOfPts[pt2d]]);
						nextSegOn1 = DSegment3d::From(poly[setOfPts[pt2d]], setOfPts[pt2d] == poly.size() - 1 ? poly[1] : poly[setOfPts[pt2d] + 1]);
						lastSegOn2 = DSegment3d::From(nPtsSeen == 1 ? poly2[poly2.size() - 2] : poly2[nPtsSeen - 2], poly2[nPtsSeen - 1]);
						nextSegOn2 = DSegment3d::From(poly2[nPtsSeen - 1], nPtsSeen == poly2.size() ? poly2[1] : poly2[nPtsSeen]);

						intersectingVertices.push_back(pt);
						intersectingContext.push_back(make_bpair(make_bpair(lastSegOn1, nextSegOn1), make_bpair(lastSegOn2, nextSegOn2)));
						consecutiveIntersectingVertices++;
					}
					else
					{
						if (nPtsSeen == 2)
						{
							loopNPts = consecutiveIntersectingVertices;
						}
						else if (nPtsSeen != 2 && consecutiveIntersectingVertices > 0)
							minConsecutiveIntersectingVertices = std::min(consecutiveIntersectingVertices, minConsecutiveIntersectingVertices);
						if (consecutiveIntersectingVertices > 1)
						{
							intersectingVertices.resize(intersectingVertices.size() - consecutiveIntersectingVertices);
							intersectingContext.resize(intersectingContext.size() - consecutiveIntersectingVertices);
						}
						consecutiveIntersectingVertices = 0;
					}
				}

				if (loopNPts != 0)
				{
					consecutiveIntersectingVertices += loopNPts - 1;
					if (consecutiveIntersectingVertices > 0)
						minConsecutiveIntersectingVertices = std::min(consecutiveIntersectingVertices, minConsecutiveIntersectingVertices);
					consecutiveIntersectingVertices = 0;
				}

				//No single vertex intersection
				if (minConsecutiveIntersectingVertices > 1) continue;
				if (!intersectingVertices.empty())
				{
					size_t nColinear = 0;
					for (size_t i = 0; i < intersectingVertices.size(); ++i)
					{
						std::vector<DPoint3d> pts = { intersectingContext[i].first.first.point[0],intersectingContext[i].first.first.point[1], intersectingContext[i].second.first.point[0] };
						if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
						{
							nColinear++;
							continue;
						}
						pts = { intersectingContext[i].first.first.point[0],intersectingContext[i].first.first.point[1], intersectingContext[i].second.second.point[1] };
						if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
						{
							nColinear++;
							continue;
						}
						pts = { intersectingContext[i].first.second.point[0],intersectingContext[i].first.second.point[1], intersectingContext[i].second.first.point[0] };
						if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
						{
							nColinear++;
							continue;
						}
						pts = { intersectingContext[i].first.second.point[0],intersectingContext[i].first.second.point[1], intersectingContext[i].second.second.point[1] };
						if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
						{
							nColinear++;
							continue;
						}

					}
					if (nColinear == intersectingVertices.size())
						continue;
					bvector<DPoint3d> withoutIntersect;
					if (poly.size() < poly2.size())
					{
						for (auto& pt : poly)
						{
							bool insert = true;
							for (auto& ptB : intersectingVertices)
								if (bsiDPoint3d_pointEqualTolerance(&pt, &ptB, 1e-8)) insert = false;
							if (insert) withoutIntersect.push_back(pt);
						}
					}
					else
					{
						for (auto& pt : poly2)
						{
							bool insert = true;
							for (auto& ptB : intersectingVertices)
								if (bsiDPoint3d_pointEqualTolerance(&pt, &ptB, 1e-8)) insert = false;
							if (insert) withoutIntersect.push_back(pt);
						}
					}
					if (poly.size() < poly2.size())
					{
						poly = poly2;
						poly_2d = poly2_2d;
						range = DRange3d::From(poly);
					}
					if (!withoutIntersect.empty() && !bsiDPoint3d_pointEqualTolerance(&withoutIntersect.front(), &withoutIntersect.back(), 1e-8)) withoutIntersect.push_back(withoutIntersect.front());
					if (withoutIntersect.size() > 4)
					{
						poly2 = withoutIntersect;
					}
					else poly2.clear();

				}
			}
		}
	}

    for (auto& poly : polygons)
        {
		if (!available[&poly - &polygons.front()]) continue;
        if (used[&poly - &polygons[0]]) continue;
		if (poly.empty()) continue;
		bvector<DPoint3d> poly_2d = poly;
		for (auto&pt : poly_2d) pt.z = 0;

		//pre-compute the union of polys with this function because apparently sometimes Unify hangs
		for (auto& poly2 : polygons)
		{
			if (!available[&poly2 - &polygons.front()]) continue;
			if (&poly == &poly2) continue;
			if (poly2.empty()) continue;
			if (used[&poly2 - &polygons[0]]) continue;

			bvector<DPoint3d> poly2_2d = poly2;
			for (auto&pt : poly2_2d) pt.z = 0;

			if (bsiDPoint3dArray_polygonClashXYZ(&poly.front(), (int)poly.size(), &poly2.front(), (int)poly2.size())
				|| bsiDPoint3dArray_polygonClashXYZ(&poly_2d.front(), (int)poly_2d.size(), &poly2_2d.front(), (int)poly2_2d.size()))
                {
				VuPolygonClassifier vu(1e-8, 0);
                vu.ClassifyAUnionB(poly, poly2);
                bvector<DPoint3d> xyz;
				bvector<bvector<DPoint3d>> faces;
                for (; vu.GetFace(xyz);)
                    {
                    if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
                    else
                        {
                        //  postFeatureBoundary.push_back(xyz);
						faces.push_back(xyz);

                        }

                    }
				if (faces.size() == 1)
				    {
					poly = faces.front();
					used[&poly2 - &polygons[0]] = true;
				    }
			/*	else
				   {
					//compute intersects on vertices
					bset<DPoint3d, DPoint3dZYXTolerancedSortComparison> setOfPts(DPoint3dZYXTolerancedSortComparison(1e-8, 0));
					bvector<DPoint3d> intersectingVertices;
					for (auto& pt : poly)
						setOfPts.insert(pt);
					for (auto& pt : poly2)
						if (setOfPts.count(pt))
							intersectingVertices.push_back(pt);
					if (!intersectingVertices.empty())
					{
						bvector<DPoint3d> withoutIntersect;
						if (poly.size() < poly2.size())
						{
							for (auto& pt : poly)
							{
								bool insert = true;
								for (auto& ptB : intersectingVertices)
									if (bsiDPoint3d_pointEqualTolerance(&pt, &ptB, 1e-8)) insert = false;
								if(insert) withoutIntersect.push_back(pt);
							}
						}
						else
						{
							for (auto& pt : poly2)
							{
								bool insert = true;
								for (auto& ptB : intersectingVertices)
									if (bsiDPoint3d_pointEqualTolerance(&pt, &ptB, 1e-8)) insert = false;
								if (insert) withoutIntersect.push_back(pt);
							}
						}
						if (poly.size() < poly2.size()) poly = poly2;

						if (!withoutIntersect.empty() && !bsiDPoint3d_pointEqualTolerance(&withoutIntersect.front(), &withoutIntersect.back(), 1e-8)) withoutIntersect.push_back(withoutIntersect.front());
						if (withoutIntersect.size() > 4)
						{
							poly2 = withoutIntersect;
						}
						else used[&poly2 - &polygons[0]] = true;
					}
			        }*/
                }

            }
        }

    //keep unique resulting polygons
    for (auto& poly : polygons)
    {
        if (!available[&poly - &polygons.front()]) continue;
        if (used[&poly - &polygons[0]]) continue;
        if (poly.empty()) continue;
        for (auto& poly2 : polygons)
        {
            if (!available[&poly2 - &polygons.front()]) continue;
            if (&poly == &poly2) continue;
            if (used[&poly2 - &polygons[0]]) continue;
            if (poly2.empty()) continue;
            
            if (poly2.size() != poly.size()) continue;
            size_t i = 0;
            for (i = 0; i < poly.size(); ++i)
                if (!bsiDPoint3d_pointEqualTolerance(&poly[i], &poly2[i], 1e-8))
                    break;
            if (i == poly.size())
                used[&poly2 - &polygons[0]] = true;
        }
    }

    for (auto& poly : polygons)
        {
		if (!available[&poly - &polygons.front()]) continue;
        if (used[&poly - &polygons[0]]) continue;
		if (poly.empty()) continue;

        UntieLoopsFromPolygon(poly);
        HArrayAutoPtr<double> tempBuffer(new double[poly.size() * 2]);

        int bufferInd = 0;

        for (size_t pointInd = 0; pointInd < poly.size(); pointInd++)
            {
            tempBuffer[bufferInd * 2] = poly[pointInd].x;
            tempBuffer[bufferInd * 2 + 1] = poly[pointInd].y;
            bufferInd++;
            }
        HVE2DPolygonOfSegments polygon(poly.size() * 2, tempBuffer, coordSysPtr);

        HFCPtr<HVEShape> subShapePtr = new HVEShape(polygon);
        allPolyShape->Unify(*subShapePtr);
        }

    AddLoopsFromShape(newUnifiedPoly, allPolyShape->GetLightShape(), afterPolygonAdded);
    polygons = newUnifiedPoly;
    }

void circumcircle(DPoint3d& center, double& radius, const DPoint3d* triangle)
    {
    double det = RotMatrix::FromRowValues(triangle[0].x, triangle[0].y, 1, triangle[1].x, triangle[1].y, 1, triangle[2].x, triangle[2].y, 1).Determinant();
    double mags[3] = { triangle[0].MagnitudeSquaredXY(),
        triangle[1].MagnitudeSquaredXY(),
        triangle[2].MagnitudeSquaredXY() };
    double det1 = 0.5*(RotMatrix::FromRowValues(mags[0], triangle[0].y, 1, mags[1], triangle[1].y, 1, mags[2], triangle[2].y, 1).Determinant());
    double det2 = 0.5*RotMatrix::FromRowValues(triangle[0].x, mags[0], 1, triangle[1].x, mags[1], 1, triangle[2].x, mags[2], 1).Determinant();
    center.x = det1 / det;
    center.y = det2 / det;
    double detR = RotMatrix::FromRowValues(triangle[0].x, triangle[0].y, mags[0], triangle[1].x, triangle[1].y, mags[1], triangle[2].x, triangle[2].y, mags[2]).Determinant();
    radius = sqrt(detR / det + DPoint3d::From(det1, det2, 0).MagnitudeSquaredXY() / (det*det));
    }