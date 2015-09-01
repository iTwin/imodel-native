//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMutex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCMutex
//-----------------------------------------------------------------------------
// Creates or gives access to an event .  Used for multithread/multiprocess
// synchronization purposes.
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCSynchro.h>

BEGIN_IMAGEPP_NAMESPACE

class HFCMutex : public HFCSynchro
    {
public:
    //--------------------------------------
    // Construction - Destruction
    //--------------------------------------

    HFCMutex(bool          pi_Claimed    = false);
    HFCMutex(const WString& pi_rName,
             bool          pi_Claimed    = false);
    virtual         ~HFCMutex();


    //--------------------------------------
    // Open a named event - must have been
    // previously created with the named
    // contructor
    //--------------------------------------

    static HFCMutex*
    Open(const WString& pi_rName);


    //--------------------------------------
    // Synchronization
    //--------------------------------------

    // claims the mutex.  Only calls WaitUntilSignaled, but implemented to
    // be used in a HFCGenericMonitor<class T>, which uses the ClaimKey and
    // ReleaseKey methods of the templated object.
    void            ClaimKey();

    // releases the mutex.
    void            ReleaseKey();


protected:
    //--------------------------------------
    // Methods
    //--------------------------------------

    // Creates the actual object or gives the handle
    // to it.
    virtual HFCHandle
    GetHandle() const;


private:
    //--------------------------------------
    // Methods not implemented
    //--------------------------------------

    HFCMutex(const HFCMutex& pi_rObj);
    HFCMutex& operator=(const HFCMutex&);


    //--------------------------------------
    // Members
    //--------------------------------------

    // Used to create the actual event when GetHandle() is invoked.
    const WString   m_Name;
    const bool     m_Claimed;

    // Handle to the event object
    HAutoPtr<HFCHandle>
    m_pHandle;

    // Used to synchronize the creation of the actual object in GetHandle()
#ifdef _WIN32
    static struct InternalKey
        {
        InternalKey();
        ~InternalKey();
        void ClaimKey();
        void ReleaseKey();
        CRITICAL_SECTION m_Key;
        }* s_pInternalKey;

    friend struct EventKeyCreator;
#endif
    };

END_IMAGEPP_NAMESPACE

#include "HFCMutex.hpp"

