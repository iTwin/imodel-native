/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceDescription.cpp $
|    $RCSfile: ScalableMeshSourceDescription.cpp,v $
|   $Revision: 1.20 $
|       $Date: 2012/01/06 16:30:11 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
  
#include <ScalableMesh/IScalableMeshSourceDescription.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>

#include <ScalableMesh/Import/ContentDescriptor.h>
#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/SourceReference.h>
#include <ScalableMesh/Import/Exceptions.h>

#include <ScalableMesh/Import/Error/Source.h>
#include <ScalableMesh/Import/ImportSequenceVisitor.h>
#include <ScalableMesh/Import/ImportSequence.h>
#include <ScalableMesh/Import/Command/All.h>

#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh/IScalableMeshPolicy.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* NOTE: Definition is located in creator module.
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef CreateSourceRefFromIDTMSource (const IDTMSource& source, const WString& stmPath);


struct SourceLayerDescriptorHolder
    {
    SourceLayerDescriptor           m_held;

                                    SourceLayerDescriptorHolder        (const SourceLayerDescriptor&    rhs) 
        : m_held(rhs) 
        {}
    SourceLayerDescriptorHolder&    operator=                          (const SourceLayerDescriptor&    rhs) 
        { m_held = rhs; return *this; }

    explicit                        SourceLayerDescriptorHolder        (UInt                            layer,
                                                                        const Import::LayerDescriptor&  descriptor)
        :   m_held(layer, descriptor)
        {}

    };

namespace {
static_assert(sizeof(SourceLayerDescriptorHolder) == sizeof(SourceLayerDescriptor), "");

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceLayerDescriptor::SourceLayerDescriptor   (UInt                    layer,
                                                const LayerDescriptor&  descriptor)
    :   m_id(layer),
        m_descriptorP(&descriptor),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceLayerDescriptor::SourceLayerDescriptor (const SourceLayerDescriptor& rhs)
    :   m_id(rhs.m_id),
        m_descriptorP(rhs.m_descriptorP),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceLayerDescriptor& SourceLayerDescriptor::operator= (const SourceLayerDescriptor& rhs)

