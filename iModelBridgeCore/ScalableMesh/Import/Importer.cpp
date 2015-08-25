/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/Importer.cpp $
|    $RCSfile: Importer.cpp,v $
|   $Revision: 1.20 $
|       $Date: 2011/09/07 14:21:05 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/Importer.h>
#include "ImporterImpl.h"
#include "ImporterCommandVisitor.h"

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/ContentDescriptor.h>

#include <ScalableMesh/Import/Warnings.h>
#include <ScalableMesh/Import/Exceptions.h>
#include <ScalableMesh/Memory/Allocation.h>

#include <ScalableMesh/Import/FilterFactory.h>

#include <ScalableMesh/Memory/PacketAccess.h>

#include "InputExtractor.h"
#include "Sink.h"


#include <ScalableMesh/GeoCoords/GCS.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES




BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

using namespace Internal;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const Config& ImporterImpl::DefaultImportConfig ()
    {
    static const Config DEFAULT_IMPORT_CONFIG;
    return DEFAULT_IMPORT_CONFIG;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Importer::Importer (ImporterImpl* pi_rpImpl)
    :   m_pImpl(pi_rpImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Importer::~Importer ()
    {

    }

ImporterImpl& Importer::GetImpl ()
    {
    return *m_pImpl;
    }

const ImporterImpl& Importer::GetImpl () const
    {
    return *m_pImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterImpl::ImporterImpl     (const Plugin::ExtractorRegistry::CreatorRange&      foundCreatorRange,
                                const SourceCPtr&                                   sourcePtr,
                                const SinkPtr&                                      sinkPtr,
                                const ImportPolicy&                                 policy,
                                const FilterFactory&                                filterFactory,
                                Log&                                         log)
    :   m_pSource(sourcePtr),
        m_sinkPtr(sinkPtr),
        m_sourceTypeSelectionPolicy(policy.GetSourceTypeSelection().Clone()),
        m_targetTypeSelectionPolicy(policy.GetTargetTypeSelection().Clone()),
        m_allocatorP(policy.GetMemoryAllocationPolicy().Clone()),
        m_sourceDesc(sourcePtr->GetDescriptor()),
        m_targetDesc(sinkPtr->GetDescriptor()),
        m_filterFactory(filterFactory),
        m_warningLog(log)
    {
    std::copy(foundCreatorRange.first, foundCreatorRange.second, std::back_inserter(m_pluginCreators));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterImpl::~ImporterImpl  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Importer::Status Importer::Import  (const ImportSequence&   sequence,
                                    const ImportConfig&     config)
    {
    try
        {
        Config importConfig;
        config.Accept(importConfig);

        m_pImpl->Import(sequence, importConfig);
        return S_SUCCESS;
        }
    catch (const Exception&)
        {
        return S_ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Importer::Status Importer::Import  (const ImportSequence&   sequence)
    {
    try
        {
        m_pImpl->Import(sequence, ImporterImpl::DefaultImportConfig());
        return S_SUCCESS;
        }
    catch (const Exception&)
        {
        return S_ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Importer::Status Importer::Import  (const ImportCommand&    command,
                                    const ImportConfig&     config)
    {
    try
        {
        Config importConfig;
        config.Accept(importConfig);

        m_pImpl->Import(command, importConfig);
        return S_SUCCESS;
        }
    catch (const Exception&)
        {
        return S_ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Importer::Status Importer::Import  (const ImportCommand&    command)
    {
    try
        {
        m_pImpl->Import(command, ImporterImpl::DefaultImportConfig());
        return S_SUCCESS;
        }
    catch (const Exception&)
        {
        return S_ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterImpl::Import      (const ImportSequence&   sequence,
                                const Config&           config)
    {
    CommandVisitor sequenceVisitor(*this, config);
    sequence.Accept(sequenceVisitor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterImpl::Import      (const ImportCommand&    command,
                                const Config&           config)
    {
    CommandVisitor commandVisitor(*this, config);
    command.Accept(commandVisitor);
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterImpl::Import  (UInt                    sourceLayerID,
                            const DataTypeFamily&   sourceTypeFamily,
                            UInt                    targetLayerID,
                            const DataTypeFamily&   targetTypeFamily,
                            const Config&           config)
    {
    if (!m_sourceDesc.IsValidLayer(sourceLayerID) || 
        !m_targetDesc.IsValidLayer(targetLayerID))
        throw CustomException (L"Source/Target layer id is invalid");


    const LayerDesc& sourceLayerDesc = m_sourceDesc.GetLayer(sourceLayerID);

    LayerDesc::TypeCIterator sourceTypeIt = m_sourceTypeSelectionPolicy->Select(sourceTypeFamily, 
                                                                                sourceLayerDesc.TypesBegin(), 
                                                                                sourceLayerDesc.TypesEnd());
    if (sourceLayerDesc.TypesEnd() == sourceTypeIt)
        return; // Nothing to import

    const LayerDesc& targetLayerDesc = m_targetDesc.GetLayer(targetLayerID);
    LayerDesc::TypeCIterator targetTypeIt = m_targetTypeSelectionPolicy->Select(targetTypeFamily,
                                                                                targetLayerDesc.TypesBegin(),
                                                                                targetLayerDesc.TypesEnd());
    if(targetLayerDesc.TypesEnd() == targetTypeIt)
        throw CustomException (L"Target layer does not actually contain a type for specified type family");

    Import(sourceLayerID, *sourceTypeIt, targetLayerID, *targetTypeIt, config);

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterImpl::Import  (UInt                                sourceLayerID,
                            const DataType&                     sourceType,
                            UInt                                targetLayerID,
                            const DataTypeFamily&               targetTypeFamily,
                            const Config&                       config)
    {
    if (!m_targetDesc.IsValidLayer(targetLayerID))
        throw CustomException (L"Target layer id is invalid");

    const LayerDesc& targetLayerDesc = m_targetDesc.GetLayer(targetLayerID);
    LayerDesc::TypeCIterator targetTypeIt = m_targetTypeSelectionPolicy->Select(targetTypeFamily,
                                                                                targetLayerDesc.TypesBegin(),
                                                                                targetLayerDesc.TypesEnd());
    if(targetLayerDesc.TypesEnd() == targetTypeIt)
        throw CustomException (L"Target layer does not actually contain a type for specified type family");

    Import(sourceLayerID, sourceType, targetLayerID, *targetTypeIt, config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterImpl::Import  (UInt                        sourceLayerID,   
                            const DataType&             sourceType,
                            UInt                        targetLayerID,   
                            const DataType&             targetType,
                            const Config&               config)
    {
    Source& source = *m_pSource;

    ContentDescriptor descriptor = source.GetDescriptor();

    ContentDescriptor::iterator layerIt;
    layerIt = descriptor.FindLayerFor(targetLayerID);

    SourceImportConfig* sourceImportConf = source.GetSourceImportConfig();
    ScalableMeshData data = sourceImportConf->GetReplacementSMData();

    const InputExtractorCreator* factoryP = GetPluginCreatorFor(sourceType/*, data.IsGroundDetection()*/);
    if (0 == factoryP)
        return; // Nothing to import NTERAY: Return warning instead??


    ExtractionQuery extractionQuery(sourceLayerID, sourceType);

    PacketGroup srcPackets(factoryP->GetOutputCapacities(source, extractionQuery), GetAllocator());
    
    // NTERAY: This is weak... Should be sized in the _Assign method.
    PacketGroup dstPackets(targetType.GetDimensionOrgCount(), GetAllocator());


    InputExtractorPtr sourceExtractorPtr = factoryP->Create(source, 
                                                            extractionQuery,
                                                            srcPackets, 
                                                            config.GetExtractionConfig(),
                                                            m_warningLog);

    if (0 == sourceExtractorPtr.get())
        throw CustomException (L"Could not create importer");

    DataType adaptedSourceType(sourceType);
    factoryP->AdaptOutputType(adaptedSourceType);

    const FilterCreatorCPtr pFilterCreator = GetFilterCreatorFor(sourceLayerID, adaptedSourceType, 
                                                                 targetLayerID, targetType, 
                                                                 config);
    if (0 == pFilterCreator.get())
        throw CustomException (L"No filter found");

    const FilterPtr filter = pFilterCreator->Create(srcPackets, dstPackets, config.GetFilteringConfig(), m_warningLog);

    const BackInserterPtr sinkInserterPtr = m_sinkPtr->CreateBackInserterFor(dstPackets, 
                                                                             targetLayerID, targetType,
                                                                             m_warningLog);
    if (0 == sinkInserterPtr.get())
        throw CustomException (L"Could not create appropriate sink inserter");

    DRange3d range;
    range.low.x = DBL_MAX;
    range.low.y = DBL_MAX;
    range.low.z = DBL_MAX;
    range.high.x = -DBL_MAX;
    range.high.y = -DBL_MAX;
    range.high.z = -DBL_MAX;

    

    bool is3D;
    if(data.IsRepresenting3dData() == SMis3D::is3D)
        is3D = true;
    else if (data.IsRepresenting3dData() == SMis3D::is25D)
        is3D = false;
    else
        {
        is3D = descriptor.CanRepresent3dData();
        }

    if(data.IsGroundDetection())
        {
        //sourceExtractorPtr->SetGroundDetection(true);
        }

    //sinkInserterPtr->SetIs3dData(data.IsRepresenting3dData());
    sinkInserterPtr->SetIs3dData(is3D);

    for (bool HasData = true; HasData; HasData = sourceExtractorPtr->Next())
        {
        // NEEDS_WORK_SM entry point to calculate some stuff like extent
        sourceExtractorPtr->Read();
        filter->Run();
        // Filter points
        // NEEDS_WORK_SM : need to add this !!!!
        if(data.GetUpToDateState() == UpToDateState::PARTIAL_ADD)
            {
            range = data.GetExtentByLayer(targetLayerID);
            // NEEDS_WORK_SM : move to filter
            Filter(data.GetVectorRangeAdd(), dstPackets);
            }
        else
            CreateExtent(range, dstPackets);

        sinkInserterPtr->Write();
        }

    data.SetExtent(targetLayerID, range);
    sourceImportConf->SetReplacementSMData(data);
    }

void ImporterImpl::Filter(std::vector<DRange3d> vecRangeFilter, PacketGroup& dstSource)
    {
    Memory::ClassPacketProxy<IDTMFile::Point3d64f>        pointPacket;

    HPRECONDITION(1 <= dstSource.GetSize());
    
    size_t pointPacketInd = 0;

    if (dstSource.GetSize() > 1)
        {
        for (; pointPacketInd < dstSource.GetSize(); pointPacketInd++)
            {                            
            if (dstSource[pointPacketInd].GetRawPacket().IsPODLifeCycleCompatibleWith(sizeof(IDTMFile::Point3d64f)))
                {
                break;
                }
            }

        assert(pointPacketInd < dstSource.GetSize());
        }
#ifndef NDEBUG
    else
        {
        assert(dstSource[pointPacketInd].GetRawPacket().IsPODLifeCycleCompatibleWith(sizeof(IDTMFile::Point3d64f)));
        }
#endif

    dstSource[pointPacketInd].SetReadOnly(false);
    pointPacket.AssignTo(dstSource[pointPacketInd]);
    std::vector<IDTMFile::Point3d64f> pts;

    Memory::ClassPacketProxy<IDTMFile::Point3d64f>::iterator ptIt;
    Memory::ClassPacketProxy<IDTMFile::Point3d64f>::iterator ptIt2;
    size_t size = 0;

    ptIt = pointPacket.begin();
    ptIt2 = ptIt;
    while(ptIt != pointPacket.end() && ptIt2 != pointPacket.end())
        {
        bool isContained = false;
        double x = PointOp<IDTMFile::Point3d64f>::GetX(*ptIt2);
        double y = PointOp<IDTMFile::Point3d64f>::GetY(*ptIt2);
        double z = PointOp<IDTMFile::Point3d64f>::GetZ(*ptIt2);

        for(std::vector<DRange3d>::iterator it = vecRangeFilter.begin(); it != vecRangeFilter.end(); it++)
            {
            if(it->IsContained(x,y,z))
                {
                isContained = true;
                break;
                }
            }

        if(isContained)
            {
            (*ptIt)=(*ptIt2);
            ptIt++;
            ptIt2++;
            size++;
            }
        else
            ptIt2++;
        }

    pointPacket.SetSize(size);
    }

void ImporterImpl::CreateExtent(DRange3d& range, PacketGroup& dstSource)
    {
    size_t pointIndex = 0;

    Memory::ConstPacketProxy<IDTMFile::Point3d64f>        pointPacket;


    HPRECONDITION(1 <= dstSource.GetSize());
    
    size_t pointPacketInd = 0;

    if (dstSource.GetSize() > 1)
        {
        for (; pointPacketInd < dstSource.GetSize(); pointPacketInd++)
            {                            
            if (dstSource[pointPacketInd].GetRawPacket().IsPODLifeCycleCompatibleWith(sizeof(IDTMFile::Point3d64f)))
                {
                break;
                }
            }

        assert(pointPacketInd < dstSource.GetSize());
        }
#ifndef NDEBUG
    else
        {
        assert(dstSource[pointPacketInd].GetRawPacket().IsPODLifeCycleCompatibleWith(sizeof(IDTMFile::Point3d64f)));
        }
#endif

    pointPacket.AssignTo(dstSource[pointPacketInd]);
    size_t countOfPoints = pointPacket.GetSize();

    while (pointIndex < countOfPoints)
        {
        double x = PointOp<IDTMFile::Point3d64f>::GetX(pointPacket.Get()[pointIndex]);
        double y = PointOp<IDTMFile::Point3d64f>::GetY(pointPacket.Get()[pointIndex]);
        double z = PointOp<IDTMFile::Point3d64f>::GetZ(pointPacket.Get()[pointIndex]);

        if (range.high.x < x)
            range.high.x = x;
        if (range.high.y < y)
            range.high.y = y;
        if (range.high.z < z)
            range.high.z = z;

        if (range.low.x > x)
            range.low.x = x;
        if (range.low.y > y)
            range.low.y = y;
        if (range.low.z > z)
            range.low.z = z;
        pointIndex++;
        }
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const InputExtractorCreator* ImporterImpl::GetPluginCreatorFor (const DataType& pi_rSourceType)
    {
    struct PluginSupports
        {
        const DataType& m_rType;
        explicit PluginSupports (const DataType& pi_rType) : m_rType(pi_rType) {}

        bool operator() (const InputExtractorCreator& pi_pPlugin) const
            {
            return pi_pPlugin.Supports(m_rType);
            }
        };    

    PluginCreatorList::const_iterator foundPluginCreatorIt = 
        std::find_if(m_pluginCreators.begin(), m_pluginCreators.end(), PluginSupports(pi_rSourceType));

    return (m_pluginCreators.end() == foundPluginCreatorIt) ? 0 : 
                                                               &*foundPluginCreatorIt;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr ImporterImpl::GetFilterCreatorFor    (UInt                    sourceLayerID,
                                                        const DataType&         sourceType,
                                                        UInt                    targetLayerID,
                                                        const DataType&         targetType,
                                                        const Config&           config)
    {
    const LayerDesc& sourceLayerDesc = m_sourceDesc.GetLayer(sourceLayerID);
    const LayerDesc& targetLayerDesc = m_targetDesc.GetLayer(targetLayerID);

    const GCS& sourceGCS = sourceLayerDesc.GetGCS();
    const GCS& targetGCS = targetLayerDesc.GetGCS();

    const DRange3d* extentP = sourceLayerDesc.GetExtentP();

    const bool hasSourceGCS = !sourceGCS.IsNull();
    const bool hasTargetGCS = !targetGCS.IsNull();

    const GCS& adaptedSourceGCS = (!hasSourceGCS && hasTargetGCS && config.HasDefaultSourceGCS()) ? config.GetDefaultSourceGCS() : sourceGCS;
    const GCS& adaptedTargetGCS = (!hasTargetGCS && hasSourceGCS && config.HasDefaultTargetGCS()) ? config.GetDefaultTargetGCS() : targetGCS;

    return m_filterFactory.FindCreatorFor (sourceType, targetType,
                                           adaptedSourceGCS, adaptedTargetGCS, extentP, 
                                           config.GetSourceFilters(), config.GetTargetFilters());
    }




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImporterFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    const ImportPolicy                  m_policy;                 
    const ExtractorRegistry&            m_extractorRegistry;
    const FilterFactory                 m_filterFactory;
    Log&                         m_warningLog;

    explicit                            Impl                               (const ImportPolicy&             policy,
                                                                            const ExtractorRegistry&        extractorRegistry,
                                                                            const FilterFactory&            filterFactory,
                                                                            Log&                     log)
        :   m_policy(policy),
            m_extractorRegistry(extractorRegistry),
            m_filterFactory(filterFactory),
            m_warningLog(log)
        {

        }



    ImporterPtr                         CreateImporterFor                  (const SourceCPtr&               sourcePtr,
                                                                            const SinkPtr&                  sinkPtr) const;

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterFactory::ImporterFactory (Log& log)
    :   m_pImpl(new Impl (ImportPolicy(), Plugin::ExtractorRegistry::GetInstance(), FilterFactory(log), log))
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterFactory::ImporterFactory   (const ImportPolicy& policy,
                                    Log&         log)
    :   m_pImpl(new Impl (policy, Plugin::ExtractorRegistry::GetInstance(), FilterFactory(log), log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterFactory::ImporterFactory   (const ImportPolicy&         policy,
                                    const ExtractorRegistry&    extractorRegistry,
                                    const FilterFactory&        filterFactory,
                                    Log&                 log)
    :   m_pImpl(new Impl (policy, extractorRegistry, filterFactory, log))
    {
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterFactory::ImporterFactory (const ImporterFactory& rhs)
    :   m_pImpl(rhs.m_pImpl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterFactory::~ImporterFactory ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterPtr ImporterFactory::Create    (const SourceCPtr&       sourcePtr,
                                        const SinkPtr&          sinkPtr) const
    {
    return m_pImpl->CreateImporterFor(sourcePtr, sinkPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterPtr ImporterFactory::Impl::CreateImporterFor   (const SourceCPtr&           sourcePtr,
                                                        const SinkPtr&              sinkPtr) const
    {
    try
        {
        if (0 == sourcePtr.get() || 0 == sinkPtr.get())
            {
            assert(!"Trying to create importer for null source or null sink!!");
            return 0;
            }

        Plugin::ExtractorRegistry::CreatorRange foundCreatorRange
            = m_extractorRegistry.FindAppropriateCreator(sourcePtr->GetClassID());


        // Return early when importer could not import anything
        if (foundCreatorRange.first == foundCreatorRange.second)
            {
            assert(!"This plug-in import nothing?");
            return 0; 
            }

        return new Importer(new ImporterImpl(foundCreatorRange, sourcePtr, sinkPtr, m_policy, m_filterFactory, m_warningLog));

        }
    catch (const Exception&)
        {
        return 0;
        }
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
