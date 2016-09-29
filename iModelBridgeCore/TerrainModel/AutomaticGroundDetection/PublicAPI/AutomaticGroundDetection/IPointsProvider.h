/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PublicAPI/AutomaticGroundDetection/IPointsProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/NonCopyableClass.h>
#include <Bentley/BeFileName.h>

#include <AutomaticGroundDetection\GroundDetectionMacros.h>

GROUND_DETECTION_TYPEDEF(IPointsProvider)

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
* @bsiclass                                     		Marc.Bedard     12/2015
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
    IPointsProvider(DRange3d const& boundingBoxInUors);
    IPointsProvider(IPointsProvider const& object);
    IPointsProvider();

    size_t          ComputeMemorySize() const { return _ComputeMemorySize(); }

    virtual IPointsProviderPtr _Clone() const=0;
    virtual DRange3d        _GetBoundingBox() const;
    virtual void            _SetBoundingBox(DRange3d const& boundingBoxInUors);    
    virtual size_t          _GetMemorySize() const;
    virtual size_t          _ComputeMemorySize() const;
    virtual const_iterator  _begin() const = 0;
    virtual const_iterator  _end() const = 0;
    virtual void            _PrefetchPoints() = 0;
    virtual size_t          _GetPrefetchedPoints(DPoint3d* points, size_t maxSize);
    virtual void            _ClearPrefetchedPoints() = 0;
    virtual Transform       _GetMeterToNativeTransform() const=0;
    virtual Transform       _GetUorToMeterTransform(bool useGlobalOrigin) const;
    virtual BeFileName      _GetFileName() const=0;
    virtual void            _SetMaxPointsToPrefetch(int value);
    virtual int             _GetMaxPointsToPrefetch() const;
    virtual void            _SetExportResolution(double exportResolution);
    virtual double          _GetExportResolution() const;
    
    virtual void            _SetUseMultiThread(bool value);
    virtual bool            _GetUseMultiThread() const;    
    virtual void            _SetUseMeterUnit(bool value); 
    virtual bool            _GetUseMeterUnit() const;

	Transform GetUorToMeterTransformIntern(/*DgnModelRefP model,*/ bool useGlobalOrigin);
    
    DRange3d                            m_boundingBoxInUors;
    mutable bool                        m_prefetchPoints;
    mutable bvector<IPointsProvider::IPointsProviderIteratorImpl::ReturnType>  m_prefetchedPoints;
    int                                 m_maxPointsToPreFetch; //-1 means we don't want to cap the number of points to prefetch
    double                              m_exportResolution;//In Meters

//__PUBLISH_CLASS_VIRTUAL__
public:
     static IPointsProviderPtr CreateFrom(DRange3d* pRange = NULL);                 

    IPointsProviderPtr Clone() const    {return _Clone();}
    void        PrefetchPoints() { return _PrefetchPoints(); }
    size_t      GetPrefetchedPoints(DPoint3d* points, size_t maxSize) { return _GetPrefetchedPoints(points, maxSize); }
    void        ClearPrefetchedPoints() { return _ClearPrefetchedPoints(); }

    size_t      GetMemorySize() const { return _GetMemorySize(); }
    
    void        SetUseMultiThread(bool value) { _SetUseMultiThread(value); }
    bool        GetUseMultiThread() const { return _GetUseMultiThread(); }    
    void        SetMaxPointsToPrefetch(int value) { _SetMaxPointsToPrefetch (value); };
    int         GetMaxPointsToPrefetch() const    { return _GetMaxPointsToPrefetch(); };
    void        SetExportResolution(double exportResolution)    { _SetExportResolution(exportResolution); }
    double      GetExportResolution() const                     { return _GetExportResolution(); }

    //Point always returned in meters, you can set this to false if you want otherwise and point will be transformed to UORs
    //before being returned
    void        SetUseMeterUnit(bool value) { _SetUseMeterUnit(value); }
    bool        GetUseMeterUnit() const { return _GetUseMeterUnit(); }	
    Transform   GetUorToMeterTransform(bool useGlobalOrigin) const { return _GetUorToMeterTransform(useGlobalOrigin); }
    Transform   GetMeterToNativeTransform() const { return _GetMeterToNativeTransform(); }

    BeFileName GetFileName() const { return _GetFileName(); }
    DRange3d   GetBoundingBox() const  { return _GetBoundingBox(); }
    void       SetBoundingBox(DRange3d& boundingBoxInUors)  { return _SetBoundingBox(boundingBoxInUors); }    

    const_iterator begin() const { return _begin(); }
    const_iterator end() const { return _end(); }



private:
    bool                                m_useViewFilters;
    bool                                m_isMultiThread;    
    float                               m_density;
    int                                 m_queryView;
    bool                                m_useMeterUnit; //if true, point will be returned in meter unit instead of UORs


}; // IPointsProvider

END_GROUND_DETECTION_NAMESPACE
