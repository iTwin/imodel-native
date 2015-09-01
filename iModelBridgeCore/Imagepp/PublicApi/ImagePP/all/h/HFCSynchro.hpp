//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSynchro.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HFCSynchro
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 The constructor for this class.

 The copy constructor and assignment operators are disabled for this class.

 @inheritance In derived classes, the internal resources needed to
              operate the synchronization object may be allocated in the
              constructor, but they can be allocated later for economy
              purposes.  The protected virtual method GetHandle is
              provided for the latter situation.It must be overridden by
              all derived classes, in order to provide the handle to the
              OS handle; it must return the internally stored handle,
              allocating it if necessary.  That method is used by the
              wait methods.
-----------------------------------------------------------------------------*/
inline HFCSynchro::HFCSynchro()
    {
    }


/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
inline HFCSynchro::~HFCSynchro()
    {
//:> Inlined even if virtual because it is small and may be inlined
//:> in the destruction of inherited classes
    }


/**----------------------------------------------------------------------------
 Waits until the synchronization object becomes signaled.  If the object
 is not already signaled, the method pauses until the object is signaled.

 @see WaitForMultipleObjects

 @inheritance Cannot be overridden.
-----------------------------------------------------------------------------*/
inline void HFCSynchro::WaitUntilSignaled() const
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != NULL);

    // Wait for an infinite time for the object to be signaled
#ifdef __HMR_DEBUG
    while (!WaitUntilSignaled(100))
        ;
#else
    WaitForSingleObject(GetHandle(), INFINITE);
#endif
#endif
    }


/**----------------------------------------------------------------------------
 Waits until the synchronization object becomes signaled.  If the object
 is not already signaled, the method pauses until the object is signaled
 or until the time out expires.

 @return true if the object was signaled before the timeout expired.

 @see WaitForMultipleObjects
-----------------------------------------------------------------------------*/
inline bool HFCSynchro::WaitUntilSignaled(uint32_t pi_TimeOut) const
    {
    HPRECONDITION(pi_TimeOut >= 0);

#ifdef _WIN32
    HPRECONDITION(GetHandle() != NULL);

    // Wait until the timeout expires or that the object is signaled
    // by another thread.
    return (WaitForSingleObject(GetHandle(), pi_TimeOut) == WAIT_OBJECT_0);
#endif
    }


/**----------------------------------------------------------------------------
 Static method. Waits until a one or all of the objects in the container become
 signaled.

 @param pi_rSynchros Constant reference to a list of pointers to
                     synchronization objects to wait for.

 @param pi_WaitAll   If set to true this method will wait until all specified
                     synchronization objects are signaled, otherwise it will
                     return as soon as one is signaled.

 @return The index, into the specified list, of the semaphore that was
         signaled.  If @r{pi_WaitAll} was set, the returned value will be
         @k{HFC_SYNCHRO_ALL}.
-----------------------------------------------------------------------------*/
inline uint32_t HFCSynchro::WaitForMultipleObjects(const HFCSynchroContainer& pi_rSynchros,
                                                 bool                      pi_WaitAll)
    {
#ifdef _WIN32
    HPRECONDITION(pi_rSynchros.m_Synchros.size() > 0);
    HPRECONDITION(pi_rSynchros.m_Synchros.size() < MAXIMUM_WAIT_OBJECTS); // Win32 limitations

    // Wait...
    DWORD APIResult = ::WaitForMultipleObjects((DWORD)pi_rSynchros.m_Synchros.size(),
                                               pi_rSynchros.GetHandleArray(),
                                               pi_WaitAll,
                                               INFINITE);

    // If we wait all, return the ALL value otherwise return the
    // index of the object that caused the return of the API function.
    return (pi_WaitAll ? HFC_SYNCHRO_ALL : APIResult - WAIT_OBJECT_0);
#endif
    }


