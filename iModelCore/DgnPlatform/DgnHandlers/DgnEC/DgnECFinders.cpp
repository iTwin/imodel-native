/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/DgnECFinders.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include    <DgnPlatform/DgnHandlers/DgnECFinders.h>
#include    <DgnPlatform/DgnHandlers/DgnECModelLinkageProvider.h>
#include    "DgnECModelProvider.h"
#include    "DgnECFileProvider.h"

using namespace std;

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_LOGGING

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    adam.klatzkin                   08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSubstringPropertyValueFilterPtr    ECSubstringPropertyValueFilter::Create (WCharCP pv)
    {
    return new ECSubstringPropertyValueFilter (pv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSubstringPropertyValueFilter::ECSubstringPropertyValueFilter (WCharCP pv) 
    : 
    m_target(pv) 
    {
    BeStringUtilities::Wcslwr ((wchar_t*)m_target.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSubstringPropertyValueFilter::_Accept (WCharCP accessString, ECN::ECValueCR v)
    {
    // because we now support installed types using IECXATypeHandler it is possible to get non-string values here
    // so we must now check and just return false if the value is not a string
    if (!v.IsString())
        return false;

    WCharCP target = m_target.c_str();
    size_t  targetlen = m_target.size();
    for (WCharCP substr = v.GetString0(); *substr; ++substr)
        {
        if (*target != tolower(*substr))
            continue;
        if (0 == BeStringUtilities::Wcsnicmp (substr, target, targetlen))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Note: this works because our providers all provide either intrinsic or extrinsic
* instances. If this changes, or if we add new ECQueryInitializers, this method
* will need to be revisited.
* @bsimethod                                    Paul.Connelly                   04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool providerSupportsQuery (ECQueryCP query, UInt16 providerId)
    {
    if (NULL == query || query->IsFilteredByClassList() || query->IsSearchAllClasses())
        return true;

    switch (providerId)
        {
    case PROVIDERID_ECXAttributeData:
    case PROVIDERID_ECXData:
    case DgnECModelLinkageProvider::PROVIDERID:
        return query->IncludesExtrinsic();
    case PROVIDERID_Element:
    case PROVIDERID_ModelProvider:
    case DgnFileProvider::PROVIDERID_FileProvider:
        return query->IncludesIntrinsic();
    default:
        BeAssert (false && "Unrecognized provider");
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool                 InstanceIdIterationState::_MoveToNextInstance()
    {
    m_currentInstance = NULL;

    if (_IsAtEnd())
        return false;

    // the loop below will ensure a valid instance is available if true is returned.
    while (true)
        {
        m_current++;
        if (_IsAtEnd())
            return false;

        m_currentInstance = DgnECManager::RetrieveInstanceFromId(m_current->c_str(), m_pepDependentModel);
        if (m_currentInstance.IsValid() && providerSupportsQuery (m_query, m_currentInstance->GetDgnECInstanceEnabler().GetProviderId()))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Returns the DgnModel that is to be used when creating the persistent element path used for the InstanceId.
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    InstanceIdIterationState::_GetPepDependentModel() 
    {
    return m_pepDependentModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstanceP& InstanceIdIterationState::_GetCurrentInstance()
    {
    if (!m_currentInstance.IsValid())
        m_currentInstance = DgnECManager::RetrieveInstanceFromId(m_current->c_str(), m_pepDependentModel);
    return m_currentInstance.GetR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool                 InstanceIdIterationState::_IsAtEnd()
    {
    return (m_current == m_instanceIdVector->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool                 InstanceIdIterationState::_IsEqual(InstanceIterationState* rhs)
    {
    InstanceIdIterationState* rhsP = static_cast<InstanceIdIterationState*>(rhs);
    if (m_pepDependentModel != rhsP->m_pepDependentModel)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceIdIterationState::InstanceIdIterationState (bvector<WString>& instanceIdVector, DgnModelP pepDependentModel, ECQueryP query) : m_query (query)
    {
    m_instanceIdVector = &instanceIdVector;
    m_pepDependentModel = pepDependentModel;
    m_current = m_instanceIdVector->begin();

    // make sure we can get a valid instance...if not the _MoveToNextInstance will 
    // cause subsequent calls to _IsAtEnd() to return true
    m_currentInstance = DgnECManager::RetrieveInstanceFromId(m_current->c_str(), m_pepDependentModel);
    if (m_currentInstance.IsNull())
        _MoveToNextInstance ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<InstanceIterationState> InstanceIdIterationState::Create (bvector<WString>& instanceIdVector, DgnModelP pepDependentModel, ECQueryP query)
    {
    return new InstanceIdIterationState (instanceIdVector, pepDependentModel, query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceIdIterationData::InstanceIdIterationData (bvector<WString>& instanceIdVector, DgnModelP pepDependentModel, ECQueryCR query) : m_query (query)
    {
    m_pepDependentModel = pepDependentModel;

    // copy the instanceIds to the vector in this class
    FOR_EACH (WString id , instanceIdVector)
        m_instanceIdVector.push_back(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    InstanceIdIterationData::_GetInitialIterationState(RefCountedPtr<InstanceIterationState>& state)
    {
    state = InstanceIdIterationState::Create (m_instanceIdVector, m_pepDependentModel, &m_query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<InstanceIterationData> InstanceIdIterationData::Create (bvector<WString>& instanceIdVector, DgnModelP pepDependentModel, ECQueryCR query)
     {
     return new InstanceIdIterationData (instanceIdVector, pepDependentModel, query);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<InstanceCountIterable::IteratorState> InstanceCountIterable::IteratorState::Create (FindInstancesScopeCR scope, const SearchClassList & requestedClasses)
    { 
    return new InstanceCountIterable::IteratorState(scope, requestedClasses); 
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<InstanceIterationData> FindInstanceIterationData::Create (ElementCollectionBaseR elements, ECQueryCR query)
    {
    return new FindInstanceIterationData(elements, query); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstanceIterationData::FindInstanceIterationData (ElementCollectionBaseR elements, ECQueryCR query) 
    : m_elements (&elements), m_query (query)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<InstanceIterationState> FindInstanceIterationState::Create (ElementIteratorBaseR elementIterator, ECQueryP query) 
    { 
    return new FindInstanceIterationState(elementIterator, query); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    FindInstanceIterationData::_GetInitialIterationState(RefCountedPtr<InstanceIterationState>& state)
    {
    state = FindInstanceIterationState::Create (*m_elements->begin(), &m_query);
    if (state->IsAtEnd())
        return;
 
    state->MoveToNextInstance();     // initial state should be at first instance
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        InstanceIterationState::MoveToNextInstance()
    {
    return _MoveToNextInstance();
    }

/*---------------------------------------------------------------------------------**//**
* Returns the DgnModel that is to be used when creating the persistent element path used for the InstanceId.
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP            InstanceIterationState::GetPepDependentModel()
    {
    return _GetPepDependentModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstanceP& InstanceIterationState::GetCurrentInstance()
    {
    return _GetCurrentInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        InstanceIterationState::IsAtEnd()
    {
    return _IsAtEnd();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::DgnElementInstanceIterable()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::DgnElementInstanceIterable (InstanceIterationData& instanceIterationData)
    {
    m_instanceIterationDataPtr = &instanceIterationData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::const_iterator::const_iterator () : m_isEnd(true) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::const_iterator::const_iterator 
(
InstanceIterationData& instanceIterationData
) : m_isEnd (false)
    {
    instanceIterationData.GetInitialIterationState (m_state);
    if (m_state->IsAtEnd())
        m_isEnd = true;
    else
        m_isEnd = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::const_iterator  DgnElementInstanceIterable::begin () const
    {
    if (m_instanceIterationDataPtr.IsNull())
        return const_iterator();

    return DgnElementInstanceIterable::const_iterator(*m_instanceIterationDataPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::const_iterator  DgnElementInstanceIterable::end () const
    {
    return const_iterator();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void                InstanceIterationData::GetInitialIterationState(RefCountedPtr<InstanceIterationState>& state)
    {
    _GetInitialIterationState(state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterable::const_iterator& DgnElementInstanceIterable::const_iterator::operator++()
    {
    BeAssert (!m_isEnd && "Do not attempt to iterate beyond the end of the instances.");
    m_state->MoveToNextInstance();
    if (m_state->IsAtEnd())
        m_isEnd = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceIterationState::IsEqual(InstanceIterationState* rhs)
    {
    return _IsEqual(rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    DgnElementInstanceIterable::const_iterator::operator!= (const_iterator const& rhs) const
    {
    if (m_isEnd && rhs.m_isEnd)
        return false;
    if (m_state.IsNull() && !(rhs.m_state.IsNull()))
        return true;
    if (!(m_state.IsNull()) && rhs.m_state.IsNull())
        return true;

    return !m_state->IsEqual (rhs.m_state.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnElementInstanceIterable::const_iterator::operator== (const_iterator const& rhs) const
    {
    return !(rhs != *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstanceP& DgnElementInstanceIterable::const_iterator::operator*() const
    {
    static DgnElementECInstanceP s_null;
    BeAssert (!m_isEnd && "Do not attempt to get the value of the iterator when it is at its end.");
    if (m_isEnd)
        return s_null;

    DgnElementECInstanceP& instance = m_state->GetCurrentInstance();
    // m_dgnecInstances[m_instanceIndex];
    BeAssert (instance != NULL && "Enablers should never return NULL instances.");
    return instance;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstanceIterationState::FindInstanceIterationState
(
ElementIteratorBaseR    elementIterator, 
ECQueryP                query
) 
    : m_elementIterator(&elementIterator), m_query (query),
      m_finderIndex(0), m_finderFile(NULL), m_instanceIndex(0)
    {
    InitFinderListForCurrentElement();
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool FindInstanceIterationState::_IsEqual(InstanceIterationState* rhs)
    {
    FindInstanceIterationState* rhsP = static_cast<FindInstanceIterationState*>(rhs);

    return (!(m_finderIndex     != rhsP->m_finderIndex ||
              m_instanceIndex   != rhsP->m_instanceIndex ||
              m_elementIterator != rhsP->m_elementIterator));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void FindInstanceIterationState::BuildFinderList (DgnProjectR dgnFile)
    {
    m_finders.clear();

    // Ask each provider to construct a finder based on the query
    DgnECManagerR manager = DgnECManager::GetManager();
    
    DgnECPerFileCacheR perFileCache = manager.GetPerFileCache (dgnFile);

    FOR_EACH (IDgnECProviderP provider , manager.GetAllProviders())
        {
        if (provider->GetHostType() != DgnECHostType_Element)
            continue;

        void* providerPerFileCache = perFileCache.GetProviderPerFileCache (*provider);
        IDgnECInstanceFinderPtr finder = provider->CreateFinder (dgnFile, *m_query, providerPerFileCache);
        if (!finder.IsValid())
            continue;

        BeAssert (finder->GetHostType() == provider->GetHostType());//This is by design. So simply assert it in debug without adding the overhead.
        //DgnECManager::GetManager().GetLogger().debugv ("Adding %ls to DgnElementInstanceIterable for %S.", typeid(*finder).name(), m_query.ToString().c_str());

        IDgnElementECInstanceFinderPtr elemFinder(static_cast<IDgnElementECInstanceFinder*> (finder.get()));//This is the only place we cast. So not creating a seperate function for it.
        m_finders.push_back (elemFinder);

        }
    m_query->m_uniqueClassesByFile.clear(); // don't bother caching those search classes any longer... we'll rarely use them
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void FindInstanceIterationState::InitFinderListForCurrentElement()
    {
    // make sure there is an available element
    if (m_elementIterator->IsEnd())
        return;

    DgnProjectP    dgnFile = GetCurrentElement().GetDgnProject();

    if (NULL == dgnFile)
        { BeAssert (false); return; }

    // Remember which file is being used to get the finder list
    m_finderFile = dgnFile;

    // Try to locate a finder list in our cache
    IDgnElementECInstanceFinderMap::const_iterator it = m_finderMap.find(dgnFile);
    if (it != m_finderMap.end())
        {
        DgnECManager::GetManager().GetLogger().infov(L"Amazing... we finally made use of our finderMap");
        m_finders = it->second;
        return;
        }

    // Couldn't locate a finder list, build a new one
    BuildFinderList (*dgnFile);  

    // Cache it for later use
    //m_finderMap[dgnFile] = m_finders; //stop caching. We'll rarely see the same file again and the cache increases peak memory for cases with hundreds of file and complicates memory leak detection
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle   FindInstanceIterationState::GetCurrentElement()
    {
    return m_elementIterator->GetElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle   ElementIteratorBase::GetCurrentTopLevelElement() const
    {
    if (m_supportingElementIndex < m_supportingElements.size())
        return m_supportingElements[m_supportingElementIndex];

    BeAssert (m_processingPrimaryElem);

    return GetPrimaryElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ElementIteratorBase::GetHostPepStringCP () const
    {
    if (m_childElements.empty())
        return NULL;

    return m_childElements[m_childElementIndex].GetHostPepStringCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle   ElementIteratorBase::GetElement() const
    {
    ElementHandle   topLevelElem = GetCurrentTopLevelElement();

    if (m_childElements.empty())
        return topLevelElem;

    ElementRefP     elemRef = m_childElements[m_childElementIndex].GetElementRef();

    BeAssert (elemRef && "should never have NULL elemRef.");
    return ElementHandle (elemRef, topLevelElem.GetDgnModel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementIteratorBase::CollectSupportingElements (ElementHandleCR primaryElem)
    {
#if defined (WIP_NG)

    NamedGroup::ElementRefList namedGroupElemRefs;
    NamedGroup::GetGroupsContaining (namedGroupElemRefs, primaryElem.GetElementRef());

    FOR_EACH (ElementRefP namedGroupElemRef , namedGroupElemRefs)
        {
        ElemModelPair elemModelPair (namedGroupElemRef, namedGroupElemRef->GetDgnModelP()); // it is proper to use GetDgnModelP here so we don't process the same namedGroup more than once

        if (m_supportingElementsToIgnore.end() != m_supportingElementsToIgnore.find (elemModelPair))
            continue;

        // for namedGroups use the modelRef of the primary element so that the ECInstance's modelref will have the proper context. If not the InstanceId
        // will not contain a Pep that can be found from the current root model.
        ElementHandle supportingElement (namedGroupElemRef, primaryElem.GetDgnModel());

        m_supportingElements.push_back (supportingElement);
        m_supportingElementsToIgnore.insert (elemModelPair);

        // recursively find named groups containing this named group (to address defect D-71369)
        CollectSupportingElements (supportingElement);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementIteratorBase::PrepareForNewPrimaryElement()
    {
    if (IsEnd())
        return;

    ElementHandle   primaryElement = GetPrimaryElement();

#ifdef DEBUG_ITERATOR
    ElementId elementId = primaryElement.GetElementRef()->GetElementId();
    int       elementType = primaryElement.GetElementRef()->GetElementType();
    WCharCP   modelName = primaryElement.GetDgnModelP()->GetModelNameCP();

    wprintf (L"PrepareForNewPrimaryElement: ElementId=%llu type=%d model=%ls\n", elementId, elementType, modelName);
#endif

    // Build list of supporting elements for the current primary element
    CollectSupportingElements (primaryElement);
    m_supportingElementIndex = 0;

    // Any non-graphic element is potentially a supporting element.  Put it into the
    // ignore list so we don't process it again later.
    if ( ! primaryElement.GetElementRef()->IsGraphics())
        {
        ElemModelPair elemModelPair (primaryElement.GetElementRef(), primaryElement.GetDgnModelP()); // it is proper to use GetDgnModelP here so we don't process the same namedGroup more than once

        m_supportingElementsToIgnore.insert (elemModelPair);
        }

    // If there are any supporting elements do them first
    m_processingPrimaryElem = (0 == m_supportingElements.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementIteratorBase::CollectChildElementsFromSharedCellInstance (ElementHandleCR primaryElem)
    {
    Handler&  elmHandler = primaryElem.GetHandler ();

    ISharedCellQuery* sharedCellQuery = dynamic_cast <ISharedCellQuery*> (&elmHandler);
    if (NULL == sharedCellQuery)
        return false;

    ElementRefP definitionElemRef = sharedCellQuery->GetDefinition (primaryElem, *primaryElem.GetDgnProject());
    if (NULL == definitionElemRef)
        return false;

#ifndef NDEBUG
    if (DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_TRACE))
        {
        WChar cellName[256];
        sharedCellQuery->ExtractName (cellName, _countof (cellName), primaryElem);
        DgnECManager::GetManager().GetLogger().debugv (L"Adding ShareCell Definition and its children for cell '%ls'.",  cellName); 
        }
#endif

    if (0 == m_sharedCellInstancePep.size())
        {
        ElementHandle primaryElement = GetPrimaryElement();
        PersistentElementPath path (primaryElement.GetDgnModel(), primaryElement.GetElementRef());
        m_sharedCellInstancePep = path.ToWString();
        }

    m_childElements.push_back (ChildElement (definitionElemRef, m_sharedCellInstancePep.c_str()));

    // process child elements of definition
    ElementHandle sharedCellDefinitionElem (definitionElemRef);
    CollectChildElements (sharedCellDefinitionElem);

    m_sharedCellInstancePep.clear();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementIteratorBase::CollectChildElements (ElementHandleCR parentElem) 
    {
    if (!m_includeChildElements)
        return;

    if (CollectChildElementsFromSharedCellInstance (parentElem))
        return;

    for (ChildElemIter iter (parentElem, ExposeChildrenReason::Edit); iter.IsValid (); iter = iter.ToNext ())
        {
        m_childElements.push_back (ChildElement (iter.GetElementRef(), m_sharedCellInstancePep.size() > 0 ? m_sharedCellInstancePep.c_str() : NULL));
        CollectChildElements (iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementIteratorBase::ToNext()
    {
    /*--------------------------------------------------------------------------
      Definitions:
        Primary    - The element we got from m_elementIterator.  It might have
                     supporting elements or public children, but its never a 
                     component element.
        Supporting - An element that we need to search whenever we search its
                     primary element.  Ex. A namedgroup supports its members.
                     It might have public children, but its never a component
                     element.
        TopLevel   - Either a Primary or Supporting element.
        Current    - The element we will pass to finder->FindElementInstances.  Might
                     be a TopLevel or a component element.

      The traversal order for elements looks like this:

      * for each PrimaryElement
      *   {
      *   for each SupportingElement
      *     {
      *     process the SupportingElement and its children
      *     }
      *
      *   process the PrimaryElement and its children
      *   }

    --------------------------------------------------------------------------*/

    // If we have a top level element, use it to build a child list if we haven't yet
    if (m_includeChildElements && 0 == m_childElements.size())
        {
        CollectChildElements (GetCurrentTopLevelElement());

        // If we found any children, the first one will be current.
        if (0 < m_childElements.size())
            {
            m_childElementIndex = 0;
            return;
            }
        }

    // Advance the child index.  If there is another child it will be current.
    ++m_childElementIndex;
    if (m_childElementIndex < m_childElements.size())
        return;

    //--------------------------------------------------------------------------
    // We are finished with the current top level element.
    //--------------------------------------------------------------------------
    
    // Clear out the child list
    m_childElements.clear();
    m_childElementIndex = 0;

    // Advance the supporting element index.  If there is another supporting element it will be current.
    ++m_supportingElementIndex;
    if (m_supportingElementIndex < m_supportingElements.size())
        return;

    // If we haven't processed the current primary element yet, now is the time
    if ( ! m_processingPrimaryElem)
        {
        m_processingPrimaryElem = true;
        return;
        }

    //--------------------------------------------------------------------------
    // We are finished with the current primary element.
    //--------------------------------------------------------------------------

    // Clear out the supporting element list
    m_supportingElements.clear();
    m_supportingElementIndex = 0;

    // Advance to the next primary element
    ToNextPrimaryElement();

    // Setup the supporting element list, etc.
    PrepareForNewPrimaryElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool FindInstanceIterationState::MoveToNextFinderAndOrElement()
    {
    // make sure we are not already at the end
    if (m_elementIterator->IsEnd())
        {
        // that was the last element, we are done with the entire iteration
        return false;
        }

    ++m_finderIndex;
    if (m_finderIndex < m_finders.size())
        return true;
        
    // We must be completely finished with the current element
    // Move on to the next element and reset to the first finder.
    m_finderIndex = 0;

    while (true)
        {
        m_elementIterator->ToNext();
        if (m_elementIterator->IsEnd())
            {
            // that was the last element, we are done with the entire iteration
            return false;
            }

        // Get a new finder list only if the curent one is not compatible with the new element
        if (GetCurrentElement().GetDgnProject() != m_finderFile)
            InitFinderListForCurrentElement();
 
        // If we have finders we might find instances, so jump out to examine this element
        if (0 < m_finders.size())
            return true;

        // The finder list is empty so we won't find any instances, go to the next element        
        }

    BeAssert (false && "This code should be unreachable");
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool FindInstanceIterationState::_IsAtEnd()
    {
    return m_elementIterator->IsEnd();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool FindInstanceIterationState::_MoveToNextInstance()
    {
    m_currentInstance = NULL;

    BeAssert (!m_elementIterator->IsEnd() && "Do not attempt to iterate beyond the end of the instances.");
    if (m_elementIterator->IsEnd())
        return false;
        
    if (m_dgnecInstances.size() > 0)
        {
        // We are in the middle of iterating through the results from some finder, just move to the next one
        ++m_instanceIndex;
        
        if (m_instanceIndex < m_dgnecInstances.size())
            {    
            BeAssert (m_dgnecInstances[m_instanceIndex].IsValid() && "It is a violation of the IFindInstances contract to return NULL instances.");
            return true;
            }
        else
            {
            m_dgnecInstances.clear(); // We are finished with the current crop of instances
            if (!MoveToNextFinderAndOrElement())
                return false; // we have searched everything. The end is here.
                
            // fall through to below, where we will see if our new enabler/element can find some instances.
            }
        }

    // At this point m_dgnecInstances is empty and m_elementIterator and m_finders are valid
    // We are ready to ask the current finder if it finds any Instances on the element.
    BeAssert (m_dgnecInstances.size() == 0);

    while (true)
        {
        StatusInt status = ERROR;

        if (0 < m_finders.size())
            {
            IDgnElementECInstanceFinderPtr finder = m_finders[m_finderIndex];
            BeAssert (finder.IsValid() && "We should never have a NULL finder");

            ElementHandle eh = GetCurrentElement();
            status = finder->FindElementInstances (m_dgnecInstances, eh, _GetPepDependentModel(), m_elementIterator->GetHostPepStringCP ());

            if (DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
                {
                if (SUCCESS == status && m_dgnecInstances.size() > 0)
                    DgnECManager::GetManager().GetLogger().debugv (L"Found %d instances on Element ID=%d (type-%d)", (int)m_dgnecInstances.size(), eh.GetElementId(), eh.GetElementType ()); 
                }
            }

        if (SUCCESS != status || m_dgnecInstances.size() == 0)
            {
            if (!MoveToNextFinderAndOrElement())
                return false; // we have searched everything. The end is nigh.
            
            continue; // Try again with the next enabler/element
            }
        

        // The enabler has found something, set the m_instanceIndex to the first one
        BeAssert (m_dgnecInstances.size() > 0);
        m_instanceIndex = 0;   
        BeAssert (m_dgnecInstances[m_instanceIndex].IsValid() && "It is a violation of the IFindInstances contract to return NULL instances.");
        return true; // we have found the next IECInstance!
        }
        
    BeAssert (false && "This code should be unreachable");
    }

/*---------------------------------------------------------------------------------**//**
* Returns the DgnModel that is to be used when creating the persistent element path used for the InstanceId.
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    FindInstanceIterationState::_GetPepDependentModel() 
    {
    return GetCurrentElement().GetDgnModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstanceP& FindInstanceIterationState::_GetCurrentInstance()
    {
    if (NULL == m_currentInstance)
        {
        if (!m_elementIterator->IsEnd())
            {
            m_currentInstance = m_dgnecInstances[m_instanceIndex].get();
            BeAssert (NULL != m_currentInstance && "Enablers should never return NULL instances.");
            }
        }
    return m_currentInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::const_iterator InstanceCountIterable::begin() const
    {
    return InstanceCountIterable::const_iterator(m_scope, m_requestedClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::const_iterator InstanceCountIterable::end() const
    {
    return InstanceCountIterable::const_iterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::const_iterator::const_iterator
(
FindInstancesScopeCR scope,
const SearchClassList & requestedClasses
)
    {
    m_state = IteratorState::Create(scope, requestedClasses);
    if (m_state->m_instanceCountResults->end() == m_state->m_instanceCountResultIterator)
        m_isEnd = true;
    else
        m_isEnd = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::const_iterator& InstanceCountIterable::const_iterator::operator++()
    {
    m_state->m_instanceCountResultIterator++;
    if (m_state->m_instanceCountResults->end() == m_state->m_instanceCountResultIterator)
        m_isEnd = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCountIterable::const_iterator::operator!= (const_iterator const& rhs) const
    {
    if (m_isEnd && rhs.m_isEnd)
        return false;
    if (m_state.IsNull() && !(rhs.m_state.IsNull()))
        return true;
    if (!(m_state.IsNull()) && rhs.m_state.IsNull())
        return true;
    return (m_state->m_instanceCountResultIterator != rhs.m_state->m_instanceCountResultIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountEntry const& InstanceCountIterable::const_iterator::operator*() const
    {
    static InstanceCountEntry s_null;
    if (m_isEnd)
        return s_null;
    return *m_state->m_instanceCountResultIterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::IteratorState::IteratorState
(
FindInstancesScopeCR scope,
const SearchClassList & requestedClasses
)
    {
    ECQueryPtr nquery;

    if (requestedClasses.size() > 0)
        {
        nquery = ECQuery::CreateQuery (ECQUERY_INIT_Empty);

        FOR_EACH (SearchClass requestedClass , requestedClasses)
            {
            nquery->AddSearchClass(requestedClass);
            }
        }
    else
        {
        nquery = ECQuery::CreateQuery (ECQUERY_INIT_SearchAllClasses);
        }

    nquery->SetSelectProperties(false);

    DgnECInstanceIterable countIterable = DgnECManager::GetManager().FindInstances(scope,*nquery);
    
    bmap<ECSchemaCP, WString>   schemaNames;
    bmap<ECClassCP, int>         counts;
    for (DgnECInstanceIterable::const_iterator iter = countIterable.begin(); iter != countIterable.end(); ++iter)
        {
        ECClassCR cls = (*iter)->GetClass();
        counts[&cls]++;
        
        ECSchemaCR sc = cls.GetSchema();
        bmap<ECSchemaCP, WString>::iterator isc = schemaNames.find (&sc);
        if (isc == schemaNames.end())
            {
            wchar_t fullSchemaName[256];
            BeStringUtilities::Snwprintf(fullSchemaName, _countof(fullSchemaName), L"%ls.%02d.%02d", sc.GetName().c_str(), sc.GetVersionMajor(), sc.GetVersionMinor());
            schemaNames[&sc] = fullSchemaName;
            }
        }

    m_instanceCountResults = new InstanceCountByClass();
    for (bmap<ECClassCP, int>::const_iterator it = counts.begin(); it != counts.end(); ++it)
        {
        ECClassCP cls = it->first;
        ECN::SchemaNameClassNamePair schemaClassPair(schemaNames[&cls->GetSchema()], cls->GetName());
        (*m_instanceCountResults)[schemaClassPair] = counts[cls];
        }

    m_instanceCountResultIterator = m_instanceCountResults->begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::IteratorState::~IteratorState()
    {
    delete m_instanceCountResults;
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct  ModelElementCollectionAdapter : ElementCollectionBase
{
private:
    DgnModel::ElementsCollection    m_collection;

    ModelElementCollectionAdapter (DgnModel::ElementsCollection const& collection, bool includeChildren) 
        : m_collection (collection), ElementCollectionBase(includeChildren)
        { }

public:
    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct IteratorAdapter : public ElementIteratorBase
    {
    private:
        DgnModel::ElementRefIterator  m_iterator;

        IteratorAdapter (DgnModel::ElementRefIterator const& iterator, bool includeChildren)
            : m_iterator (iterator),ElementIteratorBase(includeChildren)  { }

    public:
        friend struct ModelElementCollectionAdapter;
        
        void                        ToNextPrimaryElement()            override { ++m_iterator; }
        //ElementIteratorBasePtr      operator++()        override { return new IteratorAdapter(++m_iterator, m_includeChildElements); }
        bool                        IsEnd()       const override { return m_iterator.HitEOF(); }
        ElementHandle               GetPrimaryElement () const override 
            {
            ElementRefP  elemRef = *m_iterator;
            if (!EXPECTED_CONDITION (elemRef && "should never have NULL elemRef."))
                return ElementHandle();

            return ElementHandle (elemRef, elemRef->GetDgnModelP());   // this means that Instances on models cannot be found via InstanceId unless this model is passed as the root
            }
    };

    ElementIteratorBasePtr   begin() const override { return new IteratorAdapter (m_collection.begin(), m_includeChildElements); }
    ElementIteratorBasePtr   end()   const override { return new IteratorAdapter (m_collection.end(), m_includeChildElements); }

    static ElementCollectionBasePtr Create (DgnModel::ElementsCollection const& collection, bool includeChildren) 
        {
        return new ModelElementCollectionAdapter (collection, includeChildren); 
        }
};

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct  ReachableElementCollectionAdapter : ElementCollectionBase
{
private:
    ReachableElementCollection      m_collection;

    ReachableElementCollectionAdapter (ReachableElementCollection const& collection, bool includeChildElements) 
        : m_collection (collection), ElementCollectionBase(includeChildElements) { }

public:
    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct IteratorAdapter : ElementIteratorBase
    {
    private:
        ReachableElementCollection::const_iterator  m_iterator;
        ReachableElementCollection::const_iterator  m_iteratorEnd;

        IteratorAdapter (ReachableElementCollection const& elements, bool includeChildElements)
            :
            m_iterator (elements.begin()),
            m_iteratorEnd (elements.end()),
            ElementIteratorBase(includeChildElements)
            {
            }

    public:
        friend struct ReachableElementCollectionAdapter;

        void                        ToNextPrimaryElement() override { ++m_iterator; }
        bool                        IsEnd()       const override { return  (m_iterator == m_iteratorEnd); }
        ElementHandle               GetPrimaryElement () const override { return *m_iterator; }
    };

    ElementIteratorBasePtr   begin() const override { return new IteratorAdapter (m_collection, m_includeChildElements); }
    ElementIteratorBasePtr   end()   const override { return new IteratorAdapter (m_collection, m_includeChildElements); }

    static ElementCollectionBasePtr Create (ReachableElementCollection const& collection, bool includeChildren)
        {
        return new ReachableElementCollectionAdapter (collection, includeChildren);
        }
};

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct  ElementAgendaCollectionAdapter : ElementCollectionBase
{
private:
    ElementAgenda       m_agenda;

    ElementAgendaCollectionAdapter (bool includeChildElements) 
        :ElementCollectionBase(includeChildElements){}

    ElementAgendaCollectionAdapter (ElementAgendaCR agenda, bool includeChildElements)
        :ElementCollectionBase(includeChildElements)
        { 
        size_t size = agenda.size();
        if (size > 0)
            {
            m_agenda.SetCapacity ((int)size);
            FOR_EACH (EditElementHandleCR eeh , agenda)
                {
                m_agenda.Insert (eeh.GetElementRef(), eeh.GetDgnModel());
                }
            }
        }

public:
    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct IteratorAdapter : ElementIteratorBase
    {
    private:
        ElementAgendaCR                m_agenda;
        ElementAgenda::const_iterator  m_iterator;
        bool                           m_initialized;
        IteratorAdapter (ElementAgenda::const_iterator const& iterator, ElementAgendaCR agenda, bool includeChildren)
            : m_iterator (iterator), m_agenda (agenda) , ElementIteratorBase(includeChildren)
            {}

    public:
        friend struct ElementAgendaCollectionAdapter;

        void                    ToNextPrimaryElement()  override { ++m_iterator; }
        bool                    IsEnd()       const override { return m_iterator == m_agenda.end(); }
        ElementHandle           GetPrimaryElement () const override { return *m_iterator; }
    };

    ElementIteratorBasePtr   begin() const override { return new IteratorAdapter (m_agenda.begin(), m_agenda, m_includeChildElements); }

    ElementIteratorBasePtr   end()   const override { return new IteratorAdapter (m_agenda.end(), m_agenda, m_includeChildElements); }


    static ElementCollectionBasePtr Create (Viewport& viewport, bool includeChildren)
        {
        ElementAgendaCollectionAdapter* adapter = new ElementAgendaCollectionAdapter(includeChildren);
        adapter->m_agenda.BuildFromViewport(viewport);
        return adapter;
        }

    static ElementCollectionBasePtr Create (ElementAgendaCR agenda, bool includeChildren)
        {
        return new ElementAgendaCollectionAdapter (agenda, includeChildren); 
        }
};

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct  SingleElementCollectionAdapter : ElementCollectionBase
{
private:
    ElementHandle       m_element;
    bool                m_includeChildren;

    SingleElementCollectionAdapter (ElementHandleCR element, bool includeChildren) 
        : m_element(element), 
          ElementCollectionBase (includeChildren) {}

public:
    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct IteratorAdapter : ElementIteratorBase
    {
    private:
        ElementHandleCP                m_element;
        IteratorAdapter (ElementHandleCP element, bool includeChildren) 
            : m_element (element), ElementIteratorBase(includeChildren)
            { }

    public:
        friend struct SingleElementCollectionAdapter;

        void                    ToNextPrimaryElement() override {  m_element = NULL; }
        bool                    IsEnd()       const override { return NULL == m_element; }
        ElementHandle           GetPrimaryElement () const override { return *m_element; }
    };

    ElementIteratorBasePtr   begin() const override { return new IteratorAdapter (&m_element, m_includeChildElements); }
    ElementIteratorBasePtr   end()   const override { return new IteratorAdapter (NULL, m_includeChildElements); }
    
    static ElementCollectionBasePtr Create (ElementHandleCR element, bool includeChildren)
        {
        return new SingleElementCollectionAdapter (element, includeChildren); 
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class FindModelElementInstancesScope : public FindElementInstancesScope
    {
    friend struct FindInstancesScope;
    DgnModelR    m_modelRef;
    ReachableDgnModelOptionsPtr m_options;

    FindModelElementInstancesScope (DgnModelR modelRef, bool ___includeAttachments_unused, ReachableDgnModelOptionsP options, bool includeChildElements)
            :m_modelRef (modelRef), m_options(options), FindElementInstancesScope(includeChildElements)
            {}

    public:
        

    virtual DgnModelP _GetDgnModel () const override {return &m_modelRef;}

    virtual ElementCollectionBasePtr    GetElementCollection    () const override
        {
        return ModelElementCollectionAdapter::Create(m_modelRef.GetElementsCollection(), m_includeChildElements);
        }
    
    virtual bool                     ScopeSpansSingleFile    () const {return true;}

    static FindModelElementInstancesScope* Create (DgnModelR modelRef, bool ___includeAttachments_unused, ReachableDgnModelOptionsP options, bool includeChildren) 
        {
        return new FindModelElementInstancesScope(modelRef, ___includeAttachments_unused, options, includeChildren);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class ViewPortFindInstancesScope : public FindElementInstancesScope
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    friend struct FindInstancesScope;
    ViewportR       m_viewport;
    
    public:
        ViewPortFindInstancesScope (ViewportR viewPort, bool includeChildElements)
            :m_viewport (viewPort), FindElementInstancesScope(includeChildElements)
            {}
        virtual bool    AllowProcessingByInstanceId (ECQueryCR query) const override {return false;}
        virtual DgnModelP _GetDgnModel () const override {return m_viewport.GetRootModel();}
        
        virtual ElementCollectionBasePtr    GetElementCollection    () const override
            {
            return ElementAgendaCollectionAdapter::Create (m_viewport, m_includeChildElements);
            }
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/09
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstancesScopePtr   FindInstancesScope::CreateScope (ViewportR viewport, bool includeChildren)
    {
    return FindElementInstancesScope::CreateScope (viewport, includeChildren);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
FindElementInstancesScopePtr    FindElementInstancesScope::CreateScope (ViewportR viewport, bool includeChildren)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    return new ViewPortFindInstancesScope (viewport, includeChildren);
#endif
    return  0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class ElementAgendaFindInstancesScope: public FindElementInstancesScope
    {
    friend struct FindInstancesScope;
    ElementAgendaCR     m_agenda;
    DgnModelR        m_modelRef;
    virtual bool _Empty () const override {return m_agenda.empty();}
    public:
    ElementAgendaFindInstancesScope (DgnModelR modelRef, ElementAgendaCR agenda, bool includeChildElements)
        :m_agenda(agenda), m_modelRef(modelRef), FindElementInstancesScope(includeChildElements)
        {
        }

    virtual DgnModelP                _GetDgnModel ()          const override {return &m_modelRef;}
    virtual ElementCollectionBasePtr    GetElementCollection () const override
            {
            return ElementAgendaCollectionAdapter::Create (m_agenda, m_includeChildElements);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Abeesh.Basheer  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstancesScopePtr   FindInstancesScope::CreateScope (DgnModelR modelRef, ElementAgendaCR agenda, bool includeChildElements)
    {
    return FindElementInstancesScope::CreateScope (modelRef, agenda, includeChildElements);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
FindElementInstancesScopePtr    FindElementInstancesScope::CreateScope (DgnModelR modelRef, ElementAgendaCR agenda, bool includeChildElements)
    {
    return new ElementAgendaFindInstancesScope (modelRef, agenda, includeChildElements);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
class SingleElementFindInstancesScope: public FindElementInstancesScope
    {
    ElementHandle   m_element;
    bool            m_includeChildren;
    virtual bool _Empty () const override {return false;}
    public:
    SingleElementFindInstancesScope (ElementHandleCR eh, bool includeChildren) : m_element(eh), FindElementInstancesScope(includeChildren) {}

    virtual DgnModelP                _GetDgnModel () const override {return m_element.GetDgnModel();}
    virtual ElementCollectionBasePtr    GetElementCollection () const override { return SingleElementCollectionAdapter::Create (m_element, m_includeChildElements); }
    virtual bool                     ScopeSpansSingleFile    () const {return true;}
    };

        static FindInstancesScopePtr    CreateScope (DgnModelR modelRef, bool ___includeAttachments_unused);
        static FindInstancesScopePtr    CreateScope (ElementHandleCR elemHandle);
        static FindInstancesScopePtr    CreateElementScope (ElementHandleCR elemHandle, bool includeChildElements);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP                    FindInstancesScope::GetDgnModel () const
    {
    return _GetDgnModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstancesScopePtr   FindInstancesScope::CreateScope (ElementHandleCR eh, bool includeChildren)
    {
    return FindElementInstancesScope::CreateScope (eh, includeChildren);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
FindElementInstancesScopePtr   FindElementInstancesScope::CreateScope (ElementHandleCR eh, bool includeChildren)
    {
    return new SingleElementFindInstancesScope (eh, includeChildren);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            FindElementInstancesScope::AllowProcessingByInstanceId (ECQueryCR query) const
    {
    return query.IsInstanceIdQuery ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECQueryPtr      ECQuery::CreateQuery (ECQueryInitializer initializer) // WIP_CEM: A CreateSearchAllClassesQuery method would be more discoverable
    {
    ECQueryP ecQuery = new ECQuery (initializer);

    return ecQuery;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECQueryPtr      ECQuery::CreateQuery (WCharCP schemaName, WCharCP className, bool isPolymorphic)
    {
    ECQueryP ecQuery = new ECQuery();

    if (NULL != schemaName && NULL != className)
        ecQuery->AddSearchClass (schemaName, className, isPolymorphic);

    return ecQuery;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECQueryPtr      ECQuery::CreateQuery (ECClassCR ecClass, bool isPolymorphic)
    {
    WCharCP   schemaName = ecClass.GetSchema().GetName().c_str();
    WCharCP   className  = ecClass.GetName().c_str();

    return CreateQuery (schemaName, className, isPolymorphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECQuery::ToString()
    {
    WString s = L"{ECQuery: ";

    if (!IsFilteredByClassList())
        {
        if (IsSearchAllClasses())
            s += L"<FIND_ALL> }";
        else if (IncludesIntrinsic())
            s += L"<FIND_ALL_INTRINSIC> }";
        else if (IncludesExtrinsic())
            s += L"<FIND_ALL_EXTRINSIC> }";
        else
            {
            BeAssert (false);
            s += L"<UNRECOGNIXED_QUERY_TYPE> }";
            }

        return s;
        }

    bool firstTime = true;
    FOR_EACH (SearchClass searchClass , m_searchClassNameList)
        {
        if (!firstTime)
            s += L", ";
        else
            firstTime = false;
            
        s += searchClass.m_class.m_schemaName + L":" + searchClass.m_class.m_className;
        }
    s += L"}";
    
    return s;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool	        ECQuery::IsInstanceIdQuery () const
    {
    if (!m_searchClassNameList.empty())
        false;

    if (!m_uniqueClassesByFile.empty())
        false;

    if (!m_valueFilter.IsNull())
        false;

    WhereCriterionPtr whereCriterionPtr = GetWhereCriterionPtr ();
    if (whereCriterionPtr.IsNull())
        return false;

    bvector<WString> instanceIdVector;
    return whereCriterionPtr->ContainsOnlyInstanceIdsToQuery ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            addClassToSet (bset<ECClassCP>& uniqueClasses, ECClassCR ecClass, bool isPolymorphic)
    {
    uniqueClasses.insert(&ecClass); // no-op if it already existed in the set

    if ( ! isPolymorphic)
        return;

    FOR_EACH (ECClassCP derivedClass, ecClass.GetDerivedClasses ())
        addClassToSet (uniqueClasses, *derivedClass, isPolymorphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
T_ECClassCPSet const* ECQuery::LookupSearchClasses (DgnProjectR dgnFile) const
    {
    if (!IsFilteredByClassList())
        return NULL;

    ClassesByFile::const_iterator it = m_uniqueClassesByFile.find (&dgnFile);
    if (it != m_uniqueClassesByFile.end())
        return &it->second;

    T_ECClassCPSet uniqueClasses;

    DgnECManagerR dgnECManager = DgnECManager::GetManager();
    FOR_EACH (SearchClass searchClass , GetSearchClassList())
        {
        // We get the enabler (even though we only use the ECClass, here) to ensure that the enabler has been lazy-loaded
        WStringCR   schemaName = searchClass.m_class.m_schemaName;
        WStringCR   className  = searchClass.m_class.m_className;

        DgnECInstanceEnablerP enabler = dgnECManager.ObtainInstanceEnablerByName (schemaName.c_str(), className.c_str(), dgnFile);
        if (NULL == enabler)
            {
            m_anyMissingClasses = true;
            continue;
            }

        addClassToSet (uniqueClasses, enabler->GetClass(), searchClass.m_isPolymorphic);
        }
    
    m_uniqueClassesByFile[&dgnFile] = uniqueClasses;

    size_t nBoundClasses = uniqueClasses.size();
    if (DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        {
        if (nBoundClasses == 0)
            {
            DgnECManager::GetManager().GetLogger().debugv (L"ECQuery [0x%x]: Lookup for search classes found none in '%ls'.",
                    this, dgnFile.GetFileName().c_str());        
            }
        else        
            {
            DgnECManager::GetManager().GetLogger().debugv (L"ECQuery [0x%x]: Lookup for %d search classes found %d in file '%ls' Classes will be cached in the ECQuery for future use.",
                    this, m_searchClassNameList.size(), nBoundClasses, dgnFile.GetFileName().c_str());  
            }
        }

    return &m_uniqueClassesByFile[&dgnFile];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IDgnElementECInstanceFinder::FindElementInstances (DgnElementECInstanceVector& results, ElementHandleCR eh, DgnModelP pepDependentModel, WCharCP hostPepString) const 
    {
    return _FindInstances(results, eh, pepDependentModel, hostPepString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::InstanceCountIterable (FindInstancesScopeCR scope, const SearchClassList & requestedClasses)
    :m_scope(scope), m_requestedClasses(requestedClasses)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCountIterable::const_iterator::const_iterator () 
    {
    m_isEnd = true;
    }

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompositeScope : public FindInstancesScope
    {
    bvector<FindInstancesScopePtr>      m_scopes;
    DgnECHostType                       m_hostType;
    DgnProjectR                            m_file;

    virtual DgnECHostType           _GetHostType () const override {return m_hostType;}
    virtual DgnProjectP                _GetFile () const override {return &m_file;}
    virtual DgnECInstanceIterable   _FindInstances (ECQueryCR query) const override;
    virtual bool                    _Empty() const override {return m_scopes.empty();}
    
    virtual BentleyStatus           _LocateSchemaXml (WStringR schemaXml, SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const override;
    virtual ECN::ECSchemaPtr         _LocateSchema (SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const override;
    virtual DgnModelP            _GetDgnModel () const override {return m_file.GetDictionaryModel();}

    CompositeScope (bvector<FindInstancesScopePtr> const& scopes, DgnECHostType hostType, DgnProjectR file)
        :m_scopes(scopes), m_hostType(hostType), m_file(file)
        {}

    static CompositeScope* Create (bvector<FindInstancesScopePtr> const& scopes, DgnECHostType hostType, DgnProjectR file)
        {
        return new CompositeScope (scopes, hostType, file);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CompositeScope::_LocateSchemaXml (WStringR schemaXml, SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const
    {
    for (bvector<FindInstancesScopePtr>::const_iterator iter = m_scopes.begin(); iter != m_scopes.end(); ++iter)
        {
        if (SUCCESS == (*iter)->_LocateSchemaXml(schemaXml, schemaInfo, matchType))
            return SUCCESS;
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECSchemaPtr CompositeScope::_LocateSchema (SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const
    {
    FindElementInstancesScope* scope = NULL;
    for (bvector<FindInstancesScopePtr>::const_iterator iter = m_scopes.begin(); iter != m_scopes.end(); ++iter)
        {
        scope = dynamic_cast<FindElementInstancesScope*> ((*iter).get());
        if (NULL != scope)
            break;
        }

    return (NULL != scope) ? scope->LocateSchema(schemaInfo, matchType) : FindInstancesScope::_LocateSchema (schemaInfo, matchType);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            TempSchemaCache::TryGetCachedSchemaCP (ECN::ECSchemaCP& schema, SchemaKeyCR schemaKey)
    {
    schema = NULL;
    SchemaMap::const_iterator it = m_externalSchemas.find(schemaKey);
    if (it == m_externalSchemas.end())
        return false;
    
    schema = it->second.get();
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            TempSchemaCache::TryGetCachedSchemaCP (ECN::ECSchemaCP& schema, ElementId elementId)
    {
    schema = NULL;
    SchemaMapByElementId::const_iterator it = m_storedSchemas.find(elementId);
    if (it == m_storedSchemas.end())
        return false;

    schema = it->second;
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                            TempSchemaCache::SetCachedSchemaCP (ElementId elementId, ECSchemaCP schema)
    {
#ifndef NDEBUG
    ECN::ECSchemaCP s;    
    BeAssert(!TryGetCachedSchemaCP(s, elementId));
#endif    
    m_storedSchemas[elementId] = schema;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                            TempSchemaCache::SetCachedSchemaCP (SchemaKeyCR schemaKey, ECSchemaCP schema)
    {
#ifndef NDEBUG
    ECN::ECSchemaCP s;    
    BeAssert(!TryGetCachedSchemaCP(s, schemaKey));
#endif
    m_keys.push_back(schemaKey.m_schemaName.c_str());
    m_externalSchemas[schemaKey] = (ECSchemaP)schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable::DgnECInstanceIterable (IInstanceCollectionAdapterPtr collection)
    :m_collectionPtr(collection)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable::const_iterator   DgnECInstanceIterable::begin() const
    {
    if (m_collectionPtr.IsNull())
        return DgnECInstanceIterable::const_iterator();

    return m_collectionPtr->begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnECInstanceIterable::empty() const
    {
    if (m_collectionPtr.IsNull())
        return true;

    return m_collectionPtr->begin() == m_collectionPtr->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnECInstanceIterable::IsNull() const
    {
    return m_collectionPtr.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable::const_iterator   DgnECInstanceIterable::end() const
    {
    if (m_collectionPtr.IsNull())
        return DgnECInstanceIterable::const_iterator();

    return m_collectionPtr->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECHostType   IDgnECInstanceFinder::GetHostType () const
    {
    return _GetHostType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECHostType   FindInstancesScope::GetHostType () const
    {
    return _GetHostType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable   IDgnECInstanceFinder::GetRelatedInstances (DgnECInstanceCR instance, QueryRelatedClassSpecifier const& relationshipClassSpecifier) const
    {
    return _GetRelatedInstances (instance, relationshipClassSpecifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnRelatedECInstanceFinder::AddSearchClasses (bvector<ECClassP> const& classList, bool isPolyMorphic)
    {
    for (bvector<ECN::ECClassP>::const_iterator iter = classList.begin(); iter != classList.end(); ++iter)
        {
        m_query->AddSearchClass ((*iter)->GetSchema().GetName().c_str(), (*iter)->GetName().c_str());
        if (!isPolyMorphic)
            continue;

        AddSearchClasses ((*iter)-> GetDerivedClasses(), isPolyMorphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRelatedECInstanceFinder::DgnRelatedECInstanceFinder(DgnECHostType hostType, ECRelationshipClassCR relationShip)
    :m_relationShip(relationShip), m_hostType(hostType), IDgnECInstanceFinder(NULL)
    {
    m_query = ECQuery::CreateQuery (ECQUERY_INIT_Empty);
    
    ECRelationshipConstraintR targetRelConstrain = relationShip.GetTarget();

    AddSearchClasses (targetRelConstrain.GetClasses (), targetRelConstrain.GetIsPolymorphic ());
    }

DgnECRelationshipIterable IDgnECRelationshipFinder::GetRelationshipInstances() const { return _GetRelationshipInstances(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WhereCriteriaPredicate::WhereCriteriaPredicate ()
    :m_criteria (NULL)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WhereCriteriaPredicate::WhereCriteriaPredicate (QueryRelatedClassSpecifier const& criteria)
    :m_criteria(&criteria)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            WhereCriteriaPredicate::operator () (DgnECInstanceCP const& instance) const
    {
    if (NULL == instance)
        return false;

    if (NULL == m_criteria)
        return true;

    ECN::ECClassCR classObj = instance->GetClass ();
    return m_criteria->Accept (classObj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceFilteredIterable::DgnECInstanceFilteredIterable (FindInstancesScopePtr scope, QueryRelatedClassSpecifier const& criteria, ECQueryCR query)
    :m_predicate (criteria), m_query(query), m_scope(scope), m_collection(DgnECManager::GetManager().FindInstances (*scope, query))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnRelatedECInstanceFinder::IsClassInstanceSpecified (ECN::ECClassCR classInstance, bvector<ECN::ECClassP> const& classList, bool isPolyMorphic)
    {
    bool hasInstanceClassSpecified = false;
    for (bvector<ECN::ECClassP>::const_iterator iter = classList.begin(); iter != classList.end(); ++iter)
        {
        if ((*iter)->Is(&classInstance))
            {
            hasInstanceClassSpecified = true;
            break;
            }
        
        if (!isPolyMorphic)
            continue;

        hasInstanceClassSpecified = IsClassInstanceSpecified (classInstance, (*iter)-> GetDerivedClasses(), isPolyMorphic);
        if (hasInstanceClassSpecified)
            break;
        }
    return hasInstanceClassSpecified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnRelatedECInstanceFinder::IsRelationShipSupportedForClass (ECN::ECClassCR classInstance, ECRelationshipClassCR relationShipClass)
    {
    ECRelationshipConstraintR sourceRelConstrain = relationShipClass.GetSource();
    return IsClassInstanceSpecified (classInstance, sourceRelConstrain.GetClasses (), sourceRelConstrain.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECHostType   DgnRelatedECInstanceFinder::_GetHostType () const
    {
    return m_hostType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRelatedECInstanceFinder* DgnRelatedECInstanceFinder::Create (DgnECHostType hostType, DgnECInstanceCR source, ECN::ECRelationshipClassCR relationShipClass)
    {
    if (!IsRelationShipSupportedForClass(source.GetClass(), relationShipClass))
        return NULL;

    return new DgnRelatedECInstanceFinder(hostType, relationShipClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable DgnRelatedECInstanceFinder::_GetRelatedInstances (DgnECInstanceCR instance, QueryRelatedClassSpecifier const& relationshipClassSpecifier) const
    {
    FindInstancesScopePtr scope = instance.GetRelatedInstanceScope ();
    BeAssert (scope.IsValid());

    return DgnECInstanceIterable (new InstanceCollectionAdapter<DgnECInstanceFilteredIterable> (*new DgnECInstanceFilteredIterable(scope, relationshipClassSpecifier, *m_query)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectP        FindInstancesScope::GetFile() const
    {
    return _GetFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectP FindElementInstancesScope::_GetFile() const
    {
    DgnModelP modelRef = GetDgnModel();
    BeAssert (NULL != modelRef);
    return modelRef->GetDgnProject();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            FindInstancesScope::HasFinder (ECQueryCR query) const
    {
    DgnECManagerR manager = DgnECManager::GetManager();
    
    DgnProjectP file = GetFile();
    if (NULL == file)
        return false;

    DgnECPerFileCacheP perFileCache = &manager.GetPerFileCache (*file);
    
    FOR_EACH (IDgnECProviderP provider , manager.GetAllProviders())
        {
        if (provider->GetHostType() != GetHostType())
            continue;
        
        void* providerPerFileCache = perFileCache->GetProviderPerFileCache (*provider);
        IDgnECInstanceFinderPtr finder = provider->CreateFinder (*file, query, providerPerFileCache);
        if (finder.IsNull() || finder->GetHostType() != GetHostType())
            continue;

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            FindInstancesScope::GetFinderList (IDgnECInstanceFinderList& finderList, ECQueryCR query) const
    {
    DgnECManagerR manager = DgnECManager::GetManager();
    
    DgnProjectP file = GetFile();
    if (NULL == file)
        return;

    DgnECPerFileCacheP perFileCache = &manager.GetPerFileCache (*file);
    
    FOR_EACH (IDgnECProviderP provider , manager.GetAllProviders())
        {
        if (provider->GetHostType() != GetHostType())
            continue;
        
        void* providerPerFileCache = perFileCache->GetProviderPerFileCache (*provider);
        IDgnECInstanceFinderPtr finder = provider->CreateFinder (*file, query, providerPerFileCache);
        if (finder.IsNull() || finder->GetHostType() != GetHostType())
            continue;

        finderList.push_back(finder);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable FindInstancesScope::FindInstances (ECQueryCR query) const
    {
    return _FindInstances(query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
class DgnElementInstanceIterableAdapter : public IInstanceCollectionAdapter
    {
    RefCountedPtr<InstanceIterationData> m_instanceIterationDataPtr;
    
    public:
        class DgnElementInstanceIterableAdapterIterator : public IInstanceCollectionIteratorAdapterImpl
            {
            RefCountedPtr<InstanceIterationState>   m_state;
            mutable DgnECInstanceP                  m_instance;
            public:
                DgnElementInstanceIterableAdapterIterator ()
                    :m_state(NULL), m_instance(NULL)
                    {}

                DgnElementInstanceIterableAdapterIterator (InstanceIterationData& data)
                    :m_instance(NULL), m_state(NULL)
                    {
                    data.GetInitialIterationState(m_state);
                    }
                
                virtual void MoveToNext() override
                    {
                    m_instance = NULL;
                    if (m_state.IsValid())
                        m_state->MoveToNextInstance();
                    }
                
                virtual bool                IsDifferent (IInstanceCollectionIteratorAdapterImpl const &rhsIter) const override;
            
                virtual DgnECInstanceP const& GetCurrent () const override;

                virtual bool                IsAtEnd () const
                    {
                    if (m_state.IsNull())
                        return true;

                    return m_state->IsAtEnd();
                    }
                
                static DgnElementInstanceIterableAdapterIterator* Create(InstanceIterationData* data) 
                    {
                    return NULL == data ? new DgnElementInstanceIterableAdapterIterator() : new DgnElementInstanceIterableAdapterIterator(*data);
                    }

            };
       

        DgnElementInstanceIterableAdapter (InstanceIterationData& instanceIterationData);
        IInstanceCollectionIteratorAdapter begin () const;
        IInstanceCollectionIteratorAdapter end () const 
            {
            static IInstanceCollectionIteratorAdapter s_end(*DgnElementInstanceIterableAdapterIterator::Create(NULL));
            return s_end;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceP const& DgnElementInstanceIterableAdapter::DgnElementInstanceIterableAdapterIterator::GetCurrent () const
    {
    if (m_state.IsNull() || NULL != m_instance)
        return m_instance;

    m_instance = m_state->GetCurrentInstance();
    return m_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnElementInstanceIterableAdapter::DgnElementInstanceIterableAdapterIterator:: IsDifferent (IInstanceCollectionIteratorAdapterImpl const &rhsIter) const
    {
    bool leftIsNull = m_state.IsNull();

    DgnElementInstanceIterableAdapterIterator const* rhs = static_cast<DgnElementInstanceIterableAdapterIterator const*> (&rhsIter);
    bool rightIsNull = NULL == rhs || rhs->m_state.IsNull();
        
    if (leftIsNull && rightIsNull)
        return false;

    if (!leftIsNull && !rightIsNull)
        return !m_state->IsEqual (rhs->m_state.get());

    //bool leftAtEnd =  (leftIsNull || m_state->IsAtEnd());
    //bool rightAtEnd =  (rightIsNull || rhs->m_state->IsAtEnd());
        
    return (leftIsNull || m_state->IsAtEnd()) != (rightIsNull || rhs->m_state->IsAtEnd());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementInstanceIterableAdapter::DgnElementInstanceIterableAdapter (InstanceIterationData& instanceIterationData)
    :m_instanceIterationDataPtr(&instanceIterationData)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IInstanceCollectionAdapter::IInstanceCollectionIteratorAdapter DgnElementInstanceIterableAdapter::begin() const
    {
    return (*DgnElementInstanceIterableAdapterIterator::Create(m_instanceIterationDataPtr.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable FindElementInstancesScope::_FindInstances (ECQueryCR query) const
    {
    RefCountedPtr<InstanceIterationData> instanceDataPtr = DgnECManager::GetManager().CreateInstanceIterationData (*this, query);
    if (instanceDataPtr.IsNull())
        return DgnECInstanceIterable(NULL);

    return DgnECInstanceIterable (new DgnElementInstanceIterableAdapter(*instanceDataPtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECInstanceFinder::IDgnECInstanceFinder (IECPropertyValueFilterPtr valueFilter)
    :m_propertyValueFilter(valueFilter)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnElementECInstanceFinder::IDgnElementECInstanceFinder (IECPropertyValueFilterPtr valueFilter)
    :IDgnECInstanceFinder (valueFilter)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IDgnECInstanceFinder::InstanceSupportsValueFilter (DgnECInstanceCR instance) const
    {
    if (m_propertyValueFilter.IsNull())
        return true;

    ECValuesCollectionPtr valuesCollection = ECValuesCollection::Create(instance);
    for (ECValuesCollection::const_iterator iter = valuesCollection->begin(); iter != valuesCollection->end(); ++iter)
        {
        if (!m_propertyValueFilter->_Accept ((*iter).GetValueAccessor().GetAccessString(), (*iter).GetValue()))
            continue;
                
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstancesScopePtr   FindInstancesScope::CreateScope (DgnModelR modelRef, bool ___includeAttachments_unused, ReachableDgnModelOptionsP options, bool includeChildElements, DgnECHostType hostType)
    {
    bvector<FindInstancesScopePtr> scopes;

    FindInstancesScopePtr tmpScope;
    if (hostType & DgnECHostType_Element)
        {
        tmpScope= FindModelElementInstancesScope::Create (modelRef, ___includeAttachments_unused, options, includeChildElements);
        scopes.push_back(tmpScope);
        }
    if (hostType & DgnECHostType_Model)
        {
        tmpScope = FindModelInstanceScope::CreateScope(modelRef, ___includeAttachments_unused, options);
        scopes.push_back(tmpScope);
        }

    if (1 == scopes.size())
        return scopes[0];

    if (scopes.empty())
        return tmpScope;

#ifdef DGNV10FORMAT_CHANGES_WIP
    return CompositeScope::Create (scopes, hostType, *modelRef.GetDgnProject());
#endif
    return  0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
FindElementInstancesScopePtr   FindElementInstancesScope::CreateScope (DgnModelR modelRef, 
            bool ___includeAttachments_unused, ReachableDgnModelOptionsP options, bool includeChildren)
    {
    return FindModelElementInstancesScope::Create (modelRef, ___includeAttachments_unused, options, includeChildren);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FindInstancesScopePtr    FindInstancesScope::CreateScope (DgnProjectR file, bool ___includeAttachments_unused,
        ReachableDgnModelOptionsP options, bool includeChildren, DgnECHostType hostType)
    {
    bvector<FindInstancesScopePtr> scopes;
    
#ifdef DGNV10FORMAT_CHANGES_WIP
    if (hostType & DgnECHostType_Element)
        {
        // set up separate scope to look at elements in dictionary model and every loaded model.
        DgnModelP nonModelCache = file.GetDictionaryModel();
        if (NULL != nonModelCache)
            {
            FindInstancesScopePtr nodeModelScope = FindModelElementInstancesScope::Create (*nonModelCache, false, NULL, includeChildren);
            scopes.push_back (nodeModelScope);
            }

        DgnFile::LoadedModelsCollection modelCollection = file.GetLoadedModelsCollection();
        for (DgnFile::LoadedModelsCollection::const_iterator iter = modelCollection.begin(); iter != modelCollection.end(); ++iter)
            {
            FindInstancesScopePtr modelScope = FindModelElementInstancesScope::Create (**iter, ___includeAttachments_unused, options, includeChildren);
            if (modelScope.IsNull())
                continue;
            scopes.push_back (modelScope);
            }
        }

    if (hostType & DgnECHostType_Model)
        scopes.push_back(FindModelInstanceScope::CreateScope(file, ___includeAttachments_unused, options));
#endif

    if (hostType & DgnECHostType_File)
        scopes.push_back(FindFileInstanceScope::CreateScope(file));

    if (scopes.empty())
        return NULL;

    if (1 == scopes.size())
        return scopes[0];

#ifdef DGNV10FORMAT_CHANGES_WIP
    return CompositeScope::Create (scopes, hostType, file);
#endif
    return  0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompositeScopeInstanceIterable
    {
    typedef bvector<FindInstancesScopePtr>   T_ScopeList;
    private:
        T_ScopeList                         m_primitiveScopes;
        ECQueryCR                           m_query;
    public:
        
        struct CompositeScopeInstanceIterator: public boost::iterator_facade <CompositeScopeInstanceIterator,DgnECInstanceIterable const, boost::forward_traversal_tag>
            {
            friend class boost::iterator_core_access;
        private:
            DgnECInstanceIterable                       m_state;
            CompositeScopeInstanceIterable const&       m_compositeInstanceIterableParent;
            T_ScopeList::const_iterator                 m_iter;
        public:
            CompositeScopeInstanceIterator (CompositeScopeInstanceIterable const& parent, bool begin)
                :m_state (NULL), m_compositeInstanceIterableParent (parent), m_iter(begin ? parent.m_primitiveScopes.begin() : parent.m_primitiveScopes.end())
                {
                if (!begin || m_iter == parent.m_primitiveScopes.end())
                    return;
                
                m_state = (**m_iter).FindInstances (m_compositeInstanceIterableParent.m_query);
                }

            void                increment  ()
                {
                //BeAssert (NULL != m_hostScope_FinderList_AdapterParent);
                ++m_iter;
                if (m_iter != m_compositeInstanceIterableParent.m_primitiveScopes.end())
                    m_state = (**m_iter).FindInstances (m_compositeInstanceIterableParent.m_query);
                }
            bool    equal (CompositeScopeInstanceIterator const & rhs) const
                {
                return m_iter == rhs.m_iter;
                }

             DgnECInstanceIterable const&   dereference () const
                {
                return m_state;
                }
            };

        typedef CompositeScopeInstanceIterator const_iterator;
        CompositeScopeInstanceIterable (T_ScopeList const& scopeList, ECQueryCR query)
            :m_primitiveScopes (scopeList), m_query(query)
            {
            }

        const_iterator begin() const {return CompositeScopeInstanceIterator (*this, true);}
        const_iterator end() const {return CompositeScopeInstanceIterator (*this, false);}

        typedef FlatteningCollection <CompositeScopeInstanceIterable>    FlatCollection;
    };

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 DgnECInstanceIterable CompositeScope::_FindInstances (ECQueryCR query) const
     {
     if (m_scopes.empty())
        {
        IInstanceCollectionAdapterPtr  nullCollection;
        return DgnECInstanceIterable (nullCollection);
        }
    
    CompositeScopeInstanceIterable::FlatCollection* flatCollection = new CompositeScopeInstanceIterable::FlatCollection (*new CompositeScopeInstanceIterable(m_scopes, query));
    return DgnECInstanceIterable(new InstanceCollectionAdapter<CompositeScopeInstanceIterable::FlatCollection> (*flatCollection));
     }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            FindInstancesScope::Empty () const
    {
    return _Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            FindElementInstancesScope::_Empty() const
    {
    ElementCollectionBasePtr collection = GetElementCollection();
    if (collection.IsNull())
        return true;

    return collection->begin()->IsEnd();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECRelationshipIterable::DgnECRelationshipIterable (IDgnECRelationshipInstanceCollectionPtr collection) : m_collection (collection)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECRelationshipIterable::const_iterator DgnECRelationshipIterable::begin() const
    {
    return m_collection.IsValid() ? m_collection->begin() : const_iterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECRelationshipIterable::const_iterator DgnECRelationshipIterable::end() const
    {
    return m_collection.IsValid() ? m_collection->end() : const_iterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FindInstancesScope::_LocateSchemaXml (WStringR schemaXml, SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const
    {
    return DgnECManager::GetManager().LocateSchemaXmlInDgnFile (schemaXml, schemaInfo, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FindInstancesScope::LocateSchemaXml (WStringR schemaXml, SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const
    {
    return _LocateSchemaXml (schemaXml, schemaInfo, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECSchemaPtr FindInstancesScope::LocateSchema (SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) const
    {
    return _LocateSchema(schemaInfo, matchType);
    }
