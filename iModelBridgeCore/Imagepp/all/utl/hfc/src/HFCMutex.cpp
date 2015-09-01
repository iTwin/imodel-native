//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMutex.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCMutex
//-----------------------------------------------------------------------------


//####################################################
// INCLUDE FILES
//####################################################

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCMutex.h>
#include <Imagepp/all/h/HFCException.h>

#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
HFCMutex::InternalKey* HFCMutex::s_pInternalKey = 0;

static struct ImagePP::EventKeyCreator
    {
    EventKeyCreator()
        {
        if (HFCMutex::s_pInternalKey == 0)
            HFCMutex::s_pInternalKey = new HFCMutex::InternalKey;
        }

    // NO DESTRUCTOR!!!!
    // This will cause a leak, but this is necessary to support static event keys.
    // We cannot control the order of deletion of static objects.
    } s_EventKeyCreator;

#endif


//-----------------------------------------------------------------------------
// Windows Access control utility functions.
//-----------------------------------------------------------------------------
void* BuildRestrictedSD(void* pSD);
void  FreeRestrictedSD(void* ptr);


//-----------------------------------------------------------------------------
// Constructor that creates a new unnamed event.
//-----------------------------------------------------------------------------
HFCMutex::HFCMutex(bool pi_Claimed)
    : HFCSynchro(),
      m_Claimed(pi_Claimed)
    {
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)
    if (s_pInternalKey == 0)
        s_pInternalKey = new InternalKey;
#endif
    }


//-----------------------------------------------------------------------------
// Constructor that creates an event.
//-----------------------------------------------------------------------------
HFCMutex::HFCMutex(const WString& pi_rName,
                   bool          pi_Claimed)
    : HFCSynchro(),
      m_Claimed(pi_Claimed),
      m_Name(pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    if (s_pInternalKey == 0)
        s_pInternalKey = new InternalKey;
#endif

    // since this is a named Event, create the handle now
    if (GetHandle() == 0)
        throw HFCCannotCreateSynchroObjException(HFCCannotCreateSynchroObjException::MUTEX);
    }


//-----------------------------------------------------------------------------
// Destructor.  It deletes the Event.
//-----------------------------------------------------------------------------
HFCMutex::~HFCMutex()
    {
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)
    if (m_pHandle != 0)
        CloseHandle(*m_pHandle);
#endif
    }


//-----------------------------------------------------------------------------
// Static method that constructs an HFCMutex object that will access an
// existent named Event.  Object must be deleted by user.
//-----------------------------------------------------------------------------
HFCMutex* HFCMutex::Open(const WString& pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());
    HAutoPtr<HFCMutex> pResult;

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    // Retrieve the handle associated with the name
    HFCHandle Handle = OpenMutexW(MUTEX_ALL_ACCESS , true, pi_rName.c_str());
    if (Handle != NULL)
        {
        // Create a new object on top of the handle
        pResult = new HFCMutex;
        pResult->m_pHandle = new HFCHandle(Handle);
        }
