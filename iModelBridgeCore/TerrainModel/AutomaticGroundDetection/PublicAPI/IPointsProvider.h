/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/NonCopyableClass.h>
#include <Bentley/BeFileName.h>

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>

GROUND_DETECTION_TYPEDEF(IPointsProvider)
GROUND_DETECTION_TYPEDEF(IPointsProviderCreator)


BEGIN_GROUND_DETECTION_NAMESPACE
    
/*---------------------------------------------------------------------------------**//**
* IPointsProviderIterator
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Impl> struct IPointsProviderIterator : std::iterator<std::forward_iterator_tag, typename Impl::ReturnType>
    {
    private:
        RefCountedPtr<Impl> m_impl;

    public:
        IPointsProviderIterator() {}
        IPointsProviderIterator(Impl& state) : m_impl(&state) {}

        typename Impl::ReturnType& operator*() const { return m_impl->GetCurrent(); }
        typename Impl::ReturnType* operator->() const { return &m_impl->GetCurrent(); }

        bool        operator==(IPointsProviderIterator const& rhs) const { return !(*this != rhs); }
        bool        operator!=(IPointsProviderIterator const& rhs) const
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

        IPointsProviderIterator&   operator++()
            {
            m_impl->MoveToNext();

            return *this;
            }

        Impl* GetImpl() { return m_impl.get(); }
    };


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     12/2015
+===============+===============+===============+===============+===============+======*/
struct IPointsProvider : public RefCountedBase
{
static const uint32_t DATA_QUERY_BUFFER_SIZE;

public : 
  //=======================================================================================
    //! @bsiclass
    //===============+===============+===============+===============+===============+=======
    struct      IPointsProviderIteratorImpl : public RefCountedBase, NonCopyableClass
        {
        public:
            friend IPointsProvider;
            typedef DPoint3d ReturnType;

        protected:
            IPointsProviderIteratorImpl() {}
            ~IPointsProviderIteratorImpl() {}

            virtual bool          _IsDifferent(IPointsProviderIteratorImpl const& rhs) const=0;
            virtual void          _MoveToNext()=0;
            virtual ReturnType&   _GetCurrent() const=0;
            virtual bool          _IsAtEnd() const=0;


        public:
            bool          IsDifferent(IPointsProviderIteratorImpl const& rhs) const { return _IsDifferent(rhs); }
            void          MoveToNext() { return _MoveToNext(); }
            ReturnType&   GetCurrent() const { return _GetCurrent(); }
            bool          IsAtEnd() const { return _IsAtEnd(); }
        };

    typedef IPointsProviderIterator<IPointsProviderIteratorImpl> const_iterator;

protected:
    GROUND_DETECTION_EXPORT IPointsProvider(DRange3d const& boundingBoxInUors);
    GROUND_DETECTION_EXPORT IPointsProvider(IPointsProvider const& object);
    GROUND_DETECTION_EXPORT IPointsProvider();

    GROUND_DETECTION_EXPORT size_t          ComputeMemorySize() const { return _ComputeMemorySize(); }

    GROUND_DETECTION_EXPORT virtual IPointsProviderPtr _Clone() const=0;
    GROUND_DETECTION_EXPORT virtual DRange3d        _GetBoundingBox() const;
    GROUND_DETECTION_EXPORT virtual void            _SetBoundingBox(DRange3d const& boundingBoxInUors);    
    GROUND_DETECTION_EXPORT virtual size_t          _GetMemorySize() const;
    GROUND_DETECTION_EXPORT virtual size_t          _ComputeMemorySize() const;
    GROUND_DETECTION_EXPORT virtual const_iterator  _begin() const = 0;
    GROUND_DETECTION_EXPORT virtual const_iterator  _end() const = 0;
    GROUND_DETECTION_EXPORT virtual void            _PrefetchPoints() = 0;
    GROUND_DETECTION_EXPORT virtual size_t          _GetPrefetchedPoints(DPoint3d* points, size_t maxSize);
    GROUND_DETECTION_EXPORT virtual void            _ClearPrefetchedPoints() = 0;
    GROUND_DETECTION_EXPORT virtual Transform       _GetMeterToNativeTransform() const=0;
    GROUND_DETECTION_EXPORT virtual Transform       _GetUorToMeterTransform(bool useGlobalOrigin) const;
    GROUND_DETECTION_EXPORT virtual BeFileName      _GetFileName() const=0;
    GROUND_DETECTION_EXPORT virtual void            _SetMaxPointsToPrefetch(int value);
    GROUND_DETECTION_EXPORT virtual int             _GetMaxPointsToPrefetch() const;
    GROUND_DETECTION_EXPORT virtual void            _SetExportResolution(double exportResolution);
    GROUND_DETECTION_EXPORT virtual double          _GetExportResolution() const;

