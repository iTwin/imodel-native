/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"

#include <set>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct PointWithId
    {
    DPoint3d m_point;
    size_t m_id;
    uint64_t m_polyID;
    };

struct PolylineArea
    {
    mutable double m_area;
    mutable double m_edgeSquaredLength;
    std::list<PointWithId>::iterator m_listItr;
    uint64_t m_key;
    double m_maxLength;

    explicit PolylineArea(std::list<PointWithId>::iterator ptPtr, const double& tolerance) : m_listItr(ptPtr), m_maxLength(tolerance)
        {
        m_area = ComputeArea();
        m_edgeSquaredLength = ComputeEdgeSquaredLength();
        m_key = (uint64_t)m_listItr->m_id | (uint64_t)(m_listItr->m_polyID << 32);
        }
    PolylineArea(const PolylineArea& other) 
        : m_listItr(other.m_listItr), 
        m_area(other.m_area), 
        m_edgeSquaredLength(other.m_edgeSquaredLength), 
        m_key(other.m_key), 
        m_maxLength(other.m_maxLength) 
        {}

    double ComputeArea() const
        {
        auto left = m_listItr; --left;
        auto right = m_listItr; ++right;
        double dX0 = left->m_point.x, dY0 = left->m_point.y;
        double dX1 = m_listItr->m_point.x, dY1 = m_listItr->m_point.y;
        double dX2 = right->m_point.x, dY2 = right->m_point.y;
        double dArea = ((dX1 - dX0)*(dY2 - dY0) - (dX2 - dX0)*(dY1 - dY0)) / 2.0;
        return (dArea > 0.0) ? dArea : -dArea;
        }
    double ComputeEdgeSquaredLength()
        {
        auto left = m_listItr; --left;
        auto right = m_listItr; ++right;
        return (DSegment3d::From(right->m_point, left->m_point).LengthSquared());
        }
    bool operator <(PolylineArea const& other) const
        {
        if(m_key == other.m_key)
            return false;
        if(m_edgeSquaredLength < m_maxLength && other.m_edgeSquaredLength < other.m_maxLength)
            return (m_area < other.m_area) || (m_area == other.m_area && m_key < other.m_key);
        else if(other.m_edgeSquaredLength < other.m_maxLength)
            return false;
        else if(m_edgeSquaredLength < m_maxLength)
            return true;
        return (m_area < other.m_area) || (m_area == other.m_area && m_key < other.m_key);
        //return (m_edgeSquaredLength < other.m_edgeSquaredLength || m_edgeSquaredLength == other.m_edgeSquaredLength && m_key < other.m_key);
        }
    };

struct priority_queue_unique : public std::set<PolylineArea>
    {
    void push(const PolylineArea& item)
        {
        std::pair<std::set<PolylineArea>::iterator, bool> ret = insert(item);
        if(!ret.second)
            {
            // Item already exists, update area
            PolylineArea newItem(ret.first->m_listItr, item.m_maxLength);
            erase(ret.first);
            insert(newItem);
            }
        }

    PolylineArea top()
        {
        return *begin();
        }

    void pop()
        {
        erase(begin());
        }
    };

double HausdorffDistance(const bvector<DPoint3d>& A, const bvector<DPoint3d>& B)
    {
    double cmax = 0.0;
    for(auto const& pA : A)
        {
        double cmin = std::numeric_limits<double>::max();
        for(size_t i = 0; i < B.size()-1; i++)
            {
            DSegment3d segment = DSegment3d::From(B[i], B[i+1]);
            double param;
            DPoint3d closestPt;
            segment.ProjectPointXY(closestPt, param, pA);

            double distance = pA.DistanceSquaredXY(closestPt);
            if(distance <= cmax)
                {
                cmin = 0.0;
                break;
                }
            cmin = std::min(cmin, distance);
            }
        cmax = std::max(cmax, cmin);
        }
    return std::sqrt(cmax);
    }

