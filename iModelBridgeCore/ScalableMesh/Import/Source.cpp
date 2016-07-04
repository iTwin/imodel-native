/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/Source.cpp $
|    $RCSfile: Source.cpp,v $
|   $Revision: 1.18 $
|       $Date: 2011/11/22 20:04:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Warnings.h>
#include <ScalableMesh/Import/Exceptions.h>

#include <ScalableMesh/Import/SourceReferenceVisitor.h>
#include <ScalableMesh/Import/Plugin/SourceRegistry.h>

#include <ScalableMesh/Import/Plugin/SourceV0.h>
#include <ScalableMesh/Import/ContentConfig.h>

#include <ScalableMesh/Import/ImportSequence.h>

#include "InternalSourceHandler.h"

#include "SourcePlugin.h"

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Source* InternalSourceHandler::CreateFromBase (Base* baseP)
    {
    if (0 == baseP)
        return 0;

    return new Source(baseP);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InternalSourceHandler::Base& InternalSourceHandler::GetOriginalBaseFor (Source& source)
    {
    return source.m_originalBase;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const InternalSourceHandler::Base& InternalSourceHandler::GetOriginalBaseFor (const Source& source)
    {
    return source.m_originalBase;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline InternalSourceHandler::Base& InternalSourceHandler::GetBaseFor (Source& source)
    {
    return *source.m_baseP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline const InternalSourceHandler::Base& InternalSourceHandler::GetBaseFor (const Source& source)
    {
    return *source.m_baseP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline const ContentDescriptor& InternalSourceHandler::GetDescriptorFor (const Source& source)
    {
    return source.m_baseP->GetDescriptor();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Source::Source  (Base* baseP)
    :   m_baseP(baseP),
        m_classID(baseP->_GetClassID()),
        m_originalBase(baseP->_ResolveOriginal())
    {
    assert(0 != m_classID);
    m_sourceImportConf = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Source::~Source ()
    {
    try
        {
        const Status errorCode = Close();
        assert(S_SUCCESS == errorCode);
        }
    catch (...)
        {
        assert(!"UncaughException!");
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Source::Status Source::Close ()
    {
    if (0 == m_baseP.get())
        return S_SUCCESS; // Source already closed

    try
        {
        m_baseP->_Close();
        }
    catch (const Exception& ex)
        {
        const StatusInt errorCode = ex.GetErrorCode();
        assert(BSIERROR == errorCode);
        return S_ERROR;
        }
    m_baseP.reset();
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* NOTE: This method is no throw as throwing _CreateDescriptor method was forced to
*       be called on source creation. So any exiting source comes with a valid/complete
*       descriptor. See SourceFactory::Impl::CreateSourceFor.
*       
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ContentDescriptor& Source::GetDescriptor () const
    {
    assert(0 != m_baseP.get());
    return m_baseP->GetDescriptor();
    }


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* Source::GetTypeCStr () const
    {
    assert(0 != m_baseP.get());
    return m_baseP->_GetType();
    }



namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConfiguredSourceDecorator : public Plugin::V0::SourceBase
    {
private:
    SourcePtr                       m_originalSourcePtr;
    ContentDescriptor               m_contentDesc;

    virtual ClassID                 _GetClassID                        () const override 
        { 
        return m_originalSourcePtr->GetClassID(); 
        }

    virtual SourceBase&             _ResolveOriginal                   () override
        {
        return InternalSourceHandler::GetOriginalBaseFor(*m_originalSourcePtr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                             () override
        {
        m_originalSourcePtr->Close();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor       _CreateDescriptor                  () const override
        {
        return m_contentDesc;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _ExtendDescription                 (ContentDescriptor&          description) const
        {
        // TDORAY: What do we do here?
        assert(!"Needs work!");
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*         _GetType                           () const override
        {
        return m_originalSourcePtr->GetTypeCStr();
        }


public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        ConfiguredSourceDecorator          (const SourcePtr&            sourcePtr,
                                                                        const ContentDescriptor&    contentDesc) 
        :   m_originalSourcePtr(sourcePtr),
            m_contentDesc(contentDesc)
        {
        }

    };

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr Configure    (const SourcePtr&        sourcePtr,
                        const ContentConfig&    config,
                        Log&                    log)
    {
    return Configure(sourcePtr, config, ContentConfigPolicy(), log);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr Configure    (const SourcePtr&                sourcePtr,
                        const ContentConfig&            config,
                        const ContentConfigPolicy&      policy, 
                        Log&                            log)
    {
    if (0 == sourcePtr.get())
        return 0;

    if (config.IsEmpty())
        return sourcePtr; // Nothing to configure. Return original.

    ContentDescriptor newContentDesc(sourcePtr->GetDescriptor());
    ContentDescriptor::Status status = newContentDesc.Configure(config, policy, log);

    if (ContentDescriptor::S_SUCCESS != status)
        return 0;

    return InternalSourceHandler::CreateFromBase(new ConfiguredSourceDecorator(sourcePtr, newContentDesc));
    }


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    const Registry&                                 m_registry;
    Log&                                            m_log;

    explicit                                        Impl                                   (const Registry&         registry,
                                                                                            Log&                    log)
        :   m_registry(registry),
            m_log(log)
        {
        }

    template <typename SourceRefT>
    SourcePtr                                       CreateSourceFor                        (const SourceRefT&       sourceRef,
                                                                                            Status&                 status) const;
    template <typename SourceRefT>
    SourcePtr                                       CreateSourceFor                        (const SourceRefT&       sourceRef,
                                                                                            Status&                 status,
                                                                                            StatusInt&              statusEx) const;

    template <typename CreatorT, typename SourceRefT>
    SourcePtr                                       CreateSourceFromCreator                (const CreatorT*         creatorP,
                                                                                            const SourceRefT&       sourceRef) const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFactory::SourceFactory (Log& log)
    :   m_pImpl(new Impl(Plugin::SourceRegistry::GetInstance(), log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFactory::SourceFactory (const Registry&   registry,
                              Log&       log)
    :   m_pImpl(new Impl(registry, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFactory::SourceFactory (const SourceFactory& rhs)
    :   m_pImpl(rhs.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFactory::~SourceFactory ()
    {
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceFactory::Create   (const SourceRef&    sourceRef) const
    {
    Status status(S_SUCCESS);
    StatusInt statusEx(0);
    return Create(sourceRef, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceFactory::Create    (const SourceRef&    sourceRef,
                                    Status&             status) const
    {
    StatusInt statusEx(0);
    return Create(sourceRef, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceFactory::Create    (const SourceRef&        sourceRef,
                                    Status&                 status,
                                    StatusInt&              statusEx) const
    {
    if (sourceRef.m_basePtr.get() != nullptr)
        {
        auto* dgnElementSource = dynamic_cast<DGNElementSourceRef*>(sourceRef.m_basePtr.get());
        if (dgnElementSource != nullptr)
            {
            return m_pImpl->CreateSourceFor(*dgnElementSource, status, statusEx);
            }
        auto* localFileSource = dynamic_cast<LocalFileSourceRef*>(sourceRef.m_basePtr.get());
        if (localFileSource != nullptr)
            {
            return m_pImpl->CreateSourceFor(*localFileSource, status, statusEx);
            }
        }

    return m_pImpl->CreateSourceFor(sourceRef, status, statusEx);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CreatorT, typename SourceRefT>
SourcePtr SourceFactory::Impl::CreateSourceFromCreator      (const CreatorT*     creatorP,
                                                             const SourceRefT&   sourceRef) const
    {
    if (0 == creatorP)
        throw PluginNotFoundException();

    return creatorP->Create(sourceRef, m_log);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SourceRefT>
SourcePtr SourceFactory::Impl::CreateSourceFor     (const SourceRefT&       sourceRef,
                                                    Status&                 status) const
    {
    StatusInt statusEx(0);
    return CreateSourceFor(sourceRef, status, statusEx);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SourceRefT>
SourcePtr SourceFactory::Impl::CreateSourceFor     (const SourceRefT&       sourceRef,
                                                    Status&                 status,
                                                    StatusInt&              statusEx) const
    {
    try
        {
        SourcePtr sourcePtr = CreateSourceFromCreator(m_registry.FindAppropriateCreator(sourceRef),
                                                      sourceRef);

        // Make sure we have a valid descriptor (as it may throws, we will catch any thrown exception here).
        const ContentDescriptor& descriptor = InternalSourceHandler::GetDescriptorFor(*sourcePtr);
        descriptor.GetName();

        status = (0 == sourcePtr.get()) ? S_ERROR : S_SUCCESS;
        return sourcePtr;
        }
    catch (const FileIOException& ex)
        {
        status = S_ERROR;
        statusEx = ex.GetErrorCode();
        return 0;
        }
    catch (const SourceNotFoundException& ex)
        {
        status = S_ERROR_NOT_FOUND;
        statusEx = ex.GetErrorCode();
        return 0;
        }
    catch (const PluginNotFoundException& ex)
        {
        status = S_ERROR_NOT_SUPPORTED;
        statusEx = ex.GetErrorCode();
        return 0;
        }
    catch (const Exception& ex)
        {
        status = S_ERROR;
        statusEx = ex.GetErrorCode();
        return 0;
        }  
    }







END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
