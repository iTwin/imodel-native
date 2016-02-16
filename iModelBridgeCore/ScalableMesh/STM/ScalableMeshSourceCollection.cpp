/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCollection.cpp $
|    $RCSfile: ScalableMeshSourceCollection.cpp,v $
|   $Revision: 1.14 $
|       $Date: 2011/10/26 17:55:30 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include <ScalableMesh/GeoCoords/Definitions.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include "ScalableMeshSources.h"

#include "ScalableMeshEditListener.h"




#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/IScalableMeshSourceDescription.h>


#include "../Import/Sink.h"









USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
    USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceCollection::IteratorBase::Impl
    {
    typedef list<IDTMSourcePtr>::iterator iterator;
    typedef list<IDTMSourcePtr>::const_iterator const_iterator;

    typedef IteratorBase::Impl
                            IteratorImpl;

    explicit                Impl            ()   
        :   m_iter() 
        {
        }

    explicit                Impl            (iterator   iter)   
        :   m_iter(iter) 
        {
        }

    const_iterator          GetInternal     () const 
        { 
        return m_iter; 
        }
    iterator                GetInternal     () 
        { 
        return m_iter; 
        }

private:
    friend struct           IteratorBase;

    iterator                m_iter;
    };

IDTMSourceCollection::IteratorBase::IteratorBase (Impl* implP)
    :   m_implP(implP)
    {
    assert(0 != implP);
    }

IDTMSourceCollection::IteratorBase::IteratorBase ()
    :   m_implP(new Impl)
    {

    }

IDTMSourceCollection::IteratorBase::~IteratorBase ()
    {
    
    }

IDTMSourceCollection::IteratorBase::IteratorBase (const IteratorBase& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    
    }

IDTMSourceCollection::IteratorBase& IDTMSourceCollection::IteratorBase::operator= (const IteratorBase& rhs)
    {
    m_implP->m_iter = rhs.m_implP->m_iter;
    return *this;
    }

IDTMSourceCollection::IteratorBase::const_reference IDTMSourceCollection::IteratorBase::Dereference () const
    {
    return **m_implP->m_iter;
    }

IDTMSourceCollection::IteratorBase::reference IDTMSourceCollection::IteratorBase::Dereference ()
    {
    return **m_implP->m_iter;
    }

void IDTMSourceCollection::IteratorBase::Increment ()
    {
    ++m_implP->m_iter;
    }

void IDTMSourceCollection::IteratorBase::Decrement ()
    {
    --m_implP->m_iter;
    }