void SimplifyPolylines(bvector<bvector<DPoint3d>>& polylines, bvector<DTMFeatureType>& types, bvector<DPoint3d>& removedPoints, double distanceTol, size_t targetNumPoints)
    {
    auto isClosedFeatureType = [](DTMFeatureType type) -> bool
        { 
        bool isSpecialHullType = (uint32_t(type) >> 16) > 0;
        return (type == DTMFeatureType::Hole || type == DTMFeatureType::Island || type == DTMFeatureType::Void || type == DTMFeatureType::BreakVoid ||
            type == DTMFeatureType::Polygon || type == DTMFeatureType::Region || type == DTMFeatureType::Contour || type == DTMFeatureType::Hull ||
            type == DTMFeatureType::TinHull || type == DTMFeatureType::DrapeVoid || 
                isSpecialHullType);
        };
    // Filter lines that are too close to each other
    for(int i = 0; i < polylines.size() - 1; i++)
        {
        if(!isClosedFeatureType(types[i]) && !polylines[i].empty())
            {
            for(int j = i + 1; j < polylines.size(); j++)
                {
                if(!isClosedFeatureType(types[j]) && !polylines[j].empty())
                    {
                    if(distanceTol >= HausdorffDistance(polylines[i], polylines[j]))
                        {
                        for(auto pt : polylines[i]) removedPoints.push_back(pt);
                        polylines[i].clear();
                        if(removedPoints.size() >= targetNumPoints) return;
                        break;
                        }
                    else if(distanceTol >= HausdorffDistance(polylines[j], polylines[i]))
                        {
                        for(auto pt : polylines[j]) removedPoints.push_back(pt);
                        polylines[j].clear();
                        if(removedPoints.size() >= targetNumPoints) return;
                        }
                    }
                }
            }
        }

    // Update target to reflect filtered features in the previous step
    targetNumPoints -= removedPoints.size();

    priority_queue_unique pointQueue;
    bvector<std::list<PointWithId>> polyline2(polylines.size());
    for(auto& polyline : polylines)
        {
        auto polyIdx = &polyline - &polylines.front();
        if(polyline.empty()) 
            continue;
        if(types[polyIdx] == DTMFeatureType::Hull || types[polyIdx] == DTMFeatureType::TinHull || ((uint32_t)types[polyIdx] >> 16) == 1)
            continue; // Don't simplify hulls, they are too important
        bool isLoop = polyline.front().IsEqual(polyline.back(), 1.e-5);
        if(isLoop && polyline.size() < 5) continue;
        else if(polyline.size() < 3) continue;

        auto& poly2 = polyline2[&polyline - &polylines.front()];
        for(size_t i = 0; i < polyline.size(); ++i)
            poly2.push_back({ polyline[i], i , (uint64_t)(&polyline - &polylines.front()) });

        for(auto it = ++poly2.begin(); it != --poly2.end(); ++it)
            {
            pointQueue.push(PolylineArea(it, distanceTol));
            }
        }

    while(pointQueue.size() > targetNumPoints)
        {
        auto topItem = pointQueue.top();

        // remove point from polyline and update areas of adjacent points
        auto current = topItem.m_listItr;
        removedPoints.push_back(current->m_point);

        auto previous = current; --previous;
        PolylineArea previousPolylineArea(previous, distanceTol);
        auto next = current; ++next;
        PolylineArea nextPolylineArea(next, distanceTol);

        auto currentPolyID = current->m_polyID;

        bool canRemovePoint =   (!isClosedFeatureType(types[current->m_polyID]) && polyline2[current->m_polyID].size() > 2) ||
                                (polyline2[current->m_polyID].size() > 4);
        if (canRemovePoint)
            polyline2[current->m_polyID].erase(current);

        pointQueue.pop();

        // Only update neighbors if the point has been removed
        if(canRemovePoint)
            {
            if(previous->m_id > 0) pointQueue.push(previousPolylineArea);
            if(next->m_id < polylines[currentPolyID].size() - 1) pointQueue.push(nextPolylineArea);
            }
        }

    for(auto& polyline : polylines)
        {
        if(polyline.empty()) continue;
        bool isSpecialHullType = (uint32_t(types[&polyline - &polylines.front()]) >> 16) > 0;

        if(types[&polyline - &polylines.front()] == DTMFeatureType::Hull || types[&polyline - &polylines.front()] == DTMFeatureType::TinHull || isSpecialHullType)
            continue;
        polyline.clear();

        for (auto pt : polyline2[&polyline - &polylines.front()])
            polyline.push_back(pt.m_point);

        }
    
    // // boost uses Douglas-Peucker algorithm to simplify a polyline. This algorithm requires a tolerance
    // // parameter which is not well suited for our need to target a specified number of points.
    //typedef boost::geometry::model::d2::point_xy<double> xy;
    //for (auto& polyline : polylines)
    //    {
    //    typedef std::map<DPoint3d, double, DPoint3dYXTolerancedSortComparison> MapOfPoints;
    //    MapOfPoints pointElevationMap(DPoint3dYXTolerancedSortComparison(1e-5));
    //    DRange3d ext = DRange3d::From(polyline);
    //    boost::geometry::model::linestring<xy> line;
    //    for(auto&pt : polyline)
    //        {
    //        if(pointElevationMap.count(pt) == 0) pointElevationMap.insert(std::make_pair(pt, pt.z));
    //        line.push_back(xy(pt.x, pt.y));
    //        }
    //
    //        boost::geometry::model::linestring<xy> simplified;
    //    boost::geometry::simplify(line, simplified, 0.5);
    //
    //    polyline.clear();
    //    for(auto& pt : simplified)
    //        {
    //        DPoint3d newPoint = DPoint3d::From(pt.x(), pt.y(), 0);
    //        newPoint = DPoint3d::From(newPoint.x, newPoint.y, pointElevationMap[newPoint]);
    //        polyline.push_back(newPoint);
    //        }
    //    }

    }
END_BENTLEY_SCALABLEMESH_NAMESPACE
