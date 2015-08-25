/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/FilterFactory.cpp $
|    $RCSfile: FilterFactory.cpp,v $
|   $Revision: 1.11 $
|       $Date: 2011/09/07 14:21:02 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/FilterFactory.h>

#include <ScalableMesh/Import/Warnings.h>

#include <ScalableMesh/Import/Plugin/FilterV0.h>

#include <ScalableMesh/GeoCoords/Reprojection.h>

#include "TypeConversionFilterFactory.h"
#include "ReprojectionFilterFactory.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TypeConvertAndReprojectFilter : public FilterBase
    {
    PacketGroupPtr                  m_intermediatePtr; 
    FilterPtr                       m_typeConverterPtr;
    FilterPtr                       m_reprojectorPtr;


    virtual void                    _Assign                            (const PacketGroup&              src,
                                                                        PacketGroup&                    dst) override
        {
        // Both filters were already assigned
        }

    virtual void                    _Run                               () override
        {
        m_typeConverterPtr->Run();
        m_reprojectorPtr->Run();
        }

public:
    explicit                        TypeConvertAndReprojectFilter      (const FilterPtr&               typeConverterPtr,
                                                                        const FilterPtr&               reprojectorPtr,
                                                                        const PacketGroupPtr&           intermediatePtr)
        :   m_intermediatePtr(intermediatePtr),
            m_typeConverterPtr(typeConverterPtr), 
            m_reprojectorPtr(reprojectorPtr)
        {
        }
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class TypeConvertAndReprojectFilterFactory : public FilterCreatorBase
    {
    FilterCreatorCPtr               m_pTypeConvertionFactory;
    FilterCreatorCPtr               m_pReprojectionFactory;

    virtual void                    _Bind                              (const PacketGroup&          pi_Src,
                                                                        PacketGroup&                po_Dst) const override
        {
        // Both filters were already bound
        }

    
    template <typename SrcPacketGroupT>
    FilterBase*                     CommonCreate                       (SrcPacketGroupT&            pi_Src,
                                                                        PacketGroup&                po_Dst,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        warningLog) const
        {
        PacketGroupPtr intermediatePtr(new PacketGroup(m_pTypeConvertionFactory->GetTargetType().GetDimensionOrgCount(),
                                                       po_Dst.GetAllocator()));

        FilterPtr typeConvFilterPtr = m_pTypeConvertionFactory->Create(pi_Src, *intermediatePtr, config, warningLog);
        FilterPtr reprojFilterPtr = m_pReprojectionFactory->Create(*intermediatePtr, po_Dst, config, warningLog);

        return new TypeConvertAndReprojectFilter(typeConvFilterPtr, reprojFilterPtr, intermediatePtr);
        }


    virtual FilterBase*             _Create                            (const PacketGroup&          pi_Src,
                                                                        PacketGroup&                po_Dst,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        warningLog) const override
        { return CommonCreate(pi_Src, po_Dst, config, warningLog); }

    virtual const DataType&        _GetSourceType                      () const override
        {
        return m_pTypeConvertionFactory->GetSourceType();
        }

    virtual const DataType&        _GetTargetType                      () const override
        {
        return m_pTypeConvertionFactory->GetTargetType();
        }

    explicit                        TypeConvertAndReprojectFilterFactory
                                                                       (const FilterCreatorCPtr&    typeConvertionFactory,
                                                                        const FilterCreatorCPtr&    reprojectionFactory)
        :   m_pTypeConvertionFactory(typeConvertionFactory),
            m_pReprojectionFactory(reprojectionFactory)
        {
        }

public:
    static FilterCreator*           CreateFrom                         (const FilterCreatorCPtr&    typeConvertionFactory,
                                                                        const FilterCreatorCPtr&    reprojectionFactory)
        {
        return CreateFromBase(new TypeConvertAndReprojectFilterFactory(typeConvertionFactory, reprojectionFactory));
        }
    };





/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFilterSequenceFilter : public FilterBase
    {
    typedef bvector<PacketGroupPtr>
                                    PacketGroupSequence;

    typedef bvector<FilterPtr>      Sequence;

private:
    std::auto_ptr<const PacketGroupSequence>   
                                    m_pPacketGroupSequence;
    Sequence                        m_filterSequence;


    struct RunFilter
        {
        void                        operator ()                        (const FilterPtr&                rhs) const
            {
            rhs->Run();
            }
        };


    virtual void                    _Assign                            (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) override
        {
        // All filters were already assigned
        }

    virtual void                    _Run                               () override
        {
        std::for_each(m_filterSequence.begin(), m_filterSequence.end(), RunFilter());
        }

public:
    explicit                        CustomFilterSequenceFilter         (Sequence&                       pi_filterSequence,
                                                                        const PacketGroupSequence*      pi_pPacketGroupSequence)
        :   m_pPacketGroupSequence(pi_pPacketGroupSequence),
            m_filterSequence(pi_filterSequence)
        {

        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFilterSequenceFilterFactory : public FilterCreatorBase
    {
    typedef bvector <FilterCreatorCPtr>  
                                    Sequence;

private:
    Sequence                        m_filterFactorySequence;


    struct CreateFilter
        {
        CustomFilterSequenceFilter::PacketGroupSequence&
                                    m_rPacketGroupSequence;
        const MemoryAllocator&      m_rAllocator;
        const FilteringConfig&      m_config;
        Log&                        m_warningLog;
    

        explicit                    CreateFilter                       (CustomFilterSequenceFilter::PacketGroupSequence&
                                                                                                    packetGroupSequence,
                                                                        const MemoryAllocator&      allocator,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        log)
            :   m_rPacketGroupSequence(packetGroupSequence),
                m_rAllocator(allocator),
                m_config(config),
                m_warningLog(log)
            { 
            assert(!m_rPacketGroupSequence.empty()); 
            }
        
        FilterPtr                   operator ()                        (const FilterCreatorCPtr&    pi_pFactory)
            {
            PacketGroup& rSourcePacketGroup = *m_rPacketGroupSequence.back();
            assert(rSourcePacketGroup.GetSize() == pi_pFactory->GetSourceType().GetDimensionOrgCount());

            m_rPacketGroupSequence.push_back(new PacketGroup(pi_pFactory->GetTargetType().GetDimensionOrgCount(),
                                                             m_rAllocator));
            PacketGroup& rTargetPacketGroup = *m_rPacketGroupSequence.back();

            return pi_pFactory->Create(rSourcePacketGroup, rTargetPacketGroup, m_config, m_warningLog);
            }
        };



    virtual void                    _Bind                              (const PacketGroup&          pi_Src,
                                                                        PacketGroup&                po_Dst) const override
        {
        // All filters were already bound
        }


    template <typename SrcPacketGroupT>
    FilterBase*                     CommonCreate                       (SrcPacketGroupT&            pi_Src,
                                                                        PacketGroup&                po_Dst,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        warningLog) const
        {
        std::auto_ptr<CustomFilterSequenceFilter::PacketGroupSequence> 
            pPacketGroupSequence(new CustomFilterSequenceFilter::PacketGroupSequence);

        CustomFilterSequenceFilter::Sequence filterSequence;

        // Create first filter
            {
            const FilterCreator& rFirstFilterFactory = *m_filterFactorySequence.front();

            pPacketGroupSequence->push_back(new PacketGroup(rFirstFilterFactory.GetTargetType().GetDimensionOrgCount(),
                                                            po_Dst.GetAllocator()));
            PacketGroup& rTargetPacketGroup = *pPacketGroupSequence->back();

            filterSequence.push_back(rFirstFilterFactory.Create(pi_Src, rTargetPacketGroup, config, warningLog));
            }

        // Create intermediary filters
        std::transform(m_filterFactorySequence.begin() + 1, m_filterFactorySequence.end() - 1, 
                       std::back_inserter(filterSequence),
                       CreateFilter(*pPacketGroupSequence, po_Dst.GetAllocator(), config, warningLog));


        // Create last filter
            {
            const FilterCreator& rLastFilterFactory = *m_filterFactorySequence.back();

            PacketGroup& rSourcePacketGroup = *pPacketGroupSequence->back();
            filterSequence.push_back(rLastFilterFactory.Create(rSourcePacketGroup, po_Dst, config, warningLog));
            }

        return new CustomFilterSequenceFilter(filterSequence, pPacketGroupSequence.release());
        }

    virtual FilterBase*             _Create                            (const PacketGroup&          pi_Src,
                                                                        PacketGroup&                po_Dst,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        warningLog) const override
        { return CommonCreate(pi_Src, po_Dst, config, warningLog); }

    virtual const DataType&        _GetSourceType                      () const override
        {
        return m_filterFactorySequence.front()->GetSourceType();
        }

    virtual const DataType&        _GetTargetType                      () const override
        {
        return m_filterFactorySequence.back()->GetTargetType();
        }

    explicit                        CustomFilterSequenceFilterFactory  (const Sequence&             pi_filterFactorySequence)
        :   m_filterFactorySequence(pi_filterFactorySequence)
        {
        assert(2 <= m_filterFactorySequence.size());
        }

public:
    static FilterCreator*           CreateFrom                         (const Sequence&             pi_filterFactorySequence)
        {
        return CreateFromBase(new CustomFilterSequenceFilterFactory(pi_filterFactorySequence));
        }

    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    const TypeConversionFilterFactory   m_typeConversionFilterFactory;
    const ReprojectionFilterFactory     m_reprojectionFilterFactory;
    Log&                         m_warningLog;

    explicit                            Impl                                   (const TypeConversionFilterFactory&  typeConversionFactory,
                                                                                const ReprojectionFilterFactory&    reprojectionFilterFactory,
                                                                                Log&                         log)
        :   m_typeConversionFilterFactory(typeConversionFactory),
            m_reprojectionFilterFactory(reprojectionFilterFactory),
            m_warningLog(log)
        {
        
        }

    explicit                            Impl                                   (Log&                         log)
        :   m_typeConversionFilterFactory(log),
            m_reprojectionFilterFactory(log),
            m_warningLog(log)
        {
        }

    bool                                FindFilterFactoriesForCustomFilters    (CustomFilterSequenceFilterFactory::Sequence&    
                                                                                                                filterFactorySequence,
                                                                                const DataType*&                currentTypeP,
                                                                                const CustomFilteringSequence&  customFilterSequence) const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterFactory::FilterFactory(Log& log)
    :   m_pImpl(new Impl(log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterFactory::FilterFactory   (const TypeConversionFilterFactory&  typeConversionFactory,
                                const ReprojectionFilterFactory&    reprojectionFilterFactory,
                                Log&                         log)
    :   m_pImpl(new Impl(typeConversionFactory, reprojectionFilterFactory, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterFactory::FilterFactory (const TypeConversionFilterFactory&  typeConversionFactory,
                              Log&                         log)
    :   m_pImpl(new Impl(typeConversionFactory, ReprojectionFilterFactory(log), log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterFactory::FilterFactory (const ReprojectionFilterFactory&    reprojectionFilterFactory,
                              Log&                         log)
    :   m_pImpl(new Impl(TypeConversionFilterFactory(log), reprojectionFilterFactory, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterFactory::FilterFactory (const FilterFactory& rhs)
    :   m_pImpl(rhs.m_pImpl)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterFactory::~FilterFactory ()
    {
    
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr FilterFactory::FindCreatorFor    (const DataType&             pi_rSourceType,
                                                    const DataType&             pi_rTargetType) const
    {
    return m_pImpl->m_typeConversionFilterFactory.FindCreatorFor(pi_rSourceType, pi_rTargetType);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr FilterFactory::FindCreatorFor(const DataType&                     sourceType,
                                                const DataType&                     targetType,
                                                const GCS&                          sourceGCS,
                                                const GCS&                          targetGCS,
                                                const DRange3d*                     sourceExtentP) const
    {
    const Reprojection reprojection(m_pImpl->m_reprojectionFilterFactory.CreateReprojectionFor(sourceGCS, 
                                                                                               targetGCS, 
                                                                                               sourceExtentP));

    if (reprojection.IsNull())
        return m_pImpl->m_typeConversionFilterFactory.FindCreatorFor(sourceType, targetType);

    return FindCreatorFor(sourceType, targetType, reprojection);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool FilterFactory::Impl::FindFilterFactoriesForCustomFilters  (CustomFilterSequenceFilterFactory::Sequence&    filterFactorySequence,
                                                                const DataType*&                                currentTypeP,
                                                                const CustomFilteringSequence&                  customFilterSequence) const
    {
    assert(0 != currentTypeP);

    for (CustomFilteringSequence::const_iterator cutomFilterIt = customFilterSequence.begin(),
         cutomFilterEnd = customFilterSequence.end();
         cutomFilterIt != cutomFilterEnd;
         ++cutomFilterIt)
        {
        filterFactorySequence.push_back(cutomFilterIt->FindCreatorFor(*currentTypeP, m_warningLog));
        if (0 == filterFactorySequence.back().get())
            {
            assert(!"Could not get factory from custom filter");
            return false;
            }

        currentTypeP = &filterFactorySequence.back()->GetTargetType();
        }



    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr FilterFactory::FindCreatorFor    (const DataType&                 sourceType,
                                                    const DataType&                 targetType,
                                                    const GCS&                      sourceGCS,
                                                    const GCS&                      targetGCS,
                                                    const DRange3d*                 sourceExtentP,
                                                    const CustomFilteringSequence&  sourceFilters,
                                                    const CustomFilteringSequence&  targetFilters) const
    {
    if (0 == sourceFilters.GetCount() && 0 == targetFilters.GetCount())
        return FindCreatorFor(sourceType, targetType, sourceGCS, targetGCS, sourceExtentP);


    CustomFilterSequenceFilterFactory::Sequence subFactorySequence;

    const DataType* currentTypeP = &sourceType;

    // Find factories for source custom filters
    if (!m_pImpl->FindFilterFactoriesForCustomFilters(subFactorySequence, currentTypeP, sourceFilters))
        return 0;

    // Find factory for type conversion and reprojection
        {
        subFactorySequence.push_back(FindCreatorFor(*currentTypeP, targetType, 
                                                    sourceGCS, targetGCS, sourceExtentP));

        if (0 == subFactorySequence.back().get())
            {
            assert(!"Could not find appropriate type convertion/reprojection factory");
            return 0;
            }

        currentTypeP = &subFactorySequence.back()->GetTargetType();
        }

    // Find factories for target custom filters
    if (!m_pImpl->FindFilterFactoriesForCustomFilters(subFactorySequence, currentTypeP, targetFilters))
        return 0;

    if (*currentTypeP != targetType)
        {
        subFactorySequence.push_back(FindCreatorFor(*currentTypeP, targetType));

        if (0 == subFactorySequence.back().get())
            {
            assert(!"Could not find appropriate type convertion factory");
            return 0;
            }
        }

    return CustomFilterSequenceFilterFactory::CreateFrom(subFactorySequence);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Let user handle the whole filtering sequence
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr FilterFactory::FindCreatorFor (const DataType&                    sourceType,
                                                 const CustomFilteringSequence&     filters,
                                                 const DataType&                    targetType) const
    {
    if (0 == filters.GetCount())
        {
        assert(!"Error. Empty filtering sequence specified");
        // TODRAY: Return no effect filter.
        return 0;
        }


    CustomFilterSequenceFilterFactory::Sequence subFactorySequence;

    const DataType* currentTypeP = &sourceType;

    // Find factories for source custom filters
    if (!m_pImpl->FindFilterFactoriesForCustomFilters(subFactorySequence, currentTypeP, filters))
        return 0;

    if (*currentTypeP != targetType)
        {
        subFactorySequence.push_back(FindCreatorFor(*currentTypeP, targetType));

        if (0 == subFactorySequence.back().get())
            {
            assert(!"Could not find appropriate type convertion factory");
            return 0;
            }
        }
    

    assert(!"Implement");
    return CustomFilterSequenceFilterFactory::CreateFrom(subFactorySequence);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr FilterFactory::FindCreatorFor    (const DataType&                 pi_rSourceType,
                                                    const DataType&                 pi_rTargetType,
                                                    const Reprojection&             pi_rReprojection) const
    {
    assert(!pi_rReprojection.IsNull());

    FilterCreatorCPtr pTypeConverterFactory = m_pImpl->m_typeConversionFilterFactory.FindCreatorFor(pi_rSourceType, pi_rTargetType);
    FilterCreatorCPtr pReprojectorFactory = m_pImpl->m_reprojectionFilterFactory.FindCreatorFor(pi_rTargetType, pi_rReprojection);

    if (0 == pTypeConverterFactory.get() || 0 == pReprojectorFactory.get()) 
        return FilterCreatorCPtr();

    const bool NeedTypeConvertion = !(pTypeConverterFactory->GetSourceType() == pReprojectorFactory->GetTargetType());

    if (!NeedTypeConvertion)
        return pReprojectorFactory;

    return TypeConvertAndReprojectFilterFactory::CreateFrom(pTypeConverterFactory, pReprojectorFactory);
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