bool IDTMSourceCollection::IteratorBase::EqualTo (const IteratorBase& rhs) const
    {
    return m_implP->m_iter == rhs.m_implP->m_iter;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceCollection::Impl : public EditListener
    {
    typedef list<IDTMSourcePtr>     Container;
    Container                           m_sources;

    typedef vector<EditListener*>       ListenerList;
    ListenerList                        m_editListeners;

    typedef IteratorBase::Impl          IteratorImpl;

    explicit                            Impl                               ();

                                        ~Impl                              ();

    virtual void                        _NotifyOfPublicEdit                () override;
    virtual void                        _NotifyOfLastEditUpdate            (Time                        updatedLastEditTime) override;

    uint32_t                                GetCount                           () const;

    void                                Add                                (const IDTMSourcePtr&    sourcePtr);

    IteratorBase                        Release                            (const IteratorBase&         sourceIt,
                                                                            IDTMSourcePtr&          sourcePtr);

    void                                push_back                          (const IDTMSourcePtr&    dataSourcePtr);

    IteratorBase                        Begin                              ();   
    IteratorBase                        End                                ();

    IteratorBase                        Remove                             (const IteratorBase&         sourceIt);


    IteratorBase                        UnGroup                            (const IteratorBase&         groupSourceIt);

    IteratorBase                        MoveDown                           (const IteratorBase&         sourceIt);
    IteratorBase                        MoveToBottom                       (const IteratorBase&         sourceIt);
    IteratorBase                        MoveToPos                          (const IteratorBase&         sourceIt, 
                                                                            const IteratorBase&         destinationIt);
    IteratorBase                        MoveToTop                          (const IteratorBase&         sourceIt);
    IteratorBase                        MoveUp                             (const IteratorBase&         sourceIt);

    IteratorBase                        GetIterFor                         (const IDTMSource&       source);
    IteratorBase                        GetIterAt                          (uint32_t                        index);

    static Container::iterator          GetIter                            (const IteratorBase&         iter);

    static IteratorBase                 CreateIterFrom                     (Container::iterator         iter);


    static IteratorBase                 CreateIterator                     (IteratorImpl*               iterImpl);


    void                                OnPublicEdit                       ();

    struct                              UnregisterAsEditListener;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceCollection::Impl::UnregisterAsEditListener
    {
    const EditListener&             m_listener;
    explicit                        UnregisterAsEditListener           (const EditListener&         listener) : m_listener(listener) {}

    void                            operator ()                        (const IDTMSourcePtr&    sourcePtr) const
        {
        sourcePtr->UnregisterEditListener(m_listener);
        }
    };



IDTMSourceCollection::Impl::Impl ()
    {
    }

IDTMSourceCollection::Impl::~Impl ()
    {
    std::for_each(m_sources.begin(), m_sources.end(), UnregisterAsEditListener(*this));
    }


void IDTMSourceCollection::Impl::OnPublicEdit ()
    {
    struct Notify
        {
        void operator () (EditListener* listenerP) const
            {
            listenerP->NotifyOfPublicEdit();
            }
        };
    std::for_each(m_editListeners.begin(), m_editListeners.end(), Notify());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::Impl::_NotifyOfPublicEdit ()
    {
    OnPublicEdit();
    }

void IDTMSourceCollection::Impl::_NotifyOfLastEditUpdate (Time updatedLastEditTime)
    {
    struct Notify
        {
        Time m_updatedLastEditTime;
        explicit Notify (Time updatedLastEditTime) : m_updatedLastEditTime(updatedLastEditTime) {}

        void operator () (EditListener* listenerP) const
            {
            listenerP->NotifyOfLastEditUpdate(m_updatedLastEditTime);
            }
        };
    std::for_each(m_editListeners.begin(), m_editListeners.end(), Notify(updatedLastEditTime));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::GetIterFor (const IDTMSource& source)
    {
    struct FindFromKey : unary_function<const IDTMSourcePtr, bool>
        {
        const IDTMSource* m_keyP;
        explicit FindFromKey (const IDTMSource* keyP) : m_keyP(keyP) {}

        bool operator() (const IDTMSourcePtr& sourcePtr) const
            {
            return sourcePtr.get() == m_keyP;
            }
        };

    Container::iterator foundIt = std::find_if(m_sources.begin(), m_sources.end(), FindFromKey(&source));
    assert(foundIt != m_sources.end());
    
    return IteratorBase(new IteratorImpl(foundIt)); 
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::GetIterAt (uint32_t index)
    {
    assert(index < m_sources.size());

    Container::iterator sourceIter(m_sources.begin());    
    std::advance(sourceIter, index);

    return IteratorBase(new IteratorImpl(sourceIter)); 
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline IDTMSourceCollection::Impl::Container::iterator IDTMSourceCollection::Impl::GetIter (const IteratorBase& iter)
    {
    return iter.m_implP->GetInternal();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::CreateIterFrom (Impl::Container::iterator iter)
    {
    return IteratorBase(new IteratorImpl(iter));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::Impl::Add(const IDTMSourcePtr& sourcePtr)
    {        
    sourcePtr->RegisterEditListener(*this);
    m_sources.push_back(sourcePtr);    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::Impl::push_back (const IDTMSourcePtr& dataSourcePtr)
    {
    m_sources.push_back(dataSourcePtr);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::Begin ()
    {
    return CreateIterFrom(m_sources.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::End ()
    {
    return CreateIterFrom(m_sources.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t IDTMSourceCollection::Impl::GetCount() const
    {       
    return (uint32_t)m_sources.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::Remove(const IteratorBase& sourceIt)
    {  
    Container::iterator sourceIter(GetIter(sourceIt));
    if (m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    (**sourceIter).UnregisterEditListener(*this);

    return CreateIterFrom(m_sources.erase(sourceIter));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::Release(const IteratorBase&   sourceIt,
                                                                       IDTMSourcePtr&    sourcePtr)
    {  
    Container::iterator sourceIter(GetIter(sourceIt));
    if (m_sources.end() == sourceIter)
        {
        sourcePtr = 0;
        return CreateIterFrom(m_sources.end());
        }

    (**sourceIter).UnregisterEditListener(*this);

    sourcePtr = (*sourceIter);
    return CreateIterFrom(m_sources.erase(sourceIter));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::UnGroup(const IteratorBase& sourceIt)
    {    
    Container::iterator sourceIter(GetIter(sourceIt));
    if (m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    IDTMSourceCollection& groupSourceCollection = dynamic_cast<IDTMSourceGroup&>(**sourceIter).GetImpl().GetSources();

    const ptrdiff_t groupSize = groupSourceCollection.GetCount();

    for (const_iterator groupSourceIt = groupSourceCollection.Begin(), groupSourcesEnd = groupSourceCollection.End();
         groupSourceIt != groupSourcesEnd;)
        {
        IDTMSourcePtr releasedSourcePtr;
        groupSourceIt = groupSourceCollection.Release(groupSourceIt, releasedSourcePtr);

        (**m_sources.insert(sourceIter, 
                            releasedSourcePtr)).RegisterEditListener(*this);
        }

    Container::iterator groupNewEnd(m_sources.erase(sourceIter));
    
    std::advance(groupNewEnd, -groupSize);
    return CreateIterFrom(groupNewEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::MoveDown(const IteratorBase& sourceIt)
    {    
    Container::iterator sourceIter(GetIter(sourceIt));            
    if (m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    Container::iterator nextSourceIter(sourceIter);
    ++nextSourceIter;

    if (m_sources.end() == nextSourceIter)
        return sourceIt;

    std::swap(*sourceIter, *nextSourceIter);


    return CreateIterFrom(nextSourceIter);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::MoveToBottom(const IteratorBase& sourceIt)
    {
    Container::iterator sourceIter(GetIter(sourceIt));    
    if (m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    m_sources.push_back(*sourceIter);
    m_sources.erase(sourceIter);
          
    Container::iterator newPos(m_sources.end());
    --newPos;
    return CreateIterFrom(newPos);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::MoveToPos(const IteratorBase& sourceIt, const IteratorBase& destinationIt)
    {        
    Container::iterator destinationIter(GetIter(destinationIt));
    Container::iterator sourceIter(GetIter(sourceIt));

    if (m_sources.end() == destinationIter || m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    Container::iterator insertedPos(m_sources.insert(destinationIter, *sourceIter));
    m_sources.erase(sourceIter);

    return CreateIterFrom(insertedPos);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::MoveToTop(const IteratorBase& sourceIt)
    {     
    Container::iterator sourceIter(GetIter(sourceIt));      
    if (m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    m_sources.push_front(*sourceIter);
    m_sources.erase(sourceIter);
          
    return CreateIterFrom(m_sources.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::Impl::MoveUp(const IteratorBase& sourceIt)
    {    
    Container::iterator sourceIter(GetIter(sourceIt));                   
    if (m_sources.end() == sourceIter)
        return CreateIterFrom(m_sources.end());

    if (m_sources.begin() == sourceIter)
        return sourceIt;

    Container::iterator previousSourceIter(sourceIter);
    --previousSourceIter;

    std::swap(*previousSourceIter, *sourceIter);


    return CreateIterFrom(previousSourceIter);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::~IDTMSourceCollection  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IDTMSourceCollection ()
    :   m_implP(new Impl)
    {
    
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::RegisterEditListener (EditListener& listener)
    {
    m_implP->m_editListeners.push_back(&listener);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::UnregisterEditListener (const EditListener& listener)
    {

    Impl::ListenerList::iterator newEnd = remove(m_implP->m_editListeners.begin(), m_implP->m_editListeners.end(), &listener);
    assert(m_implP->m_editListeners.empty() || newEnd != m_implP->m_editListeners.end());
    m_implP->m_editListeners.erase(newEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::_OnPublicEdit ()
    {
    m_implP->OnPublicEdit();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IDTMSourceCollection (const IDTMSourceCollection& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection& IDTMSourceCollection::operator= (const IDTMSourceCollection& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_Begin () const
    {
    return m_implP->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_End () const
    {
    return m_implP->End();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_BeginEdit ()
    {
    return m_implP->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_EndEdit ()
    {
    return m_implP->End();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_BeginEditInternal ()
    {
    return m_implP->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_EndEditInternal ()
    {
    return m_implP->End();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_Release  (const IteratorBase& sourceIt,
                                                                    IDTMSourcePtr&  sourcePtr)
    {
    _OnPublicEdit();
    return m_implP->Release(sourceIt, sourcePtr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_GetIterFor (const IDTMSource& source) const
    {
    return m_implP->GetIterFor(source);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_EditIterFor (const IDTMSource& source)
    {
    return m_implP->GetIterFor(source);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_GetIterAt (uint32_t index) const
    {
    return m_implP->GetIterAt(index);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_EditIterAt (uint32_t index)
    {
    return m_implP->GetIterAt(index);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_Remove (const IteratorBase& sourceIt)
    {
    _OnPublicEdit();
    return m_implP->Remove(sourceIt);
    }

BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::UpToDateState IDTMSourceCollection::_GetUpToDateState(const IteratorBase& sourceIt)
    {
    list<IDTMSourcePtr>::iterator sourceIter(sourceIt.m_implP->GetInternal());
    SourceImportConfig& conf = (**sourceIter).EditConfig();
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::ScalableMeshData scalableMeshData = conf.GetReplacementSMData();
    return scalableMeshData.GetUpToDateState();
    }

IDTMSourceCollection::IteratorBase IDTMSourceCollection::_SetRemoveState(const IteratorBase& sourceIt)
    {
    _OnPublicEdit();
    list<IDTMSourcePtr>::iterator sourceIter(sourceIt.m_implP->GetInternal());
    SourceImportConfig& conf = (**sourceIter).EditConfig();
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::ScalableMeshData scalableMeshData = conf.GetReplacementSMData();
    scalableMeshData.SetUpToDateState(BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::UpToDateState::REMOVE);
    conf.SetReplacementSMData(scalableMeshData);
    return sourceIt;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_UnGroup (const IteratorBase& groupSourceIt)
    {
    _OnPublicEdit();
    return m_implP->UnGroup(groupSourceIt);
    }

IDTMSourceCollection::IteratorBase IDTMSourceCollection::_MoveDown (const IteratorBase& sourceIt)
    {
    _OnPublicEdit();
    return m_implP->MoveDown(sourceIt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_MoveToBottom (const IteratorBase& sourceIt)
    {
    _OnPublicEdit();
    return m_implP->MoveToBottom(sourceIt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_MoveToPos(const IteratorBase& sourceIt, 
                                                                    const IteratorBase& destinationIt)
    {
    _OnPublicEdit();
    return m_implP->MoveToPos(sourceIt, destinationIt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_MoveToTop (const IteratorBase& sourceIt)
    {
    _OnPublicEdit();
    return m_implP->MoveToTop(sourceIt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourceCollection::IteratorBase IDTMSourceCollection::_MoveUp (const IteratorBase& sourceIt)
    {
    _OnPublicEdit();
    return m_implP->MoveUp(sourceIt);
    }

void IDTMSourceCollection::_Clear()
    {
    m_implP->m_sources.clear();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t IDTMSourceCollection::GetCount () const
    {
    return m_implP->GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const IDTMSource& IDTMSourceCollection::GetAt (uint32_t index) const
    {
    const_iterator foundIt = GetIterAt(index);
    assert(End() != foundIt);

    return *foundIt;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSource& IDTMSourceCollection::EditAt (uint32_t index)
    {
    iterator foundIt = EditIterAt(index);
    assert(EndEdit() != foundIt);

    return *foundIt;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMSourceCollection::AddInternal (const IDTMSourcePtr& sourcePtr)
    {
    assert(0 != sourcePtr.get());
    assert(!sourcePtr->IsPartOfCollection());

    m_implP->Add(sourcePtr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IDTMSourceCollection::Add (const IDTMSourcePtr& sourcePtr)
    {
    assert(0 != sourcePtr.get());
    if (0 == sourcePtr.get())
        return BSIERROR;

    if (sourcePtr->IsPartOfCollection())
        {
        assert(!"Do not support sharing of sources in multiple collection!");
        return BSIERROR;
        }

    _OnPublicEdit();
    SourceImportConfig& conf = sourcePtr->EditConfig();
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::ScalableMeshData scalableMeshData = conf.GetReplacementSMData();
    scalableMeshData.SetUpToDateState(BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::UpToDateState::ADD);
    conf.SetReplacementSMData(scalableMeshData);
    m_implP->Add(sourcePtr);
    return BSISUCCESS;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
