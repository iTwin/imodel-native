/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnCoreEvent.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
template <class ITYPE> class AbstractEventHandlerList
{
protected:
    class Entry
        {
        ITYPE*      m_handler;

     public:
        explicit Entry (ITYPE* handler) {m_handler = handler;}
        void     Clear() {m_handler=NULL; }
        ITYPE*   GetHandler() const {return m_handler;}
        };

    bool            m_locked;
    bvector<Entry>  m_entries;

    typedef typename bvector<Entry>::iterator          EntryIter;
    typedef typename bvector<Entry>::const_iterator    EntryCIter;

public:

AbstractEventHandlerList() {m_locked = false;}
Entry* GetFirstEntry (){ return m_entries.empty () ? 0 : &m_entries.front();}

bool HasHandler (ITYPE* handler)
    {
    for (EntryCIter curr = m_entries.begin(); curr!=m_entries.end(); ++curr)
        {
        if (handler == curr->GetHandler())
            return  true;
        }
    return  false;
    }

bool HasValidHandler ()
    {
    for (EntryCIter curr = m_entries.begin(); curr!=m_entries.end(); ++curr)
        {
        if (NULL != curr->GetHandler())
            return  true;
        }
    return  false;
    }

void AddHandler (ITYPE* handler)
    {
    if (m_locked)
        {
        BeAssert (0);
        return;
        }

    if (!HasHandler (handler))
        m_entries.push_back (Entry (handler));
    }

void DropHandler (ITYPE* handler)
    {
    for (EntryIter curr = m_entries.begin(); curr!=m_entries.end(); ++curr)
        {
        if (handler == curr->GetHandler())
            {
            curr->Clear();      // doesn't get removed until next traversal
            return;
            }
        }
    }

template <class CALLER> void CallAllHandlers (CALLER& caller)
    {
    if (m_entries.empty())
        return;


    m_locked = true;
    try
        {
        for (EntryIter curr = m_entries.begin(); curr!=m_entries.end(); )
            {
            if (NULL == curr->GetHandler())
                {
                curr = m_entries.erase(curr);
                }
            else
                {
                caller.CallHandler (*curr->GetHandler());
                ++curr;
                }
            }
        }
    catch (...)
        {
        }

    m_locked = false;
    }

template <class CALLER> void CallAllHandlers (CALLER const& caller)
    {
    if (m_entries.empty())
        return;

    m_locked = true;
    try
        {
        for (EntryIter curr = m_entries.begin(); curr!=m_entries.end(); )
            {
            if (NULL == curr->GetHandler())
                {
                curr = m_entries.erase(curr);
                }
            else
                {
                caller.CallHandler (*curr->GetHandler());
                ++curr;
                }
            }
        }
    catch (...)
        {
        }

    m_locked = false;
    }


template <class CALLER> bool CallAllHandlers (CALLER& caller, bool stopIfTrue)
    {
    if (m_entries.empty())
        return false;

    m_locked = true;

    bool     anyTrues = false;
    try
        {
        for (EntryIter curr = m_entries.begin(); curr<m_entries.end(); )
            {
            if (NULL == curr->GetHandler())
                {
                curr = m_entries.erase(curr);
                }
            else
                {
                anyTrues |= caller.CallHandler (*curr->GetHandler());
                ++curr;
                }
            }
        }
    catch (...)
        {
        }

    m_locked = false;
    return  anyTrues;
    }

template <class CALLER> bool CallAllHandlers (CALLER const& caller, bool stopIfTrue)
    {
    if (m_entries.empty())
        return false;

    m_locked = true;

    bool     anyTrues = false;
    try
        {
        for (EntryIter curr = m_entries.begin(); curr<m_entries.end(); )
            {
            if (NULL == curr->GetHandler())
                {
                curr = m_entries.erase(curr);
                }
            else
                {
                anyTrues |= caller.CallHandler (*curr->GetHandler());
                ++curr;
                }
            }
        }
    catch (...)
        {
        }

    m_locked = false;
    return  anyTrues;
    }

};

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
template <class ITYPE> class EventHandlerList : public AbstractEventHandlerList<ITYPE>
{
};

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
template <class ITYPE> class PrioritizedEventHandlerList: public EventHandlerList<ITYPE>
{
public:
    DEFINE_T_SUPER(EventHandlerList<ITYPE>);
    typedef typename T_Super::Entry Entry;

void            AddPrioritized (ITYPE* handlerToAdd)
    {
    if (this->m_locked)
        {
        BeAssert (0);
        return;
        }

    Entry*     currProv = this->GetFirstEntry();
    size_t     count = this->m_entries.size();
    size_t     index = 0;
    int        newPriority = handlerToAdd->GetHandlerPriority();

    while (index < count)
        {
        ITYPE* handler = currProv->GetHandler();

        if (handler && (newPriority > handler->GetHandlerPriority()))
            break;

        currProv++;
        index++;
        }

    this->m_entries.insert (this->m_entries.begin()+index, Entry (handlerToAdd));
    }
};

END_BENTLEY_DGN_NAMESPACE
