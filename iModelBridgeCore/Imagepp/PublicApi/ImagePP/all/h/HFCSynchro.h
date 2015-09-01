//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSynchro.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCSynchro
//-----------------------------------------------------------------------------

#pragma once

#include "HFCHandle.h"

BEGIN_IMAGEPP_NAMESPACE

#define HFC_SYNCHRO_TIMEOUT     (ULONG_MAX    )
#define HFC_SYNCHRO_ALL         (ULONG_MAX - 1)

/**

    This is a companion class for the HFCSynchro class.  It encapsulates a vector
    of pointers to synchronization objects.  This type is used with the method
    @k{WaitForMultipleObjects} to provide a list of objects to wait for their
    signals.

    @see HFCSynchro

*/

class HFCSynchro;

class HFCSynchroContainer
    {
    friend class HFCSynchro;

public:

    HFCSynchroContainer();
    HFCSynchroContainer(const HFCSynchroContainer& pi_rObj);
    virtual         ~HFCSynchroContainer();

    HFCSynchroContainer&
    operator=(const HFCSynchroContainer& pi_rObj);

    void            AddSynchro(const HFCSynchro* pi_pSynchro);
    void            RemoveSynchro(const HFCSynchro* pi_pSynchro);
    size_t          CountObjects() const;
    const HFCSynchro*
    GetObject(size_t pi_Index) const;


private:

    // Builds (if needed) an array of HANDLE for multiple waits
    const HFCHandle*    GetHandleArray() const;

    vector<const HFCSynchro*>           m_Synchros;
    mutable HArrayAutoPtr<HFCHandle>    m_pHandles;
    };


/**

    This abstract class encapsulates the behavior of synchronization objects
    in a multi-thread environment.  A synchronization object can have either
    one of two states: signaled and not signaled.

    Threads that need to access data that is synchronized by a descendant of
    this class must verify if the object is signaled or not.  The
    @k{WaitUntilSignaled} method waits on a single synchronization object.  The
    @k{WaitForMultipleObjects} method waits for a collection of various
    synchronization objects (regardless of the type).

*/

class HNOVTABLEINIT HFCSynchro
    {
    friend class HFCSynchroContainer;

public:

    //:> Construction/Destruction

    HFCSynchro();
    virtual         ~HFCSynchro() = 0;

    //:> Single object synchronization

    void            WaitUntilSignaled() const;
    bool           WaitUntilSignaled(uint32_t pi_TimeOut) const;

    //:> Multiple object synchronization

    static uint32_t WaitForMultipleObjects(const HFCSynchroContainer& pi_rSynchroList,
                                           bool                      pi_WaitAll = true);
    static uint32_t WaitForMultipleObjects(const HFCSynchroContainer& pi_rSynchroList,
                                           uint32_t                   pi_TimeOut,
                                           bool                      pi_WaitAll = true);

protected:

    virtual HFCHandle
    GetHandle() const = 0;

private:

    // Methods not implemented
    HFCSynchro(const HFCSynchro& pi_rObj);
    HFCSynchro& operator=(const HFCSynchro&);
    };

END_IMAGEPP_NAMESPACE

#include "HFCSynchro.hpp"