/**----------------------------------------------------------------------------
 Static method. Waits until a one or all of the objects in the container
 become signaled.

 @param pi_rSynchros Constant reference to a list of pointers to
                     synchronization objects to wait for.

 @param pi_TimeOut   Maximal delay of waiting for a signal, in milliseconds.

 @param pi_WaitAll   If set to true this method will wait until all specified
                     synchronization objects are signaled, otherwise it will
                     return as soon as one is signaled.

 @return The index, into the specified list, of the semaphore that was
         signaled.  If @r{pi_WaitAll} was set, the returned value will be
         @k{HFC_SYNCHRO_ALL}.  If the timeout occurs before the arrival of the
         signal(s), the returned value will be @k{HFC_SYNCHRO_TIMEOUT}.
-----------------------------------------------------------------------------*/
inline uint32_t HFCSynchro::WaitForMultipleObjects(const HFCSynchroContainer& pi_rSynchros,
                                                 uint32_t                   pi_TimeOut,
                                                 bool                      pi_WaitAll)
    {
    HPRECONDITION(pi_TimeOut >= 0);

#ifdef _WIN32
    HPRECONDITION(pi_rSynchros.m_Synchros.size() > 0);
    HPRECONDITION(pi_rSynchros.m_Synchros.size() < MAXIMUM_WAIT_OBJECTS); // Win32 limitations

    // Wait...
    DWORD APIResult = ::WaitForMultipleObjects((DWORD)pi_rSynchros.m_Synchros.size(),
                                               pi_rSynchros.GetHandleArray(),
                                               pi_WaitAll,
                                               pi_TimeOut);

    // Analize the result
    uint32_t Result;
    if (APIResult == WAIT_TIMEOUT)
        Result = HFC_SYNCHRO_TIMEOUT;
    else
        Result = (pi_WaitAll ? HFC_SYNCHRO_ALL : APIResult - WAIT_OBJECT_0);

    return (Result);
#endif
    }


/**----------------------------------------------------------------------------
 The constructor for this class.
-----------------------------------------------------------------------------*/
inline HFCSynchroContainer::HFCSynchroContainer()
    {
    };


/**----------------------------------------------------------------------------
 The copy constructor for this class.
-----------------------------------------------------------------------------*/
inline HFCSynchroContainer::HFCSynchroContainer(const HFCSynchroContainer& pi_rObj)
    {
    m_Synchros = pi_rObj.m_Synchros;
    }


/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
inline HFCSynchroContainer::~HFCSynchroContainer()
    {
    };


/**----------------------------------------------------------------------------
 The assignment operator for this class.
-----------------------------------------------------------------------------*/
inline HFCSynchroContainer&
HFCSynchroContainer::operator=(const HFCSynchroContainer& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Synchros = pi_rObj.m_Synchros;
        }

    return *this;
    }


/**----------------------------------------------------------------------------
 Adds a reference to a synchronization object to this container.
-----------------------------------------------------------------------------*/
inline void HFCSynchroContainer::AddSynchro(const HFCSynchro* pi_pSynchro)
    {
    // push_back into the vector
    m_Synchros.push_back(pi_pSynchro);

#ifdef _WIN32
    // invalidate the current handles array
    m_pHandles = 0;
#endif
    }


/**----------------------------------------------------------------------------
 Removes a reference to a synchronization object from this container.
-----------------------------------------------------------------------------*/
inline void HFCSynchroContainer::RemoveSynchro(const HFCSynchro* pi_pSynchro)
    {
    // push_back into the vector
    m_Synchros.erase(find(m_Synchros.begin(), m_Synchros.end(), pi_pSynchro));

#ifdef _WIN32
    // invalidate the current handles array
    m_pHandles = 0;
#endif
    }

/**----------------------------------------------------------------------------
 Returns the number of synchronization objects refered by this container.
-----------------------------------------------------------------------------*/
inline size_t HFCSynchroContainer::CountObjects() const
    {
    return m_Synchros.size();
    }


/**----------------------------------------------------------------------------
 Get the synchronization object reference at the given index in this container.
-----------------------------------------------------------------------------*/
inline const HFCSynchro* HFCSynchroContainer::GetObject(size_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountObjects());

    return m_Synchros[pi_Index];
    }



#ifdef _WIN32
//-----------------------------------------------------------------------------
inline const HFCHandle* HFCSynchroContainer::GetHandleArray() const
    {
    // if the handles array does not exist, create it
    if (m_pHandles == 0)
        {
        // create the array of HANDLES
        m_pHandles = new HANDLE[m_Synchros.size()];

        // fill it with the HANDLE of each synchro object included
        // in this container
        uint32_t Index = 0;
        vector<const HFCSynchro*>::const_iterator Itr = m_Synchros.begin();
        while (Itr != m_Synchros.end())
            {
            m_pHandles[Index] = (*Itr)->GetHandle();
            Itr++;
            Index++;
            }

        }

    return (m_pHandles.get());
    }
#endif
END_IMAGEPP_NAMESPACE