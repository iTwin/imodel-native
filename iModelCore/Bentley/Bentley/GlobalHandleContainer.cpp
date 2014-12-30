/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/GlobalHandleContainer.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BentleyInternal.h"
#include <vector>
#include <deque>
#include <Bentley/GlobalHandleContainer.h>

/*=================================================================================**//**
* It is not clear that this should be a generic class.  This depends on whether or
* not handles should be type safe and whether or not a handle has to be unique across 
* all types or just for a specific type. 
* @bsiclass                                                     john.gooding    12/2009
+===============+===============+===============+===============+===============+======*/
template <typename T> struct PointerHandles
    {
private:
    struct HandleData
        {
        uint32_t m_handle;
        T*      m_pointer;
        
        HandleData (uint32_t handle, T*pointer) : m_handle (handle), m_pointer (pointer) 
            {
            IncrementUseCount ();  //  Nothing is allowed to have a use count of 0
            }
        
        //  No copy constructor or assignment operator because memberwise copy is the desired behavior
        
        void IncrementUseCount ()
            {
            uint32_t count = (m_handle >> 24) + 1;
            m_handle = (count << 24) | (m_handle & 0xFFFFFF);
            }
            
        uint32_t GetIndex ()
            {
            return m_handle & 0xFFFFFF;
            }

        uint32_t GetHandle ()
            {
            return m_handle;
            }

        T* GetPointer ()
            {
            return m_pointer;
            }

        void SetPointer (T* pointer)
            {
            m_pointer = pointer;
            }
        };
        
    typedef std::vector<HandleData> HandlesContainer_T;
    typedef std::deque<uint32_t> FreeList_T;
    FreeList_T              m_nextFree;
    HandlesContainer_T      m_handles;
    
public:
    PointerHandles () {}
    bool IsHandleValid (uint32_t handle)
        {
        uint32_t index = handle & 0xFFFFFF;
        if (index >= m_handles.size ())
            return false;
            
        return m_handles [index].GetHandle () == handle;
        }
        
    uint32_t AllocateHandle (T*pointer) 
        {
        if (0 == m_nextFree.size ())
            {
            uint32_t handle = static_cast <uint32_t> (m_handles.size ());
            m_handles.push_back (HandleData (handle, pointer));
            return m_handles [handle].GetHandle ();
            }

        uint32_t handle = m_nextFree.front ();
        m_nextFree.pop_front ();
        HandleData& handleData = m_handles [handle];
        handleData.SetPointer (pointer);

        return handleData.GetHandle ();
        }
        
    void ReleaseHandle (uint32_t handle) 
        {
        if (!IsHandleValid (handle))
            return;
            
        uint32_t index = handle & 0xFFFFFF;
        HandleData& handleData = m_handles [index];
        handleData.IncrementUseCount ();
        handleData.SetPointer (NULL);
        m_nextFree.push_back (index);
        }

    T* GetPointer (uint32_t handle)
        {
        if (!IsHandleValid (handle))
            return NULL;
            
        uint32_t index = handle & 0xFFFFFF;
        HandleData& handleData = m_handles [index];
        return handleData.GetPointer ();
        }        
    }; // PointerHandles


static PointerHandles <void>*    s_handles;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool GlobalHandleContainer::IsHandleValid (uint32_t handle)
    {
    if (NULL == s_handles)
        return false;

    return s_handles->IsHandleValid (handle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t GlobalHandleContainer::AllocateHandle (void* pointer)
    {
    if (NULL == s_handles)
        s_handles = new PointerHandles <void>;

    return s_handles->AllocateHandle (pointer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void GlobalHandleContainer::ReleaseHandle (uint32_t handle)
    {
    if (NULL == s_handles)
        return;

    s_handles->ReleaseHandle (handle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void* GlobalHandleContainer::GetPointer (uint32_t handle)
    {
    if (NULL == s_handles)
        return NULL;

    return s_handles->GetPointer (handle);
    }