    GROUND_DETECTION_EXPORT virtual void            _SetUseMultiThread(bool value);
    GROUND_DETECTION_EXPORT virtual bool            _GetUseMultiThread() const;    
    GROUND_DETECTION_EXPORT virtual void            _SetUseMeterUnit(bool value); 
    GROUND_DETECTION_EXPORT virtual bool            _GetUseMeterUnit() const;

    Transform GetUorToMeterTransformIntern(/*DgnModelRefP model,*/ bool useGlobalOrigin);
    
    DRange3d                            m_boundingBoxInUors;
    mutable bool                        m_prefetchPoints;
    mutable bvector<IPointsProvider::IPointsProviderIteratorImpl::ReturnType>  m_prefetchedPoints;
    int                                 m_maxPointsToPreFetch; //-1 means we don't want to cap the number of points to prefetch
    double                              m_exportResolution;//In Meters

//__PUBLISH_CLASS_VIRTUAL__
public:
     static IPointsProviderPtr CreateFrom(IPointsProviderCreatorPtr& pointsProviderCreator, DRange3d* pRange = NULL);                 

    GROUND_DETECTION_EXPORT IPointsProviderPtr Clone() const    {return _Clone();}
    GROUND_DETECTION_EXPORT void        PrefetchPoints() { return _PrefetchPoints(); }
    GROUND_DETECTION_EXPORT size_t      GetPrefetchedPoints(DPoint3d* points, size_t maxSize) { return _GetPrefetchedPoints(points, maxSize); }
    GROUND_DETECTION_EXPORT void        ClearPrefetchedPoints() { return _ClearPrefetchedPoints(); }

    GROUND_DETECTION_EXPORT size_t      GetMemorySize() const { return _GetMemorySize(); }
    
    GROUND_DETECTION_EXPORT void        SetUseMultiThread(bool value) { _SetUseMultiThread(value); }
    GROUND_DETECTION_EXPORT bool        GetUseMultiThread() const { return _GetUseMultiThread(); }    
    GROUND_DETECTION_EXPORT void        SetMaxPointsToPrefetch(int value) { _SetMaxPointsToPrefetch (value); };
    GROUND_DETECTION_EXPORT int         GetMaxPointsToPrefetch() const    { return _GetMaxPointsToPrefetch(); };
    GROUND_DETECTION_EXPORT void        SetExportResolution(double exportResolution)    { _SetExportResolution(exportResolution); }
    GROUND_DETECTION_EXPORT double      GetExportResolution() const                     { return _GetExportResolution(); }

    //Point always returned in meters, you can set this to false if you want otherwise and point will be transformed to UORs
    //before being returned
    GROUND_DETECTION_EXPORT void        SetUseMeterUnit(bool value) { _SetUseMeterUnit(value); }
    GROUND_DETECTION_EXPORT bool        GetUseMeterUnit() const { return _GetUseMeterUnit(); }    
    GROUND_DETECTION_EXPORT Transform   GetUorToMeterTransform(bool useGlobalOrigin) const { return _GetUorToMeterTransform(useGlobalOrigin); }
    GROUND_DETECTION_EXPORT Transform   GetMeterToNativeTransform() const { return _GetMeterToNativeTransform(); }

    GROUND_DETECTION_EXPORT BeFileName GetFileName() const { return _GetFileName(); }
    GROUND_DETECTION_EXPORT DRange3d   GetBoundingBox() const  { return _GetBoundingBox(); }
    GROUND_DETECTION_EXPORT void       SetBoundingBox(DRange3d& boundingBoxInUors)  { return _SetBoundingBox(boundingBoxInUors); }    

    GROUND_DETECTION_EXPORT const_iterator begin() const { return _begin(); }
    GROUND_DETECTION_EXPORT const_iterator end() const { return _end(); }



private:
    
    bool                                m_useViewFilters;
    bool                                m_isMultiThread;    
    float                               m_density;
    int                                 m_queryView;
    bool                                m_useMeterUnit; //if true, point will be returned in meter unit instead of UORs


}; // IPointsProvider

/*=================================================================================**//**
* @bsiclass                                                                 10/2016
+===============+===============+===============+===============+===============+======*/
struct IPointsProviderCreator : public RefCountedBase
    {
    protected : 


        virtual IPointsProviderPtr _CreatePointProvider(DRange3d const& boundingBoxInUors) = 0;

        virtual IPointsProviderPtr _CreatePointProvider() = 0;

       virtual void               _GetAvailableRange(DRange3d& availableRange) = 0;

    
    public : 

        IPointsProviderPtr CreatePointProvider(DRange3d const& boundingBoxInUors);

        IPointsProviderPtr CreatePointProvider();

        GROUND_DETECTION_EXPORT void GetAvailableRange(DRange3d& availableRange);
    };


END_GROUND_DETECTION_NAMESPACE
