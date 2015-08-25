/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshPolicy.cpp $
|    $RCSfile: ScalableMeshPolicy.cpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/18 15:50:41 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/IScalableMeshPolicy.h>

#include <ScalableMesh/Foundations/Log.h>
#include <ScalableMesh/Foundations/Error.h>
#include <ScalableMesh/Foundations/Warning.h>
#include <ScalableMesh/Foundations/Message.h>

#include <ScalableMesh/Import/DataTypeFamily.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Importer.h>
#include <ScalableMesh/Import/ImportPolicy.h>

#include <ScalableMesh/Memory/Allocation.h>

#include <ScalableMesh/Import/Plugin/ExtractorRegistry.h>
#include <ScalableMesh/Import/Plugin/SourceRegistry.h>

#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include "../Import/ReprojectionFilterFactory.h"
#include <ScalableMesh/Import/FilterFactory.h>


 

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_SCALABLEMESH_FOUNDATIONS

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportMemoryAllocator : public Memory::MemoryAllocator
    {
private:

    struct                              Impl;
    SharedPtrTypeTrait<Impl>::type      m_implP;

    virtual void*                       _Allocate                          (size_t                                  capacity) const override;

    virtual void                        _Deallocate                        (void*                                   memory) const override;

    virtual MemoryAllocator*            _Clone                             () const override;

                                        ImportMemoryAllocator              (const ImportMemoryAllocator&            rhs);


public:
    explicit                            ImportMemoryAllocator              ();
    virtual                             ~ImportMemoryAllocator             ();

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportMemoryAllocator::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    static HPMMemoryMgr&            GetMemoryMgrInstance           ();

    static UInt                     s_InstanceCount;
    HPMMemoryMgr&                   m_memMgr;
    mutable UInt                    m_allocatedBlockCount;    

public:
    static size_t                   GetMemoryMgrKeptBlockCount     ();

    explicit                        Impl                           ();
                                    ~Impl                          ();
    

    HPMMemoryMgr&                   GetMemoryMgr                   () const { return m_memMgr; }

    void*                           AllocateBlock                  (size_t              pi_capacity) const
        {
        // If triggered, MemoryMgrKeptBlockCount must be incremented as this may lead to memory fragmentation.
        assert(m_allocatedBlockCount < GetMemoryMgrKeptBlockCount());
        ++m_allocatedBlockCount;

        return GetMemoryMgr().AllocMemory(pi_capacity);
        }

    void                            DeallocateBlock                (void*               pi_memory) const
        {
        if (0 == pi_memory)
            return;

        assert(0 < m_allocatedBlockCount);
        --m_allocatedBlockCount;

        GetMemoryMgr().FreeMemory(static_cast<byte*>(pi_memory), 0);
        }

    };