#endif

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Returns the handle to the actual OS synchro object.  If it does not exist,
// it will be created here.
//-----------------------------------------------------------------------------
HFCHandle HFCMutex::GetHandle() const
    {
    // A mutable of the m_pHandle member would not be logical so we const_cast
    // the class to its non-const form.
    HFCMutex* pThis = const_cast<HFCMutex*>(this);

    // If the handle does not exists already, we must create the OS object
    if (m_pHandle == 0)
        {
#if defined (ANDROID) || defined (__APPLE__)
            //DM-Android
#elif defined (_WIN32)
        try
            {
            s_pInternalKey->ClaimKey();
            if (m_pHandle == 0)
                {
                pThis->m_pHandle = new HFCHandle;

                if (m_Name.empty())
                    {
                    *(pThis->m_pHandle) = CreateMutex(0, m_Claimed, NULL);
                    }
                else
                    {
                    SECURITY_DESCRIPTOR sd;
                    PVOID ptr = BuildRestrictedSD(&sd);
                    if (ptr)
                        {
                        SECURITY_ATTRIBUTES sa;
                        sa.nLength = sizeof(sa);
                        sa.lpSecurityDescriptor = &sd;
                        sa.bInheritHandle = false;

                        *(pThis->m_pHandle) = CreateMutexW(&sa,
                                                          m_Claimed,
                                                          m_Name.c_str());

                        HDEBUGCODE(if (*(pThis->m_pHandle) == 0))
                        HDEBUGCODE({     DWORD LastError = 0; LastError= GetLastError();})  // Assign to 0 to avoid C4189

                            FreeRestrictedSD(ptr);
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



// The following function initializes the supplied security descriptor
// with a DACL that grants everyone MUTEX_ALL_ACCESS access.
//
// The function returns NULL if any of the access control APIs fail.
// Otherwise, it returns a PVOID pointer that should be freed by calling
// FreeRestrictedSD() after the security descriptor has been used to
// create the object.
//
// Code taken directly from Microsoft KB article Q106387

void* BuildRestrictedSD(void* pSD)
    {
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)

    DWORD  dwAclLength;

    PSID   pAuthenticatedUsersSID = NULL;

    PACL   pDACL   = NULL;
    BOOL   bResult = false;

    // Avoid C4189: PACCESS_ALLOWED_ACE pACE = NULL;

    SID_IDENTIFIER_AUTHORITY siaNT = SECURITY_WORLD_SID_AUTHORITY;

    // Avoid C4189: SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

    HDEBUGCODE(DWORD LastError = 0;)    // Assign to 0 to avoid C4189

    __try
        {

        // initialize the security descriptor
        if (!InitializeSecurityDescriptor(pSD,
        SECURITY_DESCRIPTOR_REVISION))
            {
            HDEBUGCODE(LastError = GetLastError();)
            __leave;
            }

        // obtain a sid for the Authenticated Users Group
        if (!AllocateAndInitializeSid(&siaNT, 1,
        SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
        &pAuthenticatedUsersSID))
            {
            HDEBUGCODE(LastError = GetLastError();)
            __leave;
            }

        // NOTE:
        //
        // If access must be restricted to a specific user or group,
        // the SID can be constructed using the
        // LookupAccountSid() API based on a user or group name.

        // calculate the DACL length
        dwAclLength = sizeof(ACL)
        // add space for Authenticated Users group ACE
        + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)
        + GetLengthSid(pAuthenticatedUsersSID);

        // allocate memory for the DACL
        pDACL = (PACL) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        dwAclLength);
        if (!pDACL)
            {
            HDEBUGCODE(LastError = GetLastError();)
            __leave;
            }

        // initialize the DACL
        if (!InitializeAcl(pDACL, dwAclLength, ACL_REVISION))
            {
            HDEBUGCODE(LastError = GetLastError();)
            __leave;
            }

        // add the ACE to the DACL with MUTEX_ALL_ACCESS access
        if (!AddAccessAllowedAce(pDACL, ACL_REVISION,
        MUTEX_ALL_ACCESS,
        pAuthenticatedUsersSID))
            {
            HDEBUGCODE(LastError = GetLastError();)
            __leave;
            }

        // set the DACL in the security descriptor
        if (!SetSecurityDescriptorDacl(pSD, true, pDACL, false))
            {
            HDEBUGCODE(LastError = GetLastError();)
            __leave;
            }

        bResult = true;

        } __finally {

        if (pAuthenticatedUsersSID) FreeSid(pAuthenticatedUsersSID);
        }

    if (bResult == false) {
        if (pDACL) HeapFree(GetProcessHeap(), 0, pDACL);
        pDACL = NULL;
        }

    return (PVOID) pDACL;
#else
    return 0;
#endif
    }

// The following function frees memory allocated in the
// BuildRestrictedSD() function
void FreeRestrictedSD(void* ptr)
    {
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)

    if (ptr) HeapFree(GetProcessHeap(), 0, ptr);

#endif
    return;
    }

