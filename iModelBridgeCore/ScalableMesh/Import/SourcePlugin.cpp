/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/SourcePlugin.cpp $
|    $RCSfile: SourcePlugin.cpp,v $
|   $Revision: 1.15 $
|       $Date: 2012/02/16 00:36:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include "SourcePlugin.h"
#include <ScalableMesh/Import/SourceReferenceVisitor.h>

#include <STMInternal/Foundations/PrivateStringTools.h>
#include "PluginRegistryHelper.h"

#include <ScalableMesh/Import/Plugin/SourceV0.h>
#include <ScalableMesh/Import/Source.h>

#include "InternalSourceHandler.h"

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceBase::Impl
    {   
    mutable std::auto_ptr<ContentDescriptor> m_fileDescP; 

    explicit                                    Impl                               ()
        :   m_fileDescP(0) 
        {
        }

    const ContentDescriptor&                    GetDescriptor                      (const SourceBase&           source) const
        {
        if (0 == m_fileDescP.get())
            {
            m_fileDescP.reset(new ContentDescriptor(source._CreateDescriptor()));
            assert(0 < m_fileDescP->GetLayerCount());
            }
        
        return *m_fileDescP;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase::SourceBase ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase::~SourceBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase& SourceBase::_ResolveOriginal ()
    {
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceBase::_ExtendDescription (ContentDescriptor& description) const
    {
    // Default. Nothing to do.
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ContentDescriptor& SourceBase::GetDescriptor () const
    {
    return m_pImpl->GetDescriptor(*this);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceCreatorBase::SourceCreatorBase ()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceCreatorBase::~SourceCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileSourceCreatorBase::Impl
    {
    typedef bvector<WString>                    ExtensionList;
    mutable ExtensionList                       m_extensions;

    typedef bvector<WString>::const_iterator
                                                ExtensionCIter;


    ExtensionCIter                              ExtensionsBegin                (const LocalFileSourceCreatorBase&   creator) const
        {
        if (m_extensions.empty())
            InitExtensions(creator);

        return m_extensions.begin();
        }

    ExtensionCIter                              ExtensionsEnd                  (const LocalFileSourceCreatorBase&   creator) const
        {
        if (m_extensions.empty())
            InitExtensions(creator);

        return m_extensions.end();
        }


    void                                        InitExtensions                 (const LocalFileSourceCreatorBase&   creator) const;

    bool                                        Supports                       (const LocalFileSourceCreatorBase&   creator,
                                                                                const LocalFileSourceRef&           pi_rSourceRef) const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalFileSourceCreatorBase::Impl::InitExtensions (const LocalFileSourceCreatorBase&   creator) const
    {
    assert(m_extensions.empty());

    const WString& extStr(creator._GetExtensions());

    WString::size_type ExtEnd = extStr.find(L'*');
    assert(0 == ExtEnd);

    WString::size_type ExtBegin = ExtEnd + 1;

    while (WString::npos != ExtEnd)
        {
        ExtEnd = extStr.find(L";*", ExtBegin);

        if (ExtBegin >= ExtEnd)
            break;

        if (L'.' == extStr[ExtBegin])
            ++ExtBegin;

        m_extensions.push_back(extStr.substr(ExtBegin, (WString::npos == ExtEnd) ? ExtEnd : ExtEnd - ExtBegin));

        ExtBegin = ExtEnd + 2;
        }

    assert(WString::npos == ExtEnd);
    assert(!m_extensions.empty());

    // Sort extensions in reverse order (starting from the last character) in order to be able to use
    // divide and conquer strategies to search the extension list.
    std::sort(m_extensions.begin(), m_extensions.end(), ReverseStringLessThan());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalFileSourceCreatorBase::Impl::Supports(const LocalFileSourceCreatorBase&   creator,
                                                const LocalFileSourceRef&           pi_rSourceRef) const
    {
    if (m_extensions.empty())
        InitExtensions(creator);

    // As extensions are guaranteed to be sorted in reverse order, use the same technique as used in source
    // plug-in search to known whether the path has one of the specified extension.
    ExtensionCIter foundIt 
        = std::upper_bound(m_extensions.begin(), m_extensions.end(), pi_rSourceRef.GetPath(), ReverseStringLessThan());

    if (foundIt == m_extensions.begin())
        return false;

    return pi_rSourceRef.HasExtension(*(--foundIt));

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreatorBase::LocalFileSourceCreatorBase ()
    :   m_pImpl(new Impl)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalFileSourceCreatorBase::DefaultSupports (const LocalFileSourceRef& pi_rSourceRef) const
    {
    return m_pImpl->Supports(*this, pi_rSourceRef);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreatorBase::~LocalFileSourceCreatorBase ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceCreatorBase::DGNElementSourceCreatorBase ()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceCreatorBase::~DGNElementSourceCreatorBase ()
    {
    }


END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE



BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourceCreator::SourceCreator (const Base& base)
    :   m_baseP(&base)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourceCreator::~SourceCreator () 
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceCreator::Supports (const SourceRef& sourceRef) const 
    {
    return m_baseP->_Supports(sourceRef);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr SourceCreator::Create    (const SourceRef&    sourceRef,
                                    Log&         log) const
    {
    return InternalSourceHandler::CreateFromBase(m_baseP->_Create(sourceRef, log));
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreator::LocalFileSourceCreator (const Base& base)
    :   m_baseP(&base)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreator::~LocalFileSourceCreator () 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalFileSourceCreator::Supports (const LocalFileSourceRef& sourceRef) const
    {
    return m_baseP->_Supports(sourceRef);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr LocalFileSourceCreator::Create   (const LocalFileSourceRef&   sourceRef,
                                            Log&                 log) const
    {
    return InternalSourceHandler::CreateFromBase(m_baseP->_Create(sourceRef, log));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreator::ExtensionCIter LocalFileSourceCreator::ExtensionsBegin () const
    {
    return m_baseP->m_pImpl->ExtensionsBegin(*m_baseP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreator::ExtensionCIter LocalFileSourceCreator::ExtensionsEnd () const
    {
    return m_baseP->m_pImpl->ExtensionsEnd(*m_baseP);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNElementID::DGNElementID  (uint32_t    type,
                                    uint32_t    handlerID)
    :   m_type(type),
        m_handlerID(handlerID)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool operator<  (const DGNElementID& lhs,
                        const DGNElementID& rhs)
    {
    return lhs.m_type < rhs.m_type || 
           (lhs.m_type == rhs.m_type && lhs.m_handlerID < rhs.m_handlerID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool operator== (const DGNElementID& lhs,
                        const DGNElementID& rhs)
    {
    return lhs.m_type == rhs.m_type && lhs.m_handlerID == rhs.m_handlerID;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceCreator::DGNElementSourceCreator (const Base& base)
    :   m_baseP(&base),
        m_elementID(base._GetElementType(), base._GetElementHandlerID())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceCreator::~DGNElementSourceCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DGNElementSourceCreator::GetElementType () const
    {
    return m_baseP->_GetElementType();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DGNElementSourceCreator::GetElementHandlerID () const
    {
    return m_baseP->_GetElementHandlerID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DGNElementSourceCreator::Supports (const DGNElementSourceRef&  sourceRef) const
    {
    return m_baseP->_Supports(sourceRef);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcePtr DGNElementSourceCreator::Create  (const DGNElementSourceRef&  sourceRef,
                                            Log&                 log) const
    {
    return InternalSourceHandler::CreateFromBase(m_baseP->_Create(sourceRef, log));
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE



BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceRegistryImpl
    {
    struct SourceReg : public PluginRegistry<SourceCreator>
        {

        const CreatorPlugin*                    FindAppropriateCreator             (const SourceRef&                sourceRef) const;

        } m_sourceReg;


    struct LocalFileReg : public PluginRegistry<LocalFileSourceCreator>
        {
        typedef std::pair<WString, LocalFileSourceCreator>
                                                ExtensionCreatorPair;
        typedef bvector<ExtensionCreatorPair>   PerExtensionCreatorMapping;
        typedef bvector<LocalFileSourceCreator::ID>
                                                CreatorIDList;


        mutable PerExtensionCreatorMapping      m_perExtensionCreators;
        mutable bool                            m_perExtensionCreatorsSorted;
        mutable CreatorIDList                   m_unregisteredlocalFileCreators;

        explicit                                LocalFileReg                          ()
            :   m_perExtensionCreatorsSorted(true)
            {
            }



        const CreatorPlugin*                    FindAppropriateCreator             (const LocalFileSourceRef&       sourceRef) const;


        LocalFileSourceCreator::ID              Register                           (const CreatorPlugin&            creator);
        void                                    Unregister                         (CreatorPluginID                 creatorID);

        bool                                    HasPostponedUnregister             () const { return !m_unregisteredlocalFileCreators.empty(); }
        void                                    RunPostponedUnregister             () const;

        } m_localFileReg;


    struct DGNElementReg : public PluginRegistry<DGNElementSourceCreator>
        {

        const CreatorPlugin*                    FindAppropriateCreator             (const DGNElementSourceRef&     sourceRef) const;

        } m_dgnElementReg;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceCreator* 
    SourceRegistryImpl::SourceReg::FindAppropriateCreator (const SourceRef&  pi_rSourceRef) const
    {
    struct SupportInputFile
        {
        const SourceRef& m_rSourceRef;
        explicit SupportInputFile (const SourceRef& pi_rSourceRef) : m_rSourceRef(pi_rSourceRef) {}

        bool operator()  (const SourceCreator&  rhs) const
            { return rhs.Supports(m_rSourceRef); }
        };

    return BruteForceFindCreator(SupportInputFile(pi_rSourceRef));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LocalFileSourceCreator* 
    SourceRegistryImpl::LocalFileReg::FindAppropriateCreator (const LocalFileSourceRef& pi_rSourceRef) const
    {
    // Use an ordering starting from the last character to the first. This avoids us
    // having to extract the extension from the path prior to searching. Path can
    // be used as is (as a keyword to search algorithms)
    struct ExtensionCompare
        {
        ReverseStringLessThan m_lessThan;

        bool operator () (const ExtensionCreatorPair& lhs, const ExtensionCreatorPair& rhs) const
            { return m_lessThan (lhs.first, rhs.first); }
        bool operator () (const ExtensionCreatorPair& lhs, const WString& rhs) const
            { return m_lessThan (lhs.first, rhs); }
        bool operator () (const WString& lhs, const ExtensionCreatorPair& rhs) const
            { return m_lessThan (lhs, rhs.first); }
        };

    struct ExtensionIsPartOf : std::unary_function<ExtensionCreatorPair, bool>
        {
        const LocalFileSourceRef& m_rSource;
        explicit ExtensionIsPartOf (const LocalFileSourceRef& pi_rSource) : m_rSource(pi_rSource) {}

        bool operator () (const ExtensionCreatorPair& rhs) const
            { return m_rSource.HasExtension(rhs.first); }
        };

    struct SupportInputFile
        {
        const LocalFileSourceRef& m_rSourceRef;
        explicit SupportInputFile (const LocalFileSourceRef& pi_rSourceRef) : m_rSourceRef(pi_rSourceRef) {}

        bool operator()  (const LocalFileSourceCreator&  rhs) const
            { return rhs.Supports(m_rSourceRef); }
        bool operator()  (const ExtensionCreatorPair&  rhs) const
            { return rhs.second.Supports(m_rSourceRef); }
        };

    if (HasPostponedUnregister())
        RunPostponedUnregister();

    // Try to find a plug-in using the file's extension
        {
        if (!m_perExtensionCreatorsSorted)
            {
            // Sort extensions in reverse order (starting from the last character) in order to be able to use
            // divide and conquer strategies to search the extension list.
            sort(m_perExtensionCreators.begin(), m_perExtensionCreators.end(), ExtensionCompare());
            m_perExtensionCreatorsSorted = true;
            }


        typedef std::reverse_iterator<PerExtensionCreatorMapping::const_iterator> ResultRevIter;
        typedef std::pair<ResultRevIter, ResultRevIter> ResultRevRange;

        // Find a range of compatible extensions starting from upper_bound where path would be placed (begin) in the 
        // list going down the list until extension is not part of the path (end).
        ResultRevRange foundRevRange(ResultRevIter(upper_bound(m_perExtensionCreators.begin(), 
                                                               m_perExtensionCreators.end(), 
                                                               pi_rSourceRef.GetPath(), ExtensionCompare())), 
                                     m_perExtensionCreators.rend());

        foundRevRange.second = find_if(foundRevRange.first, foundRevRange.second, std::not1(ExtensionIsPartOf(pi_rSourceRef)));
        

        // Most suitable plug-in has more chance to be the one having the longest extension that is part of the path and
        // extension length grow going toward the end of the list. Being the case use reverse iterators to iterate 
        // backward in order to apply "is supported" to the plug-in that has more chance to be appropriate.
        ResultRevIter foundIt 
            = find_if(foundRevRange.first, foundRevRange.second, SupportInputFile(pi_rSourceRef));


        if (foundIt != foundRevRange.second)
            {
            return &foundIt->second;
            }
        }

    // Try to find the plugin by brute force
    return BruteForceFindCreator(SupportInputFile(pi_rSourceRef));
    }



/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceCreator::ID SourceRegistryImpl::LocalFileReg::Register (const LocalFileSourceCreator& pi_rCreator)
    {
    struct AddExtMapping
        {
        const LocalFileSourceCreator& m_rCreator;
        PerExtensionCreatorMapping& m_rMapping;

        explicit AddExtMapping (const LocalFileSourceCreator& pi_rCreator, PerExtensionCreatorMapping& po_rMapping)
            :   m_rCreator(pi_rCreator), m_rMapping(po_rMapping) {}

        void operator () (const WString& pi_rExt)
            {
            m_rMapping.push_back(std::make_pair(pi_rExt, m_rCreator));
            }
        };

    if (HasPostponedUnregister())
        RunPostponedUnregister();

    m_perExtensionCreatorsSorted = false;

    std::for_each(pi_rCreator.ExtensionsBegin(), pi_rCreator.ExtensionsEnd(), 
             AddExtMapping(pi_rCreator, m_perExtensionCreators));

    return super_class::Register(pi_rCreator);
    }


void SourceRegistryImpl::LocalFileReg::RunPostponedUnregister () const
    {
    struct CreatorLessThan
        {
        bool operator () (const ExtensionCreatorPair& lhs, const ExtensionCreatorPair& rhs) const
            { return lhs.second.GetID() < rhs.second.GetID(); }
        bool operator () (const ExtensionCreatorPair& lhs, LocalFileSourceCreator::ID rhs) const
            { return lhs.second.GetID() < rhs; }
        bool operator () (LocalFileSourceCreator::ID lhs, const ExtensionCreatorPair& rhs) const
            { return lhs < rhs.second.GetID(); }
        };

    assert(HasPostponedUnregister());

    UnregisterCreators(m_unregisteredlocalFileCreators, m_perExtensionCreators, CreatorLessThan());
    m_unregisteredlocalFileCreators.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceRegistryImpl::LocalFileReg::Unregister (LocalFileSourceCreator::ID pi_CreatorID)
    {
    m_perExtensionCreatorsSorted = false;
    m_unregisteredlocalFileCreators.push_back(pi_CreatorID);

    super_class::Unregister(pi_CreatorID);
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DGNElementSourceCreator* SourceRegistryImpl::DGNElementReg::FindAppropriateCreator (const DGNElementSourceRef& sourceRef) const
    {
    struct CreatorLess : std::binary_function<DGNElementSourceCreator, DGNElementSourceCreator, bool>
        {
        bool operator () (const DGNElementSourceCreator& lhs, const DGNElementSourceCreator& rhs) const
            { return lhs.GetElementID() < rhs.GetElementID(); }

        bool operator () (const DGNElementSourceCreator& lhs, DGNElementID rhs) const
            { return lhs.GetElementID() < rhs; }

        bool operator () (DGNElementID lhs, const DGNElementSourceCreator& rhs) const
            { return lhs < rhs.GetElementID(); }
        };

    const CreatorListRange foundCreators = FindCreatorsFor(DGNElementID(sourceRef.GetElementType(), sourceRef.GetElementHandlerID()), 
                                                           CreatorLess());


    struct Supports
        {
        const DGNElementSourceRef& m_sourceRef;
        explicit Supports (const DGNElementSourceRef& sourceRef) : m_sourceRef(sourceRef) {}

        bool operator()  (const DGNElementSourceCreator&  rhs) const
            { return rhs.Supports(m_sourceRef); }
        };

    CreatorListRange::first_type foundCreatorIt 
            = find_if(foundCreators.first, foundCreators.second, Supports(sourceRef));

    return (foundCreators.second != foundCreatorIt) ? &*foundCreatorIt : 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry& SourceRegistry::GetInstance ()
    {
    static SourceRegistry SINGLETON;
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry::SourceRegistry ()
    :   m_implP(new SourceRegistryImpl)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry::~SourceRegistry ()
    {

    }



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceCreator* SourceRegistry::FindAppropriateCreator (const SourceRef& sourceRef) const
    { 
    return m_implP->m_sourceReg.FindAppropriateCreator(sourceRef); 
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LocalFileSourceCreator* SourceRegistry::FindAppropriateCreator (const LocalFileSourceRef& sourceRef) const
    { 
    return m_implP->m_localFileReg.FindAppropriateCreator(sourceRef); 
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DGNElementSourceCreator* SourceRegistry::FindAppropriateCreator (const DGNElementSourceRef& sourceRef) const
    {
    return m_implP->m_dgnElementReg.FindAppropriateCreator(sourceRef);
    }


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry::V0ID SourceRegistry::Register (const V0Creator& creator)
    {
    return m_implP->m_sourceReg.Register(SourceCreator(creator));
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceRegistry::Unregister (V0ID creatorID)
    {
    m_implP->m_sourceReg.Unregister(creatorID);
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry::V0LocalFileID SourceRegistry::Register (const V0LocalFileCreator& creator)
    {
    return m_implP->m_localFileReg.Register(LocalFileSourceCreator(creator)); 
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry::V0LocalFileID SourceRegistry::Register (const V0LocalFileCreator&   creator,
                                                        int                         priority)
    {
    assert(!"Implement correct behavior for priority");
    return m_implP->m_localFileReg.Register(LocalFileSourceCreator(creator)); 
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceRegistry::Unregister (V0LocalFileID creatorID)
    {
    m_implP->m_localFileReg.Unregister(creatorID); 
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRegistry::V0DGNElementID SourceRegistry::Register (const V0DGNElementCreator& creator)
    {
    return m_implP->m_dgnElementReg.Register(DGNElementSourceCreator(creator)); 
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceRegistry::Unregister (V0DGNElementID creatorID)
    {
    m_implP->m_dgnElementReg.Unregister(creatorID); 
    }



END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE




