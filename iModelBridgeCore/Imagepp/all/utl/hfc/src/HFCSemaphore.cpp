//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCSemaphore.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCSemaphore
//-----------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCSemaphore.h>
#include <Imagepp/all/h/HFCException.h>

//:Ignore
#ifdef _WIN32
HFCSemaphore::InternalKey* HFCSemaphore::s_pInternalKey = 0;
static struct ImagePP::EventKeyCreator
    {
    EventKeyCreator()
        {
        if (HFCSemaphore::s_pInternalKey == 0)
            HFCSemaphore::s_pInternalKey = new HFCSemaphore::InternalKey;
        }

    // NO DESTRUCTOR!!!!
    // This will cause a leak, but this is necessary to support static event keys.
    // We cannot control the order of deletion of static objects.
    } s_EventKeyCreator;

#endif
//:End Ignore

/**----------------------------------------------------------------------------
 A constructor for this class.  It creates a new unnamed semaphore object.
 Upon creation of a new semaphore, the state is set to not signaled.

 @param pi_InitCount The initial count of the semaphore. This value must be
                     greater than or equal to zero and less than or equal
                     to @r{pi_MaxCount}.
 @param pi_MaxCount  Specifies the maximum count for the semaphore. This
                     value must be greater than 0.

 @note copy constructor and assignment operators are disabled for this class.
-----------------------------------------------------------------------------*/
HFCSemaphore::HFCSemaphore(int32_t pi_InitCount,
                           int32_t pi_MaxCount)
    : HFCSynchro(),
      m_InitCount(pi_InitCount),
      m_MaxCount (pi_MaxCount)
    {
#ifdef _WIN32
    if (s_pInternalKey == 0)
        s_pInternalKey = new InternalKey;
#endif

    // since this is a named semaphore, create the handle now
    if (GetHandle() == 0)
        throw HFCUnknownException();
    }


/**----------------------------------------------------------------------------
 A constructor for this class. It creates a named semaphore. Upon creation of a
 new semaphore, the state is set to not signaled.

 @param pi_rName     A constant reference to a string that contains the name
                     to give to the semaphore.
 @param pi_InitCount The initial count of the semaphore. This value must be
                     greater than or equal to zero and less than or equal
                     to @r{pi_MaxCount}.
 @param pi_MaxCount  Specifies the maximum count for the semaphore. This
                     value must be greater than 0.

 @note copy constructor and assignment operators are disabled for this class.
-----------------------------------------------------------------------------*/
HFCSemaphore::HFCSemaphore(const WString& pi_rName,
                           int32_t       pi_InitCount,
                           int32_t       pi_MaxCount)
    : HFCSynchro(),
      m_Name(pi_rName),
      m_InitCount(pi_InitCount),
      m_MaxCount (pi_MaxCount)
    {
    HPRECONDITION(!pi_rName.empty());

#ifdef _WIN32
    if (s_pInternalKey == 0)
        s_pInternalKey = new InternalKey;
#endif

    // since this is a named semaphore, create the handle now
    if (GetHandle() == 0)
        throw HFCCannotCreateSynchroObjException(HFCCannotCreateSynchroObjException::SEMAPHORE);
    }


/**----------------------------------------------------------------------------
 The destructor for this class.  It deletes the semaphore, excepted if this
 object maps to an existent semaphore created elsewhere (if this object has
 been created by the @k{Open} static method).
-----------------------------------------------------------------------------*/
HFCSemaphore::~HFCSemaphore()
    {
#ifdef _WIN32
    if (m_pHandle != 0)
        CloseHandle(*m_pHandle);
#endif
    }


/**----------------------------------------------------------------------------
 @b{Static method}.   It allocates a new instance of the HFCSemaphore class,
 which will handle an existent semaphore, using the specified name to
 find it.

 The open method enables multiple processes to open instances of
 the same semaphore object. The function succeeds only if some process
 has already created the semaphore by using the named constructor. The
 calling process can use the returned object in any function that
 requires a semaphore object.

 The semaphore object is allocated on the heap and must be destroyed
 manually when no longer required.

 @param pi_rName Pointer to a NULL-terminated string that contains the name
                 of the semaphore to open.

 @return A pointer to a new semaphore object, or NULL if the requested
         semaphore cannot be found.
-----------------------------------------------------------------------------*/
HFCSemaphore* HFCSemaphore::Open(const WString& pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());
    HAutoPtr<HFCSemaphore> pResult;

#ifdef _WIN32
    // Retrieve the handle associated with the name
    HFCHandle Handle = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, true, pi_rName.c_str());
    if (Handle != NULL)
        {
        // Create a new object on top of the handle
        pResult = new HFCSemaphore;
        pResult->m_pHandle = new HFCHandle(Handle);
        }
#endif

    return pResult.release();
    }


/**----------------------------------------------------------------------------
 Overriden method, see ancestor class for details.

 @see HFCSynchro::GetHandle
 @see HFCSynchro
-----------------------------------------------------------------------------*/
inline HFCHandle HFCSemaphore::GetHandle() const
    {
    // A mutable of the m_pHandle member would not be logical so we const_cast
    // the class to its non-const form.
    HFCSemaphore* pThis = const_cast<HFCSemaphore*>(this);

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
                *(pThis->m_pHandle) = CreateSemaphoreW(NULL,
                                                      m_InitCount,
                                                      m_MaxCount,
                                                      (m_Name.empty() ? NULL : m_Name.c_str()));
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
