//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCEvent.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCEvent
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCEvent.h>
#include <Imagepp/all/h/HFCException.h>

//:Ignore
#ifdef _WIN32
HFCEvent::InternalKey* HFCEvent::s_pInternalKey = 0;
static struct ImagePP::EventKeyCreator
    {
    EventKeyCreator()
        {
        if (HFCEvent::s_pInternalKey == 0)
            HFCEvent::s_pInternalKey = new HFCEvent::InternalKey;
        }

    // NO DESTRUCTOR!!!!
    // This will cause a leak, but this is necessary to support static event keys.
    // We cannot control the order of deletion of static objects.
    } s_EventKeyCreator;
#endif
//:End Ignore

/**----------------------------------------------------------------------------
 Constructor that creates a new unnamed event. The constructor creates a
 new event object with its initial state signaled or not depending on the
 @r{pi_Signaled} parameter and with an automatic or manual state management
 depending on the @r{pi_ManualReset} parameter.

 @i{Note}: copy constructor and assignment operators are disabled for this class.

 @param pi_ManualReset Specifies if the state of the event is a manually
                       reset or not.
 @param pi_Signaled    Specifies if the event is initially signaled or not.

 @see Open
-----------------------------------------------------------------------------*/
HFCEvent::HFCEvent(bool pi_ManualReset,
                   bool pi_Signaled)
    : HFCSynchro(),
      m_ManualReset(pi_ManualReset),
      m_InitialState(pi_Signaled)
    {
#ifdef _WIN32
    if (s_pInternalKey == 0)
        s_pInternalKey = new InternalKey;
#endif

    // since this is a named Event, create the handle now
    if (GetHandle() == 0)
        throw HFCUnknownException();
    }


/**----------------------------------------------------------------------------
 Constructor that creates an event. The constructor creates a
 new event object with its initial state signaled or not depending on the
 @r{pi_Signaled} parameter and with an automatic or manual state management
 depending on the @r{pi_ManualReset} parameter.

 The event will be named.  This allows for opening the same semaphore
 from another thread or process that does not have access to this
 HFCEvent instance, through the static method Open.

 @i{Note}: copy constructor and assignment operators are disabled for this class.

 @param pi_rName       A constant reference to the name to give to the
                       new event.
 @param pi_ManualReset Specifies if the state of the event is a manually
                       reset or not.
 @param pi_Signaled    Specifies if the event is initially signaled or not.

 @see Open
-----------------------------------------------------------------------------*/
HFCEvent::HFCEvent(const WString& pi_rName,
                   bool          pi_ManualReset,
                   bool          pi_Signaled)
    : HFCSynchro(),
      m_ManualReset(pi_ManualReset),
      m_InitialState(pi_Signaled),
      m_Name(pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());

#ifdef _WIN32
    if (s_pInternalKey == 0)
        s_pInternalKey = new InternalKey;
#endif

    // since this is a named Event, create the handle now
    if (GetHandle() == 0)
        throw HFCCannotCreateSynchroObjException(HFCCannotCreateSynchroObjException::EVENT);
    }


/**----------------------------------------------------------------------------
 Destructor.  It deletes the Event.
-----------------------------------------------------------------------------*/
HFCEvent::~HFCEvent()
    {
#ifdef _WIN32
    if (m_pHandle != 0)
        CloseHandle(*m_pHandle);
#endif
    }


/**----------------------------------------------------------------------------
 @b{Static method}. It allocates a new instance of the HFCEvent class,
 which will handle an existent event, using the specified name to find
 it.

 The open method enables multiple processes to open instances of the
 same event object. The function succeeds only if some process has
 already created the event by using the named constructor. The calling
 process can use the returned object in any function that requires an
 event object.

 The event object is allocated on the heap and must be destroyed
 manually when no longer required.

 @param pi_rName A constant reference to the string that contains the name
                 of the event to open.

 @return A pointer to a new event object, or NULL if the requested event
         cannot be found.
-----------------------------------------------------------------------------*/
HFCEvent* HFCEvent::Open(const WString& pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());
    HAutoPtr<HFCEvent> pResult;

#ifdef _WIN32
    HFCHandle Handle;

    // Retrieve the handle associated with the name
    Handle = OpenEventW(EVENT_ALL_ACCESS, true, pi_rName.c_str());

    if (Handle != NULL)
        {
        // Create a new object on top of the handle
        pResult = new HFCEvent;
        pResult->m_pHandle = new HFCHandle(Handle);
        }
#endif

    return pResult.release();
    }


/**----------------------------------------------------------------------------
 Protected method.  Returns the handle to the actual OS synchronization
 object.  Needed to implement a abstract class for multiple object
 waiting.

 The actual creation of the OS handle can be delayed until the
 first call to this method.  This prevents any system call, which may be
 costly, in the constructor of a derived class, delaying the creation OS
 handle until the last moment.

 @return The handle to the actual synchronization object.

 @see HFCSynchro
 @see HFCSynchro::WaitUntilSignaled
 @see HFCSynchro::WaitForMultipleObjects
-----------------------------------------------------------------------------*/
HFCHandle HFCEvent::GetHandle() const
    {
    // A mutable of the m_pHandle member would not be logical so we const_cast
    // the class to its non-const form.
    HFCEvent* pThis = const_cast<HFCEvent*>(this);

    // If the handle does not exists already, we must create the OS object
    if (m_pHandle == 0)
        {
#ifdef _WIN32
        try
            {
            s_pInternalKey->ClaimKey();
            if (m_pHandle == 0)
                {
                pThis->m_pHandle = new HFCHandle;

                if (m_Name.empty())
                    {
                    *(pThis->m_pHandle) = CreateEvent(NULL,
                                                      m_ManualReset,
                                                      m_InitialState,
                                                      NULL);
                    }
                else
                    {
                    SECURITY_ATTRIBUTES sa;
                    SECURITY_DESCRIPTOR sd;

                    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
                    sa.bInheritHandle       = true;
                    sa.lpSecurityDescriptor = &sd;

                    // Add a NULL DACL to the security descriptor, so that processes running under
                    // a different user can access the named event
                    if (::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) &&
                        ::SetSecurityDescriptorDacl   (&sd, true, (PACL)NULL, false))
                        {
                        *(pThis->m_pHandle) = CreateEventW(&sa,
                                                          m_ManualReset,
                                                          m_InitialState,
                                                          m_Name.c_str());
                        }
                    }
                }
            s_pInternalKey->ReleaseKey();
            }
        catch(...)
            {
            // to avoid corruption of the static key in case something goes wrong.
            s_pInternalKey->ReleaseKey();
            }
#endif
        }

    return *m_pHandle;
    }