UInt ImportMemoryAllocator::Impl::s_InstanceCount = 0;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ImportMemoryAllocator::Impl::GetMemoryMgrKeptBlockCount ()
    {
    return 10;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HPMMemoryMgr& ImportMemoryAllocator::Impl::GetMemoryMgrInstance ()
    {
    static HPMMemoryMgrReuseAlreadyAllocatedBlocks MEMORY_MGR(GetMemoryMgrKeptBlockCount());
    return MEMORY_MGR;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportMemoryAllocator::Impl::Impl ()
    :   m_memMgr(GetMemoryMgrInstance()),
        m_allocatedBlockCount(0)
    {
    // There should be one memory manager per instance. If we get more than one instance at a time
    // memory manager should not be a singleton anymore.
    //assert(0 == s_InstanceCount); // TDORAY: Reconsider
    ++s_InstanceCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportMemoryAllocator::Impl::~Impl ()
    {
    assert(0 < s_InstanceCount);
    assert(0 == m_allocatedBlockCount);
    --s_InstanceCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportMemoryAllocator::ImportMemoryAllocator ()
    :   m_implP(new Impl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportMemoryAllocator::~ImportMemoryAllocator ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportMemoryAllocator::ImportMemoryAllocator   (const ImportMemoryAllocator&    rhs)
    :   m_implP(rhs.m_implP)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void* ImportMemoryAllocator::_Allocate (size_t pi_capacity) const
    {
    return m_implP->AllocateBlock(pi_capacity);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportMemoryAllocator::_Deallocate (void* pi_memory) const
    {
    m_implP->DeallocateBlock(pi_memory);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryAllocator* ImportMemoryAllocator::_Clone () const
    {
    return new ImportMemoryAllocator(*this);
    }






/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConvertion
    {
    DataTypeFamily                      m_sourceType;
    DataTypeFamily                      m_targetType;
    explicit                            TypeConvertion                         (const DataTypeFamily&           sourceType,
                                                                                const DataTypeFamily&           targetType)
        :   m_sourceType(sourceType),
            m_targetType(targetType)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct TypeConvertionMap
    {
    typedef vector<TypeConvertion>      Map;    
    typedef Map::const_iterator         CIter;
    typedef pair<Map::const_iterator, Map::const_iterator>
                                        Range;
private:
    Map                                 m_map;
    struct Compare
        {
        bool operator() (const TypeConvertion& lhs, const TypeConvertion& rhs) const { return lhs.m_sourceType < rhs.m_sourceType; }
        bool operator() (const DataTypeFamily& lhs, const TypeConvertion& rhs) const { return lhs < rhs.m_sourceType; }
        bool operator() (const TypeConvertion& lhs, const DataTypeFamily& rhs) const { return lhs.m_sourceType < rhs; }
        };
public:
    explicit                            TypeConvertionMap                      ()
        {
        m_map.push_back(TypeConvertion(LinearTypeFamilyCreator().Create(), PointTypeFamilyCreator().Create()));

        m_map.push_back(TypeConvertion(MeshTypeFamilyCreator().Create(), LinearTypeFamilyCreator().Create()));
        m_map.push_back(TypeConvertion(MeshTypeFamilyCreator().Create(), PointTypeFamilyCreator().Create()));

        m_map.push_back(TypeConvertion(TINTypeFamilyCreator().Create(), LinearTypeFamilyCreator().Create()));
        m_map.push_back(TypeConvertion(TINTypeFamilyCreator().Create(), PointTypeFamilyCreator().Create()));

        stable_sort(m_map.begin(), m_map.end(), Compare());
        }

    Range                               FindConvertionsFor                     (const DataTypeFamily&           sourceType) const
        {
        return equal_range(s_TypeConvertionMap.m_map.begin(), s_TypeConvertionMap.m_map.end(), sourceType, Compare());
        }

    } s_TypeConvertionMap;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportTargetTypeSelectionPolicy : public TypeSelectionPolicy
    {
private:
    virtual TypeSelectionPolicy*            _Clone                                 () const;
    virtual const DataType*                 _Select                                (const DataTypeFamily&           typeFamily,
                                                                                    const DataType*                 typesBegin,
                                                                                    const DataType*                 typesEnd) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeSelectionPolicy* ImportTargetTypeSelectionPolicy::_Clone () const
    {
    return new ImportTargetTypeSelectionPolicy(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType* ImportTargetTypeSelectionPolicy::_Select   (const DataTypeFamily&   typeFamily,
                                                            const DataType*         typesBegin,
                                                            const DataType*         typesEnd) const
    {   
    const DataType* foundType = std::find (typesBegin, typesEnd, typeFamily);

    if (typesEnd != foundType)
        return foundType;

    // Fallback on possible inter family conversions
    TypeConvertionMap::Range foundRange(s_TypeConvertionMap.FindConvertionsFor(typeFamily));

    
    for (TypeConvertionMap::CIter convIt = foundRange.first; convIt != foundRange.second; ++convIt)
        {
        foundType = std::find (typesBegin, typesEnd, convIt->m_targetType);
        if (typesEnd != foundType)
            break;
        }

    return foundType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportWarningLog : public Foundations::Log
    {
private:
    struct Impl : public ShareableObjectTypeTrait<Impl>::type
        {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit                        Impl                               () {}

        vector<ErrorItem>               m_errors;
        vector<WarningItem>             m_warnings;
        vector<MessageItem>             m_messages;
        };
    
    typedef SharedPtrTypeTrait<Impl>::type 
                                        ImplPtr;
    ImplPtr                             m_implP;

                                        ImportWarningLog                   (const ImportWarningLog&     rhs)
        :   m_implP(rhs.m_implP)
        {
        }   

    virtual Log*                        _Clone                             () const override;

    virtual void                        _Add                               (const Error&                error) override;
    virtual void                        _Add                               (const Warning&              warning) override;
    virtual void                        _Add                               (const Message&              message) override;

public:
    explicit                            ImportWarningLog                   ()
        :   m_implP(new Impl)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Log* ImportWarningLog::_Clone () const
    {
    return new ImportWarningLog(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportWarningLog::_Add (const Error& error)
    {
    m_implP->m_errors.push_back(error);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportWarningLog::_Add (const Warning& warning)
    {
    m_implP->m_warnings.push_back(warning);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportWarningLog::_Add (const Message& message)
    {
    m_implP->m_messages.push_back(message);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFactory CreateSourceFactory ()
    {
    const SourceFactory sourceFactory(Plugin::SourceRegistry::GetInstance(), GetLog());
    return sourceFactory;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterFactory CreateImporterFactory ()
    {
    const ReprojectionFilterFactory reprojectionFilterFactory(GetReprojectionFactory(), GetLog());
    const FilterFactory filterFactory(reprojectionFilterFactory, GetLog());

    const ImportPolicy importPolicy(GetMemoryAllocator(), 
                                    ImportPolicy::GetDefaultSourceTypeSelectionPolicy(),
                                    ImportTargetTypeSelectionPolicy());

    const ImporterFactory importerFactory(importPolicy,
                                          Plugin::ExtractorRegistry::GetInstance(),
                                          filterFactory,
                                          GetLog());

    return importerFactory;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFactory CreateReprojectionFactory ()
    {
    const double angularToLinearUnitRatio(GetAngularToLinearRatio(Unit::GetMeter(), Unit::GetDegree()));

    return ReprojectionFactory(ReprojectionPolicy().
                                    AllowConversionBetweenUnitBases(true).
                                    AllowGCSToLCS(true).
                                    AllowLCSToGCS(true).
                                    SetAngularToLinearUnitRatio(angularToLinearUnitRatio), 
                               GetLog());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSFactory CreateGCSFactory ()
    {
    const GCSFactory gcsFactory(GetLog());
    return gcsFactory;
    }

}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEYSTM_EXPORT const Memory::MemoryAllocator& GetMemoryAllocator ()
    {
    static const ImportMemoryAllocator MEMORY_ALLOCATOR_INSTANCE;
    return MEMORY_ALLOCATOR_INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEYSTM_EXPORT Foundations::Log& GetLog ()
    {
    static ImportWarningLog WARNING_LOG;
    return WARNING_LOG;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEYSTM_EXPORT const GeoCoords::GCSFactory& GetGCSFactory ()
    {
    static const GCSFactory INSTANCE (CreateGCSFactory());
    return INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GeoCoords::ReprojectionFactory& GetReprojectionFactory ()
    {
    static const ReprojectionFactory INSTANCE (CreateReprojectionFactory());
    return INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEYSTM_EXPORT const Import::SourceFactory& GetSourceFactory ()
    {
    static const SourceFactory INSTANCE(CreateSourceFactory());
    return INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Import::ImporterFactory& GetImporterFactory ()
    {
    static const ImporterFactory INSTANCE(CreateImporterFactory());
    return INSTANCE;
    }



END_BENTLEY_SCALABLEMESH_NAMESPACE