    {
    m_id = rhs.m_id;
    m_descriptorP = rhs.m_descriptorP;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt SourceLayerDescriptor::GetID () const
    {
    return m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* SourceLayerDescriptor::GetName () const
    {
    return m_descriptorP->GetNameCStr();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LayerDescriptor& SourceLayerDescriptor::GetDescriptor () const
    {
    return *m_descriptorP;
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceDescriptor::Impl
    {
    ContentDescriptor               m_descriptor;
    vector<SourceLayerDescriptorHolder>   
                                    m_layerSelection;
    vector<IncompleteType>          m_incompleteTypes;


    explicit                        Impl                                       (const ContentDescriptor&        descriptor,
                                                                                const SourceImportConfig&       config)
        :   m_descriptor(descriptor)
        {
        InitLayerSelection(config.GetSequence());

        if (descriptor.HoldsIncompleteTypes())
            InitLayersWithIncompleteTypes();
        }

    void                            InitLayerSelection                         (const ImportSequence&           importSequence)
        {
        const UInt layerCount = m_descriptor.GetLayerCount();
        
        if (1 == layerCount)
            return InitSingleLayerSelection(importSequence);
        if (0 == layerCount)
            return;

        return InitMultipleLayerSelection(importSequence);
        }

    void                            InitSingleLayerSelection                   (const ImportSequence&           importSequence);
    void                            InitMultipleLayerSelection                 (const ImportSequence&           importSequence);




    void                            InitLayersWithIncompleteTypes              ();

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceDescriptor::Impl::InitSingleLayerSelection (const ImportSequence&  importSequence)
    {
    class SingleLayerSelectionVisitor : public IImportSequenceVisitor
        {
        const LayerDescriptor&          m_layerDesc;
        bool                            m_selected;
        

        void                            SelectAll                  ()
            { m_selected = true; }

        void                            Select                     (const UInt                                  layerID)
            { m_selected = m_selected || 0 == layerID; }

        void                            SelectIfHasType            (const UInt                                  layerID,
                                                                    const DataTypeFamily&                       type)
            {
            m_selected = m_selected || 
                         (0 == layerID && m_layerDesc.TypesEnd() != m_layerDesc.FindTypeFor(type));
            }

        void                            SelectAllWithType          (const DataTypeFamily&                       type)
            { m_selected = m_selected || m_layerDesc.TypesEnd() != m_layerDesc.FindTypeFor(type); }

        virtual void                    _Visit                     (const ImportAllCommand&                     command) override
            { SelectAll(); }
        virtual void                    _Visit                     (const ImportAllToLayerCommand&              command) override
            { SelectAll(); }
        virtual void                    _Visit                     (const ImportAllToLayerTypeCommand&          command) override
            { SelectAll(); }
        virtual void                    _Visit                     (const ImportAllToTypeCommand&               command) override
            { SelectAll(); }

        virtual void                    _Visit                     (const ImportLayerCommand&                   command) override
            { Select(command.GetSourceLayer()); }
        virtual void                    _Visit                     (const ImportLayerToLayerCommand&            command) override
            { Select(command.GetSourceLayer()); }
        virtual void                    _Visit                     (const ImportLayerToLayerTypeCommand&        command) override
            { Select(command.GetSourceLayer()); }
        virtual void                    _Visit                     (const ImportLayerToTypeCommand&             command) override
            { Select(command.GetSourceLayer()); }

        virtual void                    _Visit                     (const ImportLayerTypeCommand&               command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerCommand&        command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerTypeCommand&    command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportLayerTypeToTypeCommand&         command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }

        virtual void                    _Visit                     (const ImportTypeCommand&                    command) override
            { SelectAllWithType(command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportTypeToLayerCommand&             command) override
            { SelectAllWithType(command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportTypeToLayerTypeCommand&         command) override
            { SelectAllWithType(command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportTypeToTypeCommand&              command) override
            { SelectAllWithType(command.GetSourceType()); }

    public:
        void                            CopyTo                     (vector<SourceLayerDescriptorHolder>&        selection) const
            {
            if (m_selected)
                selection.push_back(SourceLayerDescriptorHolder(0, m_layerDesc));
            }

        explicit                        SingleLayerSelectionVisitor      (ContentDescriptor&                          descriptor)
            :   m_layerDesc(*descriptor.LayersBegin()),
                m_selected(false)
            {
            assert(1 == descriptor.GetLayerCount());
            }
        };

    SingleLayerSelectionVisitor visitor(m_descriptor);
    importSequence.Accept(visitor);


    visitor.CopyTo(m_layerSelection);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceDescriptor::Impl::InitMultipleLayerSelection (const ImportSequence&  importSequence)
    {
    class MultipleLayerSelectionVisitor : public IImportSequenceVisitor
        {
        typedef vector<bool>         SelectedList;

        typedef ContentDescriptor::const_iterator
                                        LayerDescCIter;

        const LayerDescCIter            m_layersBegin;
        const LayerDescCIter            m_layersEnd;
        SelectedList                    m_selected;
        

        void                            SelectAll                  ()
            {
            fill(m_selected.begin(), m_selected.end(), true);
            }

        void                            Select                     (const UInt                                  layerID)
            {
            if (layerID >= m_selected.size())
                return;

            m_selected[layerID] = true;
            }

        void                            SelectIfHasType            (const UInt                                  layerID,
                                                                    const DataTypeFamily&                       type)
            {
            const LayerDescCIter layerIt(m_layersBegin + layerID);
            if (m_layersEnd <= layerIt)
                return;

            if (layerIt->TypesEnd() == layerIt->FindTypeFor(type))
                return;

            m_selected[layerID] = true;
            }

        void                            SelectAllWithType          (const DataTypeFamily&                       type)
            {
            struct SelectLayer
                {
                const DataTypeFamily& m_type;
                explicit SelectLayer (const DataTypeFamily& type) : m_type(type) {}

                bool operator () (bool alreadySelected, const LayerDescriptor& layer) const 
                    { return alreadySelected || layer.TypesEnd() != layer.FindTypeFor(m_type); } 
                };

            std::transform(m_selected.begin(), m_selected.end(), 
                           m_layersBegin, 
                           m_selected.begin(), 
                           SelectLayer(type));
            }

        virtual void                    _Visit                     (const ImportAllCommand&                     command) override
            { SelectAll(); }
        virtual void                    _Visit                     (const ImportAllToLayerCommand&              command) override
            { SelectAll(); }
        virtual void                    _Visit                     (const ImportAllToLayerTypeCommand&          command) override
            { SelectAll(); }
        virtual void                    _Visit                     (const ImportAllToTypeCommand&               command) override
            { SelectAll(); }

        virtual void                    _Visit                     (const ImportLayerCommand&                   command) override
            { Select(command.GetSourceLayer()); }
        virtual void                    _Visit                     (const ImportLayerToLayerCommand&            command) override
            { Select(command.GetSourceLayer()); }
        virtual void                    _Visit                     (const ImportLayerToLayerTypeCommand&        command) override
            { Select(command.GetSourceLayer()); }
        virtual void                    _Visit                     (const ImportLayerToTypeCommand&             command) override
            { Select(command.GetSourceLayer()); }

        virtual void                    _Visit                     (const ImportLayerTypeCommand&               command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerCommand&        command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerTypeCommand&    command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportLayerTypeToTypeCommand&         command) override
            { SelectIfHasType(command.GetSourceLayer(), command.GetSourceType()); }

        virtual void                    _Visit                     (const ImportTypeCommand&                    command) override
            { SelectAllWithType(command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportTypeToLayerCommand&             command) override
            { SelectAllWithType(command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportTypeToLayerTypeCommand&         command) override
            { SelectAllWithType(command.GetSourceType()); }
        virtual void                    _Visit                     (const ImportTypeToTypeCommand&              command) override
            { SelectAllWithType(command.GetSourceType()); }

    public:
        void                            CopyTo                     (vector<SourceLayerDescriptorHolder>&        selection) const
            {
            SelectedList::const_iterator selectedIt = m_selected.begin();
            UInt layerID = 0;

            for (ContentDescriptor::const_iterator layerIt = m_layersBegin;
                 m_layersEnd != layerIt;
                 ++layerIt, ++selectedIt, ++layerID)
                {
                if (*selectedIt)
                    selection.push_back(SourceLayerDescriptorHolder(layerID, *layerIt));
                }
            }

        explicit                        MultipleLayerSelectionVisitor      (ContentDescriptor&                          descriptor)
            :   m_layersBegin(descriptor.LayersBegin()),
                m_layersEnd(descriptor.LayersEnd()),
                m_selected(std::distance(m_layersBegin, m_layersEnd), false)
            {
            }
        };

    MultipleLayerSelectionVisitor visitor(m_descriptor);
    importSequence.Accept(visitor);


    visitor.CopyTo(m_layerSelection);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceDescriptor::Impl::InitLayersWithIncompleteTypes              ()
    {
    for (ContentDescriptor::const_iterator layerIt = m_descriptor.LayersBegin(), layerEnd = m_descriptor.LayersEnd();
         layerIt != layerEnd;
         ++layerIt)
        {
        if (layerIt->HoldsIncompleteTypes())
            {
            for (LayerDescriptor::const_iterator typeIter = layerIt->TypesBegin(), typeEnd = layerIt->TypesEnd();
                 typeIter != typeEnd;
                 ++typeIter)
                {
                m_incompleteTypes.push_back(IncompleteType(m_descriptor.GetLayerIDFor(layerIt), typeIter->GetType()));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::IncompleteType::IncompleteType   (UInt                layer,
                                                    const DataType&     type)
    :   m_layerID(layer),
        m_type(type)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt SourceDescriptor::IncompleteType::GetLayerID () const
    {
    return m_layerID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& SourceDescriptor::IncompleteType::GetType () const
    {
    return m_type;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::SourceDescriptor (Impl* implP)
    :   m_implP(implP)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::~SourceDescriptor ()
    {

    }


namespace {
const SourceFactory         SOURCE_FACTORY (GetSourceFactory());

inline SourceDescriptor::Status MapStatus(const SourceFactory::Status factoryStatus)
    {
    assert(SourceDescriptor::S_QTY > factoryStatus);

    switch(factoryStatus)
        {
        case SourceFactory::S_SUCCESS:
            return SourceDescriptor::S_SUCCESS;
        case SourceFactory::S_ERROR:
            return SourceDescriptor::S_ERROR;
        case SourceFactory::S_ERROR_NOT_SUPPORTED:
            return SourceDescriptor::S_ERROR_NOT_SUPPORTED;
        case SourceFactory::S_ERROR_NOT_FOUND:
            return SourceDescriptor::S_ERROR_NOT_FOUND;
        default:
            return SourceDescriptor::S_ERROR;
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline SourceDescriptorCPtr  SourceDescriptor::CreateFor   (const Import::Source&       source,
                                                            const SourceImportConfig&   config)
    {
    return new SourceDescriptor (new Impl(source.GetDescriptor(), config));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptorCPtr SourceDescriptor::CreateOriginalFor (const IDTMSource& source)
    {
    Status status(S_SUCCESS);
    StatusInt statusEx(0);
    return CreateOriginalFor(source, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptorCPtr SourceDescriptor::CreateOriginalFor (const IDTMSource&     source,
                                                          Status&               status)
    {
    StatusInt statusEx(0);
    return CreateOriginalFor(source, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptorCPtr SourceDescriptor::CreateOriginalFor (const IDTMSource&     source,
                                                          Status&               status,
                                                          StatusInt&            statusEx)
    {
    try
        {
        SourceFactory::Status factoryStatus(SourceFactory::S_SUCCESS);

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(source, L""));
        const SourcePtr sourcePtr(SOURCE_FACTORY.Create(sourceRef, factoryStatus, statusEx));

        status = MapStatus(factoryStatus);

        if (0 == sourcePtr.get())
            return 0;

        return CreateFor(*sourcePtr, source.GetConfig());
        }
    catch (const SourceNotFoundException&)
        {
        status = S_ERROR_NOT_FOUND;
        return 0;
        }
    catch (const exception&)
        {
        return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptorCPtr SourceDescriptor::CreateFor (const IDTMSource& source)
    {
    Status status(S_SUCCESS);
    StatusInt statusEx(0);
    return CreateFor(source, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptorCPtr SourceDescriptor::CreateFor (const IDTMSource& source,
                                                  Status&               status)
    {
    StatusInt statusEx(0);
    return CreateFor(source, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptorCPtr SourceDescriptor::CreateFor (const IDTMSource& source,
                                                  Status&               status,
                                                  StatusInt&            statusEx)
    {
    try
        {
        SourceFactory::Status factoryStatus(SourceFactory::S_SUCCESS);

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(source, L""));
        const SourcePtr sourcePtr(Configure(SOURCE_FACTORY.Create(sourceRef, factoryStatus, statusEx), 
                                            source.GetConfig().GetContentConfig(),
                                            GetLog()));

        status = MapStatus(factoryStatus);

        if (0 == sourcePtr.get())
            return 0;

        return CreateFor(*sourcePtr, source.GetConfig());
        }
    catch (const SourceNotFoundException&)
        {
        status = S_ERROR_NOT_FOUND;
        return 0;
        }
    catch (const exception&)
        {
        return 0;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceDescriptor::CreateOriginalSourceFor (const IDTMSource&  source)
    {
    Status status(S_SUCCESS);
    StatusInt statusEx(0);
    return CreateOriginalSourceFor(source, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceDescriptor::CreateOriginalSourceFor (const IDTMSource&      source,
                                                     Status&                status)
    {
    StatusInt statusEx(0);
    return CreateOriginalSourceFor(source, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceDescriptor::CreateOriginalSourceFor (const IDTMSource&      source,
                                                     Status&                status,
                                                     StatusInt&             statusEx)
    {
    try
        {
        SourceFactory::Status factoryStatus(SourceFactory::S_SUCCESS);

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(source, L""));
        const SourcePtr sourcePtr(SOURCE_FACTORY.Create(sourceRef, factoryStatus, statusEx));

        status = MapStatus(factoryStatus);

        return sourcePtr;
        }
    catch (const SourceNotFoundException&)
        {
        status = S_ERROR_NOT_FOUND;
        return 0;
        }
    catch (const exception&)
        {
        return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceDescriptor::ConfigureSource (const Import::SourcePtr&    sourcePtr,
                                                    const SourceImportConfig&   config)
    {
    return Configure(sourcePtr, config.GetContentConfig(), GetLog());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceDescriptor::HoldsIncompleteTypes () const
    {
    return m_implP->m_descriptor.HoldsIncompleteTypes();
    }

bool SourceDescriptor::IsPod () const
    {
    return m_implP->m_descriptor.IsPod();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::IncompleteTypeCIter SourceDescriptor::IncompleteTypesBegin () const
    {
    return &*m_implP->m_incompleteTypes.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::IncompleteTypeCIter SourceDescriptor::IncompleteTypesEnd () const
    {
    return &*m_implP->m_incompleteTypes.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt SourceDescriptor::GetLayerSelectionSize () const
    {
    return static_cast<UInt>(m_implP->m_layerSelection.size());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::LayerCIter SourceDescriptor::LayerSelectionBegin () const
    {
    return &m_implP->m_layerSelection.begin()->m_held;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceDescriptor::LayerCIter SourceDescriptor::LayerSelectionEnd () const
    {
    return &m_implP->m_layerSelection.end()->m_held;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt SourceDescriptor::GetLayerCount () const
    {
    return m_implP->m_descriptor.GetLayerCount();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ContentDescriptor& SourceDescriptor::GetDescriptor () const
    {
    return m_implP->m_descriptor;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
