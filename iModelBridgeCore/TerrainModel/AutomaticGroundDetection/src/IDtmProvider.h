/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/IDtmProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/NonCopyableClass.h>
#include "DiscreetHistogram.h"
#include <TerrainModel/AutomaticGroundDetection/IGroundDetectionServices.h>

BEGIN_GROUND_DETECTION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct Triangle
    {
    static const double TOLERANCE_FACTOR;

    public:
        Triangle()
            {
            //Arbitrary values
            m_point[0] = { 0.0, 1.0, 0.0};
            m_point[1] = { 1.0, 0.0, 0.0 };
            m_point[2] = { 1.0, 1.0, 0.0 };
            //Compute centroid
            m_centroid.x = (m_point[0].x + m_point[1].x + m_point[2].x) / 3.0;
            m_centroid.y = (m_point[0].y + m_point[1].y + m_point[2].y) / 3.0;
            m_centroid.z = (m_point[0].z + m_point[1].z + m_point[2].z) / 3.0;
            m_plane = DPlane3d::From3Points(m_point[0], m_point[1], m_point[2]);
            if (m_plane.normal.z < 0)
                m_plane.normal.Negate();

            m_range = DRange3d::From(m_point, 3);
            }

        Triangle(DPoint3d const& pt1, DPoint3d const& pt2, DPoint3d const& pt3)
            {
            m_point[0] = pt1;
            m_point[1] = pt2;
            m_point[2] = pt3;
            //Compute centroid
            m_centroid.x = (m_point[0].x + m_point[1].x + m_point[2].x) / 3.0;
            m_centroid.y = (m_point[0].y + m_point[1].y + m_point[2].y) / 3.0;
            m_centroid.z = (m_point[0].z + m_point[1].z + m_point[2].z) / 3.0;
            m_plane = DPlane3d::From3Points(m_point[0], m_point[1], m_point[2]);
            if (m_plane.normal.z < 0)
                m_plane.normal.Negate();
            m_range = DRange3d::From(m_point, 3);
            }
        ~Triangle() {}

        DPoint3d const& GetCentroid() const        { return m_centroid; }

        bool IsEqualToOneCoordinate(DPoint3d const& point) const
            {
            if (point.IsEqual(m_point[0], TOLERANCE_FACTOR) ||
                point.IsEqual(m_point[1], TOLERANCE_FACTOR) ||
                point.IsEqual(m_point[2], TOLERANCE_FACTOR))
                return true;
            return false;
            }

        bool IsPointOnPlaneInside(DPoint3d pointOnPlane, bool strictlyInside) const;

        DPoint3d const& GetPoint(short index) const      { if (index>=0 && index < 3) return m_point[index]; assert(!"Bad index"); return m_point[0];}
        DPlane3d   GetPlane() const                 { return m_plane; }
        DRange3d   GetDRange3d() const              { return m_range; }
    private:
        DPoint3d m_point[3];
        DPoint3d m_centroid;
        DPlane3d m_plane;
        DRange3d m_range;
    };

/*---------------------------------------------------------------------------------**//**
* IDtmProvider
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Impl> struct IDtmProviderIterator : std::iterator<std::forward_iterator_tag, typename Impl::ReturnType>
    {
    private:
        RefCountedPtr<Impl> m_impl;

    public:
        IDtmProviderIterator() {}
        IDtmProviderIterator(Impl& state) : m_impl(&state) {}

        typename Impl::ReturnType& operator*() const { return m_impl->GetCurrent(); }
        typename Impl::ReturnType* operator->() const { return &m_impl->GetCurrent(); }

        bool        operator==(IDtmProviderIterator const& rhs) const { return !(*this != rhs); }
        bool        operator!=(IDtmProviderIterator const& rhs) const
            {
            bool leftIsNull = m_impl.IsNull();
            bool rightIsNull = rhs.m_impl.IsNull();

            if (leftIsNull && rightIsNull)
                return false;

            if (!leftIsNull && !rightIsNull)
                return m_impl->IsDifferent (*rhs.m_impl.get());

            bool leftAtEnd =  leftIsNull || m_impl->IsAtEnd();
            bool rightAtEnd =  rightIsNull || rhs.m_impl->IsAtEnd();

            return leftAtEnd != rightAtEnd;
            }

        IDtmProviderIterator&   operator++()
            {
            m_impl->MoveToNext();

            return *this;
            }

        Impl* GetImpl() { return m_impl.get(); }
    };


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     12/2015
+===============+===============+===============+===============+===============+======*/
struct IDtmProvider : public RefCountedBase, NonCopyableClass
{

//__PUBLISH_CLASS_VIRTUAL__
public:
    //=======================================================================================
    //! @bsiclass
    //===============+===============+===============+===============+===============+=======
    struct      IDtmProviderIteratorImpl : public RefCountedBase, NonCopyableClass
        {
        public:
            friend IDtmProvider;
            typedef Triangle ReturnType;

        protected:
            IDtmProviderIteratorImpl() {}
            ~IDtmProviderIteratorImpl() {}

            virtual bool          _IsDifferent(IDtmProviderIteratorImpl const& rhs) const=0;
            virtual void          _MoveToNext()=0;
            virtual ReturnType&   _GetCurrent() const=0;
            virtual bool          _IsAtEnd() const=0;


        public:
            bool          IsDifferent(IDtmProviderIteratorImpl const& rhs) const { return _IsDifferent(rhs); }
            void          MoveToNext() { return _MoveToNext(); }
            ReturnType&   GetCurrent() const { return _GetCurrent(); }
            bool          IsAtEnd() const { return _IsAtEnd(); }
        };

    typedef IDtmProviderIterator<IDtmProviderIteratorImpl> const_iterator;

    const_iterator begin() const { return _begin(); }
    const_iterator end() const { return _end(); }
    size_t         size() const { return _size(); }

    size_t      GetTriangleCount() const { return _GetTriangleCount(); }
     size_t     GetPointCount() const { return _GetPointCount(); }
     void       AddPoint(DPoint3d const& point) { return _AddPoint(point); }
     long       ComputeTriangulation() { return _ComputeTriangulation(); }
    bool        FindNearestTriangleDistanceFromPoint(Triangle* pTri, double& distance, DPoint3d const& point) const { return _FindNearestTriangleDistanceFromPoint(pTri, distance, point); }
    void        ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats) { return _ComputeStatisticsFromDTM(angleStats, heightStats); }     
     StatusInt  GetDTMPoints(DPoint3d* pPoints) const { return _GetDTMPoints(pPoints); }

     

protected:
    virtual const_iterator _begin() const=0;
    virtual const_iterator _end() const=0;
    virtual size_t         _size() const=0;

    virtual size_t      _GetTriangleCount() const = 0;
    virtual size_t      _GetPointCount() const = 0;
    virtual void        _AddPoint(DPoint3d const& point) = 0;
    virtual long        _ComputeTriangulation() = 0;
    virtual bool        _FindNearestTriangleDistanceFromPoint(Triangle* pTri, double& distance, DPoint3d const& point) const = 0;
    virtual void        _ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats) const = 0;    
    virtual StatusInt   _GetDTMPoints(DPoint3d* pPoints) const = 0;
}; // IDtmProvider

END_GROUND_DETECTION_NAMESPACE
